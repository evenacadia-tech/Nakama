# Figma-Stände — die Vorgabe

**Herkunft:** Figma, gestaltet vom **User** (Gen, Probeeq) bzw. von Claude auf
seinen Auftrag in seiner Figma-Datei (Suna, siehe unten). Exportiert vom User
am 2026-08-21 (20:04 / 20:05 / 22:18) in seinen Downloads-Ordner, von dort
**bytegleich** hierher kopiert (SHA-256 unten). Die Dateien werden nicht
bearbeitet, nicht verkleinert, nicht umbenannt.

**Status: aktueller Stand — Vorgabe für die Übersetzung.** Ein Design
entwickelt sich, neuere Stände ersetzen diese. Wörtlich, Interview
2026-08-21:

> „das finale design wird aktuell in figma gemacht . alle 3 apps werden ein
> design haben mit der selben identität. alle alten sind alt. Ein Design
> entwickelt sich und ist nicht einfach da. was jetzt aktuell ist kann auch
> wieder weiter entwickelt werden."

Und zur Einordnung jedes Bildes (2026-08-21):

> „Das ist ein Designprototyp keine technikanleitung, design passt sich am
> ende der funktion an."

**Daraus folgt die Leseregel:** Was hier zu sehen ist, ist die Richtung und
die Anordnung — das Repo übersetzt sie in lebende Blätter und prüft, was ein
Bild nicht prüfen kann: **Zustände, Größen, Grenzfälle.** Keine eigene
Stilsuche, keine Varianten-Befragung. Wo ein Bild etwas anderes zeigt als eine
bindende Abnahme in `abnahmen/`, wird das beim Übersetzen **benannt, nicht
still entschieden** (Beispiel: `abnahmen/2026-08-21-designvertrag.md`,
„Was ausdrücklich offen bleibt").

## Regeln für neue Stände

1. Neuer Export vom User → neue Datei `JJJJ-MM-TT-<app>.png` (`gen`,
   `probeeq`, `suna`). Der alte Stand bleibt liegen — er ist Verlauf, und er
   belegt, wogegen ein Blatt gebaut wurde.
2. Diese Datei bekommt eine Zeile je Stand; der jeweils neueste ist die
   Vorgabe. Kein Bild wird überschrieben.
3. Ein Stand wird hier **beschrieben** (was zu sehen ist), nicht beurteilt.

## Was in der Truhe liegt

| Datei | Export | Pixel | SHA-256 (Anfang) | Gerätekörper im Bild | zeigt |
|---|---|---|---|---|---|
| `2026-08-21-gen.png` | 2026-08-21 20:05 | 3296×1976 RGBA | `6d3f7378fe0087de` | 3040×1720 px **= 4 × 760×430** (gemessen über `alpha == 255`; der Rest ist Schatten) | **Nakama Gen** (Main) |
| `2026-08-21-probeeq.png` | 2026-08-21 20:04 | 3600×2080 RGBA | `c8c10a5e43bfe680` | 2800×1680 px **= 4 × 700×420** (gemessen über die Helligkeitskante; der Rest ist Hintergrund) | **Nakama Probeeq** (aktive Sonde) |
| `2026-08-21-suna.png` | 2026-08-21 22:18 | 3244×2080 RGBA | `d6d288d603750e0d` | 1040 px breit **= 4 × 260** (gemessen über die Helligkeitskante der Schale bei x 1280–2319); Höhe 336 = 4 × 84, gemessen: Glas 992 px = 4 × 248 breit. Der Rest ist der Host-Grund 811×520 (#121315, Ecken transparent) | **Nakama Suna** (passive Sonde) |

Alle drei Geräteflächen entsprechen **genau** den am 2026-08-20 abgenommenen
Fenstergrößen (`abnahmen/2026-08-20-groessen-alle-drei.md`), vierfach
exportiert.

**Herkunft des Suna-Stands, anders als bei Gen und Probeeq:** die Kachel hat
**Claude** am 2026-08-21 in der Figma-Datei des Users entworfen — auf dessen
Auftrag, in der Sprache des markierten Probeeq-Stands
(`abnahmen/2026-08-21-suna-auftrag-figma.md`, Studie 03, Figma-Node 6039:2).
Der User hat sie am selben Abend aus Figma exportiert und hier abgelegt. Mit
dem Export ist sie User-Wort (CLAUDE.md, Regel 4: „Ein neuer Figma-Stand ist
neues User-Wort") — der Stand gilt, die Urheberschaft bleibt vermerkt. Die
Export-Datei hieß `Suna.png` im Downloads-Ordner (22:18), kopiert 22:20.

## `2026-08-21-gen.png` — Nakama Gen

Dunkle Fläche in einem abgerundeten Gehäuserahmen mit weichem Schatten;
Akzentfarbe Cyan; Beschriftungen in Monospace-Versalien, der Fließtext des
Advisors proportional. Alle Texte englisch.

- **Kopfzeile:** Wortmarke `NAKAMA / MAIN` · vier Reiter `OBSERVE · ADVISE ·
  AUDITION · APPLY` (`ADVISE` aktiv, cyan unterstrichen) · rechts `5 PROBES` ·
  `1 STALE` · `● MEASURING 68%` · `WIDTH UNAVAILABLE`.
- **`PROBE OVERVIEW`** mit Kopf `FOCUS / BASS / 180-260 HZ` und rechts
  `5 / 5 LINKED`. Neun Frequenzspalten `20 · 60 · 120 · 250 · 500 · 1K · 2K ·
  4K · 8K`.
- **Fünf Zeilen** mit Nummer, Name und Zustandswort: `01 LEAD VOX LIVE` ·
  `02 DRUMS LIVE` · `03 BASS FOCUS` · `04 PIANO LIVE` · `05 SYNTH PAD STALE`.
  Die Zellen sind gefüllte dunkle Kacheln; bei `BASS` sind die Zellen `120`
  und `250` cyan umrandet (der Befundort) und die Zeile trägt links einen
  Marker; die `STALE`-Zeile ist durchgehend **gestrichelt** gezeichnet.
  Am rechten Rand der Übersicht steht ein schmaler senkrechter cyan Balken
  ohne Beschriftung.
- **`ADVISOR`** mit `FINDING 1 OF 3` und rechts `5 / 5`, darunter die
  Fünf-Teile-Form als Tabelle: `PRIORITY` „Bass against piano, 180–260 Hz" ·
  `LIKELY CAUSE` „Bass carries 3.4 dB too much there" · `SMALLEST TEST`
  „Bass −2 dB at 220 Hz, Q 1.2" · `LISTEN FOR` „Piano left hand, bars 33–41"
  · `THEN` „Re-check the same passage".
- **Knöpfe** `AUDITION / HOLD` (cyan umrandet) · `APPLY` · `DISCARD`; rechts
  daneben `● TEST READY / BASS −2.0 DB @ 220 HZ / Q 1.2`.

Anmerkung, keine Wertung: die Wortmarke im Bild lautet noch `NAKAMA / MAIN`;
der Name **Gen** stammt aus dem Interview vom selben Tag.

## `2026-08-21-probeeq.png` — Nakama Probeeq

Dunkle Fläche in einem abgerundeten Gehäuserahmen auf dunklem Hintergrund.
Die Kurve und ihre Füllung sind rötlich, die Bandgriffe tragen einen kühlen
Leuchtsaum; Beschriftungen in Monospace-Versalien. Alle Texte englisch.

- **Kopf:** `ACTIVE PROBE · PIANO BUS`.
- **Kasten oben links** über dem Kurvenfeld: `DRAFT FROM MAIN` · `limit ±3 dB`
  · `Band 3 · 240 Hz · −1.5 → −3.0 dB · Q 1.6` · Knöpfe `HOLD TO AUDITION`
  (umrandet) · `APPLY` · `REJECT`.
- **Kurvenfeld** dominiert die Fläche. dB-Achse rechts `+12 / +6 / 0 / −6 /
  −12`, Frequenzachse unten `50 · 100 · 200 · 500 · 1k · 2k · 5k · 10k`.
  **Acht nummerierte Griffe** auf der Summenkurve; Band `3` ist gewählt
  (gefüllt, größer), Band `7` ist hohl und gestrichelt (aus). Unter Band 3
  hängt ein zweiter, gestrichelter Griff mit gestrichelter Kurve — der Draft.
  Zwischen etwa 3 k und 5 k ein **schraffiertes senkrechtes Band** mit
  Schloss und `PROTECTED`.
- **Eine Parameterzeile** unter dem Feld, für das gewählte Band, mit einem
  Trennstrich zwischen EQ und Dynamik: `BAND 3 ●` · `TYPE BELL` · `FREQ 240 Hz`
  · `GAIN −1.5 dB` · `Q 1.20` · `MODE STEREO` ‖ `DYN ON` · `THRESH −24 dB` ·
  `RANGE −6 dB` · `ATTACK 12 ms` · `HOLD 0 ms` · `RELEASE 180 ms` — **zwölf
  sichtbare Parameter**.
- **Fußzeile:** `● CONNECTED · PAIRED` links · Wortmarke `NAKAMA PROBEEQ`
  mittig (`NAKAMA` klein und mehrfarbig, `PROBEEQ` groß, als Kontur) ·
  rechts `GLOBAL` · `BYPASS` · `UNDO · 3`.

Anmerkung, keine Wertung: zwölf Parameter je Band sind sichtbar; ob eine
Sidechain-Quelle als dreizehnter dazukommt, ist im Technik-Repo offen
(`docs/oberflaechen-spezifikation.md`, Abschnitt 2).


## `2026-08-21-suna.png` — Nakama Suna

Dieselbe Materialsprache wie Probeeq: dunkle Graphit-Schale mit abgerundeten
Ecken und weichem Schatten auf dem Host-Grund, darin eine dunklere,
eingelassene Glasfläche; unter dem Glas ein schmaler Körperstreifen (Kinn).
Beschriftungen in Geist-Versalien, Werte in Geist Mono. Alle Texte englisch.
Gezeigt ist **eine** Kachel im Schönfall; der Export enthält keine Zustände.

- **Glas, Kopfzeile:** `PASSIVE PROBE` links oben, klein und grau.
- **Glas, Parameterzeile** in Label-über-Wert-Form, drei Spalten:
  `BUS` / `PIANO` (hell) · `ROLE` / `CARRIES` (grau) · `POSITION` / `POST`
  (grau). Rechts oben im Glas ist nichts — der Hinweisplatz ist leer.
- **Kinn:** links ein kleiner Punkt mit kühlem Leuchtsaum und `CONNECTED`;
  mittig die Wortmarke `NAKAMA` (klein, Buchstaben rot→blau verlaufend) über
  `SUNA` (groß, dünn, mit Verlauf); rechts `MEASURING`.
- **Maße im Bild:** Schale 1040 px breit (gemessen; = 260) und 336 px hoch
  (= 84), Glas 992 px breit (gemessen; = 248) mit 24 px Rand (= 6); Kinn
  rechnerisch 112 px (= 28).

Anmerkung, keine Wertung: der Export zeigt den Host-Grund 811 Einheiten breit
statt 900 wie bei Probeeq; die Kachel sitzt bei x = 320, also nicht mittig im
exportierten Grund. Für die Übersetzung ist nur die Kachel maßgeblich.
