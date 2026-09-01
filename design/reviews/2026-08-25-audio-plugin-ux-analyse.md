# Audio-Plugin-UX-Analyse — Nakama / EQ-Copilot

**Stand:** 2026-08-25  
**Geltungsbereich:** aktueller JUCE-Editor, Nakama-Produktvertrag und der geplante Gen-/Probeeq-Workflow  
**Methode:** Audio-Plugin-UX-Skill mit Evidenzklassen, Listening-Loop, Sonic-Object-Map, Placement Proofs, Zustandsmatrix und Host-Testplan.

> **Fortschreibung 01.09.2026:** Diese Analyse bleibt ein Befund ihres
> Prüfdatums. Ihre Aussagen, 760×430 sei die einzige aktuelle Gen-Größe und
> Fenster-Resize sei noch offen, sind durch
> [`../abnahmen/2026-09-01-gen-fenstergroesse-und-resize.md`](../abnahmen/2026-09-01-gen-fenstergroesse-und-resize.md)
> abgelöst: 760×430 ist das kompakte Minimum, 950×538 der Standard im festen
> 76:43-Rahmen. Native Umsetzung und FL-Prüfung bleiben offen.

## Kurzurteil

**Observed:** Der aktuell gebaute `EqCopilotEditor` ist als Mess- und Diagnosewerkzeug brauchbar: Er zeigt eine LTAS-Kurve, Messstatus, LUFS/True-Peak/Dynamik, Befunde, Vergleich, Reset, Bindung und eine Hör-Markierung. Der Offscreen-Sichttest rendert bei 1200×832 und 600×416.

**Documented:** Die aktuelle Produktentscheidung beschreibt dagegen Nakama Gen als Hub mit einer zweiten EQ-Center-Seite und Nakama Probeeq als vollwertigen, manuell und remote bedienbaren EQ.

**Inferred:** Für die wichtigste wiederholte Arbeit — *Befund hören → richtige Sonde wählen → EQ-Band bearbeiten → anwenden/verwerfen → erneut vergleichen* — fehlt im aktuellen Produktpfad noch der zentrale Handlungsort. Die derzeitige Oberfläche ist deshalb kein fertiger Probeeq-/Gen-EQ-Workflow, sondern ein Analyzer-/Advisor-Zwischenstand.

Die wichtigste Konsequenz lautet: **Vor visueller Politur muss die Zustands- und Ownership-Kette eines einzelnen EQ-Bands end-to-end funktionieren.** Erst danach lohnt die Skalierung auf acht Bänder, mehrere Sonden und Master-EQ.

## 1. Bewertungsgrundlage und Quellenkonflikte

Die Quellen werden getrennt behandelt:

1. Source und Tests beschreiben, was heute tatsächlich läuft.
2. Der aktuelle Produkt-/Designvertrag beschreibt, was Gen und Probeeq werden sollen.
3. Der spätere User-Entscheid vom 25.08. setzt drei gelieferte Rework-Bilder als aktuelle visuelle Wahrheit in progress; Figma-Exporte und Live-Nodes sind frühere Stände.
4. Die Audio-Plugin-UX-Prinzipien liefern Prüfmechanismen, aber keine eigene Nakama-Optik.

Es gibt einen relevanten Dokumentkonflikt:

- [`oberflaechen-spezifikation.md`](../docs/oberflaechen-spezifikation.md) beschreibt noch drei Oberflächen und an mehreren Stellen den älteren ±3-dB-Remote-Cap.
- [`ui-spielregeln-eq-zentrale.md`](../docs/ui-spielregeln-eq-zentrale.md) und die Abnahmen vom 23.–25.08. beschreiben die spätere Zwei-App-Architektur, 760×430 für beide Gen-Seiten, zentralen Probeeq-EQ, Draft-/Lease-Zustände und die neue Regel, dass der User-EQ nicht auf den alten Remote-Cap beschränkt ist.

**Open:** Dieser Review nimmt die neueren EQ-Center-Regeln als Zielvertrag. Der Dokumentkonflikt sollte vor Implementierung bereinigt werden, sonst können UI und Tests unterschiedliche Produktwahrheiten behaupten.

## 2. Was heute vorhanden ist

### 2.1 Aktueller Haupteditor: Analyzer/Advisor

**Observed aus Source und Sichttest:**

- [`PluginEditor.cpp`](../../eq-copilot/plugin/src/PluginEditor.cpp) erzeugt einen Material-/JUCE-Editor mit fester 750:520-Gerätegeometrie.
- Der Default ist 1200×832; erlaubt sind 600×416 bis 1950×1352.
- Die zentrale Anzeige ist eine aggregierte LTAS-Kurve über 20 Hz–20 kHz, ergänzt um Live-Kurve, optionale Vergleichskurve, Resonanzmarker und Abdeckungszonen.
- Die obere Werkzeugzeile enthält Glättung, natürliche/begradigte Ansicht, Festhalten, Vergleich, Befunde, Messpunkt und Neu messen.
- Die Statuszeile zeigt Verbindung, Messfortschritt, LUFS-I, LUFS-S, True Peak, Dynamik und Datenfehler/Drops.
- Befunde erscheinen erst über den Button `hinweisKnopf` in einem Callout/Viewport. Jede Karte kann `Solo` oder `Puls` auslösen.
- Der Editor liest einen Snapshot pro Timer-Tick; die Messdaten werden in der Engine ungefähr mit 20 Hz publiziert, die Diagnose läuft ungefähr einmal pro Sekunde.
- Read-only-State und Kennungskonflikt werden sichtbar gemacht. Die Hör-Markierung endet bei Fensterabschluss, Offline/Freilauf, Sampleratewechsel oder Totmann-Timeout.

### 2.2 Aktuelle Probeeq-/Sonden-Schicht

**Documented und durch Tests bestätigt:**

- [`SondeProcessor.h`](../../eq-copilot/plugin/sonde/SondeProcessor.h) meldet `hasEditor() == false` und liefert keinen Editor.
- Nakama Probeeq ist heute audio-neutral; der DSP ist laut CMake-Kommentar bis zum späteren DSP-Schritt zurückgestellt.
- Der State-Vertrag definiert für `active_probe` 109 serialisierte Parameter: fünf globale Werte und acht Bänder mit je 13 Feldern.
- Diese 109 Werte sind **nicht** gleichbedeutend mit aktuellen Host-Parametern oder einer fertigen EQ-Bedienung. Der Nulltest bestätigt, dass Probeeq aktuell keine Hostparameter exponiert.
- Der Capability-Report markiert `remote_control` derzeit als `unsupported`. Eine UI darf daher keine fertig bedienbare Remote-Aktion vortäuschen, bevor Pairing, Revision, Lease und Revert als Produktpfad bewiesen sind.

### 2.3 Sichtbefund

Der echte Offscreen-Sichttest aus [`ShotTestMain.cpp`](../../eq-copilot/plugin/tests/ShotTestMain.cpp) zeigt:

- **Stärke:** Die Oberfläche hat eine klare zentrale Hör-/Messfläche; Graph, Meter und Statuswerte gehören sichtbar zusammen.
- **Stärke:** Bei fehlendem Broker wird `APP AUS / LOKAL` statt eines grünen Verbindungsversprechens gezeigt.
- **Risiko:** Die Fläche ist bei 600×416 stark verdichtet; die 600er-Grenze ist ein technischer Randtest, nicht die aktuelle Produktgröße.
- **Risiko:** Die sichtbare Oberfläche ist überwiegend deutsch (`FESTHALTEN`, `NEU MESSEN`, `HÖRT ZU`, `APP AUS / LOKAL`), obwohl der Produktvertrag Englisch als UI-Sprache festlegt.
- **Risiko:** Die aktuelle Editor-Geometrie 750:520 passt nicht zum aktuellen Zielvertrag 760×430 für Gen Overview und EQ Center.
- **Zentraler Gap:** Es gibt keinen EQ-Handle, keine Bandauswahl, keine Frequenz-/Gain-/Q-Manipulation, keinen Bypass des EQ-Zustands und keinen Probe-/Master-Selector.

## 3. Listening Loop und Aufgabenfrequenz

Der maßgebliche Loop ist:

**hören → orientieren → Klangobjekt wählen → manipulieren → hör-/sichtbare Konsequenz prüfen → vergleichen → behalten, ändern oder zurücksetzen**

| Häufigkeit | Aufgabe | Heute | UX-Bewertung |
|---|---|---|---|
| häufig | aktuelle Messlage und Befund verstehen | Kurve, Marker, Status und Befunddialog vorhanden | **teilweise erfüllt**; Befund ist nicht im dauernden Arbeitskontext |
| häufig | Frequenzbereich hörbar prüfen | `Solo`/`Puls` vorhanden, Playback-/Realtime-Gates vorhanden | **für Analyzer gut**, aber nicht als Hold-to-Audition des EQ-Drafts |
| häufig | EQ-Band einer bestimmten Sonde ändern | keine EQ-Bedienung im aktuellen Editor | **nicht erfüllt** |
| häufig | temporär anwenden und anschließend bestätigen oder verwerfen | aktueller `Festhalten`-Vergleich ist kein EQ-Apply-Lease | **nicht erfüllt** |
| häufig | Vorher/Nachher level-gematcht beurteilen | Vergleichslinie vorhanden; Festhalten schreibt zusätzlich Snapshot-Datei | **teilweise erfüllt**, zwei Jobs sind gekoppelt |
| häufig | zur nächsten Sonde wechseln | kein Sonden-Durchschalter im aktuellen Editor | **nicht erfüllt** |
| gelegentlich | Messpunkt benennen/binden | Callout mit Rolle, Label und Pair-ID | **erfüllt**, aber nicht Teil des täglichen Hörloops |
| gelegentlich | Diagnose-Snapshot exportieren | vorhanden | **erfüllt** |
| recovery | no signal / noch nicht belastbar | `keine Daten`, `sammelt`, 15-Sekunden-Schwelle | **gut sichtbar** |
| recovery | State nicht lesbar | dauerhafter Read-only-Hinweis und byte-preserving State-Verhalten | **stark** |
| recovery | Broker-/Sondenverbindung oder Remote-Command verloren | aktueller Editor kennt nur seine Pipe-/App-Verbindung; der geplante Probeeq-Pfad ist noch nicht implementiert | **offen** |

## 4. Sonic-Object-Map und Fokus

| Ebene | Klangobjekt | Besitz/Scope | Sichtbare Konsequenz |
|---|---|---|---|
| Host/Session | Transport, Aufnahme, Automation, Sample-Rate | Host | bestimmt, ob Preview/Audition zulässig ist |
| Mix/Gen | Gesamtmix und Befund | Gen | Quellen-/Band-Zusammenhang, Konfidenz, Ursache |
| Sonde | eine ausgewählte Instrumenten-/Bus-Spur | Probeeq-Instanz | bestätigte Kurve, Link-Health, Bypass, Revision |
| Band | ein EQ-Band in der ausgewählten Spur | Band-Slot | Frequenz, Gain, Q, Typ, Kanal und Dynamik |
| Draft | vorgeschlagener oder temporär angewendeter Bandzustand | Gen/Remote → Probeeq | proposed, auditioning, applied-temporary, stale, rejected |
| Master | eigener EQ-Zielpfad | Gen | nicht als weitere Radposition verstecken |
| Global | Bypass, Input/Output, Width, Mono-Bass, später Auto-Gain/Mix | ausgewählte EQ-Spur bzw. globaler Layer | globaler Klang-/Sicherheitsstatus |

**Resting focus heute:** aggregierte Messkurve plus Diagnose.  
**Resting focus im Zielprodukt:** die aktuell ausgewählte Spur mit ihrer bestätigten EQ-Kurve; bei aktiver Bearbeitung der ausgewählte Band-Handle innerhalb des Gesamtresultats. Die Kurve muss dabei den gesamten Kontext behalten.

Wenn ein Gen-Befund eine Sonde adressiert, darf die Oberfläche nicht nur den Vorschlag zeigen. Sie muss immer erklären: **welche Sonde ist ausgewählt, welcher Zustand ist bestätigt, welcher Zustand ist nur vorgeschlagen, und wohin wirkt die nächste Geste?**

## 5. Sichtbarkeitsmodell

| Klasse | Elemente | Begründung |
|---|---|---|
| Persistent | Gesamt-EQ-Kurve, aktive Spur/Master, Bandidentitäten, Bypass-Wahrheit, Fresh/Stale, Konfidenz, Link-/Capability-Gesundheit | Orientierung und auditive Wahrheit dürfen nicht verschwinden |
| Contextual | Präzisionswerte des ausgewählten Bands, Kanalmodus, Dynamik-Details, Schutzbereich am ausgewählten Objekt | erscheinen am Owner; halten den Graph-Kontext frei |
| Transient | Hover-Wert, exakte Frequenz/Gain/Q, Fine-Adjust-Hinweis, Hold-Audition-Region, kurze Lease-Restzeit | unterstützt die konkrete Geste ohne permanenten Parameterwall |
| Conditional | Dynamik-Panel nur bei aktivierter Dynamik; Sidechain im ersten Release nicht sichtbar; Global-Panel eingeklappt; zweiter Graph-Track nur bei Bedarf | Fähigkeit erst zeigen, wenn sie existiert; aktiven Zustand am Einstieg markieren |
| Pinned | vom User angeheftetes Global-/Detail-Panel | Expert:innen dürfen Wiederholungsarbeit verkürzen, ohne Default-Komplexität zu erzwingen |
| Exceptional | BROKER OFFLINE, PROBE OFFLINE, STALE, command rejected, host automation changed target, protected violation, no signal, recording/unknown blockiert Audition | Ausnahme spricht auf der Ebene, auf der sie entsteht |
| Unavailable | Remote-Steuerung vor Capability-Nachweis, Sidechain-Quelle im ersten Release, nicht messbare Frequenzbereiche | nicht als bedienbare Funktion darstellen; bei aktiver Erwartung mit Grund deaktivieren |

## 6. Placement Proofs

| Control/Gruppe | Listening-Job | Owner | Placement Proof | Sichtbare Zustände | Kosten bei Verlagerung | Evidenz |
|---|---|---|---|---|---|---|
| Gesamt-EQ-Graph | Ergebnis sehen und Klangentscheidung treffen | ausgewählte Spur oder Master | zentrale Fläche; Frequenz horizontal und Gain vertikal sind domain-konform | persistent, no signal, stale, selected track | ein Karten-/Dashboard-Layout trennt Objekt und hörbare Konsequenz | Documented + Inferred, hoch |
| Band-Handle | Frequenz/Gain direkt und reversibel ändern | Band-Slot | Handle bleibt am Kurvenlocus; Auswahl hebt genau dieses Band hervor | normal, selected, disabled, protected, automation-moved | eine entfernte Parameterzeile zwingt zur mentalen Zuordnung und verlängert den Motorweg | Documented, hoch |
| Selected-band detail | Q, Typ, Kanal, Dynamik präzisieren | ausgewähltes Band | kontextuell beim Handle oder in einem stabilen Detailbereich, ohne Kurve zu verdecken | selected, dynamic active/collapsed, unavailable | globales Panel suggeriert falsche Ownership; wandernde Popovers brechen Motorlernen | Inferred, mittel-hoch |
| Sonden-Durchschalter | schnell zur nächsten Spur wechseln | EQ-Center/Gen | direkt auf Seite 2, gleiche Position auf beiden Seiten; Master als eigenes Ziel | selected, disconnected, stale | Auswahl auf Seite 1 erzwingt den ausdrücklich abgelehnten Mehrklick-Weg | User-decided + Documented, hoch |
| Draft-Kasten | Vorschlag hören, temporär anwenden, bestätigen/rejecten | Remote-Draft der ausgewählten Sonde | nahe am Zielobjekt, aber außerhalb der Kurvenlocus; `HOLD TO AUDITION`, `APPLY`, Bestätigen, `REJECT` als klarer Automat | none, ready, auditioning, temporary apply, stale, rejected, blocked | reines Popup verliert Ownership; ein globaler Draft-Button verschleiert das Ziel | User-decided + Documented, hoch |
| Global-Panel | seltenere Spur-/Output-Entscheidungen | ausgewählte EQ-Spur/global layer | eingeklappt als `GLOBAL ⌄`; AUTO direkt neben OUTPUT, wenn vertraglich aktiv | collapsed, expanded, bypassed, unavailable | permanent sichtbare globale Regler konkurrieren mit dem Band-Job | User-decided, hoch |
| Bypass | schnelle Hörvergleichsentscheidung | EQ-Spur | persistent und unabhängig vom Detailpanel; Hard-Bypass bleibt als Wahrheitssignal sichtbar | active, bypassed, unavailable | im Global-Panel verborgen verlängert den A/B-Weg und lässt den Zustand leichter übersehen | Documented, hoch |
| Undo/Revision | bestätigte Änderung zurücknehmen | ausgewählte Sonde | kleines, stabiles Symbol nahe der Spur-/Display-Chrome; Verlauf bleibt nachvollziehbar | available, just-undone, unavailable | großer globaler Button nimmt dem Graph Platz; bloß `UNDO · n` erklärt nicht, was zurückgeht | User-decided + Documented, hoch |
| Link-/Capability-Status | kann ich dieser Anzeige glauben? | Broker, Probe, Host | Status am Header/Spur-Owner; BROKER OFFLINE und PROBE OFFLINE getrennt | healthy silent, measuring, stale, disconnected, degraded | ein allgemeines `CONNECTED` behauptet zu viel und lässt den Ausfallort offen | User-decided + Documented, hoch |
| Page tabs | zwischen Overview und EQ Center wechseln | Gen-App | identisches Tab-Paar an identischer Stelle auf beiden Seiten | active, keyboard focus | freier Pfeil/Play-Icon ist ein toter oder falsch interpretierter Handgriff | User-decided, hoch |

### Swap-Test-Ergebnis

- Wenn der Sonden-Selector in die Overview wandert, steigt der Weg zum häufigen EQ-Job und widerspricht der bindenden Seite-2-Entscheidung.
- Wenn Global-Controls neben einem Band erscheinen, wird unklar, ob `OUTPUT`, `WIDTH` oder `MONO BASS` nur dieses Band oder die ganze Spur betrifft.
- Wenn Draft-Status ausschließlich im Header steht, fehlt die Ownership am betroffenen Band.
- Wenn Bypass nur im eingeklappten Global-Panel liegt, wird der schnelle Hörvergleich unnötig langsam.
- Wenn ein ausgewähltes Band den Gesamtgraphen ersetzt, verliert der User die Vergleichsumgebung für die Hörentscheidung.

## 7. Zustandsmatrix für die Zieloberfläche

| Element | Idle | Hover | Selected | Hold/Audition | Temporary Apply | Confirmed | Host Automation | Offline/Stale | No signal/blocked |
|---|---|---|---|---|---|---|---|---|---|
| Gesamtgraph | bestätigt, ausgewählte Spur | Locus-Preview | aktive Kurve + Kontext | Hörbereich sichtbar | candidate/applied eindeutig getrennt | committed result | Kurve darf sich extern ändern | einfrieren, Stale markieren | Analyse nicht als gültig zeichnen |
| Band-Handle | normal | exakter Wert transient | Ring/Focus + Detail | Preview-Zone am Band | candidate marker | committed marker | extern bewegbar, Herkunft sichtbar | disabled oder read-only mit Grund | unavailable, kein Ghost-Handle |
| Spur-Selector | aktuelle Spur | mögliche Zielspur | Zielspur klar markiert | bleibt stabil | bleibt stabil | bleibt stabil | Ziel kann stale werden | Probe offline sichtbar | unavailable ohne Probe |
| Draft-Kasten | `DRAFT` nur wenn vorhanden | Werte/Hint | Zielspur/Band lesbar | `AUDITIONING`, Release beendet sofort | `CONFIRM` + Restzeit, `REJECT` | kein Draft, Revisionseintrag | Draft wird stale | blockierter Grund | Transport/Recording/Unknown als Grund |
| Bypass | aktiv/bypassed | Fokus | kein Ownership-Wechsel | Vergleich bleibt hörbar | candidate nicht mit Bypass verwechseln | committed | Hostautomation sichtbar | disabled bei fehlender Capability | unavailable mit Grund |
| Global | `GLOBAL ⌄` | Fokus | expanded | bleibt nicht im Hörweg | keine falsche Candidate-Anzeige | Werte bestätigt | extern aktualisieren | spurbezogen stale | nicht meaningful deaktivieren |
| Undo | available/unavailable | Symbol-Fokus | Verlauf der Sonde | Preview erzeugt keinen Eintrag | Lease erzeugt keinen bestätigten Eintrag | Revision sichtbar | Automation nicht als Maus-Undo ausgeben | disabled mit Grund | unavailable |
| Status | gesund still | Detail optional | Owner sichtbar | Audition-Gate sichtbar | command pending | committed/healthy | automation conflict | BROKER/PROBE/STale getrennt | no signal/degraded |

### Kritische Übergänge

1. **Idle → Selected:** Auswahl darf den Graph-Kontext nicht löschen.
2. **Selected → Hold:** Hold startet nur bei gültigem Playback und beendet beim Release sichtbar und unmittelbar.
3. **Hold → Temporary Apply:** erster Klick macht den Zustand hörbar, aber noch nicht persistent; Lease und Rückfall müssen sichtbar sein.
4. **Temporary Apply → Confirmed:** zweiter Klick erzeugt genau eine bestätigte Revision.
5. **Temporary Apply → Revert:** Stop, Lease-Ablauf, Host-Epochenwechsel, Fehler oder Reject kehren zum bestätigten State zurück.
6. **Manual/Remote → Host Automation:** der Wert darf sich extern ändern; die Oberfläche muss Herkunft und Stale-Konflikt erklären.
7. **Connected → Probe Offline:** die bestätigte Kurve bleibt als letzter gültiger Stand sichtbar, Regler werden gesperrt; keine editierbare Geisterkurve.

## 8. Stärken des aktuellen Stands

1. **Snapshot-Konsistenz:** Graph, Diagnose und Export verwenden denselben kopierten Messstand. Das verhindert konkurrierende UI-Wahrheiten.
2. **Ehrliche Messgrenzen:** ungültige LTAS-Bänder bleiben Lücken; die Engine erfindet keine Randwerte.
3. **Gute Ausnahme-Grundlage:** no data, collecting, read-only, Kennungskonflikt, Drops und NaN/Inf werden nicht still verschluckt.
4. **Hör-Markierung mit Sicherheitsnetz:** Die Tests bestätigen Transport-/Realtime-Gates, Fade-out und bitgleichen Rückweg nach Ende.
5. **Kein Fake-Parameterbestand:** Probeeq meldet aktuell keine Hostparameter, solange der DSP nicht existiert. Das ist technisch ehrlich, macht aber die UI-Aufgabe sichtbar statt sie zu lösen.

## 9. Wichtigste UX-Risiken

### P0 — Zielprodukt und aktuelles Plugin sind nicht derselbe Handlungsort

Der aktuelle Main-Editor ist Analyzer/Advisor; Probeeq hat weder Editor noch DSP-Bedienpfad. Die Kernhandlung der Produktvision ist dadurch nicht testbar.

**Nachweis:** `PluginEditor.*`, `SondeProcessor.h`, `nakama-parameter-v1.json`, `SondeNullTest`.

### P0 — Geometrie-Konflikt

Der aktuelle Editor hält 750:520 und erlaubt 600×416. Der aktuelle Gen-Vertrag verlangt 760×430 für Overview und EQ Center. Das ist keine reine Skalierungsfrage: Bei gleichem Fensterformat ändern sich Graphfläche, Detailbereich und erreichbare Trefferziele.

### P1 — `Festhalten` koppelt zwei verschiedene Jobs

Der Button erzeugt sowohl die Vergleichslinie als auch eine Snapshot-Datei. Vergleich ist ein häufiger Hörschritt; Export ist ein gelegentlicher Dokumentationsschritt. Diese Kopplung erzeugt unnötige Nebenwirkung und macht den Button semantisch schwer lesbar.

### P1 — Befunde sind hinter einem Dialog versteckt

Die Diagnose ist ein Kern des Produkts, aber im Default nur als `1 AUFFÄLLIGKEIT`/Hinweis-Button sichtbar. Der User sieht den Frequenzmarker, muss für Ursache, Wirkung und nächsten Schritt aber in ein Callout wechseln. Für gelegentliche Inspektion ist das okay; für den wiederholten „Befund → Eingriff“-Loop fehlt der direkte Übergang zum Zielband.

### P1 — Audition-Geste passt noch nicht zum neuen Draft-Vertrag

Der aktuelle Analyzer latcht `Solo`/`Puls` per Klick. Der neue EQ-Center-Vertrag definiert `HOLD TO AUDITION` für Drafts. Beides kann koexistieren, muss aber visuell und technisch als zwei unterschiedliche Hörverträge erkennbar bleiben.

### P1 — Verbindungsstatus ist noch auf die alte Pipe-Sicht begrenzt

`APP VERBUNDEN` beschreibt die App-/Broker-Pipe, nicht automatisch die Erreichbarkeit einer konkreten Probe. Die neue Oberfläche braucht getrennte Wahrheit für Broker, ausgewählte Probe, Pairing und die Frische des bestätigten States.

### P2 — Sprach- und Interaktionsfidelity

Die aktuelle UI verwendet deutsche Produkttexte, während der Designvertrag englische Produkttexte verlangt. Zusätzlich ist im geprüften Editor kein belastbarer Accessibility-/Keyboard-/Tooltip-Pfad für die grafische Kurve nachgewiesen. Das ist noch kein Beweis für eine vollständige Barrierefreiheitsverletzung, aber ein offener Validierungspunkt.

## 10. Empfohlene Reihenfolge

### Phase A — Vertrag bereinigen

- ältere Drei-App-/±3-dB-Aussagen markieren oder aktualisieren;
- `Gen Overview`, `Gen EQ Center` und `Probeeq fallback` als getrennte Oberflächen mit eindeutiger Owner-Sprache festhalten;
- für jeden Zustand die Begriffe `committed`, `proposed`, `auditioning`, `temporary`, `stale`, `rejected`, `bypassed` festlegen;
- 760×430 als konkrete Zielgröße und nicht nur als Figma-Maß behandeln.

### Phase B — Ein vertikaler EQ-Slice

Mit **einem Band auf einer lokalen Probeeq-Instanz** beginnen:

1. Band-Handle auswählen.
2. Frequenz/Gain ändern.
3. Kurve und Audio folgen ohne Verzögerung.
4. Bypass und Undo funktionieren.
5. State wird gespeichert und wiederhergestellt.
6. Host-Automation bewegt den Wert sichtbar und erzeugt keinen erfundenen Maus-Undo.

Erst wenn dieser Slice in echtem Audio und im Host funktioniert, Remote-Draft und Gen-Sprung anschließen.

### Phase C — Remote-Draft und Gen-EQ-Center

- eine Probe auswählen und direkt auf Seite 2 bearbeiten;
- Hold-to-Audition mit Recording/Unknown/Stop-Gegenpfaden;
- erster Klick = temporärer Apply-Lease, zweiter Klick = bestätigte Revision;
- Reject, Revert, stale durch Automation und Probe Offline;
- danach zwei Tracks im selben Graph und Master als eigenes Ziel.

### Phase D — Disclosure und Skalierung

- acht Bänder über persistenten Graph + kontextuelles Detail;
- Global standardmäßig collapsed, mit eindeutigem Active-Marker;
- Dynamics/Channel nur bei Capability und aktiver Relevanz;
- keine frei wandernden Popovers, keine Parameterwand als Default;
- 760×430, 600er Randtest, DPI-/Host-Resize und Keyboard-Fokus prüfen.

## 11. Host-/Audio-Validierungsplan

| Test | Erwartung | Aktueller Nachweis |
|---|---|---|
| 760×430 nominal | beide Gen-Seiten gleich groß, Kernobjekt lesbar | **Open**; aktueller Editor hat andere Ratio |
| 600×416 kleinster aktueller Editor | kein Überdecken, Targets erreichbar | **Observed** per EqCopShot; kein interaktiver Hosttest |
| no signal | keine falsche Kurve/Diagnose, Grund sichtbar | **Documented/Observed** in Codepfad |
| 15 s collecting → ready | Diagnose erst bei ausreichender Messung | **Tested** durch Analyse-/Shot-Pfad |
| Hold-Audition playback | hörbar nur bei gültigem Playback; Release beendet | **Tested** für aktuelle Markierung, **Open** für Draft |
| recording / unknown | Audition sichtbar blockiert | **Documented**, UI-Gegenpfad für neue EQ-Fläche offen |
| bypass → restore | Audiozustand und Anzeige stimmen überein | **Open** für EQ; Nulltest beweist nur aktuellen Passthrough |
| host automation | UI folgt externer Änderung, Draft wird stale | **Open**; Capability `remote_control` aktuell unsupported |
| Broker offline vs Probe offline | Fehlerquelle getrennt benannt | **Open** für Gen/Probeeq |
| read-only State | Änderungen gesperrt, Audio nicht überschrieben | **Tested**: State-Migration 132/132, read-only bytes preserved |
| duplicate identity | sichtbare Reparatur und Host-dirty | **Tested** im State-Migration-Test |
| offline render / transport stop | kein flüchtiger Hörzustand im Render | **Tested** für Markierung |
| 8 Bänder / 2 Tracks / Master | Ownership und Kurvenfarbe jederzeit eindeutig | **Open**; EQ-DSP/UI noch nicht vorhanden |

## 12. Evidence Ledger

| Aussage | Klasse | Beleg |
|---|---|---|
| Der aktuelle JUCE-Editor ist ein Analyzer-/Advisor-Editor | Observed | `PluginEditor.cpp`, EqCopShot-Render |
| Der Editor hält 750:520 und erlaubt 600×416 bis 1950×1352 | Observed | `PluginEditor.cpp` Konstruktor |
| Der aktuelle Editor hat keine EQ-Band-Controls | Observed | `PluginEditor.h/.cpp`: nur Mess-, Diagnose-, Vergleichs-, Bindungs- und Markierungszustand |
| Probeeq besitzt aktuell keinen Editor und keine Hostparameter | Documented + Observed | `SondeProcessor.h`, `SondeNullTestMain.cpp`, `CMakeLists.txt` |
| Der State-Vertrag hat 109 Parameter für active_probe | Documented | `nakama-parameter-v1.json`, `NakamaParameter.h` |
| Remote-Control ist noch nicht capability-bewiesen | Documented | `host-capabilities-fl-v1.json` |
| Gen ist Hub mit EQ Center auf Seite 2; zwei Tracks in einem Graph | User-decided + Documented | `2026-08-23-gen-eq-zentrale.md`, `ui-spielregeln-eq-zentrale.md` |
| 760×430 gilt für beide Gen-Seiten | User-decided + Documented | `2026-08-24-seite2-groesse-keine-toten-elemente.md` |
| AUTO standardmäßig aus und neben OUTPUT | User-decided + Documented | `2026-08-24-auto-gain-schalter.md` |
| Drei gelieferte Rework-Bilder sind die aktuelle visuelle Wahrheit in progress | User-decided + Documented | `2026-08-25-rework-referenz-drei-designs.md`, `design/LIES-MICH.md` |
| Die Übereinstimmung von Rework-Referenzen, Live-Host und EQ-Center-Interaktion ist bewiesen | Open | kein interaktiver FL-Hostlauf gegen die drei Referenzbilder in diesem Review |
| Ein Dialog für alle Befunde erhöht die Unterbrechung im täglichen EQ-Loop | Inferred | Placement-/Focus-Analyse; mit User-/Hosttest zu bestätigen |
| `Festhalten` koppelt Vergleich und Export | Observed | `PluginEditor.cpp`: `schreibeSnapshotDatei` und Vergleichslinie im selben Callback |
| Die aktuelle Markierung ist ein anderer Hörvertrag als Hold-to-Audition-Draft | Documented + Inferred | aktuelle Markierungsimplementierung vs. EQ-Center-Regeln |

## Schlussfolgerung

Der aktuelle Code ist an den Stellen stark, an denen er Wahrheit, Messgrenzen, Snapshot-Konsistenz und sichere Hör-Markierung behandelt. Er ist aber noch nicht der Plugin-EQ, den der aktuelle Nakama-Vertrag beschreibt.

Die nächste belastbare UX-Abnahme sollte deshalb nicht mit Farben oder weiteren Layoutvarianten beginnen, sondern mit einem nachweisbaren Pfad:

**eine Sonde → ein ausgewähltes Band → eine hörbare, reversible Änderung → Bypass/Undo → Host-/State-Rückweg.**

Wenn dieser Pfad stimmt, können Gen-Durchschalter, Draft-Lease, Zwei-Track-Graph und Master-EQ darauf aufbauen, ohne Ownership und Zustandswahrheit nachträglich neu zu erfinden.
