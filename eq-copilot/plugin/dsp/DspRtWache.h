#pragma once

/*  SONDE-015 Etappe 3 - die Echtzeitwache des DSP-Kerns.

    §44.5 und §49.2 Gate 2 verlangen: im Callback keine Sperre, keine
    Allokation, kein IO, kein Logging, keine Ausnahme. Ein Kommentar
    beweist das nicht - ein Zaehler tut es (M-47).

    WIE DIE WACHE EHRLICH BLEIBT: sie zaehlt nicht sich selbst. Der Kern
    setzt beim Betreten des Audiopfads ein THREAD-LOKALES Flag; die
    Zaehlfunktionen erhoehen nur, solange dieses Flag steht. Verdrahtet
    wird die Allokationsseite vom TEST, der `operator new` global
    ueberschreibt und `meldeAllokation()` ruft. Damit misst der Zaehler
    ECHTE Allokationen im echten Audiopfad und ist kein strukturelles Null,
    das nie etwas gesehen hat.

    WARUM THREAD-LOKAL: der Control-Worker DARF allozieren - er baut
    Programme und rechnet 121 Gitterstellen. Ein globaler Zaehler schriebe
    dessen Allokationen dem Audiothread zu und waere entweder falsch-rot
    oder durch eine Toleranz entschaerft, die dann alles durchliesse
    (dieselbe Begruendung wie in B4, `QueueStressTestMain.cpp:17-22`).
*/

#include <atomic>
#include <cstdint>

namespace nakama::dsp
{

class RtWache
{
public:
    /** Betreten und Verlassen des Audiopfads. Der Kern ruft beides selbst;
        `Bereich` unten macht daraus ein Paar, das auch bei einem fruehen
        `return` haelt. */
    static void betrete() noexcept  { tiefe() += 1; }
    static void verlasse() noexcept { if (tiefe() > 0) tiefe() -= 1; }
    static bool imAudiopfad() noexcept { return tiefe() > 0; }

    /** Vom Test aus `operator new` gerufen. Zaehlt nur im Audiopfad. */
    static void meldeAllokation() noexcept
    {
        if (imAudiopfad()) allokationenZaehler().fetch_add (1, std::memory_order_relaxed);
    }

    /** Gerufen von `GemeldeteSperre` - der EINZIGE zulaessige Weg, im Kern
        eine Sperre zu nehmen (B-13). Es gibt im Audiopfad keine; der Zaehler
        macht das messbar, und der Textriegel in B6 haelt jede Sperre
        ausserhalb dieses Wrappers fern. */
    static void meldeSperre() noexcept
    {
        if (imAudiopfad()) sperrenZaehler().fetch_add (1, std::memory_order_relaxed);
    }

    /** Gerufen von `leiteAutoGainAb` (B-18, M-39): die Ableitung zaehlt
        getrennt nach Rechenort. Im Audiopfad bleibt der Zaehler 0; ausserhalb
        steigt er mit jedem gebauten Programm - der Beleg, dass die Ableitung
        wirklich lief und nicht nur nicht im Callback. */
    static void meldeAbleitung() noexcept
    {
        if (imAudiopfad()) ableitungenAudioZaehler().fetch_add (1, std::memory_order_relaxed);
        else               ableitungenWorkerZaehler().fetch_add (1, std::memory_order_relaxed);
    }

    static std::uint64_t ableitungenImAudiopfad() noexcept
    {
        return ableitungenAudioZaehler().load (std::memory_order_relaxed);
    }

    static std::uint64_t ableitungenAusserhalb() noexcept
    {
        return ableitungenWorkerZaehler().load (std::memory_order_relaxed);
    }

    /** Die gemeldete Sperre: ein RAII-Paar ueber einen beliebigen Lockable.
        Bewusst ein Template ohne `<mutex>` - dieser Kopf nennt keine
        Sperrklasse und bleibt damit selbst unter dem Textriegel. */
    template <typename Lockable>
    class GemeldeteSperre
    {
    public:
        explicit GemeldeteSperre (Lockable& l) noexcept (noexcept (l.lock())) : sperre (l)
        {
            RtWache::meldeSperre();
            sperre.lock();
        }
        ~GemeldeteSperre() { sperre.unlock(); }
        GemeldeteSperre (const GemeldeteSperre&) = delete;
        GemeldeteSperre& operator= (const GemeldeteSperre&) = delete;

    private:
        Lockable& sperre;
    };

    static std::uint64_t allokationen() noexcept
    {
        return allokationenZaehler().load (std::memory_order_relaxed);
    }

    static std::uint64_t sperren() noexcept
    {
        return sperrenZaehler().load (std::memory_order_relaxed);
    }

    static void zuruecksetzen() noexcept
    {
        allokationenZaehler().store (0, std::memory_order_relaxed);
        sperrenZaehler().store (0, std::memory_order_relaxed);
        ableitungenAudioZaehler().store (0, std::memory_order_relaxed);
        ableitungenWorkerZaehler().store (0, std::memory_order_relaxed);
    }

    /** RAII-Paar fuer den Audiopfad. */
    struct Bereich
    {
        Bereich() noexcept  { RtWache::betrete(); }
        ~Bereich() noexcept { RtWache::verlasse(); }
        Bereich (const Bereich&) = delete;
        Bereich& operator= (const Bereich&) = delete;
    };

private:
    // Funktionslokale Statics: keine Initialisierungsreihenfolge ueber
    // Uebersetzungseinheiten hinweg, und der Kern ist eine Static-Lib, die
    // in mehrere Ziele geht.
    static int& tiefe() noexcept
    {
        static thread_local int t = 0;
        return t;
    }

    static std::atomic<std::uint64_t>& allokationenZaehler() noexcept
    {
        static std::atomic<std::uint64_t> z { 0 };
        return z;
    }

    static std::atomic<std::uint64_t>& sperrenZaehler() noexcept
    {
        static std::atomic<std::uint64_t> z { 0 };
        return z;
    }

    static std::atomic<std::uint64_t>& ableitungenAudioZaehler() noexcept
    {
        static std::atomic<std::uint64_t> z { 0 };
        return z;
    }

    static std::atomic<std::uint64_t>& ableitungenWorkerZaehler() noexcept
    {
        static std::atomic<std::uint64_t> z { 0 };
        return z;
    }
};

} // namespace nakama::dsp
