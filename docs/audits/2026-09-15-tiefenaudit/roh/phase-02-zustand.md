URTEIL: DEFEKTE 4 · LÜCKEN 2 · HÄRTUNGEN 1 — Die Feldinventur ist geschlossen; an gültigen Randständen und ungefragten Rückschreibwegen bleiben konkrete State- und Vertragsbrüche.

# Tiefenaudit 3 — Phase 02: Zustandstreue

## Kopf und Beweisgrenze

- Auftrag: gebundener lesender Prüfauftrag des Dirigenten vom 15.09.2026, 20:00 Uhr.
- Prüfer: Codex gpt-6-astra, Effort max; eigener Skeptikerpass.
- Branch: `master`; gebundener HEAD: `3f2619c74d39483c9487296667b2bfa9e4246e75`.
- Beginn: 15.09.2026, 20:00:29 Europe/Berlin; Ende und Dauer siehe Abschlussvermerk.
- Verfahren: Quellen vollständig für den State-Kern, Feldinventur der vorhandenen Fixtures, gezielte Lektüre der Host-, Editor- und Wire-Kanten, Vergleich mit vorhandenen Testquellen und Register.
- **Kein Produktpfad wurde ausgeführt.** Kein Build, Test, Kanon, Brokerstart, Hostlauf oder Installationslauf. „Quellbeweis“ bedeutet eine zwingend hergeleitete Folge des gelesenen Codes; vorhandene Testtexte sind Gegenbelege und Zusagen, keine frischen Testergebnisse.
- Schreibumfang: ausschließlich dieser Bericht und der eigene Anhang in `BEFUNDE.md`. Keine Produktänderung, kein Stage, Commit oder Push.
- Kontextdokumente, Register und Manifeste wurden als Zusagen und zur Dublettenprüfung verwendet. Sie sind keine Befundfläche. Die vorhandene Oberfläche wurde ausschließlich an ihren State-Schreibaufrufen gelesen.

## Befundtabelle

| ID | Schwere | Klasse | Kategorie | Ort | Befund | Beleg | Bricht | Vorschlag | Status |
|---|---|---|---|---|---|---|---|---|---|
| T3-02-01 | high | K4 | DEFEKT | eq-copilot/plugin/src/prozessor/Analyse.cpp:1299; eq-copilot/plugin/src/prozessor/Analyse.cpp:1315; eq-copilot/plugin/state/NakamaState.cpp:1905 | bekannt NAK-283: Zusätzlich zu den geschlossenen F11-Stellen inkrementiert assistentVersuchStarten die geladene Assistentenrevision ohne Obergrenze; int64max erreicht nach erfolgreichem Experimentbeginn signed-integer-Überlauf. | Quellkette: gültiger offener assistant_step_v1 mit Revision 9223372036854775807 → beginneVersuch erfolgreich → experimentId-Zuweisung → revision += 1. M-10-Tests fahren nur die drei State-Helfer; der Prozessoreinstieg fehlt. | „Kein Überlauf, kein unlesbarer Stand.“ (NAK-283, M-10); „State bleibt verlustfrei.“ (CLAUDE.md:198) | Revision vor dem Experimentbeginn prüfen und die anschließende State-Änderung über den gemeinsamen geprüften Mutator führen. | offen |
| T3-02-02 | critical | K4 | DEFEKT | eq-copilot/plugin/state/NakamaState.cpp:85; eq-copilot/plugin/state/NakamaState.cpp:821; eq-copilot/plugin/state/NakamaState.cpp:1116; eq-copilot/plugin/state/NakamaState.cpp:1503 | bekannt NAK-252: Der Verdacht ist um einen vollständig spezifizierten, schreibbar akzeptierten Grenzstand ergänzt: 65.536 verschiedene Common-Eigenschaften ohne label werden beim Save zu 65.537; der eigene nächste Load ignoriert den ganzen State. | Quellbeweis ohne Ausführung: Root schema=2; Common mit schema, instance_id, plugin_kind=legacy, measurement_position=insert sowie 65.532 verschiedenen unbekannten int32-Eigenschaften. Alle Eingangsgrenzen halten; der Writer ergänzt label, die Headroomprüfung prüft ausschließlich Bytes. | „Der eigene Reader nimmt jeden selbst erzeugten Stand an, und ein abgewiesener Handgriff verändert die gespeicherten Bytes nicht.“ (NAK-283, M-11); „State bleibt verlustfrei.“ (CLAUDE.md:198) | Alle Writer-Headroom-Kandidaten auch gegen die tatsächlichen Struktur-, Tiefen- und Sammlungsgrenzen des Byte-Lesers prüfen. | offen |
| T3-02-03 | high | K3 | DEFEKT | eq-copilot/plugin/state/NakamaState.cpp:1696; eq-copilot/plugin/src/prozessor/Ipc.cpp:586; eq-copilot/plugin/src/prozessor/Ipc.cpp:601; eq-copilot/plugin/src/prozessor/Ipc.cpp:674; eq-copilot/plugin/core/ipc/controlclient/Intern.h:443; broker/src/vertrag.rs:244 | State und v3 haben verschiedene Revisionsbereiche: gültig geladene Gen-Revisionen ab 9007199254740992 werden vom Broker verworfen; Probeeq meldet solche state_revision-Werte stattdessen als 9007199254740991. | Vier Felder verfolgt: Intent-Bestandsrevision, einzelne SourceIntent-Revision, Assistentenrevision und Dsp.state_revision. Binärleser akzeptiert int64; Gen schreibt Dezimaltext, Broker-Textriegel begrenzt auf 2^53−1; stateReportJson verwendet min statt einer verlustfreien Abbildung. | „Main meldet seinen vollständigen Intent-Bestand — auch einen leeren — mit einer Vollständigkeitsmarke samt Revision, bevor der Broker eine Hypothese oder ein Proposal rechnet.“ (SONDE-014, M-86; Markdown-Hervorhebung entfernt) | Einen gemeinsamen verlustfreien Revisionsvertrag für State, Mutatoren und Wire festlegen; nicht sendbare Altstände ausdrücklich behandeln und niemals eine andere Revision ausgeben. | offen |
| T3-02-04 | critical | K4 | DEFEKT | eq-copilot/plugin/src/PluginEditor.cpp:683; eq-copilot/plugin/src/PluginEditor.cpp:688; eq-copilot/plugin/src/PluginEditor.cpp:705; eq-copilot/plugin/src/prozessor/State.cpp:307; eq-copilot/plugin/state/NakamaState.cpp:1503 | Unverändertes Schließen des Messpunktpanels kann gültig geladene Common-Daten überschreiben: label wird auf 120 und pair_id auf 60 Zeichen gekürzt; außerdem kann eine leere Main-Projektbindung dabei ungefragt entstehen. | Quellweg: gültiger Legacy-pre-State mit 121 ASCII-Zeichen label und/oder 61 Zeichen pair_id → Panel übernimmt Originaltexte → Destruktor ruft uebernehmen → substring → setzeBindung → Dirty und nächster Save enthalten gekürzte Werte; kein Edit nötig. | „State bleibt verlustfrei.“ (CLAUDE.md:198) | Geladene Originalwerte beim unveränderten Schließen erhalten und setzeBindung nur für einen tatsächlich ausgeführten Änderungs- oder Initialisierungshandgriff aufrufen. | offen |
| T3-02-05 | medium | K4 | LÜCKE | eq-copilot/plugin/state/NakamaState.cpp:281; eq-copilot/plugin/state/NakamaState.cpp:2117; eq-copilot/build/_deps/juce-src/modules/juce_data_structures/values/juce_ValueTree.cpp:1069; eq-copilot/build/_deps/juce-src/modules/juce_core/containers/juce_NamedValueSet.cpp:187 | Doppelte Eigenschaftsnamen im binären Host-State werden akzeptiert und still auf den letzten Wert reduziert; eine ausdrückliche Regel für solche mehrdeutigen Fremdbytes fehlt. | Zwei unbekannte Common-Eigenschaften future=A und future=B passieren den Byte-Riegel; JUCE properties.set ersetzt A durch B vor dem Nakama-Leser. Save enthält nur B. JSON-Duplikat- und doppelte-Common-Kind-Tests decken diesen Fall nicht. | keine Zusage | Doppelte Namen pro ValueTree-Knoten vor JUCE erkennen und für bekannte Wurzeln verlustfrei als read-only mit Originalbytes halten. | offen |
| T3-02-06 | medium | K4 | LÜCKE | eq-copilot/plugin/src/prozessor/State.cpp:320; eq-copilot/plugin/state/NakamaState.cpp:1009; eq-copilot/plugin/state/NakamaState.cpp:2018 | Beim Wechsel main → legacy bleiben sechs MainProject-Bestände im Speicher, verschwinden aber aus dem Save; die Rückkehr nach main stellt sie nur ohne zwischenzeitlichen Reload wieder her. | Rollenwahl hub → sensor → hub erhält Passagen, Intents, Schutzangaben, Beziehungen, Bestandsrevision und Assistent im selben Prozessor. Mit Save/Load im sensor-Zustand setzt leseSchema2 sie leer. Nur confirmed_members_v1 wird bereits beim Rollenwechsel gelöscht. | keine Zusage | Eine gemeinsame Regel für das Behalten oder Löschen von MainProject beim Rollenwechsel festlegen und in Live-State sowie Save/Load gleich anwenden. | offen |
| T3-02-07 | low | K4 | HÄRTUNG | eq-copilot/plugin/state/NakamaState.cpp:824; eq-copilot/plugin/tests/StateMigrationTestMain.cpp:1003; eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp:2639; tools/eq-copilot/erzeuge_state_fixtures.py:1084 | bekannt NAK-267: Zusätzlich zu den drei A-3-Feldern fehlt Common.project_binding_id in sämtlichen eingefrorenen State-Binärfixtures; der vorhandene Prozessor-Roundtrip ersetzt keine eingefrorene historische Eingabe. | Alle 8 schema2-Binärfixtures feldweise gelesen: 132 von 136 benannten Property-Orten vertreten; project_binding_id fehlt gemeinsam mit den bereits registrierten drei MainProject-Feldern. Vier Legacy-Identity-Blobs enthalten ebenfalls keine Projektbindung. | keine Zusage | Ein vom Produktwriter erzeugtes Fixture mit gesetzter project_binding_id einfrieren und dessen Feldpräsenz neben dem vorhandenen Roundtrip sichern. | offen |

## Herleitung und Skeptikerpass je Befund

### T3-02-01 — ein vierter Assistenten-Inkrementweg außerhalb des reparierten Helfers

**Eingang:** Ein lesbarer Main-State enthält einen gesetzten, offenen Assistentenschritt mit Revision `9223372036854775807`, eine gültige Projektbindung und eine manuelle Passage. Der Binärleser fordert lediglich einen ganzzahligen Wert ≥ 1 (`NakamaState.cpp:1905`). Die hohen Revisionen entstehen hier durch den ausdrücklich unterstützten Ladeweg; der Beweis setzt keine astronomische Zahl von Klicks voraus.

**Kette:** `assistentVersuchStarten` prüft zunächst nur gesetzt/offen (`Analyse.cpp:1299`). `beginneVersuch` prüft Main/read-only, Passage, laufenden Versuch, gebundenes Messfenster, Vergleichspegel und Wirekopf (`:790` bis `:925`), aber nicht die Assistentenrevision. Nach angenommener P0-Einreihung setzt es `versuchIdAktiv` und liefert true (`:925` bis `:931`). Anschließend schreibt der Aufrufer `experimentId` und führt `revision += 1` aus (`:1314`, `:1315`).

Damit ist signed-integer-Überlauf zwingend erreicht. **Kein konkreter negativer Laufzeitwert wird für diese Session behauptet:** C++ definiert den Überlauf nicht. Ein Überlauf wie im historischen F11-Beleg ließe den eigenen Reader wegen Revision < 1 auf read-only fallen. Bereits die ungeschützte Operation bricht M-10. Die Einstufung high bezieht sich auf den akzeptierten State und Vertragsbruch; ein konkreter Absturz wurde nicht gemessen.

**Skeptiker:** `NakamaState.cpp:2677` schützt die drei Bibliothekswege. `Sonde014IntentTest.cpp:1488` prüft genau diese drei; `StateMigrationTestMain.cpp:2360` prüft den Roundtrip nach solchen Handgriffen. `Sonde014AssistentTest.cpp:579` prüft den Experimentstart ohne erfüllte Messvoraussetzungen. Der erfolgreiche bestehende Prozessoraufbau steht dagegen in `Sonde013PassageStateTest.cpp:3174` bis `:3235`; er ruft `beginneVersuch`, nicht den Assistentenwrapper am Revisionsmaximum. Neu gegenüber **NAK-283/F11** ist nur dieser zusätzliche Prozessoreinstieg. Ein Fix erst nach `beginneVersuch` käme zu spät: der Versuch wäre bereits begonnen.

### T3-02-02 — vollständig beschriebener Gegenstand zum bisherigen Headroom-Verdacht

Der folgende **Binärbaum ist eine Spezifikation für den späteren Reproduktionstest**, keine in dieser Session angelegte oder ausgeführte Fixture:

```text
NakamaState
  schema: int32(2)
  genau ein Kind Common:
    schema: int32(1)
    instance_id: string("11111111111111111111111111111111")
    plugin_kind: string("legacy")
    measurement_position: string("insert")
    x00000 ... x65531: jeweils int32(0), alle Namen verschieden
    keine label-, pair_id- oder project_binding_id-Eigenschaft
    keine Kinder
```

- Common trägt **4 + 65.532 = 65.536** Eigenschaften. Zusammen mit Root-Eigenschaft und Root-Kind zählt der Byte-Riegel **65.538** Einträge; Tiefe ist 2. Keine Arrayverschachtelung, ungültige UTF-8-Folge oder unbekannte Variante ist nötig.
- JUCE-Längenrechnung: 41 Bytes bis zu den Common-Eigenschaften; die vier bekannten Eigenschaften 115 Bytes; jeder der 65.532 unbekannten Einträge 14 Bytes; abschließender Kinderzähler 1 Byte. **917.605 Bytes** insgesamt. Das ist eine Rechnung aus der gelesenen Codierung, kein gemessener Produktwriter-Output.
- `label` darf fehlen (`NakamaState.cpp:1503`). Die Klasse/Kind-Matrix für legacy+insert ist erfüllt (`:1494`, `:1519`). Die Headroom-Kandidaten bleiben auch mit ihren bekannten Maximalfeldern weit unter 16 MiB.
- `hatWriterHeadroom` serialisiert die Kandidaten und vergleicht nur ihre Bytezahl (`:1114` bis `:1116`). Die 65.536er-Grenze des eigenen Vorlesers wird dabei nicht angewandt.
- `synchronisiert` ergänzt das leere `label` unbedingt (`:821`): **65.537** Common-Eigenschaften, rechnerisch **917.615 Bytes**. Der nächste `lade`-Aufruf fällt vor JUCE am Sammlungszähler durch (`:278`, `:2108`) und liefert **ignoriert**, nicht read-only mit Originalbytes.
- Beide Prozessoren behalten bei ignoriert ihren bisherigen State (`src/prozessor/State.cpp:147`; `sonde/SondeProcessor.cpp:1028`). Eine neue Instanz erhält somit den gespeicherten Projektinhalt nicht; ein folgender Save kann ihren Default/Altstand an dessen Stelle schreiben.

**Skeptiker:** `StateMigrationTestMain.cpp:333` baut für die Sammlungsgrenze wiederholte Namen `x` ohne gültigen Common-Unterbaum. Damit wird kein schreibbar angenommener Grenzstand durch Save/Load geführt. Der Headroom-Test ab `:2096` sucht die **Bytegrenze** mittels großem Ballast. **NAK-252** beschreibt bereits den Mechanismus und verlangt einen vollständigen Grenzfall; die zusätzliche Leistung hier ist dessen konkrete, widerspruchsfreie Spezifikation einschließlich aller Lesergrenzen und Folgezustand. Der tatsächliche Read/Write/Read-Lauf bleibt wegen des Auftragsverbots offen.

### T3-02-03 — persistente Revision und Wire-Revision meinen nicht dieselbe Zahl

Die vollständige Revisionsteilpopulation an dieser Naht umfasst vier persistente Felder:

| State-Feld | State-Leser | Sender | tatsächliche Wire-Wirkung |
|---|---|---|---|
| `MainProject.intent_revision_v1` | `NakamaState.cpp:1851`: int64 ≥ 1, fehlend = 0 | `Ipc.cpp:586` | unveränderter Dezimaltext; ab 2^53 abgewiesen |
| Revision jedes `source_intents_v1`-Eintrags | `NakamaState.cpp:1696`: int64 ≥ 1 | `Ipc.cpp:601` | unveränderter Dezimaltext; ab 2^53 abgewiesen |
| Revision in `assistant_step_v1` | `NakamaState.cpp:1905`: int64 ≥ 1 | `Ipc.cpp:674` | unveränderter Dezimaltext; ab 2^53 abgewiesen |
| `Dsp.state_revision` | `NakamaState.cpp:1308`: int64 ≥ 0 | `SondeProcessor.cpp:1109` → `controlclient/Intern.h:443` | `min(r, 9007199254740991)`: mehrere verschiedene Revisionen werden gleich gemeldet |

Minimaler Gen-Fall: lesbarer, gebundener Main mit **leerem** Intent-Bestand und `intent_revision_v1 = 9007199254740992`. Es ist kein Audio und kein großer Intent-Bestand nötig. Der Reconnect baut den Vollbericht aus diesem State. `sendeP1` prüft die Payloadgröße, nicht diesen Zahlenbereich (`controlclient/Nachrichten.cpp:402`). Der Broker wendet den Textriegel vor der JSON-Schemaprüfung an (`broker/src/coordinator/schema.rs:422`); `broker/src/vertrag.rs:244` verwirft das Literal. Damit kann die zugesagte Vollständigkeitsmeldung diesen gültigen State nicht darstellen. Entsprechendes gilt für den Assistentenspiegel.

**Skeptiker:** Die v3-Grenze ist bewusst und sprachübergreifend festgelegt (`schemas/v3/README.md:62`); sie wird hier nicht als Parserfehler beanstandet. Die int64-Maximum-Roundtrips aus NAK-283 beweisen in ihrer Testabsicht nur Persistenz, nicht Transport. Die Sonde verwendet **Sättigung, keinen Modulo-Umlauf**. Fehlende `state_report.dsp`-Nutzlast ist dagegen **NAK-110 / Etappe 4b** und kein neuer Befund dieses Berichts. Der Fehler besteht bereits an der vorhandenen Revisionsausgabe.

### T3-02-04 — Rückschreiben ohne ausgeführten Änderungshandgriff

Der Common-Leser bewahrt lange bestehende Texte: `label` muss nur ein String sein, `pair_id` bei Präsenz ein nichtleerer String (`NakamaState.cpp:1503` bis `:1511`). Die Beschreibung unterscheidet diese Lesbarkeit ausdrücklich von den Grenzen des **eigenen** Eingabewriters. Auch die Schema-1-Migration übernimmt beide Texte ohne Kürzung (`:2077`).

**Minimalfall:** Gültiger legacy+pre-State, label = 121-mal ASCII `L`, pair_id = 61-mal ASCII `P`. Keine Grenzgröße, kein ungültiger Zeichensatz. Das Messpunktpanel befüllt seine Textfelder mit den vollständigen geladenen Werten (`PluginEditor.cpp:683`). Beim unveränderten Schließen ruft sein Destruktor `uebernehmen` (`:688`); dort werden 120/60 Zeichen abgeschnitten (`:705`, `:706`). `setzeBindung` erhält nun andere Werte, übernimmt sie und meldet Dirty. Das nächste Speichern enthält die gekürzten Strings. **Die verlorenen Zeichen und der fehlende ursprüngliche User-Edit begründen critical nach der Schwerevorgabe des Auftrags; Audio ist nicht betroffen.**

Derselbe unbedingte Rückschreibweg wirkt auf weitere Common-Felder:

- Bei geladenem Main ohne `project_binding_id` erzeugt `setzeBindung` eine neue Bindung (`State.cpp:315`), obwohl im Panel keine Rolle geändert wurde.
- Bei sensor/hub wird eine geladene nichtleere `pair_id` vom Panel unabhängig von einer Bearbeitung durch leer ersetzt (`PluginEditor.cpp:704`, `:706`).

**Skeptiker:** Der Gleichheitsriegel in `State.cpp:318` schützt nur bereits gleich übergebene Werte. Er kann die vorherige Kürzung oder Umformung nicht erkennen. `StateMigrationTestMain.cpp:2276` prüft einen unveränderten **API-Aufruf**; `:2293` prüft die Ablehnung zu langer **API-Eingaben**. Beide umgehen das Panel und den Unterschied zwischen Originaltext und Rückschreibtext. Registersuche nach substring, 120/60, Labelverlust, Schließen/Dirty ergab keinen passenden bekannten Punkt. Es geht um das Verhalten der vorhandenen State-Bedienung, nicht um Design oder Optik.

### T3-02-05 — Fremdbytes mit doppelten Eigenschaftsnamen

Für einen ansonsten gültigen Common-Knoten seien zwei binäre Eigenschaftseinträge `future = "A"` und `future = "B"` hintereinander vorhanden. Die Längen und UTF-8-Folgen sind gültig. Der Nakama-Vorleser prüft jeden Namen und jede Variante einzeln (`NakamaState.cpp:281`), führt aber keine Menge schon gelesener Namen. JUCE verwendet anschließend `NamedValueSet::set` (`juce_ValueTree.cpp:1069`, `juce_NamedValueSet.cpp:187`): der spätere Wert ersetzt den früheren. Der Nakama-Leser sieht nur noch eine Eigenschaft und stuft den State normal ein.

Der nächste Save kann `A` und die ursprüngliche Zweifachbelegung nicht zurückgeben. Doppelte bekannte Eigenschaften können entsprechend die Versionsauslegung mehrdeutig machen. **Als LÜCKE eingeordnet:** Ein regulärer JUCE-ValueTree-Writer erzeugt solche doppelten Namen nicht; keine gelesene Zusage definiert ausdrücklich die Behandlung dieses syntaktisch vollständigen, semantisch mehrdeutigen Fremdblobs. Der erreichbare Eingang ist der rohe Host-State, keine Sabotage an Repoquellen.

**Skeptiker:** `StateMigrationTestMain.cpp:1963` prüft zwei **Common-Kinder**, nicht zwei Eigenschaftsnamen. JSON-Duplikate werden im DTO/Preset bereits gesondert verworfen; `dto/ungueltig/doppelter-schluessel.json` ist kein binäres State-Fixture. Nicht verlustfrei lesbare Variantenmarker werden vor JUCE read-only gehalten (`NakamaState.cpp:2111`); für Namensduplikate fehlt diese Behandlung.

### T3-02-06 — Rollenrückkehr hängt vom Reload ab

Für einen Main mit gefüllten Passagen, musikalischem Intent und offenem Assistenten führt der **vorhandene** Rollenpicker `setzeBindung("sensor", ...)` aus. Der Setter leert ausschließlich `mainProjectMitglieder` (`State.cpp:321`). Die übrigen sechs State-Bestände bleiben stehen:

1. `manuellePassagen`
2. `sourceIntents`
3. `schutzangaben`
4. `intentBeziehungen`
5. `intentBestandRevision`
6. `assistent`

Der Serializer entfernt bei legacy das gesamte MainProject-Kind aus seiner **Kopie** (`NakamaState.cpp:1009`). Ein anschließendes Zurückschalten nach hub ohne Reload serialisiert die sechs alten Bestände wieder. Mit Save/Load als sensor werden die sechs Werte im Leser stattdessen aus leeren lokalen Defaults übernommen (`:2018` bis `:2023`), und sie sind nach der Rückkehr nach hub weg. `project_binding_id` bleibt in Common bestehen.

**Skeptiker:** Die Kind-Matrix verbietet MainProject für legacy; dessen Weglassen aus dem Legacy-Save ist für sich richtig. Die bestehende Rollenwechselprüfung (`StateMigrationTestMain.cpp:2271`) fährt pre → post → hub, ohne gefüllten MainProject-Rückweg. Es fehlt eine Regel zur Lebensdauer des Projektinhalts beim bewussten Verlassen und Wiederbetreten der Main-Rolle. Deshalb LÜCKE, keine erfundene Pflicht zum Aufbewahren oder Löschen. Siehe Produktfrage P-01.

### T3-02-07 — zusätzliche fehlende historische Eingabe

Alle **acht** eingefrorenen Dateien unter `fixtures/state/schema2/` wurden als Binärinhalt gelesen, einschließlich aller Eigenschaftsnamen und verschachtelten Werte; die Dateigrenze wurde bei jedem vorhandenen Blob vollständig erreicht. Alle vier Schema-1-Identity-Blobs wurden ebenfalls gelesen. Ergebnis der Vereinigungsmenge: **132 von 136** benannten Schema-2-Property-Orten kommen vor.

Nicht vorhanden sind:

- `Common.project_binding_id` — **zusätzlicher Befund**
- `MainProject.confirmed_members_v1` — bereits NAK-267/A-3
- `MainProject.manual_passages_v1` — bereits NAK-267/A-3
- `MainProject.assistant_step_v1` — bereits NAK-267/A-3

**Skeptiker:** Ein gesetzter Binding-Wert wird im echten Prozessor-Roundtrip geprüft (`Sonde012ProjectReloadTest.cpp:2639`). Das alte Migrationsgolden soll die Bindung dagegen gerade **nicht** erfinden (`StateMigrationTestMain.cpp:1013`). Diese beiden Tests werden nicht als fehlend ausgegeben. Es fehlt nur die eingefrorene positive historische Eingabe für die gesetzte Bindung; `erzeuge_state_fixtures.py:1084` führt weiterhin dieselben acht Binärgoldens. Manifest und Abdeckungskarte wurden nicht als fehlerhafte Dokumente bewertet.

## Population und Feldparität

### Zählregel

Gezählt wird jeder benannte Property-Ort im Schema-2-Baum, einschließlich der fünf versionsgebenden `schema`-Eigenschaften: **1 Root + 7 Common + 8 MainProject + 114 Parameters + 6 Dsp = 136**. Eine Property mit einer Liste ist ein benannter Ort; ihre vollständigen inneren Records stehen zusätzlich unten. Die fünf Schema-1-Root-Eigenschaften bilden eine getrennte Altpopulation. Die 120 DSP-Zellen sind **112 Hostparameter + 8 Belegungsbits**, keine 120 verschiedenen Hostparameter.

Kurzpfade in den folgenden Tabellen sind relativ zu `eq-copilot/plugin/state/`, sofern ein anderer Teilbaum nicht genannt ist.

### Alle nicht parametrischen Property-Orte und ihre Gegenwege

| Feld / vollständige Gruppe | Anzahl | Schreiber | Leser | Migration, Fixture und Feldwache |
|---|---:|---|---|---|
| `NakamaState.schema` | 1 | `NakamaState.cpp:809` | `:1448` | 2; Schema-1-Migrator `:2082`; alle 8 State-Bins, davon ein absichtlich fremdes Major |
| `Common.schema` | 1 | `:817` | `:1473` | 1; alle 8 State-Bins |
| `Common.instance_id` | 1 | `:818` | `:1480` | Legacy-`sensor_id` bleibt erhalten; alle 8 State-Bins; Duplicate-/Recall-Fälle in `tests/StateMigrationTestMain.cpp:2240` |
| `Common.plugin_kind` | 1 | `:819` | `:1484` | geschlossene Klasse; vier Schema-1-Rollen; alle 8 State-Bins |
| `Common.measurement_position` | 1 | `:820` | `:1489` | Klassen-/Positionsmatrix `:1498`; alle 8 State-Bins |
| `Common.label` | 1 | `:821` | `:1503` | leer/fehlend lesbar; vorhandene lange Werte bleiben; alle 8 State-Bins; T3-02-02/04 betreffen Folgeschreiber |
| `Common.pair_id` | 1 | `:822` | `:1506` | fehlend = kein Paar; präsent nichtleer; pre/post-Goldens; T3-02-04 |
| `Common.project_binding_id` | 1 | `:824` | `:1512` | fehlend = ungebunden, sonst lowercase hex32; Migration erfindet nichts; Prozessor-Recall vorhanden, kein eingefrorenes positives Binärfixture (T3-02-07) |
| `MainProject.schema` | 1 | `:835` | `:1530` | 1; `aus-schema1-hub.bin`, `main-intent-v1.bin` |
| `confirmed_members_v1` | 1 | `:836`, `:854` | `:1539` | fehlend = leer; Prozessor-Roundtrip `Sonde012ProjectReloadTest.cpp:2639`; Fixturelücke bekannt NAK-267 |
| `manual_passages_v1` | 1 | `:860`, `:882` | `:1586` | fehlend = leer; `Sonde013PassageStateTest.cpp:331` und Lesergrenzen; Fixturelücke bekannt NAK-267 |
| `source_intents_v1` | 1 | `:893`, `:917` | `:1653` | fehlend = leer; `main-intent-v1.bin`; Intent-Test einschließlich 256er-Bestand |
| `intent_protections_v1` | 1 | `:920`, `:945` | `:1734` | fehlend = leer; `main-intent-v1.bin`; API und Reader teilen Bandregel `:344` |
| `intent_relations_v1` | 1 | `:948`, `:971` | `:1793` | fehlend = leer; `main-intent-v1.bin`; Eindeutigkeit/Zyklusprüfung |
| `intent_revision_v1` | 1 | `:979`, `:982` | `:1843` | 0 durch Abwesenheit, sonst ≥ 1; `main-intent-v1.bin`; M-07–12; T3-02-03 |
| `assistant_step_v1` | 1 | `:991`, `:1006` | `:1877` | Abwesenheit = nie gesetzt; Assistenten-Roundtrip `Sonde014AssistentTest.cpp:728`; Fixturelücke bekannt NAK-267; T3-02-01/03 |
| `Parameters.schema` | 1 | `:1022` | `:1976` | 1; `layout-v1.bin`, `dsp-v2-voll.bin` |
| `Parameters.dsp_schema_version` | 1 | `NakamaParameter.cpp:570` | `:628` | fehlend/1 = Layout v1; 2 = v2; fremd = read-only; positives Feld in `dsp-v2-voll.bin` |
| 112 einzelne `Parameters.<ID>` | 112 | `NakamaParameter.cpp:573` | `:659` | jede ID unten ausgeschrieben; `layout-v1.bin` trägt 109, `dsp-v2-voll.bin` alle 112 |
| `Dsp.schema` | 1 | `NakamaState.cpp:1051` | `:2002` | 1; `dsp-v2-voll.bin` |
| `Dsp.state_revision` | 1 | `:1052` | `:1304` | bei Dsp Pflicht; int64 ≥ 0; Default ohne Dsp 0; `dsp-v2-voll.bin`; T3-02-03 |
| `Dsp.occupied_v1` | 1 | `:1054` | `:1313` | genau acht bools; v1-Ableitung `NakamaParameter.cpp:164`; `dsp-v2-voll.bin` |
| `Dsp.schutz_zonen_v1` | 1 | `:1059` | `:1260` | fehlend = leer; vier Werte je Zone; `dsp-v2-voll.bin` und DTO-/Preset-Zonenfixtures |
| `Dsp.undo_ring_v1` | 1 | `:1064` | `:1338` | fehlend = leer; bis 32 innere Arrays; `dsp-v2-voll.bin`; voller Ring im Roundtriptest |
| `Dsp.undo_cursor` | 1 | `:1075` | `:1422` | ohne Ring unzulässig; bei Ring 0…Ringgröße; `dsp-v2-voll.bin` |
| **Summe Schema 2** | **136** | | | **132 Property-Orte in den eingefrorenen State-Bins vertreten** |
| Schema 1: `schema`, `sensor_id`, `role`, `label`, `pair_id` | **5** | historischer Writer; vier Identity-Blobs | `NakamaState.cpp:2054` | reine Abbildung nach Schema 2; kein eigener Parameterbestand im alten EqCopilotState |

Die unbekannte Menge ist offen und daher nicht endlich aufzählbar. Ihre **gesamte Behandlungsklasse** wurde verfolgt: zusätzliche Eigenschaften im bekannten Baum bleiben in `Zustand.baum`, `synchronisiert` editiert eine Kopie, nicht verlustfrei lesbare Varianten führen auf Originalbytes. Unbekannte Kinder/Majors und unerlaubte Matrizen führen auf read-only; **nicht jede unbekannte Struktur ist ein erlaubtes Minor-Addendum**. Namensduplikate sind der gesonderte Fall T3-02-05.

### Vollständige innere Record-Population

| Record | Vollständige Feldfolge | Grenze / Typ / Präsenz | Schreiber ↔ Leser |
|---|---|---|---|
| Mitglied | instance_id, label | bis 64; hex32 eindeutig, String ≤ 120 Codepoints | `NakamaState.cpp:849` ↔ `:1550` |
| Manuelle Passage | passage_id, label, projekt_start, projekt_ende | bis 64; hex32 eindeutig, Label ≤ 120; int/int64, 0 ≤ start < end ≤ int64max | `:875` ↔ `:1597` |
| SourceIntent | quelle_id, passage_id, rolle, revision, herkunft, konfidenz | bis 256; Quelle hex32, Scope leer oder hex32; fünf Rollen, drei Herkünfte, Revision ≥ 1, endliche Konfidenz [0,1]; (Quelle,Scope) eindeutig | `:908` ↔ `:1666` |
| Schutzangabe | quelle_id, eigenschaft, band_von, band_bis | bis 256; vier Eigenschaften; bei band: 0 ≤ von < bis ≤ 221; sonst -1/-1; Tupel eindeutig | `:938` ↔ `:1747`, gemeinsamer Riegel `:344` |
| Beziehung | quelle_a, quelle_b, art | bis 256; unterschiedliche hex32, drei Arten; keine doppelte Kante, kein führt-vor-Zyklus | `:965` ↔ `:1807` |
| Assistent | step_id, schritt, revision, offen, finding_id, proposal_id, experiment_id, ergebnis | genau acht Positionen; gesetzter Schritt durch Property-Präsenz; IDs hex32 bzw. drei Referenzen leer; Revision ≥ 1; offen bool; preview in P5 abgewiesen; geschlossenes Ergebniswort | `:998` ↔ `:1885` |
| Belegung | occupied[0], occupied[1], occupied[2], occupied[3], occupied[4], occupied[5], occupied[6], occupied[7] | genau acht bools, kein Hostparameter | `:1055` ↔ `:1321` |
| Schutzzone | id, low_hz, high_hz, enabled | bis 8; id int 0…7, eindeutig/streng sortiert; double 20 ≤ low < high ≤ 20000; enabled bool | `:768` ↔ `:1270` |
| Undo-Eintrag | art, slot, revision, **alle 120 DSP-Zellen in Vertragsreihenfolge**, anschließend 0…8 Zonen mit jeweils denselben vier Feldern | 123 + 4z Arraywerte; bis 32 Records; sechs Undo-Arten; slot int -1…7; Revision int/int64 ≥ 0; Enum-Zellen hier **Index**, in Parameters dagegen **Wort**; jeweiliger Leser entspricht seinem Writer | `:782` ↔ `:1347` |
| Undo-Cursor | Zahl der Redo-Schritte ab Ringende | int 0…Ringgröße; frischer/fehlender Ring 0; kein unabhängiger Stand ohne Ring | `:1075` ↔ `:1422` |

Die Zonenwerte sind im Undo-Record **flach angehängt**, kein zusätzliches verschachteltes Zonenarray. Das ganze Undo-Record ist ein eigenes Array. Keine zweite Kopie des aktuellen DSP liegt im Dsp-Kind: der aktuelle Satz verteilt sich auf Parameters und occupied; die vollständigen alten Sätze im Undo-Ring sind seine Rückwege.

### Alle 120 DSP-Zellen

Für **jede** Zeile gilt derselbe nachgelesene Paritätsweg:

- **P:** State-Writer `NakamaParameter.cpp:563` → State-Reader `:624`; Binärfixture `dsp-v2-voll.bin`; die ersten 109 zusätzlich `layout-v1.bin`.
- **O:** `NakamaState.cpp:1055` → `:1321`; Binärfixture `dsp-v2-voll.bin`. Aus Layout v1 wird occupied je Slot aus enabled **oder einer bitgenauen Defaultabweichung eines der 13 Werte** abgeleitet (`NakamaParameter.cpp:164`).
- Alle **120** durch DTO-Writer `NakamaParameter.cpp:312` und -Reader `:438`; alle außer `v2.global.eq_enabled` durch Preset-Writer/-Reader `NakamaPreset.cpp:41`/`:92`; alle erneut im Undo-Record.
- Feldmengen-/Metadaten-Test: `tests/StateMigrationTestMain.cpp:418` (alle IDs und Reihenfolge), `:472` (Typ, Grenzen, Defaults und Enums), `:516` (109 v1-Einträge unverändert abgeleitet). Gelesen, **nicht ausgeführt**.
- Typensumme: **27 bool, 69 double, 24 enum**. Hostbestand: 19 bool + 69 double + 24 enum = 112. State speichert physikalische Werte, der Host normalisierte floats.

| Index | ID (vollständige Population) | State-Typ | Grenze / Enum | Default | Definition | Pfad |
|---|---|---|---|---|---|---|
| 0 | `v1.global.bypass` | bool | false / true | false | `NakamaParameter.cpp:49` | P |
| 1 | `v1.global.input_trim_db` | double | [-24, 24] | 0 | `NakamaParameter.cpp:50` | P |
| 2 | `v1.global.output_trim_db` | double | [-24, 24] | 0 | `NakamaParameter.cpp:51` | P |
| 3 | `v1.global.width` | double | [0, 2] | 1 | `NakamaParameter.cpp:52` | P |
| 4 | `v1.global.mono_bass_hz` | double | [0, 500] | 0 | `NakamaParameter.cpp:53` | P |
| 5 | `v1.band.0.enabled` | bool | false / true | false | `NakamaParameter.cpp:62` | P |
| 6 | `v1.band.0.type` | enum | bell, low_shelf, high_shelf, notch, low_cut, high_cut | bell | `NakamaParameter.cpp:63` | P |
| 7 | `v1.band.0.freq_hz` | double | [20, 20000] | 1000 | `NakamaParameter.cpp:64` | P |
| 8 | `v1.band.0.q` | double | [0.15, 24] | 1 | `NakamaParameter.cpp:65` | P |
| 9 | `v1.band.0.gain_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:66` | P |
| 10 | `v1.band.0.channel_mode` | enum | stereo, left, right, mid, side | stereo | `NakamaParameter.cpp:67` | P |
| 11 | `v1.band.0.dynamic_enabled` | bool | false / true | false | `NakamaParameter.cpp:68` | P |
| 12 | `v1.band.0.dynamic_range_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:69` | P |
| 13 | `v1.band.0.threshold_db` | double | [-60, 0] | -20 | `NakamaParameter.cpp:70` | P |
| 14 | `v1.band.0.attack_ms` | double | [0.1, 500] | 10 | `NakamaParameter.cpp:71` | P |
| 15 | `v1.band.0.hold_ms` | double | [0, 500] | 0 | `NakamaParameter.cpp:72` | P |
| 16 | `v1.band.0.release_ms` | double | [5, 5000] | 100 | `NakamaParameter.cpp:73` | P |
| 17 | `v1.band.0.sidechain_source` | enum | none, internal, priority_sidechain | none | `NakamaParameter.cpp:74` | P |
| 18 | `v1.band.1.enabled` | bool | false / true | false | `NakamaParameter.cpp:62` | P |
| 19 | `v1.band.1.type` | enum | bell, low_shelf, high_shelf, notch, low_cut, high_cut | bell | `NakamaParameter.cpp:63` | P |
| 20 | `v1.band.1.freq_hz` | double | [20, 20000] | 1000 | `NakamaParameter.cpp:64` | P |
| 21 | `v1.band.1.q` | double | [0.15, 24] | 1 | `NakamaParameter.cpp:65` | P |
| 22 | `v1.band.1.gain_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:66` | P |
| 23 | `v1.band.1.channel_mode` | enum | stereo, left, right, mid, side | stereo | `NakamaParameter.cpp:67` | P |
| 24 | `v1.band.1.dynamic_enabled` | bool | false / true | false | `NakamaParameter.cpp:68` | P |
| 25 | `v1.band.1.dynamic_range_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:69` | P |
| 26 | `v1.band.1.threshold_db` | double | [-60, 0] | -20 | `NakamaParameter.cpp:70` | P |
| 27 | `v1.band.1.attack_ms` | double | [0.1, 500] | 10 | `NakamaParameter.cpp:71` | P |
| 28 | `v1.band.1.hold_ms` | double | [0, 500] | 0 | `NakamaParameter.cpp:72` | P |
| 29 | `v1.band.1.release_ms` | double | [5, 5000] | 100 | `NakamaParameter.cpp:73` | P |
| 30 | `v1.band.1.sidechain_source` | enum | none, internal, priority_sidechain | none | `NakamaParameter.cpp:74` | P |
| 31 | `v1.band.2.enabled` | bool | false / true | false | `NakamaParameter.cpp:62` | P |
| 32 | `v1.band.2.type` | enum | bell, low_shelf, high_shelf, notch, low_cut, high_cut | bell | `NakamaParameter.cpp:63` | P |
| 33 | `v1.band.2.freq_hz` | double | [20, 20000] | 1000 | `NakamaParameter.cpp:64` | P |
| 34 | `v1.band.2.q` | double | [0.15, 24] | 1 | `NakamaParameter.cpp:65` | P |
| 35 | `v1.band.2.gain_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:66` | P |
| 36 | `v1.band.2.channel_mode` | enum | stereo, left, right, mid, side | stereo | `NakamaParameter.cpp:67` | P |
| 37 | `v1.band.2.dynamic_enabled` | bool | false / true | false | `NakamaParameter.cpp:68` | P |
| 38 | `v1.band.2.dynamic_range_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:69` | P |
| 39 | `v1.band.2.threshold_db` | double | [-60, 0] | -20 | `NakamaParameter.cpp:70` | P |
| 40 | `v1.band.2.attack_ms` | double | [0.1, 500] | 10 | `NakamaParameter.cpp:71` | P |
| 41 | `v1.band.2.hold_ms` | double | [0, 500] | 0 | `NakamaParameter.cpp:72` | P |
| 42 | `v1.band.2.release_ms` | double | [5, 5000] | 100 | `NakamaParameter.cpp:73` | P |
| 43 | `v1.band.2.sidechain_source` | enum | none, internal, priority_sidechain | none | `NakamaParameter.cpp:74` | P |
| 44 | `v1.band.3.enabled` | bool | false / true | false | `NakamaParameter.cpp:62` | P |
| 45 | `v1.band.3.type` | enum | bell, low_shelf, high_shelf, notch, low_cut, high_cut | bell | `NakamaParameter.cpp:63` | P |
| 46 | `v1.band.3.freq_hz` | double | [20, 20000] | 1000 | `NakamaParameter.cpp:64` | P |
| 47 | `v1.band.3.q` | double | [0.15, 24] | 1 | `NakamaParameter.cpp:65` | P |
| 48 | `v1.band.3.gain_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:66` | P |
| 49 | `v1.band.3.channel_mode` | enum | stereo, left, right, mid, side | stereo | `NakamaParameter.cpp:67` | P |
| 50 | `v1.band.3.dynamic_enabled` | bool | false / true | false | `NakamaParameter.cpp:68` | P |
| 51 | `v1.band.3.dynamic_range_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:69` | P |
| 52 | `v1.band.3.threshold_db` | double | [-60, 0] | -20 | `NakamaParameter.cpp:70` | P |
| 53 | `v1.band.3.attack_ms` | double | [0.1, 500] | 10 | `NakamaParameter.cpp:71` | P |
| 54 | `v1.band.3.hold_ms` | double | [0, 500] | 0 | `NakamaParameter.cpp:72` | P |
| 55 | `v1.band.3.release_ms` | double | [5, 5000] | 100 | `NakamaParameter.cpp:73` | P |
| 56 | `v1.band.3.sidechain_source` | enum | none, internal, priority_sidechain | none | `NakamaParameter.cpp:74` | P |
| 57 | `v1.band.4.enabled` | bool | false / true | false | `NakamaParameter.cpp:62` | P |
| 58 | `v1.band.4.type` | enum | bell, low_shelf, high_shelf, notch, low_cut, high_cut | bell | `NakamaParameter.cpp:63` | P |
| 59 | `v1.band.4.freq_hz` | double | [20, 20000] | 1000 | `NakamaParameter.cpp:64` | P |
| 60 | `v1.band.4.q` | double | [0.15, 24] | 1 | `NakamaParameter.cpp:65` | P |
| 61 | `v1.band.4.gain_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:66` | P |
| 62 | `v1.band.4.channel_mode` | enum | stereo, left, right, mid, side | stereo | `NakamaParameter.cpp:67` | P |
| 63 | `v1.band.4.dynamic_enabled` | bool | false / true | false | `NakamaParameter.cpp:68` | P |
| 64 | `v1.band.4.dynamic_range_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:69` | P |
| 65 | `v1.band.4.threshold_db` | double | [-60, 0] | -20 | `NakamaParameter.cpp:70` | P |
| 66 | `v1.band.4.attack_ms` | double | [0.1, 500] | 10 | `NakamaParameter.cpp:71` | P |
| 67 | `v1.band.4.hold_ms` | double | [0, 500] | 0 | `NakamaParameter.cpp:72` | P |
| 68 | `v1.band.4.release_ms` | double | [5, 5000] | 100 | `NakamaParameter.cpp:73` | P |
| 69 | `v1.band.4.sidechain_source` | enum | none, internal, priority_sidechain | none | `NakamaParameter.cpp:74` | P |
| 70 | `v1.band.5.enabled` | bool | false / true | false | `NakamaParameter.cpp:62` | P |
| 71 | `v1.band.5.type` | enum | bell, low_shelf, high_shelf, notch, low_cut, high_cut | bell | `NakamaParameter.cpp:63` | P |
| 72 | `v1.band.5.freq_hz` | double | [20, 20000] | 1000 | `NakamaParameter.cpp:64` | P |
| 73 | `v1.band.5.q` | double | [0.15, 24] | 1 | `NakamaParameter.cpp:65` | P |
| 74 | `v1.band.5.gain_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:66` | P |
| 75 | `v1.band.5.channel_mode` | enum | stereo, left, right, mid, side | stereo | `NakamaParameter.cpp:67` | P |
| 76 | `v1.band.5.dynamic_enabled` | bool | false / true | false | `NakamaParameter.cpp:68` | P |
| 77 | `v1.band.5.dynamic_range_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:69` | P |
| 78 | `v1.band.5.threshold_db` | double | [-60, 0] | -20 | `NakamaParameter.cpp:70` | P |
| 79 | `v1.band.5.attack_ms` | double | [0.1, 500] | 10 | `NakamaParameter.cpp:71` | P |
| 80 | `v1.band.5.hold_ms` | double | [0, 500] | 0 | `NakamaParameter.cpp:72` | P |
| 81 | `v1.band.5.release_ms` | double | [5, 5000] | 100 | `NakamaParameter.cpp:73` | P |
| 82 | `v1.band.5.sidechain_source` | enum | none, internal, priority_sidechain | none | `NakamaParameter.cpp:74` | P |
| 83 | `v1.band.6.enabled` | bool | false / true | false | `NakamaParameter.cpp:62` | P |
| 84 | `v1.band.6.type` | enum | bell, low_shelf, high_shelf, notch, low_cut, high_cut | bell | `NakamaParameter.cpp:63` | P |
| 85 | `v1.band.6.freq_hz` | double | [20, 20000] | 1000 | `NakamaParameter.cpp:64` | P |
| 86 | `v1.band.6.q` | double | [0.15, 24] | 1 | `NakamaParameter.cpp:65` | P |
| 87 | `v1.band.6.gain_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:66` | P |
| 88 | `v1.band.6.channel_mode` | enum | stereo, left, right, mid, side | stereo | `NakamaParameter.cpp:67` | P |
| 89 | `v1.band.6.dynamic_enabled` | bool | false / true | false | `NakamaParameter.cpp:68` | P |
| 90 | `v1.band.6.dynamic_range_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:69` | P |
| 91 | `v1.band.6.threshold_db` | double | [-60, 0] | -20 | `NakamaParameter.cpp:70` | P |
| 92 | `v1.band.6.attack_ms` | double | [0.1, 500] | 10 | `NakamaParameter.cpp:71` | P |
| 93 | `v1.band.6.hold_ms` | double | [0, 500] | 0 | `NakamaParameter.cpp:72` | P |
| 94 | `v1.band.6.release_ms` | double | [5, 5000] | 100 | `NakamaParameter.cpp:73` | P |
| 95 | `v1.band.6.sidechain_source` | enum | none, internal, priority_sidechain | none | `NakamaParameter.cpp:74` | P |
| 96 | `v1.band.7.enabled` | bool | false / true | false | `NakamaParameter.cpp:62` | P |
| 97 | `v1.band.7.type` | enum | bell, low_shelf, high_shelf, notch, low_cut, high_cut | bell | `NakamaParameter.cpp:63` | P |
| 98 | `v1.band.7.freq_hz` | double | [20, 20000] | 1000 | `NakamaParameter.cpp:64` | P |
| 99 | `v1.band.7.q` | double | [0.15, 24] | 1 | `NakamaParameter.cpp:65` | P |
| 100 | `v1.band.7.gain_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:66` | P |
| 101 | `v1.band.7.channel_mode` | enum | stereo, left, right, mid, side | stereo | `NakamaParameter.cpp:67` | P |
| 102 | `v1.band.7.dynamic_enabled` | bool | false / true | false | `NakamaParameter.cpp:68` | P |
| 103 | `v1.band.7.dynamic_range_db` | double | [-12, 12] | 0 | `NakamaParameter.cpp:69` | P |
| 104 | `v1.band.7.threshold_db` | double | [-60, 0] | -20 | `NakamaParameter.cpp:70` | P |
| 105 | `v1.band.7.attack_ms` | double | [0.1, 500] | 10 | `NakamaParameter.cpp:71` | P |
| 106 | `v1.band.7.hold_ms` | double | [0, 500] | 0 | `NakamaParameter.cpp:72` | P |
| 107 | `v1.band.7.release_ms` | double | [5, 5000] | 100 | `NakamaParameter.cpp:73` | P |
| 108 | `v1.band.7.sidechain_source` | enum | none, internal, priority_sidechain | none | `NakamaParameter.cpp:74` | P |
| 109 | `v2.global.eq_enabled` | bool | false / true | false | `NakamaParameter.cpp:79` | P |
| 110 | `v2.global.mix` | double | [0, 1] | 1 | `NakamaParameter.cpp:80` | P |
| 111 | `v2.global.auto_gain` | bool | false / true | false | `NakamaParameter.cpp:81` | P |
| 112 | `v2.band.0.occupied` | bool | false / true | false | `NakamaParameter.cpp:87` | O |
| 113 | `v2.band.1.occupied` | bool | false / true | false | `NakamaParameter.cpp:87` | O |
| 114 | `v2.band.2.occupied` | bool | false / true | false | `NakamaParameter.cpp:87` | O |
| 115 | `v2.band.3.occupied` | bool | false / true | false | `NakamaParameter.cpp:87` | O |
| 116 | `v2.band.4.occupied` | bool | false / true | false | `NakamaParameter.cpp:87` | O |
| 117 | `v2.band.5.occupied` | bool | false / true | false | `NakamaParameter.cpp:87` | O |
| 118 | `v2.band.6.occupied` | bool | false / true | false | `NakamaParameter.cpp:87` | O |
| 119 | `v2.band.7.occupied` | bool | false / true | false | `NakamaParameter.cpp:87` | O |

### Migration, Preset und Hostprogramm

| Eingangsfassung / Weg | Ergebnis und Abbildung | Eingefrorener Eingang / Gegenbeleg |
|---|---|---|
| EqCopilotState schema=1, role=sensor | legacy+insert; sensor_id → instance_id, Label/Paar erhalten | `identity/state-schema1-sensor.bin` → `schema2/aus-schema1-sensor.bin` |
| role=hub | main+insert; MainProject schema=1; keine erfundene Bindung | entsprechende hub-Blobs |
| role=pre / post | legacy+pre / post; Paar erhalten | beide entsprechenden Blobpaare |
| role fehlt / ID leer | fehlende Rolle historisch sensor; leere fehlende ID erhält UUID, ausdrücklich als kaputter Altstand behandelt | `NakamaState.cpp:2063` bis `:2076`; kein neuer Verlust eines vorhandenen Feldwerts behauptet |
| Schema 2, Layout v1 | 109 Werte unverändert; eq_enabled=false, mix=1, auto_gain=false; occupied bitgenau abgeleitet; Zonen/Ring leer, Revision 0 | `schema2/layout-v1.bin`; `NakamaState.cpp:1980` bis `:2012` |
| Schema 2, Layout v2 | alle 112 Hostwerte plus 8 occupied und Zonen/Ring/Revision | `schema2/dsp-v2-voll.bin` |
| Root/Kind/Layout unbekannt oder Matrix unzulässig | read-only mit Originalbytes; keine Normalisierung dieser Bytes beim Save | `schema2/fremdes-major-3.bin`; `StateMigrationTestMain.cpp:1950`; `NakamaState.cpp:2095`, `:2165` |
| DSP-DTO | genau drei Rootfelder: dsp_schema_version, parameters, schutz_zonen; 120 IDs, exakte Typ-/Bereichsprüfung | 7 positive und 29 negative JSON-Fixtures |
| Preset v1 / DSP-Layout v2 | genau vier Writer-Rootfelder: preset_schema_version, dsp_schema_version, parameters, schutz_zonen; 119 IDs; eq_enabled des Zielzustands bleibt erhalten | 3 positive und 17 negative JSON-Fixtures; `NakamaPreset.cpp:23`, `:92` |
| JUCE-Programm/Bank | Gen und Probeeq melden je ein Programm; keine zusätzliche eigene Bankliste; JUCE-Program-State ruft denselben get/set-State auf | `PluginProcessor.h:124`; `SondeProcessor.h:182`; `juce_AudioProcessor.cpp:921` |
| Replace-V2 | in beiden Produktzielen deaktiviert, keine zu migrierende zusätzliche VST2-Produktkennung | `plugin/CMakeLists.txt:152`, `:353`; bestehende eingefrorene Identität bleibt unangetastet |

Die Presetdatei trägt weder Identität noch state_hash, state_revision oder Undo-Ring. Die sechs verbotenen Rootnamen sind adresse, instance_id, host_bus_name, label, eq_enabled, undo_ring. Ein unbekanntes zusätzliches Preset-Rootfeld wird gemäß R12 ignoriert; das ist **nicht** die Originalbytes-Regel des Hostprojekts. Dateidialog und Browser sind S31b, kein heimlich vorhandener zusätzlicher Save-/Load-Pfad. Der Gen-Messsnapshot (`Analyse.cpp:1321`, `:1395`) exportiert Instanz-ID, alte Rollenabbildung, Label/Paar und Messwerte; er wird nicht als Host-State zurückgeladen.

### Vollständiger Dirty-Signalweg

**Gen:** Die Quellsuche erfasst **elf** `meldeHostDirty()`-Aufrufstellen: neun in `State.cpp`, eine in `Analyse.cpp`, eine in `Ipc.cpp`. Sie münden in **einen** Hostaufruf `Hostbruecke.cpp:162`. Die Handgriffe teilen sich diesen Weg wie folgt:

| Persistenter Handgriff / Zweig | vollständige Population | Meldung / Riegel |
|---|---|---|
| neue Instanz-ID | `State.cpp:54` | read-only verweigert; Dirty `:62` |
| Klasse, Position, Common-Label/Paar, erste Main-Projektbindung | `State.cpp:287` | Gleichheit `:318`; Sources-Publikationsauswertung; Sonderfall T3-02-04 |
| Passage merken / vergessen | `State.cpp:365`, `:403` | Änderungen zählen, Ablehnung/No-op schweigt; Dirty `:398`, `:428` |
| Quellenrolle setzen / entfernen | `:480`, `:509` | bool veraendert; Dirty `:498`, `:523` |
| Schutz setzen / entfernen | `:535`, `:563` | bool veraendert; Dirty `:551`, `:579` |
| Beziehung setzen / entfernen | `:591`, `:627` | bool veraendert; Dirty `:608`, `:641`; Gleichrangig-Alias delegiert |
| Assistent starten, weiter, zurück, überspringen, abbrechen, antworten, Versuch starten | `Analyse.cpp:1138`, `:1154`, `:1173`, `:1187`, `:1201`, `:1225`, `:1291` | gemeinsame Meldung `:1117`; Start eines Experiments als zusätzlicher Mutator T3-02-01 |
| Mitglied umbenennen / lokal entfernen | `Ipc.cpp:1205`, `:1245` | Gleichheit/Existenz, Main und read-only geprüft; gemeinsame Publikationsauswertung |
| bestätigter Join / Unbind | `Ipc.cpp:1428`, `:1437` | Quellen- und Reloadgeneration geprüft; nur echte Änderung wird gezählt |
| Drain im Worker, Editor-Tick und beim Save | `Ipc.cpp:1538`; `State.cpp:69` | **je geändertem Befehl** eine Meldung, auch bei mehreren Befehlen im Batch: `Ipc.cpp:1586`, `State.cpp:111`; kein gemeinsames bool verliert die zweite Änderung |
| normaler Load / Migration / read-only-Load | `State.cpp:140` | kein Host-Dirty; flüchtige v3-Revision darf steigen, weil eine neue Publikation nötig ist |

Der Sources-Publikationsriegel unterscheidet Reload-Ablehnung von einer überholten Kopie derselben Generation (`Ipc.cpp:1462`). Nur beim Reload entfällt Dirty für die verworfene Änderung; eine überholte **Darstellung** nimmt der wirksam gebliebenen Änderung ihre Meldung nicht. Die Folgenummer wird unter der State-Sperre gezogen und saturiert (`:1446`); `SourcesModel.cpp:335` prüft Generation und Folge zusammen.

**Probeeq:** Genau **eine** Host-Dirty-Aufrufstelle steht in `SondeProcessor.cpp:1228`. Alle **neun** Transaktionsarten — apply, revert, neutralisieren, remove, undo, redo, presetLaden, gestus, bandBelegen (`NakamaTransaktion.h:70`) — durchlaufen `fuehreTransaktionAus`. Nur Ausgang commit holt den Dirty-Merker ab. Ein Commit verändert Revision/Undo selbst dann, wenn ein ausdrücklich ausgeführter Ganzzustandsauftrag gleiche Klangwerte enthält; das ist nicht automatisch ein No-op. Wiederholte Transaktions-ID, Ablehnung und nicht geänderte Geste erzeugen keinen zweiten Commit. Read-only sperrt diesen Transaktionsweg (`SondeProcessor.cpp:1202`).

Die 112 Parameter-Callbacks führen keine direkte persistente Zuweisung aus: Mailbox → AutomationOverlay → erst abgeschlossene Usergeste als Transaktion. Hostabgleich unterdrückt den eigenen Callback durch den Synchronisationsmarker (`SondeProcessor.cpp:1388`). Der verbleibende Mailbox-/Readonly-Automationsfall ist bereits offene SONDE-015-Nacharbeit, unten gesondert genannt.

### Lebenszyklus und Snapshot-Publikation

| Paar / Containerpopulation | gelesener Pfad | Ergebnis |
|---|---|---|
| Host-Save ↔ Host-Load Gen | `State.cpp:69`, `:140` | bestätigte Befehle vor Serialisierung; State unter Bindungssperre; Laden validiert zunächst in lokales Objekt |
| Host-Save ↔ Host-Load Probeeq | `SondeProcessor.cpp:990`, `:1017` | Save liest den bestätigten Transaktionskern, nicht die möglicherweise automatisierten Host-floats; read-only gibt Originalbytes |
| Main-Sources-Modell: **5 Container** | `SourcesModel.h:391`, `:421`; `SourcesModel.cpp:456` | eintraege und persistenteMitglieder aus geladenem State neu; experimente, paare, befunde leer; sechs Identitäts-/Zielstrings leer; findingsOffen und Evidenzrücknahme mitgeleert |
| Main-Commands: **2 Container** | `State.cpp:115` | ausstehendeSourcesCommands und bestaetigteSourcesCommands gemeinsam mit Generationswechsel unter dem zugehörigen Mutex geleert |
| Vergleichssitzung | `Analyse.cpp:576` | laufende Versuch-/Passagen-ID, Blindvergleich, Fensterbindung und Vergleichspegel zurückgesetzt; Löschwunsch erreicht Engine über Steuerpfad; letzter P0-Text bleibt als Diagnosezeuge, nicht als fortgesetzter Versuch |
| Probeeq-Transaktion | `NakamaTransaktion.cpp:234` | r0=r=geladene Revision; Register vollständig leer; Automationpräsenz und Preview beendet; geladener Undo-Ring/Cursor bleiben |
| Probeeq-Geste / Hörmatrix | `SondeProcessor.cpp:1057` | gesteOffen und gesteBeteiligt leer, Hoermatrix=processed; Hostparameter danach synchronisiert |
| Bewusst erhaltene Laufzeitstände | `SourcesModel.cpp:473`; `PluginProcessor.cpp:576`; `PluginProcessor.h:1760` | monotone Publikationsmarken/Diagnosezähler sowie Interventions-Sendebuchhaltung sind keine gespeicherten Projektfelder; kein pauschales „alles muss clear()“ behauptet |
| Offene Reloadkante | `SondeProcessor.cpp:1296`, `:1413` | Host-Mailbox wird vom Ladestart nicht mitgeräumt; bereits bekannte SONDE-015-Nacharbeit |
| Ausgabeparameter vor Audiobank | `SondeProcessor.cpp:1063`, `:1069` | Bank wird bereits vor dem Hostparameterabgleich publiziert; Übernahme liegt im Audio-Callback. Ein gleichzeitig laufender Callback wurde hier nicht gefahren; die zeitliche M-08-Zusage erhält aus dieser Lektüre keinen frischen Laufzeitbeleg |
| Writer-Abbruch / Teilpuffer | `NakamaState.cpp:2162` | Writer leert Ziel vor Serialisierung; kein eigener temporärer vollständiger Commit-Puffer. Kein herbeigeführter OOM, Hostabbruch oder Teilstream; kein unbelegter Datenverlustbefund aus dieser Reihenfolge |

### State ↔ Wire: vollständige Feldgruppen

- **Common:** instance_id und project_binding_id reisen als Adresse; eine alte nicht-hex-ID wird erst an der Wiregrenze durch ihren kompatiblen Alias adressiert. Klasse/Position reisen als Vertragswörter; Label/Paar werden als Runtime-/Legacy- beziehungsweise Diagnoseangaben abgeleitet. Persistente Identität wird dafür nicht umbenannt. Laufzeitnonce und Sessionepoche sind **keine** gespeicherten Common-Felder.
- **Mitglieder:** persistente IDs/Labels speisen das Sources-Modell und die bestätigten Bindungsabläufe. Sitzungsstatus, Verfügbarkeit, Befunde und Messfrische kommen nicht aus diesem gespeicherten Paar.
- **Passagen:** die vier State-Felder werden beim Experimentbeginn verwendet (`Analyse.cpp:914` bis `:921`); projekt_start/ende bleiben Samplepositionen. State erlaubt int64, der JSON-Weg hat die sichere Ganzzahlgrenze; die E2E-Erreichbarkeit eines tatsächlichen Messfensters oberhalb 2^53 wurde nicht ausgeführt. Dieser zusätzliche Zeitbereich wird nicht als weiterer nachgewiesener Revisionsbefund gezählt.
- **Intent:** alle sechs SourceIntent-Zellen, vier Schutz-Zellen und drei Beziehungs-Zellen werden in `Ipc.cpp:583` bis `:635` abgebildet. Leerer Scope wird zu **fehlendem** passage_id, nicht null. Bandgrenzen bleiben Gitterindizes, keine Hz. Die double-Konfidenz wird mit `wireZahl` formatiert; Wire-Textpräzision ist enger als die binäre State-Zelle. Die drei Revisionswerte siehe T3-02-03.
- **Assistent:** step_id, schritt, revision, offen und drei optionale Referenz-IDs reisen über `v3AssistantStepJson`. `gesetzt` ist ein State-Präsenzmerkmal; `ergebnis` hat in dieser Nachrichtenfamilie kein eigenes Feld, Userurteile haben einen getrennten P0-Weg. Der Sender liest den restaurierten State, keinen Schatten (`Analyse.cpp:447`; `Ipc.cpp:646`).
- **DSP:** `state_report` enthält heute Schema, Revision, Hash, Recordstatus; dessen Revision stammt aus dem bestätigten Kern. Alle 120 Werte/Zonen liegen im kanonischen DTO/berichtDto, sind aber noch keine vollständig ausgelieferte state_report-Nutzlast (NAK-110). Frequenzwerte bleiben in Hz, dynamische Zeiten in ms, Pegel in dB; eine Laufzeit-Nyquistkappung überschreibt keinen persistenten Wert.
- **Undo:** Ring, Slot und Cursor bleiben Projektzustand. Der lokale DspBericht leitet undoTiefe ab; kein Rückwärtsimport eines Broker-Snapshots überschreibt den autoritativen Kern.

## Bereits bekannt, deshalb kein zusätzlicher Befund

Diese Punkte sind **nicht als korrekt widerlegt**. Ihre Neuheit ist widerlegt; die folgenden offenen bzw. terminierten Arbeiten werden nicht ein zweites Mal gezählt.

| Punkt | frischer Quellenabgleich | bestehender Eintrag / Abgrenzung |
|---|---|---|
| Host-Mailbox nach Abweisung/Reload | `SondeProcessor.cpp:1017`, `:1296`, `:1388`, `:1413`: Synchronisationsmarker unterdrückt neue eigene Callbacks, entfernt aber keine alten wartenden Hostwerte | `SONDE-015.md:4335`, Regel R-4a-A `:4348`; bekannte offene Etappe-4a-Nacharbeit |
| Automation auf read-only | Kontrolltakt kann Overlay/Programm veröffentlichen; der Transaktionsriegel `:1202` schützt diese zweite Eingangstür nicht | `SONDE-015.md:4337`, `:4348`; nicht als neuer Fund zu NAK-283 verkauft |
| Drei fehlende MainProject-Goldens | bestätigt durch vollständige Binärinventur | NAK-267/A-3; allein die zusätzliche Bindungslücke ist T3-02-07 |
| Fehlendes Retry nach abgewiesenem state_report | Senderergebnis ist keine verlässliche Zustellbestätigung | NAK-183; T3-02-03 betrifft dagegen den gültigen Wertbereich und den vom Broker verworfenen Inhalt |
| Gen-Master-EQ und DSP-Rücknutzlast | Gen führt heute keine eigenen Host-DSP-Parameter; state_report übermittelt noch keinen vollständigen Probeeq-DSP | NAK-105/S28b und NAK-110/Etappe 4b; keine fehlenden heutigen State-Felder erfunden |
| InterventionsRing::zuruecksetzen ohne Produktaufrufer | kein Anlass, alle Interventions-Sendebücher als Projektstate zu löschen; sie schließen bestätigte Begin/End-Paare | NAK-268/B-8; kein neuer allein aus fehlendem clear() abgeleiteter Befund |
| Assistent-finding_id ohne eigenen UI-Writer | State hält die Referenz, Wire kann sie tragen; aktueller Urteilspfad nimmt sie als Argument | NAK-208/S31b; keine neue Implementierungsforderung für geparkte Bedienarbeit |

## Widerlegte Verdachte

**12** Verdachte verworfen; jeweils gilt die konkrete Abgrenzung, kein generelles PASS über den gesamten Bereich.

| Nr. | Verdacht | Widerlegung an der Quelle / vorhandene Wache |
|---|---|---|
| W-01 | Jede unbekannte Minor-Eigenschaft geht beim Save verloren oder wird umsortiert | `NakamaState.cpp:808` kopiert den gehaltenen Baum, bekannte Eigenschaften werden in place geändert; `StateMigrationTestMain.cpp:1484` prüft die additive Eigenschaft. Namensduplikate und Größenwachstum sind die ausdrücklich gesonderten Fälle T3-02-05/02. |
| W-02 | Unbekanntes Major oder unzulässige Matrix wird mit Defaults überschrieben | `NakamaState.cpp:2095` hält Originalbytes, `:2165` gibt sie zurück; `StateMigrationTestMain.cpp:1950` enthält Main-/Klassen-/Kind-Gegenfälle. Keine Gleichsetzung mit unvollständigen/ungültigen Bytes. |
| W-03 | Ein abgeschnittener Blob oder kaputtes UTF-8 liefert einen teilweise geladenen Zustand | Vollständiger Vorleser samt UTF-8, Längen, EOF, Tiefe und Eintragsbudget vor JUCE (`NakamaState.cpp:95`); `lade` gibt bei ungueltig ignoriert zurück, beide Hostsetter tauschen dann nichts. |
| W-04 | 120 Zeichen werden beim Laden als 120 UTF-8-Bytes missverstanden | Main-Mitglieder/Passagen verwenden `String::length()`; Headroom rechnet vier Bytes je Codepoint (`NakamaState.cpp:1092`). Common akzeptiert längere Alttexte ausdrücklich; deren spätere Panelkürzung ist T3-02-04. |
| W-05 | Jeder Hostparameter-Abgleich zerstört gespeicherte double-Präzision oder Minusnull | `NakamaParameter.cpp:583` erhält -0 beim Baumschreiben; `SondeProcessor.cpp:1360` bewahrt die originale double-Zelle, wenn ihre float-Hostdarstellung gleich geblieben ist. JCS fasst -0 und +0 absichtlich zusammen, Binärstate nicht. |
| W-06 | Layout-v1-Migration vergisst belegte, aber ausgeschaltete Bänder | `NakamaParameter.cpp:147`, `:164`: bitgenaue Abweichung eines der 13 Slotwerte genügt für occupied, zusätzlich enabled. Alle 109 v1-Definitionen werden im Feldmengentest verglichen (`StateMigrationTestMain.cpp:516`). |
| W-07 | Zonen-ID und Bandrand werden vor der Bereichsprüfung gefährlich verengt | Binärzonen verlangen int32-ID und exakte double-Kanten (`NakamaState.cpp:1276`), DTO-Zonen prüfen den Wert vor der Konvertierung (`NakamaParameter.cpp:350`); Schutz nutzt 0 ≤ von < bis ≤ 221 beziehungsweise -1/-1 (`NakamaState.cpp:344`). D8-Grenzfixtures vorhanden. |
| W-08 | Laden, Migration und jeder No-op melden stets Dirty | Gen-Hostsetter meldet nicht (`State.cpp:222`); Sonde synchronisiert ohne Dirty (`SondeProcessor.cpp:1068`). Normale Setter prüfen Gleichheit/veraendert. Beim Save bestätigte, noch nicht angewandte Befehle zu übernehmen ist eine wirkliche Änderung, keine falsche Lademeldung (`State.cpp:103`). |
| W-09 | Die reparierten F11-Entfernwege löschen weiterhin vor der Revisionsprüfung | Vorprüfung steht heute vor erase; gemeinsamer Assistenten-Helfer prüft MAX vor Zuweisung (`NakamaState.cpp:2299`, `:2358`, `:2476`, `:2677`). T3-02-01 liegt außerhalb dieser reparierten Helfer. |
| W-10 | D6: Experimente, Paare und Befunde stehen nach jedem Reload weiter | Alle drei Container werden über `SourcesModel.cpp:515` aus `projektReload` und Subscription-Aufbau geleert; Quellenkarten, IDs und Rücknahmezahlen ebenfalls. Nicht mit den bekannten Host-Mailbox-Resten verwechseln. |
| W-11 | Preset-Laden importiert Identität, schaltet eq_enabled ein oder ersetzt den Projekt-Undo-Ring | `NakamaPreset.cpp:11`, `:23`, `:92` sperren die sechs Namen und eq_enabled; der Transaktionskern legt einen Rückweg an (`NakamaTransaktion.cpp:611`). Ignorieren unbekannter Preset-Rootfelder ist die ausdrückliche Presetregel. |
| W-12 | Verborgene Programmbank oder Replace-V2 nutzt einen zweiten nicht geprüften State-Leser | Beide Produkte melden ein Programm, die JUCE-Programm-State-Defaults delegieren an get/setState (`juce_AudioProcessor.cpp:921`); beide Produktziele bauen Replace-V2=0 (`CMakeLists.txt:152`, `:353`). |

## Produktfragen

- **P-01 / T3-02-06:** Welche Lebensdauer soll MainProject-Inhalt nach einem bewussten Wechsel von Main zu Messpunkt haben: dauerhaft verworfen oder bei Rückkehr erhalten? Der gegenwärtige Unterschied zwischen Rückkehr mit und ohne Reload braucht eine einheitliche Regel. Der Audit trifft diese Produktentscheidung nicht.

## Nicht geprüft

- Laufzeitreproduktionen der sieben Befunde, vorhandene Tests, Kanon, FL, echter VST3-Host, Installation, Nulltest und DSP-Klang: durch den Auftrag ausgeschlossen.
- Prozessbeendigung mitten in `getStateInformation`, OOM/Allokationsausnahme, tatsächlicher Teilpuffer und Host-Dateiatomarität. Die Writerstruktur wurde gelesen; kein Crash- oder Dateisystembeweis wurde erzeugt.
- Tatsächliche Interleavings von Hostparameterabgleich, Audio-Bankübernahme und Reload. Kein Concurrency-/Sanitizerlauf; insbesondere kein frischer M-08-Lauf.
- Vollständiger Broker-Store-/Crash-Audit, gesamte v3-Feldpopulation und reale Transportversuche für Passagen-Samplepositionen oberhalb 2^53. Gelesen wurden hier die State-Sender und die relevante Eingangsschranke.
- Gesamte Analysehistorie, sämtliche Interventions-Replay-Szenarien, DSP-Bankressourcen und Diagnose-Exportdateien als eigenständige Persistenzsysteme. Die für State-Restore sichtbaren Kanten sind beschrieben; daraus folgt kein vollständiges Lebenslaufzertifikat.
- Implementierung künftiger Gen-DSP-, Remote-DSP-, Presetbrowser- oder S31b-Oberflächen. Design, Optik, Legacy-Namensbereinigung, geparkte Studien und Dokumentqualität waren keine Befundfläche.
- Python wurde nicht als Test- oder Generatorlauf benutzt. Der zu Beginn versuchte reine Leseaufruf über `py -3.13 -c` scheiterte am lokalen Store-Python-Anmeldesitzungsfehler; die notwendigen Inhalte wurden danach mit PowerShell und reinem Auslesen im Speicher zugänglich.

## Zähler und Abschluss

**4 DEFEKTE · 2 LÜCKEN · 1 HÄRTUNG.** Schwere: **critical 2 · high 2 · medium 2 · low 1**. **12 widerlegte Verdachte · 1 Produktfrage.**

Bekannte Punkte mit zusätzlicher Information sind NAK-283/F11, NAK-252 und NAK-267/A-3. Die anderen bekannten Punkte oben bleiben ohne neue Audit-ID.

Abschluss: **15.09.2026, 20:48:53 Europe/Berlin**; Dauer seit Beginn **48 min 24 s**. Die Abschlusskontrolle ergab: Branch `master`, HEAD vor und nach dem Lesegang identisch `3f2619c74d39483c9487296667b2bfa9e4246e75`; sieben Tabellenzeilen in Bericht und Anhang wortgleich; `git diff --check` ohne Fehler. Der ursprüngliche `BEFUNDE.md`-Inhalt ist als Prefix von **15.563 Bytes** unverändert, geprüft über SHA-256 `99FB9AD2C24363BDFA6E96A5F05FC4AE40399DE0AFFACBEF7FE48AFEC36B47A9`. Der eigene Diff hängt dort ausschließlich den Phase-02-Abschnitt an. Kein Produkt-/Testpfad wurde ausgeführt.

**Parallelbetrieb:** Zu Beginn standen nur die fremden untracked Pfade `.claude/settings.local.json`, `briefing-hub/`, `roh/phase-02-auftrag.txt` im Auditordner und `nimbalyst-local/` im Status. Bei der Abschlusskontrolle kamen fremde Änderungen an `.claude/skills/dirigent/SKILL.md`, `docs/offene-punkte.md` und den Auftragsdateien `auftraege/KOPF.txt`, `phase-08.txt`, `phase-10.txt`, `phase-11.txt`, `phase-12.txt` hinzu. Diese Pfade wurden nicht geschrieben. Der zusätzlich gelesene Registerdiff ändert ausschließlich NAK-308 zur Prüfsession und keine Einordnung eines hier erhobenen Befunds. Eigene Schreibziele bleiben genau die zwei beauftragten Auditdateien; HEAD und Produktquellen blieben unverändert.

## Scope-Beweis

### Vollständig gelesene Kernpopulation — 101 Dateien

- `eq-copilot/plugin/state/**`: **13/13 Dateien**, vollständiger Inhalt: `NakamaState.h`, `NakamaState.cpp`, `NakamaParameter.h`, `NakamaParameter.cpp`, `NakamaPreset.cpp`, `NakamaTransaktion.cpp`, `NakamaLebenslauf.cpp`, `NakamaLebenslauf.h`, `NakamaKernRiegel.h`, `NakamaPreset.h`, `NakamaKanon.h`, `NakamaTransaktion.h`, `NakamaKanon.cpp`.
- `eq-copilot/schemas/state/**`: **4/4 Dateien**, vollständiger Inhalt: `nakama-state-v2.md`, `nakama-parameter-v1.json`, `nakama-parameter-v2.json`, `nakama-preset-v1.json`.
- `eq-copilot/fixtures/state/**`: **79/79 Dateien**: 1 MANIFEST, 8 Binärstates, 36 DTO-JSONs, 20 Preset-JSONs, 14 JCS-JSONs. Das Manifest wurde als Fixtureindex gelesen, nicht auditiert.
- `eq-copilot/fixtures/identity/**`: **4/4 Dateien**, sämtliche Schema-1-Binärstates.
- `tools/eq-copilot/erzeuge_state_fixtures.py`: **1/1 Datei**, vollständiger Inhalt; **nicht ausgeführt**.
- Population daraus: **136** benannte Schema-2-Property-Orte, **5** Schema-1-Rootfelder, sämtliche inneren Recordfelder, **120** DSP-IDs mit Definition, Writer, Reader, Migration und Fixture-/Feldtestzuordnung. **Kein Pflichtpfad dieser Kernpopulation fehlt.**

#### Alle 12 binären Eingaben

„Gelesen bis Byte“ bezeichnet das vollständige strukturelle Auslesen der vorhandenen Datei im Arbeitsspeicher. Es ist **kein bestandener Produkt-Ladetest**.

| Datei | Dateigröße (Bytes) | Gelesen bis Byte |
|---|---:|---:|
| `eq-copilot/fixtures/state/schema2/aus-schema1-hub.bin` | 205 | 205 |
| `eq-copilot/fixtures/state/schema2/aus-schema1-post.bin` | 197 | 197 |
| `eq-copilot/fixtures/state/schema2/aus-schema1-pre.bin` | 195 | 195 |
| `eq-copilot/fixtures/state/schema2/aus-schema1-sensor.bin` | 178 | 178 |
| `eq-copilot/fixtures/state/schema2/dsp-v2-voll.bin` | 4802 | 4802 |
| `eq-copilot/fixtures/state/schema2/fremdes-major-3.bin` | 186 | 186 |
| `eq-copilot/fixtures/state/schema2/layout-v1.bin` | 3458 | 3458 |
| `eq-copilot/fixtures/state/schema2/main-intent-v1.bin` | 972 | 972 |
| `eq-copilot/fixtures/identity/state-schema1-hub.bin` | 125 | 125 |
| `eq-copilot/fixtures/identity/state-schema1-post.bin` | 135 | 135 |
| `eq-copilot/fixtures/identity/state-schema1-pre.bin` | 133 | 133 |
| `eq-copilot/fixtures/identity/state-schema1-sensor.bin` | 128 | 128 |

#### Alle 70 JSON-Fixturedateien

Je Gruppe sind sämtliche Dateien aufgeführt; Prefix ist `eq-copilot/fixtures/state/`.

- `dto/gueltig/` — **7 Dateien**: `default.json`, `ganzzahl-schreibweise.json`, `gemischt.json`, `maxima.json`, `minima.json`, `minus-null.json`, `zone-einzeln.json`.
- `dto/ungueltig/` — **29 Dateien**: `bereich-gain.json`, `bereich-ganzzahl-2e53.json`, `bereich-q-unten.json`, `doppelter-schluessel.json`, `dsp-schema-version-1.json`, `enum-unbekannt.json`, `fehlende-v2-kennung.json`, `fehlender-schluessel.json`, `nichtendlich-exponent.json`, `nichtendlich-unterlauf.json`, `schutz-zonen-fehlt.json`, `struktur-array.json`, `struktur-extra-wurzel.json`, `typ-bool-als-zahl.json`, `typ-enum-als-bool.json`, `typ-zahl-als-string.json`, `unbekannter-schluessel.json`, `zone-doppelte-id.json`, `zone-enabled-als-string.json`, `zone-id-2147483648.json`, `zone-id-8.json`, `zone-id-9007199254740991.json`, `zone-id-minus-1.json`, `zone-low-gleich-high.json`, `zone-neunte.json`, `zone-ueber-20-khz.json`, `zone-unsortiert.json`, `zone-unter-20-hz.json`, `zone-zusatzfeld.json`.
- `jcs/` — **1 Dateien**: `zahlen.json`.
- `jcs/dokumente/` — **13 Dateien**: `array-von-objekten.json`, `escape-eingabeformen.json`, `leeres-array.json`, `leeres-objekt.json`, `literale.json`, `rfc-beispiel.json`, `sortierung-utf16.json`, `steuerzeichen.json`, `tiefe-struktur.json`, `unicode-roh.json`, `verschachtelt.json`, `whitespace.json`, `zahlen-im-kontext.json`.
- `preset/gueltig/` — **3 Dateien**: `default.json`, `gemischt.json`, `unbekanntes-top-level-feld.json`.
- `preset/ungueltig/` — **17 Dateien**: `bereich-gain.json`, `dsp-schema-version-1.json`, `eq-enabled-in-parametern.json`, `fehlender-schluessel.json`, `preset-major-2.json`, `unbekannter-schluessel.json`, `verboten-adresse.json`, `verboten-eq-enabled.json`, `verboten-host-bus-name.json`, `verboten-instance-id.json`, `verboten-label.json`, `verboten-undo-ring.json`, `zone-id-2147483648.json`, `zone-id-8.json`, `zone-id-9007199254740991.json`, `zone-id-minus-1.json`, `zone-unsortiert.json`.

Zusätzlich wurde `MANIFEST.json` vollständig gelesen. `jcs/zahlen.json` enthält 54 Zahlenvektoren; die 13 Dokumentfixtures wurden mit ihren Eingabe-/Erwartungstexten gelesen. Keine Hash-/Kanon-Neuberechnung des Produktcodes und keine Negativfixture wurde ausgeführt.

### Weitere Produkt-, Verbraucher- und Testquellen — 28 Dateien

Die Zeilenangaben sind die im Lesegang eingesehenen Funktionskörper bzw. Bereiche. Außer ausdrücklich „vollständig“ bedeuten sie **keine Volllektüre der jeweiligen Datei**. Ergänzende `rg`-Suchen dienten der Aufruf-, Mutator-, Setter-, Register- und Testsuche.

| Pfad | tatsächlich eingesehener Umfang |
|---|---|
| `eq-copilot/plugin/src/prozessor/State.cpp` | vollständig |
| `eq-copilot/plugin/sonde/SondeProcessor.cpp` | 1–540, 767–850, 950–1175, 1177–1448 |
| `eq-copilot/plugin/sonde/SondeProcessor.h` | vollständig |
| `eq-copilot/plugin/tests/TransactionTestMain.cpp` | 1858–2050, 2170–2249 |
| `eq-copilot/plugin/src/prozessor/Analyse.cpp` | 403–478, 540–655, 780–955, 1085–1440 |
| `eq-copilot/plugin/src/prozessor/Hostbruecke.cpp` | vollständig |
| `eq-copilot/plugin/tests/Sonde014AssistentTest.cpp` | 558–620, 630–781 |
| `eq-copilot/build/_deps/juce-src/modules/juce_data_structures/values/juce_ValueTree.cpp` | 1028–1102 |
| `eq-copilot/build/_deps/juce-src/modules/juce_core/containers/juce_NamedValueSet.cpp` | 90–221 |
| `eq-copilot/build/_deps/juce-src/modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp` | 2880–2963 |
| `eq-copilot/plugin/src/prozessor/Ipc.cpp` | 114–155, 346–372, 452–736, 786–855, 892–987, 1188–1649 |
| `eq-copilot/plugin/src/SourcesModel.cpp` | 335–428, 456–543 |
| `eq-copilot/plugin/src/SourcesModel.h` | 1–80, 382–460 |
| `eq-copilot/plugin/tests/StateMigrationTestMain.cpp` | 148–212, 320–372, 450–590, 948–1072, 1484–1641, 1656–1757, 1950–1974, 2030–2124, 2240–2330, 2360–2448 |
| `eq-copilot/plugin/tests/Sonde014IntentTest.cpp` | 1203–1297, 1452–1539 |
| `eq-copilot/plugin/vertrag/NakamaVertrag.cpp` | 310–335, 381–430, 438–477 |
| `broker/src/coordinator/schema.rs` | 413–445 |
| `eq-copilot/plugin/core/ipc/ControlClient.cpp` | 1–161 |
| `eq-copilot/plugin/src/PluginProcessor.h` | 120–151, 1308–1380, 1630–1653, 1760–1865 |
| `eq-copilot/plugin/core/ipc/controlclient/Intern.h` | 18–65, 375–390, 425–458 |
| `eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp` | 98–153, 398–468 |
| `eq-copilot/plugin/tests/Sonde012ProjectReloadTest.cpp` | 2628–2720 |
| `eq-copilot/plugin/tests/Sonde013PassageStateTest.cpp` | 191–345, 3172–3225 |
| `broker/src/vertrag.rs` | 115–140, 230–309, 335–371 |
| `eq-copilot/plugin/src/PluginEditor.cpp` | 625–711 |
| `eq-copilot/plugin/src/PluginProcessor.cpp` | 125–171, 240–314, 566–611 |
| `eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessor.cpp` | 918–932 |
| `eq-copilot/plugin/CMakeLists.txt` | 140–156, 340–357 |

### Kontext und Zusagen — gezielt, keine Befundfläche

- `AGENTS.md` und `CLAUDE.md`: **2 Dateien vollständig** gelesen; Weisung für Phase 02 aus dem gebundenen Userauftrag.
- `.agents/skills/nakama-plugin-quality/SKILL.md`: **1 Skilldatei** gelesen; angewandt wurden Belegtiefe, Quellenabgleich und Skeptikerpass innerhalb des engeren Userauftrags.
- `docs/plugin-wissen.md`: Abschnitt zur aktuellen Zustands-/Parameterarchitektur, Zeilen 217–310.
- `docs/gesundheit/KONZEPT.md`: §3 und §4.3, Zeilen 127–160 und 234–365.
- `docs/gesundheit/abdeckungskarte.md`: K4/A-3, Zeilen 133–150; `docs/gesundheit/lebenslauf-audit.md`: Register-/Befundüberblick, Zeilen 1–40 und 70–98.
- `docs/offene-punkte.md`: gezielte Registersuche und vollständige passende Einträge; unter anderem NAK-105, 110, 183, 208, 252, 267, 268, 279, 283. Keine Volllektüre des großen Registers behauptet.
- `docs/FL-Nakama-Sonden-Design-Entwurf.md`: §33.5, §44.3/44.4, §53.8; Zeilen 2150–2177, 3030–3095, 4052–4114.
- `docs/plan/plan.json`: Einträge S7, S9/S9b, S26–28, S28b; rein lesende Auswahl, kein Planstand-Läufer.
- Verhaltensmatrizen: `docs/beweise/SONDE-006.md` Abnahmezeilen 1–5/State; `SONDE-014.md` insbesondere M-81/85/86/88/89; `SONDE-015.md` M-08 und State-/Preset-/Dirty-Zeilen M-74–98 sowie §12.1/R-4a-A; `NAK-283.md` M-07–12, M-71/72. **4 Manifestdateien gezielt** gelesen; keine historischen Grünzahlen als Sessionbeweis verwendet.
- `docs/audits/2026-09-12-code-review-2/UEBERGABE-AN-CLAUDE.md`: vollständige Übersicht der 14 Befundgruppen, D1–D10-Rückblick und für diese Phase besonders F01/F11; zusätzliche Detailabschnitte gelesen. Keine früheren Reproduktionsläufe wiederholt.
- `eq-copilot/schemas/v3/README.md`: Zahlenvertrag, Zeilen 35–66 und 95–128; `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`: gezielte Suche der State-/Revisions-/Assistentenfelder.
- `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md`: bisheriger Inhalt zur Dubletten- und Anhangsprüfung gelesen; der eigene Abschnitt wird angehängt.

### Nicht gelesene Pfade und fehlende Pfade

- In `plugin/state/**`, `schemas/state/**`, `fixtures/state/**`, `fixtures/identity/**` und dem beauftragten Generator: **keine ungelesene Datei, keine fehlende Datei**.
- Außerhalb dieser Kernpopulation: nicht gelesene Restbereiche der oben mit Zeilenumfang genannten Dateien; übrige Dateien in `plugin/tests/**`, `plugin/core/analysis/**`, `plugin/dsp/**`, `broker/src/store/**`, `broker/tests/**` und der Großteil von `schemas/v3/**`. Eine Symbolsuche wurde nicht als vollständige Lektüre gezählt.
- `design/**`, Studien-/Archivbäume, Installationssystem und übrige Prüfskripte/Runner waren keine Befundfläche und wurden für diese Phase nicht flächig gelesen.
- **Kein fehlender oder unlesbarer Pflichtpfad blockierte den Scope.** Der nicht startbare Python-Launcher wurde nicht benötigt; alle Pflichtdateien konnten mit den zugelassenen Lesewegen ausgewertet werden.
