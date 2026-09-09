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
#include "BrokerInstallBinding.h"
#include "PipeToken.h"
#include "prozessor/Intern.h"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstring>
#include <limits>

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

} // namespace eqcop
