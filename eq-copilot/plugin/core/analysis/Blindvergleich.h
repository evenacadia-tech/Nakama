// SONDE-013 M-43 und M-44: der Blindvergleich auf der Gen-Seite (§43.1).
// JUCE-frei, allokationsfrei, ohne jede Sicht — derselbe Kernvertrag wie
// `Konfidenz.h` und `Vergleichspegel.h`.
//
// ── DIE KANTE LIEGT IM TYP, NICHT IN DER DISZIPLIN DER UI ────────────────
//
// M-44 woertlich: „Identität und Reihenfolge werden erst NACH dem Urteil
// aufgedeckt und vorher im append-only Experimentereignis gebunden — das ist
// die Kante, die verhindert, dass die Reihenfolge nachträglich zum Urteil
// passend erzählt wird."
//
// Eine UI, die die Reihenfolge kennt und sich nur verpflichtet, sie nicht zu
// zeigen, ist kein Blindvergleich: ein Tooltip, ein Debugausdruck oder eine
// spaetere Sortierung reichen, und niemand merkt es. Deshalb gibt dieser Typ
// sie vor dem Urteil GAR NICHT heraus. `welcheSeiteIstA()` existiert nicht;
// stattdessen fragt die UI `seiteAKlingtWie()` — und bekommt bis zum Urteil
// nur eine Nummer, die nichts verraet.
#pragma once

#include <cstdint>

#include "Vergleichspegel.h"

namespace nakama::analyse
{

/** Welche Seite im A/B zuerst liegt (`experiment_manual_result`). */
enum class Blindreihenfolge : std::uint8_t
{
    baselineZuerst = 0,
    kandidatZuerst = 1
};

/** Das Hoerurteil des Users — ausdruecklich USERDATEN, keine Messung. */
enum class Hoerurteil : std::uint8_t
{
    baseline = 0,
    kandidat = 1,
    keinUnterschied = 2,
    enthaltung = 3
};

/** Warum ein Urteil nicht angenommen wurde. */
enum class Urteilsperre : std::uint8_t
{
    /// Kein eingefrorener Vergleichspegel. §15 woertlich: „Eine Klangwertung
    /// ohne vorherigen Lautheitsabgleich ist unzulaessig" (M-43).
    ohneLautheitsabgleich = 0,
    /// Die Blindreihenfolge ist noch nicht gebunden (M-44).
    reihenfolgeNichtGebunden = 1,
    /// Es liegt schon ein Urteil vor. Ein zweites deutete das erste um.
    schonGeurteilt = 2,
    /// Frei.
    frei = 3
};

/** Der Blindvergleich einer Passage. */
class Blindvergleich
{
public:
    /** Bindet die Reihenfolge, BEVOR der User urteilt.

        Ein zweiter Aufruf aendert sie nicht und meldet `false`: sonst liesse
        sie sich nach dem Hoeren noch drehen, und das append-only Ereignis
        waere eine Behauptung ohne Wert. */
    bool bindeReihenfolge (Blindreihenfolge r) noexcept
    {
        if (gebunden || hatUrteil)
            return false;
        reihenfolge = r;
        gebunden = true;
        return true;
    }

    bool reihenfolgeGebunden() const noexcept { return gebunden; }

    /** Uebernimmt den eingefrorenen Vergleichspegel (M-43) — GENAU EINMAL.

        Er wird KOPIERT und nicht referenziert: der Pegel der Passage gehoert
        zum Versuch, und ein spaeter geloeschter oder neu gemessener Pegel
        darf ein bereits gefaelltes Urteil nicht ruecklaeufig entwerten.

        ⚠️ Ein ZWEITER Pegel wird abgelehnt, auch vor dem Urteil. M-43 sagt
        „fuer die Dauer des Versuchs eingefroren" und zaehlt den Match-Gain zu
        den UNVERAENDERLICHEN Referenzen (§43.1). Solange ein zweiter Aufruf
        `gainDb` ueberschrieb, war „eingefroren" eine Absichtserklaerung der
        UI und keine Eigenschaft des Typs: zwei Aufrufe mit verschiedenen
        Pegeln verschoben den Bezugspunkt des Vergleichs, und das Urteil
        stuende danach gegen einen anderen Pegel als den gemessenen. Wer den
        Pegel wirklich neu setzen will, verwirft den Versuch (`loeschen()`). */
    bool uebernimmVergleichspegel (const Vergleichspegel& p) noexcept
    {
        if (hatUrteil || gainGesetzt || ! p.eingefroren() || ! p.gainGesetzt())
            return false;
        gainDb = p.gainDb();
        gainGesetzt = true;
        return true;
    }

    bool lautheitAbgeglichen() const noexcept { return gainGesetzt; }

    /** Was einem Urteil im Weg steht — oder `frei`. */
    Urteilsperre sperre() const noexcept
    {
        if (hatUrteil)
            return Urteilsperre::schonGeurteilt;
        if (! gainGesetzt)
            return Urteilsperre::ohneLautheitsabgleich;
        if (! gebunden)
            return Urteilsperre::reihenfolgeNichtGebunden;
        return Urteilsperre::frei;
    }

    /** Nimmt das Urteil des Users an. `false`, wenn eine Sperre greift. */
    bool urteile (Hoerurteil h) noexcept
    {
        if (sperre() != Urteilsperre::frei)
            return false;
        urteil = h;
        hatUrteil = true;
        return true;
    }

    bool geurteilt() const noexcept { return hatUrteil; }
    Hoerurteil holeUrteil() const noexcept { return urteil; }

    /** Die aufgedeckte Reihenfolge — NUR nach dem Urteil (M-44).

        `false` heisst „noch nicht aufgedeckt" und nicht „baselineZuerst";
        `aus` bleibt dann unberuehrt. Ein Rueckgabewert, den man versehentlich
        als Reihenfolge lesen kann, waere genau das Leck, das dieser Typ
        schliesst. */
    bool aufgedeckteReihenfolge (Blindreihenfolge& aus) const noexcept
    {
        if (! hatUrteil || ! gebunden)
            return false;
        aus = reihenfolge;
        return true;
    }

    /** Was die UI VOR dem Urteil ueber eine Seite wissen darf: eine Nummer,
        die konstant bleibt, und sonst nichts.

        Sie erlaubt der UI, zwei Schaltflaechen auseinanderzuhalten und den
        Wechsel zu zeigen, ohne die Identitaet zu verraten. */
    static constexpr int seiteA = 0;
    static constexpr int seiteB = 1;

    /** Verwirft alles — beim Wechsel der Passage. */
    void loeschen() noexcept
    {
        gebunden = false;
        hatUrteil = false;
        gainGesetzt = false;
        gainDb = 0.0;
        reihenfolge = Blindreihenfolge::baselineZuerst;
        urteil = Hoerurteil::enthaltung;
    }

private:
    // Privat, und es gibt keinen Getter ausser `aufgedeckteReihenfolge`.
    Blindreihenfolge reihenfolge { Blindreihenfolge::baselineZuerst };
    bool   gebunden { false };
    bool   hatUrteil { false };
    bool   gainGesetzt { false };
    double gainDb { 0.0 };
    Hoerurteil urteil { Hoerurteil::enthaltung };
};

constexpr const char* sperrWort (Urteilsperre s) noexcept
{
    switch (s)
    {
        case Urteilsperre::ohneLautheitsabgleich:    return "ohne Lautheitsabgleich";
        case Urteilsperre::reihenfolgeNichtGebunden: return "Reihenfolge nicht gebunden";
        case Urteilsperre::schonGeurteilt:           return "schon geurteilt";
        case Urteilsperre::frei:                     return "frei";
    }
    return "unbekannt";
}

} // namespace nakama::analyse
