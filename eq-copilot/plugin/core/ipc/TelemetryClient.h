// SONDE-010 — v3-Telemetry-Client (P2), Entwurf §33.1, §53.9.
//
// Die zweite der beiden gekoppelten Verbindungen. Sie traegt ausschliesslich
// verlusttolerante Featureframes; ihr Ausfall degradiert die Analyse, nie die
// Steuerung — genau dafuer sind es zwei Verbindungen und nicht eine.
//
// Kopplung: dieser Client verbindet erst, wenn der `ControlClient` `link_id`
// und `challenge` aus seinem `welcome` hat. Er erfindet sie NIE; ein
// ungekoppelter Telemetry-Connect wird vom Broker geschlossen (§53.9), und ein
// Client, der es trotzdem versucht, wuerde nur Verbindungsslots verbrennen.
//
// Uebergabe: `veroeffentlichen()` schreibt in eine VORALLOKIERTE SPSC-
// Schleuse (`P2Schleuse`) — ohne Lock, ohne Allokation, ohne Warten. Damit
// darf der erzeugende Worker sie direkt fuellen, und die Regel aus §48.1
// ("Audio → Worker ausschliesslich ueber vorallokierte SPSC-Strukturen")
// bricht an dieser Grenze nicht.
#pragma once

#include "ControlClient.h"
#include "IpcQueues.h"
#include "IpcVerbindung.h"
#include "WireEnvelope.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace nakama::ipc
{

struct TelemetryHello
{
    Adresse     adresse;
    std::string pluginVersion = "0.3.0";
    /// Aus dem `welcome` der Control-Verbindung. Leer ⇒ es wird nicht
    /// verbunden.
    std::string linkId;
    std::string challenge;
};

class TelemetryClient
{
public:
    enum class Status { getrennt, wartetAufKopplung, verbindet, verbunden };

    struct Snapshot
    {
        Status        status = Status::getrennt;
        std::string   letzterFehler;
        int           verbindungsVersuche = 0;
        std::uint64_t gesendet = 0;
        std::uint64_t ersetzt = 0;      ///< aelteste ungesendete Frames (Cap 2)
        std::uint64_t zuGross = 0;      ///< Frames ueber der Slotgroesse
        /// Wie oft der Erzeuger auf den Platz lief, den der Telemetriethread
        /// gerade beansprucht hatte, und die Position deshalb uebersprang.
        std::uint64_t kollisionsLoecher = 0;
        /// Wie oft dabei GAR KEIN Platz zu holen war und der neue Frame fiel.
        /// Muss bei einem Verbraucher 0 bleiben — die Zahl ist die Wache
        /// darueber, dass replace-oldest nie zu replace-newest wird.
        std::uint64_t beanspruchtVerworfen = 0;
        std::uint64_t envelopeAbweisungen = 0;
        /// Broker→Main-Liveupdates, die auf dieser Verbindung ankamen. Sie
        /// sind vertragsgemaess (§33.1), haben in diesem Ticket aber noch
        /// keinen Verbraucher — die Landkarte ist `SONDE-012`. Die Zahl macht
        /// sichtbar, dass sie verworfen werden.
        std::uint64_t empfangen = 0;
        /// P0/P1 auf der Telemetrieverbindung — vertragswidrig, schliesst.
        std::uint64_t familieAbweisungen = 0;
        /// Wie oft die Nachrichtenratengrenze (§33.1) die Verbindung beendet
        /// hat.
        std::uint64_t rateAbweisungen = 0;
        /// Wie oft die Control-Verbindung neue Kopplungswerte hatte und diese
        /// Verbindung deshalb neu gekoppelt hat.
        std::uint64_t kopplungswechsel = 0;
    };

    TelemetryClient (std::function<TelemetryHello()> helloProvider, std::string pipeName);
    ~TelemetryClient();

    TelemetryClient (const TelemetryClient&) = delete;
    TelemetryClient& operator= (const TelemetryClient&) = delete;

    void start();
    void stop();
    void reconnect();

    /// Erzeugerseite: allokationsfrei, lockfrei, wartefrei. Bei vollem Puffer
    /// weicht der AELTESTE ungesendete Frame (§53.9, Cap 2).
    bool veroeffentlichen (const std::uint8_t* daten, std::size_t laenge) noexcept;

    Snapshot snapshot() const;

private:
    void threadLauf();
    bool eineVerbindung (std::uint64_t generation, const TelemetryHello& hello);
    /// Leerlauf einer stehenden Verbindung: LIEST mit Frist, statt zu
    /// schlafen. `false` ⇒ die Verbindung ist zu beenden (Pipe zu, Envelope
    /// abgelehnt, falsche Familie oder Ratengrenze).
    bool leerlaufLesen (StromLeser& leser, Ratengrenze& rate,
                        std::chrono::steady_clock::time_point rateBeginn,
                        std::uint64_t generation);
    bool sollAbbrechen (std::uint64_t generation) const noexcept;

    std::function<TelemetryHello()> helloProvider;
    std::string pipeName;

    IpcVerbindung verbindung;
    P2Schleuse<8192> schleuse;

    std::mutex   lebenslaufMutex;
    std::thread  thread;
    std::atomic<bool> laeuft { false };
    std::atomic<std::uint64_t> verbindungsGeneration { 0 };
    std::mutex   wartemutex;
    std::condition_variable warte;

    mutable std::mutex zustandMutex;
    Snapshot zustand;
};

} // namespace nakama::ipc
