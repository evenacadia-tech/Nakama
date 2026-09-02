# 2026-09-02 — Technische UI-Architektur, Fünferblock 05 (delegierte Ableitung, gemeinsame Regeln)

## Status

**Zwei Entscheidungen abgeleitet, gemeinsamer Umbau mit Block 04.** Grundlage
ist derselbe User-Auftrag vom 02.09.2026 wie in
[`2026-09-02-technische-ui-architektur-fuenferblock-04.md`](2026-09-02-technische-ui-architektur-fuenferblock-04.md):

> „ich habe gerade keine zeit, kannst du nicht die restlichen offenen UX
> entscheidungen anhand von best practice beispielen lösen ? es gibt ja
> manuals , handbücher mit denen du gute menschliche Bedienprinzipien super
> nachvollziehen und anwenden kannst“

Der Block bleibt mit zwei Entscheiden kürzer als fünf, weil damit die
offenen Bedienfragen aus Blueprint Abschnitt 15 und dem Skizzen-Leseblatt
erschöpft sind; ein Block wird nicht mit erfundenen Fragen aufgefüllt.

## 1/2 — Tastatur, Fokus und Radbesitz über beide Flächen

**Belegte Muster.** W3C-APG *Tabs*: `Left/Right Arrow` wechseln den Fokus
zwischen den Reitern, `Home/End` springen an Anfang und Ende, „Space or
Enter: Activates the tab if it was not activated automatically on focus“;
`aria-selected` markiert den aktiven Reiter. W3C-APG *Disclosure*: `Enter`
und `Space` schalten den Inhalt, `aria-expanded` und `aria-controls` binden
Knopf und Inhalt. W3C-APG *Menu Button*: Öffnen setzt den Fokus in das
erste Element; nach dem Schließen kehrt der Fokus zum Knopf zurück. Pro-Q 4:
„Perhaps the easiest way to make adjustments is by using the mouse wheel
when you hover over a knob“ (unter Windows nach einem Klick in die
Oberfläche), `Shift` für Feinschritte, `Ctrl`-Klick setzt auf den
Standardwert zurück, Doppelklick öffnet die Texteingabe mit Kurzformen wie
„1k“; im Display ändert das Rad die Güte der ausgewählten Bänder.
Kirchhoff-EQ: „drag or use mouse wheel to adjust“ an jedem Wertknopf, Rad
auf dem Typ-Symbol wechselt den Filtertyp. Der Entscheid vom 23.08.2026:
Das Rad wechselt die Sonde nur, wenn nichts per Klick markiert ist.

**Entscheidung für beide Gen-Flächen:**

- **Kopf-Umschalter = Tabs-Muster mit manueller Aktivierung.** `Tab` führt
  auf den aktiven Reiter; `Left/Right` bewegen den Fokus zwischen
  `OVERVIEW` und `EQ`, `Home/End` an die Enden; `Enter` oder `Space`
  wechseln die Fläche. Der Wechsel folgt nicht dem Fokus, weil er eine
  ganze Fläche tauscht.
- **Jede Disclosure folgt einer Regel:** `Enter`/`Space` öffnen und
  schließen; der Einstieg trägt `aria-expanded` und `aria-controls`;
  `Escape` schließt das oberste nichtmodale Panel und gibt den Fokus an
  seinen Einstieg zurück (Global-Sektion an `GLOBAL`, Preset-Liste an
  `PRESET`, Band-Panel an den Bandpunkt, Kopie-Wahl an `COPY →`). Eine
  laufende Texteingabe verwirft `Escape` zuerst (Block 01, 4/5).
- **Radbesitz:** Das Rad gehört dem Wert, über dem der Zeiger steht: über
  einem Wertefeld ändert es diesen Wert in Schritten, mit `Shift` fein;
  über dem Graphen mit ausgewähltem Band ändert es die Güte des Bands; über
  dem Graphen ohne Auswahl wechselt es die Sonde (23.08.2026); über der
  Sources-Liste scrollt es die Liste. `Ctrl`-Klick auf ein Wertefeld setzt
  den Standardwert. Doppelklick öffnet die Texteingabe; `Enter` übernimmt,
  `Escape` verwirft.
- **Fokusordnung:** Kopf, Zielleiste beziehungsweise Sources-Spalte,
  Arbeitsfläche, untere Zeile. Ein geöffnetes Panel liegt in der Ordnung
  direkt hinter seinem Einstieg.
- Die Blueprint-Hypothese **[A] Radbesitz** aus Abschnitt 4.2 wird damit
  **[U]** im Sinn dieser delegierten Ableitung.

## 2/2 — Fehler- und Degradationszustände über beide Flächen

**Belegte Muster.** Nielsen, Heuristik 1: „keep users informed about what
is going on“; NN/g: Indikatoren sind kontextuell und passiv, Validierungen
hängen an der Eingabe, Benachrichtigungen gelten Ereignissen ohne
Nutzerauslösung; „a critical error should never use a dismissible toast
notification, nor should routine feedback interrupt users with modal
dialogs“. NN/g-Fehlerregeln: nahe an der Quelle, redundant und zugänglich,
konstruktiv, Eingabe erhalten. Pro-Q 4 zeigt Bypass eines Bands als
„dimmed in the display and a red light glows in the bypass button“, also
doppelt.

**Entscheidung für beide Gen-Flächen und Probeeq:**

- **Systemwahrheit steht im Kopf**, dauerhaft und passiv: Verbindung
  (`SESSION LINKED` / `SESSION LINK LOST`), Autorität und Revision. Sie
  wechselt nie in einen Dialog.
- **Ein degradiertes Objekt bleibt an seinem Ort und trägt seinen
  Zustand** (Quelle `STALE`, `PARTIAL`, `LOW SIGNAL`; Befund `MORE DATA`,
  `BLOCKED · LINK`; Band `OFF`; Ziel `LINK LOST`). Nichts verschwindet,
  nichts wird nur farblich markiert.
- **Validierung sitzt am Feld:** Eine unbrauchbare Eingabe wird verworfen,
  der alte Wert bleibt, und eine kurze Zeile am Feld sagt es (`WIDTH ·
  KEPT 100 %`); keine Sperre, kein Dialog.
- **Ereignisse ohne Nutzerauslösung** (Link-Verlust, ausgelaufener
  Kandidat, blockierte Fernänderung) erscheinen als Hinweiszeile am
  betroffenen Ort (Graph-Hinweis, Transaktionszeile, Probeeq-Zeile 2) mit
  dem nächsten Schritt; sie sind nicht wegklickbar, sondern enden mit dem
  Zustand.
- **Modale Dialoge gibt es nicht.** Die einzige zweistufige Handlung ist
  die Bestätigung einer überschreibenden Aktion (Preset überschreiben,
  Kurve kopieren, `APPLY` als 10-s-Kandidat), und sie läuft im selben
  Körper wie die Aktion.

## Quellenledger

Siehe das Ledger von Fünferblock 04; zusätzlich für diesen Block dieselben
W3C-APG-Seiten (Tabs, Disclosure, Menu Button, Combobox) und die
NN/g-Artikel (Heuristiken, Error-Message Guidelines, Indicators/Validations/
Notifications).

## Gemeinsamer Umbau und Sichtbeleg (Block 04 und 05)

Wird nach dem Umbau der Skizze hier ergänzt.
