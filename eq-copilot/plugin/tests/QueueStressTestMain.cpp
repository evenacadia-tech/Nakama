// SONDE-008 (S10-11) — Ganzblock-Queue, Ein-Block-Quarantaene, Blockgroessen-
// Stress (Kanon B4, Entwurf §66.1: "Ganzblockaufnahme, Drop, Quarantaene, Wrap
// und Oversize").
//
// ZWEI EBENEN IN EINEM ZIEL, dieselbe Form wie EqCopLebenslaufTest:
//   TEIL 1 — Queue und Quarantaene PUR (core/StampedAudioQueue.h). Hier lassen
//            sich Ueberlauf, Wrap und Oversize deterministisch herbeifuehren;
//            im laufenden Plugin waeren sie Glueckssache.
//   TEIL 2 — DERSELBE Weg verdrahtet im echten `EqCopilotProcessor`. Ein Ring,
//            der stimmt, aber nirgends haengt, verriegelt nichts. Teil 2 misst
//            deshalb Audio (bitgleicher Passthrough), Allokationsfreiheit des
//            Audiothreads und den Blockgroessen-Stress ueber
//            `maximumExpectedSamplesPerBlock` hinaus - FL zerteilt Puffer bis
//            auf 1 Sample (Capabilityreport S4), und der Prueferauftrag zu
//            diesem Ticket verlangt ausdruecklich einen Blockgroessen-Fuzz.
//
// ALLOKATIONSZAEHLER: thread_local, und das ist kein Detail. Der Prozessor
// haelt einen Workerthread, der sehr wohl allozieren DARF (er rechnet die
// Messung). Ein globaler Zaehler wuerde dessen Allokationen dem Audiothread
// zuschreiben und waere entweder falsch-rot oder - schlimmer - durch eine
// Toleranz entschaerft, die dann alles durchliesse.
//
// Exit 0 nur bei "QUEUE-STRESSTEST OK".
#include "PluginProcessor.h"
#include "StampedAudioQueue.h"
#include "WorkerCadence.h"

#include <atomic>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <string>
#include <thread>
#include <vector>

using namespace eqcop;
using namespace nakama::echtzeit;

//==============================================================================
namespace
{
    thread_local bool          zaehleAllokationen = false;
    thread_local std::uint64_t allokationen       = 0;
}

void* operator new (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAllokationen) ++allokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

//==============================================================================
namespace
{
    int fehler = 0, geprueft = 0;

    void pruefe (bool ok, const std::string& was, const std::string& detail = {})
    {
        ++geprueft;
        std::cout << (ok ? "  ok      " : "  FEHLER  ") << was;
        if (! detail.empty()) std::cout << "  [" << detail << "]";
        std::cout << std::endl;
        if (! ok) ++fehler;
    }

    /** Kleines Testlayout: absichtlich WINZIG, damit Ueberlauf und Wrap in
        Millisekunden erreichbar sind statt in Minuten. Dass die Klasse ein
        Template ist, ist genau dafuer da - der Produktionsstrom `GenStrom`
        hat dieselbe Implementierung, nur andere Zahlen. */
    using MiniStrom = StreamLayout</*taps*/ 1, /*budgetFrames*/ 1024,
                                   /*deskriptorPlaetze*/ 8, /*maxBlockFrames*/ 256>;
    /** Zweiter Tap-Satz: beweist, dass `taps` wirklich ein Parameter ist und
        keine als Template getarnte Einzweckklasse. */
    using DreiTapStrom = StreamLayout<3, 512, 8, 128>;

    /** Deterministisches Muster: jeder Frame traegt seine globale Nummer,
        L und R unterscheidbar. Damit faellt jede Verschiebung um auch nur
        einen Frame auf. */
    float musterL (std::uint64_t frame) { return (float) ((double) frame * 1.0e-3); }
    float musterR (std::uint64_t frame) { return (float) (-(double) frame * 1.0e-3 - 0.5); }

    std::uint32_t lcg = 0x13579bdfu;
    std::uint32_t zufall (std::uint32_t obenExkl)
    {
        lcg = lcg * 1664525u + 1013904223u;
        return (lcg >> 8) % obenExkl;
    }

    Stempel stempelBei (std::int64_t zeit, bool spielt = true, bool zeitGueltig = true)
    {
        Stempel s;
        s.kontextAnwesend = true;
        s.zeitGueltig     = zeitGueltig;
        s.projectSampleStart = zeit;
        s.spieltGueltig   = true;
        s.spielt          = spielt;
        return s;
    }

    struct TestPlayHead : juce::AudioPlayHead
    {
        bool spielt = true;
        juce::int64 pos = 0;
        juce::Optional<PositionInfo> getPosition() const override
        {
            PositionInfo p;
            p.setIsPlaying (spielt);
            p.setTimeInSamples (pos);
            return p;
        }
    };

    /** Testbarriere fuer das Reset-Race: `getPosition()` liegt im echten
        Prozesspfad vor der Queue-Publikation und Fensterfortschreibung. Der
        Steuerthread kann dort deterministisch eine neue Generation anfordern
        und danach genau diesen Callback freigeben. */
    struct BlockierenderPlayHead : juce::AudioPlayHead
    {
        void setzePosition (juce::int64 neuePosition)
        {
            std::lock_guard<std::mutex> l (mutex);
            position = neuePosition;
        }

        void blockiereNaechstenAbruf (juce::int64 neuePosition)
        {
            std::lock_guard<std::mutex> l (mutex);
            position = neuePosition;
            sollBlockieren = true;
            eingetreten = false;
            darfWeiter = false;
        }

        bool warteBisEingetreten()
        {
            std::unique_lock<std::mutex> l (mutex);
            return zustand.wait_for (l, std::chrono::seconds (2),
                                     [this] { return eingetreten; });
        }

        void freigeben()
        {
            {
                std::lock_guard<std::mutex> l (mutex);
                darfWeiter = true;
            }
            zustand.notify_all();
        }

        juce::Optional<PositionInfo> getPosition() const override
        {
            std::unique_lock<std::mutex> l (mutex);
            if (sollBlockieren)
            {
                eingetreten = true;
                zustand.notify_all();
                zustand.wait (l, [this] { return darfWeiter; });
                sollBlockieren = false;
            }
            const auto aktuell = position;
            l.unlock();

            PositionInfo p;
            p.setIsPlaying (true);
            p.setTimeInSamples (aktuell);
            return p;
        }

        mutable std::mutex mutex;
        mutable std::condition_variable zustand;
        mutable bool sollBlockieren = false;
        mutable bool eingetreten = false;
        mutable bool darfWeiter = false;
        mutable juce::int64 position = 0;
    };

    /** Wie EqCopMarkierungTest: den echten Weg des Users gehen (Editor auf,
        Rolle „hub"), nicht den Testschalter. */
    bool alsMainKlassifizieren (EqCopilotProcessor& p)
    {
        p.setzeEditorOffen (true);
        p.setzeBindung ("hub", {}, {});
        return p.holeKlassifikation() == nakama::state::Klassifikation::main;
    }
}

//==============================================================================
int main()
{
    juce::ScopedJuceInitialiser_GUI juceInit;
    std::cout << "== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==" << std::endl;
    std::cout << "Produktionsstrom GenStrom: " << GenStrom::taps << " Tap(s), "
              << GenStrom::budgetFrames << " Frames Ring, "
              << GenStrom::deskriptorPlaetze << " Deskriptoren, max "
              << GenStrom::maxBlockFrames << " Frames je Block." << std::endl;
    std::cout << std::endl;

    //==========================================================================
    std::cout << "== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==" << std::endl;
    {
        StampedAudioQueue<MiniStrom> q;
        q.vorbereiten();
        std::vector<float> l (256), r (256), aus (512);

        std::uint64_t global = 0;
        bool inhaltOk = true, stempelOk = true;
        for (int block = 0; block < 3; ++block)
        {
            const int n = 100 + block * 20;
            for (int i = 0; i < n; ++i) { l[(std::size_t) i] = musterL (global + (std::uint64_t) i);
                                          r[(std::size_t) i] = musterR (global + (std::uint64_t) i); }
            StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), r.data() };
            if (! q.veroeffentliche (&t, 1, 2, n, stempelBei ((std::int64_t) global)))
                inhaltOk = false;
            global += (std::uint64_t) n;
        }

        std::uint64_t erwartet = 0;
        int gelesen = 0;
        while (const auto* b = q.spitze())
        {
            q.lies (*b, 0, aus.data());
            for (std::uint32_t i = 0; i < b->sampleCount; ++i)
            {
                if (aus[(std::size_t) i * 2]     != musterL (erwartet + i)) inhaltOk = false;
                if (aus[(std::size_t) i * 2 + 1] != musterR (erwartet + i)) inhaltOk = false;
            }
            if (b->stromVon != erwartet) stempelOk = false;
            if (b->projectSampleStart != (std::int64_t) erwartet) stempelOk = false;
            if ((b->flags & kFlagLueckeDavor) != 0 && gelesen > 0) stempelOk = false;
            erwartet += b->sampleCount;
            ++gelesen;
            q.freigeben();
        }
        pruefe (gelesen == 3, "drei Bloecke, drei Deskriptoren", std::to_string (gelesen));
        pruefe (inhaltOk, "jeder Frame kommt bitgleich und an derselben Stelle heraus");
        pruefe (stempelOk, "Stempel: lokaler Strom und Projektzeit stimmen je Block");
        pruefe (q.spitze() == nullptr, "nach dem Leeren ist die Queue leer");
        pruefe (q.dropsGesamt() == 0 && q.verloreneFrames() == 0, "kein Verlust bei Normallast");
    }

    //==========================================================================
    std::cout << "== B - Wrap: der Ring laeuft mehrfach um ==" << std::endl;
    {
        StampedAudioQueue<MiniStrom> q;
        q.vorbereiten();
        std::vector<float> l (200), r (200), aus (400);
        std::uint64_t schreibGlobal = 0, leseGlobal = 0;
        bool ok = true;
        int umlaeufe = 0;

        // 1024 Frames Ring, Bloecke zu 200 -> nach ~5 Bloecken der erste Wrap.
        for (int block = 0; block < 60; ++block)
        {
            const int n = 200;
            for (int i = 0; i < n; ++i) { l[(std::size_t) i] = musterL (schreibGlobal + (std::uint64_t) i);
                                          r[(std::size_t) i] = musterR (schreibGlobal + (std::uint64_t) i); }
            StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), r.data() };
            if (! q.veroeffentliche (&t, 1, 2, n, stempelBei ((std::int64_t) schreibGlobal)))
                ok = false;
            const std::uint64_t vorher = schreibGlobal / MiniStrom::budgetFrames;
            schreibGlobal += (std::uint64_t) n;
            if (schreibGlobal / MiniStrom::budgetFrames != vorher) ++umlaeufe;

            // Sofort wieder leeren - so bleibt Platz und der Ring wandert.
            while (const auto* b = q.spitze())
            {
                q.lies (*b, 0, aus.data());
                for (std::uint32_t i = 0; i < b->sampleCount; ++i)
                {
                    if (aus[(std::size_t) i * 2]     != musterL (leseGlobal + i)) ok = false;
                    if (aus[(std::size_t) i * 2 + 1] != musterR (leseGlobal + i)) ok = false;
                }
                leseGlobal += b->sampleCount;
                q.freigeben();
            }
        }
        pruefe (umlaeufe >= 10, "der Ring ist wirklich mehrfach umgelaufen",
                std::to_string (umlaeufe) + " Umlaeufe");
        pruefe (ok, "ueber jeden Wrap hinweg bitgleicher Inhalt");
        pruefe (leseGlobal == schreibGlobal, "kein Frame verloren, keiner doppelt",
                std::to_string (leseGlobal) + "/" + std::to_string (schreibGlobal));
    }

    //==========================================================================
    std::cout << "== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==" << std::endl;
    {
        StampedAudioQueue<MiniStrom> q;
        q.vorbereiten();
        std::vector<float> l (256, 1.0f), r (256, 2.0f), aus (512);
        StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), r.data() };

        int angenommen = 0;
        std::int64_t zeit = 0;
        bool lueckeBeiAbweisung = true;
        // 1024 Frames Ring / 256 je Block => vier passen, der fuenfte nicht.
        for (int i = 0; i < 6; ++i)
        {
            const bool an = q.veroeffentliche (
                &t, 1, 2, 256, stempelBei (zeit), nullptr,
                i == 4 ? &lueckeBeiAbweisung : nullptr);
            if (an) ++angenommen;
            zeit += 256;
        }
        pruefe (angenommen == 4, "vier Bloecke passen, danach ist Schluss",
                std::to_string (angenommen));
        pruefe (q.dropsUeberlauf() == 2, "beide abgewiesenen Bloecke sind gezaehlt",
                std::to_string (q.dropsUeberlauf()));
        pruefe (q.verloreneFrames() == 512, "verlorene Frames = 2 x 256 (keine Teilmenge)",
                std::to_string (q.verloreneFrames()));
        pruefe (! lueckeBeiAbweisung,
                "ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst");

        // Nichts Halbes im Ring: genau vier Deskriptoren, jeder voll.
        int drin = 0; bool vollstaendig = true;
        while (const auto* b = q.spitze())
        {
            if (b->sampleCount != 256) vollstaendig = false;
            ++drin;
            q.freigeben();
        }
        pruefe (drin == 4 && vollstaendig, "im Ring liegen NUR vollstaendige Bloecke",
                std::to_string (drin));

        // Gegenpfad fuellen<->leeren: nach dem Leeren nimmt sie wieder an, und
        // der erste neue Block traegt die Luecke samt neuem Segment.
        bool lueckeVorBlock = false;
        const bool wiederAn = q.veroeffentliche (
            &t, 1, 2, 256, stempelBei (zeit), nullptr, &lueckeVorBlock);
        const auto* b = q.spitze();
        pruefe (wiederAn && b != nullptr, "nach dem Leeren nimmt der Ring wieder an");
        pruefe (lueckeVorBlock,
                "die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock");
        pruefe (b != nullptr && (b->flags & kFlagLueckeDavor) != 0,
                "der erste Block nach dem Verlust traegt kFlagLueckeDavor");
        pruefe (b != nullptr && b->segment == 1,
                "und beginnt ein neues continuity_segment",
                b != nullptr ? std::to_string (b->segment) : "-");
    }

    //==========================================================================
    std::cout << "== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==" << std::endl;
    {
        // 8 Deskriptoren, 1024 Frames: mit Ein-Frame-Bloecken geht der
        // Deskriptorring zuerst aus. Ohne die zweite Pruefung im Produzenten
        // liefe dieser Fall in einen ueberschriebenen Deskriptor.
        StampedAudioQueue<MiniStrom> q;
        q.vorbereiten();
        float l = 0.25f, r = -0.25f;
        StampedAudioQueue<MiniStrom>::TapQuelle t { &l, &r };
        int angenommen = 0;
        for (int i = 0; i < 12; ++i)
            if (q.veroeffentliche (&t, 1, 2, 1, stempelBei (i))) ++angenommen;

        pruefe (angenommen == MiniStrom::deskriptorPlaetze,
                "genau `deskriptorPlaetze` Bloecke passen", std::to_string (angenommen));
        pruefe (q.dropsUeberlauf() == 4, "die vier ueberzaehligen sind gezaehlt",
                std::to_string (q.dropsUeberlauf()));
        int drin = 0;
        while (q.spitze()) { ++drin; q.freigeben(); }
        pruefe (drin == MiniStrom::deskriptorPlaetze,
                "und genau so viele stehen zum Lesen bereit", std::to_string (drin));
    }

    //==========================================================================
    std::cout << "== E - Oversize: ein Block ueber der Slotkapazitaet ==" << std::endl;
    {
        StampedAudioQueue<MiniStrom> q;
        q.vorbereiten();
        std::vector<float> gross (MiniStrom::maxBlockFrames + 1, 0.5f);
        StampedAudioQueue<MiniStrom>::TapQuelle t { gross.data(), gross.data() };

        const bool an = q.veroeffentliche (&t, 1, 2, MiniStrom::maxBlockFrames + 1, stempelBei (0));
        pruefe (! an, "ein Block ueber maxBlockFrames wird als GANZES abgewiesen");
        pruefe (q.dropsOversize() == 1, "und als Oversize gezaehlt, nicht als Ueberlauf",
                std::to_string (q.dropsOversize()) + "/" + std::to_string (q.dropsUeberlauf()));
        pruefe (q.dropsUeberlauf() == 0, "der Ueberlaufzaehler bleibt sauber");
        pruefe (q.spitze() == nullptr, "nichts Halbes im Ring");
        pruefe (q.verloreneFrames() == (std::uint64_t) MiniStrom::maxBlockFrames + 1,
                "die verlorene Zeit ist vollstaendig gemeldet",
                std::to_string (q.verloreneFrames()));

        // Genau auf der Kante darf er noch.
        const bool kante = q.veroeffentliche (&t, 1, 2, MiniStrom::maxBlockFrames, stempelBei (0));
        pruefe (kante, "genau maxBlockFrames passt noch (Kante, nicht Ausschluss)");
    }

    //==========================================================================
    std::cout << "== F - Flush (numSamples == 0) ist kein Verlust ==" << std::endl;
    {
        StampedAudioQueue<MiniStrom> q;
        q.vorbereiten();
        float x = 0.0f;
        StampedAudioQueue<MiniStrom>::TapQuelle t { &x, &x };
        pruefe (! q.veroeffentliche (&t, 1, 2, 0, stempelBei (0)),
                "ein Flush-Block wird nicht eingestellt");
        pruefe (q.dropsGesamt() == 0 && q.verloreneFrames() == 0,
                "und zaehlt NICHT als Verlust - er traegt keine Audiozeit");
        pruefe (q.veroeffentliche (&t, 1, 2, 1, stempelBei (0)), "der Folgeblock geht normal");
        const auto* b = q.spitze();
        pruefe (b != nullptr && (b->flags & kFlagLueckeDavor) == 0,
                "und traegt keine Luecke");
    }

    //==========================================================================
    std::cout << "== G - Mono und Mehrfach-Tap ==" << std::endl;
    {
        StampedAudioQueue<MiniStrom> q;
        q.vorbereiten();
        std::vector<float> l (64), aus (128);
        for (int i = 0; i < 64; ++i) l[(std::size_t) i] = musterL ((std::uint64_t) i);
        StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), nullptr };
        q.veroeffentliche (&t, 1, 1, 64, stempelBei (0));
        const auto* b = q.spitze();
        bool dupliziert = (b != nullptr);
        if (b != nullptr)
        {
            q.lies (*b, 0, aus.data());
            for (int i = 0; i < 64; ++i)
                if (aus[(std::size_t) i * 2] != l[(std::size_t) i]
                    || aus[(std::size_t) i * 2 + 1] != l[(std::size_t) i])
                    dupliziert = false;
        }
        pruefe (dupliziert, "Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)");
        pruefe (b != nullptr && b->kanaele == 1, "und der Deskriptor sagt kanaele == 1");
    }
    {
        StampedAudioQueue<DreiTapStrom> q;
        q.vorbereiten();
        std::vector<float> a (32, 0.1f), b (32, 0.2f), c (32, 0.3f), aus (64);
        StampedAudioQueue<DreiTapStrom>::TapQuelle t[3] {
            { a.data(), a.data() }, { b.data(), b.data() }, { nullptr, nullptr } };
        const bool an = q.veroeffentliche (t, 3, 2, 32, stempelBei (0));
        const auto* d = q.spitze();
        pruefe (an && d != nullptr, "Drei-Tap-Layout: Block angenommen");
        pruefe (d != nullptr && d->tapMaske == 0b011u,
                "das Bitset meldet genau die belegten Taps",
                d != nullptr ? std::to_string (d->tapMaske) : "-");
        bool inhalt = (d != nullptr);
        if (d != nullptr)
        {
            q.lies (*d, 0, aus.data()); if (aus[0] != 0.1f) inhalt = false;
            q.lies (*d, 1, aus.data()); if (aus[0] != 0.2f) inhalt = false;
            if (q.lies (*d, 2, aus.data())) inhalt = false;   // inaktiv => false
        }
        pruefe (inhalt, "jeder Tap liegt getrennt und richtig; der inaktive liefert false");
        pruefe (q.veroeffentliche (t, 2, 2, 32, stempelBei (0)) == false,
                "eine falsche Tapzahl wird abgewiesen statt halb kopiert");
        // Kein einziger aktiver Tap: Verlust, nicht leerer Deskriptor.
        StampedAudioQueue<DreiTapStrom>::TapQuelle leer[3] {};
        const auto vorher = q.dropsGesamt();
        pruefe (! q.veroeffentliche (leer, 3, 2, 32, stempelBei (0))
                    && q.dropsGesamt() == vorher + 1,
                "ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor");
    }

    //==========================================================================
    std::cout << "== H - Ein-Block-Quarantaene ==" << std::endl;
    {
        StampedAudioQueue<MiniStrom> q; q.vorbereiten();
        Blockquarantaene<MiniStrom> quar; quar.vorbereiten();
        std::vector<float> l (64), r (64);
        for (int i = 0; i < 64; ++i) { l[(std::size_t) i] = musterL ((std::uint64_t) i);
                                       r[(std::size_t) i] = musterR ((std::uint64_t) i); }
        StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), r.data() };

        // Lueckenloser Strom: jeder Block wird vom naechsten versiegelt, der
        // JUENGSTE bleibt zurueck.
        int versiegelt = 0;
        std::int64_t zeit = 0;
        for (int i = 0; i < 5; ++i)
        {
            q.veroeffentliche (&t, 1, 2, 64, stempelBei (zeit)); zeit += 64;
            while (const auto* b = q.spitze())
            {
                const auto frei = quar.schiebe (q, *b);
                q.freigeben();
                if (frei) ++versiegelt;
            }
        }
        pruefe (versiegelt == 4, "5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene",
                std::to_string (versiegelt));
        pruefe (quar.haeltBlock(), "und die Quarantaene haelt ihn wirklich");
        pruefe (quar.kontinuitaetsbrueche() == 0, "kein Bruch bei lueckenlosem Strom");

        // Ein Zeitsprung (Seek): der gehaltene Block faellt, der Strom laeuft.
        const std::uint64_t vorher = quar.verworfeneBloecke();
        q.veroeffentliche (&t, 1, 2, 64, stempelBei (zeit + 100000));    // Seek
        while (const auto* b = q.spitze()) { quar.schiebe (q, *b); q.freigeben(); }
        pruefe (quar.kontinuitaetsbrueche() == 1, "der Seek ist als Bruch erkannt",
                std::to_string (quar.kontinuitaetsbrueche()));
        pruefe (quar.verworfeneBloecke() == vorher + 1,
                "genau EIN Block faellt an der Grenze - der Strom laeuft weiter");
        pruefe (quar.haeltBlock(), "der Block NACH der Grenze ist schon wieder in Quarantaene");

        // Stopp: was gehalten wird, bleibt unvollstaendig (§32.3).
        const std::uint64_t v2 = quar.verworfeneBloecke();
        quar.zuruecksetzen();
        pruefe (quar.verworfeneBloecke() == v2 + 1 && ! quar.haeltBlock(),
                "Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)");
    }
    {
        // Stehende Projektzeit ist KEIN Bruch (FL zerteilt Puffer bis 1 Sample).
        StampedAudioQueue<MiniStrom> q; q.vorbereiten();
        Blockquarantaene<MiniStrom> quar; quar.vorbereiten();
        std::vector<float> l (32, 0.1f);
        StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), l.data() };
        for (int i = 0; i < 6; ++i)
        {
            q.veroeffentliche (&t, 1, 2, 32, stempelBei (4711));   // IMMER dieselbe Zeit
            while (const auto* b = q.spitze()) { quar.schiebe (q, *b); q.freigeben(); }
        }
        pruefe (quar.kontinuitaetsbrueche() == 0,
                "stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch",
                std::to_string (quar.kontinuitaetsbrueche()));
        pruefe (quar.versiegelteBloecke() == 5, "und alle bis auf den juengsten sind versiegelt",
                std::to_string (quar.versiegelteBloecke()));
    }
    {
        // Transportkante und Queue-Luecke sind Brueche.
        StampedAudioQueue<MiniStrom> q; q.vorbereiten();
        Blockquarantaene<MiniStrom> quar; quar.vorbereiten();
        std::vector<float> l (32, 0.1f);
        StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), l.data() };
        std::int64_t zeit = 0;
        auto zug = [&] (const Stempel& s)
        {
            q.veroeffentliche (&t, 1, 2, 32, s);
            while (const auto* b = q.spitze()) { quar.schiebe (q, *b); q.freigeben(); }
        };
        zug (stempelBei (zeit)); zeit += 32;
        zug (stempelBei (zeit)); zeit += 32;
        pruefe (quar.kontinuitaetsbrueche() == 0, "Vorlauf ohne Bruch");
        zug (stempelBei (zeit, /*spielt*/ false)); zeit += 32;
        pruefe (quar.kontinuitaetsbrueche() == 1, "Transportkante play->stop ist ein Bruch",
                std::to_string (quar.kontinuitaetsbrueche()));
        // Ohne Zeitbeweis: nur der lokale Strom zaehlt, kein Bruch.
        StampedAudioQueue<MiniStrom> q2; q2.vorbereiten();
        Blockquarantaene<MiniStrom> quar2; quar2.vorbereiten();
        for (int i = 0; i < 4; ++i)
        {
            Stempel s;   // gar nichts bewiesen: kein Kontext, keine Zeit, kein Transport
            q2.veroeffentliche (&t, 1, 2, 32, s);
            while (const auto* b = q2.spitze()) { quar2.schiebe (q2, *b); q2.freigeben(); }
        }
        pruefe (quar2.kontinuitaetsbrueche() == 0,
                "ohne Hostkontext traegt der lokale monotone Strom (§53.6)");
        pruefe (quar2.versiegelteBloecke() == 3, "und versiegelt normal",
                std::to_string (quar2.versiegelteBloecke()));
    }
    {
        // Numerischer Rand: Projektzeit nahe INT64_MAX und negativ.
        StampedAudioQueue<MiniStrom> q; q.vorbereiten();
        Blockquarantaene<MiniStrom> quar; quar.vorbereiten();
        std::vector<float> l (32, 0.1f);
        StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), l.data() };
        // Der gehaltene Block liegt so nah am Rand, dass `start + sampleCount`
        // ueberliefe. Der Nachfolger traegt bewusst eine ANDERE Zeit - bei
        // gleicher Zeit greift die Teilpuffer-Regel und die Rechnung faende gar
        // nicht statt.
        const std::int64_t nahAmRand = std::numeric_limits<std::int64_t>::max() - 16;
        q.veroeffentliche (&t, 1, 2, 32, stempelBei (nahAmRand));
        q.veroeffentliche (&t, 1, 2, 32, stempelBei (nahAmRand - 8));
        while (const auto* b = q.spitze()) { quar.schiebe (q, *b); q.freigeben(); }
        pruefe (quar.kontinuitaetsbrueche() == 1,
                "Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung");

        StampedAudioQueue<MiniStrom> qn; qn.vorbereiten();
        Blockquarantaene<MiniStrom> quarn; quarn.vorbereiten();
        for (int i = 0; i < 4; ++i)
        {
            qn.veroeffentliche (&t, 1, 2, 32, stempelBei (-1000 + i * 32));
            while (const auto* b = qn.spitze()) { quarn.schiebe (qn, *b); qn.freigeben(); }
        }
        pruefe (quarn.kontinuitaetsbrueche() == 0,
                "negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)");
    }

    //==========================================================================
    std::cout << "== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==" << std::endl;
    {
        StampedAudioQueue<MiniStrom> q; q.vorbereiten();
        Blockquarantaene<MiniStrom> quar; quar.vorbereiten();
        std::vector<float> l (MiniStrom::maxBlockFrames + 64), r (l.size()), aus (l.size() * 2);
        std::uint64_t schreib = 0, geprueftFrames = 0;
        bool inhaltOk = true, teilblock = false;
        int oversize = 0;

        for (int i = 0; i < 20000; ++i)
        {
            const int n = 1 + (int) zufall (MiniStrom::maxBlockFrames + 32);
            for (int k = 0; k < n; ++k)
            {
                l[(std::size_t) k] = musterL (schreib + (std::uint64_t) k);
                r[(std::size_t) k] = musterR (schreib + (std::uint64_t) k);
            }
            StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), r.data() };
            const bool an = q.veroeffentliche (&t, 1, 2, n, stempelBei ((std::int64_t) schreib));
            if (! an && n > MiniStrom::maxBlockFrames) ++oversize;
            schreib += (std::uint64_t) n;

            while (const auto* b = q.spitze())
            {
                if (b->sampleCount == 0) teilblock = true;
                q.lies (*b, 0, aus.data());
                for (std::uint32_t k = 0; k < b->sampleCount; ++k)
                    if (aus[(std::size_t) k * 2] != musterL (b->stromVon + k)) inhaltOk = false;
                geprueftFrames += b->sampleCount;
                const auto frei = quar.schiebe (q, *b);
                q.freigeben();
                (void) frei;
            }
        }
        std::cout << "  20 000 Bloecke, Groessen 1.." << (MiniStrom::maxBlockFrames + 32)
                  << ": angenommen " << q.bloeckeAngenommen()
                  << ", Ueberlauf-Drops " << q.dropsUeberlauf()
                  << ", Oversize-Drops " << q.dropsOversize()
                  << ", groesster Block " << q.groessterBlock()
                  << ", versiegelt " << quar.versiegelteBloecke()
                  << ", Brueche " << quar.kontinuitaetsbrueche() << std::endl;
        pruefe (inhaltOk, "jeder gelesene Frame steht an der Stelle, die sein Stempel nennt");
        pruefe (! teilblock, "es gab nie einen Block mit sampleCount 0");
        pruefe (oversize > 0 && q.dropsOversize() == (std::uint64_t) oversize,
                "die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt",
                std::to_string (oversize));
        pruefe (q.groessterBlock() <= (std::uint32_t) MiniStrom::maxBlockFrames,
                "kein angenommener Block war je groesser als die Slotkapazitaet",
                std::to_string (q.groessterBlock()));
        pruefe (geprueftFrames > 0, "es wurde wirklich gelesen",
                std::to_string (geprueftFrames) + " Frames");
    }

    //==========================================================================
    std::cout << "== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==" << std::endl;
    {
        constexpr double fs = 48000.0;
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, 512);
        p.prepareToPlay (fs, 512);            // Hinweis: 512
        TestPlayHead kopf; kopf.spielt = true;
        p.setPlayHead (&kopf);

        // Absichtlich WEIT ueber den Hinweis hinaus - genau das ist der Fall,
        // den §48.1 "nur ein Hinweis" nennt.
        const int groessen[] = { 1, 2, 3, 7, 15, 16, 31, 64, 127, 128, 333, 512,
                                 1024, 2048, 4096, 8192, 12345, 16384 };
        juce::AudioBuffer<float> puffer (2, 20000), kopie (2, 20000);
        juce::MidiBuffer midi;
        bool bitgleich = true;
        std::uint32_t muster = 1u;
        for (const int n : groessen)
        {
            for (int k = 0; k < 2; ++k)
            {
                float* d = puffer.getWritePointer (k);
                for (int i = 0; i < n; ++i)
                {
                    muster = muster * 1664525u + 1013904223u;
                    d[i] = (float) ((double) (muster >> 8) / (double) 0x00ffffff) * 2.0f - 1.0f;
                }
                std::memcpy (kopie.getWritePointer (k), d, (std::size_t) n * sizeof (float));
            }
            juce::AudioBuffer<float> sicht (puffer.getArrayOfWritePointers(), 2, n);
            p.processBlock (sicht, midi);
            kopf.pos += n;
            for (int k = 0; k < 2; ++k)
                if (std::memcmp (puffer.getReadPointer (k), kopie.getReadPointer (k),
                                 (std::size_t) n * sizeof (float)) != 0)
                    bitgleich = false;
        }
        pruefe (bitgleich, "18 Blockgroessen von 1 bis 16 384: kein Sample veraendert");
        pruefe (p.getLatencySamples() == 0, "0 Samples Latenz",
                std::to_string (p.getLatencySamples()));
        pruefe (p.getTailLengthSeconds() == 0.0, "kein Tail");
        std::cout << "  angenommen " << p.analyseBloeckeAngenommen()
                  << ", groesster Block " << p.analyseGroessterBlock()
                  << ", Ueberlauf " << p.analyseDropsUeberlauf()
                  << ", Oversize " << p.analyseDropsOversize() << std::endl;
        p.setPlayHead (nullptr);
    }

    //==========================================================================
    std::cout << "== K - verdrahtet: Oversize aendert das Audio nicht ==" << std::endl;
    {
        constexpr double fs = 48000.0;
        const int n = EqCopilotProcessor::analyseMaxBlockFrames() + 1;
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, 512);
        p.prepareToPlay (fs, 512);
        TestPlayHead kopf; kopf.spielt = true;
        p.setPlayHead (&kopf);

        juce::AudioBuffer<float> puffer (2, n), kopie (2, n);
        juce::MidiBuffer midi;
        for (int k = 0; k < 2; ++k)
        {
            float* d = puffer.getWritePointer (k);
            for (int i = 0; i < n; ++i) d[i] = std::sin ((float) i * 0.01f) * 0.5f;
            std::memcpy (kopie.getWritePointer (k), d, (std::size_t) n * sizeof (float));
        }
        const auto vorher = p.analyseDropsOversize();
        p.processBlock (puffer, midi);
        bool gleich = true;
        for (int k = 0; k < 2; ++k)
            if (std::memcmp (puffer.getReadPointer (k), kopie.getReadPointer (k),
                             (std::size_t) n * sizeof (float)) != 0)
                gleich = false;
        pruefe (gleich, "ein Block ueber der Slotkapazitaet laeuft unveraendert durch");
        pruefe (p.analyseDropsOversize() == vorher + 1,
                "und ist als Oversize-Drop der ANALYSE gezaehlt",
                std::to_string (p.analyseDropsOversize()));

        const auto nachDrop = p.messKompakt();
        pruefe (! nachDrop.fensterGueltig,
                "der abgewiesene Oversize-Block erscheint nicht im Projektfenster");

        kopf.pos += n;
        constexpr int folgeFrames = 64;
        juce::AudioBuffer<float> folge (puffer.getArrayOfWritePointers(), 2, folgeFrames);
        p.processBlock (folge, midi);
        const auto nachLuecke = p.messKompakt();
        pruefe (nachLuecke.fensterGueltig
                    && nachLuecke.fensterVon == n
                    && nachLuecke.fensterBis == n + folgeFrames,
                "der naechste angenommene Lueckenblock beginnt das Fenster exakt neu",
                std::to_string (nachLuecke.fensterVon) + ".."
                    + std::to_string (nachLuecke.fensterBis));
        pruefe (nachLuecke.fensterSpruenge == 0,
                "der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben",
                std::to_string (nachLuecke.fensterSpruenge));
        p.setPlayHead (nullptr);
    }

    //==========================================================================
    std::cout << "== L - verdrahtet: der Audiothread alloziert nicht ==" << std::endl;
    {
        constexpr double fs = 48000.0;
        constexpr int bs = 512;
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        TestPlayHead kopf; kopf.spielt = true;
        p.setPlayHead (&kopf);
        pruefe (alsMainKlassifizieren (p), "als Main klassifiziert (§53.5)");

        juce::AudioBuffer<float> puffer (2, 4096);
        juce::MidiBuffer midi;
        for (int k = 0; k < 2; ++k)
        {
            float* d = puffer.getWritePointer (k);
            for (int i = 0; i < 4096; ++i) d[i] = 0.3f * std::sin ((float) i * 0.02f);
        }

        // Wechselnde Groessen, Transportkanten und Ueberlauf im selben Lauf -
        // der Audiothread darf in KEINEM davon allozieren.
        allokationen = 0;
        zaehleAllokationen = true;
        for (int i = 0; i < 4000; ++i)
        {
            const int n = 1 + (int) zufall (4096);
            juce::AudioBuffer<float> sicht (puffer.getArrayOfWritePointers(), 2, n);
            if ((i % 500) == 0) kopf.spielt = ! kopf.spielt;
            p.processBlock (sicht, midi);
            kopf.pos += n;
        }
        zaehleAllokationen = false;
        pruefe (allokationen == 0,
                "4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen",
                std::to_string (allokationen));
        std::cout << "  dabei Ueberlauf-Drops " << p.analyseDropsUeberlauf()
                  << " (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - "
                  << "genau dafuer ist der Ganzblockdrop da)" << std::endl;
        p.setPlayHead (nullptr);
    }

    //==========================================================================
    std::cout << "== M - verdrahtet: die Messung bekommt den Strom wirklich ==" << std::endl;
    {
        constexpr double fs = 48000.0;
        constexpr int bs = 512;
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        TestPlayHead kopf; kopf.spielt = true;
        p.setPlayHead (&kopf);

        juce::AudioBuffer<float> puffer (2, bs);
        juce::MidiBuffer midi;
        const int bloecke = (int) (6.0 * fs / bs);      // ~6 s
        double phase = 0.0;
        for (int b = 0; b < bloecke; ++b)
        {
            for (int i = 0; i < bs; ++i)
            {
                const float v = 0.4f * (float) std::sin (phase);
                phase += 2.0 * juce::MathConstants<double>::pi * 440.0 / fs;
                puffer.setSample (0, i, v);
                puffer.setSample (1, i, v);
            }
            p.processBlock (puffer, midi);
            kopf.pos += bs;
            std::this_thread::sleep_for (std::chrono::milliseconds (2));
        }
        // Worker ausrechnen lassen.
        MessSnapshot m;
        for (int versuch = 0; versuch < 100; ++versuch)
        {
            m = p.messSnapshot();
            // Warten, bis alles bis auf den EINEN Block in Quarantaene da ist -
            // genau die Zahl, die die Pruefung danach behauptet.
            if (m.verarbeiteteSamples >= (juce::uint64) ((bloecke - 1) * bs)) break;
            std::this_thread::sleep_for (std::chrono::milliseconds (50));
        }
        const juce::uint64 eingespeist = (juce::uint64) bloecke * (juce::uint64) bs;
        std::cout << "  eingespeist " << eingespeist
                  << ", analysiert " << m.verarbeiteteSamples
                  << ", Drops " << p.analyseDropsUeberlauf()
                  << ", versiegelt " << p.analyseVersiegelt()
                  << ", Quarantaene-Verwuerfe " << p.analyseQuarantaeneVerworfen()
                  << ", Brueche " << p.analyseKontinuitaetsbrueche() << std::endl;
        pruefe (p.analyseDropsUeberlauf() == 0 && p.analyseDropsOversize() == 0,
                "bei Normallast kein einziger Drop");
        pruefe (p.analyseKontinuitaetsbrueche() == 0,
                "und kein Kontinuitaetsbruch (lueckenloser Transport)");
        // Die Quarantaene haelt genau EINEN Block zurueck.
        pruefe (m.verarbeiteteSamples >= eingespeist - (juce::uint64) bs
                    && m.verarbeiteteSamples <= eingespeist,
                "die Engine sieht den ganzen Strom bis auf den Block in Quarantaene",
                std::to_string ((long long) (eingespeist - m.verarbeiteteSamples)) + " Frames offen");
        pruefe (m.lufsShortGueltig && std::isfinite (m.lufsShort),
                "Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)",
                std::to_string (m.lufsShort));
        pruefe (p.statsSnapshot().framesDropped == 0,
                "framesDropped im Stats-Snapshot kommt aus derselben Quelle");
        p.setPlayHead (nullptr);
    }

    //==========================================================================
    std::cout << "== N - Gegenpfad: prepareToPlay mitten im Betrieb ==" << std::endl;
    {
        constexpr double fs = 48000.0;
        constexpr int bs = 256;
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        TestPlayHead kopf; kopf.spielt = true;
        p.setPlayHead (&kopf);
        juce::AudioBuffer<float> puffer (2, bs);
        juce::MidiBuffer midi;
        for (int k = 0; k < 2; ++k)
        {
            float* d = puffer.getWritePointer (k);
            for (int i = 0; i < bs; ++i) d[i] = 0.2f;
        }
        for (int i = 0; i < 40; ++i) { p.processBlock (puffer, midi); kopf.pos += bs; }

        // Samplerate-Wechsel: der Nachrichtenthread meldet nur einen Neuanlauf.
        p.setPlayConfigDetails (2, 2, 44100.0, bs);
        p.prepareToPlay (44100.0, bs);
        allokationen = 0;
        zaehleAllokationen = true;
        for (int i = 0; i < 40; ++i) { p.processBlock (puffer, midi); kopf.pos += bs; }
        zaehleAllokationen = false;
        pruefe (allokationen == 0, "auch der erste Block nach prepareToPlay alloziert nicht",
                std::to_string (allokationen));
        std::this_thread::sleep_for (std::chrono::milliseconds (200));
        std::cout << "  veraltete Bloecke (alter Anlauf, nicht analysiert): "
                  << p.analyseVeraltet() << std::endl;
        pruefe (p.analyseDropsOversize() == 0, "ein Neuanlauf ist kein Oversize");
        p.setPlayHead (nullptr);
    }

    //==========================================================================
    // Nacharbeit T2-3: die Zusage im Kopf von `neustartAnfordern()` lautet „der
    // Consument erkennt Bloecke aus dem alten Anlauf an ihrer kleineren
    // startFolge". Gemessen hat der T2-Pruefer 0 von 3 - weil die Nummer erst
    // beim naechsten Audioblock stieg, `prepareToPlay` aber typischerweise
    // ruft, WAEHREND der Callback steht. Genau diese Sequenz faehrt O.
    std::cout << "== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==" << std::endl;
    {
        StampedAudioQueue<MiniStrom> q; q.vorbereiten();
        std::vector<float> l (64, 0.1f);
        StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), l.data() };
        std::int64_t zeit = 0;
        for (int i = 0; i < 3; ++i)
        {
            q.veroeffentliche (&t, 1, 2, 64, stempelBei (zeit));
            zeit += 64;
        }
        const std::uint32_t vorher = q.aktuellerAnlauf();

        // Der Nachrichtenthread meldet den Neuanlauf - KEIN Audioblock dazwischen.
        q.neustartAnfordern();
        pruefe (q.aktuellerAnlauf() != vorher,
                "der Anlauf steigt sofort, nicht erst beim naechsten Audioblock",
                std::to_string (vorher) + " -> " + std::to_string (q.aktuellerAnlauf()));

        int alt = 0, aktuell = 0;
        const std::uint32_t anlauf = q.aktuellerAnlauf();
        while (const auto* b = q.spitze())
        {
            if (b->startFolge != anlauf) ++alt; else ++aktuell;
            q.freigeben();
        }
        std::cout << "  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt " << alt
                  << ", als aktuell durchgelassen " << aktuell << std::endl;
        pruefe (alt == 3 && aktuell == 0,
                "alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar",
                std::to_string (alt) + "/3");

        // Gegenprobe: der erste Block DANACH traegt die neue Nummer - sonst
        // koennte O gruen sein, weil gar nichts mehr durchkommt.
        bool neustartUebernommen = false;
        q.veroeffentliche (&t, 1, 2, 64, stempelBei (zeit), &neustartUebernommen);
        pruefe (neustartUebernommen,
                "der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt");
        const auto* neu = q.spitze();
        pruefe (neu != nullptr && neu->startFolge == q.aktuellerAnlauf(),
                "Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell");
        pruefe (neu != nullptr && (neu->flags & kFlagLueckeDavor) != 0,
                "und er traegt die Luecke, die der Neuanlauf gerissen hat");

        neustartUebernommen = true;  // Out-Parameter muss immer beschrieben werden.
        q.veroeffentliche (&t, 1, 2, 64, stempelBei (zeit + 64), &neustartUebernommen);
        pruefe (! neustartUebernommen,
                "der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut");

        // Zwei Neuanlaeufe ohne Audioblock dazwischen duerfen sich nicht
        // gegenseitig verschlucken.
        const std::uint32_t a1 = q.aktuellerAnlauf();
        q.neustartAnfordern();
        q.neustartAnfordern();
        pruefe (q.aktuellerAnlauf() == a1 + 2,
                "zwei Neuanlaeufe hintereinander gehen beide nicht verloren",
                std::to_string (a1) + " -> " + std::to_string (q.aktuellerAnlauf()));
        while (q.spitze()) q.freigeben();
        neustartUebernommen = false;
        q.veroeffentliche (&t, 1, 2, 64, stempelBei (zeit + 128),
                           &neustartUebernommen);
        const auto* nach = q.spitze();
        pruefe (neustartUebernommen,
                "zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration");
        pruefe (nach != nullptr && nach->startFolge == q.aktuellerAnlauf(),
                "und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste");

        // Ein Flush hat keine Audiozeit und darf die wartende Generation nicht
        // konsumieren. Der Out-Parameter bleibt dabei trotzdem definiert.
        while (q.spitze()) q.freigeben();
        q.neustartAnfordern();
        neustartUebernommen = true;
        pruefe (! q.veroeffentliche (&t, 1, 2, 0, stempelBei (zeit + 192),
                                     &neustartUebernommen)
                    && ! neustartUebernommen,
                "Nullframe konsumiert die wartende Generation nicht");
        q.veroeffentliche (&t, 1, 2, 64, stempelBei (zeit + 192),
                           &neustartUebernommen);
        pruefe (neustartUebernommen,
                "der naechste echte Block uebernimmt sie weiterhin");
    }

    //==========================================================================
    // Nacharbeit T2-4: ein Seek bei gestopptem Transport ist eine Grenze
    // (§32.3 „ein Sprung"). Gemessen hatte der Pruefer: Sprung um 10 s => 0
    // Brueche. Die Kehrseite - eine STEHENDE Zeit bei Stopp ist weiterhin kein
    // Bruch - steht als zweite Haelfte daneben, sonst waere der Fix eine
    // Ruecknahme der Entscheidung aus §4.3 Punkt 2.
    std::cout << "== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==" << std::endl;
    {
        auto fahre = [] (bool spielt, std::int64_t sprung, std::uint64_t& brueche)
        {
            StampedAudioQueue<MiniStrom> q; q.vorbereiten();
            Blockquarantaene<MiniStrom> quar; quar.vorbereiten();
            std::vector<float> l (64, 0.1f);
            StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), l.data() };
            std::int64_t zeit = 5000;
            auto zug = [&] (std::int64_t bei)
            {
                q.veroeffentliche (&t, 1, 2, 64, stempelBei (bei, spielt));
                while (const auto* b = q.spitze()) { quar.schiebe (q, *b); q.freigeben(); }
            };
            // Drei Bloecke ohne Sprung. Bei laufendem Transport wandert die
            // Zeit mit, bei Stopp steht sie - beides ist der Normalfall.
            for (int i = 0; i < 3; ++i) { zug (zeit); if (spielt) zeit += 64; }
            const std::uint64_t vor = quar.kontinuitaetsbrueche();
            zug (zeit + sprung);
            zug (zeit + sprung + (spielt ? 64 : 0));
            brueche = quar.kontinuitaetsbrueche() - vor;
        };

        std::uint64_t b = 0;
        fahre (/*spielt*/ false, /*sprung*/ 480000, b);      // 10 s, Transport steht
        pruefe (b == 1, "Seek um 10 s bei gestopptem Transport ist EIN Bruch",
                std::to_string (b));

        fahre (/*spielt*/ false, /*sprung*/ 0, b);
        pruefe (b == 0, "Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)",
                std::to_string (b));

        fahre (/*spielt*/ true, /*sprung*/ 480000, b);
        pruefe (b == 1, "und bei laufendem Transport ist der Seek weiterhin ein Bruch",
                std::to_string (b));

        // Der Fall, an dem die ERSTE Fassung der Regel gescheitert waere
        // (Selbstaudit derselben Session): ein Host, der „gestoppt" meldet und
        // die Projektzeit trotzdem LUECKENLOS fortschreibt. „Bei Stopp ist jede
        // Bewegung ein Bruch" haette hier jeden Block verworfen und die Analyse
        // dort ganz sterben lassen. Eine lueckenlose Fortsetzung ist eine
        // Fortsetzung - der Befund war ein Sprung, kein Fortschritt.
        {
            StampedAudioQueue<MiniStrom> q; q.vorbereiten();
            Blockquarantaene<MiniStrom> quar; quar.vorbereiten();
            std::vector<float> l (64, 0.1f);
            StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), l.data() };
            for (int i = 0; i < 6; ++i)
            {
                q.veroeffentliche (&t, 1, 2, 64, stempelBei (7000 + (std::int64_t) i * 64,
                                                             /*spielt*/ false));
                while (const auto* bl = q.spitze()) { quar.schiebe (q, *bl); q.freigeben(); }
            }
            pruefe (quar.kontinuitaetsbrueche() == 0,
                    "bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch",
                    std::to_string (quar.kontinuitaetsbrueche()));
            pruefe (quar.versiegelteBloecke() == 5,
                    "und alle bis auf den juengsten sind versiegelt",
                    std::to_string (quar.versiegelteBloecke()));
        }

        // Der dritte Fall, den die Regel bewusst NICHT anfasst: ohne gueltiges
        // „spielt" koennte eine wandernde Zeit ganz normale Wiedergabe sein.
        // Waere sie dort ein Bruch, stuerbe jede Analyse ohne Transportbeweis.
        {
            StampedAudioQueue<MiniStrom> q; q.vorbereiten();
            Blockquarantaene<MiniStrom> quar; quar.vorbereiten();
            std::vector<float> l (64, 0.1f);
            StampedAudioQueue<MiniStrom>::TapQuelle t { l.data(), l.data() };
            for (int i = 0; i < 5; ++i)
            {
                Stempel s;                       // Transport UNBEKANNT
                s.kontextAnwesend = true;
                s.zeitGueltig = true;
                s.projectSampleStart = 1000 + (std::int64_t) i * 64;
                q.veroeffentliche (&t, 1, 2, 64, s);
                while (const auto* bl = q.spitze()) { quar.schiebe (q, *bl); q.freigeben(); }
            }
            pruefe (quar.kontinuitaetsbrueche() == 0,
                    "ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet",
                    std::to_string (quar.kontinuitaetsbrueche()));
        }
    }

    //==========================================================================
    std::cout << "== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==" << std::endl;
    {
        using Kadenz = eqcop::detail::WorkerKadenz;
        using namespace std::chrono_literals;
        const Kadenz::Zeitpunkt null {};
        Kadenz k (null);

        const auto sofort = k.faellig (null);
        pruefe (sofort.leicht && ! sofort.schwer,
                "nach Start ist nur die leichte Publikation sofort faellig");

        bool batchSturmBeschleunigt = false;
        for (int zug = 0; zug < 10000; ++zug)
            batchSturmBeschleunigt = batchSturmBeschleunigt || k.faellig (null + 1ms).schwer;
        pruefe (! batchSturmBeschleunigt,
                "10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus");

        const auto vorDeadline = k.faellig (null + 249ms);
        pruefe (! vorDeadline.schwer, "vor 250 ms bleibt Gating/Kandidaten gesperrt");
        const auto aufDeadline = k.faellig (null + 250ms);
        pruefe (aufDeadline.schwer && ! aufDeadline.leicht,
                "bei 250 ms ist genau die schwere Runde faellig");
        pruefe (! k.faellig (null + 250ms).schwer,
                "dieselbe Deadline kann nicht zweimal verbraucht werden");

        const auto nachLangerPause = k.faellig (null + 10s);
        const auto gleicherZeitpunkt = k.faellig (null + 10s);
        pruefe (nachLangerPause.schwer && ! gleicherZeitpunkt.schwer,
                "nach Pause genau eine Runde, kein Catch-up-Sturm");

        k.zuruecksetzen (null + 20s);
        const auto resetSofort = k.faellig (null + 20s);
        const auto resetVorher = k.faellig (null + 20249ms);
        const auto resetExakt = k.faellig (null + 20250ms);
        pruefe (resetSofort.leicht && ! resetSofort.schwer
                    && ! resetVorher.schwer && resetExakt.schwer,
                "Reset/Generation startet die 250-ms-Deadline neu");
    }

    //==========================================================================
    std::cout << "== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==" << std::endl;
    {
        using Uhr = std::chrono::steady_clock;
        constexpr double fs = 48000.0;
        constexpr int bs = 512;
        auto p = std::make_unique<EqCopilotProcessor>();
        p->setPlayConfigDetails (2, 2, fs, bs);
        p->prepareToPlay (fs, bs);
        TestPlayHead kopf; kopf.spielt = true;
        p->setPlayHead (&kopf);

        juce::AudioBuffer<float> puffer (2, bs);
        juce::MidiBuffer midi;
        for (int kanal = 0; kanal < 2; ++kanal)
            for (int i = 0; i < bs; ++i)
                puffer.setSample (kanal, i, 0.25f * std::sin ((float) i * 0.031f));

        // Erst beim ersten Ueberlauf stoppen: zu diesem Zeitpunkt steht der
        // Produktionsring nachweislich unter Rueckstau, aber es kommt KEIN
        // spaeterer Block mehr, der aus dem Drop eine neue Kontinuitaetskante
        // machen und damit die Messung zuruecksetzen koennte.
        int eingespeist = 0;
        while (p->analyseDropsUeberlauf() == 0 && eingespeist < 20000)
        {
            p->processBlock (puffer, midi);
            kopf.pos += bs;
            ++eingespeist;
        }
        pruefe (p->analyseDropsUeberlauf() > 0,
                "Test hat echten Queue-Rueckstau hergestellt",
                std::to_string (eingespeist) + " Bloecke bis zum ersten Drop");

        const auto schwerVorher = p->analyseSchwereAuswertungen();
        std::this_thread::sleep_for (std::chrono::milliseconds (600));
        const auto schwerNachher = p->analyseSchwereAuswertungen();
        const auto schwereRunden = schwerNachher - schwerVorher;
        pruefe (schwereRunden >= 1 && schwereRunden <= 3,
                "unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms",
                std::to_string (schwereRunden) + " Runden in 600 ms");

        // Same-rate-prepare ist ebenfalls eine Queue-Generation. Der
        // verdrahtete Gegenpfad muss die Deadline neu beginnen, nicht einen
        // fast faelligen Takt aus der alten Epoche erben.
        p->prepareToPlay (fs, bs);
        const auto generationVorher = p->analyseSchwereAuswertungen();
        for (int i = 0; i < 32; ++i)
        {
            p->processBlock (puffer, midi);
            kopf.pos += bs;
        }
        std::this_thread::sleep_for (std::chrono::milliseconds (100));
        const auto generationDelta = p->analyseSchwereAuswertungen() - generationVorher;
        pruefe (generationDelta <= 1,
                "neue Generation erbt keinen Batch-beschleunigten Schwertakt",
                std::to_string (generationDelta) + " Runden in den ersten 100 ms");

        // Noch einmal bis zum Drop fuellen und dann direkt zerstoeren. Stop
        // setzt das Atomic, weckt den 50-ms-Wait und joint; ein voller Ring darf
        // daraus weder einen unbeschraenkten Drain noch ein langes Warten machen.
        while (p->analyseDropsUeberlauf() < 2 && eingespeist < 40000)
        {
            p->processBlock (puffer, midi);
            kopf.pos += bs;
            ++eingespeist;
        }

        std::atomic<bool> los { false };
        std::atomic<long long> frameMs { -1 }, resetMs { -1 };
        std::thread frameLeser ([&]
        {
            while (! los.load()) std::this_thread::yield();
            const auto von = Uhr::now();
            (void) p->merkmalFrame();
            frameMs.store (std::chrono::duration_cast<std::chrono::milliseconds>
                (Uhr::now() - von).count());
        });
        std::thread resetSteuerer ([&]
        {
            while (! los.load()) std::this_thread::yield();
            const auto von = Uhr::now();
            p->fordereMessResetAn();
            resetMs.store (std::chrono::duration_cast<std::chrono::milliseconds>
                (Uhr::now() - von).count());
        });
        los.store (true);
        frameLeser.join();
        resetSteuerer.join();
        pruefe (frameMs.load() >= 0 && frameMs.load() < 1000,
                "Frame-Leser wird unter Rueckstau explizit vorgelassen",
                std::to_string (frameMs.load()) + " ms");
        pruefe (resetMs.load() >= 0 && resetMs.load() < 1000,
                "Reset-Steuerer wird unter Rueckstau explizit vorgelassen",
                std::to_string (resetMs.load()) + " ms");

        p->setPlayHead (nullptr);
        const auto stopStart = Uhr::now();
        p.reset();
        const auto stopMs = std::chrono::duration_cast<std::chrono::milliseconds>
            (Uhr::now() - stopStart).count();
        pruefe (stopMs < 2000,
                "Stop/Join bleibt auch mit Queue-Rest begrenzt",
                std::to_string (stopMs) + " ms");
    }

    //==========================================================================
    std::cout << "== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==" << std::endl;
    {
        constexpr double fs = 48000.0;
        constexpr int bs = 64;
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);

        BlockierenderPlayHead kopf;
        kopf.setzePosition (0);
        p.setPlayHead (&kopf);
        juce::AudioBuffer<float> puffer (2, bs);
        puffer.clear();
        juce::MidiBuffer midi;

        p.processBlock (puffer, midi);
        const auto altesFenster = p.messKompakt();
        pruefe (altesFenster.fensterGueltig && altesFenster.fensterVon == 0
                    && altesFenster.fensterBis == bs,
                "Gegenprobe: erste Generation hat ihr eigenes Projektfenster");

        // Der zweite Callback hat den fruehen Callbackteil bereits passiert
        // und steht in getPosition(), also sicher VOR Queue-Publikation. Genau
        // hier fordert der normale Steuerthread Reset + neue Generation an.
        constexpr juce::int64 neuerStart = 10000;
        kopf.blockiereNaechstenAbruf (neuerStart);
        std::thread audio ([&] { p.processBlock (puffer, midi); });
        const bool anKante = kopf.warteBisEingetreten();
        pruefe (anKante, "Testbarriere liegt deterministisch im laufenden Audiocallback");
        p.fordereMessResetAn();
        kopf.freigeben();
        audio.join();

        const auto neuesFenster = p.messKompakt();
        pruefe (neuesFenster.fensterGueltig
                    && neuesFenster.fensterVon == neuerStart
                    && neuesFenster.fensterBis == neuerStart + bs,
                "der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster",
                std::to_string (neuesFenster.fensterVon) + ".."
                    + std::to_string (neuesFenster.fensterBis));
        pruefe (neuesFenster.fensterSpruenge == 0,
                "die Generationskante ist kein Seek innerhalb des alten Fensters",
                std::to_string (neuesFenster.fensterSpruenge));
        p.setPlayHead (nullptr);
    }

    //==========================================================================
    std::cout << "== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==" << std::endl;
    {
        constexpr double fs = 48000.0;
        constexpr int bs = 512;
        EqCopilotProcessor p;
        p.setPlayConfigDetails (2, 2, fs, bs);
        p.prepareToPlay (fs, bs);
        TestPlayHead kopf;
        p.setPlayHead (&kopf);
        juce::AudioBuffer<float> puffer (2, bs);
        juce::MidiBuffer midi;
        for (int kanal = 0; kanal < 2; ++kanal)
            for (int i = 0; i < bs; ++i)
                puffer.setSample (kanal, i, 0.2f * std::sin ((float) i * 0.02f));

        auto speise = [&] (int bloecke)
        {
            for (int i = 0; i < bloecke; ++i)
            {
                p.processBlock (puffer, midi);
                kopf.pos += bs;
                std::this_thread::sleep_for (std::chrono::milliseconds (2));
            }
        };
        speise (24);
        for (int i = 0; i < 100; ++i)
        {
            if (p.messSnapshot().verarbeiteteSamples > 0 && p.merkmaleBloecke() > 0)
                break;
            std::this_thread::sleep_for (std::chrono::milliseconds (10));
        }
        pruefe (p.messSnapshot().verarbeiteteSamples > 0 && p.merkmaleBloecke() > 0,
                "Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine");

        // prepareToPlay sanitisiert NaN auf 0 und setzt eine neue Generation.
        // Die danach wirklich eingestellten Bloecke duerfen keine Engine mit
        // der alten 48-kHz-Zuordnung weiterfuettern.
        p.prepareToPlay (std::numeric_limits<double>::quiet_NaN(), bs);
        speise (64);
        std::this_thread::sleep_for (std::chrono::milliseconds (350));
        const auto ohneRate = p.messSnapshot();
        pruefe (p.holeSamplerate() == 0.0,
                "nichtendliche Hostrate wird fail-closed auf 0 gespiegelt");
        pruefe (ohneRate.verarbeiteteSamples == 0,
                "M1 verarbeitet unter ungueltiger Rate keinen Block",
                std::to_string (ohneRate.verarbeiteteSamples));
        pruefe (p.merkmaleBloecke() == 0,
                "FeatureEngine verarbeitet unter ungueltiger Rate keinen Block",
                std::to_string (p.merkmaleBloecke()));
        pruefe (! p.messKompakt().fensterGueltig,
                "ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung");

        p.prepareToPlay (fs, bs);
        speise (24);
        for (int i = 0; i < 100; ++i)
        {
            if (p.messSnapshot().verarbeiteteSamples > 0 && p.merkmaleBloecke() > 0)
                break;
            std::this_thread::sleep_for (std::chrono::milliseconds (10));
        }
        pruefe (p.messSnapshot().verarbeiteteSamples > 0 && p.merkmaleBloecke() > 0,
                "eine folgende gueltige Generation aktiviert beide Engines wieder");
        p.setPlayHead (nullptr);
    }

    //==========================================================================
    std::cout << std::endl << geprueft << " Pruefungen, " << fehler << " Fehler." << std::endl;
    std::cout << (fehler == 0 ? "QUEUE-STRESSTEST OK" : "QUEUE-STRESSTEST FEHLGESCHLAGEN")
              << std::endl;
    return fehler == 0 ? 0 : 1;
}
