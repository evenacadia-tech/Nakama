# Werkzeug — Messinstrumente, keine Entwürfe

Hier liegen Hilfsmittel, mit denen die Vorstellung des Users **gemessen** wird.
Nichts davon ist ein Gestaltungsvorschlag. Layoutentscheidungen gehören nach
`prototyp/` und in Phase 1 des Arbeitsplans.

## Reihenfolge — nach den Stufen aus `assets/GUI-Tips.pdf`

**Stufe 1 — The Feeling** *(abgeschlossen)*
1. `weltenwahl.html` — welche Welt? → **Glas und Licht**
2. `glaswelt.html` — welche Verwirklichung? → **dunkles Glas, ein Lichtleiter**

**Phase 0.5 — Formfaktor** *(hier stehen wir)*
3. `formfaktor.html` — wie groß darf das Fenster sein, und welche Form hat es?
   Läuft bei **1:1**, nichts wird skaliert, und es misst, ab wann Inhalt
   abgeschnitten wird. Muss vor jedem Wireframe entschieden sein.

**Stufe 2 — UX Layout**
4. `wireframe-main.html` — Schritt 1: Wo liegt was? Graustufen, keine Optik.
   **Main zuerst**, danach Active-Probe-Editor, dann Passive-Kachel.
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

Fünf Verwirklichungen der abgenommenen Welt, gleiche Information an gleicher
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
offen ist die **Karte** — das einzige Stück Mix, das dauerhaft dasteht. Vier
Kartenarten nebeneinander, alle mit demselben Streifen und derselben
Arbeitsfläche. Verschieden ist nur, **wie die Karte 16 Quellen auf wenig Höhe
bringt.**

### Die Klemme, gemessen auf 760×430

Der Streifen kostet dauerhaft **28 px**; für Karte und Arbeitsfläche bleiben
**370**. Gemessene natürliche Höhen bei 744 px Breite:

| | braucht |
|---|---|
| Karte ohne Namen, 16 Quellen | 212 px (rund 10 px je Zeile) |
| Karte **mit** Namen, 16 Quellen | 308 px — geht nicht |
| Assistent, Fünf-Teile-Form | **172 px** |
| Alle drei Befunde | 219 px |
| A/B-Urteil | 120 px · Detail einer Quelle 69 px |

Daraus: alle 16 dauerhaft (212) lässt der Arbeit **158** — der Assistent
braucht 172. Bekommt der Assistent seine 172, trägt die Karte **14**. Sollen
alle drei Befunde stehen (219), sind es **neun**. Die Klemme ist echt und
keine der vier Karten umgeht sie.

### Was jede Kartenart im schlechtesten der drei Schritte liefert

| Kartenart | Karte | Assistent | Befunde |
|---|---|---|---|
| Alle 16, gleich hoch | 16/16 | **3/5** | **1/3** |
| Karte weicht der Arbeit | **7/16** | 5/5 | 3/3 |
| Karte nach Bedeutung | 16/16 (3 mit Namen) | 5/5 | **1/3** |
| Karte nach Rollen | 5/5 Gruppen | 5/5 | 3/3 |

Die **Fünf-Teile-Form des Assistenten ist Spezifikation**, kein Geschmack —
`3/5` heißt, dass zwei vorgeschriebene Teile fehlen.

### Zwei Dinge, die aus Runde 1 gelernt und hier eingebaut sind

- Der Assistent ist eine **ehrliche Liste**: wenn ihm die Höhe fehlt, zeigt er
  `3/5` und „2 more not shown", statt heimlich abzuschneiden oder bloß rot
  aufzuleuchten. Man sieht, **welcher** Teil fehlt.
- Der gemessene Zähler hat eine **eigene Form** (`3/5`), weil im selben Kopf
  oft ein geschriebenes „finding 1 of 3" steht. Zwei gleich aussehende Zähler
  mit verschiedener Bedeutung sind schlimmer als keiner.
- Eine Prüfung schlägt an, wenn **Listenname und Bausteinname auseinander
  laufen** — genau daran hat der Vertrag in Runde 1 stumm keine Zahl gemeldet.

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

18 Runden Zwangswahl auf echtem Nakama-Inhalt, pro Runde genau ein Token
verschieden. Erst sinnvoll, wenn die Welt steht — dann müssen ihre Spezimen
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
