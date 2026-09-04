/*  EqCopSonde013ExperimentUiTest — die zwei Kanten des Blindvergleichs
    (SONDE-013 M-43, M-44; §43.1, §15).

    Beide Zusagen sind Sperren, und beide schuetzen den User vor sich selbst:

    **Kein Klangurteil ohne Lautheitsabgleich** (§15 woertlich: „Eine
    Klangwertung ohne vorherigen Lautheitsabgleich ist unzulaessig"). Lauter
    klingt besser — das ist keine Meinung, sondern eine seit Jahrzehnten
    gemessene Eigenschaft des Gehoers. Ein A/B ohne Pegelabgleich misst
    zuverlaessig, welche Seite lauter war.

    **Die Reihenfolge wird vor dem Urteil gebunden und erst danach
    aufgedeckt** (M-44). Sonst laesst sich nachtraeglich erzaehlen, welche
    Seite die eigene war.

    ── WARUM DAS EIN UI-BEIN IST, OHNE EIN UI-ELEMENT ZU BAUEN ──────────────

    Die Kante liegt im TYP. Eine UI, die die Reihenfolge kennt und sich nur
    verpflichtet, sie nicht zu zeigen, ist kein Blindvergleich: ein Tooltip,
    ein Debugausdruck oder eine spaetere Sortierung reichen. Dieses Bein
    misst, dass es keinen Weg an der Sperre vorbei gibt — nicht, dass eine
    Flaeche richtig aussieht.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/Blindvergleich.h"

#include <iostream>
#include <vector>

using nakama::analyse::Blindreihenfolge;
using nakama::analyse::Blindvergleich;
using nakama::analyse::Hoerurteil;
using nakama::analyse::sperrWort;
using nakama::analyse::Urteilsperre;
using nakama::analyse::Vergleichspegel;

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
    ok ? ++bestanden : ++fehler;
}

void abschnitt (const char* name)
{
    std::cout << std::endl << "== " << name << " ==" << std::endl;
}

/** Ein eingefrorener Vergleichspegel aus echtem Material. */
Vergleichspegel eingefrorenerPegel (double faktor = 2.0)
{
    constexpr double fs = 48000.0;
    constexpr int block = 512;
    Vergleichspegel p;
    p.vorbereiten (fs);
    std::vector<float> a ((std::size_t) block), b ((std::size_t) block);
    for (int i = 0; i < 48; ++i)
    {
        for (int k = 0; k < block; ++k)
        {
            const double t = (double) (i * block + k) / fs;
            const double x = 0.3 * std::sin (6.283185307179586 * 220.0 * t);
            a[(std::size_t) k] = (float) x;
            b[(std::size_t) k] = (float) (x * faktor);
        }
        p.speise (a.data(), b.data(), block);
    }
    p.friereEin();
    return p;
}

// ═════════════════════════════════════════════════════════════════════════
// M-43 · no_sound_verdict_before_loudness_match
// ═════════════════════════════════════════════════════════════════════════
void noSoundVerdictBeforeLoudnessMatch()
{
    abschnitt ("M-43  no_sound_verdict_before_loudness_match");

    Blindvergleich v;
    pruefe (! v.lautheitAbgeglichen() && ! v.geurteilt(),
            "frisch: kein Abgleich, kein Urteil");
    pruefe (v.sperre() == Urteilsperre::ohneLautheitsabgleich,
            "und die Sperre nennt genau diesen Grund", sperrWort (v.sperre()));

    // Auch mit gebundener Reihenfolge bleibt sie zu - der Abgleich ist die
    // haertere Bedingung und kommt deshalb zuerst.
    pruefe (v.bindeReihenfolge (Blindreihenfolge::kandidatZuerst),
            "die Reihenfolge laesst sich schon vorher binden");
    pruefe (v.sperre() == Urteilsperre::ohneLautheitsabgleich,
            "die Sperre bleibt trotzdem der fehlende Abgleich", sperrWort (v.sperre()));
    pruefe (! v.urteile (Hoerurteil::kandidat),
            "und ein Urteil wird NICHT angenommen - lauter klingt besser, und "
            "ein A/B ohne Pegelabgleich misst genau das");
    pruefe (! v.geurteilt(), "es steht also auch keines da");

    // Ein NICHT eingefrorener Pegel zaehlt nicht: „gerade am Messen" ist
    // kein Abgleich.
    Vergleichspegel laufend;
    laufend.vorbereiten (48000.0);
    pruefe (! v.uebernimmVergleichspegel (laufend),
            "ein noch nicht eingefrorener Pegel wird abgelehnt");
    pruefe (! v.lautheitAbgeglichen(), "und aendert nichts");

    // Mit eingefrorenem Pegel geht es.
    const auto p = eingefrorenerPegel();
    pruefe (v.uebernimmVergleichspegel (p), "ein eingefrorener Pegel wird uebernommen");
    pruefe (v.lautheitAbgeglichen(), "der Abgleich steht");
    pruefe (v.sperre() == Urteilsperre::frei, "und die Sperre ist frei",
            sperrWort (v.sperre()));
    pruefe (v.urteile (Hoerurteil::kandidat), "jetzt wird das Urteil angenommen");
    pruefe (v.geurteilt() && v.holeUrteil() == Hoerurteil::kandidat,
            "und steht unveraendert da");

    // Ein zweites Urteil deutete das erste um.
    pruefe (! v.urteile (Hoerurteil::baseline), "ein zweites Urteil wird abgelehnt");
    pruefe (v.holeUrteil() == Hoerurteil::kandidat, "das erste bleibt stehen");
    pruefe (v.sperre() == Urteilsperre::schonGeurteilt, "mit benanntem Grund",
            sperrWort (v.sperre()));

    // Nach dem Urteil aendert auch ein neuer Pegel nichts mehr: der Pegel
    // gehoert zum Versuch, und ein spaeter neu gemessener darf ein
    // gefaelltes Urteil nicht ruecklaeufig entwerten.
    pruefe (! v.uebernimmVergleichspegel (eingefrorenerPegel (4.0)),
            "und ein neuer Pegel wird nach dem Urteil nicht mehr uebernommen");

    // Der Passagenwechsel raeumt alles ab - der Gegenpfad gehoert in
    // denselben Aenderungssatz.
    v.loeschen();
    pruefe (! v.geurteilt() && ! v.lautheitAbgeglichen() && ! v.reihenfolgeGebunden(),
            "der Passagenwechsel raeumt Urteil, Abgleich und Bindung ab");
    pruefe (v.sperre() == Urteilsperre::ohneLautheitsabgleich,
            "und der Automat steht wieder am Anfang");
}

// ═════════════════════════════════════════════════════════════════════════
// M-44 · ui_cannot_read_the_order_before_the_verdict
// ═════════════════════════════════════════════════════════════════════════
void uiCannotReadTheOrderBeforeTheVerdict()
{
    abschnitt ("M-44  ui_cannot_read_the_order_before_the_verdict");

    Blindvergleich v;
    Blindreihenfolge gelesen = Blindreihenfolge::baselineZuerst;

    pruefe (! v.aufgedeckteReihenfolge (gelesen),
            "ungebunden: es gibt nichts zu lesen");
    pruefe (v.bindeReihenfolge (Blindreihenfolge::kandidatZuerst), "gebunden");
    pruefe (v.reihenfolgeGebunden(),
            "die UI darf WISSEN, dass ein Blindvergleich laeuft");
    pruefe (! v.aufgedeckteReihenfolge (gelesen),
            "sie darf aber NICHT wissen, welche Seite welche ist - das ist die "
            "Kante aus M-44");

    // Der Rueckgabewert ist ein `bool`, und `aus` bleibt unberuehrt. Ein
    // Rueckgabewert, den man versehentlich als Reihenfolge lesen kann, waere
    // genau das Leck, das dieser Typ schliesst.
    gelesen = Blindreihenfolge::baselineZuerst;
    v.aufgedeckteReihenfolge (gelesen);
    pruefe (gelesen == Blindreihenfolge::baselineZuerst,
            "und der Ausgabeparameter bleibt unberuehrt, statt still einen Wert "
            "zu tragen, den jemand liest");

    // Ein zweiter Bindeversuch dreht sie nicht.
    pruefe (! v.bindeReihenfolge (Blindreihenfolge::baselineZuerst),
            "ein zweites Binden wird abgelehnt - sonst liesse sie sich nach dem "
            "Hoeren noch zum Urteil passend drehen");

    // Erst das Urteil deckt auf, und zwar GENAU die gebundene.
    const auto p = eingefrorenerPegel();
    v.uebernimmVergleichspegel (p);
    pruefe (v.urteile (Hoerurteil::baseline), "das Urteil faellt");
    pruefe (v.aufgedeckteReihenfolge (gelesen) && gelesen == Blindreihenfolge::kandidatZuerst,
            "jetzt ist sie aufgedeckt - und es ist die gebundene, keine andere");

    // Die Gegenprobe mit der anderen Reihenfolge, sonst misst der Fall oben
    // nur einen Vorgabewert.
    Blindvergleich w;
    w.bindeReihenfolge (Blindreihenfolge::baselineZuerst);
    w.uebernimmVergleichspegel (p);
    w.urteile (Hoerurteil::kandidat);
    Blindreihenfolge zweite = Blindreihenfolge::kandidatZuerst;
    pruefe (w.aufgedeckteReihenfolge (zweite) && zweite == Blindreihenfolge::baselineZuerst,
            "die andere Reihenfolge kommt ebenso zurueck - der Wert ist gebunden, "
            "nicht geraten");

    // Ohne Bindung gibt es kein Urteil (der Automat laesst es gar nicht zu),
    // also auch nichts aufzudecken.
    Blindvergleich x;
    x.uebernimmVergleichspegel (p);
    pruefe (x.sperre() == Urteilsperre::reihenfolgeNichtGebunden,
            "ohne Bindung nennt die Sperre genau das", sperrWort (x.sperre()));
    pruefe (! x.urteile (Hoerurteil::keinUnterschied), "und das Urteil wird abgelehnt");
    Blindreihenfolge nichts = Blindreihenfolge::baselineZuerst;
    pruefe (! x.aufgedeckteReihenfolge (nichts), "es gibt nichts aufzudecken");

    // Die zwei Seitennummern erlauben der UI, zwei Schaltflaechen
    // auseinanderzuhalten, ohne die Identitaet zu verraten.
    pruefe (Blindvergleich::seiteA != Blindvergleich::seiteB,
            "die UI kann zwei Seiten unterscheiden, ohne zu wissen, welche welche ist");

    // Jede Sperre traegt ein Wort - ein gesperrter Zustand ohne Grund waere
    // ein totes Element.
    for (auto s : { Urteilsperre::ohneLautheitsabgleich, Urteilsperre::reihenfolgeNichtGebunden,
                    Urteilsperre::schonGeurteilt, Urteilsperre::frei })
        pruefe (juce::String (sperrWort (s)).isNotEmpty()
                    && juce::String (sperrWort (s)) != "unbekannt",
                juce::String ("die Sperre traegt ein Wort: ") + sperrWort (s));
}

} // namespace

int main()
{
    std::cout << "== Nakama SONDE-013 - die zwei Kanten des Blindvergleichs (§43.1) =="
              << std::endl;
    noSoundVerdictBeforeLoudnessMatch();
    uiCannotReadTheOrderBeforeTheVerdict();
    std::cout << std::endl << bestanden << " bestanden, " << fehler << " gescheitert"
              << std::endl;
    return fehler == 0 ? 0 : 1;
}
