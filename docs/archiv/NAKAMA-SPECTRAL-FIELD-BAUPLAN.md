> **ARCHIV (21.08.2026). Bauplan 2.0 „Spectral Field“ (16./17.08.). Sein „Verbindlich“ war Claude-Text ohne User-Zitat; User 21.08.: „alle alten sind alt“. Keine Vorgabe mehr. Gültige Produktdefinition: CLAUDE.md (Wahrheitskern + Entscheide-Register); technischer Entwurf: docs/FL-Nakama-Sonden-Design-Entwurf.md.**

# Nakama — Spectral Field

## Verbindlicher Bauplan 2.0 für die VST3-Hauptansicht

> **Stand:** 2026-08-17
> **Produktname:** **Nakama**
> **Ziel:** Windows 11 · FL Studio · VST3 · JUCE 8/C++20
> **Scope-Besitzer:** `cpp-cmake`
> **Status:** Der interaktive 2D-Vorentwurf besitzt die verbindliche
> Nebel→Licht-Evidenzgrammatik. Der Umbau der produktiven JUCE-Ansicht ist noch
> nicht erfolgt.

Dieser Bauplan ersetzt die frühere HUD-, Chip-, Top-Drei- und Futura-Fassung
vollständig. Er beschreibt die gemeinsam festgelegte Richtung nach der
Besprechung von Smooth Operator Pro und DERES.

---

## 1. Vorrang und Referenzen

Bei Widersprüchen gilt:

1. Quellcode und ausführbare Verträge sind Wahrheit für den Ist-Zustand.
2. Dieses Dokument ist der Zielvertrag für die neue sichtbare Hauptansicht.
3. `eq-copilot/schemas/*.schema.json` bleibt für persistierte und übertragene
   Daten maßgeblich. Neue Berichtsfelder erfordern zuerst eine Versionierung.
4. DSP-, Realtime- und Broker-Regeln aus `FL-EQ-Copilot-Recherche.md` bleiben
   bestehen, sofern dieser Bauplan sie nicht enger fasst.
5. Der HTML-Entwurf ist eine Geometrie-, Bewegungs- und Interaktionsreferenz,
   aber kein DSP-Beweis. Seine Beispieldaten dürfen nicht in das Plugin
   übernommen werden.

Referenzartefakte:

- Interaktiver Entwurf:
  `eq-copilot/design/nakama-spectral-field-vorentwurf.html`
- Verbindlicher Zeichen- und Schriftatlas:
  `eq-copilot/design/concepts/nakama-field-alphabet-v2.html`
- Historische Markenreferenz:
  `eq-copilot/design/concepts/nakama-symbolalphabet-v1.png`
- Wiederverwendbare Vektorquelle für Marke, Probleme und Werkzeuge:
  `eq-copilot/design/assets/nakama-symbols.svg`
- Spiegelung der kanonischen SVG-Symbole in Entwurf und Atlas:
  `node eq-copilot/design/sync-nakama-symbols.mjs`
- Unverbindliche Bewegungsstudie für hochwertigere Problemreaktionen
  (`Ensō`, `Siegel`, `Material`); sie ersetzt die festgelegte Markersprache erst
  nach einer ausdrücklichen Richtungsentscheidung:
  `eq-copilot/design/nakama-problem-effects-studie.html`
- Vergleichsbilder der beiden Stressfälle:
  `eq-copilot/design/vorentwurf-renders/21-nakama-resonanz-effekte.png` und
  `eq-copilot/design/vorentwurf-renders/22-nakama-maskierung-effekte.png`
- Bewegte Handyvorschauen derselben Stressfälle:
  `eq-copilot/design/vorentwurf-renders/23-nakama-resonanz-effekte.gif` und
  `eq-copilot/design/vorentwurf-renders/24-nakama-maskierung-effekte.gif`
- Lokale Schriften:
  `eq-copilot/design/fonts/NakamaPhase-Regular.ttf` und
  `eq-copilot/design/fonts/Manrope-Variable.ttf`
- Reproduzierbare Quelle der Nakama-Schrift:
  `eq-copilot/design/fonts/gen-nakama-phase.py`
- Schriftlizenz:
  `eq-copilot/design/fonts/OFL-Manrope.txt`
- Geprüfte Referenzbilder:
  `eq-copilot/design/vorentwurf-renders/`

---

## 2. Produktentscheidung

### 2.1 Ein Satz

**Nakama ist ein bildschirmfüllendes Spektralfeld, dessen Kurve sich wie eine
gespannte, leichte Membran bewegt; Probleme erscheinen direkt an ihrer
Frequenz als überlagerbare Symbolkreise, während alle Zahlen und Erklärungen
erst im bewusst geöffneten Befundarchiv sichtbar werden.**

### 2.2 Qualitätsziel

Die Ansicht soll gleichzeitig vermitteln:

- Wertigkeit statt generischem Dashboard;
- Freude durch eine flüssige, physisch glaubwürdige Darstellung;
- Leidenschaft und Detailarbeit statt „ein Prompt, ein UI“;
- unmittelbares Verständnis ohne Fließtext;
- fachliche Tiefe, sobald der User sie bewusst öffnet.

### 2.3 Nicht verhandelbar

1. Der Graph nutzt die gesamte Editorfläche. Es gibt keinen Gerätebody um
   einen kleineren Plot.
2. In der Hauptansicht stehen dauerhaft nur die **Nakama-Bildmarke**,
   Frequenzachse, dB-Achse und Symbole. Keine Sätze, Statuspills oder
   Zahlenkarten.
3. Achsen bleiben gut lesbar, wirken durch Tonwert und Gewicht aber eine Ebene
   hinter Kurve und Befunden.
4. Befunde tragen kein Textetikett. Jeder Typ hat ein eigenes Symbol in einem
   Kreis.
5. Die Füllung des Kreisrings zeigt die Dringlichkeit. Farbe allein ist nie
   Bedeutungsträger.
6. Gleichzeitige Probleme dürfen sich überlagern. Es gibt kein Top-Drei-Limit,
   keine künstliche Entzerrung und kein Verstecken eines schwierigen Mixes.
7. Ein kurzzeitiges Problem reagiert sofort und normalisiert sich sichtbar
   wieder. Sein Bericht bleibt im Archiv erhalten.
8. Ein Marker öffnet nicht automatisch Details. Das Befundarchiv wird nur über
   sein festes Symbol in der Ecke geöffnet und geschlossen.
9. Liegen Berichte vor, glimmt das Archivsymbol gedimmt. Kein Badge, keine Zahl,
   kein blinkender Alarm.
10. Tooltips sind global zuschaltbar. Im Normalzustand bleiben sie aus.
11. Es gibt mehrere umschaltbare Farbpakete. Die Informationslogik bleibt in
    jedem Paket identisch.
12. Nakama berät nur. Es setzt keine EQ-Parameter, schreibt keine Automation
    und verändert das Audiosignal nicht.

---

## 3. Informationsarchitektur

### 3.1 Ebenen von hinten nach vorn

1. Grundfläche des gewählten Farbpakets
2. logarithmisches Frequenzraster und dB-Raster
3. gespeicherte Vergleichslinie, falls vorhanden
4. Materialfläche der geglätteten Messkurve
5. feine Live-Faser
6. typabhängige Problemfelder
7. Symbolkreise mit Dringlichkeitsring
8. Marke und Werkzeugkreise
9. bewusst geöffnetes Farbpaket- oder Befund-Popout
10. Tooltip, nur wenn aktiviert

Die Ebenen dürfen nicht zu einer milchigen Alpha-Suppe werden. Jede Ebene hat
eine Aufgabe und einen klar begrenzten Kontrast.

### 3.2 Marke und permanenter Text

Die Wortmarke `NAKAMA` wird nicht mehr gesetzt. Links oben steht die eigene,
vertikale Bildmarke **仲間**. Ihre unregelmäßige Pinselkontur bildet den
organischen Gegenpol zu Raster, Achsen und Werkzeugkreisen. Ein einzelner
Pinselabschluss trägt das feste Markengold `#d4a15d`; der übrige Körper folgt
dem hellen beziehungsweise dunklen Textton des Farbpakets. Die Bildmarke wird
als Pfadgeometrie gezeichnet, nicht als CJK-Schriftzeichen aus einer
Systemschrift und nicht als Bitmap.

In der geschlossenen Hauptansicht ist nur folgender Text erlaubt:

- X-Achse: `20`, `50`, `100`, `200`, `500`, `1k`, `2k`, `5k`, `10k`, `20k`
- Y-Achse: `−6`, `−12`, `−18`, `−24`, `−30`, `−36`, `−42`, `−48`
- die zurückgenommenen Einheiten `HZ` und `DB`

Kein Messstatus, keine ausgeschriebenen Problemnamen und keine permanenten
Kennzahlen kommen hinzu.

### 3.3 Werkzeugkreise

Oben rechts, in dieser Reihenfolge:

1. Tooltips ein/aus
2. Farbpaket wählen
3. Befundarchiv öffnen/schließen
4. Schnappschuss speichern
5. Messung starten/stoppen

Alle Werkzeuge sind kreisförmige Vektorsymbole mit verborgener zugänglicher
Beschriftung. Ihre Zeichen stammen aus demselben Nakama-Field-Alphabet wie die
Befunde; keine fremde Outline-Iconfamilie wird zugemischt. Der Start/Stop-Kreis
ist leicht größer. Der Archivkreis glimmt gedimmt, sobald mindestens ein
gespeicherter Befund vorliegt.

### 3.4 Popouts

- Farbpakete erscheinen als kleines Popout unter den Werkzeugen.
- Das Befundarchiv erscheint ab 901 px Breite als rechte Seitenfläche.
- Bis 900 px Breite wird das Archiv zu einem unteren Sheet.
- Beide schließen nur bewusst: Schließen-Symbol, erneuter Werkzeugklick oder
  `Esc`.
- Das Öffnen eines Popouts stoppt die Messung nicht.

---

## 4. Typografie und Sprache

### 4.1 Schrift

Verbindlich ist eine Zweiteilung:

- **Nakama Phase** ist die eigene Interface-Stimme für Achsen, Einheiten,
  Überschriften, Befundnamen, Kurzmetadaten und Kennungen. Ihre verjüngten
  Laststriche, bewussten Atemschnitte und kleinen Ankerrauten folgen exakt der
  Grammatik der Diagnose- und Werkzeugzeichen. Die Geometrie ist
  projektbesessen und wird reproduzierbar durch `gen-nakama-phase.py` als TTF
  und WOFF2 gebaut.
- **Manrope Variable** bleibt die Leseschrift für längere Maßnahmen,
  Erklärungen und Berichtswerte. Hier gewinnen offene Formen,
  Gewichtsabstufungen und tabellarische Ziffern gegen Markenwirkung.

Beide Fonts liegen lokal. Manrope steht unter OFL; Nakama Phase besteht aus
eigener Geometrie. Im JUCE-Build werden die TTF-Dateien über
`juce_add_binary_data` oder einen äquivalenten eingebetteten Ressourcenweg
geladen. Kein Laufzeit-Download, keine Abhängigkeit von installierten
Windows-Schriften. Nakama Phase darf wegen ihrer absichtlich offenen Striche
nicht für längere Prosa eingesetzt werden.

### 4.2 Grade

- Bildmarke: 29–50 px breit bei unverändertem Seitenverhältnis `510:890`
- Achsen: Nakama Phase, mindestens 11,5 px äquivalent, Tracking etwa `0.085em`
- Einheiten: Nakama Phase, mindestens 9,5 px äquivalent, Tracking etwa `0.14em`
- Berichtsnamen: Nakama Phase, 13–15 px äquivalent, Tracking etwa `0.09em`
- Berichtswerte: 13–15 px äquivalent, tabellarische Ziffern
- Metadaten: Nakama Phase, 9–11 px äquivalent, nur kurze Fachwörter

### 4.3 Formulierung

Die Hauptansicht benutzt keine Prosa. Im Archiv gelten kurze, professionelle
Begriffe, zum Beispiel:

- `Resonanz`, `Aufbau`, `Mittenloch`, `Maskierung`, `Schärfe`
- `dauerhaft`, `kurzzeitig`, `akut`, `prüfen`, `Politur`
- `Überstand`, `Musikzeit`, `Breite`, `Sicherheit`

Eine Handlungsempfehlung bleibt eine knappe Fachzeile wie
`Dynamisch · 3,2–3,8 kHz`. Keine erklärenden Absätze im Popout.

### 4.4 Tooltips

- standardmäßig aus;
- global über das Hilfesymbol schaltbar;
- erscheinen bei Hover und Tastaturfokus;
- enthalten nur den Werkzeug- oder Problemnamen;
- ersetzen keine zugänglichen Namen.

---

## 5. Farbpakete

Die Pakete sind Alternativen, keine gleichzeitige Regenbogenpalette. Pro Paket
gibt es eine Grundfläche, eine Materialkurve, zurückgenommene Rasterfarben,
einen Akzent und semantische Erfolgs-/Fehlerfarben.

| Paket | Charakter | Grundfläche | Akzent |
|---|---|---:|---:|
| Graphit | warmes Studio-Schwarz, ruhig, materiell | `#080a0b` | `#d4a15d` |
| Rosé | Smooth-Operator-Nähe ohne Kopie | `#09080b` | `#ee93c3` |
| Signal | kühl, präzise, technisch | `#080a0b` | `#1fd3df` |
| Violett | nächtlich, atmosphärisch | `#08090d` | `#b89cf1` |
| Papier | helle, taktile Alternative | `#d5d3cc` | `#a6613e` |
| Sumi-Washi | japanisch, naturverbunden; Washi, Indigo, Moos und Zinnober | `#0d0d0b` | `#527987` |

Im Sumi-Washi-Paket bleibt die Fläche neutrales, warmes Sumi statt farbigem
Grün. Die Naturakzente sind sichtbar gebunden: Zinnober an Resonanzen, Gold an
Transienten, Moos an Balancefeldern und Indigo an Maskierung. Symbol und Form
bleiben jeweils der primäre Bedeutungsträger; Farbe unterstützt sie nur.

Regeln:

- maximal ein dominanter Akzent pro Paket;
- das feste Markengold `#d4a15d` bleibt als kleiner Identitätsakzent in allen
  Paketen erhalten und übernimmt keine Zustandsbedeutung;
- Kurve und Achsen müssen in jedem Paket den gleichen Informationsrang haben;
- Dringlichkeit bleibt Ringfüllung plus Form, nicht Rot gegen Grün;
- Kontrast wird für Achsen, Fokus und Berichtswerte in jedem Paket separat
  geprüft;
- die Auswahl wird als reine Anzeigepräferenz gespeichert und beim Laden
  wiederhergestellt; ungültige oder alte Werte fallen auf `graphite` zurück.

---

## 6. Materialgraph

### 6.1 Wahrnehmungsziel

Die Kurve soll sich nicht wie ein nervöses FFT-Polyline bewegen. Sie wirkt wie
eine dünne, gespannte Membran oder ein leichtes Papier:

- Impulse drücken lokal in die Fläche;
- benachbarte Punkte ziehen einander mit;
- Bewegung besitzt geringe Trägheit;
- nach einem Ereignis kehrt die Fläche weich zurück;
- die Live-Faser darf schneller sein als die Hauptfläche, bleibt aber leise.

### 6.2 Datenfluss

```text
Audio-Thread
  -> lockfreie Analyseframes
AnalyseEngine-Worker
  -> kohärenter MessSnapshot, ca. 20 Hz
Editor
  -> Zielkurve aus neuem Snapshot
  -> Feder/Dämpfung/Flächenspannung zwischen Snapshots
  -> Darstellung mit Ziel 60 Hz, ohne neue DSP-Berechnung
```

Die Engine kennt weder Farbpaket noch Materialbewegung. Die physische
Interpolation lebt ausschließlich auf der Snapshot-Kopie im Editor.

### 6.3 Bewegungsmodell

Für jeden sichtbaren Kurvenpunkt werden mindestens gehalten:

- aktuelle Y-Position;
- Geschwindigkeit;
- Ziel-Y aus dem jüngsten Snapshot;
- temporäre Beschleunigung.

Pro Darstellungsframe:

```text
feder    = (ziel - position) * steifigkeit
spannung = (links + rechts - 2 * position) * flächenspannung
dämpfung = -geschwindigkeit * dämpfung
```

Das Modell ist zeitbasiert und auf große `dt` geklemmt. Arrays werden einmal
angelegt und wiederverwendet. Es gibt keine Heap-Allokation pro Kurvenpunkt und
Frame.

### 6.4 Kurvenebenen

- Materialfläche und Tuschestrich: das geglättete LTAS-Komposit
- Live-Faser: die 3-s-EMA, dünner, transparenter und etwas schneller
- lokale Diagnosebasis: kurze Bleistiftstücke nur über `f_von_hz..f_bis_hz`
  samt Schulterbereich; niemals eine globale Sollkurve
- Abweichungsfahne: Fläche zwischen Messkurve und lokaler Basis
- Zeitgedächtnis: zurückgenommener Tusche-Pool unter der Kurve
- Schnappschuss: gestrichelt, farblich getrennt, statisch

Textur ist Materialhinweis, kein flächiges Rauschen über der ganzen UI.

### 6.5 Messzustände

- **laufend:** Zielkurve und Problemaktivität werden aktualisiert.
- **Stop:** sichtbare Kurve, Ringe und Zeit frieren ein.
- **neue Messung:** Zeit und Live-Zustand beginnen bei null; ein vorhandener
  Schnappschuss darf als Vergleich bestehen bleiben.
- **Schnappschuss:** kopiert genau den sichtbaren kohärenten Messstand und
  speichert ihn über den vorhandenen Snapshot-Pfad.

Start und Stop sind Gegenpfade. Save und Load des Schnappschusses werden im
selben Änderungssatz geprüft.

### 6.6 Diagnostische Materialgrammatik

Die ungewöhnliche Optik ist nur zulässig, wenn jede Schicht genau eine
Messbedeutung besitzt:

| Bildmittel | Datenbedeutung |
|---|---|
| Tuschestrich | `ltasKompositDb` |
| dünne Live-Faser | `ltasLiveDb` |
| Breite eines markierten Bereichs | `f_von_hz..f_bis_hz` |
| kurzes Bleistiftstück | Eigenkurven-/Schulterbasis des konkreten Befunds |
| Abstand Strich ↔ Basis | vorzeichenrichtige `staerke_db` |
| Nebel | junge, lückenhafte oder unsichere Evidenz |
| gerichtetes Licht | belastbare, handlungsreife Evidenz |
| Ringfüllung | normalisierte Dringlichkeit nach §7.2 |
| Tusche-Pool | zeitliches Energiegedächtnis |
| Kasure | fehlende spektrale Abdeckung |

Nebel und Befundlicht sind **kein additiver Doppelschmuck**, sondern zwei Enden
derselben Evidenzachse. Pro Befund gilt `nebel_anteil + licht_anteil = 1` vor
den rein visuellen Intensitätsreglern. Niedrige Evidenz erscheint diffus;
steigende Evidenz löst denselben Bereich in geometrisch gerichtetes Licht auf.

Es gibt keinen globalen Zielkorridor. `ltasReferenzDb` ist die reine
8192er-Messachse und ausdrücklich keine Sollkurve. Zonenbefunde beziehen sich
auf ihre eigenen Schulterlinien; Resonanzen auf ihre lokale Hüllkurve. Eine
gespeicherte Vergleichskurve bleibt davon getrennt und wird nur als expliziter
Schnappschuss gezeichnet.

Alle Geometrieschwellen werden zuerst in dB, Frequenz, Abdeckung und Evidenz
entschieden und erst danach in Pixel umgerechnet. Resize darf daher nie die
Befundklasse oder Dichtestufe ändern. Blattlicht, Befundlicht, Strichschein,
Fehlstellenlicht, momentane Abweichung und Zeitgedächtnis besitzen getrennte
Darstellungsregler; kein Regler darf mehrere Bedeutungen koppeln.

---

## 7. Befunde im Feld

### 7.1 Symboltypen

Alle Befundzeichen gehören zu einem eigenen Symbolalphabet: gefüllte,
leicht unregelmäßige Pinselkeile innen, ein geometrisch präziser Kreis außen.
Sie sind keine Font-Glyphen und keine generischen Icon-Library-Symbole. Die
Pfade liegen gemeinsam mit der Bildmarke in
`eq-copilot/design/assets/nakama-symbols.svg`. Der HTML-Entwurf verwendet sie
bereits; die spätere JUCE-Ansicht muss dieselben Silhouetten übernehmen.

| Typ | Symbol | Feldverhalten | Zeitverhalten |
|---|---|---|---|
| schmale Resonanz/Ringing | schmale Glocke | schmale Kontur am Peak | Ring bleibt bei Persistenz stabil |
| breiter Aufbau oder Loch | Bogen über Basisstrich | weicher, breiter Schleier | langsam steigend/fallend |
| Transientenschärfe | Blitz | kurzer lokaler Impuls | schnelles Füllen, weiches Abklingen |
| Maskierung/Kollision | zwei gegeneinander laufende Wellen | fein gemustertes Frequenzband | leicht atmend, solange beide Quellen kollidieren |

Aufbau und Loch teilen die Symbolfamilie, unterscheiden sich aber in Richtung
der Feldverformung und in den Archivdaten.

### 7.2 Dringlichkeitsring

Der sichtbare Ringwert ist ein normalisierter UI-Score aus mindestens:

- aktueller Stärke;
- Persistenz oder Ereignisdauer;
- diagnostischer Sicherheit;
- Problemklasse.

`0` bedeutet nahezu leer, `1` nahezu voller Ring. Der Ring darf bei einem
Transienten schnell wachsen und danach gedämpft leerlaufen. Die archivierte
Spitzenstärke bleibt davon unabhängig erhalten.

### 7.3 Ehrliche Überlagerung

- alle aktiven Befunde werden gezeichnet;
- gleiche oder benachbarte Frequenzen dürfen sich überlagern;
- es gibt kein Top-Drei, kein automatisches Wegschieben in eine Rail und kein
  `+4 weitere`;
- Marker dürfen minimal vertikal versetzt werden, aber nicht die Frequenz
  verfälschen;
- bei hoher Dichte wird das Bild bewusst wilder — als ehrliches Signal, dass
  der Mix viele Probleme trägt.

### 7.4 Kurzzeitige Probleme

Ein Ereignis besitzt mindestens die Zustände:

```text
beobachtet -> aktiv -> abklingend -> beendet/archiviert
```

Die Hauptansicht zeigt nur den aktuellen Zustand. Das Archiv behält Start,
Ende, Spitzenwert und Kontext. Ein verschwundener Marker löscht keinen Bericht.

### 7.5 Markerinteraktion

Ein Klick oder Fokus auf einen Marker darf ihn optisch hervorheben. Er öffnet
nicht das Archiv und springt nicht automatisch zu einer Detailkarte. Der User
behält die Kontrolle über die Informationsdichte.

---

## 8. Befundarchiv

### 8.1 Verhalten

- festes Dokument-/Archivsymbol oben rechts;
- gedimmtes Glimmen, sobald Berichte existieren;
- manueller Toggle, kein automatisches Aufspringen;
- Sortierung zunächst nach Dringlichkeit, innerhalb gleicher Dringlichkeit nach
  jüngster Aktivität;
- genau eine Karte darf standardmäßig aufgeklappt sein;
- weitere Karten öffnen unabhängig per Klick;
- Sessionnummer und Messzeit stehen leise im Fuß.

### 8.2 Karteninhalt

Geschlossene Karte:

- Symbolring;
- Problemtyp;
- kurze Klassifikation;
- Frequenz oder Bereich.

Geöffnete Karte:

- vier kompakte Messwerte;
- eine knappe fachliche Handlungsrichtung;
- kein Fließtext;
- kein `Anwenden` oder Auto-EQ.

### 8.3 Persistenzvertrag

Ein archivierter Eintrag benötigt mindestens:

```text
id
session_id
type
frequency_hz
range_hz
started_at
ended_at|null
peak_severity
persistence
confidence
evidence
compact_action
status
```

Vor produktiver Speicherung wird das zuständige JSON-Schema versioniert. Alte
Snapshots müssen ohne Berichtsfelder weiter laden. Unbekannte neue Felder
dürfen alte Consumer nicht zerstören. Save und Load sowie Start und Abschluss
eines Ereignisses werden gemeinsam getestet.

---

## 9. Responsive Vertrag

Die UI besitzt kein festes Seitenverhältnis.

### 9.1 Referenzgrößen

- kompakt: `720 × 450`
- mittel: `1200 × 675`
- groß: `1920 × 1080`

Alle drei Größen müssen ohne horizontales Scrollen, abgeschnittene Achsen oder
überlappende Marke/Werkzeuge funktionieren.

### 9.2 Plotabstände

- links: 42–78 px, abhängig von Breite;
- rechts: 30–62 px;
- oben: 78–136 px;
- unten: 54–88 px.

Die Abstände sind geklemmt, nicht aus einem global skalierten 1920er Canvas
abgeleitet. Schrift und Hitflächen skalieren nicht unter ihre Mindestgröße.

### 9.3 Breakpoints

- bis 900 px: Archiv als unteres Sheet; Schnappschuss verschwindet aus der
  Dauerleiste, bleibt über Tastatur/Host-Menü erreichbar;
- bis 620 px: Farbauswahl darf aus der Dauerleiste in ein Menü wandern;
- unter 600 px ist nur ein robuster Notbetrieb geschuldet, nicht die volle
  Qualitätsabnahme.

Die produktive JUCE-Ansicht entfernt die bisherige feste 750:520-Ratio und
setzt sinnvolle freie Resize-Grenzen.

---

## 10. Interaktion und Accessibility

### 10.1 Tastatur

- `Space`: Messung starten/stoppen, sofern kein anderes Control editiert wird
- `R`: Befundarchiv öffnen/schließen
- `H`: Tooltips ein/aus
- `Esc`: offenes Popout schließen, danach Markerauswahl lösen
- `Tab`: Werkzeuge, sichtbare Marker, Popout-Controls
- `Enter`/`Space` auf Marker: optische Auswahl

### 10.2 Fokus

Der Fokusring ist in jedem Farbpaket klar erkennbar und besitzt mindestens zwei
Tonwertkanten: Feldabstand plus Akzent. Fokus darf nicht allein als Farbwechsel
erscheinen.

### 10.3 Zugängliche Namen

Symbole erhalten ausgeschriebene Namen, zum Beispiel:

- `Farbpaket wählen`
- `Befundarchiv öffnen`
- `Schnappschuss speichern`
- `Messung beenden`
- `Resonanz, 118 Hertz`

Die visuellen Symbole bleiben trotzdem textfrei.

### 10.4 Reduzierte Bewegung

Bei aktivierter reduzierter Bewegung:

- weniger Überschwingen;
- stärkere Dämpfung;
- niedrigere Zielaktualisierung;
- keine pulsierenden Glows;
- Diagnose und Ringfüllung bleiben vollständig verständlich.

---

## 11. Produktionsarchitektur

### 11.1 Bestehende Wahrheit

Vor Änderungen sind mindestens zu lesen:

- `eq-copilot/plugin/src/PluginEditor.h`
- `eq-copilot/plugin/src/PluginEditor.cpp`
- `eq-copilot/plugin/src/PluginProcessor.h`
- `eq-copilot/plugin/src/PluginProcessor.cpp`
- `eq-copilot/plugin/src/AnalyseEngine.h`
- `eq-copilot/plugin/src/Diagnose.h`
- `eq-copilot/plugin/src/EqCopilotAssetKit.h`
- `eq-copilot/design/assets/nakama-symbols.svg`
- `eq-copilot/design/tokens.json`
- `eq-copilot/schemas/*.schema.json`

Die aktuelle Engine, Snapshot-Kohärenz, Diagnose und Pipe bleiben erhalten.
Die sichtbare Material-Gerätefront und ihre alten Dauercontrols werden ersetzt.

### 11.2 Editorverantwortung

Der Editor hält ausschließlich Anzeigezustand:

- gewähltes Farbpaket;
- Tooltipmodus;
- Popoutstatus;
- Markerauswahl;
- Materialkurvenposition und -geschwindigkeit;
- sichtbare Vergleichskurve;
- Darstellung der strukturierten Befunde.

Keiner dieser Zustände verändert Analyseparameter oder Audio.

### 11.3 Komponenten

Empfohlene Trennung:

- `SpectralFieldComponent` — Plot, Raster, Materialkurve
- `IssueLayerComponent` — Felder, Marker, Hit-Testing
- `ToolClusterComponent` — fünf Werkzeugkreise
- `ThemePopoverComponent` — Farbpakete
- `ReportPanelComponent` — Archiv und Karten
- `NakamaBrandComponent` — skalierbare 仲間-Bildmarke
- `NakamaLookAndFeel` — Schrift, Fokus, Kreise, Popouts
- kleine wertartige Modelle für Farbpaket, Marker und Bericht

Komponenten dürfen im selben Quellpaar beginnen, müssen aber getrennte
Verantwortung und eigene Bounds besitzen.

### 11.4 Migration der alten Controls

Aus der Daueransicht verschwinden:

- Glättungs-ComboBox;
- ausgeschriebene Ansichtsbuttons;
- `Festhalten`, `Vergleich aus`, `Neu messen`;
- ausgeschriebener Hinweisknopf;
- Messpunkt- und Konflikttext im Hauptfeld;
- Kennzahlenkarten und seitliche Meter.

Funktional notwendige Pfade werden nicht gelöscht:

- Schnappschuss bleibt über das Kamerasymbol;
- Reset wird Teil von `neue Messung`;
- Messpunkt/Konflikt bleiben in einem sekundären Host-/Kontextmenü oder einem
  gesonderten Statuspfad, bis ihre neue symbolische Form entschieden ist;
- Vergleichsdaten bleiben ladbar und als gestrichelte Linie darstellbar.

### 11.5 Gegenpfade

In jedem Änderungssatz gemeinsam prüfen:

- Messung starten ↔ stoppen/neue Messung
- Archiv öffnen ↔ schließen
- Farbpaket speichern ↔ laden/ungültigen Wert zurücksetzen
- Schnappschuss speichern ↔ laden/entfernen
- Befund aktivieren ↔ abklingen/archivieren
- Plugin installieren ↔ Rollback/Entfernen

---

## 12. Performance und Realtime

Unverändert gelten:

- Passthrough sampleidentisch;
- 0 Samples gemeldete Latenz;
- kein Tail;
- im Audiothread keine Sperre, Allokation, Datei, Pipe, Netzwerk oder Logausgabe;
- Überlast verwirft Analyseframes, nie Audio.

UI-Budgets:

- Ziel: 60 Darstellungsframes/s bei sichtbarer laufender Messung;
- Mindestziel: keine sichtbaren Sprünge zwischen den ca. 20-Hz-Snapshots;
- `paint()` p95 unter 8 ms bei `1920 × 1080` auf dem Zielsystem;
- keine Array-Allokation pro Frame;
- Gradienten, Pfade, Schrift und Symbolgeometrie cachen, wenn ihre Bounds gleich
  bleiben;
- im Leerlauf oder bei verborgenem Editor Repaint drosseln;
- ein Snapshot-Zug pro UI-Tick, keine inkohärenten Einzelreads.

`EqCopPaintBench` wird erweitert, damit Materialbewegung, überlagerte Befunde
und geöffnetes Archiv abgedeckt sind.

---

## 13. Implementierungsreihenfolge

1. Freies Resize und neue Bounds ohne feste Ratio.
2. Manrope als eingebettete Ressource und neues Look-and-Feel.
3. Farbpaketmodell mit sicherem Default und Save/Load.
4. Spectral Field aus realem `MessSnapshot`.
5. Materialinterpolation zwischen Snapshotrevisionen.
6. Symbolkreis und Dringlichkeitsring für alle Problemtypen.
7. ehrliche Überlagerung aller aktiven Befunde.
8. Werkzeugkreise und Tastaturpfade.
9. versioniertes Archivmodell mit temporären Ereignissen.
10. rechte Seitenfläche und kompaktes unteres Sheet.
11. Schnappschuss-, Reset-, Messpunkt- und Konflikt-Gegenpfade migrieren.
12. Visual-, Performance-, Realtime- und Hostprüfung.

Kein Schritt darf erfundene Diagnosewerte anzeigen. Fehlt ein Produktionsfeld,
bleibt die entsprechende Darstellung verborgen, bis Schema und Producer real
existieren.

---

## 14. Prüfung

### 14.1 Vorentwurf

- JavaScript-Syntax mit `node --check` prüfen.
- `node eq-copilot/design/render-blatt.mjs pruefe` muss alle sechs
  Semantik-/Gegenpfadchecks mit `OK` melden.
- `node eq-copilot/design/render-blatt.mjs` rendert den deterministischen
  `n4-*`-Satz.
- Browserprüfung bei `720 × 450`, `1200 × 675`, `1920 × 1080`.
- beide dunklen Farbpakete schalten;
- Tooltips ein/aus;
- Archiv öffnen/schließen und Karten ausklappen;
- Start, Stop, neue Messung und Schnappschuss prüfen;
- früher/unsicherer Befund: Nebelanteil größer als Lichtanteil;
- belastbarer Befund: Lichtanteil größer als Nebelanteil;
- Leerzustand: keine Diagnosebasis, kein Nebel, kein Befundlicht;
- gleiche Messdaten in allen drei Referenzgrößen: gleiche dB-Dichtestufe;
- breite positive und negative Abweichung sowie kurzzeitigen Befund prüfen;
- mehrere überlagerte Befunde sichtbar prüfen.

### 14.2 C++-Build

```powershell
$cmake = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -S eq-copilot -B eq-copilot/build -G "Visual Studio 17 2022" -A x64
& $cmake --build eq-copilot/build --config Release --target EqCopilot_VST3 EqCopShot EqCopPaintBench EqCopNullTest EqCopGoldenTest EqCopPipeProbe
```

### 14.3 Pflichtbeweise

```powershell
eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures
cargo test --manifest-path plugin-hub-app/src-tauri/Cargo.toml eq_copilot
```

Zusätzlich:

- `EqCopShot` in allen drei Referenzgrößen;
- `EqCopPaintBench` mit 0, 1 und vielen überlagerten Befunden;
- `EqCopPipeProbe` gegen den separaten Probe-Pipenamen;
- pluginval;
- reale Resize-, DPI-, Start/Stop-, Save/Load- und Reopen-Prüfung in FL Studio.

Systemweite Repositoryprüfungen bleiben:

```powershell
cargo test --manifest-path plugin-hub-app/src-tauri/Cargo.toml
npm --prefix plugin-hub-app run build
npm --prefix plugin-hub-app run check
```

---

## 15. Definition of Done

Die produktive Ansicht ist erst fertig, wenn:

- die eigene 仲間-Bildmarke pfadbasiert und ohne Bitmap-/Systemfont-Abhängigkeit
  gezeichnet wird;
- Nakama außer Achsen keine permanente Textlast trägt;
- die Kurve bei Bewegung wie ein zusammenhängendes Material und nicht wie ein
  springender Analyzer wirkt;
- alle Problemtypen durch das eigene Field-Symbolalphabet ohne Text
  unterscheidbar sind;
- die Ringfüllung Dringlichkeit sofort vermittelt;
- kurzzeitige Probleme sichtbar abklingen und dennoch archiviert bleiben;
- beliebig viele gleichzeitig aktive Befunde nicht künstlich unterdrückt
  werden;
- das Archiv nur manuell öffnet und bei Inhalt gedimmt glimmt;
- alle sechs Farbpakete vollständig funktionieren;
- Nakama Phase reproduzierbar gebaut und zusammen mit Manrope lokal eingebettet
  ist;
- die Achsen in allen Referenzgrößen lesbar, aber visuell nachgeordnet sind;
- Start/Stop, Save/Load, Open/Close und Install/Rollback geprüft sind;
- Nulltest, GoldenTest, PipeProbe, PaintBench, pluginval und FL-Hostprüfung
  bestanden sind;
- Dokumentation und sichtbare Produktbehauptungen dem gebauten Stand
  entsprechen.
