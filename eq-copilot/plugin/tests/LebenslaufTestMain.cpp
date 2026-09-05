/*  S9 / SONDE-007b Abschnitt 3 — die Lifecycle-Klassifikation, gemessen.

    GETESTET WIRD §53.5 SATZ FUER SATZ, in zwei Ebenen:

      TEIL 1 — der Automat pur (`nakama::state::Lebenslauf`, im Kern). Hier
               lassen sich auch die Faelle fahren, die ein Bundle im Betrieb
               nie sieht (eine Sonde, der jemand einen Main-Stand reicht).
      TEIL 2 — derselbe Automat VERDRAHTET im echten `EqCopilotProcessor`.
               Ein Automat, der stimmt, aber nirgends haengt, ist ein Riegel,
               der nichts verriegelt. Teil 2 misst deshalb AUDIO: dieselbe
               Markierung, die in EqCopMarkierungTest faerbt, muss hier
               stumm bleiben, solange nicht klassifiziert ist - und faerben,
               sobald es so weit ist.

    Der Scannerlauf ist die wichtigste Zeile dieses Beins. §53.5 sagt "ein
    Scannerlauf klassifiziert nicht"; im Code ist das keine Sonderbehandlung,
    sondern eine Folge: ein Scanner instanziiert, fragt ab, verarbeitet
    vielleicht und zerstoert wieder - er ruft nie `setStateInformation` und
    oeffnet nie einen Editor. Genau diese Sequenz faehrt P1 und misst sie,
    statt sie zu behaupten.

    Aufruf: EqCopLebenslaufTest.exe        Exit 0 gruen, 1 rot.
*/

#include "PluginProcessor.h"
#include "../core/ipc/PipeToken.h"

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>
#include <chrono>
#include <thread>
#include "HoerMarkierung.h"

#include <pluginterfaces/vst/ivstprocesscontext.h>

#include <cmath>
#include <cstring>
#include <iostream>

using namespace eqcop;
using nakama::state::Klasse;
using nakama::state::Klassifikation;
using nakama::state::LadeErgebnis;
using nakama::state::Lebenslauf;

using VstKontext = Steinberg::Vst::ProcessContext;

namespace
{

int okZahl = 0;
int fehlerZahl = 0;

void pruefe (bool bedingung, const juce::String& text, const juce::String& zusatz = {})
{
    const auto zeile = text + (zusatz.isNotEmpty() ? ("  [" + zusatz + "]") : juce::String());
    if (bedingung) { ++okZahl;     std::cout << "  ok      " << zeile << std::endl; }
    else           { ++fehlerZahl; std::cout << "  FEHLER  " << zeile << std::endl; }
}

juce::File wurzel()
{
    auto d = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    while (d.exists() && ! d.getChildFile ("eq-copilot").isDirectory())
        d = d.getParentDirectory();
    return d;
}

// ── Staende bauen (bewusst mit den Vertrags-LITERALEN, nicht mit den
//    internen Identifiern von NakamaState.cpp: dieses Bein misst den Vertrag
//    von aussen, so wie ein fremdes Projekt es taete) ────────────────────────

juce::MemoryBlock alsBytes (const juce::ValueTree& baum)
{
    juce::MemoryBlock bytes;
    juce::MemoryOutputStream strom (bytes, false);
    baum.writeToStream (strom);
    strom.flush();
    return bytes;
}

/** Schema-1-Altstand, wie ihn ein FL-Projekt von 0.1 bis 0.3 traegt. */
juce::MemoryBlock schema1 (const juce::String& rolle)
{
    juce::ValueTree t ("EqCopilotState");
    t.setProperty ("schema", 1, nullptr);
    t.setProperty ("sensor_id", "11111111-2222-3333-4444-555555555555", nullptr);
    t.setProperty ("role", rolle, nullptr);
    t.setProperty ("label", "Klavier", nullptr);
    return alsBytes (t);
}

/** Schema-2-Stand der gewuenschten Klasse - ueber den echten Schreiber, damit
    Kind-Matrix und Reihenfolge stimmen (`speichere` haengt `MainProject` bzw.
    `Parameters` selbst an). */
juce::MemoryBlock schema2 (Klasse klasse)
{
    auto z = nakama::state::frisch ("99999999-8888-7777-6666-555555555555");
    z.common.klasse = klasse;
    z.common.position = nakama::state::Messposition::insert;
    z.hatParameters = (klasse == Klasse::active_probe);
    juce::MemoryBlock bytes;
    nakama::state::speichere (z, bytes);
    return bytes;
}

/** Ein Stand mit unbekanntem Major - der Leser gibt ihn read-only zurueck. */
juce::MemoryBlock fremdesMajor()
{
    juce::ValueTree t ("NakamaState");
    t.setProperty ("schema", 99, nullptr);
    return alsBytes (t);
}

/** Kein ValueTree - `lade()` meldet `ignoriert`. */
juce::MemoryBlock muell()
{
    const char rohdaten[] = "das ist kein ValueTree, sondern Text";
    return juce::MemoryBlock (rohdaten, sizeof (rohdaten));
}

/** Faehrt einen Stand durch den echten Leser und den Automaten. */
Klassifikation durchLaufen (Lebenslauf& automat, const juce::MemoryBlock& bytes,
                            const nakama::state::Bundle& bundle)
{
    nakama::state::Zustand z;
    const auto ergebnis = nakama::state::lade (bytes.getData(), bytes.getSize(), bundle, z);
    automat.stateRestauriert (ergebnis, z);
    return automat.klassifikation();
}

// ── Audio-Hilfen fuer Teil 2 ────────────────────────────────────────────────

juce::uint32 lcg = 0x2545f491u;
float zufall()
{
    lcg = lcg * 1664525u + 1013904223u;
    return ((float) (lcg >> 8) / (float) 0x00ffffff) * 2.0f - 1.0f;
}

/** Genau der Markierungsauftrag aus EqCopMarkierungTest T2 (Zonen-Solo
    120-300 Hz). Dass es DERSELBE ist, ist der Punkt: dort faerbt er, hier
    haengt es allein an der Klassifikation. */
MarkierungsAuftrag soloAuftrag (double fs)
{
    MarkierungsWunsch w;
    w.modus = MarkierungsModus::solo;
    w.istResonanz = false;
    w.fVon = 120.0; w.fBis = 300.0; w.fSchwerpunkt = 200.0;
    w.fs = fs;
    MarkierungsAuftrag a;
    baueMarkierungsAuftrag (a, w);
    return a;
}

/** Laufender Transport fuer die Audio-Haelfte.

    ⚠️ S10-11/SONDE-008: Seit dem User-Entscheid vom 22.08. (Hub `U10`: „Nein,
    nur mit Signal") verlangt die Hoer-Markierung ein GUELTIGES „spielt" - das
    fail-open `(spielt ∨ ¬hatTransport)` ist gefallen. Ohne Playhead faerbte
    dieser Test ab jetzt NIE, und die Haelfte „und faerben, sobald es so weit
    ist" waere still zu einer Tautologie geworden: sie haette gruen gemeldet,
    weil die Markierung generell stumm ist, nicht weil die Klassifikation
    greift. `testForciereEchtzeit` hilft dagegen nicht - der Schalter umgeht
    nur, was an der Wanduhr haengt. */
struct TestPlayHead : juce::AudioPlayHead
{
    bool spielt = true;
    juce::int64 pos = 0;
    juce::Optional<PositionInfo> getPosition() const override
    {
        PositionInfo p;
        p.setIsPlaying (spielt);
        p.setTimeInSamples (pos);
        return p;
    }
};

/** Faehrt `bloecke` Sinusbloecke und meldet, ob EIN Sample abwich.

    ⚠️ S20-22/SONDE-013 M-33 — WARUM HIER SEIT DIESEM TICKET EINE HOSTBRUECKE
    LAEUFT.

    §58 verlangt fuer den Hoermarker vier fail-closed-Terme: `playing=true`,
    `recording=false`, Realtime und Editor offen. Der Aufnahmezustand kommt
    AUSSCHLIESSLICH ueber die Hostbruecke — JUCEs oeffentlicher Playhead
    traegt ihn nicht, und B5 misst ausdruecklich, dass ueber ihn nur zwei
    Gueltigkeitsbits durchkommen. Ohne Bruecke ist er unbekannt, und ein
    unbekannter blockiert wie ein aktiver.

    Dieser Helfer hat den Test deshalb nach M-33 fallen lassen: „main faerbt
    jetzt" war gruen, solange kein Aufnahmeterm existierte. Er faehrt jetzt
    denselben Weg wie das Produkt — Bruecke pro Block, wie der gepatchte
    Wrapper. */
bool faerbtAudio (EqCopilotProcessor& p, double fs, int bs, int bloecke)
{
    juce::AudioBuffer<float> puffer (2, bs), kopie (2, bs);
    juce::MidiBuffer midi;
    TestPlayHead kopf;
    p.setPlayHead (&kopf);
    eqcop::hostbruecke::Bruecke bruecke;
    bruecke.verbinde (&p);
    double phase = 0.0;
    const double dPhase = 2.0 * juce::MathConstants<double>::pi * 200.0 / fs;
    bool gefaerbt = false;
    for (int block = 0; block < bloecke && ! gefaerbt; ++block)
    {
        for (int i = 0; i < bs; ++i)
        {
            const float v = 0.5f * (float) std::sin (phase);
            phase += dPhase;
            puffer.setSample (0, i, v);
            puffer.setSample (1, i, v);
        }
        kopie.makeCopyOf (puffer);
        {
            VstKontext c {};
            c.state = VstKontext::kContTimeValid;
            if (kopf.spielt)
                c.state |= VstKontext::kPlaying;
            // Kein kRecording: der Host nimmt nachweislich NICHT auf.
            c.projectTimeSamples = kopf.pos;
            c.continousTimeSamples = kopf.pos;
            c.sampleRate = fs;
            bruecke.beginneBlock ((std::uint32_t) bs);
            bruecke.kontextAus (c);
            bruecke.uebergib();
        }
        p.processBlock (puffer, midi);
        kopf.pos += bs;
        for (int k = 0; k < 2; ++k)
            if (std::memcmp (puffer.getReadPointer (k), kopie.getReadPointer (k),
                             (size_t) bs * sizeof (float)) != 0)
                gefaerbt = true;
    }
    bruecke.verbinde (nullptr);
    p.setPlayHead (nullptr);     // der Playhead lebt nur in diesem Aufruf
    return gefaerbt;
}

} // namespace

int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    constexpr double fs = 48000.0;
    constexpr int bs = 512;

    const auto eqcp = nakama::state::Bundle::eqcp();
    const auto nkpr = nakama::state::Bundle::nkpr();
    const auto nkac = nakama::state::Bundle::nkac();

    std::cout << "LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation" << std::endl;

    // ═══ TEIL 1 — der Automat pur ═══════════════════════════════════════════
    std::cout << "\n[1] Der Automat (nakama::state::Lebenslauf)" << std::endl;

    {
        Lebenslauf a;
        pruefe (a.klassifikation() == Klassifikation::unclassified,
                "frisch: unclassified", nakama::state::wort (a.klassifikation()));
        pruefe (a.istNeutral(), "frisch: neutral");
        pruefe (! a.audioAusnahmeErlaubt(), "frisch: keine Audio-Ausnahme (audio-neutral)");
        pruefe (! a.darfBrokerStarten(), "frisch: kein Brokerstart");
    }

    // §53.5: Schema-1 sensor|pre|post -> legacy, immer passiv.
    for (const char* rolle : { "sensor", "pre", "post" })
    {
        Lebenslauf a;
        const auto k = durchLaufen (a, schema1 (rolle), eqcp);
        pruefe (k == Klassifikation::legacy,
                juce::String ("Schema-1 '") + rolle + "' -> legacy",
                nakama::state::wort (k));
        pruefe (! a.audioAusnahmeErlaubt(),
                juce::String ("Schema-1 '") + rolle + "': bleibt passiv (keine Audio-Ausnahme)");
    }

    // §53.5: Schema-1 hub -> main.
    {
        Lebenslauf a;
        const auto k = durchLaufen (a, schema1 ("hub"), eqcp);
        pruefe (k == Klassifikation::main, "Schema-1 'hub' -> main", nakama::state::wort (k));
        pruefe (a.audioAusnahmeErlaubt(), "Schema-1 'hub': Audio-Ausnahme moeglich");
    }

    // §53.5: bestaetigter Schema-2-Main-State -> main. "Bestaetigt" ist der
    // Leser selbst: er hat die Kind-Matrix §2.1 geprueft und damit ein
    // MainProject-Kind gesehen, sonst waere der Stand read-only.
    {
        Lebenslauf a;
        pruefe (durchLaufen (a, schema2 (Klasse::main), eqcp) == Klassifikation::main,
                "Schema-2 main (Kind-Matrix bestaetigt) -> main");
    }
    {
        Lebenslauf a;
        pruefe (durchLaufen (a, schema2 (Klasse::legacy), eqcp) == Klassifikation::legacy,
                "Schema-2 legacy -> legacy");
    }

    // Read-only ist kein vollstaendiger Restore - und nimmt eine frueher
    // erteilte Klassifikation WIEDER WEG.
    {
        Lebenslauf a;
        pruefe (durchLaufen (a, schema2 (Klasse::main), eqcp) == Klassifikation::main,
                "Vorbereitung: erst main");
        pruefe (durchLaufen (a, fremdesMajor(), eqcp) == Klassifikation::unclassified,
                "unbekanntes Major danach -> zurueck auf unclassified");
    }

    // `ignoriert` ist KEIN Ereignis: der Automat bleibt stehen, wie auch der
    // gehaltene Zustand des Prozessors stehen bleibt.
    {
        Lebenslauf a;
        durchLaufen (a, schema2 (Klasse::main), eqcp);
        pruefe (durchLaufen (a, muell(), eqcp) == Klassifikation::main,
                "Muell (ignoriert) aendert nichts - main bleibt main");
    }
    {
        Lebenslauf a;
        pruefe (durchLaufen (a, muell(), eqcp) == Klassifikation::unclassified,
                "Muell auf einen frischen Automaten klassifiziert nicht");
    }

    // §53.5 dritter Punkt: Editor UND explizite Initialisierung - beides.
    {
        Lebenslauf a;
        auto frisch = nakama::state::frisch ("aaaa-bbbb");
        frisch.common.klasse = Klasse::main;

        pruefe (! a.expliziteInitialisierung (frisch),
                "explizite Initialisierung OHNE Editor: abgelehnt");
        pruefe (a.klassifikation() == Klassifikation::unclassified,
                "... und der Automat bleibt neutral");

        a.editorOffen (true);
        pruefe (a.klassifikation() == Klassifikation::unclassified,
                "der geoeffnete Editor allein klassifiziert NICHT");
        pruefe (a.expliziteInitialisierung (frisch),
                "Editor offen + explizite Initialisierung: angenommen");
        pruefe (a.klassifikation() == Klassifikation::main, "... -> main");
        pruefe (a.darfBrokerStarten(), "main + offener Editor darf den Broker starten");

        a.editorOffen (false);
        pruefe (a.klassifikation() == Klassifikation::main,
                "Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)");
        pruefe (! a.darfBrokerStarten(), "main OHNE offenen Editor darf NICHT starten");

        // Der Rueckweg (Gegenpfad): der User stellt die Rolle zurueck.
        a.editorOffen (true);
        auto zurueck = frisch;
        zurueck.common.klasse = Klasse::legacy;
        pruefe (a.expliziteInitialisierung (zurueck),
                "Rueckweg: derselbe Akt fuehrt von main zurueck");
        pruefe (a.klassifikation() == Klassifikation::legacy, "... -> legacy");
        pruefe (! a.darfBrokerStarten(), "legacy darf nie starten - auch mit Editor nicht");
        pruefe (! a.audioAusnahmeErlaubt(), "legacy ist wieder audio-neutral");
    }

    // §53.5 letzter Punkt: feste Produktklasse, aber neutral bis gueltigem State.
    {
        Lebenslauf suna (Klasse::passive_probe);
        pruefe (suna.klassifikation() == Klassifikation::unclassified,
                "Sonde (NkPr) frisch: neutral trotz fester Produktklasse");
        pruefe (durchLaufen (suna, schema2 (Klasse::passive_probe), nkpr)
                    == Klassifikation::passive_probe,
                "Sonde (NkPr): gueltiger eigener Stand -> passive_probe");
        pruefe (! suna.darfBrokerStarten(), "Sonde (NkPr): kein Brokerstart");
        suna.editorOffen (true);
        pruefe (! suna.darfBrokerStarten(),
                "Sonde (NkPr): auch mit offenem Editor kein Brokerstart");
        auto egal = nakama::state::frisch ("x");
        egal.common.klasse = Klasse::main;
        pruefe (! suna.expliziteInitialisierung (egal),
                "Sonde (NkPr): kein Akt macht sie zum Main");
    }
    {
        Lebenslauf probeeq (Klasse::active_probe);
        pruefe (durchLaufen (probeeq, schema2 (Klasse::active_probe), nkac)
                    == Klassifikation::active_probe,
                "Sonde (NkAc): gueltiger eigener Stand -> active_probe");
        // Ein Main-Stand kann eine Sonde gar nicht erreichen - der
        // Bundlevertrag §2.3 macht ihn read-only. Gemessen statt geglaubt.
        pruefe (durchLaufen (probeeq, schema2 (Klasse::main), nkac)
                    == Klassifikation::unclassified,
                "Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main");
    }

    // ═══ TEIL 2 — verdrahtet im echten Prozessor ════════════════════════════
    std::cout << "\n[2] Verdrahtet: EqCopilotProcessor (Gen)" << std::endl;

    // Die Produkt-v3-Adresse wird an einem eigenen Prozessor gemessen. Dieser
    // geladene Main-State darf den unmittelbar folgenden Scannerfall nicht
    // vorab klassifizieren.
    {
        EqCopilotProcessor p;
        auto z = nakama::state::frisch (
            "99999999-8888-7777-6666-555555555555");
        z.common.klasse = Klasse::main;
        z.common.projectBindingId = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
        juce::MemoryBlock bytes;
        nakama::state::speichere (z, bytes);
        p.setStateInformation (bytes.getData(), static_cast<int> (bytes.getSize()));
        const auto produktHello = p.v3HelloFuerTest();
        pruefe (produktHello.adresse.projectBindingId
                        == "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                    && nakama::ipc::instanceAdresseAusState (
                           produktHello.adresse.instanceId)
                        == nakama::ipc::instanceAdresseAusState (
                            "99999999-8888-7777-6666-555555555555")
                    && nakama::ipc::istHex32 (produktHello.adresse.sessionEpoch)
                    && nakama::ipc::istHex32 (produktHello.adresse.runtimeNonce)
                    && produktHello.pluginKind == "main"
                    && produktHello.hostAngeben && produktHello.hostPid != 0,
                "produkt_v3_hello_bindet_originalidentitaet_rolle_und_host");
        pruefe (p.v3ProduktstatusVerdrahtetFuerTest(),
                "Gen-ControlClient traegt den produktiven Heartbeat-/State-Report-Provider");
    }

    // P1 · Der Scannerlauf. Genau die Sequenz eines Plugin-Scanners:
    // instanziieren, Busse/Parameter abfragen, verarbeiten, zerstoeren.
    {
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        (void) p.getTotalNumInputChannels();
        (void) p.getParameters().size();
        (void) p.getName();

        pruefe (p.holeKlassifikation() == Klassifikation::unclassified,
                "Scannerlauf: klassifiziert NICHT (53.5)",
                nakama::state::wort (p.holeKlassifikation()));
        pruefe (! p.darfBrokerStarten(), "Scannerlauf: kein Brokerstart");

        // Und jetzt die Audio-Seite desselben Satzes ("audio-neutral"):
        // dieselbe Markierung wie in EqCopMarkierungTest T2, dieselbe
        // Echtzeit-Freistellung - und trotzdem kein einziges gefaerbtes Sample.
        p.testForciereEchtzeit (true);
        p.markierungEinreichen (soloAuftrag (fs));
        pruefe (! faerbtAudio (p, fs, bs, 80),
                "Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)");
        pruefe (! p.markierungHoerbar(), "Scannerlauf: Markierung meldet sich nicht hoerbar");
    }

    // P2 · Ein Altprojekt mit `sensor` -> legacy, und legacy bleibt passiv.
    {
        const auto bytes = schema1 ("sensor");
        EqCopilotProcessor p;
        p.setStateInformation (bytes.getData(), (int) bytes.getSize());
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        p.setzeEditorOffen (true);
        p.markierungEinreichen (soloAuftrag (fs));

        pruefe (p.holeKlassifikation() == Klassifikation::legacy,
                "Altprojekt 'sensor' -> legacy", nakama::state::wort (p.holeKlassifikation()));
        pruefe (! p.darfBrokerStarten(), "legacy mit offenem Editor: kein Brokerstart");
        pruefe (! faerbtAudio (p, fs, bs, 80),
                "legacy ist 'immer passiv' - kein Sample gefaerbt");
    }

    // P3 · Ein Altprojekt mit `hub` -> main. Ab hier faerbt dieselbe Markierung.
    {
        const auto bytes = schema1 ("hub");
        EqCopilotProcessor p;
        p.setStateInformation (bytes.getData(), (int) bytes.getSize());
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        p.markierungEinreichen (soloAuftrag (fs));

        pruefe (p.holeKlassifikation() == Klassifikation::main,
                "Altprojekt 'hub' -> main", nakama::state::wort (p.holeKlassifikation()));
        pruefe (p.v3HelloFuerTest().adresse.projectBindingId.empty(),
                "Schema-1-Migration erfindet keine Projektbindung");
        pruefe (! p.darfBrokerStarten(),
                "main, aber Editor zu: noch kein Brokerstart");
        p.setzeEditorOffen (true);
        pruefe (p.darfBrokerStarten(), "main + offener Editor: Brokerstart erlaubt");
        pruefe (p.setzeBindung ("hub", "Klavier", {})
                    && nakama::ipc::istHex32 (
                           p.v3HelloFuerTest().adresse.projectBindingId),
                "expliziter Main-Bindungsakt erzeugt die autoritative Projektbindung");
        const auto binding = p.v3HelloFuerTest().adresse.projectBindingId;
        juce::MemoryBlock gespeichert;
        p.getStateInformation (gespeichert);
        EqCopilotProcessor erneut;
        erneut.setStateInformation (gespeichert.getData(), (int) gespeichert.getSize());
        pruefe (erneut.v3HelloFuerTest().adresse.projectBindingId == binding,
                "Projektbindung reist im bestehenden Host-State verlustfrei");
        pruefe (faerbtAudio (p, fs, bs, 120),
                "main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)");
    }

    // P4 · Der Weg des Users an einer frischen Instanz: Editor auf, Rolle
    // "hub" - und wieder zurueck. Ohne diesen Gegenpfad waere die
    // Klassifikation eine Einbahnstrasse.
    {
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);

        // Die Reihenfolge ist die Regel, nicht Bequemlichkeit: erst der
        // Editor, dann der Akt. Umgekehrt lehnt der Automat ab - und genau
        // das misst die erste Zeile hier.
        pruefe (p.setzeBindung ("hub", {}, {}), "Vorbereitung: Rollenwahl aendert den Stand");
        pruefe (nakama::ipc::istHex32 (
                    p.v3HelloFuerTest().adresse.projectBindingId),
                "frischer expliziter Main-Akt stellt vor dem Connect eine Bindung bereit");
        pruefe (p.holeKlassifikation() == Klassifikation::unclassified,
                "Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)",
                nakama::state::wort (p.holeKlassifikation()));

        p.setzeEditorOffen (true);
        pruefe (p.holeKlassifikation() == Klassifikation::unclassified,
                "der geoeffnete Editor allein holt es auch nicht nach");
        pruefe (p.setzeBindung ("sensor", {}, {}) && p.setzeBindung ("hub", {}, {}),
                "Vorbereitung: der User waehlt die Rolle im offenen Editor");
        pruefe (p.holeKlassifikation() == Klassifikation::main,
                "frische Instanz: Editor offen + Rollenwahl 'hub' -> main");
        p.markierungEinreichen (soloAuftrag (fs));
        pruefe (faerbtAudio (p, fs, bs, 120), "frische Instanz als main: faerbt");

        p.setzeBindung ("sensor", {}, {});
        pruefe (p.holeKlassifikation() == Klassifikation::legacy,
                "Rueckweg: 'sensor' klassifiziert zurueck auf legacy");
        // ⚠️ S20-22/SONDE-013 M-34 hat diese Zusage GEAENDERT, und zwar
        // bewusst. Bis hierher hiess sie "der Schnitt auf neutral greift
        // sofort" — genau der Sofortschnitt aus NAK-47, der einen hoerbaren
        // Klick erzeugt (§49.3). Jetzt laeuft eine Rampe; neutral wird der
        // Pfad NACH ihr. Gemessen wird deshalb beides: dass die Rampe
        // endlich ist, und dass danach kein Sample mehr abweicht.
        p.markierungEinreichen (soloAuftrag (fs));
        (void) faerbtAudio (p, fs, bs, 40);          // die Rampe auslaufen lassen
        pruefe (! faerbtAudio (p, fs, bs, 80),
                "Rueckweg/M-34: nach dem Ausfade greift neutral - und bleibt es "
                "ueber 80 Bloecke");
    }

    // P5 · Ein Stand, den dieser Build nicht lesen darf, nimmt die
    // Klassifikation wieder weg - samt Audio-Ausnahme.
    {
        const auto main = schema1 ("hub");
        const auto fremd = fremdesMajor();
        EqCopilotProcessor p;
        p.setStateInformation (main.getData(), (int) main.getSize());
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        p.testForciereEchtzeit (true);
        p.setzeEditorOffen (true);
        pruefe (p.holeKlassifikation() == Klassifikation::main, "Vorbereitung: main");

        p.setStateInformation (fremd.getData(), (int) fremd.getSize());
        pruefe (p.stateNurLesen(), "der fremde Stand kommt read-only zurueck");
        pruefe (p.holeKlassifikation() == Klassifikation::unclassified,
                "read-only -> zurueck auf unclassified");
        pruefe (! p.darfBrokerStarten(), "read-only: kein Brokerstart");
        p.markierungEinreichen (soloAuftrag (fs));
        pruefe (! faerbtAudio (p, fs, bs, 80), "read-only: wieder audio-neutral");
    }

    // NAK-13 / Phase B: Die Startfreigabe bleibt exakt der bestehende
    // Lebenslauf-Haken; weder der Launcher noch der Audiothread besitzen eine
    // zweite Rollenentscheidung.
    std::cout << "\n[3] Phase B — Broker-Lifecycle-Gate und Audiothread-Nulltest"
              << std::endl;
    {
        bool alleNegativ = true;
        Lebenslauf scanner;
        alleNegativ = alleNegativ && ! scanner.darfBrokerStarten();

        Lebenslauf legacy;
        durchLaufen (legacy, schema1 ("sensor"), eqcp);
        legacy.editorOffen (true);
        alleNegativ = alleNegativ && ! legacy.darfBrokerStarten();

        Lebenslauf mainOhneEditor;
        durchLaufen (mainOhneEditor, schema1 ("hub"), eqcp);
        alleNegativ = alleNegativ && ! mainOhneEditor.darfBrokerStarten();

        Lebenslauf passiv;
        durchLaufen (passiv, schema2 (Klasse::passive_probe), nkpr);
        passiv.editorOffen (true);
        alleNegativ = alleNegativ && ! passiv.darfBrokerStarten();

        Lebenslauf aktiv;
        durchLaufen (aktiv, schema2 (Klasse::active_probe), nkac);
        aktiv.editorOffen (true);
        alleNegativ = alleNegativ && ! aktiv.darfBrokerStarten();

        Lebenslauf unbekannt;
        durchLaufen (unbekannt, fremdesMajor(), eqcp);
        unbekannt.editorOffen (true);
        alleNegativ = alleNegativ && ! unbekannt.darfBrokerStarten();
        pruefe (alleNegativ, "brokerstart_gate_alle_negativzustaende");
    }

    {
        std::atomic<int> pruefungen { 0 }, spawns { 0 };
        nakama::ipc::BrokerLifecycleHooks hooks;
        hooks.verbunden = [] { return false; };
        hooks.connectFehlgeschlagen = [] { return true; };
        hooks.darfStarten = [] { return false; };
        hooks.pruefen = [&] { ++pruefungen; return nakama::ipc::BrokerPruefBericht {}; };
        hooks.spawn = [&] { ++spawns; return true; };
        hooks.mutexName = L"Local\\Nakama.PhaseB.LebenslaufTest";
        nakama::ipc::BrokerLifecycle lifecycle (std::move (hooks));
        lifecycle.tickFuerTest (0);
        pruefe (pruefungen.load() == 0 && spawns.load() == 0,
                "launcher_bleibt_vor_gate_unberuehrt");
    }

    {
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        juce::AudioBuffer<float> puffer (2, bs);
        juce::MidiBuffer midi;
        puffer.clear();

        nakama::ipc::brokerLifecycleAudioTestZaehlerLoeschen();
        nakama::ipc::brokerLifecycleAudioTestBeginn();
        for (int i = 0; i < 128; ++i)
            p.processBlock (puffer, midi);
        nakama::ipc::brokerLifecycleAudioTestEnde();
        const auto audioOperationen =
            nakama::ipc::brokerLifecycleOperationenImAudiothread();
        pruefe (audioOperationen
                    == nakama::ipc::AUDIO_THREAD_BROKER_OPERATIONEN_MAX,
                "broker_lifecycle_aufrufe_im_audiothread_null");
        pruefe (audioOperationen == 0,
                "brokerstop_nie_im_processblock");

        nakama::ipc::brokerLifecycleAudioTestZaehlerLoeschen();
        nakama::ipc::brokerLifecycleAudioTestBeginn();
        (void) p.brokerLifecycleSnapshot();
        nakama::ipc::brokerLifecycleAudioTestEnde();
        pruefe (nakama::ipc::brokerLifecycleOperationenImAudiothread() == 1,
                "broker_lifecycle_aufrufe_im_audiothread_null Gegenprobe sieht echte Sperre");
        nakama::ipc::brokerLifecycleAudioTestZaehlerLoeschen();

        const auto repo = wurzel();
        const auto processorText = repo.getChildFile (
            "eq-copilot/plugin/src/PluginProcessor.cpp").loadFileAsString().toStdString();
        const auto anfang = processorText.find ("void EqCopilotProcessor::processBlock");
        const auto ende = processorText.find (
            "void EqCopilotProcessor::nakamaBlockEmpfangen", anfang);
        const bool blockGefunden = anfang != std::string::npos
                                && ende != std::string::npos && ende > anfang;
        const auto block = blockGefunden
            ? processorText.substr (anfang, ende - anfang) : std::string {};
        auto freiVon = [&] (std::initializer_list<const char*> nadeln) {
            if (! blockGefunden)
                return false;
            for (const auto* nadel : nadeln)
                if (block.find (nadel) != std::string::npos)
                    return false;
            return true;
        };
        const bool dateiFrei = freiVon ({ "juce::File", "FileInputStream",
                                          "FileOutputStream", "CreateFileW",
                                          "ReadFile", "WriteFile", "std::fstream",
                                          "QueryFullProcessImageNameW",
                                          "brokerBinaryPruefen" });
        const bool pipeFrei = freiVon ({ "ControlClient", "TelemetryClient",
                                         "IpcVerbindung", "NamedPipe",
                                         "CreateNamedPipeW",
                                         "namedPipeErreichbar",
                                         "namedPipeServerAuthentisieren",
                                         "GetNamedPipeServerProcessId" });
        const bool prozessFrei = freiVon ({ "BrokerLifecycle", "CreateProcessW",
                                            "brokerVerborgenStarten", "ShellExecute",
                                            "OpenProcess", "OpenProcessToken" });
        const bool logFrei = freiVon ({ "juce::Logger", "std::cout", "printf (",
                                        "fprintf (", "OutputDebugString" });
        const auto storeText = repo.getChildFile ("broker/src/store/mod.rs").loadFileAsString();
        const bool storeNull = storeText.contains (
            "pub const AUDIO_THREAD_STORE_WAIT_MS_MAX: u64 = 0;")
                            && freiVon ({ "StoreWriter", "StoreHandle", "store.append",
                                         "condition_variable", "wait_for" });
        pruefe (audioOperationen == 0 && dateiFrei,
                "processblock_quellschnitt_dateioperationen_null");
        pruefe (audioOperationen == 0 && pipeFrei,
                "processblock_quellschnitt_pipeoperationen_null");
        pruefe (audioOperationen == 0 && prozessFrei,
                "processblock_quellschnitt_prozessoperationen_null");
        pruefe (audioOperationen == 0 && logFrei,
                "processblock_quellschnitt_logoperationen_null");
        pruefe (audioOperationen == 0 && storeNull,
                "processblock_quellschnitt_store_wait_null");
    }

    std::cout << std::endl;
    if (fehlerZahl == 0)
    // ── NAK-180 R6/N-20: `stop()` kehrt in gemessener Frist zurueck, auch
    //    wenn ein Tick gerade in `namedPipeErreichbar` steht ────────────────
    //
    // Der Befund aus G4 §7 (Part 05): der Lebenslaufthread haelt das
    // Win32-Startmutex, `stop()` joint ihn fristlos, und `stop()` ist die
    // ERSTE Anweisung des Prozessordestruktors. Stand der Tick in einem
    // `WaitNamedPipeW` mit der Frist eines FREMDEN Pipe-Besitzers, hing der
    // Message-Thread des Hosts fuer die vom Angreifer gewaehlte Dauer.
    //
    // Gemessen wird die Frist von `stop()` selbst, gegen eine Pipe mit
    // `nDefaultTimeOut = 0xFFFFFFFE` ohne `ConnectNamedPipe` - im
    // PROBE-Namensraum, nie auf dem produktiven Namen.
    {
        const auto pipe = std::string (nakama::ipc::kPipePraefixProbe)
                        + "test.nak180-stopfrist."
                        + std::to_string ((int) GetCurrentProcessId());
        std::wstring breit;
        for (char c : pipe)
            breit.push_back (static_cast<wchar_t> (static_cast<unsigned char> (c)));
        HANDLE feindlich = CreateNamedPipeW (
            breit.c_str(), PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            1, 4096, 4096, 0xFFFFFFFE, nullptr);
        pruefe (feindlich != INVALID_HANDLE_VALUE,
                "NAK-180 N-20: die feindliche Pipe steht");

        if (feindlich != INVALID_HANDLE_VALUE)
        {
            std::atomic<int> ticks { 0 };
            nakama::ipc::BrokerLifecycleHooks hooks;
            hooks.verbunden = [] { return false; };
            hooks.connectFehlgeschlagen = [] { return true; };
            hooks.darfStarten = [] { return true; };
            hooks.pruefen = [&] { ++ticks; return nakama::ipc::BrokerPruefBericht {}; };
            hooks.spawn = [] { return false; };
            hooks.mutexName = L"Local\Nakama.NAK180.Stopfrist";
            hooks.pipeName = pipe;          // hier steht der Tick im Wait
            nakama::ipc::BrokerLifecycle lifecycle (std::move (hooks));
            lifecycle.start();

            // Dem Thread Zeit geben, in den Tick und damit in den Wait zu
            // laufen; der Takt ist 25 ms.
            const auto los = std::chrono::steady_clock::now();
            while (std::chrono::steady_clock::now() - los < std::chrono::milliseconds (120))
                std::this_thread::sleep_for (std::chrono::milliseconds (5));

            const auto vorStop = std::chrono::steady_clock::now();
            lifecycle.stop();
            const auto dauerMs = (long long) std::chrono::duration_cast<std::chrono::milliseconds> (
                std::chrono::steady_clock::now() - vorStop).count();

            pruefe (dauerMs < 1000,
                    "NAK-180 N-20: stop_kehrt_in_gemessener_frist_zurueck - auch wenn ein "
                    "Tick gerade in `namedPipeErreichbar` steht, kehrt `stop()` unter 1 s "
                    "zurueck; mit der Serverfrist stuende er bis zu 49,7 Tage",
                    std::to_string (dauerMs) + " ms");
            CloseHandle (feindlich);
        }
    }

    {
        std::cout << "LEBENSLAUF-TEST OK - " << okZahl << " Pruefungen ok, 0 Fehler" << std::endl;
        return 0;
    }
    std::cout << "LEBENSLAUF-TEST FEHLGESCHLAGEN - " << okZahl << " ok, "
              << fehlerZahl << " Fehler" << std::endl;
    return 1;
}
