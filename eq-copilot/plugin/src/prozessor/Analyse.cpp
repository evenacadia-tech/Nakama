// EqCopilotProcessor — Analyse-Anstoss: Worker, Snapshots, Assistent, Versuch.
//
// NAK-225/S25d (09.09.2026), herausgeloest aus src/PluginProcessor.cpp. Inhalt:
//
//   workerLauf           Der Analyse-Thread. Er nimmt die Bloecke, die der
//                        Audio-Thread in die Queue gelegt hat, fuettert die
//                        FeatureEngine und veroeffentlicht Snapshots.
//   schreibeSnapshotDatei
//                        Der Evidenz-Snapshot als Datei.
//   bindePassagenfenster, bindePassagenfensterMitEpoche,
//   passagenfensterWunschFuerTest, passagenfensterFuehrt,
//   passagenfensterInEngine, loesePassagenfenster,
//   versuchAufgenommeneBloecke, engineabzugLesen
//                        Das Passagenfenster: welcher Abschnitt gemessen wird.
//   beginneVersuch, erfasseKandidat, urteileVersuch, brichVersuchAb,
//   laufenderVersuch, versuchReferenzJson, versuchKopfJson, sendeVersuchP0,
//   versuchLautheitAbgeglichen, versuchMatchGainDb,
//   versuchLautheitAbgeglichenLebendFuerTest, vergleichspegelZaehlerstand,
//   versuchNichtEndlicheSamples, vergleichszustandLeeren
//                        Der Blindvergleich samt Lautheitsabgleich.
//   setzeAssistentSchritt, assistentSchrittKopie, assistentAenderungMelden,
//   assistentStarten, assistentWeiter, assistentZurueck,
//   assistentUeberspringen, assistentAbbrechen, assistentAntwort,
//   assistentAusState, assistentFortsetzen, assistentVersuchStarten
//                        Der regelbasierte Assistent. Er schlaegt vor; er
//                        greift nie selbst ein (CLAUDE.md, Grundgesetz).
//
// Alles hier laeuft auf dem Worker- oder Nachrichtenthread, nie auf dem
// Audio-Thread. Der Audio-Thread liefert nur Bloecke in die StampedAudioQueue;
// bei Ueberlast werden Analyseframes verworfen, nie Audio.
//
// NaN-Ehrlichkeit gilt auch hier: nicht-endliche Werte werden verriegelt und
// gezaehlt, nie stillschweigend ersetzt. Der Helfer `zahl` unten schreibt
// deshalb "0" statt "nan" — und der Zaehler, der den Fall meldet, bleibt.

#include "../PluginProcessor.h"
#include "../Diagnose.h"
#include "../WorkerCadence.h"
#include "Intern.h"
#include <chrono>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace eqcop
{

using prozessor_intern::uuidHex32;

namespace
{
/** Ein Fingerprint als JSON-Objekt des v3-Vertrags. */
std::string fingerprintJson (const nakama::analyse::Fingerprint& f)
{
    auto liste = [] (const std::uint8_t* werte, int n)
    {
        std::string s = "[";
        for (int i = 0; i < n; ++i)
        {
            if (i > 0) s += ",";
            s += std::to_string ((int) werte[(std::size_t) i]);
        }
        return s + "]";
    };
    std::string s = "{\"version\":";
    s += std::to_string (f.version);
    s += ",\"band_energie\":" + liste (f.bandEnergie, nakama::analyse::Fingerprint::kBaender);
    s += ",\"chroma\":"       + liste (f.chroma,      nakama::analyse::Fingerprint::kChroma);
    s += ",\"onset\":"        + liste (f.onset,       nakama::analyse::Fingerprint::kOnsets);
    return s + "}";
}

/** Eine Zahl in der Form, die der Textriegel und beide Leser annehmen. */
std::string zahl (double x)
{
    if (! std::isfinite (x))
        return "0";
    std::ostringstream aus;
    aus.imbue (std::locale::classic());
    aus << std::setprecision (10) << x;
    return aus.str();
}

std::string jsonText (const juce::String& s)
{
    return juce::JSON::toString (juce::var (s), true).toStdString();
}
} // namespace

void EqCopilotProcessor::workerLauf()
{
    // FP-Modi sind threadlokal: der ScopedNoDenormals im Audiocallback
    // schuetzt diesen Worker und seine rekursiven K-Filter nicht.
    juce::ScopedNoDenormals keineDenormals;
    // Leert die Analysequeue in begrenzten Zuegen; ohne Rueckstau schlaeft der
    // Worker bis zu 50 ms, bei Rueckstau arbeitet er direkt weiter. Leichte und
    // schwere Publikation bleiben trotzdem an 50-/250-ms-Wanduhrdeadlines.
    // Die Engine gehoert exklusiv diesem Thread; Reset/Samplerate kommen als
    // Atomics herein. Kein Realtime-Anspruch — Überlast verwirft ganze Bloecke.
    quarantaene.vorbereiten();     // einmalige Allokation, im Worker, vor dem ersten Zug
    juce::uint64 unverarbeitet = 0;   // Samples seit der letzten Schwer-Auswertung
    auto workerAnlauf = queue.aktuellerAnlauf();
    detail::WorkerKadenz kadenz;
    while (workerLaeuft.load())
    {
        // Explizite Uebergabe statt Fairness-Hoffnung: sobald Prepare, Reset
        // oder ein Frame-Leser wartet, konkurriert der Worker nicht um den
        // naechsten Zug. Der laufende Zug bleibt durch acht Bloecke begrenzt.
        if (analyseSteuerWartende.load() != 0)
        {
            std::this_thread::yield();
            continue;
        }

        bool queueHatRest = false;
        {
            std::unique_lock<std::mutex> steuerung (analyseSteuerMutex);
            // Schliesst die Luecke zwischen Vorpruefung und Lock-Erwerb: hat
            // sich dort jemand angemeldet, geben wir sofort wieder frei.
            if (analyseSteuerWartende.load() != 0)
            {
                steuerung.unlock();
                std::this_thread::yield();
                continue;
            }

            const double srWunsch = samplerateAtomic.load();
            const bool analyseRateGueltig = srWunsch > 0.0;
            if (analyseRateGueltig)
            {
                engine.vorbereiten (srWunsch);          // no-op bei gleicher Rate
            }
            // Auch die ungueltige Rate muss die FeatureEngine sehen: sie
            // deaktiviert damit eine eventuell alte, gueltige Binzuordnung.
            merkmale.vorbereiten (srWunsch);

            const auto aktuellerAnlauf = queue.aktuellerAnlauf();
            if (aktuellerAnlauf != workerAnlauf)
            {
                // Auch ein same-rate-prepare ist eine Messgrenze. M1 besitzt
                // keinen Deskriptor und muss sie hier explizit bekommen.
                engine.zuruecksetzen();
                quarantaene.zuruecksetzen();
                unverarbeitet = 0;
                workerAnlauf = aktuellerAnlauf;
                kadenz.zuruecksetzen (detail::WorkerKadenz::Uhr::now());
            }

            if (messResetWunsch.exchange (false))
            {
                engine.zuruecksetzen();
                merkmale.zuruecksetzen();
                // Gegenpfad: was in Quarantäne liegt, gehört zur alten Messung.
                quarantaene.zuruecksetzen();
                unverarbeitet = 0;
                kadenz.zuruecksetzen (detail::WorkerKadenz::Uhr::now());
            }

            // 🔑 SONDE-013 Nacharbeit 2 (Befund R03, M-03/M-25): DER
            // Produktaufrufer von `setzePassagenfenster`.
            //
            // `merkeManuellePassage` schrieb bis hierher nur Plugin-State; die
            // Engine erfuhr von der markierten Passage NIE, und ihre
            // Passagenmetriken liefen weiter seit der letzten Transportgrenze.
            // Die Uebergabe laeuft ueber denselben Wunsch-Weg wie
            // `messResetWunsch` — auf DIESEM Thread, unter DIESER Sperre, nie
            // aus dem Nachrichten- oder Audiothread.
            //
            // 🔑 Nacharbeit 3 (Befund C2, Paragraph 32.4): die Epoche kommt aus
            // dem WUNSCH, nicht aus der Engine.
            //
            // Die Runde 2 las sie hier mit `merkmale.transportEpocheJetzt()` —
            // also aus genau der Quelle, gegen die `setzePassagenfenster` sie
            // vergleicht. Der Riegel war damit tautologisch erfuellt und
            // konnte nie greifen. Ein Seek zwischen Markierung und diesem Lauf
            // liess die alten Grenzen unter der NEUEN Epoche durch, und die
            // Passagenmetriken beschrieben danach eine andere Stelle der Musik
            // als die markierte. Die Epoche muss die des MARKIERENS sein.
            const auto fensterGeneration = passagenfensterGeneration.load();
            if (passagenfensterLoeschen.exchange (false))
                merkmale.loeschePassagenfenster();
            if (passagenfensterWunsch.exchange (false))
            {
                // Lehnt die Engine ab — der Epochenvergleich ist der einzige
                // Grund —, bleibt KEIN Fenster stehen. Das alte gehoerte einer
                // Passage, die seit der neuen Bindung niemand mehr fuehrt;
                // es weiterlaufen zu lassen waere die stille Verwechslung, die
                // Befund C3 beschreibt.
                if (! merkmale.setzePassagenfenster (passagenfensterStart.load(),
                                                     passagenfensterEnde.load(),
                                                     passagenfensterEpocheWunsch.load()))
                    merkmale.loeschePassagenfenster();
            }
            // Befund C4: was der Audiothread ueber das Fenster wissen muss.
            // Grenzen zuerst, Publikationsbit danach — ein Leser, der das Bit
            // schon sieht, sieht dann nie alte Grenzen. Umgekehrt beim
            // Loeschen: erst das Bit weg, dann duerfen die Grenzen veralten.
            //
            // Die Generation entscheidet, ob diese Aussage ueberhaupt noch
            // gilt: hat der Nachrichtenthread waehrenddessen neu gebunden,
            // schweigt der Worker und sagt es im naechsten Zug.
            if (passagenfensterGeneration.load() == fensterGeneration
                && merkmale.passagenfensterIntakt())
            {
                pegelFensterStart.store (merkmale.passagenfensterStart(),
                                         std::memory_order_relaxed);
                pegelFensterEnde.store (merkmale.passagenfensterEnde(),
                                        std::memory_order_relaxed);
                pegelFensterAktiv.store (true, std::memory_order_release);
            }
            else
            {
                pegelFensterAktiv.store (false, std::memory_order_release);
            }

            // SONDE-008: Block für Block durch die Ein-Block-Quarantäne.
            // Die Steuer-Sperre bleibt bewusst auf einen kleinen Zug begrenzt.
            // Ein dauerhaft voller Producer darf Prepare/Reset/Snapshot nicht
            // hinter einem unendlichen Drain verhungern lassen.
            constexpr int kMaxBloeckeJeSteuerzug = 8;
            int bloeckeInDiesemZug = 0;
            while (bloeckeInDiesemZug < kMaxBloeckeJeSteuerzug)
            {
                // Kommt waehrend des Zugs ein Steuerer hinzu, endet der Zug
                // nach hoechstens dem gerade laufenden Block statt erst bei 8.
                if (analyseSteuerWartende.load() != 0)
                    break;
                const auto* roh = queue.spitze();
                if (roh == nullptr)
                    break;
                ++bloeckeInDiesemZug;
                if (roh->startFolge != queue.aktuellerAnlauf())
                {
                    veralteteBloecke.fetch_add (1);
                    quarantaene.zuruecksetzen();
                    queue.freigeben();
                    continue;
                }

                const auto bruecheVorher = quarantaene.kontinuitaetsbrueche();
                const auto frei = quarantaene.schiebe (queue, *roh);
                queue.freigeben();
                if (quarantaene.kontinuitaetsbrueche() != bruecheVorher)
                {
                    // FeatureEngine erkennt die Grenze am naechsten freigegebenen
                    // Deskriptor selbst. M1 sieht nur Samples und braucht den
                    // expliziten Gegenpfad fuer FFT-, K- und Loudness-Zustaende.
                    engine.zuruecksetzen();
                    unverarbeitet = 0;
                    kadenz.zuruecksetzen (detail::WorkerKadenz::Uhr::now());
                }
                if (frei)
                {
                    // Fail-closed: ein Hostblock ohne gueltige aktuelle Rate
                    // darf weder die alte FeatureEngine noch M1 fuettern. Die
                    // Queue/Quarantaene werden weiterhin begrenzt geleert;
                    // beim naechsten Prepare trennt die Generation den Rest.
                    if (! analyseRateGueltig)
                        continue;

                    const auto grenzenVorher = merkmale.getrennteFenster();
                    const auto straddlesVorher = merkmale.straddleVerworfen();
                    if (merkmale.nimmBlock (*frei.block, frei.audio))
                        merkmalFrames.fetch_add (1);

                    const bool featureGrenze = merkmale.getrennteFenster() != grenzenVorher;
                    const bool blockVerworfen = merkmale.straddleVerworfen() != straddlesVorher;
                    if (featureGrenze)
                    {
                        engine.zuruecksetzen();
                        unverarbeitet = 0;
                        kadenz.zuruecksetzen (detail::WorkerKadenz::Uhr::now());
                    }
                    if (! blockVerworfen)
                    {
                        engine.verarbeite (frei.audio, (int) frei.block->sampleCount,
                                           (int) frei.block->kanaele);
                        samplesAnalysiert.fetch_add ((juce::uint64) frei.block->sampleCount);
                        unverarbeitet += (juce::uint64) frei.block->sampleCount;
                    }
                }
            }
            queueHatRest = queue.spitze() != nullptr;

            // Eine volle Queue fuehrt sofort zum naechsten begrenzten Zug.
            // Daher ist nur die monotone Deadline ein Zeitbeweis; eine Anzahl
            // abgearbeiteter Zuege wuerde Gating/Kandidaten unter Rueckstau
            // bis auf CPU-Geschwindigkeit beschleunigen.
            // Eine bereits sichtbare Steueranfrage geht auch vor einer jetzt
            // faelligen Auswertung. Die Deadline wird dann nicht verbraucht,
            // sondern nach der Uebergabe im naechsten Workerzug bedient.
            const auto faellig = analyseRateGueltig && analyseSteuerWartende.load() == 0
                ? kadenz.faellig (detail::WorkerKadenz::Uhr::now())
                : detail::WorkerKadenz::Faelligkeit {};
            if (faellig.schwer)
            {
                if (unverarbeitet > 0)
                {
                    unverarbeitet = 0;
                    schwereAuswertungen.fetch_add (1);
                    engine.auswerten();
                }
            }
            else if (faellig.leicht)
            {
                engine.auswertenLeicht();
            }
        }

        // SONDE-013 M-37/M-38: den Interventionsring leeren und senden.
        //
        // Er wird in JEDEM Workerzug geleert, nicht an einer Kadenz — die
        // Kadenz ist der Grund, warum das Heartbeat-Bit nicht reichte. Der
        // Zug laeuft spaetestens alle 50 ms, bei Rueckstau sofort.
        //
        // NAK-180 Nacharbeit 1 (EP-19): das Pausebit ist ein Testhaken. Es
        // steht im Produkt nie; nur so ist die Lage aus N-08 (Ring NICHT leer
        // beim Linkaufbau) erzwingbar statt vom Workertakt abhaengig.
        if (! senderPauseFuerTest.load (std::memory_order_relaxed))
            interventionenSenden();
        else
            // NAK-180 Nacharbeit 3 (WA-02/WA-04): die QUITTUNG der Pause. Erst
            // sie beweist einem Bein, dass der Worker aus `interventionenSenden`
            // heraus ist und keinen neuen Zug beginnt - das blosse Setzen des
            // Bits ordnet ihn nicht (er hat die Pruefung womoeglich schon
            // passiert). Im Produkt steht das Bit nie; dieser Zweig laeuft dort
            // also nicht.
            senderPauseQuittungFuerTest.fetch_add (1, std::memory_order_release);

        if (queueHatRest)
        {
            std::this_thread::yield();
            continue;
        }

        std::unique_lock<std::mutex> l (workerWarteMutex);
        workerWarte.wait_for (l, std::chrono::milliseconds (50),
                              [this] { return ! workerLaeuft.load(); });
    }
}

/*  Leert den RT→Control-Ring und schickt jedes Ereignis als P0
    (SONDE-013 M-37, M-38, M-39).

    ⚠️ Der Ueberlauf wird NICHT stillschweigend geschluckt. §34.2 verlangt
    sticky `intervention_state_unknown`; der Empfaenger leitet ihn heute aus
    der SEQUENZLUECKE ab, und genau deshalb vergibt der Audiothread die
    Nummer und nicht dieser Sender: ein Ereignis, das den Ring nie erreicht
    hat, hat seine Nummer trotzdem verbraucht, und die Luecke kommt beim
    Coordinator an, ohne dass ein zusaetzliches Feld noetig waere. Der
    Zaehler hier ist die lokale Gegenprobe dazu.
*/

bool EqCopilotProcessor::setzeAssistentSchritt (const juce::String& stepId,
                                                const juce::String& schritt,
                                                bool offen,
                                                const juce::String& findingId,
                                                const juce::String& proposalId,
                                                const juce::String& experimentId)
{
    // Die geschlossene Menge des Vertrags, hier als Riegel. Sie steht in
    // `$defs/assistant_schritt`; `preview` gehoert ausdruecklich dazu — dass
    // P5 keine Kante dorthin fuehrt, ist die Zustandsmaschine aus Etappe G,
    // nicht der Vertrag (E-07).
    static const char* const kSchritte[] = {
        "coverage", "finding", "evidence", "listen",
        "proposal", "preview", "remeasure", "verdict"
    };
    bool bekannt = false;
    for (const auto* s : kSchritte)
        bekannt = bekannt || schritt == s;
    if (! bekannt || ! nakama::ipc::istHex32 (stepId.toStdString()))
        return false;

    // 🔑 NR-08: dieser Weg SCHREIBT nichts mehr. Die Zustandsmaschine des
    // Schritts liegt im Main (E-08) und laeuft ueber `assistentStarten`,
    // `assistentWeiter` und ihre Geschwister; ein zweiter Schreibweg waere
    // eine zweite Zustandsmaschine, und ein Schatten mit eigener Revision war
    // genau der Defekt EP-08.
    //
    // Was bleibt, ist der Vertragsriegel und die Meldung: gesendet wird der
    // Schritt, den der MAIN-STATE haelt. Stimmen die genannten Werte nicht mit
    // ihm ueberein, meldet dieser Weg das ehrlich mit `false`, statt eine
    // Nachricht ueber einen Schritt zu bauen, den es nicht gibt.
    const auto kopie = assistentAusState();
    if (! kopie.gesetzt
        || kopie.stepId != stepId
        || juce::String (nakama::state::wort (kopie.schritt)) != schritt
        || kopie.offen != offen
        || (findingId.isNotEmpty()    && kopie.findingId    != findingId)
        || (proposalId.isNotEmpty()   && kopie.proposalId   != proposalId)
        || (experimentId.isNotEmpty() && kopie.experimentId != experimentId))
        return false;
    // Eine leere Nachricht ist KEIN Fehler: ohne gueltige Wire-Adresse gibt
    // es nichts zu senden, und der Schritt im Main-State steht trotzdem.
    // Genau dieser Rueckgabewert traegt `assistentStarten` und seine
    // Geschwister nach aussen.
    const auto json = v3AssistantStepJson();
    if (! json.empty())
        controlV3.sendeP1 ("assistant_step:" + v3Hello().adresse.sessionEpoch, json);
    return true;
}

EqCopilotProcessor::AssistentSchritt EqCopilotProcessor::assistentSchrittKopie() const
{
    // 🔑 NR-08: ABGELEITET aus dem Main-State, nie aus einem zweiten Feld.
    const auto z = assistentAusState();
    AssistentSchritt aus;
    aus.gesetzt      = z.gesetzt;
    aus.stepId       = z.stepId;
    aus.schritt      = juce::String (nakama::state::wort (z.schritt));
    aus.revision     = z.revision;
    aus.offen        = z.offen;
    aus.findingId    = z.findingId;
    aus.proposalId   = z.proposalId;
    aus.experimentId = z.experimentId;
    return aus;
}

bool EqCopilotProcessor::bindePassagenfenster (const juce::String& passageId,
                                               std::int64_t projektStart,
                                               std::int64_t projektEnde)
{
    // 🔑 Nacharbeit 3 (Befund C2/C3): DIE Bindung. Sie friert die
    // Transportepoche des Markierens ein und merkt sich, WELCHE Passage das
    // Fenster fuehrt.
    //
    // Die Epoche wird HIER gelesen, im Nachrichtenthread, unter derselben
    // Steuersperre wie jeder andere externe Engine-Leser — nicht spaeter im
    // Worker. Genau der Unterschied zwischen „vor dem Seek markiert" und
    // „nach dem Seek gesetzt" ist der Fehler, den der Vergleich in
    // `setzePassagenfenster` finden soll.
    std::uint64_t epoche = 0;
    {
        auto l = externerAnalyseSteuerZug();
        epoche = merkmale.transportEpocheJetzt();
    }
    return bindePassagenfensterMitEpoche (passageId, projektStart, projektEnde, epoche);
}

bool EqCopilotProcessor::bindePassagenfensterMitEpoche (const juce::String& passageId,
                                                        std::int64_t projektStart,
                                                        std::int64_t projektEnde,
                                                        std::uint64_t epoche)
{
    {
        std::lock_guard<std::mutex> l (passagenBindungMutex);
        gebundenePassageId = passageId;
        gebundenerStart = projektStart;
        gebundenesEnde  = projektEnde;
        gebundeneEpoche = epoche;
    }
    passagenfensterGeneration.fetch_add (1);
    passagenfensterStart.store (projektStart);
    passagenfensterEnde.store (projektEnde);
    passagenfensterEpocheWunsch.store (epoche);
    passagenfensterLoeschen.store (false);
    passagenfensterWunsch.store (true);
    // Befund R06/M-43: mit der markierten Passage beginnt die VORABmessung des
    // Vergleichspegels. `beginneVersuch` friert ihn spaeter ein; bis dahin
    // sammelt er. Ein frueher gemessener Pegel derselben Passage waere ein
    // anderer Bezugspunkt — deshalb faengt er hier von vorn an.
    //
    // Der Audiothread speist erst, wenn der Worker das Fenster WIRKLICH
    // gesetzt hat (`pegelFensterAktiv`); bis dahin bleibt der Pegel leer.
    // Lehnt die Engine das Fenster wegen Epochenwechsel ab, wird er nie
    // gefuellt — und ohne Material entsteht kein Versuch. Fail-closed.
    pegelFensterAktiv.store (false, std::memory_order_release);
    vergleichspegel.vorbereiten (getSampleRate());
    versuchNichtEndlich.store (0, std::memory_order_relaxed);
    versuchspegelSpeist.store (true, std::memory_order_release);
    v3StateRevision.fetch_add (1);
    return true;
}

bool EqCopilotProcessor::passagenfensterWunschFuerTest (const juce::String& passageId,
                                                        std::int64_t projektStart,
                                                        std::int64_t projektEnde,
                                                        std::uint64_t transportEpoche)
{
    return bindePassagenfensterMitEpoche (passageId, projektStart, projektEnde,
                                          transportEpoche);
}

bool EqCopilotProcessor::passagenfensterFuehrt (const juce::String& passageId) const
{
    {
        std::lock_guard<std::mutex> l (passagenBindungMutex);
        if (gebundenePassageId != passageId)
            return false;
    }
    // Die Bindung allein reicht nicht: die Engine kann den Wunsch abgelehnt
    // haben (Epochenwechsel) oder eine Transportgrenze kann durch das Fenster
    // gelaufen sein. Gefragt ist, was die Engine WIRKLICH fuehrt.
    auto l = externerAnalyseSteuerZug();
    return merkmale.passagenfensterIntakt();
}

juce::uint64 EqCopilotProcessor::versuchAufgenommeneBloecke() const
{
    return (juce::uint64) vergleichspegel.aufgenommeneBloecke();
}

void EqCopilotProcessor::vergleichszustandLeeren()
{
    // 🔑 NAK-181 R3 (G4-Befund V03, M-50): der Projektwechsel beendet den
    // Vergleichszustand.
    //
    // Die Menge ist genau die von `loesePassagenfenster` daneben, plus der
    // Versuchszustand — binden und loesen liegen damit in einem Aenderungssatz,
    // und wer die eine Menge erweitert, erweitert die andere.
    //
    // WAS HIER NICHT PASSIERT: es reist kein `experiment_abort`. Der v3-Vertrag
    // sagt es woertlich („Sitzungsende, Projektwechsel, Reconnect, UI-Neustart
    // und Brokerneustart brechen NICHT ab - ein offener Versuch ueberdauert sie
    // und bleibt danach rekonstruierbar"), und M-50 verlangt nicht, dass der
    // BELEG verschwindet, sondern dass die MESSUNG nicht stillschweigend
    // weiterlaeuft. Der persistierte Versuch bleibt beim Broker offen unter
    // seiner alten Bindung.
    //
    // Threads: alles laeuft auf dem Nachrichtenthread. Die vom Audiothread
    // gelesenen Flags bleiben Atomics mit derselben Ordnung wie in
    // `loesePassagenfenster`; `processBlock` nimmt keine der zwei Sperren.
    {
        std::lock_guard<std::mutex> l (versuchMutex);
        versuchIdAktiv = {};
        versuchPassageId = {};
    }
    blindvergleich.loeschen();
    {
        std::lock_guard<std::mutex> l (passagenBindungMutex);
        gebundenePassageId = {};
        gebundenerStart = gebundenesEnde = 0;
        gebundeneEpoche = 0;
    }
    passagenfensterGeneration.fetch_add (1);
    passagenfensterWunsch.store (false);
    passagenfensterLoeschen.store (true);
    pegelFensterAktiv.store (false, std::memory_order_release);
    versuchspegelSpeist.store (false, std::memory_order_release);
    vergleichspegel.loeschen();
    versuchNichtEndlich.store (0, std::memory_order_relaxed);
    // 🔑 NAK-181 Nacharbeit 1 (EP-04/NR-04): `letzterVersuchP0` bleibt
    // UNANGETASTET.
    //
    // Er ist der Mitschnitt dessen, was zuletzt WIRKLICH abgesetzt wurde —
    // der Zeuge fuer „aus dem Reload-Zug reist kein Byte" (N-09). Bis zu
    // dieser Runde loeschte der geprueft Zug seinen eigenen Zeugen: die
    // `.empty()`-Assertions danach waren danach in JEDEM Fall wahr, auch wenn
    // der Zug selbst einen Befehl eingereiht haette. Die Tests halten den
    // Stand VOR dem Reload fest und vergleichen ihn danach.
}

bool EqCopilotProcessor::loesePassagenfenster (const juce::String& passageId)
{
    {
        std::lock_guard<std::mutex> l (passagenBindungMutex);
        if (gebundenePassageId != passageId)
            return false;              // Eine FREMDE Passage loest nichts.
        gebundenePassageId = {};
        gebundenerStart = gebundenesEnde = 0;
        gebundeneEpoche = 0;
    }
    passagenfensterGeneration.fetch_add (1);
    passagenfensterWunsch.store (false);
    passagenfensterLoeschen.store (true);
    pegelFensterAktiv.store (false, std::memory_order_release);
    versuchspegelSpeist.store (false, std::memory_order_release);
    v3StateRevision.fetch_add (1);
    return true;
}

EqCopilotProcessor::Engineabzug EqCopilotProcessor::engineabzugLesen() const
{
    // 🔑 Nacharbeit 3 (Befund C7): EIN Zug fuer alles, was der Experimentpfad
    // aus der Engine braucht. Ohne ihn las er Fingerprint, Frame und
    // Passagenepoche ungesperrt, waehrend der Analyseworker dieselbe Engine
    // mutierte — ein Datenrennen, und drei Werte aus drei Staenden.
    auto l = externerAnalyseSteuerZug();
    Engineabzug a;
    a.fingerprint = merkmale.fingerprint();
    a.passagenEpoche = merkmale.passagenfensterEpoche();
    a.fensterGesetzt = merkmale.passagenfensterGesetzt();
    const auto& f = merkmale.frame();
    a.abdeckungGesetzt = f.abdeckungGesetzt;
    a.abdeckung = f.abdeckungGesetzt ? (double) f.abdeckung : 0.0;
    return a;
}

bool EqCopilotProcessor::passagenfensterInEngine (std::int64_t& start,
                                                  std::int64_t& ende) const
{
    // Die Engine gehoert dem Analyseworker; gelesen wird unter derselben
    // Steuersperre wie beim Snapshot daneben.
    auto l = externerAnalyseSteuerZug();
    if (! merkmale.passagenfensterGesetzt())
        return false;
    start = merkmale.passagenfensterStart();
    ende  = merkmale.passagenfensterEnde();
    return true;
}

std::string EqCopilotProcessor::versuchReferenzJson (const Engineabzug& abzug) const
{
    const auto& fp = abzug.fingerprint;
    const auto sicht = sourcesModel.sicht();
    std::string quellen = "[";
    std::string klassen = "[";
    int gezaehlt = 0;
    for (const auto& q : sicht.quellen)
    {
        // Nur klassifizierte Quellen mit gueltiger ID und BEKANNTEM Messpunkt:
        // eine Quelle ohne Messpunkt traegt keine Zuordnung, und eine geratene
        // waere schlimmer als keine (M-28/M-55).
        if (! nakama::ipc::istHex32 (q.instanceId)
            || q.messpunkt == SourcesModel::Messpunkt::unbekannt)
            continue;
        if (gezaehlt > 0) { quellen += ","; klassen += ","; }
        quellen += "\"" + q.instanceId + "\"";
        klassen += "\"";
        klassen += q.messpunkt == SourcesModel::Messpunkt::insert ? "insert"
                 : q.messpunkt == SourcesModel::Messpunkt::pre    ? "pre" : "post";
        klassen += "\"";
        ++gezaehlt;
    }
    if (gezaehlt == 0)
        return {};                       // Ohne aktives Quellenset kein Versuch.
    quellen += "]";
    klassen += "]";

    std::string s = "{\"passage_fingerprint\":" + fingerprintJson (fp);
    // Der Upstream-Fingerprint ist heute derselbe Traeger: das Plugin misst
    // genau EINEN Punkt der Kette. Ihn zu erfinden waere schlimmer als ihn
    // gleich zu setzen — und M-31 vergleicht ihn ohnehin nur mit sich selbst
    // ueber die Zeit.
    s += ",\"upstream_fingerprint\":" + fingerprintJson (fp);
    s += ",\"aktive_quellen\":" + quellen;
    s += ",\"messpunktklassen\":" + klassen;
    // 🔑 NAK-181 R1 (G4-Befund V01, M-43): der Match-Gain kommt aus der
    // EINGEFRORENEN Referenz, nicht aus dem lebenden Pegel.
    //
    // Der Blindvergleich haelt ihn als Kopie, seit `beginneVersuch` ihn
    // uebernommen hat; der lebende Pegel wird von `prepareToPlay` (`:485`) und
    // von jedem neuen Binden (`:2609`) geleert. Bis hierher las diese Zeile
    // den lebenden — und ein Kandidat nach einem Sampleratenwechsel trug
    // `match_gain_db: 0`, also „gleich laut" statt „nie gemessen"
    // (`Vergleichspegel.h:29-31`).
    //
    // Ohne eingefrorenen Gain entsteht KEINE Referenz und damit kein Kandidat:
    // beide Aufrufer pruefen `referenz.empty()`. Das ist der Riegel, den
    // `beginneVersuch` schon hatte und `erfasseKandidat` nicht.
    double eingefrorenerGain = 0.0;
    if (! blindvergleich.gainDbEingefroren (eingefrorenerGain))
        return {};
    s += ",\"match_gain_db\":" + zahl (eingefrorenerGain);
    // 🔑 Nacharbeit 3 (Befund C5, M-07/R06): der Nichtendlich-Zaehler REIST.
    //
    // Die Runde 2 machte ihn nur ueber einen Test-Getter sichtbar; im
    // Wirezustand stand er nicht, und der R06-Fall rief genau diesen Getter
    // auf. Damit war „reist in den Wirezustand" eine Behauptung ueber eine
    // Zeile, die es nicht gab. Das Feld ist optional in der Fassung 2 und
    // traegt 0 als „nachweislich keines", nicht als „nicht gemessen".
    // 🔑 NAK-181 R1: und derselbe Zaehler, den `beginneVersuch` eingefroren
    // hat (`:2816`), nicht der laufende. Ein nach dem Versuchsbeginn
    // gespeistes nichtendliches Sample gehoert zur naechsten Passage; im
    // Kandidaten dieses Versuchs waere es eine falsche Aussage — und ein nach
    // `vorbereiten()` genullter Zaehler die Behauptung „nachweislich keines"
    // (`Vergleichspegel.h:134-135`).
    s += ",\"nicht_endliche_samples\":"
       + std::to_string ((unsigned long long) versuchNichtEndlich.load (
             std::memory_order_relaxed));
    // M-21: kein Host validiert heute die Presentation-Abbildung. `probable`
    // ist die staerkste Klasse, die dieser Pfad tragen darf.
    s += ",\"alignment\":\"probable\"}";
    return s;
}

std::string EqCopilotProcessor::versuchKopfJson (const juce::String& commandId) const
{
    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    if (! nakama::ipc::adresseGueltig (h.adresse))
        return {};
    std::string s = "{\"command_id\":\"" + commandId.toStdString() + "\"";
    s += ",\"ziel\":" + nakama::ipc::adresseAlsJson (h.adresse);
    // 🔑 SONDE-014 WN-01 (Nacharbeit 2, 07.09.2026): die GEMELDETE Revision.
    //
    // Hier stand `v3StateRevision.load()` - der lokale Zaehler, den
    // `assistentAenderungMelden` unmittelbar davor erhoeht. Der `state_report`
    // reist als P1 im 1-Hz-Takt, der Befehl als P0; der P0 ueberholt den
    // Bericht strukturell, und `befehl.rs` antwortet auf einen Kopf mit einer
    // Zahl, die der Broker nicht kennt, mit `revision_conflict`. Der Konflikt
    // war damit der Regelfall, nicht ein Zeitfenster (WP1-1).
    //
    // 🔑 KR-01 (E-15): hier stand bis zur Runde 3 ein `baseRevision`-Parameter,
    // mit dem die Wiederholung nach einem `konflikt`-ACK ihre Zahl setzte. Sie
    // baut den Kopf nicht mehr neu, sondern ersetzt die Zahl im gesendeten
    // Auftrag (`nakama::ipc::auftragMitBasisRevision`) - ein Neubau haette
    // jedes andere Feld aus dem aktuellen Zustand gezogen.
    s += ",\"base_revision\":" + std::to_string (controlV3.gemeldeteStateRevision());
    s += ",\"ttl_ms\":2000,\"schema_major\":3,\"schema_minor\":0}";
    return s;
}

bool EqCopilotProcessor::sendeVersuchP0 (const std::string& json)
{
    if (! controlV3.sendePersistenzP0 (json))
        return false;
    std::lock_guard<std::mutex> l (versuchWireMutex);
    letzterVersuchP0 = json;
    return true;
}

bool EqCopilotProcessor::beginneVersuch (const juce::String& passageId)
{
    if (! nakama::ipc::istHex32 (passageId.toStdString()))
        return false;
    nakama::state::ManuellePassage passage;
    bool gefunden = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        for (const auto& p : zustand.manuellePassagen)
            if (p.passageId == passageId) { passage = p; gefunden = true; break; }
    }
    if (! gefunden)
        return false;

    {
        std::lock_guard<std::mutex> l (versuchMutex);
        if (versuchIdAktiv.isNotEmpty())
            return false;                // Ein Versuch nach dem anderen.
    }

    // 🔑 Nacharbeit 3 (Befund C3, M-25): der Versuch gilt GENAU der Passage,
    // die das Fenster gerade fuehrt.
    //
    // Bis dahin adressierte `beginneVersuch` per ID, fror aber den zuletzt
    // gesetzten GLOBALEN Stand ein. Wer A und danach B markierte und dann A
    // begann, bekam den Vergleichspegel von B unter der ID von A — eine
    // Lautheitszahl, die zu anderem Material gehoert. Ist eine andere Passage
    // gebunden, wird das Fenster auf DIESE umgehaengt und der Versuch
    // abgelehnt: der Pegel dieser Passage ist dann noch gar nicht gemessen,
    // und Paragraph 15 laesst keine Klangwertung ohne Lautheitsabgleich zu.
    bool istGebunden = false;
    {
        std::lock_guard<std::mutex> l (passagenBindungMutex);
        istGebunden = (gebundenePassageId == passageId);
    }
    if (! istGebunden)
    {
        bindePassagenfenster (passageId, passage.projektStart, passage.projektEnde);
        return false;
    }

    // 🔑 M-43/§15: der Vergleichspegel wird EINGEFROREN, bevor irgendetwas
    // gesendet wird. Gelingt das nicht — zu wenig Material oder nichtendliche
    // Samples —, entsteht kein Versuch. Eine Klangwertung ohne vorherigen
    // Lautheitsabgleich ist unzulaessig.
    versuchspegelSpeist.store (false, std::memory_order_release);
    const bool eingefroren = vergleichspegel.friereEin();
    versuchNichtEndlich.store (vergleichspegel.nichtEndlicheSamples(),
                               std::memory_order_relaxed);
    if (! eingefroren || ! vergleichspegel.gainGesetzt())
    {
        // 🔑 NAK-181 R2 (G4-Befund V02): „zu wenig Material" ist KEIN
        // Endzustand — die Speisung geht wieder an.
        //
        // `friereEin()` kennt drei Ausgaenge, und nur der Rueckgabewert
        // unterscheidet sie nicht: bei nichtendlichen Samples friert der Pegel
        // EIN (ohne Wert, M-07 „ein gesperrter Zustand ohne Wert ist
        // ehrlich"), bei zu wenig Material ausdruecklich NICHT
        // (`Vergleichspegel.h:203-208`, auf Retry ausgelegt). `eingefroren()`
        // trennt die beiden.
        //
        // Bis hierher blieb `versuchspegelSpeist` in beiden Faellen aus. Der
        // einzige Setzer auf `true` ist `bindePassagenfensterMitEpoche`; bei
        // GEBUNDENER Passage war der Handgriff damit beim zweiten Druck stumm
        // tot — derselbe Fehler wie ein totes Element, und der Code benennt
        // ihn bei `:2152-2159` selbst.
        if (! vergleichspegel.eingefroren())
            versuchspegelSpeist.store (true, std::memory_order_release);
        return false;
    }
    if (! blindvergleich.uebernimmVergleichspegel (vergleichspegel))
        return false;

    // 🔑 NAK-181 Nacharbeit 1 (EP-02/NR-02a): ein eingefrorener Gain im
    // Blindvergleich existiert NUR bei offenem Versuch.
    //
    // Ab hier haelt der Blindvergleich den Gain, und `versuchIdAktiv` ist noch
    // leer. Bis zu dieser Runde fuehrten vier Fehlwege (`:2928` kein Fenster,
    // `:2931` leere Referenz, `:2936` kein Kopf, `:2958` Sendefehler) mit
    // genau diesem Zustand hinaus — den E1 ausschliesst. Zwei Folgen: die
    // Getter meldeten „abgeglichen" ohne Versuch, und weil
    // `Blindvergleich::uebernimmVergleichspegel` eine ZWEITE Uebernahme
    // ablehnt (`Blindvergleich.h:93`), scheiterte danach JEDER weitere Beginn
    // bis zum Projektwechsel — ein stumm totes Bedienelement.
    //
    // Die Ruecknahme haengt am Geltungsbereich statt an vier Zeilen: ein
    // spaeter eingefuegter Fehlweg ist damit von selbst richtig, und genau
    // diese Frage stellt der Selbstaudit („gibt es noch einen Weg zu Gain
    // eingefroren, kein Versuch offen?"). Nachrichtenthread; `processBlock`
    // nimmt keine der Sperren.
    struct Ruecknahme
    {
        nakama::analyse::Blindvergleich* ziel { nullptr };
        bool behalten { false };
        ~Ruecknahme() { if (! behalten && ziel != nullptr) ziel->loeschen(); }
    } ruecknahme { &blindvergleich, false };

    // Befund C7: EIN Zug fuer Fingerprint, Passagenepoche und Abdeckung.
    const auto abzug = engineabzugLesen();
    // Ohne gebundenes Fenster in der Engine gibt es keine Passagenmessung —
    // und ohne die traegt der Versuch weder Fingerprint noch Abdeckung dieser
    // Passage. Das ist der Fall, in dem die Engine das Fenster wegen eines
    // Epochenwechsels ABGELEHNT hat (Befund C2).
    if (! abzug.fensterGesetzt)
        return false;
    const auto referenz = versuchReferenzJson (abzug);
    if (referenz.empty())
        return false;
    const juce::String versuchId { uuidHex32() };
    const juce::String commandId { uuidHex32() };
    const auto kopf = versuchKopfJson (commandId);
    if (kopf.empty())
        return false;

    const auto& fp = abzug.fingerprint;
    std::string quellenTeil = referenz.substr (referenz.find ("\"aktive_quellen\""));
    quellenTeil = quellenTeil.substr (0, quellenTeil.find (",\"match_gain_db\""));

    std::string json = "{\"type\":\"experiment_begin\",\"kopf\":" + kopf;
    json += ",\"experiment_id\":\"" + versuchId.toStdString() + "\"";
    json += ",\"execution_mode\":\"manual_external\"";
    json += ",\"reproduzierbarkeit\":\"manuell_nicht_wiederherstellbar\"";
    json += ",\"passage\":{\"passage_id\":\"" + passageId.toStdString() + "\"";
    json += ",\"projekt_von\":" + std::to_string (passage.projektStart);
    json += ",\"projekt_bis\":" + std::to_string (passage.projektEnde);
    json += ",\"transport_epoch\":" + std::to_string (abzug.passagenEpoche);
    json += "," + quellenTeil;
    json += ",\"abdeckung\":" + zahl (abzug.abdeckung);
    json += ",\"label\":" + (passage.label.isEmpty() ? std::string ("null")
                                                     : jsonText (passage.label));
    json += ",\"fingerprint\":" + fingerprintJson (fp) + "}";
    json += ",\"referenz\":" + referenz + "}";

    if (! sendeVersuchP0 (json))
        return false;
    ruecknahme.behalten = true;      // ab hier traegt der offene Versuch den Gain
    std::lock_guard<std::mutex> l (versuchMutex);
    versuchIdAktiv = versuchId;
    versuchPassageId = passageId;
    return true;
}

bool EqCopilotProcessor::erfasseKandidat (bool kandidatZuerst)
{
    juce::String versuchId;
    {
        std::lock_guard<std::mutex> l (versuchMutex);
        versuchId = versuchIdAktiv;
    }
    if (versuchId.isEmpty())
        return false;
    // M-44: die Reihenfolge wird HIER gebunden — vor dem Hoeren. Ein zweiter
    // Aufruf aendert sie nicht; `bindeReihenfolge` meldet das mit `false`, und
    // das ist kein Fehler, sondern die Zusage.
    blindvergleich.bindeReihenfolge (kandidatZuerst
                                       ? nakama::analyse::Blindreihenfolge::kandidatZuerst
                                       : nakama::analyse::Blindreihenfolge::baselineZuerst);
    const auto referenz = versuchReferenzJson (engineabzugLesen());
    if (referenz.empty())
        return false;
    const juce::String commandId { uuidHex32() };
    const auto kopf = versuchKopfJson (commandId);
    if (kopf.empty())
        return false;
    std::string json = "{\"type\":\"experiment_candidate\",\"kopf\":" + kopf;
    json += ",\"experiment_id\":\"" + versuchId.toStdString() + "\"";
    json += ",\"referenz\":" + referenz;
    json += ",\"blindreihenfolge\":\"";
    json += kandidatZuerst ? "kandidat_zuerst" : "baseline_zuerst";
    json += "\"}";
    return sendeVersuchP0 (json);
}

bool EqCopilotProcessor::urteileVersuch (const juce::String& hoerurteil,
                                         const juce::String& notiz,
                                         const juce::String& werkzeug)
{
    if (hoerurteil != "baseline" && hoerurteil != "kandidat"
        && hoerurteil != "kein_unterschied" && hoerurteil != "enthaltung")
        return false;
    juce::String versuchId;
    {
        std::lock_guard<std::mutex> l (versuchMutex);
        versuchId = versuchIdAktiv;
    }
    if (versuchId.isEmpty())
        return false;
    // Der Riegel liegt IM Typ: ohne Lautheitsabgleich und ohne gebundene
    // Reihenfolge nimmt `Blindvergleich` kein Urteil an (M-43/M-44).
    const auto urteil = hoerurteil == "baseline"   ? nakama::analyse::Hoerurteil::baseline
                      : hoerurteil == "kandidat"   ? nakama::analyse::Hoerurteil::kandidat
                      : hoerurteil == "kein_unterschied"
                            ? nakama::analyse::Hoerurteil::keinUnterschied
                            : nakama::analyse::Hoerurteil::enthaltung;
    if (! blindvergleich.urteile (urteil))
        return false;
    nakama::analyse::Blindreihenfolge aufgedeckt {};
    if (! blindvergleich.aufgedeckteReihenfolge (aufgedeckt))
        return false;
    const juce::String commandId { uuidHex32() };
    const auto kopf = versuchKopfJson (commandId);
    if (kopf.empty())
        return false;
    std::string json = "{\"type\":\"experiment_manual_result\",\"kopf\":" + kopf;
    json += ",\"experiment_id\":\"" + versuchId.toStdString() + "\"";
    json += ",\"hoerurteil\":\"" + hoerurteil.toStdString() + "\"";
    json += ",\"blindreihenfolge\":\"";
    json += aufgedeckt == nakama::analyse::Blindreihenfolge::kandidatZuerst
              ? "kandidat_zuerst" : "baseline_zuerst";
    json += "\"";
    json += ",\"notiz\":" + (notiz.isEmpty() ? std::string ("null") : jsonText (notiz));
    json += ",\"werkzeug\":" + (werkzeug.isEmpty() ? std::string ("null")
                                                   : jsonText (werkzeug));
    json += "}";
    if (! sendeVersuchP0 (json))
        return false;
    std::lock_guard<std::mutex> l (versuchMutex);
    versuchIdAktiv = {};
    versuchPassageId = {};
    // 🔑 Nacharbeit 3 (Befund C8, Arbeitsregel „aktivieren↔abklingen"): der
    // Blindvergleich wird geleert wie im Abbruchpfad.
    //
    // Ohne diese Zeile behielt er Urteil und `gainGesetzt` ueber das Ergebnis
    // hinaus; der ZWEITE Versuch scheiterte danach dauerhaft an
    // `uebernimmVergleichspegel`, weil ein Pegel genau einmal je Vergleich
    // uebernommen werden darf. Ein Handgriff, der beim zweiten Mal stumm
    // nicht mehr geht, ist derselbe Fehler wie ein totes Element.
    blindvergleich.loeschen();
    return true;
}

bool EqCopilotProcessor::brichVersuchAb()
{
    juce::String versuchId;
    {
        std::lock_guard<std::mutex> l (versuchMutex);
        versuchId = versuchIdAktiv;
    }
    if (versuchId.isEmpty())
        return false;
    const juce::String commandId { uuidHex32() };
    const auto kopf = versuchKopfJson (commandId);
    if (kopf.empty())
        return false;
    std::string json = "{\"type\":\"experiment_abort\",\"kopf\":" + kopf;
    json += ",\"experiment_id\":\"" + versuchId.toStdString() + "\"";
    json += ",\"grund\":\"user_abbruch\"}";
    if (! sendeVersuchP0 (json))
        return false;
    std::lock_guard<std::mutex> l (versuchMutex);
    versuchIdAktiv = {};
    versuchPassageId = {};
    blindvergleich.loeschen();
    return true;
}

juce::String EqCopilotProcessor::laufenderVersuch() const
{
    std::lock_guard<std::mutex> l (versuchMutex);
    return versuchIdAktiv;
}

bool EqCopilotProcessor::versuchLautheitAbgeglichen() const
{
    // 🔑 NAK-181 R1: bei OFFENEM Versuch antwortet die eingefrorene Referenz.
    // Bis hierher las auch dieser Leser den lebenden Pegel und behauptete nach
    // einem `prepareToPlay` „kein Lautheitsabgleich", obwohl der Versuch seinen
    // Gain laengst haelt — dieselbe Falschaussage wie V01, nur auf der
    // Anzeigeseite. Ohne offenen Versuch bleibt der lebende Pegel die richtige
    // Antwort: dort lautet die Frage „ist schon genug Material da".
    //
    // 🔑 NAK-181 Nacharbeit 1 (EP-02/NR-02b): „offener Versuch" wird GEFRAGT,
    // nicht aus dem gesetzten Gain geschlossen. N-04 Satz 2 sagt woertlich
    // „Ohne offenen Versuch lesen beide weiter den lebenden Pegel"; ein
    // Blindvergleich mit Gain und leerer `versuchIdAktiv` ist nach NR-02a
    // unerreichbar — dieser Riegel macht die Zusage unabhaengig davon wahr.
    if (! laufenderVersuch().isEmpty())
    {
        double unbenutzt = 0.0;
        if (blindvergleich.gainDbEingefroren (unbenutzt))
            return true;
    }
    return vergleichspegel.eingefroren() && vergleichspegel.gainGesetzt();
}

double EqCopilotProcessor::versuchMatchGainDb() const
{
    // 🔑 NAK-181 Nacharbeit 1 (EP-02/NR-02b): dieselbe Frage wie nebenan.
    if (! laufenderVersuch().isEmpty())
    {
        double eingefroren = 0.0;
        if (blindvergleich.gainDbEingefroren (eingefroren))
            return eingefroren;
    }
    return vergleichspegel.gainDb();
}

bool EqCopilotProcessor::versuchLautheitAbgeglichenLebendFuerTest() const
{
    return vergleichspegel.eingefroren() && vergleichspegel.gainGesetzt();
}

void EqCopilotProcessor::vergleichspegelZaehlerstand (juce::uint64& bloecke,
                                                       juce::uint64& endliche,
                                                       juce::uint64& nichtEndliche) const
{
    // 🔑 NAK-181 R1a (Matrixnacharbeit 1, MP1-8): der Leser fuer den LEBENDEN
    // Zaehler. `versuchNichtEndlicheSamples()` liest das eingefrorene Atomic —
    // den Stand vom letzten Binden oder Beginnversuch — und ist damit der
    // falsche Zeuge fuer die Frage „waechst der lebende Zaehler weiter".
    //
    // Drei Zaehler in EINEM Torzug, wie `Vergleichspegel::zaehlerstand` sie
    // herausgibt: drei Einzelaufrufe koennten drei Staende sehen.
    std::uint64_t b = 0, e = 0, n = 0;
    vergleichspegel.zaehlerstand (b, e, n);
    bloecke = (juce::uint64) b;
    endliche = (juce::uint64) e;
    nichtEndliche = (juce::uint64) n;
}

juce::uint64 EqCopilotProcessor::versuchNichtEndlicheSamples() const
{
    return (juce::uint64) versuchNichtEndlich.load (std::memory_order_relaxed);
}

bool EqCopilotProcessor::assistentAenderungMelden (bool veraendert)
{
    if (! veraendert)
        return true;
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
    nakama::state::Assistentenzustand kopie {};
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        kopie = zustand.assistent;
    }
    if (! kopie.gesetzt)
        return true;
    // M-88: der Schritt reist unter `assistant_step:<session_epoch>` zum
    // Broker, der ihn versioniert spiegelt. Der Spiegel ist NIE autoritativ.
    return setzeAssistentSchritt (kopie.stepId,
                                  juce::String (nakama::state::wort (kopie.schritt)),
                                  kopie.offen, kopie.findingId, kopie.proposalId,
                                  kopie.experimentId);
}

bool EqCopilotProcessor::assistentStarten (const juce::String& stepId)
{
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        if (! nakama::state::setzeAssistentenschritt (
                zustand, stepId, nakama::state::Assistentenschritt::coverage,
                veraendert, grund))
            return false;
    }
    return assistentAenderungMelden (veraendert);
}

bool EqCopilotProcessor::assistentWeiter (nakama::state::Assistentenschritt schritt)
{
    bool veraendert = false;
    juce::String stepId;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        if (! zustand.assistent.gesetzt)
            return false;
        stepId = zustand.assistent.stepId;
        juce::String grund;
        if (! nakama::state::setzeAssistentenschritt (zustand, stepId, schritt,
                                                      veraendert, grund))
            return false;
    }
    return assistentAenderungMelden (veraendert);
}

bool EqCopilotProcessor::assistentZurueck()
{
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        if (! nakama::state::assistentZurueck (zustand, veraendert, grund))
            return false;
    }
    return assistentAenderungMelden (veraendert);
}

bool EqCopilotProcessor::assistentUeberspringen()
{
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        if (! nakama::state::assistentUeberspringen (zustand, veraendert, grund))
            return false;
    }
    return assistentAenderungMelden (veraendert);
}

bool EqCopilotProcessor::assistentAbbrechen()
{
    bool veraendert = false;
    bool hatVersuch = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        if (! nakama::state::assistentAbbrechen (zustand, veraendert, grund))
            return false;
        hatVersuch = zustand.assistent.experimentId.isNotEmpty();
    }
    // ⚠️ REIHENFOLGE AUS M-58: Terminalereignis → Projektion → Outbox → erst
    // DANACH den Slot freigeben. `brichVersuchAb` sendet das
    // `experiment_abort` und raeumt `versuchIdAktiv` in genau dieser Ordnung;
    // wer den Slot vorher freigaebe, koennte einen zweiten Versuch starten,
    // waehrend der erste noch nicht terminal ist.
    const bool gemeldet = assistentAenderungMelden (veraendert);
    if (hatVersuch)
        brichVersuchAb();
    return gemeldet;
}

bool EqCopilotProcessor::assistentAntwort (nakama::state::Assistentenergebnis ergebnis,
                                           const nakama::state::Userurteil* urteil,
                                           const juce::String& findingId,
                                           const juce::String& notiz)
{
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        juce::String grund;
        if (! nakama::state::setzeAssistentenergebnis (zustand, ergebnis, veraendert, grund))
            return false;
    }
    const bool gemeldet = assistentAenderungMelden (veraendert);
    // 🔑 NR-10 (Nacharbeit 1), M-73/E-09: das Urteil reist ZUSAETZLICH zum
    // `assistant_step_update` — als persistenzpflichtiger P0, der nicht
    // koalesziert.
    //
    // Die Reihenfolge ist Absicht: erst der Schritt (P1, koaleszierend), dann
    // das Urteil (P0). Der Schritt beschreibt, WO der Assistent steht; das
    // Urteil ist die Aussage des Users darueber und darf nie von einem
    // spaeteren Schritt ueberholt werden.
    if (urteil != nullptr)
    {
        // 🔑 WN-05 (Nacharbeit 2, 07.09.2026), M-73: der Rueckgabewert der
        // Queue-Politik wird AUSGEWERTET.
        //
        // Bis hierher ging er in den Abgrund: bei voller 64er-P0-Queue liefert
        // `sendePersistenzP0` false, und diese Methode meldete trotzdem
        // Erfolg. Das Urteil war weder persistiert noch beim Reconnect
        // wiederholbar - genau das, was M-73 mit "nie stillschweigend
        // geloescht" ausschliesst (WP1-5).
        //
        // Seit WN-05 bleibt der abgewiesene Auftrag im In-Flight-Register und
        // geht an `beiP0Verworfen`; hier faellt nur noch die ehrliche Antwort:
        // eingereiht oder nicht.
        const auto json = v3UserVerdictJson (*urteil, findingId, notiz);
        if (json.empty())
            return false;
        if (! controlV3.sendePersistenzP0 (json))
            return false;
    }
    return gemeldet;
}

nakama::state::Assistentenzustand EqCopilotProcessor::assistentAusState() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.assistent;
}

bool EqCopilotProcessor::assistentFortsetzen (nakama::state::Assistentenzustand& aus) const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return nakama::state::assistentResume (zustand, aus);
}

bool EqCopilotProcessor::assistentVersuchStarten (const juce::String& passageId)
{
    // M-62: der Assistent erzeugt KEINE eigene Experimentfamilie. Er ruft den
    // bestehenden Weg — `experiment_begin` → `experiment_candidate` →
    // `experiment_manual_result` beziehungsweise `experiment_abort` — und
    // schreibt nie selbst in den Experimentstore.
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (! zustand.assistent.gesetzt || ! zustand.assistent.offen)
            return false;
    }
    if (! beginneVersuch (passageId))
        return false;
    juce::String versuchId;
    {
        std::lock_guard<std::mutex> l (versuchMutex);
        versuchId = versuchIdAktiv;
    }
    bool veraendert = false;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.assistent.experimentId == versuchId)
            return true;
        zustand.assistent.experimentId = versuchId;
        zustand.assistent.revision += 1;
        veraendert = true;
    }
    return assistentAenderungMelden (veraendert);
}

bool EqCopilotProcessor::schreibeSnapshotDatei (juce::String& pfadOderFehler)
{
    const auto m = engine.snapshot();
    if (m.zustand == MessZustand::keineDaten)
    {
        pfadOderFehler = "noch keine Messdaten";
        return false;
    }

    auto zahl = [] (double v, bool gueltig = true)
    {
        return (gueltig && std::isfinite (v)) ? juce::var (v) : juce::var();
    };
    auto* wurzel = new juce::DynamicObject();
    // v3 (M3a): zusätzlich Band-Perzentile, Zonen-Zeitverlauf und Konvergenz —
    // v2 brachte die Befundkarten. Ältere Leser ignorieren neue Felder.
    wurzel->setProperty ("snapshot_version", 3);
    wurzel->setProperty ("metrics_version", kMetricsVersion);
    wurzel->setProperty ("diagnose_version", kDiagnoseVersion);
    wurzel->setProperty ("created_utc", juce::Time::getCurrentTime().toISO8601 (true));

    {
        auto* sensor = new juce::DynamicObject();
        std::lock_guard<std::mutex> l (bindungMutex);
        const auto& c = zustand.common;
        sensor->setProperty ("sensor_id", c.instanceId);
        sensor->setProperty ("role", nakama::state::v2Rolle (c));
        sensor->setProperty ("label", c.label);
        sensor->setProperty ("pair_id", c.pairId.isEmpty() ? juce::var() : juce::var (c.pairId));
        sensor->setProperty ("samplerate", m.samplerate);
        sensor->setProperty ("channels", kanaeleAtomic.load());
        wurzel->setProperty ("sensor", juce::var (sensor));
    }

    wurzel->setProperty ("zustand", m.zustand == MessZustand::messbereit ? "messbereit" : "sammelt");
    wurzel->setProperty ("aktiv_sekunden", m.aktivSekunden);
    wurzel->setProperty ("gesamt_sekunden", m.gesamtSekunden);
    // Ehrlichkeits-Ausweis (Paket C): so viele nicht-endliche Eingangssamples
    // hat die Analyse seit dem Messstart durch Stille ersetzt.
    wurzel->setProperty ("nan_ersetzt_samples", (juce::int64) m.nanErsetzt);

    {
        auto* loud = new juce::DynamicObject();
        loud->setProperty ("lufs_integriert", zahl (m.lufsIntegriert, m.lufsGueltig));
        loud->setProperty ("lufs_short", zahl (m.lufsShort, m.lufsShortGueltig));
        loud->setProperty ("true_peak_dbtp", zahl (m.truePeakDb));
        loud->setProperty ("crest_db", zahl (m.crestDb, m.crestGueltig));
        wurzel->setProperty ("loudness", juce::var (loud));
    }
    {
        auto* sp = new juce::DynamicObject();
        sp->setProperty ("centroid_mag_hz", zahl (m.centroidMagHz, m.spektralGueltig));
        sp->setProperty ("rolloff_hz", zahl (m.rolloffHz, m.spektralGueltig));
        sp->setProperty ("low_frac", zahl (m.lowFrac, m.spektralGueltig));
        sp->setProperty ("flatness", zahl (m.flatness, m.spektralGueltig));
        juce::Array<juce::var> band;
        for (const double v : m.band8Prozent)
            band.add (zahl (v, m.spektralGueltig));
        sp->setProperty ("band_pct", band);
        wurzel->setProperty ("spektral", juce::var (sp));
    }
    {
        auto* st = new juce::DynamicObject();
        st->setProperty ("width", zahl (m.width, m.stereoGueltig));
        st->setProperty ("corr", zahl (m.corr, m.stereoGueltig));
        st->setProperty ("echt_stereo", m.stereoGueltig);
        wurzel->setProperty ("stereo", juce::var (st));
    }
    {
        auto* ltas = new juce::DynamicObject();
        juce::Array<juce::var> zentren, komposit, referenzKurve;
        for (int b = 0; b < kLtasBaender; ++b)
        {
            zentren.add (m.ltasZentrenHz[(size_t) b]);
            komposit.add (zahl (m.ltasKompositDb[(size_t) b], m.ltasGueltig));
            referenzKurve.add (zahl (m.ltasReferenzDb[(size_t) b], m.ltasGueltig));
        }
        ltas->setProperty ("zentren_hz", zentren);
        ltas->setProperty ("komposit_db", komposit);
        ltas->setProperty ("referenz_8192_db", referenzKurve);
        wurzel->setProperty ("ltas", juce::var (ltas));
    }
    {
        juce::Array<juce::var> ab;
        for (const auto& a : m.abdeckung)
        {
            auto* o = new juce::DynamicObject();
            o->setProperty ("zentrum_hz", a.zentrumHz);
            o->setProperty ("anteil", a.anteil);
            o->setProperty ("klasse", a.klasse == AbdeckungsKlasse::belastbar ? "belastbar"
                                    : a.klasse == AbdeckungsKlasse::eingeschraenkt ? "eingeschraenkt"
                                                                                   : "nicht_messbar");
            ab.add (juce::var (o));
        }
        wurzel->setProperty ("abdeckung", ab);
    }
    {
        // M3a: Band-Perzentile (1-dB-Quantisierung, Zuständigkeits-Stufe).
        auto* pz = new juce::DynamicObject();
        juce::Array<juce::var> p10, p50, p95;
        for (int b = 0; b < kLtasBaender; ++b)
        {
            p10.add (zahl (m.perzentilP10[(size_t) b], m.perzentileGueltig));
            p50.add (zahl (m.perzentilP50[(size_t) b], m.perzentileGueltig));
            p95.add (zahl (m.perzentilP95[(size_t) b], m.perzentileGueltig));
        }
        pz->setProperty ("p10_db", p10);
        pz->setProperty ("p50_db", p50);
        pz->setProperty ("p95_db", p95);
        wurzel->setProperty ("perzentile", juce::var (pz));
    }
    {
        // M3a: Zonen-Zeitverlauf (Anteil aktiver 1-s-Ticks jenseits der
        // Regel-Schwelle; Reihenfolge = ZonenRegeln.h).
        static const char* zonenNamen[kZonenAnzahl] = { "mitten_loch", "mulm", "haerte", "hoehen_hype" };
        juce::Array<juce::var> zz;
        for (int z = 0; z < kZonenAnzahl; ++z)
        {
            const auto& w = m.zonenZeit[(size_t) z];
            auto* o = new juce::DynamicObject();
            o->setProperty ("zone", zonenNamen[z]);
            o->setProperty ("gueltig", w.gueltig);
            o->setProperty ("anteil_jenseits", w.anteil);
            o->setProperty ("ticks", (int) w.ticks);
            zz.add (juce::var (o));
        }
        wurzel->setProperty ("zonen_zeit", zz);
    }
    {
        // M3a: Konvergenz „Kurve steht" (rein informativ).
        auto* ko = new juce::DynamicObject();
        ko->setProperty ("gueltig", m.konvergenzGueltig);
        ko->setProperty ("mean_db", zahl (m.konvergenzMeanDb, m.konvergenzGueltig));
        ko->setProperty ("max_db", zahl (m.konvergenzMaxDb, m.konvergenzGueltig));
        wurzel->setProperty ("konvergenz", juce::var (ko));
    }
    {
        juce::Array<juce::var> res;
        for (const auto& r : m.resonanzen)
        {
            auto* o = new juce::DynamicObject();
            o->setProperty ("freq_hz", r.freqHz);
            o->setProperty ("db_over", r.dbOver);
            o->setProperty ("persistenz", r.persistenzAnteil);
            o->setProperty ("breite_okt", r.breiteOktaven);
            o->setProperty ("klasse", r.klasse == ResonanzKlasse::dauerhaft ? "dauerhaft" : "zeitweise");
            res.add (juce::var (o));
        }
        wurzel->setProperty ("resonanzen", res);
    }
    {
        // M3-Kern: die Befundkarten (deterministisch aus GENAU diesem
        // Messstand — dieselbe Funktion wie Hinweis-Knopf und Golden-Test).
        const auto befunde = diagnose (m, holeRolle());
        juce::Array<juce::var> bf;
        for (const auto& b : befunde)
        {
            auto* o = new juce::DynamicObject();
            o->setProperty ("klasse", b.klasse == BefundKlasse::resonanz ? "resonanz"
                                    : b.klasse == BefundKlasse::mittenLoch ? "mitten_loch"
                                    : b.klasse == BefundKlasse::mulm ? "mulm"
                                    : b.klasse == BefundKlasse::haerte ? "haerte"
                                                                       : "hoehen_hype");
            o->setProperty ("f_von_hz", b.fVon);
            o->setProperty ("f_bis_hz", b.fBis);
            o->setProperty ("f_schwerpunkt_hz", b.fSchwerpunkt);
            o->setProperty ("staerke_db", b.staerkeDb);
            o->setProperty ("titel", b.titel);
            o->setProperty ("gemessen", b.gemessen);
            o->setProperty ("wirkung", b.wirkung);
            o->setProperty ("tu", b.tu);
            o->setProperty ("warum", b.warum);
            o->setProperty ("hoeren", b.hoeren);
            o->setProperty ("konfidenz", b.konfidenz == Konfidenz::hoch ? "hoch"
                                       : b.konfidenz == Konfidenz::mittel ? "mittel" : "niedrig");
            o->setProperty ("konfidenz_grund", b.konfidenzGrund);
            o->setProperty ("top_rang", b.topRang);
            bf.add (juce::var (o));
        }
        wurzel->setProperty ("befunde", bf);
    }
    wurzel->setProperty ("raw_audio", juce::var());   // per Vertrag immer null

    auto ordner = juce::File::getSpecialLocation (juce::File::windowsLocalAppData)
                      .getChildFile ("evenacadia").getChildFile ("EQ-Copilot")
                      .getChildFile ("snapshots");
    if (! ordner.createDirectory())
    {
        pfadOderFehler = "Ordner nicht anlegbar: " + ordner.getFullPathName();
        return false;
    }
    juce::String labelTeil;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        for (const auto z : zustand.common.label)
            if (juce::CharacterFunctions::isLetterOrDigit (z) || z == '-')
                labelTeil += z;
    }
    const auto zeit = juce::Time::getCurrentTime().formatted ("%Y%m%d-%H%M%S");
    const auto datei = ordner.getChildFile ("snapshot-" + zeit
                                            + (labelTeil.isEmpty() ? "" : "-" + labelTeil.substring (0, 40))
                                            + ".json");
    if (! datei.replaceWithText (juce::JSON::toString (juce::var (wurzel), false), false, false, "\n"))
    {
        pfadOderFehler = "Schreiben fehlgeschlagen: " + datei.getFullPathName();
        return false;
    }
    pfadOderFehler = datei.getFullPathName();
    return true;
}

} // namespace eqcop
