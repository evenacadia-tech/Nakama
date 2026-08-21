# 2026-08-21 — Offen: zehn Codex-Befunde an der Messbank, NICHT verifiziert

> **⚠ Berichtigt.** Hier stand zuerst „drei Befunde". Das war falsch — ich
> hatte nur abgelesen, was auf dem Bildschirm sichtbar war, statt die
> Codex-Sitzungsdatei zu öffnen. Es sind **zehn**: vier P1, fünf P2, einer
> P3. Genau der Fehler, den dieses Projekt „Sekundärquellen glauben" nennt,
> diesmal von mir selbst begangen.

**Volltext, unverändert:** [2026-08-21-codex-review-volltext.md](2026-08-21-codex-review-volltext.md)

Eine Codex-Sitzung („Review EQ-Sondenplan implementation", 21.08. 08:20 bis
09:16) hat `HEAD 86f04c1` geprüft. Kurzurteil im Wortlaut:

> „Der Stand auf `86f04c1` ist eine gute, reproduzierbare Messbank – aber
> noch kein belastbarer EQ-Editor. Die zentrale Erkenntnis stimmt: 700×420
> reicht nicht für acht dauerhaft vollständige Bandzeilen plus eine
> brauchbare EQ-Kurve. Mehrere Prüfungen und Exporte erzeugen jedoch noch
> falsche Sicherheit."

> ⚠ **Nichts davon ist verifiziert.** Rund ein Viertel der AI-Auditbefunde
> war in diesem Projekt falsch. **Vor jedem Handgriff die Quelldatei
> öffnen.** Kein Befund verschwindet still — gefixt, widerlegt (mit
> Protokoll) oder als offene Zeile.

## Die vier P1

| # | Befund | Ort |
|---|---|---|
| 1 | Der Ergebnisexport exportiert nur `name`, `w`, `h` — Überlauf, Richtung und verbleibende Kurvenhöhe fehlen. Ein gespeichertes Ergebnis sieht positiv aus, obwohl beide Oberflächen unbrauchbar sind | `werkzeug/formfaktor.html:806` |
| 2 | Die Maßstabs-Gegenprobe misst zweimal denselben Maßstab: `--s:1` wird gesetzt, dann ersetzt `zeichnen()` die Knoten. Real `.46 → 1 → .46` | `werkzeug/pruefung/sondenprobe.mjs:120` |
| 3 | Die Zustandsmatrix prüft nur Fall A und nur `.slotz`; Gain, Kanal, dyn-Indikator und die Kontextstreifen aus Fall D fehlen. Deshalb meldet sie 16/16 sauber, obwohl bei Bypass `dyn`, `24 Hz` und `86 Hz · −3.5 dB` stehenbleiben | `werkzeug/pruefung/sondenprobe.mjs:342` |
| 4 | Der Grenzfall-Gate ruft `z(true, …)` — er kann gar nicht scheitern. Auch der normale verbundene Zustand mit realistischen Werten erscheint grün | `werkzeug/sonde-messung.html:1398` |

## Die fünf P2 und der P3, in Stichworten

5. Der „längstmögliche Slot" beruft sich auf die Spezifikation — die kennt
   aber **nur** Gain ±12 dB. Q 24, −60 dB, 300/500/3000 ms sind Annahmen.
   Dasselbe gilt für die Lesbarkeitsschwelle 15,4 px.
6. Draft/Discard/Undo/Neutralise sind keine ehrliche Zustandsmaschine —
   Discard lässt die Draft-Zeile stehen, Neutralise hat gar keinen Handler,
   Apply ändert keine Bänder.
7. `offsetHeight` zählt zwei Rahmenpixel mit — alle px/dB-Zahlen sind zu
   hoch. A/B haben statt 3 px nur **1 px** Innenraum, Formfaktor Active
   statt 7 px nur **5 px**.
8. Das Standband von `formfaktor.html` steht im DOM, ist aber vollständig
   verdeckt; die Blattprüfung testet nur Existenz, nicht Sichtbarkeit.
9. Der Kommentar der Passiv-Kachel behauptet, die **Breite** entscheide —
   gemessen ist der Überlauf ausschließlich **vertikal** (23–25 px, 0 px breit).
10. (P3) `dbText()` schreibt `−0.0 dB`; nach Apply setzt `streifen()` den
    Zustand auf `apply`, obwohl es nur `edit` und `audition` als Reiter gibt.

## Warum 2 und 4 besonders weh tun

Beide sind **dieselbe Krankheit**, gegen die die Sonde überhaupt gebaut
wurde: *ein Gate, das nicht scheitern kann, beweist nichts*
(`werkzeug/PRUEFLISTE.md` 7). Die Sonde entstand, WEIL die erste Fassung
zwei gleich falsche Werte verglich und zufrieden „0 Abweichungen" meldete.
Befund 4 ist dabei kein subtiler Fehler, sondern ein hart geschriebenes
`true`.

**Erster Handgriff der nächsten Runde:** Befund 4 gegen
`werkzeug/sonde-messung.html` verifizieren (die Zeile aufschlagen, nicht
suchen), dann Befund 2 mit einem künstlichen Fehler, der nur im großen
Maßstab sichtbar wäre.

## Codex' eigene Reihenfolge-Empfehlung

> „zuerst Export und falsche Gates reparieren, dann Parametergrenzen und
> Lesbarkeitsheuristik als Annahmen klären, anschließend Aufgabeninterview
> und erst danach C/D als echte Disclosure-Varianten entwerfen."

## Wo der Rest steht

- Messbank, Sonde, Formfaktor-Umbau committet (`d3541c1`, `86f04c1`,
  `8184474`) — aber zwei der zwölf Proben stehen unter Verdacht.
- Offene Designfrage, unverändert: **wer gibt bei Platzmangel nach —
  Kurve oder Bänder?**
- Die Aufgabenliste nach Häufigkeit ist weiter **Entwurf**
  (`docs/sondenplan.md` §3). Ohne sie kein Wireframe.
- **Nakama-Design hat kein Git-Remote.** 40+ Commits nur auf
  SCHUBBINATOR200; der User hat die Sicherung am 21.08. bewusst offen
  gelassen.
