#pragma once

/*  S9 / SONDE-007b Abschnitt 3 - Lifecycle-Klassifikation (Entwurf §53.5).

    DER SATZ, DEN DIESE DATEI ZU CODE MACHT:

      "Der bestehende Entry bleibt beim Laden zunaechst `unclassified` und
       audio-neutral. Erst nach vollstaendigem State-Restore gilt:
         - Schema-1-`sensor|pre|post` -> `legacy`, immer passiv;
         - Schema-1-`hub` oder bestaetigter Schema-2-Main-State -> `main`;
         - leerer, nie gespeicherter Altstate -> Main erst nach geoeffnetem
           Editor und expliziter Initialisierung; ein Scannerlauf
           klassifiziert nicht;
         - die beiden neuen Bundles haben eine feste Produktklasse, bleiben
           aber bis gueltigem State neutral.
       Alle Instanzen duerfen im Worker nur verbinden. Ausschliesslich ein
       positiv klassifiziertes Main mit geoeffnetem Editor darf den
       installierten Broker starten. Klassifikation, Spawn und Pipe-I/O
       liegen nie im Audiocallback."

    ZWEI DINGE, DIE HIER BEWUSST GETRENNT BLEIBEN:

      PRODUKTKLASSE  - steht fest am Bundle (Eqcp laedt {main, legacy};
                       NkPr ist passive_probe; NkAc ist active_probe). Sie
                       kommt aus der duennen Target-Schicht, nie aus dem
                       gemeinsamen Code (§53.4).
      KLASSIFIKATION - entsteht erst aus dem restaurierten Stand. Sie ist das,
                       was dieser Automat haelt.

    Fuer Gen sind beide verschieden (das Bundle kann main ODER legacy sein);
    fuer Suna und Probeeq faellt die Klassifikation nach gueltigem State mit
    der Produktklasse zusammen - aber erst DANN, und das ist der Punkt des
    letzten §53.5-Aufzaehlungspunkts.

    WARUM "ein Scannerlauf klassifiziert nicht" hier keine eigene Regel ist:
    Ein Scanner instanziiert, fragt Busse und Parameter ab und zerstoert
    wieder - er ruft nie `setStateInformation`. Der Automat startet auf
    `unclassified` und bekommt nie ein Ereignis. Die Regel ist damit eine
    FOLGE der Startbedingung, keine Sonderbehandlung; der Lebenslauftest
    faehrt genau diese Sequenz und misst sie.

    KEINE JucePlugin_*-Konstante, nur juce_core/juce_data_structures ueber
    NakamaState.h - diese Datei gehoert in den gemeinsamen Kern (S8) und
    faellt unter K1/K2/K3. Kein Audiothread: der Automat wird vom
    Nachrichten-/Hostthread gefuehrt, der Audiothread liest nur eine
    Atomic-Spiegelung im Prozessor.
*/

#include "NakamaState.h"

namespace nakama::state
{

/** §53.5: die vier Produktklassen plus den Zustand VOR jeder Aussage.

    `unclassified` ist kein Fehler und kein Uebergangsflackern, sondern der
    ehrliche Normalfall einer frisch geladenen Instanz: niemand hat bisher
    gesagt, was sie ist. */
enum class Klassifikation { unclassified, main, legacy, passive_probe, active_probe };

const char* wort (Klassifikation k);

/** Die Zustandsmaschine aus §53.5. Reine Regelarbeit: kein Audio, kein
    Pipe-I/O, keine Datei, keine Allokation. */
class Lebenslauf
{
public:
    /** Das Main-Bundle (Eqcp): keine feste Produktklasse - es klassifiziert
        sich ausschliesslich aus dem restaurierten Stand. */
    Lebenslauf() noexcept = default;

    /** Die beiden neuen Bundles (NkPr/NkAc): feste Produktklasse, aber
        neutral bis gueltigem State (§53.5, letzter Aufzaehlungspunkt).
        `main` und `legacy` sind hier unzulaessig - sie waeren die Behauptung,
        ein Sondenbundle koenne das Main sein. */
    explicit Lebenslauf (Klasse festeProduktklasse) noexcept;

    Klassifikation klassifikation() const noexcept { return zustand; }

    /** Neutral heisst: keine Audio-Ausnahme, kein Brokerstart, keine
        Produktaussage. Verbinden darf eine neutrale Instanz sehr wohl. */
    bool istNeutral() const noexcept { return zustand == Klassifikation::unclassified; }
    bool istMain()    const noexcept { return zustand == Klassifikation::main; }
    bool editorIstOffen() const noexcept { return editor; }

    /** Der Host hat einen Stand restauriert. `ergebnis` und `z` kommen
        unveraendert aus `lade()` - dieser Automat liest keine Bytes selbst.

        `ignoriert` (fremder Baumtyp/Muell) ist KEIN Restore: der Automat
        bleibt stehen. `nurLesen` faellt auf `unclassified` zurueck - ein
        Stand, den dieser Build nicht interpretieren darf, ist das Gegenteil
        eines "vollstaendigen State-Restore", und ein zuvor klassifizierter
        Entry darf seine Rechte nicht behalten, wenn der Host ihm einen
        unlesbaren Stand nachreicht. */
    void stateRestauriert (LadeErgebnis ergebnis, const Zustand& z) noexcept;

    /** Editor auf/zu. Klassifiziert fuer sich allein NICHTS (§53.5 verlangt
        Editor UND explizite Initialisierung) - aber der Brokerstart haengt
        daran. */
    void editorOffen (bool offen) noexcept { editor = offen; }

    /** §53.5, dritter Aufzaehlungspunkt: die "explizite Initialisierung".

        Der User hat im GEOEFFNETEN Editor die Bindung gesetzt; `z` ist der
        Stand DANACH. Das ist der einzige Akt, der einen nie gespeicherten
        Entry zu `main` machen darf. Ein Scannerlauf kann ihn nicht ausloesen:
        er oeffnet keinen Editor und bedient nichts.

        Umgekehrt gilt derselbe Weg: setzt der User die Rolle von `hub` zurueck
        auf `sensor`, faellt die Klassifikation auf `legacy`. Alles andere
        hiesse, der Automat behauptete etwas anderes als der Stand, den
        dieselbe Instanz im naechsten Projekt-Save schreibt.

        Rueckgabe: true, wenn der Akt gezaehlt hat. */
    bool expliziteInitialisierung (const Zustand& z) noexcept;

    /** §53.5, letzter Satz: "Ausschliesslich ein positiv klassifiziertes Main
        mit geoeffnetem Editor darf den installierten Broker starten."

        Heute gibt es im Plugin keinen Spawn-Pfad (Befund SONDE-007b §1) -
        diese Frage ist der Ort, an dem SONDE-010 ihn haengen MUSS, statt eine
        neue Bedingung zu erfinden. Verbinden ist davon unberuehrt: das duerfen
        alle Instanzen, auch die neutralen. */
    bool darfBrokerStarten() const noexcept
    {
        return zustand == Klassifikation::main && editor;
    }

    /** §53.5, Satz 1: bis zur positiven Klassifikation ist der Entry
        audio-neutral.

        Fuer Gen ist die Hoer-Markierung die EINZIGE Audio-Ausnahme des
        Grundgesetzes (CLAUDE.md, Wahrheitskern) - sie faellt damit unter
        diesen Satz. `legacy` bleibt zu, weil §53.5 es "immer passiv" nennt.
        Diese Frage ersetzt die uebrigen Terme der Markierungs-Verriegelung
        nicht, sie kommt hinzu. */
    bool audioAusnahmeErlaubt() const noexcept { return zustand == Klassifikation::main; }

private:
    Klassifikation aus (Klasse k) const noexcept;

    Klassifikation zustand = Klassifikation::unclassified;
    bool   editor      = false;
    bool   festeKlasse = false;
    Klasse produkt     = Klasse::legacy;   ///< nur gueltig, wenn festeKlasse
};

} // namespace nakama::state
