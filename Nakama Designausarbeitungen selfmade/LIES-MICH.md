# Designausarbeitungen des Users — work in progress

Hier liegen **eigene Entwürfe des Users**, keine Werkzeuge und keine Abnahmen.
Sie sind ausdrücklich vorläufig. Wörtlich, 2026-08-21:

> „ignoriere das aktuelle design, das wird alles überschrieben ganz am ende.
> es geht um das layout und die technik am ende kommt das design. ist doch
> vollkommen egal was für eine körnung es hat"

**Daraus folgt die Leseregel dieser Mappe:** Was hier an *Material, Farbe,
Licht und Korn* zu sehen ist, ist **kein Gegenstand** — nicht als Vorbild und
nicht als Messung. Phase 1 läuft. Was hier an **Anordnung** zu sehen ist, ist
sehr wohl einer, denn genau darum geht es gerade.

---

## Was in der Mappe liegt

| Datei | Stand | was es zeigt |
|---|---|---|
| `1. Entwurf.png` · `Matte Polycarbonate.png` | 2026-08-20 | **pixelgleich** (gemessen: max. Abweichung 5 Stufen, 0,00 % der Pixel über 8) — dasselbe Bild zweimal exportiert. Materialstudie mit einer Spektrumsanzeige, ohne Bedienteil. |
| `Grundfläche Matte polycarbonat.png` | 2026-08-20 | das nackte Gehäuse ohne Inhalt |
| `polycarbonate-micrograin-512-seamless.png` | 2026-08-20 | Kornkachel, nahtlos, für `mix-blend-mode: overlay` gebaut |
| **Editor-Entwurf** *(unten beschrieben)* | 2026-08-21 | **liegt nicht als Datei vor** — kam über die Fernverbindung ins Gespräch. `Downloads\Frame 1.png` (01:33) ist ein älterer Stand **ohne** Bedienteil. Wer den neuen exportiert, legt ihn hier ab und trägt ihn hier ein. |

---

## Der Editor-Entwurf vom 2026-08-21 — und was er an Anordnung entscheidet

Er zeigt den **Active-Probe-Editor**, nicht Main. Beschrieben ist nur die
Anordnung; die Optik bleibt hier absichtlich unerwähnt.

### Was zu sehen ist

- **Kopf:** `ACTIVE PROBE · PIANO BUS` — Name und Rolle der Quelle stehen oben.
- **Kurvenfeld dominiert die Fläche.** Achsen `+12 / +6 / 0 / −6 / −12` dB
  rechts, `50 · 100 · 200 · 500 · 1k · 2k · 5k · 10k` unten.
- **Die acht Bänder sind nummerierte Griffe auf der Kurve** (1–8), keine
  gestapelten Zeilen. Band **3** ist gewählt (heller, größer), Band **7** ist
  hohl und gestrichelt gezeichnet — **aus**.
- **Genau EINE Bandzeile** unter dem Feld, für das gewählte Band, mit **allen
  zwölf** Parametern und einem Trennstrich zwischen EQ und Dynamik:
  `BAND 3 · TYPE BELL · FREQ 240 Hz · GAIN −1.5 dB · Q 1.20 · MODE STEREO`
  ‖ `DYN ON · THRESH −24 dB · RANGE −6 dB · ATTACK 12 ms · HOLD 0 ms · RELEASE 180 ms`
- **Draft vom Main** als eigener Kasten **über** dem Kurvenfeld, oben links:
  `DRAFT FROM MAIN` · `limit ±3 dB` · `Band 3 · 240 Hz · −1.5 → −3.0 dB · Q 1.6`
  · Knöpfe `HOLD TO AUDITION` · `APPLY` · `REJECT`. Im Kurvenfeld läuft der
  Draft als **gestrichelte** Kurve neben der durchgezogenen.
- **Schutzbereich** als schraffiertes senkrechtes Band im Kurvenfeld, mit
  Schloss und `PROTECTED`.
- **Statuszeile unten:** `CONNECTED · PAIRED` links · Wortmarke mittig ·
  rechts `GLOBAL` · `BYPASS` · `UNDO · 3`.

### Was das für `docs/sondenplan.md` heißt

| Offene Frage dort | Was der Entwurf zeigt |
|---|---|
| **„Wer gibt bei Platzmangel nach — Kurve oder Bänder?"** (Abschnitt 4) | **Die Bänder.** Der Entwurf ist **keine der fünf gemessenen Anordnungen**: nicht `D` („1 Slot voll + 7 **waagerechte Streifen**", 111 px Bandkasten) und nicht `E` („nur die Kurve, Werte auf Abruf", 0 px). Er liegt **dazwischen**: eine dauerhafte Bandzeile, und die sieben anderen Bänder leben **ganz im Kurvenfeld als Griffe** — also gar keine zweite gestapelte Zeile. Was dieser Bandkasten wirklich kostet, ist **noch nicht gemessen**; die Zahl gehört an `werkzeug/sonde-messung.html`, nicht in diese Datei. |
| **Ist Draft/Preview häufig oder Sonderfall?** (Abschnitt 3, Frage 2) | Der Entwurf behandelt ihn als **häufig**: eigener, dauerhaft sichtbarer Kasten mit drei Knöpfen. |
| **Annahme 3:** „kein Global-Bereich, keine Draft-Knopfreihe, kein Undo-Verlauf im Blatt" | Teils überholt. **Global** und **Undo** sind **Türen** in der Statuszeile (ein Knopf, ein Zähler), kosten also fast nichts. Die **Draft-Knopfreihe ist da** und kostet Höhe. |
| **Entwurfs-These:** „die Disclosure-Trennlinie liefe quer durch jedes Band" (die sechs Dynamik-Werte seien selten) | **Widerlegt vom Entwurf.** Die eine Zeile zeigt alle zwölf Werte gleichzeitig, Dynamik eingeschlossen. Die Trennlinie läuft **zwischen den Bändern**, nicht durch sie. |

### Was daran noch nicht beantwortet ist

- **Wie wählt man ein Band ohne die Kurve?** Acht Griffe auf einer Kurve sind
  nach Fitts kleine Ziele, und zwei dicht benachbarte Bänder überlappen.
- **Was zeigt die Zeile, wenn kein Band gewählt ist** — und was, wenn `DYN`
  aus ist? Fünf der zwölf Werte sind dann tot. Regel dafür steht bereits:
  was nichts bewirkt, ist abgeschaltet und zeigt „—".
- **Wo lebt der Undo-Verlauf**, wenn `UNDO · 3` nur ein Zähler ist?
- **Der Grenzfall fehlt im Bild:** acht Bänder aktiv, alle dynamisch, Bypass
  an, vom Main getrennt, Schutzbereich getroffen. Der Entwurf zeigt den
  Schönfall — und der Schönfall kostet die Hälfte.

**Nichts davon ist abgenommen.** Der Entwurf ist eine Vorstellung des Users,
kein Urteil über vorgelegte Varianten. Er ersetzt weder die Aufgabenliste nach
Häufigkeit (die fehlt weiterhin und blockt das Wireframe) noch die Messung am
lebenden Blatt — er sagt nur, wohin es gehen soll.

---

## Der Main-Entwurf vom 2026-08-21 — „erster ernstzunehmender Prototyp"

Zeigt **Main**. Anders als beim Editor ist Main auf Wireframe-Ebene
**vollständig abgenommen** — dieser Entwurf lässt sich also gegen bindende
Urteile prüfen, nicht nur beschreiben. Auch hier: nur Anordnung und Zeichen,
keine Optik. Liegt ebenfalls nicht als Datei vor.

### Was zu sehen ist

- **Streifen:** `NAKAMA / MAIN` · vier Reiter `OBSERVE · ADVISE · AUDITION ·
  APPLY` (ADVISE aktiv, unterstrichen) · rechts `5 PROBES` · `1 STALE` ·
  `● MEASURING 68%` · `WIDTH UNAVAILABLE`.
- **`PROBE OVERVIEW`** mit Kopfzeile `FOCUS / BASS / 180-260 HZ` und rechts
  `5 / 5 LINKED`. Frequenzspalten `20 · 60 · 120 · 250 · 500 · 1K · 2K · 4K · 8K`.
- **Fünf Zeilen, mit Namen und Nummer:** `01 LEAD VOX LIVE` · `02 DRUMS LIVE` ·
  `03 BASS FOCUS` · `04 PIANO LIVE` · `05 SYNTH PAD STALE`.
  Die BASS-Zeile trägt links einen Marker; ihre Zellen bei 120 und 250 sind
  umrandet — der Befundort.
- **`ADVISOR`** mit `FINDING 1 OF 3` und rechts `5 / 5`, darunter die
  Fünf-Teile-Form: `PRIORITY` · `LIKELY CAUSE` · `SMALLEST TEST` ·
  `LISTEN FOR` · `THEN`.
- **Knöpfe** `AUDITION / HOLD` · `APPLY` · `DISCARD`, rechts daneben
  `● TEST READY / BASS −2.0 DB @ 220 HZ / Q 1.2`.

### Was mit den Abnahmen übereinstimmt

- **Overview + Detail** — dauerhafte Karte oben, wechselnde Arbeitsfläche
  darunter. Genau die abgenommene Mechanik.
- **Die Fünf-Teile-Form des Assistenten** stimmt auf den Punkt mit
  `docs/oberflaechen-spezifikation.md`: Priorität → wahrscheinliche Ursache →
  kleinster Versuch → worauf hören → danach dieselbe Passage erneut prüfen.
- **Vier Bedienebenen, sichtbar getrennt**, der aktive leuchtet.
- **Degradiert wird ehrlich gezeigt:** `WIDTH UNAVAILABLE` steht im Streifen,
  nicht versteckt.
- **Der gemessene Zähler hat seine eigene Form** (`5 / 5`, `5 / 5 LINKED`)
  neben dem geschriebenen `FINDING 1 OF 3` — genau `PRUEFLISTE` 1.2, die aus
  dem Fehler „finding 1 of 3 · 3 of 5" entstanden ist.
- **Der kleinste Versuch ist ein vorbereiteter Zustand**, nicht nur Text:
  `TEST READY / BASS −2.0 dB @ 220 Hz / Q 1.2`.

### Drei Abweichungen von bindenden Urteilen

**1. `05 SYNTH PAD` ist als *getrennt* gezeichnet und als *veraltet*
beschriftet.** `abnahmen/2026-08-20-zustaende-nur-ausnahme.md` legt fest:

| Zustand | Zeichen |
|---|---|
| frisch | gefüllte Zellen |
| veraltet | **hohle** Zellen — Daten da, aber alt |
| getrennt | **Strichlinie**, keine Zellen — keine Daten |

Die Zeile trägt die **Strichlinie** und das Wort **`STALE`**. Zeichen und Wort
sagen Verschiedenes. Nach der Abnahme müsste eine veraltete Zeile **hohl**
sein. (Das ist die Klasse Fehler, für die `PRUEFLISTE` 4.4 existiert: keine
Beschriftung darf behaupten, was das Bild widerlegt.)

**2. `1 STALE` nennt den Namen nicht.** Dieselbe Abnahme, Punkt 2: *„Die
Ausnahme spricht mit Namen … Regel: **Namen, solange es eine Aufzählung ist**
— bis zwei je Sorte."* Bei **einer** veralteten Quelle ist der Name Pflicht:
`1 stale (Synth Pad)`. Die Begründung steht in der Abnahme: eine
Zustandsmarke ohne Namen sagt nur die Position, nicht die Quelle.

**3. Der Zeilenmarker ist doppelt belegt.**
`abnahmen/2026-08-20-vorhoeren-markierte-zeile.md` reserviert die **markierte
Zeile** für **das Vorhören**: *„Die betroffene Quelle in der Karte wird
markiert, **solange gehalten wird**; beim Loslassen ist die Markierung weg."*
Im Entwurf markiert derselbe Balken in der **Ruhelage** die Fokusquelle
(`03 BASS FOCUS`). Zwei Begriffe unter einem Zeichen — genau der Fehler aus
`PRUEFLISTE` 4.1, der schon einmal passiert ist (`fokus` hieß gleichzeitig
Disclosure-Behandlung und betroffene Quelle). Entweder braucht *Fokus* ein
eigenes Zeichen, oder *Vorhören* eines.

### Zwei Pflichtangaben der Spezifikation fehlen

`docs/oberflaechen-spezifikation.md`, Abschnitt 1, verlangt je Befund: Ort ·
Beobachtung · Begründung · **Sicherheit (hoch / mittel / noch unklar)** ·
**mögliche Alternativursachen** · billigster nächster Beweisschritt.

Vorhanden sind Ort (`BASS / 180-260 Hz`), Begründung (`LIKELY CAUSE`) und der
billigste Beweisschritt (`SMALLEST TEST`). **Sicherheit und Alternativursachen
haben keinen Ort** — beide stehen schon in `abnahmen/2026-08-21-codereview.md`
als Lücke, hier sind sie es wieder.

### Der Lastfall ist der Schönfall

Das Bild zeigt **fünf** Quellen. Die Spezifikation nennt **bis 16**, und die
gesamte Kartenabnahme wurde bei 16 gemessen und entschieden. Zwei Folgen:

- **Die Namen in der Karte passen hier, bei 16 nicht.** Gemessen: Karte ohne
  Namen 212 px, **mit** Namen 308 px — mehr, als da ist. Deshalb war die
  abgenommene Karte namenlos.
- **Genau darum ist das trotzdem interessant.** Die offene Frage des letzten
  Sessionendes lautete wörtlich: *„Wie kommt man von einer hohlen Zeile zur
  Quelle? Die Karte ist namenlos."* Dieser Entwurf **antwortet** darauf — er
  beantwortet sie nur für den kleinen Mix. Was bei 16 Quellen passiert, ist
  die eigentliche Frage.

**Auch das ist keine Abnahme.** Die drei Abweichungen sind keine Kritik am
Entwurf, sondern Stellen, an denen entweder das Bild oder das frühere Urteil
nachzieht — und das entscheidet der User, nicht Claude.
