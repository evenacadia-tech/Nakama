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
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace nakama::analyse { struct FeatureFrame; }

namespace nakama::ipc
{

/// Minor 1 fuehrt Frame.band_stereo (Feld-ID 10) sowie das optionale
/// LUFS-I-Paar und seinen Gegenstatus (Feld-IDs 11..13) ein. Der echte
/// P2-Sendepfad benutzt diese Konstante; Tests duerfen den Wert nicht am
/// Envelope vorbei selbst einsetzen.
inline constexpr std::uint8_t kFeatureBatchSchemaMinor = 1;

/// Worker-seitige, allokierende Serialisierung eines Analyseframes. Der
/// Audiothread ruft sie nicht; erst das fertige Bytefeld geht in die
/// vorallokierte SPSC-Schleuse.
bool featureFrameAlsFlatbuffer (const analyse::FeatureFrame& frame,
                                const Adresse& quelle,
                                std::vector<std::uint8_t>& ausgabe);

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
        ServerPruefStatus serverPruefstatus = ServerPruefStatus::nichtGeprueft;
        ServerPruefFehler serverPrueffehler = ServerPruefFehler::keiner;
        std::uint32_t serverPid = 0;
        std::uint64_t serverPruefungen = 0;
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
        /// Broker→Main-Liveupdates, die dem optionalen Verbraucher auf dieser
        /// Verbindung zugestellt wurden.
        std::uint64_t empfangen = 0;
        /// P0/P1 auf der Telemetrieverbindung — vertragswidrig, schliesst.
        std::uint64_t familieAbweisungen = 0;
        /// Wie oft die Nachrichtenratengrenze (§33.1) die Verbindung beendet
        /// hat.
        std::uint64_t rateAbweisungen = 0;
        /// Wie oft die Control-Verbindung neue Kopplungswerte hatte und diese
        /// Verbindung deshalb neu gekoppelt hat.
        std::uint64_t kopplungswechsel = 0;
        /// Wie oft `stop()` den Clientthread ABGELOEST hat, statt ihn zu
        /// joinen, weil ein Callback laenger als `kStopFristMs` stand
        /// (Matrix `B-TC-07`). `stop()` kehrt trotzdem zurueck.
        std::uint64_t stopFristUeberschritten = 0;
    };

    TelemetryClient (std::function<TelemetryHello()> helloProvider,
                     std::string pipeName,
                     std::function<void (const std::uint8_t*, std::size_t,
                                         std::uint8_t schemaMinor)> beiFrame = {},
                     ServerErwartung serverErwartung = serverErwartungFuerEigenprozessTest());
    ~TelemetryClient();

    TelemetryClient (const TelemetryClient&) = delete;
    TelemetryClient& operator= (const TelemetryClient&) = delete;

    void start();

    /// Kehrt IMMER zurueck (Matrix `B-TC-07`, `B-TC-09`, Regel 6) — in JEDEM
    /// Zustand: `wartetAufKopplung`, `verbindend`, `verbunden` und
    /// rueckstauend mit blockierendem P2-Write. Aus dem `helloProvider` heraus
    /// ohne Self-Join, von aussen mit der Frist `kStopFristMs`; laeuft sie ab,
    /// wird der Thread abgeloest und `Snapshot::stopFristUeberschritten`
    /// waechst. Danach wird kein Callback mehr gerufen.
    void stop();
    void reconnect();

    /// Erzeugerseite: allokationsfrei, lockfrei, wartefrei. Bei vollem Puffer
    /// weicht der AELTESTE ungesendete Frame (§53.9, Cap 2).
    bool veroeffentlichen (const std::uint8_t* daten, std::size_t laenge) noexcept;

    /// Worker-Pfad: serialisiert den vollstaendigen FeatureFrame inklusive
    /// optionalem band_stereo und veroeffentlicht danach die Bytes.
    bool veroeffentlichen (const analyse::FeatureFrame& frame, const Adresse& quelle);

    Snapshot snapshot() const;

private:
    /// Wie beim `ControlClient`: alles, was der Clientthread anfasst, liegt
    /// GETEILT hinter einem `shared_ptr`. Ein nach `kStopFristMs` abgeloester
    /// Thread darf den Client danach nicht mehr beruehren.
    struct Laufzeit;
    std::shared_ptr<Laufzeit> k;

    std::mutex   lebenslaufMutex;
    std::thread  thread;
};

} // namespace nakama::ipc
