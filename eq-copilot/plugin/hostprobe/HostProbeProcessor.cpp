#include "HostProbeProcessor.h"
#include "HostProbeEditor.h"

#include <thread>

namespace eqcop::hostprobe
{

namespace
{
    /** Setzt beim ersten Kontextblock die „immer gueltig"-Maske, danach wird sie
        nur noch abgeschwaecht. So trennt der Bericht „FL sagt das IMMER" von
        „FL sagt das MANCHMAL" — beides sind verschiedene Capability-Aussagen. */
    void verundeGueltigkeiten (Gueltigkeiten& ziel, const Gueltigkeiten& neu, bool erster)
    {
        if (erster) { ziel = neu; return; }
        ziel.projektZeit &= neu.projektZeit;
        ziel.fortlaufend &= neu.fortlaufend;
        ziel.tempo       &= neu.tempo;
        ziel.ppq         &= neu.ppq;
        ziel.taktbeginn  &= neu.taktbeginn;
        ziel.schleife    &= neu.schleife;
        ziel.samplerate  &= neu.samplerate;
    }

    void verodereGueltigkeiten (Gueltigkeiten& ziel, const Gueltigkeiten& neu)
    {
        ziel.projektZeit |= neu.projektZeit;
        ziel.fortlaufend |= neu.fortlaufend;
        ziel.tempo       |= neu.tempo;
        ziel.ppq         |= neu.ppq;
        ziel.taktbeginn  |= neu.taktbeginn;
        ziel.schleife    |= neu.schleife;
        ziel.samplerate  |= neu.samplerate;
    }

    juce::var gueltigkeitenAlsVar (const Gueltigkeiten& g)
    {
        auto* o = new juce::DynamicObject();
        o->setProperty ("project_time_samples",   g.projektZeit);
        o->setProperty ("continous_time_samples", g.fortlaufend);
        o->setProperty ("tempo",                  g.tempo);
        o->setProperty ("ppq_position",           g.ppq);
        o->setProperty ("bar_position",           g.taktbeginn);
        o->setProperty ("cycle_bounds",           g.schleife);
        o->setProperty ("sample_rate",            g.samplerate);
        return juce::var (o);
    }
}

//==============================================================================
const char* HostProbeProcessor::artName (Art a) noexcept
{
    switch (a)
    {
        case Art::ErsterBlock:           return "erster_block";
        case Art::KontextDa:             return "kontext_da";
        case Art::KontextWeg:            return "kontext_weg";
        case Art::TransportAn:           return "transport_an";
        case Art::TransportAus:          return "transport_aus";
        case Art::AufnahmeAn:            return "aufnahme_an";
        case Art::AufnahmeAus:           return "aufnahme_aus";
        case Art::SchleifeAn:            return "schleife_an";
        case Art::SchleifeAus:           return "schleife_aus";
        case Art::ZeitsprungVor:         return "zeitsprung_vor";
        case Art::ZeitsprungZurueck:     return "zeitsprung_zurueck";
        case Art::OfflineAn:             return "offline_an";
        case Art::OfflineAus:            return "offline_aus";
        case Art::GenauigkeitFloat:      return "genauigkeit_float";
        case Art::GenauigkeitDouble:     return "genauigkeit_double";
        case Art::LatenzGemeldet:        return "latenz_gemeldet";
        case Art::AutomationPunkt:       return "automation_punkt";
        case Art::AutomationMehrpunkt:   return "automation_mehrpunkt";
        case Art::AutomationUnplausibel: return "automation_unplausibel";
        case Art::BlockOhneVerarbeitung: return "block_ohne_verarbeitung";
    }
    return "unbekannt";
}

//==============================================================================
HostProbeProcessor::HostProbeProcessor()
    : juce::AudioProcessor (BusesProperties()
                                .withInput  ("Eingang", juce::AudioChannelSet::stereo(), true)
                                .withOutput ("Ausgang", juce::AudioChannelSet::stereo(), true))
{
    // Ein automatisierbarer Parameter, damit der User in FL eine Automationskurve
    // zeichnen kann. Er STEUERT nichts — sein einziger Zweck ist, gemessen zu
    // werden; die Oberflaeche sagt das ausdruecklich, damit sie nicht luegt.
    addParameter (testwert = new juce::AudioParameterFloat (
        juce::ParameterID { "testwert", 1 }, "Testwert (nur Messung)", 0.0f, 1.0f, 0.5f));

    setLatencySamples (0);
}

void HostProbeProcessor::prepareToPlay (double samplerate, int blockgroesse)
{
    setLatencySamples (0);

    generation.fetch_add (1, std::memory_order_release);
    stand.samplerate = samplerate;
    if (stand.kleinsterBlock < 0 || blockgroesse < stand.kleinsterBlock) stand.kleinsterBlock = blockgroesse;
    if (blockgroesse > stand.groessterBlock) stand.groessterBlock = blockgroesse;
    generation.fetch_add (1, std::memory_order_release);
}

bool HostProbeProcessor::isBusesLayoutSupported (const BusesLayout& layout) const
{
    const auto ein = layout.getMainInputChannelSet();
    const auto aus = layout.getMainOutputChannelSet();
    if (ein != aus) return false;
    return aus == juce::AudioChannelSet::mono() || aus == juce::AudioChannelSet::stereo();
}

//==============================================================================
void HostProbeProcessor::merke (Art art, juce::int64 projektZeit, juce::int64 vorherige,
                                double ppq, double zusatz, int ganzzahl) noexcept
{
    ++stand.ereignisseGesamt;
    ++stand.artZaehler[(int) art];

    if (! aufzeichnung.load (std::memory_order_relaxed))
        return;                                   // Leser kopiert gerade

    const int i = schreibIndex.load (std::memory_order_relaxed);
    protokoll[i % kMaxEreignisse] = Ereignis { art, stand.bloecke, projektZeit, vorherige,
                                               ppq, zusatz, ganzzahl };
    schreibIndex.store (i + 1, std::memory_order_release);
}

void HostProbeProcessor::zaehleBlock (bool doppeltePraezision, int samples, bool offline) noexcept
{
    generation.fetch_add (1, std::memory_order_release);

    ++stand.bloecke;
    ++stand.verarbeiteteBloecke;

    if (doppeltePraezision)
    {
        if (stand.bloeckeDouble == 0) merke (Art::GenauigkeitDouble, -1, -1, 0.0, 0.0, samples);
        ++stand.bloeckeDouble;
    }
    else
    {
        if (stand.bloeckeFloat == 0) merke (Art::GenauigkeitFloat, -1, -1, 0.0, 0.0, samples);
        ++stand.bloeckeFloat;
    }

    if (offline)
    {
        if (stand.bloeckeOffline == 0) merke (Art::OfflineAn, -1, -1, 0.0, 0.0, samples);
        ++stand.bloeckeOffline;
    }
    else
    {
        if (stand.bloeckeEchtzeit == 0 && stand.bloeckeOffline > 0)
            merke (Art::OfflineAus, -1, -1, 0.0, 0.0, samples);
        ++stand.bloeckeEchtzeit;
    }

    if (samples > 0)
    {
        if (stand.kleinsterBlock < 0 || samples < stand.kleinsterBlock) stand.kleinsterBlock = samples;
        if (samples > stand.groessterBlock) stand.groessterBlock = samples;
    }

    senkeSeitLetztemBlock = false;

    generation.fetch_add (1, std::memory_order_release);
}

void HostProbeProcessor::processBlock (juce::AudioBuffer<float>& puffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals keineDenormals;
    zaehleBlock (false, puffer.getNumSamples(), isNonRealtime());
    // Kein Eingriff: der Puffer bleibt unangetastet (Grundgesetz).
}

void HostProbeProcessor::processBlock (juce::AudioBuffer<double>& puffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals keineDenormals;
    zaehleBlock (true, puffer.getNumSamples(), isNonRealtime());
}

//==============================================================================
void HostProbeProcessor::nakamaBlockEmpfangen (const eqcop::hostbruecke::Blockbefund& b) noexcept
{
    brueckeGesehen.store (true, std::memory_order_relaxed);

    generation.fetch_add (1, std::memory_order_release);

    if (zuruecksetzen.exchange (false, std::memory_order_acq_rel))
    {
        stand = Messstand {};
        schreibIndex.store (0, std::memory_order_release);
        ersterBlockGesehen = false;
        senkeSeitLetztemBlock = false;
        letzteBlockGroesse = 0;
        zeitBasisNeu = true;
        spielteVorher = false;
    }

    // Senke gerufen, ohne dass seit dem letzten Mal ein processBlock lief:
    // genau die Asymmetrie aus dem Bridge-Vertrag (Parameter-Flush, Hostriegel).
    if (senkeSeitLetztemBlock)
        merke (Art::BlockOhneVerarbeitung, -1, -1, 0.0, 0.0, (int) b.blockGroesse);

    senkeSeitLetztemBlock = true;
    ++stand.senkeAufrufe;

    if (! ersterBlockGesehen)
    {
        ersterBlockGesehen = true;
        merke (Art::ErsterBlock, -1, -1, 0.0, 0.0, (int) b.blockGroesse);
    }

    const auto& k = b.kontext;

    if (k.processContextPresent)
    {
        if (! stand.kontextZuletztDa || stand.bloeckeMitKontext == 0)
            merke (Art::KontextDa, k.projectTimeSamples.oder (-1), -1, k.ppqPosition.oder (0.0), 0.0, -1);

        const bool erster = (stand.bloeckeMitKontext == 0);
        ++stand.bloeckeMitKontext;
        stand.kontextZuletztDa = true;

        const Gueltigkeiten jetzt {
            k.projectTimeSamples.gueltig, k.continuousTimeSamples.gueltig,
            k.tempo.gueltig, k.ppqPosition.gueltig, k.barPositionPpq.gueltig,
            k.cycle.gueltig, k.sampleRate.gueltig
        };
        verodereGueltigkeiten (stand.jeGueltig, jetzt);
        verundeGueltigkeiten  (stand.immerGueltig, jetzt, erster);

        if (k.tempo.gueltig)       stand.letztesTempo = k.tempo.wert;
        if (k.ppqPosition.gueltig) stand.letztesPpq   = k.ppqPosition.wert;

        // --- Transport, Aufnahme, Schleife ---------------------------------
        const bool spielt = k.playing.oder (false);
        if (spielt != stand.spieltGerade)
        {
            merke (spielt ? Art::TransportAn : Art::TransportAus,
                   k.projectTimeSamples.oder (-1), stand.letzteProjektZeit,
                   k.ppqPosition.oder (0.0), 0.0, -1);
            stand.spieltGerade = spielt;
            zeitBasisNeu = true;     // nach einem Transportwechsel ist jeder Zeitwert legitim
        }

        const bool nimmtAuf = k.recording.oder (false);
        if (nimmtAuf != aufnahmeVorher)
        {
            merke (nimmtAuf ? Art::AufnahmeAn : Art::AufnahmeAus,
                   k.projectTimeSamples.oder (-1), -1, k.ppqPosition.oder (0.0), 0.0, -1);
            aufnahmeVorher = nimmtAuf;
        }

        if (k.cycle.aktiv != stand.schleifeAktiv)
        {
            merke (k.cycle.aktiv ? Art::SchleifeAn : Art::SchleifeAus,
                   k.projectTimeSamples.oder (-1), -1,
                   k.cycle.gueltig ? k.cycle.startPpq : 0.0,
                   k.cycle.gueltig ? k.cycle.endePpq  : 0.0, k.cycle.gueltig ? 1 : 0);
            stand.schleifeAktiv = k.cycle.aktiv;
        }

        // --- Projektzeit: Sprung oder Fortsetzung? -------------------------
        if (k.projectTimeSamples.gueltig)
        {
            const auto jetztZeit = k.projectTimeSamples.wert;

            if (! zeitBasisNeu && spielt && spielteVorher && stand.letzteProjektZeit >= 0)
            {
                const auto erwartet   = stand.letzteProjektZeit + letzteBlockGroesse;
                const auto abweichung = jetztZeit - erwartet;

                if (abweichung > 0)
                {
                    ++stand.zeitspruengeVor;
                    if (abweichung > stand.groessterSprungVor) stand.groessterSprungVor = abweichung;
                    merke (Art::ZeitsprungVor, jetztZeit, stand.letzteProjektZeit,
                           k.ppqPosition.oder (0.0), (double) abweichung, k.cycle.aktiv ? 1 : 0);
                }
                else if (abweichung < 0)
                {
                    ++stand.zeitspruengeZurueck;
                    if (-abweichung > stand.groessterSprungZur) stand.groessterSprungZur = -abweichung;
                    merke (Art::ZeitsprungZurueck, jetztZeit, stand.letzteProjektZeit,
                           k.ppqPosition.oder (0.0), (double) abweichung, k.cycle.aktiv ? 1 : 0);
                }
            }

            stand.letzteProjektZeit = jetztZeit;
            zeitBasisNeu = false;
        }

        spielteVorher = spielt;
    }
    else
    {
        if (stand.kontextZuletztDa || stand.bloeckeOhneKontext == 0)
            merke (Art::KontextWeg, -1, stand.letzteProjektZeit, 0.0, 0.0, (int) b.blockGroesse);
        ++stand.bloeckeOhneKontext;
        stand.kontextZuletztDa = false;
        zeitBasisNeu = true;      // ohne Context ist die Zeitachse unbewiesen
        spielteVorher = false;
    }

    letzteBlockGroesse = (juce::int64) b.blockGroesse;

    // --- Automation ------------------------------------------------------
    if (b.anzahl > 0)
    {
        if (stand.automationPunkte == 0)
            merke (Art::AutomationPunkt, k.projectTimeSamples.oder (-1), -1,
                   k.ppqPosition.oder (0.0), (double) b.anzahl, (int) b.ereignisse[0].sampleOffset);

        stand.automationPunkte += (juce::int64) b.anzahl;
        if ((int) b.anzahl > stand.maxPunkteProBlock) stand.maxPunkteProBlock = (int) b.anzahl;

        for (juce::uint32 i = 0; i < b.anzahl; ++i)
        {
            const int off = (int) b.ereignisse[i].sampleOffset;
            if (stand.kleinsterOffset < 0 || off < stand.kleinsterOffset) stand.kleinsterOffset = off;
            if (off > stand.groesterOffset) stand.groesterOffset = off;
        }

        if (b.anzahl > 1)
        {
            if (stand.bloeckeMitMehrpunkt == 0)
                merke (Art::AutomationMehrpunkt, k.projectTimeSamples.oder (-1), -1,
                       k.ppqPosition.oder (0.0), (double) b.anzahl,
                       (int) b.ereignisse[b.anzahl - 1].sampleOffset);
            ++stand.bloeckeMitMehrpunkt;
        }
    }

    const auto unplausibel = b.unplausibleOffsets + b.unplausibleWerte;
    if (unplausibel > 0)
    {
        if (stand.automationUnplausibel == 0)
            merke (Art::AutomationUnplausibel, -1, -1, 0.0, (double) unplausibel, -1);
        stand.automationUnplausibel += (juce::int64) unplausibel;
    }
    stand.automationUeberlaeufe += (juce::int64) b.ueberlaeufe;

    // --- Presentation-Latency --------------------------------------------
    if (! stand.latenzJeGemeldet)
    {
        for (juce::int32 bus = 0; bus < eqcop::hostbruecke::kMaxBusse; ++bus)
        {
            const auto& ein = k.presentationLatency.hole (true, bus);
            if (ein.gemeldet)
            {
                stand.latenzJeGemeldet = true;
                stand.latenzEingangBus = (int) bus;
                stand.latenzEingangWert = ein.samples;
                merke (Art::LatenzGemeldet, -1, -1, 0.0, (double) ein.samples, (int) bus);
                break;
            }
            const auto& aus = k.presentationLatency.hole (false, bus);
            if (aus.gemeldet)
            {
                stand.latenzJeGemeldet = true;
                stand.latenzAusgangBus = (int) bus;
                stand.latenzAusgangWert = aus.samples;
                merke (Art::LatenzGemeldet, -1, -1, 0.0, (double) aus.samples, -(int) bus - 1);
                break;
            }
        }
    }

    generation.fetch_add (1, std::memory_order_release);
}

//==============================================================================
Messstand HostProbeProcessor::messstand() const noexcept
{
    Messstand kopie {};
    for (int versuch = 0; versuch < 512; ++versuch)
    {
        const auto g1 = generation.load (std::memory_order_acquire);
        if ((g1 & 1u) != 0u) { std::this_thread::yield(); continue; }

        std::memcpy ((void*) &kopie, (const void*) &stand, sizeof (Messstand));

        const auto g2 = generation.load (std::memory_order_acquire);
        if (g1 == g2) return kopie;

        std::this_thread::yield();
    }
    return kopie;   // extrem unwahrscheinlich; lieber leicht veraltet als blockiert
}

int HostProbeProcessor::ereignisseLesen (Ereignis* ziel, int platz) const noexcept
{
    aufzeichnung.store (false, std::memory_order_release);

    const int geschrieben = schreibIndex.load (std::memory_order_acquire);
    const int vorhanden   = juce::jmin (geschrieben, kMaxEreignisse);
    const int anzahl      = juce::jmin (vorhanden, platz);
    const int start       = geschrieben > kMaxEreignisse ? geschrieben % kMaxEreignisse : 0;

    for (int i = 0; i < anzahl; ++i)
        ziel[i] = protokoll[(start + i) % kMaxEreignisse];

    aufzeichnung.store (true, std::memory_order_release);
    return anzahl;
}

void HostProbeProcessor::messungZuruecksetzen() noexcept
{
    zuruecksetzen.store (true, std::memory_order_release);
}

//==============================================================================
juce::File HostProbeProcessor::berichtsOrdner()
{
    return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
               .getChildFile ("evenacadia")
               .getChildFile ("nakama")
               .getChildFile ("spike");
}

juce::String HostProbeProcessor::berichtAlsJson() const
{
    const auto s = messstand();

    juce::HeapBlock<Ereignis> ereignisse (kMaxEreignisse);
    const int anzahl = ereignisseLesen (ereignisse.get(), kMaxEreignisse);

    auto* wurzel = new juce::DynamicObject();
    wurzel->setProperty ("schema", "nakama-hostprobe-1");
    wurzel->setProperty ("erzeugt", juce::Time::getCurrentTime().toISO8601 (true));
    wurzel->setProperty ("host", juce::PluginHostType().getHostDescription());
    wurzel->setProperty ("wrapper", juce::AudioProcessor::getWrapperTypeDescription (wrapperType));
    wurzel->setProperty ("bruecke_liefert", brueckeLiefert());

    auto* z = new juce::DynamicObject();
    z->setProperty ("bloecke",               (juce::int64) s.bloecke);
    z->setProperty ("senke_aufrufe",         (juce::int64) s.senkeAufrufe);
    z->setProperty ("verarbeitete_bloecke",  (juce::int64) s.verarbeiteteBloecke);
    z->setProperty ("mit_kontext",           (juce::int64) s.bloeckeMitKontext);
    z->setProperty ("ohne_kontext",          (juce::int64) s.bloeckeOhneKontext);
    z->setProperty ("offline",               (juce::int64) s.bloeckeOffline);
    z->setProperty ("echtzeit",              (juce::int64) s.bloeckeEchtzeit);
    z->setProperty ("float",                 (juce::int64) s.bloeckeFloat);
    z->setProperty ("double",                (juce::int64) s.bloeckeDouble);
    z->setProperty ("samplerate",            s.samplerate);
    z->setProperty ("blockgroesse_min",      s.kleinsterBlock);
    z->setProperty ("blockgroesse_max",      s.groessterBlock);
    wurzel->setProperty ("bloecke", juce::var (z));

    wurzel->setProperty ("gueltig_mindestens_einmal", gueltigkeitenAlsVar (s.jeGueltig));
    wurzel->setProperty ("gueltig_immer",             gueltigkeitenAlsVar (s.immerGueltig));

    auto* zeit = new juce::DynamicObject();
    zeit->setProperty ("spruenge_vorwaerts",   (juce::int64) s.zeitspruengeVor);
    zeit->setProperty ("spruenge_rueckwaerts", (juce::int64) s.zeitspruengeZurueck);
    zeit->setProperty ("groesster_sprung_vor",   (juce::int64) s.groessterSprungVor);
    zeit->setProperty ("groesster_sprung_zurueck", (juce::int64) s.groessterSprungZur);
    zeit->setProperty ("letzte_projektzeit",   (juce::int64) s.letzteProjektZeit);
    zeit->setProperty ("letztes_ppq",          s.letztesPpq);
    zeit->setProperty ("letztes_tempo",        s.letztesTempo);
    wurzel->setProperty ("projektzeit", juce::var (zeit));

    auto* au = new juce::DynamicObject();
    au->setProperty ("punkte_gesamt",         (juce::int64) s.automationPunkte);
    au->setProperty ("max_punkte_pro_block",  s.maxPunkteProBlock);
    au->setProperty ("bloecke_mit_mehrpunkt", (juce::int64) s.bloeckeMitMehrpunkt);
    au->setProperty ("kleinster_offset",      s.kleinsterOffset);
    au->setProperty ("groesster_offset",      s.groesterOffset);
    au->setProperty ("ueberlaeufe",           (juce::int64) s.automationUeberlaeufe);
    au->setProperty ("unplausibel",           (juce::int64) s.automationUnplausibel);
    // DIE Frage, fuer die der Bridge-Patch gebaut wurde.
    au->setProperty ("samplegenau_belegt",    s.bloeckeMitMehrpunkt > 0);
    wurzel->setProperty ("automation", juce::var (au));

    auto* lat = new juce::DynamicObject();
    lat->setProperty ("je_gemeldet",   s.latenzJeGemeldet);
    lat->setProperty ("eingang_bus",   s.latenzEingangBus);
    lat->setProperty ("eingang_wert",  (juce::int64) s.latenzEingangWert);
    lat->setProperty ("ausgang_bus",   s.latenzAusgangBus);
    lat->setProperty ("ausgang_wert",  (juce::int64) s.latenzAusgangWert);
    wurzel->setProperty ("presentation_latency", juce::var (lat));

    auto* arten = new juce::DynamicObject();
    for (int i = 0; i < kArtAnzahl; ++i)
        arten->setProperty (artName ((Art) i), (juce::int64) s.artZaehler[i]);
    wurzel->setProperty ("ereignisse_je_art", juce::var (arten));
    wurzel->setProperty ("ereignisse_gesamt", (juce::int64) s.ereignisseGesamt);
    wurzel->setProperty ("ereignisse_gespeichert", anzahl);

    juce::Array<juce::var> liste;
    for (int i = 0; i < anzahl; ++i)
    {
        const auto& e = ereignisse[i];
        auto* o = new juce::DynamicObject();
        o->setProperty ("art",          artName (e.art));
        o->setProperty ("block",        (juce::int64) e.block);
        o->setProperty ("projektzeit",  (juce::int64) e.projektZeit);
        o->setProperty ("vorherige",    (juce::int64) e.vorherige);
        o->setProperty ("ppq",          e.ppq);
        o->setProperty ("zusatz",       e.zusatz);
        o->setProperty ("ganzzahl",     e.ganzzahl);
        liste.add (juce::var (o));
    }
    wurzel->setProperty ("ereignisse", liste);

    return juce::JSON::toString (juce::var (wurzel), false);
}

juce::File HostProbeProcessor::berichtSchreiben() const
{
    auto ordner = berichtsOrdner();
    if (ordner.createDirectory().failed())
        return {};

    const auto name = "host-probe-" + juce::Time::getCurrentTime().formatted ("%Y%m%d-%H%M%S") + ".json";
    auto datei = ordner.getChildFile (name);
    if (! datei.replaceWithText (berichtAlsJson()))
        return {};

    return datei;
}

juce::AudioProcessorEditor* HostProbeProcessor::createEditor()
{
    return new HostProbeEditor (*this);
}

} // namespace eqcop::hostprobe
