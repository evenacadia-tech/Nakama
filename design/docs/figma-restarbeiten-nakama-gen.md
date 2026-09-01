# Nakama Gen – offene Figma-Arbeiten und Übergabekriterien

- **Stand:** 24.08.2026
- **Status dieses Dokuments:** Historische Arbeits- und Übergabeliste, seit dem
  Funktionsneustart vom 31.08.2026 keine aktuelle Layoutautorität
- **Geltungsbereich:** Nakama Gen Seite 1 „Overview“, Seite 2 „EQ Center“, die zugehörigen Zustände und Motion-Demos sowie die minimale Rückfallfläche von Nakama Probeeq
- **Nicht im Geltungsbereich:** DSP-Implementierung, Broker/IPC, JUCE-Bau und helle Produktfassung

Der aktuelle funktionale Größenvertrag steht in
[`../abnahmen/2026-09-01-gen-fenstergroesse-und-resize.md`](../abnahmen/2026-09-01-gen-fenstergroesse-und-resize.md):
760×430 ist das kompakte Minimum, 950×538 die Standardgröße im festen
76:43-Rahmen. Die Maße und Figma-Nodes unten bleiben Belege des Stands vom
24.08. und dürfen diese neuere Entscheidung nicht überschreiben.

## 1. Kurzurteil

Die visuelle Grundrichtung ist weit genug, um nicht noch einmal bei null zu beginnen. Die einzige aktuelle Figma-Quelle ist immer **Nakama-Design** (`NPCQYSkoZEd4Av0NlKxBOd`). Der Live-Stand dort wurde am 24.08.2026 direkt per Figma-API gelesen:

- Overview: Wrapper `25:443`, innerer Frame `25:444`, **760×430**
- EQ Center: Wrapper `28:993`, innerer Frame `28:994`, **760×430**
- Seite-1-Zustände: `25:805`
- Seite-2-Zustände: `25:901`

Die Größenabweichung ist damit geschlossen. Der User hat den fehlerhaften
700×420-Stand im verbindlichen Figma durch den neuen 760×430-Stand ersetzt.
Die früheren EQ-Nodes `25:583`/`25:584` und alle Fassungen aus anderen
Figma-Dateien sind Verlauf.

Die vier aktuellen Depotbereiche sind weiterhin **Vorschläge und nicht als
Gesamtdesign vom User abgenommen**. Der erneute Live-Check nach der Korrektur
fand in ihren insgesamt 554 Nodes **0 Prototyp-Reaktionen** und **0 Component-,
Component-Set- oder Instance-Nodes**. Das bedeutet: Die statische
Designsprache ist vorhanden, die eigentliche Komponenten-, Interaktions- und
Motion-Übergabe fehlt noch.

Der Web-Simulator ist beim Zustandsmodell inzwischen weiter als Figma. Er besitzt unter anderem:

- die Drei-Stufen-Geste aus Halten, zehnsekündiger Probe und Bestätigung;
- Draft-, Lease-, Stale-, Aufnahme-, Transport- und Verbindungszustände;
- direkte Band- und Parameterbearbeitung einschließlich vertikalem Ziehen, `Shift`-Feinmodus und numerischer Eingabe;
- harte Schutzbereiche;
- eine zweite Vergleichskurve im Zustandsmodell;
- Global-Panel, Bypass, Undo, externe Automation und sechs reproduzierbare Prüfszenarien.

Figma muss diese vorhandene Logik nicht neu erfinden. Es muss sie als eindeutige Komponentenvarianten, Zustandsframes und Prototypsequenzen sichtbar und abnehmbar machen.

## 2. Quellenhierarchie und verbindliches Depot

Für Entscheidungen gilt diese Reihenfolge:

1. wörtliche User-Entscheide in [`design/abnahmen/`](../abnahmen/);
2. der technische Lieferplan in [`docs/plan/plan.json`](../../docs/plan/plan.json);
3. die verbindlichen [UI-Spielregeln](ui-spielregeln-eq-zentrale.md);
4. die [Interaktions- und Motion-Spezifikation](interaktions-und-motion-spezifikation.md);
5. der aktuelle Live-Stand in `Nakama-Design` (`NPCQYSkoZEd4Av0NlKxBOd`);
6. der Web-Simulator als ausführbare Zustands- und Motion-Referenz.

Der User hat am 24.08. ausdrücklich bekräftigt: „die sind IMMER in diesem figma, nirgends wo anders. hier ist immer das aktuellste“. Damit ist die frühere Lesart „in einer Copy arbeiten und später übertragen“ aufgehoben. Andere Figma-Dateien und ihre Node-IDs sind Verlauf. Repo-Exporte bleiben Belege eines früheren Snapshots, aber keine Quelle für den aktuellen Designstand.

## 3. Statusübersicht

| Bereich | Stand | Was noch fehlt |
|---|---|---|
| Gemeinsame dunkle 760×430-Schale | vorhanden, nicht abgenommen | Overview und aktuelles EQ Center sind beide 760×430; gemeinsame Schale ist sichtbar |
| Overview-Hauptframe | teilweise | Node `25:444`; statischer Normalzustand vorhanden, vollständige Ergebnis-, Warte-, Compare- und Fehlerzustände fehlen |
| EQ-Center-Hauptframe | teilweise | Node `28:994`; Größen- und Schalenkorrektur ist erledigt, Bedien-, Varianten- und Gegenpfade fehlen |
| Seite-1-Zustandstafel | teilweise | Frische/GAP/EPOCH und Teile der Geste vorhanden; Compare, Draft-Sent, Unsicherheit und beschlossener Revisionsverlauf fehlen |
| Seite-2-Zustandstafel | teilweise | mehrere Zustände vorhanden; Vergleichsauswahl, Band-Mikrozustände, Bypass, Schutzverletzung und Master fehlen |
| Komponentenbibliothek | fehlt | Live-Check: 0 Component-, Component-Set- oder Instance-Nodes in den vier aktuellen Gen-Bereichen |
| Produktinteraktion in Figma | fehlt | Live-Check: 0 Reaktionen in den vier aktuellen Gen-Bereichen |
| Sichtbare zweite Vergleichsquelle | fehlt | Zustandsmodell und Web-Inspector können sie wählen, aber die Pluginfläche hat keinen abgenommenen Controller |
| Parameter-Präzisionsbedienung | Figma fehlt | Web-Simulator kann vertikal ziehen und numerisch editieren; Figma zeigt das noch nicht |
| Minimal-Rückfallfläche Probeeq | Basis vorhanden | Suna-Entwurf `6:2864` ist die visuelle Basis; Umbau auf Probeeq-Pflichtinhalt und Zustände fehlt |
| Mix | geplant | Platz ist markiert; echte Bedienzustände erst nach versioniertem Parametervertrag finalisieren |
| Sidechain-Quelle | entschieden | im ersten Release unsichtbar; kein Feld und kein DYN-Untermenü gestalten |
| Skalierungs- und Assetpaket | teilweise | RC0-Exporte existieren; nach jeder abgenommenen Figma-Änderung neu erzeugen und hashen |
| Helle Fassung | später | ausdrücklich nicht Teil der aktuellen Abnahme |

## 4. Figma-Datei sauber strukturieren

Das verbindliche `Nakama-Design` braucht eine klare Trennung zwischen aktueller Übergabe, Varianten und Archiv. Empfohlene Seitenstruktur:

1. `00 STATUS / README`
2. `01 FOUNDATIONS`
3. `02 COMPONENTS`
4. `10 GEN · OVERVIEW`
5. `11 GEN · EQ CENTER`
6. `12 PROBEEQ · FALLBACK`
7. `20 STATES · OVERVIEW`
8. `21 STATES · EQ CENTER`
9. `30 MOTION / INTERACTIONS`
10. `40 HANDOFF / EXPORTS`
11. `90 ARCHIVE`

Auf der Statusseite müssen stehen:

- Designstatus: `WORKING`, `REVIEW`, `APPROVED` oder `ARCHIVE`;
- Figma-Datei und Node-ID der beiden Hauptframes;
- 760×430 als logische Größe beider Gen-Seiten;
- Theme `DARK`;
- Datum und Versionsbezeichnung;
- Link auf dieses Dokument und die UI-Spielregeln;
- Liste der bewusst offenen Punkte;
- Hinweis, dass der Web-Simulator Design-/Motion-Referenz und keine eingebettete Webseite des Plugins ist.

Der neue 760×430-Frame `28:994` erfüllt die technische Größenentscheidung.
Der ersetzte 700×420-Frame `25:584` ist Verlauf und darf nicht mehr als
aktueller Handoff referenziert werden. Frühere Versuche aus anderen
Figma-Dateien werden nicht als Alternativen importiert. Die beiden aktuellen
Hauptframes müssen genau einmal als „zur Abnahme“ gekennzeichnet sein.

## 5. Foundations und gemeinsame Komponenten

### 5.1 Verbindliche Foundations

In Figma als Styles oder Variables festhalten:

- Farben für Grundfläche, Glas, Slate-Akzent, Signalrot, Text-Hierarchien, deaktiviert, Draft, Vergleichskurve, Schutzbereich und Fehler;
- Geist und Geist Mono mit den tatsächlich eingesetzten Größen, Schnitten, Zeilenhöhen und Spationierungen;
- Raster, Abstände, Radien, Konturen, Schatten und Materialebenen der 760×430-Schale;
- Kurven- und Analyzer-Strichstärken;
- Fokus-, Hover-, Pressed-, Selected-, Disabled- und Error-Stile;
- Motion-Tokens aus [`motion-tokens.json`](../prototyp/contract/motion-tokens.json).

Die Materialschale, Wortmarke, Mikrograin, Reflexionen und statische Lichtdetails sind Exportmaterial. Text, Zustände, Heatmap, Spektrum, EQ-Kurven, Bänder, Draft, Auswahl und Schutzbereiche bleiben getrennte Live-Layer.

### 5.2 Gemeinsame Komponenten, die vollständig variantisiert werden müssen

- `AppShell / Dark / 760×430`
- `Header / PageTabs` mit `OVERVIEW ACTIVE` und `EQ CENTER ACTIVE`
- `Header / ExceptionStatus` für still, measuring, stale, incomplete, broker offline, probe offline und capability unavailable
- `Button` mit default, hover, pressed, keyboard focus, disabled, blocked und destructive/reject
- `HoldButton` mit idle, held, release-to-stop und blocked
- `SourceSwitcher` für 1–16 Busse, aktives Ziel, Master und Rad-/Mausradwechsel
- `DraftCard` für none, ready, auditioning, temporary lease, stale, blocked und command pending
- `ParameterValue` für idle, drag, fine drag, numeric edit, invalid und disabled
- `EQBandHandle` für normal, selected, disabled, protected, draft target, dynamic active und external-automation violation
- `ProtectedRange` für normal, collision, suggested, applying, violated-by-host und unlocked/editable
- `GlobalDisclosure` für collapsed und expanded
- `Bypass` für active, bypassed und unavailable
- `Undo` für available, unavailable und just-undone
- `LinkStatus` für healthy/silent, broker offline, probe offline und command pending
- `MeasurementStatus` für measuring, fresh, incomplete, stale, GAP und EPOCH
- `FocusRing` und unsichtbare Trefferflächen als dokumentierte Komponenteneigenschaft

Jede sichtbare Variante braucht einen klaren Auslöser. Ein Zustand darf nicht allein deshalb existieren, weil er dekorativ gut aussieht.

## 6. Seite 1 – Overview

### 6.1 Hauptframe abschließen

Der vorhandene Frame ist eine brauchbare Normalansicht. Vor der Abnahme ist zu prüfen und festzuschreiben:

- Page-Tabs sitzen auf Overview und EQ Center an exakt derselben Position wie auf Seite 2.
- Die aktive Seite ist eindeutig, ohne einen zusätzlichen toten Seitentitel.
- `OBSERVE`, `ADVISE`, `AUDITION`, `APPLY` sind echte Lebenszykluszustände und keine reine Dekoration.
- Quellenzeile, Heatmap, Advisor und Aktionszeile besitzen eine klare gemeinsame Auswahl.
- `WIDTH UNAVAILABLE` ist fokussier- beziehungsweise anklickbar und erklärt den Grund; sonst ist es totes UI.
- `OPEN IN EQ` öffnet die betroffene Sonde direkt auf Seite 2.
- Dauer-Normalzustände wie `LIVE` werden nach dem Ausnahmeprinzip überprüft: Nur Abweichungen sollen sprechen, sofern der Text keinen eigenen Bediennutzen hat.

### 6.2 Fehlende Overview-Zustände

Als Komponentenvarianten und mindestens einmal als vollständiger Kontextframe anlegen:

1. **Measuring** mit realem Fortschritt; der Punkt pulsiert nur während tatsächlicher Messung.
2. **Fresh** als ruhiger Normalzustand.
3. **Incomplete** mit sichtbarem Grund.
4. **Stale** mit Alter beziehungsweise Ursache.
5. **GAP** als Segmentlücke.
6. **EPOCH** als Host-/Transport-Sprung – visuell eindeutig anders als GAP.
7. **Broker offline** und **Probe offline** als zwei unterschiedliche Fehler.
8. **Recording / unknown record state**: Audition sichtbar blockiert.
9. **Transport stopped**: Audition sichtbar blockiert.
10. **More data needed** als gültiges Advisor-Ergebnis.
11. **No change recommended** als gültiges Advisor-Ergebnis.
12. **Draft sent / waiting for Probeeq** nach dem Overview-Apply; keine vorgetäuschte Klangänderung.
13. **Command acknowledged / awaiting state report** als unterscheidbarer Zwischenzustand.
14. **Draft stale because host automation changed target**.
15. **Compare measuring** nach einer bestätigten Änderung.
16. **Compare result** mit `KEEP`, `REVERT` und `UNCLEAR`.
17. **Kleiner Revisionsverlauf** – Verlaufssymbol sowie leerer, verfügbarer
    und geöffneter Verlauf bestätigter Änderungen; kein bloßes `UNDO · n`.

### 6.3 Overview-Sequenz für den Figma-Prototyp

Die klickbare Sequenz muss vollständig durchlaufen:

`SOURCE SELECT` → `FINDING` → `HOLD TO AUDITION` → Loslassen → `APPLY / SEND DRAFT` → `WAITING FOR PROBEEQ` → `OPEN IN EQ` → bestätigte Änderung → zurück zu Overview → erneute Messung → `KEEP / REVERT / UNCLEAR`.

Zusätzlich braucht dieselbe Sequenz zwei Gegenpfade:

- Audition wird wegen Stop/Recording/Unknown abgelehnt und nennt den Grund.
- Verbindung fällt während Audition oder Warten aus; der Zustand kehrt ehrlich zum bestätigten Stand zurück.

## 7. Seite 2 – EQ Center

### 7.1 Hauptframe und Zielauswahl

Der vorhandene Frame zeigt eine Spur und acht Bänder. Noch zu gestalten:

- ausgewählter Bus;
- Master-EQ als eigener Zielzustand, nicht als 17. Radposition;
- Auswahl einer optionalen zweiten Spur;
- eindeutiges Bearbeitungsziel, wenn zwei Kurven sichtbar sind;
- Entfernen beziehungsweise Austauschen der zweiten Spur;
- genau eine Spektrumanzeige: immer die aktuell bearbeitete Spur;
- klare Farbzuteilung der beiden Kurven in normal, selected, draft und stale.

**Größte aktuelle Figma-Lücke:** Der Web-Vertrag kennt `comparisonSourceId` und kann eine zweite Kurve zeichnen. Der derzeitige Web-Simulator wählt sie aber nur über das externe Prüfpanel. Auf der eigentlichen Pluginfläche fehlt ein abgenommenes Bedienelement. Figma muss deshalb einen kompakten, nicht toten Controller entwerfen, der:

1. die zweite Spur hinzufügt;
2. ihren Bus frei auswählt;
3. das Bearbeitungsziel zwischen erster und zweiter Kurve umschaltet;
4. die zweite Spur wieder entfernt;
5. jederzeit zeigt, welche Kurve Maus, Parameterzeile und Spektrum steuert.

Die fünf Funktionen sind bereits durch das Interview vom 23.08. entschieden.
Für die optische Form hat der User ausdrücklich Pro-Q und Smooth Operator Pro
als Handling-Referenzen freigegeben. Sie wird deshalb als konkreter
Figma-Vorschlag gebaut und anschließend visuell abgenommen; dafür wird keine
abstrakte Grundsatzfrage erneut gestellt.

### 7.2 Band- und Parameterbedienung

Figma muss die heute im Simulator vorhandene Präzisionsbedienung nachziehen:

- Bandgriff anklicken: Band und Parameterzeile werden gemeinsam ausgewählt.
- horizontal ziehen: Frequenz und Kurve folgen ohne Verzögerung.
- vertikal ziehen: Gain und Kurve folgen ohne Verzögerung.
- Wertfeld vertikal ziehen: Parameter ändert sich direkt.
- `Shift` beim Ziehen: Feinmodus mit sichtbarem, aber zurückhaltendem Hinweis.
- Doppelklick oder `Enter/F2`: numerische Eingabe.
- `Enter`: übernehmen; `Escape`: verwerfen; Fokusverlust: eindeutiges Ergebnis.
- überlappende Griffe: ausgewählter Griff liegt vorn; `BAND` bietet den sicheren Auswahlweg 1–8.
- ausgeschaltetes Band auswählen: auswählen, aber nicht still einschalten.
- deaktivierte Dynamikparameter: sichtbar deaktiviert, nicht wie gültige Werte behandeln.

Benötigte Bandvarianten:

- normal;
- hover/focus;
- selected;
- disabled;
- draft target;
- auditioning;
- protected collision;
- dynamic reduction active;
- switching;
- moved by external automation;
- host automation violating protection.

Filtertyp, Kanalmodus und Dynamic sind strukturelle Wechsel. Dafür fehlt ein kurzer **SWITCHING**-Zustand. Er darf die technische Audiozeit nicht behaupten; er zeigt lediglich, dass der sichere Blockwechsel gerade läuft.

### 7.3 Schutzbereiche

Der heutige Hauptframe zeigt einen statischen `PROTECTED`-Bereich. Für eine vollständige UI fehlen:

- Zone auswählen;
- Zone aus einem Gen-Befund als Vorschlag zeigen;
- vorgeschlagene Zone durch User-Klick tatsächlich anlegen;
- Zone verschieben oder Grenzen bearbeiten;
- Zone lösen beziehungsweise entfernen;
- Bandzug an der Grenze stoppen;
- numerische Eingabe in die Zone abweisen beziehungsweise auf die Grenze begrenzen;
- Host-Automation innerhalb der Zone als sichtbare Verletzung markieren, ohne zu behaupten, sie sei verhindert worden.

Bereits entschieden sind: harte Sperre für alle Nakama-Bedienwege, Gen darf
eine Zone aus einem Befund vorschlagen, und angelegt wird sie erst durch einen
User-Klick. Die noch nicht gezeichneten Detailgesten für manuelles Verschieben,
Grenzenbearbeitung und Lösen werden als Figma-Vorschlag auf dieser Grundlage
gebaut und zusammen mit den Gegenpfaden abgenommen. Die Grundwirkung wird
nicht erneut erfragt.

### 7.4 Draft-, Audition- und Apply-Automat

Die jüngere Drei-Stufen-Geste ersetzt ältere direkte Apply-Lesarten:

1. **Halten:** Vorschau nur solange gehalten.
2. **Erster Klick:** zehn Sekunden temporär angewendet.
3. **Zweiter Klick:** bestätigt und als Revision gespeichert.

In Figma benötigt der Draft-Kasten diese Varianten:

- `DRAFT / READY`;
- `DRAFT / AUDITIONING` mit `RELEASE TO STOP`;
- `DRAFT / APPLIED TEMPORARILY / CONFIRM · 10 s`;
- Countdown-Zwischenstand, zum Beispiel `7 s`;
- letzte Sekunde;
- automatischer Ablauf und Rückkehr zur bestätigten Kurve;
- `REJECT` während der Lease;
- bestätigt, Draft-Karte verschwindet, Revision/Undo aktualisiert;
- `TRANSPORT STOPPED`;
- `RECORDING – AUDITION LOCKED`;
- `RECORD STATE UNKNOWN – AUDITION LOCKED`;
- `DRAFT STALE – HOST AUTOMATION CHANGED TARGET`;
- Broker-/Probe-Verlust während Preview beziehungsweise Lease;
- `COMMAND SENT / AWAITING ACK`;
- Reject/Fehler durch Zielsonde.

Die temporäre Kurve, Draft-Kurve und bestätigte Kurve müssen in jeder Variante unterscheidbar sein. Erst die Bestätigung darf wie ein gespeicherter Zustand aussehen.

### 7.5 Global, Bypass, Undo, Mix und Master

**Global**

- collapsed ist Standard;
- expanded ist ein verankertes Panel, kein dauerhafter zweiter Footer;
- Inhalt: Input Trim, Output Trim, Width und Mono Bass;
- **dazu der AUTO-Schalter neben OUTPUT** (Entscheid 24.08.,
  Spielregel 47, Wortlaut `../abnahmen/2026-08-24-auto-gain-schalter.md`)
  — zieht den Pegelgewinn der Kurve am Ausgang wieder ab, damit ein
  An/Aus-Vergleich die Klangfarbe zeigt und nicht „lauter". Standard
  **aus**; im Zustand **an** muss der Ausgangswert sichtbar mitwandern,
  ein stiller Ausgleich wäre ein unsichtbarer Eingriff. Damit sind es
  fünf Bedienelemente im Panel, nicht vier;
- Chevron sitzt im Button;
- Panel braucht Focus-, Changed- und Disabled-Zustände.

**Bypass**

- aktiv und bypassed müssen im Graph, Button und Zustandsmodell zusammenpassen;
- Bypass ist Hörvergleich, nicht derselbe Zustand wie EQ-Mode/Instanz-Lebenslauf;
- kein rein dekoratives Abdunkeln ohne lesbaren Zustand.

**Undo**

- auf Seite 2 kleines Symbol im Display;
- ein Klick nimmt den letzten bestätigten Schritt der gewählten Sonde zurück;
- disabled, available und just-undone gestalten;
- Draft, Hold-Preview und abgelaufene Probe erzeugen keinen Undo-Schritt.

**Mix**

- der Knob sitzt unten rechts;
- jede Sonde besitzt einen eigenen Dry/Wet-Wert;
- der Knob gilt zusätzlich für die ganze App;
- Klick auf den Knob schaltet zwischen App-global und der gewählten Sonde um;
- der heutige `PLANNED`-Platz darf bis zum Vertrag im Arbeitsframe bleiben;
- vor dem versionierten Mix-Parametervertrag darf er nicht wie ein funktionsfähiger Regler aussehen;
- nach S26–31 zeigen Varianten eindeutig `GLOBAL` oder die gewählte Sonde.

Diese Bedienlogik ist bereits entschieden. Offen ist nur ihre visuelle
Ausarbeitung und spätere Abnahme, nicht erneut das Grundprinzip.

**Master**

- eigener ausgewählter Hauptframe oder vollständige Frame-Variante;
- Master ist nicht Teil des Source-Wheels;
- Beschriftung, Kurvenfarbe, Draft-Ziel, Global-Werte, Schutzbereiche und Undo-Scope müssen beim Wechsel eindeutig bleiben.

### 7.6 Sidechain-Quelle

Im ersten Release nicht gestalten. Der User hat am 24.08. entschieden, dass
die Sidechain-Quelle zunächst unsichtbar bleibt. Der technische Parameter
bleibt mit Default `none` im Zustand, aber die Figma-Bandzeile erhält weder
ein dreizehntes Feld noch ein DYN-Untermenü. Eine sichtbare Bedienung folgt
erst mit einem später technisch freigegebenen Sidechain-Schritt.

## 8. Minimale Probeeq-Rückfallfläche

S31b verlangt zusätzlich zur Gen-EQ-Zentrale eine minimale native
Rückfallfläche der Sonde. Dafür existiert im verbindlichen Figma bereits die
visuelle Basis `Suna`: Workspace `6:2863`, eigentlicher 260×84-Entwurf
`6:2864`. Performance zuerst und der Pflichtinhalt sind bereits entschieden.
Der vorhandene Entwurf wird auf Probeeq umgebaut; ein fremder neuer Stil wird
nicht erfunden.

Festgelegter Inhalt:

- Linkzustand: connected beziehungsweise konkret broker/probe offline;
- EQ-Mode-Punkt grün/rot als Lebenslauf-/Modezustand;
- Bypass als echter bedienbarer Hostparameter;
- aktuell veränderte EQ-Werte als kompakte Anzeige;
- Mix je Sonde erst dann bedienbar, wenn der Parametervertrag existiert.

Der aktuelle Suna-Entwurf zeigt PASSIVE PROBE, Bus, Rolle, Position,
CONNECTED, MEASURING und die Wortmarke NAKAMA SUNA. Für Probeeq müssen Name
und Informationshierarchie angepasst und die beschlossenen Inhalte
Connection-Ausnahmen, EQ-Mode, geänderte EQ-Werte, Bypass und später Mix
untergebracht werden. Die Messposition PRE/INSERT/POST ist bisher eine
dokumentierte, risikoarme Lesart von „Mode der Probe“, aber kein wörtlicher
Userentscheid. Der Prüfstein lautet: Zeigt die Fläche nur das, wofür man
Probeeq tatsächlich öffnen würde? Alles andere gehört in Gen.

## 9. Motion und Prototyping in Figma

### 9.1 Tokens

| Ereignis | Vorgabe |
|---|---|
| direkter Band-/Parameterzug | 0 ms; Griff, Wert und Kurve im selben sichtbaren Zustand |
| unmittelbares Feedback | 100 ms |
| nicht direkt manipulierte Zustandsänderung | 140 ms |
| Seitenwechsel | 180 ms |
| Easing | `cubic-bezier(0.22, 1, 0.36, 1)` |
| echte Measuring-Pulse | 1600 ms |
| Spektraldaten | 20 Hz; Anzeige darf zwischen Samples interpolieren |
| Reduced Motion | keine Transform-Animation, Opacity höchstens 100 ms, Measuring als statischer Aktivindikator |

Keine Springs, kein Bounce und keine dekorative Endlosschleife. Audio-Rampen und DSP-Crossfades übernehmen niemals diese UI-Zeiten.

### 9.2 Pflichtsequenzen

Auf `30 MOTION / INTERACTIONS` müssen mindestens diese klickbaren Abläufe entstehen:

1. **Band bearbeiten:** Band wählen → horizontal/vertikal ziehen → Wertfeld fein ziehen → Doppelklick und Zahl eingeben → über `BAND` einen überlappenden Griff wählen.
2. **Draft anwenden:** Ready → Hold → Release → erster Klick/10-s-Lease → Confirm → Undo.
3. **Draft-Gegenpfade:** Stop/Recording/Unknown blockiert; Reject; Lease-Ablauf; Verbindungsverlust; externe Automation macht Draft stale.
4. **Overview-Entscheidung:** Quelle → Finding → Audition → Draft senden → warten → EQ öffnen → Compare → Keep/Revert/Unclear.
5. **Zwei Spuren:** zweite Spur hinzufügen → Bearbeitungsziel wechseln → nur Ziel-Spektrum anzeigen → zweite Spur entfernen.
6. **Schutzbereich:** Vorschlag → User bestätigt Zone → Band stößt an Grenze → Host-Automation verletzt Zone sichtbar.
7. **Global/Bypass/Undo:** Global öffnen/schließen → Wert ändern → Bypass → letzter bestätigter Schritt zurück.
8. **Navigation:** Overview ↔ EQ Center mit identischer Schale und ohne zweites Fenster.

Smart Animate dient nur der Vorführung. Die native Anwendung übernimmt später die echte Zustandsbewegung.

## 10. Reproduzierbare Abnahmeframes

Figma und Simulator sollten dieselben benannten Prüfszenarien besitzen:

| Fixture | Figma-Inhalt |
|---|---|
| `overview-golden` | abnahmefähiger Overview-Normalzustand |
| `eq-golden` | abnahmefähiger EQ-Center-Normalzustand |
| `sixteen-sources` | 16 Busse, Rad-/Mausradlogik, Master separat |
| `disconnected` | getrennte Broker- und Probe-Fehlerzustände, Regler ehrlich gesperrt |
| `recording-gate` | Audition wegen Recording/Unknown sichtbar blockiert |
| `stale-automation` | Host-Automation gewinnt, Draft wird stale, Schutzverletzung sichtbar |

Zusätzlich empfohlen:

- `comparison-two-sources`;
- `temporary-apply-countdown`;
- `protected-range-collision`;
- `master-selected`;
- `reduced-motion`.

Jeder Frame bekommt einen kurzen Kommentar: Ausgangszustand, User-Aktion, sichtbare Änderung, Endzustand und Gegenpfad.

## 11. Export- und Handoff-Paket

Nach der Designabnahme müssen die Exporte neu aus dem akzeptierten Figma-Stand erzeugt werden. Das vorhandene Paket `nakama-gen-dark-2026-08-24-rc0` wurde aus der inzwischen als Verlauf eingeordneten Copy-Datei erzeugt. Es bleibt ein ehrlicher historischer Snapshot, ist aber **keine aktuelle Figma-Baseline** und darf nicht als unverändert weiterverwendet werden.

Wichtig: Die heutigen `qualityTiers` im Asset-Manifest sind aus dem 4×-Golden abgeleitet. Das ist für die laufende Simulatorprüfung brauchbar, erfüllt aber noch nicht die abgenommene Vorgabe „eigener Figma-Exportsatz je Skalierungsstufe“. Auch die heutige Runtime verwendet von der Schale vor allem das Mikrograin als Bild und rekonstruiert weitere Materialanteile live. Für die finale Pixelübergabe muss deshalb geprüft werden, welche Schalen-/Glas-/Lichtanteile vollständig gebacken werden müssen, damit FL Studio den akzeptierten Figma-Stand tatsächlich trifft.

### 11.1 Gebackene Assets

Jeweils für **100 %, 125 %, 150 % und 200 %**:

- Schale/Glas/Kinn einschließlich Material, Mikrograin, Reflexionen und statischer Lichtdetails;
- Wortmarke mit ihrem Figma-Effekt;
- gegebenenfalls Leuchtsaum- und Fokus-Sprites, falls live gerendert sichtbar abweicht.

### 11.2 Live-Layer

Separat und nicht in die Schale backen:

- Produkttexte;
- Status und Fehler;
- Heatmap und Messfortschritt;
- Spektrum;
- bestätigte, temporäre, Draft- und Vergleichskurven;
- Bandgriffe und Auswahl;
- Schutzbereiche;
- Controls und Fokuszustände.

### 11.3 Übergabe-Metadaten

- Figma-Datei, Node-ID und Designstatus;
- Exportdatum und logische Größe;
- Farb-/Typografie-/Motion-Tokens;
- Assetklasse `baked`, `live`, `planned` oder `reference-only`;
- SHA-256 für alle versionierten Exporte;
- Fontdateien und Lizenz;
- Zuordnung von Komponente/Variante zu Zustandsnamen im UI-Vertrag;
- Golden-Export jedes Abnahmefixtures bei 1:1 sowie die vier Skalierungsstufen.

## 12. Bewusst nicht jetzt in Figma finalisieren

- **Helle Fassung:** geplant, aber aktuell ist nur dunkel verbindlich.
- **Sidechain-Source:** im ersten Release entschieden unsichtbar; erst nach
  technischer Freigabe sichtbar ausarbeiten.
- **Funktionsfähiger Mix:** wartet auf die versionierte Erweiterung aus S26–31; bis dahin klar `PLANNED`.
- **Native JUCE-Details:** Figma beschreibt Oberfläche und Zustände, nicht Klassenstruktur oder Renderer.
- **Weitere dB-Meter:** nur mit realer Datenquelle und klarer Lesefrage.
- **Samplegenaue Automation:** FL liefert diese Zusage nicht; Figma darf sie nicht suggerieren.
- **Linear Phase, Lookahead oder zusätzliche Filtertypen:** nicht Teil des ersten technischen Vertrags.
- **Dekoratives Dauer-Live oder Pulsieren:** verboten, wenn kein echter Messzustand dahintersteht.
- **Altes Remote-Limit ±3 dB/Q 0,4–2:** überholt; für Hand und Advisor gelten dieselben vollen Parameterbereiche.

## 13. Was tatsächlich noch Userentscheid braucht

Die früher hier aufgeführten Grundfragen zu Vergleich, PROTECTED, Mix,
Sidechain, Undo und dem Pflichtinhalt der Rückfallfläche sind beantwortet.
Noch offen sind nur:

1. Visuelle Abnahme oder Korrektur der gebauten Figma-Vorschläge in den vier
   aktuellen Gen-Bereichen `25:443`, `28:993`, `25:805` und `25:901`.
2. Visuelle Abnahme oder Korrektur des Umbaus der vorhandenen Suna-Basis
   `6:2864` zur Probeeq-Rückfallfläche.

Die Vergleichsauswahl und die Detailgesten für Schutzbereiche werden zunächst
auf Basis der festgelegten Bedienlogik und der freigegebenen Referenzen
gestaltet. Der User entscheidet daran visuell, statt dieselben Grundfragen
noch einmal beantworten zu müssen. Der Revisionsverlauf auf Seite 1, die
unsichtbare Sidechain im ersten Release und die zweistufige Mix-Umschaltung
sind ebenfalls beschlossen.

## 14. Empfohlene Reihenfolge

1. Den korrigierten EQ-Center-Frame `28:994` zusammen mit Overview statisch prüfen und abnehmen.
2. Verbleibende statische Korrekturen an Overview und EQ Center einarbeiten.
3. gemeinsame Komponenten und Varianten vervollständigen.
4. zweite Vergleichsquelle und Schutzbereich-Interaktion nach den bestehenden Entscheiden gestalten und zur visuellen Abnahme vorlegen.
5. die sechs Kern-Fixtures als vollständige Kontextframes bauen.
6. die acht Motion-Sequenzen verbinden und Reduced Motion ergänzen.
7. Suna-Basis `6:2864` zur minimalen Probeeq-Rückfallfläche umbauen und abnehmen.
8. nur die abgenommenen Frames im verbindlichen Depot als `CURRENT / APPROVED` markieren; Vorstände dort archivieren.
9. gebackene Assets und Goldens in vier Skalierungsstufen neu exportieren.
10. Node-IDs, Hashes und Abnahme im Repo dokumentieren.

## 15. Definition of Done für den Figma-Stand

Der Figma-Teil ist erst übergabefähig, wenn alle folgenden Punkte erfüllt sind:

- [x] Overview und EQ Center sind im verbindlichen Figma dunkel und 760×430.
- [ ] Overview und EQ Center sind als Gesamtdesign vom User abgenommen.
- [ ] Beide Seiten verwenden dieselbe Schale und dasselbe Tab-Paar an derselben Position.
- [ ] Jedes sichtbare Nicht-Material-Element ist Control oder ehrlicher Zustand; es gibt keine toten Elemente.
- [ ] Normal-, Ausnahme-, Disabled-, Focus- und Gegenpfade sind als Komponentenvarianten vorhanden.
- [ ] GAP und EPOCH sowie Broker und Probe offline bleiben jeweils unterscheidbar.
- [ ] Drei-Stufen-Geste und automatischer Rückfall sind vollständig prototypisiert.
- [ ] Direkte Band-/Parameterzüge besitzen kein Nachziehen und keinen Bounce.
- [ ] Zweite Vergleichsquelle und Bearbeitungsziel sind auf der Pluginfläche bedienbar.
- [ ] Spektrum gehört bei zwei Kurven ausschließlich zum Bearbeitungsziel.
- [ ] Schutzbereiche haben Erstellen, Bearbeiten, Kollisions- und Host-Verletzungszustand.
- [ ] Global, Bypass, Undo und Master besitzen vollständige Zustände und Gegenpfade.
- [ ] Sidechain und Mix behaupten keine Funktion vor ihrem Vertrag.
- [ ] Die sechs Kern-Fixtures sind als benannte Abnahmeframes vorhanden.
- [ ] Reduced Motion ist dokumentiert und demonstriert.
- [ ] Akzeptierte Frames liegen im verbindlichen Depot.
- [ ] Exporte und Goldens für 100/125/150/200 % sind neu erzeugt, benannt und gehasht.
- [ ] Die Abnahme ist unter `design/abnahmen/` mit dem wörtlichen User-Urteil festgehalten.

Erst danach ist die Eintrittslinie von **S31b / SONDE-020** erfüllt: Die native EQ-Zentrale darf gegen einen abgenommenen, zustandsvollständigen Figma-Stand gebaut werden.

## 16. Repo-Belege

- [UI-Spielregeln der EQ-Zentrale](ui-spielregeln-eq-zentrale.md)
- [Interaktions- und Motion-Spezifikation](interaktions-und-motion-spezifikation.md)
- [Oberflächen-Spezifikation](oberflaechen-spezifikation.md)
- [Zweiter Figma-Durchgang und Node-IDs](spielregeln-umbau-2026-08-23/LIES-MICH.md)
- [Entscheid: gleiche Größe und keine toten Elemente](../abnahmen/2026-08-24-seite2-groesse-keine-toten-elemente.md)
- [Entscheid: verbindliches Figma-Depot](../abnahmen/2026-08-22-figma-depot.md)
- [Entscheid: dieses Depot ist immer die einzige aktuelle Quelle](../abnahmen/2026-08-24-figma-depot-immer-aktuell.md)
- [Erfolgskriterium: exakt wie Figma](../abnahmen/2026-08-22-erfolgskriterium-exakt-wie-figma.md)
- [Skalierungsstufen 100/125/150/200](../abnahmen/2026-08-22-hub-antworten-35.md)
- [UI-Vertrag des Web-Simulators](../prototyp/contract/ui-contract.json)
- [Motion-Tokens des Web-Simulators](../prototyp/contract/motion-tokens.json)
- [Asset- und Golden-Manifest](../prototyp/contract/asset-manifest.json)
- [Technischer Lieferplan S26–31, G7 und S31b](../../docs/plan/plan.json)
- [Technischer Entwurf mit Drei-Stufen-Erratum](../../docs/FL-Nakama-Sonden-Design-Entwurf.md)
