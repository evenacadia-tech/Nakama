/*  S9 / SONDE-007b, umgeschrieben in SONDE-015 Etappe 4a (Bauplan §4.4) - der
    Nulltest von Nakama Probeeq.

    WARUM DIESER TEST EXISTIERT: Das Grundgesetz (CLAUDE.md, Wahrheitskern)
    sagt "nichts Ungefragtes": ausgeschaltet ist der Pfad im Nulltest
    bitidentisch, sonst ist der Passthrough sampleidentisch, ohne Latenz oder
    Tail. Bis SONDE-015 war Probeeq in JEDEM Zustand ein Passthrough ohne
    Hostparameter, und genau das mass dieses Bein. Seit Etappe 4a traegt er den
    aktiven DSP-Kern, 112 Host-Parameter und den Transaktionskern - der alte
    Satz "kein Hostparameter" ist damit falsch und faellt (M-116).

    WAS DIESES BEIN JETZT MISST (Manifest SONDE-015 §4.4):
      - Default (`eq_enabled` aus): bitidentisch ueber 1000 Bloecke wechselnder
        Groesse bei 44,1 / 48 / 96 / 192 kHz, danach keine Bank belegt (M-01);
      - `eq_enabled` an, `bypass` aus, sonst alles auf Default: bitidentisch
        (M-02);
      - `eq_enabled` an, `bypass` an, ein hoerbarer Zustand dahinter:
        bitidentisch (M-05);
      - nach `eq_enabled` aus wieder bitidentisch, sobald der Fade vorbei ist
        (M-04);
      - Mix 0 mit Output-Trim 0 dB: bitidentisch, trotz Input-Trim und Band
        (M-33);
      - der Passthrough sanitisiert nichts (M-50);
      - 0 Samples Latenz; kein Tail im Passthrough und im Hard-Bypass (M-51);
      - Speichern, Laden, Speichern bytegleich im Layout v2 mit Kind `Dsp`
        (M-93).
    Ein eingeschalteter resonanter Filter klingt naturgemaess aus; das ist
    kein Tail im Sinne des Hostvertrags, und dieses Bein behauptet dazu
    nichts.

    Gemessen wird die ECHTE Produktklasse SondeProcessor - dasselbe .cpp wie
    im Bundle, ohne Plugin-Wrapper. Die Gate-7- und Bundlepruefungen aus S9
    und G1 bleiben.

    LANDMINE NAK-175: jeder Prozessor liegt auf dem Heap.

    Aufruf: EqCopProbeeqNullTest.exe        Exit 0 gruen, 1 rot.
*/

#include "SondeProcessor.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>

namespace
{

using Prozessor = nakama::sonde::SondeProcessor;
namespace tx    = nakama::transaktion;
namespace param = nakama::parameter;

int okZahl = 0;
int fehlerZahl = 0;

void pruefe (bool bedingung, const juce::String& text, const juce::String& zusatz = {})
{
    const auto zeile = text + (zusatz.isNotEmpty() ? ("  [" + zusatz + "]") : juce::String());
    if (bedingung) { ++okZahl;     std::cout << "  ok      " << zeile << std::endl; }
    else           { ++fehlerZahl; std::cout << "  FEHLER  " << zeile << std::endl; }
}

void abschnitt (const char* titel)
{
    std::cout << std::endl << "-- " << titel << std::endl;
}

/** Reproduzierbares Rauschen - dieselbe Saat ergibt dieselbe Folge. */
void fuelle (juce::AudioBuffer<float>& puffer, juce::Random& wuerfel)
{
    for (int k = 0; k < puffer.getNumChannels(); ++k)
        for (int n = 0; n < puffer.getNumSamples(); ++n)
            puffer.setSample (k, n, wuerfel.nextFloat() * 1.8f - 0.9f);
}

bool istHex (const std::string& text, std::size_t laenge)
{
    return text.size() == laenge
        && std::all_of (text.begin(), text.end(), [] (char c) {
               return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
           });
}

juce::File wurzel()
{
    auto d = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    while (d.exists() && ! d.getChildFile ("eq-copilot").isDirectory())
    {
        const auto oben = d.getParentDirectory();
        if (oben == d) break;
        d = oben;
    }
    return d;
}

std::unique_ptr<Prozessor> vorbereitet (double rate, int maxBlock)
{
    auto p = std::make_unique<Prozessor>();
    p->setRateAndBufferSizeDetails (rate, maxBlock);
    p->prepareToPlay (rate, maxBlock);
    return p;
}

/** Eine Transaktion ueber die oeffentliche Prozessorschnittstelle. */
tx::Ergebnis setze (Prozessor& p, const param::DspSatz& z)
{
    tx::Auftrag a;
    a.tid          = p.neueTid();
    a.baseRevision = p.stateRevision();
    a.art          = tx::Art::apply;
    a.satz         = z;
    return p.fuehreTransaktionAus (a);
}

int iBand (int slot, int feld) { return param::indexBandV1 (slot, feld); }
int iGlobal (const char* id)   { return param::indexVonId (id); }

/** Ein hoerbarer Zustand: +12-dB-Bell bei 1 kHz, Q 1, auf Slot 0. */
void setzeHoerbaresBand (param::DspSatz& z)
{
    z.werte[(size_t) param::indexOccupied (0)].b        = true;
    z.werte[(size_t) iBand (0, param::kEnabled)].b      = true;
    z.werte[(size_t) iBand (0, param::kFreqHz)].zahl    = 1000.0;
    z.werte[(size_t) iBand (0, param::kGainDb)].zahl    = 12.0;
}

/** Die wechselnden Blockgroessen der Nulllaeufe - von 1 Sample bis zum
    vorbereiteten Maximum. FL zerteilt Puffer an Automationspunkten bis auf
    1 Sample (host-capabilities-fl-v1.json). */
const int kBlockgroessen[] = { 1, 7, 64, 128, 333, 512, 1024, 2048, 3, 480 };

struct Nulllauf
{
    std::int64_t samples    = 0;
    std::int64_t abweichend = 0;   ///< Samples, die nicht bitgleich zur Eingangskopie sind
    bool         latenzNull = true;
};

/** Faehrt `bloecke` Bloecke Rauschen durch. Abweichungen zaehlen erst ab dem
    globalen Sample `zaehlenAb` - davor liegt ein Fade, der hier nicht
    gemessen wird. */
Nulllauf fahreNull (Prozessor& p, int bloecke, juce::Random& wuerfel, std::int64_t zaehlenAb = 0)
{
    Nulllauf l;
    juce::MidiBuffer midi;
    for (int b = 0; b < bloecke; ++b)
    {
        const int groesse = kBlockgroessen[(size_t) b % std::size (kBlockgroessen)];
        juce::AudioBuffer<float> puffer (2, groesse), kopie (2, groesse);
        fuelle (puffer, wuerfel);
        kopie.makeCopyOf (puffer);
        p.processBlock (puffer, midi);
        for (int n = 0; n < groesse; ++n, ++l.samples)
        {
            if (l.samples < zaehlenAb) continue;
            for (int k = 0; k < 2; ++k)
                if (std::memcmp (puffer.getReadPointer (k) + n, kopie.getReadPointer (k) + n, sizeof (float)) != 0)
                    { ++l.abweichend; break; }
        }
        if (p.getLatencySamples() != 0) l.latenzNull = false;
    }
    return l;
}

} // namespace

int main()
{
    std::cout << "SONDE-NULLTEST - " << std::make_unique<Prozessor>()->getName()
              << " (Produktklasse " << nakama::state::wort (nakama::sonde::kProduktklasse) << ")" << std::endl;

    auto prozessor = std::make_unique<Prozessor>();

    // -- 1. Produktklasse und Bundlevertrag passen zueinander --------------
    // Ein Bundle, dessen frischer Zustand seinen eigenen Vertrag verletzt,
    // kaeme beim ersten Speichern+Laden als read-only zurueck.
    abschnitt ("1. Produktklasse und Bundlevertrag");
    pruefe (prozessor->zustandLesen().common.klasse == nakama::sonde::kProduktklasse,
            "frischer Zustand traegt die Produktklasse des Bundles",
            nakama::state::wort (prozessor->zustandLesen().common.klasse));
    pruefe (nakama::sonde::bundleVertrag().erlaubt (nakama::sonde::kProduktklasse),
            "der Bundlevertrag laesst die eigene Produktklasse zu");
    pruefe (! nakama::sonde::bundleVertrag().erlaubt (nakama::state::Klasse::main),
            "der Bundlevertrag laesst 'main' NICHT zu");
    pruefe (! nakama::sonde::bundleVertrag().erlaubt (nakama::state::Klasse::legacy),
            "der Bundlevertrag laesst 'legacy' NICHT zu");

    // -- 2. Hostparameter und Oberflaeche -----------------------------------
    // M-116: bis SONDE-015 stand hier "das Bundle meldet dem Host keinen
    // Parameter". Die Reihenfolge und die Grenzen der 112 misst B7 (M-88).
    abschnitt ("2. Hostparameter und Oberflaeche");
    pruefe (prozessor->getParameters().size() == param::kHostParameter,
            "das Bundle meldet dem Host die 112 Host-Parameter des Layouts v2",
            juce::String (prozessor->getParameters().size()));
    pruefe (! prozessor->hasEditor(),
            "keine erfundene Oberflaeche (Gestaltung kommt aus Figma)");

    // -- 2b. Lifecycle: neutral bis gueltigem State (§53.5, S9 Abschnitt 3) --
    pruefe (prozessor->klassifikation() == nakama::state::Klassifikation::unclassified,
            "frische Instanz ist neutral, trotz fester Produktklasse",
            nakama::state::wort (prozessor->klassifikation()));
    pruefe (! prozessor->darfBrokerStarten(),
            "eine Sonde darf den Broker nie starten");

    // -- 3. M-01: der Default ist der Passthrough --------------------------
    abschnitt ("3. M-01 default_ist_bitidentisch_ueber_1000_bloecke");
    for (const double rate : { 44100.0, 48000.0, 96000.0, 192000.0 })
    {
        auto p = vorbereitet (rate, 2048);
        juce::Random wuerfel ((juce::int64) rate);
        const auto lauf = fahreNull (*p, 1000, wuerfel);
        pruefe (lauf.abweichend == 0 && lauf.latenzNull,
                "default_ist_bitidentisch_ueber_1000_bloecke bei " + juce::String (rate, 0) + " Hz, Bloecke 1 bis 2048 Samples",
                juce::String (lauf.samples) + " Samples, " + juce::String (lauf.abweichend) + " abweichend");
        pruefe (! p->bestaetigterZustand().werte[(size_t) param::kIndexEqEnabled].b
                    && p->dspKernFuerTest().pool().belegteSlots() == 0,
                "  eq_enabled steht auf false, und nach dem Lauf ist keine Bank belegt",
                "belegt " + juce::String (p->dspKernFuerTest().pool().belegteSlots()));
    }

    // -- 4. M-51: Latenz und Tail -------------------------------------------
    abschnitt ("4. M-51 latenz_bleibt_null und kein_tail_im_passthrough");
    {
        auto p = vorbereitet (48000.0, 512);
        const auto stilleNachRauschen = [] (Prozessor& q)
        {
            juce::MidiBuffer midi;
            juce::Random w (7);
            for (int i = 0; i < 20; ++i)
            {
                juce::AudioBuffer<float> b (2, 512);
                fuelle (b, w);
                q.processBlock (b, midi);
            }
            juce::AudioBuffer<float> still (2, 512);
            still.clear();
            q.processBlock (still, midi);
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < 512; ++n)
                    if (std::memcmp (still.getReadPointer (k) + n, "\0\0\0\0", sizeof (float)) != 0) return false;
            return true;
        };
        const bool ausgeschaltet = stilleNachRauschen (*p);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        z.werte[(size_t) iGlobal ("v1.global.bypass")].b = true;
        setzeHoerbaresBand (z);
        const auto e = setze (*p, z);
        const bool hardBypass = stilleNachRauschen (*p);
        pruefe (p->getLatencySamples() == 0 && p->getTailLengthSeconds() == 0.0,
                "latenz_bleibt_null: 0 Samples gemeldet, Tail 0,0 s",
                juce::String (p->getLatencySamples()) + " / " + juce::String (p->getTailLengthSeconds()));
        pruefe (ausgeschaltet && hardBypass && e.ausgang == tx::Ausgang::commit,
                "kein_tail_im_passthrough: nach Rauschen bleibt Stille bitgenau still - ausgeschaltet und im Hard-Bypass");
    }

    // -- 5. M-50: der Passthrough sanitisiert nichts ------------------------
    abschnitt ("5. M-50 passthrough_sanitisiert_nichts");
    {
        auto p = vorbereitet (48000.0, 256);
        const auto nichtEndlichBleibt = [] (Prozessor& q)
        {
            juce::MidiBuffer midi;
            juce::AudioBuffer<float> b (2, 256), kopie (2, 256);
            juce::Random w (11);
            fuelle (b, w);
            b.setSample (0, 10, std::numeric_limits<float>::quiet_NaN());
            b.setSample (1, 20, std::numeric_limits<float>::infinity());
            b.setSample (0, 30, -std::numeric_limits<float>::infinity());
            kopie.makeCopyOf (b);
            q.processBlock (b, midi);
            for (int k = 0; k < 2; ++k)
                for (int n = 0; n < 256; ++n)
                    if (std::memcmp (b.getReadPointer (k) + n, kopie.getReadPointer (k) + n, sizeof (float)) != 0) return false;
            return true;
        };
        const bool ausgeschaltet = nichtEndlichBleibt (*p);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        z.werte[(size_t) iGlobal ("v1.global.bypass")].b = true;
        setzeHoerbaresBand (z);
        setze (*p, z);
        const bool hardBypass = nichtEndlichBleibt (*p);
        pruefe (ausgeschaltet && hardBypass && p->dspKernFuerTest().nichtEndlicheEingaenge() == 0,
                "passthrough_sanitisiert_nichts: NaN, +Inf und -Inf kommen bitgleich heraus, ausgeschaltet und im Hard-Bypass; kein Zaehler steigt",
                "Zaehler " + juce::String ((juce::int64) p->dspKernFuerTest().nichtEndlicheEingaenge()));
    }

    // -- 6. M-02: engagiert, aber neutral ----------------------------------
    abschnitt ("6. M-02 eq_an_bypass_aus_alles_neutral_ist_bitidentisch");
    for (const double rate : { 44100.0, 96000.0 })
    {
        auto p = vorbereitet (rate, 2048);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        const auto e = setze (*p, z);
        juce::Random wuerfel ((juce::int64) rate + 2);
        const auto lauf = fahreNull (*p, 1000, wuerfel);
        int aktiv = -1, quelle = -1, kandidat = -1, kandidatQuelle = -1;
        p->dspKernFuerTest().gefahreneSlots (aktiv, quelle, kandidat, kandidatQuelle);
        const bool rechnet = aktiv >= 0 && p->dspKernFuerTest().pool().bank (aktiv).programm.eqEngagiert
                          && ! p->dspKernFuerTest().pool().bank (aktiv).programm.hardBypass;
        pruefe (e.ausgang == tx::Ausgang::commit && lauf.abweichend == 0 && rechnet,
                "eq_an_bypass_aus_alles_neutral_ist_bitidentisch bei " + juce::String (rate, 0) + " Hz - samt Engagier-Fade, durch Trims, M/S und Filterbank",
                juce::String (lauf.samples) + " Samples, " + juce::String (lauf.abweichend) + " abweichend, aktive Bank " + juce::String (aktiv));
    }

    // -- 7. M-05: der Hard-Bypass ------------------------------------------
    abschnitt ("7. M-05 eq_an_bypass_an_ist_bitidentisch");
    {
        auto p = vorbereitet (48000.0, 2048);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        z.werte[(size_t) iGlobal ("v1.global.bypass")].b = true;
        z.werte[(size_t) iGlobal ("v1.global.width")].zahl = 1.7;
        setzeHoerbaresBand (z);
        const auto e = setze (*p, z);
        juce::Random wuerfel (5);
        const auto lauf = fahreNull (*p, 1000, wuerfel);
        int aktiv = -1, quelle = -1, kandidat = -1, kandidatQuelle = -1;
        p->dspKernFuerTest().gefahreneSlots (aktiv, quelle, kandidat, kandidatQuelle);
        pruefe (e.ausgang == tx::Ausgang::commit && lauf.abweichend == 0 && aktiv >= 0
                    && p->dspKernFuerTest().pool().bank (aktiv).programm.hardBypass,
                "eq_an_bypass_an_ist_bitidentisch: +12-dB-Bell und Width 1,7 liegen hinter dem Hard-Bypass und schlagen nicht durch",
                juce::String (lauf.samples) + " Samples, " + juce::String (lauf.abweichend) + " abweichend");
    }

    // -- 8. M-04: nach dem Ausschalten wieder bitidentisch -----------------
    abschnitt ("8. M-04 nach_dem_fade_wieder_bitidentisch");
    {
        auto p = vorbereitet (48000.0, 2048);
        auto z = p->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        setzeHoerbaresBand (z);
        setze (*p, z);
        juce::Random wuerfel (21);
        const auto an = fahreNull (*p, 200, wuerfel);
        auto aus = p->bestaetigterZustand();
        aus.werte[(size_t) param::kIndexEqEnabled].b = false;
        const auto e = setze (*p, aus);
        // Der Fade laeuft kFadeSamples Samples ab dem ersten Block nach der
        // Publikation; gezaehlt wird ab dem ersten Sample danach.
        const auto nach = fahreNull (*p, 400, wuerfel, nakama::dsp::kFadeSamples);
        int aktiv = -1, quelle = -1, kandidat = -1, kandidatQuelle = -1;
        p->dspKernFuerTest().gefahreneSlots (aktiv, quelle, kandidat, kandidatQuelle);
        pruefe (an.abweichend > 0 && e.ausgang == tx::Ausgang::commit && nach.abweichend == 0 && aktiv < 0 && quelle < 0,
                "nach_dem_fade_wieder_bitidentisch: nach " + juce::String (nakama::dsp::kFadeSamples)
                    + " Fade-Samples bitgleich, keine Bank faehrt mehr",
                "eingeschaltet " + juce::String (an.abweichend) + " abweichende Samples (das Band war hoerbar); danach "
                    + juce::String (nach.abweichend) + " abweichend in " + juce::String (nach.samples - nakama::dsp::kFadeSamples));
    }

    // -- 9. M-33: Mix 0 ----------------------------------------------------
    abschnitt ("9. M-33 mix_null_ist_bitidentisch");
    {
        auto zustandMitMix = [] (double mix)
        {
            param::DspSatz z;
            z.werte[(size_t) param::kIndexEqEnabled].b = true;
            setzeHoerbaresBand (z);
            z.werte[(size_t) iGlobal ("v1.global.input_trim_db")].zahl = 9.0;
            z.werte[(size_t) param::kIndexMix].zahl = mix;
            return z;
        };
        const std::int64_t einschwingen = nakama::dsp::kFadeSamples + nakama::dsp::kRampeSamples;
        auto p = vorbereitet (48000.0, 2048);
        const auto e = setze (*p, zustandMitMix (0.0));
        juce::Random wuerfel (33);
        const auto lauf = fahreNull (*p, 1000, wuerfel, einschwingen);
        auto gegen = vorbereitet (48000.0, 2048);
        setze (*gegen, zustandMitMix (1.0));
        juce::Random wuerfelGegen (33);
        const auto gegenLauf = fahreNull (*gegen, 1000, wuerfelGegen, einschwingen);
        pruefe (e.ausgang == tx::Ausgang::commit && lauf.abweichend == 0 && gegenLauf.abweichend > 0,
                "mix_null_ist_bitidentisch: trotz +9 dB Input-Trim und +12-dB-Bell, Output-Trim 0 dB",
                "Mix 0: " + juce::String (lauf.abweichend) + " abweichend; Gegenprobe Mix 1: "
                    + juce::String (gegenLauf.abweichend) + " abweichend (das Band ist hoerbar)");
    }

    // -- 10. M-93: Speichern, Laden, Speichern im Layout v2 ----------------
    abschnitt ("10. M-93 speichern_laden_speichern_bytegleich_in_v2");
    {
        auto a = vorbereitet (48000.0, 512);
        auto z = a->bestaetigterZustand();
        z.werte[(size_t) param::kIndexEqEnabled].b = true;
        setzeHoerbaresBand (z);
        z.zonen = { { 2, 3000.0, 4000.0, true } };
        setze (*a, z);
        auto z2 = a->bestaetigterZustand();
        z2.werte[(size_t) iBand (0, param::kGainDb)].zahl = -3.0;
        z2.werte[(size_t) iBand (0, param::kQ)].zahl = 0.7071067811865476;
        setze (*a, z2);
        tx::Auftrag undo;
        undo.tid = a->neueTid();
        undo.baseRevision = a->stateRevision();
        undo.art = tx::Art::undo;
        a->fuehreTransaktionAus (undo);

        juce::MemoryBlock erst;
        a->getStateInformation (erst);
        auto b = std::make_unique<Prozessor>();
        b->setStateInformation (erst.getData(), (int) erst.getSize());
        juce::MemoryBlock zweit;
        b->getStateInformation (zweit);

        const auto baum = juce::ValueTree::readFromData (erst.getData(), erst.getSize());
        const auto dsp  = baum.getChildWithName ("Dsp");
        const auto par  = baum.getChildWithName ("Parameters");
        pruefe (erst == zweit && dsp.isValid() && (juce::int64) dsp.getProperty ("state_revision") == (juce::int64) a->stateRevision()
                    && par.isValid() && (int) par.getProperty ("dsp_schema_version") == 2
                    && b->stateRevision() == a->stateRevision() && b->stateHashText() == a->stateHashText()
                    && ! b->zustandLesen().nurLesen,
                "speichern_laden_speichern_bytegleich_in_v2: Kind Dsp mit Revision, Zone und Undo-Ring",
                juce::String ((int) erst.getSize()) + " Bytes, Revision " + juce::String ((juce::int64) a->stateRevision()));
    }

    // -- 11. Gegenpfad: speichern <-> laden (Common, Klassifikation) --------
    // Invariante des Hauses: beide Haelften im selben Aenderungssatz.
    abschnitt ("11. Speichern und Laden: Identitaet und Klassifikation");
    {
        juce::MemoryBlock bytes;
        prozessor->getStateInformation (bytes);
        pruefe (bytes.getSize() > 0, "Zustand laesst sich speichern",
                juce::String ((int) bytes.getSize()) + " Bytes");

        const auto vorher = prozessor->zustandLesen().common;
        auto zweiter = std::make_unique<Prozessor>();
        zweiter->setStateInformation (bytes.getData(), (int) bytes.getSize());

        pruefe (zweiter->zustandLesen().common == vorher,
                "geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)");
        pruefe (! zweiter->zustandLesen().nurLesen,
                "der eigene Stand kommt NICHT als read-only zurueck",
                zweiter->zustandLesen().grund);
        pruefe (zweiter->klassifikation()
                    == (nakama::sonde::kProduktklasse == nakama::state::Klasse::passive_probe
                            ? nakama::state::Klassifikation::passive_probe
                            : nakama::state::Klassifikation::active_probe),
                "nach gueltigem State traegt der Lebenslauf die Produktklasse",
                nakama::state::wort (zweiter->klassifikation()));

        juce::MemoryBlock zurueck;
        zweiter->getStateInformation (zurueck);
        pruefe (zurueck == bytes, "speichern -> laden -> speichern ist bytegleich",
                juce::String ((int) zurueck.getSize()) + " Bytes");
    }

    // -- 11a. Probeeq ist ein gebundener reiner v3-Connector ---------------
    // Die Testschale startet bewusst keine Produktpipe; sie greift aber auf
    // die echten, im Produktprozessor gehaltenen Control-/Telemetry-Clients
    // und deren echte Provider zu.
    {
        nakama::state::Zustand gebunden =
            nakama::state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        gebunden.common.klasse = nakama::state::Klasse::active_probe;
        gebunden.common.projectBindingId = "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb";
        gebunden.hatParameters = true;
        juce::MemoryBlock bytes;
        nakama::state::speichere (gebunden, bytes);

        auto connector = std::make_unique<Prozessor>();
        connector->setStateInformation (bytes.getData(), (int) bytes.getSize());
        connector->setRateAndBufferSizeDetails (48000.0, 512);
        connector->prepareToPlay (48000.0, 512);
        const auto hello = connector->v3HelloFuerTest();
        const auto status = connector->v3StatusFuerTest();
        juce::String sollHash, grund;
        const bool hashOk = nakama::parameter::stateHash (gebunden.dspDto(), sollHash, grund);

        pruefe (hello.adresse.projectBindingId == "bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"
                 && hello.adresse.instanceId == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                 && hello.pluginKind == "active_probe"
                 && hello.adresse.sessionEpoch == hello.adresse.projectBindingId
                 && istHex (hello.adresse.runtimeNonce, 32),
                "Probeeq-Controlprovider erfindet keine eigene Session-Epoche");
        // SONDE-015 4a: Revision und Hash kommen aus dem Transaktionskern. Der
        // geladene Stand traegt state_revision 0; bis 4a zaehlte hier ein
        // Ladezaehler, der mit einer bestaetigten Revision nichts zu tun hatte.
        pruefe (status.dspSchemaVersion == nakama::parameter::kDspSchemaVersion
                    && status.stateRevision == 0 && status.stateRevision == connector->stateRevision()
                    && hashOk && status.stateHash == sollHash.toStdString()
                    && status.stateHash == connector->stateHashText().toStdString()
                    && ! status.recordStateValid && ! status.recording,
                "Probeeq-Stateprovider meldet Revision und Hash des Transaktionskerns und keinen erfundenen Record-State");
        pruefe (connector->controlV3FuerTest().status == nakama::ipc::ControlClient::Status::getrennt
                    && connector->telemetryV3FuerTest().status == nakama::ipc::TelemetryClient::Status::getrennt
                    && ! connector->darfBrokerStarten(),
                "Testschale belegt beide Connectoren; Probeeq besitzt keinen Startpfad");
        pruefe (connector->v3ProduktstatusVerdrahtetFuerTest(),
                "Probeeq-ControlClient traegt seinen produktiven Statusprovider");

        const auto produktquelle = wurzel()
            .getChildFile ("eq-copilot/plugin/sonde/SondeProcessor.cpp")
            .loadFileAsString();
        pruefe (produktquelle.contains ("controlV3.start();")
                    && produktquelle.contains ("telemetryV3.start();")
                    && ! produktquelle.contains ("BrokerLifecycle"),
                "Probeeq startet produktiv beide v3-Connectoren, aber niemals einen Broker");
    }

    // -- 5b. Gate 7 auf State-Ebene, gemessen AM BUNDLE (G1 §4.2) ----------
    // Der Gate-Lauf G1 vom 24.08.2026 ist genau diese Kette gefahren:
    // Host-State-Restore -> lade() -> positionAusWort -> positionErlaubt ->
    // uebernommen -> beim naechsten Speichern wieder hinausgeschrieben. Eine
    // Sonde mit genau einem Stereo-Bus (SondeProcessor.cpp:7-9) konnte sich so
    // dauerhaft `post_fader_contribution` nennen - die exakte Bezeichnung
    // eines Mastersummenbeitrags auf einem Aux-Bus, den dieses Bundle nicht
    // hat. Das ist Gate 7 aus §49.2 Nr. 7 im Wortlaut.
    //
    // Der Riegel dagegen sitzt seit a2fe0f5 in `positionErlaubt`
    // (state/NakamaState.cpp) und hat zwei Haelften: Riegel 1 ist die
    // CAPABILITY-Vorpruefung (`kContributionAuxVerfuegbar`, heute gemessen
    // unsupported), Riegel 2 die Klassenmatrix. Beide lehnen
    // `post_fader_contribution` heute fuer jede Klasse ab; von aussen ist nur
    // das GEMEINSAME Ergebnis sichtbar, nicht welche Haelfte greift. Die
    // VOLLSTAENDIGE Matrix - alle vier Klassen, jede in dem Bundle, das sie
    // zulaesst, `passive_probe` also mit Bundlevertrag `nkpr()` - misst B2
    // `EqCopStateMigrationTest` (Block G8b) auf `lade()`-Ebene.
    //
    // 🔑 Hier faehrt die GANZE Kette durch die echte Sondenschale. Das ist
    // keine Wiederholung: `SondeProcessor::setStateInformation` hat einen
    // eigenen Weg - bei `ignoriert` kehrt er VOR dem Schloss um, bei
    // `nurLesen` nicht -, und das Artefakt dieses Tickets ist das Bundle,
    // nicht `lade()`.
    //
    // ⚠️ WIE WEIT DAS TRAEGT - genau und nicht weiter: die Schale traegt
    // GENAU EINE Klasse je Uebersetzung (`kProduktklasse`), und gebaut wird
    // seit S9b/`SONDE-007c` nur noch `active_probe`. Der Durchgriff durch die
    // Schale ist damit fuer DIESE EINE Klasse gemessen, nicht fuer vier. Dass
    // `positionErlaubt` die Position fuer ALLE VIER Klassen ablehnt, misst
    // Punkt 7 unten (direkt an der Funktion) und B2 auf `lade()`-Ebene - beide
    // messen das gemeinsame Ergebnis beider Riegelhaelften, keine von beiden
    // trennt sie auf. Der urspruengliche G1-§4.2-Traeger - eine passive Sonde
    // als PRODUKT - existiert seit S9b nicht mehr.
    {
        // Ein sonst GUELTIGER Stand dieses Bundles: eigene Produktklasse,
        // eigener Bundlevertrag, richtige Kind-Matrix (§2.1) - nur die
        // Messposition ist die verbotene. Gebaut ueber den oeffentlichen
        // State-Weg (`speichere`), nicht ueber eine Hintertuer im Produktcode.
        nakama::state::Zustand gebastelt =
            nakama::state::frisch ("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
        gebastelt.common.klasse   = nakama::sonde::kProduktklasse;
        gebastelt.common.position = nakama::state::Messposition::post_fader_contribution;
        gebastelt.hatParameters   = (nakama::sonde::kProduktklasse
                                        == nakama::state::Klasse::active_probe);

        juce::MemoryBlock verboten;
        nakama::state::speichere (gebastelt, verboten);

        // Ohne diese Probe waere der ganze Block aus dem FALSCHEN Grund gruen:
        // haette `speichere` die Position stillschweigend begradigt, gaebe es
        // unten gar nichts mehr abzuwehren.
        const auto probe = juce::ValueTree::readFromData (verboten.getData(), verboten.getSize());
        const auto probeWort = probe.isValid()
            ? probe.getChildWithName ("Common").getProperty ("measurement_position").toString()
            : juce::String ("<kein Baum>");
        pruefe (probeWort == "post_fader_contribution",
                "die Probebytes tragen wirklich measurement_position=post_fader_contribution",
                probeWort);

        auto opfer = std::make_unique<Prozessor>();
        opfer->setStateInformation (verboten.getData(), (int) verboten.getSize());

        // 1) Der Stand wird NICHT als eigener uebernommen: `leseSchema2`
        //    bricht an `positionErlaubt` ab, `lade()` faellt auf read-only mit
        //    Grund, und der Prozessor haelt genau diesen Zustand.
        pruefe (opfer->zustandLesen().nurLesen,
                "insert_probe_is_never_called_master_sum: der Stand kommt als read-only zurueck, nicht als eigener",
                opfer->zustandLesen().grund);
        pruefe (opfer->zustandLesen().grund.isNotEmpty(),
                "read-only nennt seinen Grund");
        pruefe (opfer->zustandLesen().common.position
                    != nakama::state::Messposition::post_fader_contribution,
                "das Bundle FUEHRT die verbotene Position nicht",
                nakama::state::wort (opfer->zustandLesen().common.position));

        // 2) Die Instanz bleibt neutral - keine Klassifikation auf die
        //    Produktklasse, kein Brokerstart. §53.5: read-only ist das
        //    Gegenteil eines vollstaendigen State-Restore.
        pruefe (opfer->klassifikation() == nakama::state::Klassifikation::unclassified,
                "die Instanz klassifiziert NICHT auf die Produktklasse, sie bleibt neutral",
                nakama::state::wort (opfer->klassifikation()));
        pruefe (! opfer->darfBrokerStarten(),
                "eine read-only-Sonde darf den Broker nicht starten");

        // 3) §53.8 verlustfrei: die Originalbytes reisen unveraendert zum Host
        //    zurueck. Ein Altprojekt verliert seinen Stand nicht - es darf ihn
        //    nur nicht mehr behaupten.
        juce::MemoryBlock zurueck;
        opfer->getStateInformation (zurueck);
        pruefe (zurueck == verboten,
                "Vertrag 53.8: dieselben Originalbytes gehen unveraendert an den Host zurueck",
                juce::String ((int) zurueck.getSize()) + " Bytes");

        // 4) Und der Rueckweg waescht nichts: eine DRITTE frische Instanz, die
        //    genau die herausgegebenen Bytes laedt, kommt wieder read-only und
        //    neutral. Damit gibt es keinen Umweg, ueber den das Bundle die
        //    Position doch als gueltigen eigenen Stand fuehrt.
        auto dritte = std::make_unique<Prozessor>();
        dritte->setStateInformation (zurueck.getData(), (int) zurueck.getSize());
        pruefe (dritte->zustandLesen().nurLesen,
                "der Rueckweg waescht nichts: erneut geladen bleibt read-only",
                dritte->zustandLesen().grund);
        pruefe (dritte->klassifikation() == nakama::state::Klassifikation::unclassified,
                "und erneut geladen bleibt die Instanz neutral",
                nakama::state::wort (dritte->klassifikation()));

        // 5) Gegenprobe: DERSELBE Stand mit einer fuer die Klasse erlaubten
        //    Position laedt normal und klassifiziert. Ohne sie wuesste
        //    niemand, ob oben die POSITION abgewiesen wurde oder irgendetwas
        //    anderes am Bastelstand.
        gebastelt.common.position = nakama::state::Messposition::insert;
        juce::MemoryBlock erlaubteBytes;
        nakama::state::speichere (gebastelt, erlaubteBytes);

        auto gegenprobe = std::make_unique<Prozessor>();
        gegenprobe->setStateInformation (erlaubteBytes.getData(), (int) erlaubteBytes.getSize());
        pruefe (! gegenprobe->zustandLesen().nurLesen,
                "Gegenprobe: derselbe Stand mit erlaubter Position laedt normal",
                gegenprobe->zustandLesen().grund);
        pruefe (gegenprobe->zustandLesen().common == gebastelt.common,
                "Gegenprobe: der geladene Common ist derselbe (Position insert)",
                nakama::state::wort (gegenprobe->zustandLesen().common.position));
        pruefe (gegenprobe->klassifikation()
                    == (nakama::sonde::kProduktklasse == nakama::state::Klasse::passive_probe
                            ? nakama::state::Klassifikation::passive_probe
                            : nakama::state::Klassifikation::active_probe),
                "Gegenprobe: nach gueltigem Stand traegt der Lebenslauf die Produktklasse",
                nakama::state::wort (gegenprobe->klassifikation()));

        // 6) Und der Weg, den FL wirklich geht: die Instanz steht laengst im
        //    Projekt und ist klassifiziert, DANN reicht der Host ihr den
        //    verbotenen Stand nach (Preset-Browser, Copy/Paste, geoeffnetes
        //    Altprojekt). Punkt 1-5 haben nur frische Instanzen gemessen -
        //    haette der Riegel hier eine Luecke, waere sie die einzige, die im
        //    Betrieb ueberhaupt erreichbar ist.
        //
        //    §53.5: read-only ist das Gegenteil eines vollstaendigen Restore,
        //    also faellt auch eine ZUVOR positiv klassifizierte Instanz auf
        //    neutral zurueck. Sie darf ihre Rechte nicht behalten, nur weil sie
        //    sie einmal hatte.
        pruefe (gegenprobe->klassifikation() != nakama::state::Klassifikation::unclassified,
                "Nachreichen: die Instanz ist VOR dem verbotenen Stand klassifiziert",
                nakama::state::wort (gegenprobe->klassifikation()));

        gegenprobe->setStateInformation (verboten.getData(), (int) verboten.getSize());

        pruefe (gegenprobe->zustandLesen().nurLesen,
                "Nachreichen: der verbotene Stand kommt auch bei einer laufenden Instanz read-only",
                gegenprobe->zustandLesen().grund);
        pruefe (gegenprobe->klassifikation() == nakama::state::Klassifikation::unclassified,
                "Nachreichen: die Klassifikation faellt zurueck auf neutral, alte Rechte bleiben nicht",
                nakama::state::wort (gegenprobe->klassifikation()));

        // §53.8 auch hier: der Prozessor erfindet keinen Stand und faellt auch
        // nicht auf den vorherigen zurueck - er gibt heraus, was der Host ihm
        // gegeben hat. Alles andere waere ein stiller Stand-Tausch hinter dem
        // Ruecken des Projekts.
        juce::MemoryBlock nachReichen;
        gegenprobe->getStateInformation (nachReichen);
        pruefe (nachReichen == verboten,
                "Nachreichen: der Host bekommt genau die Bytes zurueck, die er gab - kein stiller Tausch",
                juce::String ((int) nachReichen.getSize()) + " Bytes");

        // 7) `positionErlaubt` lehnt `post_fader_contribution` fuer ALLE VIER
        //    Klassen ab - gemessen an genau der Funktion, an der Punkt 0-6
        //    oben abbiegen.
        //
        //    ⚠️ WAS DIESE PRUEFUNG IST UND WAS SIE NICHT IST. Gemessen wird das
        //    GEMEINSAME Ergebnis beider Riegelhaelften: Riegel 1 (die
        //    Capability-Vorpruefung `kContributionAuxVerfuegbar`) und Riegel 2
        //    (die Klassenmatrix) lehnen die Position heute beide ab. WELCHE
        //    Haelfte im Einzelfall sperrt, sagt diese Pruefung NICHT -
        //    `positionErlaubt` gibt nur `false` zurueck, und eine Abfrage, die
        //    die Haelften trennt, gibt es im Kern nicht. Die Aussage bleibt
        //    deshalb genau: abgelehnt fuer alle vier Klassen.
        //
        //    Punkt 0-6 fahren die ganze Kette durch das echte Bundle, aber nur
        //    fuer EINE Klasse: `kProduktklasse` ist ein Uebersetzungsschalter
        //    (`plugin/CMakeLists.txt`, `nakama_sonde_nulltest`), und seit
        //    S9b/`SONDE-007c` setzt KEIN Bauziel mehr `NAKAMA_SONDE_PASSIV` -
        //    Nakama Suna ist stillgelegt (`SondeProcessor.h:5-12`). Diese
        //    Schale kann heute also nur `active_probe` bauen.
        //
        //    Der urspruengliche Traeger der G1-§4.2-Regression war aber
        //    gerade `passive_probe`: vor `a2fe0f5` gab es Riegel 1 nicht und
        //    die Matrix trug `case Klasse::passive_probe: return true;`. Setzt
        //    man beides auf jenen Stand zurueck, bleiben Punkt 0-6 alle gruen
        //    - sie werden weiter an der unveraenderten `active_probe`-Zeile
        //    abgewiesen. Ohne die vier Zeilen hier faenge dieses Bein die
        //    Regression, gegen die es antritt, NICHT; mit ihnen faellt genau
        //    die `passive_probe`-Zeile (Mutationsprobe Runde 2, SONDE-007b).
        //
        //    Ein passives Bauziel wiederzubeleben ist der falsche Weg dagegen
        //    (es naehme `SONDE-007c` zurueck), eine Testhintertuer im
        //    Produktcode ebenso. Also wird die Ablehnung direkt an der
        //    oeffentlichen `positionErlaubt` gemessen: derselben Funktion, die
        //    die Schale oben ueber `lade()` aufruft.
        //
        //    Die VOLLSTAENDIGE 16er-Matrix samt Bundlevertraegen - jede Klasse
        //    in dem Bundle, das sie zulaesst, `passive_probe` also in `nkpr()`
        //    - misst B2 `EqCopStateMigrationTest`, Block G8b. Nicht dieses
        //    Bein, und das soll es auch nicht: hier steht die Kette am
        //    gebauten Bundle, dort der Vertrag.
        for (const auto klasse : { nakama::state::Klasse::main,
                                   nakama::state::Klasse::passive_probe,
                                   nakama::state::Klasse::active_probe,
                                   nakama::state::Klasse::legacy })
            pruefe (! nakama::state::positionErlaubt (
                        klasse, nakama::state::Messposition::post_fader_contribution),
                    juce::String ("positionErlaubt (Riegel 1 und Klassenmatrix gemeinsam) "
                                  "lehnt post_fader_contribution ab fuer '")
                        + nakama::state::wort (klasse) + "'");
    }
    // -- 6. Muell aendert nichts --------------------------------------------
    {
        const auto vorher = prozessor->zustandLesen().common;
        const char muell[] = "das ist kein NakamaState";
        prozessor->setStateInformation (muell, (int) sizeof (muell));
        pruefe (prozessor->zustandLesen().common == vorher,
                "Muellbytes lassen den gehaltenen Zustand unveraendert");
        prozessor->setStateInformation (nullptr, 0);
        pruefe (prozessor->zustandLesen().common == vorher,
                "Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert");
    }

    std::cout << std::endl
              << (fehlerZahl == 0 ? "SONDE-NULLTEST OK - " : "SONDE-NULLTEST FEHLGESCHLAGEN - ")
              << okZahl << " Pruefungen ok, " << fehlerZahl << " Fehler" << std::endl;
    return fehlerZahl == 0 ? 0 : 1;
}
