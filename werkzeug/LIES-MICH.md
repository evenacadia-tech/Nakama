# Werkzeug — Messinstrumente, keine Entwürfe

Hier liegen Hilfsmittel, mit denen die Vorstellung des Users **gemessen** wird.
Nichts davon ist ein Gestaltungsvorschlag. Layoutentscheidungen gehören nach
`prototyp/` und in Phase 1 des Arbeitsplans.

## Reihenfolge — nach den Stufen aus `assets/GUI-Tips.pdf`

> **Hier stehen wir (Stand 2026-08-21):** Main ist auf Wireframe-Ebene
> **vollständig abgenommen**; der aktuelle Entwurf liegt in `zustaende.html`.
> Als Nächstes: **Active-Probe-Editor** (700×420), danach **Passive-Kachel**
> (260×84). Fahrplan: `docs/arbeitsplan.md` · Auftrag und Klemme:
> `docs/sondenplan.md` · Prüfbericht: `abnahmen/2026-08-21-codereview.md`.

**Stufe 1 — The Feeling** *(erhoben — ⚠ Arbeitsannahme, nicht abgenommen)*
1. `weltenwahl.html` — welche Welt? → **Glas und Licht**
2. `glaswelt.html` — welche Verwirklichung? → **dunkles Glas, ein Lichtleiter**

Beide wurden an einem Ausschnitt von 448×310 gewählt, ohne Layout und ohne
echte Fenstergröße. Der User hat sie ausdrücklich **nicht** bestätigt: „es war
zu dem zeitpunkt logisch. wir werden sehen wenn das layout steht ob es dann
noch passt." **Prüfpunkt:** erneut vorlegen auf 760×430. Bis dahin darf
niemand sie als abgenommen zitieren (`abnahmen/LIES-MICH.md`).

**Phase 0.5 — Formfaktor** *(abgeschlossen — drei Größen abgenommen)*
3. `formfaktor.html` — wie groß darf das Fenster sein, und welche Form hat es?
   Läuft bei **1:1**, nichts wird skaliert, und es misst, ab wann Inhalt
   abgeschnitten wird. Muss vor jedem Wireframe entschieden sein.

**Blattprüfung** — `pruefung/pruefen.mjs`: lädt jedes Blatt headless und
meldet JS-Fehler, leere Blätter und fehlende Standbänder.
`node werkzeug/pruefung/pruefen.mjs` (alle) bzw. `--gegenprobe` (beweist, dass
sie scheitern kann). Nach JEDEM Umbau laufen lassen — ein Syntaxfehler macht
ein Blatt tonlos tot. Details: `pruefung/LIES-MICH.md`.

**Prüfliste** — `PRUEFLISTE.md`: was an jedem Blatt gemessen wird, bevor es
gezeigt wird, mit dem echten Fehler hinter jeder Regel. Vor jedem neuen Blatt
lesen; jede Zeile dort hat einmal Arbeit gekostet.

**Stufe 2 — UX Layout** *(Main abgeschlossen, zwei Oberflächen offen)*
4a. `wireframe-main.html` — welche Disclosure-Mechanik? → **Overview + Detail**
    *(entschieden, eingefroren)*
4b. `wireframe-main-karte.html` — was trägt die dauerhafte Karte? → **jede
    Quelle, Überschuss wird geteilt** *(entschieden, eingefroren)*
4c. `bedienebenen.html` — woran sieht man das Vorhören? → **an der markierten
    Zeile**, App-Umrandung verworfen *(entschieden, eingefroren)*
4d. `zustaende.html` — wo lebt ein Zustand? → **nur die Ausnahme spricht**
    *(entschieden)* — **aktueller Main-Entwurf, hier weiterarbeiten**
4e. *(offen)* **Active-Probe-Editor**, 700×420 — der nächste Schritt.
    Auftrag, Aufgabenliste und vorgerechnete Klemme: `docs/sondenplan.md`.
4f. *(offen)* **Passive-Kachel**, 260×84.
5. *(offen)* Schritt 2: Early Visual Layer — Kontrast und Gruppierung, noch
   ohne fertigen Look.
6. *(offen)* Schritt 3: UI Design Stage — Stil und Feinproportionen.

**Stufe 5/6 — Licht und Material** *(stillgelegt bis Stufe 2 durch ist)*
7. `licht.html` — **nicht benutzen.** Zwei Gründe: es kommt zu früh, und sein
   Modell ist falsch (vier sich ausschließende Herkünfte). Licht ist additiv,
   kommt auch aus den UI-Elementen selbst — passiv wie aktiv — und Material
   entscheidet, wie eine Fläche darauf antwortet. Wird vor dem Einsatz neu
   gebaut. Siehe
   `abnahmen/2026-08-20-korrektur-reihenfolge-und-lichtmodell.md`.
8. `stilbefragung.html` — zuletzt. Ihre Spezimen zeigen noch die verworfene
   Welt und müssen vorher umgestellt werden.

Zweimal ist genau dieser Reihenfolgefehler passiert: einmal wurden Radien
innerhalb einer selbst gewählten Welt abgefragt, einmal Licht verhandelt, bevor
die Anordnung stand. Beide Male hat der User es gemerkt, nicht Claude.

## weltenwahl.html

Sechs Formsprachen nebeneinander, **gleiche Information** (Quelle,
Frequenzkurve, ein Wert, ein Zustand), **alles andere verschieden**: Material,
Licht, ob es überhaupt ein Gehäuse gibt, wie viel Text, wie bedient wird.

Die Achsen stammen aus `assets/GUI-Tips.pdf` (Voger Design, „Quality GUI
Checklist", Stufe 1) — Entity Type, Material, Licht, Temperatur — nicht aus
Claudes Geschmack. Zwei Welten sind direkte Übersetzungen von Referenzen aus
der Truhe: „Dunkles Gerät" aus `301.zip`, „Helles Objekt" aus `110.zip`.

Ablauf: sechs → zwei in die engere Wahl → Zweikampf groß → Ergebnis als JSON.

## glaswelt.html

Fünf Verwirklichungen der als **Arbeitsannahme** gewählten Welt (nicht
abgenommen — Prüfpunkt: 760×430), gleiche Information an gleicher
Stelle: Milchglas vor kaltem Licht · die Messung ist die Lichtquelle ·
geschichtete Scheiben · dunkles Glas mit einem Lichtleiter · Glas mit Griff
(nimmt Korn und Leuchtring aus dem unterlegenen „dunklen Gerät" mit).

Der Befund erscheint hier erstmals **ohne Text**: ein Ring sagt, wie sicher die
Aussage ist, ein weiches Band im Kurvenfeld sagt, wo sie gilt.

Die Thesen zu den Varianten sind standardmäßig ausgeblendet — wer sie vorher
liest, wählt den besseren Satz statt das bessere Bild.

## formfaktor.html

Zeigt **echte Pixel** auf einem nachgestellten FL-Studio-Bildschirm. Nichts
wird skaliert — eine Größenentscheidung ist anders nicht beurteilbar. Ein Band
oben prüft und meldet, ob gerade wirklich 1:1 gilt, und bietet Vollbild an.

Zwei Inhaltsmengen umschaltbar: **alles gleichzeitig** (die alte Denkweise)
gegen **nur der aktuelle Handgriff** (Aufgabe 2 der Häufigkeitsliste). Das
Werkzeug misst auf jeder Ebene — Fensterleib und jeder einzelne Kasten — ob
Inhalt abgeschnitten wird, und schreibt den Fehlbetrag in den Fenstertitel.
Gemessenes Ergebnis: 980×560 gegen ~520×340, also **rund dreifache Fläche**
für dieselbe Aufgabe.

> **⚠ Nachgemessen am 2026-08-21.** Die beiden Zahlen sind **Voreinstellungen
> aus der Kandidatenliste**, keine Suchergebnisse — und bei beiden wird Inhalt
> abgeschnitten: bei 980×560 fehlen gemessen 40 px. Die *ehrlichen* Minima
> liegen bei rund **900×666** gegen **410×348**, also eher **vierfache**
> Fläche. Die Aussage („Progressive Disclosure ist die Bedingung, nicht die
> Kür") trägt damit stärker, nicht schwächer. Grund für den alten Wert: die
> Überlaufmessung des Blattes war blind — sie sah nur `.inh`, nur die Höhe
> und nur `#main`. Das ist am 2026-08-21 behoben; das Blatt meldet die
> Fehlbeträge jetzt selbst.

Main ist zieh- und größenveränderbar, Active-Probe und drei Passive-Kacheln
lassen sich dazuschalten — die reale Situation, nicht ein Fenster allein.

## wireframe-main.html

Stufe 2, Schritt 1. Fünf **Disclosure-Mechaniken** von **Main** auf der
abgenommenen Größe **760×430**, in reinen Graustufen — kein Farbton, kein
Material, kein Licht. Varianten unterscheiden sich in der Mechanik, nicht in
der Kastenanordnung (`docs/DESIGN-GESETZE.md`, verbindliche Folge 3):

1. **Schritt für Schritt** — der Bildschirm zeigt genau den Schritt, in dem
   du bist, sonst nichts.
2. **Focus + Context** — eine Fläche; was zählt, wächst, der Rest schrumpft
   zu Streifen.
3. **Overview + Detail** — dauerhafte kleine Karte oben, wechselnde
   Arbeitsfläche darunter.
4. **Nur auf Abruf** — ein Satz, was zu tun ist; alles andere hinter
   benannten, geschlossenen Türen.
5. **Semantischer Zoom** — dieselbe Fläche in drei Dichtestufen
   (Liste → Heatmap → Detail).

Alle fünf zeigen denselben Pflichtinhalt aus
`docs/oberflaechen-spezifikation.md`. Im Raster sind die Strukturen zu
erkennen, in der Großansicht (Zweikampf, 1:1) die Beschriftungen zu lesen.

### Was das Werkzeug misst, statt es zu behaupten

Drei Messungen laufen bei jeder Neuzeichnung. Sie sind der Grund, warum dem
Blatt zu trauen ist:

1. **Ehrliche Listen.** Jede Liste zeigt nur, was ganz hineinpasst, schreibt
   die wahre Zahl in ihren Kopf („2 of 3") und gibt darunter zu, was fehlt
   („1 more not shown"). Ohne das war der Vergleich wertlos: eine Kachel
   behauptete im Kopf `11 of 16` und zeigte **fünf** Zeilen.
2. **Überlauf, den kein Weglassen heilt.** Rekursiv über *alle* Nachfahren.
   Die alte Messung sah nur vier Klassen; ein verschachteltes
   `overflow:hidden` schluckte die Überlänge, bevor der Elternkasten sie sah
   — gemeldet wurden 36 px, tatsächlich fehlten **111**.
3. **Der Dauerhaft-Vertrag.** Schalter „Was bleibt dauerhaft stehen?".
   Ein Baustein gilt als dauerhaft, wenn er in **allen drei**
   Arbeitsschritten auf dem Blatt steht — gemessen, nicht behauptet. Jeder
   Baustein trägt im Kurzhinweis seinen **Handgriff** und seine **Quelle**
   (`BELEG` im Code); ein Baustein ohne Beleg wird rot und müsste vom Blatt
   verschwinden (`DESIGN-GESETZE.md`, Folge 2).

Der Regler **„Quellen im Mix"** (4–16) ist keine Entwurfsentscheidung,
sondern die Belastungsprobe: was bei 8 trägt und bei 16 bricht, trägt nicht.

### Entschieden

**Overview + Detail** ist gewählt (2026-08-20, engere Wahl gegen
Focus + Context) — `abnahmen/2026-08-20-mechanik-main-overview-detail.md`.
Das Werkzeug bleibt als Beleg stehen und wird nicht umgebaut: es zeigt, was
zur Wahl stand und woran gemessen wurde.

### Gemessener Stand (2026-08-20, 760×430, 16 Quellen)

Nach der Reparatur passt **jede** der fünf Mechaniken — kein Blatt schneidet
mehr ab. Der Unterschied liegt jetzt dort, wo er hingehört: darin, **wieviel
vom Mix dauerhaft stehen bleibt.**

| Mechanik | dauerhaft sichtbar | davon vom Mix |
|---|---|---|
| Schritt für Schritt | Bedienebenen+Status · Wegweiser | **0 von 16** |
| Nur auf Abruf | Bedienebenen+Status | **0 von 16** |
| Semantischer Zoom | Bedienebenen+Status · Zoomleiste | **0 von 16** |
| Focus + Context | Bedienebenen+Status · Heatmap mit Namen | **7 von 16** |
| Overview + Detail | Bedienebenen+Status · Karte ohne Namen | **11 von 16** |

**Focus + Context erfüllt seine eigene These auf dieser Fläche nicht:** „der
Kontext verschwindet nie" hält für 7 der 16 Quellen, neun fallen weg. Die
These im Werkzeug wurde entsprechend berichtigt.

Die frühere Messtabelle (Sessionende 2026-08-20, „drei Mechaniken scheitern,
mit 12 statt 16 Quellen würden sie passen") ist **überholt und war in 6 von
15 Feldern falsch**. Nachgemessen galt: Focus+Context brauchte 11, nicht 12;
semantischer Zoom 13; Overview+Detail scheiterte an der Befundkarte und wäre
mit *keiner* Quellenzahl heil geworden. Ursache war teils die blinde
Messung, teils Deckel unter der Inhaltshöhe.

## wireframe-main-karte.html

Stufe 2, Main, **nach** der Abnahme von Overview + Detail. Die Mechanik steht;
offen ist die **Karte** — das einzige Stück Mix, das dauerhaft dasteht.

### Die Aufteilung ist entschieden (User, 2026-08-20, per Einzeichnung)

Der User hat auf einem Screenshot magenta markiert, wo die Grenze zwischen
Karte und Arbeitsfläche in zwei Entwürfen lag, und **gelb, wo sie liegen
soll**. Ausgemessen aus dem Bild (Maßstab über die bekannte Blattbreite):

| | Grenze ab Blattoberkante |
|---|---|
| „Alle 16, gleich hoch" — Karte wuchs nur bis zur natürlichen Höhe | 201 px |
| „Karte weicht der Arbeit" — Karte nahm allen Rest | 282 px |
| **gelbe Linie des Users** | **≈ 250 px** |

Die Regel dahinter: **Karte und Arbeitsfläche bekommen erst ihre natürliche
Höhe, der Überschuss wird geteilt.** Nicht „jede die Hälfte der Gesamthöhe" —
das wären 229 px gewesen und hätte die Linie verfehlt. In CSS ist das genau
`flex: 1 1 auto` in einer Flex-Spalte, nicht `1fr 1fr` im Gitter.

Zusatz aus der Abnahme: die **Karte darf wachsen, aber nicht schrumpfen**
(`flex:1 0 auto`) — sie ist das dauerhafte Ding und muss ihr Versprechen
halten. Der Druck landet unten, wo das Blatt ehrlich sagt, was fehlt.
Gemessen liegt die Grenze jetzt bei **245 px** (10 Quellen, Schritt
„Urteilen").

Damit sind die beiden reinen **Aufteilungs**-Varianten erledigt; übrig
bleiben die drei, die sich im **Inhalt** der Karte unterscheiden.

### Die Klemme, gemessen auf 760×430

Der Streifen kostet dauerhaft **28 px**; für Karte und Arbeitsfläche bleiben
**370**. Natürliche Höhen bei 744 px Breite:

| | braucht |
|---|---|
| Karte ohne Namen, 16 Quellen | 212 px (rund 10 px je Zeile) |
| Karte **mit** Namen, 16 Quellen | 308 px — geht nicht (einzige Zahl dieser Tabelle ohne Beleg im Quelltext) |
| Assistent, Fünf-Teile-Form | **172 px** |
| Alle drei Befunde | 219 px |
| A/B-Urteil 120 px · Detail einer Quelle | 69 px |

Beides zugleich — alle 16 Quellen und ein vollständiger Assistent — geht auf
dieser Fläche nicht.

### Entschieden: „Alle 16, gleich hoch"

Gewählt am 2026-08-20 (engere Wahl gegen „nach Rollen") —
`abnahmen/2026-08-20-karte-alle-quellen.md`. Die Karte trägt jede Quelle,
jede bekommt dieselbe Zeile.

**Der genannte Preis ist danach eingelöst worden.** Stand jetzt, 16 Quellen:

| Kartenart | Karte | Assistent | Befunde |
|---|---|---|---|
| **Alle 16, gleich hoch** (gewählt) | 16/16 | 5/5 | 3/3 |
| Karte nach Bedeutung | 16/16 | 5/5 | 3/3 |
| Karte nach Rollen | 5/5 | 5/5 | 3/3 |

Zum Zeitpunkt des Urteils stand dort noch `3/5` und `1/3` — die Wahl fiel
mit sichtbarem Preis. Drei Eingriffe haben ihn beseitigt:

1. **Ratsche in der Aufteilung.** Die Karte wuchs auf 230 px, obwohl sie für
   16 Zeilen nur 212 braucht: die Arbeitsfläche kürzte ihre Listen, dadurch
   schrumpfte deren Flex-Basis, und die Karte wuchs nach. Reine CSS-Flexbox
   kann das nicht auflösen, weil das Kürzen **nach** dem Layout passiert.
   `aufteilen()` misst deshalb einmal die ungekürzten Höhen und setzt fest:
   **Überschuss teilen, Mangel nicht.**
2. **Befunde mit Disclosure**: einer offen, die anderen als Zeile mit Ort und
   Sicherheit.
3. **Assistent dichter, nicht kleiner**: die 16 px kamen aus Abständen, nicht
   aus Inhalt oder Schriftgröße. 154 px bei 156 verfügbaren.

### Was aus Runde 1 gelernt und hier eingebaut ist

- Der Assistent ist eine **ehrliche Liste**: fehlt ihm Höhe, zeigt er `3/5`
  und „2 more not shown" statt heimlich abzuschneiden. Man sieht, **welcher**
  Teil fehlt.
- Der gemessene Zähler hat eine **eigene Form** (`3/5`), weil im selben Kopf
  oft ein geschriebenes „finding 1 of 3" steht.
- Wo **Listenname und Bausteinname auseinander laufen**, meldet der Vertrag
  stumm keine Zahl — daran ist er zweimal gescheitert. **⚠ Berichtigt
  2026-08-21:** hier stand, eine Prüfung schlage in diesem Fall an. Das ist
  falsch, eine solche Prüfung existiert in keinem Blatt (`zahl()` liefert
  einfach eine leere Zeichenkette). Der Fall wird bis heute nur durch
  Aufpassen vermieden. Wer den Editor baut, sollte die Prüfung bauen.
- **Zahlwörter werden hergeleitet, nicht geschrieben** (`wieViele()`): beim
  Streichen einer Variante stand sonst „in allen vier gleich" unter drei
  Kacheln.

## bedienebenen.html

Stufe 2, Main, auf dem abgenommenen Entwurf (Overview + Detail, Karte trägt
jede Quelle, Überschuss wird geteilt). Die Frage ist die Sicherheitsfrage der
Spezifikation: **„Vier Bedienebenen, sichtbar getrennt (nie vermischen)"**.

**Dieses Blatt ist bedienbar** — als einziges bisher. Grund: eine
Zustandsfrage lässt sich am Standbild nicht beurteilen
(`DESIGN-GESETZE.md`, verbindliche Folge 4). `Audition (hold)` reagiert auf
echtes `pointerdown`/`pointerup`, nicht auf einen Schalter; `Apply` und
`Undo` sind bewusste Klicks. Deshalb sind die Kacheln **kein `<button>`**
mehr — gewählt wird über die Zeile darunter.

### Die vier sind keine vier Geschwister

- **Beobachten** und **Beraten** sind Aufenthalte — man ist dort.
- **Vorhören** ist eine flüchtige Tat, hörbar nur solange gehalten.
- **Anwenden** ist eine bewusste Tat, sie bleibt und ist umkehrbar.

Vier gleich aussehende Reiter wären genau der Fehler, den Von Restorff
beschreibt: fällt alles auf, fällt nichts auf.

### Entschieden: nur die markierte Zeile

> `sieger: "zeile"`, engere Wahl gegen „beides" —
> `abnahmen/2026-08-20-vorhoeren-markierte-zeile.md`

**Die App-Umrandung ist verworfen** — gebaut, danebengestellt, gehalten und
abgelehnt. Vorhören zeigt sich **am Ort der Wirkung**: die betroffene Quelle
in der Karte wird markiert, solange gehalten wird.

Drei Zeichen, drei verschiedene Fragen, keines doppelt:

| Zeichen | Ort | beantwortet |
|---|---|---|
| `AUDITION`-Reiter leuchtet | Streifen | *auf welcher Ebene bin ich?* |
| Griff sichtbar gedrückt | unter dem Finger | *ich tue es gerade* |
| Zeile markiert | in der Karte | *und zwar hier* |

Damit ist die offene Frage der Vorrunde beantwortet: der Reiter leuchtet
weiter — er ist die einzige globale Auskunft.

**Der Weg dorthin ist lehrreich und steht deshalb hier:** die Wahl war
vorher zweimal anders angesagt (erst „1 und 2", was ich falsch als Streifen
+ Blattrahmen übersetzte; dann „markierte Zeile + App-Umrandung"). Erst am
bedienbaren Blatt, mit gehaltenem Griff, fiel sie endgültig. Genau dafür
muss eine Zustandsgestaltung bedienbar sein.

### Drei Befunde, die erst das 1:1-Bild gezeigt hat

0. **Die übergroße Zeile in der Karte war ein Fehler, kein Entwurf.** In
   `heatmap()` hieß zweierlei gleich: `fokus` als *Disclosure-Mechanik*
   (eine Zeile vierfach hoch, der Rest 5-px-Streifen) und `fokus` als
   *Zustand* („welche Quelle geht die Änderung an"). In der Ruhelage griff
   die falsche Bedeutung. Getrennt in `opt.fokus` und `opt.betroffen`.
1. **`ueberlauf()` maß nur die Höhe.** Der Streifen schnitt rechts ab und
   nichts meldete sich. Misst jetzt **beide Richtungen** — und fand sofort
   18 px in Ruhe, 36 px beim Halten.
2. **Overlays fallen durch jede Überlaufmessung.** Das Band lief nicht
   über — es lag nur über dem Inhalt. Dafür braucht es eine eigene Prüfung
   (`::after` wirklich lesen, nicht eine Höhe annehmen).
3. **Die Lage hing erst beim Neuzeichnen am Blatt**, nicht beim ersten
   Zeichnen. Interaktiv fiel das nie auf; ein frisch gebautes Blatt zeigte
   einen Zustand nicht, den es hatte.

### Was der Streifen kostet, gemessen

| | Breite |
|---|---|
| vier Ebenen-Reiter | 254 px |
| Statuszeile (frisch/veraltet/getrennt, Messung, degradiert) | 328 px |
| verfügbar im Blatt | 742 px |
| **bleibt für den Lage-Chip** | **160 px** |

„Auditioning — release to stop" brauchte 164, „Nothing is changing your mix"
180. Beides passte nicht. Der Chip sagt jetzt nur noch, **was der Reiter
nicht sagen kann** — beim Halten „release to stop", nach dem Anwenden
„1 change applied", sonst nichts.

## zustaende.html

Stufe 2, Main, der letzte offene Punkt der Spezifikation:

> „Zustände, die immer ablesbar sein müssen: Quelle frisch / veraltet /
> getrennt; Messung läuft / unvollständig; eine Funktion ist auf diesem
> System nicht verfügbar (degradiert) — **ehrlich zeigen, nie kaschieren.**"

Bisher standen sie nur als Text im Streifen. Die Karte trug sechzehn Zeilen,
die alle gleich aussahen — obwohl drei davon veraltet oder getrennt waren.

### Drei Sorten Zustand, auseinandergehalten

1. **je Quelle** — frisch / veraltet / getrennt → *die Variable dieser Runde*
2. **global** — Messung läuft / unvollständig
3. **die App** — eine Funktion ist nicht verfügbar (degradiert)

Nur Sorte 1 unterscheidet die drei Kacheln; 2 und 3 bleiben gleich, sonst
vergleicht man zwei Dinge gleichzeitig.

### Entschieden: nur die Ausnahme spricht

> „3. ist nicht perfekt aber das einzige was sinn ergibt" —
> `abnahmen/2026-08-20-zustaende-nur-ausnahme.md`

| Mechanik | in der Karte | im Streifen | Preis |
|---|---|---|---|
| Nur die Sammelzeile spricht | veraltet sieht aus wie frisch | 13 / 2 / 1 | das Blatt weiß mehr, als es sagt |
| Jede Zeile trägt ihren Zustand | Marke an jeder der 16 Zeilen | 13 / 2 / 1 | 16 Marken für etwas, das meist „in Ordnung" heißt |
| **Nur die Ausnahme spricht** ✔ | veraltet = hohl, getrennt = leer | nur Abweichungen | „alles gut" wird durch Abwesenheit gezeigt |

**„Nicht perfekt" war die Aufgabe.** Der genannte Preis ist mit zwei
gemessenen Eingriffen beantwortet:

1. **Abwesenheit bekommt eine positive Aussage.** Ist nichts zu melden,
   sagt der Streifen „All 16 fresh" — genau dann, wenn sonst nichts dort
   stünde. Nicht das „13 fresh" der Sammelzeile: das zählte immer mit.
2. **Die Ausnahme spricht mit Namen**: „2 stale (Room Mic, Backing Vox) ·
   1 offline (Shaker)". Grenze gemessen, nicht gedreht — bei acht veralteten
   Quellen sprengt die Namensliste den Streifen um **42 px**. Regel: Namen,
   solange es eine Aufzählung ist (bis zwei je Sorte); acht Namen sind keine
   Aufzählung mehr, sondern eine Zahl.

Zustandsbilder in Graustufen, ohne dass eine Zeile ihre Höhe ändert:
**gefüllt = frisch · hohl = veraltet · Strichlinie = getrennt.**
Die Höhe darf sich nicht ändern, sonst liest man Energie, wo Alter gemeint ist.

### Die Zustandsmatrix

Sechs Szenarien durchschaltbar (gemischt · alles frisch · viel veraltet ·
alles getrennt · nur 5 Quellen · keine Quelle), dazu Messung 0/68/100 % und
degradierte Funktion an/aus. **Ein Blatt, das nur im Schönfall geprüft wurde,
ist nicht geprüft.**

### Was der schlimme Fall ans Licht gebracht hat

Beim Umschalten auf *alles getrennt* zeigte das Blatt weiter drei Befunde
über Bass, Choir und Drums — als wäre gerade gemessen worden. Behoben:

- **Befunde tragen den Zustand ihrer Quelle**: „source offline — from last
  measurement" bzw. „source stale — may be out of date".
- **Der Assistent lässt auf einer getrennten Quelle nichts mehr zu.** Ein
  Knopf, der sich drücken lässt und nichts tut, ist eine Lüge in der
  Oberfläche — dieselbe Regel, die schon in `licht.html` galt.
- **Messen setzt Kontakt voraus.** „Measurement complete" bei null Quellen
  und „Measuring 68 %" bei sechzehn getrennten sind dieselbe Lüge in zwei
  Feinheitsgraden. Jetzt: *Nothing to measure* bzw. *Measurement stalled —
  no source reachable*.

Der Leerzustand sagt, **warum** nichts da ist. Ein leeres Feld ohne
Erklärung wäre ein Ausfall, kein Zustand.

## licht.html

Die Fassung steht, offen ist das Licht. Vier **Herkünfte** (nur der Grund
angehoben · der Lichtleiter strahlt in den Raum · ein Schein hinter dem Glas ·
die Glaskanten fangen Licht) und vier Regler (Grundhelligkeit, Lichtmenge,
Farbanteil, Streuung).

Zwei Dinge stecken im Code, die leicht zu übersehen wären:

- Alles läuft über CSS-Variablen, nichts wird neu gebaut — der Regler wirkt
  ohne Sprung, und das Auge kann kleinen Unterschieden folgen.
- **Regler, die in der gewählten Herkunft nichts bewirken, sind abgeschaltet
  und zeigen „—".** Ein Regler, der sich bewegen lässt und nichts tut, ist eine
  Lüge in der Oberfläche.

„Original halten" zeigt gedrückt die unveränderte Ausgangsfassung — Vergleich
ohne Umschalten, damit sich das Auge zwischendurch nicht anpasst.

## stilbefragung.html

19 Runden auf echtem Nakama-Inhalt — 16 Zwangswahlen (pro Runde genau ein
Token verschieden) und 3 Reglerrunden zum Nachjustieren. (Stand vor dem
2026-08-21 stand hier „18 Runden"; gezählt sind es 19.) Erst sinnvoll, wenn die Welt steht — dann müssen ihre Spezimen
auf diese Welt umgestellt werden.

## Prinzipien, die im Code stehen und nicht verhandelbar sind

1. **Eine Variable pro Runde** — aber nur unterhalb der Weltebene. Auf
   Weltebene ändert sich absichtlich alles gleichzeitig, weil eine Welt kein
   Parameter ist.
2. **Kein Variantenname während der Wahl.** Ein Label wie „technisch" nimmt die
   Antwort vorweg. Die Kacheln heißen 1 bis 6; die Namen kommen erst im
   Ergebnis, als Vokabular für das Gespräch danach.
3. **Neutrales Mittelgrau als Umgebung.** Ein dunkles Werkzeug-Chrome ließe
   dunkle Kacheln gewinnen, ein helles die hellen.
4. **Die Großansicht skaliert dieselbe Kachel**, sie baut sie nicht neu — sonst
   verglichest du in Stufe 2 etwas anderes als das, was du ausgewählt hast.
5. **Kornstärke ist pro Welt verschieden.** Die Welt, deren Aussage „kein
   Material" ist, bekommt keine Struktur — sonst widerlegt die Kachel ihre
   eigene These.
6. **Demo-Kennzeichnung gehört zum Werkzeug, nicht zum Spezimen.** Sonst wäre
   sie Teil dessen, was beurteilt wird.
7. **Ablehnung wird erfasst.** Bei Abneigung sind Menschen präziser als bei
   Zuneigung, und der Designvertrag braucht Antireferenzen ohnehin.

## textur/

`korn.png` — 512er-Kachel, aus `assets/301.zip` herauskopiert (Mittelwert
exakt 128, also für `mix-blend-mode: overlay` gebaut), Kanten überblendet damit
sie nahtlos kachelt. Die Truhe selbst wurde nicht verändert.

## truhe/

Die **aufgeschlossene Assettruhe**. `assets/` enthält nur Formate, die eine
Sitzung nicht öffnen kann (`.sketch`, `.psd`, `.fig`) — Regel 1 („erst in die
Truhe schauen") war damit unbefolgbar. `truhe/aufschliessen.py` leitet lesend
Vorschauen ab; die Truhe selbst bleibt unangetastet (Prüfsummen gleich).

Was dabei sichtbar wurde und vorher niemand nachsehen konnte: **301 ist ein
Knopf-Kit ausdrücklich für Audio-VSTs** (ein grüner Akzent, immer als Wert,
Grund `#191A1E`, Korn über allem) — die Vorlage der Welt „Dunkles Gerät" und
die Quelle von `textur/korn.png`. **110** ist ein heller Thermostat-Ring mit
einem warmen Verlauf als Wert. **297** ist ein *Social*-Kit und hat keine
Vorschau (Vektor im alten Figma-Format) — ob es als Referenz oder als
Antireferenz taugt, entscheidet der User. Belege: `truhe/LIES-MICH.md`.
