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

namespace
{
struct AuthHandle
{
    HANDLE wert = nullptr;
    explicit AuthHandle (HANDLE h = nullptr) : wert (h) {}
    ~AuthHandle()
    {
        if (wert != nullptr && wert != INVALID_HANDLE_VALUE)
            CloseHandle (wert);
    }
    AuthHandle (const AuthHandle&) = delete;
    AuthHandle& operator= (const AuthHandle&) = delete;
};

bool tokenUserSid (HANDLE token, std::vector<std::uint64_t>& speicher,
                   PSID& sid, DWORD& win32)
{
    DWORD noetig = 0;
    SetLastError (ERROR_SUCCESS);
    GetTokenInformation (token, TokenUser, nullptr, 0, &noetig);
    const DWORD groessenFehler = GetLastError();
    if (noetig < sizeof (TOKEN_USER) || groessenFehler != ERROR_INSUFFICIENT_BUFFER)
    {
        win32 = groessenFehler;
        return false;
    }
    speicher.resize ((static_cast<std::size_t> (noetig) + sizeof (std::uint64_t) - 1)
                     / sizeof (std::uint64_t));
    DWORD geschrieben = noetig;
    if (GetTokenInformation (token, TokenUser, speicher.data(), noetig,
                             &geschrieben) == FALSE
        || geschrieben < sizeof (TOKEN_USER))
    {
        win32 = GetLastError();
        return false;
    }
    sid = reinterpret_cast<TOKEN_USER*> (speicher.data())->User.Sid;
    return true;
}

bool dateiIdentisch (HANDLE a, HANDLE b)
{
    BY_HANDLE_FILE_INFORMATION ai {}, bi {};
    return GetFileInformationByHandle (a, &ai) != FALSE
        && GetFileInformationByHandle (b, &bi) != FALSE
        && ai.dwVolumeSerialNumber == bi.dwVolumeSerialNumber
        && ai.nFileIndexHigh == bi.nFileIndexHigh
        && ai.nFileIndexLow == bi.nFileIndexLow;
}

ServerPruefBericht authFehler (ServerPruefFehler fehler, DWORD win32 = 0,
                              DWORD pid = 0)
{
    ServerPruefBericht bericht;
    bericht.status = ServerPruefStatus::belegtAberUnverifiziert;
    bericht.fehler = fehler;
    bericht.win32Fehler = win32;
    bericht.serverPid = pid;
    return bericht;
}
} // namespace

ServerErwartung serverErwartungFuerTestdatei (const std::wstring& absoluterPfad,
                                              std::uint32_t erwarteterPid)
{
    ServerErwartung erwartung;
    erwartung.absoluterBrokerPfad = absoluterPfad;
    erwartung.erwarteterPid = erwarteterPid;
    if (! std::filesystem::path (absoluterPfad).is_absolute()
        || ! sha256Datei (absoluterPfad, erwartung.sha256))
    {
        erwartung.absoluterBrokerPfad.clear();
        erwartung.sha256.clear();
    }
    return erwartung;
}

ServerErwartung serverErwartungFuerEigenprozessTest()
{
    static const ServerErwartung erwartung = [] {
        std::wstring bild (32768, L'\0');
        DWORD laenge = static_cast<DWORD> (bild.size());
        if (QueryFullProcessImageNameW (GetCurrentProcess(), 0, bild.data(), &laenge) == FALSE
            || laenge == 0)
            return ServerErwartung {};
        bild.resize (laenge);
        return serverErwartungFuerTestdatei (bild, GetCurrentProcessId());
    }();
    return erwartung;
}

const char* serverPruefFehlerName (ServerPruefFehler fehler) noexcept
{
    switch (fehler)
    {
        case ServerPruefFehler::keiner: return "keiner";
        case ServerPruefFehler::pipeFehlt: return "pipeFehlt";
        case ServerPruefFehler::pipeOeffnen: return "pipeOeffnen";
        case ServerPruefFehler::erwartungUngueltig: return "erwartungUngueltig";
        case ServerPruefFehler::serverPidNichtErmittelbar: return "serverPidNichtErmittelbar";
        case ServerPruefFehler::serverPidFalsch: return "serverPidFalsch";
        case ServerPruefFehler::serverprozessNichtOeffnen: return "serverprozessNichtOeffnen";
        case ServerPruefFehler::serverTokenNichtOeffnen: return "serverTokenNichtOeffnen";
        case ServerPruefFehler::pluginTokenNichtOeffnen: return "pluginTokenNichtOeffnen";
        case ServerPruefFehler::serverTokenUserNichtLesbar: return "serverTokenUserNichtLesbar";
        case ServerPruefFehler::pluginTokenUserNichtLesbar: return "pluginTokenUserNichtLesbar";
        case ServerPruefFehler::serverSidUngueltig: return "serverSidUngueltig";
        case ServerPruefFehler::pluginSidUngueltig: return "pluginSidUngueltig";
        case ServerPruefFehler::serverSidFalsch: return "serverSidFalsch";
        case ServerPruefFehler::prozessbildNichtErmittelbar: return "prozessbildNichtErmittelbar";
        case ServerPruefFehler::prozessbildNichtOeffnen: return "prozessbildNichtOeffnen";
        case ServerPruefFehler::erwarteteDateiNichtOeffnen: return "erwarteteDateiNichtOeffnen";
        case ServerPruefFehler::dateiidentitaetFalsch: return "dateiidentitaetFalsch";
        case ServerPruefFehler::hashFalsch: return "hashFalsch";
        case ServerPruefFehler::signaturFehltOderUngueltig:
            return "signaturFehltOderUngueltig";
        case ServerPruefFehler::signerFalsch: return "signerFalsch";
    }
    return "unbekannt";
}

ServerPruefBericht namedPipeServerAuthentisieren (void* pipeHandle,
                                                  const ServerErwartung& erwartung)
{
    operationZaehlen();
    if (pipeHandle == nullptr || pipeHandle == INVALID_HANDLE_VALUE
        || erwartung.absoluterBrokerPfad.empty()
        || ! std::filesystem::path (erwartung.absoluterBrokerPfad).is_absolute()
        || ! istHex (erwartung.sha256, 64)
        || (! erwartung.authenticodeThumbprint.empty()
            && ((erwartung.authenticodeThumbprint.size() != 40
                 && erwartung.authenticodeThumbprint.size() != 64)
                || ! istHex (erwartung.authenticodeThumbprint,
                             erwartung.authenticodeThumbprint.size()))))
        return authFehler (ServerPruefFehler::erwartungUngueltig);

    ULONG serverPid = 0;
    if (erwartung.testFehler == ServerPruefFehler::serverPidNichtErmittelbar
        || GetNamedPipeServerProcessId (static_cast<HANDLE> (pipeHandle), &serverPid) == FALSE
        || serverPid == 0)
        return authFehler (ServerPruefFehler::serverPidNichtErmittelbar,
                           GetLastError());
    if (erwartung.testFehler == ServerPruefFehler::serverPidFalsch
        || (erwartung.erwarteterPid != 0 && serverPid != erwartung.erwarteterPid))
        return authFehler (ServerPruefFehler::serverPidFalsch, 0, serverPid);

    if (erwartung.testFehler == ServerPruefFehler::serverprozessNichtOeffnen)
        return authFehler (ServerPruefFehler::serverprozessNichtOeffnen,
                           ERROR_ACCESS_DENIED, serverPid);
    AuthHandle prozess (OpenProcess (PROCESS_QUERY_LIMITED_INFORMATION, FALSE, serverPid));
    if (prozess.wert == nullptr)
        return authFehler (ServerPruefFehler::serverprozessNichtOeffnen,
                           GetLastError(), serverPid);

    HANDLE serverTokenRoh = nullptr;
    if (erwartung.testFehler == ServerPruefFehler::serverTokenNichtOeffnen
        || OpenProcessToken (prozess.wert, TOKEN_QUERY, &serverTokenRoh) == FALSE)
        return authFehler (ServerPruefFehler::serverTokenNichtOeffnen,
                           GetLastError(), serverPid);
    AuthHandle serverToken (serverTokenRoh);

    HANDLE pluginTokenRoh = nullptr;
    if (erwartung.testFehler == ServerPruefFehler::pluginTokenNichtOeffnen
        || OpenProcessToken (GetCurrentProcess(), TOKEN_QUERY, &pluginTokenRoh) == FALSE)
        return authFehler (ServerPruefFehler::pluginTokenNichtOeffnen,
                           GetLastError(), serverPid);
    AuthHandle pluginToken (pluginTokenRoh);

    std::vector<std::uint64_t> serverSidSpeicher, pluginSidSpeicher;
    PSID serverSid = nullptr, pluginSid = nullptr;
    DWORD sidFehler = 0;
    if (erwartung.testFehler == ServerPruefFehler::serverTokenUserNichtLesbar
        || ! tokenUserSid (serverToken.wert, serverSidSpeicher, serverSid, sidFehler))
        return authFehler (ServerPruefFehler::serverTokenUserNichtLesbar,
                           sidFehler, serverPid);
    if (erwartung.testFehler == ServerPruefFehler::pluginTokenUserNichtLesbar
        || ! tokenUserSid (pluginToken.wert, pluginSidSpeicher, pluginSid, sidFehler))
        return authFehler (ServerPruefFehler::pluginTokenUserNichtLesbar,
                           sidFehler, serverPid);
    if (erwartung.testFehler == ServerPruefFehler::serverSidUngueltig
        || IsValidSid (serverSid) == FALSE)
        return authFehler (ServerPruefFehler::serverSidUngueltig, 0, serverPid);
    if (erwartung.testFehler == ServerPruefFehler::pluginSidUngueltig
        || IsValidSid (pluginSid) == FALSE)
        return authFehler (ServerPruefFehler::pluginSidUngueltig, 0, serverPid);
    if (erwartung.testFehler == ServerPruefFehler::serverSidFalsch
        || EqualSid (serverSid, pluginSid) == FALSE)
        return authFehler (ServerPruefFehler::serverSidFalsch, 0, serverPid);

    if (erwartung.testFehler == ServerPruefFehler::prozessbildNichtErmittelbar)
        return authFehler (ServerPruefFehler::prozessbildNichtErmittelbar,
                           ERROR_ACCESS_DENIED, serverPid);
    std::wstring bild (32768, L'\0');
    DWORD bildLaenge = static_cast<DWORD> (bild.size());
    if (QueryFullProcessImageNameW (prozess.wert, 0, bild.data(), &bildLaenge) == FALSE
        || bildLaenge == 0)
        return authFehler (ServerPruefFehler::prozessbildNichtErmittelbar,
                           GetLastError(), serverPid);
    bild.resize (bildLaenge);

    if (erwartung.testFehler == ServerPruefFehler::prozessbildNichtOeffnen)
        return authFehler (ServerPruefFehler::prozessbildNichtOeffnen,
                           ERROR_FILE_NOT_FOUND, serverPid);
    AuthHandle bildDatei (CreateFileW (
        bild.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (bildDatei.wert == INVALID_HANDLE_VALUE)
        return authFehler (ServerPruefFehler::prozessbildNichtOeffnen,
                           GetLastError(), serverPid);

    if (erwartung.testFehler == ServerPruefFehler::erwarteteDateiNichtOeffnen)
        return authFehler (ServerPruefFehler::erwarteteDateiNichtOeffnen,
                           ERROR_FILE_NOT_FOUND, serverPid);
    AuthHandle erwartetDatei (CreateFileW (
        erwartung.absoluterBrokerPfad.c_str(), FILE_READ_ATTRIBUTES,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr));
    if (erwartetDatei.wert == INVALID_HANDLE_VALUE)
        return authFehler (ServerPruefFehler::erwarteteDateiNichtOeffnen,
                           GetLastError(), serverPid);
    if (erwartung.testFehler == ServerPruefFehler::dateiidentitaetFalsch
        || ! dateiIdentisch (bildDatei.wert, erwartetDatei.wert))
        return authFehler (ServerPruefFehler::dateiidentitaetFalsch, 0, serverPid);

    if (erwartung.testVorFehlerErreicht != nullptr
        && erwartung.testFehlerFreigeben != nullptr)
    {
        erwartung.testVorFehlerErreicht->store (true);
        while (! erwartung.testFehlerFreigeben->load())
            Sleep (1);
    }

    if (erwartung.testFehler == ServerPruefFehler::hashFalsch)
        return authFehler (ServerPruefFehler::hashFalsch, 0, serverPid);
    if (erwartung.testFehler == ServerPruefFehler::signaturFehltOderUngueltig)
        return authFehler (ServerPruefFehler::signaturFehltOderUngueltig, 0, serverPid);
    if (erwartung.testFehler == ServerPruefFehler::signerFalsch)
        return authFehler (ServerPruefFehler::signerFalsch, 0, serverPid);

    const auto binaer = brokerBinaryPruefen (bild, erwartung.sha256,
                                             erwartung.authenticodeThumbprint);
    if (! binaer.ok())
    {
        switch (binaer.fehler)
        {
            case BrokerPruefFehler::signaturFehltOderUngueltig:
                return authFehler (ServerPruefFehler::signaturFehltOderUngueltig,
                                   0, serverPid);
            case BrokerPruefFehler::signerFalsch:
                return authFehler (ServerPruefFehler::signerFalsch, 0, serverPid);
            default:
                return authFehler (ServerPruefFehler::hashFalsch, 0, serverPid);
        }
    }

    ServerPruefBericht bericht;
    bericht.status = ServerPruefStatus::verifiziert;
    bericht.serverPid = serverPid;
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
    auto serverStatus = [&] {
        if (hooks.serverPruefstatus)
            return hooks.serverPruefstatus();
        if (hooks.verbunden && hooks.verbunden())
            return ServerPruefStatus::verifiziert;
        if (hooks.connectFehlgeschlagen && hooks.connectFehlgeschlagen())
            return ServerPruefStatus::nichtDa;
        return ServerPruefStatus::nichtGeprueft;
    };

    const auto anfangsStatus = serverStatus();
    if (anfangsStatus == ServerPruefStatus::belegtAberUnverifiziert)
    {
        // Fail-closed: kein weiterer Reconnect und kein Spawn gegen den
        // belegten Namen. Das Startmutex bleibt bis Stop oder einer bewussten
        // externen Zustandsaenderung beim Gewinner; ein zweiter Pluginprozess
        // darf denselben Sicherheitsfehler nicht in einen Spawn umdeuten.
        phase = Phase::blockiertUnverifiziert;
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.wartetAufBereit = false;
        zustand.wartetAufServerpruefung = false;
        zustand.serverNichtVerifiziert = true;
        zustand.imCooldown = false;
        zustand.letzterServerPruefstatus = anfangsStatus;
        return;
    }

    // Mit dem NAK-123-Hook ist ausschliesslich das Urteil des tatsaechlichen
    // Handles autoritativ. Zwei getrennte Snapshot-Aufrufe (`verbunden` und
    // Status) koennten sonst ueber eine Reconnectkante "alt verbunden" mit
    // "neu unverifiziert" mischen und den Sicherheitsfehler als bereit
    // zaehlen. `verbunden` bleibt nur der Kompatibilitaetspfad alter Tests.
    if ((! hooks.serverPruefstatus && hooks.verbunden && hooks.verbunden())
        || anfangsStatus == ServerPruefStatus::verifiziert)
    {
        phase = Phase::bereit;
        startMutexFreigeben();
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.wartetAufBereit = false;
        zustand.wartetAufServerpruefung = false;
        zustand.serverNichtVerifiziert = false;
        zustand.imCooldown = false;
        zustand.letzterServerPruefstatus = anfangsStatus;
        return;
    }

    if (phase == Phase::blockiertUnverifiziert)
    {
        // Nur ein expliziter Reconnect setzt den Controlstatus wieder auf
        // `nichtGeprueft`. Dann wird genau dieser neue Handle erneut geprueft;
        // ein alter PID-/Image-Erfolg wird nie wiederverwendet.
        phase = Phase::wartetAufServerpruefung;
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.serverNichtVerifiziert = false;
        zustand.wartetAufServerpruefung = true;
        zustand.letzterServerPruefstatus = anfangsStatus;
        return;
    }

    auto vorhandenePipeUebernehmen = [&] {
        if (hooks.pipeName.empty() || ! namedPipeErreichbar (hooks.pipeName))
            return false;
        // WaitNamedPipe ist nur der Anstoss. Bei einem produktiven
        // `serverPruefstatus` bleibt das Mutex gehalten, bis der ControlClient
        // genau den daraufhin geoeffneten Handle als verifiziert meldet.
        if (hooks.reconnect)
            hooks.reconnect();
        phase = hooks.serverPruefstatus ? Phase::wartetAufServerpruefung
                                       : Phase::wartetAufConnect;
        if (! hooks.serverPruefstatus)
            startMutexFreigeben(); // nur Rueckwaertskompatibilitaet der alten Tests
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.wartetAufBereit = false;
        zustand.wartetAufServerpruefung = static_cast<bool> (hooks.serverPruefstatus);
        zustand.serverNichtVerifiziert = false;
        zustand.imCooldown = false;
        return true;
    };

    if (phase == Phase::wartetAufServerpruefung)
    {
        if (anfangsStatus == ServerPruefStatus::nichtGeprueft)
            return;
        // `verifiziert` und `belegtAberUnverifiziert` sind oben behandelt.
        // `nichtDa` oeffnet erst jetzt den regulaeren, manifestgeprueften
        // Spawnweg.
        phase = Phase::wartetAufConnect;
        std::lock_guard<std::mutex> l (zustandMutex);
        zustand.wartetAufServerpruefung = false;
        zustand.letzterServerPruefstatus = anfangsStatus;
    }

    if (phase == Phase::wartetAufBroker)
    {
        if (spawnBereitTimeoutAbgelaufen (jetztMs - spawnZeitMs))
        {
            phase = Phase::cooldown;
            cooldownSeitMs = jetztMs;
            std::lock_guard<std::mutex> l (zustandMutex);
            ++zustand.cooldowns;
            zustand.wartetAufBereit = false;
            zustand.wartetAufServerpruefung = false;
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

    const bool brokerFehlt = hooks.serverPruefstatus
        ? anfangsStatus == ServerPruefStatus::nichtDa
        : hooks.connectFehlgeschlagen && hooks.connectFehlgeschlagen();
    if (! brokerFehlt)
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
    if ((! hooks.serverPruefstatus && hooks.verbunden && hooks.verbunden())
        || serverStatus() == ServerPruefStatus::verifiziert)
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
    zustand.wartetAufServerpruefung = false;
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
