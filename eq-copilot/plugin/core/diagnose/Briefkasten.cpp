// K1 (S8/SONDE-007a): der Kompilier-Riegel steht am Anfang UND am Ende jeder
// Kern-Uebersetzungseinheit.
#include "NakamaKernRiegel.h"

#include "Briefkasten.h"
#include "controlclient/Schleuse.h"

#include <juce_cryptography/juce_cryptography.h>

#include <algorithm>
#include <cstdio>
#include <ctime>

#ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>

namespace nakama::diagnose
{

namespace
{
bool hexKlein (char c) noexcept
{
    return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f');
}

/** Ein JSON-String nach RFC 8259 §7, dekodiert nach UTF-8. `false` bei jeder
    Verletzung und wenn der Wert nicht in `platz` passt - ein so langer Wert
    kann ohnehin keiner der erwarteten sein. */
bool liesString (const char* d, std::size_t n, std::size_t& i,
                 char* aus, std::size_t platz, std::size_t& laenge) noexcept
{
    laenge = 0;
    if (i >= n || d[i] != '"')
        return false;
    ++i;

    const auto schreibe = [&] (std::uint32_t byte) noexcept
    {
        if (laenge >= platz)
            return false;
        aus[laenge++] = (char) (unsigned char) byte;
        return true;
    };
    const auto utf8 = [&] (std::uint32_t cp) noexcept
    {
        if (cp < 0x80)
            return schreibe (cp);
        if (cp < 0x800)
            return schreibe (0xC0 | (cp >> 6)) && schreibe (0x80 | (cp & 0x3F));
        if (cp < 0x10000)
            return schreibe (0xE0 | (cp >> 12)) && schreibe (0x80 | ((cp >> 6) & 0x3F))
                && schreibe (0x80 | (cp & 0x3F));
        return schreibe (0xF0 | (cp >> 18)) && schreibe (0x80 | ((cp >> 12) & 0x3F))
            && schreibe (0x80 | ((cp >> 6) & 0x3F)) && schreibe (0x80 | (cp & 0x3F));
    };
    const auto hex4 = [&] (std::uint32_t& wert) noexcept
    {
        if (n - i < 4)
            return false;
        wert = 0;
        for (int k = 0; k < 4; ++k)
        {
            const char c = d[i++];
            const int v = (c >= '0' && c <= '9') ? c - '0'
                        : (c >= 'a' && c <= 'f') ? c - 'a' + 10
                        : (c >= 'A' && c <= 'F') ? c - 'A' + 10 : -1;
            if (v < 0)
                return false;
            wert = (wert << 4) | (std::uint32_t) v;
        }
        return true;
    };

    while (i < n)
    {
        const auto c = (unsigned char) d[i++];
        if (c == '"')
            return true;
        if (c < 0x20)
            return false;
        if (c != '\\')
        {
            if (! schreibe (c))
                return false;
            continue;
        }
        if (i >= n)
            return false;
        switch (d[i++])
        {
            case '"':  if (! schreibe ('"'))  return false; break;
            case '\\': if (! schreibe ('\\')) return false; break;
            case '/':  if (! schreibe ('/'))  return false; break;
            case 'b':  if (! schreibe (0x08)) return false; break;
            case 'f':  if (! schreibe (0x0C)) return false; break;
            case 'n':  if (! schreibe (0x0A)) return false; break;
            case 'r':  if (! schreibe (0x0D)) return false; break;
            case 't':  if (! schreibe (0x09)) return false; break;
            case 'u':
            {
                std::uint32_t cp = 0;
                if (! hex4 (cp))
                    return false;
                if (cp >= 0xD800 && cp <= 0xDBFF)
                {
                    std::uint32_t tief = 0;
                    if (n - i < 6 || d[i] != '\\' || d[i + 1] != 'u')
                        return false;
                    i += 2;
                    if (! hex4 (tief) || tief < 0xDC00 || tief > 0xDFFF)
                        return false;
                    cp = 0x10000 + ((cp - 0xD800) << 10) + (tief - 0xDC00);
                }
                else if (cp >= 0xDC00 && cp <= 0xDFFF)
                {
                    return false;
                }
                if (! utf8 (cp))
                    return false;
                break;
            }
            default:
                return false;
        }
    }
    return false;
}

bool gleich (const char* a, std::size_t n, const char* b) noexcept
{
    return std::strlen (b) == n && std::memcmp (a, b, n) == 0;
}

std::wstring breit (const std::string& s)
{
    std::wstring w;
    w.reserve (s.size());
    for (const unsigned char c : s)
        w.push_back ((wchar_t) c);
    return w;
}

bool istVerlinkt (const DateiAttribute& a) noexcept
{
    return a.reparse;
}
} // namespace

//==============================================================================

bool kennungGueltig (const char* zeichen, std::size_t anzahl) noexcept
{
    if (zeichen == nullptr || anzahl != 32)
        return false;
    for (std::size_t k = 0; k < anzahl; ++k)
        if (! hexKlein (zeichen[k]))
            return false;
    return true;
}

bool anfrageLesen (const char* d, std::size_t n, Kennung& aus) noexcept
{
    if (d == nullptr)
        return false;
    // UTF-8 OHNE BOM (F-2): eine Datei mit BOM ist fremd geformt.
    if (n >= 3 && (unsigned char) d[0] == 0xEF && (unsigned char) d[1] == 0xBB
               && (unsigned char) d[2] == 0xBF)
        return false;

    std::size_t i = 0;
    const auto leer = [&] () noexcept
    {
        while (i < n && (d[i] == ' ' || d[i] == '\t' || d[i] == '\n' || d[i] == '\r'))
            ++i;
    };

    leer();
    if (i >= n || d[i] != '{')
        return false;
    ++i;

    bool habeFormat = false;
    bool habeKennung = false;
    Kennung kennung;
    for (int glied = 0; glied < 2; ++glied)
    {
        leer();
        char schluessel[16];
        std::size_t schluesselLaenge = 0;
        if (! liesString (d, n, i, schluessel, sizeof (schluessel), schluesselLaenge))
            return false;
        leer();
        if (i >= n || d[i] != ':')
            return false;
        ++i;
        leer();
        char wert[64];
        std::size_t wertLaenge = 0;
        if (! liesString (d, n, i, wert, sizeof (wert), wertLaenge))
            return false;

        if (gleich (schluessel, schluesselLaenge, "format"))
        {
            if (habeFormat || ! gleich (wert, wertLaenge, kAnfrageFormat))
                return false;
            habeFormat = true;
        }
        else if (gleich (schluessel, schluesselLaenge, "anfrage_id"))
        {
            if (habeKennung || ! kennungGueltig (wert, wertLaenge))
                return false;
            std::memcpy (kennung.z.data(), wert, kennung.z.size());
            habeKennung = true;
        }
        else
        {
            return false;
        }

        leer();
        if (glied == 0)
        {
            if (i >= n || d[i] != ',')
                return false;
            ++i;
        }
    }
    leer();
    if (i >= n || d[i] != '}')
        return false;
    ++i;
    leer();
    if (i != n || ! habeFormat || ! habeKennung)
        return false;
    aus = kennung;
    return true;
}

std::string isoUtc (std::int64_t ms)
{
    std::int64_t sekunden = ms / 1000;
    int milli = (int) (ms % 1000);
    if (milli < 0)
    {
        milli += 1000;
        --sekunden;
    }
    const __time64_t t = (__time64_t) sekunden;
    std::tm tm {};
    if (sekunden < 0 || _gmtime64_s (&tm, &t) != 0)
        return "1970-01-01T00:00:00.000Z";
    char puffer[32];
    std::snprintf (puffer, sizeof (puffer), "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
                   tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                   tm.tm_hour, tm.tm_min, tm.tm_sec, milli);
    return puffer;
}

//== Fassaden =================================================================

DateiAttribute DateisystemFassade::attribute (const std::wstring& pfad)
{
    zExistenz.fetch_add (1, std::memory_order_relaxed);
    return attributeImpl (pfad);
}

bool DateisystemFassade::lies (const std::wstring& pfad, char* ziel, std::size_t hoechstens,
                               std::size_t& gelesen)
{
    zLese.fetch_add (1, std::memory_order_relaxed);
    gelesen = 0;
    const bool ok = liesImpl (pfad, ziel, hoechstens, gelesen);
    zBytes.fetch_add (gelesen, std::memory_order_relaxed);
    return ok;
}

DateisystemFassade::Handle DateisystemFassade::legeExklusivAn (const std::wstring& pfad)
{
    zAnlegen.fetch_add (1, std::memory_order_relaxed);
    return legeExklusivAnImpl (pfad);
}

bool DateisystemFassade::schreibeUndSpuele (Handle datei, const char* daten, std::size_t anzahl)
{
    zSchreiben.fetch_add (1, std::memory_order_relaxed);
    return schreibeUndSpueleImpl (datei, daten, anzahl);
}

void DateisystemFassade::schliesse (Handle datei)
{
    schliesseImpl (datei);
}

bool DateisystemFassade::groesse (const std::wstring& pfad, std::int64_t& aus)
{
    zGroesse.fetch_add (1, std::memory_order_relaxed);
    aus = 0;
    return groesseImpl (pfad, aus);
}

bool DateisystemFassade::benenneUmOhneErsetzen (const std::wstring& von, const std::wstring& nach)
{
    zUmbenennen.fetch_add (1, std::memory_order_relaxed);
    return benenneUmOhneErsetzenImpl (von, nach);
}

bool DateisystemFassade::legeOrdnerAn (const std::wstring& pfad)
{
    zOrdner.fetch_add (1, std::memory_order_relaxed);
    return legeOrdnerAnImpl (pfad);
}

bool DateisystemFassade::loesche (const std::wstring& pfad)
{
    zLoeschen.fetch_add (1, std::memory_order_relaxed);
    return loescheImpl (pfad);
}

FassadenStand DateisystemFassade::stand() const noexcept
{
    FassadenStand s;
    s.existenzpruefungen = zExistenz.load (std::memory_order_relaxed);
    s.leseoeffnungen     = zLese.load (std::memory_order_relaxed);
    s.geleseneBytes      = zBytes.load (std::memory_order_relaxed);
    s.anlegeversuche     = zAnlegen.load (std::memory_order_relaxed);
    s.schreibvorgaenge   = zSchreiben.load (std::memory_order_relaxed);
    s.groessenabfragen   = zGroesse.load (std::memory_order_relaxed);
    s.umbenennungen      = zUmbenennen.load (std::memory_order_relaxed);
    s.ordneranlagen      = zOrdner.load (std::memory_order_relaxed);
    s.loeschungen        = zLoeschen.load (std::memory_order_relaxed);
    return s;
}

DateiAttribute EchteDateisystemFassade::attributeImpl (const std::wstring& pfad)
{
    // GetFileAttributesExW folgt einem Link in der LETZTEN Komponente nicht:
    // ein Symlink oder eine Junction meldet sich selbst mit REPARSE_POINT.
    DateiAttribute a;
    WIN32_FILE_ATTRIBUTE_DATA daten {};
    if (GetFileAttributesExW (pfad.c_str(), GetFileExInfoStandard, &daten) == FALSE)
        return a;
    a.existiert      = true;
    a.verzeichnis    = (daten.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
    a.reparse        = (daten.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
    a.groesse        = (std::int64_t) (((std::uint64_t) daten.nFileSizeHigh << 32) | daten.nFileSizeLow);
    a.aenderungszeit = (std::int64_t) (((std::uint64_t) daten.ftLastWriteTime.dwHighDateTime << 32)
                                       | daten.ftLastWriteTime.dwLowDateTime);
    return a;
}

bool EchteDateisystemFassade::liesImpl (const std::wstring& pfad, char* ziel, std::size_t hoechstens,
                                        std::size_t& gelesen)
{
    // FILE_FLAG_OPEN_REPARSE_POINT: die Datei selbst, nie das Ziel eines Links
    // (F-11, T-6). Geteilt lesbar, schreib- und loeschbar: der Anfragende darf
    // die Anfrage waehrend des Lesens ersetzen; der Merker sieht es am naechsten
    // Takt an Aenderungszeit und Groesse.
    const HANDLE h = CreateFileW (pfad.c_str(), GENERIC_READ,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                                  nullptr, OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    bool ok = true;
    while (gelesen < hoechstens)
    {
        DWORD stueck = 0;
        const DWORD wunsch = (DWORD) std::min<std::size_t> (hoechstens - gelesen, 65536);
        if (ReadFile (h, ziel + gelesen, wunsch, &stueck, nullptr) == FALSE)
        {
            ok = false;
            break;
        }
        if (stueck == 0)
            break;
        gelesen += stueck;
    }
    CloseHandle (h);
    return ok;
}

DateisystemFassade::Handle EchteDateisystemFassade::legeExklusivAnImpl (const std::wstring& pfad)
{
    const HANDLE h = CreateFileW (pfad.c_str(), GENERIC_WRITE, 0, nullptr, CREATE_NEW,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
    return h == INVALID_HANDLE_VALUE ? nullptr : h;
}

bool EchteDateisystemFassade::schreibeUndSpueleImpl (Handle datei, const char* daten, std::size_t anzahl)
{
    if (datei == nullptr)
        return false;
    const HANDLE h = datei;
    std::size_t geschrieben = 0;
    while (geschrieben < anzahl)
    {
        DWORD stueck = 0;
        const DWORD wunsch = (DWORD) std::min<std::size_t> (anzahl - geschrieben, 1u << 20);
        if (WriteFile (h, daten + geschrieben, wunsch, &stueck, nullptr) == FALSE || stueck == 0)
            return false;
        geschrieben += stueck;
    }
    return FlushFileBuffers (h) != FALSE;
}

void EchteDateisystemFassade::schliesseImpl (Handle datei)
{
    if (datei != nullptr)
        CloseHandle (datei);
}

bool EchteDateisystemFassade::groesseImpl (const std::wstring& pfad, std::int64_t& aus)
{
    WIN32_FILE_ATTRIBUTE_DATA daten {};
    if (GetFileAttributesExW (pfad.c_str(), GetFileExInfoStandard, &daten) == FALSE)
        return false;
    aus = (std::int64_t) (((std::uint64_t) daten.nFileSizeHigh << 32) | daten.nFileSizeLow);
    return true;
}

bool EchteDateisystemFassade::benenneUmOhneErsetzenImpl (const std::wstring& von, const std::wstring& nach)
{
    // Ohne MOVEFILE_REPLACE_EXISTING: ein vorhandener Name laesst den Aufruf
    // scheitern (Muster broker/src/persistenz.rs:78-109) - nie ein Ersetzen.
    return MoveFileExW (von.c_str(), nach.c_str(), MOVEFILE_WRITE_THROUGH) != FALSE;
}

bool EchteDateisystemFassade::legeOrdnerAnImpl (const std::wstring& pfad)
{
    return CreateDirectoryW (pfad.c_str(), nullptr) != FALSE;
}

bool EchteDateisystemFassade::loescheImpl (const std::wstring& pfad)
{
    return DeleteFileW (pfad.c_str()) != FALSE;
}

bool WurzelFassade::wurzel (std::wstring& pfad)
{
    zAbfragen.fetch_add (1, std::memory_order_relaxed);
    pfad.clear();
    return wurzelImpl (pfad);
}

bool EchteWurzelFassade::wurzelImpl (std::wstring& pfad)
{
    // Wie der Knopfweg (src/prozessor/Analyse.cpp): %LOCALAPPDATA% ueber JUCE.
    const auto ordner = juce::File::getSpecialLocation (juce::File::windowsLocalAppData);
    const auto& text = ordner.getFullPathName();
    pfad = text.toWideCharPointer();
    return text.isNotEmpty() && ordner.isDirectory();
}

std::int64_t EchteUhrFassade::jetztUtcMs()
{
    return juce::Time::currentTimeMillis();
}

//== Der Kern ===================================================================

struct Briefkasten::Kern
{
    std::shared_ptr<WurzelFassade>      wurzel;
    std::shared_ptr<DateisystemFassade> fs;
    std::shared_ptr<UhrFassade>         uhr;
    Antwortquelle                       quelle;
    std::function<void()>               hakenVorSchleuse;
    std::function<void()>               hakenBeimStopp;

    ipc::CallbackSchleuse schleuse;
    std::atomic<bool>     taktLaeuft { false };
    std::atomic<bool>     gestartet { false };
    std::atomic<bool>     gestoppt { false };
    std::atomic<int>      grund { (int) Startgrund::nichtGestartet };

    // Ab dem Start unveraenderlich.
    std::string                 rolle;
    std::string                 laufzeit32;
    std::uint32_t               pid = 0;
    std::array<std::wstring, 4> ebene;      ///< evenacadia, nakama, diagnose, antwort
    std::wstring                anfrage;

    // Nur im Takt (Message-Thread).
    struct Merker
    {
        bool         gueltig = false;
        bool         fremd = false;
        std::int64_t zeit = 0;
        std::int64_t groesse = 0;
        Kennung      kennung;
    } merker;

    struct Offen
    {
        bool    gueltig = false;
        bool    begonnen = false;
        bool    aufgegeben = false;
        int     versuche = 0;
        Kennung kennung;
    } offen;

    std::array<Kennung, kKennungsring> ring {};
    int ringNaechster = 0;
    int ringBelegt = 0;

    std::atomic<std::uint64_t> starts { 0 }, begonnen { 0 }, takte { 0 }, abgewiesen { 0 },
                               uebersprungen { 0 }, laufend { 0 }, laufendMax { 0 },
                               warteTakte { 0 }, schreibversuche { 0 }, antworten { 0 },
                               aufgegeben { 0 }, ausnahmen { 0 };

    bool ringEnthaelt (const Kennung& k) const noexcept
    {
        for (int i = 0; i < ringBelegt; ++i)
            if (ring[(std::size_t) i] == k)
                return true;
        return false;
    }

    void ringMerke (const Kennung& k) noexcept
    {
        // FIFO fester Groesse: die 257. Kennung verdraengt die aelteste (F-13).
        ring[(std::size_t) ringNaechster] = k;
        ringNaechster = (ringNaechster + 1) % kKennungsring;
        ringBelegt = std::min (ringBelegt + 1, kKennungsring);
    }

    /** F-11 (3): die Ebenen vor dem Lesen (3) beziehungsweise Schreiben (4). */
    bool ebenenIntakt (int anzahl)
    {
        for (int i = 0; i < anzahl; ++i)
        {
            const auto a = fs->attribute (ebene[(std::size_t) i]);
            if (! a.existiert || ! a.verzeichnis || istVerlinkt (a))
                return false;
        }
        return true;
    }

    /** F-8, sechs Schritte, jeder ueber die Fassade. `false` ist ein
        gescheiterter Schreibversuch; eine liegengebliebene Temp-Datei bleibt
        liegen (kein Aufraeumer, F-9). */
    bool schreibe (const Kennung& k, const Antwort& a)
    {
        if (a.umschlag.empty())
            return false;
        const auto hash = juce::SHA256 (a.instanzId.data(), a.instanzId.size()).toHexString();
        const std::string instanz16 = hash.substring (0, 16).toStdString();
        const auto name = breit (k.text() + "." + rolle + "." + std::to_string (pid) + "."
                                 + instanz16 + "." + laufzeit32 + ".json");

        // (1) `antwort\` wird nie angelegt; fehlt er oder ist eine Ebene
        //     verlinkt, ist das ein gescheiterter Versuch.
        if (! ebenenIntakt (4))
            return false;
        const auto ziel = ebene[3] + L"\\" + name;
        const auto temp = ziel + L".tmp-" + breit (std::to_string (pid));

        // (2) exklusiv anlegen - ein vorhandener Name ist ein gescheiterter Versuch.
        const auto datei = fs->legeExklusivAn (temp);
        if (datei == nullptr)
            return false;
        // (3) schreiben, spuelen, schliessen.
        const bool geschrieben = fs->schreibeUndSpuele (datei, a.umschlag.data(), a.umschlag.size());
        fs->schliesse (datei);
        if (! geschrieben)
            return false;
        // (4) Groesse: groesser 0 und gleich der geschriebenen Bytes.
        std::int64_t groesse = 0;
        if (! fs->groesse (temp, groesse) || groesse <= 0
            || groesse != (std::int64_t) a.umschlag.size())
            return false;
        // (5) ohne Ersetzen in den Antwortnamen. (6) kein Aufraeumer.
        return fs->benenneUmOhneErsetzen (temp, ziel);
    }

    void rumpf()
    {
        // (2) GENAU EINE Existenzpruefung je Takt (F-14).
        const auto a = fs->attribute (anfrage);
        if (! a.existiert || a.verzeichnis || istVerlinkt (a))
        {
            merker = {};
            offen = {};
            return;
        }

        Kennung kennung;
        if (merker.gueltig && merker.zeit == a.aenderungszeit && merker.groesse == a.groesse)
        {
            // Unveraendert: nichts lesen (T-5).
            if (merker.fremd)
                return;
            kennung = merker.kennung;
        }
        else
        {
            // Groesse vor dem Oeffnen (F-2): eine zu grosse Datei wird nie gelesen.
            if (a.groesse < 0 || (std::uint64_t) a.groesse > kAnfrageGrenzeBytes)
            {
                merker = { true, true, a.aenderungszeit, a.groesse, {} };
                offen = {};
                return;
            }
            if (! ebenenIntakt (3))
                return;
            std::array<char, kAnfrageGrenzeBytes + 1> puffer {};
            std::size_t gelesen = 0;
            if (! fs->lies (anfrage, puffer.data(), puffer.size(), gelesen))
                return;   // gesperrt: kein Merker, der naechste Takt liest erneut (F-10)
            if (gelesen > kAnfrageGrenzeBytes || ! anfrageLesen (puffer.data(), gelesen, kennung))
            {
                merker = { true, true, a.aenderungszeit, a.groesse, {} };
                offen = {};
                return;
            }
            merker = { true, false, a.aenderungszeit, a.groesse, kennung };
        }

        // Genau einmal, solange die Kennung im Ring steht (F-13, §10.2 P-1).
        if (ringEnthaelt (kennung))
        {
            offen = {};
            return;
        }
        if (! offen.gueltig || offen.kennung != kennung)
        {
            offen = {};
            offen.gueltig = true;
            offen.kennung = kennung;
        }
        if (offen.aufgegeben || ! quelle)
            return;

        Anfrage anf;
        anf.kennung    = kennung;
        anf.pid        = pid;
        anf.erzeugtUtc = isoUtc (uhr->jetztUtcMs());
        anf.zyklusNeu  = ! offen.begonnen;
        offen.begonnen = true;
        const auto antwort = quelle (anf);
        if (antwort.wartet)
        {
            // Kein Schreibversuch, kein Fehler, nichts angelegt (F-13).
            warteTakte.fetch_add (1, std::memory_order_relaxed);
            return;
        }

        ++offen.versuche;
        schreibversuche.fetch_add (1, std::memory_order_relaxed);
        if (schreibe (kennung, antwort))
        {
            ringMerke (kennung);
            antworten.fetch_add (1, std::memory_order_relaxed);
            offen = {};
            return;
        }
        if (offen.versuche >= kSchreibversuche)
        {
            offen.aufgegeben = true;
            aufgegeben.fetch_add (1, std::memory_order_relaxed);
        }
    }

    static void takt (const std::shared_ptr<Kern>& k)
    {
        k->begonnen.fetch_add (1, std::memory_order_relaxed);
        const auto jetzt = k->laufend.fetch_add (1) + 1;
        for (auto hoechst = k->laufendMax.load(); jetzt > hoechst
             && ! k->laufendMax.compare_exchange_weak (hoechst, jetzt);)
        {
        }
        struct Ende
        {
            Kern& kern;
            ~Ende() { kern.laufend.fetch_sub (1); }
        } ende { *k };

        if (k->hakenVorSchleuse)
            k->hakenVorSchleuse();

        // F-12: nach `stoppe()` weist die Schleuse jeden Takt ab.
        const auto zug = k->schleuse.betreten();
        if (! zug)
        {
            k->abgewiesen.fetch_add (1, std::memory_order_relaxed);
            return;
        }
        if (! k->gestartet.load())
            return;
        bool frei = false;
        if (! k->taktLaeuft.compare_exchange_strong (frei, true))
        {
            k->uebersprungen.fetch_add (1, std::memory_order_relaxed);
            return;
        }
        struct Freigabe
        {
            std::atomic<bool>& laeuft;
            ~Freigabe() { laeuft.store (false); }
        } freigabe { k->taktLaeuft };

        k->takte.fetch_add (1, std::memory_order_relaxed);
        try
        {
            k->rumpf();
        }
        catch (...)
        {
            k->ausnahmen.fetch_add (1, std::memory_order_relaxed);
        }
    }
};

//== Briefkasten ==============================================================

Briefkasten::Briefkasten()
    : kern (std::make_shared<Kern>())
{
    kern->wurzel = std::make_shared<EchteWurzelFassade>();
    kern->fs     = std::make_shared<EchteDateisystemFassade>();
    kern->uhr    = std::make_shared<EchteUhrFassade>();
}

Briefkasten::~Briefkasten()
{
    stoppe();
}

void Briefkasten::setzeFassaden (std::shared_ptr<WurzelFassade> wurzel,
                                 std::shared_ptr<DateisystemFassade> dateisystem,
                                 std::shared_ptr<UhrFassade> uhr)
{
    if (kern->gestartet.load())
        return;
    if (wurzel != nullptr)      kern->wurzel = std::move (wurzel);
    if (dateisystem != nullptr) kern->fs = std::move (dateisystem);
    if (uhr != nullptr)         kern->uhr = std::move (uhr);
}

void Briefkasten::setzeAntwortquelle (Antwortquelle quelle)
{
    kern->quelle = std::move (quelle);
}

void Briefkasten::setzeHakenVorSchleuse (std::function<void()> haken)
{
    kern->hakenVorSchleuse = std::move (haken);
}

void Briefkasten::setzeHakenBeimStopp (std::function<void()> haken)
{
    kern->hakenBeimStopp = std::move (haken);
}

Startgrund Briefkasten::starte (const Konfiguration& konfiguration, bool mitTimer)
{
    auto& k = *kern;
    if (k.gestartet.load())
        return Startgrund::gestartet;
    const auto setze = [&k] (Startgrund g)
    {
        k.grund.store ((int) g);
        return g;
    };

    // Die Wurzel kommt vom Prozessor (P-11): genau drei Ebenen, keine davon
    // leer, kein Laufwerk, kein `.` oder `..`.
    std::array<std::wstring, 3> teile;
    {
        std::size_t anzahl = 0;
        std::wstring stueck;
        bool zuViele = false;
        for (const wchar_t c : konfiguration.wurzelRelativ + L"\\")
        {
            if (c != L'\\')
            {
                stueck.push_back (c);
                continue;
            }
            if (anzahl >= teile.size())
            {
                zuViele = true;
                break;
            }
            teile[anzahl++] = stueck;
            stueck.clear();
        }
        bool gueltig = ! zuViele && anzahl == teile.size();
        for (const auto& t : teile)
            gueltig = gueltig && ! t.empty() && t != L"." && t != L".."
                   && t.find_first_of (L"/:") == std::wstring::npos;
        if (! gueltig)
            return setze (Startgrund::konfiguration);
    }
    if ((konfiguration.rolle != "gen" && konfiguration.rolle != "probeeq")
        || ! kennungGueltig (konfiguration.laufzeit32.data(), konfiguration.laufzeit32.size())
        || k.wurzel == nullptr || k.fs == nullptr || k.uhr == nullptr)
        return setze (Startgrund::konfiguration);

    // F-1: genau eine Wurzelabfrage ...
    std::wstring wurzelPfad;
    if (! k.wurzel->wurzel (wurzelPfad) || wurzelPfad.empty())
        return setze (Startgrund::keineWurzel);
    while (! wurzelPfad.empty() && (wurzelPfad.back() == L'\\' || wurzelPfad.back() == L'/'))
        wurzelPfad.pop_back();

    std::array<std::wstring, 4> ebene;
    {
        auto pfad = wurzelPfad;
        for (std::size_t i = 0; i < teile.size(); ++i)
        {
            pfad += L"\\" + teile[i];
            ebene[i] = pfad;
        }
        ebene[3] = pfad + L"\\antwort";
    }

    // ... und die Ebenen von oben, je eine Attributpruefung, bis zur ersten
    // fehlenden Ebene oder zum ersten Reparse-Punkt (k + 1 Pruefungen).
    for (const auto& pfad : ebene)
    {
        const auto a = k.fs->attribute (pfad);
        if (! a.existiert)
            break;
        if (istVerlinkt (a))
            return setze (Startgrund::reparsePunkt);
        if (! a.verzeichnis)
            break;
    }

    k.ebene      = ebene;
    k.anfrage    = ebene[2] + L"\\anfrage.json";
    k.rolle      = konfiguration.rolle;
    k.laufzeit32 = konfiguration.laufzeit32;
    k.pid        = (std::uint32_t) GetCurrentProcessId();
    k.gestartet.store (true);
    k.starts.fetch_add (1, std::memory_order_relaxed);
    if (mitTimer)
        startTimer (kTaktMs);
    return setze (Startgrund::gestartet);
}

void Briefkasten::stoppe()
{
    auto& k = *kern;
    if (k.gestoppt.exchange (true))
        return;
    if (k.hakenBeimStopp)
        k.hakenBeimStopp();
    stopTimer();
    k.schleuse.schliessen();
}

void Briefkasten::takt()
{
    const auto k = kern;
    Kern::takt (k);
}

void Briefkasten::timerCallback()
{
    const auto k = kern;
    Kern::takt (k);
}

Startgrund Briefkasten::startgrund() const noexcept
{
    return (Startgrund) kern->grund.load();
}

Zaehler Briefkasten::zaehler() const noexcept
{
    const auto& k = *kern;
    Zaehler z;
    z.starts              = k.starts.load();
    z.takteBegonnen       = k.begonnen.load();
    z.takte               = k.takte.load();
    z.abgewieseneTakte    = k.abgewiesen.load();
    z.uebersprungeneTakte = k.uebersprungen.load();
    z.laufendMax          = k.laufendMax.load();
    z.warteTakte          = k.warteTakte.load();
    z.schreibversuche     = k.schreibversuche.load();
    z.antworten           = k.antworten.load();
    z.fehlerzaehler       = k.aufgegeben.load();
    z.ausnahmen           = k.ausnahmen.load();
    return z;
}

FassadenStand Briefkasten::dateisystemStand() const noexcept
{
    return kern->fs != nullptr ? kern->fs->stand() : FassadenStand {};
}

std::uint64_t Briefkasten::wurzelabfragen() const noexcept
{
    return kern->wurzel != nullptr ? kern->wurzel->wurzelabfragen() : 0;
}

std::function<Zaehler()> Briefkasten::zaehlerZeuge() const
{
    return [k = kern]
    {
        Zaehler z;
        z.starts              = k->starts.load();
        z.takteBegonnen       = k->begonnen.load();
        z.takte               = k->takte.load();
        z.abgewieseneTakte    = k->abgewiesen.load();
        z.uebersprungeneTakte = k->uebersprungen.load();
        z.laufendMax          = k->laufendMax.load();
        z.warteTakte          = k->warteTakte.load();
        z.schreibversuche     = k->schreibversuche.load();
        z.antworten           = k->antworten.load();
        z.fehlerzaehler       = k->aufgegeben.load();
        z.ausnahmen           = k->ausnahmen.load();
        return z;
    };
}

} // namespace nakama::diagnose

#include "NakamaKernRiegel.h"   // K1-Endpruefung nach allen Kern-Headern
