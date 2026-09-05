#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EqCopilotIds.h"
#include "Diagnose.h"
#include "WorkerCadence.h"
#include "BrokerInstallBinding.h"
#include "PipeToken.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <process.h>

namespace eqcop
{

namespace
{
bool projektEnde (juce::int64 start, int samples, juce::int64& aus) noexcept
{
    if (samples < 0 || start > std::numeric_limits<juce::int64>::max() - (juce::int64) samples)
        return false;
    aus = start + (juce::int64) samples;
    return true;
}

bool projektAbstandGroesserAls64 (juce::int64 a, juce::int64 b) noexcept
{
    // Vorzeichenbit kippen bildet int64 streng monoton auf uint64 ab. Die
    // anschliessende Differenz ist auch zwischen INT64_MIN/MAX definiert.
    constexpr std::uint64_t bias = std::uint64_t { 1 } << 63u;
    const auto ua = static_cast<std::uint64_t> (a) ^ bias;
    const auto ub = static_cast<std::uint64_t> (b) ^ bias;
    return (ua >= ub ? ua - ub : ub - ua) > 64u;
}

std::string uuidHex32()
{
    std::string roh = juce::Uuid().toString().toStdString();
    std::string aus;
    aus.reserve (32);
    for (char c : roh)
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            aus.push_back (c);
    return aus.size() == 32 ? aus
                            : nakama::ipc::instanceAdresseAusState ("runtime:" + roh);
}

std::string alsHex32 (const juce::String& wert, const char* domain)
{
    std::string roh = wert.toStdString();
    std::string aus;
    aus.reserve (32);
    for (char c : roh)
        if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'))
            aus.push_back (c);
    return aus.size() == 32 ? aus
                            : nakama::ipc::instanceAdresseAusState (
                                  std::string (domain) + ":" + roh);
}

std::wstring brokerStartMutexName (const std::string& sid)
{
    std::wstring aus = L"Local\\NakamaBrokerStart.";
    aus.append (sid.begin(), sid.end());
    return aus;
}

nakama::ipc::ServerErwartung brokerServerErwartung()
{
    return { nakama::ipc::installbindung::brokerPfad,
             nakama::ipc::installbindung::brokerSha256,
             nakama::ipc::installbindung::authenticodeThumbprint };
}
} // namespace

EqCopilotProcessor::EqCopilotProcessor()
    : juce::AudioProcessor (BusesProperties()
          .withInput ("Eingang", juce::AudioChannelSet::stereo(), true)
          .withOutput ("Ausgang", juce::AudioChannelSet::stereo(), true)),
      v3LogonSid (nakama::ipc::aktuelleLogonSid()),
      v3PipeName (nakama::ipc::pipeNameV3 (v3LogonSid)),
      v3SessionEpoch (uuidHex32()),
      pipe ([this] {
                HelloInfo h;
                {
                    std::lock_guard<std::mutex> l (bindungMutex);
                    h.sensorId = zustand.common.instanceId;
                    h.role     = nakama::state::v2Rolle (zustand.common);
                    h.label    = zustand.common.label;
                    h.pairId   = zustand.common.pairId;
                }
                h.instanceNonce = instanceNonce;
                h.samplerate = samplerateAtomic.load();
                h.blockSize  = blockSizeAtomic.load();
                h.channels   = kanaeleAtomic.load();
                return h;
            },
            [this] { return statsSnapshot(); },
            [this] { return messKompakt(); }, {},
            std::chrono::milliseconds { 5000 }, brokerServerErwartung()),
      controlV3 ([this] { return v3Hello(); }, v3PipeName,
                 {},
                 [this] { return v3Status(); },
                  [this] (bool verbunden) { v3ControlLink (verbunden); },
                  [this] (const std::string& json, std::uint8_t schemaMinor)
                  { v3Antwort (json, schemaMinor); }, brokerServerErwartung()),
      telemetryV3 ([this] { return v3TelemetryHello(); }, v3PipeName,
                   [this] (const std::uint8_t* daten, std::size_t laenge,
                            std::uint8_t minor)
                   { v3Frame (daten, laenge, minor); }, brokerServerErwartung()),
      brokerLifecycle (nakama::ipc::BrokerLifecycleHooks {
          [this] {
              return controlV3.snapshot().status
                     == nakama::ipc::ControlClient::Status::verbunden;
          },
          [this] {
              const auto s = controlV3.snapshot();
              return s.status == nakama::ipc::ControlClient::Status::getrennt
                  && s.brokerPipeFehlt;
          },
          [this] { return controlV3.snapshot().serverPruefstatus; },
          [this] { return darfBrokerStarten(); },
          [this] { controlV3.reconnect(); },
          [] {
              return nakama::ipc::brokerBinaryPruefen (
                  nakama::ipc::installbindung::brokerPfad,
                  nakama::ipc::installbindung::brokerSha256,
                  nakama::ipc::installbindung::authenticodeThumbprint);
          },
          [] {
              return nakama::ipc::brokerVerborgenStarten (
                  nakama::ipc::installbindung::brokerPfad);
          },
          brokerStartMutexName (v3LogonSid),
          v3PipeName })
{
    // Frische Instanz (nie restauriert): legacy + insert, v2-Rolle "sensor".
    // Der Lebenslauf-Automat startet auf `unclassified` (§53.5) und bekommt
    // hier KEIN Ereignis - genau das ist der Grund, warum "ein Scannerlauf
    // klassifiziert nicht" keine Sonderbehandlung braucht: ein Scanner
    // instanziiert, fragt Busse und Parameter ab und zerstoert wieder. Er
    // ruft nie `setStateInformation` und oeffnet nie einen Editor.
    zustand = nakama::state::frisch (juce::Uuid().toString());
    instanceNonce = juce::Uuid().toString();
    // SONDE-008: der gesamte Backing-Store beider Ringe entsteht HIER - vor dem
    // Start des Workers und lange vor dem ersten Audioblock. `prepareToPlay`
    // fasst danach keinen Speicher mehr an, es meldet nur einen Neuanlauf.
    // 🔑 NAK-180 R7/R12: die drei Rueckwege des Sendepfads.
    //
    // Alle drei laufen unter `sendeMutex` des ControlClients. Sie duerfen den
    // Sendezustand nehmen (Ordnung: sendeMutex VOR sendeZustandMutex), aber
    // NIE erneut senden — das waere Rekursion auf derselben Sperre.
    controlV3.setzeP0Rueckmeldung (
        [this] (std::uint64_t marke, std::uint64_t generation)
        {
            // Wire-Commit: erst JETZT ist das Begin beim Broker. Die
            // Generation, auf der das geschah, entscheidet spaeter, ob ein
            // Replay noetig ist — ein Vergleich statt einer Umschreibung beim
            // Linkende (E10).
            //
            // 🔑 Nacharbeit 1: sie kommt vom EINTRAG, nicht aus
            // `wireGenerationJetzt()`. Ein Callback, der die aktuelle Zahl
            // liest, stempelt bei einem Aufbau zwischen Draht und Meldung G+1
            // auf ein Begin, das auf G zugestellt wurde - und der
            // Generationsvergleich unterdrueckte danach genau das Replay, das
            // er ausloesen soll.
            std::lock_guard<std::mutex> l (sendeZustandMutex);
            if (offenesBegin.gueltig && offenesBegin.marke == marke)
            {
                offenesBegin.zustand = 2;
                offenesBegin.zustellGeneration = generation;
            }
            // 🔑 EP-01/N-36: der Wire-Commit des `end` schliesst das Paar.
            // Vorher bleibt das Begin stehen, damit der Aufbauzug des naechsten
            // Links es findet und sein Replay voranstellt.
            if (abschlussBegin.gueltig && abschlussBegin.endeMarke == marke)
                abschlussBegin = AbschlussBegin {};
            // 🔑 EP-02/N-05: JETZT beginnt der Broker den Nachlauf zu zaehlen.
            // Vor dessen Ablauf verwirft er ein `false`, und niemand
            // wiederholt es.
            if (letzteEndeMarke != 0 && letzteEndeMarke == marke)
            {
                nachlaufFristSetzen (letzterEndeTail);
                letzteEndeMarke = 0;
            }
            // 🔑 EP-13/R7: Mitschnitt und Zaehler entstehen am WIRE-COMMIT,
            // nicht beim Einreihen. „Gesendet" heisst Draht.
            mitschnittZustellen (marke);
        },
        [this] (std::uint64_t marke)
        {
            // Verworfen: das Begin ist NICHT beim Broker und liegt auch nicht
            // mehr in der Queue. Es faellt auf "nicht eingereiht" zurueck,
            // und der naechste Zug replayt es (R8).
            std::lock_guard<std::mutex> l (sendeZustandMutex);
            if (offenesBegin.gueltig && offenesBegin.marke == marke)
            {
                offenesBegin.zustand = 0;
                offenesBegin.marke = 0;
            }
            mitschnittVerwerfen (marke);
        });
    controlV3.setzeReplayBeginHook (
        [this] (std::uint64_t generation, std::uint64_t marke) -> std::string
        {
            // 🔑 R12, Zustellpruefung. Der Aufbauzug hat ein EREIGNIS aelterer
            // Generation in der Queue gefunden; darunter kann ein `end` sein,
            // dessen Begin auf dem alten Link zugestellt wurde. Dann geht das
            // Replay-Begin voran — sonst traefe das `end` auf nichts.
            //
            // Dieser Hook LIEST und FORMT nur; er sendet nicht und wartet
            // nicht. Der Wiretext bleibt damit beim Prozessor, und der
            // Transport interpretiert nichts.
            std::lock_guard<std::mutex> l (sendeZustandMutex);
            // 🔑 EP-01/N-36: das Begin, dessen `end` bereits in der Queue
            // liegt, hat Vorrang - genau fuer diesen Fall gibt es die
            // Zustellpruefung. Erst danach das noch laufende offene Begin.
            const bool ausAbschluss = abschlussBegin.gueltig;
            if (! ausAbschluss && (! offenesBegin.gueltig || offenesBegin.zustand != 2))
                return {};
            auto h = v3Hello();
            h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
            if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))
                return {};
            const std::string adresseJson =
                std::string ("{\"logon_sid\":\"") + h.adresse.logonSid
                + "\",\"project_binding_id\":\"" + h.adresse.projectBindingId
                + "\",\"session_epoch\":\"" + h.adresse.sessionEpoch
                + "\",\"instance_id\":\"" + h.adresse.instanceId
                + "\",\"runtime_nonce\":\"" + h.adresse.runtimeNonce + "\"}";
            // Dieselbe Regel wie im Sendezug: ein Replay ist eine
            // Wiederholung und traegt die ORIGINALSEQUENZ. Eine frische Zahl
            // stuende ueber der des Backlogs und waere beim Broker eine Luecke.
            auto ev = ausAbschluss ? abschlussBegin.ereignis : offenesBegin.ereignis;
            ev.beginn = true;
            // 🔑 EP-09/N-27: das Replay ist EINGEREIHT, und der Zustellstand
            // sagt das auch. Mit Marke 0 blieb er auf „nicht eingereiht", der
            // geweckte Worker reihte dasselbe Begin ein zweites Mal ein, und
            // beim Broker stand die doppelte `intervention_id`. Fuer den
            // Abschlussfall gibt es kein offenes Begin mehr, dessen Stand zu
            // fuehren waere — dort traegt allein die Queue die Ordnung.
            if (! ausAbschluss)
            {
                offenesBegin.zustand = 1;
                offenesBegin.marke = marke;
            }
            // Ein noch offenes `replayFaellig` AELTERER Generation ist damit
            // erledigt; es wird nur aufgeraeumt (CAS auf den beobachteten
            // Wert), nie blind ueberschrieben (R13). Ein bereits gesetztes
            // G+1 bleibt stehen - der positive Callback laeuft erst nach
            // diesem Zug.
            auto gesehen = replayFaellig.load();
            if (gesehen != 0 && gesehen < generation)
                replayFaellig.compare_exchange_strong (gesehen, 0);
            // 🔑 EP-13/R7: auch DIESER Text ist eine gesendete Intervention.
            //
            // Der Client stellt ihn voran, nicht der Sendezug - er kaeme also
            // nie durch `sende()` und damit nie in die Buchfuehrung. Ein
            // Replay, das der Aufbauzug voranstellt, waere sonst unsichtbar:
            // der Zaehler zaehlte es nicht, und ein Bein saehe auf dem Draht
            // ein `end` ohne sein Begin, obwohl das Begin da war.
            auto json = interventionsWireJson (ev, adresseJson);
            if (marke != 0)
            {
                if (ausstehendeMitschnitte.size() >= kAusstehendDeckel)
                    ausstehendeMitschnitte.pop_front();
                ausstehendeMitschnitte.emplace_back (marke, json);
            }
            return json;
        });

    queue.vorbereiten();

    workerLaeuft.store (true);
    worker = std::thread ([this] { workerLauf(); });
    pipe.start();
#if ! defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    controlV3.start();
    telemetryV3.start();
    brokerLifecycle.start();
#endif
}

EqCopilotProcessor::~EqCopilotProcessor()
{
    brokerLifecycle.stop();
    telemetryV3.stop();
    controlV3.stop();
    pipe.stop();
    workerLaeuft.store (false);
    {
        std::lock_guard<std::mutex> l (workerWarteMutex);
        workerWarte.notify_all();
    }
    if (worker.joinable())
        worker.join();
}

//==============================================================================
// NAK-180 Nacharbeit 1: die Helfer des Sendezustands. Alle vier setzen
// `sendeZustandMutex` als GEHALTEN voraus (Sperrenordnung: `sendeMutex` des
// ControlClients zuerst, dann dieser).

/** EP-02/N-05: die Frist, vor deren Ablauf kein `false` reisen darf.

    Der Broker zaehlt `tail_samples_offen` ab dem Empfang des `end` in
    ECHTZEIT herunter. Ein `false`, das ihn waehrenddessen erreicht, wird
    verworfen — und weil `berichtOffen` und die Aufbauaussage mit ihm
    verbraucht sind, wiederholt es niemand. Die Frist beginnt am WIRE-COMMIT,
    weil der Broker vorher nichts gesehen hat, und traegt eine
    Heartbeat-Periode Marge: der Abschluss reist als naechster Heartbeat, und
    zwischen Frist und Draht liegt bis zu ein Takt. */
void EqCopilotProcessor::nachlaufFristSetzen (std::uint64_t tailSamples)
{
    double rate = letzteGueltigeSamplerate.load (std::memory_order_relaxed);
    if (! (std::isfinite (rate) && rate > 0.0))
        rate = 48000.0;                      // dieselbe Vorgabe wie im Broker
    const double sekunden = (double) tailSamples / rate;
    // Saettigen statt ueberlaufen: `tailSamples` kann bis an den u64-Rand
    // gehen (N-18), und eine Frist von 10^12 Sekunden waere eine Sperre ohne
    // Ende. Eine Stunde ist weit jenseits jedes echten Nachlaufs und bleibt
    // in `int64`-Nanosekunden rechenbar.
    constexpr double kDeckelSekunden = 3600.0;
    const double gedeckelt = std::isfinite (sekunden)
        ? std::min (std::max (sekunden, 0.0), kDeckelSekunden) : 0.0;
    const auto jetzt = std::chrono::steady_clock::now().time_since_epoch();
    const auto jetztNs =
        std::chrono::duration_cast<std::chrono::nanoseconds> (jetzt).count();
    const std::int64_t tailNs = (std::int64_t) (gedeckelt * 1e9);
    const std::int64_t margeNs =
        (std::int64_t) nakama::ipc::kHeartbeatTaktMs * 1000000LL;
    nachlaufFristNs.store (jetztNs + tailNs + margeNs, std::memory_order_relaxed);
}

bool EqCopilotProcessor::nachlaufAbgelaufen() const
{
    const auto frist = nachlaufFristNs.load (std::memory_order_relaxed);
    if (frist == 0)
        return true;
    const auto jetzt = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::nanoseconds> (jetzt).count()
           >= frist;
}

/** EP-13/R7: „gesendet" heisst Wire-Commit.

    Mitschnitt und Zaehler entstanden bisher unmittelbar nach dem Enqueue. Die
    Passage-Tests ernteten ihn dann meist ohne `zustelleAllesFuerTest` und
    blieben gruen, obwohl ein Write scheiterte oder der Aufbaufilter den
    Eintrag verwarf — sie massen das Einreihen und nannten es Senden. */
void EqCopilotProcessor::mitschnittZustellen (std::uint64_t marke)
{
    if (marke == 0)
        return;
    for (auto it = ausstehendeMitschnitte.begin();
         it != ausstehendeMitschnitte.end(); ++it)
    {
        if (it->first != marke)
            continue;
        if (gesendeteInterventionen.size() >= kMitschnittDeckel)
            gesendeteInterventionen.pop_front();
        gesendeteInterventionen.push_back (std::move (it->second));
        ausstehendeMitschnitte.erase (it);
        interventionenGesendet.fetch_add (1, std::memory_order_relaxed);
        return;
    }
}

void EqCopilotProcessor::mitschnittVerwerfen (std::uint64_t marke)
{
    if (marke == 0)
        return;
    for (auto it = ausstehendeMitschnitte.begin();
         it != ausstehendeMitschnitte.end(); ++it)
    {
        if (it->first == marke)
        {
            ausstehendeMitschnitte.erase (it);
            return;
        }
    }
}

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

void EqCopilotProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals rein;

    const int kanaele = juce::jmin (buffer.getNumChannels(), 2);
    const int n = buffer.getNumSamples();
    if (n == 0 || kanaele == 0)
        return;

    // Der Puffer bleibt bis zum Blockende unangetastet; erst die Hör-Markierung
    // am Ende darf färben — NACH RMS-Scan und Analyse-Abgriff, damit Messung
    // und Meter immer den echten Mix sehen (Konzept v2 §5). Ohne aktive,
    // erlaubte Markierung ist Passthrough weiterhin das Nichtstun.

    bool nan = false;
    for (int k = 0; k < kanaele; ++k)
    {
        const float* d = buffer.getReadPointer (k);
        float summe = 0.0f;
        for (int i = 0; i < n; ++i)
        {
            const float v = d[i];
            if (! std::isfinite (v))
                nan = true;
            summe += v * v;
        }
        const float rms = std::sqrt (summe / (float) n);
        (k == 0 ? rmsL : rmsR).store (std::isfinite (rms) ? rms : 0.0f);
    }
    if (kanaele == 1)
        rmsR.store (rmsL.load());
    if (nan)
        nanSeen.store (true);

    // ── Zeitstempel: woher die Zeit kommt, und wie sicher (SONDE-008) ──────
    // Zwei Quellen, in dieser Reihenfolge, und die Reihenfolge ist der Punkt:
    //  1. die HOSTBRÜCKE (SONDE-003). Nur sie kann „Context fehlt" überhaupt
    //     ausdrücken — JUCEs VST3-Playhead liefert nie `nullopt`, weil der
    //     Wrapper seinen internen Context nullt und daraus ein gefülltes
    //     PositionInfo baut (NakamaHostBridge.h, Kopf; NAK-24).
    //  2. der Playhead als Rückfallweg für Ziele OHNE gepatchten Wrapper
    //     (Konsolentests, andere Formate). Dort ist „PositionInfo da" ehrlich
    //     das Beste, was zu wissen ist — und ohne Playhead ist Transport
    //     ausdrücklich UNBEKANNT, nicht „gestoppt" und nicht „läuft".
    nakama::echtzeit::Stempel stempel;
    if (brueckeStand.frisch)
    {
        // Der Stand IST der Stempel (SONDE-009) — nichts wird umkopiert, also
        // kann auch nichts beim Umkopieren vergessen werden.
        stempel = brueckeStand.stempel;
        brueckeStand.frisch = false;
    }
    else if (auto* kopf = getPlayHead())
    {
        if (const auto pos = kopf->getPosition())
        {
            stempel.kontextAnwesend = true;
            stempel.spieltGueltig   = true;
            stempel.spielt          = pos->getIsPlaying();
            if (const auto zeit = pos->getTimeInSamples())
            {
                stempel.zeitGueltig        = true;
                stempel.projectSampleStart = *zeit;
            }
            // SONDE-009: was der öffentliche Playhead über den Rest sagen kann.
            // Weniger als die Brücke, und das ist ehrlich so: JUCE hat kein
            // Gültigkeitsbit für `isRecording`, weshalb hier bewusst KEINS
            // gesetzt wird — „PositionInfo da" beweist Transport, nicht
            // Aufnahmezustand. Was die Brücke unterscheiden kann, unterscheidet
            // nur die Brücke.
            if (const auto ppq = pos->getPpqPosition())
            {
                stempel.ppqGueltig  = true;
                stempel.ppqPosition = *ppq;
            }
            if (const auto bpm = pos->getBpm())
            {
                stempel.tempoGueltig = true;
                stempel.tempo        = *bpm;
            }
            stempel.cycleAktiv = pos->getIsLooping();
            if (const auto schleife = pos->getLoopPoints())
            {
                stempel.cycleGrenzenGueltig = true;
                stempel.cycleStartPpq = schleife->ppqStart;
                stempel.cycleEndePpq  = schleife->ppqEnd;
            }
        }
    }
    // `isNonRealtime()` fragt den PROZESSOR, nicht den Hostkontext — es steht
    // deshalb hinter beiden Zweigen und überschreibt keinen Brückenwert.
    stempel.nichtEchtzeit = isNonRealtime();
    // Die Samplerate ist im Playhead-Zweig nicht erfragbar; die des Prozessors
    // ist der einzige Wert, den wir hier ehrlich behaupten können, und er
    // stammt aus `prepareToPlay`.
    if (! stempel.sampleRateGueltig)
    {
        const double fs = samplerateAtomic.load();
        if (fs > 0.0)
        {
            stempel.sampleRateGueltig = true;
            stempel.sampleRate        = fs;
        }
    }

    // Nach außen sichtbarer Transportstand (Editor, Heartbeat). `hatTransport`
    // heißt ab hier „Transport ist BEKANNT" statt „irgendein PositionInfo kam" —
    // mit der Brücke ist das erstmals unterscheidbar (NAK-24).
    hatTransport.store (stempel.spieltGueltig);
    transportSpielt.store (stempel.spieltGueltig && stempel.spielt);
    aufnahmeGueltig.store (stempel.recordingGueltig);
    aufnahmeAktiv.store (stempel.recordingGueltig && stempel.recording);

    // Hostzeit (M0-Prüfpunkt §9.3) — Projektzeit-Fenster der Messung (Plan
    // §5.7): nur während Play akkumulieren, der stehende Playhead ist kein
    // Fenster.
    // NAK-24 zweite Hälfte: die Projektzeit trägt ihr eigenes Gültigkeitsbit.
    // Bis 23.08. blieb `projektZeitSamples` stehen, wenn der Context wegfiel —
    // ein alter Wert sah aus wie eine aktuelle Position.
    projektZeitGueltig.store (stempel.zeitGueltig);
    if (stempel.zeitGueltig)
        projektZeitSamples.store (stempel.projectSampleStart);

    // Analyseweg: der GANZE Block in die zeitgestempelte Queue — oder gar
    // nicht. Der Out-Parameter ist die einzige Wahrheit fuer die tatsaechlich
    // im Produzenten uebernommene Generation. Ein Resetwunsch kann waehrend
    // dieses Callbacks eintreffen; ein separates, frueher gelesenes Atomic
    // koennte dann einen neuen Block noch ins alte Projektfenster schreiben.
    Queue::TapQuelle abgriff;
    abgriff.links  = buffer.getReadPointer (0);
    abgriff.rechts = kanaele > 1 ? buffer.getReadPointer (1) : nullptr;
    bool neustartUebernommen = false;
    bool lueckeVorBlock = false;
    const bool queueAngenommen = queue.veroeffentliche (
        &abgriff, 1, kanaele, n, stempel, &neustartUebernommen, &lueckeVorBlock);

    // Nur der Audiothread schreibt das Projektfenster. Generation und
    // Kontinuitaetskante kommen direkt aus demselben Queue-Zug, der diesen
    // Block gestempelt hat: kein Lock, keine Allokation und keine
    // atomuebergreifende Sichtbarkeitsannahme. Ein abgewiesener Block gehoert
    // nicht zur M1-Messung; erst der naechste angenommene Block beginnt mit der
    // dort wirklich veroeffentlichten Luecke ein neues Projektfenster.
    if (neustartUebernommen || lueckeVorBlock)
    {
        fensterAktiv.store (false);
        fensterSpruenge.store (0);
        fensterErwartetGueltig = false;
    }

    const bool analyseRateGueltig = samplerateAtomic.load() > 0.0;
    if (queueAngenommen && analyseRateGueltig
        && stempel.zeitGueltig && stempel.spieltGueltig && stempel.spielt)
    {
        const juce::int64 t = stempel.projectSampleStart;
        juce::int64 ende = 0;
        if (! projektEnde (t, n, ende))
        {
            // Ein nicht darstellbares Hostintervall ist keine echte
            // Projektposition. Vorheriges Fenster nicht damit vermischen.
            fensterAktiv.store (false);
            fensterErwartetGueltig = false;
            fensterSpruenge.fetch_add (1);
        }
        else if (! fensterAktiv.load())
        {
            // `fensterAktiv` ist zugleich das Publikationsbit fuer Leser:
            // Grenzen zuerst schreiben, sonst koennte ein Heartbeat die neue
            // Generation schon als gueltig mit den alten Grenzen beobachten.
            fensterVon.store (t);
            fensterBis.store (ende);
            fensterErwartet = ende;
            fensterErwartetGueltig = true;
            fensterAktiv.store (true);
        }
        else
        {
            // Sprung = Loop/Seek/Stop-Rücksprung. Toleranz 64 Samples für
            // Rundungen des Hosts. Auch über eine Pause hinweg gemessen:
            // Resume an anderer Stelle IST eine Lücke im Fenster.
            if (fensterErwartetGueltig && projektAbstandGroesserAls64 (t, fensterErwartet))
                fensterSpruenge.fetch_add (1);
            if (t < fensterVon.load())
                fensterVon.store (t);
            if (ende > fensterBis.load())
                fensterBis.store (ende);
            fensterErwartet = ende;
            fensterErwartetGueltig = true;
        }
    }

    // ── Hör-Markierung (Konzept v2): Erlaubnis prüfen, dann färben ─────────
    // Reihenfolge ist Vertrag: RMS + Analyse-Abgriff liegen OBEN — die Messung
    // sieht nie das gefärbte Signal (Beweis: Markierungstest T4).
    const bool spielt = stempel.spieltGueltig && stempel.spielt;
    lebenszeichen (n, spielt);
    // §53.5 Satz 1 (S9/SONDE-007b Abschnitt 3): bis zur positiven
    // Klassifikation ist der Entry AUDIO-NEUTRAL. Die Hoer-Markierung ist die
    // einzige Audio-Ausnahme des Grundgesetzes und faellt damit unter diesen
    // Satz; `legacy` bleibt zu, weil §53.5 es "immer passiv" nennt. Gelesen
    // wird die Atomic-Spiegelung, nie der Automat selbst - "Klassifikation,
    // Spawn und Pipe-I/O liegen nie im Audiocallback".
    // Der Term steht ZUERST, damit beim Lesen sofort klar ist: ohne Main
    // faerbt hier nichts, egal was die uebrigen Bedingungen sagen.
    //
    // ⚠️ TRANSPORT-TERM, geändert mit SONDE-008 (User-Entscheid 22.08., Hub
    // `U10`: „Nein, nur mit Signal"). Bis 23.08. stand hier
    // `(spielt ∨ ¬hatTransport)` — ein fail-open: wo kein Transport gemeldet
    // wurde, färbte die Markierung. Verlangt ist ein GÜLTIGES „spielt".
    // Der Term war nicht früher zu schließen, weil „Transport unbekannt" bis
    // zur Verdrahtung der Hostbrücke gar nicht ausdrückbar war (Entwurf §0.1:
    // „Der eigentliche Mangel ist, dass `hatTransport` ‚Transport unbekannt'
    // gar nicht ausdrücken kann"); genau diese Verdrahtung bringt SONDE-008
    // für den Zeitstempel mit. NAK-35/NAK-24.
    // In FL ändert sich dadurch nichts: dort lag `hatTransport` ab dem ersten
    // Block auf true, der fail-open-Zweig war ein toter Zweig (Prüfbericht 1.2).
    // Ohne Playhead und ohne Brücke — also headless — färbt jetzt nichts mehr.
    //
    // `testForciereEchtzeit` umgeht diesen Term ABSICHTLICH NICHT: der Schalter
    // umgeht, was an der Wanduhr hängt (Lebenszeichen, Editor). Transport hängt
    // an nichts dergleichen; ein Test, der ihn mit umginge, prüfte einen Pfad,
    // den das Produkt nicht hat (dieselbe Begründung wie beim §53.5-Term).
    // SONDE-013 M-33: der vierte Term. §58 verlangt fail-closed
    // `playing=true`, `recording=false`, Realtime und Editor offen — der
    // Aufnahmezustand FEHLTE hier, obwohl er im Prozessor vorliegt und im
    // `state_report` bereits reist.
    //
    // ⚠️ Ein UNBEKANNTER Aufnahmezustand blockiert wie ein aktiver. Das ist
    // der Unterschied zwischen fail-closed und fail-open: ohne
    // `aufnahmeGueltig` weiss niemand, ob gerade aufgenommen wird, und eine
    // Faerbung, die in eine Aufnahme laeuft, steht danach in der Datei.
    //
    // ⚠️ `testForciereEchtzeit` umgeht diesen Term ABSICHTLICH NICHT —
    // dieselbe Begruendung wie beim Transportterm daneben: der Schalter
    // umgeht, was an der Wanduhr haengt (Lebenszeichen, Editor). Der
    // Aufnahmezustand haengt an nichts dergleichen, er kommt aus der
    // Hostbruecke. Waere er mit umgangen, pruefte der Test einen Pfad, den
    // das Produkt nicht hat.
    const bool aufnahmeAus = aufnahmeGueltig.load (std::memory_order_relaxed)
                          && ! aufnahmeAktiv.load (std::memory_order_relaxed);
    const bool erlaubt = istMainKlassifiziert.load (std::memory_order_relaxed)
                      && (echtzeitOk.load (std::memory_order_relaxed)
                          || testEchtzeit.load (std::memory_order_relaxed))
                      && spielt
                      && aufnahmeAus
                      && ! isNonRealtime()
                      && (editorOffen.load (std::memory_order_relaxed)
                          || testEchtzeit.load (std::memory_order_relaxed));
    // 🔑 SONDE-013 Nacharbeit 2 (Befund R06, M-20/Paragraph 38.3): der
    // VERGLEICHSPEGEL wird gespeist — mit den beiden Signalen, die der User im
    // A/B wirklich gegeneinander hoert: dem ungefaerbten Monitorsignal und
    // demselben Signal nach der Hoermarkierung.
    //
    // Er laeuft NUR, solange ein Versuch vorbereitet wird; ausserhalb kostet
    // die Zeile nichts. Die Trockenkopie ist in `prepareToPlay` vorallokiert —
    // im Audiothread wird nie vergroessert, nie gesperrt und nie geloggt.
    //
    // 🔑 Nacharbeit 3 (Befund C4, M-20/Paragraph 38.3 woertlich: „fuer die
    // GEWAEHLTE PASSAGE vorab gemessen"): der Pegel nimmt nur Material aus dem
    // Fenster der gebundenen Passage.
    //
    // Bis zur Runde 2 war `versuchspegelSpeist` das einzige Tor. Wer nach dem
    // Markieren des Refrains an anderer Stelle abspielte, fuellte damit die
    // 400-ms-Schwelle mit FREMDEM Material, und dessen Verhaeltnis reiste als
    // `match_gain_db` des Refrains. Der Ausschnitt wird deshalb aus der
    // Projektzeit des Blocks und den vom Analyseworker VEROEFFENTLICHTEN
    // Grenzen gerechnet — samplegenau, damit auch die Passagengrenze mitten im
    // Block richtig faellt.
    //
    // `pegelFensterAktiv` traegt die Epochenbindung mit: der Worker setzt es
    // nur, solange `passagenfensterIntakt()` gilt, und eine Transportgrenze im
    // Fenster loescht es. Der Audiothread muss die Epoche damit nicht kennen —
    // er koennte sie auch nicht lesen, ohne die Engine anzufassen.
    int pegelVon = 0, pegelBis = 0;
    if (versuchspegelSpeist.load (std::memory_order_relaxed)
        && pegelFensterAktiv.load (std::memory_order_acquire)
        && spielt
        && stempel.zeitGueltig
        && kanaele > 0
        && (std::size_t) (buffer.getNumSamples() * kanaele) <= versuchTrocken.size())
    {
        // Selbstaudit der Runde 3: ein NEGATIVER Blockanfang ist keine Stelle
        // in der Passage — ihre Grenzen sind nach `merkeManuellePassage` beide
        // >= 0. Er wird deshalb ausgeschlossen, BEVOR gerechnet wird: die
        // Differenz `ende - blockAnfang` liefe sonst am i64-Rand ueber, und
        // ein Ueberlauf ergaebe ein Fenster, das es nicht gibt (M-17).
        const std::int64_t blockAnfang = stempel.projectSampleStart;
        std::int64_t von = 0, bis = 0;
        if (blockAnfang >= 0)
        {
            von = pegelFensterStart.load (std::memory_order_relaxed) - blockAnfang;
            bis = pegelFensterEnde.load (std::memory_order_relaxed) - blockAnfang;
        }
        else
        {
            von = 1;
            bis = 0;                     // leerer Ausschnitt: nichts wird aufgenommen
        }
        if (von < 0) von = 0;
        if (bis > (std::int64_t) buffer.getNumSamples()) bis = buffer.getNumSamples();
        if (von < bis)
        {
            pegelVon = (int) von;
            pegelBis = (int) bis;
        }
    }
    const bool pegelSpeist = pegelBis > pegelVon;
    if (pegelSpeist)
        for (int c = 0; c < kanaele; ++c)
            std::memcpy (versuchTrocken.data() + (std::size_t) c * (std::size_t) buffer.getNumSamples(),
                         buffer.getReadPointer (c),
                         (std::size_t) buffer.getNumSamples() * sizeof (float));

    const auto schritt = markierung.verarbeite (buffer, kanaele, erlaubt);

    if (pegelSpeist)
        for (int c = 0; c < kanaele; ++c)
            vergleichspegel.speise (
                versuchTrocken.data() + (std::size_t) c * (std::size_t) buffer.getNumSamples()
                    + (std::size_t) pegelVon,
                buffer.getReadPointer (c) + pegelVon,
                pegelBis - pegelVon);

    // SONDE-013 M-37/M-38: die zwei Uebergaenge gehen SOFORT in den
    // vorallokierten RT→Control-Ring. Der Audiothread beruehrt die Pipe nie —
    // er reiht nur ein; der Worker sendet.
    //
    // Der Rueckgabewert von `schreibe()` wird ausgewertet und NICHT
    // verworfen: bei Ueberlauf steht das Sticky-Bit, und der Worker meldet
    // es als `intervention_state_unknown`. Ein verlorenes Begin darf niemals
    // eine scheinbar saubere Baseline erzeugen (§34.2).
    if (schritt.begann || schritt.endete)
    {
        nakama::ipc::Interventionsereignis e;
        e.beginn = schritt.begann;
        // 🔑 Nacharbeit 3 (Befund C1, M-61): die erste Sequenz nach einem
        // bestaetigten Resync ist EINS, nicht null.
        //
        // Der Broker setzt mit `resync_bestaetigen(link, 0)` seine Basis auf 0
        // und liest das als „die naechste ist 1". Das Plugin vergab bis dahin
        // `fetch_add` OHNE Inkrement, sendete also 0 — der Broker verwarf die
        // erste Intervention jeder Verbindung als Luecke und setzte
        // `taint.unknown` sofort wieder. Der R01-Fix hob sich damit selbst auf.
        // Das Inkrement steht VOR dem Senden; die Zahl auf der Leitung ist
        // damit die des Ereignisses und nicht die davor.
        e.sequenz = interventionsSequenz.fetch_add (1, std::memory_order_relaxed) + 1;
        // Beim Beginn eine neue Eingriffsnummer ziehen, beim Ende die des
        // laufenden Eingriffs behalten.
        if (schritt.begann)
            e.nummer = interventionsNummer.fetch_add (1, std::memory_order_relaxed);
        else
            e.nummer = interventionsNummer.load (std::memory_order_relaxed) - 1;
        e.projektzeitGesetzt = projektZeitGueltig.load (std::memory_order_relaxed);
        // 🔑 SONDE-013 M-38/M-52: `projektZeitSamples` ist der BLOCKANFANG.
        // Fuer das Ende ist das die falsche Zahl: der Ausfade endet irgendwo
        // IM Block, und ein `project_sample_end` am Blockanfang liesse den
        // gesamten Ausfade — bis zu einen vollen Hostblock gefaerbtes Audio —
        // ausserhalb der Invalidierung. Die Markierung liefert deshalb den
        // Offset des letzten gefaerbten Samples, und er wird hier addiert.
        const std::int64_t blockAnfang =
            (std::int64_t) projektZeitSamples.load (std::memory_order_relaxed);
        e.projektSample = blockAnfang;
        if (schritt.endete)
        {
            // M-17: der i64-Rand wird gesaettigt, nicht ueberlaufen. Ein
            // Ueberlauf ergaebe eine Projektzeit VOR dem Blockanfang, und die
            // Invalidierung nutzte dann einen Bereich, den es nicht gibt.
            const std::int64_t offset = (std::int64_t) schritt.endeOffsetSamples;
            e.projektSample = blockAnfang > std::numeric_limits<std::int64_t>::max() - offset
                                ? std::numeric_limits<std::int64_t>::max()
                                : blockAnfang + offset;
            e.dauerSamples = schritt.dauerSamples;
            // Konservativ (§34.2): der Bereich wird LAENGER quarantaenisiert
            // als der Eingriff dauerte. Der Faktor ist doppelt plus ein
            // festes Polster — der Filternachklang der Markierung ist
            // biquadratisch und damit theoretisch unendlich, praktisch nach
            // wenigen Millisekunden unter dem Rauschen. Zu kurz waere hier
            // der teure Fehler, zu lang nur eine verzoegerte Freigabe.
            // 🔑 NAK-180 R5: die Rate kommt aus `letzteGueltigeSamplerate`,
            // nicht aus JUCEs ungeprueftem `getSampleRate()`. Eine
            // nicht-endliche oder ueber INT_MAX liegende Hostrate machte
            // `(int) getSampleRate()` zu UB; auf MSVC/x64 liefert `cvttsd2si`
            // dann INT_MIN, `INT_MIN/10` ist negativ, und `std::max(1, ...)`
            // ergab genau EIN Sample Nachlauf statt rund 100 ms — der Fall,
            // den der Kommentar oben ausschliesst. Die Rechnung saettigt an
            // beiden Raendern: das Polster ist gedeckelt, und die Verdopplung
            // laeuft nicht ueber.
            e.tailSamples = nakama::ipc::tailSamplesFuer (
                schritt.dauerSamples,
                letzteGueltigeSamplerate.load (std::memory_order_relaxed));
        }
        if (! interventionsRing.schreibe (e))
            interventionsRingUeberlauf.store (true, std::memory_order_relaxed);
    }
}

//==============================================================================
// SONDE-008: die Gegenseite der Hostbrücke. Läuft auf DEMSELBEN Thread wie
// `processBlock` und unmittelbar davor — der gepatchte Wrapper ruft
// `uebergib()` zwischen `kontextAus()` und dem Prozessoraufruf.
//
// ⚠️ Der Befund ist NICHT 1:1 mit `processBlock` gepaart (NakamaHostBridge.h,
// `uebergib()`): ein Parameter-Flush (`blockGroesse == 0`) und der
// Wavelab-Riegel liefern einen Befund, ohne dass danach ein Block verarbeitet
// wird. Deshalb ein `frisch`-Bit statt einer Annahme: `processBlock`
// VERBRAUCHT es, und ein Befund ohne Folgeblock wird schlicht vom nächsten
// überschrieben.
void EqCopilotProcessor::nakamaBlockEmpfangen (const eqcop::hostbruecke::Blockbefund& befund) noexcept
{
    const auto& k = befund.kontext;
    auto& s = brueckeStand.stempel;
    s = nakama::echtzeit::Stempel {};     // kein Feld erbt den Vorblock

    s.kontextAnwesend = k.processContextPresent;
    // `projectTimeSamples` ist laut VST3-Doku gültig, SOBALD ein Context
    // existiert — die Brücke setzt das Gültigkeitsbit deshalb genau dann.
    s.zeitGueltig        = k.processContextPresent && k.projectTimeSamples.gueltig;
    s.projectSampleStart = (std::int64_t) k.projectTimeSamples.oder (0);
    s.spieltGueltig      = k.processContextPresent && k.playing.gueltig;
    s.spielt             = k.playing.oder (false);

    // ── SONDE-009: der Rest des Transportstempels (§32.3) ──────────────────
    // Jedes Feld hängt an `processContextPresent` UND an seinem eigenen Bit.
    // Der Kontext ist die Voraussetzung dafür, dass die Zahl überhaupt etwas
    // bedeutet; das Einzelbit sagt, ob der Host sie in DIESEM Block gemeldet
    // hat. Beide Fragen sind verschieden, deshalb beide Konjunktionen.
    s.recordingGueltig      = k.processContextPresent && k.recording.gueltig;
    s.recording             = k.recording.oder (false);
    s.continuousGueltig     = k.processContextPresent && k.continuousTimeSamples.gueltig;
    s.continuousTimeSamples = (std::int64_t) k.continuousTimeSamples.oder (0);
    s.tempoGueltig          = k.processContextPresent && k.tempo.gueltig;
    s.tempo                 = k.tempo.oder (0.0);
    s.ppqGueltig            = k.processContextPresent && k.ppqPosition.gueltig;
    s.ppqPosition           = k.ppqPosition.oder (0.0);
    s.sampleRateGueltig     = k.processContextPresent && k.sampleRate.gueltig;
    s.sampleRate            = k.sampleRate.oder (0.0);

    // Schleife: `aktiv` und `gueltig` sind ZWEI Aussagen. VST3 kann melden
    // „die Schleife läuft" und trotzdem keine brauchbaren Grenzen liefern
    // (`kCycleValid` fehlt) — dann ist der Loop-Wrap bekannt, seine Lage aber
    // nicht. §32.3 nennt genau diesen Fall: „Liegen nur PPQ-Bounds vor oder
    // fehlen die Bounds, wird der mögliche Straddle als ungültig markiert."
    s.cycleAktiv          = k.processContextPresent && k.cycle.aktiv;
    s.cycleGrenzenGueltig = k.processContextPresent && k.cycle.gueltig;
    s.cycleStartPpq       = k.cycle.startPpq;
    s.cycleEndePpq        = k.cycle.endePpq;

    // Presentation-Latency: Bus 0 je Richtung — der Main-Bus. Die weiteren
    // Busse gehören zu Probeeqs Aux-Wegen (P6) und haben heute keinen
    // Verbraucher. `gemeldet` trennt „der Host hat 0 gesagt" von „der Host hat
    // nie etwas gesagt"; ein Latenzwert 0 kann beides heißen (§32.3).
    const auto& ein = k.presentationLatency.hole (true, 0);
    const auto& aus = k.presentationLatency.hole (false, 0);
    s.eingangLatenzGemeldet = ein.gemeldet;
    s.eingangLatenzSamples  = ein.samples;
    s.ausgangLatenzGemeldet = aus.gemeldet;
    s.ausgangLatenzSamples  = aus.samples;

    brueckeStand.frisch = true;
}

// „Neutral, bis Echtzeit bewiesen" (Konzept v2 §4): zwei Fenster mit
// Audiozeit/Echtzeit ≈ 1 schalten frei; Transportkante oder Verarbeitungs-
// lücke setzt den Beweis zurück; Freilauf (Verhältnis > 1,5) löscht ihn und
// meldet den Latch-Fall an den Editor. Ein Offline-Render besteht den Beweis
// nie — Fenster schließen dort nach Audiozeit, nicht nach Wanduhr.
void EqCopilotProcessor::lebenszeichen (int samples, bool spielt)
{
    using namespace std::chrono;
    const juce::int64 jetzt =
        duration_cast<nanoseconds> (steady_clock::now().time_since_epoch()).count();
    const double fs = samplerateAtomic.load();
    if (fs <= 0.0)
        return;

    bool zuruecksetzen = false;
    if (lzLetzterNs > 0 && jetzt - lzLetzterNs > 250'000'000LL)
        zuruecksetzen = true;                    // Verarbeitungslücke (Suspend/Bypass)
    if (spielt != lzPrevSpielt)
    {
        zuruecksetzen = true;                    // Transportkante (Start UND Stopp)
        lzPrevSpielt = spielt;
    }
    lzLetzterNs = jetzt;
    if (zuruecksetzen)
    {
        echtzeitOk.store (false);
        lzBestanden = 0;
        lzBucketStartNs = jetzt;
        lzBucketSamples = 0;
        return;
    }
    if (lzBucketStartNs == 0)
    {
        lzBucketStartNs = jetzt;
        lzBucketSamples = 0;
    }
    lzBucketSamples += samples;

    const double wandS  = (double) (jetzt - lzBucketStartNs) * 1.0e-9;
    const double audioS = (double) lzBucketSamples / fs;
    // Beweisen langsam (500 ms), kippen schnell (250 ms Audiozeit): im
    // Freilauf schließt das Fenster nach Audiozeit fast sofort.
    const bool fensterZu = wandS >= 0.5
                        || audioS >= (echtzeitOk.load() ? 0.25 : 0.5);
    if (! fensterZu)
        return;

    const double verhaeltnis = audioS / std::max (wandS, 1.0e-6);
    if (verhaeltnis >= 0.5 && verhaeltnis <= 1.2)
    {
        if (++lzBestanden >= 2)
            echtzeitOk.store (true);
    }
    else
    {
        lzBestanden = 0;
        if (verhaeltnis > 1.5)
        {
            echtzeitOk.store (false);
            if (markierung.zielGesetzt())
                freilaufKill.store (true);       // Editor löst den Latch sichtbar
        }
        // 1,2 < Verhältnis ≤ 1,5: Host-Burst — Beweis bleibt, zählt nur nicht.
    }
    lzBucketStartNs = jetzt;
    lzBucketSamples = 0;
}

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
std::string EqCopilotProcessor::interventionsWireJson (
    const nakama::ipc::Interventionsereignis& e, const std::string& adresseJson) const
{
    // 🔑 Nacharbeit 3 (Befund C1): der Wiretext entsteht getrennt vom Senden.
    //
    // Bis dahin lag beides in einer Schleife, und die Zahl, die WIRKLICH auf
    // die Leitung geht, war ausserhalb des Prozessors nicht messbar. Ein Bein
    // konnte den Handschlag deshalb nur an lokalen Flags pruefen — genau die
    // Sorte Test, die an einem Vertragsbruch zwischen zwei Sprachen nicht
    // fallen kann.
    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    std::string json = "{\"type\":\"";
    json += e.beginn ? "audible_intervention_begin" : "audible_intervention_end";
    json += "\",\"intervention_id\":\"";
    json += nakama::ipc::instanceAdresseAusState (
                "intervention:" + h.adresse.instanceId + ":"
                + std::to_string (e.nummer));
    json += "\",\"adresse\":";
    json += adresseJson;
    json += ",\"event_sequence\":";
    json += std::to_string (e.sequenz);
    if (e.beginn)
    {
        // SONDE-013 §7.1 E-08: der Hoermarker bleibt in Gen/Main, und
        // dieses Ticket baut nur ihn. Die drei uebrigen Arten aus dem
        // Schema bekommen ihre Erzeuger in P6 und P7.
        json += ",\"art\":\"hoermarkierung\"";
        json += ",\"project_sample_start\":";
        json += e.projektzeitGesetzt ? std::to_string (e.projektSample)
                                     : std::string ("null");
    }
    else
    {
        json += ",\"project_sample_end\":";
        json += e.projektzeitGesetzt ? std::to_string (e.projektSample)
                                     : std::string ("null");
        json += ",\"tail_samples\":";
        json += std::to_string (e.tailSamples);
    }
    json += "}";
    return json;
}

void EqCopilotProcessor::interventionenSenden()
{
    // 🔑 NAK-180 R8: der Ring ist nicht mehr die einzige Quelle.
    //
    // Ein Replay steht auch dann an, wenn der Ring LEER ist — genau dann
    // naemlich, wenn ein Begin bereits gesendet wurde und sein Marker
    // weiterklingt (C2). Die alte Abbruchbedingung liess diesen Zug nie
    // laufen.
    const bool etwasZuTun = interventionsRing.fuellstand() != 0
                         || replayFaellig.load() != 0
                         || berichtOffen.load() != 0;
    if (! etwasZuTun)
        return;
    // 🔑 NAK-40: DIESELBE Wireadresse wie der Control-Bootstrap. `v3Hello()`
    // liefert die PERSISTENTE Instance-ID; der Alias entsteht erst an der
    // v3-Grenze. Ohne ihn pruefte dieser Pfad eine Adresse, die so nie auf
    // der Leitung steht — bei einer hex32-ID faellt das nicht auf, bei einer
    // unterstuetzten Legacy-ID schlug `adresseGueltig` fehl und der Ring
    // wurde kommentarlos GELEERT: der Marker verschwand.
    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))
    {
        // Ohne gueltige Adresse kann kein Ereignis reisen. Es wird trotzdem
        // ENTNOMMEN: ein Ring, der sich bei fehlender Adresse fuellt, liefe
        // ueber und meldete einen Ueberlauf, der keiner ist.
        nakama::ipc::Interventionsereignis weg;
        while (interventionsRing.lies (weg)) {}
        return;
    }

    const auto adresseJson = [&h]
    {
        std::string a = "{\"logon_sid\":\"" + h.adresse.logonSid
                      + "\",\"project_binding_id\":\"" + h.adresse.projectBindingId
                      + "\",\"session_epoch\":\"" + h.adresse.sessionEpoch
                      + "\",\"instance_id\":\"" + h.adresse.instanceId
                      + "\",\"runtime_nonce\":\"" + h.adresse.runtimeNonce + "\"}";
        return a;
    }();

    // 🔑 NAK-180 R12: EIN Zug unter `sendeMutex`. Der Vergleich der
    // Zustellgeneration und das Einreihen der Folgenachricht liegen damit in
    // derselben kritischen Zone — zwischen ihnen kann kein Link aufgebaut
    // werden. Vorher entschied die Ordnung beim Einreihen, ihre Gueltigkeit
    // hing aber am Link bei der Zustellung (MP4-1).
    bool neutralErreicht = false;
    std::uint64_t abschlussGeneration = 0;
    controlV3.interventionsZug (
        [this, &adresseJson, &neutralErreicht, &abschlussGeneration]
        (std::uint64_t generation, const nakama::ipc::ControlClient::ZugSenke& senke)
    {
        std::lock_guard<std::mutex> l (sendeZustandMutex);   // Ordnung: sendeMutex zuerst
        if (zugHakenFuerTest)
            zugHakenFuerTest (0);            // Zug begonnen (Test, EP-16/EP-20)

        auto sende = [&] (const nakama::ipc::Interventionsereignis& ev,
                          nakama::ipc::P0Klasse klasse) -> std::uint64_t
        {
            // 🔑 NAK-180 Nacharbeit 1 (EP-07): die Marke kommt aus dem EINEN
            // Markenraum des ControlClients. Zwei Folgen mit eigenem Nullpunkt
            // liessen einen Aufbau-Heartbeat und das erste Marker-Begin
            // dieselbe Zahl tragen; der Zustellrueckruf des Heartbeats buchte
            // dann das Begin als zugestellt, und ein Reconnect erzeugte ein
            // doppeltes Replay (N-27/N-36).
            auto json = interventionsWireJson (ev, adresseJson);
            const auto marke = senke (json, klasse);
            if (marke == 0)
            {
                // Der P0-Ueberlauf ist derselbe Fall wie der Ringueberlauf:
                // die Sequenzluecke kommt beim Coordinator an.
                interventionsRingUeberlauf.store (true, std::memory_order_relaxed);
                return 0;
            }
            // 🔑 EP-13/R7: Mitschnitt und Zaehler entstehen NICHT hier. Das
            // Einreihen ist kein Senden; erst der Wire-Commit ist es. Der Text
            // wartet unter seiner Marke auf den Zustellrueckruf.
            if (ausstehendeMitschnitte.size() >= kAusstehendDeckel)
                ausstehendeMitschnitte.pop_front();
            ausstehendeMitschnitte.emplace_back (marke, std::move (json));
            return marke;
        };

        // Bildet das Replay-Begin des offenen lokalen Begins. Dieselbe
        // `intervention_id` (sie entsteht deterministisch aus `instance_id`
        // und Nummer), dieselbe `art`, dieselbe Projektzeit — nur die
        // Sequenznummer ist die naechste.
        // Darf das offene lokale Begin repliziert werden? (E6, dreiwertiger
        // Zustellstand)
        //   0 — nicht eingereiht        → ja, es ist nirgends
        //   1 — eingereiht, nicht auf dem Draht → NEIN, es liegt in der Queue
        //       und reist auf dem neuen Link von selbst; ein Replay waere die
        //       doppelte `intervention_id` aus N-27
        //   2 — zugestellt              → ja, wenn auf einer AELTEREN
        //       Generation; der laufende Link kennt es dann nicht
        auto replayNoetig = [&] (std::uint64_t g)
        {
            if (! offenesBegin.gueltig)
                return false;
            if (offenesBegin.zustand == 1)
                return false;
            if (offenesBegin.zustand == 2)
                return offenesBegin.zustellGeneration != g;
            return true;                       // Zustand 0
        };

        auto replaySenden = [&] () -> bool
        {
            if (! offenesBegin.gueltig)
                return false;
            // 🔑 NAK-180 Messlauf: das Replay traegt die ORIGINALSEQUENZ.
            //
            // Es ist eine WIEDERHOLUNG, keine neue Nachricht - dieselbe
            // `intervention_id`, dieselbe Nummer. Zog es eine frische Zahl,
            // stand sie ueber der des Ring-Backlogs, das der Audiothread
            // schon nummeriert hat: der Broker sah 3, dann 2, wertete das als
            // Luecke und setzte `unknown` (`sequenz_annehmen`). Der frische
            // Link fuehrt `letzte_event_sequence = None` und nimmt die erste
            // Zahl vorbehaltlos an; das Backlog zaehlt danach lueckenlos
            // weiter.
            auto ev = offenesBegin.ereignis;
            ev.beginn = true;
            const auto marke = sende (ev, nakama::ipc::P0Klasse::bericht);
            if (marke == 0)
                return false;
            offenesBegin.zustand = 1;              // eingereiht
            offenesBegin.marke = marke;
            return true;
        };

        // 🔑 NAK-180 Nacharbeit 1 (EP-01/N-36): das offene Begin faellt erst
        // mit dem WIRE-COMMIT seines `end`, nicht mit dessen Einreihen.
        //
        // Stirbt der Link dazwischen, ueberlebt das `end` als Ereignis in der
        // Queue — der Aufbauzug des naechsten Links faende ohne diese
        // Aufzeichnung kein Begin mehr, stellte kein Replay voran, und das
        // `end` traefe beim Broker auf nichts: die Sitzung nullt nie. Und
        // derselbe Rueckruf stellt die Nachlauffrist (EP-02): erst ab dem
        // Draht zaehlt der Broker `tail_samples_offen`.
        auto sendeEnde = [&] (const nakama::ipc::Interventionsereignis& ende)
        {
            // Lag das Begin selbst noch in der Queue (Zustand 1), reist es vor
            // dem `end` und braucht keine Aufzeichnung — die FIFO-Ordnung
            // traegt sie. Nur ein bereits ZUGESTELLTES Begin ist nirgends mehr
            // greifbar.
            const bool warZugestellt = offenesBegin.gueltig && offenesBegin.zustand == 2;
            auto begin = offenesBegin.ereignis;
            begin.beginn = true;
            const auto marke = sende (ende, nakama::ipc::P0Klasse::intervention);
            if (marke != 0)
            {
                letzteEndeMarke = marke;
                letzterEndeTail = ende.tailSamples;
                if (warZugestellt)
                    abschlussBegin = AbschlussBegin { begin, true, marke };
            }
            offenesBegin = OffenesBegin {};
            sendeBeginOffen.store (false, std::memory_order_relaxed);
        };

        // 🔑 EP-08/N-10, zweiter Fall: `prepareToPlay` hat den Marker hart
        // abgeschaltet, waehrend sein Begin noch im RT-Ring lag. Der Uebergang
        // wartet mit seiner Sequenz; er wird genau vor dem ersten Ringereignis
        // mit HOEHERER Sequenz eingereiht, damit die Folge lueckenlos bleibt.
        auto totEndeSenden = [&] ()
        {
            auto ende = offenesBegin.ereignis;
            ende.beginn = false;
            ende.projektzeitGesetzt = false;   // die Endzeit ist ehrlich unbekannt
            ende.sequenz = ausstehenderTotUebergang.sequenz;
            ende.dauerSamples = ausstehenderTotUebergang.dauerSamples;
            ende.tailSamples = ausstehenderTotUebergang.tailSamples;
            if (replayNoetig (generation))
                replaySenden();
            sendeEnde (ende);
            ausstehenderTotUebergang = TotUebergang {};
        };

        // (1) Faelliges Replay des Aufbaus — nur fuer DIESE Generation.
        auto faellig = replayFaellig.load();
        if (faellig != 0)
        {
            if (faellig == generation)
            {
                if (replayFaellig.compare_exchange_strong (faellig, 0)
                    && replayNoetig (generation))
                    replaySenden();
            }
            else
            {
                // Fremde Generation: nicht werten, nur aufraeumen — und auch
                // das nur per CAS, damit ein gerade geschriebenes G+1 stehen
                // bleibt (R13).
                replayFaellig.compare_exchange_strong (faellig, 0);
            }
        }

        // (1b) Ein TOTES Begin wird VOR dem Ringinhalt geschlossen.
        //
        // `prepareToPlay` hat den Marker hart abgeschaltet; sein `end` kommt
        // vom Audiothread nie. Es steht fertig im Sendezustand - mit einer
        // Sequenz, die VOR der des neuen Begins liegt, das der wieder
        // hochfahrende Marker gleich schreibt. Erst danach darf der Ring
        // gelesen werden: ein neues Begin ueberschriebe den Sendezustand,
        // und das tote Intervall bliebe beim Broker fuer immer offen.
        if (offenesBegin.gueltig && offenesBegin.tot)
        {
            if (replayNoetig (generation))
                replaySenden();
            sendeEnde (offenesBegin.totesEnde);
        }

        // (2) Der Ringinhalt, in unveraenderter Reihenfolge.
        nakama::ipc::Interventionsereignis e;
        while (interventionsRing.lies (e))
        {
            // EP-08: das wartende tote Ende gehoert VOR jedes Ereignis mit
            // hoeherer Sequenz — sonst stuende auf dem Draht Begin(n-1),
            // Begin(n+1), End(n), und der Broker saehe eine Luecke.
            if (ausstehenderTotUebergang.gueltig && offenesBegin.gueltig
                && e.sequenz > ausstehenderTotUebergang.sequenz)
            {
                totEndeSenden();
            }

            if (e.beginn)
            {
                const auto marke = sende (e, nakama::ipc::P0Klasse::intervention);
                offenesBegin.ereignis = e;
                offenesBegin.gueltig = true;
                offenesBegin.tot = false;
                offenesBegin.zustand = marke != 0 ? 1 : 0;
                offenesBegin.marke = marke;
                offenesBegin.zustellGeneration = 0;
                sendeBeginOffen.store (true, std::memory_order_relaxed);
                continue;
            }

            // 🔑 R8, Ordnungsregel: ein `end` reist nur, wenn sein Begin auf
            // DIESEM Link zugestellt ist (Zustand 2 mit der laufenden
            // Generation) oder in derselben Queue vor ihm liegt (Zustand 1 —
            // die FIFO-Ordnung stellt es von selbst zu). Sonst geht das
            // Replay-Begin unmittelbar voran; ohne das traefe das `end` beim
            // Broker auf kein Begin und setzte `unknown`.
            if (replayNoetig (generation))
                replaySenden();

            // EP-16/N-35: das Fenster zwischen Vergleich und Einreihen. Im
            // Produkt gibt es keines - beide liegen unter derselben Sperre.
            // Der Haken macht genau das messbar: ein zweiter Faden, der den
            // Linkwechsel HIER erzwingen will, kommt erst nach dem Einreihen
            // zum Zug.
            if (zugHakenFuerTest)
                zugHakenFuerTest (1);

            sendeEnde (e);
        }

        // Ring erschoepft: ein noch wartendes totes Ende geht jetzt raus.
        // Ohne jedes Begin — es ging im Ringueberlauf verloren — faellt es
        // ersatzlos weg; das Sticky-Bit des Ueberlaufs traegt den Zustand.
        if (ausstehenderTotUebergang.gueltig)
        {
            if (offenesBegin.gueltig)
                totEndeSenden();
            else
                ausstehenderTotUebergang = TotUebergang {};
        }

        // (4) Abschluss des Neuaufbau-Berichts (E3.3): ist der Prozessor nach
        //     einem NICHT neutralen Aufbau wieder neutral, darf jetzt genau
        //     ein `false` reisen. Nur fuer die laufende Generation.
        auto bericht = berichtOffen.load();
        if (bericht != 0)
        {
            if (bericht != generation)
            {
                berichtOffen.compare_exchange_strong (bericht, 0);
            }
            else if (interventionsRing.fuellstand() == 0 && ! offenesBegin.gueltig
                     && ! abschlussBegin.gueltig
                     && ! ausstehenderTotUebergang.gueltig
                     // 🔑 EP-02, zweite Haelfte: ein `end`, das eingereiht,
                     // aber noch NICHT auf dem Draht ist, hat den Nachlauf
                     // beim Broker noch gar nicht gestartet. Schloesse der
                     // Bericht hier, reiste das `false` unmittelbar hinter dem
                     // `end` - und traefe dort auf den vollen Nachlauf. Die
                     // Frist allein genuegt nicht: sie steht erst ab dem
                     // Wire-Commit.
                     && letzteEndeMarke == 0
                     && ! markierung.hoerbar()
                     // 🔑 NAK-180 Nacharbeit 1 (EP-02/N-05/M-58): der
                     // NACHLAUF des zuletzt zugestellten `end` muss in
                     // Echtzeit abgelaufen sein. Erreicht das einmalige
                     // `false` den Broker waehrend `tail_samples_offen > 0`,
                     // verwirft er es — und niemand wiederholt es, weil
                     // `berichtOffen` und die Aufbauaussage mit ihm
                     // verbraucht sind. `berichtOffen` bleibt bis dahin
                     // stehen; der naechste Zug prueft erneut.
                     && nachlaufAbgelaufen())
            {
                if (berichtOffen.compare_exchange_strong (bericht, 0))
                {
                    neutralErreicht = true;
                    // 🔑 EP-03/R12: das Urteil gilt GENAU DIESER Generation.
                    abschlussGeneration = generation;
                }
            }
        }

        if (zugHakenFuerTest)
            zugHakenFuerTest (2);            // Zug beendet (Test)
    });

    // AUSSERHALB des Zugs: `meldeAufbauUrteil` nimmt `sendeMutex` nicht, aber
    // die Regel "im Zug wird nicht gesendet" gilt trotzdem — der naechste
    // Heartbeat traegt das `false`.
    //
    // 🔑 EP-03: die Generation reist MIT. Baut G+1 in diesem Fenster auf,
    // wendet der ControlClient den veralteten Abschluss nicht an — sonst
    // ersetzte ein `false` von G das frische, nicht neutrale Urteil von G+1,
    // und der Bericht des neuen Links waere nie eroeffnet.
    if (neutralErreicht)
        controlV3.meldeAufbauUrteil (true, abschlussGeneration);
}

nakama::ipc::ControlHello EqCopilotProcessor::v3Hello() const
{
    nakama::ipc::ControlHello h;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        h.adresse.logonSid = v3LogonSid;
        h.adresse.projectBindingId = zustand.common.projectBindingId.toStdString();
        h.adresse.sessionEpoch = v3SessionEpoch;
        // Die persistente Original-ID bleibt bis zur gemeinsamen v3-Grenze
        // im ControlClient erhalten; erst dort wird der Phase-A-Wirealias
        // gebildet. So kann er nie in den Host-State zurueckfliessen.
        h.adresse.instanceId = zustand.common.instanceId.toStdString();
        h.adresse.runtimeNonce = alsHex32 (instanceNonce, "runtime_nonce");
        h.pluginKind = nakama::state::wort (zustand.common.klasse);
    }
    h.hostAngeben = true;
    h.hostPid = static_cast<std::uint32_t> (_getpid());
    h.pluginVersion = kPluginVersion;
    h.samplerate = samplerateAtomic.load();
    h.blockSize = blockSizeAtomic.load();
    h.channels = kanaeleAtomic.load();
    return h;
}

nakama::ipc::ControlStatus EqCopilotProcessor::v3Status() const
{
    nakama::ipc::ControlStatus s;
    s.dspSchemaVersion = 1;
    s.stateRevision = v3StateRevision.load();
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.hatParameters && ! zustand.nurLesen)
        {
            juce::String hash, grund;
            if (nakama::parameter::stateHash (zustand.parameters, hash, grund))
                s.stateHash = hash.toStdString();
        }
    }
    s.recordStateValid = aufnahmeGueltig.load();
    s.recording = aufnahmeAktiv.load();
    s.framesDropped = queue.verloreneFrames();
    // 🔑 SONDE-013 M-39: das Sticky-Bit des RT→Control-Rings wird GELESEN.
    //
    // Vorher setzten beide Ueberlaufpfade (Audiothread und P0-Sender) das Bit,
    // und niemand las es: ein verlorenes Begin oder End erzeugte dann keine
    // Sequenzluecke mehr — die Nummer war verbraucht, aber wenn das LETZTE
    // Ereignis fiel, folgte kein weiteres, an dem die Luecke sichtbar wuerde.
    // Der Broker blieb scheinbar sauber, und genau das verbietet §34.2:
    // "Ein verlorenes Begin oder End darf niemals eine scheinbar saubere
    // Baseline erzeugen". Der 1-Hz-Heartbeat traegt das Bit deshalb aktiv;
    // beim Empfaenger loest es dasselbe sticky Unknown aus wie die Luecke.
    //
    // Es heilt nicht von selbst: nur `interventionsRing.resync()` bzw.
    // `zuruecksetzen()` loeschen es, und beide laufen ausserhalb des
    // Audiothreads am bestaetigten Neuaufbau.
    s.interventionStateUnknown = interventionsRingUeberlauf.load (std::memory_order_relaxed)
                              || interventionsRing.ueberlaufGesehen();
    return s;
}

nakama::ipc::TelemetryHello EqCopilotProcessor::v3TelemetryHello() const
{
    nakama::ipc::TelemetryHello t;
    const auto h = v3Hello();
    t.adresse = h.adresse;
    t.pluginVersion = h.pluginVersion;
    controlV3.kopplung (t.linkId, t.challenge);
    return t;
}

std::string EqCopilotProcessor::v3SubscribeJson() const
{
    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))
        return {};
    return std::string ("{\"type\":\"subscribe_session\",\"adresse\":")
         + nakama::ipc::adresseAlsJson (h.adresse)
         + ",\"session_epoch\":\"" + h.adresse.sessionEpoch + "\"}";
}

void EqCopilotProcessor::v3ControlLink (bool verbunden)
{
    if (! verbunden)
    {
        // 🔑 NAK-180 R11/R13: loeschen NUR fuer die sterbende Generation, und
        // NUR per CAS. Positiver und negativer Callback koennen ueberlappen —
        // `meldeLinkStatus` serialisiert das Statusbit, nicht die
        // Callback-Ausfuehrung, und `reconnect()`/`stop()` rufen den negativen
        // synchron auf ihrem Aufruferthread (MP3-1). Ein blindes Loeschen
        // naehme dem naechsten Link seine gerade geschriebene Aussage mit.
        //
        // Das ist HYGIENE, nicht die Zusage: die Korrektheit traegt der
        // Generationsvergleich beim Verbraucher. Deshalb ist es auch
        // ungefaehrlich, dass hier gar kein zweiter Ende-Callback kommt, wenn
        // das Statusbit schon `false` war.
        // 🔑 NAK-180 Nacharbeit 1 (EP-05/N-37 Fall 2): die STERBENDE
        // Generation kommt vom ControlClient, nicht aus `wireGenerationJetzt()`.
        //
        // Bei `reconnect()` kann der Clientthread G beenden und G+1
        // vollstaendig aufbauen, bevor der externe Aufrufer seinen
        // verspaeteten `false`-Callback erreicht. Er las dann G+1 und loeschte
        // genau dessen Urteil, Replay und Bericht — die Wirkung des NEUEN
        // Links. Der Client hinterlegt die Zahl beim Statuswechsel, und
        // `meldeLinkStatus` serialisiert den per `exchange`.
        const auto sterbend = controlV3.sterbendeGenerationJetzt();
        if (sterbend == 0)
        {
            // Kein gemeldeter Link — es gibt nichts zu loeschen. `0` traefe
            // ohnehin keinen Zustand, der Fruehausstieg macht es sichtbar.
            sourcesModel.controlEnde();
            telemetryV3.reconnect();
            return;
        }
        controlV3.loescheAufbauUrteil (sterbend);
        auto a = sterbend;
        replayFaellig.compare_exchange_strong (a, 0);
        auto b = sterbend;
        berichtOffen.compare_exchange_strong (b, 0);
        sourcesModel.controlEnde();
        telemetryV3.reconnect();
        return;
    }

    // 🔑 NAK-180 N-11: der Klassentest steht VOR dem Neutralitaetsurteil.
    //
    // Er stand bisher dahinter, und damit gab auch eine als Sonde
    // klassifizierte Instanz eine Aussage ueber Interventionen ab — sie hat
    // aber gar keine Marker (§7.1 E-08). Ein ueberlaufbedingtes `true` aus
    // `v3Status()` reist unveraendert weiter; das ist keine Aussage ueber den
    // Aufbau, sondern fail-closed ueber den Ring.
    auto hVoraus = v3Hello();
    hVoraus.adresse = nakama::ipc::wireAdresseAusState (hVoraus.adresse);
    const bool darfAufbauUrteilMelden =
        hVoraus.pluginKind == "main" && nakama::ipc::adresseGueltig (hVoraus.adresse);

    // 🔑 SONDE-013 Nacharbeit 2 (Befund R01, M-61): DER Produktaufrufer von
    // `InterventionsRing::resync()`.
    //
    // Das Sticky-Bit des Ueberlaufs hatte bis hierher KEINEN Loeschpfad im
    // Produkt: `v3Status()` las es, `resync()` und `interventionsRingUeberlauf
    // .store(false)` rief niemand. Nach dem ersten Ueberlauf meldete jeder
    // Heartbeat dauerhaft `intervention_state_unknown`, und die Sperre auf
    // starker Evidenz fiel nie wieder — ein sticky Bit ohne Gegenpfad ist
    // dieselbe Sorte Fehler wie ein fehlendes.
    //
    // Der bestaetigte Neuaufbau des Control-Links IST der Resync (§34.2: eine
    // AUSDRUECKLICHE, bestaetigte Lebenszyklusoperation): die Bruecke ist neu,
    // der Broker fuehrt fuer diesen Link noch keine Sequenzbasis, und die
    // naechste Nummer wird darum vorbehaltlos angenommen.
    //
    // ⚠️ Er behauptet Neutralitaet nur, wenn sie WIRKLICH gilt: kein Ereignis
    // wartet mehr im Ring, und es ist gerade kein Marker hoerbar. Sonst bleibt
    // das Bit stehen — eine Selbstheilung waere genau das, was §34.2 verbietet.
    // 🔑 NAK-180 E1: Neutralitaet ist SENDEBUCHFUEHRUNG, nicht `hoerbar()`.
    //
    // `hoerbar()` allein genuegt nicht: `prepareToPlay` ruft `vorbereiten()`
    // und `setzeSamplerate()`, beide setzen `warHoerbar = false` und ueber
    // `hartAus()` `fade = 0`, OHNE `hoerbarAtomic` zu beruehren — das wird
    // erst im naechsten Audioblock nachgezogen, bei gestopptem Transport also
    // nie. Der dritte Term ist die einzige Aussage darueber, was der BROKER
    // gesehen hat. Jeder Term darf allein sperren, keiner allein entsperren
    // (§34.2: der teure Fehler liegt auf der Seite "zu frueh entsperrt").
    const bool neutral = interventionsRing.fuellstand() == 0
                      && ! sendeBeginOffen.load (std::memory_order_relaxed)
                      && ! markierung.hoerbar();
    if (neutral)
    {
        interventionsRing.resync();
        interventionsRingUeberlauf.store (false, std::memory_order_relaxed);
        // 🔑 Befund C1: der Zaehler faellt MIT auf null zurueck.
        //
        // Der Broker fuehrt fuer diesen Link ab hier die Basis 0. Liefe das
        // Plugin mit seiner alten, hohen Nummer weiter, waere der Abstand eine
        // Luecke — und der Resync haette den Zustand nicht geklaert, sondern
        // gerade erst wieder unbekannt gemacht.
        interventionsSequenz.store (0, std::memory_order_relaxed);
    }

    // 🔑 NAK-180 R1/R2: die Aussage des Aufbaus reist — als Generationszahl.
    //
    // Neutral: der erste Heartbeat dieses Links traegt
    // `intervention_state_unknown: false`, und DAS ist der einzige Ausloeser
    // von `resync_bestaetigen` im Broker (D-01).
    //
    // Nicht neutral: er traegt ausdrueckliches `true`. Ein FRISCHER Broker
    // haette sonst kein `unknown` — sein `Stand` ist leer, `taint` wird nicht
    // persistiert —, und die Sitzung waere von der ersten Sekunde an
    // faelschlich sauber, waehrend der Marker klingt (C2). Danach stellt das
    // Replay den wahren Zustand her, und der Bericht bleibt offen, bis der
    // Prozessor wieder neutral ist (E3.3).
    //
    // ⚠️ Der Klassentest umschliesst NUR die Wireaussage, nicht den lokalen
    // Resync darueber: der Ring, sein Sticky-Bit und der Sequenzzaehler sind
    // Prozessorzustand und gehoeren jeder Klasse. Nur die BEHAUPTUNG ueber
    // Interventionen darf eine Sonde nicht abgeben — sie hat gar keine
    // Marker (§7.1 E-08).
    if (! darfAufbauUrteilMelden)
    {
        telemetryV3.reconnect();
        return;
    }

    const auto meineGeneration = controlV3.meldeAufbauUrteil (neutral);
    if (! neutral && meineGeneration != 0)
    {
        // 🔑 NAK-180 Nacharbeit 1 (EP-04/R13/N-37): CAS statt `store`.
        //
        // Ein verspaeteter positiver Callback von G konnte nach dem Aufbau von
        // G+1 diese beiden Zustaende blind mit G ueberschreiben. Der Worker
        // raeumte den fremden Wert danach auf und verlor damit Bericht und
        // Replay von G+1 — das Lost-Update, das R13 gerade verbietet. Gesetzt
        // wird nur, solange kein NEUERER Link den Platz belegt hat.
        auto setzeGeneration = [] (std::atomic<std::uint64_t>& ziel, std::uint64_t g)
        {
            auto gesehen = ziel.load();
            while (gesehen < g)
                if (ziel.compare_exchange_weak (gesehen, g))
                    return;
        };
        setzeGeneration (berichtOffen, meineGeneration);
        setzeGeneration (replayFaellig, meineGeneration);
        // Den Worker wecken, damit das Replay nicht auf den 50-ms-Takt
        // wartet. Eine veraltete Weckung ist harmlos: der Zug prueft
        // `replayFaellig` gegen die laufende Generation.
        std::lock_guard<std::mutex> l (workerWarteMutex);
        workerWarte.notify_all();
    }

    auto h = v3Hello();
    h.adresse = nakama::ipc::wireAdresseAusState (h.adresse);
    if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))
        return;
    sourcesModel.beginneSubscription (h.adresse.projectBindingId,
                                      h.adresse.sessionEpoch,
                                      h.adresse.instanceId);
    const auto subscribe = v3SubscribeJson();
    const auto ergebnis = subscribe.empty()
        ? nakama::ipc::P1Ergebnis::abgewiesen
        : controlV3.sendeP1 ("subscribe_session", subscribe);
    if (ergebnis == nakama::ipc::P1Ergebnis::abgewiesen
        || ergebnis == nakama::ipc::P1Ergebnis::zuGross)
    {
        sourcesModel.controlEnde();
        controlV3.reconnect();
        return;
    }
    telemetryV3.reconnect();
}

void EqCopilotProcessor::v3Antwort (const std::string& json,
                                    std::uint8_t schemaMinor)
{
    nakama::ipc::GelesenesCommandAck ack;
    if (nakama::ipc::commandAckHaeltVertrag (json, ack))
    {
        std::lock_guard<std::mutex> l (sourcesCommandMutex);
        const auto it = ausstehendeSourcesCommands.find (ack.commandId);
        if (it != ausstehendeSourcesCommands.end())
        {
            if (ack.erfolgreich)
                bestaetigteSourcesCommands.push_back (it->second);
            ausstehendeSourcesCommands.erase (it);
        }
        return;
    }
    juce::String fehler;
    // SONDE-013 Nacharbeit 2 (Befund R28): der LESER der Evidenzruecknahme.
    //
    // Bis hierher kannte diese Stelle genau zwei Nachrichten - Command-ACK und
    // Sessionsnapshot. Der Broker legte fuer `evidence_invalidate` eine
    // Outbox-Schuld an, die niemand abtrug: ein aktiver Subscriber erhielt die
    // Ruecknahme nie, und Gen zeigte weiter Zahlen, deren Grundlage
    // zurueckgezogen war. Eine Zustellschuld ohne Leser ist ein Defekt.
    const auto ruecknahme = sourcesModel.uebernehmeEvidenzruecknahme (
        json, schemaMinor, fehler);
    if (ruecknahme != SourcesModel::RuecknahmeErgebnis::ignoriert)
    {
        if (ruecknahme == SourcesModel::RuecknahmeErgebnis::ungueltig)
            sourcesModel.setzeDiagnoseFuerSichtbeweis (
                SourcesModel::Diagnose::incompatible, false);
        return;
    }
    const auto ergebnis = sourcesModel.uebernehmeSessionSnapshot (
        json, schemaMinor, SourcesModel::Uhr::now(), fehler);
    if (ergebnis == SourcesModel::SnapshotErgebnis::ungueltig)
        sourcesModel.setzeDiagnoseFuerSichtbeweis (
            SourcesModel::Diagnose::incompatible, false);
}

void EqCopilotProcessor::v3Frame (const std::uint8_t* daten, std::size_t laenge,
                                  std::uint8_t schemaMinor)
{
    juce::String fehler;
    sourcesModel.uebernehmeP2 (daten, laenge, schemaMinor,
                               SourcesModel::Uhr::now(), fehler);
}

void EqCopilotProcessor::sourcesTick()
{
    wendeBestaetigteSourcesCommandsAn();
#if ! defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
    sourcesModel.tick (SourcesModel::Uhr::now());
    sourcesModel.setzeControlTransport (controlV3.snapshot());
#endif
}

void EqCopilotProcessor::reconnectSources()
{
    controlV3.reconnect();
}

StatsSnapshot EqCopilotProcessor::statsSnapshot() const
{
    StatsSnapshot s;
    s.rmsL = rmsL.load();
    s.rmsR = rmsR.load();
    // SONDE-008: EINE Wahrheit. Der Zähler lebt in der Queue, die den Verlust
    // verursacht; ein zweiter Atomic daneben könnte nur auseinanderlaufen.
    // Einheit unverändert: verlorene Analyse-FRAMES.
    s.framesDropped = queue.verloreneFrames();
    s.nanSeen = nanSeen.load();
    // NAK-180 R4: DER Produktleser des Wet-Riegels. Ein gesetzter Zaehler
    // ohne Leser ist dasselbe wie kein Zaehler (Pruefliste A).
    s.wetNichtEndlich = markierung.nichtEndlicheWetSamples();
    s.hasTransport = hatTransport.load();
    s.transportPlaying = transportSpielt.load();
    s.projectTimeValid = projektZeitGueltig.load();
    s.projectTimeSamples = projektZeitSamples.load();
    return s;
}

// Kompakter Messstand für den v2-Heartbeat (Vertrag: measurement in
// eq-ipc.schema.json). Läuft 1×/s im Pipe-Thread; engine.snapshot() ist die
// threadsichere Kopie. LTAS wird auf 0,1 dB gerundet — die volle Auflösung
// bleibt der lokalen Snapshot-Datei vorbehalten.
MessKompakt EqCopilotProcessor::messKompakt() const
{
    const auto m = engine.snapshot();
    MessKompakt k;
    k.vorhanden = true;   // auch »keine_daten« ist Information für die Übersicht
    k.zustand = m.zustand == MessZustand::messbereit ? "messbereit"
              : m.zustand == MessZustand::sammelt    ? "sammelt"
                                                     : "keine_daten";
    k.metricsVersion = kMetricsVersion;
    k.aktivS  = m.aktivSekunden;
    k.gesamtS = m.gesamtSekunden;
    k.lufsGueltig = m.lufsGueltig;           k.lufsI = m.lufsIntegriert;
    k.lufsShortGueltig = m.lufsShortGueltig; k.lufsS = m.lufsShort;
    k.truePeakGueltig = m.zustand != MessZustand::keineDaten && m.truePeakDb > -199.0;
    k.truePeakDb = m.truePeakDb;
    k.crestGueltig = m.crestGueltig;         k.crestDb = m.crestDb;
    k.spektralGueltig = m.spektralGueltig;
    k.centroidHz = m.centroidMagHz;          k.lowFrac = m.lowFrac;
    k.stereoGueltig = m.stereoGueltig;
    k.width = m.width;                       k.corr = m.corr;
    if (m.ltasGueltig)
    {
        k.ltasKompositDb.resize ((size_t) kLtasBaender);
        for (int b = 0; b < kLtasBaender; ++b)
        {
            const double v = m.ltasKompositDb[(size_t) b];
            k.ltasKompositDb[(size_t) b] = std::isfinite (v)
                ? std::round (v * 10.0) / 10.0
                : std::numeric_limits<double>::quiet_NaN();   // ⇒ null im JSON
        }
    }
    k.fensterSpruenge = fensterSpruenge.load();
    if (fensterAktiv.load())
    {
        k.fensterGueltig  = true;
        k.fensterVon      = fensterVon.load();
        k.fensterBis      = fensterBis.load();
    }
    // Hör-Markierung aktiv ⇒ nachgelagerte Sensoren hören gefärbtes Signal —
    // der Broker markiert den Messstand und pausiert fremde Aggregate
    // (Konzept v2 §5; Broker-Seite folgt nach dem Harness-Lauf).
    k.hoermarkierung = markierung.hoerbar();
    return k;
}

// Sichtbare Antwort auf einen Kennungs-Konflikt (Plan §8.4): DIESE Instanz
// bekommt eine frische persistente ID und meldet sich neu an. Der Host
// speichert sie mit dem nächsten Projekt-Save (getStateInformation) — dafür
// MUSS er die Änderung kennen: Host-Dirty (Vertrag nakama-state-v2.md §6).
bool EqCopilotProcessor::neueSensorId()
{
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen)
            return false;
        zustand.common.instanceId = juce::Uuid().toString();
    }
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
    pipe.reconnect();
    controlV3.reconnect();
    return true;
}

void EqCopilotProcessor::meldeHostDirty()
{
    updateHostDisplay (juce::AudioProcessorListener::ChangeDetails().withNonParameterStateChanged (true));
}

// ── State: Schema 2 `NakamaState` (SONDE-006) ─────────────────────────────
// Vertrag: eq-copilot/schemas/state/nakama-state-v2.md. Schema 1 wird rein
// und deterministisch migriert; ein Stand, den dieser Build nicht
// interpretieren darf, wird read-only gehalten und bytegleich zurueckgegeben.
void EqCopilotProcessor::getStateInformation (juce::MemoryBlock& ziel)
{
    std::lock_guard<std::mutex> l (bindungMutex);
    nakama::state::speichere (zustand, ziel);
}

void EqCopilotProcessor::setStateInformation (const void* daten, int groesse)
{
    if (daten == nullptr || groesse <= 0)
        return;

    nakama::state::Zustand geladen;
    const auto ergebnis = nakama::state::lade (daten, (size_t) groesse, bundleVertrag(), geladen);
    if (ergebnis == nakama::state::LadeErgebnis::ignoriert)
        return;   // fremder Baumtyp / Muell: Zustand bleibt (wie seit 0.1)
    {
        std::lock_guard<std::mutex> l (sourcesCommandMutex);
        ausstehendeSourcesCommands.clear();
        bestaetigteSourcesCommands.clear();
    }

    if (ergebnis == nakama::state::LadeErgebnis::nurLesen)
    {
        // Keine vertrauenswuerdige Identitaet ⇒ keine Anmeldung beim Broker.
        // ERST stoppen, DANN tauschen: ein gerade laufender (Re-)Connect liest
        // den Zustand im hello-Lambda - nach dem Tausch waere das ein hello
        // mit leerer instance_id (T2-Befund SONDE-006).
        pipe.stop();
        {
            std::lock_guard<std::mutex> l (bindungMutex);
            zustand = geladen;
            // §53.5: read-only ist kein vollstaendiger State-Restore. Zurueck
            // auf neutral - auch aus einer frueheren positiven Klassifikation.
            lebenslauf.stateRestauriert (ergebnis, geladen);
            spiegleKlassifikation();
        }
        sourcesModel.projektReload ({});
        v3StateRevision.fetch_add (1);
        controlV3.reconnect();
        return;
    }

    {
        std::lock_guard<std::mutex> l (bindungMutex);
        zustand = geladen;
        // §53.5: JETZT, nach vollstaendigem Restore, darf klassifiziert
        // werden - Schema-1 `sensor|pre|post` ist zu `legacy` migriert,
        // Schema-1 `hub` und ein bestaetigter Schema-2-Main-State zu `main`.
        lebenslauf.stateRestauriert (ergebnis, geladen);
        spiegleKlassifikation();
    }
    sourcesModel.projektReload (geladen.mainProjectMitglieder);
    pipe.start();       // No-Op, wenn sie laeuft; hebt einen frueheren read-only-Stopp auf
    pipe.reconnect();   // frisches hello mit der geladenen Bindung
    v3StateRevision.fetch_add (1);
    controlV3.reconnect();
    // Kein Host-Dirty: Laden und Migration sind keine Aenderung des Users.
}

juce::String EqCopilotProcessor::holeSensorId() const { std::lock_guard<std::mutex> l (bindungMutex); return zustand.common.instanceId; }
juce::String EqCopilotProcessor::holeRolle() const    { std::lock_guard<std::mutex> l (bindungMutex); return nakama::state::v2Rolle (zustand.common); }
juce::String EqCopilotProcessor::holeLabel() const    { std::lock_guard<std::mutex> l (bindungMutex); return zustand.common.label; }
juce::String EqCopilotProcessor::holePaarId() const   { std::lock_guard<std::mutex> l (bindungMutex); return zustand.common.pairId; }

bool EqCopilotProcessor::stateNurLesen() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.nurLesen;
}

nakama::state::Herkunft EqCopilotProcessor::holeStateHerkunft() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.herkunft;
}

juce::String EqCopilotProcessor::holeStateGrund() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.grund;
}

int EqCopilotProcessor::holeStateFremdesMajor() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.fremdesMajor;
}

nakama::state::Zustand EqCopilotProcessor::holeZustandKopie() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand;
}

// ── Lifecycle-Klassifikation (§53.5) ───────────────────────────────────────

void EqCopilotProcessor::spiegleKlassifikation()
{
    // Aufrufer haelt `bindungMutex`. Der Audiothread liest ausschliesslich
    // diese Atomic; er befragt den Automaten nie (§53.5: "Klassifikation,
    // Spawn und Pipe-I/O liegen nie im Audiocallback").
    // Der Store bleibt bewusst seq_cst (Vorgabe) wie `editorOffen` und
    // `echtzeitOk` daneben: er laeuft nie im Audiothread, die Ordnung kostet
    // hier nichts, und eine dritte Ordnungsregel im selben Zustandsblock waere
    // eine Frage, die ein Leser jedes Mal neu beantworten muesste. Gelesen
    // wird im processBlock relaxed - dort haengt kein anderer Wert daran.
    istMainKlassifiziert.store (lebenslauf.audioAusnahmeErlaubt());
}

nakama::state::Klassifikation EqCopilotProcessor::holeKlassifikation() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return lebenslauf.klassifikation();
}

bool EqCopilotProcessor::darfBrokerStarten() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return lebenslauf.darfBrokerStarten();
}

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

bool EqCopilotProcessor::setzeBindung (const juce::String& r, const juce::String& lbl, const juce::String& p)
{
    // Dieselben Grenzen wie der einzige UI-Aufrufer. Sie gehoeren auch an die
    // API-Kante: der State-Leser beweist seinen Writer-Headroom gegen genau
    // diese Grenzen; ein kuenftiger Caller darf sie nicht umgehen.
    if (lbl.length() > 120 || p.length() > 60)
        return false;

    nakama::state::Klasse klasse;
    nakama::state::Messposition position;
    if (! nakama::state::ausV2Rolle (r, klasse, position))
        return false;

    std::vector<nakama::state::MainProjectMitglied> mainMitglieder;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen)
            return false;
        auto neu = zustand.common;
        neu.klasse = klasse;
        neu.position = position;
        neu.label = lbl;
        neu.pairId = p;
        // Die Projektbindung entsteht nur an diesem sichtbaren User-Akt.
        // Frischzustand und Migration bleiben leer; der State selbst wird
        // danach zur autoritativen, persistierten Quelle fuer alle Clients.
        if (klasse == nakama::state::Klasse::main
            && neu.projectBindingId.isEmpty())
            neu.projectBindingId = juce::String (uuidHex32());
        if (neu == zustand.common)
            return false;   // keine Aenderung: kein Dirty, kein Reconnect-Geflacker
        zustand.common = neu;
        if (klasse != nakama::state::Klasse::main)
            zustand.mainProjectMitglieder.clear();
        mainMitglieder = zustand.mainProjectMitglieder;

        // §53.5, dritter Punkt: "leerer, nie gespeicherter Altstate → Main
        // erst nach geoeffnetem Editor UND expliziter Initialisierung". Genau
        // hier ist dieser Akt - der einzige Aufrufer von `setzeBindung` ist
        // die Rollenwahl im Editor (PluginEditor.cpp), und der Automat
        // verlangt zusaetzlich selbst einen offenen Editor. Ein Scannerlauf
        // kann ihn nicht ausloesen: er bedient nichts und oeffnet nichts.
        //
        // Der Weg gilt in BEIDE Richtungen: stellt der User `hub` zurueck auf
        // `sensor`, faellt die Klassifikation auf `legacy`. Sonst behauptete
        // der Automat etwas anderes als der Stand, den dieselbe Instanz im
        // naechsten Projekt-Save schreibt.
        lebenslauf.expliziteInitialisierung (zustand);
        spiegleKlassifikation();
    }
    sourcesModel.setzePersistenteMitglieder (mainMitglieder);
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
    pipe.reconnect();
    controlV3.reconnect();
    return true;
}

bool EqCopilotProcessor::bindeSourcesHauptziel (const std::string& erwarteteInstanceId)
{
    return sendeSourcesCommand (SourcesCommandArt::confirmJoin, erwarteteInstanceId);
}

bool EqCopilotProcessor::benenneSourcesHauptziel (const std::string& erwarteteInstanceId,
                                                  const juce::String& label)
{
    if (label.length() > 120 || ! sourcesModel.istAktuellesHauptziel (erwarteteInstanceId)
        || ! sourcesModel.sicht().mainDarfSchreiben)
        return false;
    std::vector<nakama::state::MainProjectMitglied> kopie;
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main
            || ! sourcesModel.istAktuellesHauptziel (erwarteteInstanceId))
            return false;
        const auto gefunden = std::find_if (
            zustand.mainProjectMitglieder.begin(), zustand.mainProjectMitglieder.end(),
            [&] (const auto& m) { return m.instanceId.toStdString() == erwarteteInstanceId; });
        if (gefunden == zustand.mainProjectMitglieder.end() || gefunden->label == label)
            return false;
        gefunden->label = label;
        kopie = zustand.mainProjectMitglieder;
    }
    sourcesModel.setzePersistenteMitglieder (kopie);
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
    return true;
}

bool EqCopilotProcessor::entferneSourcesHauptziel (const std::string& erwarteteInstanceId)
{
    const auto sicht = sourcesModel.sicht();
    if (! sicht.mainDarfSchreiben
        || ! sourcesModel.istAktuellesHauptziel (erwarteteInstanceId)
        || ! nakama::ipc::istHex32 (erwarteteInstanceId))
        return false;
    const auto quelle = std::find_if (sicht.quellen.begin(), sicht.quellen.end(),
        [&] (const auto& q) { return q.instanceId == erwarteteInstanceId && q.hauptziel; });
    if (quelle == sicht.quellen.end()
        || quelle->mitgliedschaft != SourcesModel::Mitgliedschaft::bestaetigt)
        return false;
    if (! nakama::ipc::istHex32 (quelle->runtimeNonce))
    {
        std::vector<nakama::state::MainProjectMitglied> kopie;
        {
            std::lock_guard<std::mutex> l (bindungMutex);
            if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
                return false;
            const auto gefunden = std::find_if (
                zustand.mainProjectMitglieder.begin(), zustand.mainProjectMitglieder.end(),
                [&] (const auto& m) {
                    return m.instanceId.toStdString() == erwarteteInstanceId;
                });
            if (gefunden == zustand.mainProjectMitglieder.end())
                return false;
            zustand.mainProjectMitglieder.erase (gefunden);
            kopie = zustand.mainProjectMitglieder;
        }
        sourcesModel.setzePersistenteMitglieder (kopie);
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
        return true;
    }
    return sendeSourcesCommand (SourcesCommandArt::unbindProbe, erwarteteInstanceId);
}

bool EqCopilotProcessor::merkeManuellePassage (const juce::String& passageId,
                                               const juce::String& label,
                                               juce::int64 projektStart,
                                               juce::int64 projektEnde)
{
    // Die Riegel VOR dem Schloss: sie brauchen keinen Zustand und halten die
    // kritische Strecke kurz.
    if (! nakama::ipc::istHex32 (passageId.toStdString()) || label.length() > 120)
        return false;
    if (projektStart < 0 || projektEnde <= projektStart)
        return false;

    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        if (static_cast<int> (zustand.manuellePassagen.size()) >= nakama::state::maxManuellePassagen)
            return false;
        const auto gefunden = std::find_if (
            zustand.manuellePassagen.begin(), zustand.manuellePassagen.end(),
            [&] (const auto& s) { return s.passageId == passageId; });
        if (gefunden != zustand.manuellePassagen.end())
            return false;
        zustand.manuellePassagen.push_back ({ passageId, label, projektStart, projektEnde });
    }
    // 🔑 SONDE-013 Nacharbeit 2 (Befund R03, M-03/M-25): die gespeicherte
    // Passage erreicht die ENGINE. Bis hierher blieb sie Plugin-State, und die
    // Passagenmetriken liefen weiter seit der letzten Transportgrenze — eine
    // leise Passage nach einem lauten Abschnitt erbte dessen Spitze.
    //
    // Der Nachrichtenthread fasst die Engine nicht an; er hinterlegt den Wunsch
    // und der Analyseworker loest ihn unter seiner Steuersperre ein.
    bindePassagenfenster (passageId, projektStart, projektEnde);
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
    return true;
}

bool EqCopilotProcessor::vergissManuellePassage (const juce::String& passageId)
{
    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        const auto gefunden = std::find_if (
            zustand.manuellePassagen.begin(), zustand.manuellePassagen.end(),
            [&] (const auto& s) { return s.passageId == passageId; });
        if (gefunden == zustand.manuellePassagen.end())
            return false;
        zustand.manuellePassagen.erase (gefunden);
    }
    // Der Gegenpfad zu `merkeManuellePassage`: die Engine loest die Bindung
    // wieder — danach gilt wieder die Transportepoche als Fenster, also der
    // Fall „der User hat keine Passage markiert" (Befund R03).
    //
    // 🔑 Nacharbeit 3 (Befund C3): NUR, wenn diese Passage die gebundene ist.
    // Bis dahin loeschte jedes Vergessen bedingungslos das globale Fenster und
    // stoppte den Pegel — auch den einer ganz anderen, gerade laufenden
    // Passage. Der State haelt bis zu 64 Passagen; ein globaler Slot ohne
    // Zuordnung war die eigentliche Ursache.
    loesePassagenfenster (passageId);
    // Das Vergessen meldet Dirty wie das Merken. Ein Loeschen, das der Host
    // nicht mitbekommt, kaeme beim naechsten Oeffnen zurueck.
    meldeHostDirty();
    v3StateRevision.fetch_add (1);
    return true;
}

//==============================================================================
// SONDE-013 M-40 bis M-51, Nacharbeit 2 (Befund R06): der Experimentpfad.
//
// 🔑 `Vergleichspegel` und `Blindvergleich` waren uebersetzt und im Produkt
// UNBENUTZT. `nichtEndlicheSamples()` hatte ausserhalb der C++-Tests keinen
// Leser: ein nichtendliches Sample im Vergleichsmaterial verriegelte lokal den
// Gain und blieb im Produkt ungezaehlt — genau die stille Beschoenigung, die
// M-07 und CLAUDE.md ausschliessen.
//
// Diese Schicht ist MODELL und Nachrichtenweg, kein sichtbares Element: die
// Bedienfragen P-01 bis P-06 gehoeren dem User (§4.2).

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
    s += ",\"match_gain_db\":" + zahl (vergleichspegel.gainDb());
    // 🔑 Nacharbeit 3 (Befund C5, M-07/R06): der Nichtendlich-Zaehler REIST.
    //
    // Die Runde 2 machte ihn nur ueber einen Test-Getter sichtbar; im
    // Wirezustand stand er nicht, und der R06-Fall rief genau diesen Getter
    // auf. Damit war „reist in den Wirezustand" eine Behauptung ueber eine
    // Zeile, die es nicht gab. Das Feld ist optional in der Fassung 2 und
    // traegt 0 als „nachweislich keines", nicht als „nicht gemessen".
    s += ",\"nicht_endliche_samples\":"
       + std::to_string ((unsigned long long) vergleichspegel.nichtEndlicheSamples());
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
    s += ",\"base_revision\":" + std::to_string (v3StateRevision.load());
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
        return false;
    if (! blindvergleich.uebernimmVergleichspegel (vergleichspegel))
        return false;

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
    return vergleichspegel.eingefroren() && vergleichspegel.gainGesetzt();
}

double EqCopilotProcessor::versuchMatchGainDb() const
{
    return vergleichspegel.gainDb();
}

juce::uint64 EqCopilotProcessor::versuchNichtEndlicheSamples() const
{
    return (juce::uint64) versuchNichtEndlich.load (std::memory_order_relaxed);
}

std::vector<nakama::state::ManuellePassage> EqCopilotProcessor::manuellePassagen() const
{
    std::lock_guard<std::mutex> l (bindungMutex);
    return zustand.manuellePassagen;
}

bool EqCopilotProcessor::sendeSourcesCommand (SourcesCommandArt art,
                                               const std::string& erwarteteInstanceId)
{
    const auto sicht = sourcesModel.sicht();
    if (! sicht.mainDarfSchreiben || ! sourcesModel.istAktuellesHauptziel (erwarteteInstanceId)
        || ! nakama::ipc::istHex32 (erwarteteInstanceId))
        return false;
    const auto quelle = std::find_if (sicht.quellen.begin(), sicht.quellen.end(),
        [&] (const auto& q) { return q.instanceId == erwarteteInstanceId && q.hauptziel; });
    if (quelle == sicht.quellen.end() || ! nakama::ipc::istHex32 (quelle->runtimeNonce))
        return false; // Ohne aktuellen Brokeradressaten kein behaupteter Wire-Unbind.

    {
        std::lock_guard<std::mutex> l (bindungMutex);
        if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main)
            return false;
        const auto hat = std::any_of (
            zustand.mainProjectMitglieder.begin(), zustand.mainProjectMitglieder.end(),
            [&] (const auto& m) { return m.instanceId.toStdString() == erwarteteInstanceId; });
        if ((art == SourcesCommandArt::confirmJoin && (hat
                || zustand.mainProjectMitglieder.size()
                    >= static_cast<std::size_t> (nakama::state::maxMainProjectMitglieder)))
            || (art == SourcesCommandArt::unbindProbe && ! hat))
            return false;
    }

    auto h = v3Hello();
    h.adresse.instanceId = erwarteteInstanceId;
    h.adresse.runtimeNonce = quelle->runtimeNonce;
    if (! nakama::ipc::adresseGueltig (h.adresse))
        return false;
    SourcesCommand auftrag;
    auftrag.art = art;
    auftrag.commandId = uuidHex32();
    auftrag.instanceId = erwarteteInstanceId;
    auftrag.projectBindingId = h.adresse.projectBindingId;
    auftrag.sessionEpoch = h.adresse.sessionEpoch;
    if (art == SourcesCommandArt::confirmJoin)
        auftrag.label = quelle->userLabel.substring (0, 120);
    const char* command = art == SourcesCommandArt::confirmJoin
                            ? "confirm_join" : "unbind_probe";
    auftrag.json = std::string ("{\"type\":\"session_command\",\"command\":\"")
                 + command + "\",\"command_id\":\"" + auftrag.commandId
                 + "\",\"ziel\":" + nakama::ipc::adresseAlsJson (h.adresse)
                 + ",\"session_epoch\":\"" + h.adresse.sessionEpoch + "\"}";
    {
        std::lock_guard<std::mutex> l (sourcesCommandMutex);
        if (std::any_of (ausstehendeSourcesCommands.begin(),
                         ausstehendeSourcesCommands.end(),
                         [&] (const auto& paar) {
                             return paar.second.instanceId == erwarteteInstanceId;
                         }))
            return false;
        ausstehendeSourcesCommands.emplace (auftrag.commandId, auftrag);
    }
    if (controlV3.sendePersistenzP0 (auftrag.json))
        return true;
    std::lock_guard<std::mutex> l (sourcesCommandMutex);
    ausstehendeSourcesCommands.erase (auftrag.commandId);
    return false;
}

void EqCopilotProcessor::wendeBestaetigteSourcesCommandsAn()
{
    std::vector<SourcesCommand> befehle;
    {
        std::lock_guard<std::mutex> l (sourcesCommandMutex);
        befehle.swap (bestaetigteSourcesCommands);
    }
    for (const auto& befehl : befehle)
    {
        std::vector<nakama::state::MainProjectMitglied> kopie;
        bool geaendert = false;
        {
            std::lock_guard<std::mutex> l (bindungMutex);
            if (zustand.nurLesen || zustand.common.klasse != nakama::state::Klasse::main
                || zustand.common.projectBindingId.toStdString() != befehl.projectBindingId
                || v3SessionEpoch != befehl.sessionEpoch)
                continue; // ACK eines vor Reload gueltigen Laufs mutiert den neuen State nie.
            auto gefunden = std::find_if (
                zustand.mainProjectMitglieder.begin(), zustand.mainProjectMitglieder.end(),
                [&] (const auto& m) { return m.instanceId.toStdString() == befehl.instanceId; });
            if (befehl.art == SourcesCommandArt::confirmJoin
                && gefunden == zustand.mainProjectMitglieder.end()
                && zustand.mainProjectMitglieder.size()
                    < static_cast<std::size_t> (nakama::state::maxMainProjectMitglieder))
            {
                zustand.mainProjectMitglieder.push_back (
                    { juce::String (befehl.instanceId), befehl.label });
                geaendert = true;
            }
            else if (befehl.art == SourcesCommandArt::unbindProbe
                     && gefunden != zustand.mainProjectMitglieder.end())
            {
                zustand.mainProjectMitglieder.erase (gefunden);
                geaendert = true;
            }
            kopie = zustand.mainProjectMitglieder;
        }
        if (! geaendert)
            continue;
        sourcesModel.setzePersistenteMitglieder (kopie);
        meldeHostDirty();
        v3StateRevision.fetch_add (1);
    }
}

#if defined(NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3)
std::string EqCopilotProcessor::ausstehenderSourcesCommandFuerTest() const
{
    std::lock_guard<std::mutex> l (sourcesCommandMutex);
    return ausstehendeSourcesCommands.empty()
             ? std::string() : ausstehendeSourcesCommands.begin()->second.json;
}
#endif

// ── Lokaler Mess-Snapshot als Datei (M1 §11: "lokale Snapshot-Erfassung") ──
// Kein Roh-Audio, keine Historie im Plugin-State — nur der Messstand.
// NaN/±inf werden als JSON-null geschrieben (juce::JSON kennt kein NaN).
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

juce::AudioProcessorEditor* EqCopilotProcessor::createEditor()
{
    return new EqCopilotEditor (*this);
}

} // namespace eqcop
