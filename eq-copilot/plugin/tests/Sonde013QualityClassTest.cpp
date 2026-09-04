/*  EqCopSonde013QualityClassTest — die Gesamtklasse nach §34.3
    (SONDE-013 M-06).

    Der Satz, den dieses Bein misst, steht im Entwurf woertlich:

        "Eine Gesamtklasse wird nicht aus einem schoenen Mittelwert gerettet:
         ein harter Mangel bei Session, Passage, Coverage oder Alignment
         begrenzt die Gesamtaussage."

    ── WARUM DAS EIN EIGENES BEIN BRAUCHT ───────────────────────────────────

    Weil die Zusage eine ueber ALLE Faelle ist und nicht ueber ein Beispiel.
    `gesamtklasse` ist `constexpr` und hat genau zehn Eingaben, davon acht
    binaer — also laesst sich der Raum der harten Maengel VOLLSTAENDIG
    abfahren (256 Kombinationen) und die Zusage als Invariante pruefen statt
    an fuenf handverlesenen Zeilen. Genau das tut der letzte Abschnitt.

    Die Zusage in ihrer scharfen Form lautet: es gibt KEINE Kombination, in
    der ein zusaetzlicher harter Mangel die Klasse HEBT. Ein Mittelwert
    koennte das - ein Deckel nicht. Wer die Regel je auf einen Score
    umstellte, faellt hier.
*/

#include <juce_core/juce_core.h>

#include "../core/analysis/Konfidenz.h"

#include <iostream>
#include <string>

using nakama::analyse::Konfidenzklasse;
using nakama::analyse::Konfidenzlage;
using nakama::analyse::gesamtklasse;
using nakama::analyse::klasseName;

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

/** Eine Lage, in der ALLES bekannt und ALLES gut ist — der einzige
    Ausgangspunkt, von dem aus `stark` ueberhaupt erreichbar ist. */
Konfidenzlage volleLage()
{
    Konfidenzlage l;
    l.sessionBekannt = l.passageBekannt = l.coverageBekannt = l.alignmentBekannt = true;
    l.abdeckungGesetzt = true;
    l.abdeckung = 1.0f;
    l.verteilungFenster = 64;
    return l;
}
} // namespace

int main()
{
    std::cout << "== Nakama SONDE-013 M-06 - Gesamtklasse nach §34.3 ==" << std::endl;

    // ── Die Ordnung selbst ────────────────────────────────────────────────
    //
    // Ohne sie ist „deckeln" bedeutungslos. Sie steht als erste Pruefung,
    // weil jede weitere davon abhaengt.
    abschnitt ("Die vier Klassen sind geordnet - `deckeln` ist ein Minimum");
    {
        pruefe (Konfidenzklasse::unbrauchbar < Konfidenzklasse::schwach
                  && Konfidenzklasse::schwach < Konfidenzklasse::mittel
                  && Konfidenzklasse::mittel < Konfidenzklasse::stark,
                "unbrauchbar < schwach < mittel < stark");
        pruefe (std::string (klasseName (Konfidenzklasse::stark)) == "stark"
                  && std::string (klasseName (Konfidenzklasse::mittel)) == "mittel"
                  && std::string (klasseName (Konfidenzklasse::schwach)) == "schwach"
                  && std::string (klasseName (Konfidenzklasse::unbrauchbar)) == "unbrauchbar",
                "und ihre Namen sind genau die vier des Vertrags (§defs/konfidenz)");
    }

    // ── hard_deficiency_caps_overall_class ────────────────────────────────
    abschnitt ("M-06  hard_deficiency_caps_overall_class");
    {
        auto gut = volleLage();
        pruefe (gesamtklasse (gut) == Konfidenzklasse::stark,
                "eine Lage ohne jeden Mangel und mit voller Abdeckung ist `stark` - "
                "sonst koennte der Test unten nichts zeigen",
                klasseName (gesamtklasse (gut)));

        // Jeder der vier harten Maengel EINZELN.
        const struct { const char* name; bool Konfidenzlage::* bit; } quellen[] = {
            { "Session",   &Konfidenzlage::sessionHart },
            { "Passage",   &Konfidenzlage::passageHart },
            { "Coverage",  &Konfidenzlage::coverageHart },
            { "Alignment", &Konfidenzlage::alignmentHart },
        };
        for (const auto& q : quellen)
        {
            auto l = volleLage();
            l.*(q.bit) = true;
            const auto k = gesamtklasse (l);
            pruefe (k == Konfidenzklasse::schwach,
                    juce::String ("ein harter Mangel bei ") + q.name
                    + " deckelt auf `schwach` - er wird NICHT gegen die drei guten "
                      "Nachbarquellen verrechnet",
                    klasseName (k));
        }

        // Zwei Maengel: unbrauchbar. Das ist die scharfe Form des Satzes.
        for (const auto& a : quellen)
            for (const auto& b : quellen)
            {
                if (a.bit == b.bit) continue;
                auto l = volleLage();
                l.*(a.bit) = true;
                l.*(b.bit) = true;
                if (gesamtklasse (l) != Konfidenzklasse::unbrauchbar)
                {
                    pruefe (false,
                            juce::String ("zwei harte Maengel (") + a.name + " + "
                            + b.name + ") ergeben `unbrauchbar`",
                            klasseName (gesamtklasse (l)));
                    goto zweiFertig;
                }
            }
        pruefe (true, "zwei harte Maengel ergeben `unbrauchbar` - in ALLEN zwoelf "
                      "Paarungen");
        zweiFertig: ;
    }

    // ── Was der Erzeuger nicht weiss, deckelt auch ────────────────────────
    //
    // Der zweite Teil von §34.3, und der Grund, warum eine Sonde nie `stark`
    // meldet: sie kennt von den vier Quellen genau eine.
    abschnitt ("M-06  was nicht beurteilbar ist, deckelt auf `mittel`");
    {
        const struct { const char* name; bool Konfidenzlage::* bit; } bekannt[] = {
            { "Session",   &Konfidenzlage::sessionBekannt },
            { "Passage",   &Konfidenzlage::passageBekannt },
            { "Coverage",  &Konfidenzlage::coverageBekannt },
            { "Alignment", &Konfidenzlage::alignmentBekannt },
        };
        for (const auto& q : bekannt)
        {
            auto l = volleLage();
            l.*(q.bit) = false;
            const auto k = gesamtklasse (l);
            pruefe (k == Konfidenzklasse::mittel,
                    juce::String ("wer ") + q.name + " nicht beurteilen kann, kommt "
                    "hoechstens auf `mittel`",
                    klasseName (k));
        }

        // Die Lage einer SONDE: sie kennt nur ihre Coverage. Genau das ist
        // die Regel hinter dem Deckel, den Etappe B als Literal gesetzt hat.
        Konfidenzlage sonde;
        sonde.coverageBekannt = true;
        sonde.abdeckungGesetzt = true;
        sonde.abdeckung = 1.0f;
        sonde.verteilungFenster = 64;
        pruefe (gesamtklasse (sonde) == Konfidenzklasse::mittel,
                "eine Sonde mit PERFEKTER Abdeckung kommt trotzdem nur auf `mittel` - "
                "sie sieht Session, Passage und Alignment nicht, und `stark` waere eine "
                "Aussage ueber drei Dinge, die sie nicht kennt",
                klasseName (gesamtklasse (sonde)));
    }

    // ── Die Basis: ohne Abdeckung keine Aussage ───────────────────────────
    abschnitt ("M-06  die Basis kommt aus der gemessenen Abdeckung");
    {
        auto ohne = volleLage();
        ohne.abdeckungGesetzt = false;
        pruefe (gesamtklasse (ohne) == Konfidenzklasse::unbrauchbar,
                "ohne Abdeckungsbit ist die Aussage `unbrauchbar` - nicht weil nichts "
                "da waere, sondern weil niemand weiss, ob etwas da war",
                klasseName (gesamtklasse (ohne)));

        auto null = volleLage();
        null.abdeckung = 0.0f;
        pruefe (gesamtklasse (null) == Konfidenzklasse::unbrauchbar,
                "und bei Abdeckung 0 ebenfalls");

        // Die zwei Schwellen, je an ihrer Kante und knapp darunter (Muster C-09).
        const struct { float wert; Konfidenzklasse erwartet; const char* was; } kanten[] = {
            { 0.80f, Konfidenzklasse::stark,   "genau an der Starkschwelle" },
            { 0.79f, Konfidenzklasse::mittel,  "knapp darunter" },
            { 0.50f, Konfidenzklasse::mittel,  "genau an der Mittelschwelle" },
            { 0.49f, Konfidenzklasse::schwach, "knapp darunter" },
        };
        for (const auto& k : kanten)
        {
            auto l = volleLage();
            l.abdeckung = k.wert;
            pruefe (gesamtklasse (l) == k.erwartet,
                    juce::String ("Abdeckung ") + juce::String (k.wert, 2) + " ("
                    + k.was + ") ergibt `" + klasseName (k.erwartet) + "`",
                    klasseName (gesamtklasse (l)));
        }
    }

    // ── Die duenne Verteilung ─────────────────────────────────────────────
    abschnitt ("M-06  eine duenne Verteilung traegt keine starke Aussage");
    {
        auto duenn = volleLage();
        duenn.verteilungFenster = nakama::analyse::kKonfidenzFensterStark - 1;
        pruefe (gesamtklasse (duenn) == Konfidenzklasse::mittel,
                "unter der Fensterschwelle ist hoechstens `mittel` moeglich, auch bei "
                "voller Abdeckung und ohne jeden Mangel",
                klasseName (gesamtklasse (duenn)));

        auto genug = volleLage();
        genug.verteilungFenster = nakama::analyse::kKonfidenzFensterStark;
        pruefe (gesamtklasse (genug) == Konfidenzklasse::stark,
                "genau an der Schwelle ist `stark` wieder moeglich - die Grenze faellt "
                "am richtigen Punkt (Muster C-09)",
                klasseName (gesamtklasse (genug)));
    }

    // ── DIE INVARIANTE UEBER DEN GANZEN RAUM ──────────────────────────────
    //
    // Die eigentliche Zusage von §34.3 ist keine ueber fuenf Beispiele,
    // sondern eine ueber ALLE Kombinationen: ein zusaetzlicher harter Mangel
    // darf die Klasse NIE heben. Ein Mittelwert koennte das (ein Mangel bei
    // einer Quelle, deren Gewicht klein ist, zoege den Schnitt kaum), ein
    // Deckel nicht. Wer die Regel je auf einen Score umstellt, faellt hier.
    abschnitt ("M-06  Invariante: ein zusaetzlicher Mangel hebt die Klasse NIE");
    {
        bool Konfidenzlage::* const hart[] = {
            &Konfidenzlage::sessionHart, &Konfidenzlage::passageHart,
            &Konfidenzlage::coverageHart, &Konfidenzlage::alignmentHart,
        };
        bool Konfidenzlage::* const bekannt[] = {
            &Konfidenzlage::sessionBekannt, &Konfidenzlage::passageBekannt,
            &Konfidenzlage::coverageBekannt, &Konfidenzlage::alignmentBekannt,
        };

        int geprueft = 0, verletzt = 0;
        // Alle 256 Kombinationen aus vier `bekannt`- und vier `hart`-Bits,
        // dreimal mit verschiedener Abdeckung und Fensterzahl.
        const struct { float abd; std::uint32_t fenster; } lagen[] = {
            { 1.00f, 64 }, { 0.65f, 16 }, { 0.30f, 4 },
        };
        for (const auto& basis : lagen)
            for (int maske = 0; maske < 256; ++maske)
            {
                Konfidenzlage l;
                l.abdeckungGesetzt = true;
                l.abdeckung = basis.abd;
                l.verteilungFenster = basis.fenster;
                for (int i = 0; i < 4; ++i)
                {
                    l.*(bekannt[i]) = (maske & (1 << i)) != 0;
                    l.*(hart[i])    = (maske & (1 << (i + 4))) != 0;
                }
                const auto vorher = gesamtklasse (l);

                // Jeden noch nicht gesetzten harten Mangel EINZELN dazu.
                for (int i = 0; i < 4; ++i)
                {
                    if (l.*(hart[i]))
                        continue;
                    auto mehr = l;
                    mehr.*(hart[i]) = true;
                    ++geprueft;
                    if (gesamtklasse (mehr) > vorher)
                        ++verletzt;
                }
            }
        pruefe (geprueft > 1000,
                "der Raum ist wirklich abgefahren - alle 256 Bitkombinationen in drei "
                "Basislagen",
                juce::String (geprueft) + " Uebergaenge");
        pruefe (verletzt == 0,
                "und in KEINEM davon hebt ein zusaetzlicher harter Mangel die Klasse. "
                "Das ist §34.3 als Invariante statt als Beispiel - ein Mittelwert "
                "koennte diese Zusage nicht halten",
                juce::String (verletzt) + " Verletzungen");

        // Gegenprobe: die Invariante ist nicht trivial erfuellt, weil die
        // Klasse ueberhaupt variiert. Ohne sie waere „hebt nie" auch dann
        // gruen, wenn die Funktion konstant `unbrauchbar` liefaere.
        int verschiedene = 0;
        bool gesehen[4] = { false, false, false, false };
        for (const auto& basis : lagen)
            for (int maske = 0; maske < 256; ++maske)
            {
                Konfidenzlage l;
                l.abdeckungGesetzt = true;
                l.abdeckung = basis.abd;
                l.verteilungFenster = basis.fenster;
                for (int i = 0; i < 4; ++i)
                {
                    l.*(bekannt[i]) = (maske & (1 << i)) != 0;
                    l.*(hart[i])    = (maske & (1 << (i + 4))) != 0;
                }
                gesehen[(int) gesamtklasse (l)] = true;
            }
        for (bool g : gesehen)
            if (g) ++verschiedene;
        pruefe (verschiedene == 4,
                "und alle VIER Klassen kommen im Raum wirklich vor - die Invariante ist "
                "nicht dadurch erfuellt, dass die Funktion immer dasselbe liefert",
                juce::String (verschiedene) + " von 4");
    }

    std::cout << "\n-----------------------------------------" << std::endl;
    std::cout << bestanden << " bestanden, " << fehler << " gescheitert" << std::endl;
    return fehler == 0 ? 0 : 1;
}
