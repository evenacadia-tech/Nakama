# Prüfliste — was an jedem Blatt gemessen wird, bevor es gezeigt wird

Jede Zeile hier entstand aus einem **echten Fehler**, der einmal durchging.
Die Fehler stehen dabei, weil eine Regel ohne ihren Anlass nach Bürokratie
aussieht und dann übersprungen wird.

Diese Liste ist werkzeugunabhängig. Die Messungen wurden am 2026-08-20 mit
Playwright-Skripten ausgeführt; die Skripte waren sitzungslokal und sind weg,
die Regeln nicht.

---

## 1. Ehrlichkeit des Blattes

| Regel | Der Fehler dahinter |
|---|---|
| **Keine Liste darf mehr behaupten, als sie zeigt.** Kopfzahl kommt aus der Messung nach dem Rendern, nie aus dem Quelltext. | Eine Kachel schrieb `11 of 16` in den Kopf und zeigte **fünf** Zeilen. |
| **Der gemessene Zähler braucht eine eigene Form** (`3/5`), wenn im selben Kopf ein geschriebener Text mit Zahl steht. | „finding 1 of 3 · 3 of 5" — zwei gleich aussehende Zähler, zwei Bedeutungen. |
| **Was der Streifen zählt, muss die Karte zeigen.** Eine Quelle der Wahrheit, kein zweiter Zähler. | — |
| **„Ehrlich zeigen" gilt für alles, was aus dem Zustand FOLGT**, nicht nur für die Zustandsanzeige. | Bei 16 getrennten Quellen standen drei Befunde da, als wäre gerade gemessen worden; der Assistent bot Vorhören an; der Streifen meldete „Measuring 68 %". |
| **Ein Knopf, der sich drücken lässt und nichts tut, ist eine Lüge.** | Vorhören auf einer getrennten Quelle. |
| **Ein leeres Feld ohne Erklärung ist ein Ausfall, kein Zustand.** | — |

## 2. Messung von Überlauf und Verdeckung

| Regel | Der Fehler dahinter |
|---|---|
| **Rekursiv über ALLE Nachfahren messen**, nicht über ausgewählte Klassen. | Ein verschachteltes `overflow:hidden` schluckte die Überlänge: gemeldet 36 px, tatsächlich **111**. |
| **Beide Richtungen messen**, Höhe UND Breite. | Der Streifen schnitt rechts ab („Width: unav…") und nichts meldete sich — 18 px in Ruhe, 36 px beim Halten. |
| **Overlays fallen durch jede Überlaufmessung.** Sie brauchen eine eigene Prüfung, die das `::after` wirklich liest. | Ein Band verdeckte 6 px des Assistenten-Kastens; nichts lief über, es lag nur etwas darüber. |
| **Zweimal messen, nicht einmal hoffen.** Auf dem ersten Frame nach großem `innerHTML` stehen die Schriftmaße noch nicht. | Die Trimmschleife hielt eine Zeile zu früh an: 14 px Rest. |
| **Was gemessen wird, muss vorher zurückgesetzt werden.** | Der zweite Durchgang maß bereits gekürzte Inhalte als „natürliche Höhe" — eine Ratsche: Karte wuchs, Arbeitsfläche kürzte weiter. |

## 3. Layout

| Regel | Der Fehler dahinter |
|---|---|
| **Kein Deckel unter der Inhaltshöhe.** Statt Magiewert `max-content`. | Heatmapzeilen mit `min-height:8px` trugen einen 14-px-Namen; Kontextzellen 7 px in einer 5-px-Zeile. |
| **Kästen müssen ihren Rahmen füllen** (`flex:1`). Tote Fläche gilt laut `docs/arbeitsplan.md` als Fehler. | Kästen waren inhaltsbreit statt blattbreit — schmalster 149 px statt 740. Ein 760er Blatt wurde auf 240 px beurteilt. |
| **Kein leerer Kopfbalken.** | Eine Änderung an `kasten()` erzeugte 12 px Balken ohne Text. |
| **Überschuss teilen, Mangel nicht.** Bei Platzmangel hält das dauerhafte Element seine natürliche Höhe. | Die Karte wuchs auf 230 px, obwohl sie 212 braucht — und nahm der Arbeitsfläche genau die Pixel, die dem Assistenten fehlten. |

## 4. Begriffe und Beschriftungen

| Regel | Der Fehler dahinter |
|---|---|
| **Nie zwei Begriffe unter einem Namen.** | `fokus` hieß gleichzeitig „Focus+Context-Behandlung" und „welche Quelle ist betroffen". In der Ruhelage griff die falsche Bedeutung: eine Zeile wurde vierfach hoch. |
| **Listenname = Bausteinname.** Sonst findet der Vertrag die Zahl nicht und meldet stumm nichts. | Zweimal passiert (`Heatmap mit Namen` vs. `Heatmap (mit Namen)`). |
| **Zahlwörter herleiten, nicht schreiben.** | „in allen vier gleich" stand unter drei Kacheln. |
| **Keine Beschriftung darf behaupten, was die Messung widerlegt.** | Eine These sagte „verschwindet aber nie", gemessen blieben 7 von 16. |
| **Skript-Syntax prüfen.** Ein Syntaxfehler macht das Blatt beim Doppelklick **tonlos tot**. | Ein falsches Anführungszeichen in einer Belegtabelle. |

## 5. Was ein Blatt über sich selbst sagen muss

| Regel | Der Fehler dahinter |
|---|---|
| **Ein Blatt sagt, was es ist** — entschieden / eingefroren / Arbeitsannahme / stillgelegt. Sichtbar beim ersten Blick, außerhalb des beurteilten Inhalts, mit Datum und Verweis auf die Abnahme. | Neun Blätter verschwiegen ihren Stand. Drei öffneten beim Doppelklick eine Frage, die längst entschieden war; eines nannte im Kopf die **verworfene** Variante „Deine Wahl“; eines sah aus wie ein gültiges Werkzeug, war aber stillgelegt. Genau die „Ausbessern-Zeremonie“, die `CLAUDE.md` Regel 4 verhindern soll. |
| **Eine Arbeitsannahme wird nie als abgenommen zitiert** — auch nicht beiläufig, auch nicht in einem Kommentar. | Vier Dokumentstellen nannten Welt und Fassung „abgenommen“, eine davon in `abnahmen/` selbst. Das ist der Fehler, aus dem die Unterscheidung überhaupt entstanden ist — ein zweites Mal. |
| **Demo-Daten tragen ihre Marke im Werkzeug-Chrome**, nicht im Spezimen. | `grep -i demo` fand im aktuellen Main-Entwurf null Treffer, obwohl `CLAUDE.md` Regel 6 die Kennzeichnung verlangt. |

## 6. Löschen und Umbauen

| Regel | Der Fehler dahinter |
|---|---|
| **Eine Löschung wird mit einem Fingerabdruck abgesichert**, nicht mit Zuversicht: Verhalten über die volle Zustandsmatrix vorher festhalten (je Kachel Text, Zähler, Überlauf, Vertrag), löschen, neu festhalten, vergleichen. Null Abweichungen oder die Löschung war zu groß. | Eine Dead-Code-Löschung hat einmal neu eingebaute Funktionen mitgerissen. Beim nächsten Mal wurden 114 Zeilen entfernt und über 54 Zustände gegengeprüft: 0 Abweichungen. |
| **Toter Code, der eine Lüge enthält, ist schlimmer als toter Code.** Wer ein Blatt als Vorlage kopiert, greift die naheliegende Fassung — nicht die richtige. | `ebenen()` war seit Runden tot und trug eine **zweite** Statuszeile mit hart geschriebenem „Measuring 68 %“, während `statusText()` daneben wirklich misst. |
| **Nach jeder mechanischen Ersetzung: Blatt einmal zeichnen lassen.** Ein Zeilenumbruch in einer einfach zitierten Zeichenkette ist ein Syntaxfehler; das Blatt ist danach beim Doppelklick tonlos tot. | Genau das ist am 2026-08-21 passiert — vier Blätter auf einmal, gefunden nach zwanzig Sekunden nur, weil eine Blattprüfung lief. |

## 7. Die Prüfung selbst

| Regel | Der Fehler dahinter |
|---|---|
| **Ein Gate, das nicht scheitern kann, beweist nichts.** Jede neue Prüfung mit einem künstlichen Fehler gegenprüfen. | Nach einem Umbau griff ein Prüfmuster ins Leere und meldete fröhlich „heil". |
| **Je Kachel prüfen, nicht global.** | Ein globaler Treffer hätte eine stumme Variante gedeckt, weil die Nachbarin das Zeichen lieferte. |
| **Zustände einzeln prüfbar halten**, damit jeder für sich scheitern kann. | „Halten" und „Angewandt" hingen an einem Selektor. |
| **Im Grenzfall prüfen, nicht im Schönfall.** | Alle drei Zustandsvarianten sahen im Normalfall tadellos aus. Erst „alles getrennt" zeigte drei Lügen. |
| **Nie eine Höhe annehmen, immer lesen.** | Eine Verdeckungsprüfung nahm 15 px an und meldete eine Verdeckung, die es nicht mehr gab. |

---

## Wie man das nachbaut

Die Skripte liefen mit `playwright-core` gegen die vorhandene
Chromium-Installation (kein Browser-Download nötig), luden das Blatt per
`file://`, und maßen im Seitenkontext. Jede Prüfung endete mit einer Zeile,
die entweder `ok` sagte oder das Problem benannte — und mit `process.exit(1)`
bei Befund, damit sie nicht übersehen werden kann.
