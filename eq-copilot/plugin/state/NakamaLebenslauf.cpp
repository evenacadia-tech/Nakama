#include "NakamaKernRiegel.h"   // S8/SONDE-007a: K1 — keine JucePlugin_*-Konstante im Kern
#include "NakamaLebenslauf.h"

namespace nakama::state
{

const char* wort (Klassifikation k)
{
    switch (k)
    {
        case Klassifikation::unclassified:  return "unclassified";
        case Klassifikation::main:          return "main";
        case Klassifikation::legacy:        return "legacy";
        case Klassifikation::passive_probe: return "passive_probe";
        case Klassifikation::active_probe:  return "active_probe";
    }
    return "unclassified";
}

Lebenslauf::Lebenslauf (Klasse festeProduktklasse) noexcept
    : festeKlasse (true), produkt (festeProduktklasse)
{
    // Ein Bundle mit fester Produktklasse ist eine SONDE. Waere hier `main`
    // oder `legacy` moeglich, koennte ein Sondenbundle sich zum Main
    // erklaeren. Der Aufrufer garantiert das per static_assert (SondeProcessor.h);
    // faellt die Garantie doch, bleibt dieser Automat dauerhaft neutral -
    // `aus()` gibt fuer main/legacy bei fester Klasse `unclassified` zurueck.
    // Neutral ist die sichere Richtung: keine Rechte, keine Behauptung.
    jassert (festeProduktklasse == Klasse::passive_probe
          || festeProduktklasse == Klasse::active_probe);
}

Klassifikation Lebenslauf::aus (Klasse k) const noexcept
{
    switch (k)
    {
        case Klasse::main:          return festeKlasse ? Klassifikation::unclassified : Klassifikation::main;
        case Klasse::legacy:        return festeKlasse ? Klassifikation::unclassified : Klassifikation::legacy;
        case Klasse::passive_probe: return Klassifikation::passive_probe;
        case Klasse::active_probe:  return Klassifikation::active_probe;
    }
    return Klassifikation::unclassified;
}

void Lebenslauf::stateRestauriert (LadeErgebnis ergebnis, const Zustand& z) noexcept
{
    // `ignoriert`: fremder Baumtyp oder Muell - der Host hat NICHTS
    // restauriert, und der gehaltene Zustand des Prozessors bleibt ebenfalls
    // stehen (PluginProcessor/SondeProcessor kehren an derselben Stelle um).
    // Ein Automat, der hier etwas aenderte, klassifizierte nach einem
    // Ereignis, das gar nicht stattgefunden hat.
    if (ergebnis == LadeErgebnis::ignoriert)
        return;

    // §53.5 verlangt einen VOLLSTAENDIGEN State-Restore. Read-only ist dessen
    // Gegenteil: der Stand liegt vor, darf aber nicht interpretiert werden
    // (§5 des State-Vertrags). Zurueck auf neutral - auch aus einer frueheren
    // positiven Klassifikation. Der Host darf derselben Instanz nacheinander
    // mehrere Staende reichen (Preset-Browser, Copy/Paste); der letzte gilt.
    if (ergebnis == LadeErgebnis::nurLesen || z.nurLesen)
    {
        zustand = Klassifikation::unclassified;
        return;
    }

    if (festeKlasse)
    {
        // §53.5, letzter Aufzaehlungspunkt: feste Produktklasse, aber neutral
        // bis GUELTIGEM State. Ein Stand mit fremder Klasse kann hier nicht
        // ankommen - der Bundlevertrag (§2.3) haette ihn read-only gemacht.
        // Gemessen statt geglaubt: kommt er doch, bleibt die Instanz neutral.
        zustand = (z.common.klasse == produkt) ? aus (produkt)
                                               : Klassifikation::unclassified;
        return;
    }

    // Main-Bundle. Die beiden ersten §53.5-Punkte fallen hier zusammen, und
    // das ist Absicht statt Nachlaessigkeit:
    //   - Schema-1 `sensor|pre|post` hat die Migration bereits nach `legacy`
    //     abgebildet (§32.2), Schema-1 `hub` nach `main`;
    //   - ein Schema-2-Stand mit `main` ist genau der "bestaetigte
    //     Schema-2-Main-State": `lade()` hat die Kind-Matrix §2.1 geprueft und
    //     damit ein `MainProject`-Kind gesehen, sonst waere er read-only.
    // Eine zweite Pruefung hier waere eine Kopie jener Matrix - und Kopien
    // laufen auseinander.
    zustand = aus (z.common.klasse);
}

bool Lebenslauf::expliziteInitialisierung (const Zustand& z) noexcept
{
    // Die beiden neuen Bundles kennen diesen Akt nicht: ihre Produktklasse ist
    // fest, es gibt nichts zu initialisieren (§53.5).
    if (festeKlasse)
        return false;

    // "erst nach geoeffnetem Editor" - woertlich. Das ist zugleich der Riegel
    // gegen den Scannerlauf: ein Scanner oeffnet keinen Editor.
    if (! editor)
        return false;

    // Ein Stand, der nicht interpretiert werden darf, kann auch nicht
    // ausdruecklich initialisiert werden.
    if (z.nurLesen)
        return false;

    const auto neu = aus (z.common.klasse);
    if (neu == Klassifikation::unclassified)
        return false;

    if (neu == zustand)
        return false;   // kein Ereignis: der Akt hat nichts geaendert

    zustand = neu;
    return true;
}

} // namespace nakama::state
