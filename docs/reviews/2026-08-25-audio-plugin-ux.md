# Audio-Plugin-UX-Review — aktueller EqCopilot-Editor

**Datum:** 2026-08-25
**Umfang:** Der vorhandene JUCE-Editor `EqCopilotEditor` (EQ Copilot), nicht
der noch zu übertragende Figma-Stand von Nakama Gen/Probeeq. Der Befund trennt
also bewusst aktuelle Implementierung von künftig verbindlichem Produktdesign.

## Fazit

Der Editor hat einen tragfähigen Ruhefokus: Die Messkurve ist das größte und
zentralste Objekt; Messbereitschaft, Datenqualität und Verbindung werden nicht
verschwiegen. Die beabsichtigte Schleife ist damit klar:

> Wiedergabe starten → Messung reift → Befund öffnen → Bereich hören → mit
> der Gesamtmessung vergleichen → festhalten, erneut messen oder verlassen.

Der wichtigste Bruch dieser Schleife ist die **Hör-Markierung**. Im aktuellen
Editor schalten `Solo` und `Puls` per Klick einen anhaltenden Latch ein. Der
neue, verbindliche UX-Vertrag verlangt dagegen „HOLD TO AUDITION“: Der Effekt
ist nur unter dem gedrückten Finger aktiv und endet beim Loslassen. Die
vorhandenen Sicherheitsnetze (sichtbarer Aus-Knopf, Fensterschluss,
Sampleratenwechsel, Offline-Render und zehn Minuten Totmann) reduzieren das
Risiko, ersetzen aber nicht die unmittelbare, körperliche Rückkopplung einer
Haltedauer.

**Bewertung:** Als Diagnose-/Übergabeeditor ist der aktuelle Stand
gebrauchstauglich und auffallend ehrlich. Als bereits produktive Nakama-Gen-
Oberfläche hat er eine prioritäre Interaktionslücke und weitere
Migrationsdifferenzen zum akzeptierten Produktvertrag.

## Bewertungsgrundlage und Grenzen

| Kennzeichnung | Bedeutung in diesem Review |
|---|---|
| **Dokumentiert** | Durch aktuellen Quellcode oder eine repositoryeigene Abnahme belegt. |
| **Beobachtet** | Im datierten Figma-Beleg sichtbar; nicht als aktueller Live-Figma-Stand ausgegeben. |
| **Inferiert** | Begründete UX-Folge, die mit einem Host-/Audiotest bestätigt werden muss. |
| **Offen** | Mit den verfügbaren Belegen nicht beantwortbar. |

Es wurde kein VST3 in FL Studio mit Audio bedient und kein aktueller
Live-Figma-Knoten abgefragt. Deshalb sind Haptik, tatsächliche Verdeckung von
Popovers, Lesbarkeit auf dem kleinsten Hostmaß und die hörbare Qualität des
Vergleichs **offen**. Der vorhandene `EqCopShot` kann den echten Editor
offscreen rendern, beweist aber laut seiner eigenen Dokumentation weder
Interaktion im Host noch Barrierefreiheit.

## Aufgaben-, Objekt- und Fokusmodell

### Häufige Aufgaben

1. **Messung orientieren:** Ist Signal vorhanden, reif und glaubwürdig?
2. **Auffälligkeit einordnen:** Welcher Frequenzbereich und welche Diagnose
   verdienen Aufmerksamkeit?
3. **Hörhypothese prüfen:** Nur den betroffenen Bereich hören oder ihn
   pulsieren lassen, dann sofort zum unveränderten Mix zurückkehren.
4. **Vergleichen/weiterarbeiten:** Messstand für einen A/B-Vergleich merken,
   Messung neu beginnen oder den Befund verwerfen.
5. **Seltene Wiederherstellung:** Messpunkt binden, Dubletten-Konflikt lösen,
   Read-only-Zustand verstehen.

### Sonic- und Ownership-Map

| Objekt | Besitzer | Hör-/Entscheidungsnutzen | Sichtbare Repräsentation |
|---|---|---|---|
| Eingehender Mix | Host/Spur | Was liegt tatsächlich am Messpunkt an? | Live- und Kompositkurve, LUFS, dBTP |
| Messpunkt | Plugin-Instanz | Wo in der Session wird gemessen? | Rolle und Name im Kopf; Bindung nur im Popover |
| Messfenster | Analyse | Ist eine Diagnose belastbar? | „WARTET“, „HÖRT ZU“ oder „BEREIT“, Abdeckungsleiste |
| Befund/Frequenzbereich | Diagnose | Was soll gehört und beurteilt werden? | Marker in der Kurve; Detailkarte im Hinweise-Popover |
| Hör-Markierung | Temporärer Editor-/Audiozustand | Hypothese auditiv isolieren bzw. kontrastieren | Feldtönung, Mikrotext, Not-Aus |
| Vergleichskurve | Lokaler Editorzustand plus Dateiexport | Vorher/nachher bzw. Stand A gegen B prüfen | dritte Kurve und Punkt in der Mikrolegende |

**Fokusstatement (dokumentiert):** In der Ruhelage besitzt die
aggregierte Messkurve die Fläche. Die oberen Werkzeuge und unteren
Statuszellen unterstützen Orientierung; sie sind nicht das eigentliche
Arbeitsobjekt. Das passt zur Hörschleife eines Analyse-Plugins.

## Was bereits gut gelöst ist

1. **Die Oberfläche behauptet keine EQ-Bearbeitung, die nicht existiert.**
   Ohne freigegebene Hör-Markierung bleibt der Audiopuffer unverändert; die
   Messung liest den echten Mix vor einer möglichen Markierung. Das schützt
   vor dekorativen, aber falschen Reglern. **Dokumentiert.**

2. **Die Messung spricht ehrlich über ihre Aussagekraft.** No-data,
   Sammelphase, belastbare Messung, nicht abgedeckte Frequenzzonen,
   Frame-Drops und nicht-endliche Samples erhalten eigene sichtbare Signale.
   Insbesondere werden Befunde vor Messbereitschaft gesperrt statt als
   sichere Empfehlung zu erscheinen. **Dokumentiert.**

3. **Vergleich bleibt im gleichen Sichtkontext.** Die gespeicherte Kurve
   liegt nicht in einer anderen Seite, sondern über dem gleichen Frequenzraster;
   Glättung und Ansichtstransformation werden auf beide Kurven angewandt.
   Das vermeidet einen falschen visuellen Unterschied durch unterschiedliche
   Darstellung. **Dokumentiert.**

4. **Aktive Ausnahmezustände sind sichtbar und mit einer Rückkehr verbunden.**
   Der Read-only-Zustand sperrt die Bindung mit Grund; ein Kennungs-Konflikt
   bleibt rot sichtbar; die Hör-Markierung färbt ihren Frequenzbereich und
   bietet einen Not-Aus. **Dokumentiert.**

5. **Seltene Einrichtung wird nicht zur Startbarriere.** Der Messpunkt lebt
   bewusst in einem Popover, sodass „Plugin laden → Musik abspielen → Befund"
   ohne vorherige Formulararbeit möglich bleibt. **Dokumentiert.**

## Priorisierte Befunde

### P1 — Hör-Markierung ist ein Klick-Latch statt einer Halte-Geste

**Priorität: hoch, sofern der Editor eine produktive Nakama-Oberfläche sein
soll.**

- **Dokumentiert:** `Solo`/`Puls` rufen beim Klick `schalteMarkierung` auf.
  Ein zweiter Klick oder der separate Knopf „Markierung aus“ beendet den
  Zustand. Weitere Abbrüche sind technisch: Editor zu, Sampleratenwechsel,
  Offline/Freilauf oder zehn Minuten Inaktivität.
- **Dokumentiert:** Der produktübergreifende UX-Vertrag beschreibt hingegen
  „HOLD TO AUDITION“, aktiv nur während des Haltens und beendet beim
  Loslassen.
- **Kosten:** Die Hand kann den Hörvergleich nicht als vorübergehende Geste
  kodieren. Der Anwender muss sich nach dem Klick zusätzlich vergewissern,
  dass der Mix wieder normal ist. Das ist besonders teuer, weil die Aktion
  den Audiopfad verändert und sich Diagnose, DAW-Transport und Gehör zugleich
  Aufmerksamkeit teilen.
- **Wichtige Einordnung:** Die aktuelle Quelle nennt diese Bedienung selbst
  eine Interims-UI. Daher ist dies kein Grund, das Figma-Design selbstständig
  umzudesignen, sondern ein klarer Übergabe-/Migrationspunkt.
- **Abnahmekriterium:** Drücken aktiviert die Markierung; Loslassen beendet
  sie innerhalb eines Audio-Blocks. Der betroffene Befund bleibt während des
  Haltens lesbar, der Rückweg verlangt keinen zweiten Klick. Offline,
  Aufnahme, unbekannter Transport und Fensterschluss bleiben harte,
  verständlich angezeigte Sperren.

### P2 — Der Befund- und Hörkontext kann den Graphen verdecken

**Priorität: mittel; Ursache dokumentiert, tatsächliche Verdeckung offen.**

- **Dokumentiert:** „Hinweise“ öffnet eine bis zu 560 px hohe CallOutBox;
  bei längeren Listen kommt ein scrollbarer Viewport hinzu. In den Karten
  liegen die `Solo`-/`Puls`-Aktionen.
- **Inferiert:** Ein so großes, am Werkzeugstreifen verankertes Popover kann
  auf dem 600×416-Minimum die Kurve überdecken, die während der Hörprüfung
  die Orientierung geben soll. Damit entstünden Auswahl und akustische
  Prüfung ohne konstanten Blick auf Bereich, Gesamtform und andere Marker.
- **Prüfung:** `EqCopShot` bei 600, 760 und 1200 px erzeugen; anschließend
  im echten Host einen langen Befundtext plus aktives Vorhören testen. Nur
  wenn Kurve, ausgewählter Befund und Stop-Zustand gleichzeitig lesbar bleiben,
  ist der Kontextnachweis erbracht.

### P3 — „Festhalten“ koppelt zwei unterschiedliche Absichten

**Priorität: mittel.**

- **Dokumentiert:** Ein Klick speichert gleichzeitig eine Snapshot-Datei und
  aktiviert die Vergleichskurve. Die Meldung erklärt beide Folgen erst nach
  dem Klick.
- **Kosten:** „Für den Vergleich merken“ ist ein häufiger, reversibler
  Hörschritt; „Datei exportieren“ ist eine seltene, externe Persistenzaktion.
  Die Kopplung macht aus einer einfachen A/B-Geste eine Aktion mit Nebenwirkung
  und erschwert das mentale Modell, wenn der Vergleich entfernt wird, die
  Datei aber bestehen bleibt.
- **Offene Produktentscheidung:** Soll jeder Vergleich beweisbar archiviert
  werden, ist die Kopplung korrekt, braucht aber einen vorweg klaren Namen.
  Andernfalls müssen Vergleich und Export getrennte, stabil auffindbare
  Aktionen sein. Diese Entscheidung gehört in Figma/Produktvertrag, nicht
  in eine eigenmächtige UI-Änderung.

### P4 — Gleichrangige Werkzeugzeile mischt Hörarbeit, Anzeige und Setup

**Priorität: mittel.**

- **Dokumentiert:** Glättung, Ansicht, Festhalten, Vergleich löschen, neu
  messen, Messpunkt und Hinweise teilen sich eine einzige dauerhaft sichtbare
  Reihe mit ähnlicher Gewichtung.
- **Kosten (inferiert):** Während der häufigen Aufgaben „Kurve verstehen"
  und „Befund hören“ konkurrieren seltener Messpunkt-Setup und Reset visuell
  mit den Vergleichswerkzeugen. Das erhöht Blickwechsel und kann den Nutzer
  dazu verleiten, einen Global-Reset als lokalen Korrekturschritt zu lesen.
- **Placement-Swap-Test:** Das Verschieben von `Messpunkt…` aus der Ruhezeile
  senkt die Aufmerksamkeit dort, ohne die Messung zu blockieren; das
  Verschieben von `Hinweise` oder `Festhalten` würde dagegen einen häufigen
  Schritt der Hör-/Vergleichsschleife verlängern. Die vorhandene Platzierung
  von Messpunkt ist folglich schwächer begründet als die der beiden anderen.

### P5 — Kurze Erfolgsmeldungen verdecken zentrale Messzellen

**Priorität: niedrig bis mittel.**

- **Dokumentiert:** Meldungen nach Festhalten, Reset und Markierung liegen
  für vier bis acht Sekunden über der Mitte der Statuszeile und verdrängen
  dort mehrere Kennzahlen.
- **Kosten (inferiert):** Direkt nach einer Vergleichs- oder Höraktion sind
  LUFS, Peak und Dynamik gerade relevante Gegenbelege. Eine Rückmeldung ist
  richtig, das Überdecken derselben Prüfwerte unterbricht aber die
  Entscheidungsschleife.
- **Prüfung:** Mit realem Material prüfen, ob Anwender die Werte in dieser
  Phase suchen. Falls ja, muss die Rückmeldung räumlich beim Auslöser oder
  außerhalb der Kennzahlen stattfinden.

### P6 — Produktsprach- und Designmigration sind ungeklärt

**Priorität: mittel für einen Release, nicht als Fehler am Diagnoseprototyp.**

- **Dokumentiert:** Der aktuelle Editor verwendet deutsche Beschriftungen
  wie „Kurve mittel“, „Festhalten“, „Neu messen“ und „Hinweise“ sowie die
  aktuelle Materialoberfläche.
- **Dokumentiert:** Die verbindliche Produktarbeit verlangt englischen
  Produkttext; Figma ist die Quelle der zukünftigen Nakama-Optik. Der
  datierte Figma-Beleg zeigt eine andere Informationsarchitektur mit Gen-
  Overview/EQ Center und Probeeq.
- **Folge:** Der C++-Editor darf nicht als visuell oder interaktiv
  freigegebene Umsetzung des neuen Figma-Produkts gelten. Seine
  Diagnosemechanik ist ein technischer Ausgangspunkt, nicht sein
  Erscheinungsbild.

## Platzierungsbeweise

| Gruppe | Hör-/Arbeitsjob | Scope | Sichtbarkeit | Begründung / Kosten eines Tauschs |
|---|---|---|---|---|
| Messkurve, Achsen, Marker | Ursache lokalisieren und Gesamtmix einordnen | Mix + Diagnose | persistent | Muss im Ruhefokus liegen; ein Tausch mit der Werkzeugzeile machte Steuerung wichtiger als die hörbare Evidenz. |
| Live-/Komposit-/Vergleichskurve | Veränderung gegen Kontext beurteilen | Messung | persistent bei gültigen Daten; Vergleich conditional | Gleiche räumliche Achse erlaubt visuelle A/B-Prüfung; ein separater Vergleichsbereich bräche die direkte Zuordnung. |
| Hinweise + Befundkarte | Aus Messung eine Hörhypothese machen | einzelner Befund | Entry persistent, Detail contextual | Die Karte gehört zum Befund, nicht zum globalen Setup. Sie darf jedoch die Kurve während der Prüfung nicht verdecken (P2). |
| Solo/Puls bzw. künftig Hold-to-audition | Hypothese auditiv isolieren | einzelner Befund, temporär | nur bei vorhandenem Befund; während Geste betont | Gehört neben den Befund. Der aktive Zustand muss an Befund und Bereich sichtbar sein; der aktuelle Latch verletzt die Gesten-/Rückkehrlogik (P1). |
| Festhalten / Vergleich | A/B-Entscheidung wiederholen | Messung, lokal im Editor | persistent, „Vergleich aus“ conditional | Häufig genug für die Nähe zur Kurve. Die Dateischreibung besitzt aber einen anderen Scope (P3). |
| Messpunkt / Konflikt | Setup und Recovery | Instanz/Session | Messpunkt on demand; Konflikt exceptional | Setup darf nicht die Ruhelage dominieren; Konflikt muss dagegen sichtbar bleiben, solange er besteht. |
| Messstatus, Verbindung, Datenqualität | Glaubwürdigkeit der sichtbaren Analyse einschätzen | Messung/Integration | persistent; Ausnahme betont | Sinnvoll am Rand des Hauptobjekts. Hinweise sollten Kennzahlen nicht verdecken (P5). |

## Sichtbarkeitsmatrix

| Element | Keine Daten | Messung läuft | Messbereit | Befund / Vorhören | Read-only, Konflikt, Offline |
|---|---|---|---|---|---|
| Messkurve | ersetzt durch Aufforderung zur Wiedergabe | sichtbar, Diagnose gesperrt | Hauptfokus | muss Kontext bleiben | nur gültige Daten zeigen; keine neue Behauptung |
| Hinweise/Befunde | keine oder gesperrte Befunde | gesperrt | verfügbar | Detailkarte + betroffener Bereich sichtbar | bei fehlender Grundlage nicht aktivierbar |
| Hör-Markierung | nicht verfügbar | nur nach Freigabe | verfügbar | heute Latch + Feldtönung + Not-Aus; Ziel: Halten + Loslassen | sofort beenden/sperren, Grund sichtbar |
| Vergleich | leer/deaktiviert | nach gültiger Kurve möglich | sichtbar, falls gehalten | bleibt als Kontext sichtbar | darf keine ungültige Messung als aktuell ausgeben |
| Messpunkt | verfügbar | verfügbar | verfügbar | nicht Teil der Hörgeste | read-only deaktiviert mit Grund |
| Kennungs-Konflikt | nur falls vorhanden | nur falls vorhanden | nur falls vorhanden | sichtbar zusätzlich | rot und auflösbar, bis gelöst |

## Empfohlene Reihenfolge vor einer Produktübernahme

1. **Rolle des aktuellen Editors entscheiden:** Ist er dauerhaftes
   Diagnosewerkzeug oder eine Brücke zu Nakama Gen? Nur im zweiten Fall sind
   P1 und P6 Release-Blocker.
2. **Audition-Vertrag an einer Stelle festziehen:** Halten/Lossen mit echter
   Host- und Audioverifikation implementieren oder den UX-Vertrag bewusst
   ändern. Beides darf nicht parallel gelten.
3. **P2 bis P5 an der echten, kleinsten Hostgröße testen:** keine weitere
   Platzierungsentscheidung aus dem Quellcode ableiten.
4. **Erst danach in die drei aktuellen Rework-Referenzen übertragen und per
   Pixelvergleich abnehmen.** Frühere Figma-Knoten und der datierte Figma-
   Beleg sind seit dem neueren User-Entscheid nur noch Verlauf.

## Quellen

- [Aktueller Editor: Interaktion, Layout und Darstellung](../../eq-copilot/plugin/src/PluginEditor.cpp)
- [Editorzustand und Komponenten](../../eq-copilot/plugin/src/PluginEditor.h)
- [Audio-Passthrough und Analysepfad](../../eq-copilot/plugin/src/PluginProcessor.cpp)
- [Headless-Sichtwerkzeug](../../eq-copilot/plugin/tests/ShotTestMain.cpp)
- [Aktueller Editor-/Diagnosestand](../../openwiki/plugin/editor-and-diagnostics.md)
- [Verbindlicher UX-Vertrag](../../UX-CONTRACT.md)
- [Designautorität und Produktumfang](../../design/LIES-MICH.md)
- [Datierter Figma-Beleg, beobachtet](../../design/assets/figma/2026-08-24-ui-assets/beleg-review-a-b-c-viewport.png)
