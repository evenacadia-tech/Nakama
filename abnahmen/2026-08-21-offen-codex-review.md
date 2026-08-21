# 2026-08-21 — Offen: drei Codex-Befunde an der Messbank, NICHT verifiziert

Eine Codex-Session („Review EQ-Sondenplan implementation", cwd
`Projekte\Nakama-Design`, 21.08. ab 08:20) hat den Stand auf `86f04c1`
geprüft. Ihr Kurzurteil: *„eine gute, reproduzierbare Messbank — aber noch
kein belastbarer EQ-Editor. Mehrere Prüfungen und Exporte erzeugen jedoch
noch falsche Sicherheit."*

> ⚠ **Diese Befunde sind SEKUNDÄRQUELLE und stehen ungeprüft hier.**
> `CLAUDE.md` und der Prüfbericht sagen beide: rund ein Viertel der
> AI-Auditbefunde war in diesem Projekt falsch. **Vor jedem Handgriff die
> Quelldatei öffnen.** Kein Befund darf still verschwinden — gefixt,
> widerlegt (mit Protokoll) oder als offene Zeile.

## Die drei Befunde im Wortlaut der Meldung

1. **[P1] Der Ergebnisexport verliert ausgerechnet die entscheidenden
   Messwerte.** `formfaktor.html` (Zeile 806) exportiert für Active und
   Passive nur `name`, `w` und `h`. Im Browser wurden Active 0 px Überlauf
   bei nur 5 px nutzbarer Kurvenhöhe sowie Passive 23 px vertikaler
   Überlauf gemessen — nichts davon landet im JSON. Ein gespeichertes
   Ergebnis sieht dadurch positiv aus, obwohl beide Oberflächen unbrauchbar
   sind.

2. **[P1] Die Maßstabs-Gegenprobe misst zweimal denselben Maßstab.**
   `sondenprobe.mjs` (Zeile 120) setzt `--s:1` auf die vorhandenen Rahmen;
   unmittelbar danach ersetzt `zeichnen()` diese DOM-Knoten. Reale Folge:
   `.46 → 1 → .46`, `beiKlein` und `beiGross` werden somit beide im kleinen
   Maßstab gelesen — obwohl die Ausgabe fünf geprüfte Maßstabsfälle
   behauptet.

3. **[P1] Die neue Zustandsmatrix übersieht genau die noch falschen
   Darstellungen.** `sondenprobe.mjs` (Zeile 342) prüft nur Fall A und darin
   nur `.slotz` sowie ausgewählte Felder. Gain, Kanal, der
   dynamisch-an/aus-Indikator und die Kontextstreifen aus Fall D fehlen.
   Deshalb meldet die Probe 16/16 sauber.

## Warum Befund 2 besonders weh tut

Wenn er stimmt, ist es **derselbe Fehler ein zweites Mal**: Die Sonde wurde
eigens gebaut, weil die erste Fassung zwei gleich falsche Werte verglich und
zufrieden „0 Abweichungen" meldete. Der Maßstabs-Schritt war die Reparatur —
und wäre dann selbst blind. Genau der Fall aus `werkzeug/PRUEFLISTE.md` 7:
*ein Gate, das nicht scheitern kann, beweist nichts.*

**Erster Handgriff der nächsten Runde:** Befund 2 gegen
`werkzeug/pruefung/sondenprobe.mjs` verifizieren — mit einem künstlichen
Fehler, der nur im großen Maßstab sichtbar wäre. Fällt die Probe nicht
darauf, ist der Befund bestätigt.

## Wo der Rest steht

- Messbank, Sonde, Formfaktor-Umbau: committet (`d3541c1`, `86f04c1`,
  `8184474`), Blattprüfung 10/10, Sondenprobe 12/12 — aber siehe oben,
  zwei dieser Proben stehen unter Verdacht.
- Offene Designfrage, unverändert: **wer gibt bei Platzmangel nach —
  Kurve oder Bänder?** Beide Antworten sind in `sonde-messung.html`
  vorgerechnet, entschieden ist keine.
- Die Aufgabenliste nach Häufigkeit ist weiter **Entwurf**
  (`docs/sondenplan.md` §3). Ohne sie kein Wireframe.
- **Nakama-Design hat kein Git-Remote.** 40 Commits liegen nur auf
  SCHUBBINATOR200. Der User hat die Sicherung am 21.08. bewusst offen
  gelassen.
