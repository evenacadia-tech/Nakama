/*  NAK-286 Etappe 2 - der Diagnose-Briefkasten im Plugin (Kanon B30).

    Misst die Verhaltensmatrix aus docs/beweise/NAK-286.md, Block 4.3 (M-23 bis
    M-40, M-77, M-79, M-80, M-81) und die C++-Haelften aus 4.5 und 4.6 (M-48
    bis M-54), an den ECHTEN Prozessoren Gen (`EqCopilotProcessor`) und
    Probeeq (`SondeProcessor`).

    Aufruf:
      EqCopBriefkastenTest.exe             alle Faelle; Exit 0 gruen, 1 rot
      EqCopBriefkastenTest.exe --erzeuge   schreibt die Referenz des Knopfwegs
                                           (M-39, Manifest §5.2) nach
                                           eq-copilot/fixtures/diagnose/
                                           festhalten-referenz.json

    Jede Zeile beginnt mit `[ok]  ` oder `[ROT] ` und nennt die Matrixzeile;
    ein Rotbeweis zaehlt nur mit einer `[ROT]`-Zeile, die den Traeger der
    Zusage nennt (Manifest §5.1).

    LANDMINE NAK-175: jeder Prozessor liegt auf dem Heap.
*/

#include "PluginProcessor.h"
#include "SondeProcessor.h"

#include <juce_cryptography/juce_cryptography.h>

#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <new>
#include <set>
#include <string>
#include <thread>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
 #define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
 #define NOMINMAX
#endif
#include <windows.h>
#include <aclapi.h>
#include <sddl.h>

// M-51: Allokationen zaehlen, nur solange ein Fall es auf SEINEM Thread
// einschaltet (Muster Sonde012LoudnessSourceTest.cpp).
namespace
{
thread_local bool zaehleAllokationen = false;
std::atomic<std::uint64_t> allokationen { 0 };
}

void* operator new (std::size_t groesse)
{
    if (zaehleAllokationen) allokationen.fetch_add (1, std::memory_order_relaxed);
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAllokationen) allokationen.fetch_add (1, std::memory_order_relaxed);
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

namespace
{

int bestanden = 0;
int fehlgeschlagen = 0;

void fall (const char* zeile, const char* name, bool ok, const juce::String& detail = {})
{
    std::cout << (ok ? "[ok]  " : "[ROT] ") << zeile << " " << name;
    if (detail.isNotEmpty())
        std::cout << " [" << detail.toStdString() << "]";
    std::cout << std::endl;
    ok ? ++bestanden : ++fehlgeschlagen;
}

bool warteBis (const std::function<bool()>& bedingung, int fristMs)
{
    const auto ende = std::chrono::steady_clock::now() + std::chrono::milliseconds (fristMs);
    for (;;)
    {
        if (bedingung())
            return true;
        if (std::chrono::steady_clock::now() >= ende)
            return bedingung();
        std::this_thread::sleep_for (std::chrono::milliseconds (1));
    }
}

juce::File repoWurzel()
{
    auto d = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
    while (d.exists() && ! d.getChildFile ("eq-copilot").isDirectory())
    {
        const auto oben = d.getParentDirectory();
        if (oben == d)
            break;
        d = oben;
    }
    return d;
}

juce::String sha256Hex (const juce::MemoryBlock& daten)
{
    return juce::SHA256 (daten.getData(), daten.getSize()).toHexString();
}

//==============================================================================
// REFERENZBUEHNE DES KNOPFWEGS (M-39, Manifest §5.2).
//
// Dieser Abschnitt erzeugt am Basis-SHA der Etappe 2 mit der UNVERAENDERTEN
// `schreibeSnapshotDatei` die Referenz und misst danach denselben Weg gegen
// sie. Er bleibt nach der Erzeugung unveraendert: jede Aenderung hier waere
// eine andere Buehne, und die Referenz bewiese nichts mehr.
//
// Deterministisch heisst hier drei Dinge:
//   1. fester Zustand (Instanzkennung, Label) und fester Sampleplan;
//   2. die schwere Auswertung faellt NUR an Zuggrenzen: der Zug wird
//      eingestellt, der Worker verbraucht ihn (die Quarantaene haelt den
//      letzten Block), und erst dann darf die faellige Auswertung laufen.
//      Faellt eine mitten in einen Zug, ist der Versuch ungueltig und wird
//      wiederholt (hoechstens dreimal) - die Konvergenzreferenz haengt sonst
//      an der Wanduhr;
//   3. keine leichte Publikation nach der letzten schweren (ohne neue
//      Samples publiziert `auswertenLeicht` nichts).
namespace referenz
{
constexpr double kRate          = 48000.0;
constexpr int    kBlock         = 512;
constexpr int    kBloeckeJeZug  = 40;
constexpr int    kZuege         = 40;
constexpr double kPi            = 3.14159265358979323846;
constexpr const char* kInstanz  = "0123456789abcdef0123456789abcdef";
constexpr const char* kLabel    = "NAK286Referenz";

struct Rauschen
{
    std::uint32_t s = 0x2545f491u;
    float naechster() noexcept
    {
        s = s * 1664525u + 1013904223u;
        return ((float) (s >> 8) / (float) 0x00ffffff) * 2.0f - 1.0f;
    }
};

void fuelleBlock (juce::AudioBuffer<float>& b, std::int64_t start, Rauschen& r)
{
    for (int i = 0; i < kBlock; ++i)
    {
        const double t = (double) (start + i) / kRate;
        const float bett = 0.05f * r.naechster();
        const float l = bett + 0.2f * (float) std::sin (2.0 * kPi * 250.0 * t)
                             + 0.08f * (float) std::sin (2.0 * kPi * 1500.0 * t);
        const float rechts = 0.9f * l + 0.03f * r.naechster();
        b.setSample (0, i, l);
        b.setSample (1, i, rechts);
    }
}

struct Ergebnis
{
    bool         ok = false;
    juce::String grund;
    int          versuche = 0;
    juce::String dateiname;
    juce::String ordner;
    bool         dateiEntfernt = false;
    std::string  maskiert;
};

/** Ersetzt den Wert von `created_utc` durch eine feste Marke. */
bool maskiere (std::string& text)
{
    const std::string schluessel = "\"created_utc\": \"";
    const auto a = text.find (schluessel);
    if (a == std::string::npos || text.find (schluessel, a + 1) != std::string::npos)
        return false;
    const auto wertAnfang = a + schluessel.size();
    const auto wertEnde = text.find ('"', wertAnfang);
    if (wertEnde == std::string::npos)
        return false;
    text.replace (wertAnfang, wertEnde - wertAnfang, "<maskiert>");
    return true;
}

Ergebnis fahre()
{
    Ergebnis e;
    for (int versuch = 1; versuch <= 3; ++versuch)
    {
        e.versuche = versuch;
        auto p = std::make_unique<eqcop::EqCopilotProcessor>();

        auto z = nakama::state::frisch (kInstanz);
        z.common.label = kLabel;
        juce::MemoryBlock stand;
        nakama::state::speichere (z, stand);
        p->setStateInformation (stand.getData(), (int) stand.getSize());

        p->setPlayConfigDetails (2, 2, kRate, kBlock);
        p->prepareToPlay (kRate, kBlock);

        juce::AudioBuffer<float> puffer (2, kBlock);
        juce::MidiBuffer midi;
        Rauschen rauschen;
        std::int64_t zeit = 0;
        std::uint64_t gefuettert = 0;
        bool gueltig = true;

        for (int zug = 0; zug < kZuege && gueltig; ++zug)
        {
            const auto schwerVorher = p->analyseSchwereAuswertungen();
            for (int b = 0; b < kBloeckeJeZug; ++b)
            {
                eqcop::hostbruecke::Blockbefund befund;
                befund.kontext.processContextPresent = true;
                befund.kontext.projectTimeSamples.setze (zeit);
                befund.kontext.playing.setze (true);
                befund.kontext.recording.setze (false);
                befund.kontext.sampleRate.setze (kRate);
                befund.blockGroesse = (std::uint32_t) kBlock;
                p->nakamaBlockEmpfangen (befund);
                fuelleBlock (puffer, zeit, rauschen);
                p->processBlock (puffer, midi);
                zeit += kBlock;
                ++gefuettert;
            }
            // Die Quarantaene haelt den juengsten Block: verbraucht sind alle
            // bis auf ihn. `merkmaleBloecke` liest unter der Steuersperre und
            // kehrt deshalb erst zwischen zwei Workerzuegen zurueck.
            if (! warteBis ([&] { return p->merkmaleBloecke() + 1 >= gefuettert; }, 10000))
            {
                e.grund = "der Worker hat den Zug nicht verbraucht";
                gueltig = false;
                break;
            }
            if (p->analyseSchwereAuswertungen() != schwerVorher)
            {
                e.grund = "schwere Auswertung mitten im Zug " + juce::String (zug);
                gueltig = false;
                break;
            }
            if (! warteBis ([&] { return p->analyseSchwereAuswertungen() == schwerVorher + 1; }, 10000))
            {
                e.grund = "keine schwere Auswertung am Ende von Zug " + juce::String (zug);
                gueltig = false;
                break;
            }
            (void) p->merkmaleBloecke();   // wartet die laufende Auswertung ab
        }
        if (! gueltig)
            continue;

        juce::String pfad;
        if (! p->schreibeSnapshotDatei (pfad))
        {
            e.grund = "der Knopfweg schrieb nicht: " + pfad;
            return e;
        }
        const juce::File datei (pfad);
        e.dateiname = datei.getFileName();
        e.ordner = datei.getParentDirectory().getFullPathName();
        juce::MemoryBlock bytes;
        const bool gelesen = datei.loadFileAsData (bytes);
        e.dateiEntfernt = datei.deleteFile() && ! datei.exists();
        if (! gelesen)
        {
            e.grund = "Snapshotdatei nicht lesbar: " + pfad;
            return e;
        }
        e.maskiert.assign ((const char*) bytes.getData(), bytes.getSize());
        if (! maskiere (e.maskiert))
        {
            e.grund = "created_utc nicht genau einmal gefunden";
            return e;
        }
        e.ok = true;
        return e;
    }
    return e;
}

juce::File referenzDatei()
{
    return repoWurzel().getChildFile ("eq-copilot").getChildFile ("fixtures")
                       .getChildFile ("diagnose").getChildFile ("festhalten-referenz.json");
}

juce::File snapshotOrdner()
{
    return juce::File::getSpecialLocation (juce::File::windowsLocalAppData)
               .getChildFile ("evenacadia").getChildFile ("EQ-Copilot").getChildFile ("snapshots");
}

bool nameWieKnopfweg (const juce::String& name)
{
    // snapshot-<JJJJMMTT>-<HHMMSS>-<Label>.json
    const juce::String kopf = "snapshot-";
    const juce::String fuss = juce::String ("-") + kLabel + ".json";
    if (! name.startsWith (kopf) || ! name.endsWith (fuss))
        return false;
    const auto zeit = name.substring (kopf.length(), name.length() - fuss.length());
    if (zeit.length() != 15 || zeit[8] != '-')
        return false;
    for (int i = 0; i < zeit.length(); ++i)
        if (i != 8 && ! juce::CharacterFunctions::isDigit (zeit[i]))
            return false;
    return true;
}
} // namespace referenz
// ENDE DER REFERENZBUEHNE
//==============================================================================

int erzeugeReferenz()
{
    const auto e = referenz::fahre();
    if (! e.ok)
    {
        std::cout << "REFERENZ NICHT ERZEUGT: " << e.grund.toStdString()
                  << " (Versuche " << e.versuche << ")" << std::endl;
        return 1;
    }
    const auto ziel = referenz::referenzDatei();
    if (! ziel.getParentDirectory().createDirectory())
    {
        std::cout << "REFERENZ NICHT ERZEUGT: Ordner nicht anlegbar: "
                  << ziel.getParentDirectory().getFullPathName().toStdString() << std::endl;
        return 1;
    }
    if (! ziel.replaceWithData (e.maskiert.data(), e.maskiert.size()))
    {
        std::cout << "REFERENZ NICHT ERZEUGT: nicht schreibbar: " << ziel.getFullPathName().toStdString() << std::endl;
        return 1;
    }
    juce::MemoryBlock geschrieben;
    ziel.loadFileAsData (geschrieben);
    const auto json = juce::JSON::parse (juce::String::fromUTF8 (e.maskiert.data(), (int) e.maskiert.size()));
    std::cout << "REFERENZ ERZEUGT: " << ziel.getFullPathName().toStdString() << std::endl
              << "  Bytes " << geschrieben.getSize() << ", SHA-256 " << sha256Hex (geschrieben).toStdString() << std::endl
              << "  Versuche " << e.versuche << ", Knopfwegdatei " << e.dateiname.toStdString()
              << " in " << e.ordner.toStdString() << ", entfernt " << (e.dateiEntfernt ? "ja" : "nein") << std::endl
              << "  zustand " << json["zustand"].toString().toStdString()
              << ", resonanzen " << json["resonanzen"].size()
              << ", befunde " << json["befunde"].size()
              << ", konvergenz.gueltig " << (bool) json["konvergenz"]["gueltig"] << std::endl;
    return e.dateiEntfernt ? 0 : 1;
}

void festhaltenBytegleich()
{
    const auto referenzDatei = referenz::referenzDatei();
    juce::MemoryBlock soll;
    if (! referenzDatei.loadFileAsData (soll))
    {
        fall ("M-39", "festhalten_bytegleich", false,
              "Referenz fehlt: " + referenzDatei.getFullPathName());
        return;
    }
    const auto e = referenz::fahre();
    if (! e.ok)
    {
        fall ("M-39", "festhalten_bytegleich", false, "Buehne: " + e.grund);
        return;
    }
    const bool ordnerOk = juce::File (e.ordner) == referenz::snapshotOrdner();
    const bool nameOk = referenz::nameWieKnopfweg (e.dateiname);
    fall ("M-39", "festhalten_ordner_und_name_wie_knopfweg", ordnerOk && nameOk,
          e.ordner + "\\" + e.dateiname);
    fall ("M-39", "festhalten_test_entfernt_eigene_datei", e.dateiEntfernt, e.dateiname);

    const bool gleich = soll.getSize() == e.maskiert.size()
                     && std::memcmp (soll.getData(), e.maskiert.data(), e.maskiert.size()) == 0;
    juce::String detail = "Referenz " + juce::String ((juce::int64) soll.getSize()) + " Bytes, Knopfweg "
                        + juce::String ((juce::int64) e.maskiert.size()) + " Bytes, Versuche "
                        + juce::String (e.versuche);
    if (! gleich)
    {
        std::size_t i = 0;
        const auto n = std::min (soll.getSize(), e.maskiert.size());
        while (i < n && ((const char*) soll.getData())[i] == e.maskiert[i])
            ++i;
        detail += ", erste Abweichung bei Byte " + juce::String ((juce::int64) i);
    }
    fall ("M-39", "festhalten_bytegleich (Knopfweg gegen Referenz, created_utc maskiert)", gleich, detail);
}

//==============================================================================
// BUEHNE DER UEBRIGEN FAELLE: Temp-Wurzel, Fassaden-Attrappen, Zeugen.
//
// Jeder Fall gibt Temp-Wurzel und Fassaden herein und startet den Briefkasten
// ueber den Testzugang - dieselbe Startfunktion, die der Produktkonstruktor
// ruft (F-16). Ein Takt laeuft synchron auf dem Message-Thread (dem Hauptthread
// dieses Ziels); nur M-30, M-34 und M-36 fahren den echten Timer.

namespace dg = nakama::diagnose;
using Gen = eqcop::EqCopilotProcessor;
using Probeeq = nakama::sonde::SondeProcessor;

juce::String zahl (std::uint64_t v) { return juce::String ((juce::int64) v); }

/** Simuliert %LOCALAPPDATA% unter %TEMP%, mit den Ebenen aus F-1. */
struct TempWurzel
{
    juce::File wurzel;
    std::vector<std::wstring> links;   ///< vor dem Aufraeumen einzeln entfernt

    explicit TempWurzel (bool mitDiagnose = true, bool mitAntwort = true)
    {
        static std::atomic<int> laufend { 0 };
        wurzel = juce::File::getSpecialLocation (juce::File::tempDirectory)
                     .getChildFile ("nak286-" + juce::String ((int) GetCurrentProcessId()) + "-"
                                    + juce::String (++laufend));
        wurzel.deleteRecursively();
        nakama().createDirectory();
        if (mitDiagnose)
            diagnose().createDirectory();
        if (mitDiagnose && mitAntwort)
            antwort().createDirectory();
    }
    ~TempWurzel()
    {
        for (auto it = links.rbegin(); it != links.rend(); ++it)
        {
            const auto a = GetFileAttributesW (it->c_str());
            if (a == INVALID_FILE_ATTRIBUTES)
                continue;
            if ((a & FILE_ATTRIBUTE_DIRECTORY) != 0)
                RemoveDirectoryW (it->c_str());
            else
                DeleteFileW (it->c_str());
        }
        wurzel.deleteRecursively (false);
    }
    juce::File evenacadia() const { return wurzel.getChildFile ("evenacadia"); }
    juce::File nakama() const     { return evenacadia().getChildFile ("nakama"); }
    juce::File diagnose() const   { return nakama().getChildFile ("diagnose"); }
    juce::File antwort() const    { return diagnose().getChildFile ("antwort"); }
    juce::File anfrage() const    { return diagnose().getChildFile ("anfrage.json"); }
    std::wstring pfad() const     { return wurzel.getFullPathName().toWideCharPointer(); }
};

bool istReparse (const juce::File& f)
{
    const auto a = GetFileAttributesW (f.getFullPathName().toWideCharPointer());
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_REPARSE_POINT) != 0;
}

/** Eine Junction ohne Adminrechte (§5.5 Risiken (c)). */
bool legeJunctionAn (TempWurzel& t, const juce::File& link, const juce::File& ziel)
{
    juce::ChildProcess p;
    if (! p.start (juce::StringArray { "cmd.exe", "/c", "mklink", "/J",
                                        link.getFullPathName(), ziel.getFullPathName() }))
        return false;
    (void) p.readAllProcessOutput();
    p.waitForProcessToFinish (15000);
    t.links.push_back (link.getFullPathName().toWideCharPointer());
    return istReparse (link);
}

/** Ein Datei-Symlink, nur mit dem Recht dazu (M-49 (c)). */
bool legeSymlinkAn (TempWurzel& t, const juce::File& link, const juce::File& ziel, DWORD& fehler)
{
    constexpr DWORD kOhneAdmin = 0x2;   // SYMBOLIC_LINK_FLAG_ALLOW_UNPRIVILEGED_CREATE
    fehler = 0;
    if (CreateSymbolicLinkW (link.getFullPathName().toWideCharPointer(),
                             ziel.getFullPathName().toWideCharPointer(), kOhneAdmin) == 0)
    {
        fehler = GetLastError();
        return false;
    }
    t.links.push_back (link.getFullPathName().toWideCharPointer());
    return istReparse (link);
}

class TestWurzel final : public dg::WurzelFassade
{
public:
    TestWurzel (std::wstring p, bool v) : pfad (std::move (p)), vorhanden (v) {}
    std::wstring pfad;
    bool vorhanden;

protected:
    bool wurzelImpl (std::wstring& aus) override
    {
        aus = pfad;
        return vorhanden;
    }
};

class TestUhr final : public dg::UhrFassade
{
public:
    std::atomic<std::int64_t> ms { 1789387200000 };   ///< 2026-09-14T12:00:00.000Z
    std::int64_t jetztUtcMs() override { return ms.load(); }
};

/** Die echten Aufrufe unter der Temp-Wurzel, mit Injektion je Schritt (F-14)
    und dem Thread jedes Aufrufs. Ein Pfad ausserhalb der Temp-Wurzel wird nie
    ausgefuehrt, nur gezaehlt. Legt selbst keinen Speicher an (M-51). */
class PruefFassade final : public dg::EchteDateisystemFassade
{
public:
    explicit PruefFassade (std::wstring p) : praefix (std::move (p)) {}

    /// > 0: so oft scheitern; -1: immer.
    std::atomic<int> injLesen { 0 }, injAnlegen { 0 }, injSchreiben { 0 }, injUmbenennen { 0 };
    std::atomic<DWORD> audioThread { 0 };
    std::atomic<std::uint64_t> aufAudiothread { 0 }, ausserhalbMessageThread { 0 }, ausserhalbWurzel { 0 };

protected:
    dg::DateiAttribute attributeImpl (const std::wstring& p) override
    {
        if (! pruefe (p))
            return {};
        return EchteDateisystemFassade::attributeImpl (p);
    }
    bool liesImpl (const std::wstring& p, char* ziel, std::size_t hoechstens, std::size_t& gelesen) override
    {
        if (! pruefe (p) || injiziere (injLesen))
            return false;
        return EchteDateisystemFassade::liesImpl (p, ziel, hoechstens, gelesen);
    }
    Handle legeExklusivAnImpl (const std::wstring& p) override
    {
        if (! pruefe (p) || injiziere (injAnlegen))
            return 0;
        return EchteDateisystemFassade::legeExklusivAnImpl (p);
    }
    bool schreibeUndSpueleImpl (Handle datei, const char* daten, std::size_t anzahl) override
    {
        merke();
        if (injiziere (injSchreiben))
            return false;   // vor dem ersten Byte (M-53 (a))
        return EchteDateisystemFassade::schreibeUndSpueleImpl (datei, daten, anzahl);
    }
    void schliesseImpl (Handle datei) override
    {
        merke();
        EchteDateisystemFassade::schliesseImpl (datei);
    }
    bool groesseImpl (const std::wstring& p, std::int64_t& aus) override
    {
        if (! pruefe (p))
            return false;
        return EchteDateisystemFassade::groesseImpl (p, aus);
    }
    bool benenneUmOhneErsetzenImpl (const std::wstring& von, const std::wstring& nach) override
    {
        if (! pruefe (von) || ! pruefe (nach) || injiziere (injUmbenennen))
            return false;
        return EchteDateisystemFassade::benenneUmOhneErsetzenImpl (von, nach);
    }
    bool legeOrdnerAnImpl (const std::wstring& p) override
    {
        if (! pruefe (p))
            return false;
        return EchteDateisystemFassade::legeOrdnerAnImpl (p);
    }
    bool loescheImpl (const std::wstring& p) override
    {
        if (! pruefe (p))
            return false;
        return EchteDateisystemFassade::loescheImpl (p);
    }

private:
    std::wstring praefix;

    void merke() noexcept
    {
        const auto a = audioThread.load();
        if (a != 0 && GetCurrentThreadId() == a)
            aufAudiothread.fetch_add (1);
        if (! juce::MessageManager::existsAndIsCurrentThread())
            ausserhalbMessageThread.fetch_add (1);
    }
    bool pruefe (const std::wstring& p) noexcept
    {
        merke();
        if (p.size() < praefix.size() || _wcsnicmp (p.c_str(), praefix.c_str(), praefix.size()) != 0)
        {
            ausserhalbWurzel.fetch_add (1);
            return false;
        }
        return true;
    }
    static bool injiziere (std::atomic<int>& z) noexcept
    {
        int v = z.load();
        for (;;)
        {
            if (v == 0)
                return false;
            if (v < 0)
                return true;
            if (z.compare_exchange_weak (v, v - 1))
                return true;
        }
    }
};

struct FassadenSatz
{
    std::shared_ptr<TestWurzel>   wurzel;
    std::shared_ptr<PruefFassade> fs;
    std::shared_ptr<TestUhr>      uhr;
};

FassadenSatz fassaden (const TempWurzel& t, bool wurzelVorhanden = true)
{
    FassadenSatz s;
    s.wurzel = std::make_shared<TestWurzel> (t.pfad(), wurzelVorhanden);
    s.fs     = std::make_shared<PruefFassade> (t.pfad());
    s.uhr    = std::make_shared<TestUhr>();
    return s;
}

template <typename Prozessor>
dg::Startgrund starteMit (Prozessor& p, const FassadenSatz& s, bool mitTimer = false)
{
    p.briefkastenFuerTest().setzeFassaden (s.wurzel, s.fs, s.uhr);
    return p.briefkastenStartenFuerTest (mitTimer);
}

struct ZaehlLogger final : juce::Logger
{
    std::atomic<int> eintraege { 0 };
    void logMessage (const juce::String&) override { eintraege.fetch_add (1); }
};

/** Zaehlt jeden Logeintrag, solange er lebt (M-27, M-77; T-14). */
struct LoggerAufsicht
{
    ZaehlLogger   logger;
    juce::Logger* vorher = juce::Logger::getCurrentLogger();
    LoggerAufsicht()  { juce::Logger::setCurrentLogger (&logger); }
    ~LoggerAufsicht() { juce::Logger::setCurrentLogger (vorher); }
};

/** Host-Dirty und Parameterereignisse (M-37, M-77). */
struct DirtyZeuge final : juce::AudioProcessorListener
{
    std::atomic<int> geaendert { 0 }, parameter { 0 };
    void audioProcessorParameterChanged (juce::AudioProcessor*, int, float) override { parameter.fetch_add (1); }
    void audioProcessorChanged (juce::AudioProcessor*, const ChangeDetails&) override { geaendert.fetch_add (1); }
};

/** Die Nachrichtenschleife fuer hoechstens `ms` Millisekunden, wie ein Host
    sie pumpt: JUCE stellt seine Nachrichten (auch die Timer-Rueckrufe) ueber
    ein verstecktes Fenster zu. `runDispatchLoopUntil` sperrt JUCE 8 hinter
    JUCE_MODAL_LOOPS_PERMITTED, und der Kernriegel K2b laesst am Testziel keine
    abweichende JUCE-Konfiguration zu. `true`, sobald `bis` erfuellt ist. */
bool pumpe (int ms, const std::function<bool()>& bis = {})
{
    const auto ende = juce::Time::getMillisecondCounterHiRes() + (double) ms;
    for (;;)
    {
        MSG nachricht;
        while (PeekMessageW (&nachricht, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage (&nachricht);
            DispatchMessageW (&nachricht);
        }
        if (bis && bis())
            return true;
        if (juce::Time::getMillisecondCounterHiRes() >= ende)
            return bis ? bis() : true;
        MsgWaitForMultipleObjects (0, nullptr, FALSE, 5, QS_ALLINPUT);
    }
}

std::string neueKennung() { return juce::Uuid().toString().toStdString(); }

std::string anfrageText (const std::string& id)
{
    return "{\"format\":\"nakama.diagnose.anfrage.v1\",\"anfrage_id\":\"" + id + "\"}";
}

/** Je Schreibvorgang 1 s spaeter: der Merker (T-5) sieht jede Aenderung, auch
    innerhalb der Zeitaufloesung des Dateisystems. */
FILETIME naechsteAenderungszeit()
{
    static std::uint64_t t = 0;
    if (t == 0)
    {
        FILETIME jetzt;
        GetSystemTimeAsFileTime (&jetzt);
        t = ((std::uint64_t) jetzt.dwHighDateTime << 32) | jetzt.dwLowDateTime;
    }
    t += 10000000u;
    return FILETIME { (DWORD) (t & 0xffffffffu), (DWORD) (t >> 32) };
}

/** Die Buehne ist der Anfragende: eigene Aufrufe, nie die Fassade der Instanz. */
bool schreibeRoh (const juce::File& f, const std::string& bytes)
{
    const HANDLE h = CreateFileW (f.getFullPathName().toWideCharPointer(), GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_DELETE, nullptr, CREATE_ALWAYS,
                                  FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    DWORD geschrieben = 0;
    bool ok = bytes.empty()
           || (WriteFile (h, bytes.data(), (DWORD) bytes.size(), &geschrieben, nullptr) != FALSE
               && geschrieben == (DWORD) bytes.size());
    const auto zeit = naechsteAenderungszeit();
    ok = SetFileTime (h, nullptr, nullptr, &zeit) != FALSE && ok;
    CloseHandle (h);
    return ok;
}

bool schreibeAnfrage (const TempWurzel& t, const std::string& id)
{
    return schreibeRoh (t.anfrage(), anfrageText (id));
}

/** Nur die Aenderungszeit, gleicher Inhalt (M-25, M-27). */
bool beruehre (const juce::File& f)
{
    const HANDLE h = CreateFileW (f.getFullPathName().toWideCharPointer(), FILE_WRITE_ATTRIBUTES,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE)
        return false;
    const auto zeit = naechsteAenderungszeit();
    const bool ok = SetFileTime (h, nullptr, nullptr, &zeit) != FALSE;
    CloseHandle (h);
    return ok;
}

bool hexZeichen (const juce::String& s, int n)
{
    if (s.length() != n)
        return false;
    for (int i = 0; i < n; ++i)
    {
        const auto c = s[i];
        if (! ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')))
            return false;
    }
    return true;
}

/** `<anfrage_id>.<rolle>.<pid>.<instanz16>.<laufzeit32>.json` (F-3). */
bool istAntwortName (const juce::String& name, const juce::String& rolle)
{
    const auto teile = juce::StringArray::fromTokens (name, ".", "");
    return teile.size() == 6 && teile[5] == "json" && teile[1] == rolle
        && hexZeichen (teile[0], 32) && hexZeichen (teile[3], 16) && hexZeichen (teile[4], 32)
        && teile[2].isNotEmpty() && teile[2].containsOnly ("0123456789");
}

std::vector<juce::File> dateienIn (const juce::File& ordner)
{
    std::vector<juce::File> v;
    for (const auto& f : ordner.findChildFiles (juce::File::findFiles, false))
        v.push_back (f);
    return v;
}

std::vector<juce::File> antworten (const TempWurzel& t, const juce::String& rolle, const std::string& id = {})
{
    std::vector<juce::File> v;
    for (const auto& f : dateienIn (t.antwort()))
        if (istAntwortName (f.getFileName(), rolle)
            && (id.empty() || f.getFileName().startsWith (juce::String (id) + ".")))
            v.push_back (f);
    return v;
}

juce::var liesJson (const juce::File& f, bool& ok)
{
    juce::MemoryBlock bytes;
    ok = f.loadFileAsData (bytes);
    juce::var v;
    const auto r = juce::JSON::parse (juce::String::fromUTF8 ((const char*) bytes.getData(), (int) bytes.getSize()), v);
    ok = ok && r.wasOk() && v.getDynamicObject() != nullptr;
    return v;
}

std::set<std::string> schluessel (const juce::var& v)
{
    std::set<std::string> s;
    if (auto* o = v.getDynamicObject())
        for (const auto& p : o->getProperties())
            s.insert (p.name.toString().toStdString());
    return s;
}

bool hatGrund (const juce::var& u, const char* feld, const char* code)
{
    if (const auto* g = u["gruende"].getArray())
        for (const auto& e : *g)
            if (e["feld"].toString() == feld && e["code"].toString() == code)
                return true;
    return false;
}

juce::String sha256Datei (const juce::File& f)
{
    juce::MemoryBlock b;
    f.loadFileAsData (b);
    return sha256Hex (b);
}

juce::String stand (const dg::FassadenStand& s)
{
    return "Existenz " + zahl (s.existenzpruefungen) + ", Lese " + zahl (s.leseoeffnungen)
         + ", Bytes " + zahl (s.geleseneBytes) + ", Anlegen " + zahl (s.anlegeversuche)
         + ", Schreib " + zahl (s.schreibvorgaenge) + ", Groesse " + zahl (s.groessenabfragen)
         + ", Umbenennen " + zahl (s.umbenennungen) + ", Ordner " + zahl (s.ordneranlagen)
         + ", Loesch " + zahl (s.loeschungen);
}

dg::FassadenStand differenz (const dg::FassadenStand& b, const dg::FassadenStand& a)
{
    dg::FassadenStand d;
    d.existenzpruefungen = b.existenzpruefungen - a.existenzpruefungen;
    d.leseoeffnungen     = b.leseoeffnungen - a.leseoeffnungen;
    d.geleseneBytes      = b.geleseneBytes - a.geleseneBytes;
    d.anlegeversuche     = b.anlegeversuche - a.anlegeversuche;
    d.schreibvorgaenge   = b.schreibvorgaenge - a.schreibvorgaenge;
    d.groessenabfragen   = b.groessenabfragen - a.groessenabfragen;
    d.umbenennungen      = b.umbenennungen - a.umbenennungen;
    d.ordneranlagen      = b.ordneranlagen - a.ordneranlagen;
    d.loeschungen        = b.loeschungen - a.loeschungen;
    return d;
}

/** Deterministischer Sampleplan: Rauschbett und zwei Toene, stereo. */
void fuelle (juce::AudioBuffer<float>& b, std::int64_t start, double rate, referenz::Rauschen& r)
{
    for (int i = 0; i < b.getNumSamples(); ++i)
    {
        const double t = (double) (start + i) / rate;
        const float l = 0.05f * r.naechster() + 0.2f * (float) std::sin (2.0 * referenz::kPi * 250.0 * t)
                      + 0.08f * (float) std::sin (2.0 * referenz::kPi * 1500.0 * t);
        b.setSample (0, i, l);
        b.setSample (1, i, 0.9f * l + 0.03f * r.naechster());
    }
}

/** Ein Hostblock mit Hostzeit ueber die Hostbruecke, wie im Produkt. */
template <typename Prozessor>
void gibBlock (Prozessor& p, juce::AudioBuffer<float>& puffer, std::int64_t zeit, double rate)
{
    eqcop::hostbruecke::Blockbefund befund;
    befund.kontext.processContextPresent = true;
    befund.kontext.projectTimeSamples.setze (zeit);
    befund.kontext.playing.setze (true);
    befund.kontext.recording.setze (false);
    befund.kontext.sampleRate.setze (rate);
    befund.blockGroesse = (std::uint32_t) puffer.getNumSamples();
    p.nakamaBlockEmpfangen (befund);
    juce::MidiBuffer midi;
    p.processBlock (puffer, midi);
}

void ladeZustand (juce::AudioProcessor& p, const juce::String& instanz, const juce::String& label, bool probeeq)
{
    auto z = nakama::state::frisch (instanz);
    z.common.label = label;
    if (probeeq)
    {
        z.common.klasse = nakama::state::Klasse::active_probe;
        z.hatParameters = true;
    }
    juce::MemoryBlock bytes;
    nakama::state::speichere (z, bytes);
    p.setStateInformation (bytes.getData(), (int) bytes.getSize());
}

juce::MemoryBlock zustandBytes (juce::AudioProcessor& p)
{
    juce::MemoryBlock b;
    p.getStateInformation (b);
    return b;
}

std::vector<float> parameterWerte (juce::AudioProcessor& p)
{
    std::vector<float> v;
    for (auto* q : p.getParameters())
        v.push_back (q->getValue());
    return v;
}

struct GenLauf
{
    std::unique_ptr<Gen> p = std::make_unique<Gen>();   // NAK-175: Heap
    referenz::Rauschen r;
    double rate = 48000.0;
    int groesse = 512;
    std::uint64_t gefuettert = 0;
    std::uint64_t verworfen = 0;   ///< Bloecke, die die Quarantaene an einem Bruch verwarf

    void bereite (double sr, int g)
    {
        rate = sr;
        groesse = g;
        p->setPlayConfigDetails (2, 2, sr, g);
        p->prepareToPlay (sr, g);
    }
    /// Die Quarantaene haelt den juengsten Block; verbraucht sind alle anderen.
    bool verbraucht()
    {
        const auto soll = gefuettert;
        return warteBis ([&] { return p->merkmaleBloecke() + 1 + verworfen >= soll; }, 10000);
    }
    bool fuettere (int n, std::int64_t& zeit)
    {
        juce::AudioBuffer<float> puffer (2, groesse);
        for (int i = 0; i < n; ++i)
        {
            fuelle (puffer, zeit, rate, r);
            gibBlock (*p, puffer, zeit, rate);
            zeit += groesse;
            ++gefuettert;
            if (gefuettert % 16 == 0 && ! verbraucht())
                return false;
        }
        return verbraucht();
    }
    /// Messdaten: n Bloecke, dann eine Publikation mit Daten.
    bool mitDaten (int n)
    {
        bereite (48000.0, 512);
        std::int64_t zeit = 0;
        return fuettere (n, zeit)
            && warteBis ([&] { return p->messSnapshot().zustand != eqcop::MessZustand::keineDaten; }, 10000);
    }
};

struct ProbeeqLauf
{
    std::unique_ptr<Probeeq> p = std::make_unique<Probeeq>();   // NAK-175: Heap
    referenz::Rauschen r;
    double rate = 48000.0;
    int groesse = 512;

    void bereite (double sr, int g)
    {
        rate = sr;
        groesse = g;
        p->setPlayConfigDetails (2, 2, sr, g);
        p->prepareToPlay (sr, g);
    }
    bool geleert() { return warteBis ([&] { return p->analyseQueueLeerFuerTest(); }, 10000); }
    bool fuettere (int n, std::int64_t& zeit)
    {
        juce::AudioBuffer<float> puffer (2, groesse);
        for (int i = 0; i < n; ++i)
        {
            fuelle (puffer, zeit, rate, r);
            gibBlock (*p, puffer, zeit, rate);
            zeit += groesse;
            if ((i + 1) % 16 == 0 && ! geleert())
                return false;
        }
        return geleert();
    }
    /// Ein voller Workerdurchlauf nach jetzt (der laufende zaehlt nicht).
    bool durchlauf()
    {
        const auto d0 = p->workerDurchlaeufeFuerTest();
        return warteBis ([&] { return p->workerDurchlaeufeFuerTest() >= d0 + 2; }, 5000);
    }
    /// P-9: der erste Takt wartet, ein Workerdurchlauf, der zweite schreibt.
    bool taktPaar()
    {
        p->briefkastenFuerTest().takt();
        const bool ok = durchlauf();
        p->briefkastenFuerTest().takt();
        return ok;
    }
};

/// M-40 liest die Antworten aus M-24, M-31 und M-32.
juce::var antwortM24, antwortM31Gen, antwortM31Probeeq, antwortM32;

//==============================================================================
// M-23 ohne_anfrage_nur_existenzpruefung
void ohneAnfrageNurExistenzpruefung()
{
    TempWurzel t;
    auto gen = std::make_unique<Gen>();
    auto pq = std::make_unique<Probeeq>();
    const auto fg = fassaden (t);
    const auto fp = fassaden (t);
    const bool start = starteMit (*gen, fg) == dg::Startgrund::gestartet
                    && starteMit (*pq, fp) == dg::Startgrund::gestartet;
    fall ("M-23", "ohne_anfrage_nur_existenzpruefung vorbedingung (beide gestartet, keine anfrage.json)",
          start && ! t.anfrage().exists());

    const auto miss = [&] (const char* rolle, dg::Briefkasten& bk, const FassadenSatz& f)
    {
        // Taktfenster: Differenz gegen den Stand nach der Startfunktion (F-14).
        const auto s0 = f.fs->stand();
        const auto w0 = f.wurzel->wurzelabfragen();
        for (int i = 0; i < 600; ++i)
            bk.takt();
        const auto d = differenz (f.fs->stand(), s0);
        const auto w = f.wurzel->wurzelabfragen() - w0;
        const bool ok = d.existenzpruefungen == 600 && d.leseoeffnungen == 0 && d.geleseneBytes == 0
                     && d.anlegeversuche == 0 && d.schreibvorgaenge == 0 && d.groessenabfragen == 0
                     && d.umbenennungen == 0 && d.ordneranlagen == 0 && d.loeschungen == 0 && w == 0;
        fall ("M-23", (std::string ("ohne_anfrage_nur_existenzpruefung ") + rolle
                       + " (600 Takte: 600 Existenzpruefungen, sonst 0)").c_str(),
              ok, stand (d) + ", Wurzel " + zahl (w));
    };
    miss ("gen", gen->briefkastenFuerTest(), fg);
    miss ("probeeq", pq->briefkastenFuerTest(), fp);
    fall ("M-23", "ohne_anfrage_nur_existenzpruefung antwortordner_leer", dateienIn (t.antwort()).empty());
}

//==============================================================================
// M-24 anfrage_gen_genau_eine_antwort
void anfrageGenGenauEineAntwort()
{
    TempWurzel t;
    GenLauf g;
    const auto f = fassaden (t);
    const bool daten = g.mitDaten (200);
    const bool start = starteMit (*g.p, f) == dg::Startgrund::gestartet;
    fall ("M-24", "anfrage_gen_genau_eine_antwort vorbedingung (Messdaten, gestartet)", daten && start);

    const auto id = neueKennung();
    schreibeAnfrage (t, id);
    const auto anfrageBytes = (std::uint64_t) t.anfrage().getSize();
    const auto s0 = f.fs->stand();
    g.p->briefkastenFuerTest().takt();
    const auto d = differenz (f.fs->stand(), s0);
    const auto z = g.p->briefkastenFuerTest().zaehler();
    const auto alle = dateienIn (t.antwort());
    const auto passend = antworten (t, "gen", id);
    fall ("M-24", "anfrage_gen_genau_eine_antwort genau_eine_datei_im_ersten_takt",
          alle.size() == 1 && passend.size() == 1 && z.takte == 1 && z.antworten == 1,
          "Dateien " + juce::String ((int) alle.size()) + ", Takte " + zahl (z.takte)
              + ", Antworten " + zahl (z.antworten));
    // Ein Takt mit Antwort: 1 + 3 Ebenen vor dem Lesen + 4 vor dem Schreiben.
    fall ("M-24", "anfrage_gen_genau_eine_antwort jeder_zugriff_an_der_fassade",
          d.existenzpruefungen == 8 && d.leseoeffnungen == 1 && d.geleseneBytes == anfrageBytes
              && d.anlegeversuche == 1 && d.schreibvorgaenge == 1 && d.groessenabfragen == 1
              && d.umbenennungen == 1 && d.ordneranlagen == 0 && d.loeschungen == 0
              && f.fs->ausserhalbWurzel.load() == 0,
          stand (d));
    if (passend.size() != 1)
        return;

    bool ok = false;
    const auto u = liesJson (passend[0], ok);
    const auto zustand = g.p->holeZustandKopie();
    const auto instanz = zustand.common.instanceId;
    const auto instanz16 = juce::SHA256 (instanz.toRawUTF8(), instanz.getNumBytesAsUTF8()).toHexString().substring (0, 16);
    const auto laufzeit = g.p->holeRuntimeNonce();
    const auto pid = juce::String ((juce::int64) GetCurrentProcessId());
    const auto teile = juce::StringArray::fromTokens (passend[0].getFileName(), ".", "");
    fall ("M-24", "anfrage_gen_genau_eine_antwort name (pid, instanz16, laufzeit32)",
          teile[2] == pid && teile[3] == instanz16 && teile[4] == laufzeit, passend[0].getFileName());
    fall ("M-24", "anfrage_gen_genau_eine_antwort kopf",
          ok && u["format"].toString() == "nakama.diagnose.antwort.v1" && u["anfrage_id"].toString() == juce::String (id)
              && u["rolle"].toString() == "gen" && u["instanz_id"].toString() == instanz
              && u["laufzeit_id"].toString() == laufzeit && u["pid"].toString() == pid
              && u["erzeugt_utc"].toString() == "2026-09-14T12:00:00.000Z",
          "rolle " + u["rolle"].toString() + ", laufzeit_id " + u["laufzeit_id"].toString()
              + ", erzeugt_utc " + u["erzeugt_utc"].toString());
    fall ("M-24", "anfrage_gen_genau_eine_antwort snapshot_v3_frame_und_aggregat_null_mit_grund",
          u["snapshot"].getDynamicObject() != nullptr && (int) u["snapshot"]["snapshot_version"] == 3
              && u["frame"].getDynamicObject() != nullptr && u["aggregat"].isVoid()
              && hatGrund (u, "aggregat", "rolle_ohne_aggregat"));
    antwortM24 = u;
}

//==============================================================================
// M-25 gleiche_kennung_keine_zweite_antwort
void gleicheKennungKeineZweiteAntwort()
{
    TempWurzel t;
    auto g = std::make_unique<Gen>();
    const auto f = fassaden (t);
    starteMit (*g, f);
    auto& bk = g->briefkastenFuerTest();
    const auto id = neueKennung();
    schreibeAnfrage (t, id);
    bk.takt();
    const bool erste = antworten (t, "gen", id).size() == 1;
    fall ("M-25", "gleiche_kennung_keine_zweite_antwort vorbedingung (erste Antwort auf K)", erste);

    const auto s0 = f.fs->stand();
    for (int i = 0; i < 150; ++i)
        bk.takt();
    const auto s1 = f.fs->stand();
    const bool neu = beruehre (t.anfrage()) && schreibeAnfrage (t, id);   // gleiche Kennung, neue Aenderungszeit
    for (int i = 0; i < 150; ++i)
        bk.takt();
    const auto s2 = f.fs->stand();
    const auto nachher = differenz (s2, s0);
    const auto z = bk.zaehler();
    fall ("M-25", "gleiche_kennung_keine_zweite_antwort 0_anlegeversuche_0_schreibvorgaenge (300 Takte)",
          neu && nachher.anlegeversuche == 0 && nachher.schreibvorgaenge == 0
              && antworten (t, "gen").size() == 1 && z.fehlerzaehler == 0,
          stand (nachher) + ", Fehler " + zahl (z.fehlerzaehler));
    fall ("M-25", "gleiche_kennung_keine_zweite_antwort nach_dem_neuschreiben_genau_eine_leseoeffnung",
          differenz (s1, s0).leseoeffnungen == 0 && differenz (s2, s1).leseoeffnungen == 1,
          "vorher " + zahl (differenz (s1, s0).leseoeffnungen) + ", nachher " + zahl (differenz (s2, s1).leseoeffnungen));
}

//==============================================================================
// M-26 neue_kennung_neue_antwort
void neueKennungNeueAntwort()
{
    TempWurzel t;
    auto g = std::make_unique<Gen>();
    const auto f = fassaden (t);
    starteMit (*g, f);
    auto& bk = g->briefkastenFuerTest();
    const auto k = neueKennung();
    schreibeAnfrage (t, k);
    bk.takt();
    const auto kDateien = antworten (t, "gen", k);
    const auto shaVorher = kDateien.size() == 1 ? sha256Datei (kDateien[0]) : juce::String();
    fall ("M-26", "neue_kennung_neue_antwort vorbedingung (Antwort auf K)", kDateien.size() == 1);

    const auto l = neueKennung();
    schreibeAnfrage (t, l);
    bk.takt();
    const auto lDateien = antworten (t, "gen", l);
    fall ("M-26", "neue_kennung_neue_antwort genau_eine_datei_fuer_L",
          lDateien.size() == 1 && dateienIn (t.antwort()).size() == 2,
          "L " + juce::String ((int) lDateien.size()) + ", alle " + juce::String ((int) dateienIn (t.antwort()).size()));
    fall ("M-26", "neue_kennung_neue_antwort datei_fuer_K_byteweise_erhalten",
          kDateien.size() == 1 && sha256Datei (kDateien[0]) == shaVorher, shaVorher);
    fall ("M-26", "neue_kennung_neue_antwort anfrage_nie_geloescht",
          f.fs->stand().loeschungen == 0 && t.anfrage().existsAsFile());
}

//==============================================================================
// M-27 fremde_anfragen_werden_ignoriert (acht Unterfaelle)
void fremdeAnfragenWerdenIgnoriert()
{
    TempWurzel t;
    auto g = std::make_unique<Gen>();
    const auto f = fassaden (t);
    starteMit (*g, f);
    auto& bk = g->briefkastenFuerTest();
    LoggerAufsicht log;
    const auto k = neueKennung();

    struct Variante { const char* name; std::string bytes; bool verzeichnis; int lesenSoll; };
    std::string gross = anfrageText (k);
    gross.append (1025 - gross.size(), ' ');
    const auto mitId = [] (const std::string& id) { return anfrageText (id); };
    const std::vector<Variante> varianten {
        { "(a) 1025 Bytes", gross, false, 0 },
        { "(b) kein JSON", "das ist kein JSON", false, 1 },
        { "(c) zusaetzlicher Schluessel",
          "{\"format\":\"nakama.diagnose.anfrage.v1\",\"anfrage_id\":\"" + k + "\",\"x\":1}", false, 1 },
        { "(d) format falsch", "{\"format\":\"nakama.diagnose.anfrage.v2\",\"anfrage_id\":\"" + k + "\"}", false, 1 },
        { "(e) Grossbuchstaben", mitId ("ABCDEF" + k.substr (6)), false, 1 },
        { "(e) 31 Zeichen", mitId (k.substr (0, 31)), false, 1 },
        { "(e) 33 Zeichen", mitId (k + "a"), false, 1 },
        { "(e) mit ..", mitId (".." + k.substr (2)), false, 1 },
        { "(e) mit Backslash", mitId ("\\\\" + k.substr (1)), false, 1 },
        { "(e) mit /", mitId ("/" + k.substr (1)), false, 1 },
        { "(e) mit NUL", mitId ("\\u0000" + k.substr (1)), false, 1 },
        { "(f) Verzeichnis", {}, true, 0 },
        { "(g) leere Datei", std::string(), false, 1 },
        { "(h) UTF-8 mit BOM", "\xEF\xBB\xBF" + anfrageText (k), false, 1 },
    };

    for (const auto& v : varianten)
    {
        t.anfrage().deleteRecursively();
        bk.takt();   // ohne Datei: Merker und offene Kennung fallen
        const bool angelegt = v.verzeichnis ? t.anfrage().createDirectory().wasOk()
                                            : schreibeRoh (t.anfrage(), v.bytes);
        const auto s0 = f.fs->stand();
        for (int i = 0; i < 5; ++i)
            bk.takt();
        const auto d1 = differenz (f.fs->stand(), s0);
        // Erst eine geaenderte Datei wird wieder gelesen (T-5).
        if (! v.verzeichnis)
            beruehre (t.anfrage());
        const auto s1 = f.fs->stand();
        for (int i = 0; i < 5; ++i)
            bk.takt();
        const auto d2 = differenz (f.fs->stand(), s1);
        const auto z = bk.zaehler();
        const bool gelesenWieSoll = d1.leseoeffnungen == (std::uint64_t) v.lesenSoll
                                 && d2.leseoeffnungen == (std::uint64_t) (v.verzeichnis ? 0 : v.lesenSoll);
        const bool bytesWieSoll = v.lesenSoll != 0 || (d1.geleseneBytes == 0 && d2.geleseneBytes == 0);
        fall ("M-27", (std::string ("fremde_anfragen_werden_ignoriert ") + v.name).c_str(),
              angelegt && gelesenWieSoll && bytesWieSoll && dateienIn (t.antwort()).empty()
                  && z.antworten == 0 && z.schreibversuche == 0 && d1.anlegeversuche == 0 && d2.anlegeversuche == 0,
              "Lese " + zahl (d1.leseoeffnungen) + "+" + zahl (d2.leseoeffnungen) + ", Bytes "
                  + zahl (d1.geleseneBytes + d2.geleseneBytes) + ", Dateien "
                  + juce::String ((int) dateienIn (t.antwort()).size()) + ", Antworten " + zahl (z.antworten));
    }
    fall ("M-27", "fremde_anfragen_werden_ignoriert kein_logeintrag", log.logger.eintraege.load() == 0,
          juce::String (log.logger.eintraege.load()));
}

//==============================================================================
// M-28 gesperrte_anfrage_im_naechsten_takt
void gesperrteAnfrageImNaechstenTakt()
{
    TempWurzel t;
    auto g = std::make_unique<Gen>();
    const auto f = fassaden (t);
    starteMit (*g, f);
    auto& bk = g->briefkastenFuerTest();
    const auto id = neueKennung();
    schreibeAnfrage (t, id);
    f.fs->injLesen.store (1);   // "Zugriff verweigert" an der Leseoeffnung (F-14)
    const auto s0 = f.fs->stand();
    bk.takt();
    const auto s1 = f.fs->stand();
    const bool ohneAntwort = dateienIn (t.antwort()).empty();
    bk.takt();
    const auto s2 = f.fs->stand();
    fall ("M-28", "gesperrte_anfrage_im_naechsten_takt gesperrt_keine_antwort",
          differenz (s1, s0).leseoeffnungen == 1 && ohneAntwort, stand (differenz (s1, s0)));
    fall ("M-28", "gesperrte_anfrage_im_naechsten_takt naechster_takt_liest_und_antwortet",
          differenz (s2, s1).leseoeffnungen == 1 && antworten (t, "gen", id).size() == 1
              && bk.zaehler().fehlerzaehler == 0,
          stand (differenz (s2, s1)));
}

//==============================================================================
// M-29 antwortordner_fehlt_drei_versuche
void antwortordnerFehltDreiVersuche()
{
    const auto versuche = [] (dg::Briefkasten& bk, int takte)
    {
        juce::String folge;
        for (int i = 0; i < takte; ++i)
        {
            bk.takt();
            folge << (folge.isEmpty() ? "" : ",") << zahl (bk.zaehler().schreibversuche);
        }
        return folge;
    };
    {
        TempWurzel t (true, false);   // (a) antwort\ fehlt
        auto g = std::make_unique<Gen>();
        const auto f = fassaden (t);
        starteMit (*g, f);
        schreibeAnfrage (t, neueKennung());
        const auto folge = versuche (g->briefkastenFuerTest(), 6);
        const auto z = g->briefkastenFuerTest().zaehler();
        const auto s = f.fs->stand();
        fall ("M-29", "antwortordner_fehlt_drei_versuche (a) antwort fehlt",
              folge == "1,2,3,3,3,3" && z.fehlerzaehler == 1 && s.ordneranlagen == 0 && s.anlegeversuche == 0
                  && ! t.antwort().exists() && z.ausnahmen == 0,
              "Versuche je Takt " + folge + ", Fehler " + zahl (z.fehlerzaehler) + ", " + stand (s));
    }
    {
        TempWurzel t;   // (b) exklusives Anlegen scheitert in jedem Takt
        auto g = std::make_unique<Gen>();
        const auto f = fassaden (t);
        starteMit (*g, f);
        f.fs->injAnlegen.store (-1);
        schreibeAnfrage (t, neueKennung());
        const auto folge = versuche (g->briefkastenFuerTest(), 6);
        const auto z = g->briefkastenFuerTest().zaehler();
        const auto s = f.fs->stand();
        fall ("M-29", "antwortordner_fehlt_drei_versuche (b) anlegen scheitert immer",
              folge == "1,2,3,3,3,3" && z.fehlerzaehler == 1 && s.anlegeversuche == 3 && s.ordneranlagen == 0
                  && dateienIn (t.antwort()).empty(),
              "Versuche je Takt " + folge + ", Fehler " + zahl (z.fehlerzaehler) + ", " + stand (s));
    }
    {
        TempWurzel t (true, false);   // (c) antwort\ erscheint vor dem dritten Versuch
        auto g = std::make_unique<Gen>();
        const auto f = fassaden (t);
        starteMit (*g, f);
        const auto id = neueKennung();
        schreibeAnfrage (t, id);
        auto folge = versuche (g->briefkastenFuerTest(), 2);
        t.antwort().createDirectory();   // die Buehne, nie die Instanz
        folge << "," << versuche (g->briefkastenFuerTest(), 2);
        const auto z = g->briefkastenFuerTest().zaehler();
        fall ("M-29", "antwortordner_fehlt_drei_versuche (c) ordner erscheint vor dem dritten versuch",
              folge == "1,2,3,3" && z.antworten == 1 && z.fehlerzaehler == 0 && antworten (t, "gen", id).size() == 1
                  && dateienIn (t.antwort()).size() == 1 && f.fs->stand().ordneranlagen == 0,
              "Versuche je Takt " + folge + ", Antworten " + zahl (z.antworten));
    }
}

//==============================================================================
// M-31 ohne_messdaten_antwort_mit_grund (beide Rollen)
void ohneMessdatenAntwortMitGrund()
{
    TempWurzel t;
    auto gen = std::make_unique<Gen>();
    ProbeeqLauf q;
    q.bereite (48000.0, 512);   // Transport gestoppt: kein processBlock seit dem Start
    const auto fg = fassaden (t);
    const auto fp = fassaden (t);
    const bool start = starteMit (*gen, fg) == dg::Startgrund::gestartet
                    && starteMit (*q.p, fp) == dg::Startgrund::gestartet;
    fall ("M-31", "ohne_messdaten_antwort_mit_grund vorbedingung (keineDaten, 0 Rahmen, beide gestartet)",
          start && gen->messSnapshot().zustand == eqcop::MessZustand::keineDaten && gen->merkmaleFrames() == 0
              && q.p->analyseSnapshotFuerTest().zustand == eqcop::MessZustand::keineDaten
              && q.p->framesGebautFuerTest() == 0);

    const auto id = neueKennung();
    schreibeAnfrage (t, id);
    const auto pruefeUmschlag = [&] (const char* rolle, const std::vector<juce::File>& dateien, juce::var& ablage,
                                     const juce::String& zusatz, bool zusatzOk)
    {
        bool ok = false;
        const auto u = dateien.size() == 1 ? liesJson (dateien[0], ok) : juce::var();
        fall ("M-31", (std::string ("ohne_messdaten_antwort_mit_grund ") + rolle).c_str(),
              dateien.size() == 1 && ok && u["snapshot"].isVoid() && u["frame"].isVoid()
                  && hatGrund (u, "snapshot", "noch_keine_messdaten") && hatGrund (u, "frame", "noch_keine_messdaten")
                  && hatGrund (u, "aggregat", "rolle_ohne_aggregat") && zusatzOk,
              "Dateien " + juce::String ((int) dateien.size()) + ", " + zusatz);
        ablage = u;
    };

    gen->briefkastenFuerTest().takt();
    const auto zg = gen->briefkastenFuerTest().zaehler();
    pruefeUmschlag ("gen (im ersten Takt)", antworten (t, "gen", id), antwortM31Gen,
                    "Takte " + zahl (zg.takte) + ", wartend " + zahl (zg.warteTakte),
                    zg.takte == 1 && zg.warteTakte == 0);

    auto& bk = q.p->briefkastenFuerTest();
    const auto s0 = fp.fs->stand();
    bk.takt();
    const bool ersterWartet = differenz (fp.fs->stand(), s0).anlegeversuche == 0 && bk.zaehler().warteTakte == 1
                           && antworten (t, "probeeq", id).empty();
    const bool durch = q.durchlauf();
    bk.takt();
    const auto zq = bk.zaehler();
    pruefeUmschlag ("probeeq (im Takt nach genau einem wartenden)", antworten (t, "probeeq", id), antwortM31Probeeq,
                    "erster wartet " + juce::String (ersterWartet ? "ja" : "nein") + ", wartend "
                        + zahl (zq.warteTakte) + ", Fehler " + zahl (zq.fehlerzaehler),
                    ersterWartet && durch && zq.warteTakte == 1 && zq.fehlerzaehler == 0);
}

//==============================================================================
// M-48 pfadtraversal_unmoeglich (C++-Haelfte)
void pfadtraversalUnmoeglich()
{
    TempWurzel t;
    const auto waechterNeben = t.diagnose().getChildFile ("waechter");    // ..\waechter von antwort\ aus
    const auto waechterOben  = t.evenacadia().getChildFile ("waechter");  // ..\..\..\waechter
    waechterNeben.createDirectory();
    waechterOben.createDirectory();
    auto g = std::make_unique<Gen>();
    const auto f = fassaden (t);
    starteMit (*g, f);
    auto& bk = g->briefkastenFuerTest();

    std::string arabisch16;
    for (int i = 0; i < 16; ++i)
        arabisch16 += "\\u0660";
    struct Fall { const char* name; std::string json; };   // der Wert als JSON-Text
    const std::vector<Fall> faelle {
        { "..\\..\\x", "..\\\\..\\\\x" },
        { "C:\\x", "C:\\\\x" },
        { "/x", "/x" },
        { "..\\waechter\\ (32 Zeichen)", "..\\\\waechter\\\\0123456789abcdef0123" },
        { "..\\..\\..\\waechter\\ (32 Zeichen)", "..\\\\..\\\\..\\\\waechter\\\\0123456789abcd" },
        { "32 Zeichen mit :", "c:0123456789abcdef0123456789abcd" },
        { "32 Zeichen mit NUL", "\\u0000123456789abcdef0123456789abcdef" },
        { "Unicode-Ziffern (16 Zeichen, 32 Bytes)", arabisch16 },
        { "Unicode-Ziffern (32 Zeichen)", arabisch16 + arabisch16 },
    };
    for (const auto& c : faelle)
    {
        const auto s0 = f.fs->stand();
        schreibeRoh (t.anfrage(), "{\"format\":\"nakama.diagnose.anfrage.v1\",\"anfrage_id\":\"" + c.json + "\"}");
        for (int i = 0; i < 5; ++i)
            bk.takt();
        const auto d = differenz (f.fs->stand(), s0);
        const bool waechterLeer = waechterNeben.findChildFiles (juce::File::findFilesAndDirectories, true).isEmpty()
                               && waechterOben.findChildFiles (juce::File::findFilesAndDirectories, true).isEmpty();
        const bool sonstNichts = dateienIn (t.antwort()).empty() && dateienIn (t.nakama()).empty()
                              && dateienIn (t.evenacadia()).empty() && dateienIn (t.wurzel).empty()
                              && dateienIn (t.diagnose()).size() == 1;
        fall ("M-48", (std::string ("pfadtraversal_unmoeglich ") + c.name).c_str(),
              waechterLeer && sonstNichts && d.anlegeversuche == 0 && d.umbenennungen == 0
                  && bk.zaehler().antworten == 0 && f.fs->ausserhalbWurzel.load() == 0,
              juce::String ("Waechter leer ") + (waechterLeer ? "ja" : "nein") + ", sonst nichts "
                  + (sonstNichts ? "ja" : "nein") + ", " + stand (d));
    }
}

//==============================================================================
// M-51 kennungsring_fifo_256 (C++-Haelfte)
void kennungsringFifo256()
{
    TempWurzel t;
    auto g = std::make_unique<Gen>();
    const auto f = fassaden (t);
    starteMit (*g, f);
    auto& bk = g->briefkastenFuerTest();

    std::vector<std::string> ids;
    juce::String erstesErzeugt;
    bool alleBeantwortet = true;
    for (int i = 0; i < 257; ++i)
    {
        ids.push_back (neueKennung());
        schreibeAnfrage (t, ids.back());
        bk.takt();
        const auto a = antworten (t, "gen", ids.back());
        alleBeantwortet = alleBeantwortet && a.size() == 1;
        if (i == 0 && a.size() == 1)
        {
            bool ok = false;
            erstesErzeugt = liesJson (a[0], ok)["erzeugt_utc"].toString();
        }
        f.uhr->ms += 1000;
    }
    fall ("M-51", "kennungsring_fifo_256 vorbedingung (257 Kennungen je genau einmal beantwortet)",
          alleBeantwortet && bk.zaehler().antworten == 257, zahl (bk.zaehler().antworten));

    // Nach dem Anlauf: Takte ohne Schreiben legen keinen Speicher an.
    allokationen.store (0);
    zaehleAllokationen = true;
    for (int i = 0; i < 100; ++i)
        bk.takt();
    zaehleAllokationen = false;
    fall ("M-51", "kennungsring_fifo_256 keine_allokation_je_takt_nach_dem_anlauf (100 Takte)",
          allokationen.load() == 0, zahl (allokationen.load()));

    // Die Buehne raeumt vor jeder Wiederholung ab: zuerst die Anfrage, dann die
    // Antworten samt .tmp- (P-1, T-2) - mit eigenen Aufrufen.
    const auto raeumeAb = [&]
    {
        t.anfrage().deleteFile();
        for (const auto& d : dateienIn (t.antwort()))
            d.deleteFile();
        bk.takt();
    };

    raeumeAb();
    f.uhr->ms += 60000;
    schreibeAnfrage (t, ids[255]);   // die zweitjuengste
    auto s0 = f.fs->stand();
    for (int i = 0; i < 3; ++i)
        bk.takt();
    auto d = differenz (f.fs->stand(), s0);
    fall ("M-51", "kennungsring_fifo_256 zweitjuengste_erneut_0_anlegeversuche",
          d.anlegeversuche == 0 && d.schreibvorgaenge == 0 && dateienIn (t.antwort()).empty(), stand (d));

    raeumeAb();
    f.uhr->ms += 60000;
    schreibeAnfrage (t, ids[0]);   // die aelteste, verdraengt
    s0 = f.fs->stand();
    bk.takt();
    d = differenz (f.fs->stand(), s0);
    const auto a = antworten (t, "gen", ids[0]);
    bool ok = false;
    const auto neuesErzeugt = a.size() == 1 ? liesJson (a[0], ok)["erzeugt_utc"].toString() : juce::String();
    fall ("M-51", "kennungsring_fifo_256 aelteste_verdraengt_genau_ein_anlegen_ein_schreiben_neues_erzeugt_utc",
          d.anlegeversuche == 1 && d.schreibvorgaenge == 1 && a.size() == 1 && ok && neuesErzeugt.isNotEmpty()
              && neuesErzeugt != erstesErzeugt && bk.zaehler().fehlerzaehler == 0,
          stand (d) + ", erzeugt_utc " + erstesErzeugt + " -> " + neuesErzeugt);
    fall ("M-51", "kennungsring_fifo_256 loeschzaehler_der_instanz_0", f.fs->stand().loeschungen == 0,
          zahl (f.fs->stand().loeschungen));
}

//==============================================================================
// M-52 uhrsprung_aendert_zuordnung_nicht (C++-Haelfte)
void uhrsprungAendertZuordnungNicht()
{
    TempWurzel t;
    auto g = std::make_unique<Gen>();
    const auto f = fassaden (t);
    starteMit (*g, f);
    auto& bk = g->briefkastenFuerTest();
    const auto utcVon = [&] (const std::string& id)
    {
        const auto a = antworten (t, "gen", id);
        bool ok = false;
        return a.size() == 1 ? liesJson (a[0], ok)["erzeugt_utc"].toString() : juce::String ("(keine Antwort)");
    };
    const auto k = neueKennung();
    schreibeAnfrage (t, k);
    bk.takt();
    const auto utcK = utcVon (k);
    f.uhr->ms -= 2 * 3600 * 1000;   // die Uhr springt 2 h zurueck
    const auto l = neueKennung();
    schreibeAnfrage (t, l);
    bk.takt();
    const auto utcL = utcVon (l);
    fall ("M-52", "uhrsprung_aendert_zuordnung_nicht neue_antwort_nach_ruecksprung_utc_aus_der_uhr",
          utcK == "2026-09-14T12:00:00.000Z" && utcL == "2026-09-14T10:00:00.000Z" && bk.zaehler().antworten == 2,
          "K " + utcK + ", L " + utcL);
}

//==============================================================================
// M-53 keine_leere_antwort (Lagen (a) und (b), C++-Haelfte)
void keineLeereAntwort()
{
    const auto pid = juce::String ((juce::int64) GetCurrentProcessId());
    const auto lage = [&] (const char* name, bool vorDemErstenByte)
    {
        TempWurzel t;
        auto g = std::make_unique<Gen>();
        const auto f = fassaden (t);
        starteMit (*g, f);
        (vorDemErstenByte ? f.fs->injSchreiben : f.fs->injUmbenennen).store (1);
        schreibeAnfrage (t, neueKennung());
        for (int i = 0; i < 5; ++i)
            g->briefkastenFuerTest().takt();
        const auto dateien = dateienIn (t.antwort());
        int temp = 0, json = 0;
        juce::int64 tempBytes = -1;
        for (const auto& d : dateien)
        {
            const auto n = d.getFileName();
            if (n.endsWith (".tmp-" + pid) && istAntwortName (n.upToLastOccurrenceOf (".tmp-", false, false), "gen"))
            {
                ++temp;
                tempBytes = d.getSize();
            }
            if (n.endsWithIgnoreCase (".json"))
                ++json;
        }
        const auto z = g->briefkastenFuerTest().zaehler();
        const bool ok = temp == 1 && json == 0 && (int) dateien.size() == 1 && f.fs->stand().loeschungen == 0
                     && z.fehlerzaehler == 1 && (vorDemErstenByte ? tempBytes == 0 : tempBytes > 0);
        fall ("M-53", (std::string ("keine_leere_antwort ") + name).c_str(), ok,
              "Temp " + juce::String (temp) + " (" + juce::String (tempBytes) + " Bytes), json "
                  + juce::String (json) + ", Dateien " + juce::String ((int) dateien.size()) + ", Loesch "
                  + zahl (f.fs->stand().loeschungen) + ", Fehler " + zahl (z.fehlerzaehler));
    };
    lage ("(a) fehler vor dem ersten byte: keine datei mit antwortnamen", true);
    lage ("(b) fehler beim umbenennen: genau eine temp-datei, loeschzaehler 0", false);
}

//==============================================================================
// M-77 startfehler_wurzel_ohne_takt (beide Durchlaeufe, beide Rollen)
template <typename Prozessor>
void startfehlerRolle (const char* durchlauf, const char* rolle, Prozessor& p, const FassadenSatz& f,
                       bool reparse, const juce::File& antwortOrdner, ZaehlLogger& logger)
{
    auto& bk = p.briefkastenFuerTest();
    const auto zustandVorher = zustandBytes (p);
    const auto parameterVorher = parameterWerte (p);
    DirtyZeuge dz;
    p.addListener (&dz);
    const auto logVorher = logger.eintraege.load();
    const auto name = [&] (const char* teil) { return std::string ("startfehler_wurzel_ohne_takt ") + durchlauf + " " + rolle + " " + teil; };

    bk.setzeFassaden (f.wurzel, f.fs, f.uhr);
    const auto grund = p.briefkastenStartenFuerTest (true);   // mit Timer: ein Startfehler startet keinen
    // Startfenster: die Staende nach der Startfunktion (F-1, F-14).
    const auto sStart = f.fs->stand();
    const auto wStart = f.wurzel->wurzelabfragen();
    const bool startfenster = wStart == 1
        && (reparse ? sStart.existenzpruefungen == 1 && sStart.aufrufe() == 1 : sStart.aufrufe() == 0)
        && grund == (reparse ? dg::Startgrund::reparsePunkt : dg::Startgrund::keineWurzel)
        && bk.startgrund() == grund;
    fall ("M-77", name ("startfenster").c_str(), startfenster,
          "Wurzelabfragen " + zahl (wStart) + ", " + stand (sStart) + ", Grund " + juce::String ((int) bk.startgrund()));

    for (int i = 0; i < 5; ++i)
        bk.takt();
    const auto d = differenz (f.fs->stand(), sStart);
    const auto z = bk.zaehler();
    const bool taktfenster = d.aufrufe() == 0 && f.wurzel->wurzelabfragen() == wStart && z.starts == 0
                          && z.takte == 0 && bk.taktIntervallMs() == 0
                          && antwortOrdner.findChildFiles (juce::File::findFiles, false).isEmpty();
    fall ("M-77", name ("taktfenster (5 Taktausloesungen: 0 Aufrufe, kein Takt, keine Antwort)").c_str(), taktfenster,
          stand (d) + ", Starts " + zahl (z.starts) + ", Takte " + zahl (z.takte) + ", Intervall "
              + juce::String (bk.taktIntervallMs()));

    const bool neutral = zustandBytes (p) == zustandVorher && parameterWerte (p) == parameterVorher
                      && dz.geaendert.load() == 0 && dz.parameter.load() == 0 && logger.eintraege.load() == logVorher;
    fall ("M-77", name ("state_parameter_dirty_und_log_unveraendert").c_str(), neutral,
          "Dirty " + juce::String (dz.geaendert.load()) + ", Parameterereignisse " + juce::String (dz.parameter.load())
              + ", Log " + juce::String (logger.eintraege.load() - logVorher));
    p.removeListener (&dz);
}

void startfehlerWurzelOhneTakt()
{
    LoggerAufsicht log;
    {
        TempWurzel t;
        schreibeAnfrage (t, neueKennung());   // eine gueltige Anfrage liegt
        auto gen = std::make_unique<Gen>();
        auto pq = std::make_unique<Probeeq>();
        startfehlerRolle ("durchlauf 1 (keine wurzel)", "gen", *gen, fassaden (t, false), false, t.antwort(), log.logger);
        startfehlerRolle ("durchlauf 1 (keine wurzel)", "probeeq", *pq, fassaden (t, false), false, t.antwort(), log.logger);
    }
    {
        TempWurzel t;
        schreibeAnfrage (t, neueKennung());
        // evenacadia\ ist beim Start eine Junction auf den Baum mit der Anfrage.
        const auto ziel = t.wurzel.getChildFile ("evenacadia-ziel");
        const bool link = t.evenacadia().moveFileTo (ziel) && legeJunctionAn (t, t.evenacadia(), ziel);
        fall ("M-77", "startfehler_wurzel_ohne_takt durchlauf 2 vorbedingung (evenacadia ist eine Junction, Anfrage liegt)",
              link && t.anfrage().existsAsFile());
        const auto antwortOrdner = ziel.getChildFile ("nakama").getChildFile ("diagnose").getChildFile ("antwort");
        auto gen = std::make_unique<Gen>();
        auto pq = std::make_unique<Probeeq>();
        startfehlerRolle ("durchlauf 2 (evenacadia reparse)", "gen", *gen, fassaden (t), true, antwortOrdner, log.logger);
        startfehlerRolle ("durchlauf 2 (evenacadia reparse)", "probeeq", *pq, fassaden (t), true, antwortOrdner, log.logger);
    }
}

//==============================================================================
// M-38 testbau_startet_keinen_briefkasten
void testbauStartetKeinenBriefkasten()
{
    auto gen = std::make_unique<Gen>();
    auto pq = std::make_unique<Probeeq>();
    // Die Fassaden des Produkts: im Testbau gibt es keinen Start nach F-1.
    const auto still = [] (dg::Briefkasten& bk)
    {
        const auto z = bk.zaehler();
        return z.starts == 0 && z.takteBegonnen == 0 && bk.wurzelabfragen() == 0
            && bk.dateisystemStand().aufrufe() == 0 && bk.taktIntervallMs() == 0
            && bk.startgrund() == dg::Startgrund::nichtGestartet;
    };
    const auto detail = [] (const char* rolle, dg::Briefkasten& bk)
    {
        return juce::String (rolle) + ": Starts " + zahl (bk.zaehler().starts) + ", Wurzelabfragen "
             + zahl (bk.wurzelabfragen()) + ", " + stand (bk.dateisystemStand());
    };
    auto& bg = gen->briefkastenFuerTest();
    auto& bp = pq->briefkastenFuerTest();
    fall ("M-38", "testbau_startet_keinen_briefkasten startfenster (nach dem Konstruktor)", still (bg) && still (bp),
          detail ("gen", bg) + "; " + detail ("probeeq", bp));
    pumpe (5000);
    fall ("M-38", "testbau_startet_keinen_briefkasten taktfenster (5 s Nachrichtenschleife)", still (bg) && still (bp),
          detail ("gen", bg) + "; " + detail ("probeeq", bp));
}

//==============================================================================
// M-79 summenzaehler_ueber_abschluesse_und_grenze
void summenzaehlerUeberAbschluesseUndGrenze()
{
    namespace ez = nakama::echtzeit;
    auto e = std::make_unique<nakama::analyse::FeatureEngine>();   // ~0,5 MB: Heap (NAK-175)
    constexpr double rate = 48000.0;
    constexpr int n = 4800;   // ein 100-ms-Rahmen je Block
    e->vorbereiten (rate);
    std::vector<float> audio ((std::size_t) (2 * n));
    referenz::Rauschen r;
    ez::StampedBlock b;
    b.startFolge = 1;
    b.kanaele = 2;
    b.tapMaske = 1;
    b.sampleCount = (std::uint32_t) n;
    b.sampleRate = rate;
    b.flags = ez::kFlagKontextAnwesend | ez::kFlagZeitGueltig | ez::kFlagSpieltGueltig | ez::kFlagSpielt
            | ez::kFlagSampleRateGueltig;

    std::int64_t zeit = 96000;
    std::uint64_t strom = 0;
    int vorGrenze = 0, nachGrenze = 0, grenzen = 0;
    bool faellt = false, fremdesWachstum = false, deltaFalsch = false, grenzeBewegt = false, aktivUeber = false;
    std::uint64_t fensterJeBlock = 0;
    juce::String protokoll;
    for (int i = 0; i < 48; ++i)
    {
        if (i == 24)
            zeit += 48000;   // Zeitsprung: eine Grenze mitten im Plan
        const bool leise = (i % 6) == 5;   // Fenster ohne Aktivitaet
        for (int k = 0; k < n; ++k)
        {
            const double t = (double) (zeit + k) / rate;
            const float l = leise ? 1.0e-6f * r.naechster()
                                  : 0.05f * r.naechster() + 0.2f * (float) std::sin (2.0 * referenz::kPi * 250.0 * t);
            audio[(std::size_t) (2 * k)] = l;
            audio[(std::size_t) (2 * k + 1)] = 0.9f * l;
        }
        b.projectSampleStart = zeit;
        b.stromVon = strom;
        b.ringVon = strom;
        const auto g0 = e->summeFensterGesamt();
        const auto a0 = e->summeFensterAktiv();
        const auto offen0 = e->evidenzFensterGesamtJetzt();
        const auto grenzen0 = e->getrennteFenster();
        const bool rahmen = e->nimmBlock (b, audio.data());
        const auto g1 = e->summeFensterGesamt();
        const auto a1 = e->summeFensterAktiv();
        const bool grenze = e->getrennteFenster() != grenzen0;
        if (grenze)
        {
            ++grenzen;
            grenzeBewegt = grenzeBewegt || g1 != g0 || a1 != a0;
        }
        if (g1 < g0 || a1 < a0)
        {
            faellt = true;
        }
        else
        {
            const auto dG = g1 - g0, dA = a1 - a0;
            aktivUeber = aktivUeber || dA > dG;
            if (rahmen && e->frame().evidenzFrisch)
            {
                ++(grenzen == 0 ? vorGrenze : nachGrenze);
                const auto& f = e->frame();
                // Dieselben Zaehler, aus denen `abdeckung` entsteht (Frame.h).
                const bool gleich = f.abdeckungGesetzt && dG > 0
                                 && (float) ((double) dA / (double) dG) == f.abdeckung
                                 && dG >= offen0 && dG - offen0 <= fensterJeBlock + 1;
                deltaFalsch = deltaFalsch || ! gleich;
                protokoll << (protokoll.isEmpty() ? "" : " ") << "+" << zahl (dG) << "/" << zahl (dA);
            }
            else
            {
                fremdesWachstum = fremdesWachstum || dG != 0 || dA != 0;
                if (! grenze)
                    fensterJeBlock = std::max (fensterJeBlock, e->evidenzFensterGesamtJetzt() - offen0);
            }
        }
        zeit += n;
        strom += n;
    }
    const auto detail = "Abschluesse " + juce::String (vorGrenze) + " vor, " + juce::String (nachGrenze)
                      + " nach der Grenze; Grenzen " + juce::String (grenzen) + "; Zuwachs gesamt/aktiv je Abschluss "
                      + protokoll + "; Endstand " + zahl (e->summeFensterGesamt()) + "/" + zahl (e->summeFensterAktiv());
    fall ("M-79", "summenzaehler_ueber_abschluesse_und_grenze vorbedingung (je zwei Abschluesse vor und nach einer Grenze)",
          vorGrenze >= 2 && nachGrenze >= 2 && grenzen == 1, detail);
    fall ("M-79", "summenzaehler_ueber_abschluesse_und_grenze fallen_nie", ! faellt, detail);
    fall ("M-79", "summenzaehler_ueber_abschluesse_und_grenze je_evidenzrahmen_genau_dessen_fensterzaehler",
          ! deltaFalsch && ! fremdesWachstum, detail);
    fall ("M-79", "summenzaehler_ueber_abschluesse_und_grenze an_der_grenze_stehen_sie", grenzen == 1 && ! grenzeBewegt, detail);
    fall ("M-79", "summenzaehler_ueber_abschluesse_und_grenze aktiv_hoechstens_gesamt", ! aktivUeber, detail);
}

//==============================================================================
// M-32 probeeq_antwort_ohne_v3_verbindung
void probeeqAntwortOhneV3Verbindung()
{
    TempWurzel t;
    ProbeeqLauf q;
    q.bereite (48000.0, 512);
    const auto f = fassaden (t);
    const bool start = starteMit (*q.p, f) == dg::Startgrund::gestartet;
    std::int64_t zeit = 0;
    const bool geleert = q.fuettere (282, zeit);   // 3 s bei 48 kHz
    nakama::analyse::FeatureFrame produzent {};
    const bool ohneV3 = ! q.p->letzterProducerFrameFuerTest (produzent);
    const auto gebaut = q.p->framesGebautFuerTest();
    fall ("M-32", "probeeq_antwort_ohne_v3_verbindung vorbedingung (3 s verarbeitet, Queue leer, kein Producer-Frame)",
          start && geleert && ohneV3 && gebaut > 0, "frames_gebaut " + zahl (gebaut));

    const auto id = neueKennung();
    schreibeAnfrage (t, id);
    const bool paar = q.taktPaar();
    const auto a = antworten (t, "probeeq", id);
    bool ok = false;
    const auto u = a.size() == 1 ? liesJson (a[0], ok) : juce::var();
    const auto zustand = q.p->zustandLesen();
    const auto instanz = zustand.common.instanceId;
    const auto instanz16 = juce::SHA256 (instanz.toRawUTF8(), instanz.getNumBytesAsUTF8()).toHexString().substring (0, 16);
    const auto laufzeit = juce::String (q.p->v3HelloFuerTest().adresse.runtimeNonce);
    const auto teile = a.size() == 1 ? juce::StringArray::fromTokens (a[0].getFileName(), ".", "") : juce::StringArray();
    fall ("M-32", "probeeq_antwort_ohne_v3_verbindung datei_und_kopf (rolle probeeq, laufzeit_id = v3RuntimeNonce)",
          paar && a.size() == 1 && ok && teile[3] == instanz16 && teile[4] == laufzeit
              && u["rolle"].toString() == "probeeq" && u["laufzeit_id"].toString() == laufzeit
              && u["instanz_id"].toString() == instanz,
          a.size() == 1 ? a[0].getFileName() : juce::String ("keine Datei"));

    const auto fr = u["frame"];
    const auto sequenzOhneNak29 = q.p->merkmaleSequenzFuerTest() - q.p->nak29AbgelehntFuerTest();
    fall ("M-32", "probeeq_antwort_ohne_v3_verbindung frame_aus_der_engine (frames_gebaut, summe > 0, aktivitaet)",
          fr.getDynamicObject() != nullptr && (juce::int64) fr["frames_gebaut"] == (juce::int64) gebaut
              && (std::uint64_t) (juce::int64) fr["frames_gebaut"] == sequenzOhneNak29
              && (juce::int64) fr["summe_fenster_gesamt"] > 0 && ! fr["aktivitaet"].isVoid()
              && ! fr["evidenz_frisch"].isVoid(),
          "frames_gebaut " + fr["frames_gebaut"].toString() + ", Sequenz ohne NAK-29 " + zahl (sequenzOhneNak29)
              + ", summe_fenster_gesamt " + fr["summe_fenster_gesamt"].toString() + ", aktivitaet "
              + fr["aktivitaet"].toString());
    const bool evidenz = (bool) fr["evidenz_frisch"];
    fall ("M-32", "probeeq_antwort_ohne_v3_verbindung abdeckung_konvergenz_evidenz_fenster_nur_im_evidenzrahmen",
          fr.getDynamicObject() != nullptr
              && (evidenz ? ! fr["evidenz_fenster"].isVoid()
                          : fr["abdeckung"].isVoid() && fr["konvergenz"].isVoid() && fr["evidenz_fenster"].isVoid()),
          "evidenz_frisch " + fr["evidenz_frisch"].toString() + ", abdeckung " + fr["abdeckung"].toString()
              + ", konvergenz " + fr["konvergenz"].toString() + ", evidenz_fenster " + fr["evidenz_fenster"].toString());
    const auto s = u["snapshot"];
    const auto* so = s.getDynamicObject();
    fall ("M-32", "probeeq_antwort_ohne_v3_verbindung snapshot_ohne_befunde_rolle_aus_v2rolle",
          so != nullptr && ! so->hasProperty ("befunde") && ! so->hasProperty ("diagnose_version")
              && s["sensor"]["role"].toString() == nakama::state::v2Rolle (zustand.common),
          "role " + s["sensor"]["role"].toString() + ", v2Rolle " + nakama::state::v2Rolle (zustand.common));
    antwortM32 = u;
}

//==============================================================================
// M-80 probeeq_publiziert_auf_anfrage (Lagen a bis c)
void probeeqPubliziertAufAnfrage()
{
    {
        TempWurzel t;
        ProbeeqLauf q;
        q.bereite (48000.0, 512);
        const auto f = fassaden (t);
        starteMit (*q.p, f);
        const auto zustandVorher = zustandBytes (*q.p);
        const auto parameterVorher = parameterWerte (*q.p);

        // Der Sampleplan auf einem eigenen Audiothread, Ausgang gegen Eingang.
        std::atomic<bool> bitgleich { true };
        std::size_t audioHash = 0;
        std::thread audio ([&]
        {
            f.fs->audioThread.store (GetCurrentThreadId());
            audioHash = std::hash<std::thread::id> {} (std::this_thread::get_id());
            juce::AudioBuffer<float> puffer (2, 512), kopie (2, 512);
            referenz::Rauschen r;
            std::int64_t zeit = 0;
            for (int i = 0; i < 282; ++i)
            {
                fuelle (puffer, zeit, 48000.0, r);
                kopie.makeCopyOf (puffer, true);
                gibBlock (*q.p, puffer, zeit, 48000.0);
                for (int c = 0; c < 2; ++c)
                    if (std::memcmp (puffer.getReadPointer (c), kopie.getReadPointer (c), sizeof (float) * 512) != 0)
                        bitgleich = false;
                zeit += 512;
                if ((i + 1) % 32 == 0)
                    q.geleert();
            }
        });
        audio.join();
        const bool geleert = q.geleert();
        auto& bk = q.p->briefkastenFuerTest();

        for (int i = 0; i < 600; ++i)
            bk.takt();
        const auto auswA = q.p->auswertungenAufAnfrageFuerTest();
        fall ("M-80", "probeeq_publiziert_auf_anfrage (a) 600 takte ohne anfrage: 0 auswertungen, keineDaten",
              geleert && auswA == 0 && q.p->analyseSnapshotFuerTest().zustand == eqcop::MessZustand::keineDaten,
              "Auswertungen " + zahl (auswA));

        const auto id = neueKennung();
        schreibeAnfrage (t, id);
        const auto s0 = f.fs->stand();
        bk.takt();
        const auto d1 = differenz (f.fs->stand(), s0);
        const auto z1 = bk.zaehler();
        fall ("M-80", "probeeq_publiziert_auf_anfrage (b) erster takt setzt das flag und schreibt nicht",
              d1.anlegeversuche == 0 && z1.fehlerzaehler == 0 && z1.warteTakte == 1 && antworten (t, "probeeq", id).empty(),
              stand (d1) + ", wartend " + zahl (z1.warteTakte));
        const bool durch = q.durchlauf();
        const auto auswB = q.p->auswertungenAufAnfrageFuerTest();
        fall ("M-80", "probeeq_publiziert_auf_anfrage (b) worker wertet im naechsten durchlauf genau einmal aus",
              durch && auswB == 1, "Auswertungen " + zahl (auswB));
        bk.takt();
        const auto a = antworten (t, "probeeq", id);
        bool ok = false;
        const auto u = a.size() == 1 ? liesJson (a[0], ok) : juce::var();
        const double gesamt = (double) u["snapshot"]["gesamt_sekunden"];
        const double schwer = (double) u["frame"]["schwer_sekunden"];
        fall ("M-80", "probeeq_publiziert_auf_anfrage (b) naechster takt schreibt eine antwort mit snapshot, schwer_sekunden = gesamt_sekunden",
              a.size() == 1 && ok && u["snapshot"].getDynamicObject() != nullptr && gesamt > 0.0 && schwer == gesamt,
              "gesamt_sekunden " + juce::String (gesamt, 6) + ", schwer_sekunden " + juce::String (schwer, 6));
        fall ("M-80", "probeeq_publiziert_auf_anfrage (b) 0 fassadenaufrufe und 0 auswertungen auf dem audiothread",
              f.fs->aufAudiothread.load() == 0 && q.p->auswertungThreadFuerTest() != 0
                  && q.p->auswertungThreadFuerTest() != audioHash,
              "Fassade " + zahl (f.fs->aufAudiothread.load()));
        fall ("M-80", "probeeq_publiziert_auf_anfrage (b) audio bitgleich, state und parameter unveraendert",
              bitgleich.load() && zustandBytes (*q.p) == zustandVorher && parameterWerte (*q.p) == parameterVorher);
    }

    const auto ohneBloecke = [] (const char* lage, bool mitRate)
    {
        TempWurzel t;
        ProbeeqLauf q;
        if (mitRate)
            q.bereite (48000.0, 512);
        const auto f = fassaden (t);
        starteMit (*q.p, f);
        auto& bk = q.p->briefkastenFuerTest();
        const auto id = neueKennung();
        schreibeAnfrage (t, id);
        const auto name = [&] (const char* teil) { return std::string ("probeeq_publiziert_auf_anfrage ") + lage + " " + teil; };

        q.durchlauf();   // jeder Takt erst nach mindestens einem Workerdurchlauf
        const auto s0 = f.fs->stand();
        bk.takt();
        const auto d1 = differenz (f.fs->stand(), s0);
        const auto z1 = bk.zaehler();
        fall ("M-80", name ("erster takt setzt das flag und schreibt nicht").c_str(),
              d1.anlegeversuche == 0 && z1.fehlerzaehler == 0 && z1.warteTakte == 1, stand (d1));
        const bool durch = q.durchlauf();
        const auto ausw1 = q.p->auswertungenAufAnfrageFuerTest();
        fall ("M-80", name ("worker bedient das flag ohne block genau einmal").c_str(), durch && ausw1 == 1,
              "Auswertungen " + zahl (ausw1));
        bk.takt();
        const auto a = antworten (t, "probeeq", id);
        bool ok = false;
        const auto u = a.size() == 1 ? liesJson (a[0], ok) : juce::var();
        const auto z2 = bk.zaehler();
        fall ("M-80", name ("naechster takt schreibt eine antwort ohne messdaten, genau ein wartender takt").c_str(),
              a.size() == 1 && ok && u["snapshot"].isVoid() && u["frame"].isVoid()
                  && hatGrund (u, "snapshot", "noch_keine_messdaten") && z2.warteTakte == 1 && z2.antworten == 1,
              "Dateien " + juce::String ((int) a.size()) + ", wartend " + zahl (z2.warteTakte));
        const bool zwei = q.durchlauf();
        const auto ausw2 = q.p->auswertungenAufAnfrageFuerTest();
        fall ("M-80", name ("nach zwei weiteren workerdurchlaeufen weiter 1 auswertung").c_str(),
              zwei && ausw2 == 1 && q.p->analyseSnapshotFuerTest().zustand == eqcop::MessZustand::keineDaten,
              "Auswertungen " + zahl (ausw2));
    };
    ohneBloecke ("(c1) ohne bloecke, gueltige rate", true);
    ohneBloecke ("(c2) ohne bloecke, ohne gueltige rate", false);
}

//==============================================================================
// M-81 materialzeit_anker_leicht_und_schwer (drei Durchlaeufe)
struct Anker
{
    bool ok = false;
    double schwer = 0.0, gesamt = 0.0;
    long long nL = 0;
    juce::var ende, fortlaufend, stillstand, bloeckeMax, start, anzahl, spielt;
};

Anker liesAnker (const std::vector<juce::File>& dateien, double rate)
{
    Anker a;
    if (dateien.size() != 1)
        return a;
    bool ok = false;
    const auto u = liesJson (dateien[0], ok);
    const auto s = u["snapshot"];
    const auto fr = u["frame"];
    a.ok = ok && s.getDynamicObject() != nullptr && fr.getDynamicObject() != nullptr;
    a.schwer = (double) fr["schwer_sekunden"];
    a.gesamt = (double) s["gesamt_sekunden"];
    a.nL = std::llround (a.gesamt * rate);
    a.ende = fr["material_ende_projektsample"];
    a.fortlaufend = fr["hostzeit_fortlaufend_samples"];
    a.stillstand = fr["hostzeit_stillstand_bloecke"];
    a.bloeckeMax = fr["bloecke_max_samples"];
    a.start = fr["projekt_sample_start"];
    a.anzahl = fr["sample_count"];
    a.spielt = fr["spielt"];
    return a;
}

juce::String ankerText (const Anker& a)
{
    return "schwer " + juce::String (a.schwer, 6) + " s, gesamt " + juce::String (a.gesamt, 6) + " s, n_L "
         + juce::String (a.nL) + ", Ende " + a.ende.toString() + ", fortlaufend " + a.fortlaufend.toString()
         + ", Stillstand " + a.stillstand.toString() + ", bloecke_max " + a.bloeckeMax.toString();
}

bool gleichZahl (const juce::var& v, juce::int64 soll) { return ! v.isVoid() && (juce::int64) v == soll; }

constexpr std::int64_t kKadenzT0 = 1000000000000;   ///< Testuhr der Workerkadenz (ns)

/** Durchlauf 1 (ab H) und 2 (Grenze mitten im Plan, danach ab H2): eine schwere
    Auswertung, weitere Bloecke, eine leichte Publikation, ein Block ohne. */
void genAnker (const char* durchlauf, std::int64_t anker, bool mitGrenze)
{
    TempWurzel t;
    GenLauf g;
    const auto f = fassaden (t);
    g.p->setzeKadenzUhrFuerTest (kKadenzT0);
    g.bereite (48000.0, 512);
    starteMit (*g.p, f);
    std::int64_t zeit = mitGrenze ? 0 : anker;
    bool ok = true;
    if (mitGrenze)
    {
        ok = g.fuettere (20, zeit);
        zeit = anker;       // Zeitsprung: die Grenze setzt Engine und Zaehler zurueck
        g.verworfen = 1;    // die Quarantaene verwirft den Block vor dem Sprung
    }
    const auto ab = g.gefuettert;
    ok = ok && g.fuettere (40, zeit);
    const auto schwer0 = g.p->analyseSchwereAuswertungen();
    g.p->setzeKadenzUhrFuerTest (kKadenzT0 + 300000000);
    const bool schwer = warteBis ([&] { return g.p->analyseSchwereAuswertungen() == schwer0 + 1; }, 5000);
    const auto samplesSchwer = (std::int64_t) (g.gefuettert - ab - 1) * 512;
    ok = ok && g.fuettere (20, zeit);
    const auto samplesLeicht = (std::int64_t) (g.gefuettert - ab - 1) * 512;
    g.p->setzeKadenzUhrFuerTest (kKadenzT0 + 400000000);
    const bool leicht = warteBis ([&] { return std::llround (g.p->messSnapshot().gesamtSekunden * 48000.0) == samplesLeicht; }, 5000);
    ok = ok && g.fuettere (1, zeit);   // ein weiterer Block ohne Publikation
    const bool ohne = std::llround (g.p->messSnapshot().gesamtSekunden * 48000.0) == samplesLeicht;

    const auto id = neueKennung();
    schreibeAnfrage (t, id);
    g.p->briefkastenFuerTest().takt();
    const auto rahmen = g.p->merkmalFrame();
    const auto a = liesAnker (antworten (t, "gen", id), 48000.0);
    const auto name = [&] (const char* teil) { return std::string ("materialzeit_anker_leicht_und_schwer ") + durchlauf + " gen " + teil; };
    const auto ende = a.ende.isVoid() ? (juce::int64) -1 : (juce::int64) a.ende;
    fall ("M-81", name ("vorbedingung (schwere, dann leichte Publikation, dann ein Block ohne)").c_str(),
          ok && schwer && leicht && ohne && a.ok,
          "schwer " + juce::String (samplesSchwer) + " Samples, leicht " + juce::String (samplesLeicht) + " Samples");
    fall ("M-81", name ("schwer_sekunden = schwere samples / rate, kleiner als gesamt_sekunden").c_str(),
          a.ok && std::abs (a.schwer - (double) samplesSchwer / 48000.0) <= 1.0e-9 && a.schwer < a.gesamt
              && a.nL == samplesLeicht,
          ankerText (a));
    fall ("M-81", name ("material_ende = hostende vor der leichten publikation, K = anker").c_str(),
          a.ok && ende == anker + samplesLeicht && ende - a.nL == anker,
          "Ende " + a.ende.toString() + " (soll " + juce::String (anker + samplesLeicht) + "), K "
              + juce::String (ende - a.nL) + " (soll " + juce::String (anker) + ")");
    fall ("M-81", name ("fortlaufend = n_L, stillstand 0, bloecke_max 512").c_str(),
          a.ok && gleichZahl (a.fortlaufend, a.nL) && gleichZahl (a.stillstand, 0) && gleichZahl (a.bloeckeMax, 512),
          ankerText (a));
    const bool spieltGesetzt = (rahmen.transport.gueltigkeit & nakama::analyse::kGPlayState) != 0;
    fall ("M-81", name ("ausschnitt des rahmens aus seinem transportstempel").c_str(),
          a.ok && rahmen.transport.project_sample_start_gesetzt
              && gleichZahl (a.start, (juce::int64) rahmen.transport.project_sample_start)
              && gleichZahl (a.anzahl, (juce::int64) rahmen.transport.sample_count)
              && a.spielt.isVoid() == ! spieltGesetzt && (a.spielt.isVoid() || (bool) a.spielt == rahmen.transport.playing),
          "projekt_sample_start " + a.start.toString() + ", sample_count " + a.anzahl.toString() + ", spielt "
              + a.spielt.toString());
}

void probeeqAnker (std::int64_t anker)
{
    TempWurzel t;
    ProbeeqLauf q;
    q.bereite (48000.0, 512);
    const auto f = fassaden (t);
    starteMit (*q.p, f);
    std::int64_t zeit = anker;
    const bool ok = q.fuettere (60, zeit);
    const auto id = neueKennung();
    schreibeAnfrage (t, id);
    const bool paar = q.taktPaar();
    const auto a = liesAnker (antworten (t, "probeeq", id), 48000.0);
    const std::int64_t nSoll = 59 * 512;
    const auto ende = a.ende.isVoid() ? (juce::int64) -1 : (juce::int64) a.ende;
    fall ("M-81", "materialzeit_anker_leicht_und_schwer durchlauf 1 probeeq schwer_sekunden = gesamt_sekunden, K = H",
          ok && paar && a.ok && a.schwer == a.gesamt && a.nL == nSoll && ende == anker + nSoll && ende - a.nL == anker
              && gleichZahl (a.fortlaufend, nSoll) && gleichZahl (a.stillstand, 0) && gleichZahl (a.bloeckeMax, 512),
          ankerText (a));
    const auto s = a.start.isVoid() ? (juce::int64) -1 : (juce::int64) a.start;
    const auto n = a.anzahl.isVoid() ? (juce::int64) -1 : (juce::int64) a.anzahl;
    fall ("M-81", "materialzeit_anker_leicht_und_schwer durchlauf 1 probeeq rahmen traegt projekt_sample_start, sample_count, spielt",
          a.ok && s >= anker && n > 0 && s + n <= anker + 60 * 512 && ! a.spielt.isVoid() && (bool) a.spielt,
          "projekt_sample_start " + a.start.toString() + ", sample_count " + a.anzahl.toString() + ", spielt " + a.spielt.toString());
}

/** Durchlauf 3 (§16.1 Befund 1, P-10): 44,1 kHz, zwei 256er-Bloecke mit Stempel
    1 024, ein dritter mit demselben Stempel gibt den zweiten frei. */
void teilbloeckeAnker()
{
    const auto pruefe = [] (const char* rolle, const Anker& a, bool vorbedingung)
    {
        const auto ende = a.ende.isVoid() ? (juce::int64) -1 : (juce::int64) a.ende;
        fall ("M-81", (std::string ("materialzeit_anker_leicht_und_schwer durchlauf 3 ") + rolle
                       + " teilbloecke: n_L 512, ende 1280, fortlaufend 256, stillstand 1").c_str(),
              vorbedingung && a.ok && a.nL == 512 && ende == 1280 && gleichZahl (a.fortlaufend, 256)
                  && gleichZahl (a.stillstand, 1),
              ankerText (a) + ", K " + juce::String (ende - a.nL) + " (Kopf 1024)");
    };
    {
        TempWurzel t;
        GenLauf g;
        const auto f = fassaden (t);
        g.p->setzeKadenzUhrFuerTest (kKadenzT0);
        g.bereite (44100.0, 512);
        starteMit (*g.p, f);
        juce::AudioBuffer<float> puffer (2, 256);
        for (int i = 0; i < 3; ++i)
        {
            fuelle (puffer, 1024 + i * 256, 44100.0, g.r);
            gibBlock (*g.p, puffer, 1024, 44100.0);
            ++g.gefuettert;
        }
        const bool verbraucht = g.verbraucht();
        const auto schwer0 = g.p->analyseSchwereAuswertungen();
        g.p->setzeKadenzUhrFuerTest (kKadenzT0 + 300000000);
        const bool publiziert = warteBis ([&] { return g.p->analyseSchwereAuswertungen() == schwer0 + 1; }, 5000);
        const auto id = neueKennung();
        schreibeAnfrage (t, id);
        g.p->briefkastenFuerTest().takt();
        pruefe ("gen", liesAnker (antworten (t, "gen", id), 44100.0), verbraucht && publiziert);
    }
    {
        TempWurzel t;
        ProbeeqLauf q;
        q.bereite (44100.0, 512);
        const auto f = fassaden (t);
        starteMit (*q.p, f);
        juce::AudioBuffer<float> puffer (2, 256);
        for (int i = 0; i < 3; ++i)
        {
            fuelle (puffer, 1024 + i * 256, 44100.0, q.r);
            gibBlock (*q.p, puffer, 1024, 44100.0);
        }
        const bool geleert = q.geleert();
        const auto id = neueKennung();
        schreibeAnfrage (t, id);
        const bool paar = q.taktPaar();
        pruefe ("probeeq", liesAnker (antworten (t, "probeeq", id), 44100.0), geleert && paar);
    }
}

void materialzeitAnkerLeichtUndSchwer()
{
    genAnker ("durchlauf 1", 96000, false);
    probeeqAnker (96000);
    genAnker ("durchlauf 2 (grenze, H2)", 480000, true);
    teilbloeckeAnker();
}

//==============================================================================
// M-33 instanzen_je_eine_datei (Lagen a bis d)
void instanzenJeEineDatei()
{
    const auto lage = [] (const char* name, int anzahl, bool gleicheKennung)
    {
        TempWurzel t;
        std::vector<ProbeeqLauf> q;
        std::vector<FassadenSatz> f;
        q.reserve ((std::size_t) anzahl);
        const auto gemeinsam = juce::String (neueKennung());
        for (int i = 0; i < anzahl; ++i)
        {
            q.emplace_back();
            ladeZustand (*q.back().p, gleicheKennung ? gemeinsam : juce::String (neueKennung()), "M33", true);
            f.push_back (fassaden (t));
            starteMit (*q.back().p, f.back());
        }
        const auto id = neueKennung();
        schreibeAnfrage (t, id);
        for (auto& x : q)
            x.p->briefkastenFuerTest().takt();   // jede wartet einmal (P-9)
        bool durch = true;
        for (auto& x : q)
            durch = x.durchlauf() && durch;
        for (auto& x : q)
            x.p->briefkastenFuerTest().takt();   // jede schreibt
        const auto a = antworten (t, "probeeq", id);
        std::set<std::string> instanz16, laufzeit32;
        for (const auto& d : a)
        {
            const auto teile = juce::StringArray::fromTokens (d.getFileName(), ".", "");
            instanz16.insert (teile[3].toStdString());
            laufzeit32.insert (teile[4].toStdString());
        }
        std::uint64_t fehler = 0;
        for (auto& x : q)
            fehler += x.p->briefkastenFuerTest().zaehler().fehlerzaehler;
        const auto soll16 = gleicheKennung ? 1u : (std::size_t) anzahl;
        fall ("M-33", (std::string ("instanzen_je_eine_datei ") + name).c_str(),
              durch && (int) a.size() == anzahl && instanz16.size() == soll16 && (int) laufzeit32.size() == anzahl
                  && fehler == 0,
              "Dateien " + juce::String ((int) a.size()) + ", instanz16 " + juce::String ((int) instanz16.size())
                  + ", laufzeit32 " + juce::String ((int) laufzeit32.size()) + ", Fehler " + zahl (fehler));
    };
    lage ("(a) zwei instanzen, verschiedene kennungen", 2, false);
    lage ("(b) zwei instanzen, gleiche kennung", 2, true);
    lage ("(c) zehn instanzen, gleiche kennung", 10, true);

    // (d) Der Zielname ist durch eine Markendatei belegt: nie ersetzt.
    TempWurzel t;
    ProbeeqLauf q;
    const auto f = fassaden (t);
    starteMit (*q.p, f);
    auto& bk = q.p->briefkastenFuerTest();
    const auto k1 = neueKennung();
    schreibeAnfrage (t, k1);
    q.taktPaar();
    const auto a1 = antworten (t, "probeeq", k1);
    const auto k2 = neueKennung();
    juce::File marke;
    juce::String shaVorher;
    if (a1.size() == 1)
    {
        marke = t.antwort().getChildFile (juce::String (k2) + a1[0].getFileName().fromFirstOccurrenceOf (".", true, false));
        schreibeRoh (marke, "MARKE: dieser Zielname ist belegt");
        shaVorher = sha256Datei (marke);
    }
    fall ("M-33", "instanzen_je_eine_datei (d) vorbedingung (K1 beantwortet, Markendatei unter dem Zielnamen fuer K2)",
          a1.size() == 1 && marke.existsAsFile(), marke.getFileName());
    schreibeAnfrage (t, k2);
    const auto z0 = bk.zaehler();
    bk.takt();   // der wartende Takt zaehlt nicht
    const bool durch = q.durchlauf();
    juce::String folge;
    for (int i = 0; i < 5; ++i)
    {
        bk.takt();
        folge << (folge.isEmpty() ? "" : ",") << zahl (bk.zaehler().schreibversuche - z0.schreibversuche);
    }
    const auto z = bk.zaehler();
    const auto pid = juce::String ((juce::int64) GetCurrentProcessId());
    int temp = 0, unterZiel = 0;
    for (const auto& d : dateienIn (t.antwort()))
    {
        if (d.getFileName() == marke.getFileName() + ".tmp-" + pid)
            ++temp;
        if (d.getFileName() == marke.getFileName())
            ++unterZiel;
    }
    fall ("M-33", "instanzen_je_eine_datei (d) keine datei wird je ersetzt",
          durch && marke.existsAsFile() && sha256Datei (marke) == shaVorher && unterZiel == 1 && temp == 1
              && f.fs->stand().loeschungen == 0 && folge == "1,2,3,3,3" && z.warteTakte - z0.warteTakte == 1
              && z.fehlerzaehler - z0.fehlerzaehler == 1,
          "Marke SHA gleich " + juce::String (sha256Datei (marke) == shaVorher ? "ja" : "nein") + ", Temp "
              + juce::String (temp) + ", Versuche je Takt " + folge + ", Fehler " + zahl (z.fehlerzaehler - z0.fehlerzaehler)
              + ", Loesch " + zahl (f.fs->stand().loeschungen));
}

//==============================================================================
// M-37 state_und_parameter_unveraendert
void stateUndParameterUnveraendert()
{
    TempWurzel t;
    GenLauf g;
    ProbeeqLauf q;
    const bool daten = g.mitDaten (100);
    q.bereite (48000.0, 512);
    std::int64_t zeit = 0;
    const bool geleert = q.fuettere (60, zeit);
    const auto fg = fassaden (t);
    const auto fp = fassaden (t);
    starteMit (*g.p, fg);
    starteMit (*q.p, fp);

    DirtyZeuge dzG, dzQ;
    g.p->addListener (&dzG);
    q.p->addListener (&dzQ);
    const auto zustandG = zustandBytes (*g.p);
    const auto zustandQ = zustandBytes (*q.p);
    const auto parameterG = parameterWerte (*g.p);
    const auto parameterQ = parameterWerte (*q.p);
    for (int i = 0; i < 100; ++i)
    {
        schreibeAnfrage (t, neueKennung());
        g.p->briefkastenFuerTest().takt();
        q.taktPaar();
    }
    const auto antwortenG = g.p->briefkastenFuerTest().zaehler().antworten;
    const auto antwortenQ = q.p->briefkastenFuerTest().zaehler().antworten;
    fall ("M-37", "state_und_parameter_unveraendert vorbedingung (je 100 beantwortete anfragen)",
          daten && geleert && antwortenG == 100 && antwortenQ == 100,
          "gen " + zahl (antwortenG) + ", probeeq " + zahl (antwortenQ));
    fall ("M-37", "state_und_parameter_unveraendert gen (state-bytes, parameter, kein host-dirty)",
          zustandBytes (*g.p) == zustandG && parameterWerte (*g.p) == parameterG && dzG.geaendert.load() == 0
              && dzG.parameter.load() == 0,
          "State " + juce::String ((int) zustandG.getSize()) + " Bytes, Parameter " + juce::String ((int) parameterG.size())
              + ", Dirty " + juce::String (dzG.geaendert.load()) + ", Parameterereignisse " + juce::String (dzG.parameter.load()));
    fall ("M-37", "state_und_parameter_unveraendert probeeq (state-bytes, 112 parameter, kein host-dirty)",
          zustandBytes (*q.p) == zustandQ && parameterWerte (*q.p) == parameterQ && parameterQ.size() == 112
              && dzQ.geaendert.load() == 0 && dzQ.parameter.load() == 0,
          "State " + juce::String ((int) zustandQ.getSize()) + " Bytes, Parameter " + juce::String ((int) parameterQ.size())
              + ", Dirty " + juce::String (dzQ.geaendert.load()) + ", Parameterereignisse " + juce::String (dzQ.parameter.load()));
    g.p->removeListener (&dzG);
    q.p->removeListener (&dzQ);
}

//==============================================================================
// M-40 antwort_feldmenge_wie_schema (liest die Schemadateien)
juce::var liesSchema (const char* relativ)
{
    const auto datei = repoWurzel().getChildFile ("eq-copilot").getChildFile ("schemas").getChildFile (relativ);
    return juce::JSON::parse (datei.loadFileAsString());
}

std::set<std::string> schemaSchluessel (const juce::var& schema, const char* feld)
{
    std::set<std::string> s;
    if (auto* p = schema[feld].getDynamicObject())
        for (const auto& e : p->getProperties())
            s.insert (e.name.toString().toStdString());
    return s;
}

std::set<std::string> pflicht (const juce::var& schema)
{
    std::set<std::string> s;
    if (const auto* r = schema["required"].getArray())
        for (const auto& e : *r)
            s.insert (e.toString().toStdString());
    return s;
}

/** Jedes Objekt: Schluessel nur aus `properties`, jede Pflicht vorhanden; rekursiv. */
bool wieSchema (const juce::var& wert, const juce::var& schema, const juce::String& pfad, juce::String& fehler)
{
    if (auto* o = wert.getDynamicObject())
    {
        auto* p = schema["properties"].getDynamicObject();
        if (p == nullptr)
            return true;
        for (const auto& e : o->getProperties())
            if (! p->hasProperty (e.name))
            {
                fehler = pfad + "." + e.name.toString() + " steht nicht im Schema";
                return false;
            }
        for (const auto& r : pflicht (schema))
            if (! o->hasProperty (juce::Identifier (juce::String (r))))
            {
                fehler = pfad + "." + juce::String (r) + " fehlt";
                return false;
            }
        for (const auto& e : o->getProperties())
            if (! wieSchema (e.value, p->getProperty (e.name), pfad + "." + e.name.toString(), fehler))
                return false;
        return true;
    }
    if (const auto* a = wert.getArray())
    {
        int i = 0;
        for (const auto& x : *a)
            if (! wieSchema (x, schema["items"], pfad + "[" + juce::String (i++) + "]", fehler))
                return false;
    }
    return true;
}

void antwortFeldmengeWieSchema()
{
    const auto antwortSchema = liesSchema ("diagnose/nakama-diagnose-antwort-v1.schema.json");
    const auto snapshotSchema = liesSchema ("eq-snapshot.schema.json");
    const auto kopf = schemaSchluessel (antwortSchema, "properties");
    const auto rahmen = schemaSchluessel (antwortSchema["properties"]["frame"], "properties");
    const auto grund = schemaSchluessel (antwortSchema["properties"]["gruende"]["items"], "properties");
    fall ("M-40", "antwort_feldmenge_wie_schema vorbedingung (schema gelesen: 12 kopf-, 25 rahmen-, 3 grundfelder)",
          kopf.size() == 12 && pflicht (antwortSchema) == kopf && rahmen.size() == 25
              && pflicht (antwortSchema["properties"]["frame"]) == rahmen && grund.size() == 3);

    const auto pruefe = [&] (const char* name, const juce::var& u)
    {
        juce::String fehler;
        bool ok = u.getDynamicObject() != nullptr && schluessel (u) == kopf;
        if (! ok)
            fehler = "Kopf: " + juce::String ((int) schluessel (u).size()) + " Schluessel";
        if (ok && u["frame"].getDynamicObject() != nullptr && schluessel (u["frame"]) != rahmen)
        {
            ok = false;
            fehler = "frame: " + juce::String ((int) schluessel (u["frame"]).size()) + " Schluessel";
        }
        if (ok)
            if (const auto* g = u["gruende"].getArray())
                for (const auto& e : *g)
                    if (schluessel (e) != grund)
                    {
                        ok = false;
                        fehler = "gruende: Schluessel weichen ab";
                    }
        if (ok && u["snapshot"].getDynamicObject() != nullptr)
            ok = wieSchema (u["snapshot"], snapshotSchema, "snapshot", fehler);
        fall ("M-40", (std::string ("antwort_feldmenge_wie_schema ") + name).c_str(), ok, fehler);
    };
    pruefe ("M-24 gen mit messdaten", antwortM24);
    pruefe ("M-31 gen ohne messdaten", antwortM31Gen);
    pruefe ("M-31 probeeq ohne messdaten", antwortM31Probeeq);
    pruefe ("M-32 probeeq mit messdaten", antwortM32);

    // Die bestehenden Vertraege bytegleich zum Basis-SHA, und A11 zaehlt weiter fuenf.
    const auto schemas = repoWurzel().getChildFile ("eq-copilot").getChildFile ("schemas");
    const auto shaSnapshot = sha256Datei (schemas.getChildFile ("eq-snapshot.schema.json"));
    const auto shaAggregat = sha256Datei (schemas.getChildFile ("eq-aggregat.schema.json"));
    fall ("M-40", "antwort_feldmenge_wie_schema eq-snapshot und eq-aggregat bytegleich zum basis-sha",
          shaSnapshot.equalsIgnoreCase ("7C2A1003EDABCEB215EFE9ED90F9650D34D27AB8135BB651A4AE4E2E38ACF7B7")
              && shaAggregat.equalsIgnoreCase ("EFBD8F12A03E4AD71940A0810B8D1D7E13DEFAF581A50DFAFCA1223E1B499A68"),
          shaSnapshot + ", " + shaAggregat);
    const auto oben = schemas.findChildFiles (juce::File::findFiles, false, "*.schema.json");
    fall ("M-40", "antwort_feldmenge_wie_schema oberste ebene traegt weiter fuenf schemadateien (A11)", oben.size() == 5,
          juce::String (oben.size()));
}

//==============================================================================
// M-49 reparse_punkte_werden_ignoriert (C++-Haelfte)
void reparsePunkteWerdenIgnoriert()
{
    {
        TempWurzel t;   // (a) antwort\ wird nach dem Start eine Junction
        auto g = std::make_unique<Gen>();
        const auto f = fassaden (t);
        starteMit (*g, f);
        const auto waechter = t.wurzel.getChildFile ("waechter-antwort");
        waechter.createDirectory();
        const bool link = t.antwort().deleteRecursively() && legeJunctionAn (t, t.antwort(), waechter);
        schreibeAnfrage (t, neueKennung());
        const auto s0 = f.fs->stand();
        for (int i = 0; i < 5; ++i)
            g->briefkastenFuerTest().takt();
        const auto d = differenz (f.fs->stand(), s0);
        fall ("M-49", "reparse_punkte_werden_ignoriert (a) antwort ist eine junction",
              link && dateienIn (waechter).empty() && d.anlegeversuche == 0 && g->briefkastenFuerTest().zaehler().antworten == 0,
              "Junction " + juce::String (link ? "ja" : "nein") + ", Waechter " + juce::String ((int) dateienIn (waechter).size())
                  + ", " + stand (d));
    }
    {
        TempWurzel t;   // (b) diagnose\ wird nach dem Start eine Junction
        auto g = std::make_unique<Gen>();
        const auto f = fassaden (t);
        starteMit (*g, f);
        const auto waechter = t.wurzel.getChildFile ("waechter-diagnose");
        waechter.getChildFile ("antwort").createDirectory();
        schreibeRoh (waechter.getChildFile ("anfrage.json"), anfrageText (neueKennung()));
        const bool link = t.diagnose().deleteRecursively() && legeJunctionAn (t, t.diagnose(), waechter);
        const auto s0 = f.fs->stand();
        for (int i = 0; i < 5; ++i)
            g->briefkastenFuerTest().takt();
        const auto d = differenz (f.fs->stand(), s0);
        const auto imWaechter = dateienIn (waechter.getChildFile ("antwort"));
        fall ("M-49", "reparse_punkte_werden_ignoriert (b) diagnose ist eine junction",
              link && t.anfrage().existsAsFile() && imWaechter.empty() && d.leseoeffnungen == 0 && d.anlegeversuche == 0
                  && g->briefkastenFuerTest().zaehler().antworten == 0,
              "Junction " + juce::String (link ? "ja" : "nein") + ", Waechter " + juce::String ((int) imWaechter.size())
                  + ", " + stand (d));
    }
    {
        TempWurzel t;   // (c) anfrage.json ist ein Symlink
        auto g = std::make_unique<Gen>();
        const auto f = fassaden (t);
        starteMit (*g, f);
        const auto ziel = t.wurzel.getChildFile ("waechter-anfrage.json");
        schreibeRoh (ziel, anfrageText (neueKennung()));
        DWORD fehler = 0;
        if (! legeSymlinkAn (t, t.anfrage(), ziel, fehler))
        {
            std::cout << "[--]  M-49 reparse_punkte_werden_ignoriert (c) symlink auf anfrage.json nicht erzeugbar [Fehler "
                      << fehler << "] - Lage (c) ungemessen" << std::endl;
            return;
        }
        const auto s0 = f.fs->stand();
        for (int i = 0; i < 5; ++i)
            g->briefkastenFuerTest().takt();
        const auto d = differenz (f.fs->stand(), s0);
        fall ("M-49", "reparse_punkte_werden_ignoriert (c) anfrage.json ist ein symlink",
              d.leseoeffnungen == 0 && d.anlegeversuche == 0 && dateienIn (t.antwort()).empty()
                  && g->briefkastenFuerTest().zaehler().antworten == 0,
              stand (d));
    }
}

//==============================================================================
// M-50 label_mit_steuerzeichen_bleibt_gueltiges_json
#if defined (_MSC_VER)
 #pragma comment (lib, "advapi32.lib")
#endif

juce::String sddl (const juce::File& f)
{
    PSECURITY_DESCRIPTOR sd = nullptr;
    constexpr SECURITY_INFORMATION was = DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION;
    if (GetNamedSecurityInfoW (f.getFullPathName().toWideCharPointer(), SE_FILE_OBJECT, was,
                               nullptr, nullptr, nullptr, nullptr, &sd) != ERROR_SUCCESS)
        return {};
    LPWSTR text = nullptr;
    juce::String s;
    if (ConvertSecurityDescriptorToStringSecurityDescriptorW (sd, SDDL_REVISION_1, was, &text, nullptr))
    {
        s = juce::String (text);
        LocalFree (text);
    }
    LocalFree (sd);
    return s;
}

bool nurGeerbteRechte (const juce::File& f, juce::String& detail)
{
    PACL dacl = nullptr;
    PSECURITY_DESCRIPTOR sd = nullptr;
    if (GetNamedSecurityInfoW (f.getFullPathName().toWideCharPointer(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION,
                               nullptr, nullptr, &dacl, nullptr, &sd) != ERROR_SUCCESS)
        return false;
    SECURITY_DESCRIPTOR_CONTROL steuerung = 0;
    DWORD revision = 0;
    GetSecurityDescriptorControl (sd, &steuerung, &revision);
    int geerbt = 0, eigen = 0;
    if (dacl != nullptr)
        for (DWORD i = 0; i < dacl->AceCount; ++i)
        {
            LPVOID ace = nullptr;
            if (GetAce (dacl, i, &ace))
                ((((ACE_HEADER*) ace)->AceFlags & INHERITED_ACE) != 0) ? ++geerbt : ++eigen;
        }
    const bool geschuetzt = (steuerung & SE_DACL_PROTECTED) != 0;
    detail = "ACEs geerbt " + juce::String (geerbt) + ", eigene " + juce::String (eigen) + ", DACL geschuetzt "
           + (geschuetzt ? "ja" : "nein");
    LocalFree (sd);
    return dacl != nullptr && ! geschuetzt && eigen == 0 && geerbt > 0;
}

void labelMitSteuerzeichenBleibtGueltigesJson()
{
    TempWurzel t;
    GenLauf g;
    juce::String label ("Q\"B\\S");
    label += juce::String::charToString ((juce::juce_wchar) 1);
    label += "\nZ";
    while (label.length() < 4000)
        label += "x";
    ladeZustand (*g.p, juce::String (neueKennung()), label, false);
    const bool geladen = g.p->holeZustandKopie().common.label == label;
    const bool daten = g.mitDaten (100);
    const auto f = fassaden (t);
    starteMit (*g.p, f);
    fall ("M-50", "label_mit_steuerzeichen_bleibt_gueltiges_json vorbedingung (label mit \", \\, U+0001, \\n, 4000 zeichen geladen, messdaten)",
          geladen && daten, "Laenge " + juce::String (label.length()));

    const auto rechteOrdnerVorher = sddl (t.antwort());
    const auto id = neueKennung();
    schreibeAnfrage (t, id);
    g.p->briefkastenFuerTest().takt();
    const auto a = antworten (t, "gen", id);
    juce::MemoryBlock bytes;
    if (a.size() == 1)
        a[0].loadFileAsData (bytes);
    const auto* roh = (const char*) bytes.getData();
    const bool ohneBom = bytes.getSize() >= 3 && ! ((unsigned char) roh[0] == 0xEF && (unsigned char) roh[1] == 0xBB && (unsigned char) roh[2] == 0xBF);
    const bool utf8 = bytes.getSize() > 0 && juce::CharPointer_UTF8::isValidString (roh, (int) bytes.getSize());
    juce::var u;
    const bool geparst = utf8 && juce::JSON::parse (juce::String::fromUTF8 (roh, (int) bytes.getSize()), u).wasOk();
    const bool zurueck = geparst && u["snapshot"]["sensor"]["label"].toString() == label;
    fall ("M-50", "label_mit_steuerzeichen_bleibt_gueltiges_json antwort ist gueltiges utf-8-json ohne bom, label unveraendert",
          a.size() == 1 && ohneBom && utf8 && geparst && zurueck,
          "Bytes " + juce::String ((int) bytes.getSize()) + ", ohne BOM " + juce::String (ohneBom ? "ja" : "nein")
              + ", UTF-8 " + juce::String (utf8 ? "ja" : "nein") + ", geparst " + juce::String (geparst ? "ja" : "nein")
              + ", Label zurueck " + juce::String (zurueck ? "ja" : "nein"));
    juce::String rechte;
    const bool geerbt = a.size() == 1 && nurGeerbteRechte (a[0], rechte);
    fall ("M-50", "label_mit_steuerzeichen_bleibt_gueltiges_json keine rechteaenderung (datei nur geerbte rechte, ordner unveraendert)",
          geerbt && rechteOrdnerVorher.isNotEmpty() && sddl (t.antwort()) == rechteOrdnerVorher, rechte);
}

//==============================================================================
// M-30 antwort_ohne_editor (echter Timer, Nachrichtenschleife laeuft)
void antwortOhneEditor()
{
    TempWurzel t;
    auto gen = std::make_unique<Gen>();
    ProbeeqLauf q;
    q.bereite (48000.0, 512);
    const auto fg = fassaden (t);
    const auto fp = fassaden (t);
    auto& bg = gen->briefkastenFuerTest();
    auto& bq = q.p->briefkastenFuerTest();
    const bool start = starteMit (*gen, fg, true) == dg::Startgrund::gestartet
                    && starteMit (*q.p, fp, true) == dg::Startgrund::gestartet;
    fall ("M-30", "antwort_ohne_editor vorbedingung (timer 1000 ms gestartet, kein editor)",
          start && gen->getActiveEditor() == nullptr && q.p->getActiveEditor() == nullptr && ! q.p->hasEditor()
              && bg.taktIntervallMs() == 1000 && bq.taktIntervallMs() == 1000,
          "Intervall gen " + juce::String (bg.taktIntervallMs()) + ", probeeq " + juce::String (bq.taktIntervallMs()));

    const auto id = neueKennung();
    const auto takteVorher = bg.zaehler().takte;
    schreibeAnfrage (t, id);
    const bool genDa = pumpe (4000, [&] { return antworten (t, "gen", id).size() == 1; });
    const auto zg = bg.zaehler();
    fall ("M-30", "antwort_ohne_editor gen antwortet im ersten takt des timers",
          genDa && zg.takte == takteVorher + 1 && zg.antworten == 1 && zg.warteTakte == 0
              && fg.fs->ausserhalbMessageThread.load() == 0,
          "Takte " + zahl (zg.takte - takteVorher) + ", Antworten " + zahl (zg.antworten) + ", ausserhalb Message-Thread "
              + zahl (fg.fs->ausserhalbMessageThread.load()));
    const bool probeeqDa = pumpe (6000, [&] { return antworten (t, "probeeq", id).size() == 1; });
    const auto zq = bq.zaehler();
    fall ("M-30", "antwort_ohne_editor probeeq antwortet im takt nach genau einem wartenden",
          probeeqDa && zq.antworten == 1 && zq.warteTakte == 1 && fp.fs->ausserhalbMessageThread.load() == 0,
          "Antworten " + zahl (zq.antworten) + ", wartend " + zahl (zq.warteTakte) + ", ausserhalb Message-Thread "
              + zahl (fp.fs->ausserhalbMessageThread.load()));
}

//==============================================================================
// M-34 zerstoerung_bei_laufendem_takt (erzwungenes Interleaving)
void zerstoerungBeiLaufendemTakt()
{
    {
        // Keine Anfrage: ein Takt, der die Schleuse passierte, zaehlte eine
        // Existenzpruefung - der Kanarienwert - und fasste sonst nichts an.
        TempWurzel t;
        auto gen = std::make_unique<Gen>();
        const auto f = fassaden (t);
        const bool start = starteMit (*gen, f, true) == dg::Startgrund::gestartet;
        const auto zeuge = gen->briefkastenFuerTest().zaehlerZeuge();
        struct Barriere { std::atomic<bool> scharf { true }, erreicht { false }, zerstoert { false }, abgelaufen { false }; };
        const auto b = std::make_shared<Barriere>();
        gen->briefkastenFuerTest().setzeHakenVorSchleuse ([b]
        {
            if (! b->scharf.exchange (false))
                return;
            b->erreicht = true;
            const auto bis = std::chrono::steady_clock::now() + std::chrono::seconds (15);
            while (! b->zerstoert.load() && std::chrono::steady_clock::now() < bis)
                std::this_thread::sleep_for (std::chrono::milliseconds (1));
            b->abgelaufen = ! b->zerstoert.load();
        });
        const auto kanarieVorher = f.fs->stand().existenzpruefungen;
        std::thread zerstoerer ([&gen, b]
        {
            const auto bis = std::chrono::steady_clock::now() + std::chrono::seconds (5);
            while (! b->erreicht.load() && std::chrono::steady_clock::now() < bis)
                std::this_thread::sleep_for (std::chrono::milliseconds (1));
            gen.reset();   // der Destruktor auf einem zweiten Thread
            b->zerstoert = true;
        });
        // Der Timer feuert auf dem Message-Thread; der Takt haelt am Haken, bis der
        // Destruktor zurueckgekehrt ist, und laeuft dann weiter.
        const bool zerstoert = pumpe (20000, [&] { return b->zerstoert.load(); });
        zerstoerer.join();
        const auto nachFreigabe = zeuge();
        const auto kanarie = f.fs->stand().existenzpruefungen;
        pumpe (2500);
        const auto spaeter = zeuge();
        fall ("M-34", "zerstoerung_bei_laufendem_takt vorbedingung (takt am haken, destruktor auf zweitem thread zurueck)",
              start && b->erreicht.load() && zerstoert && ! b->abgelaufen.load());
        fall ("M-34", "zerstoerung_bei_laufendem_takt angehaltener takt abgewiesen, kanarienwert unberuehrt",
              nachFreigabe.abgewieseneTakte == 1 && kanarie == kanarieVorher && nachFreigabe.takte == 0,
              "abgewiesen " + zahl (nachFreigabe.abgewieseneTakte) + ", Kanarienwert " + zahl (kanarieVorher) + " -> "
                  + zahl (kanarie) + ", Takte " + zahl (nachFreigabe.takte));
        fall ("M-34", "zerstoerung_bei_laufendem_takt nach der rueckkehr des destruktors beginnt kein takt mehr (2,5 s)",
              spaeter.takteBegonnen == nachFreigabe.takteBegonnen,
              "begonnen " + zahl (nachFreigabe.takteBegonnen) + " -> " + zahl (spaeter.takteBegonnen));
    }
    {
        // Zerstoerung auf dem Message-Thread: der Stopp ist der erste Schritt.
        TempWurzel t;
        auto gen = std::make_unique<Gen>();
        const auto f = fassaden (t);
        starteMit (*gen, f, true);
        auto* roh = gen.get();
        int intervall = -1;
        bool schleuseOffen = false;
        gen->briefkastenFuerTest().setzeHakenBeimStopp ([&]
        {
            intervall = roh->briefkastenFuerTest().taktIntervallMs();
            const auto s = roh->callbackSchleuseFuerTest();
            schleuseOffen = s != nullptr && ! s->istGeschlossen();
        });
        gen.reset();
        fall ("M-34", "zerstoerung_bei_laufendem_takt gen: stopp ist der erste destruktorschritt (timer laeuft, callbackschleuse offen)",
              intervall == 1000 && schleuseOffen,
              "Intervall " + juce::String (intervall) + ", Schleuse offen " + juce::String (schleuseOffen ? "ja" : "nein"));
    }
    {
        TempWurzel t;
        auto pq = std::make_unique<Probeeq>();
        const auto f = fassaden (t);
        starteMit (*pq, f, true);
        auto* roh = pq.get();
        int intervall = -1;
        bool workerLebt = false;
        pq->briefkastenFuerTest().setzeHakenBeimStopp ([&]
        {
            intervall = roh->briefkastenFuerTest().taktIntervallMs();
            const auto d0 = roh->workerDurchlaeufeFuerTest();
            workerLebt = warteBis ([&] { return roh->workerDurchlaeufeFuerTest() >= d0 + 2; }, 2000);
        });
        pq.reset();
        fall ("M-34", "zerstoerung_bei_laufendem_takt probeeq: stopp ist der erste destruktorschritt (timer laeuft, worker lebt)",
              intervall == 1000 && workerLebt,
              "Intervall " + juce::String (intervall) + ", Worker lebt " + juce::String (workerLebt ? "ja" : "nein"));
    }
}

//==============================================================================
// M-36 kadenz_hoechstens_ein_hertz
void kadenzHoechstensEinHertz()
{
    TempWurzel t;
    const auto f = fassaden (t);
    auto bk = std::make_unique<dg::Briefkasten>();
    bk->setzeFassaden (f.wurzel, f.fs, f.uhr);
    dg::Konfiguration k;
    k.wurzelRelativ = dg::kWurzelRelativ;
    k.rolle = "gen";
    k.laufzeit32 = neueKennung();
    const bool start = bk->starte (k, true) == dg::Startgrund::gestartet;
    fall ("M-36", "kadenz_hoechstens_ein_hertz intervall 1000 ms (getTimerInterval)",
          start && bk->taktIntervallMs() == 1000, juce::String (bk->taktIntervallMs()) + " ms");

    const auto e0 = f.fs->stand().existenzpruefungen;
    const auto b0 = bk->zaehler().takteBegonnen;
    pumpe (10000);
    std::cout << "[roh] M-36 kadenz_hoechstens_ein_hertz takte in 10 s wanduhr: "
              << (bk->zaehler().takteBegonnen - b0) << " begonnen, "
              << (f.fs->stand().existenzpruefungen - e0) << " existenzpruefungen" << std::endl;

    // Zweiter Durchlauf: der laufende Takt haelt 2,5 s an einer Barriere.
    struct Barriere
    {
        std::atomic<bool> scharf { true }, erreicht { false }, frei { false };
        std::atomic<std::uint64_t> an { 0 }, nach { 0 };
    };
    const auto b = std::make_shared<Barriere>();
    const auto zeuge = bk->zaehlerZeuge();
    bk->setzeHakenVorSchleuse ([b, zeuge]
    {
        if (! b->scharf.exchange (false))
            return;
        b->an = zeuge().takteBegonnen;
        b->erreicht = true;
        const auto bis = std::chrono::steady_clock::now() + std::chrono::seconds (10);
        while (! b->frei.load() && std::chrono::steady_clock::now() < bis)
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
        b->nach = zeuge().takteBegonnen;
    });
    std::thread loeser ([b]
    {
        const auto bis = std::chrono::steady_clock::now() + std::chrono::seconds (5);
        while (! b->erreicht.load() && std::chrono::steady_clock::now() < bis)
            std::this_thread::sleep_for (std::chrono::milliseconds (1));
        std::this_thread::sleep_for (std::chrono::milliseconds (2500));
        b->frei = true;
    });
    pumpe (6000);
    loeser.join();
    const auto z = bk->zaehler();
    fall ("M-36", "kadenz_hoechstens_ein_hertz barriere: kein zweiter takt (laufend hoechstens 1, begonnen waehrend der barriere 0)",
          b->erreicht.load() && z.laufendMax <= 1 && b->nach.load() == b->an.load(),
          "laufend hoechstens " + zahl (z.laufendMax) + ", begonnen an der Barriere " + zahl (b->an.load())
              + ", bei der Freigabe " + zahl (b->nach.load()));
    bk->stoppe();
}

//==============================================================================
/** Ein Audiothread wie im Host: Rauschen in Echtzeit durch Gen und Probeeq,
    jeder Ausgang Sample fuer Sample gegen den Eingang (M-35, M-54). */
struct Audiothread
{
    Gen& gen;
    Probeeq& pq;
    PruefFassade& fg;
    PruefFassade& fp;
    std::atomic<bool> laeuft { true }, bereit { false }, bitgleichGen { true }, bitgleichProbeeq { true };
    std::atomic<std::uint64_t> bloecke { 0 };
    std::thread faden;

    Audiothread (Gen& g, Probeeq& q, PruefFassade& a, PruefFassade& b) : gen (g), pq (q), fg (a), fp (b)
    {
        faden = std::thread ([this] { lauf(); });
        warteBis ([this] { return bereit.load(); }, 5000);
    }
    ~Audiothread() { anhalten(); }
    void anhalten()
    {
        laeuft = false;
        if (faden.joinable())
            faden.join();
    }
    void lauf()
    {
        const auto id = GetCurrentThreadId();
        fg.audioThread.store (id);
        fp.audioThread.store (id);
        bereit = true;
        juce::AudioBuffer<float> a (2, 512), b (2, 512), soll (2, 512);
        juce::Random zufall (286);
        std::int64_t zeit = 0;
        auto naechster = std::chrono::steady_clock::now();
        while (laeuft.load())
        {
            for (int c = 0; c < 2; ++c)
                for (int i = 0; i < 512; ++i)
                    soll.setSample (c, i, zufall.nextFloat() * 1.8f - 0.9f);
            a.makeCopyOf (soll, true);
            b.makeCopyOf (soll, true);
            gibBlock (gen, a, zeit, 48000.0);
            gibBlock (pq, b, zeit, 48000.0);
            for (int c = 0; c < 2; ++c)
            {
                if (std::memcmp (a.getReadPointer (c), soll.getReadPointer (c), sizeof (float) * 512) != 0)
                    bitgleichGen = false;
                if (std::memcmp (b.getReadPointer (c), soll.getReadPointer (c), sizeof (float) * 512) != 0)
                    bitgleichProbeeq = false;
            }
            zeit += 512;
            bloecke.fetch_add (1);
            naechster += std::chrono::microseconds (10667);   // 512 Samples bei 48 kHz
            std::this_thread::sleep_until (naechster);
        }
    }
};

//==============================================================================
// M-35 nulltest_mit_briefkasten
void nulltestMitBriefkasten()
{
    TempWurzel t;
    auto gen = std::make_unique<Gen>();
    auto pq = std::make_unique<Probeeq>();
    gen->setPlayConfigDetails (2, 2, 48000.0, 512);
    gen->prepareToPlay (48000.0, 512);
    pq->setPlayConfigDetails (2, 2, 48000.0, 512);
    pq->prepareToPlay (48000.0, 512);
    const auto fg = fassaden (t);
    const auto fp = fassaden (t);
    starteMit (*gen, fg);
    starteMit (*pq, fp);
    auto& bg = gen->briefkastenFuerTest();
    auto& bq = pq->briefkastenFuerTest();

    std::vector<std::string> ids;
    auto audio = std::make_unique<Audiothread> (*gen, *pq, *fg.fs, *fp.fs);
    const auto anfang = std::chrono::steady_clock::now();
    for (;;)
    {
        const auto ms = std::chrono::duration_cast<std::chrono::milliseconds> (std::chrono::steady_clock::now() - anfang).count();
        if (ms >= 10000)
            break;
        if ((long long) ids.size() * 1000 <= ms && ids.size() < 10)
        {
            ids.push_back (neueKennung());
            schreibeAnfrage (t, ids.back());
        }
        bg.takt();   // Takte auf dem Message-Thread, nie im Audiothread
        bq.takt();
        pumpe (100);
    }
    audio->anhalten();
    for (int i = 0; i < 4; ++i)   // die letzte Probeeq-Anfrage fertig beantworten
    {
        bg.takt();
        bq.takt();
        pumpe (100);
    }
    int beantwortet = 0;
    for (const auto& id : ids)
        if (antworten (t, "gen", id).size() == 1 && antworten (t, "probeeq", id).size() == 1)
            ++beantwortet;
    const auto bloecke = audio->bloecke.load();
    fall ("M-35", "nulltest_mit_briefkasten gen: ausgang sample fuer sample gleich dem eingang (10 s rauschen)",
          audio->bitgleichGen.load() && bloecke >= 800, zahl (bloecke) + " Bloecke");
    fall ("M-35", "nulltest_mit_briefkasten probeeq: ausgang sample fuer sample gleich dem eingang (10 s rauschen)",
          audio->bitgleichProbeeq.load() && bloecke >= 800, zahl (bloecke) + " Bloecke");
    fall ("M-35", "nulltest_mit_briefkasten 0 fassadenaufrufe mit der thread-id des audiothreads",
          fg.fs->aufAudiothread.load() == 0 && fp.fs->aufAudiothread.load() == 0,
          "gen " + zahl (fg.fs->aufAudiothread.load()) + ", probeeq " + zahl (fp.fs->aufAudiothread.load()));
    fall ("M-35", "nulltest_mit_briefkasten die antworten entstehen trotzdem (10 anfragen, je gen und probeeq)",
          ids.size() == 10 && beantwortet == 10, juce::String (beantwortet) + " von " + juce::String ((int) ids.size()));
}

//==============================================================================
// M-54 langsamer_takt_ohne_audiowirkung
void langsamerTaktOhneAudiowirkung()
{
    TempWurzel t;
    auto gen = std::make_unique<Gen>();
    auto pq = std::make_unique<Probeeq>();
    gen->setPlayConfigDetails (2, 2, 48000.0, 512);
    gen->prepareToPlay (48000.0, 512);
    pq->setPlayConfigDetails (2, 2, 48000.0, 512);
    pq->prepareToPlay (48000.0, 512);
    const auto fg = fassaden (t);
    const auto fp = fassaden (t);
    starteMit (*gen, fg);
    starteMit (*pq, fp);
    auto& bg = gen->briefkastenFuerTest();
    auto& bq = pq->briefkastenFuerTest();
    auto audio = std::make_unique<Audiothread> (*gen, *pq, *fg.fs, *fp.fs);
    warteBis ([&] { return audio->bloecke.load() > 100; }, 5000);
    const auto probeeqDurchlauf = [&]
    {
        const auto d0 = pq->workerDurchlaeufeFuerTest();
        return warteBis ([&] { return pq->workerDurchlaeufeFuerTest() >= d0 + 2; }, 5000);
    };

    // Durchlauf 1: zwischen Kopie und Schreiben prueft ein ZWEITER Thread die Sperre.
    int freiGen = -1, freiProbeeq = -1;
    auto* g = gen.get();
    auto* q = pq.get();
    gen->setzeDiagnoseHakenFuerTest ([&freiGen, g]
    { freiGen = std::async (std::launch::async, [g] { return g->analyseSteuerSperreFreiFuerTest (500); }).get() ? 1 : 0; });
    pq->setzeDiagnoseHakenFuerTest ([&freiProbeeq, q]
    { freiProbeeq = std::async (std::launch::async, [q] { return q->analyseSchlossFreiFuerTest (500); }).get() ? 1 : 0; });
    const auto id = neueKennung();
    schreibeAnfrage (t, id);
    bg.takt();
    bq.takt();
    probeeqDurchlauf();
    bq.takt();
    fall ("M-54", "langsamer_takt_ohne_audiowirkung gen: beim schreiben ist die analyse-steuersperre frei",
          freiGen == 1 && antworten (t, "gen", id).size() == 1, "frei " + juce::String (freiGen));
    fall ("M-54", "langsamer_takt_ohne_audiowirkung probeeq: beim schreiben ist analyseSchloss frei",
          freiProbeeq == 1 && antworten (t, "probeeq", id).size() == 1, "frei " + juce::String (freiProbeeq));
    fall ("M-54", "langsamer_takt_ohne_audiowirkung erster durchlauf: audio bitgleich (gen, probeeq)",
          audio->bitgleichGen.load() && audio->bitgleichProbeeq.load(), zahl (audio->bloecke.load()) + " Bloecke");

    // Durchlauf 2: der Takt schlaeft 2,5 s - der Message-Thread ist blockiert.
    gen->setzeDiagnoseHakenFuerTest ([] { std::this_thread::sleep_for (std::chrono::milliseconds (2500)); });
    pq->setzeDiagnoseHakenFuerTest ([] { std::this_thread::sleep_for (std::chrono::milliseconds (2500)); });
    const auto id2 = neueKennung();
    schreibeAnfrage (t, id2);
    const auto b0 = audio->bloecke.load();
    bg.takt();
    const auto waehrendGen = audio->bloecke.load() - b0;
    bq.takt();
    probeeqDurchlauf();
    const auto b1 = audio->bloecke.load();
    bq.takt();
    const auto waehrendProbeeq = audio->bloecke.load() - b1;
    audio->anhalten();
    fall ("M-54", "langsamer_takt_ohne_audiowirkung zweiter durchlauf: 2,5 s blockierter takt, audio laeuft weiter und bleibt bitgleich",
          audio->bitgleichGen.load() && audio->bitgleichProbeeq.load() && waehrendGen >= 150 && waehrendProbeeq >= 150
              && antworten (t, "gen", id2).size() == 1 && antworten (t, "probeeq", id2).size() == 1,
          "Bloecke waehrend des Gen-Takts " + zahl (waehrendGen) + ", waehrend des Probeeq-Takts " + zahl (waehrendProbeeq));
}

} // namespace

int main (int argc, char* argv[])
{
    juce::ScopedJuceInitialiser_GUI juceStart;

    if (argc > 1 && std::strcmp (argv[1], "--erzeuge") == 0)
        return erzeugeReferenz();

    festhaltenBytegleich();
    ohneAnfrageNurExistenzpruefung();
    anfrageGenGenauEineAntwort();
    gleicheKennungKeineZweiteAntwort();
    neueKennungNeueAntwort();
    fremdeAnfragenWerdenIgnoriert();
    gesperrteAnfrageImNaechstenTakt();
    antwortordnerFehltDreiVersuche();
    ohneMessdatenAntwortMitGrund();
    pfadtraversalUnmoeglich();
    kennungsringFifo256();
    uhrsprungAendertZuordnungNicht();
    keineLeereAntwort();
    startfehlerWurzelOhneTakt();
    testbauStartetKeinenBriefkasten();
    summenzaehlerUeberAbschluesseUndGrenze();
    probeeqAntwortOhneV3Verbindung();
    probeeqPubliziertAufAnfrage();
    materialzeitAnkerLeichtUndSchwer();
    instanzenJeEineDatei();
    stateUndParameterUnveraendert();
    antwortFeldmengeWieSchema();
    reparsePunkteWerdenIgnoriert();
    labelMitSteuerzeichenBleibtGueltigesJson();
    antwortOhneEditor();
    zerstoerungBeiLaufendemTakt();
    nulltestMitBriefkasten();
    kadenzHoechstensEinHertz();
    langsamerTaktOhneAudiowirkung();

    std::cout << "NAK-286 BRIEFKASTEN: " << bestanden << " bestanden, "
              << fehlgeschlagen << " fehlgeschlagen" << std::endl;
    return fehlgeschlagen == 0 ? 0 : 1;
}
