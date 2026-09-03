// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit. Diese hier sieht zwar gar kein JUCE, aber die
// Regel gilt fuer die MENGE, nicht fuer den Einzelfall - eine Ausnahme waere
// die Stelle, an der die naechste Quelle sie stillschweigend erbt.
#include "NakamaKernRiegel.h"

#include "IpcVerbindung.h"
#include "WireEnvelope.h"

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
    if (WaitNamedPipeW (breit.c_str(), 0) != FALSE)
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
    schliessen();
    abbruch.store (false);
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

/// Liest einen JSON-String OHNE Escapes. Ein Backslash fuehrt zur Ablehnung —
/// nicht zur Interpretation.
bool leseString (const std::string& t, std::size_t& i, std::string& ziel)
{
    if (i >= t.size() || t[i] != '"')
        return false;
    ++i;
    ziel.clear();
    while (i < t.size() && t[i] != '"')
    {
        const unsigned char c = static_cast<unsigned char> (t[i]);
        if (c == '\\' || c < 0x20)
            return false;
        ziel.push_back (t[i]);
        ++i;
    }
    if (i >= t.size())
        return false;
    ++i;  // schliessendes "
    return true;
}
} // namespace

bool flachesJsonObjekt (const std::string& text, std::vector<JsonFeld>& felder)
{
    felder.clear();
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
        if (! leseString (text, i, schluessel))
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
            if (! leseString (text, i, wert))
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

        // Doppelter Schluessel ist eine Ablehnung, keine "letzter gewinnt"-Regel.
        for (const auto& f : felder)
            if (f.name == schluessel)
                return false;
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
