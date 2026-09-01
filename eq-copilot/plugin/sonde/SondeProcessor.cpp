#include "SondeProcessor.h"
#include "BrokerInstallBinding.h"
#include "PipeToken.h"

#include <algorithm>
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
                   brokerServerErwartung())
{
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
    workerWarte.notify_all();
    controlV3.reconnect();
    telemetryV3.reconnect();
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

        AnalyseQueue::TapQuelle abgriff;
        abgriff.links = puffer.getReadPointer (0);
        abgriff.rechts = kanaele > 1 ? puffer.getReadPointer (1) : nullptr;
        // Ganz oder gar nicht. Rueckstau verwirft Analyse, niemals Audio.
        analyseQueue.veroeffentliche (&abgriff, 1, kanaele, samples, stempel);
    }

    // Sampleidentischer Passthrough: die Analyse liest nur. Bei der erlaubten
    // gleichen Busbelegung laeuft diese JUCE-Sicherheitsschleife null Mal.
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

    while (workerLaeuft.load())
    {
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
                    // Queue-Drop/Deskriptorsprung: kein alter K-/Loudness-
                    // oder Featurezustand darf die Luecke ueberbruecken.
                    analyseEngine.zuruecksetzen();
                    merkmale.zuruecksetzen();
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
    nakama::state::speichere (zustand, ziel);
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

    {
        const juce::ScopedLock l (zustandSchloss);
        // §53.5: erst der Restore klassifiziert. `read-only` faellt auf neutral
        // zurueck, ein gueltiger Stand mit der eigenen Produktklasse setzt sie.
        // Der Automat bekommt den geladenen Stand, BEVOR er weggezogen wird -
        // `std::move` laesst `geladen` sonst als Huelle zurueck.
        lebenslauf.stateRestauriert (ergebnis, geladen);
        zustand = std::move (geladen);
    }
    v3StateRevision.fetch_add (1);
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
    s.stateRevision = v3StateRevision.load();
    {
        const juce::ScopedLock l (zustandSchloss);
        if (zustand.hatParameters && ! zustand.nurLesen)
        {
            juce::String hash, grund;
            if (nakama::parameter::stateHash (zustand.parameters, hash, grund))
                s.stateHash = hash.toStdString();
        }

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

} // namespace nakama::sonde
