// SONDE-013 M-20 und M-24: der Vergleichspegel und die Delta-Sperre (§38.3,
// §38.4). JUCE-frei und allokationsfrei — derselbe Kernvertrag wie
// `Konfidenz.h` und `TruePeak.h`.
#pragma once

#include <atomic>
#include <cmath>
#include <cstdint>
#include <thread>

namespace nakama::analyse
{

/** Der Vergleichspegel einer Passage (M-20).

    §38.3 wörtlich: „Der Vergleichspegel wird für die gewählte Passage vorab
    gemessen und während A/B eingefroren. Eine kontinuierliche automatische
    Nachregelung während des Umschaltens ist ungeeignet: sie kann Transienten
    und Stille falsch bewerten und wird selbst zum hörbaren Prozessor."

    Der letzte Halbsatz ist der Grund für die Bauform. Eine mitlaufende
    Regelung wäre nicht bloss ungenau — sie veränderte das, was der User
    hört, während er es beurteilt, und machte den Vergleich wertlos. Deshalb
    gibt es hier kein „nachregeln"-Wort: nach `friereEin()` nimmt `speise()`
    zwar weiter entgegen (der Aufrufer im Audiothread soll nicht verzweigen
    müssen), ändert den Wert aber nicht mehr. Der Riegel liegt IM Typ und
    nicht in der Disziplin des Aufrufers.

    Fail-closed: ohne genug Material gibt es keinen Wert, und `gainGesetzt()`
    bleibt falsch. Eine gemeldete 0 dB wäre die Behauptung „die beiden sind
    gleich laut" — „nie gemessen" ist etwas anderes. */
class Vergleichspegel
{
public:
    /** Wie viel Material der Pegel mindestens braucht, bevor er einfrierbar
        ist: 400 ms. Kürzer misst man einen Transienten und nennt ihn Pegel. */
    static constexpr double kMindestSekunden = 0.4;

    /** Der Typ bleibt kopierbar, OBWOHL er seit Befund C6 ein Atomic haelt.

        Das Tor gehoert zur INSTANZ, nicht zum Messwert: eine Kopie ist ein
        Abzug der Zahlen, und der entsteht unter dem Zug der Quelle — nie
        halb. Die Kopie startet mit freiem Tor. Ohne diese beiden Zeilen waere
        der Typ still unbeweglich geworden, und jede Funktion, die einen Pegel
        ZURUECKGIBT, haette aufgehoert zu uebersetzen. */
    Vergleichspegel() = default;
    Vergleichspegel (const Vergleichspegel& andere) noexcept
    {
        const Abzug a = andere.abzug();
        uebernehmen (a);
    }
    Vergleichspegel& operator= (const Vergleichspegel& andere) noexcept
    {
        if (this != &andere)
        {
            // Erst den Abzug der Quelle, dann den eigenen Zug: zwei Zuege
            // gleichzeitig zu halten waere die einzige Stelle, an der zwei
            // Steuerthreads einander blockieren koennten.
            const Abzug a = andere.abzug();
            Steuerzug zug (tor);
            uebernehmenIntern (a);
        }
        return *this;
    }

    void vorbereiten (double abtastrate) noexcept
    {
        Steuerzug zug (tor);
        fs = abtastrate > 0.0 ? abtastrate : 48000.0;
        mindestSamples = (std::uint64_t) (kMindestSekunden * fs);
        leerenIntern();
    }

    /** Nimmt Material auf. Nach dem Einfrieren ohne Wirkung — das IST die
        Zusage, nicht eine Bequemlichkeit.

        🔑 SONDE-013 Nacharbeit 3 (Befund C6, CLAUDE.md „Audio bleibt
        echtzeitfest"): Diese Methode laeuft im AUDIOthread, `vorbereiten`,
        `friereEin` und `loeschen` im Nachrichtenthread — auf denselben
        nichtatomaren Feldern. Bis zur Runde 2 trennte sie nur ein Flag
        AUSSERHALB dieses Typs; ein Callback, der das Flag bereits gelesen
        hatte, schrieb `summeA`, waehrend der Nachrichtenthread dieselben
        Felder las. Das ist ein Datenrennen und damit undefiniertes Verhalten,
        kein Genauigkeitsproblem.

        Der Ausschluss liegt jetzt IM Typ und heisst `tor`. Der Audiothread
        VERSUCHT ihn zu nehmen und laesst den Block aus, wenn gerade der
        Nachrichtenthread darin steht — er wartet NIE, allokiert nicht und
        loggt nicht. Ein ausgelassener Block ist ein nicht gezaehltes Sample,
        keine falsche Zahl; die 400-ms-Schwelle wird dadurch spaeter, nie
        falsch erreicht. */
    void speise (const float* a, const float* b, int n) noexcept
    {
        if (a == nullptr || b == nullptr || n <= 0)
            return;
        if (! audioZugNehmen())
            return;                    // Der Steuerthread ist drin: Block auslassen.
        if (! istEingefroren)
        {
            for (int i = 0; i < n; ++i)
            {
                const double x = (double) a[i];
                const double y = (double) b[i];
                // 🔑 SONDE-013 M-07: nicht-endliche Werte werden GEZAEHLT und
                // VERRIEGELN den Pegel.
                //
                // Vorher wurden sie nur uebersprungen. Eine Passage mit
                // beschaedigten Samples lieferte danach denselben gueltigen
                // eingefrorenen Gain wie eine kuerzere saubere — der Fehler war
                // hinterher unsichtbar, obwohl CLAUDE.md ausdruecklich
                // „verriegelt und gezaehlt" verlangt. Jetzt merkt der Typ sich
                // den Fall: `friereEin()` liefert danach KEINEN Wert, und
                // `nichtEndlicheSamples()` sagt, wie viele es waren.
                if (! std::isfinite (x) || ! std::isfinite (y))
                {
                    ++nichtEndlich;
                    continue;
                }
                summeA += x * x;
                summeB += y * y;
                ++gesehen;
            }
            ++bloeckeAufgenommen;
        }
        audioZugGeben();
    }

    bool bereit() const noexcept
    {
        Steuerzug zug (tor);
        return bereitIntern();
    }

    /** Wie viele nicht-endliche Samples der Pegel gesehen hat. 0 heisst
        nachweislich keines, nicht „nicht gemessen". */
    std::uint64_t nichtEndlicheSamples() const noexcept
    {
        Steuerzug zug (tor);
        return nichtEndlich;
    }

    /** Wie viele Bloecke der Pegel wirklich AUFGENOMMEN hat.

        Die Gegenzahl zu „gespeist": ein Block, den das Passagenfenster oder
        das Tor aussortiert, erscheint hier nicht. Ein Bein misst damit, dass
        Material AUSSERHALB der markierten Passage den Pegel nicht erreicht
        (Befund C4) — ohne diese Zahl liesse sich „nicht aufgenommen" nicht von
        „aufgenommen und zufaellig gleich laut" unterscheiden. */
    std::uint64_t aufgenommeneBloecke() const noexcept
    {
        Steuerzug zug (tor);
        return bloeckeAufgenommen;
    }

    /** Wie viele ENDLICHE Samples in die Summen eingegangen sind.

        Die Gegenprobe zu `aufgenommeneBloecke()`: beide Zahlen entstehen im
        selben Zug, also muss ihr Verhaeltnis fuer gleich grosse Bloecke exakt
        aufgehen. Ein Leser, der einen halb aufgenommenen Block sieht, sieht
        genau hier einen Rest (Befund C6). */
    std::uint64_t gezaehlteSamples() const noexcept
    {
        Steuerzug zug (tor);
        return gesehen;
    }

    /** Alle drei Zaehler in EINEM Zug — die Form, in der ihre Konsistenz
        ueberhaupt pruefbar ist. Drei Einzelaufrufe koennten drei Staende
        sehen und faenden den Riss nie. */
    void zaehlerstand (std::uint64_t& bloecke, std::uint64_t& endliche,
                       std::uint64_t& nichtEndliche) const noexcept
    {
        Steuerzug zug (tor);
        bloecke = bloeckeAufgenommen;
        endliche = gesehen;
        nichtEndliche = nichtEndlich;
    }

    /** Friert den Pegel ein. `false`, wenn zu wenig Material da ist — dann
        bleibt er ungesetzt, statt eine Zahl zu erfinden.

        Nacharbeit 3 (Befund C6): das Einfrieren nimmt das Tor. Ein
        Audiocallback, der gerade schreibt, haelt es; dieser Aufruf wartet
        darauf und sieht danach einen VOLLSTAENDIGEN Block, nie einen halb
        aufgenommenen. Gewartet wird ausschliesslich HIER, im
        Nachrichtenthread. */
    bool friereEin() noexcept
    {
        Steuerzug zug (tor);
        if (istEingefroren)
            return gehaltenGesetzt;
        if (nichtEndlich > 0)
        {
            // M-07: „Wert 0 mit gueltig=false". Der Pegel FRIERT hier ein —
            // ohne Wert. Ein Aufrufer, der es weiter versucht, bekaeme sonst
            // bei jedem Aufruf dieselbe Absage ohne Endzustand, und der
            // Blindvergleich haengte an einem Pegel, der nie fertig wird.
            istEingefroren = true;
            gehalten = 0.0;
            gehaltenGesetzt = false;
            return false;
        }
        if (! bereitIntern())
        {
            // Ausdrücklich KEIN Einfrieren: ein gesperrter Zustand ohne Wert
            // ist ehrlich, ein eingefrorener ohne Messung wäre eine Lüge.
            return false;
        }
        const double rmsA = std::sqrt (summeA / (double) gesehen);
        const double rmsB = std::sqrt (summeB / (double) gesehen);
        gehalten = 20.0 * std::log10 (rmsB / rmsA);
        gehaltenGesetzt = std::isfinite (gehalten);
        istEingefroren = true;
        return gehaltenGesetzt;
    }

    bool eingefroren() const noexcept { Steuerzug zug (tor); return istEingefroren; }
    bool gainGesetzt() const noexcept { Steuerzug zug (tor); return gehaltenGesetzt; }

    /** Der eingefrorene Ausgleich in dB (B relativ zu A). Nur gültig, wenn
        `gainGesetzt()`. */
    double gainDb() const noexcept { Steuerzug zug (tor); return gehalten; }

    /** Verwirft den Pegel — beim Wechsel der Passage. Danach beginnt die
        Messung von vorn, denn ein Pegel gehört zu GENAU einer Passage. */
    void loeschen() noexcept
    {
        Steuerzug zug (tor);
        leerenIntern();
    }

private:
    //== Das Tor (Befund C6) =================================================
    //
    // Drei Zustaende, ein Atomic. Der Audiothread nimmt `frei → audio` mit
    // einem CAS und gibt bei Misserfolg AUF; der Nachrichtenthread nimmt
    // `frei → steuerung` und wartet dabei. Das ist kein Mutex: der
    // Audiothread blockiert nie, es gibt keine Prioritaetsinversion, kein
    // Betriebssystemobjekt und keine Allokation.
    enum : int { torFrei = 0, torAudio = 1, torSteuerung = 2 };

    bool audioZugNehmen() const noexcept
    {
        int erwartet = torFrei;
        return tor.compare_exchange_strong (erwartet, torAudio,
                                            std::memory_order_acquire,
                                            std::memory_order_relaxed);
    }
    void audioZugGeben() const noexcept
    {
        tor.store (torFrei, std::memory_order_release);
    }

    /** RAII-Zug des Nachrichtenthreads. Er wartet — und nur er darf das. */
    struct Steuerzug
    {
        explicit Steuerzug (std::atomic<int>& t) noexcept : tor (t)
        {
            for (;;)
            {
                int erwartet = torFrei;
                if (tor.compare_exchange_weak (erwartet, torSteuerung,
                                               std::memory_order_acquire,
                                               std::memory_order_relaxed))
                    return;
                std::this_thread::yield();
            }
        }
        ~Steuerzug() { tor.store (torFrei, std::memory_order_release); }
        Steuerzug (const Steuerzug&) = delete;
        Steuerzug& operator= (const Steuerzug&) = delete;
        std::atomic<int>& tor;
    };

    /** Der reine Messstand ohne Tor — die Form, in der eine Kopie entsteht. */
    struct Abzug
    {
        double fs { 48000.0 };
        std::uint64_t mindestSamples { 19200 };
        double summeA { 0.0 }, summeB { 0.0 };
        std::uint64_t gesehen { 0 };
        std::uint64_t nichtEndlich { 0 };
        std::uint64_t bloeckeAufgenommen { 0 };
        bool istEingefroren { false };
        double gehalten { 0.0 };
        bool gehaltenGesetzt { false };
    };

    Abzug abzug() const noexcept
    {
        Steuerzug zug (tor);
        return Abzug { fs, mindestSamples, summeA, summeB, gesehen, nichtEndlich,
                       bloeckeAufgenommen, istEingefroren, gehalten, gehaltenGesetzt };
    }

    void uebernehmenIntern (const Abzug& a) noexcept
    {
        fs = a.fs;
        mindestSamples = a.mindestSamples;
        summeA = a.summeA;
        summeB = a.summeB;
        gesehen = a.gesehen;
        nichtEndlich = a.nichtEndlich;
        bloeckeAufgenommen = a.bloeckeAufgenommen;
        istEingefroren = a.istEingefroren;
        gehalten = a.gehalten;
        gehaltenGesetzt = a.gehaltenGesetzt;
    }

    /** Nur aus dem Kopierkonstruktor: `this` ist dort noch niemandem bekannt,
        also braucht es keinen eigenen Zug. */
    void uebernehmen (const Abzug& a) noexcept { uebernehmenIntern (a); }

    bool bereitIntern() const noexcept
    {
        // Ein einziges nicht-endliches Sample sperrt: der Pegel dieser Passage
        // ist nicht mehr messbar, und eine Zahl ohne diesen Vorbehalt waere
        // genau die unsichtbare Beschoenigung, gegen die M-07 steht.
        return nichtEndlich == 0
            && gesehen >= mindestSamples && summeA > 0.0 && summeB > 0.0;
    }

    void leerenIntern() noexcept
    {
        summeA = summeB = 0.0;
        gesehen = 0;
        nichtEndlich = 0;
        bloeckeAufgenommen = 0;
        istEingefroren = false;
        gehalten = 0.0;
        gehaltenGesetzt = false;
    }

    mutable std::atomic<int> tor { torFrei };
    double fs { 48000.0 };
    std::uint64_t mindestSamples { 19200 };
    double summeA { 0.0 }, summeB { 0.0 };
    std::uint64_t gesehen { 0 };
    /// M-07: gezaehlte nicht-endliche Eingangssamples. Sie verriegeln.
    std::uint64_t nichtEndlich { 0 };
    /// Befund C4: wie viele Bloecke wirklich in die Summen eingegangen sind.
    std::uint64_t bloeckeAufgenommen { 0 };
    bool istEingefroren { false };
    double gehalten { 0.0 };
    bool gehaltenGesetzt { false };
};

/** Warum hörbares Delta gesperrt ist — oder dass es frei ist (M-24). */
enum class Deltasperre : std::uint8_t
{
    /** Kein nachgewiesenes Compare-Routing. In P4 der Normalfall: Feature-
        Telemetrie kann `POST − PRE` niemals als Audio erzeugen (§38.4). */
    keinComparerouting = 0,
    /** Routing da, aber der gemessene PRE→POST-Lag reicht die
        FIR-Gruppenlaufzeit nicht aus dem früheren PRE-Nebenpfad. Den
        hörbaren POST-Pfad zu verzögern ist ausgeschlossen — das bräche den
        0-Sample-Vertrag (§38.4). */
    lagTraegtGruppenlaufzeitNicht = 1,
    /** Routing da, aber ohne belastbares Alignment. */
    alignmentUnzureichend = 2,
    /** Frei. */
    frei = 3
};

/** Die Sperre als reine Funktion (M-24).

    Sie ist `constexpr` und hat drei binäre Eingaben, also lässt sich ihr
    Raum VOLLSTÄNDIG abfahren statt an drei handverlesenen Fällen geprüft —
    dieselbe Bauform wie `gesamtklasse` in `Konfidenz.h`.

    Die Reihenfolge ist die der Härte: ohne Routing gibt es die Frage nach
    dem Lag gar nicht. Wer sie umdrehte, meldete bei fehlendem Routing einen
    Lag-Grund und legte damit nahe, ein längerer Lag würde helfen. */
constexpr Deltasperre deltaZustand (bool compareRoutingNachgewiesen,
                                    bool alignmentBelastbar,
                                    bool lagTraegtGruppenlaufzeit) noexcept
{
    if (! compareRoutingNachgewiesen)
        return Deltasperre::keinComparerouting;
    if (! alignmentBelastbar)
        return Deltasperre::alignmentUnzureichend;
    if (! lagTraegtGruppenlaufzeit)
        return Deltasperre::lagTraegtGruppenlaufzeitNicht;
    return Deltasperre::frei;
}

/** In P4 gibt es kein Compare-Routing. Diese Konstante sagt es an genau einer
    Stelle, damit die Freischaltung später ein sichtbarer Schritt ist und
    nicht eine verstreute Bedingung. */
inline constexpr bool kCompareroutingInDieserPhase = false;

constexpr const char* sperrName (Deltasperre s) noexcept
{
    switch (s)
    {
        case Deltasperre::keinComparerouting:          return "kein Compare-Routing";
        case Deltasperre::alignmentUnzureichend:       return "Alignment unzureichend";
        case Deltasperre::lagTraegtGruppenlaufzeitNicht: return "Lag traegt die Gruppenlaufzeit nicht";
        case Deltasperre::frei:                        return "frei";
    }
    return "unbekannt";
}

} // namespace nakama::analyse
