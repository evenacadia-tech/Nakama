// SONDE-011 Phase B / NAK-13 — Brokerstart ausserhalb des Audiothreads.
//
// Der Worker entscheidet nicht, WELCHES Bundle starten darf. Diese Wahrheit
// bleibt beim vorhandenen `state::Lebenslauf::darfBrokerStarten()` und wird als
// Callback injiziert. Ebenso kommen Pipe-, Installations- und Mutexname aus
// der duennen Produktzielschicht; dieser Kern erfindet keine Identitaet.
#pragma once

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

namespace nakama::ipc
{

inline constexpr std::uint64_t SPAWN_CONNECT_BACKOFF_START_MS = 250;
inline constexpr std::uint64_t SPAWN_BEREIT_TIMEOUT_MS = 10000;
inline constexpr std::uint64_t SPAWN_COOLDOWN_MS = 30000;
inline constexpr std::uint64_t BROKER_IDLE_ENDE_MS = 60000;
inline constexpr unsigned BROKER_PRO_USER_MAX = 1;
inline constexpr unsigned AUTOSTART_ARTEFAKTE_PHASE_B = 0;
inline constexpr unsigned AUDIO_THREAD_BROKER_OPERATIONEN_MAX = 0;

bool spawnRetryFaellig (std::uint64_t alterMs) noexcept;
bool spawnBereitTimeoutAbgelaufen (std::uint64_t alterMs) noexcept;
bool spawnCooldownAbgelaufen (std::uint64_t alterMs) noexcept;
bool brokerIdleEndeErreicht (std::uint64_t idleAlterMs,
                             unsigned aktiveClients) noexcept;

enum class BrokerPruefFehler
{
    keiner,
    pfadNichtAbsolut,
    dateiNichtLesbar,
    erwarteterHashUngueltig,
    hashFalsch,
    thumbprintUngueltig,
    signaturFehltOderUngueltig,
    signerFalsch
};

struct BrokerPruefBericht
{
    BrokerPruefFehler fehler = BrokerPruefFehler::keiner;
    bool hashGeprueft = false;
    bool signaturGeprueft = false;
    std::string dateiSha256;
    std::string signerThumbprint;

    bool ok() const noexcept { return fehler == BrokerPruefFehler::keiner; }
};

// Der erwartete Hash und der optionale Thumbprint werden beim CMake-Lauf aus
// `nakama-installer-v1.json` in die Produktzielschicht gebunden. Ein leerer
// Thumbprint repraesentiert dort das heutige JSON-null und erlaubt NUR den
// passenden SHA-256-Mindestschutz.
BrokerPruefBericht brokerBinaryPruefen (const std::wstring& absoluterPfad,
                                        const std::string& erwarteterSha256,
                                        const std::string& authenticodeThumbprint);

bool brokerVerborgenStarten (const std::wstring& absoluterPfad);

struct BrokerLifecycleHooks
{
    std::function<bool()> verbunden;
    std::function<bool()> connectFehlgeschlagen;
    std::function<bool()> darfStarten;
    std::function<void()> reconnect;
    std::function<BrokerPruefBericht()> pruefen;
    std::function<bool()> spawn;
    std::wstring mutexName;
    std::string pipeName;
};

class BrokerLifecycle
{
public:
    struct Snapshot
    {
        std::uint64_t pruefungen = 0;
        std::uint64_t spawnVersuche = 0;
        std::uint64_t spawnErfolge = 0;
        std::uint64_t connectRetriesNachSpawn = 0;
        std::uint64_t cooldowns = 0;
        std::uint64_t mutexVerloren = 0;
        bool wartetAufBereit = false;
        bool imCooldown = false;
        BrokerPruefFehler letzterPrueffehler = BrokerPruefFehler::keiner;
    };

    explicit BrokerLifecycle (BrokerLifecycleHooks hooks);
    ~BrokerLifecycle();

    BrokerLifecycle (const BrokerLifecycle&) = delete;
    BrokerLifecycle& operator= (const BrokerLifecycle&) = delete;

    void start();
    void stop();
    Snapshot snapshot() const;

    // Deterministischer Schritt fuer B8/B10. Der Produktthread ruft exakt
    // dieselbe Zustandsmaschine mit `steady_clock` auf.
    void tickFuerTest (std::uint64_t jetztMs);

private:
    enum class Phase { wartetAufConnect, bereit, wartetAufBroker, cooldown };

    void threadLauf();
    void tick (std::uint64_t jetztMs);
    bool startMutexNehmen();
    void startMutexFreigeben();

    BrokerLifecycleHooks hooks;
    mutable std::mutex zustandMutex;
    Snapshot zustand;
    Phase phase = Phase::wartetAufConnect;
    std::uint64_t spawnZeitMs = 0;
    std::uint64_t naechsterRetryMs = 0;
    std::uint64_t retryAbstandMs = SPAWN_CONNECT_BACKOFF_START_MS;
    std::uint64_t cooldownSeitMs = 0;
    void* startMutex = nullptr;

    std::atomic<bool> laeuft { false };
    std::mutex warteMutex;
    std::condition_variable warte;
    std::thread thread;
};

// Messhaken fuer A-07. Er zaehlt ausschliesslich Broker-Lifecycle-Operationen,
// die auf dem vom Test markierten Audiothread stattfinden. Der Produktpfad
// markiert keinen Thread; die Wache kostet dort nur einen thread_local-Read.
void brokerLifecycleAudioTestBeginn() noexcept;
void brokerLifecycleAudioTestEnde() noexcept;
void brokerLifecycleAudioTestZaehlerLoeschen() noexcept;
std::uint64_t brokerLifecycleOperationenImAudiothread() noexcept;

} // namespace nakama::ipc
