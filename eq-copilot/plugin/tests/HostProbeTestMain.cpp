// Selbsttest des Termin-B-Messgeraets (Session S3b).
//
// Zweck: das Lineal beweisen, BEVOR der User damit in FL misst. Ein Messgeraet,
// das man erst am Messobjekt kennenlernt, liefert keine Messung, sondern eine
// Vermutung. Deshalb wird hier jede Frage, die Termin B beantworten soll, mit
// synthetischen Blockbefunden durchgespielt - inklusive der Faelle, in denen
// das Geraet ausdruecklich NICHTS melden darf (Fehlalarm-Riegel).
//
// Exit 0 nur bei "HOSTPROBE OK".

#include "HostProbeProcessor.h"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <new>

using namespace eqcop::hostprobe;
using namespace eqcop::hostbruecke;

//==============================================================================
namespace
{
    bool          zaehleAllokationen = false;
    std::uint64_t allokationen       = 0;
}

void* operator new (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

//==============================================================================
namespace
{

int fehler = 0;
int geprueft = 0;

void pruefe (bool ok, const juce::String& name)
{
    ++geprueft;
    std::cout << (ok ? "  ok      " : "  FEHLER  ") << name << std::endl;
    if (! ok) ++fehler;
}

juce::uint32 lcg = 0x13579bdfu;
float zufall()
{
    lcg = lcg * 1664525u + 1013904223u;
    return ((float) (lcg >> 8) / (float) 0x00ffffff) * 2.0f - 1.0f;
}

/** Baut einen Blockbefund mit vorhandenem Context. */
Blockbefund mitKontext (juce::int64 projektZeit, bool spielt, std::uint32_t blockGroesse,
                        bool schleifeAktiv = false, bool schleifeGueltig = false)
{
    Blockbefund b;
    b.blockGroesse = blockGroesse;
    b.sampleAccurateAutomation = true;
    auto& k = b.kontext;
    k.processContextPresent = true;
    k.projectTimeSamples.setze (projektZeit);
    k.sampleRate.setze (48000.0);
    k.playing.setze (spielt);
    k.recording.setze (false);
    k.tempo.setze (120.0);
    k.ppqPosition.setze ((double) projektZeit / 24000.0);
    k.cycle.aktiv   = schleifeAktiv;
    k.cycle.gueltig = schleifeGueltig;
    k.cycle.startPpq = 0.0;
    k.cycle.endePpq  = 16.0;
    return b;
}

Blockbefund ohneKontext (std::uint32_t blockGroesse)
{
    Blockbefund b;
    b.blockGroesse = blockGroesse;
    b.sampleAccurateAutomation = false;
    return b;
}

/** Ein Block so, wie ihn der Host liefert: erst Senke, dann processBlock. */
void hostBlock (HostProbeProcessor& p, const Blockbefund& b,
                juce::AudioBuffer<float>& puffer, juce::MidiBuffer& midi)
{
    p.nakamaBlockEmpfangen (b);
    p.processBlock (puffer, midi);
}

} // namespace

//==============================================================================
int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;

    std::cout << "== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==" << std::endl;
    {
        HostProbeProcessor p;
        p.setPlayConfigDetails (2, 2, 48000.0, 512);
        p.prepareToPlay (48000.0, 512);

        pruefe (p.getLatencySamples() == 0, "meldet 0 Samples Latenz");
        pruefe (p.getTailLengthSeconds() == 0.0, "meldet 0 s Tail");

        juce::AudioBuffer<float> puffer (2, 512), kopie (2, 512);
        juce::MidiBuffer midi;
        bool gleich = true;
        for (int block = 0; block < 200 && gleich; ++block)
        {
            for (int k = 0; k < 2; ++k)
            {
                auto* d = puffer.getWritePointer (k);
                for (int i = 0; i < 512; ++i) d[i] = zufall();
            }
            kopie.makeCopyOf (puffer);
            hostBlock (p, mitKontext (block * 512, true, 512), puffer, midi);
            for (int k = 0; k < 2 && gleich; ++k)
                gleich = std::memcmp (puffer.getReadPointer (k), kopie.getReadPointer (k),
                                      512 * sizeof (float)) == 0;
        }
        pruefe (gleich, "200 Bloecke float: Ausgang bitgleich zum Eingang");

        juce::AudioBuffer<double> dPuffer (2, 256), dKopie (2, 256);
        for (int k = 0; k < 2; ++k)
        {
            auto* d = dPuffer.getWritePointer (k);
            for (int i = 0; i < 256; ++i) d[i] = (double) zufall();
        }
        dKopie.makeCopyOf (dPuffer);
        p.processBlock (dPuffer, midi);
        bool dGleich = true;
        for (int k = 0; k < 2 && dGleich; ++k)
            dGleich = std::memcmp (dPuffer.getReadPointer (k), dKopie.getReadPointer (k),
                                   256 * sizeof (double)) == 0;
        pruefe (dGleich, "double-Block: Ausgang bitgleich zum Eingang");
        pruefe (p.supportsDoublePrecisionProcessing(), "double-Praezision wird dem Host angeboten");

        juce::AudioProcessor::BusesLayout falsch;
        falsch.inputBuses.add (juce::AudioChannelSet::create5point1());
        falsch.outputBuses.add (juce::AudioChannelSet::create5point1());
        pruefe (! p.checkBusesLayoutSupported (falsch), "5.1 wird klar abgelehnt, nicht still gemischt");
    }

    std::cout << "== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==" << std::endl;
    {
        HostProbeProcessor p;
        juce::AudioProcessor* alsBasis = &p;
        auto* alsSenke = dynamic_cast<eqcop::hostbruecke::Senke*> (alsBasis);
        pruefe (alsSenke != nullptr,
                "dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch");

        HostProbeProcessor frisch;
        pruefe (! frisch.brueckeLiefert(), "vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT");
        frisch.nakamaBlockEmpfangen (ohneKontext (512));
        pruefe (frisch.brueckeLiefert(), "nach dem ersten Bruecken-Block meldet es: Bruecke liefert");
    }

    std::cout << "== C - Context-Anwesenheit und Gueltigkeiten ==" << std::endl;
    {
        HostProbeProcessor p;
        p.setPlayConfigDetails (2, 2, 48000.0, 512);
        p.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> puffer (2, 512);
        juce::MidiBuffer midi;

        for (int i = 0; i < 5; ++i) hostBlock (p, mitKontext (i * 512, true, 512), puffer, midi);
        hostBlock (p, ohneKontext (512), puffer, midi);
        for (int i = 0; i < 3; ++i) hostBlock (p, mitKontext (10000 + i * 512, true, 512), puffer, midi);

        const auto s = p.messstand();
        pruefe (s.bloeckeMitKontext == 8, "acht Bloecke mit Context gezaehlt");
        pruefe (s.bloeckeOhneKontext == 1, "der eine Block OHNE Context ist getrennt gezaehlt");
        pruefe (s.verarbeiteteBloecke == 9, "neun verarbeitete Bloecke");
        pruefe (s.senkeAufrufe == 9, "neun Senke-Aufrufe - keine Asymmetrie ohne Grund");
        pruefe (s.artZaehler[(int) Art::KontextWeg] >= 1, "der Kontextverlust ist als Ereignis vermerkt");
        pruefe (s.jeGueltig.tempo && s.immerGueltig.tempo, "Tempo war in jedem Context gueltig");
        pruefe (! s.jeGueltig.fortlaufend, "continousTimeSamples war NIE gueltig - und wird nicht erfunden");
    }

    std::cout << "== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==" << std::endl;
    {
        HostProbeProcessor p;
        p.setPlayConfigDetails (2, 2, 48000.0, 512);
        p.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> puffer (2, 512);
        juce::MidiBuffer midi;

        // Fortlaufendes Spiel: KEIN Sprung.
        for (int i = 0; i < 20; ++i) hostBlock (p, mitKontext (i * 512, true, 512), puffer, midi);
        auto s = p.messstand();
        pruefe (s.zeitspruengeVor == 0 && s.zeitspruengeZurueck == 0,
                "fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)");

        // Seek vorwaerts.
        hostBlock (p, mitKontext (500000, true, 512), puffer, midi);
        s = p.messstand();
        pruefe (s.zeitspruengeVor == 1, "Seek vorwaerts wird als Sprung erkannt");
        pruefe (s.groessterSprungVor > 400000, "die Sprungweite wird mitgemessen");

        // Seek zurueck.
        hostBlock (p, mitKontext (1000, true, 512), puffer, midi);
        s = p.messstand();
        pruefe (s.zeitspruengeZurueck == 1, "Seek rueckwaerts wird als Sprung erkannt");

        // Stop, dann wieder Play an anderer Stelle: der erste Block nach dem
        // Transportwechsel darf KEIN Sprung sein.
        const auto vorher = p.messstand();
        hostBlock (p, mitKontext (1512, false, 512), puffer, midi);   // Stop
        hostBlock (p, mitKontext (777777, true, 512), puffer, midi);  // Play woanders
        hostBlock (p, mitKontext (777777 + 512, true, 512), puffer, midi);
        s = p.messstand();
        pruefe (s.zeitspruengeVor == vorher.zeitspruengeVor
                && s.zeitspruengeZurueck == vorher.zeitspruengeZurueck,
                "nach Stop/Play wird der Positionswechsel NICHT als Sprung gemeldet");
        pruefe (s.artZaehler[(int) Art::TransportAus] >= 1 && s.artZaehler[(int) Art::TransportAn] >= 1,
                "Stop und Play sind als Transportereignisse vermerkt");

        // Schleife: Rueckwaertssprung mit aktivem Cycle.
        HostProbeProcessor q;
        q.setPlayConfigDetails (2, 2, 48000.0, 512);
        q.prepareToPlay (48000.0, 512);
        for (int i = 0; i < 4; ++i)
            hostBlock (q, mitKontext (i * 512, true, 512, true, true), puffer, midi);
        hostBlock (q, mitKontext (0, true, 512, true, true), puffer, midi);   // Loop-Wechsel
        const auto sq = q.messstand();
        pruefe (sq.zeitspruengeZurueck == 1, "Loop-Wechsel erscheint als Rueckwaertssprung");
        pruefe (sq.artZaehler[(int) Art::SchleifeAn] == 1, "Schleife-an ist genau einmal vermerkt");
        pruefe (sq.schleifeAktiv, "der Schleifenzustand wird gefuehrt");

        // Smart Disable: Luecke bei laufendem Transport.
        HostProbeProcessor r;
        r.setPlayConfigDetails (2, 2, 48000.0, 512);
        r.prepareToPlay (48000.0, 512);
        for (int i = 0; i < 3; ++i) hostBlock (r, mitKontext (i * 512, true, 512), puffer, midi);
        hostBlock (r, mitKontext (3 * 512 + 48000, true, 512), puffer, midi);   // 1 s fehlt
        const auto sr = r.messstand();
        pruefe (sr.zeitspruengeVor == 1 && sr.groessterSprungVor == 48000,
                "Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)");
    }

    std::cout << "== E - Samplegenaue Automation: der Kernbeweis von Termin B ==" << std::endl;
    {
        HostProbeProcessor p;
        p.setPlayConfigDetails (2, 2, 48000.0, 512);
        p.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> puffer (2, 512);
        juce::MidiBuffer midi;

        auto s = p.messstand();
        pruefe (s.automationPunkte == 0 && s.bloeckeMitMehrpunkt == 0,
                "ohne Automation wird nichts behauptet");

        // Ein einzelner Punkt je Block - das kann JUCE auch ohne Bruecke.
        ParameterEvent einer[1] { { 1, 0, 0.25f } };
        auto b1 = mitKontext (0, true, 512);
        b1.ereignisse = einer; b1.anzahl = 1;
        hostBlock (p, b1, puffer, midi);
        s = p.messstand();
        pruefe (s.automationPunkte == 1 && s.bloeckeMitMehrpunkt == 0,
                "ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben");

        // Drei Punkte mit Offsets - das geht NUR mit dem Bridge-Patch.
        ParameterEvent drei[3] { { 1, 0, 0.10f }, { 1, 128, 0.50f }, { 1, 400, 0.90f } };
        auto b3 = mitKontext (512, true, 512);
        b3.ereignisse = drei; b3.anzahl = 3;
        hostBlock (p, b3, puffer, midi);
        s = p.messstand();
        pruefe (s.automationPunkte == 4, "alle Punkte werden aufsummiert");
        pruefe (s.maxPunkteProBlock == 3, "der dichteste Block wird gemerkt");
        pruefe (s.bloeckeMitMehrpunkt == 1, "Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit");
        pruefe (s.kleinsterOffset == 0 && s.groesterOffset == 400,
                "kleinster und groesster Offset werden gefuehrt");
        pruefe (s.artZaehler[(int) Art::AutomationMehrpunkt] == 1,
                "der erste Mehrpunkt-Block ist als Ereignis vermerkt");

        // Unplausible Punkte werden durchgereicht, nicht verschluckt.
        auto bu = mitKontext (1024, true, 512);
        bu.ereignisse = drei; bu.anzahl = 3;
        bu.unplausibleWerte = 2;
        bu.ueberlaeufe = 7;
        hostBlock (p, bu, puffer, midi);
        s = p.messstand();
        pruefe (s.automationUnplausibel == 2 && s.automationUeberlaeufe == 7,
                "Unplausibilitaeten und Ueberlaeufe der Bruecke werden uebernommen, nicht geglaettet");
    }

    std::cout << "== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==" << std::endl;
    {
        HostProbeProcessor p;
        auto ohne = mitKontext (0, true, 512);
        p.nakamaBlockEmpfangen (ohne);
        auto s = p.messstand();
        pruefe (! s.latenzJeGemeldet && s.latenzEingangBus == -1,
                "ohne Hostmeldung: 'nie gemeldet', und der Bus bleibt -1 statt 0");

        auto mit = mitKontext (512, true, 512);
        mit.kontext.presentationLatency.eingang[0].gemeldet = true;
        mit.kontext.presentationLatency.eingang[0].samples  = 0;
        p.nakamaBlockEmpfangen (mit);
        s = p.messstand();
        pruefe (s.latenzJeGemeldet && s.latenzEingangBus == 0 && s.latenzEingangWert == 0,
                "Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten");
    }

    std::cout << "== G - Senke ohne processBlock (Flush/Hostriegel) ==" << std::endl;
    {
        HostProbeProcessor p;
        p.setPlayConfigDetails (2, 2, 48000.0, 512);
        p.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> puffer (2, 512);
        juce::MidiBuffer midi;

        hostBlock (p, mitKontext (0, true, 512), puffer, midi);
        p.nakamaBlockEmpfangen (mitKontext (512, true, 0));     // Flush: kein processBlock
        hostBlock (p, mitKontext (512, true, 512), puffer, midi);

        const auto s = p.messstand();
        pruefe (s.senkeAufrufe == 3 && s.verarbeiteteBloecke == 2,
                "Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt");
        pruefe (s.artZaehler[(int) Art::BlockOhneVerarbeitung] == 1,
                "der Block ohne Verarbeitung ist ausdruecklich vermerkt");
    }

    std::cout << "== H - Gegenpfad: Bericht schreiben und wieder einlesen ==" << std::endl;
    {
        HostProbeProcessor p;
        p.setPlayConfigDetails (2, 2, 48000.0, 512);
        p.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> puffer (2, 512);
        juce::MidiBuffer midi;

        ParameterEvent drei[3] { { 1, 0, 0.10f }, { 1, 128, 0.50f }, { 1, 400, 0.90f } };
        for (int i = 0; i < 6; ++i)
        {
            auto b = mitKontext (i * 512, true, 512);
            if (i == 3) { b.ereignisse = drei; b.anzahl = 3; }
            hostBlock (p, b, puffer, midi);
        }

        const auto text = p.berichtAlsJson();
        pruefe (text.isNotEmpty(), "der Bericht ist nicht leer");

        const auto gelesen = juce::JSON::parse (text);
        pruefe (gelesen.isObject(), "der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)");
        pruefe (gelesen["schema"].toString() == "nakama-hostprobe-1", "Schemaname steht im Bericht");
        pruefe ((bool) gelesen["automation"]["samplegenau_belegt"],
                "der Bericht traegt die Kernaussage 'samplegenau belegt'");
        pruefe ((int) gelesen["bloecke"]["verarbeitete_bloecke"] == 6,
                "die Blockzahl im Bericht deckt sich mit der Messung");
        pruefe (gelesen["ereignisse"].isArray() && gelesen["ereignisse"].size() > 0,
                "das Ereignisprotokoll liegt im Bericht");
        pruefe ((bool) gelesen["gueltig_immer"]["tempo"], "die Gueltigkeitsmaske steht im Bericht");

        // Datei-Gegenpfad: schreiben, wieder lesen, gleicher Inhalt.
        const auto datei = p.berichtSchreiben();
        pruefe (datei.existsAsFile(), "Bericht wurde als Datei geschrieben: " + datei.getFullPathName());
        if (datei.existsAsFile())
        {
            const auto ausDatei = juce::JSON::parse (datei.loadFileAsString());
            pruefe (ausDatei.isObject()
                        && (int) ausDatei["bloecke"]["verarbeitete_bloecke"] == 6,
                    "die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung");
            datei.deleteFile();
        }
    }

    std::cout << "== I - Zuruecksetzen und Ringueberlauf ==" << std::endl;
    {
        HostProbeProcessor p;
        p.setPlayConfigDetails (2, 2, 48000.0, 512);
        p.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> puffer (2, 512);
        juce::MidiBuffer midi;

        for (int i = 0; i < 10; ++i) hostBlock (p, mitKontext (i * 512, true, 512), puffer, midi);
        p.messungZuruecksetzen();
        hostBlock (p, mitKontext (0, true, 512), puffer, midi);
        auto s = p.messstand();
        pruefe (s.bloeckeMitKontext == 1 && s.zeitspruengeVor == 0 && s.zeitspruengeZurueck == 0,
                "Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung");

        // Mehr Ereignisse als der Ring fasst: die Zaehler je Art muessen den
        // Ueberlauf ueberleben, sonst verschwindet ein Befund still.
        HostProbeProcessor q;
        q.setPlayConfigDetails (2, 2, 48000.0, 512);
        q.prepareToPlay (48000.0, 512);
        for (int i = 0; i < kMaxEreignisse + 60; ++i)
        {
            // Jeder Block wechselt den Transport -> ein Ereignis je Block.
            hostBlock (q, mitKontext (i * 512, (i % 2) == 0, 512), puffer, midi);
        }
        const auto sq = q.messstand();
        pruefe (sq.ereignisseGesamt > (juce::int64) kMaxEreignisse,
                "mehr Ereignisse als Ringplaetze wurden erzeugt");
        const auto summeArten = sq.artZaehler[(int) Art::TransportAn] + sq.artZaehler[(int) Art::TransportAus];
        pruefe (summeArten > (juce::int64) kMaxEreignisse,
                "die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still");

        juce::HeapBlock<Ereignis> ziel (kMaxEreignisse);
        const int gelesen = q.ereignisseLesen (ziel.get(), kMaxEreignisse);
        pruefe (gelesen == kMaxEreignisse, "der Ring gibt genau seine Kapazitaet heraus");
        pruefe (ziel[gelesen - 1].block >= ziel[0].block,
                "die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)");
    }

    std::cout << "== J - Audiothread: keine Allokation ==" << std::endl;
    {
        HostProbeProcessor p;
        p.setPlayConfigDetails (2, 2, 48000.0, 512);
        p.prepareToPlay (48000.0, 512);
        juce::AudioBuffer<float> puffer (2, 512);
        juce::MidiBuffer midi;
        ParameterEvent punkte[8];
        for (int i = 0; i < 8; ++i) punkte[i] = ParameterEvent { (StableParameterId) i, (std::uint32_t) (i * 60), 0.5f };

        allokationen = 0;
        zaehleAllokationen = true;
        for (int i = 0; i < 500; ++i)
        {
            auto b = mitKontext (i * 512, (i % 97) != 0, 512, (i % 3) == 0, true);
            b.ereignisse = punkte; b.anzahl = 8;
            hostBlock (p, b, puffer, midi);
        }
        zaehleAllokationen = false;
        pruefe (allokationen == 0,
                "500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen");
    }

    std::cout << std::endl;
    if (fehler == 0)
        std::cout << "HOSTPROBE OK - " << geprueft << "/" << geprueft << " Pruefungen bestanden." << std::endl;
    else
        std::cout << "HOSTPROBE FEHLGESCHLAGEN - " << fehler << " von " << geprueft << " Pruefungen rot." << std::endl;

    return fehler == 0 ? 0 : 1;
}
