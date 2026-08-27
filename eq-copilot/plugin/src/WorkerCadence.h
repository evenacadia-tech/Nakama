// Monotone Wanduhr-Kadenz fuer den Analyse-Worker.
//
// Der Worker darf bei Queue-Rueckstau mehrere begrenzte Zuege unmittelbar
// hintereinander fahren. Deshalb darf die teure Kandidaten-/Gating-Auswertung
// nicht aus einer Zugzahl abgeleitet werden: acht abgearbeitete Bloecke sind
// keine 50 ms. Diese kleine, uhr-injizierbare Zustandsmaschine haelt schwere
// und leichte Publikation an echten steady_clock-Deadlines und holt nach einer
// Pause niemals mehrere verpasste Takte nach.
#pragma once

#include <chrono>

namespace eqcop::detail
{

class WorkerKadenz final
{
public:
    using Uhr = std::chrono::steady_clock;
    using Zeitpunkt = Uhr::time_point;

    struct Faelligkeit
    {
        bool leicht = false;
        bool schwer = false;
    };

    static constexpr auto leichtesIntervall = std::chrono::milliseconds (50);
    static constexpr auto schweresIntervall = std::chrono::milliseconds (250);

    explicit WorkerKadenz (Zeitpunkt jetzt = Uhr::now()) noexcept
    {
        zuruecksetzen (jetzt);
    }

    // Generation, Samplerate und Mess-Reset sind Epochenkanten. Die erste
    // schwere Auswertung der neuen Epoche bekommt wieder ein volles Intervall;
    // eine leichte Publikation darf sofort den frischen Grundzustand spiegeln.
    void zuruecksetzen (Zeitpunkt jetzt) noexcept
    {
        naechsteLeichte = jetzt;
        naechsteSchwere = jetzt + schweresIntervall;
    }

    Faelligkeit faellig (Zeitpunkt jetzt) noexcept
    {
        if (jetzt >= naechsteSchwere)
        {
            // Von JETZT aus weiterplanen. So entsteht nach Suspend, Debugger-
            // Halt oder langsamer Auswertung keine Catch-up-Schleife.
            naechsteSchwere = jetzt + schweresIntervall;
            naechsteLeichte = jetzt + leichtesIntervall;
            return { false, true };
        }

        if (jetzt >= naechsteLeichte)
        {
            naechsteLeichte = jetzt + leichtesIntervall;
            return { true, false };
        }

        return {};
    }

private:
    Zeitpunkt naechsteLeichte {};
    Zeitpunkt naechsteSchwere {};
};

} // namespace eqcop::detail
