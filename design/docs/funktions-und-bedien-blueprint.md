# Nakama Funktions- und Bedien-Blueprint

Stand: 02.09.2026 (Ursprung 31.08.2026; zuletzt fortgeschrieben in den
Abschnitten 0, 4, 15, 16 und 17; Rangfolge und Planzuordnung mit PR2 am 02.09.2026)

Status: lebende, designneutrale Produktspezifikation

Geltungsbereich: Nakama Gen, Nakama Probeeq und die dazugehörigen Betriebswege

Nächste technische Grenze: G2-Nacharbeit (NAK-123) und die Planprüfung PR2,
danach P4 bis P9. S16–17 (SONDE-011) und S18–19 (SONDE-012) sind seit dem
01.09.2026 abgenommen.

Laufende technische Skizze:
[design/skizze/nakama-ui-technical-sketch.html](../skizze/nakama-ui-technical-sketch.html)
mit Leseblatt [design/skizze/LIES-MICH.md](../skizze/LIES-MICH.md). Sie ist
der derzeitige Stand, nicht der finale (User-Wort 02.09.2026).

## 0. Auftrag, Rang und Leseregel

Der User-Auftrag vom 31.08.2026 lautet, aus dem aktuellen Implementierungsplan
abzuleiten, welche technische Funktion entsteht und wie ein Mensch sie erreichen,
verstehen, bedienen, abbrechen und rückgängig machen kann. Frühere visuelle
Entwürfe sollen dafür ausdrücklich ignoriert werden. Dieser Blueprint übernimmt
deshalb weder Aufbau, Raster, Farben, Material, Icons noch Positionsentscheidungen
aus bisherigen Figma-Ständen, PNGs, Web-Prototypen oder visuell gebundenen Teilen
früherer Design-Abnahmen. Datiertes User-Wort zu Funktion und Verhalten bleibt
dagegen bindend, soweit es im aktuellen Plan oder seinem Entscheidregister
fortgeschrieben ist; seine Verwendung ist kein Import des früheren Designs.

Bindender User-Wortlaut für die Quellenabgrenzung:
„ignoriere bisherige design die entworfen wurden.“

### Rangfolge der lebenden Spezifikationen (festgelegt mit der Planprüfung PR2, 02.09.2026)

Nakama hat heute **zwei** lebende Spezifikationen. Was jede bindet, steht seit dem 02.09.2026
gleichlautend hier, im Kopf des Entwurfs und in der Lesetabelle von
[CLAUDE.md](../../CLAUDE.md):

| Quelle | bindet | bindet nicht |
|---|---|---|
| [FL-Nakama-Sonden-Design-Entwurf.md](../../docs/FL-Nakama-Sonden-Design-Entwurf.md) (Fassung 0.5) | Technik, Verträge, Phasen, Falsifikation | Aufbau, Hierarchie und Bedienwege der Oberflächen |
| **dieser Blueprint** | Funktions- und Bedienarchitektur beider Apps | Schemas, Phasenschnitt, Gates |

**Über beiden steht der aktuelle Vertrag und der Code.** Widerspricht eine der beiden
Spezifikationen dem gebauten Vertrag, gilt der Vertrag. Ein Widerspruch zwischen Entwurf und
Blueprint wird als Zeile in [docs/offene-punkte.md](../../docs/offene-punkte.md) geführt und
**nie kaschiert** — weder durch stilles Umschreiben noch durch eine Oberfläche, die den Konflikt
verdeckt.

Der Blueprint ersetzt deshalb nicht:

- den technischen Bauplan in
  [FL-Nakama-Sonden-Design-Entwurf.md](../../docs/FL-Nakama-Sonden-Design-Entwurf.md),
- den gerechneten Status in [PLAN-STAND.md](../../docs/PLAN-STAND.md),
- die Verträge und Schemas unter [eq-copilot/schemas](../../eq-copilot/schemas),
- eine spätere visuelle Spezifikation mit Maßen, Typografie und Renderabnahme.

Er ergänzt diese Quellen um den fehlenden Vertrag zwischen Technik und Bedienung.
Wenn Technik und Oberfläche widersprechen, entscheidet der aktuelle technische
Vertrag über das Machbare; die Oberfläche darf den Widerspruch nicht kaschieren.

[DESIGN.md](../../DESIGN.md) trennt seit 01.09.2026 den aktuellen funktionalen
Größenvertrag von seinem historischen visuellen System. [UX-CONTRACT.md](../../UX-CONTRACT.md)
führt den aktuellen Größen- und Verhaltensvertrag; seine weiterhin enthaltenen
Figma-, Golden-, Raster- und Legacy-Prototypfestlegungen werden nicht zur neuen
Layoutquelle. Wo Verhalten nicht mit aktuellem Plan, aktuellem User-Wort oder
heutigem v3-Vertrag zusammenpasst, bleibt der Konflikt offen und wird vor der
nativen Übernahme ausdrücklich bereinigt; siehe Abschnitt 15.

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
- **[B]** SONDE-011 Phase A ist mit einem grünen 33/33-Lauf belegt; das
  [Manifest](../../docs/beweise/SONDE-011.md) erklärt das Ticket einschließlich
  Phase B weiterhin als nicht abgenommen. Die Phase-B-Matrix und paralleler
  uncommitteter Broker-Code sind kein Abnahmebeleg. Der Blueprint beschreibt sie
  daher nicht als fertige Produktfunktion.
- **[D]** Die neue v3-Leitung ist gebaut, aber noch nicht als produktiver Weg in
  beide Plugins verdrahtet. Probeeq ist derzeit noch eine audioseitig prüfbare,
  neutrale Shell; der aktive EQ entsteht erst in P6.
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

### 1.3 Aufgabenrang und Recovery-Kosten

Die Rangfolge trennt belegte Arbeitswege von noch zu prüfenden
Häufigkeitsannahmen. Sie ist eine Eingabe für Zustandsentwürfe, keine Behauptung
über bereits beobachtete Nutzerzahlen.

| Rang | Job | Zeitdruck und Fehlerkosten | beobachtbarer Beweis | Marke |
|---|---|---|---|---|
| häufig | zwischen Probeeq-Zielen wechseln, Filter setzen und zum Master zurückkehren | kurz; ein falsches Ziel verändert den falschen Bus | Zielname und Writer bleiben vor, während und nach jeder Geste eindeutig; Wechsel braucht keinen Menüweg | **[U]** |
| häufig | Quelle, Befund und Messqualität einordnen | mittel; stale oder unsichere Evidenz kann eine falsche Klangentscheidung auslösen | Nutzer erkennt Quelle, Frische, Support und Unsicherheit, bevor er einen Test startet | **[A]** |
| häufig | Änderung hören, vergleichen, bestätigen oder verwerfen | hoch; Preview darf weder kleben bleiben noch als bestätigt erscheinen | Hold, Kandidat, Confirmed und wirksamer DSP werden im Zustandslauf korrekt benannt und verlassen | **[U]/[D]** |
| gelegentlich | Passage, Intent, Preset, Copy, Analyzer und Evidenzdetails bearbeiten | mittel; Scope- oder Identitätsverlust erzeugt schwer erklärbare Folgefehler | Einstieg und Rückkehr behalten Quelle, Ziel, Passage und Revision | **[D]/[U]** |
| Recovery | Disconnect, Automation, stale Revision, Ablauf, ungültigen State oder Schutzkonflikt beheben | sehr hoch; der sichere Klang und die Bedienhoheit dürfen nicht unklar sein | Oberfläche nennt autoritative Wahrheit, betroffenen Scope und den kleinsten sicheren Rückweg | **[D]** |
| Betrieb | installieren, migrieren, reparieren, exportieren, löschen oder zurückrollen | selten, aber potenziell irreversibel | Version, Umfang, Wirkung und Rückweg sind vor Bestätigung prüfbar | **[D]** |

**[A] Prüfpflicht:** Die beiden als häufig angenommenen Orientierungs- und
Evidenzjobs werden im ersten Funktionsprototyp gegen reale Aufgaben geprüft.
Erweisen sich Häufigkeit oder Zeitdruck als anders, ändert sich die
Disclosure-Rangfolge, nicht still der Produktumfang.

## 2. Sonic Object Map und Zustandsautorität

Jede Bedienung muss erkennen lassen, welches Objekt betroffen ist, wie weit die
Wirkung reicht und wer den bestätigten Zustand besitzt.

| Objekt | Menschliche Frage | Scope | Autoritative Wahrheit | Sichtbare Identität |
|---|---|---|---|---|
| Projekt | In welchem musikalischen Zusammenhang arbeite ich? | Projekt | persistenter Gen Main-State | Projektidentität, bestätigte Mitglieder, führendes Gen |
| Session/Verbindung | Welche Instanzen gehören jetzt zusammen? | Session-Epoche | transienter Broker-Sessiongraph | Generation, Join-, Liveness- und Konfliktstatus |
| Quelle/Probe | Welchen Bus höre oder untersuche ich? | eine Probeeq-Instanz | Pluginidentität plus bestätigte Mitgliedschaft | hostgelieferter Busname, sonst User-Label; nie Verbindungsreihenfolge als Ersatz |
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
- Der hostgelieferte Busname hat Vorrang; fehlt er, trägt das gespeicherte
  User-Label. Das Label bleibt begrenzter, untrusted Usertext und wird nie als
  Pfad, Markup oder Hostwahrheit behandelt. Herkunft und Rückfall müssen
  nachvollziehbar sein, aber der sichtbare Name darf nicht flackern oder aus
  der Verbindungsreihenfolge erfunden werden.
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
- minimalphasiger Float-Kern ohne Lookahead und mit 0 gemeldeten Samples
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
- Ein unbekannter oder ungültiger neuerer State lädt den Audiopfad neutral,
  erklärt den Zustand und bewahrt die Originalbytes verlustfrei read-only für
  Rückweg oder spätere Migration; er wird nicht halb angewandt.

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

**[O] Direkte Bearbeitung:** Der Plan bindet von Gen erzeugte Klangänderungen an
Preview und explizites Apply, legt aber noch nicht vollständig fest, wie
Band-Drag, Textwert, Preset-Load und Curve-Copy beim Loslassen beziehungsweise
Bestätigen in Hold-Preview, 10-s-Kandidat und neue Revision übergehen. Bis P7
diese gemeinsame Transaktionsabbildung festlegt, darf die Oberfläche weder ein
sofortiges permanentes Schreiben noch einen bloß dekorativen Confirm-Schritt
vortäuschen.

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
  Standardmaximum 1,5 dB, harte Obergrenze 3 dB, sofern der technische Plan
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

Die folgende Struktur ist kein Pixelentwurf. Mit **[U]** markierte Teile wurden
in der technischen UI-Fragenrunde vom 31.08.2026 entschieden; die übrigen
Anteile bleiben **[A] Layout-Hypothesen**. Arbeitsmodus, Wortlaut und offener
Stand sind in
[`2026-08-31-technische-ui-architektur-arbeitsmodus.md`](../abnahmen/2026-08-31-technische-ui-architektur-arbeitsmodus.md)
festgehalten.

Sie verweigert bewusst drei Gleichzeitigkeitspakete: keine vollständige
Diagnose neben dem Hörhandgriff, keine Detailwerte aller Quellen neben dem
ausgewählten Befund und keine Präzisionscontrols aller Bänder oder Ziele
zugleich. Persistente Identität, hörbarer Zustand, Sicherheit und Recovery
bleiben stehen; Details werden aus ihrem Besitzer heraus geöffnet.

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

**[U] Abgenommene Grundarchitektur:** Eine vertikale Sources-Spalte bleibt
links über die nutzbare Höhe stehen. Rechts nutzt die Frequenzvisualisierung
die verfügbare Breite; darunter liegt eine waagerechte Findings-Fläche mit
fester Höhe. Der frühere rechte Inspector ist verworfen. Genau ein Finding ist
im Fokus; Navigation und Gesamtübersicht wechseln den Inhalt innerhalb dieser
festen Fläche. Finding-Auswahl markiert den betroffenen Frequenzbereich, ohne
Achsen, Ausschnitt oder Graphgröße automatisch zu verändern. Inhalt liegt links
und eine stabile Aktionszone rechts.

**[U] Quellen- und Aktionsbesitz:** Jede Quellenzeile zeigt Identität,
Signalstatus, Messaktualität und Anzahl offener Findings. Genau eine Quelle ist
Hauptziel; weitere Quellen sind nur benannte Referenzen und keine Mitbesitzer
der Aktionen.

**[U] Befund-Dichte (Fünferblock 03, 02.09.2026):** Ein Befund zeigt genau
drei Zeilen: wahrscheinliche Ursache, kleinster Test, worauf hören.
Sicherheit ist der Zustand des Befunds, kein eigener Wert: nur ein sicherer
Befund steht auf `READY TO SEND` und bietet Audition und Draft an; `MORE
DATA` und `STALE` bieten keinen Draft. Der Beleg ist die markierte Zone mit
Ziel- und Referenzkurve im Graphen; Alternativen sind weitere Befunde in der
Liste. Die sechs Teile des Ursachenbefunds aus dem Entwurf (Abschnitt 8,
„Ergebnisform“) bleiben Datenmodell; sechs Werte je Befund werden nicht
angezeigt (User-Wort 02.09.2026: „wenn wir 6 werte für eine messung anzeigen,
ist das schlichtweg schlechtes design“). `CONFIDENCE` unter dem Graphen ist
die Messqualität der Passage, nicht die Befundsicherheit.

**[U] Übergang (berichtigt 02.09.2026, Fünferblock 03):** Der Kopf von Gen
trägt beide Flächennamen als stabilen Umschalter in beide Richtungen; die
aktive Fläche ist dort erkennbar. `SEND DRAFT` bleibt der Handgriff am
Befund, der den Draft übergibt und zur EQ-Fläche wechselt, ist aber nicht
mehr der einzige Hinweg; der Entscheid vom 31.08.2026 „nur SEND DRAFT“ ist
überholt (User-Wort 02.09.2026: „nein hin geht es nicht nur über send draft,
sondern über den header wo EQ steht“). Zurück geht es über den Namen der
ersten Fläche. Nach `APPLY` fest oder `REJECT` bleibt Gen auf der EQ-Fläche;
es gibt keine automatische Rückkehr. Ein offener Draft bleibt beim Wechsel
offen und erscheint am Befund als Zustand (`DRAFT OPEN · EQ`, der Handgriff
dort führt zurück zur EQ-Fläche); ein laufender 10-s-Kandidat zählt auf
Fläche 1 sichtbar weiter und bietet keinen zweiten Draft an. Der
Flächenwechsel ist reine Anzeige und berührt die Transaktionslogik nicht.
Die Inspektionsleiste der Skizze bleibt Werkzeug. Die Flächennamen sind
Arbeitsnamen (Karte U23).

Ruhezustand: Quellenorientierung.

Auswahlzustand: eine Quelle oder Beziehung.

Fehlerzustand: betroffene Quelle bleibt an derselben Stelle und erklärt ihren
Status; sie verschwindet nicht hinter einem globalen Toast.

Bei 16 sichtbaren und 32 angeschlossenen Probes muss die festgelegte vertikale
Sources-Spalte Überblick, Suche/Quick-Jump und eine stabile Auswahl erlauben.
Zeilendichte, Scroll-/Suchmechanik und der ehrliche 32-Probes-Grenzfall bleiben
**[O]** und werden am belasteten Funktionsblatt geprüft.

### 4.2 Gen Fläche 2 — EQ und Transaktion

Die Fläche braucht fünf funktionale Zonen:

1. **Persistenter Zielkontext:** Master oder benannte Probeeq, Pairing,
   Capability und bestätigte Revision.
2. **Dominante akustische Arbeitsfläche:** Spektrum des aktuellen
   Bearbeitungsziels, Gesamtcurve, Bänder, optional eine zweite benannte Curve
   und die belegte Maskierungszone. Bei zwei Curves bleibt das Bearbeitungsziel
   zusätzlich zu Farbe eindeutig; das Live-Spektrum gehört nur zu diesem Ziel.
3. **Objektpräzision:** exakte Werte und Aktionen nur für die aktuelle
   Bandauswahl beziehungsweise Relation.
4. **Globaler Klangzustand:** Bypass, Mix, Auto-Gain/Output, A/B,
   Preset/Copy und Revision/History. Die Global-Sektion ist **[U]** im
   Ruhezustand eingeklappt. **[U]** (Fünferblock 03, 02.09.2026) Die
   eingeklappte Zeile nennt nur aktive Abweichungen vom Standard beim Namen
   (etwa `MIX 92 % · AUTO` oder `BYPASS`); sind alle globalen Werte
   Standard, steht dort nur der Einstieg `GLOBAL`. Ein Zähler wie `2 ON` und
   eine dauerhafte Kurzzeile mit Bypass, Mix und A/B sind verworfen.
   Aufgeklappt liegen Input, Output mit Auto-Gain daneben (Entscheid
   24.08.2026; der Output-Wert wandert sichtbar mit), Width, Mono-Bass,
   Bypass, Mix, A/B, Voreinstellungen und Verlauf. Der Einstieg `GLOBAL` ist
   ein Knopf fester Maße; die Nennungen stehen als Leseanzeige daneben
   (Tasten-Gesetz 25.08.2026). **[O]** Wie Voreinstellungen und Verlauf
   innerhalb der Sektion geöffnet werden, ist noch nicht gefragt; bis dahin
   sind beide Einstiege ehrlich unavailable.
5. **Transaktionslocus:** das Proposal/Draft des aktuellen Ziels mit Hold,
   10-s-Kandidat, Confirm, Reject/Revert und Blocking-Grund. Dieser Locus ist
   in jeder EQ-Zielansicht direkt erreichbar.

**[U] Arbeitsflächen- und Disclosure-Regel:** Der EQ-Graph dominiert. Eine
permanente rechte Transaktionsspalte und eine permanente Band-Parameterzeile
sind verworfen. Der aktuelle Draft besitzt stattdessen eine kompakte Zeile
unter dem Graphen, die ohne offenen Draft verschwindet. Doppelklick auf einen
vorhandenen Bandpunkt öffnet ein objektgebundenes Mini-Panel für Filter Type,
Frequency, Gain, Q und Dynamic. Der aktuelle Filtertyp steht als kompaktes
Kurvensymbol im Panel; seine Aktivierung öffnet dort eine 2×3-Auswahl derselben
sechs typischen Filterkurven. Sichtbar werden keine ausgeschriebenen
All-Caps-Typnamen benötigt. Die englischen Namen bleiben als Tooltip und
zugängliche Control-Bezeichnung erhalten. Während dieser Auswahl ersetzt die
Typmatrix innerhalb desselben Grundkörpers die drei Wertefelder; sie verlängert
das Panel nicht zu einem zweiten großen Popup. Das Panel ist zweidimensional
kompakt statt als lange horizontale Leiste aufgebaut. Es verankert sich am
betroffenen Bandpunkt und erscheint je nach freiem Raum ober- oder unterhalb;
an den Graphkanten wird es innerhalb der Arbeitsfläche gehalten. Doppelklick
auf eine freie Graphposition belegt dort den nächsten freien der acht
Band-Slots als `bell`, wählt ihn und öffnet dasselbe Panel. Bei acht belegten
Slots wird der Grenzfall sichtbar gemeldet; kein bestehendes Band wird still
überschrieben.

**[U]/[D] Bandbesitz, Kanalmodus und Aktivierung:** Ist das Mini-Panel bereits
offen, bindet ein einfacher Klick auf einen anderen belegten Bandpunkt den
gleichen Panelkörper atomar an diesen Slot und setzt nur die flüchtige
Unteransicht auf `Frequency`, `Gain` und `Q` zurück. Werte oder Zustände werden
nicht zwischen Bändern kopiert. Der aktuelle `channel_mode` bleibt als
kompakter Zustandscontrol im Panelkopf sichtbar; seine Aktivierung ersetzt die
drei Wertefelder vorübergehend durch `Stereo`, `Left`, `Right`, `Mid` und
`Side`. Eine Wahl ändert nur `channel_mode` und kehrt zur Grundansicht zurück.
Der ebenfalls stabile `ON/OFF`-Control ändert ausschließlich `enabled`. Ein
ausgeschaltetes belegtes Band bleibt als nicht nur farblich gekennzeichnetes,
auswählbares Objekt mit unveränderter ID und allen Werten im Graphen. Bypass
gibt keinen Slot frei.

**[U]/[O] Band entfernen:** `Remove Band` ist eine eigene Aktion am Rand des
Panelkopfs und von `ON/OFF`, Dynamic und Schließen getrennt. Nur Remove darf
einen der acht Slots freigeben; es schließt das Panel, entfernt den Punkt und
nummeriert andere Band-IDs nicht um. Ein unmittelbarer Undo-Rückweg stellt das
vollständige Band unter derselben ID wieder her. Der 109-Parametervertrag
besitzt derzeit kein persistentes Occupancy-/Remove-Feld. Bis Slotfreigabe,
Edit-Transaktion und Undo versioniert sind, bleibt der produktive native
Remove-Control deshalb technisch unavailable; die technische Skizze zeigt das
abgenommene Zielverhalten.

**[U]/[D] Dynamic-Disclosure:** Globale Zustände, aktiver Draft, Freeze,
Automation und Preview dürfen nie spurlos eingeklappt sein. Der Filtertyp ist
im objektgebundenen Mini-Panel sichtbar und diskret wechselbar; der feste
Band-Slot bleibt dabei erhalten. Für ein aktives Dynamic-Band wechselt
derselbe am Bandpunkt verankerte Panelkörper in eine zweireihige Ansicht für
einen kompakten `dynamic_enabled`-Zustandscontrol sowie `Range`, `Threshold`,
`Attack`, `Hold` und `Release`. Die normalen Felder `Frequency`, `Gain` und `Q`
werden für diesen Teilschritt ersetzt und nicht in einem Nebenpanel oder
Akkordeon gleichzeitig gezeigt. `DYN · OFF` aktiviert und öffnet in einem
Schritt. Bei bereits aktivem Band öffnet oder verlässt `DYN · ON` nur die
Ansicht. Der Zustandscontrol am Anfang der Ansicht ist der ausdrückliche
Ausschalter; er erhält die fünf Werte, stellt die Grundwerte wieder her und
fokussiert `DYN · OFF`. Bandidentität und Panelanker bleiben stabil;
`priority_sidechain` bleibt vor P8 unsichtbar.

**[U] Mini-Panel-Schließen:** Per Zeiger schließt nur der eigene sichtbare
Schließen-Control. Ein einfacher Klick auf freie Graphfläche lässt Panel,
Auswahl und Parameter unverändert. `Escape` verwirft zuerst eine laufende
Zahleneingabe; andernfalls schließt es das oberste nichtmodale Panel. Schließen
verändert weder Bandparameter noch `dynamic_enabled` und gibt den Fokus an den
zugehörigen Bandpunkt zurück.

**[U]/[O] Dynamic-Aktivbeleg:** Ein aktives Band besitzt eine zweite, ruhige
Kontur an seiner eingestellten Gain-Position. Diese Kontur ist zugleich der
stabile direkte Bedienort; der innere Punkt und der zugehörige Kurvenzug folgen
der tatsächlichen dynamischen Gain-Auslenkung. Bewegung und Kontur ergänzen
sich, weil ein aktives Band unterhalb des Thresholds oder bei Range 0
stillstehen kann. `Frame.band_dynamic_gain_db` ist für S26–28 als Name
reserviert; Feld-ID und Runtime-Nutzlast sind noch nicht gebaut. Probeeq führt
den Wert später mit Anzeigekadenz über den Featureframe zu Gen, Gens Master-EQ
lokal ohne IPC. Bis zu diesem versionierten Telemetrieweg bleibt die native
Bewegung technisch offen und darf nicht aus Einstellwerten erfunden werden.

**[U] Zielwechsel:** Probeeq-Ziele folgen, soweit FL es belegt liefert, der
Mixerreihenfolge. Der Master besitzt einen getrennten stabilen Einstieg und ist
kein weiterer Schritt in der Sondenfolge. Das Mausrad wechselt die Sonde nur,
wenn nichts per Klick markiert ist.

**[A] Radbesitz:** Ein fokussiertes oder markiertes Präzisionscontrol soll das
Rad für seinen eigenen Wert beanspruchen. Diese Konkretisierung des
„nichts-markiert“-Falls braucht im Prototyp einen Fehlbedienungstest.

### 4.3 Probeeq kompakt

Immer erreichbar:

- Instanzidentität und sichtbarer Name;
- Messpunkt/Signalstatus;
- Verbindung, Pairing und Capability;
- bestätigter EQ-Ein/Aus-Zustand und Bypass als zwei getrennte Wahrheiten;
- klarer Hinweis, ob Automation oder Remote-Transaktion wirkt;
- der eigene Instanz-Mix, sobald sein P6-Parametervertrag existiert;
- sicherer lokaler Bypass beziehungsweise Neutralize-Rückfall.

**[U] Form (Fünferblock 03, 02.09.2026):** Probeeq ist eine flache Kachel
mit ein bis zwei Zeilen. Zeile 1 trägt dauerhaft Name und Bus, Verbindung,
den EQ-Punkt (an/aus), Bypass und Mix; Zeile 2 erscheint nur bei
zugeschaltetem EQ und trägt die laufende Änderung (Band, Frequenz, Gain, Q)
und den Modus der Sonde (Inhalt seit 23.08.2026). Ein Feld halber Gen-Größe
und die frühere 760×430-Bühne sind verworfen: Probeeq ist auf vielen Bussen
gleichzeitig offen, jeder Pixel Höhe zählt mal Instanzen (Begründung vom
20.08.2026). Die Maße der Skizze (580×92 mit EQ, 580×52 ohne) sind
Dichteprüfung, kein Beschluss. **[O]** Ob die zweite Zeile die Fensterhöhe
ändert oder in fester Höhe ein- und ausgeblendet wird, ist eine Frage des
Host-Fensters (JUCE `setSize` unter FL) und noch nicht entschieden.

Nicht enthalten:

- vollständige Bandbearbeitung;
- lokale Proposal-/Advisor-Arbeit;
- zweite abweichende Preset- oder Copy-Logik;
- eine alternative Fernsteuerungswahrheit.

Welche zusätzlichen lokalen Notfallaktionen neben Instanz-Mix,
Bypass/Neutralize nötig sind, bleibt **[O]** und wird aus
P6/P7-Recoverytests entschieden.

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
| zweite Curve | nur wenn aktiv, mit Name und Edit-Ziel | ✓ | Sichtbarkeit | sonst verborgen |
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
| Probeeq EQ-Ein/Aus, Bypass und Instanz-Mix | ✓ auf Probeeq, sobald technisch vorhanden |  | genaue Werte | Voll-Editor nie |
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
| EQ-/Analyzer-Skalen | jeweilige Curve, Einheit und Achse | Analyzerpegel und EQ-Gain teilen Fläche, aber nicht Bedeutung | eine unbeschriftete dB-Achse für beide Systeme |
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
- Nichtkritische Hinweise, Updates oder Onboarding stehlen während Drag,
  Preview, Vergleich oder Texteingabe weder Fokus noch Eingabe. Ein
  scope-relevanter Sicherheits-, Verbindungs- oder Statefehler darf
  unterbrechen, beendet die flüchtige Klangwirkung sicher und gibt den Fokus
  anschließend an einen vorhersehbaren Besitzer zurück.
- Escape beendet den obersten flüchtigen Modus. **[A]** Beim 10-s-Kandidaten
  kehrt der Klang zu Confirmed zurück, ohne den darunterliegenden Draft oder das
  Proposal still zu verwerfen; diese Detailsemantik bleibt Teil des offenen
  P7-Vertrags. Nach Dialog, Deselect, Apply, Reject oder Fehler kehrt der Fokus
  zum auslösenden Objekt zurück.

### 7.2 Primäre Handlungen

| Handlung | Pointer | Tastatur/Präzision | Abbruch/Reset | Semantik und Antwort |
|---|---|---|---|---|
| Quelle wählen | Klick auf Quellenobjekt | Pfeile/Quick-Jump, Enter wählt | Escape schließt Quick-Jump; bestätigte Auswahl bleibt | Name, Messpunkt, Frische und Signalstatus |
| EQ-Ziel wechseln | Klick auf stabilen Zielpunkt; Mausrad nur, wenn nichts markiert ist | nächstes/vorheriges Probeeq-Ziel; Master separat | Abbruch lässt das bisherige Ziel aktiv | Zielname, Writer, Pairing, Revision und Bearbeitungsstatus |
| Quelle benennen | Rename-Aktion | Textfeld, Enter speichert | Escape verwirft | Textfeld mit Herkunft Host/User; Länge validieren |
| Passage wählen/markieren | Zeitbereich/Mark action | Start/Ende numerisch oder transportbezogene Aktion | Cancel entfernt Draft | Bereich, Fingerprint und Comparability |
| Intent setzen | direkte Rollen-/Prioritätswahl | Radiogruppe/Combobox, Richtung separat | Reset auf ungesetzt | Funktion, Schutz, scope und Konflikte |
| Evidenz öffnen | Finding aktivieren | Enter öffnet Details | Escape zurück | Aussageklasse, Beleg-IDs, Alternativen, Konfidenz |
| Band auswählen | Node/Curve | Bandliste oder nächstes/vorheriges Band | Deselect | Band-ID, Typ und aktiver Zustand |
| Band anlegen | Klick/Drag oder Spectrum Grab | Add-Band-Aktion plus Frequenzfeld | Escape verwirft Draft | Ziel, Frequenz, Gain, Q, Slotverfügbarkeit |
| Band formen | Drag und modifizierte Feingeste | Pfeile grob/fein, Textwerte | Default/Undo | Wert, Einheit, Grenzen, Clamp und Automation |
| Bandkanal wählen | kompakter aktueller Modus im Panelkopf | fünf benannte Optionen im selben Panelkörper | Escape schließt Unteransicht ohne Wertänderung | `channel_mode` des festen Slots; keine Übernahme vom vorherigen Band |
| Band umgehen | stabiler `ON/OFF`-Control im Panelkopf | Space/Enter auf demselben Control | erneutes Aktivieren; Werte bleiben | nur `enabled`; inaktiver Punkt bleibt sichtbar und belegt den Slot |
| Banddynamik | `DYN · OFF` aktiviert und öffnet; `DYN · ON` öffnet/verlässt nur die Ansicht | Zustandscontrol plus Range/Threshold/Attack/Hold/Release in zwei Reihen | Ausschalten erhält Werte und stellt Frequency/Gain/Q wieder her; Escape verwirft Zahleneingabe oder schließt | zweite Kontur belegt Aktivzustand und Sollposition; innerer Punkt und Kurvenzug folgen erst mit nutzbarer autoritativer Live-Telemetrie der Gain-Auslenkung; priority_sidechain erst bei Capability |
| Band entfernen | getrennte `Remove Band`-Aktion | benannter Tastaturcontrol; kein modifier-only Hauptweg | Undo stellt vollständiges Band mit gleicher ID wieder her | einzige Slotfreigabe; native Umsetzung wartet auf versionierten Occupancy-/Undo-Vertrag |
| Schutzbereich setzen | Range-Handles | zwei numerische Endpunkte | Reset/Cancel | Lower/Upper, Gültigkeit und betroffene Aktion |
| Preview halten | Press-and-hold | Key-down/up auf fokussierter Aktion | Release/Fokusverlust/Timeout | Momentary action, Lease und Ziel |
| 10-s-Kandidat starten | Klick | Enter/Space activation | **[A]** Escape/Expiry → Confirmed, Draft bleibt; Reject verwirft bewusst | Candidate state, Restzeit, Baseline; Detailsemantik noch **[O]** |
| Kandidat bestätigen | zweiter Klick am stabilen Locus | Enter auf Confirm | Revert danach | pending audio, active, confirmed/applied values |
| Freeze | Toggle/Hold | Space toggles; separate temporary action | Off/Release | Toggle state; Frozen ist dauerhaft markiert |
| Analyzer einstellen | Panel/Popover | Tab/Arrows/Textwerte | Reset display defaults | ausschließlich Display state |
| Preset laden/sichern | Browser/Liste | Suche, Pfeile, Enter | Cancel; bestätigter Load erhält Revision/Rückweg | Presetname/Version; Ziel separat; Load-Transaktion noch **[O]** |
| Curve kopieren | Copy-Aktion | Quelle und Ziel als zwei Felder, Confirm | Cancel; bestätigte Copy erhält Revision/Rückweg | Zusammenfassung: source → target; Identität bleibt; Transaktion noch **[O]** |
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

- hostgelieferter Busname plus ehrlicher Fallback, Suche/Quick-Jump und progressive
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
- kein permanentes Apply aus einer bloßen Analyseanzeige ohne benanntes
  Proposal, Ziel, Baseline, Kandidatenstufe und Bestätigung; der entschiedene
  Einstieg von Fläche 1 in genau diese Transaktion bleibt erlaubt;
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
| Hold-Preview | arming / audible_ready / held / releasing / expired / blocked / aborted | Committed bleibt sichere Basis; Release ist sichtbar |
| 10-s-Kandidat | ready / active / expiring / expired / rejected / conflict | Baseline, Kandidaten-ID, Restzeit und eindeutiger Rückfall |
| Apply | accepted_pending_audio / active / confirmed / clamped / conflict / reverted | angewandte Werte und Revision, nicht nur Hash |
| DSP | confirmed / bypassed / automation overlay / invalid state / neutral fallback | tatsächlich hörbarer Zustand |
| Vergleich | unarmed / collecting / comparable / degraded / incomparable / judged | Passage, Match, Alignment, Guardrails |
| Unmask | unsupported / routing incomplete / ready / active / sidechain lost / releasing to zero | Richtung, Capability, Reduction und Fallback |
| Store | healthy / migrating / degraded / recovery required / read-only | keine halb sichtbare oder doppelte Anwendung |

Toasts dürfen diese Zustände ergänzen, aber nicht besitzen. Ein Zustand, der
nach Ablauf der Meldung weiterhin wahr ist, braucht einen persistenten Ort.

## 11. Informationsbudget und Reaktionsvertrag

### 11.1 Fenster

**[U]** Beide Gen-Flächen verwenden genau eine logische Layoutgröße von
950×538. Eine Compact-Fassung, Fenster-Resize und größenabhängiges Reflow sind
bis nach Fertigstellung dieser Standardoberfläche vertagt. Damit existiert für
die aktuelle Architektur genau ein Informationsbudget und genau eine
Geometrie für Rails, Bedienziele, Graph und akustische Evidenz. Der Blueprint
vergibt darüber hinaus bewusst keine visuellen Pixelwerte. Die frische visuelle
Phase muss bei 100 Prozent auf dieser Standardgröße beweisen:

- 16 Quellen plus stabile Auswahl;
- acht aktive EQ-Bänder;
- zwei benannte Curves und Maskierungszone;
- Confirmed/Draft/Automation/Preview ohne Überdeckung;
- lange und doppelte Unicode-Namen;
- Fehlerzustand mit Recovery;
- vollständigen Tastaturfokus ohne abgeschnittene Werte.

Die beschlossenen Skalierungsstufen 100, 125, 150 und 200 Prozent bleiben ein
getrennter Implementierungsvertrag. Sie vergrößern dieselbe 950×538-Geometrie
und dürfen weder Funktionen noch Anordnung verändern. Ein sichtbarer
Größenwahlschalter, Compact, Fenster-Resize und Vollbild gehören nicht zum
aktuellen Ziel.

Vor einem Layout werden reservierter Ziel-/Statusraum, verbleibende akustische
Evidenzfläche und die logischen Rechtecke aller Primärziele bei 950×538
gemessen. Die 24×24 CSS-Pixel aus
[WCAG 2.2, Target Size (Minimum)](https://www.w3.org/WAI/WCAG22/Understanding/target-size-minimum.html)
dienen höchstens als dokumentierter Web-Risikovergleich, nicht als ungeprüftes
Gesetz für JUCE. Entscheidend sind reale logische und physische Größe, Abstand,
Präzisionsweg und äquivalenter Tastaturpfad in FL Studio bei allen
Skalierungsstufen.

### 11.2 Reaktion

- Pointer-/Tastaturinput bestätigt die Eingabe im selben sichtbaren Frame;
  Griff, Wert und Curve bleiben dem hörbaren beziehungsweise
  hostautoritativen Zustand zuordenbar.
- Autoritative Spektrum- und Messwerte ändern sich nur mit einer neuen
  Datenrevision. Eine rein visuelle Interpolation darf zwischen Revisionen
  zeichnen, wird aber nie als neue Messung ausgegeben und nie auf
  Parameterwerte angewandt.
- Statische und dynamische Ebenen sowie Stillstandslast werden im Release-Build
  gemessen. Der Blueprint schreibt vor diesem Profiling weder Cache- noch
  Repaintarchitektur vor; im Stillstand darf keine unbegründete Dauerlast
  verbleiben.
- Im Lastfall mit 16 sichtbaren Quellen und bis zu 32 verbundenen Instanzen
  darf UI-Last die Audiofrist nicht gefährden. Bei Überlast wird Analyse
  reduziert und als stale/degraded markiert; Audio bleibt unverändert.
- Gemessen werden Release-Build in FL Studio, alle Skalierungsstufen,
  Mehrinstanzlast, Stillstand, Fensterwiederherstellung und schlechtester
  Frame/Perzentile statt nur Durchschnitt.

## 12. Prüfmatrix je Fläche

### 12.1 Gen Fläche 1

- 0, 1, 16 sichtbare und 32 verbundene Quellen;
- fehlender oder wechselnder hostgelieferter Busname, langes User-Label,
  gleiche Namen;
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
- EQ-Gain- und Analyzerpegel-Skalen ohne Farbe bei 950×538 eindeutig;
- Spectrum Grab ohne freien Slot oder mit Schutzkonflikt;
- Freeze aktiv nach Panel-Close;
- Displayeinstellungen ändern keinen Finding-Test;
- Disconnect/Stop/Recording/Offline während Hold und 10-s-Kandidat;
- Ablauf, Protection-Reject, zulässiger Werte-Clamp, stale Revision,
  Duplicate ACK und Revert;
- Preset alt/neu/ungültig; Copy mit identischem Quelle/Ziel und Zielverlust;
- Drag, Texteingabe, Preset und Copy durchlaufen nach Vertragsabschluss
  dieselbe ehrliche Draft/Kandidat/Confirmed-Autorität;
- nichtkritischer Hinweis während Drag/Preview/Textwert stiehlt keinen Fokus;
  Disconnect oder Statefehler beendet nur den betroffenen flüchtigen Pfad;
- wiederholte Controls behalten in idle, hover, pressed, selected und disabled
  dasselbe Layoutrechteck und denselben gelernten Zielort;
- mehrere Plugininstanzen, kleine Blocks und UI-Last.

### 12.3 Probeeq kompakt

- Broker nicht gestartet, inkompatibel, half-open und reconnect;
- unpaired, paired, Remote-Preview und bestätigter DSP;
- Hostautomation widerspricht Gen-Draft;
- EQ-Ein/Aus, Bypass, Instanz-Mix und Neutralize ohne Broker, ohne ihre
  Zustände zu vermischen;
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

### Ready für Zustandsentwurf

Eine Funktion darf als Wireframe oder Zustandsblatt untersucht werden, wenn:

1. menschlicher Job, Objekt, Scope und bekannte Autorität benannt sind;
2. belegte Festlegung, Hypothese und offene Vertragsstelle getrennt sind;
3. Erfolgs-, Invers-, Fehler- und unavailable-Zustand mindestens als ehrliche
   Lücke darstellbar sind;
4. persistente, kontextuelle, transiente und exceptional Informationen
   klassifiziert sind;
5. Zielgröße und schlimmster wahrer Zustand feststehen;
6. der Entwurf keine fehlende Capability und keinen noch offenen Handgriff als
   funktionsfähiges Control ausgibt.

Ein frühes Zustandsblatt darf also eine Vertragslücke sichtbar machen. Es darf
nicht von bereits gebauten Fixtures abhängen; sonst könnte es die Lücke erst
zeigen, nachdem Technik und UI sie schon still entschieden haben.

### Ready für native Implementierung

Vor produktivem UI-Code müssen zusätzlich vorhanden sein:

1. Capability und ehrlicher Unsupported-Fallback;
2. abgeschlossener Zustandsautomat einschließlich Invers- und Fehlerpfad;
3. persistente/transiente Felder samt Versionierung;
4. Testfixtures für Worst-Case-Zustände;
5. englische Arbeitsbegriffe ohne erfundene Produktzusage;
6. Pointer-, Tastatur-, Präzisions- und Accessibility-Weg;
7. messbarer Reaktions- und Audio-Sicherheitsvertrag;
8. abgenommener Zustandsentwurf und geklärte Quellenautorität.

### Done

Eine UI-Funktion ist erst fertig, wenn:

- jede sichtbare Aktion echte Technik erreicht;
- Confirmed, Draft, Automation und Fehler gegeneinander getestet sind;
- Fokus, Semantik, Werte, Grenzen und Reset nativ geprüft sind;
- die eine 950×538-Geometrie bei 100/125/150/200 Prozent visuell besteht und
  dieselben Funktionen erreichbar hält;
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
    command_ack.applied_dsp, EQ-Betriebszustand,
    probe_descriptor.host_bus_name/host_mixer_index und Contribution sind
    derzeit reserviert, nicht fertig nutzbar.
11. **[D]/[O] S31b-Quellen:** Der Planschritt S31b ist am 02.09.2026 (PR2)
    von den früheren Figma-, PNG- und Spielregel-Ständen auf diesen Blueprint
    plus eine noch abzunehmende visuelle Spezifikation umgestellt; der
    Planänderungssatz liegt in `docs/plan/plan.json` und ist im gerechneten
    `docs/PLAN-STAND.md` sichtbar. Offen bleibt genau diese visuelle
    Spezifikation: Maße, Typografie und Renderabnahme existieren heute nicht;
    bis zu ihrer datierten Abnahme beginnt die sichtbare Umsetzung von S31b
    nicht.
12. **[O] 10-s-Kandidatenvertrag:** Kandidaten-ID, Timer, Message, ACK,
    Reconnect, Kill und Ablauf sind vor P7-UI zu spezifizieren; preview_begin
    und persistentes apply_transaction reichen dafür heute nicht.
13. **[O] Router-Abgleich:** Der aktuelle Ein-Größen-Vertrag 950×538 ist seit
    01.09.2026 in DESIGN.md, UX-CONTRACT.md und dem maschinenlesbaren
    UI-Vertrag vom historischen 760×430-Artefaktbestand getrennt. Offen bleibt
    die vollständige Bereinigung ihrer Figma-/Golden-/Raster- und
    Legacy-Prototypbindungen vor der nativen Übernahme.
14. **[O] Direkte Edit-Transaktion:** Band-Drag, Textwert, Preset-Load und
    Curve-Copy brauchen eine gemeinsame, widerspruchsfreie Abbildung auf
    Preview, 10-s-Kandidat, Confirmed, Host-Geste, Cancel und Revert. Die
    Drei-Stufen-Geste für Proposals beantwortet diesen Detailvertrag noch
    nicht vollständig.
15. **[O] Live-Dynamic-Auslenkung:** Aktivierung, Ausschalten, Rückweg,
    Schließen und der persistente Konturmarker sind entschieden. Der aktuelle
    v3-Vertrag reserviert `Frame.band_dynamic_gain_db` für S26–28, liefert aber
    noch keine Feld-ID oder Runtime-Nutzlast. Vor nativer Bewegung von
    Bandpunkt und Kurvenzug muss der Probeeq-Featureframe-Weg beziehungsweise
    der lokale Master-Weg gebaut und gegen stale sowie nicht-endliche Werte
    abgesichert werden.
16. **[O] Band-Occupancy und Remove-Undo:** `enabled` ist im aktuellen
    109-Parametervertrag nur der reversible Band-Bypass. Für die abgenommene
    getrennte Remove-Aktion fehlen ein persistenter Belegungszustand, die
    atomare Slotfreigabe und die Einordnung in die direkte Edit-/Undo-
    Transaktion. Vor nativer Anzeige müssen Save/Load, Host-Automation,
    Migration und Wiederherstellung unter derselben festen Band-ID gemeinsam
    versioniert und getestet werden.
17. **[D]/[O] Global-Sektion und Wortlaute:** Die Global-Sektion auf Gen
    Fläche 2 ist am 02.09.2026 entschieden (Fünferblock 03, 2/5; Abschnitt
    4.2 Zone 4): die eingeklappte Zeile nennt nur aktive Abweichungen beim
    Namen, aufgeklappt liegen alle globalen Handgriffe samt Auto-Gain neben
    Output. Offen bleiben die Disclosure von Voreinstellungen und Verlauf
    innerhalb der Sektion, der Wortlaut der Halten-Aktion (Fläche 1 `HOLD TO
    AUDITION`, Fläche 2 `AUDITION · HOLD`) und die sichtbare Kurzform `DEL`
    für `Remove Band`, die nur eine Dichteprüfung der Skizze ist.
18. **[D] Flächenwechsel Gen:** entschieden am 02.09.2026 (Fünferblock 03,
    3/5 und 4/5; Abschnitt 4.1 „Übergang“): Der Kopf schaltet beide Flächen
    in beide Richtungen, `SEND DRAFT` bleibt Handgriff am Befund, keine
    automatische Rückkehr, ein offener Draft bleibt offen. Der Entscheid vom
    31.08.2026 „nur SEND DRAFT“ ist überholt.
19. **[D]/[O] Probeeq-Rückfallfläche:** Größenklasse entschieden am
    02.09.2026 (Fünferblock 03, 5/5; Abschnitt 4.3): flache Kachel mit ein
    bis zwei Zeilen, Zeile 2 nur bei zugeschaltetem EQ. Offen: ob die zweite
    Zeile die Fensterhöhe ändert oder in fester Höhe ein- und ausgeblendet
    wird (Host-Fenster unter FL), und die Detailfragen der Kachel (Verhalten
    bei Link-Verlust, lokale Notfallaktionen) aus einem kommenden Block.

Diese Punkte sind kein Freibrief für Platzhaltercontrols. Bis zur technischen
oder User-Entscheidung bleibt die jeweilige Funktion ehrlich unavailable oder
außerhalb der Oberfläche.

## 16. Quellenledger

### 16.1 Repository-Wahrheit

| Quelle | Verwendung |
|---|---|
| [CLAUDE.md](../../CLAUDE.md) | Produktidentität, Zwei-Flächen-Modell, harte Audio-/Hostinvarianten |
| [design/LIES-MICH.md](../LIES-MICH.md) | Funktionsneustart, Abgrenzung von früheren visuellen Ständen |
| [UX-CONTRACT.md](../../UX-CONTRACT.md) | aktueller Größen-/Verhaltensvertrag und weiterhin zu bereinigende Legacy-Prototypbindung |
| [DESIGN.md](../../DESIGN.md) | aktueller funktionaler Größenvertrag, getrennt von der früheren visuellen Autorität |
| [PLAN-STAND.md](../../docs/PLAN-STAND.md) | gerechneter Status, Tickets und aktuelle Nachträge |
| [FL-Nakama-Sonden-Design-Entwurf.md](../../docs/FL-Nakama-Sonden-Design-Entwurf.md) | technische Funktionen, Objekte, Zustände und P2–P9 |
| [bauaufteilung-sonden.md](../../docs/bauaufteilung-sonden.md) | Ticketgrenzen und Verantwortungen |
| [plugin-wissen.md](../../docs/plugin-wissen.md) | heutiger Codezustand und noch nicht verdrahtete Pfade |
| [SONDE-011.md](../../docs/beweise/SONDE-011.md), [SONDE-012.md](../../docs/beweise/SONDE-012.md) | Koordinator-/Session-Schicht und passive Landkarte, beide T2 PASS 01.09.2026; das Phasengate G2 steht auf NEEDS_WORK ([G2.md](../../docs/beweise/G2.md), NAK-123) |
| [fragen.json](../../docs/plan/fragen.json) | aktuelle Funktionsentscheide und offene Usertermine |

### 16.2 Bindende User-Entscheide

| Quelle | Verwendung |
|---|---|
| [Arbeitsfluss EQ-Zentrale, 23.08.2026](../abnahmen/2026-08-23-interview-struktur.md) | häufigster Zielwechsel, drei Stufen, zwei Curves, Master separat, Probeeq-Rückfall, Mix und globale Disclosure |
| [Auto-Gain, 24.08.2026](../abnahmen/2026-08-24-auto-gain-schalter.md) | Schalter, Standard aus, sichtbarer Output-Ausgleich |
| [Rollenpräzisierung, 27.08.2026](../abnahmen/2026-08-27-arbeitsnamen-und-probeeq-doppelrolle.md) | zwei Gen-Flächen, Probeeq-Doppelrolle, kein lokaler Voll-Editor |
| [Suna-Stilllegung, 28.08.2026](../abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md) | zwei Produktbundles plus Broker |
| [Marktstandard-Fragen, 30.08.2026](../abnahmen/2026-08-30-fragenrunde-marktstandard.md) | Busnamen-Fallback, Maskierungszone, Spectrum Grab/Freeze/Display, kein Piano/Linear Phase, Presets und Copy |
| [Gen nur Standardgröße, 01.09.2026](../abnahmen/2026-09-01-gen-nur-standardgroesse.md) | beide Gen-Flächen genau 950×538; Compact und Resize vertagt; UI-Scale vergrößert dieselbe Geometrie |

### 16.3 Wissensarchiv

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

### 16.4 Marktquellen

Alle Marktquellen wurden am 31.08. und 01.09.2026 in offiziellen
Herstellerseiten oder Handbüchern geprüft. Die Ableitungen sind keine
Behauptung über Herstellerintention oder Marktanteil.

| Quelle | Verwendete übertragbare Regel |
|---|---|
| [TDR Nova Manual](https://docs.tokyodawn.net/nova-manual/) | direkter Graphzugriff; Auswahl und Aktivierung getrennt; feste UI-Skalierung als anderer Mechanismus als zusätzliche Informationsfläche |
| [FabFilter Pro-Q 4 — Band Controls](https://www.fabfilter.com/help/pro-q/using/bandcontrols) | objektnahe Controls; Bypass, Delete und Stereo Placement als getrennte Bandhandlungen |
| [FabFilter Pro-Q 4 — EQ Display](https://www.fabfilter.com/help/pro-q/using/eqdisplay) | Parameterkontext am ausgewählten Punkt; Modifikatortasten nur als zusätzliche Schnellwege |
| [FabFilter Pro-Q 4 — Undo and Redo](https://www.fabfilter.com/help/pro-q/using/undoredo) | stabiler Wiederherstellungsweg für UI-Änderungen |
| [Kirchhoff-EQ Manual](https://files.plugin-alliance.com/products/tbt_kirchhoff-eq/tbt_kirchhoff-eq_manual.pdf) | Enable/Disable, Dynamic, Stereo Mode, Bandnavigation und Remove bleiben semantisch getrennt |

## 17. Laufende technische UI-Ableitung

Die gezielten grauen Funktionsblätter werden in einer fortlaufenden
Architektur-Fragenrunde konkretisiert. Es wird immer genau eine noch offene
räumliche oder bedienerische Frage gestellt; technische Grenzen werden vorher
aus den aktuellen Verträgen, Quellen und Tests abgeleitet. Antworten werden in
festen Fünferblöcken gesammelt. Nach der fünften Antwort werden die laufende
Skizze, die betroffenen Abnahmen und dieser Blueprint gemeinsam aktualisiert
und sichtbar geprüft; erst danach beginnt der nächste Block.

Der Größenentscheid vom 01.09.2026 wurde auf ausdrückliche Useranweisung
vorgezogen querschnittlich integriert und später am selben Tag auf genau eine
Standardgröße vereinfacht, damit weder Layoutarbeit noch Kontext in zwei
Varianten auseinanderlaufen. Das ändert den Fünfer-Takt für die folgenden
Fragen nicht.

Der jeweils laufende, noch nicht batchweise integrierte Entscheidungsstand
steht in der jüngsten Datei `technische-ui-architektur-fuenferblock-*.md` unter
`design/abnahmen/`. Bis zum Abschluss des Blocks kann er den Detailabschnitten
dieses Blueprints zeitlich voraus sein.

Gen Fläche 1 und die Grundhierarchie von Gen Fläche 2 besitzen den in Abschnitt
4 beschriebenen User-entschiedenen Stand. Fünferblock 01 integriert
Filtertyp-Mechanik, freie Panelverankerung, Dynamic-Aktivierung und
-Ausschalten, Rückweg, Schließen/Fokus sowie den sichtbaren Aktivbeleg.
Fünferblock 02 integriert atomaren Panelbesitz beim Bandwechsel, den
bandlokalen Kanalmodus, den stabilen Band-Bypass sowie die Trennung von
Disable und wiederherstellbarem Remove. Beide Blöcke wurden in der laufenden
Skizze und im internen Browser geprüft.

Fünferblock 03 (02.09.2026,
[2026-09-02-technische-ui-architektur-fuenferblock-03.md](../abnahmen/2026-09-02-technische-ui-architektur-fuenferblock-03.md))
integriert die Befund-Dichte auf Fläche 1 (drei Zeilen; Sicherheit als
Zustand, Beleg im Graphen, Alternativen als Liste), die Global-Zeile mit
benannten Abweichungen, den Flächenwechsel über den Kopf in beide Richtungen
ohne automatische Rückkehr und die flache Probeeq-Kachel. Vor der ersten
Frage stellte der User den Leitsatz auf, dass sechs Werte je Befund
schlechtes Design sind; vor der fünften das Prozesswort, dass jetzt das
Fundament in der Skizze entsteht und das visuelle Design danach in Figma.
Die Prüfung lief mit Playwright und Chrome (41 Prüfpunkte, Konsole leer);
die Belege liegen unter `design/skizze/belege/` mit Präfix
`2026-09-02-b03-`.

Die Skizze liegt seit dem 02.09.2026 im Repo unter
[design/skizze/nakama-ui-technical-sketch.html](../skizze/nakama-ui-technical-sketch.html);
bis dahin lag sie nur im Codex-Visualisierungsordner des PCs. Sichtbelege des
Stands vom 02.09.2026 liegen unter `design/skizze/belege/`. Sie ist der
derzeitige Stand der entschiedenen Architektur, nicht der finale (User-Wort
02.09.2026, festgehalten in der Arbeitsmodus-Abnahme vom 31.08.2026).

Der aktuelle Größenentscheid setzt beide Gen-Flächen auf genau eine logische
Standardgröße von 950×538. Compact, Fenster-Resize und größenabhängiges Reflow
sind bis nach Fertigstellung vertagt; UI-Skalierung vergrößert dieselbe
Geometrie. Probeeq bleibt von diesem Entscheid unberührt.

Die fortlaufende Dynamic-Auslenkung und die native Remove-/Undo-Transaktion
bleiben benannte technische Vertragslücken, keine offenen Geschmacksfragen.
Vor einer weiteren Architekturfrage wird ein neuer Block aus einem wirklich
noch offenen Bedienbereich begonnen. Objektbesitz, Fokus, Tastaturweg und
Worst-Case-Zustände werden weiterhin am passenden Blatt geprüft. Farben,
Material und visuelle Feinheiten bleiben bis zum ausdrücklichen Phasenwechsel
getrennt.
