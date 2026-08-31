#include "NakamaKernRiegel.h"
#include "BrokerLifecycle.h"
#include "IpcVerbindung.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <filesystem>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>
#include <bcrypt.h>
#include <wincrypt.h>
#include <mscat.h>
#include <softpub.h>
#include <wintrust.h>

namespace nakama::ipc
{
namespace
{
thread_local bool audioTestThread = false;
std::atomic<std::uint64_t> audioOperationen { 0 };

void operationZaehlen() noexcept
{
    if (audioTestThread)
        audioOperationen.fetch_add (1, std::memory_order_relaxed);
}

bool istHex (const std::string& text, std::size_t laenge) noexcept
{
    if (text.size() != laenge)
        return false;
    for (char c : text)
        if (! ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')
              || (c >= 'A' && c <= 'F')))
            return false;
    return true;
}

std::string gross (std::string text)
{
    for (char& c : text)
        if (c >= 'a' && c <= 'f')
            c = static_cast<char> (c - 'a' + 'A');
    return text;
}

std::string hex (const std::uint8_t* daten, std::size_t n)
{
    static constexpr char ziffern[] = "0123456789ABCDEF";
    std::string aus;
    aus.resize (n * 2);
    for (std::size_t i = 0; i < n; ++i)
    {
        aus[i * 2] = ziffern[daten[i] >> 4];
        aus[i * 2 + 1] = ziffern[daten[i] & 15];
    }
    return aus;
}

bool sha256Datei (const std::wstring& pfad, std::string& aus)
{
    operationZaehlen();
    HANDLE datei = CreateFileW (pfad.c_str(), GENERIC_READ, FILE_SHARE_READ,
                                nullptr, OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
                                nullptr);
    if (datei == INVALID_HANDLE_VALUE)
        return false;

    BCRYPT_ALG_HANDLE algorithmus = nullptr;
    BCRYPT_HASH_HANDLE hash = nullptr;
    DWORD objektBytes = 0, hashBytes = 0, geholt = 0;
    bool ok = BCryptOpenAlgorithmProvider (&algorithmus, BCRYPT_SHA256_ALGORITHM,
                                           nullptr, 0) >= 0
           && BCryptGetProperty (algorithmus, BCRYPT_OBJECT_LENGTH,
                                 reinterpret_cast<PUCHAR> (&objektBytes),
                                 sizeof (objektBytes), &geholt, 0) >= 0
           && BCryptGetProperty (algorithmus, BCRYPT_HASH_LENGTH,
                                 reinterpret_cast<PUCHAR> (&hashBytes),
                                 sizeof (hashBytes), &geholt, 0) >= 0
           && hashBytes == 32;
    std::vector<std::uint8_t> objekt (ok ? objektBytes : 0);
    std::array<std::uint8_t, 32> digest {};
    if (ok)
        ok = BCryptCreateHash (algorithmus, &hash, objekt.data(), objektBytes,
                               nullptr, 0, 0) >= 0;

    std::array<std::uint8_t, 64 * 1024> puffer {};
    while (ok)
    {
        DWORD gelesen = 0;
        if (ReadFile (datei, puffer.data(), static_cast<DWORD> (puffer.size()),
                      &gelesen, nullptr) == FALSE)
        {
            ok = false;
            break;
        }
        if (gelesen == 0)
            break;
        ok = BCryptHashData (hash, puffer.data(), gelesen, 0) >= 0;
    }
    if (ok)
        ok = BCryptFinishHash (hash, digest.data(),
                               static_cast<ULONG> (digest.size()), 0) >= 0;

    if (hash != nullptr)
        BCryptDestroyHash (hash);
    if (algorithmus != nullptr)
        BCryptCloseAlgorithmProvider (algorithmus, 0);
    CloseHandle (datei);
    if (ok)
        aus = hex (digest.data(), digest.size());
    return ok;
}

bool zertifikatThumbprint (PCCERT_CONTEXT zertifikat, std::size_t hexLaenge,
                           std::string& aus)
{
    if (zertifikat == nullptr)
        return false;
    if (hexLaenge == 40)
    {
        std::array<std::uint8_t, 20> digest {};
        DWORD n = static_cast<DWORD> (digest.size());
        if (CertGetCertificateContextProperty (zertifikat, CERT_SHA1_HASH_PROP_ID,
                                               digest.data(), &n) == FALSE
            || n != digest.size())
            return false;
        aus = hex (digest.data(), digest.size());
        return true;
    }

    std::array<std::uint8_t, 32> digest {};
    DWORD n = static_cast<DWORD> (digest.size());
    if (CryptHashCertificate2 (BCRYPT_SHA256_ALGORITHM, 0, nullptr,
                               zertifikat->pbCertEncoded, zertifikat->cbCertEncoded,
                               digest.data(), &n) == FALSE
        || n != digest.size())
        return false;
    aus = hex (digest.data(), digest.size());
    return true;
}

bool authenticodePruefen (const std::wstring& pfad, std::size_t thumbHexLaenge,
                          std::string& signer)
{
    operationZaehlen();
    WINTRUST_FILE_INFO datei {};
    datei.cbStruct = sizeof (datei);
    datei.pcwszFilePath = pfad.c_str();

    WINTRUST_DATA daten {};
    daten.cbStruct = sizeof (daten);
    daten.dwUIChoice = WTD_UI_NONE;
    daten.fdwRevocationChecks = WTD_REVOKE_NONE;
    daten.dwUnionChoice = WTD_CHOICE_FILE;
    daten.pFile = &datei;
    daten.dwStateAction = WTD_STATEACTION_VERIFY;
    daten.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL;

    GUID aktion = WINTRUST_ACTION_GENERIC_VERIFY_V2;
    const LONG urteil = WinVerifyTrust (nullptr, &aktion, &daten);
    auto signerAusZustand = [&] (WINTRUST_DATA& trust) -> bool
    {
        auto* provider = WTHelperProvDataFromStateData (trust.hWVTStateData);
        auto* signerKette = provider == nullptr
            ? nullptr : WTHelperGetProvSignerFromChain (provider, 0, FALSE, 0);
        PCCERT_CONTEXT zertifikat = nullptr;
        if (signerKette != nullptr && signerKette->csCertChain > 0)
            zertifikat = signerKette->pasCertChain[0].pCert;
        return zertifikatThumbprint (zertifikat, thumbHexLaenge, signer);
    };
    bool ok = urteil == ERROR_SUCCESS && signerAusZustand (daten);

    daten.dwStateAction = WTD_STATEACTION_CLOSE;
    WinVerifyTrust (nullptr, &aktion, &daten);
    if (ok)
        return true;

    // Windows-Systemdateien und manche ausgelieferten Programme sind nicht
    // eingebettet, sondern ueber einen signierten Systemkatalog beglaubigt.
    // WinVerifyTrust mit WTD_CHOICE_FILE sucht diesen Katalog nicht selbst;
    // die vollstaendige OS-Pruefkette braucht daher den dokumentierten
    // Catalog-Fallback. Die Byte-Hashpruefung oben bleibt davon unberuehrt.
    HCATADMIN admin = nullptr;
    HANDLE dateiHandle = INVALID_HANDLE_VALUE;
    HCATINFO katalogHandle = nullptr;
    if (CryptCATAdminAcquireContext2 (&admin, nullptr, BCRYPT_SHA256_ALGORITHM,
                                      nullptr, 0) == FALSE)
        return false;

    dateiHandle = CreateFileW (pfad.c_str(), GENERIC_READ,
                               FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                               nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    DWORD hashBytes = 0;
    std::vector<std::uint8_t> dateiHash;
    if (dateiHandle != INVALID_HANDLE_VALUE
        && CryptCATAdminCalcHashFromFileHandle2 (admin, dateiHandle,
                                                 &hashBytes, nullptr, 0) != FALSE
        && hashBytes > 0)
    {
        dateiHash.resize (hashBytes);
        if (CryptCATAdminCalcHashFromFileHandle2 (admin, dateiHandle,
                                                  &hashBytes, dateiHash.data(), 0) == FALSE)
            dateiHash.clear();
    }

    if (! dateiHash.empty())
        katalogHandle = CryptCATAdminEnumCatalogFromHash (
            admin, dateiHash.data(), static_cast<DWORD> (dateiHash.size()), 0, nullptr);

    CATALOG_INFO katalog {};
    katalog.cbStruct = sizeof (katalog);
    bool katalogOk = katalogHandle != nullptr
        && CryptCATCatalogInfoFromContext (katalogHandle, &katalog, 0) != FALSE;
    if (katalogOk)
    {
        const auto tagSchmal = hex (dateiHash.data(), dateiHash.size());
        const std::wstring tag (tagSchmal.begin(), tagSchmal.end());
        WINTRUST_CATALOG_INFO katalogTrust {};
        katalogTrust.cbStruct = sizeof (katalogTrust);
        katalogTrust.pcwszCatalogFilePath = katalog.wszCatalogFile;
        katalogTrust.pcwszMemberTag = tag.c_str();
        katalogTrust.pcwszMemberFilePath = pfad.c_str();
        katalogTrust.hMemberFile = dateiHandle;
        katalogTrust.pbCalculatedFileHash = dateiHash.data();
        katalogTrust.cbCalculatedFileHash = static_cast<DWORD> (dateiHash.size());
        katalogTrust.hCatAdmin = admin;

        WINTRUST_DATA katalogDaten {};
        katalogDaten.cbStruct = sizeof (katalogDaten);
        katalogDaten.dwUIChoice = WTD_UI_NONE;
        katalogDaten.fdwRevocationChecks = WTD_REVOKE_NONE;
        katalogDaten.dwUnionChoice = WTD_CHOICE_CATALOG;
        katalogDaten.pCatalog = &katalogTrust;
        katalogDaten.dwStateAction = WTD_STATEACTION_VERIFY;
        katalogDaten.dwProvFlags = WTD_CACHE_ONLY_URL_RETRIEVAL;
        katalogOk = WinVerifyTrust (nullptr, &aktion, &katalogDaten) == ERROR_SUCCESS
                 && signerAusZustand (katalogDaten);
        katalogDaten.dwStateAction = WTD_STATEACTION_CLOSE;
        WinVerifyTrust (nullptr, &aktion, &katalogDaten);
    }

    if (katalogHandle != nullptr)
        CryptCATAdminReleaseCatalogContext (admin, katalogHandle, 0);
    if (dateiHandle != INVALID_HANDLE_VALUE)
        CloseHandle (dateiHandle);
    CryptCATAdminReleaseContext (admin, 0);
    return katalogOk;
}

std::uint64_t monotoneMs() noexcept
{
    return static_cast<std::uint64_t> (
        std::chrono::duration_cast<std::chrono::milliseconds> (
            std::chrono::steady_clock::now().time_since_epoch()).count());
}
} // namespace

bool spawnRetryFaellig (std::uint64_t alterMs) noexcept
{
    return alterMs >= SPAWN_CONNECT_BACKOFF_START_MS;
}

bool spawnBereitTimeoutAbgelaufen (std::uint64_t alterMs) noexcept
{
    return alterMs >= SPAWN_BEREIT_TIMEOUT_MS;
}

bool spawnCooldownAbgelaufen (std::uint64_t alterMs) noexcept
{
    return alterMs >= SPAWN_COOLDOWN_MS;
}

bool brokerIdleEndeErreicht (std::uint64_t idleAlterMs,
                             unsigned aktiveClients) noexcept
{
    return aktiveClients == 0 && idleAlterMs >= BROKER_IDLE_ENDE_MS;
}

BrokerPruefBericht brokerBinaryPruefen (const std::wstring& absoluterPfad,
                                        const std::string& erwarteterSha256,
                                        const std::string& authenticodeThumbprint)
{
    BrokerPruefBericht bericht;
    if (! std::filesystem::path (absoluterPfad).is_absolute())
    {
        bericht.fehler = BrokerPruefFehler::pfadNichtAbsolut;
        return bericht;
    }
    if (! istHex (erwarteterSha256, 64))
    {
        bericht.fehler = BrokerPruefFehler::erwarteterHashUngueltig;
        return bericht;
    }
    if (! sha256Datei (absoluterPfad, bericht.dateiSha256))
    {
        bericht.fehler = BrokerPruefFehler::dateiNichtLesbar;
        return bericht;
    }
    bericht.hashGeprueft = true;
    if (gross (erwarteterSha256) != bericht.dateiSha256)
    {
        bericht.fehler = BrokerPruefFehler::hashFalsch;
        return bericht;
    }

    // JSON-null wird von der Zielschicht als leere Zeichenkette gebunden.
    if (authenticodeThumbprint.empty())
        return bericht;
    if (! istHex (authenticodeThumbprint, 40)
        && ! istHex (authenticodeThumbprint, 64))
    {
        bericht.fehler = BrokerPruefFehler::thumbprintUngueltig;
        return bericht;
    }

    bericht.signaturGeprueft = true;
    if (! authenticodePruefen (absoluterPfad, authenticodeThumbprint.size(),
                               bericht.signerThumbprint))
    {
        bericht.fehler = BrokerPruefFehler::signaturFehltOderUngueltig;
        return bericht;
    }
    if (gross (authenticodeThumbprint) != bericht.signerThumbprint)
    {
        bericht.fehler = BrokerPruefFehler::signerFalsch;
        return bericht;
    }
    return bericht;
}

bool brokerVerborgenStarten (const std::wstring& absoluterPfad)
{
    operationZaehlen();
    const std::filesystem::path pfad (absoluterPfad);
    if (! pfad.is_absolute())
        return false;

    std::wstring befehl = L"\"" + absoluterPfad + L"\"";
    STARTUPINFOW start {};
    start.cb = sizeof (start);
    start.dwFlags = STARTF_USESHOWWINDOW;
    start.wShowWindow = SW_HIDE;
    PROCESS_INFORMATION prozess {};
    const std::wstring arbeitsordner = pfad.parent_path().wstring();
    const BOOL ok = CreateProcessW (absoluterPfad.c_str(), befehl.data(),
                                    nullptr, nullptr, FALSE,
                                    CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,
                                    nullptr, arbeitsordner.c_str(), &start, &prozess);
    if (ok != FALSE)
    {
        CloseHandle (prozess.hThread);
        CloseHandle (prozess.hProcess);
        return true;
    }
    return false;
}

BrokerLifecycle::BrokerLifecycle (BrokerLifecycleHooks hooksIn)
    : hooks (std::move (hooksIn))
{
}

BrokerLifecycle::~BrokerLifecycle()
{
    stop();
}

void BrokerLifecycle::start()
{
    operationZaehlen();
    if (laeuft.exchange (true))
        return;
    thread = std::thread ([this] { threadLauf(); });
}

void BrokerLifecycle::stop()
{
    operationZaehlen();
    if (! laeuft.exchange (false))
    {
        if (thread.joinable())
            thread.join();
        startMutexFreigeben();
        return;
    }
    warte.notify_all();
    if (thread.joinable())
        thread.join();
    startMutexFreigeben();
}

BrokerLifecycle::Snapshot BrokerLifecycle::snapshot() const
{
    operationZaehlen();
    std::lock_guard<std::mutex> l (zustandMutex);
    return zustand;
}

void BrokerLifecycle::tickFuerTest (std::uint64_t jetztMs)
{
    operationZaehlen();
    tick (jetztMs);
}

void BrokerLifecycle::threadLauf()
{
    while (laeuft.load())
    {
        tick (monotoneMs());
        std::unique_lock<std::mutex> l (warteMutex);
        warte.wait_for (l, std::chrono::milliseconds (25),
                        [this] { return ! laeuft.load(); });
    }
    // Ein Win32-Mutex muss auf dem besitzenden Thread freigegeben werden.
    // `stop()` joint erst danach; ein CloseHandle vom Aufruferthread waere
    // sonst nur ein verlassener Mutex, keine ordentliche Freigabe.
    startMutexFreigeben();
}

bool BrokerLifecycle::startMutexNehmen()
{
    if (startMutex != nullptr || hooks.mutexName.empty())
        return startMutex != nullptr;
    operationZaehlen();
    HANDLE mutex = CreateMutexW (nullptr, FALSE, hooks.mutexName.c_str());
    if (mutex == nullptr)
        return false;
    const DWORD warten = WaitForSingleObject (mutex, 0);
    if (warten != WAIT_OBJECT_0 && warten != WAIT_ABANDONED)
    {
        CloseHandle (mutex);
        return false;
    }
    startMutex = mutex;
    return true;
}

void BrokerLifecycle::startMutexFreigeben()
{
    if (startMutex == nullptr)
        return;
    operationZaehlen();
    HANDLE mutex = static_cast<HANDLE> (startMutex);
    ReleaseMutex (mutex);
    CloseHandle (mutex);
    startMutex = nullptr;
}

void BrokerLifecycle::tick (std::uint64_t jetztMs)
{
    if (hooks.verbunden && hooks.verbunden())
    {
        phase = Phase::bereit;
        startMutexFreigeben();
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.wartetAufBereit = false;
        zustand.imCooldown = false;
        return;
    }

    auto vorhandenePipeUebernehmen = [&] {
        if (hooks.pipeName.empty() || ! namedPipeErreichbar (hooks.pipeName))
            return false;
        // Der Mutexgewinner hat den Broker bereits hergestellt; dieser
        // Client kann noch den alten getrennten Cache tragen. Ein frischer
        // Connect ersetzt den Cache, ein zweiter Prozess waere falsch.
        if (hooks.reconnect)
            hooks.reconnect();
        phase = Phase::wartetAufConnect;
        startMutexFreigeben();
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.wartetAufBereit = false;
        zustand.imCooldown = false;
        return true;
    };

    if (phase == Phase::wartetAufBroker)
    {
        if (spawnBereitTimeoutAbgelaufen (jetztMs - spawnZeitMs))
        {
            phase = Phase::cooldown;
            cooldownSeitMs = jetztMs;
            std::lock_guard<std::mutex> l (zustandMutex);
            ++zustand.cooldowns;
            zustand.wartetAufBereit = false;
            zustand.imCooldown = true;
            return;
        }
        if (jetztMs >= naechsterRetryMs)
        {
            if (hooks.reconnect)
                hooks.reconnect();
            {
                std::lock_guard<std::mutex> l (zustandMutex);
                ++zustand.connectRetriesNachSpawn;
            }
            const auto vergangen = jetztMs - spawnZeitMs;
            const auto rest = vergangen < SPAWN_BEREIT_TIMEOUT_MS
                ? SPAWN_BEREIT_TIMEOUT_MS - vergangen : 0;
            retryAbstandMs = std::min (retryAbstandMs * 2,
                                       std::max<std::uint64_t> (1, rest));
            naechsterRetryMs = jetztMs + retryAbstandMs;
        }
        return;
    }

    if (phase == Phase::cooldown)
    {
        if (! spawnCooldownAbgelaufen (jetztMs - cooldownSeitMs))
            return;
        startMutexFreigeben();
        phase = Phase::wartetAufConnect;
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.imCooldown = false;
        return;
    }

    if (! hooks.connectFehlgeschlagen || ! hooks.connectFehlgeschlagen())
        return; // Connect-without-spawn ist noch nicht nachweislich gescheitert.
    if (! hooks.darfStarten || ! hooks.darfStarten())
        return;
    if (! startMutexNehmen())
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        ++zustand.mutexVerloren;
        return;
    }
    // Das Mutex kann erst nach der Vorpruefung frei geworden sein. Der Sieger
    // prueft deshalb die Pipe erneut, bevor er Bytes oder Prozesse anfasst.
    if (hooks.verbunden && hooks.verbunden())
    {
        phase = Phase::bereit;
        startMutexFreigeben();
        return;
    }
    if (vorhandenePipeUebernehmen())
        return;

    const auto bericht = hooks.pruefen ? hooks.pruefen() : BrokerPruefBericht {
        BrokerPruefFehler::dateiNichtLesbar };
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        ++zustand.pruefungen;
        zustand.letzterPrueffehler = bericht.fehler;
    }
    if (! bericht.ok())
    {
        phase = Phase::cooldown;
        cooldownSeitMs = jetztMs;
        std::lock_guard<std::mutex> l (zustandMutex);
        ++zustand.cooldowns;
        zustand.imCooldown = true;
        return;
    }


    // Hash-/Signaturpruefung kann langsam sein. Rolle oder Editor duerfen in
    // dieser Zeit kippen; unmittelbar vor CreateProcess gilt nur der frische
    // Gatewert. Ebenso kann ein anderer berechtigter Starter inzwischen eine
    // Pipe bereitgestellt haben.
    if (! hooks.darfStarten || ! hooks.darfStarten())
    {
        phase = Phase::wartetAufConnect;
        startMutexFreigeben();
        return;
    }
    if (vorhandenePipeUebernehmen())
        return;

    bool gestartet = false;
    {
        std::lock_guard<std::mutex> l (zustandMutex);
        ++zustand.spawnVersuche;
    }
    if (hooks.spawn)
        gestartet = hooks.spawn();
    if (! gestartet)
    {
        phase = Phase::cooldown;
        cooldownSeitMs = jetztMs;
        std::lock_guard<std::mutex> l (zustandMutex);
        ++zustand.cooldowns;
        zustand.imCooldown = true;
        return;
    }

    phase = Phase::wartetAufBroker;
    spawnZeitMs = jetztMs;
    retryAbstandMs = SPAWN_CONNECT_BACKOFF_START_MS;
    naechsterRetryMs = jetztMs + SPAWN_CONNECT_BACKOFF_START_MS;
    std::lock_guard<std::mutex> l (zustandMutex);
    ++zustand.spawnErfolge;
    zustand.wartetAufBereit = true;
}

void brokerLifecycleAudioTestBeginn() noexcept { audioTestThread = true; }
void brokerLifecycleAudioTestEnde() noexcept { audioTestThread = false; }
void brokerLifecycleAudioTestZaehlerLoeschen() noexcept { audioOperationen.store (0); }
std::uint64_t brokerLifecycleOperationenImAudiothread() noexcept
{
    return audioOperationen.load (std::memory_order_relaxed);
}

} // namespace nakama::ipc

#include "NakamaKernRiegel.h"
