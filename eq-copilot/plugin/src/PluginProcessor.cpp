// EqCopilotProcessor — Wurzel: Aufbau, Abbau und der Audio-Pfad.
//
// Diese Datei behaelt Namen und Pfad, damit die Historie des Audio-Pfads auf
// ihr bleibt (NAK-225/S25d, 09.09.2026). Sie traegt:
//
//   Konstruktor, Destruktor   Aufbau und Abbau der Instanz samt Pipe, Control,
//                             Telemetrie und Brokerlebenszyklus.
//   prepareToPlay             steht in prozessor/Hostbruecke.cpp.
//   processBlock              DER Audio-Block.
//   nakamaBlockEmpfangen      Der Rueckruf der Hostbruecke, auf demselben
//                             Thread wie processBlock.
//   lebenszeichen             Wird aus processBlock gerufen.
//   nachlaufFristSetzen, nachlaufAbgelaufen, mitschnittZustellen,
//   mitschnittVerwerfen       Der Quarantaene-Nachlauf der Hoermarkierung; er
//                             haengt am Blocktakt.
//
// AUDIO-THREAD (CLAUDE.md, Grundgesetz): keine Sperren, keine Allokationen,
// keine Datei-, Pipe- oder Netzzugriffe, kein Logging. Bei Ueberlast werden
// Analyseframes verworfen, nie Audio. Ausgeschaltet ist der Pfad im Nulltest
// bitidentisch; eingeschaltet ist der Passthrough sampleidentisch, ohne
// Latenz und ohne Tail. A1 (EqCopNullTest), A16 (EqCopProbeeqNullTest) und
// B3b (EqCopHostProbeTest, Allokationszaehler) messen genau das.
//
// Die uebrigen Fachbereiche liegen unter src/prozessor/:
//
//   Hostbruecke.cpp   prepareToPlay, isBusesLayoutSupported, setzeEditorOffen,
//                     meldeHostDirty, createEditor.
//   State.cpp         speichern, laden, Migration, Bindung, Quellenintent.
//   Ipc.cpp           Broker, Control, Telemetrie, Interventionen.
//   Analyse.cpp       Worker, Snapshots, Assistent, Versuch.
//   Intern.h          der eine Helfer, den mehrere Teile brauchen.
//
// Die anonymen Helfer, die HIER stehen, gehoeren zu genau dieser Datei:
// projektEnde und projektAbstandGroesserAls64 rechnen im Block, die beiden
// Broker-Helfer nur im Konstruktor.

#include "PluginProcessor.h"
#include "Diagnose.h"
#include "WorkerCadence.h"
#include "BrokerInstallBinding.h"
#include "PipeToken.h"
#include "prozessor/Intern.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>

namespace eqcop
{

using prozessor_intern::uuidHex32;

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
            //
            // 🔑 Nacharbeit 2 (WN-03): JEDES zugestellte `end` stellt seine
            // Frist, und `nachlaufFristSetzen` nimmt das Maximum - genau wie
            // der Broker. Vorher trug nur das zuletzt eingereihte `end` seine
            // Marke, und ein langes verlor seine Frist an ein kurzes.
            for (auto it = ausstehendeEnden.begin(); it != ausstehendeEnden.end(); ++it)
            {
                if (it->first != marke)
                    continue;
                nachlaufFristSetzen (it->second);
                ausstehendeEnden.erase (it);
                break;
            }
            // 🔑 WN-01/N-36: erst wenn KEIN `end` mehr auf den Draht wartet,
            // darf der Aufbau eines neuen Links wieder neutral urteilen.
            abschlussOffen.store (! ausstehendeEnden.empty(), std::memory_order_relaxed);
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
    // 🔑 SONDE-014 WN-01 (Nacharbeit 2) / KR-01 (E-15, Konvergenzrunde): der
    // Weg zurueck aus einem `konflikt`-ACK. Er liest und formt nur - das
    // Einreihen macht der Client, unter derselben Sperre, die er ohnehin
    // haelt.
    //
    // Der zweite Parameter ist der AUFTRAG selbst. Bis zur Runde 3 stand hier
    // ein `_`, und der Prozessor baute den Text aus einem eigenen,
    // gedeckelten Register neu; die Gegenrichtung dazu war ein
    // `setzeAuftragAbgeschlossenHook`, der dieses Register wieder freigab.
    // Beides ist mit E-15 fort: der ControlClient haelt den Auftrag, also
    // haelt er auch den Wiederholungsinhalt.
    controlV3.setzeKonfliktWiederholungHook (
        [this] (const std::string& commandId, const std::string& auftragJson,
                std::uint64_t brokerRevision) -> std::string
        {
            return urteilMitFrischemKopf (juce::String (commandId), auftragJson,
                                          brokerRevision);
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
    zwischen Frist und Draht liegt bis zu ein Takt.

    🔑 NAK-180 Nacharbeit 2 (WN-03/N-05/M-58): die lokale Frist ist NIE
    kuerzer als die des Brokers.

    Zwei Wege hatten sie verkuerzt. Erstens ein Deckel von 3600 Sekunden: ein
    einzelner Tail darueber wurde gekappt, waehrend der Broker
    `tail_samples_offen` ungekappt herunterzaehlt
    (`intervention.rs:231`). Zweitens ein spaeteres, KUERZERES `end`: es
    ueberschrieb eine noch laufende laengere Frist, waehrend der Broker das
    MAXIMUM haelt (`tail_samples_offen.max(tail_samples)`). In beiden Faellen
    verbrauchte das Plugin sein einmaliges `false`, solange
    `tail_samples_offen > 0` — E4 verwirft es, niemand wiederholt es, und die
    Sitzung bleibt gesperrt.

    Gesaettigt wird deshalb nur noch am ZAHLENRAND (`int64`-Nanosekunden),
    nicht an einer erfundenen Stunde, und eine offene Frist wird nur
    verlaengert, nie gekuerzt. */

void EqCopilotProcessor::nachlaufFristSetzen (std::uint64_t tailSamples)
{
    double rate = letzteGueltigeSamplerate.load (std::memory_order_relaxed);
    if (! (std::isfinite (rate) && rate > 0.0))
        rate = 48000.0;                      // dieselbe Vorgabe wie im Broker
    const double sekunden = (double) tailSamples / rate;
    const auto jetzt = std::chrono::steady_clock::now().time_since_epoch();
    const auto jetztNs =
        std::chrono::duration_cast<std::chrono::nanoseconds> (jetzt).count();
    const std::int64_t margeNs =
        (std::int64_t) nakama::ipc::kHeartbeatTaktMs * 1000000LL;
    constexpr std::int64_t kMaxNs = std::numeric_limits<std::int64_t>::max();
    // Der Platz, der nach Jetzt und Marge bis zum Rand bleibt. `jetztNs` ist
    // die Laufzeit seit dem Systemstart und liegt viele Groessenordnungen
    // unter dem Rand; die Klammer schuetzt trotzdem, damit die Summe unten
    // unter keinen Umstaenden ueberlaeuft (UB).
    const std::int64_t platzNs =
        (jetztNs >= kMaxNs - margeNs) ? 0 : (kMaxNs - margeNs - jetztNs);
    std::int64_t tailNs = 0;
    if (tailSamples > 0)
    {
        const double ns = sekunden * 1e9;
        // Nicht-endlich saettigt NACH OBEN, nicht auf 0. Eine winzige (aber
        // formal gueltige) Rate laesst `sekunden` nach `+inf` laufen; ein
        // stiller Sturz auf 0 hiesse „kein Nachlauf" und liesse das einmalige
        // `false` sofort reisen - der teure Fehler liegt nach §34.2 genau auf
        // dieser Seite.
        //
        // `>=` statt `>`: der Vergleich laeuft in `double`, und
        // `(double) platzNs` rundet nach oben. Ein Gleichstand nimmt deshalb
        // den gesaettigten Zweig, nie die Umwandlung am Rand.
        tailNs = (! std::isfinite (ns) || ns >= (double) platzNs)
            ? platzNs : (std::int64_t) std::max (ns, 0.0);
    }
    const std::int64_t neu = jetztNs + tailNs + margeNs;
    // 🔑 Nur verlaengern. Eine bereits abgelaufene Frist liegt in der
    // Vergangenheit und verliert diesen Vergleich von selbst; eine noch
    // offene laengere bleibt stehen — genau wie ihr Gegenstueck beim Broker.
    if (neu > nachlaufFristNs.load (std::memory_order_relaxed))
        nachlaufFristNs.store (neu, std::memory_order_relaxed);
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
