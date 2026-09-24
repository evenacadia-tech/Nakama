// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit. Diese hier sieht zwar gar kein JUCE, aber die
// Regel gilt fuer die MENGE, nicht fuer den Einzelfall - eine Ausnahme waere
// die Stelle, an der die naechste Quelle sie stillschweigend erbt.
#include "NakamaKernRiegel.h"

#include "IpcVerbindung.h"
#include "WireEnvelope.h"
#include "../../vertrag/NakamaUtf8.h"

#include <cstdint>

#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>

namespace nakama::ipc
{

bool namedPipeErreichbar (const std::string& pipeName)
{
    if (pipeName.empty())
        return false;
    std::wstring breit;
    breit.reserve (pipeName.size());
    for (char c : pipeName)
        breit.push_back (static_cast<wchar_t> (static_cast<unsigned char> (c)));
    // 🔑 NAK-180 R6: die Frist ist UNSERE, nicht die des Servers.
    //
    // Hier stand `0` — das ist `NMPWAIT_USE_DEFAULT_WAIT`, und MSDN sagt dazu:
    // "the time-out interval is the default value specified by the server
    // process in the CreateNamedPipe function". Der Server ist an dieser
    // Stelle aber gerade der, ueber den wir noch gar nichts wissen: einen
    // `\\.\pipe\`-Namen darf jedes lokale Konto anlegen, und mit
    // `nDefaultTimeOut = 0xFFFFFFFE` plus belegter Instanz stand der Aufruf
    // bis zu ~49,7 Tage. Der Lebenslaufthread haelt dabei das Win32-Startmutex,
    // und `BrokerLifecycle::stop()` joint ihn fristlos als erste Anweisung des
    // Prozessordestruktors — Plugin entfernen oder Projekt schliessen haette
    // den Message-Thread des Hosts fuer die vom Angreifer gewaehlte Dauer
    // aufgehalten (Entwurf §48.4, G4 §7 Part 05).
    //
    // 50 ms ist keine willkuerliche Zahl: es ist genau die Frist, die unser
    // EIGENER Broker als `nDefaultTimeOut`-Vorgabe bekommt. Im Normalfall
    // aendert sich damit nichts; ein fremder Besitzer kann sie nur nicht mehr
    // strecken. `ERROR_PIPE_BUSY` und `ERROR_SEM_TIMEOUT` zaehlen weiter als
    // "erreichbar" — beide heissen "da, aber gerade keine Instanz frei", und
    // genau dieser Zustand liegt zwischen `CreateNamedPipe` und
    // `ConnectNamedPipe` unseres eigenen Servers.
    static constexpr DWORD kErreichbarWarteMs = 50;
    if (WaitNamedPipeW (breit.c_str(), kErreichbarWarteMs) != FALSE)
        return true;
    const DWORD fehler = GetLastError();
    return fehler == ERROR_PIPE_BUSY || fehler == ERROR_SEM_TIMEOUT;
}

IpcVerbindung::~IpcVerbindung()
{
    schliessen();
}

bool IpcVerbindung::offen() const noexcept
{
    std::lock_guard<std::mutex> l (handleMutex);
    return handle != nullptr;
}

bool IpcVerbindung::oeffnen (const std::string& pipeName,
                             const ServerErwartung& erwartung,
                             ServerPruefBericht& bericht,
                             std::string& fehler)
{
    // NAK-134 Nacharbeit Runde 1, Defekt 1: hier standen das Schliessen der
    // alten Verbindung und das Zuruecksetzen des Abbruchsignals. Beide sind
    // nach `neueGenerationBeginnen()` gewandert — das Signal gehoert der
    // VERBINDUNGSGENERATION, nicht diesem Aufruf. Diese Funktion liest es ab
    // jetzt nur noch und setzt es nie zurueck: ein Signal, das vor dem
    // Eintritt stand, ueberlebt und greift an der ersten Kopfpruefung der
    // Schleife. Ein Test haelt das als Codeeigenschaft fest
    // (`abbruchsignal_gehoert_der_generation/oeffnen_loescht_nie`) — auch
    // dieser Kommentar darf die beiden Aufrufe deshalb nicht woertlich
    // enthalten.
    bericht = {};

    // Pipenamen sind reines ASCII (Praefix plus Base32-Token bzw. Probename).
    std::wstring breit;
    breit.reserve (pipeName.size());
    for (char c : pipeName)
        breit.push_back (static_cast<wchar_t> (static_cast<unsigned char> (c)));

    // ERROR_PIPE_BUSY heisst NICHT "nicht da", sondern "gerade ist keine
    // Instanz im Horchzustand". Genau dafuer gibt es WaitNamedPipe. Ohne
    // diese Schleife bekaeme jeder zweite Client beim gleichzeitigen Verbinden
    // vieler Sonden einen Backoff von bis zu acht Sekunden auf einen Broker,
    // der laengst laeuft — gemessen am Lastbein: 27 von 32 Paaren standen nach
    // 20 Sekunden, die uebrigen warteten auf ihren verdoppelten Backoff.
    HANDLE h = INVALID_HANDLE_VALUE;
    DWORD letzterFehler = 0;
    for (int versuch = 0; versuch < 20; ++versuch)
    {
        if (abbruch.load())
            return false;
        h = CreateFileW (breit.c_str(),
                         GENERIC_READ | GENERIC_WRITE,
                         0, nullptr, OPEN_EXISTING,
                         // NAK-49/ACCESS-001: die SQOS-Flags sind hier
                         // Pflicht, nicht Kosmetik. Ohne sie darf der
                         // Serverkontext den DAW-Token voll impersonieren.
                         FILE_FLAG_OVERLAPPED
                             | SECURITY_SQOS_PRESENT
                             | SECURITY_IDENTIFICATION,
                         nullptr);
        if (h != INVALID_HANDLE_VALUE)
            break;
        letzterFehler = GetLastError();
        if (letzterFehler != ERROR_PIPE_BUSY)
            break;
        // Rueckgabewert bewusst ignoriert: ein Zeitlimit hier ist kein
        // Fehler, sondern der naechste Versuch.
        WaitNamedPipeW (breit.c_str(), 200);
    }
    if (h == INVALID_HANDLE_VALUE)
    {
        // NAK-134/R1 — DREI Ausgaenge, nicht zwei. Bis hierher wurde nie ein
        // Handle geoeffnet, also kann keiner davon ein Identitaetsurteil sein:
        //
        //   * FILE_NOT_FOUND  — der Name existiert nicht. Nur dieser Wert darf
        //     den Broker-Startpfad oeffnen.
        //   * PIPE_BUSY nach 20 erschoepften Warterunden — der Name existiert,
        //     alle Instanzen sind belegt, der Server wurde NIE erreicht. Das
        //     ist LIVENESS: normaler Backoff, kein Parken. Vorher fiel dieser
        //     Fall in den Sicherheitszweig und parkte den Clientthread
        //     dauerhaft; nach einem Brokerneustart mit vielen Sonden blieb ein
        //     Teil davon bis zum Neuladen der Instanz getrennt (Gate-Lauf G3).
        //   * alles Uebrige (ACCESS_DENIED und jeder UNBEKANNTE Fehler) —
        //     deutet auf einen fremden Besitzer des Namens und bleibt
        //     fail-closed auf der Sicherheitsseite (NAK-123).
        //
        // Die Verengung geschieht ueber ZWEI namentlich genannte Codes; alles
        // Unbekannte bleibt, wo es war (Pruefliste D).
        if (letzterFehler == ERROR_FILE_NOT_FOUND)
        {
            bericht.status = ServerPruefStatus::nichtDa;
            bericht.fehler = ServerPruefFehler::pipeFehlt;
        }
        else if (letzterFehler == ERROR_PIPE_BUSY)
        {
            bericht.status = ServerPruefStatus::belegtNichtErreicht;
            bericht.fehler = ServerPruefFehler::pipeBelegt;
        }
        else
        {
            bericht.status = ServerPruefStatus::belegtAberUnverifiziert;
            bericht.fehler = ServerPruefFehler::pipeOeffnen;
        }
        bericht.win32Fehler = letzterFehler;
        fehler = "Broker nicht erreichbar (Win32 " + std::to_string ((int) letzterFehler) + ")";
        return false;
    }

    // NAK-123/C: Das lokale HANDLE bleibt bis zum VOLLSTAENDIGEN Serverbeweis
    // ausschliesslich in dieser Funktion. Insbesondere wird vor diesem Aufruf
    // weder ein Hello geschrieben noch der Handle unter `handleMutex`
    // veroeffentlicht.
    bericht = namedPipeServerAuthentisieren (h, erwartung);
    if (! bericht.ok())
    {
        fehler = std::string ("Server nicht verifiziert: ")
               + serverPruefFehlerName (bericht.fehler);
        CloseHandle (h);
        return false;
    }

    HANDLE e = CreateEventW (nullptr, TRUE, FALSE, nullptr);
    if (e == nullptr)
    {
        // NAK-134/R4 — ein LOKALER Ressourcenfehler NACH bestandenem
        // Identitaetsbeweis ist ein Livenessausgang: Handle schliessen,
        // normaler Backoff, kein Parken, kein Startpfad, keine zusaetzliche
        // Serverpruefung. `nichtGeprueft` faellt aus der Positivliste von
        // `ControlClient.cpp` und `TelemetryClient.cpp` und ist nicht
        // `nichtDa`, oeffnet also auch keinen Spawn.
        //
        // Neu ist nur die letzte der fuenf Zusagen: der Win32-Fehler steht ab
        // hier IM BERICHT und nicht bloss in der Fehlerzeichenkette. Vorher
        // trug `bericht.win32Fehler` noch den Wert der bestandenen
        // Authentisierung — eine Zahl, die zu diesem Ausgang nichts sagt.
        const auto ereignisFehler = GetLastError();
        fehler = "CreateEvent Win32 " + std::to_string ((int) ereignisFehler);
        CloseHandle (h);
        bericht.status = ServerPruefStatus::nichtGeprueft;
        bericht.win32Fehler = ereignisFehler;
        return false;
    }

    std::lock_guard<std::mutex> l (handleMutex);
    handle = h;
    ereignis = e;
    return true;
}

void IpcVerbindung::schliessen()
{
    abbruch.store (true);
    HANDLE h = nullptr, e = nullptr;
    {
        std::lock_guard<std::mutex> l (handleMutex);
        h = static_cast<HANDLE> (handle);
        e = static_cast<HANDLE> (ereignis);
        handle = nullptr;
        ereignis = nullptr;
        if (h != nullptr)
            CancelIoEx (h, nullptr);
    }
    if (h != nullptr)
        CloseHandle (h);
    if (e != nullptr)
        CloseHandle (e);
}

void IpcVerbindung::neueGenerationBeginnen()
{
    // Reihenfolge ist bindend: `schliessen()` SETZT das Abbruchsignal (es
    // beendet die alte Generation), also muss das Loesen danach kommen.
    schliessen();
    abbruch.store (false);
}

void IpcVerbindung::ioAbbrechen()
{
    abbruch.store (true);
    std::lock_guard<std::mutex> l (handleMutex);
    if (handle != nullptr)
        CancelIoEx (static_cast<HANDLE> (handle), nullptr);
}

bool IpcVerbindung::schreibenGenau (const std::uint8_t* daten, std::size_t laenge,
                                    Frist frist, std::string& fehler)
{
    const std::uint8_t* p = daten;
    std::size_t rest = laenge;

    while (rest > 0)
    {
        if (abbruch.load())
            return false;
        if (std::chrono::steady_clock::now() >= frist)
        {
            fehler = "Pipe-Schreiben: Zeitlimit ueberschritten";
            return false;
        }

        OVERLAPPED ov {};
        HANDLE h = nullptr;
        BOOL sofort = FALSE;
        {
            // Submission und externes Cancel teilen denselben Mutex.
            std::lock_guard<std::mutex> l (handleMutex);
            if (handle == nullptr || abbruch.load())
                return false;
            h = static_cast<HANDLE> (handle);
            ov.hEvent = static_cast<HANDLE> (ereignis);
            ResetEvent (ov.hEvent);
            const DWORD anfordern =
                static_cast<DWORD> (rest > 0x7FFFFFFFull ? 0x7FFFFFFFull : rest);
            sofort = WriteFile (h, p, anfordern, nullptr, &ov);
        }

        if (sofort == FALSE)
        {
            const DWORD start = GetLastError();
            if (start != ERROR_IO_PENDING)
            {
                if (start != ERROR_OPERATION_ABORTED || ! abbruch.load())
                    fehler = "Pipe-Schreiben: Win32 " + std::to_string ((int) start);
                return false;
            }
            const auto jetzt = std::chrono::steady_clock::now();
            auto verbleibend = std::chrono::duration_cast<std::chrono::milliseconds> (
                frist > jetzt ? frist - jetzt : std::chrono::steady_clock::duration::zero());
            ++verbleibend;  // nicht durch Abrunden VOR der absoluten Frist abbrechen
            DWORD warte = static_cast<DWORD> (verbleibend.count());
            if (warte >= INFINITE)
                warte = INFINITE - 1;
            if (WaitForSingleObject (ov.hEvent, warte) != WAIT_OBJECT_0)
            {
                // NUR DIESEN Schreibvorgang abbrechen, nicht die Verbindung.
                //
                // Hier stand `ioAbbrechen()`. Das setzt `abbruch` fuer die
                // ganze Verbindung — und danach lieferte JEDER Lesevorgang
                // sofort `LeseAusgang::fehler`. Genau der bereits vollstaendig
                // empfangene P0-ACK, den `B-CC-07` noch melden will, bevor die
                // Verbindung endet, war damit unerreichbar: das Zeitlimit des
                // Schreibens sperrte den Lesepfad, den es gerade noch brauchte
                // (NAK-104, Pruefbefund vom 2026-08-30).
                //
                // Der Lesepfad macht es seit jeher richtig und wird hier nur
                // gespiegelt: `CancelIoEx` auf DIESES `OVERLAPPED`, unter
                // demselben Mutex wie die Submission, damit kein
                // Check-then-use auf einem wiederverwendeten HANDLE entsteht.
                // Der Aufrufer schliesst die Verbindung ohnehin gleich — aber
                // erst, nachdem er das Empfangene gemeldet hat.
                {
                    std::lock_guard<std::mutex> l (handleMutex);
                    if (handle != nullptr)
                        CancelIoEx (h, &ov);
                }
                DWORD verworfen = 0;
                GetOverlappedResult (h, &ov, &verworfen, TRUE);
                if (! abbruch.load())
                    fehler = "Pipe-Schreiben: Zeitlimit ueberschritten";
                return false;
            }
        }

        DWORD fertig = 0;
        if (! GetOverlappedResult (h, &ov, &fertig, FALSE))
        {
            const DWORD f = GetLastError();
            if (f != ERROR_OPERATION_ABORTED || ! abbruch.load())
                fehler = "Pipe-Schreiben: Win32 " + std::to_string ((int) f);
            return false;
        }
        if (fertig == 0)
        {
            fehler = "Pipe-Schreiben: 0 Bytes geschrieben";
            return false;
        }
        p += fertig;
        rest -= fertig;
    }
    return true;
}

LeseAusgang IpcVerbindung::lesen (std::uint8_t* ziel, std::size_t laenge, std::size_t& gelesen,
                                  Frist frist, std::string& fehler)
{
    gelesen = 0;
    if (laenge == 0)
        return LeseAusgang::zeitlimit;
    if (abbruch.load())
        return LeseAusgang::fehler;

    OVERLAPPED ov {};
    HANDLE h = nullptr;
    BOOL sofort = FALSE;
    {
        std::lock_guard<std::mutex> l (handleMutex);
        if (handle == nullptr || abbruch.load())
            return LeseAusgang::fehler;
        h = static_cast<HANDLE> (handle);
        ov.hEvent = static_cast<HANDLE> (ereignis);
        ResetEvent (ov.hEvent);
        const DWORD anfordern =
            static_cast<DWORD> (laenge > 0x7FFFFFFFull ? 0x7FFFFFFFull : laenge);
        sofort = ReadFile (h, ziel, anfordern, nullptr, &ov);
    }

    if (sofort == FALSE)
    {
        const DWORD start = GetLastError();
        if (start == ERROR_BROKEN_PIPE || start == ERROR_PIPE_NOT_CONNECTED
            || start == ERROR_HANDLE_EOF)
            return LeseAusgang::ende;
        if (start != ERROR_IO_PENDING)
        {
            if (start != ERROR_OPERATION_ABORTED || ! abbruch.load())
                fehler = "Pipe-Lesen: Win32 " + std::to_string ((int) start);
            return LeseAusgang::fehler;
        }

        const auto jetzt = std::chrono::steady_clock::now();
        auto verbleibend = std::chrono::duration_cast<std::chrono::milliseconds> (
            frist > jetzt ? frist - jetzt : std::chrono::steady_clock::duration::zero());
        ++verbleibend;
        DWORD warte = static_cast<DWORD> (verbleibend.count());
        if (warte >= INFINITE)
            warte = INFINITE - 1;

        if (WaitForSingleObject (ov.hEvent, warte) != WAIT_OBJECT_0)
        {
            // Zeitlimit ist KEIN Fehler. Der Lesevorgang wird abgebrochen, das
            // schon gelieferte Teilergebnis aber ausgewertet — sonst gingen
            // Bytes verloren, die der Kernel bereits kopiert hat.
            {
                std::lock_guard<std::mutex> l (handleMutex);
                if (handle != nullptr)
                    CancelIoEx (h, &ov);
            }
            DWORD fertigNachAbbruch = 0;
            const BOOL ok = GetOverlappedResult (h, &ov, &fertigNachAbbruch, TRUE);
            if (ok && fertigNachAbbruch > 0)
            {
                gelesen = fertigNachAbbruch;
                return LeseAusgang::daten;
            }
            return abbruch.load() ? LeseAusgang::fehler : LeseAusgang::zeitlimit;
        }
    }

    DWORD fertig = 0;
    if (! GetOverlappedResult (h, &ov, &fertig, FALSE))
    {
        const DWORD f = GetLastError();
        if (f == ERROR_BROKEN_PIPE || f == ERROR_PIPE_NOT_CONNECTED || f == ERROR_HANDLE_EOF)
            return LeseAusgang::ende;
        if (f != ERROR_OPERATION_ABORTED || ! abbruch.load())
            fehler = "Pipe-Lesen: Win32 " + std::to_string ((int) f);
        return LeseAusgang::fehler;
    }
    if (fertig == 0)
        return LeseAusgang::ende;

    gelesen = fertig;
    return LeseAusgang::daten;
}

bool bootstrapRahmen (const std::string& json, std::vector<std::uint8_t>& ziel)
{
    if (json.size() > kMaxBootstrapBytes)
        return false;
    const auto n = static_cast<std::uint32_t> (json.size());
    ziel.clear();
    ziel.reserve (4 + json.size());
    ziel.push_back (static_cast<std::uint8_t> (n & 0xFF));
    ziel.push_back (static_cast<std::uint8_t> ((n >> 8) & 0xFF));
    ziel.push_back (static_cast<std::uint8_t> ((n >> 16) & 0xFF));
    ziel.push_back (static_cast<std::uint8_t> ((n >> 24) & 0xFF));
    ziel.insert (ziel.end(), json.begin(), json.end());
    return true;
}

namespace
{
void ueberspringeLeerraum (const std::string& t, std::size_t& i)
{
    while (i < t.size() && (t[i] == ' ' || t[i] == '\t' || t[i] == '\n' || t[i] == '\r'))
        ++i;
}

/// Die Wortlaute des Grund-Ausgangs (NAK-313 E-313-12). Keiner enthaelt ein
/// Teilwort, an dem Gen eine Inkompatibilitaet erkennt (SourcesModel.cpp,
/// inkompatiblerFehler): ein Lesefehler bleibt dort ein Handgriff.
constexpr const char* kSyntax = "Syntax";
constexpr const char* kKeinUtf8 = "kein gueltiges UTF-8";
constexpr const char* kNulEscape = "NUL-Escape";
constexpr const char* kEinsamesSurrogat = "einsames Surrogat";
constexpr const char* kUnbekanntesEscape = "unbekanntes Escape";
constexpr const char* kKurzesEscape = "\\u-Escape ohne vier Hexziffern";
constexpr const char* kSteuerzeichen = "rohes Steuerzeichen";
constexpr const char* kDoppelterName = "doppelter Name";

/// Merkt den Wortlaut einer Ablehnung und lehnt ab.
bool ablehnen (const char*& grund, const char* wortlaut) noexcept
{
    grund = wortlaut;
    return false;
}

/// Genau vier ASCII-Hexziffern ab t[i] als Wert; i steht danach dahinter.
bool vierHexziffern (const std::string& t, std::size_t& i, std::uint32_t& wert)
{
    if (i + 4 > t.size())
        return false;
    wert = 0;
    for (std::size_t k = 0; k < 4; ++k)
    {
        const char c = t[i + k];
        std::uint32_t ziffer = 0;
        if (c >= '0' && c <= '9')      ziffer = static_cast<std::uint32_t> (c - '0');
        else if (c >= 'a' && c <= 'f') ziffer = static_cast<std::uint32_t> (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') ziffer = static_cast<std::uint32_t> (c - 'A' + 10);
        else return false;
        wert = (wert << 4) | ziffer;
    }
    i += 4;
    return true;
}

/// Ein Codepunkt als UTF-8 (hoechstens U+10FFFF; Surrogate kommen nie hierher).
void alsUtf8 (std::uint32_t cp, std::string& ziel)
{
    if (cp < 0x80)
    {
        ziel.push_back (static_cast<char> (cp));
    }
    else if (cp < 0x800)
    {
        ziel.push_back (static_cast<char> (0xC0 | (cp >> 6)));
        ziel.push_back (static_cast<char> (0x80 | (cp & 0x3F)));
    }
    else if (cp < 0x10000)
    {
        ziel.push_back (static_cast<char> (0xE0 | (cp >> 12)));
        ziel.push_back (static_cast<char> (0x80 | ((cp >> 6) & 0x3F)));
        ziel.push_back (static_cast<char> (0x80 | (cp & 0x3F)));
    }
    else
    {
        ziel.push_back (static_cast<char> (0xF0 | (cp >> 18)));
        ziel.push_back (static_cast<char> (0x80 | ((cp >> 12) & 0x3F)));
        ziel.push_back (static_cast<char> (0x80 | ((cp >> 6) & 0x3F)));
        ziel.push_back (static_cast<char> (0x80 | (cp & 0x3F)));
    }
}

/// Das Zeichen eines einfachen Escapes (RFC 8259 §7) oder 0 fuer keines.
char einfachesEscape (char e) noexcept
{
    switch (e)
    {
        case '"':  return '"';
        case '\\': return '\\';
        case '/':  return '/';
        case 'b':  return '\b';
        case 'f':  return '\f';
        case 'n':  return '\n';
        case 'r':  return '\r';
        case 't':  return '\t';
        default:   return '\0';
    }
}

/// Ein u-Escape ab t[i] (hinter Backslash und u): genau vier Hexziffern; ein
/// hohes Surrogat braucht direkt ein u-Escape mit tiefem Surrogat und wird mit
/// ihm zu EINEM Codepunkt. NUL und einsame Surrogate sind ungueltig
/// (README Regeln 4 bis 6).
bool leseUEscape (const std::string& t, std::size_t& i, std::string& ziel, const char*& grund)
{
    std::uint32_t cp = 0;
    if (! vierHexziffern (t, i, cp))
        return ablehnen (grund, kKurzesEscape);
    if (cp == 0)
        return ablehnen (grund, kNulEscape);
    if (cp >= 0xDC00 && cp <= 0xDFFF)
        return ablehnen (grund, kEinsamesSurrogat);
    if (cp >= 0xD800 && cp <= 0xDBFF)
    {
        if (i + 1 >= t.size() || t[i] != '\\' || t[i + 1] != 'u')
            return ablehnen (grund, kEinsamesSurrogat);
        i += 2;
        std::uint32_t tief = 0;
        if (! vierHexziffern (t, i, tief))
            return ablehnen (grund, kKurzesEscape);
        if (tief < 0xDC00 || tief > 0xDFFF)
            return ablehnen (grund, kEinsamesSurrogat);
        cp = 0x10000 + ((cp - 0xD800) << 10) + (tief - 0xDC00);
    }
    alsUtf8 (cp, ziel);
    return true;
}

/// Liest einen JSON-String nach RFC 8259 §7 und dekodiert seine Escapes -
/// seit NAK-313 R-313-7; bis dahin lehnte jeder Backslash ab, eine engere
/// Regel als der Vertrag. Rohe Steuerzeichen unter 0x20 bleiben verboten. Die
/// UTF-8-Gueltigkeit der Rohbytes hat flachesJsonObjekt vorher geprueft.
bool leseString (const std::string& t, std::size_t& i, std::string& ziel, const char*& grund)
{
    if (i >= t.size() || t[i] != '"')
        return false;
    ++i;
    ziel.clear();
    while (i < t.size() && t[i] != '"')
    {
        const unsigned char c = static_cast<unsigned char> (t[i]);
        if (c < 0x20)
            return ablehnen (grund, kSteuerzeichen);
        if (c != '\\')
        {
            ziel.push_back (t[i]);
            ++i;
            continue;
        }
        if (i + 1 >= t.size())
            return false;                               // offener Text: Syntax
        const char e = t[i + 1];
        i += 2;
        if (e == 'u')
        {
            if (! leseUEscape (t, i, ziel, grund))
                return false;
            continue;
        }
        const char zeichen = einfachesEscape (e);
        if (zeichen == '\0')
            return ablehnen (grund, kUnbekanntesEscape);
        ziel.push_back (zeichen);
    }
    if (i >= t.size())
        return false;
    ++i;  // schliessendes "
    return true;
}

/// Der Leser selbst; grund bleibt nullptr fuer jede Ablehnung ohne eigenen
/// Wortlaut (dann gilt "Syntax").
bool flachesObjektLesen (const std::string& text, std::vector<JsonFeld>& felder, const char*& grund)
{
    std::size_t i = 0;
    ueberspringeLeerraum (text, i);
    if (i >= text.size() || text[i] != '{')
        return false;
    ++i;
    ueberspringeLeerraum (text, i);
    if (i < text.size() && text[i] == '}')
    {
        ++i;
        ueberspringeLeerraum (text, i);
        return i == text.size();
    }

    for (;;)
    {
        ueberspringeLeerraum (text, i);
        std::string schluessel;
        if (! leseString (text, i, schluessel, grund))
            return false;
        ueberspringeLeerraum (text, i);
        if (i >= text.size() || text[i] != ':')
            return false;
        ++i;
        ueberspringeLeerraum (text, i);
        if (i >= text.size())
            return false;

        std::string wert;
        bool istString = false;
        if (text[i] == '"')
        {
            if (! leseString (text, i, wert, grund))
                return false;
            istString = true;
        }
        else if (text[i] == '{' || text[i] == '[')
        {
            return false;  // keine Verschachtelung: bewusst nicht koennen
        }
        else
        {
            const std::size_t beginn = i;
            while (i < text.size() && text[i] != ',' && text[i] != '}'
                   && text[i] != ' ' && text[i] != '\t' && text[i] != '\n' && text[i] != '\r')
                ++i;
            if (i == beginn)
                return false;
            wert = text.substr (beginn, i - beginn);
        }

        // Doppelter Schluessel ist eine Ablehnung, keine "letzter gewinnt"-Regel
        // - verglichen werden die DEKODIERTEN Namen, auch ein Escape-Alias.
        for (const auto& f : felder)
            if (f.name == schluessel)
                return ablehnen (grund, kDoppelterName);
        felder.push_back (JsonFeld { schluessel, wert, istString });

        ueberspringeLeerraum (text, i);
        if (i >= text.size())
            return false;
        if (text[i] == ',')
        {
            ++i;
            continue;
        }
        if (text[i] == '}')
        {
            ++i;
            ueberspringeLeerraum (text, i);
            return i == text.size();
        }
        return false;
    }
}
} // namespace

bool flachesJsonObjekt (const std::string& text, std::vector<JsonFeld>& felder,
                        std::string* grund)
{
    felder.clear();
    const char* ablehnung = nullptr;
    // Vor dem ersten Zeichen: der ganze Text ist gueltiges UTF-8 (NAK-313
    // R-313-7). Das deckt Control-Welcome, Telemetrie-Welcome und das ACK.
    bool gelesen = nakama::utf8::istGueltig (text.data(), text.size());
    if (! gelesen)
        ablehnung = kKeinUtf8;
    else
        gelesen = flachesObjektLesen (text, felder, ablehnung);
    if (! gelesen && grund != nullptr)
        *grund = ablehnung != nullptr ? ablehnung : kSyntax;
    return gelesen;
}

namespace
{
const JsonFeld* feldSuchen (const std::vector<JsonFeld>& felder, const std::string& name)
{
    for (const auto& f : felder)
        if (f.name == name)
            return &f;
    return nullptr;
}
} // namespace

bool jsonText (const std::vector<JsonFeld>& felder, const std::string& name,
               std::string& wert)
{
    const auto* f = feldSuchen (felder, name);
    if (f == nullptr || ! f->istString)
        return false;
    wert = f->wert;
    return true;
}

bool jsonLiteral (const std::vector<JsonFeld>& felder, const std::string& name,
                  std::string& wert)
{
    const auto* f = feldSuchen (felder, name);
    if (f == nullptr || f->istString)
        return false;
    wert = f->wert;
    return true;
}

bool feldmengeGenau (const std::vector<JsonFeld>& felder,
                     std::initializer_list<const char*> erwartet)
{
    // Doppelte Schluessel hat `flachesJsonObjekt` schon abgelehnt; ein
    // Groessenvergleich plus "jedes Erwartete ist da" ist damit exakt.
    if (felder.size() != erwartet.size())
        return false;
    for (const char* name : erwartet)
        if (feldSuchen (felder, name) == nullptr)
            return false;
    return true;
}

} // namespace nakama::ipc

// K1 erneut: die zweite Auswertung sieht Makros, die erst ein spaeter
// eingebundener Header definiert haben koennte.
#include "NakamaKernRiegel.h"
