// SONDE-013 M-20 und M-24: der Vergleichspegel und die Delta-Sperre (§38.3,
// §38.4). JUCE-frei und allokationsfrei — derselbe Kernvertrag wie
// `Konfidenz.h` und `TruePeak.h`.
#pragma once

#include <cmath>
#include <cstdint>

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

    void vorbereiten (double abtastrate) noexcept
    {
        fs = abtastrate > 0.0 ? abtastrate : 48000.0;
        mindestSamples = (std::uint64_t) (kMindestSekunden * fs);
        summeA = summeB = 0.0;
        gesehen = 0;
        nichtEndlich = 0;
        istEingefroren = false;
        gehalten = 0.0;
        gehaltenGesetzt = false;
    }

    /** Nimmt Material auf. Nach dem Einfrieren ohne Wirkung — das IST die
        Zusage, nicht eine Bequemlichkeit. */
    void speise (const float* a, const float* b, int n) noexcept
    {
        if (istEingefroren || a == nullptr || b == nullptr || n <= 0)
            return;
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
            // „verriegelt und gezaehlt" verlangt. Jetzt merkt der Typ sich den
            // Fall: `friereEin()` liefert danach KEINEN Wert, und
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
    }

    bool bereit() const noexcept
    {
        // Ein einziges nicht-endliches Sample sperrt: der Pegel dieser Passage
        // ist nicht mehr messbar, und eine Zahl ohne diesen Vorbehalt waere
        // genau die unsichtbare Beschoenigung, gegen die M-07 steht.
        return nichtEndlich == 0
            && gesehen >= mindestSamples && summeA > 0.0 && summeB > 0.0;
    }

    /** Wie viele nicht-endliche Samples der Pegel gesehen hat. 0 heisst
        nachweislich keines, nicht „nicht gemessen". */
    std::uint64_t nichtEndlicheSamples() const noexcept { return nichtEndlich; }

    /** Friert den Pegel ein. `false`, wenn zu wenig Material da ist — dann
        bleibt er ungesetzt, statt eine Zahl zu erfinden. */
    bool friereEin() noexcept
    {
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
        if (! bereit())
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

    bool eingefroren() const noexcept { return istEingefroren; }
    bool gainGesetzt() const noexcept { return gehaltenGesetzt; }

    /** Der eingefrorene Ausgleich in dB (B relativ zu A). Nur gültig, wenn
        `gainGesetzt()`. */
    double gainDb() const noexcept { return gehalten; }

    /** Verwirft den Pegel — beim Wechsel der Passage. Danach beginnt die
        Messung von vorn, denn ein Pegel gehört zu GENAU einer Passage. */
    void loeschen() noexcept
    {
        summeA = summeB = 0.0;
        gesehen = 0;
        nichtEndlich = 0;
        istEingefroren = false;
        gehalten = 0.0;
        gehaltenGesetzt = false;
    }

private:
    double fs { 48000.0 };
    std::uint64_t mindestSamples { 19200 };
    double summeA { 0.0 }, summeB { 0.0 };
    std::uint64_t gesehen { 0 };
    /// M-07: gezaehlte nicht-endliche Eingangssamples. Sie verriegeln.
    std::uint64_t nichtEndlich { 0 };
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
