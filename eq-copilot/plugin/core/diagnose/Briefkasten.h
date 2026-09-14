/*  NAK-286 Etappe 2 - der Diagnose-Briefkasten (Plan S25e (3), R-286-1, R-286-2).

    Liegt `%LOCALAPPDATA%\evenacadia\nakama\diagnose\anfrage.json`, schreibt die
    Instanz auf dem Message-Thread genau eine Antwort je Anfragekennung in den
    festen Antwortordner `antwort\`; ohne Datei geschieht nichts Messbares.
    Spezifikation: docs/beweise/NAK-286.md, §2 (F-1 bis F-16), §4.3, §5.5.

    KERN, NICHT ZIELSCHICHT. Dieser Halter liegt in NakamaKern und sieht keine
    JucePlugin_*-Konstante (K1, A14). Die Ordnernamen unter %LOCALAPPDATA%
    tragen den eingefrorenen Herstellernamen; der Prozessor gibt die Wurzel
    deshalb als Zeichenkette herein, wie den Pipenamen (§20 P-11, §7.3 Punkt 9).
    Die Wurzel bleibt fest (F-1) - nur ihr Traeger ist die Zielschicht.

    WAS DER TAKT TUT (F-2, F-8 bis F-11, F-13, F-14):
      * je Takt genau EINE Existenzpruefung (Attribute von `anfrage.json`);
        ohne Datei nichts weiter;
      * gelesen wird nur eine geaenderte Datei (Aenderungszeit, Groesse), nur
        bis 1 024 Bytes und nur, wenn `evenacadia\`, `nakama\` und `diagnose\`
        vorhandene Ordner ohne Reparse-Punkt sind;
      * eine gueltige Kennung, die nicht im Ring der 256 zuletzt beantworteten
        steht, wird ueber den eigenen Schreibweg beantwortet: Temp-Datei
        exklusiv anlegen, schreiben und spuelen, Groesse pruefen, ohne
        Ersetzen umbenennen - nie ein Ordner, nie eine Loeschung;
      * hoechstens drei Schreibversuche je Kennung in drei Takten; ein Takt,
        der auf die Auswertung wartet (Probeeq), ist keiner.

    JEDER Dateizugriff laeuft durch die Dateisystem-Fassade und ist dort
    gezaehlt; die Wurzel `%LOCALAPPDATA%` selbst bestimmt die Wurzel-Fassade
    mit dem Zaehler "Wurzelabfragen". Im Produkt sind beide die echten Aufrufe,
    im Test Attrappen (F-14, F-16).

    LEBENSDAUER (F-12, §23.2 P-12, §26.2 P-14): der Takt ist ein `juce::Timer`
    des KERNS (1 000 ms, Message-Thread); den Kern haelt der Halter ueber einen
    `shared_ptr`. `stoppe()` ist der erste Destruktorschritt des Besitzers:
    Timer aus, danach die Schleuse zu. JUCE ruft einen Timer ueber einen rohen
    Zeiger, und `stopTimer()` wartet auf keinen laufenden Rueckruf
    (juce_Timer.cpp:161-171, :207-224). Der Halter gibt seine Referenz deshalb
    nach `stoppe()` an den Message-Thread ab (`MessageManager::callAsync`), der
    eine Nachricht erst nach dem laufenden Rueckruf bearbeitet: ein Rueckruf,
    den der Timer vor dem Stopp begonnen hat - auch vor seiner ersten
    Anweisung -, fasst nur lebenden Kern an, und `Timer::~Timer` laeuft auf
    dem Message-Thread. Sofort frei wird der Kern nur, wenn kein Rueckruf
    laufen kann: der Timer lief nie, es gibt keinen MessageManager, oder der
    Halter endet auf dem Message-Thread. Lehnt JUCE die Nachricht ab, wird die
    Referenz nie auf dem aufrufenden Thread frei: sie liegt bis zum
    Prozessende im Halteplatz (P-14).

    WARTEN (§26.2 P-12 praezisiert): kein Weg wartet ueber den Message-Thread
    (kein MessageManagerLock, kein Warten auf die Zustellung einer Nachricht)
    und keiner auf einen Rueckruf, der die Schleuse noch nicht betreten hat.
    Ein Takt, der nach dem Stopp an die Schleuse kommt, wird abgewiesen - er
    ruft die Antwortquelle des zerstoerten Besitzers nie mehr. Einen Takt, der
    beim Schliessen schon hinter der Schleuse ist, wartet `stoppe()` zu Ende
    (Muster `callbackSchleuse`, NAK-246 R-D2) - begrenzt durch diesen einen
    Takt, weil er hinter der Schleuse nie auf den zerstoerenden Thread wartet:
    `stoppe()` ist der erste Destruktorschritt ohne gehaltene Sperre, und die
    Antwortquelle nimmt nur Sperren des Prozessors, deren Halter nie auf den
    Destruktor warten (Manifest §7.1 T-35, M-34 Lage (c)).
*/

#pragma once

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace nakama::diagnose
{

inline constexpr std::size_t kAnfrageGrenzeBytes = 1024;   ///< F-2
inline constexpr int         kKennungsring       = 256;    ///< F-13
inline constexpr int         kSchreibversuche    = 3;      ///< F-13
inline constexpr int         kTaktMs             = 1000;   ///< F-12, hoechstens 1 Hz
inline constexpr const char* kAnfrageFormat      = "nakama.diagnose.anfrage.v1";

/** Eine Anfragekennung: genau 32 Zeichen aus 0-9a-f (F-2). */
struct Kennung
{
    std::array<char, 32> z {};
    bool operator== (const Kennung& a) const noexcept { return z == a.z; }
    bool operator!= (const Kennung& a) const noexcept { return z != a.z; }
    std::string text() const { return std::string (z.data(), z.size()); }
};

/** true genau fuer 32 Zeichen aus 0-9a-f. */
bool kennungGueltig (const char* zeichen, std::size_t anzahl) noexcept;

/** Liest eine Anfrage nach F-2: UTF-8 ohne BOM, ein JSON-Objekt mit genau den
    Schluesseln `format` (= nakama.diagnose.anfrage.v1) und `anfrage_id`.
    Alles andere ist fremd geformt. Legt keinen Speicher an. */
bool anfrageLesen (const char* daten, std::size_t anzahl, Kennung& aus) noexcept;

/** ISO-8601 in UTC mit Millisekunden und `Z`. */
std::string isoUtc (std::int64_t millisekundenSeit1970);

//== Fassaden (F-14) ==========================================================

struct DateiAttribute
{
    bool         existiert = false;
    bool         verzeichnis = false;
    bool         reparse = false;
    std::int64_t groesse = 0;
    std::int64_t aenderungszeit = 0;   ///< nur zum Vergleich (FILETIME-Ticks)
};

/** Zaehlerstand einer Dateisystem-Fassade. Jede Zahl ist fuer sich atomar. */
struct FassadenStand
{
    std::uint64_t existenzpruefungen = 0;
    std::uint64_t leseoeffnungen = 0;
    std::uint64_t geleseneBytes = 0;
    std::uint64_t anlegeversuche = 0;
    std::uint64_t schreibvorgaenge = 0;
    std::uint64_t groessenabfragen = 0;
    std::uint64_t umbenennungen = 0;
    std::uint64_t ordneranlagen = 0;
    std::uint64_t loeschungen = 0;

    /** Alle Aufrufe zusammen. `geleseneBytes` ist keine Aufrufzahl. */
    std::uint64_t aufrufe() const noexcept
    {
        return existenzpruefungen + leseoeffnungen + anlegeversuche + schreibvorgaenge
             + groessenabfragen + umbenennungen + ordneranlagen + loeschungen;
    }
};

/** Jeder Dateizugriff des Briefkastens. Die oeffentlichen Schritte zaehlen und
    rufen dann die Implementierung; ein Zugriff an ihr vorbei waere einer, den
    kein Zaehler sieht (§10.3 (d)). */
class DateisystemFassade
{
public:
    using Handle = void*;   ///< nullptr = keine Datei (unter Win32 der HANDLE selbst)

    virtual ~DateisystemFassade() = default;

    DateiAttribute attribute (const std::wstring& pfad);
    bool   lies (const std::wstring& pfad, char* ziel, std::size_t hoechstens, std::size_t& gelesen);
    Handle legeExklusivAn (const std::wstring& pfad);
    bool   schreibeUndSpuele (Handle datei, const char* daten, std::size_t anzahl);
    void   schliesse (Handle datei);
    bool   groesse (const std::wstring& pfad, std::int64_t& aus);
    bool   benenneUmOhneErsetzen (const std::wstring& von, const std::wstring& nach);
    bool   legeOrdnerAn (const std::wstring& pfad);
    bool   loesche (const std::wstring& pfad);

    FassadenStand stand() const noexcept;

protected:
    virtual DateiAttribute attributeImpl (const std::wstring& pfad) = 0;
    virtual bool   liesImpl (const std::wstring& pfad, char* ziel, std::size_t hoechstens, std::size_t& gelesen) = 0;
    virtual Handle legeExklusivAnImpl (const std::wstring& pfad) = 0;
    virtual bool   schreibeUndSpueleImpl (Handle datei, const char* daten, std::size_t anzahl) = 0;
    virtual void   schliesseImpl (Handle datei) = 0;
    virtual bool   groesseImpl (const std::wstring& pfad, std::int64_t& aus) = 0;
    virtual bool   benenneUmOhneErsetzenImpl (const std::wstring& von, const std::wstring& nach) = 0;
    virtual bool   legeOrdnerAnImpl (const std::wstring& pfad) = 0;
    virtual bool   loescheImpl (const std::wstring& pfad) = 0;

private:
    std::atomic<std::uint64_t> zExistenz { 0 }, zLese { 0 }, zBytes { 0 }, zAnlegen { 0 },
                               zSchreiben { 0 }, zGroesse { 0 }, zUmbenennen { 0 },
                               zOrdner { 0 }, zLoeschen { 0 };
};

/** Die echten Win32-Aufrufe (F-8: `MoveFileExW` ohne REPLACE_EXISTING,
    `CreateFileW` mit CREATE_NEW; gelesen wird ohne einem Link zu folgen). */
class EchteDateisystemFassade : public DateisystemFassade
{
protected:
    DateiAttribute attributeImpl (const std::wstring& pfad) override;
    bool   liesImpl (const std::wstring& pfad, char* ziel, std::size_t hoechstens, std::size_t& gelesen) override;
    Handle legeExklusivAnImpl (const std::wstring& pfad) override;
    bool   schreibeUndSpueleImpl (Handle datei, const char* daten, std::size_t anzahl) override;
    void   schliesseImpl (Handle datei) override;
    bool   groesseImpl (const std::wstring& pfad, std::int64_t& aus) override;
    bool   benenneUmOhneErsetzenImpl (const std::wstring& von, const std::wstring& nach) override;
    bool   legeOrdnerAnImpl (const std::wstring& pfad) override;
    bool   loescheImpl (const std::wstring& pfad) override;
};

/** `%LOCALAPPDATA%`: der Pfad und ob er ein vorhandener Ordner ist (F-1). */
class WurzelFassade
{
public:
    virtual ~WurzelFassade() = default;
    bool wurzel (std::wstring& pfad);
    std::uint64_t wurzelabfragen() const noexcept { return zAbfragen.load (std::memory_order_relaxed); }

protected:
    virtual bool wurzelImpl (std::wstring& pfad) = 0;

private:
    std::atomic<std::uint64_t> zAbfragen { 0 };
};

class EchteWurzelFassade : public WurzelFassade
{
protected:
    bool wurzelImpl (std::wstring& pfad) override;
};

/** Liefert `erzeugt_utc` (F-14). Der Takt selbst zaehlt Takte, keine Zeit. */
class UhrFassade
{
public:
    virtual ~UhrFassade() = default;
    virtual std::int64_t jetztUtcMs() = 0;
};

class EchteUhrFassade : public UhrFassade
{
public:
    std::int64_t jetztUtcMs() override;
};

//== Antwortquelle ============================================================

struct Anfrage
{
    Kennung       kennung;
    std::uint32_t pid = 0;
    std::string   erzeugtUtc;
    /** Erster Aufruf fuer diese offene Kennung. Probeeq setzt hier sein
        Anfrage-Flag und meldet `wartet` (§13.2 P-9, F-5). */
    bool          zyklusNeu = false;
};

struct Antwort
{
    /** Die Auswertung ist angefordert, aber noch nicht publiziert. Ein
        wartender Takt ist kein Schreibversuch (F-13). */
    bool        wartet = false;
    /** Die persistente Instanzkennung (UTF-8). Der Name traegt die ersten 16
        Hex-Zeichen ihres SHA-256 (F-3). */
    std::string instanzId;
    /** Der fertige Umschlag nach nakama.diagnose.antwort.v1 (UTF-8). */
    std::string umschlag;
};

using Antwortquelle = std::function<Antwort (const Anfrage&)>;

//== Der Halter ===============================================================

struct Konfiguration
{
    /** Die drei Ebenen unter %LOCALAPPDATA%, mit `\` getrennt - vom Prozessor. */
    std::wstring wurzelRelativ;
    std::string  rolle;        ///< gen | probeeq
    std::string  laufzeit32;   ///< Laufzeitkennung des Instanzobjekts (F-3)
};

enum class Startgrund
{
    nichtGestartet,
    gestartet,
    konfiguration,   ///< Konfiguration unvollstaendig oder ungueltig
    keineWurzel,     ///< %LOCALAPPDATA% ist kein vorhandener Ordner (F-1)
    reparsePunkt     ///< eine vorhandene Ebene ist ein Reparse-Punkt (F-11)
};

/** Instanzlokale Zaehler, alle monoton (Testzugang). */
struct Zaehler
{
    std::uint64_t starts = 0;
    std::uint64_t takteBegonnen = 0;       ///< jeder Eintritt, auch abgewiesene
    std::uint64_t takte = 0;               ///< Takte, die gearbeitet haben
    std::uint64_t abgewieseneTakte = 0;    ///< an der Schleuse abgewiesen
    std::uint64_t uebersprungeneTakte = 0; ///< ein anderer Takt lief noch
    std::uint64_t laufendMax = 0;          ///< gleichzeitig laufende Takte, hoechstens
    std::uint64_t warteTakte = 0;
    std::uint64_t schreibversuche = 0;
    std::uint64_t antworten = 0;
    std::uint64_t fehlerzaehler = 0;       ///< aufgegebene Kennungen (F-13)
    std::uint64_t ausnahmen = 0;
};

class Briefkasten final
{
public:
    Briefkasten();
    /** `stoppe()`, danach die Referenz auf den Kern nach P-12 und P-14 abgeben. */
    ~Briefkasten();

    /** Vor dem Start. Ohne Aufruf gelten die echten Fassaden. */
    void setzeFassaden (std::shared_ptr<WurzelFassade> wurzel,
                        std::shared_ptr<DateisystemFassade> dateisystem,
                        std::shared_ptr<UhrFassade> uhr);
    void setzeAntwortquelle (Antwortquelle quelle);

    /** F-1: Wurzel und Ebenen pruefen, dann den Takt starten (1 000 ms). Ein
        Startfehler startet keinen Takt; der Grund bleibt instanzlokal (P-2). */
    Startgrund starte (const Konfiguration& konfiguration, bool mitTimer = true);

    /** F-12: Timer aus, Schleuse zu. Idempotent; erster Destruktorschritt des
        Besitzers. Wartet einen Takt zu Ende, der die Schleuse schon betreten
        hat, nie einen davor (§26.2 P-12 praezisiert, M-34 Lage (c)). */
    void stoppe();

    /** Ein Takt synchron auf dem aufrufenden Thread (Testzugang, F-16). */
    void takt();

    Startgrund startgrund() const noexcept;
    Zaehler    zaehler() const noexcept;
    int        taktIntervallMs() const noexcept;

    /** Die Zaehler der Fassaden, die der Halter gerade benutzt (M-38). */
    FassadenStand dateisystemStand() const noexcept;
    std::uint64_t wurzelabfragen() const noexcept;

    /** Liest die Zaehler auch nach dem Ende des Halters (M-34). Der Leser haelt
        nur das Zaehlwerk, nie den Kern: als Mitbesitzer des Timer-Traegers
        koennte er ihn sonst auf einem Fremdthread zerstoeren (P-12). */
    std::function<Zaehler()> zaehlerZeuge() const;

    /** Testzugang (M-34 Lage (b), P-12): `true`, solange der Kern lebt -
        beobachtet ueber einen `weak_ptr`, der ihn nicht haelt. */
    std::function<bool()> kernBeobachter() const;

    /** Testzugang (M-34 Lage (b) (4), P-14): wie viele Kerne der prozessweite
        Halteplatz haelt. Er gibt keinen je frei. */
    static std::size_t gehalteneKerne();

    /** Testhaken: ERSTE Anweisung des Timer-Rueckrufs, vor jeder Beruehrung
        des Kerns (M-34 Lage (b), P-12). */
    void setzeHakenTimerEintritt (std::function<void()> haken);
    /** Testhaken: VOR dem Eintritt in die Schleuse (M-34, M-36). */
    void setzeHakenVorSchleuse (std::function<void()> haken);
    /** Testhaken: am Anfang von `stoppe()`, vor `stopTimer` (M-34). */
    void setzeHakenBeimStopp (std::function<void()> haken);
    /** Testhaken (M-34 Lage (b) (4), P-14): liefert er `true`, behandelt die
        Freigabe die Uebergabe an den Message-Thread als abgelehnt. */
    void setzeHakenUebergabeAbgelehnt (std::function<bool()> haken);

private:
    struct Kern;
    std::shared_ptr<Kern> kern;

    JUCE_DECLARE_NON_COPYABLE (Briefkasten)
};

} // namespace nakama::diagnose
