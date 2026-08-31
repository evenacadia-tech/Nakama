# Nakama Funktions- und Bedien-Blueprint

Stand: 31.08.2026

Status: lebende, designneutrale Produktspezifikation

Geltungsbereich: Nakama Gen, Nakama Probeeq und die dazugehörigen Betriebswege

Nächste technische Grenze: S16–17 / SONDE-011, danach P3 bis P9

## 0. Auftrag, Rang und Leseregel

Der User-Auftrag vom 31.08.2026 lautet, aus dem aktuellen Implementierungsplan
abzuleiten, welche technische Funktion entsteht und wie ein Mensch sie erreichen,
verstehen, bedienen, abbrechen und rückgängig machen kann. Frühere visuelle
Entwürfe sollen dafür ausdrücklich ignoriert werden. Dieser Blueprint übernimmt
deshalb weder Aufbau, Raster, Farben, Material, Icons noch Positionsentscheidungen
aus bisherigen Figma-Ständen, PNGs, Web-Prototypen oder Design-Abnahmen.

Bindender User-Wortlaut für die Quellenabgrenzung:
„ignoriere bisherige design die entworfen wurden.“

Der Blueprint ersetzt nicht:

- den technischen Bauplan in
  [FL-Nakama-Sonden-Design-Entwurf.md](../../docs/FL-Nakama-Sonden-Design-Entwurf.md),
- den gerechneten Status in [PLAN-STAND.md](../../docs/PLAN-STAND.md),
- die Verträge und Schemas unter [eq-copilot/schemas](../../eq-copilot/schemas),
- eine spätere visuelle Spezifikation mit Maßen, Typografie und Renderabnahme.

Er ergänzt diese Quellen um den fehlenden Vertrag zwischen Technik und Bedienung.
Wenn Technik und Oberfläche widersprechen, entscheidet der aktuelle technische
Vertrag über das Machbare; die Oberfläche darf den Widerspruch nicht kaschieren.

### Evidenzmarken

| Marke | Bedeutung |
|---|---|
| **[U]** | vom User entschieden |
| **[D]** | in aktuellem Plan, Vertrag, Code oder Herstellerhandbuch dokumentiert |
| **[B]** | in der laufenden Bestandsaufnahme beobachtet |
| **[A]** | aus Quellen abgeleitete Designhypothese; noch mit Menschen zu prüfen |
| **[O]** | offen; darf nicht durch Gestaltung still entschieden werden |

### Aktueller Ausgangspunkt

- **[D]** Der gerechnete Plan führt 20 von 38 Schritten als abgenommen, keinen
  Schritt nur als gebaut und S16–17 als nächsten offenen Schritt.
- **[B]** SONDE-011 Phase A ist gebaut und geprüft; die Phase-B-Matrix und
  paralleler uncommitteter Broker-Code sind kein Abnahmebeleg. Der Blueprint
  beschreibt sie daher nicht als fertige Produktfunktion.
- **[D]** Die neue v3-Leitung ist gebaut, aber noch nicht als produktiver Weg in
  beide Plugins verdrahtet. Probeeq ist derzeit noch ein audioprüfbarer,
  neutraler Shell; der aktive EQ entsteht erst in P6.
- **[D]** Der heutige Material-Kit-Editor ist ein Provisorium und keine
  Gestaltungsquelle für diesen Blueprint.

### Blueprint in sieben Sätzen

1. P2 baut Infrastruktur und keine neue Klangbedienung.
2. Gen Fläche 1 führt ab P3 durch Quellen, Messwahrheit, Evidenz, Intent und
   den regelbasierten nächsten Test.
3. P4 bleibt passiv/manuell; aktive Candidate-Hörmodi beginnen erst mit dem
   bewiesenen DSP und der Remote-Transaktion in P6/P7.
4. Gen Fläche 2 ist ab P7 der einzige vollständige EQ-Bedienort für Master
   und ausgewählte Probeeq; Probeeq bleibt kompakter Status-/Rückfallort.
5. Jeder Klangweg trennt Confirmed, Draft, Automation, Hold-Preview,
   10-s-Kandidat und tatsächlich bestätigtes Apply.
6. P8 ist eine gerichtete, capability-gated Sidechain-Beziehung mit lokalem
   DSP und ehrlichem statischem Fallback.
7. P9 macht zwei Produktbundles plus Broker installierbar, reparierbar,
   migrierbar, löschbar und rollbackfähig.

## 1. Menschliches Ziel und Fokus

### 1.1 Kausalkette

Ein Produzent will keine Telemetrie bedienen. Er will im laufenden Projekt:

> eine hörbare Unsicherheit einer Quelle zuordnen, ihre Ursache und Belegqualität
> verstehen, die kleinste sinnvolle Änderung sicher vorhören, fair vergleichen
> und nur einen nachweislich gewünschten Klangzustand bestätigen.

Daraus folgt die Produktkette:

1. **Orientieren:** Welche Quellen existieren, welche sind messbar und welche
   Information ist frisch?
2. **Lokalisieren:** Wo und in welcher Passage entsteht ein relevantes Problem?
3. **Verstehen:** Was ist beobachtet, was nur wahrscheinlich und was alternativ
   erklärbar?
4. **Hören:** Was genau würde sich ändern, lautheitsabgeglichen und ohne
   ungewollten Restzustand?
5. **Handeln:** Welche kleinste Änderung ist auf welchem Ziel vorbereitet?
6. **Bestätigen:** Ist der neue Zustand tatsächlich im DSP aktiv, gespeichert
   und weiterhin mit Hostautomation vereinbar?
7. **Erholen:** Wie komme ich bei Fehler, Disconnect, Ablauf oder Irrtum
   eindeutig zum letzten bestätigten Zustand zurück?

### 1.2 Fokus je Bedienfläche

| Fläche | Fokus im Ruhezustand | Fokus bei Auswahl |
|---|---|---|
| **Gen Fläche 1: Quellen und Evidenz** | Welche Quelle braucht Aufmerksamkeit, und kann ich dem Befund vertrauen? | Diese Quelle, Passage oder Beziehung mit Belegen, Alternativen und nächstem Test |
| **Gen Fläche 2: EQ und Transaktion** | Welches Ziel und welcher bestätigte Klangzustand sind gerade wirksam? | Das ausgewählte Band oder Proposal im Zusammenhang mit Gesamtklang, Schutz und Transaktionsstatus |
| **Probeeq kompakt** | Ist diese Instanz richtig benannt, verbunden und klanglich sicher? | Der lokale Fehler- oder Rückfallweg; niemals ein zweiter Voll-Editor |
| **Betriebsweg** | Ist Installation, Migration und lokaler Datenspeicher gesund? | Reparatur, Rollback, Export, Löschung oder Diagnose mit klarer Auswirkung |

**[A] Fokusregel:** Auf Gen Fläche 1 ist die primäre visuelle Arbeitseinheit die
Quelle beziehungsweise gerichtete Quellenbeziehung. Auf Gen Fläche 2 ist sie das
aktuell ausgewählte DSP-Ziel und darin das ausgewählte Band. Ein generisches
Dashboard ohne eindeutiges Objekt wäre keine ausreichende Umsetzung.

## 2. Sonic Object Map und Zustandsautorität

Jede Bedienung muss erkennen lassen, welches Objekt betroffen ist, wie weit die
Wirkung reicht und wer den bestätigten Zustand besitzt.

| Objekt | Menschliche Frage | Scope | Autoritative Wahrheit | Sichtbare Identität |
|---|---|---|---|---|
| Projekt | In welchem musikalischen Zusammenhang arbeite ich? | Projekt | persistenter Gen Main-State | Projektidentität, bestätigte Mitglieder, führendes Gen |
| Session/Verbindung | Welche Instanzen gehören jetzt zusammen? | Session-Epoche | transienter Broker-Sessiongraph | Generation, Join-, Liveness- und Konfliktstatus |
| Quelle/Probe | Welchen Bus höre oder untersuche ich? | eine Probeeq-Instanz | Pluginidentität plus bestätigte Mitgliedschaft | Hostname, sonst User-Label; nie Verbindungsreihenfolge als Ersatz |
| Messpunkt | Wo in der Kette wurde gemessen? | Instanz und Signalposition | Probe-Descriptor und Host-Capability | Position und Herkunft, nicht als vermuteter Routingsatz |
| Passage | Auf welchen Musikabschnitt gilt die Aussage? | Zeitfenster/Fingerprint | Gen Main-State | Name/Markierung, Dauer, Vergleichbarkeit |
| Messung | Was hat das Plugin jetzt beobachtet? | Quelle, Zeitfenster, DSP-Revision | Plugin-Snapshot | Wert, Einheit, Support, Alter, Coverage, Unsicherheit |
| Evidence/Finding/CauseHypothesis | Was bleibt als belegte Auswertung erhalten? | Quelle oder gerichtetes Paar, Passage | SQLite-Single-Writer und Projektion | Beleg-IDs, Aussageklasse, Alternativen, Konfidenz |
| SourceIntent | Welche Quelle soll führen, tragen oder geschützt bleiben? | Quelle, Passage, Beziehung | Gen Main-State | Funktion, Priorität, Schutz und Richtung |
| Proposal | Was wäre der kleinste belegte nächste Test? | Ziel, Passage, Baseline | persistenter SQLite-Store | Ziel, Grenzen, Hörziel, Stopbedingung, Rückweg |
| Draft | Welche unbestätigte Änderung bearbeite ich gerade? | Gen-Editor und Zielrevision | transienter Gen-Workflow | Basisrevision, bearbeitete Werte, stale/conflict |
| EQ-Ziel | Welcher Audiopfad wird verändert? | Gen-Master oder eine Probeeq | jeweilige Plugininstanz | Master oder eindeutiger Sondenname, niemals nur Kurvenfarbe |
| EQ-Band | Welcher Filter wird geformt? | ein fester Slot im gewählten Ziel | bestätigter DSP der Instanz | Band-ID, Typ, Frequenz, Gain, Q, Channelmode, Dynamic- und Aktivzustand |
| DSP-Revision | Was klingt nachweislich gerade? | eine Plugininstanz | jeweilige Plugininstanz | bestätigt, pending audio, automatisiert, bypassed, Konflikt |
| Hold-Preview | Was höre ich nur während des Haltens? | Ziel plus Lease | lokale monotone Lease im Zielplugin | Preview, kurze Lease, Baseline, sichere Rückkehr |
| 10-s-Kandidat | Was höre ich zeitlich begrenzt nach dem ersten Klick? | Ziel, Baseline und Kandidaten-ID | **[O] P7-Vertrag noch zu definieren** | Kandidat, Restzeit, Baseline, Reconnect-/Ablauflogik |
| Experiment | War der Kandidat unter gleichen Bedingungen besser? | Baseline, Kandidat, Passage | append-only Store/Projektion | Comparability, Match-Gain, Guardrails, Urteil |
| Unmask-Relation | Wer hat Vorrang, wer weicht aus? | gerichtetes Quellenpaar | Main-State für Relation, Zielplugin für Wirkung | priority source → yielding target, Grenzen, Sidechainstatus |
| Preset | Welcher Klanginhalt soll wiederverwendet werden? | DSP-Inhalt ohne Identität | versioniertes Presetobjekt | Name, Herkunft, Version; Ziel wird separat gewählt |

### 2.1 Drei gleichzeitig mögliche Wahrheiten

**[D]** Die EQ-Bedienung muss mindestens diese Zustände unterscheiden:

1. **Confirmed:** im Plugin bestätigter, wirksamer und speicherbarer DSP.
2. **Draft/Candidate:** in Gen vorbereitete oder zeitlich begrenzte Änderung.
3. **Automation:** vom Host vorgegebene, aktuell wirksame Überlagerung.

Kein Zahlenfeld, Bandgriff oder Gesamtcurve darf einen gemischten oder veralteten
Zustand als einen eindeutigen Confirmed-Wert ausgeben. Bei Abweichung zeigt die
Oberfläche Ursprung und wirksamen Wert; sie überschreibt Automation nicht still.

## 3. Fähigkeitsleiter aus dem Implementierungsplan

### 3.1 S16–17 / P2-Abschluss — Infrastruktur ohne Klangversprechen

**Technisch gebaut wird**

- Coordinator als Sessionowner, authentifizierter Sessiongraph und monotone
  Eviction;
- SQLite-Migration 1, Projektionen, append-only Events und persistente Outbox;
- Broker-Lifecycle: jede Instanz versucht zuerst nur zu verbinden; ausschließlich
  ein klassifiziertes Gen mit offenem Editor darf bei Bedarf starten, Probeeq
  nie. Der Broker beendet sich selbst nach 60 Sekunden ohne Clients;
- Schließen der v3-Vertragsriegel und Reservieren später benötigter Namen.

**Menschlich erreichbar sein muss**

- **[D]** Noch keine neue Klangbedienung.
- **[A]** Diagnose muss später zwischen broker unavailable, authenticating,
  joined, join confirmation required, incompatible und store degraded
  unterscheiden können. Solange diese Zustände nicht als Produktfläche gebaut
  sind, gehören sie in beweisbare Diagnoseausgaben, nicht als tote Buttons in
  die Audiooberfläche.
- **[D]** Eine mehrdeutige Gen-Führung darf nicht still geraten werden. Sie
  verlangt sichtbare Bestätigung, sobald P3 den Userpfad öffnet.

**Sicherer Rückweg**

Reconnect erzeugt eine neue Generation. Alte ACKs und alte Sessionzugehörigkeit
dürfen nicht als aktuelle Bestätigung erscheinen. P0-Steuerdaten dürfen nie
hinter verlusttoleranter Telemetrie verschwinden.

### 3.2 P3 / S18–19 — Quellen und passive Landkarte

**Technisch gebaut wird**

- Join/Führung, bestätigte Mitgliedschaft, Namen, Messposition und Frische;
- bestätigbare Buszugehörigkeiten einschließlich Parent/Child- und
  Doppelzählungswahrheit sowie sichtbare Main-Übergabe;
- Host-Kanalkontext, soweit FL ihn wirklich liefert, sonst User-Label;
- Zustände offline, stale, disconnected, suspended und unclassified;
- Lautheit je Quelle samt Unsicherheit;
- Rekonstruktion aus State-Reports, Main-State und Storeevents.

**Menschlicher Job**

1. Quellen im aktuellen Projekt erkennen.
2. Eine Quelle benennen, beitreten lassen, entfernen oder einen Konflikt lösen.
3. Buszugehörigkeit/Parent-Child-Beziehung bestätigen oder wieder aufheben.
4. Die Gen-Führung bewusst an ein anderes berechtigtes Gen übergeben.
5. Messpunkt, Signalaktivität, Alter und Unsicherheit verstehen.
6. Eine Quelle für Evidenzarbeit auswählen.
7. Diagnose öffnen, ohne die Quelle versehentlich zu verändern.

**Bedienvertrag**

- Jede Quelle hat einen einzigen, stabilen Auswahl- und Fokuspunkt.
- Hostname hat Vorrang; fehlt er, trägt das gespeicherte User-Label. Herkunft
  und Rückfall dürfen intern nachvollziehbar sein, aber der sichtbare Name darf
  nicht flackern oder aus der Verbindungsreihenfolge erfunden werden.
- Lautheit steht bei der Quelle, nicht in der EQ-Fläche. Wert, Einheit,
  Messstatus und Unsicherheit bilden gemeinsam ein Control-/Statusobjekt.
- Missing, no signal, measuring, fresh, partial, stale, disconnected,
  suspended und invalid sind verschiedene Zustände. Keiner wird als 0 oder
  no problem gerendert.
- **[D]** Die Übersicht verspricht keine EQ-Ampel je Sonde.

**Recovery**

Entfernen hebt das Binding sichtbar auf. Eine Main-Übergabe entzieht dem alten
Gen sichtbar die Schreibfähigkeit. Ein stale Client verschwindet erst nach
Tombstone/Eviction aus flüchtigen Indizes. Eine neue oder doppelte Instanz-ID
wird nicht automatisch mit einer alten Quelle verschmolzen; eine unbestätigte
Parent/Child-Zuordnung darf keine doppelt gezählte Contribution erzeugen.

### 3.3 P4 / S20–22 — Vergleichsevidenz und manueller Versuch

**Technisch gebaut wird**

- Dynamik, Headroom, Stereo, Korrelation, Ereignisse und PRE/POST-Vergleich;
- Passage, Fingerprint, Comparability und Alignmentqualität;
- lokaler Hörmarker für einen passiven Befund; noch kein
  Candidate-DSP-Vergleich;
- immutable Baseline/Kandidat, zunächst manual_external, mit Guardrails und
  blindem Userurteil.

**Menschlicher Job**

1. Eine Passage markieren oder auswählen.
2. PRE und POST beziehungsweise Baseline und Kandidat zuordnen.
3. Vergleichbarkeit und Gründe für Herabstufung prüfen.
4. Den lokalen Hörmarker aktivieren und eindeutig wieder verlassen.
5. Einen externen Versuch blind als besser, gleich, schlechter oder
   unentschieden bewerten.
6. Erst nach dem Urteil aufdecken, ob Baseline oder Kandidat gehört wurde.

**Bedienvertrag**

- Messung, Interpretation und Kausalbehauptung stehen nicht im selben
  Wahrheitsgrad.
- Vergleich beginnt in arming und wird erst bei gültigem Signal,
  Transportzustand und Lautheitsabgleich audible_ready.
- Match-Gain, zeitlicher Offset, Passage und Vergleichbarkeitsklasse bleiben
  während des Urteils erreichbar.
- Ein manueller externer Versuch erhält keinen falschen automatischen
  Undo-Button. Der Rückweg muss die vom User tatsächlich nötige externe
  Handlung benennen.
- A/B matched, Delta, Focus und Mono für einen aktiven Candidate gehören erst
  zu P7, nachdem P6 den lokalen DSP und P7 den Candidate-Tap beweisen.
- Stop, Seek, Drop, Epochenwechsel, Marker oder Preview invalidieren die
  betroffenen Intervalle sichtbar.

**Recovery**

Loslassen, Transportstopp, Aufnahme, Offline-Render, Editor-Schließen oder
Fehler kehren beim lokalen Hörmarker fail-closed zum Committed-Signal zurück.

### 3.4 P5 / S23–25 — Intent, Ursache, Proposal und Assistent

**Technisch gebaut wird**

- SourceIntent mit Funktion, Priorität, Schutz und gerichteter Beziehung;
- versionierter Evidenzgraph, Ursachenhypothesen, Alternativen,
  Ausschlussgründe und Konfidenz;
- kleinstes deterministisches Proposal, einschließlich keine Änderung und
  mehr Daten;
- persistenter AssistantStep mit Zurück, Überspringen, Abbruch und Resume;
- frequenzaufgelöster Datenweg für die Maskierungszone.

**Menschlicher Job**

1. Musikalische Absicht und Schutzbedarf festlegen.
2. Bei einer Beziehung Quelle und nachgebendes Ziel bewusst bestätigen.
3. Beobachtung, Alternativen und Konfidenz lesen.
4. Den kleinsten Gegentest starten oder eine Enthaltung akzeptieren.
5. Den Assistenten verlassen und später an derselben belegten Stelle
   fortsetzen.

**Bedienvertrag**

- **[U]** Es gibt keine KI-Erklärungsschicht. Sprache rendert ausschließlich
  validierte, regelbasiert erzeugte Fakten.
- Jede sichtbare Behauptung öffnet ihre Evidenz-IDs, Passage, Quelle,
  Qualitätsklasse und Ausschlussgründe.
- Ein Proposal zeigt immer Ziel, Hörziel, Grenzen, Stopbedingung und Rückweg.
- Änderungen an Passage, Intent, Evidenz oder Baseline machen abhängige
  Proposals sichtbar stale.
- Die Maskierungszone ist eine zweite Darstellung desselben Finding-Datenwegs,
  keine eigene Detektion und keine konkurrierende Schwelle.

### 3.5 P6 / S26–28 und S28b — lokaler EQ-Kern

**Technisch gebaut wird**

- acht feste EQ-Slots, Hard-Bypass, Input-/Output-Trim, Channelmode, Width,
  Mono-Bass und Mix;
- lokaler dynamischer Bandmodus mit dynamic_enabled, Range, Threshold,
  Attack, Hold und Release; priority_sidechain bleibt bis P8
  capability-gated;
- minimumphasiger Float-Kern ohne Lookahead und mit 0 gemeldeten Samples
  Latenz. Double wird nur bei deklarierter Capability und eigenen Goldens
  aktiv; FL meldet diese Capability derzeit unsupported;
- vorallokierte DSP-Bänke, State, Automation, Revision, Undo und Neutralize;
- lokale A/B-Matrix, Schutz-Zonen und Auto-Gain standardmäßig aus;
- eigenes versioniertes Presetobjekt;
- derselbe Kern mit eigenem State und Parameterlayout auf Gen-Master.

**Menschliche Erreichbarkeit**

- **[D]** P6 ist zuerst eine technische Zwischenstufe. Der vollständige
  Produktbedienweg entsteht erst in P7 auf Gen Fläche 2.
- **[U]** Probeeq bekommt keinen lokalen Voll-Editor.
- **[A]** Vor P7 darf nur ein Test-/Hostzugriff existieren, der keine
  endgültige Produktoberfläche vortäuscht. Die kompakte Probeeq-Fläche zeigt
  mindestens Identität, bestätigten EQ-/Bypasszustand und einen sicheren
  lokalen Rückfall.

**Bedienvertrag**

- **[U]** Linear Phase existiert nicht: kein deaktivierter Schalter, kein
  späterer Teaser und keine Namensreservierung.
- Auto-Gain ist standardmäßig aus. Aktivität und tatsächlich mitwandernder
  Ausgangswert sind gemeinsam sichtbar.
- Dynamikcontrols erscheinen am ausgewählten Band erst bei aktiviertem
  dynamischem Modus. Eine nicht verfügbare priority_sidechain wird vor P8
  weder als scheinbar bedienbare Quelle noch als toter Modus gezeigt.
- Presets tragen Klanginhalt, nie instance_id, Zieladresse, Busname oder Label.
- Ein unbekannter oder ungültiger neuerer State lädt neutral und erklärt den
  Zustand; er wird nicht halb angewandt.

### 3.6 P7 / S29–31 und S31b — zentrale EQ-Bedienung

**Technisch gebaut wird**

- sichtbares Pairing, Authentisierung, Lease, idempotentes Apply/Revert und
  zweistufiges ACK mit tatsächlich angewandten Werten;
- Hostparametersynchronisation und sichtbare Konflikte;
- Hold-to-hear-Preview, 10-Sekunden-Kandidat und bestätigte Revision;
- Active-A/B, Delta, Focus und Mono, Candidate-Tap und
  Wiederholungsmessung; lokales ausgerichtetes Delta ausschließlich bei
  aux_compare_pre;
- vollständige EQ-Bedienung in Gen für Master und ausgewählte Probeeq;
- Spectrum Grab, Freeze, reine Anzeigeeinstellungen, Maskierungszone,
  Presets und Kurve kopieren.

**Menschlicher Job**

1. Das Ziel eindeutig wählen: Gen-Master oder genau eine Probeeq.
2. Pairing/Steuerbarkeit und bestätigte Revision prüfen.
3. Ein Band direkt oder präzise formen, ohne den Zielkontext zu verlieren.
4. Proposal oder Draft vorhören, zeitlich testen, bestätigen oder ablehnen.
5. Preset laden/speichern oder Klanginhalt mit benannter Quelle und benanntem
   Ziel kopieren.
6. Automation, Schutzablehnung, zulässigen Werte-Clamp, Disconnect oder
   stale Baseline verstehen.

**Drei-Stufen-Geste [U] mit offenem P7-Vertragsrest [O]**

| Eingabe | Klangzustand | sichtbare Antwort | Ende/Rückweg |
|---|---|---|---|
| Halten | flüchtige Preview-Lease | Preview, Ziel, Baseline und betroffener Bereich | Loslassen, Fokusverlust, Timeout, Stop oder Fehler → Committed |
| erster Klick | 10-s-Kandidat | Candidate, Restzeit, Confirm und Reject | Ablauf/Reject/Fehler → Committed |
| zweiter Klick innerhalb der Zeit | persistentes Apply derselben Revision | pending audio → active → confirmed mit angewandten Werten | Revert referenziert exakt diese Revision |

Die drei Eingaben bleiben am selben Proposal verankert. Die Oberfläche darf
nicht so umsortieren, dass der zweite Klick ein anderes Objekt trifft.
Countdowns werden visuell aktualisiert, aber nicht sekündlich an Hilfstechnik
gesendet. Start, letzte Warnung, Bestätigung und Ablauf sind einzelne
verständliche Ereignisse.

Der aktuelle v3-Vertrag bildet diesen Zwischenzustand noch nicht vollständig
ab: preview_begin erlaubt derzeit höchstens 400 ms Hold-Lease,
apply_transaction ist persistentes Apply. Vor UI-Bau muss P7 deshalb
Kandidaten-ID, 10-s-Timer,
Message-Familie, ACK, Ablauf, Reconnect, Prozesskill und Verhältnis zur
Basisrevision definieren. Der erste Klick darf nicht still als permanentes
apply_transaction implementiert werden.

**Remote-Wahrheit**

- Full manual range ist verfügbar; das frühere spezielle ±3-dB-Remote-Limit
  existiert nicht.
- Schutz-Zonen bleiben harte Bediensperren: eine Kollision wird abgelehnt und
  weder vorgehört noch angewandt. Nur Werte innerhalb eines grundsätzlich
  erlaubten Bereichs dürfen technisch geklammert werden; dann bestätigt das
  ACK die tatsächlich angewandten Werte und nennt den Grund.
- accepted_pending_audio ist nicht active. Erst die Audiobestätigung darf als
  wirksamer Klang erscheinen.
- Brokerverlust verändert einen bestätigten DSP nicht. Eine unbestätigte
  Preview darf niemals überleben.

### 3.7 P8 / S32–33 — gerichtete dynamische Entmaskierung

**Technisch gebaut wird**

- ein azyklischer Beziehungsgraph mit höchstens einer eingehenden,
  typisierten Relation priority source → yielding target je Ziel;
- echte lokale Sidechain im nachgebenden Probeeq;
- ungefähr 32 Detektorbänder als Worker-Fit zu höchstens drei breiten,
  festen Reduktionsbändern;
- Energie-Gates, Hysterese, Attack/Hold/Release und Smoothing;
- begrenzte Reduktion, Telemetrie, Preview, Apply und Wiederholungsmessung.
- kein automatischer Boost.

**Menschlicher Job**

1. Vorrangquelle und nachgebendes Ziel ausdrücklich prüfen.
2. Capability, Routing, Kanalzahl und PDC-Status verstehen.
3. Frequenzbereich, Stärke und Zeitverhalten innerhalb der Schutzgrenzen
   einstellen.
4. Nur die entfernte Wirkung hören, den Gesamtklang vergleichen und
   bestätigen oder verwerfen.

**Bedienvertrag**

- Richtung wird immer als benannte Beziehung gezeigt, nicht nur durch zwei
  Farben oder Kurven.
- Das P8-Limit bleibt von der entfernten manuellen EQ-Reichweite getrennt:
  Defaultmaximum 1,5 dB, harte Obergrenze 3 dB, sofern der technische Plan
  nicht ausdrücklich geändert wird.
- Sidechainverlust, falsche Kanalzahl oder unbekannte PDC fahren Reduction
  kontrolliert auf 0 und zeigen degraded/unsupported.
- Eine bestätigte Relation läuft bei Brokerverlust lokal weiter; eine
  unbestätigte Preview endet. Reconnect darf keine zweite Relation erzeugen.
- Telemetrie steuert nie Audio.
- Fehlt die Capability, bleibt als ehrlicher Fallback Kollisionshinweis plus
  statischer/manueller Vorschlag. Es erscheint kein funktionsloser
  Unmask-Schalter.

### 3.8 P9 / S34–35 — Auslieferung, Reparatur und Datenschutz

**Technisch gebaut wird**

- Installer/Updater für die zwei abgenommenen Produktbundles Gen und Probeeq
  plus Broker, Signatur, atomarer Austausch, Repair, Uninstall und Rollback;
- Migration aller veröffentlichten Plugin-, IPC- und DB-Schemata;
- Host-, Validator-, Realtime-, Fuzz-, Kill- und Soak-Matrix;
- Export, Retention, Delete und redigiertes Diagnosepaket.
- Altprojekte laden mindestens passiv; bestätigter DSP rendert ohne Broker;
  keine Queue, Registry- oder WAL-Datei wächst unbegrenzt.
- P8 ist entweder enthalten oder durch eine sichtbare Produktentscheidung
  aus dem R4-Versprechen genommen.

**Menschlicher Job**

- Installation oder Update mit sichtbarer Version und Signatur ausführen;
- fehlerhaften Zustand reparieren oder auf das vorige signierte Paket
  zurückrollen;
- lokale Verlaufsdaten exportieren oder löschen, ohne Projekt-/DSP-State zu
  verwechseln;
- ein Diagnosepaket vor Erzeugung und Weitergabe in seinem Umfang verstehen.

**Stale Textstelle, keine offene Produktzahl [D]**

Ein älterer P9-Text nennt noch drei Bundles. S9b hat zwei Produktbundles plus
Broker bereits abgenommen und in Installer/Code umgesetzt. Vor P9 wird nur
diese stale Textstelle korrigiert; es gibt keine offene Entscheidung über ein
drittes Produkt und keine dritte tägliche Audiofläche.

## 4. Informationsarchitektur ohne Alt-Design

Die folgende Struktur ist eine **[A] Layout-Hypothese**, kein Pixelentwurf.
Sie folgt ausschließlich Objektbesitz, Häufigkeit und Zustandswahrheit.

### 4.1 Gen Fläche 1 — Quellen und Evidenz

Die Fläche braucht vier funktionale Zonen:

1. **Kontext und Systemwahrheit:** Projekt/Session, führendes Gen und
   degradierte Systemzustände.
2. **Quellen-Locator:** alle relevanten Quellen mit Name, Messpunkt, Frische,
   Signalstatus und Lautheit/Unsicherheit; Suche oder Filter bei hoher Zahl.
3. **Auswahl-/Evidenzraum:** ausgewählte Quelle, Passage oder gerichtetes Paar
   mit Finding, Belegen, Alternativen und Messqualität.
4. **Nächster Schritt:** AssistantStep beziehungsweise Proposal mit
   Vorbedingung, Test, Abbruch, Zurück und Resume.

Ruhezustand: Quellenorientierung.

Auswahlzustand: eine Quelle oder Beziehung.

Fehlerzustand: betroffene Quelle bleibt an derselben Stelle und erklärt ihren
Status; sie verschwindet nicht hinter einem globalen Toast.

Bei 16 sichtbaren und 32 angeschlossenen Probes muss die Fläche Überblick,
Suche/Quick-Jump und eine stabile Auswahl erlauben. Ob dies Liste, Matrix oder
semantischer Zoom wird, ist **[O]** und wird erst mit frischen Wireframes
entschieden.

### 4.2 Gen Fläche 2 — EQ und Transaktion

Die Fläche braucht fünf funktionale Zonen:

1. **Persistenter Zielkontext:** Master oder benannte Probeeq, Pairing,
   Capability und bestätigte Revision.
2. **Dominante akustische Arbeitsfläche:** Spektrum, Gesamtcurve, Bänder,
   zweite Vergleichscurve und belegte Maskierungszone.
3. **Objektpräzision:** exakte Werte und Aktionen nur für die aktuelle
   Bandauswahl beziehungsweise Relation.
4. **Globaler Klangzustand:** Bypass, Mix, Auto-Gain/Output, A/B,
   Preset/Copy und Revision/History.
5. **Transaktionslocus:** Hold, 10-s-Kandidat, Confirm, Reject/Revert und
   Blocking-Grund.

**[A] Disclosure-Regel:** Der akustische Arbeitsraum dominiert im Ruhezustand.
Bandpräzision erscheint bei Auswahl, bleibt räumlich beim Bandkontext und
verschwindet nach Deselect. Globale Zustände, aktiver Draft, Freeze,
Automation und Preview dürfen dagegen nie spurlos eingeklappt sein.

### 4.3 Probeeq kompakt

Immer erreichbar:

- Instanzidentität und sichtbarer Name;
- Messpunkt/Signalstatus;
- Verbindung, Pairing und Capability;
- bestätigter EQ-/Bypasszustand;
- klarer Hinweis, ob Automation oder Remote-Transaktion wirkt;
- sicherer lokaler Bypass beziehungsweise Neutralize-Rückfall.

Nicht enthalten:

- vollständige Bandbearbeitung;
- lokale Proposal-/Advisor-Arbeit;
- zweite abweichende Preset- oder Copy-Logik;
- eine alternative Fernsteuerungswahrheit.

Welche zusätzlichen lokalen Notfallaktionen neben Bypass/Neutralize nötig
sind, bleibt **[O]** und wird aus P6/P7-Recoverytests entschieden.

### 4.4 Betriebswege

Installation, Repair, Rollback, Datenschutz und Diagnose gehören nicht in die
tägliche EQ-Fläche. Sie dürfen als Installer- oder Diagnoseroute getrennt
bleiben, müssen aber aus einem Fehlerzustand mit einer konkreten Anweisung
erreichbar sein.

## 5. Sichtbarkeitsmatrix

| Information/Aktion | Immer sichtbar | Bei Objektfokus | Auf Abruf | Nie/Später |
|---|---:|---:|---:|---:|
| ausgewähltes Ziel und Eigentümer | ✓ |  |  |  |
| Confirmed/Draft/Automation/Preview | ✓, wenn relevant |  | Details |  |
| Quelle, Frische, Missing/Invalid | ✓ an der Quelle |  | Provenienz |  |
| Lautheit plus Unsicherheit | ✓ auf Fläche 1 |  | Supportdetails |  |
| genaue Bandwerte |  | ✓ | numerischer Editor |  |
| Schutzgrenzen/Clamp | Marker bei Relevanz | ✓ | genaue Regel |  |
| Maskierungszone | bei passendem Finding | ✓ | Evidenzgraph |  |
| Analyzer Freeze | aktiver Marker |  | Einstellung |  |
| Range/Resolution/Speed/Tilt | aktiver Marker bei Abweichung |  | ✓ |  |
| Passage/Comparability | beim Vergleich | ✓ | Details |  |
| AssistantStep | aktueller Schritt | ✓ | Verlauf/Resume |  |
| Pairing/Securitydetail | Status | bei Fehler | ✓ |  |
| Undo/Revert/Revision | verfügbare Recovery | ✓ | Verlauf |  |
| Unmask-Controls | nur bei Capability/Relation | ✓ | technische Details | sonst kein Control |
| Linear Phase |  |  |  | dauerhaft nie |
| Klaviatur/Tonnamen |  |  |  | dauerhaft nie |
| EQ Match |  |  | reservierter Ausbauhinweis nur in Doku | nach R4 |

## 6. Platzierungsbeweise

| Element | Muss in der Nähe von | Warum | Gegenbeweis |
|---|---|---|---|
| Zielumschalter | Zielname, Revision und Capability | Wer wird verändert, muss vor der Kurvenaktion klar sein | verdecktes Menü, das nach Auswahl keinen Besitzer zeigt |
| Quellenstatus | Quellenname | Freshness ohne Besitzer ist wertlos | globale Ampel ohne betroffene Quelle |
| Lautheit/Unsicherheit | derselbe Quellenwert | Präzision ohne Vertrauensgrenze erzeugt Scheinsicherheit | Unsicherheit nur in Diagnoseansicht |
| Bandpräzision | ausgewähltes Band/Curve | direkte und numerische Bedienung müssen dasselbe Objekt besitzen | permanenter Inspector mit unklarem Fokus |
| Transaktionsaktion | Proposal/Draft und Ziel | Hören, Kandidat und Apply müssen dieselbe Revision meinen | primärer Button wechselt Position oder Ziel |
| Revert/History | Confirmed-Revision | Rückweg braucht die referenzierte Klangwahrheit | generisches Undo ohne Scope |
| Maskierungszone | zwei benannte Quellen und Finding | die Zone ist Belegdarstellung, kein Dekor | farbige Fläche ohne Richtung/Quelle |
| Analyzer-Einstellung | sichtbares Spektrum | sie verändert Darstellung, nicht Befund | Einstellung im Diagnose-/Advisorbereich |
| Preset/Copy | Zielkontext | Klanginhalt darf Identität nicht mitkopieren | Copy ohne sichtbare Quelle und Ziel |
| Unmask-Relation | priority source → yielding target | Wirkung ist gerichtet | symmetrisches Link-Symbol ohne Leserichtung |

## 7. Zugangs- und Antwortvertrag

Jede primäre Handlung braucht einen direkten Weg, Tastaturweg, Präzisionsweg,
Abbruch/Rückweg und native JUCE-Semantik.

### 7.1 Globale Regeln

- Alle eigenen Controls liefern Accessibility-Name, Rolle, Wert, Zustand und
  verfügbare Aktionen.
- Farbe ist nie der einzige Träger für Quelle, Curve, Warning, Auswahl oder
  Aktivität.
- Fokus folgt dem häufigen Workflow: Ziel/Quelle → Objekt → Wert → Aktion →
  Bestätigung/Recovery. Diagnose- und Hilfsaktionen unterbrechen diesen Pfad
  nicht.
- Grob-, Fein-, Default- und Grenzschritt kommen aus Parametervertrag und
  Hörtest, nicht aus einem UI-Framework-Default.
- Ungültige Textwerte werden vor der Anwendung abgewiesen. Die Oberfläche
  erklärt gültigen Bereich und Einheit.
- Direkte Manipulation, Tastatur und Texteingabe schreiben denselben
  kanonischen Zustand. Griff, Zahl, Curve und hörbare Wirkung dürfen nicht
  auseinanderlaufen.
- Meter werden für Hilfstechnik entprellt und auf Abruf zusammengefasst.
  Kritische Zustände wie Disconnect oder sicherer Abbruch haben Vorrang.
- Escape beendet den aktuellen flüchtigen Modus. Nach Dialog, Deselect,
  Apply, Reject oder Fehler kehrt der Fokus zum auslösenden Objekt zurück.

### 7.2 Primäre Handlungen

| Handlung | Pointer | Tastatur/Präzision | Abbruch/Reset | Semantik und Antwort |
|---|---|---|---|---|
| Quelle wählen | Klick auf Quellenobjekt | Pfeile/Quick-Jump, Enter wählt | Escape zur vorigen Auswahl | Name, Messpunkt, Frische und Signalstatus |
| Quelle benennen | Rename-Aktion | Textfeld, Enter speichert | Escape verwirft | Textfeld mit Herkunft Host/User; Länge validieren |
| Passage wählen/markieren | Zeitbereich/Mark action | Start/Ende numerisch oder transportbezogene Aktion | Cancel entfernt Draft | Bereich, Fingerprint und Comparability |
| Intent setzen | direkte Rollen-/Prioritätswahl | Radiogruppe/Combobox, Richtung separat | Reset auf ungesetzt | Funktion, Schutz, scope und Konflikte |
| Evidenz öffnen | Finding aktivieren | Enter öffnet Details | Escape zurück | Aussageklasse, Beleg-IDs, Alternativen, Konfidenz |
| Band auswählen | Node/Curve | Bandliste oder nächstes/vorheriges Band | Deselect | Band-ID, Typ und aktiver Zustand |
| Band anlegen | Klick/Drag oder Spectrum Grab | Add-Band-Aktion plus Frequenzfeld | Escape verwirft Draft | Ziel, Frequenz, Gain, Q, Slotverfügbarkeit |
| Band formen | Drag und modifizierte Feingeste | Pfeile grob/fein, Textwerte | Default/Undo | Wert, Einheit, Grenzen, Clamp und Automation |
| Banddynamik | Dynamic-Toggle am ausgewählten Band | Toggle plus Range/Threshold/Attack/Hold/Release-Felder | Disable/Reset | Aktivmarker und Gain-Bewegung; priority_sidechain erst bei Capability |
| Schutzbereich setzen | Range-Handles | zwei numerische Endpunkte | Reset/Cancel | Lower/Upper, Gültigkeit und betroffene Aktion |
| Preview halten | Press-and-hold | Key-down/up auf fokussierter Aktion | Release/Fokusverlust/Timeout | Momentary action, Lease und Ziel |
| 10-s-Kandidat starten | Klick | Enter/Space activation | Reject/Escape/Expiry | Candidate state, Restzeit, Baseline |
| Kandidat bestätigen | zweiter Klick am stabilen Locus | Enter auf Confirm | Revert danach | pending audio, active, confirmed/applied values |
| Freeze | Toggle/Hold | Space toggles; separate temporary action | Off/Release | Toggle state; Frozen ist dauerhaft markiert |
| Analyzer einstellen | Panel/Popover | Tab/Arrows/Textwerte | Reset display defaults | ausschließlich Display state |
| Preset laden/sichern | Browser/Liste | Suche, Pfeile, Enter | Cancel; Undo nach Load | Presetname/Version; Ziel separat |
| Curve kopieren | Copy-Aktion | Quelle und Ziel als zwei Felder, Confirm | Cancel/Undo | Zusammenfassung: source → target; Identität bleibt |
| Unmask konfigurieren | Relation/Range/Controls | gerichtete Auswahl, numerische Werte | Disable/Revert | Capability, Richtung, Limit, Reduction, Fallback |

### 7.3 Offene Accessibility-Prüfung der Hold-Geste

**[O]** Native JUCE- und Screenreader-Tests müssen klären, wie eine echte
Momentary-Aktion für Nutzer funktioniert, deren Hilfstechnik nur eine
Activation und kein verlässliches Key-down/Key-up liefert. Ein separater
zugänglicher Start/Stop-Preview-Pfad ist zulässig, sofern:

- dieselbe lokale Lease und dieselben Stop-Gates gelten,
- die Preview eine kurze harte Maximaldauer besitzt,
- Start und Stop eindeutig benannt sind,
- kein sticky Hörzustand über Fokus-, Fenster- oder Verbindungsverlust
  überleben kann.

## 8. Marktvergleich und konkrete Transferregeln

Die Auswahl bildet etablierte Funktionsbenchmarks ab; sie ist keine
Marktanteilsrangliste. Verglichen werden dokumentierte Mechanismen, nicht
visueller Stil.

| Produkt | geprüfter Stand am 31.08.2026 |
|---|---|
| [FabFilter Pro-Q 4](https://www.fabfilter.com/download) | 4.13, 25.06.2026 |
| [sonible smart:EQ 4](https://www.sonible.com/smarteq4/) | 1.1.1; Wartungsdatum nicht ausgewiesen |
| [iZotope Neutron 5](https://www.izotope.com/pages/release-notes/neutron) | 5.2.0, 04.02.2026 |
| [Voxengo SPAN Plus](https://www.voxengo.com/downloads/) | 1.25, 27.09.2025 |
| [ADPTR Metric AB](https://www.plugin-alliance.com/products/metric-ab) | 1.5.0, 29.07.2026 |

### 8.1 FabFilter Pro-Q 4

**Dokumentiert**

- Die Instance List bündelt Plugininstanzen, ordnet sie soweit möglich nach
  dem Host, bietet Suche, Quick Jump, Pinning, Minimap und mehrere
  Detailstufen.
- Höhere Detailstufen erlauben direkte Kurvenbearbeitung; der Zielname bleibt
  sichtbar.
- Freeze hält Spektralspitzen, Spectrum Grab macht Peaks temporär zu
  Bedienzielen, und Range/Resolution/Speed/Tilt sind Analyzeroptionen.
- Freeze stoppt das Fallen und akkumuliert ein Maximum; der dokumentierte
  Active-Marker gehört zu Freeze, während Spectrum Grab ein eigener
  temporärer Modus mit Exit ist.
- Collision ist ausdrücklich nur ein Hinweis; das Ohr entscheidet.
- Band Solo ist eine Hold-Aktion und visualisiert den gehörten Bereich.
- EQ Match wird erst nach gültiger Analyse aktiv und führt von Referenz über
  Vorschlagskurve zu Finish oder zurück zu Analyze.
- In der Instance List identifiziert Emphasize eine Spur momentweise hörbar.
  FabFilter dokumentiert außerdem für FL, dass CLAP Trackname/-position nicht
  liefert und latenzbehaftete Instanzen die Reihenfolge stören können.

**Transfer zu Nakama [A]**

- Hostname plus ehrlicher Fallback, Suche/Quick-Jump und progressive
  Detailstufen für viele Quellen.
- Akustische Fläche im Ruhezustand, objektnaher Präzisionsweg bei Auswahl.
- Freeze/Spectrum Grab mit sichtbarem temporärem Modus und persistentem
  Active-Marker.
- Collision/Masking bleibt Vorschlag mit Quellenbezug und Unsicherheit.
- Hold-Audition koppelt visuellen und hörbaren Fokus.
- Der spätere EQ-Match-Ausbau muss bei fehlendem/ungültigem Material gesperrt
  bleiben und vor Finish eine editierbare Vorschlagsstufe zeigen.

**Nicht transferieren**

- keine Farben, Panelgeometrie oder Pianoanzeige;
- keine Pro-Q-Modi, die Linear Phase oder nicht geplante DSP-Typen verlangen;
- keine Hostsortierung vortäuschen, wenn FL Position oder Name nicht liefert.

Quellen: [Instance List](https://www.fabfilter.com/help/pro-q/using/instance-list),
[Spectrum analyzer](https://www.fabfilter.com/help/pro-q/using/analyzer),
[Solo](https://prod.fabfilter.com/help/pro-q/using/solo),
[EQ Match](https://prod.fabfilter.com/help/pro-q/using/eqmatch).

### 8.2 sonible smart:EQ 4

**Dokumentiert**

- Group View ordnet bis zu zehn Instanzen per Drag-and-drop in Front, Middle
  und Back.
- Instanzen lassen sich aus der Gruppenansicht fernbedienen. Group on/off
  schaltet nur den Cross-Channel-Anteil; Bypass all umgeht dagegen die gesamte
  Verarbeitung aller Gruppeninstanzen.
- Track, Group und Track+Group trennen lokalen Ausgleich von
  Cross-Channel-Unmasking.
- Eine noch nicht gelernte Gruppeninstanz beeinflusst die Gruppe nicht.
  Hinzufügen oder Umordnen gelernter Mitglieder aktualisiert deren
  smart:filter unmittelbar.

**Transfer zu Nakama [A]**

- smart:EQ belegt, dass eine sichtbare Prioritätshierarchie als Musikobjekt
  bedienbar ist. Die gerichtete Relation priority source → yielding target
  stammt dagegen aus Nakamas SourceIntent-Vertrag.
- Quellenidentität, Gruppenzugehörigkeit und Scope der Wirkung müssen beim
  Remote-Zugriff erhalten bleiben.
- Eine Gruppen-/Beziehungswirkung braucht einen gemeinsamen Bypass- oder
  Vergleichsweg.
- Sandboxing, gemischte Pluginformate und inkonsistent restaurierter
  IPC-Zustand belegen, dass Gruppenkommunikation Capability-, Restore- und
  Repairzustände braucht.

**Nicht transferieren**

- keine automatische Neuverarbeitung aller Quellen allein durch Drag-and-drop;
- keine AI-Profile oder stilles Lernen;
- keine drei pauschalen Ebenen als Ersatz für Nakamas passagengebundenen
  SourceIntent, Schutz und explizite Apply-Transaktion;
- keine gelernte Gruppenanalyse als Ersatz für P8: sample-synchrones
  Echtzeit-Unmasking braucht Nakamas echten lokalen Sidechain.

Quellen: [smart:EQ 4 Manual](https://www.sonible.com/wp-content/uploads/2024/01/manual-smEQ4_EN.pdf),
[drei smart:filter-Modi](https://help.sonible.com/hc/en-us/articles/11846425347996-What-are-the-three-different-modes-of-the-smart-filter-doing),
[Produktübersicht](https://www.sonible.com/smarteq4/).

### 8.3 iZotope Neutron 5

**Dokumentiert**

- Der Masking Meter wählt eine zweite IPC-Instanz als Masker, zeigt
  momentane Maskierung und einen Verlauf; das Handbuch bezeichnet das
  Ergebnis als Hinweis, nicht als Pflicht zur Korrektur.
- Remote-EQ-Controls erscheinen dort nur, wenn die Vergleichsinstanz selbst
  einen kompatiblen Neutron-EQ besitzt; andere IPC-Quellen liefern nur Meter
  und Histogramm.
- Masking Sensitivity verändert nur die Anzeige-/Detektionsschwelle, nicht
  die Signalverarbeitung.
- Das getrennte Unmask-Modul verlangt einen echten Sidechain, senkt über 32
  Bänder nur während erkannter Maskierung dynamisch ab, zeigt beide Spektren
  und einen Difference Meter und bietet Range, Amount, Sensitivity, Attack
  und Release.
- Delta macht die vom Modul entfernte Wirkung hörbar.
- Das Handbuch warnt, dass Remote-EQ und Automation unvorhersehbar
  zusammenspielen können.
- Der getrennte Visual Mixer ist ein IPC-Controller für Level, Pan und Width
  anderer Instanzen und verarbeitet selbst kein Audio. Eine
  Mehrinstanzübersicht beweist daher nicht automatisch Remote-EQ.

**Transfer zu Nakama [A]**

- Das gerichtete Paar Quelle/Masker beziehungsweise priority/yielding bleibt
  im Graph und in den Controls benannt.
- Momentane Überdeckung und kumulative Passagehistorie sind getrennte
  Beobachtungen. Zu Nakama-Evidence werden sie erst mit Quelle, Zeitfenster,
  Support, Revision und Unsicherheit.
- Display-/Sensitivity-Regler sagen ausdrücklich, ob sie nur Darstellung/
  Detektion oder wirklich DSP verändern.
- P8 braucht Difference/Delta, Range und echte Sidechain-Fehlerzustände.
- Automationskonflikt wird als eigener Zustand behandelt, nicht als
  Nebenbemerkung.

**Nicht transferieren**

- keine direkte Remote-Änderung ohne Nakamas Lease, Revision, Schutz und
  Zwei-Klick-Bestätigung;
- kein automatisches Unmasking, solange Capability, Routing oder PDC
  unbewiesen sind;
- keine Farbzone ohne Finding, Zeitbasis und Quellenbezug.

Quellen: [Masking Meter](https://docs.izotope.com/neutron5/en/masking-meter.html),
[Unmask](https://docs.izotope.com/neutron5/en/unmask.html),
[Neutron 5](https://www.izotope.com/products/neutron?tab=features).

### 8.4 Voxengo SPAN Plus

**Dokumentiert**

- SPAN Plus bietet Multi-Track-Spektrumvergleich, statische Spektren,
  verschiedene Maxima, FFT-Größe, Overlap, Slope und Smoothing.
- Dazu kommen EBU-R128-Lautheit, True Peak, Headroom, Korrelation,
  Presetmanager, Undo/Redo und A/B für Pluginzustände. Dieses A/B ist kein
  Umschalten zweier Audiostreams.
- Multi-Track bedeutet Spektrumimport/-export; SPAN Plus ist kein Remote-EQ
  und greift abseits seiner Hör-/Solo-Hilfen nicht in den fremden Klang ein.

**Transfer zu Nakama [A]**

- Analyzerdarstellung darf konfigurierbar sein, ohne Evidenzraster oder
  Findings zu verändern.
- Mehrere Kurven brauchen zusätzlich zu Farbe eine benannte Quelle und
  eindeutige Sichtbarkeitssteuerung.
- Lautheit, Peak, Headroom und Korrelation bleiben fachlich getrennte
  Messobjekte mit eigener Zeitbasis.

**Nicht transferieren**

- kein frei konfigurierbares Analysemodell, das Nakamas reproduzierbare
  Evidenz still ändert;
- kein Farbsystem als alleinige Quellenidentität.

Quellen: [SPAN Plus](https://www.voxengo.com/product/spanplus/),
[SPAN Plus User Guide](https://www.voxengo.com/files/userguides/VoxengoSPANPlus_en.pdf/getbyname/Voxengo%20SPAN%20Plus%20User%20Guide%20en.pdf).

### 8.5 ADPTR Metric AB

**Dokumentiert**

- A/B ist direkt und per Keyboard erreichbar.
- Lautheitsabgleich, Latch-/Cue-/Sync-/Manual-Playback und mehrere
  Analyseansichten dienen dem fairen Vergleich.
- Single, Dual und Layered unterscheiden, ob A, B oder beide sichtbar sind.
- Fehlendes Audio blockiert Loudness Match statt einen falschen Erfolg
  auszugeben.

**Transfer zu Nakama [A]**

- Vergleich ist eine eigene Aufgabe mit stabiler A/B-Aktion, benannter
  Passage und sichtbarem Match-Gain.
- Vergleichsansicht unterscheidet Baseline, Kandidat und Overlay, ohne ihre
  Identität zu vermischen.
- Ungültige Vergleichsvoraussetzungen sperren das Urteil und nennen den
  konkreten Grund.

**Nicht transferieren**

- keine allgemeine Referenzbibliothek im Kernumfang;
- keine externen Referenzdateien als stiller Ersatz für Nakamas
  passagengebundene Baseline;
- kein EQ Match vor dem beschlossenen Roadmap-Ausbau.

Quellen: [Metric AB](https://www.plugin-alliance.com/products/metric-ab),
[Metric AB Manual](https://files.plugin-alliance.com/products/adptr_metricab/adptr_metricab_manual.pdf).

## 9. Was daraus konkret für Nakama folgt

### 9.1 Übernommene Muster

1. **Zielidentität bleibt stehen.** Remote- und Mehrinstanzarbeit darf den
   Besitzer einer Curve nie nur durch Farbe vermitteln.
2. **Overview → Fokus → Präzision.** Viele Quellen beginnen kompakt; Auswahl
   bringt Evidenz, höhere Präzision oder direkte Bearbeitung.
3. **Objektlokale Komplexität.** Band- und Relationsdetails erscheinen erst,
   wenn das Objekt existiert und fokussiert ist.
4. **Collapsed-but-active.** Freeze, Draft, Automation, Bypass, Pairing und
   Analyzerabweichung behalten auch bei eingeklappten Details einen Marker.
5. **Hören vor Bestätigen.** Momentary Audition, 10-s-Kandidat und bestätigte
   Revision sind unterscheidbare Stufen.
6. **Vergleichsvoraussetzungen sind sichtbar.** Passage, Signal, Match-Gain,
   Alignment und Comparability müssen gültig sein.
7. **Maskierung bleibt Kontext, nicht Befehl.** Overlay/Histogramm unterstützt
   eine Entscheidung und verweist auf Quelle, Zeitbasis und Alternativen.
8. **Nakama trennt Display und kanonische Evidenz.** Freeze, Range,
   Resolution, Speed und Tilt dürfen Nakamas Evidence-/Finding-Pfad nicht
   verändern. Wenn sichtbare Präzision, Updaterate oder Releaseverhalten
   wechseln, ist das ausdrücklich nur Displayverhalten.
9. **Remote plus Automation ist ein Konfliktzustand.** Nakama löst ihn über
   Herkunft, Revision und ACK statt über Hoffnung.
10. **Recovery hat einen stabilen Ort.** Revert, History und Bypass werden
    nicht erst nach Fehler gesucht.
11. **A/B-Typ und Scope werden benannt.** Pluginzustände, Mix-Snapshots,
    Baseline/Kandidat und Live-Eingang/Referenzdatei sind nicht
    austauschbar. Nakamas A/B nennt Writer, Ziel, Passage und Matchzustand.
12. **Undo verspricht nur seinen Writer-Scope.** Hostautomation und
    Remote-Ziele werden nicht durch eine lokale History behauptet, sondern
    über autoritatives State-Read und revisioniertes Revert behandelt.

### 9.2 Bewusst nicht übernommene Muster

- AI-generierte Klangziele oder Erklärungen;
- automatische Gruppenänderung beim bloßen Umordnen;
- Linear Phase, Piano-/Notenanzeige oder nicht geplante DSP-Modi;
- Apply direkt aus einer Analyseanzeige;
- Farbcodierung als alleinige Objekt- oder Fehleridentität;
- ein lokaler Probeeq-Voll-Editor;
- eine unverbindliche Collision-Visualisierung ohne Nakama-Evidence;
- Copy/Preset, das Zielidentität oder Routing mitnimmt.

### 9.3 Grenze der Marktbelege

Die Herstellerquellen belegen einzelne Mechanismen wie numerische Eingabe,
Accessible Colors oder ein Keyboard-Binding. Sie belegen keine vollständige
Screenreader-, Fokus- oder Switch-Control-Tauglichkeit dieser Plugins.
Nakamas Zugangsvertrag aus Abschnitt 7 bleibt deshalb eigenständig in JUCE,
Windows und FL Studio zu beweisen.

## 10. Wahrheits- und Fehlerlexikon

| Familie | Zustände, die nicht zusammenfallen dürfen | erforderliche Antwort |
|---|---|---|
| Messung | missing / no signal / measuring / fresh / partial / stale / invalid | Wert nur bei gültigem Support; Alter, Coverage, Unsicherheit |
| Verbindung | unavailable / authenticating / half-open / joined / degraded / disconnected / incompatible | Status, betroffene Richtung, Reconnect-/Repairweg |
| Mitgliedschaft | discovered / unclassified / confirmation required / joined / duplicate / tombstoned | keine automatische falsche Zuordnung |
| Proposal | unavailable / more data / ready / stale / protection conflict / manual only | Grund und nächster kleinster Schritt |
| Preview | arming / audible_ready / held / expired / blocked / aborted | Committed bleibt sichere Basis |
| Apply | candidate / accepted_pending_audio / active / confirmed / clamped / conflict / reverted | angewandte Werte und Revision, nicht nur Hash |
| DSP | confirmed / bypassed / automation overlay / invalid state / neutral fallback | tatsächlich hörbarer Zustand |
| Vergleich | unarmed / collecting / comparable / degraded / incomparable / judged | Passage, Match, Alignment, Guardrails |
| Unmask | unsupported / routing incomplete / ready / active / sidechain lost / releasing to zero | Richtung, Capability, Reduction und Fallback |
| Store | healthy / migrating / degraded / recovery required / read-only | keine halb sichtbare oder doppelte Anwendung |

Toasts dürfen diese Zustände ergänzen, aber nicht besitzen. Ein Zustand, der
nach Ablauf der Meldung weiterhin wahr ist, braucht einen persistenten Ort.

## 11. Informationsbudget und Reaktionsvertrag

### 11.1 Fenster

**[D]** Beide Gen-Flächen arbeiten im aktuellen Produktvertrag auf 760 × 430
logischen Pixeln. Der Blueprint vergibt bewusst keine historischen Pixelwerte.
Die frische visuelle Phase muss zuerst bei 100 Prozent beweisen:

- 16 Quellen plus stabile Auswahl;
- acht aktive EQ-Bänder;
- zwei benannte Curves und Maskierungszone;
- Confirmed/Draft/Automation/Preview ohne Überdeckung;
- lange und doppelte Unicode-Namen;
- Fehlerzustand mit Recovery;
- vollständigen Tastaturfokus ohne abgeschnittene Werte.

Die beschlossenen Skalierungsstufen 100, 125, 150 und 200 Prozent bleiben ein
Implementierungsvertrag; Layout darf bei höherem Scale keine Funktion
verschwinden lassen.

### 11.2 Reaktion

- Pointer-/Tastaturinput aktualisiert Griff, Wert und Curve im selben
  sichtbaren Frame.
- Spektrum und Messdaten repainten nur bei neuer Revision; statische und
  dynamische Ebenen werden getrennt invalidiert.
- Im Stillstand entsteht keine versteckte Dauer-Paintlast.
- Unter 16/32 Instanzen darf UI-Last die Audiofrist nicht gefährden. Bei
  Überlast wird Analyse reduziert und als stale/degraded markiert; Audio
  bleibt unverändert.
- Gemessen werden Release-Build in FL Studio, alle Skalierungsstufen,
  Mehrinstanzlast, Stillstand, Fensterwiederherstellung und schlechtester
  Frame/Perzentile statt nur Durchschnitt.

## 12. Prüfmatrix je Fläche

### 12.1 Gen Fläche 1

- 0, 1, 16 sichtbare und 32 verbundene Quellen;
- fehlender Hostname, wechselnder Hostname, langes User-Label, gleiche Namen;
- fresh, partial, stale, disconnected, suspended, invalid gleichzeitig;
- Lautheit ohne Signal, mit hoher Unsicherheit und nach Seek/Loop/Drop;
- zwei Projekte und doppelte Instanz-ID;
- Keyboard-only, Screenreader, 100/125/150/200 Prozent;
- gespeicherte Auswahl und Membership nach Broker-/Projektneustart;
- AssistantStep stale nach Intent-/Passageänderung.

### 12.2 Gen Fläche 2

- Master und Probeeq als zwei verschiedene Autoritäten;
- 0, 1 und 8 Bänder; ausgewählt, deaktiviert, protected und automatisiert;
- Dynamic off/on mit interner Quelle; priority_sidechain unsupported vor P8;
- Float-Pfad aktiv; fehlende Double-Capability erzeugt keinen Modusschalter;
- direkte Manipulation und identisches Text-/Tastaturergebnis;
- zwei Curves ohne Farbe, bei Farbsehschwäche und im Screenshot;
- Spectrum Grab ohne freien Slot oder mit Schutzkonflikt;
- Freeze aktiv nach Panel-Close;
- Displayeinstellungen ändern keinen Finding-Test;
- Disconnect/Stop/Recording/Offline während Hold und 10-s-Kandidat;
- Ablauf, Protection-Reject, zulässiger Werte-Clamp, stale Revision,
  Duplicate ACK und Revert;
- Preset alt/neu/ungültig; Copy mit identischem Quelle/Ziel und Zielverlust;
- mehrere Plugininstanzen, kleine Blocks und UI-Last.

### 12.3 Probeeq kompakt

- Broker nicht gestartet, inkompatibel, half-open und reconnect;
- unpaired, paired, Remote-Preview und bestätigter DSP;
- Hostautomation widerspricht Gen-Draft;
- Bypass/Neutralize ohne Broker;
- Projekt-Reload mit gültigem, altem und unbekanntem State;
- Editor geschlossen während Preview.

### 12.4 P8

- Capability unsupported und U11/PDC noch unbewiesen;
- fehlender, stummer, falscher oder wechselnder Sidechain;
- eine erlaubte und eine zweite abzuweisende eingehende Relation;
- direkte und indirekte Zyklen werden abgewiesen;
- Source-/Target-Tausch;
- Attack/Hold/Release und kontrolliertes Release auf 0;
- kein Detektorpfad erzeugt einen automatischen Boost;
- Brokerverlust bei bestätigter Relation und bei laufender Preview;
- schlechtere Guardrails gewinnen keinen Versuch.

## 13. Definition of Ready und Definition of Done für UI-Tickets

### Ready

Eine Funktion darf erst visualisiert werden, wenn vorhanden sind:

1. Objekt, Scope und autoritativer Writer;
2. Capability und ehrlicher Unsupported-Fallback;
3. Zustandsautomat einschließlich Invers- und Fehlerpfad;
4. persistente/transiente Felder samt Versionierung;
5. Testfixtures für Worst-Case-Zustände;
6. englische Arbeitsbegriffe ohne erfundene Produktzusage;
7. Pointer-, Tastatur-, Präzisions- und Accessibility-Weg;
8. messbarer Reaktions- und Audio-Sicherheitsvertrag.

### Done

Eine UI-Funktion ist erst fertig, wenn:

- jede sichtbare Aktion echte Technik erreicht;
- Confirmed, Draft, Automation und Fehler gegeneinander getestet sind;
- Fokus, Semantik, Werte, Grenzen und Reset nativ geprüft sind;
- 100/125/150/200 Prozent und 760 × 430 visuell bestehen;
- FL Studio Release-Build und Mehrinstanzlast gemessen sind;
- Loslassen, Cancel, Revert, Disconnect, Reload und Crash einen eindeutigen
  Zustand hinterlassen;
- Screenshotprüfung plus menschliche Hör-/Bedienprüfung erfolgt ist;
- kein alter visueller Entwurf als stiller Sollwert verwendet wurde.

## 14. Erweiterungsschablone

Jede neue Funktion oder Roadmap-Erweiterung bekommt eine Karte nach diesem
Schema:

| Feld | auszufüllen |
|---|---|
| Capability-ID / Ticket | technische Quelle und Releasegrenze |
| Nutzerergebnis | musikalisches Ziel in einem Satz |
| Objekt und Scope | Instanz, Paar, Passage, Session, Projekt oder Installation |
| Autorität | Writer des bestätigten Zustands |
| Voraussetzungen | Signal, Capability, Pairing, Passage, Datenqualität |
| Einstieg | wo und aus welchem fokussierten Objekt erreichbar |
| Primärhandlung | direkter, Tastatur- und Präzisionsweg |
| Zustandsfolge | unavailable → ready → draft/preview → active/confirmed |
| Sichtbarkeit | persistent, bei Fokus, auf Abruf oder nie |
| Antwort | sichtbar, hörbar, semantisch und zeitlich |
| Persistenz | transient, Pluginstate, Main-State, Preset oder SQLite |
| Fehler | missing, invalid, stale, conflict, disconnect, timeout |
| Rückweg | Cancel, Release, Reject, Revert, Repair, Rollback |
| Safety | Audio, Automation, Schutz, Privacy |
| Marktmechanismus | belegt, übertragen, bewusst nicht übertragen |
| Beweis | Tests, Fixtures, FL-Lauf, Hörtest, Accessibility, Screenshot |
| offene Entscheidung | explizite Userfrage statt Erfindung |

## 15. Offene Entscheidungen und technische Blocker

1. **[O] Frische visuelle Struktur:** Liste, Matrix oder semantischer Zoom für
   16/32 Quellen wird erst aus neuen designneutralen Wireframes entschieden.
2. **[O] Arbeitsnamen:** Die beiden Gen-Flächen haben Funktionsbeschreibungen,
   aber keine endgültigen Produktnamen.
3. **[O] Hold-Accessibility:** Screenreader-/Switch-Control-Ersatz für echtes
   Press-and-hold braucht native JUCE-Prüfung.
4. **[O] P3-Diagnose:** Lautheit/Unsicherheit ist entschieden; die übrigen
   NAK-57-Zähler besitzen noch keine Bedienarchitektur.
5. **[O] Main-Führung nach Brokerneustart:** Mehrdeutige Führung braucht einen
   bestätigbaren Main-State-Ingress.
6. **[O] P6-Testzugriff:** Der technische EQ-Kern braucht Beweise, aber keinen
   reaktivierten Probeeq-Voll-Editor.
7. **[O] Zweistufiger Mix:** ganze App ↔ gewählte Spur ist benannt; genaue
   Zustands- und Umschaltsemantik fehlt noch.
8. **[O] U11/U13:** echte FL-Messungen für Aux/PDC und Projektzeit bleiben
   offen beziehungsweise diagnostisch blockiert.
9. **[D] P9-Textpflege:** Der Produktumfang ist mit zwei Bundles plus Broker
   entschieden; nur der alte Drei-Bundle-Text ist zu bereinigen.
10. **[O] Vertragliche Nutzlasten:** state_report.dsp,
    command_ack.applied_dsp, EQ-Betriebszustand, Hostname/-index und
    Contribution sind derzeit reserviert, nicht fertig nutzbar.
11. **[O] S31b-Quellenkonflikt:** PLAN-STAND verweist für die sichtbare
    Umsetzung noch auf frühere Figma-/PNG-Stände. Vor Beginn von S31b muss
    dieser Verweis in einem eigenen Planänderungssatz auf den Blueprint plus
    eine neu abgenommene visuelle Spezifikation umgestellt werden.
12. **[O] 10-s-Kandidatenvertrag:** Kandidaten-ID, Timer, Message, ACK,
    Reconnect, Kill und Ablauf sind vor P7-UI zu spezifizieren; preview_begin
    und persistentes apply_transaction reichen dafür heute nicht.

Diese Punkte sind kein Freibrief für Platzhaltercontrols. Bis zur technischen
oder User-Entscheidung bleibt die jeweilige Funktion ehrlich unavailable oder
außerhalb der Oberfläche.

## 16. Quellenledger

### 16.1 Repository-Wahrheit

| Quelle | Verwendung |
|---|---|
| [CLAUDE.md](../../CLAUDE.md) | Produktidentität, Zwei-Flächen-Modell, harte Audio-/Hostinvarianten |
| [PLAN-STAND.md](../../docs/PLAN-STAND.md) | gerechneter Status, Tickets und aktuelle Nachträge |
| [FL-Nakama-Sonden-Design-Entwurf.md](../../docs/FL-Nakama-Sonden-Design-Entwurf.md) | technische Funktionen, Objekte, Zustände und P2–P9 |
| [bauaufteilung-sonden.md](../../docs/bauaufteilung-sonden.md) | Ticketgrenzen und Verantwortungen |
| [plugin-wissen.md](../../docs/plugin-wissen.md) | heutiger Codezustand und noch nicht verdrahtete Pfade |
| [fragen.json](../../docs/plan/fragen.json) | aktuelle Funktionsentscheide und offene Usertermine |

### 16.2 Wissensarchiv

Das Archiv liefert Transferwissen, nicht Produktwahrheit.

| ID | Prüfstand | Pfad | verwendete Regel |
|---|---|---|---|
| w-2026-08-26-zugaengliche-audio-controls | mehrfach-belegt | [Eintrag](../../wissen/ux/2026-08-26-zugaengliche-audio-controls.md) | Name/Rolle/Wert/Zustand/Aktion, Fokus, Grob/Fein, Meter auf Abruf |
| w-2026-08-26-audio-interaktion-kontext-statt-annahmen | quellen-geprueft | [Eintrag](../../wissen/ux/2026-08-26-audio-interaktion-kontext-statt-annahmen.md) | direkte und präzise Wege auf denselben Zustand |
| w-2026-08-26-nutzerzentriertes-audio-design | quellen-geprueft | [Eintrag](../../wissen/ux/2026-08-26-nutzerzentriertes-audio-design.md) | gemeinsame Interaktionssprache und funktionaler Hörprototyp |
| w-2026-08-26-visuelle-treue-native-ui-pipeline | quellen-geprueft | [Eintrag](../../wissen/ui-motion/2026-08-26-visuelle-treue-native-ui-pipeline.md) | lebende Daten statt gebackener UI-Zustände |
| w-2026-08-26-juce-ui-performance-messen | quellen-geprueft | [Eintrag](../../wissen/engineering/2026-08-26-juce-ui-performance-messen.md) | Frameantwort, Repaintgrenzen, Release-/Mehrinstanzmessung |
| w-2026-08-27-realtime-worst-case-statt-durchschnitt | mehrfach-belegt | [Eintrag](../../wissen/engineering/2026-08-27-realtime-worst-case-statt-durchschnitt.md) | Worst-Case-Kanten und Analyseabbau vor Audio |
| w-2026-08-27-messwerte-brauchen-support-und-zeitanker | mehrfach-belegt | [Eintrag](../../wissen/dsp/2026-08-27-messwerte-brauchen-support-und-zeitanker.md) | Support, Alter, Coverage, Revision und Unsicherheit |
| w-2026-08-26-plugin-modernisierung-ohne-vertrauensbruch | quellen-geprueft | [Eintrag](../../wissen/fl-studio/2026-08-26-plugin-modernisierung-ohne-vertrauensbruch.md) | State-/Automation-/Bypass-/Recall-Treue |
| w-2026-08-27-windows-pipes-sicherheit-und-lifecycle | mehrfach-belegt | [Eintrag](../../wissen/engineering/2026-08-27-windows-pipes-sicherheit-und-lifecycle.md) | Half-open, Generation, Deadline und Reconnect |
| w-2026-08-27-crashfester-store-und-outbox | mehrfach-belegt | [Eintrag](../../wissen/engineering/2026-08-27-crashfester-store-und-outbox.md) | pending/applied, Idempotenz und Crash-Recovery |
| w-2026-08-27-json-vertraege-vor-dem-parser | mehrfach-belegt | [Eintrag](../../wissen/engineering/2026-08-27-json-vertraege-vor-dem-parser.md) | invalid/incompatible statt stiller Defaults |

### 16.3 Marktquellen

Alle Marktquellen wurden am 31.08.2026 in offiziellen Herstellerseiten oder
Handbüchern geprüft. Ableitungen stehen in Abschnitt 8; sie sind keine
Behauptung über Herstellerintention oder Marktanteil.

## 17. Nächster Designschritt

Dieser Blueprint ist die Eingangsspezifikation für neue Wireframes. Der nächste
Schritt ist nicht, ein früheres Bild zu korrigieren, sondern drei frische,
graue Zustandsblätter aus denselben Objekten zu bauen:

1. Gen Fläche 1: Ruhezustand, 16-Quellen-Grenze und ehrlicher Degraded-Fall.
2. Gen Fläche 2: Confirmed, Bandfokus und Preview/10-s/Confirm-Transaktion.
3. Probeeq kompakt: verbunden, Brokerverlust und lokaler sicherer Rückfall.

Erst wenn Objektbesitz, Fokus, Tastaturweg und Worst-Case-Zustände darin
passen, beginnt die visuelle Sprache. Stilfragen werden dann getrennt von
Funktionsfragen entschieden.
