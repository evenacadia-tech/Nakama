#include "PipeClient.h"
#include "EqCopilotIds.h"
#include "../vertrag/NakamaUtf8.h"

#include <cmath>
#include <cstring>
#include <initializer_list>
#include <limits>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace eqcop
{

namespace
{
class WinHandle
{
public:
    explicit WinHandle (HANDLE hIn) noexcept : h (hIn) {}
    ~WinHandle() { if (h != nullptr && h != INVALID_HANDLE_VALUE) CloseHandle (h); }
    WinHandle (const WinHandle&) = delete;
    WinHandle& operator= (const WinHandle&) = delete;
    HANDLE get() const noexcept { return h; }
private:
    HANDLE h = nullptr;
};

std::chrono::milliseconds begrenzeIoTimeout (std::chrono::milliseconds timeout) noexcept
{
    using Rep = std::chrono::milliseconds::rep;
    constexpr auto groesstesEndlichesWait = static_cast<Rep> (INFINITE - 1u);
    const auto ms = timeout.count();
    return std::chrono::milliseconds {
        ms < Rep { 1 } ? Rep { 1 }
                       : (ms > groesstesEndlichesWait ? groesstesEndlichesWait : ms)
    };
}

bool ganzzahl64 (const juce::var& v, juce::int64& aus) noexcept
{
    if (v.isInt())
    {
        aus = static_cast<int> (v);
        return true;
    }
    if (v.isInt64())
    {
        aus = static_cast<juce::int64> (v);
        return true;
    }
    if (v.isDouble())
    {
        // JSON Schema bewertet auch 2.0 als integer. Vor dem Cast muessen
        // Endlichkeit, Ganzzahligkeit und der exakt darstellbare Castbereich
        // feststehen; andernfalls waere double->int64 undefiniert.
        const auto d = static_cast<double> (v);
        constexpr double oberhalbInt64Max = 9223372036854775808.0; // 2^63
        if (std::isfinite (d) && std::trunc (d) == d
            && d >= static_cast<double> (std::numeric_limits<juce::int64>::min())
            && d < oberhalbInt64Max)
        {
            aus = static_cast<juce::int64> (d);
            return true;
        }
    }
    return false;
}

bool hatNurFelder (const juce::var& objekt,
                    std::initializer_list<const char*> erlaubteFelder)
{
    const auto* dynamisch = objekt.getDynamicObject();
    if (dynamisch == nullptr)
        return false;

    const auto& eigenschaften = dynamisch->getProperties();
    for (int i = 0; i < eigenschaften.size(); ++i)
    {
        const auto name = eigenschaften.getName (i).toString();
        bool erlaubt = false;
        for (const auto* feld : erlaubteFelder)
        {
            if (name == feld)
            {
                erlaubt = true;
                break;
            }
        }
        if (! erlaubt)
            return false;
    }
    return true;
}

bool hatFeld (const juce::var& objekt, const char* name)
{
    if (const auto* dynamisch = objekt.getDynamicObject())
        return dynamisch->hasProperty (juce::Identifier (name));
    return false;
}

bool stringFeld (const juce::var& objekt, const juce::Identifier& name,
                 juce::String& aus, int maxZeichen, bool darfLeerSein)
{
    const auto v = objekt.getProperty (name, {});
    if (! v.isString())
        return false;
    aus = v.toString();
    return (darfLeerSein || aus.isNotEmpty()) && aus.length() <= maxZeichen;
}

juce::int64 wireZaehler (juce::uint64 wert) noexcept
{
    constexpr auto maximum = static_cast<juce::uint64> (
        std::numeric_limits<juce::int64>::max());
    return static_cast<juce::int64> (wert > maximum ? maximum : wert);
}
} // namespace

// ── JSON-Helfer ─────────────────────────────────────────────────────────────
static juce::String baueHello (const HelloInfo& h)
{
    auto* sensor = new juce::DynamicObject();
    sensor->setProperty ("sensor_id", h.sensorId);
    if (h.instanceNonce.isNotEmpty())
        sensor->setProperty ("instance_nonce", h.instanceNonce);
    sensor->setProperty ("role", h.role);
    sensor->setProperty ("label", h.label);
    sensor->setProperty ("pair_id", h.pairId.isEmpty() ? juce::var() : juce::var (h.pairId));

    auto* audio = new juce::DynamicObject();
    audio->setProperty ("samplerate", h.samplerate);
    audio->setProperty ("block_size", h.blockSize);
    audio->setProperty ("channels", h.channels);

    auto* wurzel = new juce::DynamicObject();
    wurzel->setProperty ("type", "hello");
    wurzel->setProperty ("protocol_version", kProtocolVersion);
    wurzel->setProperty ("plugin_version", kPluginVersion);
    wurzel->setProperty ("host_pid", (juce::int64) GetCurrentProcessId());
    wurzel->setProperty ("sensor", juce::var (sensor));
    wurzel->setProperty ("audio", juce::var (audio));
    return juce::JSON::toString (juce::var (wurzel), true);
}

static juce::String baueHeartbeat (const juce::String& token, juce::int64 seq,
                                   const StatsSnapshot& s, const MessKompakt& mk)
{
    auto* stats = new juce::DynamicObject();
    stats->setProperty ("rms_l", s.rmsL);
    stats->setProperty ("rms_r", s.rmsR);
    // JUCEs JSON-Ganzzahl ist i64, der interne Dropzaehler u64. Ein nackter
    // Cast oberhalb INT64_MAX waere implementationsabhaengig und verletzt die
    // gemeinsame Wire-Grenze; der kumulative Zaehler saettigt deshalb dort.
    stats->setProperty ("frames_dropped", wireZaehler (s.framesDropped));
    stats->setProperty ("nan_seen", s.nanSeen);
    stats->setProperty ("transport_playing", s.hasTransport ? juce::var (s.transportPlaying) : juce::var());
    stats->setProperty ("project_time_samples", s.projectTimeValid ? juce::var (s.projectTimeSamples) : juce::var());

    auto* wurzel = new juce::DynamicObject();
    wurzel->setProperty ("type", "heartbeat");
    wurzel->setProperty ("session_token", token);
    wurzel->setProperty ("seq", seq);
    wurzel->setProperty ("stats", juce::var (stats));

    if (mk.vorhanden)
    {
        // null heißt »nicht gültig/nicht messbar«, nie 0 (eq-ipc measurement).
        auto zahl = [] (double v, bool gueltig)
        {
            return (gueltig && std::isfinite (v)) ? juce::var (v) : juce::var();
        };
        auto* mess = new juce::DynamicObject();
        mess->setProperty ("zustand", mk.zustand);
        mess->setProperty ("metrics_version", mk.metricsVersion);
        mess->setProperty ("hoermarkierung", mk.hoermarkierung);
        mess->setProperty ("aktiv_s", mk.aktivS);
        mess->setProperty ("gesamt_s", mk.gesamtS);
        mess->setProperty ("lufs_i", zahl (mk.lufsI, mk.lufsGueltig));
        mess->setProperty ("lufs_s", zahl (mk.lufsS, mk.lufsShortGueltig));
        mess->setProperty ("true_peak_db", zahl (mk.truePeakDb, mk.truePeakGueltig));
        mess->setProperty ("crest_db", zahl (mk.crestDb, mk.crestGueltig));
        mess->setProperty ("centroid_hz", zahl (mk.centroidHz, mk.spektralGueltig));
        mess->setProperty ("low_frac", zahl (mk.lowFrac, mk.spektralGueltig));
        mess->setProperty ("width", zahl (mk.width, mk.stereoGueltig));
        mess->setProperty ("corr", zahl (mk.corr, mk.stereoGueltig));
        if (! mk.ltasKompositDb.empty())
        {
            juce::Array<juce::var> ltas;
            ltas.ensureStorageAllocated ((int) mk.ltasKompositDb.size());
            for (const double v : mk.ltasKompositDb)
                ltas.add (zahl (v, true));            // NaN ⇒ null
            mess->setProperty ("ltas_komposit_db", ltas);
        }
        if (mk.fensterGueltig)
        {
            auto* fenster = new juce::DynamicObject();
            fenster->setProperty ("von_samples", mk.fensterVon);
            fenster->setProperty ("bis_samples", mk.fensterBis);
            fenster->setProperty ("spruenge", (juce::int64) mk.fensterSpruenge);
            mess->setProperty ("projekt_fenster", juce::var (fenster));
        }
        wurzel->setProperty ("measurement", juce::var (mess));
    }
    return juce::JSON::toString (juce::var (wurzel), true);
}

// ── Lebenslauf ──────────────────────────────────────────────────────────────
PipeClient::PipeClient (std::function<HelloInfo()> hp,
                        std::function<StatsSnapshot()> sp,
                        std::function<MessKompakt()> mp,
                        const juce::String& name,
                        std::chrono::milliseconds timeout)
    : helloProvider (std::move (hp)),
      statsProvider (std::move (sp)),
      messProvider (std::move (mp)),
      pipeName (name.isNotEmpty() ? name : juce::String (juce::CharPointer_UTF16 (kPipeName))),
      ioTimeout (begrenzeIoTimeout (timeout))
{
}

PipeClient::~PipeClient() { stop(); }

void PipeClient::start()
{
    std::lock_guard<std::mutex> lebenslauf (lebenslaufMutex);
    if (laeuft.load())
        return;
    if (thread.joinable())
        thread.join();

    laeuft.store (true);
    try
    {
        thread = std::thread ([this] { threadLauf(); });
    }
    catch (...)
    {
        laeuft.store (false);
        throw;
    }
}

void PipeClient::stop()
{
    std::lock_guard<std::mutex> lebenslauf (lebenslaufMutex);
    laeuft.store (false);
    verbindungsGeneration.fetch_add (1);
    {
        std::lock_guard<std::mutex> l (wartemutex);
        warte.notify_all();
    }
    aktiveIoAbbrechen();
    if (thread.joinable())
        thread.join();
}

void PipeClient::reconnect()
{
    verbindungsGeneration.fetch_add (1);
    {
        std::lock_guard<std::mutex> l (wartemutex);
        warte.notify_all();
    }
    aktiveIoAbbrechen();
}

PipeClient::Snapshot PipeClient::snapshot() const
{
    std::lock_guard<std::mutex> l (zustandMutex);
    return zustand;
}

void PipeClient::threadLauf()
{
    int backoffMs = kBackoffStartMs;
    while (laeuft.load())
    {
        const auto generation = verbindungsGeneration.load();
        const bool stand = eineVerbindung (generation);
        if (! laeuft.load())
            break;
        if (verbindungsGeneration.load() != generation)
        {
            backoffMs = kBackoffStartMs;
            continue;
        }
        if (stand)
            backoffMs = kBackoffStartMs;           // Verbindung stand — frisch starten
        {
            std::unique_lock<std::mutex> l (wartemutex);
            warte.wait_for (l, std::chrono::milliseconds (backoffMs),
                            [this, generation] {
                                return ! laeuft.load()
                                    || verbindungsGeneration.load() != generation;
                            });
        }
        if (verbindungsGeneration.load() != generation)
        {
            backoffMs = kBackoffStartMs;
            continue;
        }
        backoffMs = juce::jmin (backoffMs * 2, kBackoffMaxMs);
    }
}

// true, wenn eine Verbindung tatsaechlich stand (Welcome kam) — steuert nur den
// Backoff-Reset; Rueckkehr heisst immer: Verbindung ist zu.
bool PipeClient::eineVerbindung (std::uint64_t generation)
{
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::verbindet;
        zustand.verbindungsVersuche++;
        zustand.brokerVersion.clear();
        zustand.sessionToken.clear();
        zustand.protokollVersion = 0;
        zustand.konflikt = false;
        zustand.letztesAck.clear();
        zustand.heartbeatsGesendet = 0;
        zustand.heartbeatsBestaetigt = 0;
    }

    HANDLE h = CreateFileW (pipeName.toWideCharPointer(),
                            GENERIC_READ | GENERIC_WRITE,
                            0, nullptr, OPEN_EXISTING,
                            FILE_FLAG_OVERLAPPED
                                | SECURITY_SQOS_PRESENT
                                | SECURITY_IDENTIFICATION,
                            nullptr);
    if (h == INVALID_HANDLE_VALUE)
    {
        const auto fehler = (int) GetLastError();
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        if (! sollAbbrechen (generation))
            zustand.letzterFehler = "Broker nicht erreichbar (Win32 " + juce::String (fehler) + ")";
        return false;
    }

    {
        std::lock_guard<std::mutex> l (handleMutex);
        aktivesHandle = h;
    }
    bool welcomeKam = false;
    juce::String token;

    do
    {
        juce::String ioFehler;
        if (sollAbbrechen (generation)
            || ! sende (h, baueHello (helloProvider()), generation, ioFehler))
        {
            if (! sollAbbrechen (generation) && ioFehler.isNotEmpty())
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = ioFehler;
            }
            break;
        }

        juce::String antwort;
        if (! empfange (h, antwort, generation, ioFehler))
        {
            if (! sollAbbrechen (generation) && ioFehler.isNotEmpty())
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = ioFehler;
            }
            break;
        }

        const auto v = juce::JSON::parse (antwort);
        if (! v.isObject())
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = "ungueltiges JSON auf hello";
            break;
        }
        const auto typ = v.getProperty ("type", {}).toString();
        if (typ == "reject")
        {
            juce::String grund;
            juce::int64 minProtocol = 0, maxProtocol = 0;
            const bool optionaleGrenzenGueltig
                = (! hatFeld (v, "min_protocol")
                   || ganzzahl64 (v.getProperty ("min_protocol", {}), minProtocol))
               && (! hatFeld (v, "max_protocol")
                   || ganzzahl64 (v.getProperty ("max_protocol", {}), maxProtocol));
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = hatNurFelder (
                                        v, { "type", "reason", "min_protocol", "max_protocol" })
                                    && stringFeld (v, "reason", grund, 512, false)
                                    && optionaleGrenzenGueltig
                ? "Broker lehnt ab: " + grund
                : "ungueltige Reject-Antwort";
            break;
        }

        juce::int64 ausgehandelt64 = 0;
        juce::String brokerVersion;
        if (! hatNurFelder (
                v, { "type", "protocol_version", "broker_version", "session_token" })
            || typ != "welcome"
            || ! ganzzahl64 (v.getProperty ("protocol_version", {}), ausgehandelt64)
            || ausgehandelt64 != kProtocolVersion
            || ! stringFeld (v, "session_token", token, 256, false)
            || ! stringFeld (v, "broker_version", brokerVersion, 128, false))
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = "unerwartete Antwort auf hello";
            break;
        }
        if (sollAbbrechen (generation))
            break;

        // Der Broker spiegelt die ANGENOMMENE Version: nur ein v2-Broker
        // quittiert Heartbeats — auf ein v1-Welcome darf der Client nie
        // lesend warten (der alte Broker sendet nichts mehr).
        const int ausgehandelt = static_cast<int> (ausgehandelt64);
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.status = Status::verbunden;
            zustand.brokerVersion = brokerVersion;
            zustand.sessionToken = token;
            zustand.protokollVersion = ausgehandelt;
            zustand.letzterFehler.clear();
        }
        welcomeKam = true;

        juce::int64 seq = 0;
        while (! sollAbbrechen (generation))
        {
            if (seq == std::numeric_limits<juce::int64>::max())
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.letzterFehler = "Heartbeat-Sequenzgrenze erreicht";
                break;
            }
            const auto aktuelleSeq = seq++;
            const auto mk = messProvider ? messProvider() : MessKompakt();
            ioFehler.clear();
            if (! sende (h, baueHeartbeat (token, aktuelleSeq, statsProvider(), mk),
                         generation, ioFehler))
            {
                if (! sollAbbrechen (generation) && ioFehler.isNotEmpty())
                {
                    std::lock_guard<std::mutex> l (zustandMutex);
                    zustand.letzterFehler = ioFehler;
                }
                break;
            }
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.heartbeatsGesendet = seq;
            }
            if (ausgehandelt >= 2)
            {
                juce::String ackJson;
                if (! empfange (h, ackJson, generation, ioFehler))
                {
                    if (! sollAbbrechen (generation) && ioFehler.isNotEmpty())
                    {
                        std::lock_guard<std::mutex> l (zustandMutex);
                        zustand.letzterFehler = ioFehler;
                    }
                    break;
                }
                const auto ack = juce::JSON::parse (ackJson);
                juce::int64 ackSeq = -1;
                const auto konflikt = ack.getProperty ("konflikt", {});
                if (! hatNurFelder (ack, { "type", "seq", "konflikt" })
                    || ack.getProperty ("type", {}).toString() != "heartbeat_ack"
                    || ! ganzzahl64 (ack.getProperty ("seq", {}), ackSeq)
                    || ackSeq != aktuelleSeq
                    || ! konflikt.isBool())
                {
                    std::lock_guard<std::mutex> l (zustandMutex);
                    zustand.letzterFehler = "ungueltiges heartbeat_ack";
                    break;
                }
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.konflikt = static_cast<bool> (konflikt);
                zustand.letztesAck = ackJson;
                zustand.heartbeatsBestaetigt++;
            }
            std::unique_lock<std::mutex> l (wartemutex);
            warte.wait_for (l, std::chrono::milliseconds (kHeartbeatMs),
                            [this, generation] { return sollAbbrechen (generation); });
        }
    }
    while (false);

    // Kein neues best-effort-Write nach Stop/Fehler: Close ist selbst das
    // eindeutige Verbindungsende. Unter demselben Mutex wie CancelIoEx kann
    // das HANDLE weder geschlossen noch wiederverwendet werden, waehrend ein
    // anderer Thread es abbricht.
    {
        std::lock_guard<std::mutex> l (handleMutex);
        if (aktivesHandle == h)
            aktivesHandle = nullptr;
        CloseHandle (h);
    }
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        zustand.brokerVersion.clear();
        zustand.sessionToken.clear();
        zustand.protokollVersion = 0;
        zustand.konflikt = false;
        zustand.letztesAck.clear();
    }
    return welcomeKam;
}

// ── Framing: u32-LE-Länge + UTF-8-JSON (eq-ipc.schema.json) ────────────────
bool PipeClient::sende (void* handle, const juce::String& json,
                        std::uint64_t generation, juce::String& fehler)
{
    const auto utf8 = json.toRawUTF8();
    const auto len = (juce::uint32) strlen (utf8);
    if (len == 0 || len > kMaxFrameBytes)
    {
        fehler = "ausgehender Pipe-Frame ausserhalb der Grenze";
        return false;
    }

    juce::uint8 kopf[4] = { (juce::uint8) (len & 0xff), (juce::uint8) ((len >> 8) & 0xff),
                            (juce::uint8) ((len >> 16) & 0xff), (juce::uint8) ((len >> 24) & 0xff) };
    const auto deadline = std::chrono::steady_clock::now() + ioTimeout;
    return ioGenau (handle, kopf, 4, true, generation, deadline, fehler)
        && ioGenau (handle, const_cast<char*> (utf8), len, true, generation, deadline, fehler);
}

bool PipeClient::empfange (void* handle, juce::String& jsonOut,
                           std::uint64_t generation, juce::String& fehler)
{
    juce::uint8 kopf[4];
    const auto deadline = std::chrono::steady_clock::now() + ioTimeout;
    if (! ioGenau (handle, kopf, 4, false, generation, deadline, fehler))
        return false;
    const juce::uint32 len = (juce::uint32) kopf[0] | ((juce::uint32) kopf[1] << 8)
                           | ((juce::uint32) kopf[2] << 16) | ((juce::uint32) kopf[3] << 24);
    if (len == 0 || len > kMaxFrameBytes)
    {
        fehler = "eingehender Pipe-Frame ausserhalb der Grenze";
        return false;
    }

    juce::HeapBlock<char> puffer (len + 1);
    if (! ioGenau (handle, puffer.getData(), len, false, generation, deadline, fehler))
        return false;

    if (std::memchr (puffer.getData(), 0, len) != nullptr)
    {
        fehler = "eingehender Pipe-Frame enthaelt rohes NUL";
        return false;
    }
    if (len >= 3 && static_cast<unsigned char> (puffer[0]) == 0xef
                 && static_cast<unsigned char> (puffer[1]) == 0xbb
                 && static_cast<unsigned char> (puffer[2]) == 0xbf)
    {
        fehler = "eingehender Pipe-Frame beginnt mit UTF-8-BOM";
        return false;
    }
    if (len > static_cast<juce::uint32> (std::numeric_limits<int>::max())
        || ! nakama::utf8::istGueltig (puffer.getData(), len))
    {
        fehler = "eingehender Pipe-Frame ist kein gueltiges UTF-8";
        return false;
    }
    puffer[len] = 0;
    jsonOut = juce::String::fromUTF8 (puffer.getData(), (int) len);
    return true;
}

bool PipeClient::ioGenau (void* handle, void* daten, unsigned bytes, bool schreiben,
                          std::uint64_t generation, IoDeadline deadline,
                          juce::String& fehler)
{
    WinHandle ereignis (CreateEventW (nullptr, TRUE, FALSE, nullptr));
    if (ereignis.get() == nullptr)
    {
        fehler = "Pipe-I/O: CreateEvent Win32 " + juce::String ((int) GetLastError());
        return false;
    }

    auto* p = static_cast<juce::uint8*> (daten);
    DWORD rest = static_cast<DWORD> (bytes);
    while (rest > 0)
    {
        if (sollAbbrechen (generation))
            return false;
        if (std::chrono::steady_clock::now() >= deadline)
        {
            fehler = "Pipe-I/O Zeitlimit ueberschritten";
            return false;
        }

        OVERLAPPED ov {};
        ov.hEvent = ereignis.get();
        BOOL sofort = FALSE;
        {
            // Submission und externes Cancel teilen denselben Mutex. Damit
            // kann stop()/reconnect() nicht genau vor ReadFile/WriteFile
            // canceln und anschliessend eine neue, ungekannte I/O entstehen.
            std::lock_guard<std::mutex> submission (handleMutex);
            if (aktivesHandle != handle || sollAbbrechen (generation))
                return false;
            if (std::chrono::steady_clock::now() >= deadline)
            {
                fehler = "Pipe-I/O Zeitlimit ueberschritten";
                return false;
            }
            ResetEvent (ereignis.get());
            sofort = schreiben
                ? WriteFile (static_cast<HANDLE> (handle), p, rest, nullptr, &ov)
                : ReadFile  (static_cast<HANDLE> (handle), p, rest, nullptr, &ov);
        }
        if (sofort == FALSE)
        {
            const auto startFehler = GetLastError();
            if (startFehler != ERROR_IO_PENDING)
            {
                if (startFehler != ERROR_OPERATION_ABORTED || ! sollAbbrechen (generation))
                    fehler = juce::String (schreiben ? "Pipe-Schreiben" : "Pipe-Lesen")
                           + ": Win32 " + juce::String ((int) startFehler);
                return false;
            }

            const auto jetzt = std::chrono::steady_clock::now();
            const auto verbleibend = deadline > jetzt ? deadline - jetzt
                                                       : IoDeadline::duration::zero();
            auto warteMs = std::chrono::duration_cast<std::chrono::milliseconds> (verbleibend);
            if (std::chrono::duration_cast<IoDeadline::duration> (warteMs) < verbleibend)
                ++warteMs; // nicht durch Abrunden vor der absoluten Deadline abbrechen
            const auto endlichesMaximum = static_cast<long long> (INFINITE - 1u);
            const auto warteMs64 = juce::jlimit<long long> (0, endlichesMaximum,
                                                            warteMs.count());
            const auto warten = WaitForSingleObject (ereignis.get(),
                                                      static_cast<DWORD> (warteMs64));
            if (warten != WAIT_OBJECT_0)
            {
                // Die OVERLAPPED-Struktur liegt auf dem Stack und muss bis zur
                // bestaetigten Completion leben. Der Peer kontrolliert diese
                // Wartezeit nicht: CancelIoEx beendet die lokale Kernel-I/O.
                CancelIoEx (static_cast<HANDLE> (handle), &ov);
                WaitForSingleObject (ereignis.get(), INFINITE);
                DWORD verworfen = 0;
                GetOverlappedResult (static_cast<HANDLE> (handle), &ov, &verworfen, FALSE);
                if (! sollAbbrechen (generation))
                    fehler = warten == WAIT_TIMEOUT
                        ? "Pipe-I/O Zeitlimit ueberschritten"
                        : "Pipe-I/O WaitForSingleObject fehlgeschlagen";
                return false;
            }
        }

        DWORD uebertragen = 0;
        if (GetOverlappedResult (static_cast<HANDLE> (handle), &ov, &uebertragen, FALSE) == FALSE)
        {
            const auto ioFehler = GetLastError();
            if (ioFehler != ERROR_OPERATION_ABORTED || ! sollAbbrechen (generation))
                fehler = juce::String (schreiben ? "Pipe-Schreiben" : "Pipe-Lesen")
                       + ": Win32 " + juce::String ((int) ioFehler);
            return false;
        }
        if (uebertragen == 0)
        {
            fehler = juce::String (schreiben ? "Pipe-Schreiben" : "Pipe-Lesen")
                   + ": Gegenstelle geschlossen";
            return false;
        }
        if (sollAbbrechen (generation))
            return false;
        p += uebertragen;
        rest -= uebertragen;
        // Auch eine synchrone Completion oder die auf Millisekunden
        // aufgerundete Win32-Wartezeit darf das Framebudget nicht dehnen.
        if (std::chrono::steady_clock::now() > deadline)
        {
            fehler = "Pipe-I/O Zeitlimit ueberschritten";
            return false;
        }
    }
    return true;
}

bool PipeClient::sollAbbrechen (std::uint64_t generation) const noexcept
{
    return ! laeuft.load() || verbindungsGeneration.load() != generation;
}

void PipeClient::aktiveIoAbbrechen()
{
    std::lock_guard<std::mutex> l (handleMutex);
    if (aktivesHandle != nullptr)
        CancelIoEx (static_cast<HANDLE> (aktivesHandle), nullptr);
}

} // namespace eqcop
