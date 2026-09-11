// ControlClient::Laufzeit — die geteilte Laufzeit als interner Kopf.
//
// NAK-225/S25d (09.09.2026): Der Pimpl-Rumpf stand im Urstand mitten in
// ControlClient.cpp. Beim Aufteilen nach Fachbereich definieren drei
// Uebersetzungseinheiten Member von ControlClient::Laufzeit; sie brauchen dafuer
// die vollstaendige Deklaration, nicht nur die Vorwaertsdeklaration aus
// ControlClient.h. Deshalb steht sie hier.
//
// Der Kopf ist bewusst intern: Laufzeit ist der private Zustand des Clients,
// und die Regel aus ControlClient.h bleibt unveraendert - der Client selbst
// haelt nur den shared_ptr darauf, seinen Thread und den Lebenslaufmutex
// (Regel 6, B-CC-12).
//
// Wer Member von Laufzeit definiert, bindet diesen Kopf ein; wer nur den
// Client benutzt, sieht ihn nie.

#pragma once

#include "ControlClient.h"
#include "Intern.h"

namespace nakama::ipc
{

// Der Rumpf misst die Stoppfrist; sie kommt aus dem internen Kopf.
using controlclient_intern::kStopFristMs;

//== Die geteilte Laufzeit ===================================================
//
// Alles, was der Clientthread anfasst. Der Client selbst haelt nur noch den
// `shared_ptr` darauf, seinen Thread und den Lebenslaufmutex — die Begruendung
// steht im Header (Regel 6, `B-CC-12`).
struct ControlClient::Laufzeit
{
    Laufzeit (std::function<ControlHello()> hp,
              std::string pn,
              std::function<void (const std::string&)> ba,
              std::function<ControlStatus()> sp,
              std::function<void (bool)> bl,
              std::function<void (const std::string&, std::uint8_t)> bva,
              ServerErwartung se)
        : helloProvider (std::move (hp)), beiAntwort (std::move (ba)),
          statusProvider (std::move (sp)), beiLinkStatus (std::move (bl)),
          beiVersionierterAntwort (std::move (bva)),
          pipeName (std::move (pn)), serverErwartung (std::move (se)) {}

    void threadLauf (std::uint64_t meinLauf, std::shared_ptr<IpcVerbindung> meine);
    bool eineVerbindung (std::uint64_t generation, std::uint64_t meinLauf,
                         IpcVerbindung& verbindung);
    bool sollAbbrechen (std::uint64_t generation) const noexcept;
    /// Ist dieser Lauf abgeloest? Ein abgeloester Lauf schreibt KEINEN
    /// gemeinsamen Zustand mehr: sein `getrennt` waere sonst eine Aussage
    /// ueber einen Lauf, den es nicht mehr gibt — und der neue steht
    /// womoeglich gerade verbunden da (`B-CC-12`, NAK-104).
    bool abgeloest (std::uint64_t meinLauf) const noexcept
    { return lebenslauf.load() != meinLauf; }
    bool sendeP0 (const std::string& json, P0Klasse klasse, std::uint64_t marke);
    /// NAK-246 D4: dreiwertig (Vertrag in ControlClient.h); der Deckel des
    /// Registers ist `kCapP0` aus IpcQueues.h.
    PersistenzP0Ergebnis sendePersistenzP0 (const std::string& json);
    P1Ergebnis sendeP1 (const std::string& schluessel, const std::string& json);
    Snapshot snapshotIntern() const;
    bool kopplung (std::string& linkId, std::string& challenge) const;
    void meldeLinkStatus (bool verbunden);
    void aufbauAussageZurueckstellen (std::uint64_t marke);
    std::uint64_t aufbauZug();
    bool heartbeatSchritt (const ControlHello& hello, std::uint64_t sequence,
                           const ControlStatus& status,
                           std::uint64_t dieseWireGeneration,
                           std::string* textAus);
    // NAK-180 R7/R12: Rueckmeldung und Zustellpruefung. Alle drei laufen
    // unter `sendeMutex`; sie duerfen den Sendezustand des Prozessors nehmen,
    // aber NIE erneut senden (Sperrenordnung: sendeMutex vor Sendezustand).
    std::function<void (std::uint64_t, std::uint64_t)> beiP0Zugestellt;
    std::function<void (std::uint64_t)> beiP0Verworfen;
    /// NAK-180 Nacharbeit 2 (WN-06/N-35): Schranke am Eintritt des Aufbauzugs.
    /// Phase 0 vor dem Anfordern von `sendeMutex`, Phase 1 nach der Uebernahme.
    std::function<void (int)> aufbauZugHakenFuerTest;
    std::function<std::string (std::uint64_t, std::uint64_t)> hookReplayBegin;
    /// SONDE-014 WN-01: der Weg zurueck aus einem `konflikt`-ACK. Laeuft
    /// unter `sendeMutex`, liest und formt nur.
    std::function<std::string (const std::string&, const std::string&, std::uint64_t)>
        hookKonfliktWiederholung;
    /// SONDE-014 WN3-01: der Auftrag ist ABGESCHLOSSEN - angewandt,
    /// idempotent wiederholt oder endgueltig ohne Erfolg. Wer sich zu einer
    /// `command_id` etwas gemerkt hat, gibt es hier frei. Laeuft OHNE
    /// `sendeMutex`, wie `beiP0Verworfen`.
    std::function<void (const std::string&)> hookAuftragAbgeschlossen;
    /// SONDE-014 WN-01: die Revision des zuletzt EINGEREIHTEN `state_report`
    /// und die des zuletzt auf den Draht GESCHRIEBENEN. Erst der Draht zaehlt:
    /// ein eingereihter Bericht steht hinter jedem P0, das vor ihm entnommen
    /// wird.
    std::atomic<std::uint64_t> stateReportRevisionEingereiht { 0 };
    std::atomic<std::uint64_t> stateReportRevisionGemeldet { 0 };

    // 🔑 NAK-180 R1/R10/R13: die Aussage des Aufbaus - als GENERATIONSZAHL,
    // nicht als Bit.
    //
    // Ein Bit haette beim Linkende umgeschrieben werden muessen, und ob das
    // rechtzeitig geschieht, haengt an der Reihenfolge zweier Threads: der
    // negative Callback laeuft synchron auf dem Aufruferthread von
    // `reconnect()`/`stop()`, waehrend der positive noch steht (MP3-1). Eine
    // Zahl muss nur VERGLICHEN werden und veraltet von selbst. `0` = keine
    // Aussage; jeder Zugriff ist ein CAS auf den beobachteten Wert (MP4-2:
    // ein blindes `store(0)` war ein Lost-Update).
    std::atomic<std::uint64_t> neutralerNeuaufbau { 0 };
    std::atomic<std::uint64_t> nichtNeutralerNeuaufbau { 0 };
    /// Fortlaufende Rueckmeldekennung fuer P0-Eintraege (0 bleibt frei).
    std::atomic<std::uint64_t> p0MarkenFolge { 0 };
    /// Marke des Aufbau-Heartbeats und die Aussage, die er mitgenommen hat -
    /// damit ein Verwurf sie zurueckstellen kann (N-12/N-25).
    std::atomic<std::uint64_t> aufbauHeartbeatMarke { 0 };
    std::atomic<std::uint64_t> aufbauHeartbeatGeneration { 0 };
    std::atomic<bool>          aufbauHeartbeatWarNeutral { false };
    /// 🔑 Nacharbeit 1 (EP-06/N-12): BEIDE Flags koennen von demselben
    /// Heartbeat verbraucht worden sein. Ein Verwurf stellt beide zurueck -
    /// eine Aussage, die nur zur Haelfte zurueckkommt, waere dieselbe
    /// verlorene Aussage in kleiner.
    std::atomic<bool>          aufbauHeartbeatWarUnbekannt { false };

    std::function<ControlHello()> helloProvider;
    std::function<void (const std::string&)> beiAntwort;
    std::function<ControlStatus()> statusProvider;
    std::function<void (bool)> beiLinkStatus;
    std::function<void (const std::string&, std::uint8_t)> beiVersionierterAntwort;
    std::atomic<bool> linkAlsVerbundenGemeldet { false };
    /// 🔑 NAK-180 Nacharbeit 1 (EP-05): die Generation, fuer die der Link als
    /// VERBUNDEN gemeldet wurde. `meldeLinkStatus` serialisiert den
    /// Statuswechsel per `exchange`; damit setzt genau ein Aufrufer diese Zahl
    /// und genau einer verbraucht sie beim Wechsel nach `false`. Der negative
    /// Callback bekommt so die STERBENDE Generation, nicht die aktuelle.
    std::atomic<std::uint64_t> gemeldeteLinkGeneration { 0 };
    std::string pipeName;
    ServerErwartung serverErwartung;
    std::atomic<std::uint64_t> heartbeatFolge { 0 };

    /// Die Verbindung des LAUFENDEN Laufs. Jeder `start()` legt eine eigene an.
    ///
    /// Eine gemeinsame Verbindung je Laufzeit reichte nicht: ein nach
    /// `kStopFristMs` ABGELOESTER Thread lebt weiter, bis sein Callback
    /// zurueckkommt. Startet der Client bis dahin erneut, endet der alte Lauf
    /// zwar (`lebenslauf`) — er erreicht aber noch das unbedingte
    /// `verbindung.schliessen()` am Ende von `eineVerbindung` und trennte damit
    /// die Pipe des NEUEN Laufs (NAK-104, Pruefbefund vom 2026-08-30). Und die
    /// Pruefung `lebenslauf` allein waere ohnehin nur ein Check-then-use: der
    /// alte Lauf haelt zwischen Pruefung und Zugriff Lese- und Schreibwege auf
    /// derselben Pipe.
    ///
    /// Mit einer eigenen Verbindung je Lauf beruehrt ein abgeloester Lauf
    /// ausschliesslich SEINE — der neue bleibt unangetastet. Der Zeiger wird
    /// nie null: `stop()` vor dem ersten `start()` findet eine geschlossene,
    /// gueltige Verbindung vor.
    mutable std::mutex verbindungMutex;
    /// NICHT direkt benutzen: `eineVerbindung` bekommt die Verbindung SEINES
    /// Laufs als Parameter. Der eigene Name macht sichtbar, dass beides nicht
    /// dasselbe ist.
    std::shared_ptr<IpcVerbindung> laufendeVerbindung = std::make_shared<IpcVerbindung>();

    std::shared_ptr<IpcVerbindung> aktuelleVerbindung() const
    {
        std::lock_guard<std::mutex> l (verbindungMutex);
        return laufendeVerbindung;
    }

    std::shared_ptr<IpcVerbindung> neueVerbindung()
    {
        auto frisch = std::make_shared<IpcVerbindung>();
        std::lock_guard<std::mutex> l (verbindungMutex);
        laufendeVerbindung = frisch;
        return frisch;
    }

    std::atomic<bool> laeuft { false };
    /// Der Thread hat `threadLauf()` verlassen. `stop()` wartet darauf, statt
    /// blind zu joinen — nur so kann es nach der Frist abloesen.
    std::atomic<bool> fertig { false };
    /// Wer ist der Clientthread? `stop()` aus einem Callback heraus liefe
    /// sonst in einen Self-Join (`B-CC-11`).
    std::atomic<std::thread::id> threadId {};
    /// Welcher Lauf ist das? Ein nach `kStopFristMs` ABGELOESTER Thread lebt
    /// weiter, bis sein Callback zurueckkommt. Startet der Client bis dahin
    /// erneut, saehe der alte Thread `laeuft == true` und liefe auf DERSELBEN
    /// Laufzeit weiter — zwei Threads auf einer Pipe. Jeder Lauf traegt
    /// deshalb seine Nummer und endet, sobald sie nicht mehr die aktuelle ist.
    std::atomic<std::uint64_t> lebenslauf { 0 };
    std::atomic<std::uint64_t> verbindungsGeneration { 0 };
    /// Anders als `verbindungsGeneration` (Abbruchsignal) waechst diese Zahl
    /// bei JEDEM erfolgreichen Pipe-Neuaufbau. Ein ungeplanter Broker-Kill
    /// aendert das Abbruchsignal nicht, muss In-Flight-Auftraege aber trotzdem
    /// erneut senden.
    std::atomic<std::uint64_t> wireGeneration { 0 };
    std::mutex   wartemutex;
    std::condition_variable warte;

    mutable std::mutex zustandMutex;
    Snapshot zustand;

    std::mutex sendeMutex;
    P0Warteschlange p0;
    P1Warteschlange p1;
    struct InFlightEintrag
    {
        std::string commandId;
        std::string json;
        std::uint64_t gesendetInGeneration = 0;
        bool inQueue = true;
        /// SONDE-014 WN-01: wie oft dieser Auftrag nach einem `konflikt`-ACK
        /// mit frischem Kopf wiederholt wurde. Ohne Deckel waere die
        /// Wiederholung eine Schleife.
        int konfliktWiederholungen = 0;
    };
    /// SONDE-014 WN-01: hoechstens so viele frische Koepfe je Auftrag.
    static constexpr int kKonfliktWiederholungenMax = 3;
    std::vector<InFlightEintrag> inFlight;

    void inFlightNachReconnect (std::uint64_t generation);
    void inFlightNachWireWrite (const std::string& json, std::uint64_t generation);
    void inFlightAck (const std::string& json);
    /// Monoton wachsender Zaehler der P0-Ueberlaeufe. Die laufende Verbindung
    /// merkt sich seinen Stand beim Verbinden und schliesst, sobald er waechst
    /// (§53.9 "nichts verwerfen; Verbindung schliessen"). Ein Ueberlauf, der
    /// VOR der Verbindung passiert ist, schliesst dagegen nichts — es gibt
    /// nichts zu schliessen, und der Aufrufer hat sein `false` bereits.
    std::atomic<std::uint64_t> p0UeberlaufZaehler { 0 };
};

} // namespace nakama::ipc
