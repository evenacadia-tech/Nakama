// SONDE-013 (S20-22) — der RT→Control-Ring für hörbare Eingriffe
// (M-37, M-38, M-39; Entwurf §34.2).
//
// ─────────────────────────────────────────────────────────────────────────────
// WARUM EIN 1-HZ-HEARTBEAT-BIT NICHT REICHT.
//
// §34.2 sagt es woertlich, und der Satz ist der ganze Grund fuer diese Datei:
//
//     "Ein 1-Hz-Heartbeat-Boolean reicht dafuer nicht: Ein kurzer Hoermarker
//      kann vollstaendig zwischen zwei Heartbeats liegen."
//
// Bis SONDE-013 fuehrte `PluginProcessor.cpp` genau so ein Bit
// (`k.hoermarkierung = markierung.hoerbar()`). Ein Marker von 300 ms zwischen
// zwei Heartbeats war damit unsichtbar — und jede nachgelagerte Sonde, die
// waehrenddessen mass, lieferte Evidenz aus verfaerbtem Audio, ohne dass es
// jemand wusste. Das ist die schlimmste Sorte Fehler, die dieses Projekt
// kennt: eine Messung, die aussieht wie eine Messung.
//
// ─────────────────────────────────────────────────────────────────────────────
// WARUM DIESER RING KEIN `drop-oldest` HAT — im Gegensatz zu P2.
//
// M-39 und §34.2 sind hier ausdruecklich: Overflow, Control-Disconnect oder
// eine Sequenzluecke setzen **sticky** `intervention_state_unknown`, und
// starke Evidenz bleibt bis zu einem expliziten Neutral-Resync gesperrt.
//
// Der Grund ist eine Asymmetrie. Ein verlorener LIVEFRAME kostet eine Anzeige;
// ein verlorenes BEGIN kostet die Wahrheit. Ohne das Begin sieht die Timeline
// aus, als waere nie etwas passiert — sie erzeugt eine "scheinbar saubere
// Baseline" (§34.2 woertlich), und alles, was darauf gebaut wird, ist falsch.
// Deshalb verwirft dieser Ring bei Ueberlauf NICHT das aelteste Ereignis,
// sondern MELDET, dass er nicht mehr vollstaendig ist. Der Verlust ist die
// Nachricht.
//
// ─────────────────────────────────────────────────────────────────────────────
// ECHTZEIT.
//
// `schreibe()` laeuft im Audiothread: keine Allokation, keine Sperre, kein
// Logging, feste Groesse. Der Ring ist ein SPSC-Puffer mit zwei Atomics und
// einem festen Feld — dieselbe Bauform wie die `StampedAudioQueue` aus
// SONDE-008, nur kleiner, weil ein Interventionsereignis 64 Byte hat und
// nicht einen Audioblock.

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

namespace nakama::ipc
{

//==============================================================================
/** Konservativer Quarantaene-Nachlauf zu einem beendeten Eingriff (NAK-180 R5).

    Zwei Zusagen stecken in dieser Zahl, und beide waren gebrochen:

    1. Sie rechnet mit einer GEPRUEFTEN Samplerate. Der Aufrufer uebergibt
       `letzteGueltigeSamplerate`, nicht JUCEs `getSampleRate()`: eine
       nicht-endliche oder ueber `INT_MAX` liegende Hostrate machte
       `(int) getSampleRate()` zu undefiniertem Verhalten. Auf MSVC/x64
       liefert `cvttsd2si` dort `INT_MIN`, `INT_MIN / 10` ist negativ, und
       `std::max (1, …)` ergab genau EIN Sample Nachlauf statt rund 100 ms —
       gefaerbtes Audio laege danach ausserhalb der Invalidierung (§34.2:
       "zu kurz waere hier der teure Fehler").
    2. Sie SAETTIGT an beiden Raendern. Die Verdopplung einer sehr langen
       Markerdauer lief vorher ueber und ergab einen Nachlauf, der KUERZER
       war als der Eingriff selbst — derselbe teure Fehler durch eine andere
       Tuer.

    Der Nachlauf ist doppelte Dauer plus ein festes Zehntel der Rate. Der
    Deckel von 76800 Samples ist ein Zehntel der hoechsten vom Plugin
    akzeptierten Rate (768 kHz, `prepareToPlay`); eine hoehere Zahl koennte
    nur aus einer Rate stammen, die nie geprueft durchgekommen waere. */
inline std::uint64_t tailSamplesFuer (std::uint64_t dauerSamples, double samplerate) noexcept
{
    constexpr std::uint64_t kMax = std::numeric_limits<std::uint64_t>::max();
    constexpr double kPolsterDeckel = 76800.0;          // 768 kHz / 10
    const double zehntel = std::isfinite (samplerate) ? samplerate / 10.0 : 0.0;
    // Ohne je gueltige Rate bleibt 1 - dieselbe untere Schranke wie zuvor.
    // Sie ist hier unerreichbar, weil der Aufrufer mit 48000 vorbelegt ist;
    // die Zeile haelt die Funktion trotzdem fuer sich allein ehrlich.
    const std::uint64_t polster =
        zehntel >= 1.0 ? (std::uint64_t) std::min (zehntel, kPolsterDeckel) : 1u;
    const std::uint64_t doppelt = dauerSamples > kMax / 2u ? kMax : dauerSamples * 2u;
    return doppelt > kMax - polster ? kMax : doppelt + polster;
}

//==============================================================================
/** Ein hoerbarer Eingriff, wie ihn der Audiothread meldet.

    Die `art` ist bewusst KEIN Feld: SONDE-013 baut nur den Hoermarker
    (§7.1, E-08 — kein Marker in Probeeq, keine Preview in diesem Ticket).
    Ein Feld mit genau einem moeglichen Wert waere ein totes Element; die
    drei uebrigen Arten aus dem Schema (`preview`, `focus_burst`,
    `experiment`) bekommen ihre Erzeuger in P6 und P7. */
struct Interventionsereignis
{
    /** `true` = `audible_intervention_begin`, `false` = `…_end`. */
    bool beginn { false };

    /** Monoton je Instanz. Eine LUECKE hier setzt beim Empfaenger sticky
        `intervention_state_unknown` — deshalb vergibt sie der Erzeuger und
        nicht der Sender: ein Ereignis, das den Ring nie erreicht hat, hat
        seine Nummer trotzdem verbraucht, und genau die Luecke ist die
        Nachricht (dieselbe Regel wie bei `frameversuch` in der
        `FeatureEngine`, T2-2). */
    std::uint64_t sequenz { 0 };

    /** Die laufende Nummer DIESES Eingriffs. Begin und End teilen sie sich —
        daraus baut der Sender die `intervention_id`.

        Sie ist bewusst getrennt von `sequenz`: die Sequenz zaehlt EREIGNISSE
        (und ihre Luecken sind die Fehlermeldung), die Nummer zaehlt
        EINGRIFFE. Aus der Sequenz die Paarung abzuleiten — etwa "das End
        gehoert zum Begin mit Sequenz minus eins" — waere genau dann falsch,
        wenn ein Ereignis verloren ging, also im einzigen Fall, in dem es
        darauf ankommt. */
    std::uint64_t nummer { 0 };

    /** Projektzeit, NUR wenn `projektzeitGesetzt`. Ohne Bit reist auf der
        Leitung `null` — nicht 0. */
    bool         projektzeitGesetzt { false };
    std::int64_t projektSample { 0 };

    /** Nur beim Ende: der konservative Nachlauf (§34.2 — "der Bereich wird
        laenger quarantaenisiert als die Intervention selbst dauerte"). */
    std::uint64_t tailSamples { 0 };

    /** Wie viele Samples der Eingriff wirklich hoerbar war. Nur beim Ende
        gefuellt; der Sender rechnet daraus `tailSamples` nicht neu, sondern
        traegt beide, damit ein Empfaenger die Konservativitaet PRUEFEN kann
        statt sie zu glauben. */
    std::uint64_t dauerSamples { 0 };
};

//==============================================================================
/** Fester SPSC-Ring vom Audiothread zum Control-Worker.

    Kapazitaet als Zweierpotenz, damit die Indexrechnung eine Maske ist. 32
    Ereignisse sind bei einem Hoermarker, der frueheste alle paar hundert
    Millisekunden ein Paar erzeugt, mehr als drei Sekunden Vorrat — und der
    Control-Worker leert ihn im 100-ms-Takt. Laeuft er trotzdem ueber, ist
    das ein Systemzustand und kein Rundungsfehler, und genau dann soll er
    melden statt zu verwerfen. */
class InterventionsRing
{
public:
    static constexpr int kPlaetze = 32;
    static_assert ((kPlaetze & (kPlaetze - 1)) == 0, "Zweierpotenz");

    /** Audiothread. `false` heisst UEBERLAUF: das Ereignis ist NICHT
        aufgenommen, und das Sticky-Bit steht.

        ⚠️ Der Rueckgabewert ist kein Hoeflichkeitswert. Ein Aufrufer, der ihn
        ignoriert, macht aus dem lautesten Fehler dieses Pfades einen stillen —
        genau die Zeile, die die Pruefliste §A mit "Rueckgabewerte und Zaehler
        werden ausgewertet" meint. */
    bool schreibe (const Interventionsereignis& e) noexcept
    {
        const auto k = kopf.load (std::memory_order_relaxed);
        const auto s = schwanz.load (std::memory_order_acquire);
        if (k - s >= (std::uint64_t) kPlaetze)
        {
            // KEIN drop-oldest (M-39). Der Verlust ist die Nachricht.
            ueberlauf.store (true, std::memory_order_release);
            verworfen.fetch_add (1, std::memory_order_relaxed);
            return false;
        }
        plaetze[(std::size_t) (k & (kPlaetze - 1))] = e;
        kopf.store (k + 1, std::memory_order_release);
        return true;
    }

    /** Control-Worker. `false` heisst leer. */
    bool lies (Interventionsereignis& aus) noexcept
    {
        const auto s = schwanz.load (std::memory_order_relaxed);
        const auto k = kopf.load (std::memory_order_acquire);
        if (s == k)
            return false;
        aus = plaetze[(std::size_t) (s & (kPlaetze - 1))];
        schwanz.store (s + 1, std::memory_order_release);
        return true;
    }

    /** Sticky: einmal gesetzt, bleibt es bis `resync()`.

        Es ist ABSICHTLICH nicht selbstheilend. §34.2: "starke Evidenz bleibt
        bis zu einem expliziten Neutral-/Sequenz-Resync gesperrt". Ein Bit,
        das sich von selbst zuruecksetzt, sobald wieder Platz ist, verloere
        genau die Information, um derentwillen es existiert. */
    bool ueberlaufGesehen() const noexcept
    { return ueberlauf.load (std::memory_order_acquire); }

    std::uint64_t verworfeneEreignisse() const noexcept
    { return verworfen.load (std::memory_order_relaxed); }

    /** Der einzige Weg, das Sticky-Bit zu loeschen. Nicht im Audiothread. */
    void resync() noexcept
    { ueberlauf.store (false, std::memory_order_release); }

    /** Wie viele Ereignisse noch auf ihre Abholung warten. */
    int fuellstand() const noexcept
    {
        const auto k = kopf.load (std::memory_order_acquire);
        const auto s = schwanz.load (std::memory_order_acquire);
        return (int) (k - s);
    }

    /** Leert den Ring UND das Sticky-Bit. Nur beim Neuaufbau der Verbindung
        oder in `prepareToPlay` — nie im Audiothread. */
    void zuruecksetzen() noexcept
    {
        kopf.store (0, std::memory_order_relaxed);
        schwanz.store (0, std::memory_order_relaxed);
        ueberlauf.store (false, std::memory_order_relaxed);
        verworfen.store (0, std::memory_order_relaxed);
    }

private:
    std::array<Interventionsereignis, kPlaetze> plaetze {};
    std::atomic<std::uint64_t> kopf { 0 };
    std::atomic<std::uint64_t> schwanz { 0 };
    std::atomic<bool>          ueberlauf { false };
    std::atomic<std::uint64_t> verworfen { 0 };
};

} // namespace nakama::ipc
