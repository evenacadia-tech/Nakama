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
