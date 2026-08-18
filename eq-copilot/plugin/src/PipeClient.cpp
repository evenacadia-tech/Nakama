#include "PipeClient.h"
#include "EqCopilotIds.h"

#include <cmath>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

namespace eqcop
{

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
    stats->setProperty ("frames_dropped", (juce::int64) s.framesDropped);
    stats->setProperty ("nan_seen", s.nanSeen);
    stats->setProperty ("transport_playing", s.hasTransport ? juce::var (s.transportPlaying) : juce::var());
    stats->setProperty ("project_time_samples", s.hasTransport ? juce::var (s.projectTimeSamples) : juce::var());

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
                        const juce::String& name)
    : helloProvider (std::move (hp)),
      statsProvider (std::move (sp)),
      messProvider (std::move (mp)),
      pipeName (name.isNotEmpty() ? name : juce::String (juce::CharPointer_UTF16 (kPipeName)))
{
}

PipeClient::~PipeClient() { stop(); }

void PipeClient::start()
{
    if (laeuft.exchange (true))
        return;
    thread = std::thread ([this] { threadLauf(); });
}

void PipeClient::stop()
{
    if (! laeuft.exchange (false))
        return;
    {
        std::lock_guard<std::mutex> l (wartemutex);
        warte.notify_all();
    }
    // Blockierende I/O des Threads abbrechen — sonst hinge der Destruktor,
    // bis der Broker etwas sendet.
    if (auto* th = threadHandle.load())
        CancelSynchronousIo ((HANDLE) th);
    if (thread.joinable())
        thread.join();
}

void PipeClient::reconnect()
{
    neuVerbinden.store (true);
    std::lock_guard<std::mutex> l (wartemutex);
    warte.notify_all();
}

PipeClient::Snapshot PipeClient::snapshot() const
{
    std::lock_guard<std::mutex> l (zustandMutex);
    return zustand;
}

void PipeClient::threadLauf()
{
    HANDLE self = nullptr;
    DuplicateHandle (GetCurrentProcess(), GetCurrentThread(), GetCurrentProcess(),
                     &self, 0, FALSE, DUPLICATE_SAME_ACCESS);
    threadHandle.store (self);

    int backoffMs = kBackoffStartMs;
    while (laeuft.load())
    {
        const bool geordnet = eineVerbindung();
        if (! laeuft.load())
            break;
        if (geordnet)
            backoffMs = kBackoffStartMs;           // Verbindung stand — frisch starten
        {
            std::unique_lock<std::mutex> l (wartemutex);
            warte.wait_for (l, std::chrono::milliseconds (backoffMs),
                            [this] { return ! laeuft.load(); });
        }
        backoffMs = juce::jmin (backoffMs * 2, kBackoffMaxMs);
    }

    threadHandle.store (nullptr);
    if (self != nullptr)
        CloseHandle (self);
}

// true, wenn eine Verbindung tatsächlich stand (Welcome kam) — steuert nur den
// Backoff-Reset; Rückkehr heißt immer: Verbindung ist zu.
bool PipeClient::eineVerbindung()
{
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::verbindet;
        zustand.verbindungsVersuche++;
    }

    HANDLE h = CreateFileW (pipeName.toWideCharPointer(),
                            GENERIC_READ | GENERIC_WRITE,
                            0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (h == INVALID_HANDLE_VALUE)
    {
        const auto fehler = (int) GetLastError();
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        zustand.letzterFehler = "Broker nicht erreichbar (Win32 " + juce::String (fehler) + ")";
        return false;
    }

    aktivesHandle.store (h);
    bool welcomeKam = false;
    juce::String token;

    do
    {
        if (! sende (h, baueHello (helloProvider())))
            break;

        juce::String antwort;
        if (! empfange (h, antwort))
            break;

        const auto v = juce::JSON::parse (antwort);
        const auto typ = v.getProperty ("type", {}).toString();
        if (typ == "reject")
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = "Broker lehnt ab: " + v.getProperty ("reason", "?").toString();
            break;
        }
        if (typ != "welcome" || (int) v.getProperty ("protocol_version", 0) != kProtocolVersion)
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.letzterFehler = "unerwartete Antwort auf hello";
            break;
        }

        token = v.getProperty ("session_token", {}).toString();
        // Der Broker spiegelt die ANGENOMMENE Version: nur ein v2-Broker
        // quittiert Heartbeats — auf ein v1-welcome darf der Client nie
        // lesend warten (der alte Broker sendet nichts mehr).
        const int ausgehandelt = (int) v.getProperty ("protocol_version", 1);
        {
            std::lock_guard<std::mutex> l (zustandMutex);
            zustand.status = Status::verbunden;
            zustand.brokerVersion = v.getProperty ("broker_version", "?").toString();
            zustand.sessionToken = token;
            zustand.protokollVersion = ausgehandelt;
            zustand.letzterFehler.clear();
        }
        welcomeKam = true;
        neuVerbinden.store (false);

        juce::int64 seq = 0;
        while (laeuft.load() && ! neuVerbinden.load())
        {
            const auto mk = messProvider ? messProvider() : MessKompakt();
            if (! sende (h, baueHeartbeat (token, seq++, statsProvider(), mk)))
                break;
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                zustand.heartbeatsGesendet = seq;
            }
            if (ausgehandelt >= 2)
            {
                juce::String ackJson;
                if (! empfange (h, ackJson))
                    break;
                const auto ack = juce::JSON::parse (ackJson);
                if (ack.getProperty ("type", {}).toString() == "heartbeat_ack")
                {
                    std::lock_guard<std::mutex> l (zustandMutex);
                    zustand.konflikt = (bool) ack.getProperty ("konflikt", false);
                    zustand.letztesAck = ackJson;
                }
            }
            std::unique_lock<std::mutex> l (wartemutex);
            warte.wait_for (l, std::chrono::milliseconds (kHeartbeatMs),
                            [this] { return ! laeuft.load() || neuVerbinden.load(); });
        }

        // Geordneter Abschied — best effort, Fehler egal.
        auto* bye = new juce::DynamicObject();
        bye->setProperty ("type", "bye");
        bye->setProperty ("session_token", token);
        sende (h, juce::JSON::toString (juce::var (bye), true));
    }
    while (false);

    aktivesHandle.store (nullptr);
    CloseHandle (h);
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.status = Status::getrennt;
        zustand.sessionToken.clear();
        zustand.protokollVersion = 0;
        zustand.konflikt = false;    // getrennt = Konfliktstand unbekannt, kein Alarm
    }
    return welcomeKam;
}

// ── Framing: u32-LE-Länge + UTF-8-JSON (eq-ipc.schema.json) ────────────────
bool PipeClient::sende (void* handle, const juce::String& json)
{
    const auto utf8 = json.toRawUTF8();
    const auto len = (juce::uint32) strlen (utf8);
    if (len > kMaxFrameBytes)
        return false;

    juce::uint8 kopf[4] = { (juce::uint8) (len & 0xff), (juce::uint8) ((len >> 8) & 0xff),
                            (juce::uint8) ((len >> 16) & 0xff), (juce::uint8) ((len >> 24) & 0xff) };
    DWORD geschrieben = 0;
    if (! WriteFile ((HANDLE) handle, kopf, 4, &geschrieben, nullptr) || geschrieben != 4)
        return false;
    if (! WriteFile ((HANDLE) handle, utf8, len, &geschrieben, nullptr) || geschrieben != len)
        return false;
    return true;
}

bool PipeClient::empfange (void* handle, juce::String& jsonOut)
{
    auto liesGenau = [handle] (void* ziel, DWORD n) -> bool
    {
        auto* p = (juce::uint8*) ziel;
        while (n > 0)
        {
            DWORD gelesen = 0;
            if (! ReadFile ((HANDLE) handle, p, n, &gelesen, nullptr) || gelesen == 0)
                return false;
            p += gelesen;
            n -= gelesen;
        }
        return true;
    };

    juce::uint8 kopf[4];
    if (! liesGenau (kopf, 4))
        return false;
    const juce::uint32 len = (juce::uint32) kopf[0] | ((juce::uint32) kopf[1] << 8)
                           | ((juce::uint32) kopf[2] << 16) | ((juce::uint32) kopf[3] << 24);
    if (len == 0 || len > kMaxFrameBytes)
        return false;

    juce::HeapBlock<char> puffer (len + 1);
    if (! liesGenau (puffer.getData(), len))
        return false;
    puffer[len] = 0;
    jsonOut = juce::String::fromUTF8 (puffer.getData(), (int) len);
    return true;
}

} // namespace eqcop
