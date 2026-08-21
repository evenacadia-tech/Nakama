# 2026-08-21 — Code-Review der bisherigen Arbeit, und klar Schiff für den Sondenplan

> **Kein User-Urteil.** Diese Datei ist ein Prüfbericht und eine Übergabe, keine
> Abnahme. Sie stellt nichts von dem in Frage, was der User entschieden hat —
> im Gegenteil: mehrere Reparaturen dienen genau dazu, dass Entschiedenes nicht
> versehentlich wieder aufgemacht wird. Der Fahrplan für den nächsten Schritt
> steht in `docs/sondenplan.md`.

---

## 1. Was geprüft wurde

| | |
|---|---|
| Lebende Blätter | **9** in `werkzeug/`, vollständig gelesen (7.048 Zeilen zu Beginn) |
| Dokumente | **26** — CLAUDE.md, 5 docs, 2 werkzeug-Texte, 15 abnahmen, 3 LIES-MICH, der interview-Skill |
| Infrastruktur | 2 Hooks, `settings.json`, `.gitignore`, Git-Historie |
| Helfer | **8 parallele Prüfagenten**, je mit eigenem Auftrag und eigener Messung |
| Geändert | **20 Dateien**, 987 Zeilen dazu, 229 weg |

**Jeder Agentenbefund wurde vor der Übernahme am Quelltext nachgeprüft.** Das
war nicht Zeremonie: zwei Befunde zitierten Textstellen mit Auszeichnungen, die
im Original nicht stehen — die Reparaturskripte haben deshalb abgebrochen statt
zu raten. Ein Verdacht von mir selbst („der Dauerhaft-Vertrag driftet nach
Apply") ließ sich **nicht** reproduzieren und steht deshalb nicht in der Liste.

---

## 2. Die Prüfungen, und wie man sie wiederholt

Der Grundsatz aus `werkzeug/PRUEFLISTE.md` 5.1 — *ein Gate, das nicht scheitern
kann, beweist nichts* — gilt auch für diese Prüfungen. Jede wurde mit einem
künstlichen Fehler gegengeprüft.

| Prüfung | Was sie beweist | Gegenprobe |
|---|---|---|
| **Blattprüfung** — lädt jedes Blatt headless, meldet JS-Fehler und leere Blätter | 9/9 Blätter heil | Syntaxfehler und leeres Blatt eingebaut → beide gemeldet, exit 1 |
| **Fingerabdruck** — 6 Szenarien × 3 Arbeitsschritte × 3 Messwerte = 54 Zustände, je Kachel Text, Zähler, Überlauf, Vertrag | Die Löschung von 114 Zeilen hat **0 Abweichungen** erzeugt | die Messung selbst zeigt Unterschiede, wo welche sind |
| **Schleusenprobe** — `.claude/hooks/schleusen-probe.sh` | 24/24: was blocken muss **und** was durchmuss | beide Richtungen sind Teil der Probe |
| **Belegprobe** — liest die `title`-Attribute der Vertrags-Chips | 90 Chips, 0 kaputt | vorher: 90 von 90 kaputt |
| **Ehrlichkeitsprobe** — fährt die Grenzfälle an | 8/8 | jede Zeile war vorher eine gemessene Lüge |

Die Skripte lagen sitzungslokal (`playwright-core` gegen die vorhandene
Chromium-Installation, `file://`, Messung im Seitenkontext). Wiederholbar sind
sie aus dieser Beschreibung; die **Schleusenprobe** ist als einzige dauerhaft
im Projekt, weil sie ohne Browser läuft.

---

## 3. Was behoben wurde

### 3.1 Zwei Wachhunde, die nicht gebellt haben

**Die Prototyp-Schleuse ließ Bash durch.** Der Matcher hieß `Write|Edit`, der
Hook las nur `file_path`. Eine Umlenkung nach `prototyp/` lief mit exit 0
hindurch — und genau dieser Weg ist in dieser Arbeitsumgebung der bevorzugte.
Zwei weitere Löcher fand erst die neue Gegenprobe: ein **relativer** Pfad fiel
durch das Muster `*/prototyp/*`, und `cp`/`mv` am Befehlsanfang galten nicht als
Befehlsanfang. Die erste Reparatur blockte dann zu viel und hat **zweimal echte
Arbeit abgewiesen** — beide Male Heredocs, die einen Schreibbefehl nur
zitierten. Text ist kein Befehl.

**Die Belege in den Vertrags-Chips waren zur Hälfte weg.** Jeder BELEG-Text
zitiert die Spezifikation mit einem geraden `"`; im `title`-Attribut riss das
die Beschriftung auf. Gemessen: 90 von 90 Chips über vier Blätter trugen sieben
Müll-Attribute und einen nach der Hälfte abgebrochenen Beleg.

### 3.2 Drei Fehler in allen vier Main-Blättern

- **Die rote Überlaufwarnung hatte keinen Rückweg** (`if(fehlt > 1)` ohne
  `else`). Einmal gesetzt, überlebte sie jede spätere heile Messung.
- **`s1()` stellte den Kopftext nie wieder her.** Nach einem Besuch im
  Zweikampf war die Erklärung des Blattes weg — `textContent` hatte die
  Auszeichnung schon gefressen.
- **Die Uhr lief auf dem Rückweg weiter**; `sekunden.schritt1` im Protokoll
  zählte die Zweikampfzeit mit. Genau diese Zahl zitieren die Abnahmen.

### 3.3 `zustaende.html` — 114 Zeilen toter Code, 18 Lügen

Entfernt: `ebenen()`, `schritte()`, `quellen()`, `rollenkarte()` samt `ROLLEN`
und `AUFFAELLIG`, `abruf()`, `minimap()`, `setzeQuellen()`, `lageAnlegen()` und
neun CSS-Blöcke.

**Die gefährlichste war `ebenen()`:** sie trug eine **zweite** Statuszeile mit
hart geschriebenem „Measuring 68 %" und „Width: unavailable", während
`statusText()`/`messText()` daneben wirklich messen. Für den Editor wird aus
diesem Blatt kopiert — die tote Fassung wäre die naheliegende gewesen.

Die achtzehn Lügen, alle im Grenzfall gegengeprüft:

| Was das Blatt sagte | Was stimmte |
|---|---|
| Assistent riet weiter auf getrennter Quelle | Streifen sagte gleichzeitig „no source reachable" |
| A/B und Detail zeigten volle Werte, Keep aktiv | Quelle war getrennt; die Befundliste zwei Kästen weiter machte es richtig |
| „Width 62 %" im Detail | „Width: unavailable" im Streifen, im selben Bild |
| drei Befunde mit „Confidence: high" | Streifen sagte „Measurement not started" |
| Quellenzustand **ersetzte** die Sicherheit | Sicherheit ist Pflichtangabe der Spezifikation |
| „Unsure" ließ sich drücken | war als einziges nicht verdrahtet |
| markierte Zelle nannte 120 Hz | Befund heißt „Bass · 180–260 Hz" (250er-Feld) |
| „finding 1 of 3" | geschrieben statt hergeleitet |
| Szenario „gemischt" leuchtete beim Laden | war nie gesetzt; es standen andere Quellen auf veraltet |
| „1:1 — echte 760×430" | Kachel war auf .92 bzw. .82 skaliert |
| „Halte Audition (hold) gedrückt" | den Griff gibt es nur in Schritt 2 |
| Markierung der betroffenen Zeile | deckte 1 px der Nachbarzeile ab |

Dazu die **Demo-Kennzeichnung**, die `CLAUDE.md` Regel 6 verlangt:
`grep -i demo` fand vorher null Treffer.

### 3.4 `formfaktor.html` — das Messgerät für die Sonden

Es war in vier Punkten blind. Alles am laufenden Blatt gemessen:

| Lage | gemeldet | wirklich | jetzt |
|---|---|---|---|
| „finden" @ 520×300 | 0 | 118 | **118 px (hoch)** |
| „lesen" @ 300 breit | 0 | 108 | **108 px (breit)** |
| „rollen" @ 760×430 *(abgenommene Größe!)* | 0 | 14 | **14 px (hoch)** |
| „alles" @ 980×560 | 0 | 40 | **40 px (hoch)** |
| **Active-Probe @ 700×420** | gar nicht | 24 | **24 px (hoch)** |
| **Passive-Kachel @ 260×84** | gar nicht | 3 | **3 px (hoch)** |
| Kopf der Quellenliste | „16 of 16" | 15 sichtbar | **„15 of 16"** |

Ursachen: die Messung sah nur `.inh` (ein verschachteltes `overflow:hidden`
schluckte den Rest), sie maß nur die Höhe, sie lief nur auf `#main`, und ein
9-px-Deckel unter einer 11-px-Kopfzeile ließ jede korrekte Messung dauerhaft
2 px rauschen.

### 3.5 Standband auf jedem Blatt

**Kein einziges der neun Blätter verriet beim Doppelklick seinen Stand.**
`zustaende.html` öffnete wieder eine Wahl, die am 2026-08-20 gefallen ist;
`bedienebenen.html` nennt im Kopf die **verworfene** Variante „Deine Wahl";
`licht.html` sieht aus wie ein gültiges Werkzeug, ist aber stillgelegt;
`weltenwahl.html` und `glaswelt.html` verschweigen, dass ihr Ergebnis nur eine
**Arbeitsannahme mit offenem Prüfpunkt** ist.

Das ist die „Ausbessern-Zeremonie", die `CLAUDE.md` Regel 4 verhindern soll.
Jedes Blatt trägt jetzt oben ein Band — *eingefroren · hier weiterarbeiten ·
Arbeitsannahme · stillgelegt* — mit Datum und Verweis auf die Abnahme. Die
Blätter selbst bleiben inhaltlich unangetastet: sie belegen weiter, was zur
Wahl stand.

### 3.6 Sprache und Zähler

29 Spezimen-Texte in vier Blättern auf Englisch umgestellt (`CLAUDE.md`,
Arbeitsweise: gilt ausdrücklich auch für die Spezimen). Vier weitere Zähler
sagen jetzt die Wahrheit: „3 named" bei zwei benannten Zeilen, „N more not
shown" für Rollen, die es im Mix nicht gibt, „8 Bänder" bei vier gezeichneten,
und ein Rückweg, der die engere Wahl vormarkiert ließ.

### 3.7 Dokumente

Was die nächste Session **falsch geglaubt hätte**, wenn sie nur die Dokumente
liest — jetzt berichtigt:

1. **„Die Schleuse blockt, solange `abnahmen/` leer ist"** (`CLAUDE.md`). Der
   Ordner hat fünfzehn Dateien, das Tor ist trotzdem zu. Verlangt wird eine
   Datei mit `designvertrag` im Namen. Das war der einzige Befund, der Arbeit
   **verhindert** statt sie zu verfälschen.
2. **„Phase 0 abgeschlossen"** — es gibt keinen Designvertrag und gab nie ein
   `/interview designvertrag`. Jetzt im Arbeitsplan vermerkt, mit dem Hinweis,
   dass Phase 0.5 und 1 zulässig ohne ihn liefen (`werkzeug/`, nicht `prototyp/`).
3. **„Phase 0.5 — hier stehen wir"** in `werkzeug/LIES-MICH.md`, während Main
   längst abgenommen ist. Wer dort einsteigt, macht Größen wieder auf.
4. **Welt und Fassung als „abgenommen" zitiert** — an vier Stellen, eine davon
   in `abnahmen/` selbst. Genau der Fehler, aus dem die Unterscheidung
   Abnahme/Arbeitsannahme entstanden ist.
5. **Die App-Umrandung stand ohne Vermerk unter „Was damit festliegt"** — sie
   ist gesehen und abgelehnt worden. Jetzt trägt die Datei ein
   ⚠-Rücknahme-Kästchen.
6. **„Acht Bänder mit je zehn Parametern … achtzig Werte"** im Auftrag für
   genau diese Runde. Es sind **zwölf** und **96** (plus fünf globale = 101).
   Der Inhalt ist 20 % größer als angesetzt — auf einer Fläche, die kleiner ist
   als Mains. Der teuerste der sechs Irrtümer, weil er die Klemmrechnung
   verfälscht, bevor sie beginnt.
7. **„370 px" bleiben für Karte + Arbeit** — es sind **368**; der 1-px-Rahmen
   war nicht abgezogen.
8. **„18 Runden"** in der Stilbefragung — es sind **19** (16 Wahl-, 3
   Reglerrunden).
9. **„980×560 gegen ~520×340, rund dreifache Fläche"** — beides sind
   Voreinstellungen, bei beiden wird abgeschnitten. Ehrlich gemessen eher
   **vierfach**. Die Aussage trägt dadurch stärker, nicht schwächer.
10. **Eine Prüfung, die es nie gab** — `LIES-MICH.md` behauptete, ein
    Auseinanderlaufen von Listenname und Bausteinname schlage an. Tut es nicht;
    der Fall fällt still durch.
11. Der **SessionStart-Primer** sagte „die vier Projektquellen" bei fünf. Die
    Zahl wird jetzt gezählt, nicht geschrieben — und der Primer warnt zusätzlich
    vor den beiden Fallen in der Assettruhe und vor der geschlossenen Schleuse.

---

## 4. Was NICHT repariert wurde, und warum

Diese Punkte sind **Gestaltungsfragen, die dem User gehören**. Sie stehen hier,
damit sie nicht verlorengehen — nicht als Mängelliste.

| Befund | Warum ich es nicht selbst entschieden habe |
|---|---|
| **„Veraltet" löscht die Energie** statt sie zu altern; hohl liest sich als „leer" und kollidiert optisch mit „getrennt" | Der User hat die Variante mit **genau diesem Bild** gewählt. Es zu ändern hieße, eine bindende Abnahme ohne sein Wort aufzumachen |
| **„1 change applied"** ist ein Zähler ohne Verlauf; die Spezifikation kennt einen Undo-Verlauf nur für den Editor | Steht schon als offener Punkt im Sessionende. Produktentscheidung |
| **APPLY bleibt als Reiter stehen** — „Anwenden" wird zum Daueraufenthalt, obwohl der Kommentar im Blatt es eine *Tat* nennt | Betrifft die abgenommene Bedienebenen-Trennung |
| **Fünf Messzustände**, wo die Spezifikation zwei kennt | Die drei zusätzlichen sind **Ehrlichkeitsgewinne** aus der Zustandsabnahme („Nothing to measure", „Measurement stalled"). Richtig wäre, die Spezifikation nachzuziehen — das ist eine User-Entscheidung über die Sachquelle |
| **Sechs Spezifikationsinhalte ohne Ort** (Suche, Pinning, Fokusgruppen, Rollen, Minimap/Zoom, Dichtestufe „Liste") | Das ist die nächste Entwurfsrunde für Main, kein Bug. Sie gehört hinter die Sonden |
| **`licht.html`: „Original halten" zeigt reines Schwarz**, die Fassung ist `#050709` — die Abnahme schließt reines Schwarz ausdrücklich aus | Das Blatt ist stillgelegt und sagt das jetzt auch. Repariert wird es, wenn es neu gebaut wird — vorher wäre die Arbeit weg |
| **`stilbefragung.html` zeigt die verworfene Welt** | Umstellen heißt neu entwerfen. Das Band warnt jetzt davor, es vorher zu benutzen |
| **Die Klemme-Tabelle in `LIES-MICH.md` ist 2–5 px zu hoch** (212/370/172/219/120/69 gegen gemessen 210/368/154/146/115/66) | Sie beschreibt **eingefrorene** Blätter zum Zeitpunkt ihres Urteils. Die Formfaktor-Zahlen sind mit einem datierten Vermerk versehen; die Klemme-Zahlen bleiben stehen, weil die Schlussfolgerung („beides zugleich geht nicht") von 2 px nicht berührt wird — die Nachmessung steht hier |

---

## 5. Was offen bleibt

- **Der Designvertrag (Phase 0).** Ohne ihn bleibt `prototyp/` gesperrt, und
  Phase 2 (Stilprobe) hat keine Erfolgskriterien, gegen die sie beurteilt
  werden könnte.
- **Zwei Fallen in der Assettruhe.** `assets/nakama-stilprofil.json` ist der
  Tokensatz aus der **verworfenen** Welt (Zeitstempel 13:35, die Weltfrage kam
  danach) — und `CLAUDE.md` Regel 1 sagt „Was dort liegt, wird verwendet".
  Der Ordner **„Nakama Designausarbeitungen selfmade"** (vier PNGs, u. a. eine
  selbstgemachte Polycarbonat-Textur) ist versioniert, aber in keinem Dokument
  erwähnt; er kam beiläufig in zwei Commits über *Zustände* mit. **Ob diese
  Textur das `korn.png` ablösen soll, weiß niemand.** Die Truhe wurde nicht
  angefasst (Regel 1); die Warnung steht jetzt im Primer.
- **Die Verdeckungsprüfung** aus `PRUEFLISTE.md` existiert in keinem Blatt als
  Code. Der Editor braucht sie dringender als Main.
- **Die Aufgabenliste des Editors** ist ein Entwurf und braucht das Wort des
  Users, samt der zwei Fragen in `docs/sondenplan.md` Abschnitt 3.
- **Wer beim Editor bei Platzmangel nachgibt** — Kurve oder Bänder — ist eine
  Produktentscheidung und keine Codefrage.

---

## 6. Eine Zeile für die Prüfliste

Aus dieser Sitzung ist eine Regel dazugekommen, die dort noch fehlt:

> **Ein Blatt muss sagen, was es ist.** Entschieden, eingefroren,
> Arbeitsannahme oder stillgelegt — sichtbar, beim ersten Blick, außerhalb des
> beurteilten Inhalts. *Der Fehler dahinter:* neun Blätter verschwiegen ihren
> Stand; drei davon öffneten beim Doppelklick eine Frage, die längst
> entschieden war.

Und eine zweite, die diese Sitzung selbst gekostet hat:

> **Eine Löschung wird mit einem Fingerabdruck abgesichert**, nicht mit
> Zuversicht: Verhalten über die volle Zustandsmatrix vorher festhalten,
> löschen, neu festhalten, vergleichen. *Der Fehler dahinter:* eine
> Dead-Code-Löschung hat schon einmal Lebendes mitgerissen — und eine
> mechanische Ersetzung in dieser Sitzung hat vier Blätter tonlos tot gemacht,
> gefunden nur, weil die Blattprüfung zwanzig Sekunden später lief.
