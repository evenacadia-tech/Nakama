// CallbackSchleuse - Besitz fuer Produkt-Callbacks (NAK-246 D2, Regel R-D2).
//
// ── WARUM ES SIE GIBT ──────────────────────────────────────────────────────
//
// `ControlClient::stop()` und `TelemetryClient::stop()` warten hoechstens
// `kStopFristMs` auf einen laufenden Callback und LOESEN den Clientthread dann
// ab (`B-CC-12`, `B-TC-07`). Der abgeloeste Thread haelt seine Laufzeit selbst
// - aber die Produkt-Lambdas darin fingen den Prozessor bis NAK-246 als rohes
// `this`. Ein Callback, der die Frist ueberlebte, lief danach auf einem
// Prozessor weiter, den der Host laengst zerstoert hatte (Auditbefund D2,
// Use-after-free; Register NAK-184, Detach-Teil).
//
// Die Schleuse gibt dem EMPFAENGER der Callbacks einen Besitzvertrag:
//
//   * Ein Callback BETRITT die Schleuse, bevor er Besitzerzustand anfasst,
//     und verlaesst sie am Ende (RAII-`Zug`).
//   * Der Besitzer SCHLIESST sie in seinem Destruktor, NACH `stop()` seiner
//     Clients: ein Callback, der danach beginnt, wird ABGEWIESEN und gezaehlt
//     (`abgewiesen`); ein Callback, der beim Schliessen bereits laeuft, wird
//     zu Ende GEWARTET - der Destruktor kehrt erst zurueck, wenn `aktiv == 0`.
//     Die Wartezeit ist durch die Callback-Dauer begrenzt und steht danach in
//     `gewartetMs` (Manifest docs/beweise/NAK-246.md, Paragraph 5.2, M-06 bis
//     M-09).
//   * Die Lambdas halten die Schleuse als `shared_ptr`. Nach der Zerstoerung
//     des Besitzers haelt die abgeloeste Laufzeit damit eine GESCHLOSSENE
//     Schleuse - "Callbacks, die niemanden mehr beruehren" (R-D2), woertlich.
//
// ── SPERRENORDNUNG ─────────────────────────────────────────────────────────
//
// Im Callback-Pfad (`betreten`, `Zug::~Zug`) gibt es KEINE Sperre, nur
// Atomics: die Callbacks laufen unter `sendeMutex` des ControlClients und
// nehmen den Sendezustand des Prozessors - eine Schleusensperre dort waere
// eine neue Verschachtelung, die dieses Projekt nirgends fuehrt. `warteMutex`
// und `warte` gehoeren ausschliesslich dem Schliesspfad. Das Wecken aus dem
// Callback-Pfad (`notify_all`) nimmt keine Nutzersperre; ein dabei verlorenes
// Wecken kostet den Schliesser hoechstens eine Wartefrist von einer
// Millisekunde, weil er mit Frist wartet und den Zaehler selbst prueft.
//
// Das Fenster zwischen "geschlossen gelesen" und "aktiv erhoeht" schliesst
// `betreten` durch ein ZWEITES Lesen nach dem Erhoehen: beide Seiten arbeiten
// sequenziell konsistent, also sieht entweder der Schliesser den erhoehten
// Zaehler (und wartet) oder der Eintretende das gesetzte Flag (und tritt
// wieder aus) - oder beides; nie keins von beiden.
//
// ── WAS SIE NICHT IST ──────────────────────────────────────────────────────
//
// Kein Ersatz fuer `sollAbbrechen` im Client (das entscheidet den START eines
// Callbacks nach `stop()`), kein Lebenszeichen des Clients und kein Mutex fuer
// Prozessorzustand. Sie entscheidet allein, ob ein Callback den Besitzer noch
// beruehren darf.

#pragma once

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>

namespace nakama::ipc
{

class CallbackSchleuse
{
public:
    /** Der Zug: solange er lebt, zaehlt der Callback als LAUFEND. Ein leerer
        Zug (`operator bool` false) heisst "abgewiesen" - der Callback fasst
        dann nichts an und kehrt sofort zurueck. */
    class Zug
    {
    public:
        Zug() noexcept = default;
        Zug (Zug&& andere) noexcept : schleuse (andere.schleuse) { andere.schleuse = nullptr; }
        Zug& operator= (Zug&& andere) noexcept
        {
            if (this != &andere)
            {
                verlassen();
                schleuse = andere.schleuse;
                andere.schleuse = nullptr;
            }
            return *this;
        }
        Zug (const Zug&) = delete;
        Zug& operator= (const Zug&) = delete;
        ~Zug() { verlassen(); }

        explicit operator bool() const noexcept { return schleuse != nullptr; }

    private:
        friend class CallbackSchleuse;
        explicit Zug (CallbackSchleuse* s) noexcept : schleuse (s) {}
        void verlassen() noexcept
        {
            if (schleuse != nullptr)
            {
                schleuse->verlassen();
                schleuse = nullptr;
            }
        }
        CallbackSchleuse* schleuse = nullptr;
    };

    /** Die Zaehler, wie ein Bein sie liest. Kein konsistenter Schnappschuss
        ueber alle fuenf - jede Zahl fuer sich ist atomar. */
    struct Stand
    {
        bool          geschlossen = false;
        std::uint32_t aktiv       = 0;   ///< gerade laufende Callbacks
        std::uint64_t betreten    = 0;   ///< je erfolgreichem `betreten()`
        std::uint64_t abgewiesen  = 0;   ///< je Versuch nach dem Schliessen
        std::uint64_t gewartetMs  = 0;   ///< was `schliessen()` gewartet hat
    };

    CallbackSchleuse() = default;
    CallbackSchleuse (const CallbackSchleuse&) = delete;
    CallbackSchleuse& operator= (const CallbackSchleuse&) = delete;

    /** Betreten - vom Callback, auf seinem Thread, ohne Sperre und ohne
        Warten. Liefert einen lebenden Zug oder einen leeren (abgewiesen). */
    [[nodiscard]] Zug betreten() noexcept
    {
        if (geschlossen.load (std::memory_order_seq_cst))
        {
            abgewiesenZaehler.fetch_add (1, std::memory_order_relaxed);
            return Zug {};
        }
        aktivZaehler.fetch_add (1, std::memory_order_seq_cst);
        // Das zweite Lesen schliesst das Fenster zwischen Pruefung und
        // Erhoehung (Kopfkommentar, Sperrenordnung).
        if (geschlossen.load (std::memory_order_seq_cst))
        {
            verlassen();
            abgewiesenZaehler.fetch_add (1, std::memory_order_relaxed);
            return Zug {};
        }
        betretenZaehler.fetch_add (1, std::memory_order_relaxed);
        return Zug { this };
    }

    /** Schliessen - vom Besitzer, in seinem Destruktor, NACH `stop()` der
        Clients. Kehrt erst zurueck, wenn kein Callback mehr laeuft; die
        Wartezeit steht danach in `gewartetMs` (aufgerundet, mindestens 1,
        wenn ueberhaupt gewartet wurde). Idempotent. */
    void schliessen()
    {
        geschlossen.store (true, std::memory_order_seq_cst);
        if (aktivZaehler.load (std::memory_order_seq_cst) == 0)
            return;

        const auto beginn = std::chrono::steady_clock::now();
        {
            std::unique_lock<std::mutex> l (warteMutex);
            // Mit Frist, nicht blind: das Wecken aus `verlassen()` laeuft ohne
            // diese Sperre und kann VOR dem Eintritt ins Warten liegen. Die
            // Frist begrenzt den Preis eines verlorenen Weckens auf 1 ms; die
            // Wahrheit ist der Zaehler, nicht das Wecken.
            while (aktivZaehler.load (std::memory_order_seq_cst) != 0)
                warte.wait_for (l, std::chrono::milliseconds (1));
        }
        const auto dauer = std::chrono::steady_clock::now() - beginn;
        const auto us = std::chrono::duration_cast<std::chrono::microseconds> (dauer).count();
        const auto ms = static_cast<std::uint64_t> (us < 0 ? 0 : (us + 999) / 1000);
        gewartetMsWert.store (ms == 0 ? 1 : ms, std::memory_order_relaxed);
    }

    bool istGeschlossen() const noexcept
    {
        return geschlossen.load (std::memory_order_seq_cst);
    }

    Stand stand() const noexcept
    {
        Stand s;
        s.geschlossen = geschlossen.load (std::memory_order_seq_cst);
        s.aktiv       = aktivZaehler.load (std::memory_order_seq_cst);
        s.betreten    = betretenZaehler.load (std::memory_order_relaxed);
        s.abgewiesen  = abgewiesenZaehler.load (std::memory_order_relaxed);
        s.gewartetMs  = gewartetMsWert.load (std::memory_order_relaxed);
        return s;
    }

private:
    /// Verlassen - vom Zug. Weckt den Schliesser, wenn er der Letzte war und
    /// die Schleuse bereits geschlossen ist. Keine Sperre.
    void verlassen() noexcept
    {
        const auto vorher = aktivZaehler.fetch_sub (1, std::memory_order_seq_cst);
        if (vorher == 1 && geschlossen.load (std::memory_order_seq_cst))
            warte.notify_all();
    }

    std::atomic<bool>          geschlossen { false };
    std::atomic<std::uint32_t> aktivZaehler { 0 };
    std::atomic<std::uint64_t> betretenZaehler { 0 };
    std::atomic<std::uint64_t> abgewiesenZaehler { 0 };
    std::atomic<std::uint64_t> gewartetMsWert { 0 };

    /// Nur der Schliesspfad. Kein Callback nimmt sie je.
    std::mutex              warteMutex;
    std::condition_variable warte;
};

} // namespace nakama::ipc
