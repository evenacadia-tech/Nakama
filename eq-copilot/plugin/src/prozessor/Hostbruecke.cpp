// EqCopilotProcessor — Hostbruecke: was der Host AUSSERHALB des Audio-Blocks ruft.
//
// NAK-225/S25d (09.09.2026), herausgeloest aus src/PluginProcessor.cpp. Inhalt:
//
//   prepareToPlay            Samplerate, Blockgroesse und Kanaele uebernehmen,
//                            Engines und Puffer dafuer vorbereiten. Laeuft VOR
//                            dem ersten Block und darf deshalb allokieren.
//   isBusesLayoutSupported   Welche Buslayouts die Instanz annimmt.
//   setzeEditorOffen         Der Host oeffnet oder schliesst das Fenster.
//   meldeHostDirty           Die einzige Stelle, an der die Instanz dem Host
//                            sagt, dass ihr Zustand sich geaendert hat.
//   createEditor             Das Fenster selbst.
//
// NICHT hier, sondern in der Wurzel src/PluginProcessor.cpp: processBlock,
// nakamaBlockEmpfangen und lebenszeichen. Sie laufen auf dem Audio-Thread,
// und dort gilt: keine Sperren, keine Allokationen, keine Datei-, Pipe- oder
// Netzzugriffe, kein Logging; bei Ueberlast werden Analyseframes verworfen,
// nie Audio (CLAUDE.md, Grundgesetz). Die Trennlinie dieser Datei ist genau
// diese Regel: was der Host ausserhalb des Blocks ruft, darf, was der Block
// ruft, darf nicht.
//
// getStateInformation und setStateInformation ruft der Host ebenfalls, sie
// stehen aber in prozessor/State.cpp — sie sind der State-Vertrag, nicht die
// JUCE-Lebenszyklusflaeche.

#include "../PluginProcessor.h"
#include "../PluginEditor.h"
#include <algorithm>
#include <cmath>

namespace eqcop
{

void EqCopilotProcessor::prepareToPlay (double samplerate, int maxBlock)
{
    const double sichereSamplerate = std::isfinite (samplerate)
                                  && samplerate > 0.0 && samplerate <= 768000.0
        ? samplerate : 0.0;
    // NAK-180 R5: NUR eine geprueft gueltige Rate wird gemerkt. Eine
    // nicht-endliche Hostrate laesst die letzte gute stehen, statt den
    // Quarantaene-Tail auf ein Sample zu kuerzen.
    if (sichereSamplerate > 0.0)
        letzteGueltigeSamplerate.store (sichereSamplerate, std::memory_order_relaxed);
    // Jeder Prepare-Aufruf ist eine Queue-Generation. Der Audiothread setzt
    // sein Projektfenster exakt dann zurueck, wenn `veroeffentliche()` diese
    // Generation wirklich uebernimmt - auch bei unveraenderter Samplerate.
    {
        auto l = externerAnalyseSteuerZug();
        samplerateAtomic.store (sichereSamplerate);
        // Samplerate und Generation werden unter EINER Steuerkante sichtbar.
        // Der Worker kann daher nie alte Bloecke mit der neuen Binzuordnung
        // auswerten.
        queue.neustartAnfordern();
    }
    blockSizeAtomic.store (maxBlock);
    kanaeleAtomic.store (getTotalNumInputChannels());
    // SONDE-008: KEIN Reset von hier aus. Bis 23.08. rief diese Zeile
    // `fifo.reset()` — der Nachrichtenthread verstellte damit beide Enden eines
    // SPSC-Rings mitten in einen laufenden Leser hinein. Stattdessen ein
    // Wunsch, den der Audiothread als Einziger einlöst; der Worker erkennt die
    // Reste des alten Anlaufs an ihrer kleineren `startFolge`.
    // Hör-Markierung: Puffer/Zustände neu, Echtzeit-Beweis verfällt — nach
    // jedem prepareToPlay (auch Render-Vorlauf) gilt wieder „neutral, bis
    // Echtzeit bewiesen" (Konzept v2 §4).
    // 🔑 NAK-180 Nacharbeit 1 (EP-08/N-10): der faellige Uebergang wird VOR
    // dem Reset erfasst, mit der GEZAEHLTEN Hoerdauer.
    //
    // Beide Vorbereiter loeschten `warHoerbar` und `hoerbareSamples`, und der
    // Prozessor versuchte danach, das `end` aus seinem Sendezustand zu
    // rekonstruieren. Lag das Begin noch im RT-Ring, war `offenesBegin`
    // ungueltig und es entstand gar kein `end`; war es entnommen, trug die
    // Kopie des BEGINS `dauerSamples == 0`, und der Nachlauf verlor die
    // gezaehlte Dauer. Beides bricht N-10.
    const auto uebergangA = markierung.setzeSamplerate (sichereSamplerate);
    const auto uebergangB = markierung.vorbereiten (maxBlock);
    const bool markerAbgebrochen = uebergangA.endete || uebergangB.endete;
    const std::uint64_t abgebrocheneDauer =
        std::max (uebergangA.dauerSamples, uebergangB.dauerSamples);
    // Befund R06: die Trockenkopie des Vergleichspegels wird HIER allokiert —
    // im Audiothread nie. Zwei Kanaele reichen dem Vertrag dieses Plugins.
    versuchTrocken.assign ((std::size_t) std::max (1, maxBlock) * 2u, 0.0f);
    vergleichspegel.vorbereiten (sichereSamplerate);
    echtzeitOk.store (false);
    lzBestanden = 0;
    lzLetzterNs = 0;
    lzBucketStartNs = 0;
    lzBucketSamples = 0;
    // 🔑 NAK-180 N-10: der Marker ist hart aus, sein `end` kommt NIE.
    //
    // `markierung.vorbereiten()` und `setzeSamplerate()` setzen `warHoerbar`
    // zurueck und loeschen den Fade, ohne den faelligen Uebergang zu melden —
    // der Audiothread erzeugt fuer dieses Intervall also kein `endete` mehr.
    // Ohne diese Markierung bliebe das Begin beim Broker fuer immer offen und
    // die Sitzung dauerhaft gesperrt. Der Sender bildet das `end` stattdessen
    // selbst, mit `project_sample_end: null` (die Endprojektzeit ist hier
    // ehrlich unbekannt) und dem Tail der letzten gueltigen Rate.
    if (markerAbgebrochen)
    {
        std::lock_guard<std::mutex> l (sendeZustandMutex);
        // Ein LEBENDES offenes Begin nimmt das Ende direkt; sonst wartet der
        // Uebergang auf das Begin, das noch im Ring liegt. Steht schon ein
        // Wartender, gaebe es zwei zu schliessende Intervalle und nur einen
        // Platz — dann sagt der Ueberlauf die Wahrheit (fail-closed, §34.2).
        const bool anLebendes = offenesBegin.gueltig && ! offenesBegin.tot;
        const bool alsWartender = ! anLebendes && ! ausstehenderTotUebergang.gueltig;
        if (! anLebendes && ! alsWartender)
        {
            interventionsRingUeberlauf.store (true, std::memory_order_relaxed);
        }
        // 🔑 Die Sequenz wird NUR gezogen, wenn sie auch reist. Eine
        // verbrauchte, nie gesendete Nummer waere beim Broker eine Luecke -
        // genau das Signal, das ein verlorenes Ereignis meldet.
        const auto sequenz = (anLebendes || alsWartender)
            ? interventionsSequenz.fetch_add (1, std::memory_order_relaxed) + 1
            : 0;
        const auto tail = nakama::ipc::tailSamplesFuer (
            abgebrocheneDauer,
            letzteGueltigeSamplerate.load (std::memory_order_relaxed));
        if (anLebendes)
        {
            offenesBegin.tot = true;
            auto ende = offenesBegin.ereignis;
            ende.beginn = false;
            ende.projektzeitGesetzt = false;   // die Endzeit ist ehrlich unbekannt
            ende.sequenz = sequenz;
            // 🔑 EP-08: die Dauer kommt aus dem UEBERGANG, nicht aus der Kopie
            // des Begins - dort steht sie nie.
            ende.dauerSamples = abgebrocheneDauer;
            ende.tailSamples = tail;
            offenesBegin.totesEnde = ende;
        }
        else if (alsWartender)
        {
            // 🔑 EP-08, zweiter Fall: das Begin liegt noch im RT-Ring. Der
            // Prozessor darf ihn hier nicht lesen — er hat genau EINEN
            // Konsumenten, den Worker (§6.6). Der Uebergang wartet deshalb
            // mit seiner Sequenz, bis der Sender das Begin entnommen hat, und
            // wird dann genau davor eingereiht.
            ausstehenderTotUebergang = TotUebergang { true, sequenz,
                                                      abgebrocheneDauer, tail };
        }
    }

    // Der v3-Hello-Provider liest Samplerate/Block/Kanaele erst beim Aufbau.
    // Prepare laeuft auf dem Host-/Nachrichtenthread, nie im Audiocallback.
    controlV3.reconnect();
}

bool EqCopilotProcessor::isBusesLayoutSupported (const BusesLayout& layout) const
{
    // V1: Mono und Stereo, Eingang == Ausgang. Anderes wird nicht still
    // heruntergemischt (Plan §9.4) — der Host bekommt ein klares Nein.
    const auto ein = layout.getMainInputChannelSet();
    const auto aus = layout.getMainOutputChannelSet();
    if (ein != aus)
        return false;
    return ein == juce::AudioChannelSet::mono() || ein == juce::AudioChannelSet::stereo();
}

void EqCopilotProcessor::meldeHostDirty()
{
    updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged (true));
}

// ── State: Schema 2 `NakamaState` (SONDE-006) ─────────────────────────────
// Vertrag: eq-copilot/schemas/state/nakama-state-v2.md. Schema 1 wird rein
// und deterministisch migriert; ein Stand, den dieser Build nicht
// interpretieren darf, wird read-only gehalten und bytegleich zurueckgegeben.

void EqCopilotProcessor::setzeEditorOffen (bool offen)
{
    // Zwei Verbraucher, ein Ereignis: der Audiothread-Term der Markierungs-
    // Verriegelung und die Editor-Haelfte der Brokerstart-Bedingung.
    editorOffen.store (offen);
    std::lock_guard<std::mutex> l (bindungMutex);
    lebenslauf.editorOffen (offen);
    // Kein spiegleKlassifikation(): der Editor allein klassifiziert nichts
    // (§53.5 verlangt Editor UND explizite Initialisierung). Die
    // Markierungs-Verriegelung traegt `editorOffen` ohnehin als eigenen Term.
}

juce::AudioProcessorEditor* EqCopilotProcessor::createEditor()
{
    return new EqCopilotEditor (*this);
}

} // namespace eqcop
