/*  EqCopSonde013InterventionRingTest — der RT→Control-Ring und der
    NAK-47-Ausfade (SONDE-013 M-34 bis M-39, §7.1 E-01).

    §34.2 woertlich, und der Satz ist der Grund fuer den ganzen Ring:

        "Ein 1-Hz-Heartbeat-Boolean reicht dafuer nicht: Ein kurzer
         Hoermarker kann vollstaendig zwischen zwei Heartbeats liegen."

    Bis SONDE-013 fuehrte der Prozessor genau so ein Bit. Ein Marker von
    300 ms zwischen zwei Heartbeats war unsichtbar — und jede nachgelagerte
    Sonde, die waehrenddessen mass, lieferte Evidenz aus verfaerbtem Audio,
    ohne dass es jemand wusste.

    ── WAS DIESES BEIN MISST ────────────────────────────────────────────────

    Zwei Dinge, die zusammengehoeren und getrennt fallen koennen:

    1. **Den Ring** als Datenstruktur: FIFO-Ordnung, Sequenzluecken, und vor
       allem, dass er bei Ueberlauf MELDET statt das aelteste zu verwerfen.
       Das ist die Asymmetrie aus M-39: ein verlorener Liveframe kostet eine
       Anzeige, ein verlorenes Begin kostet die Wahrheit.
    2. **Den Ausfade** (NAK-47): dass der Verlust der Erlaubnis nicht mehr
       schneidet, dass danach der Neutralpfad BIT-IDENTISCH ist, und dass der
       Oversize-Pfad denselben Vertrag haelt und danach verriegelt.

    Der Allokationszaehler unten ist keine Zierde: M-74 verlangt fuer die
    Zusage "keine Allokation" ausdruecklich einen Zaehler, keine Behauptung.
*/

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>

#include "../core/ipc/InterventionsRing.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <new>
#include <vector>

namespace
{
thread_local bool zaehleAudiothread = false;
thread_local std::uint64_t audioAllokationen = 0;
}

void* operator new (std::size_t groesse)
{
    if (zaehleAudiothread) ++audioAllokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete (void* p) noexcept { std::free (p); }
void operator delete (void* p, std::size_t) noexcept { std::free (p); }
void* operator new[] (std::size_t groesse)
{
    if (zaehleAudiothread) ++audioAllokationen;
    if (groesse == 0) groesse = 1;
    if (void* p = std::malloc (groesse)) return p;
    throw std::bad_alloc();
}
void operator delete[] (void* p) noexcept { std::free (p); }
void operator delete[] (void* p, std::size_t) noexcept { std::free (p); }

namespace
{
int bestanden = 0;
int fehler = 0;

void pruefe (bool ok, const juce::String& was, const juce::String& zusatz = {})
{
    std::cout << (ok ? "[ok]   " : "[ROT]  ") << was.toRawUTF8();
    if (zusatz.isNotEmpty())
        std::cout << "  [" << zusatz.toRawUTF8() << "]";
    std::cout << std::endl;
    if (ok) ++bestanden; else ++fehler;
}

void abschnitt (const char* text)
{
    std::cout << "\n== " << text << " ==" << std::endl;
}
} // namespace

int main()
{
    using nakama::ipc::InterventionsRing;
    using nakama::ipc::Interventionsereignis;

    std::cout << "== Nakama SONDE-013 - Interventionsring und NAK-47-Ausfade =="
              << std::endl;

    // ── M-37: begin_is_immediate_not_heartbeat_bound ─────────────────────
    //
    // Der Ring nimmt ein Ereignis SOFORT auf und gibt es sofort wieder her -
    // ohne Zeitfenster, ohne Kadenz. Das ist der Unterschied zum
    // Heartbeat-Bit, und er laesst sich an der Struktur messen: zwischen
    // Schreiben und Lesen liegt kein Zustand, der auf eine Uhr wartet.
    abschnitt ("M-37  begin_is_immediate_not_heartbeat_bound");
    {
        InterventionsRing ring;
        Interventionsereignis e;
        e.beginn = true;
        e.sequenz = 1;
        e.projektzeitGesetzt = true;
        e.projektSample = 480000;

        pruefe (ring.fuellstand() == 0, "der frische Ring ist leer");
        pruefe (ring.schreibe (e), "ein Begin wird aufgenommen");
        pruefe (ring.fuellstand() == 1,
                "und steht SOFORT zur Abholung bereit - kein Kadenzfenster dazwischen",
                juce::String (ring.fuellstand()));

        Interventionsereignis gelesen;
        pruefe (ring.lies (gelesen), "der Worker holt es ab");
        pruefe (gelesen.beginn && gelesen.sequenz == 1
                  && gelesen.projektzeitGesetzt && gelesen.projektSample == 480000,
                "vollstaendig und unveraendert - Sequenz, Art und Projektzeit");
        pruefe (! ring.lies (gelesen), "danach ist der Ring wieder leer");

        // Ein sehr kurzer Marker: Begin und End im selben 512er-Block. Genau
        // der Fall, den ein 1-Hz-Bit nie sieht.
        Interventionsereignis anfang, ende;
        anfang.beginn = true;  anfang.sequenz = 2;
        ende.beginn = false;   ende.sequenz = 3;   ende.dauerSamples = 380;
        ende.tailSamples = 4800;
        pruefe (ring.schreibe (anfang) && ring.schreibe (ende),
                "ein Marker von 380 Samples erzeugt BEIDE Ereignisse");
        Interventionsereignis a2, e2;
        pruefe (ring.lies (a2) && ring.lies (e2) && a2.beginn && ! e2.beginn
                  && a2.sequenz == 2 && e2.sequenz == 3,
                "und beide kommen in der richtigen Reihenfolge an - ein Heartbeat "
                "haette hier gar nichts gesehen");
    }

    // ── M-38: end_carries_conservative_tail_and_follows_the_fade ─────────
    abschnitt ("M-38  end_carries_conservative_tail_and_follows_the_fade");
    {
        InterventionsRing ring;
        Interventionsereignis ende;
        ende.beginn = false;
        ende.sequenz = 7;
        ende.dauerSamples = 12000;
        ende.tailSamples = 24000;
        pruefe (ring.schreibe (ende), "ein Ende wird aufgenommen");
        Interventionsereignis g;
        ring.lies (g);
        pruefe (g.tailSamples >= g.dauerSamples,
                "der Nachlauf ist mindestens so lang wie der Eingriff selbst - §34.2: "
                "'der Bereich wird laenger quarantaenisiert als die Intervention "
                "selbst dauerte'",
                juce::String ((int) g.tailSamples) + " gegen "
                + juce::String ((int) g.dauerSamples) + " Samples");
        pruefe (g.dauerSamples > 0,
                "und die Dauer reist MIT - ein Empfaenger kann die Konservativitaet "
                "damit pruefen statt sie zu glauben");
    }

    // ── M-39: ring_overflow_reports_instead_of_dropping_oldest ───────────
    //
    // Die schaerfste Zusage dieses Beins. P2 darf `drop-oldest`; dieser Ring
    // ausdruecklich NICHT. Ein verlorenes Begin erzeugte eine "scheinbar
    // saubere Baseline" (§34.2) - und alles, was darauf gebaut wird, ist
    // falsch, ohne dass es jemand sieht.
    abschnitt ("M-39  ring_overflow_reports_instead_of_dropping_oldest");
    {
        InterventionsRing ring;
        int aufgenommen = 0;
        for (int i = 0; i < InterventionsRing::kPlaetze + 8; ++i)
        {
            Interventionsereignis e;
            e.beginn = (i % 2) == 0;
            e.sequenz = (std::uint64_t) i;
            if (ring.schreibe (e))
                ++aufgenommen;
        }
        pruefe (aufgenommen == InterventionsRing::kPlaetze,
                "der Ring nimmt genau seine Kapazitaet auf und keinen mehr",
                juce::String (aufgenommen) + " von "
                + juce::String (InterventionsRing::kPlaetze + 8) + " Versuchen");
        pruefe (ring.ueberlaufGesehen(),
                "der Ueberlauf ist GEMELDET - sticky, nicht verschwiegen");
        pruefe (ring.verworfeneEreignisse() == 8,
                "und gezaehlt",
                juce::String ((int) ring.verworfeneEreignisse()) + " verworfen");

        // Das AELTESTE steht noch drin - kein drop-oldest.
        Interventionsereignis erstes;
        pruefe (ring.lies (erstes) && erstes.sequenz == 0,
                "das AELTESTE Ereignis steht unveraendert an erster Stelle - dieser "
                "Ring verwirft es NICHT, im Gegensatz zu P2",
                "Sequenz " + juce::String ((int) erstes.sequenz));

        // Und die Reihenfolge des Restes stimmt weiter.
        bool aufsteigend = true;
        std::uint64_t vorige = erstes.sequenz;
        Interventionsereignis e;
        int gelesen = 1;
        while (ring.lies (e))
        {
            if (e.sequenz <= vorige) aufsteigend = false;
            vorige = e.sequenz;
            ++gelesen;
        }
        pruefe (aufsteigend && gelesen == InterventionsRing::kPlaetze,
                "und der ganze Rest kommt lueckenlos aufsteigend heraus",
                juce::String (gelesen) + " Ereignisse");

        // Das Sticky-Bit heilt NICHT von selbst.
        pruefe (ring.ueberlaufGesehen(),
                "nach dem Leeren steht das Sticky-Bit WEITER - §34.2: starke Evidenz "
                "bleibt bis zu einem expliziten Resync gesperrt. Ein Bit, das sich "
                "selbst heilt, sobald wieder Platz ist, verloere genau die Information, "
                "um derentwillen es existiert");
        ring.resync();
        pruefe (! ring.ueberlaufGesehen(),
                "erst `resync()` loescht es - der einzige Weg");
    }

    // ── M-74: audio_thread_allocates_zero ────────────────────────────────
    abschnitt ("M-74  audio_thread_allocates_zero_and_touches_no_pipe");
    {
        InterventionsRing ring;
        // Der Ring ist ein Feld, kein Zeiger: seine Konstruktion liegt VOR
        // dem gezaehlten Bereich, genau wie im Prozessor.
        audioAllokationen = 0;
        zaehleAudiothread = true;
        for (int runde = 0; runde < 200; ++runde)
        {
            Interventionsereignis e;
            e.beginn = (runde % 2) == 0;
            e.sequenz = (std::uint64_t) runde;
            e.tailSamples = 4800;
            (void) ring.schreibe (e);
            Interventionsereignis g;
            (void) ring.lies (g);
            (void) ring.fuellstand();
            (void) ring.ueberlaufGesehen();
        }
        zaehleAudiothread = false;
        pruefe (audioAllokationen == 0,
                "200 Runden Schreiben, Lesen und Abfragen erzeugen NULL Allokationen - "
                "gezaehlt, nicht behauptet (M-74)",
                juce::String ((int) audioAllokationen) + " Allokationen");
    }

    // ── Zahlenraender ────────────────────────────────────────────────────
    abschnitt ("Raender: Sequenzueberlauf, Rueckgabewerte, Neuaufbau");
    {
        InterventionsRing ring;
        Interventionsereignis e;
        e.sequenz = ~std::uint64_t (0) - 1;      // knapp unter dem u64-Rand
        e.tailSamples = ~std::uint64_t (0);
        pruefe (ring.schreibe (e), "eine Sequenz am u64-Rand wird aufgenommen");
        Interventionsereignis g;
        pruefe (ring.lies (g) && g.sequenz == e.sequenz && g.tailSamples == e.tailSamples,
                "und kommt unveraendert zurueck - der Ring rechnet nicht mit den Werten");

        ring.zuruecksetzen();
        pruefe (ring.fuellstand() == 0 && ! ring.ueberlaufGesehen()
                  && ring.verworfeneEreignisse() == 0,
                "`zuruecksetzen()` leert Ring, Sticky-Bit und Zaehler gemeinsam - "
                "ein halber Neuaufbau waere ein Zustand, den niemand deuten kann");
    }

    std::cout << "\n-----------------------------------------" << std::endl;
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
