#include "SondeProcessor.h"

#include "../vertrag/NakamaEvidenz.h"
#include "BrokerInstallBinding.h"
#include "PipeToken.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <limits>
#include <process.h>
#include <utility>

namespace nakama::sonde
{

namespace
{
std::string uuidHex32()
{
    std::string roh = juce::Uuid().toString().toStdString();
    std::string aus;
    aus.reserve (32);
    for (char c : roh)
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            aus.push_back (c);
    return aus.size() == 32 ? aus
                            : nakama::ipc::instanceAdresseAusState ("probe-runtime:" + roh);
}

bool istUnicodeWhitespace (juce::juce_wchar c) noexcept
{
    // Dieselbe geschlossene White_Space-Menge wie der C++-v3-Vertragsleser.
    return (c >= 0x0009 && c <= 0x000d) || c == 0x0020 || c == 0x0085
        || c == 0x00a0 || c == 0x1680 || (c >= 0x2000 && c <= 0x200a)
        || c == 0x2028 || c == 0x2029 || c == 0x202f || c == 0x205f
        || c == 0x3000;
}

bool hostBusNamePasst (const juce::String& name) noexcept
{
    if (name.length() < 1 || name.length() > 120)
        return false;

    bool hatNichtWhitespace = false;
    for (auto z = name.getCharPointer(); ! z.isEmpty(); ++z)
    {
        const auto c = *z;
        if ((c >= 0x0000 && c <= 0x001f) || (c >= 0x007f && c <= 0x009f))
            return false;
        hatNichtWhitespace = hatNichtWhitespace || ! istUnicodeWhitespace (c);
    }
    return hatNichtWhitespace;
}

std::uint64_t addiereGesaettigt (std::uint64_t a, std::uint64_t b) noexcept
{
    const auto max = std::numeric_limits<std::uint64_t>::max();
    return b > max - a ? max : a + b;
}

nakama::ipc::ServerErwartung brokerServerErwartung()
{
    return { nakama::ipc::installbindung::brokerPfad,
             nakama::ipc::installbindung::brokerSha256,
             nakama::ipc::installbindung::authenticodeThumbprint };
}

/*  SONDE-015 4a: der Herkunftstag des Parameterabgleichs. Entwurf §44.3:
    "Ein Herkunftstag host|local_ui|remote_transaction|state_restore
    verhindert Listener-/Revisionsschleifen." `setValueNotifyingHost` ruft die
    Listener synchron im selben Thread - ein thread_local Zaehler trennt den
    eigenen Abgleich deshalb sicher von einem Hostereignis, das zugleich auf
    einem anderen Thread eintrifft. */
thread_local int abgleichTiefe = 0;

struct AbgleichHerkunft
{
    AbgleichHerkunft() noexcept  { ++abgleichTiefe; }
    ~AbgleichHerkunft() noexcept { --abgleichTiefe; }
};

/** Die 112 Host-Parameter in Vertragsreihenfolge, aus `parameter::tabelle()`.
    Name = Vertragskennung: eine Anzeigebezeichnung ist Oberflaeche (S31b). */
juce::AudioProcessorValueTreeState::ParameterLayout baueParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    const auto& t = nakama::parameter::tabelle();
    for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
    {
        const auto& b = t[(size_t) i];
        jassert (b.hostParameter);
        const juce::ParameterID id { b.id, 1 };
        switch (b.typ)
        {
            case nakama::parameter::Typ::boolean:
                layout.add (std::make_unique<juce::AudioParameterBool> (id, b.id, b.standardBool));
                break;
            case nakama::parameter::Typ::gleitkomma:
            {
                juce::NormalisableRange<float> bereich ((float) b.min, (float) b.max);
                // Logarithmisch empfundene Groessen: der Vertragsdefault liegt in
                // der Mitte des Automationswegs (Manifest §10.2).
                if (b.id.endsWith (".freq_hz") || b.id.endsWith (".q")
                    || b.id.endsWith (".attack_ms") || b.id.endsWith (".release_ms"))
                    bereich.setSkewForCentre ((float) b.standardZahl);
                layout.add (std::make_unique<juce::AudioParameterFloat> (id, b.id, bereich, (float) b.standardZahl));
                break;
            }
            case nakama::parameter::Typ::aufzaehlung:
                layout.add (std::make_unique<juce::AudioParameterChoice> (id, b.id, b.werte, b.standardIndex));
                break;
        }
    }
    return layout;
}

/** Vertragszelle -> Hostwert (denormiert). */
float hostWertAus (int index, const nakama::parameter::Zelle& z)
{
    switch (nakama::parameter::tabelle()[(size_t) index].typ)
    {
        case nakama::parameter::Typ::boolean:     return z.b ? 1.0f : 0.0f;
        case nakama::parameter::Typ::gleitkomma:  return (float) z.zahl;
        case nakama::parameter::Typ::aufzaehlung: return (float) z.enumIndex;
    }
    return 0.0f;
}
} // namespace

SondeProcessor::SondeProcessor()
    : juce::AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      v3LogonSid (nakama::ipc::aktuelleLogonSid()),
      v3PipeName (nakama::ipc::pipeNameV3 (v3LogonSid)),
      v3RuntimeNonce (uuidHex32()),
      controlV3 ([this] { return v3Hello(); }, v3PipeName, {},
                 [this] { return v3Status(); }, {}, {}, brokerServerErwartung()),
      telemetryV3 ([this] { return v3TelemetryHello(); }, v3PipeName, {},
                   brokerServerErwartung()),
      parameterBaum (*this, nullptr, "NakamaProbeeqParameter", baueParameterLayout())
{
    // SONDE-015 4a: DSP-Kern und Transaktionskern entstehen VOR dem Worker,
    // der ab seinem ersten Takt ihre Pflege uebernimmt.
    dspKern        = std::make_unique<nakama::dsp::DspKern>();
    dspAusfuehrung = std::make_unique<nakama::transaktion::DspKernAusfuehrung> (*dspKern);
    transaktion    = std::make_unique<nakama::transaktion::Transaktionskern> (*dspAusfuehrung);

    // M-119: FL meldet `sample_accurate_automation` = unsupported
    // (identity/host-capabilities-fl-v1.json, Termin B), und ein samplegenauer
    // Pfad ist nicht gebaut. Die Sonde rampt blockweise, und
    // Topologieautomation wirkt nicht (§44.3 letzter Absatz).
    transaktion->setzeSamplegenaueAutomation (false);

    const auto& parameterListe = getParameters();
    jassert (parameterListe.size() == nakama::parameter::kHostParameter);
    for (int i = 0; i < nakama::parameter::kHostParameter && i < parameterListe.size(); ++i)
    {
        hostParameter[(size_t) i] = dynamic_cast<juce::RangedAudioParameter*> (parameterListe[i]);
        hostWert[(size_t) i].store (parameterListe[i]->getValue());
        parameterListe[i]->addListener (this);
    }
    tidHoch = (std::uint64_t) juce::Random::getSystemRandom().nextInt64();

    // Frische Instanz. `frisch()` legt `legacy` an - das ist die Vorgabe des
    // Main-Bundles und fuer eine Sonde falsch: ihr Bundle-Vertrag
    // (Bundle::nkpr/nkac) laesst `legacy` gar nicht zu, ein so gespeicherter
    // Stand kaeme beim naechsten Laden als read-only zurueck. Die
    // Produktklasse ist am Bundle fest (§53.5), also steht sie hier.
    zustand = nakama::state::frisch (juce::Uuid().toString());
    zustand.common.klasse = kProduktklasse;

    // Kind-Matrix des State-Vertrags §2.1: `active_probe` verlangt GENAU EIN
    // Parameters-Kind, `passive_probe` keines. Ohne diese Zeile schrieb
    // Probeeq einen Stand, den es selbst nicht mehr laden konnte - er kam als
    // read-only mit "active_probe requires exactly one Parameters" zurueck.
    // Gefunden vom Passthrough-Beweis (EqCopProbeeqNullTest, Gegenpfad
    // speichern<->laden), nicht im Nachdenken.
    zustand.hatParameters = (kProduktklasse == nakama::state::Klasse::active_probe);

    analyseQueue.vorbereiten();
    workerLaeuft.store (true);
    worker = std::thread ([this] { workerLauf(); });

#if ! defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    // Probeeq ist reiner Connector: beide v3-Verbindungen, kein Lifecycle und
    // kein Spawn. Ohne persistierte Projektbindung lehnt der ControlClient
    // schon seine lokale Adresse ab und beruehrt die Produktionspipe nicht.
    controlV3.start();
    telemetryV3.start();
#endif
}

SondeProcessor::~SondeProcessor()
{
    // Gegenstueck zu addListener im Konstruktor: kein Parameterereignis und
    // keine Geste erreicht mehr Kern oder Transaktionskern, waehrend sie
    // abgebaut werden.
    for (auto* p : getParameters())
        p->removeListener (this);

    workerLaeuft.store (false);
    workerWarte.notify_all();
    if (worker.joinable())
        worker.join();
    telemetryV3.stop();
    controlV3.stop();
}

void SondeProcessor::prepareToPlay (double samplerate, int maxBlock)
{
    // Der Nachrichtenthread fordert nur eine neue Queue-Generation an. Alle
    // Engine-Resets und jede Allokation bleiben beim Analyseworker.
    jassert (getLatencySamples() == 0);
    const double sichereRate = std::isfinite (samplerate) && samplerate > 0.0
                            && samplerate <= 768000.0 ? samplerate : 0.0;
    {
        std::lock_guard<std::mutex> l (analyseSchloss);
        v3Samplerate.store (sichereRate);
        analyseQueue.neustartAnfordern();
    }
    v3BlockSize.store (maxBlock >= 0 ? maxBlock : 0);
    v3Channels.store (getTotalNumInputChannels());

    // SONDE-015 4a: der DSP-Kern wird hier vorallokiert (M-41) - unter dem
    // Callback-Schloss, damit kein Block in die neuen Puffer faellt, und unter
    // dem Zustandsschloss, weil der Worker dieselben Baenke pflegt. Danach
    // bekommt er den wirksamen Zustand; die Bank wird erst am ersten Block
    // aktiv (M-08, M-09).
    if (sichereRate > 0.0 && maxBlock > 0)
    {
        const juce::ScopedLock callback (getCallbackLock());
        const juce::ScopedLock l (zustandSchloss);
        dspKern->bereiteVor (sichereRate, maxBlock);
        analyseL.assign ((size_t) maxBlock, 0.0f);
        analyseR.assign ((size_t) maxBlock, 0.0f);
        transaktion->setzeSamplerate (sichereRate);
        dspAusfuehrung->vergissLetztePublikation();
        publikationOffen = ! dspAusfuehrung->publiziereWirksam (transaktion->wirksam(), true);
        if (transaktion->preview().aktiv && ! dspAusfuehrung->publizierePreview (transaktion->preview().satz))
            publikationOffen = true;
    }

    workerWarte.notify_all();
    controlV3.reconnect();
    telemetryV3.reconnect();
}

void SondeProcessor::releaseResources()
{
    // Gegenstueck zu prepareToPlay (Beziehungen mitpruefen): der Kern gibt
    // seine Blockpuffer frei und setzt seine Baenke zurueck. Ein Block ohne
    // neue Vorbereitung laeuft dann unberuehrt durch (`verarbeiteStueck`).
    // Der bestaetigte Zustand bleibt im Transaktionskern; die naechste
    // Vorbereitung publiziert ihn wieder.
    const juce::ScopedLock callback (getCallbackLock());
    const juce::ScopedLock l (zustandSchloss);
    dspKern->freigeben();
    dspAusfuehrung->vergissLetztePublikation();
    publikationOffen = false;
}

bool SondeProcessor::isBusesLayoutSupported (const BusesLayout& layout) const
{
    // Ein Passthrough, der Kanaele erfinden oder verschlucken muesste, waere
    // kein Passthrough. Deshalb nur gleiche Ein-/Ausgangsbelegung, und keine
    // deaktivierten Hauptbusse.
    const auto ein = layout.getMainInputChannelSet();
    const auto aus = layout.getMainOutputChannelSet();

    if (ein.isDisabled() || aus.isDisabled())
        return false;

    return ein == aus;
}

void SondeProcessor::processBlock (juce::AudioBuffer<float>& puffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals keineDenormals;
    const int kanaele = std::min (puffer.getNumChannels(), 2);
    const int samples = puffer.getNumSamples();

    if (samples > 0 && kanaele > 0)
    {
        // Genau wie bei Gen gewinnt die gepatchte Hostbruecke. Der oeffentliche
        // Playhead ist nur der ehrliche Rueckfall fuer headless/andere Wrapper.
        nakama::echtzeit::Stempel stempel;
        if (brueckeStand.frisch)
        {
            stempel = brueckeStand.stempel;
            brueckeStand.frisch = false;
        }
        else if (auto* kopf = getPlayHead())
        {
            if (const auto pos = kopf->getPosition())
            {
                stempel.kontextAnwesend = true;
                stempel.spieltGueltig = true;
                stempel.spielt = pos->getIsPlaying();
                if (const auto zeit = pos->getTimeInSamples())
                {
                    stempel.zeitGueltig = true;
                    stempel.projectSampleStart = *zeit;
                }
                if (const auto ppq = pos->getPpqPosition())
                {
                    stempel.ppqGueltig = true;
                    stempel.ppqPosition = *ppq;
                }
                if (const auto bpm = pos->getBpm())
                {
                    stempel.tempoGueltig = true;
                    stempel.tempo = *bpm;
                }
                stempel.cycleAktiv = pos->getIsLooping();
                if (const auto schleife = pos->getLoopPoints())
                {
                    stempel.cycleGrenzenGueltig = true;
                    stempel.cycleStartPpq = schleife->ppqStart;
                    stempel.cycleEndePpq = schleife->ppqEnd;
                }
            }
        }
        stempel.nichtEchtzeit = isNonRealtime();
        if (! stempel.sampleRateGueltig)
        {
            const auto rate = v3Samplerate.load();
            if (rate > 0.0)
            {
                stempel.sampleRateGueltig = true;
                stempel.sampleRate = rate;
            }
        }

        // SONDE-015 4a: der aktive Kern (Manifest §3.0). Bei `eq_enabled` aus
        // und beim Hard-Bypass schreibt er keinen Sample und sanitisiert nichts
        // (M-01, M-05, M-50); ein neues Programm uebernimmt er am Blockrand.
        float* kanalZeiger[2] = { puffer.getWritePointer (0),
                                  kanaele > 1 ? puffer.getWritePointer (1) : nullptr };
        dspKern->verarbeite (kanalZeiger, kanaele, samples);
        verarbeiteteSamples.fetch_add ((std::uint64_t) samples, std::memory_order_relaxed);

        // Die Analyse misst `post_committed` (§44.2: Session-Landkarte und
        // Recall beziehen sich darauf), nie den Hoermatrix-Ausgang (M-57).
        AnalyseQueue::TapQuelle abgriff;
        const double* tapL = dspKern->tap (nakama::dsp::Tap::postCommitted, 0);
        const double* tapR = dspKern->tap (nakama::dsp::Tap::postCommitted, 1);
        if (tapL != nullptr && tapR != nullptr && dspKern->tapLaenge() == samples
            && (size_t) samples <= analyseL.size())
        {
            for (int i = 0; i < samples; ++i) analyseL[(size_t) i] = (float) tapL[i];
            if (kanaele > 1)
                for (int i = 0; i < samples; ++i) analyseR[(size_t) i] = (float) tapR[i];
            abgriff.links  = analyseL.data();
            abgriff.rechts = kanaele > 1 ? analyseR.data() : nullptr;
            // Ganz oder gar nicht. Rueckstau verwirft Analyse, niemals Audio.
            analyseQueue.veroeffentliche (&abgriff, 1, kanaele, samples, stempel);
        }
        else if (committedRuhtImPassthrough())
        {
            // Ohne gueltigen Tap (unvorbereitet oder uebergrosser Block) ist der
            // ruhende Passthrough-Ausgang der unberuehrte Eingang: die Analyse
            // liest ihn wie bisher. Rechnet der Kern, ist der Tap dieses Blocks
            // verworfen und gezaehlt (M-48) - die Analyse faellt, nie Audio.
            abgriff.links  = puffer.getReadPointer (0);
            abgriff.rechts = kanaele > 1 ? puffer.getReadPointer (1) : nullptr;
            analyseQueue.veroeffentliche (&abgriff, 1, kanaele, samples, stempel);
        }
    }

    // Bei der erlaubten gleichen Busbelegung laeuft diese
    // JUCE-Sicherheitsschleife null Mal.
    for (int k = getTotalNumInputChannels(); k < getTotalNumOutputChannels(); ++k)
        puffer.clear (k, 0, puffer.getNumSamples());
}

void SondeProcessor::nakamaBlockEmpfangen (
    const eqcop::hostbruecke::Blockbefund& befund) noexcept
{
    const auto& k = befund.kontext;
    auto& s = brueckeStand.stempel;
    s = nakama::echtzeit::Stempel {};
    s.kontextAnwesend = k.processContextPresent;
    s.zeitGueltig = k.processContextPresent && k.projectTimeSamples.gueltig;
    s.projectSampleStart = (std::int64_t) k.projectTimeSamples.oder (0);
    s.spieltGueltig = k.processContextPresent && k.playing.gueltig;
    s.spielt = k.playing.oder (false);
    s.recordingGueltig = k.processContextPresent && k.recording.gueltig;
    s.recording = k.recording.oder (false);
    s.continuousGueltig = k.processContextPresent && k.continuousTimeSamples.gueltig;
    s.continuousTimeSamples = (std::int64_t) k.continuousTimeSamples.oder (0);
    s.tempoGueltig = k.processContextPresent && k.tempo.gueltig;
    s.tempo = k.tempo.oder (0.0);
    s.ppqGueltig = k.processContextPresent && k.ppqPosition.gueltig;
    s.ppqPosition = k.ppqPosition.oder (0.0);
    s.sampleRateGueltig = k.processContextPresent && k.sampleRate.gueltig;
    s.sampleRate = k.sampleRate.oder (0.0);
    s.cycleAktiv = k.processContextPresent && k.cycle.aktiv;
    s.cycleGrenzenGueltig = k.processContextPresent && k.cycle.gueltig;
    s.cycleStartPpq = k.cycle.startPpq;
    s.cycleEndePpq = k.cycle.endePpq;
    const auto& ein = k.presentationLatency.hole (true, 0);
    const auto& aus = k.presentationLatency.hole (false, 0);
    s.eingangLatenzGemeldet = ein.gemeldet;
    s.eingangLatenzSamples = ein.samples;
    s.ausgangLatenzGemeldet = aus.gemeldet;
    s.ausgangLatenzSamples = aus.samples;
    brueckeStand.frisch = true;
}

void SondeProcessor::updateTrackProperties (const TrackProperties& eigenschaften)
{
    auto* const messageManager = juce::MessageManager::getInstanceWithoutCreating();
    const bool aufMessageThread = messageManager != nullptr
        && messageManager->isThisTheMessageThread();
#if defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    hostCallbackAufMessageThread.store (aufMessageThread);
#endif
    jassert (aufMessageThread);
    if (! aufMessageThread)
        return;

    bool gemeldet = false;
    std::string name;
    if (eigenschaften.name.has_value() && hostBusNamePasst (*eigenschaften.name))
    {
        gemeldet = true;
        name = eigenschaften.name->toStdString();
    }

    const juce::ScopedLock l (hostKontextSchloss);
    hostBusNameGemeldet = gemeldet;
    hostBusName = gemeldet ? std::move (name) : std::string {};
    // Der persistente `zustand.common.label` wird absichtlich nicht beruehrt.
}

void SondeProcessor::producerStandLeeren() noexcept
{
    letzterProducerFrame = nakama::analyse::FeatureFrame {};
    letzterProducerFrameVorhanden = false;
}

void SondeProcessor::workerLauf()
{
    juce::ScopedNoDenormals keineDenormals;
    quarantaene.vorbereiten();
    auto workerAnlauf = analyseQueue.aktuellerAnlauf();

    double naechsterKontrollTakt = juce::Time::getMillisecondCounterHiRes();
    while (workerLaeuft.load())
    {
        // SONDE-015 4a: der Takt des Control-Workers - ACKs ernten,
        // Hostautomation uebernehmen, den wirksamen Zustand publizieren. Er
        // nimmt das Zustandsschloss und NIE zugleich das Analyseschloss.
        if (juce::Time::getMillisecondCounterHiRes() >= naechsterKontrollTakt)
        {
            dspKontrollTakt();
            naechsterKontrollTakt = juce::Time::getMillisecondCounterHiRes() + 5.0;
        }

        bool queueHatRest = false;
        {
            std::lock_guard<std::mutex> steuerung (analyseSchloss);
            const double rate = v3Samplerate.load();
            const bool rateGueltig = rate > 0.0;
            if (rateGueltig)
                analyseEngine.vorbereiten (rate);
            merkmale.vorbereiten (rate);

            const auto anlauf = analyseQueue.aktuellerAnlauf();
            if (anlauf != workerAnlauf)
            {
                // Auch same-rate prepare ist eine neue Messreihe. Beide
                // Engines und die Ein-Block-Quarantaene beginnen gemeinsam.
                analyseEngine.zuruecksetzen();
                merkmale.zuruecksetzen();
                quarantaene.zuruecksetzen();
                producerStandLeeren();
                workerAnlauf = anlauf;
            }

            constexpr int kMaxBloeckeJeZug = 8;
            for (int i = 0; i < kMaxBloeckeJeZug; ++i)
            {
                const auto* roh = analyseQueue.spitze();
                if (roh == nullptr)
                    break;
                if (roh->startFolge != analyseQueue.aktuellerAnlauf())
                {
                    quarantaene.zuruecksetzen();
                    analyseQueue.freigeben();
                    continue;
                }

                const auto bruecheVorher = quarantaene.kontinuitaetsbrueche();
                const auto frei = quarantaene.schiebe (analyseQueue, *roh);
                analyseQueue.freigeben();
                if (quarantaene.kontinuitaetsbrueche() != bruecheVorher)
                {
                    // 🔑 NAK-181 R7 (G4-Befund C3, M-53): kein alter
                    // K-/Loudness-Zustand darf die Luecke ueberbruecken — die
                    // FeatureEngine erkennt die Grenze am naechsten
                    // freigegebenen Deskriptor aber SELBST.
                    //
                    // Bis hierher stand hier zusaetzlich
                    // `merkmale.zuruecksetzen()`. Das setzt `habeVorigen` auf
                    // false und Epoche wie Segment auf 0; der naechste Block
                    // liefert damit `Grenzgrund::keine`, `grenzeZiehen` laeuft
                    // nie, und der Stempel traegt 0/0 — der Bruch war auf dem
                    // Draht UNSICHTBAR, und stand die Engine vorher ueber 0,
                    // meldete der Broker `Epochwechsel` auch fuer einen Drop
                    // (M-53 verlangt dort `sequenzluecke`).
                    //
                    // Gen macht es an derselben Stelle seit jeher richtig
                    // (`PluginProcessor.cpp:904-912`). `grenzeZiehen` leert
                    // dieselben Fenster wie der Reset UND zieht Epoche
                    // beziehungsweise Segment hoch; der Ereignisring bleibt
                    // dabei bewusst stehen, weil jedes Ereignis seine Epoche
                    // mittraegt (`FeatureEngine.h:3619-3622`).
                    analyseEngine.zuruecksetzen();
                    producerStandLeeren();
                }
                if (! frei || ! rateGueltig)
                    continue;

                const auto grenzenVorher = merkmale.getrennteFenster();
                const auto straddlesVorher = merkmale.straddleVerworfen();
                const bool frameBereit = merkmale.nimmBlock (*frei.block, frei.audio);
                const bool grenze = merkmale.getrennteFenster() != grenzenVorher;
                const bool blockVerworfen = merkmale.straddleVerworfen() != straddlesVorher;
                if (grenze)
                {
                    analyseEngine.zuruecksetzen();
                    producerStandLeeren();
                }
                if (! blockVerworfen)
                    analyseEngine.verarbeite (frei.audio, (int) frei.block->sampleCount,
                                               (int) frei.block->kanaele);

                if (! frameBereit)
                    continue;

                auto frame = merkmale.frame();
                const auto lautheit = analyseEngine.lautheitFuerTelemetrie();
                if (lautheit.paar)
                {
                    frame.lufsIGesetzt = true;
                    frame.lufsI = lautheit.lufsI;
                    frame.lufsIUnsicherheitGesetzt = true;
                    frame.lufsIUnsicherheit = lautheit.unsicherheitLu;

                    // SONDE-013 M-03: PLR = Passage-True-Peak-Maximum minus
                    // LUFS-I (§39.1). Die zwei Haelften entstehen an zwei
                    // Orten — der True Peak in der `FeatureEngine`, die
                    // integrierte Lautheit im `LoudnessAccumulator` —, und
                    // GENAU HIER treffen sie sich, an derselben Stelle, an
                    // der auch das Lautheitspaar zugemischt wird. PLR ohne
                    // das Paar gibt es nicht: beide Leser lehnen es als
                    // `plr_ohne_lufs_i` ab, weil die Zahl sonst gegen einen
                    // Bezugspunkt gerechnet waere, den der Frame nicht
                    // mitschickt.
                    if (frame.truePeakPassageGesetzt)
                    {
                        frame.plrGesetzt = true;
                        frame.plrDb = frame.truePeakPassageDb - lautheit.lufsI;
                    }
                }
                else
                {
                    frame.lufsIStatusGesetzt = true;
                    frame.lufsIStatus = lautheit.status;
                }

                if (telemetryV3.veroeffentlichen (frame, v3Hello().adresse))
                {
                    letzterProducerFrame = frame;
                    letzterProducerFrameVorhanden = true;
                    producerPublikationen.fetch_add (1);
                }

                // SONDE-013 M-05: der Evidenzsnapshot geht ueber P1 auf dem
                // Controllink, nicht ueber P2. §33.2 trennt beide Kadenzen
                // ausdruecklich — der Liveframe ist 10 Hz und binaer, die
                // Evidenz 1 bis 4 Hz und JSON.
                if (frame.evidenzFrisch)
                    evidenzSnapshotSenden (frame);
            }
            queueHatRest = analyseQueue.spitze() != nullptr;
        }

        if (queueHatRest)
        {
            std::this_thread::yield();
            continue;
        }
        std::unique_lock<std::mutex> l (workerWarteSchloss);
        workerWarte.wait_for (l, std::chrono::milliseconds (20),
                              [this] { return ! workerLaeuft.load(); });
    }
}

/*  SONDE-013 M-05: einen Evidenzsnapshot bauen, senden und die Kadenz
    nachfuehren.

    Laeuft im Analyseworker unter `analyseSchloss` — nie im Audiothread. Der
    Weg ist bewusst in dieser Reihenfolge:

      1. Ereignisse dieses Fensters aus dem Ring holen und den Verlust seit
         dem letzten Snapshot als DIFFERENZ bilden;
      2. Snapshot bauen (der Bauer ist fail-closed und lehnt einen
         widerspruechlichen Frame ab);
      3. senden und aus dem Ergebnis die Kadenz nachfuehren;
      4. den Ring leeren — ERST danach, denn ein nicht gebauter Snapshot
         darf die Ereignisse nicht verschlucken.

    Zur Kadenz: bei Rueckstau steigt der Abstand Richtung 1 Hz, sonst faellt
    er Richtung 4 Hz. Reduziert wird die KADENZ, nie der Inhalt — ein
    uebersprungener Snapshot wuerde sein Fenster trotzdem leeren und die
    Messung wegwerfen.

    Zur Konfidenzklasse: eine Sonde kann `stark` NICHT ehrlich behaupten.
    §34.3 begrenzt die Gesamtklasse an harten Maengeln bei Session, Passage,
    Coverage oder Alignment; von diesen vier kennt die Sonde nur die
    Coverage. Sie meldet deshalb hoechstens `mittel` und ueberlaesst dem
    Broker, weiter herabzustufen — nach oben korrigiert dort niemand.  */
void SondeProcessor::evidenzSnapshotSenden (const nakama::analyse::FeatureFrame& frame)
{
    // ── Rueckstau VOR dem Bauen, nicht danach ────────────────────────────
    //
    // 🔑 Gemessen am 04.09.2026 im G3-Soak: die P1-Queue ist in EINTRAEGEN
    // gedeckelt (128 + 128 Wiederholpuffer), nicht in Bytes. Bei einem
    // `state_report` von rund 300 Byte ist das ein Puffer von 80 KiB, bei
    // einem Evidenzsnapshot von rund 10 KiB sind es 2,5 MiB je Sonde — mit
    // 16 Sonden wuchs der Working Set des Clients um 23,4 MiB gegen ein
    // Budget von 16,8 MiB.
    //
    // Das Urteil von `sendeP1` kommt zu spaet: da liegt die Nachricht schon
    // im Puffer. Der Fuellstand ist das Signal, das VOR dem Einreihen zur
    // Verfuegung steht. Ueber der Schwelle entsteht gar kein Snapshot: ein
    // verworfener ANALYSEframe ist ausdruecklich erlaubt (§48.1, "Ueberlast
    // verwirft Analyseframes, nie Audio"), solange er GEZAEHLT wird — und
    // die Kadenz sinkt, damit das naechste Fenster laenger und seltener ist
    // statt nur spaeter.
    //
    // Warum nicht die Queue-Politik aendern: der P1-Wiederholpuffer wirft
    // ausdruecklich NICHTS weg ("Was einmal angenommen wurde, bleibt
    // angenommen", SONDE-010). Diese Zusage bleibt; wer sie nicht tragen
    // kann, reicht erst gar nicht ein.
    const auto stand = controlV3.snapshot();
    if (stand.p1Tiefe + stand.p1WiederholTiefe > kEvidenzP1Schwelle)
    {
        const double alt = merkmale.evidenzIntervallJetzt();
        merkmale.evidenzIntervallSetzen (alt * 2.0);
        if (merkmale.evidenzIntervallJetzt() > alt)
            evidenzKadenzReduktionen.fetch_add (1);
        evidenzNichtGesendet.fetch_add (1);
        // 🔑 SONDE-013 M-05: der Ring bleibt STEHEN, und die Verlustbasis
        // bleibt, wo sie ist.
        //
        // Die frueheren zwei Zeilen leerten hier den Ereignisring und setzten
        // `letzteEreignisverluste` vor. Beides zusammen loeschte angenommene
        // Ereignisse SPURLOS: der naechste Snapshot trug sie nicht mehr, und
        // weil die Basis mitgewandert war, meldete er auch keinen Verlust.
        // M-05 erlaubt bei Ueberlast ausdruecklich nur die Kadenzreduktion
        // (die darueber steht) und verlangt GEZAEHLTE Ringverluste. Der Ring
        // hat seinen eigenen Deckel mit eigenem Zaehler; laeuft er waehrend
        // des Rueckstaus ueber, ist genau das die ehrliche Meldung.
        return;
    }

    const auto verworfen = merkmale.ereignisseVerworfen();
    const std::uint64_t verlorenSeitdem = verworfen >= letzteEreignisverluste
                                        ? verworfen - letzteEreignisverluste : 0u;

    // Der Ring liegt in der Engine; hier entsteht eine flache Kopie in
    // Zeitfolge, damit der Bauer keine Engine kennt.
    const int anzahl = merkmale.ereignisAnzahlJetzt();
    std::array<nakama::analyse::Ereignis,
               nakama::analyse::FeatureEngine::kEreignisPlaetze> puffer {};
    for (int i = 0; i < anzahl; ++i)
        puffer[(std::size_t) i] = merkmale.ereignis (i);

    nakama::evidenz::Ereignisstrom strom;
    strom.eintraege = puffer.data();
    strom.anzahl    = anzahl;
    strom.verloren  = verlorenSeitdem;

    nakama::evidenz::Snapshotkopf kopf;
    kopf.evidenceId = uuidHex32();
    // 🔑 NAK-40: die Wireadresse, nicht die persistente. `v3Hello()` liefert
    // die Original-Instance-ID aus dem State; der Alias entsteht an der
    // v3-Grenze. Ohne ihn wies `evidenceSnapshotAlsJson` jede unterstuetzte
    // Legacy-ID an `adresseGueltig` zurueck, und die Evidenz dieser Sonde
    // verschwand still — derselbe Fehler wie im direkten P0-Pfad in Gen.
    kopf.adresse    = nakama::ipc::wireAdresseAusState (v3Hello().adresse);
    // SONDE-013 M-06: die Gesamtklasse kommt aus der REGEL, nicht aus einer
    // Kette von Sonderfaellen an dieser Stelle (§34.3, `Konfidenz.h`).
    //
    // Die erste Fassung stand hier als drei verschachtelte Bedingungen mit
    // dem Literal "mittel" am Ende — und dieses "mittel" war der wichtigste
    // Teil: eine Sonde kennt von den vier Mangelquellen aus §34.3 genau
    // EINE, ihre eigene Coverage. Session, Passage und Alignment entstehen
    // im Broker. Als Literal war das eine Behauptung an einer Stelle; jetzt
    // folgt es aus den drei nicht gesetzten `…Bekannt`-Bits und laesst sich
    // an der Regel selbst pruefen.
    nakama::analyse::Konfidenzlage lage;
    lage.coverageBekannt   = frame.abdeckungGesetzt;
    lage.coverageHart      = frame.abdeckungGesetzt && frame.abdeckung < 0.5f;
    lage.abdeckungGesetzt  = frame.abdeckungGesetzt;
    lage.abdeckung         = frame.abdeckung;
    lage.verteilungFenster = frame.evidenzFenster;
    // SONDE-013 M-07: der GEZAEHLTE Teil der NaN-Ehrlichkeit reist mit dem
    // Beleg, der VERRIEGELNDE deckelt hier die Klasse auf `unbrauchbar`.
    lage.sampleFehlerBekannt = true;
    lage.sampleFehler        = frame.nichtEndlichEvidenz;
    // sessionBekannt, passageBekannt und alignmentBekannt bleiben `false`:
    // eine Sonde sieht diese drei nicht. Genau das deckelt sie auf `mittel`.
    kopf.klasse = nakama::analyse::klasseName (nakama::analyse::gesamtklasse (lage));

    // SONDE-013 M-11: die bandweise Stereoevidenz kommt direkt aus der
    // Engine, nicht aus dem Frame. Sie reist nur, wenn ueberhaupt ein Band
    // eine Basis traegt - ein Satz aus 221 leeren Baendern waere 11 KiB
    // Schweigen auf der Leitung, und die Rueckstauschwelle unten misst in
    // EINTRAEGEN, nicht in Bytes.
    nakama::evidenz::Stereosicht stereo;
    if (merkmale.stereoHatInhalt())
    {
        stereo.baender = &merkmale.stereoBand (0);
        stereo.skalare = merkmale.stereoSkalare();
    }

    std::string json;
    // M-05: erst eine erfolgreiche Uebergabe entnimmt die Ereignisse.
    bool uebergeben = false;
    if (nakama::evidenz::evidenceSnapshotAlsJson (frame, kopf, strom, stereo, json))
    {
        // Leerer Koaleszenzschluessel: zwei Snapshots derselben Quelle sind
        // ZWEI Belege mit eigener evidence_id, nicht zweimal derselbe Blick
        // auf ein Objekt. Sie zu koaleszieren hiesse, angenommene Evidenz zu
        // loeschen — genau das, was die Prioritaetspolitik verbietet.
        const auto ergebnis = controlV3.sendeP1 ({}, json);
        // Die Regel steht in `IpcQueues.h` als Funktion, nicht hier als
        // Bedingung: sie gilt fuer jeden Sender, der seine Quelle leert, und
        // ein Bein kann sie ueber alle fuenf Ergebnisse abfahren.
        uebergeben = nakama::ipc::p1Uebergeben (ergebnis);
        const bool rueckstau =
            ergebnis == nakama::ipc::P1Ergebnis::zurWiederholung
         || ergebnis == nakama::ipc::P1Ergebnis::abgewiesen;
        if (rueckstau)
        {
            const double alt = merkmale.evidenzIntervallJetzt();
            merkmale.evidenzIntervallSetzen (alt * 2.0);
            if (merkmale.evidenzIntervallJetzt() > alt)
                evidenzKadenzReduktionen.fetch_add (1);
            evidenzNichtGesendet.fetch_add (1);
        }
        else
        {
            // Zurueck Richtung 4 Hz, aber in Schritten: ein Sprung zurueck
            // auf die schnellste Kadenz nach EINEM freien Platz erzeugte
            // genau das Flattern, gegen das der Rueckstau schuetzt.
            merkmale.evidenzIntervallSetzen (merkmale.evidenzIntervallJetzt() * 0.5);
            evidenzSnapshots.fetch_add (1);
        }
    }
    else
    {
        evidenzNichtGesendet.fetch_add (1);
    }

    // Der Ring ist mit diesem Fenster abgeschlossen — aber NUR, wenn der
    // Snapshot wirklich uebergeben wurde. Bliebe er sonst stehen, traegen
    // dieselben Ereignisse im naechsten Snapshot ein zweites Mal, und ein
    // Empfaenger zaehlte einen Transienten doppelt.
    //
    // 🔑 Die Bedingung ist der Fix zu M-05: fruehen entnahmen diese zwei
    // Zeilen bedingungslos. Ein abgewiesener Snapshot (P1-Puffer voll, zu
    // gross, oder `evidenceSnapshotAlsJson` fail-closed) loeschte damit die
    // Ereignisse, die er gerade NICHT transportiert hat — ein stiller
    // Verlust ohne Zaehler. Jetzt bleiben sie liegen und reisen im naechsten
    // Snapshot mit; nur der Ringdeckel selbst darf sie noch verwerfen, und
    // der zaehlt.
    if (uebergeben)
    {
        merkmale.ereignisseEntnommen();
        letzteEreignisverluste = verworfen;
    }
}

#if defined (NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
bool SondeProcessor::letzterProducerFrameFuerTest (
    nakama::analyse::FeatureFrame& aus) const
{
    std::lock_guard<std::mutex> l (analyseSchloss);
    if (! letzterProducerFrameVorhanden)
        return false;
    aus = letzterProducerFrame;
    return true;
}
#endif

void SondeProcessor::getStateInformation (juce::MemoryBlock& ziel)
{
    const juce::ScopedLock l (zustandSchloss);
    nakama::state::speichere (gehaltenerStand(), ziel);
}

nakama::state::Zustand SondeProcessor::zustandLesen() const
{
    const juce::ScopedLock l (zustandSchloss);
    return gehaltenerStand();
}

nakama::state::Zustand SondeProcessor::gehaltenerStand() const
{
    // E-1: der Transaktionskern ist die EINE Wahrheit des bestaetigten
    // Zustands. `zustand` traegt Common, Klassifikation und bei read-only die
    // Originalbytes; die Dsp-Haelfte setzt erst das Lesen aus dem Kern ein.
    // Hinter dem Commit-Punkt kopiert deshalb niemand einen Undo-Ring
    // (§5.11.4 Teil 2). Ein read-only gehaltener Stand geht unveraendert
    // zurueck (§53.8); ein Stand, den `lade` angenommen hat, besteht auch den
    // Ladestart - der Leser prueft DTO, Zonen, Ring, Cursor und Revision.
    auto stand = zustand;
    if (! stand.nurLesen && stand.hatParameters)
        transaktion->schreibeIn (stand);
    return stand;
}

void SondeProcessor::setStateInformation (const void* daten, int groesse)
{
    if (daten == nullptr || groesse <= 0)
        return;

    nakama::state::Zustand geladen;
    const auto ergebnis = nakama::state::lade (daten, (size_t) groesse, bundleVertrag(), geladen);

    // `ignoriert` heisst: fremder Baumtyp oder Muell. Dann bleibt der
    // gehaltene Zustand, wie er ist - ein halb uebernommener Stand waere
    // genau der Teilstate, den §53.8 verbietet.
    if (ergebnis == nakama::state::LadeErgebnis::ignoriert)
        return;

    nakama::parameter::Satz abgleich;
    {
        const juce::ScopedLock l (zustandSchloss);
        // §53.5: erst der Restore klassifiziert. `read-only` faellt auf neutral
        // zurueck, ein gueltiger Stand mit der eigenen Produktklasse setzt sie.
        // Der Automat bekommt den geladenen Stand, BEVOR er weggezogen wird -
        // `std::move` laesst `geladen` sonst als Huelle zurueck.
        lebenslauf.stateRestauriert (ergebnis, geladen);
        zustand = std::move (geladen);

        // SONDE-015 4a - der Ladestart (Manifest §5.11.4 Teil 1, I5, T17): keine
        // Transaktion, keine Revision; r0 = r = die gespeicherte Revision, das
        // Register ist leer, Preview und Automation enden. Ein read-only
        // gehaltener Stand laedt NEUTRAL (§44.4) - `eq_enabled` aus, der
        // Passthrough -, und seine Originalbytes bleiben im `zustand`.
        juce::String grund;
        const bool eigenerStand = ! zustand.nurLesen && zustand.hatParameters
            && transaktion->ladestart (zustand.dspDto(), (std::uint64_t) zustand.stateRevision,
                                       zustand.undoRing, zustand.undoCursor, grund);
        if (! eigenerStand)
        {
            const bool neutral = transaktion->ladestart (nakama::parameter::DspSatz {}, 0, {}, 0, grund);
            jassert (neutral);
            juce::ignoreUnused (neutral);
        }

        // R10: nach dem Laden steht die Hoermatrix IMMER auf Processed (M-52).
        dspKern->setzeHoermatrix (nakama::dsp::Hoermatrix::processed);
        gesteOffen.fill (false);
        gesteBeteiligt.fill (false);
        abgleich = transaktion->bestaetigt().werte;
        if (dspKern->samplerate() > 0.0)
            publikationOffen = ! dspAusfuehrung->publiziereWirksam (transaktion->wirksam(), true);
    }

    // M-08, M-84: State lesen -> validieren -> Committed setzen ->
    // Hostparameter synchronisieren -> der erste Block uebernimmt die Bank.
    // Laden meldet kein Host-Dirty (M-85, Bestandsregel).
    hostParameterAbgleichen (abgleich);
    controlV3.reconnect();
    telemetryV3.reconnect();
}

nakama::ipc::ControlHello SondeProcessor::v3Hello() const
{
    nakama::ipc::ControlHello h;
    {
        const juce::ScopedLock l (zustandSchloss);
        h.adresse.logonSid = v3LogonSid;
        h.adresse.projectBindingId = zustand.common.projectBindingId.toStdString();
        // Eine Probe besitzt keine Session-Epoche. Bis zum internen C-03-Join
        // traegt das bestehende Pflichtfeld denselben hex32-Wert wie die
        // persistierte Projektbindung: ein erkennbarer Join-Marker, keine
        // erfundene Sitzungsidentitaet. Nur der Coordinator darf ihn bei genau
        // einer Main-Sitzung durch deren Epoche ersetzen.
        h.adresse.sessionEpoch = h.adresse.projectBindingId;
        h.adresse.instanceId = zustand.common.instanceId.toStdString();
        h.adresse.runtimeNonce = v3RuntimeNonce;
        h.pluginKind = nakama::state::wort (zustand.common.klasse);
    }
    h.hostAngeben = true;
    h.hostPid = static_cast<std::uint32_t> (_getpid());
    h.pluginVersion = "0.3.0";
    h.samplerate = v3Samplerate.load();
    h.blockSize = v3BlockSize.load();
    h.channels = v3Channels.load();
    return h;
}

nakama::ipc::ControlStatus SondeProcessor::v3Status() const
{
    nakama::ipc::ControlStatus s;
    s.dspSchemaVersion = nakama::parameter::kDspSchemaVersion;
    {
        const juce::ScopedLock l (zustandSchloss);
        // SONDE-015 4a: Revision und Hash sind die des Transaktionskerns - EINE
        // Wahrheit (§5.11.4 Teil 1). Die Nutzlast `dsp` und das Senden nach
        // jeder Transaktion sind Etappe 4b.
        s.stateRevision = transaktion->revision();
        if (zustand.hatParameters && ! zustand.nurLesen)
            s.stateHash = nakama::transaktion::alsText (transaktion->hash()).toStdString();

        // Read-only oder ein nicht erlaubter Messpunkt wird nicht als insert
        // synthetisiert. Ohne bekannten Messpunkt reist gar kein Runtimeblock.
        if (! zustand.nurLesen && nakama::state::positionErlaubt (
                zustand.common.klasse, zustand.common.position))
        {
            switch (zustand.common.position)
            {
                case nakama::state::Messposition::insert:
                    s.runtime.gemeldet = true;
                    s.runtime.messpunkt = "insert";
                    break;
                case nakama::state::Messposition::pre:
                    s.runtime.gemeldet = true;
                    s.runtime.messpunkt = "pre";
                    break;
                case nakama::state::Messposition::post:
                    s.runtime.gemeldet = true;
                    s.runtime.messpunkt = "post";
                    break;
                case nakama::state::Messposition::post_fader_contribution:
                    break;
            }
            if (s.runtime.gemeldet)
            {
                s.runtime.labelGemeldet = true;
                s.runtime.label = zustand.common.label.toStdString();
            }
        }
    }

    if (s.runtime.gemeldet)
    {
        bool suspendiert = false;
        {
            // `AudioProcessor::suspended` ist in JUCE 8.0.9 kein Atomic. Der
            // Control-Thread liest ihn deshalb unter JUCEs Callback-Lock;
            // der Audiothread nimmt nie unseren Runtime-/State-Lock.
            const juce::ScopedLock l (getCallbackLock());
            suspendiert = isSuspended();
        }
        // JUCE 8.0.9 haelt `nonRealtime` bereits atomar; der Status liest
        // deshalb die Hostwahrheit direkt statt einen zweiten Spiegelstand.
        s.runtime.betrieb = isNonRealtime() ? "offline"
                           : suspendiert ? "suspended" : "active";

        const juce::ScopedLock l (hostKontextSchloss);
        s.runtime.hostBusNameGemeldet = hostBusNameGemeldet;
        if (hostBusNameGemeldet)
            s.runtime.hostBusName = hostBusName;
        // JUCE TrackProperties traegt keinen Mixerindex; kein erfundener Wert.
        s.runtime.hostMixerIndexGemeldet = false;
    }

    const auto ueberlauf = analyseQueue.dropsUeberlauf();
    const auto oversize = analyseQueue.dropsOversize();
    s.framesDropped = addiereGesaettigt (ueberlauf, oversize);
    s.queueOverflows = ueberlauf;

    // TrackProperties ist kein Block-Record-State. Ohne eigenes Hostbit bleibt
    // diese Achse unveraendert unbekannt.
    s.recordStateValid = false;
    s.recording = false;
    return s;
}

nakama::ipc::TelemetryHello SondeProcessor::v3TelemetryHello() const
{
    nakama::ipc::TelemetryHello h;
    h.adresse = v3Hello().adresse;
    h.pluginVersion = "0.3.0";
    controlV3.kopplung (h.linkId, h.challenge);
    return h;
}

//==============================================================================
// SONDE-015 Etappe 4a: Transaktionskern, Hostparameter, Control-Worker

nakama::transaktion::Tid SondeProcessor::neueTid() noexcept
{
    return { tidHoch, tidZaehler.fetch_add (1) + 1 };
}

nakama::transaktion::Ergebnis SondeProcessor::fuehreTransaktionAus (const nakama::transaktion::Auftrag& auftrag)
{
    nakama::transaktion::Ergebnis ergebnis;
    nakama::parameter::Satz abgleich;
    bool commit = false;
    {
        const juce::ScopedLock l (zustandSchloss);
        if (zustand.nurLesen)
        {
            ergebnis.ausgang  = nakama::transaktion::Ausgang::fehler;
            ergebnis.revision = transaktion->revision();
            ergebnis.hash     = transaktion->hash();
            ergebnis.grund    = "schreibgeschuetzt";
            return ergebnis;
        }
        ergebnis = transaktion->fuehreAus (auftrag);
        commit = ergebnis.ausgang == nakama::transaktion::Ausgang::commit;
        if (commit)
        {
            // Nichts wird gespiegelt: Speichern liest den bestaetigten Zustand
            // direkt aus dem Kern (`gehaltenerStand`, M-89, M-93).
            abgleich = transaktion->bestaetigt().werte;
            publikationOffen = false;
        }
    }

    if (commit)
    {
        // §44.3: die exponierten Hostparameter folgen dem bestaetigten
        // Zustand. Beides steht AUSSERHALB des Schlosses, weil es in den Host
        // ruft (Muster `src/prozessor/State.cpp`).
        hostParameterAbgleichen (abgleich);
        if (dspAusfuehrung->dirtyAbholen())
            updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged (true));
    }
    return ergebnis;
}

bool SondeProcessor::setzePreview (const nakama::parameter::DspSatz& satz, juce::String& grund)
{
    const juce::ScopedLock l (zustandSchloss);
    return transaktion->setzePreview (satz, grund);
}

void SondeProcessor::beendePreview()
{
    const juce::ScopedLock l (zustandSchloss);
    transaktion->beendePreview();
}

nakama::parameter::DspSatz SondeProcessor::bestaetigterZustand() const
{
    const juce::ScopedLock l (zustandSchloss);
    return transaktion->bestaetigt();
}

nakama::parameter::DspSatz SondeProcessor::wirksamerZustand() const
{
    const juce::ScopedLock l (zustandSchloss);
    return transaktion->wirksam();
}

std::uint64_t SondeProcessor::stateRevision() const
{
    const juce::ScopedLock l (zustandSchloss);
    return transaktion->revision();
}

juce::String SondeProcessor::stateHashText() const
{
    const juce::ScopedLock l (zustandSchloss);
    return nakama::transaktion::alsText (transaktion->hash());
}

std::uint64_t SondeProcessor::automationEpoche() const
{
    const juce::ScopedLock l (zustandSchloss);
    return transaktion->automation().epoche;
}

bool SondeProcessor::previewAktiv() const
{
    const juce::ScopedLock l (zustandSchloss);
    return transaktion->preview().aktiv;
}

bool SondeProcessor::dspBericht (nakama::transaktion::DspBericht& aus, juce::String& grund) const
{
    const juce::ScopedLock l (zustandSchloss);
    return nakama::transaktion::baueBericht (*transaktion, aus, grund);
}

void SondeProcessor::setNonRealtime (bool offline) noexcept
{
    juce::AudioProcessor::setNonRealtime (offline);
    if (! offline) return;
    const juce::ScopedLock l (zustandSchloss);
    transaktion->beendePreview();
    dspKern->setzeHoermatrix (nakama::dsp::Hoermatrix::processed);
}

void SondeProcessor::parameterValueChanged (int index, float neuNormiert)
{
    // Laeuft auch im Audiothread: nur Atomics, kein Schloss, keine Allokation.
    if (abgleichTiefe > 0 || index < 0 || index >= nakama::parameter::kHostParameter)
        return;
    hostWert[(size_t) index].store (neuNormiert, std::memory_order_relaxed);
    hostEreignis[(size_t) index].fetch_add (1, std::memory_order_relaxed);
    hostEreignisOffen.store (true, std::memory_order_relaxed);
}

void SondeProcessor::parameterGestureChanged (int index, bool beginnt)
{
    // Eine Geste beginnt nur die eigene Oberflaeche - der VST3-Wrapper reicht
    // keine Hostgeste an das Plugin weiter. Ein abgeschlossener manueller
    // Gestus ist EINE Transaktion (§44.3, M-82).
    if (index < 0 || index >= nakama::parameter::kHostParameter) return;
    bool abschliessen = false;
    {
        const juce::ScopedLock l (zustandSchloss);
        gesteOffen[(size_t) index] = beginnt;
        if (beginnt)
            gesteBeteiligt[(size_t) index] = true;
        else
            abschliessen = std::none_of (gesteOffen.begin(), gesteOffen.end(), [] (bool offen) { return offen; });
    }
    if (abschliessen) gestusAbschliessen();
}

void SondeProcessor::gestusAbschliessen()
{
    nakama::transaktion::Auftrag auftrag;
    std::array<bool, (size_t) nakama::parameter::kHostParameter> beteiligt {};
    {
        const juce::ScopedLock l (zustandSchloss);
        beteiligt = gesteBeteiligt;
        gesteBeteiligt.fill (false);
        auftrag.art          = nakama::transaktion::Art::gestus;
        auftrag.tid          = neueTid();
        auftrag.baseRevision = transaktion->revision();
        auftrag.satz         = transaktion->bestaetigt();
        for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
            if (beteiligt[(size_t) i])
                auftrag.satz.werte[(size_t) i] = zelleAusHost (i, hostParameter[(size_t) i]->getValue());
        if (auftrag.satz == transaktion->bestaetigt())
            return;   // ein Gestus ohne Aenderung erzeugt keine Revision
    }

    const auto ergebnis = fuehreTransaktionAus (auftrag);
    if (ergebnis.ausgang == nakama::transaktion::Ausgang::commit)
        return;

    // Abgewiesen (etwa User-Schutz, M-67): Klang und Regler kehren zum
    // bestaetigten Wert zurueck - keine halbe Anwendung.
    nakama::parameter::Satz abgleich;
    {
        const juce::ScopedLock l (zustandSchloss);
        for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
            if (beteiligt[(size_t) i]) transaktion->automationLoesen (i);
        publikationOffen = true;
        abgleich = transaktion->bestaetigt().werte;
    }
    hostParameterAbgleichen (abgleich);
}

nakama::parameter::Zelle SondeProcessor::zelleAusHost (int index, float normiert) const
{
    const auto& b = nakama::parameter::tabelle()[(size_t) index];
    const auto* p = hostParameter[(size_t) index];
    const auto& bestaetigt = transaktion->bestaetigt().werte[(size_t) index];

    // Gleicht der Hostwert dem bestaetigten Wert in Hostgenauigkeit, ist es
    // GENAU der bestaetigte Wert: ein float traegt q = 0.7071067811865476
    // nicht, und ein Rundungsrest waere eine erfundene Aenderung.
    if (p->convertTo0to1 (hostWertAus (index, bestaetigt)) == normiert)
        return bestaetigt;

    // Ein nicht-endlicher Hostwert erreicht nie Overlay, Gestus oder Programm:
    // er zaehlt als unveraendert (Manifest SONDE-015 §10.2, E4-10).
    if (! std::isfinite (normiert))
        return bestaetigt;

    const float wert = p->convertFrom0to1 (normiert);
    nakama::parameter::Zelle z;
    switch (b.typ)
    {
        case nakama::parameter::Typ::boolean:     z.b = wert >= 0.5f; break;
        case nakama::parameter::Typ::gleitkomma:  z.zahl = juce::jlimit (b.min, b.max, (double) wert); break;
        case nakama::parameter::Typ::aufzaehlung: z.enumIndex = juce::jlimit (0, b.werte.size() - 1, juce::roundToInt (wert)); break;
    }
    return z;
}

void SondeProcessor::hostParameterAbgleichen (const nakama::parameter::Satz& werte)
{
    const AbgleichHerkunft herkunft;
    for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
    {
        auto* p = hostParameter[(size_t) i];
        const float soll = p->convertTo0to1 (hostWertAus (i, werte[(size_t) i]));
        if (p->getValue() != soll)
            p->setValueNotifyingHost (soll);
    }
}

bool SondeProcessor::committedRuhtImPassthrough() const noexcept
{
    int aktiv = -1, quelle = -1, candidate = -1, candidateQuelle = -1;
    dspKern->gefahreneSlots (aktiv, quelle, candidate, candidateQuelle);
    const auto passthrough = [this] (int slot) noexcept
    {
        if (slot < 0) return true;
        const auto& p = dspKern->pool().bank (slot).programm;
        return ! p.eqEngagiert || p.hardBypass;
    };
    return passthrough (aktiv) && passthrough (quelle);
}

void SondeProcessor::dspKontrollTakt()
{
    const juce::ScopedLock l (zustandSchloss);

    // §44.2: erst nach dem ACK des Audiothreads ist eine Bank wieder frei.
    dspKern->pflege();

    // Hostereignisse -> AutomationOverlay (M-81): keine Revision, kein Undo.
    if (hostEreignisOffen.exchange (false, std::memory_order_relaxed))
    {
        for (int i = 0; i < nakama::parameter::kHostParameter; ++i)
        {
            const auto n = hostEreignis[(size_t) i].load (std::memory_order_relaxed);
            if (n == hostEreignisGesehen[(size_t) i]) continue;
            hostEreignisGesehen[(size_t) i] = n;
            transaktion->automationSchreiben (i, zelleAusHost (i, hostWert[(size_t) i].load (std::memory_order_relaxed)));
            samplesBeiLetzterAutomation = verarbeiteteSamples.load (std::memory_order_relaxed);
            publikationOffen = true;
        }
    }

    // Die Ruhegrenze, gezaehlt in verarbeiteten Audiosamples (§44.3, M-120).
    const double fs = transaktion->samplerate();
    if (transaktion->automation().laeuft && fs > 0.0
        && verarbeiteteSamples.load (std::memory_order_relaxed) - samplesBeiLetzterAutomation
               >= (std::uint64_t) (nakama::transaktion::kAutomationsRuheSekunden * fs))
        transaktion->automationRuht();

    // Den wirksamen Zustand publizieren; bei busy_retry im naechsten Takt.
    if (publikationOffen && dspKern->samplerate() > 0.0)
        publikationOffen = ! dspAusfuehrung->publiziereWirksam (transaktion->wirksam(), false);
}

} // namespace nakama::sonde
