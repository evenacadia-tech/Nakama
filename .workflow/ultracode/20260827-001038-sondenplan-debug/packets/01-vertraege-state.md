# Paket 01 — Verträge und State

Stand: 2026-08-27, reine Korrektheitsprüfung. Keine Sicherheitsbewertung, keine
Produktänderung. Zeilenangaben beziehen sich auf den bei der Prüfung sichtbaren
Arbeitsbaum; dieser war bereits vor Paketbeginn durch parallele/userseitige
Änderungen dirty.

> **Aktueller Stand (Schlussrevalidierung 01:33 CEST):** Die vier ursprünglichen
> Kernbefunde und der zwischenzeitliche Writer-Reservefehler wurden im
> Parallelstand adressiert. Die jeweils jüngste Revalidierung steht am Ende
> dieses Pakets und supersediert die historischen Zwischenläufe. Aktuell sind
> State- und Schema-Test grün; offen bleiben eine niedrige Grenzdefinitions-
> Abweichung sowie Beweis-/Dokumentationsreste, kein weiterer bestätigter
> Writer-, Parser- oder Cross-Language-Kernfehler.

## Ergebnis

Vier neue Kandidaten sind source-bestätigt:

| ID | Schwere | Status | Kurzurteil |
|---|---|---|---|
| VST-01 | hoch | **bestätigt** | Rohes NUL trennt C++ vom Rust-/Python-Urteil; betroffen sind v3-JSON und DSP-DTO. |
| VST-02 | hoch | **bestätigt** | Der State-Leser prüft weder vollständiges Decoding noch vollständigen Byteverbrauch und kann einen gültigen Präfix als ganzen State übernehmen. |
| VST-03 | mittel | **bestätigt** | `pair_id`/`project_binding_id` werden schwächer validiert als ihr Statevertrag; ein anschließendes Save normalisiert akzeptierte Eingaben. |
| VST-04 | mittel | **bestätigt** | `state_report.state_hash` ist trotz explizitem `null`-Fall und Zustandswahrheitsvertrag nicht Pflicht. |

Zusätzlich: eine stale Regelliste in der FlatBuffers-Dokumentation, eine bewusst
deferierte Capability-Kopplung und mehrere widerlegte beziehungsweise bereits
geschlossene Hypothesen. Diese werden nicht in die vier Kandidaten eingerechnet.

## Scope und gelesene Norm

Geprüft wurden:

- `eq-copilot/schemas/v3/**`, insbesondere JSON-Schema, FlatBuffers-Schema,
  Bandgitter-/Quantisierungsverträge und beide READMEs;
- `eq-copilot/schemas/state/**`, `eq-copilot/plugin/vertrag/**`,
  `eq-copilot/plugin/state/**`;
- zugehörige Fixture-Manifeste, Generatoren und C++-Tests;
- `broker/src/vertrag.rs`, `broker/src/telemetrie.rs`, `broker/src/dto.rs` und
  `broker/tests/contract_cross_language.rs`;
- aufrufende State-Pfade in `plugin/src/PluginProcessor.cpp` und
  `plugin/sonde/SondeProcessor.cpp`;
- die auf JUCE 8.0.9 aufgelösten Parserquellen im vorhandenen Buildbaum; der Pin
  steht in `eq-copilot/CMakeLists.txt:27-35`.

Als Norm wurden gezielt gelesen:

- Entwurf §32.2: `docs/FL-Nakama-Sonden-Design-Entwurf.md:1607-1650`;
- §32.3-33.3: `docs/FL-Nakama-Sonden-Design-Entwurf.md:1652-1834`;
- für die Zustandswahrheit zusätzlich §33.5:
  `docs/FL-Nakama-Sonden-Design-Entwurf.md:1867-1893`;
- §53.8-53.9: `docs/FL-Nakama-Sonden-Design-Entwurf.md:3681-3763`;
- §55: `docs/FL-Nakama-Sonden-Design-Entwurf.md:3816-3840`;
- §65 SONDE-005/006:
  `docs/FL-Nakama-Sonden-Design-Entwurf.md:4070-4077`;
- konkretisierende Verträge
  `eq-copilot/schemas/v3/README.md`,
  `eq-copilot/schemas/v3/flatbuffers/README.md` und
  `eq-copilot/schemas/state/nakama-state-v2.md`;
- nur die urteilenden/behauptenden Abschnitte der jüngsten Manifeste:
  `docs/beweise/SONDE-005a.md:1-23`,
  `docs/beweise/SONDE-005b.md:1-22`,
  `docs/beweise/SONDE-006.md:1-69`,
  `docs/beweise/G1.md:28-34` und `docs/beweise/G1.md:525-555`.

Die Ticketmanifeste markieren S5-S7 nach der G1-Nacharbeit selbst weiterhin als
`T3 NEEDS_WORK`; alte Rohlogs wurden deshalb nicht als Beleg für den heutigen
Stand verwendet.

## Bestätigte Befunde

### VST-01 — C++ beendet längenbehaftetes UTF-8 am ersten rohen NUL

**Urteil:** bestätigt, hoch. Confidence: sehr hoch.

**Konkrete Eingabeklasse:** Ein vollständiges gültiges v3-JSON- oder DSP-DTO-
Dokument, gefolgt von Byte `00` und weiteren Bytes. Bei ASCII nach dem NUL nimmt
C++ den gültigen Präfix an, während Rust/Python das Gesamtdokument wegen
nachfolgender Daten ablehnen. Steht nach dem NUL ein ungültiges UTF-8-Byte,
weicht bereits die Byteprüfung ab: C++ sieht es nicht, Rust/Python schon.

**Source-Beleg:**

- C++ übergibt zwar die Länge, verwendet aber
  `juce::CharPointer_UTF8::isValidString` und danach `String::fromUTF8`:
  `eq-copilot/plugin/vertrag/NakamaVertrag.cpp:334-354`.
- Im gepinnten JUCE 8.0.9 liefert `isValidString` beim ersten Nullbyte sofort
  `true`:
  `eq-copilot/build/_deps/juce-src/modules/juce_core/text/juce_CharPointer_UTF8.h:503-513`.
  `String::fromUTF8` ruft den Bereichskonstruktor auf
  (`.../juce_String.cpp:2144-2156`), dessen Kopierpfad bei `isEmpty()` stoppt
  (`.../juce_String.cpp:123-140`).
- Rust validiert die vollständige Slice-Länge mit `std::str::from_utf8`:
  `broker/src/vertrag.rs:173-180`; der Fixture-Caller parst danach dieselben
  Bytes vollständig mit `serde_json::from_slice`:
  `broker/tests/contract_cross_language.rs:55-97`.
- Python dekodiert die vollständigen Bytes:
  `tools/eq-copilot/pruefe_v3_vertrag.py:168-185` und parst anschließend das
  vollständige Dokument in `tools/eq-copilot/pruefe_v3_vertrag.py:519-557`.
- Der zweite reale S7-Pfad ist der DTO-Leser: C++ führt dieselbe Byteprüfung aus
  und baut erneut einen JUCE-String
  (`eq-copilot/plugin/state/NakamaParameter.cpp:186-211`); Rust parst die
  vollständige Slice (`broker/src/dto.rs:269-295`), Python das vollständige
  Textobjekt (`tools/eq-copilot/erzeuge_state_fixtures.py:234-264`).

**Warum die vorhandenen Tests grün bleiben:** Die gemeinsame Tabelle behauptet
NUL-Abdeckung, enthält aber nur den Escape-Text `\\u0000`, kein tatsächliches
Byte `00` (`eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json:5-6` und
`:281-286`). Eine read-only Auswertung aller Hexfälle ergab
`raw_nul_cases=0`. Der C++-Fixturepfad liest zwar zunächst Bytes
(`SchemaTestMain.cpp:141-167`), parst nach bestandenem Riegel aber wieder über
`loadFileAsString` (`SchemaTestMain.cpp:60-80,169-174`).

**Regressionstest-Idee:** In die gemeinsame Hex-Falltabelle je einen Fall
„gültiges Dokument + rohes NUL + ASCII“ und „gültiges Dokument + rohes NUL +
ungültiges UTF-8“ aufnehmen und alle drei Beine fahren; denselben Rand separat
am gültigen Default-DSP-DTO prüfen. Erwartung: vollständige Bytefolge wird in
allen Beinen abgelehnt.

**Reichweite:** Für v3-JSON existiert vor SONDE-010 noch kein produktiver
IPC-Caller; der Befund trifft heute den gebauten Vertragsleser/Testtreiber. Der
S7-DSP-DTO-Leser ist ebenfalls gebaut, derzeit aber nur aus Tests aufgerufen.

### VST-02 — `ValueTree::readFromData` liefert Präfixe ohne EOF-/Vollständigkeitsprüfung

**Urteil:** bestätigt, hoch. Confidence: hoch.

**Konkrete Eingabeklassen:**

1. ein vollständig gültiger bekannter Schema-2-State mit angehängten Bytes;
2. ein State, dessen Root ein weiteres Kind ankündigt, bei dem aber nach einem
   vollständigen `Common`-Kind die Bytes enden.

Im ersten Fall ignoriert der Leser den Suffix. Im zweiten gibt JUCE den bereits
gelesenen Elternbaum zurück. Für `legacy`/passive kann dieser Präfix alle von
`leseSchema2` verlangten Kinder enthalten und als schreibbar gelten. Das nächste
Save schreibt nur den interpretierten Baum und verliert die übrigen/fehlenden
Bytes, statt den Input als unlesbar/read-only zu halten.

**Source-Beleg:**

- `state::lade` ruft genau einmal `ValueTree::readFromData` auf und besitzt
  weder einen Verbrauchszähler noch einen EOF-Test:
  `eq-copilot/plugin/state/NakamaState.cpp:415-419`.
- `leseSchema2` prüft vollständig den *zurückgegebenen* Baum, kann aber nicht
  erkennen, dass der Binärleser ein angekündigtes späteres Kind nicht gelesen
  hat: `eq-copilot/plugin/state/NakamaState.cpp:262-358`.
- JUCE 8.0.9 `ValueTree::readFromStream` gibt bei einem ungültigen Kind den bis
  dahin aufgebauten Elternbaum zurück:
  `eq-copilot/build/_deps/juce-src/modules/juce_data_structures/values/juce_ValueTree.cpp:1047-1088`,
  insbesondere `:1077-1082`. `readFromData` erstellt nur einen
  `MemoryInputStream` und prüft nach `readFromStream` keinen Rest:
  dieselbe Datei `:1091-1094`.
- Ein schreibbarer Zustand wird neu serialisiert
  (`eq-copilot/plugin/state/NakamaState.cpp:462-472`); nur ein bereits als
  read-only erkanntes Objekt bekommt die Originalbytes zurück (`:465-468`).
- Das ist caller-relevant: EqCop übernimmt den Stand und startet/reconnectet die
  Pipe (`eq-copilot/plugin/src/PluginProcessor.cpp:637-673`); die Sonden-Shell
  übernimmt und klassifiziert ihn (`eq-copilot/plugin/sonde/SondeProcessor.cpp:72-92`).

Das widerspricht dem All-or-nothing-Satz in §53.8
(`docs/FL-Nakama-Sonden-Design-Entwurf.md:3694-3702`) und der State-Zusage
„nie ein Teilstate“ (`eq-copilot/schemas/state/nakama-state-v2.md:125-131`).

**Testlücke:** Der Roundtriptest deckt eigene Writerbytes und additive
Eigenschaften ab (`StateMigrationTestMain.cpp:526-558`); die read-only-Matrix
deckt semantische Baumfehler sowie sechs Müllbytes ab
(`StateMigrationTestMain.cpp:670-735`). Kein Fall prüft trailing Bytes,
Trunkierung nach einem vollständigen Kind oder eine inkonsistente deklarierte
Kindzahl.

**Regressionstest-Idee:** Drei Bibliotheks- und Produktfälle ergänzen:
vollständiger State + Suffix; abgeschnittenes zweites Kind nach gültigem
`Common`; abgeschnittene Eigenschaft. Keiner darf `geladen`/`migriert` ergeben,
und kein Save darf den Eingabepuffer still in einen kürzeren bekannten State
verwandeln.

### VST-03 — bekannte optionale Common-Felder werden nicht vertragsgemäß validiert

**Urteil:** bestätigt, mittel. Confidence: sehr hoch.

Der Statevertrag erlaubt `pair_id` nur bei nichtleerem Wert und
`project_binding_id` nur als bestätigtes `hex32`:
`eq-copilot/schemas/state/nakama-state-v2.md:17-22`. Die v3-Zieladresse
verlangt für die Bindung ebenfalls `hex32`:
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:54-68`.

Der Loader prüft dagegen nur `isString()` und übernimmt auch `pair_id=""`,
`project_binding_id=""`, Großbuchstaben oder beliebige kurze Texte:
`eq-copilot/plugin/state/NakamaState.cpp:319-327`. Beim Speichern werden leere
Werte anschließend entfernt:
`eq-copilot/plugin/state/NakamaState.cpp:218-226`.

Damit gibt es zwei beobachtbare Fehlerformen:

- ein nicht-hexadezimaler, nichtleerer Bindingwert wird als schreibbarer
  bekannter State akzeptiert, obwohl er nicht in die v3-Adresse passt;
- ein explizit vorhandenes leeres `pair_id`/`project_binding_id` wird akzeptiert
  und bei `Save(Load(x))` entfernt, statt den vertragswidrigen Input read-only
  mit Originalbytes zu halten.

Aktuell liest der v2-Hello-Caller nur `instanceId`, Rolle, Label und `pairId`,
nicht die Projektbindung (`eq-copilot/plugin/src/PluginProcessor.cpp:16-29`).
Der Binding-Teil ist deshalb heute persistenter Vertragsdrift und wird mit dem
v3-Adresscaller ab SONDE-010 unmittelbar sichtbar.

**Testlücke:** Die Tests prüfen leere Paar-ID nur als *Writer-/Migrationsoutput*
(`StateMigrationTestMain.cpp:452-458`) und enthalten keinen geladenen State mit
leerem optionalem Feld oder ungültigem `project_binding_id`.

**Regressionstest-Idee:** Schema-2-Bäume mit leerer vorhandener `pair_id`,
leerer vorhandener Bindung sowie nicht-`hex32` Bindungen laden; erwartetes
Urteil festlegen und anschließend Byteerhalt prüfen. Der aktuelle Vertrag legt
für diese ungültigen bekannten Werte fail-closed/read-only nahe.

### VST-04 — `state_report` darf `state_hash` vollständig weglassen

**Urteil:** bestätigt, mittel. Confidence: hoch.

`$defs/state_hash` unterscheidet ausdrücklich einen 64-stelligen Hash von
`null`, „wenn die Instanz keinen Stand zu melden hat“
(`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:39-44`). Trotzdem fehlt
`state_hash` in der Required-Liste von `state_report`
(`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:823-844`). Dadurch existiert
neben Hash und explizitem `null` ein dritter, ununterscheidbarer Zustand
„Eigenschaft fehlt“.

Das steht quer zu folgenden Vertragsstellen:

- Broker/Main lesen die lokale Wahrheit per `state_report`, die aktive Probe
  speichert mindestens aktuelle Revision **und** Hash:
  `docs/FL-Nakama-Sonden-Design-Entwurf.md:1867-1888`;
- der Statevertrag nennt das Format, das
  `v3 state_report.state_hash` verlangt:
  `eq-copilot/schemas/state/nakama-state-v2.md:116-122`.

Die gültige Grundfixture enthält den Hash
(`tools/eq-copilot/erzeuge_v3_fixtures.py:382-390`), die Negativfälle prüfen
Länge/Alphabet, aber nicht Abwesenheit (`:892-920`). Eine in-memory Probe mit
der gültigen Fixture, anschließendem `pop("state_hash")` und
`Draft202012Validator.iter_errors` ergab in dieser Session:

```text
state_report_without_hash_errors= 0
```

C++ und Rust folgen beide ausschließlich der Schema-`required`-Liste
(`eq-copilot/plugin/vertrag/NakamaVertrag.cpp:898-910`,
`broker/src/vertrag.rs:687-701`), daher ist dies eine gemeinsame Auslassung und
kein Cross-Language-Unterschied.

**Regressionstest-Idee:** `state-report-ohne-state-hash.json` als ungültige
Fixture ergänzen und in allen drei Schema-Beinen fahren; der „kein Stand“-Fall
soll separat mit explizitem `null` gültig bleiben.

## Klassifizierte Beobachtungen, nicht als neue Kandidaten gezählt

### Intentional/deferred — Contribution-Capability nicht im JSON-Schema gekoppelt

Eine Mutation der gültigen Beitragsfixture auf
`capabilities.contribution_aux="unsupported"` wird vom Referenzschema mit null
Fehlern akzeptiert. Der Zweig koppelt nur Position und Aussageklasse und
referenziert die allgemeine Capability-Tabelle:
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:410-431`.

Das wird **nicht** als neuer S5-S7-Fehler gezählt, weil die Beschreibung in
`:411` die konkrete Berechtigung ausdrücklich aus dieser Schemaebene herausnimmt.
Der gebaute State-Pfad sperrt die Position bei der gemessenen unsupported-
Capability separat und fail-closed
(`eq-copilot/plugin/state/NakamaState.cpp:94-165`); der Vertrag dokumentiert
die noch offene Klassenfrage als Eigentum von SONDE-011
(`eq-copilot/schemas/state/nakama-state-v2.md:54-90`). Für SONDE-011 bleibt der
Consumer-Test „Position behauptet Beitrag nur bei supported Capability“ nötig.

### Stale — FlatBuffers-README nennt nicht mehr die geschlossene Regelliste

Die Tabelle mit dem Anspruch „einer der folgenden Namen“
(`eq-copilot/schemas/v3/flatbuffers/README.md:125-155`) enthält
`ppq_verdreht` nicht. Beide Leser erzeugen die Regel jedoch
(`eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp:227-236`,
`broker/src/telemetrie.rs:399-407`) und das Manifest erwartet sie
(`eq-copilot/fixtures/v3/flatbuffers/MANIFEST.json:386-388`). Das ist stale
Dokumentation, keine aktuelle C++/Rust-Klassifikationsabweichung. Der Feld-ID-
Checker vergleicht Offsetfelder, nicht die Regelliste des README.

### Widerlegt/geschlossen

- **Unbekanntes Major verliert Bytes:** für semantisch erkanntes unbekanntes
  Major widerlegt. `lade` hält die Originalbytes
  (`NakamaState.cpp:421-431`) und `speichere` gibt sie zurück (`:462-468`);
  der aktuelle Testlauf bestätigt 17 Fälle. VST-02 ist eine andere Kante: dort
  erkennt der Binärdecoder den Input fälschlich als bekannten Präfix.
- **Positions-/Aussageklassen-Mismatch aus G1:** geschlossen; die vier
  Descriptorzweige koppeln die beiden Felder, und die Referenzprüfung kontrolliert
  die Zweigstruktur (`tools/eq-copilot/pruefe_v3_vertrag.py:419-490`).
- **`post_fader_contribution` im aktuellen State:** geschlossen; der frische
  C++-Lauf prüfte alle 16 Klasse×Position-Kombinationen und lehnt diese Position
  heute für alle gebauten Klassen ab.
- **PPQ-NaN/Inf und verdrehte PPQ-Grenzen:** geschlossen in beiden
  FlatBuffers-Lesern (`NakamaTelemetrie.cpp:218-236`,
  `broker/src/telemetrie.rs:385-407`); nur die README-Liste ist stale.
- **`state_hash`-Alphabet:** geschlossen durch `^[0-9a-f]{64}$`
  (`eq-ipc-v3.schema.json:39-44`) und aktuelle Negativfixtures.
- **Rust validiert ungültige DTOs nicht:** stale G1-Hypothese; heute validiert
  `broker/src/dto.rs:269-365`, und
  `broker/tests/contract_cross_language.rs:473-533` fährt gültige und
  ungültige DTO-Fixtures.
- **Leere Schema-1-`sensor_id` macht Migration nichtdeterministisch:** real,
  aber ausdrücklich dokumentierte/manifestierte Legacy-Ausnahme, daher
  intentional und Duplikat statt neuer Kandidat
  (`nakama-state-v2.md:96-113`, `NakamaState.cpp:396-398`,
  `docs/beweise/SONDE-006.md:397-404`).

## Ausgeführte Prüfungen

Es wurden keine Builds, Installationen oder externen Zustände verändert. Die
vorhandenen Testartefakte wurden direkt ausgeführt.

| Prüfung | Ergebnis dieser Session |
|---|---|
| `EqCopSchemaTest.exe` | Exit 0; 56 bestanden, 0 gescheitert; 164 JSON- und 55 Binärfixtures klassifiziert |
| `EqCopStateMigrationTest.exe` | Exit 0; 132 bestanden, 0 Fehler |
| vorhandenes `contract_cross_language-76d6c7b1dc93a662.exe --nocapture` | Exit 0; 8/8 Rust-Tests |
| `py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung` | Exit 0; 210/210, 164 Fixtures |
| `py -3.13 tools/eq-copilot/erzeuge_v3_fixtures.py --pruefen` | Exit 0; 166 Dateien bytegleich |
| `py -3.13 tools/eq-copilot/erzeuge_state_fixtures.py --pruefen` | Exit 0; 36 Dateien bytegleich |
| `py -3.13 tools/eq-copilot/pruefe_fbs_feldids.py` | Exit 0; 47 Felder, 0 rot |
| `py -3.13 tools/eq-copilot/pruefe_flatc_drift.py` | Exit 0; Pin 25.12.19, Drift 0 Dateien |
| in-memory `state_report` ohne Hash | Referenzschema meldet 0 Fehler (bestätigt VST-04) |
| gemeinsame Textriegel-Hextabelle nach tatsächlichem Byte `00` durchsucht | 0 Roh-NUL-Fälle (bestätigt Testlücke VST-01) |

## Verbleibende Testlücken und offene Hypothesen

1. Die vier bestätigten Randklassen besitzen noch keine Fixture und wurden
   deshalb nicht von den grünen Korpusläufen widerlegt.
2. `textriegelBytes` verengt `size_t laenge` zweimal auf `int`, ohne vorherige
   Obergrenze (`NakamaVertrag.cpp:347-354`). Derzeit existiert kein produktiver
   v3-Caller und kein Test jenseits `INT_MAX`; ohne realen Caller-/Puffervertrag
   bleibt das eine offene Integergrenzen-Hypothese, kein bestätigter Kandidat.
3. Der JCS-Korpus ist an RFC- und Handvektoren stark, aber keine generative
   Differentialprüfung aller shortest-decimal-/Unicode-Grenzen. Im vorhandenen
   Korpus wurden keine C++/Rust/Python-Hashabweichungen gefunden.
4. Die Python-Abdeckung meldet bei weichen Schema-Keywords bewusst Teilquoten
   (zum Beispiel `required` 38/190, `minimum` 6/40). Jede `$defs`-Definition hat
   einen Negativfall, aber nicht jede einzelne Feldinstanz.
5. Für die aktuell intentional externe Contribution-Capability-Entscheidung
   fehlt bis SONDE-011 der produktive Consumer-/Caller-Beweis.

## Priorisierte Fix- und Residualempfehlung (keine Umsetzung in diesem Paket)

1. **VST-02 zuerst schließen:** Den JUCE-State binär vollständig und
   längenstreng lesen; ein Fehler in Eigenschaft/Kind oder verbleibende Bytes
   dürfen keinen schreibbaren bekannten Baum ergeben. Danach die drei genannten
   Präfix-/Trunkierungsfälle sowohl gegen `state::lade/speichere` als auch gegen
   EqCop- und Sonden-`setStateInformation` fahren. Residual: JUCE-ValueTree ist
   kein sprachübergreifender Vertrag; zusätzlich bleibt ein kleiner
   mutationsbasierter Binärkorpus sinnvoll.
2. **VST-01 gemeinsam über alle drei Beine schließen:** Vor jeder
   JUCE-Stringbildung die exakt angegebene Bytefolge vollständig prüfen,
   einschließlich rohem NUL; zugleich eine dokumentierte maximale
   Dokumentgröße vor der `size_t`→`int`-Grenze setzen. Die zwei Fälle müssen in
   der *gemeinsamen* Hextabelle und im DTO-Korpus stehen. Residual: Bis
   SONDE-010 ein produktiver Framecaller existiert, beweist dies Parsergleichheit,
   noch nicht Fragmentierungs-/Envelopeverhalten.
3. **VST-03 am Statevertrag entscheiden und erzwingen:** Bekannte Felder müssen
   entweder der dokumentierten Form entsprechen oder der gesamte Input bleibt
   read-only mit Originalbytes. Leere Optionals, Klein-/Großschreibung und
   falsche Längen als Load/Save-Goldens festhalten. Residual: Die absichtlich
   nicht-hex32 alten `instance_id`-Werte bleiben davon getrennt unter NAK-40.
4. **VST-04 vor dem ersten `state_report`-Consumer schließen:** `state_hash` in
   der Report-Required-Liste führen, „kein Stand“ ausschließlich als explizites
   `null` testen und die neue Negativfixture in C++/Rust/Python fahren. Separat
   beim späteren ACK-Ticket entscheiden, bei welchen Ergebnissen
   `command_ack.state_hash` Pflicht oder `null` ist; diese bedingte ACK-Frage ist
   nicht Teil des bestätigten VST-04.
5. **Dokumentationsrest:** `ppq_verdreht` in der geschlossenen README-Liste
   nachziehen oder die Regelliste maschinenlesbar ableiten. Das ist nachrangig,
   weil die beiden Leser und das Manifest heute übereinstimmen.

Ein frisches Gate-Urteil sollte erst nach diesen Regressionen und einem erneuten
unabhängigen T3-Lauf vergeben werden. Die bereits grünen Korpusläufe bleiben
notwendige Regression, sind aber kein Gegenbeweis zu den vier fehlenden Kanten.

## Schlussurteil

Die vorhandenen Fixture- und Codegen-Gates reproduzieren ihren bekannten Korpus
sauber. Sie reichen dennoch nicht für ein frisches S5-S7-PASS: VST-01 verletzt
das §55-Ziel eines gleichen Sprachurteils, VST-02 die All-or-nothing-/
Originalbyte-Zusage des Statevertrags, VST-03 dessen eigene Common-Feldform und
VST-04 die explizite Zustandswahrheit des `state_report`. Gesamtconfidence für
die vier Kandidaten: hoch; für VST-02 wäre ein nachgezogener Runtime-Golden der
wichtigste zusätzliche Beleg.

---

## Revalidierung des aktuellen Parallelstands — 2026-08-27 01:06 CEST

Diese Ergänzung bewertet den nach dem Erstaudit sichtbar geänderten Arbeitsbaum.
Sie ersetzt nicht die historische Herleitung oben: Die vier VST-IDs bezeichnen
dort den **Vor-Fix-Stand**. Keine Produktdatei wurde in dieser Revalidierung
geändert.

### Status der vier ursprünglichen Kandidaten

| ID | aktueller Status | Beleg |
|---|---|---|
| VST-01 | **Kernfall geschlossen; ein Größenlimit-Residual offen** | C++ verwirft vor der JUCE-Stringbildung jedes rohe NUL und eine nicht als `int` darstellbare Länge (`eq-copilot/plugin/vertrag/NakamaVertrag.cpp:336-370`); Rust und Python verwerfen rohes NUL ebenfalls (`broker/src/vertrag.rs:211-221`, `tools/eq-copilot/pruefe_v3_vertrag.py:168-188`). Die gemeinsame Hextabelle enthält nun den tatsächlichen Bytewert `00` (`eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json:414-424`). |
| VST-02 | **Präfix-/Trunkierungskern geschlossen; drei Vorwärts-/Grenzresiduals offen** | Der neue Bytevorleser fordert genau einen vollständigen Baum und EOF (`NakamaState.cpp:47-61,201-230`), bevor JUCE liest (`:632-651`). Aktuelle Regressionen prüfen Suffix, spätes Abschneiden, deklarierte Riesenvariante und ValueTree-Tiefe (`StateMigrationTestMain.cpp:760-793`). |
| VST-03 | **geschlossen** | Vorhandene leere `pair_id` und nicht-lowercase-hex32 `project_binding_id` werden read-only (`NakamaState.cpp:233-240,530-544`); vier neue Fälle laufen durch die Bibliotheks- und Produktprüfung mit Originalbyte-Roundtrip (`StateMigrationTestMain.cpp:725-758`). |
| VST-04 | **geschlossen** | `state_hash` ist jetzt Pflicht (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:823-833`), und `state-report-ohne-state-hash` ist als ungültige Fixture manifestiert (`tools/eq-copilot/erzeuge_v3_fixtures.py:619-624`, `eq-copilot/fixtures/v3/MANIFEST.json:1485-1495`). Alle drei Schema-Beine klassifizieren den aktuellen 165er-Korpus gleich. |

Damit sind **4/4 ursprüngliche Fehlerklassen im beabsichtigten Kernfall
adressiert**, aber nicht alle Fixes residualfrei. Es bleiben **vier offene
Restpunkte**, davon zwei unmittelbar datenverlustrelevant.

### Offene Restpunkte nach Priorität

#### RVST-01 — Bekannter JUCE-Marker 9 und künftige Marker werden schreibbar geladen, aber beim Save normalisiert

**Urteil:** bestätigt, hoch. Confidence: sehr hoch. Unmittelbarer Residual von
VST-02 / Vorwärtskompatibilität.

Der Bytevorleser akzeptiert Marker `9` (`undefined`) und jeden unbekannten
Marker, indem er nur bis zum deklarierten Variantenende springt
(`eq-copilot/plugin/state/NakamaState.cpp:191-195`). Das wäre nur dann
verlustfrei, wenn der nachgeschaltete JUCE-Leser die Bytes hielte. Genau das tut
er nicht:

- JUCEs heutiger Writer serialisiert `juce::var::undefined()` als Länge 1,
  Marker 9 (`eq-copilot/build/_deps/juce-src/modules/juce_core/containers/juce_Variant.cpp:125-143`;
  die öffentliche Fabrik steht in `juce_Variant.h:115-116`).
- Der Reader hat weder für Marker 9 noch für einen künftigen Marker einen
  eigenen Fall; `default` überspringt die Nutzbytes und gibt anschließend eine
  leere/void-`var` zurück (`juce_Variant.cpp:838-888`).
- Eine void-`var` wird beim nächsten Save als komprimierte Länge 0 geschrieben
  (`juce_Variant.cpp:108-123`). Der Input ist damit schon bei einem **heutigen,
  vom heutigen JUCE-Writer erzeugbaren** `future_undefined` nicht bytegleich.
- Weil die Eigenschaft additiv/unbekannt ist, kann `leseSchema2` den Baum danach
  als schreibbar übernehmen (`NakamaState.cpp:567-575,680-688`), und
  `speichere` serialisiert ihn neu (`:694-704`). Das widerspricht der expliziten
  Zusage, unbekannte Eigenschaften zu halten
  (`eq-copilot/schemas/state/nakama-state-v2.md:8-10,32-43` und
  `eq-copilot/plugin/state/NakamaState.h:8-16`).

**Fehlender Test / Reproduktion:** In einem gültigen Schema-2-`Common`
`future_undefined = juce::var::undefined()` setzen, mit dem vorhandenen Writer
serialisieren, `lade`/`speichere` fahren und rohe Bytes vergleichen. Separat
denselben Streammarker auf einen unbekannten Wert ändern. Erwartung nach
Vertrag: entweder verlustfreies Laden oder read-only mit Originalbytes; nie
`geladen` plus veränderte Bytes. Der neue Variantentest deckt nur Binary und ein
flaches Array ab (`StateMigrationTestMain.cpp:795-819`).

**Fixrichtung:** Der Vorleser braucht mindestens drei statt zwei Urteile:
vollständig+heute verlustfrei lesbar / vollständig aber nur read-only
bewahrbar / syntaktisch kaputt. Marker 9 und unbekannte Marker dürfen nicht in
den schreibbaren Pfad gelangen, solange JUCE sie nicht byteerhaltend liest.

#### RVST-02 — Array-Varianten umgehen die einzige Rekursionstiefengrenze

**Urteil:** source-bestätigte Parsergrenze, hoch. Confidence: hoch; kein eigener
Runtime-Golden vorhanden.

`kMaxStateTiefe = 64` wird ausschließlich in `baum(tiefe)` geprüft
(`NakamaState.cpp:42-45,201-218`). Ein Array ruft dagegen für jedes Element
`variante()` ohne Tiefenparameter rekursiv auf (`:176-185`). Der globale
Eintragszähler begrenzt die theoretische Verschachtelung erst bei 262144
Einträgen (`:122-130`) und ist damit keine brauchbare Call-Stack-Grenze. Ein
kompakter, gültig längencodierter Baum mit einer unbekannten additiven
Eigenschaft aus vielen ineinander geschachtelten Ein-Element-Arrays durchläuft
deshalb eine ungebremste rekursive Aufrufkette, bevor ein kontrolliertes
`ignoriert`/`nurLesen`-Urteil möglich ist.

**Fehlender Test / Reproduktion:** Einen kleinen manuellen ValueTree-Stream mit
verschachtelten Marker-7-Varianten erzeugen und die exakten Grenzen `N-1/N/N+1`
fahren. Der vorhandene 70er-Test verschachtelt nur ValueTree-Kinder
(`StateMigrationTestMain.cpp:196-206,789-792`); der positive Arraytest ist nur
eine Ebene tief (`:803-819`).

**Fixrichtung:** Eine gemeinsame, explizit getestete Struktur-/Variantentiefe
durch `baum` **und** `variante` tragen. Bei einem vollständigen, aber für diesen
Build zu komplexen bekannten Nakama-State muss die Entscheidung mit der
Originalbyte-Zusage abgestimmt werden (siehe RVST-03), nicht pauschal wie Müll
behandelt werden.

#### RVST-03 — Neue State-Grenzen sind weder Vertragsgrenzen noch read-only-kompatibel

**Urteil:** bestätigt als Vertrags-/Vorwärtskompatibilitätsrest, mittel bis
hoch. Confidence: hoch.

Der Vorleser führt ohne Änderung des persistierten Vertrags Limits von 16 MiB,
64 ValueTree-Ebenen, 65536 Einträgen je Sammlung und 262144 Einträgen insgesamt
ein (`NakamaState.cpp:42-45,122-130,201-230`). Bei jeder Überschreitung liefert
`lade` sofort `ignoriert` (`:632-635`). Die Produktcaller behalten dann ihren
vorigen/frischen Zustand (`plugin/src/PluginProcessor.cpp:712-720`,
`plugin/sonde/SondeProcessor.cpp:72-84`) und ein späterer Save gibt **nicht** die
eingegangenen Bytes zurück.

Das ist für fremden Baumtyp oder Müll vertragsgemäß, nicht aber für einen
vollständig serialisierten `NakamaState` mit unbekanntem Major oder additiver
Eigenschaft: Diese Fälle sollen read-only samt Originalbytes reisen
(`nakama-state-v2.md:43,125-131`). Der neue Test `zuTieferBaum()` baut sogar
einen gültig serialisierten Baum mit Roottyp `NakamaState`, aber ohne lesbares
Major und unbekannten Kindern, und fordert nun `ignoriert`
(`StateMigrationTestMain.cpp:196-206,789-792`), obwohl der semantische Vertrag
für unbekanntes Root-Major/Kind read-only fordert. Ebenso kann der öffentliche
Writer über eine gehaltene unbekannte Binary-/Array-Eigenschaft selbst einen
Stream oberhalb der neuen Caps schreiben; `speichere` hat keine symmetrische
Fehlermeldung oder Grenze (`NakamaState.cpp:694-704`).

**Fehlende Tests:** Exakte Annahme-/Ablehnungsgrenzen für alle vier Caps;
`schema=3` knapp unter/über jeder Grenze mit anschließendem Originalbyte-Save;
writer→loader-Symmetrie für große additive Binary-/Array-Properties. Der
vorhandene Riesenvariantentest prüft eine **unvollständige deklarierte** Länge,
nicht einen vollständigen State an der 16-MiB-Grenze
(`StateMigrationTestMain.cpp:181-193,784-787`).

**Fixrichtung:** Die Limits entweder als versionierte, hostweit begründete
Persistenzgrenzen festlegen und symmetrisch im Writer/API ausdrücken, oder
vollständige, nur für diesen Build zu große/zu tiefe Nakama-Stände raw read-only
halten. Strukturell kaputte Präfixe bleiben davon getrennt `ignoriert`.

#### RVST-04 — Das neue Dokumentgrößenurteil ist nicht sprachübergreifend

**Urteil:** bestätigt als Cross-Language-Grenzrest, mittel. Confidence: hoch;
praktische Reichweite vor SONDE-010 gering.

C++ lehnt jede v3-/DTO-Bytefolge oberhalb `INT_MAX` explizit als „Dokument zu
gross“ ab (`NakamaVertrag.cpp:344-347`). Rust und Python besitzen im gemeinsamen
Bytegate keine entsprechende Obergrenze und prüfen stattdessen die komplette
Slice-/Bytefolge (`broker/src/vertrag.rs:211-221`,
`pruefe_v3_vertrag.py:168-188`). Ein ansonsten gültiges JSON mit erlaubtem
Trailing-Whitespace oberhalb der C++-Grenze hat daher kein gemeinsames
§55-Urteil. Ein produktiver v3-Framecaller existiert in S5-S7 noch nicht; ein
späteres kleineres Envelope-Limit könnte diese Eingabeklasse unerreichbar
machen, ist aber heute nicht Teil dieses Vertrags.

**Fehlender Test / Fixrichtung:** Ein gemeinsames, dokumentiertes Maximum in
allen drei Gates erzwingen. Für C++ lässt sich der `INT_MAX+1`-Abzweig mit einem
Dummy-Pointer prüfen, weil er vor jedem Speicherzugriff liegt; für das
sprachübergreifende Urteil sollte das Limit als kleine testbare Hilfsfunktion
oder per Grenzmetadatum geprüft werden, nicht durch eine reale >2-GiB-Fixture.

### Weitere Testlücken ohne offenen Verhaltenskandidaten

- Der Roh-NUL-Fall läuft jetzt durch die gemeinsame v3-Hextabelle, aber nicht
  noch einmal als vollständiges gültiges DSP-DTO. Die DTO-Caller rufen
  nachweislich dasselbe Gate vor dem Parser auf
  (`eq-copilot/plugin/state/NakamaParameter.cpp:186-209`,
  `broker/src/dto.rs:277-303`); ein DTO-Golden wäre dennoch die bessere
  caller-nahe Regression.
- Für die neue State-Vorprüfung fehlen akzeptierte Grenzfälle unmittelbar
  unterhalb von Tiefen-/Eintrags-/Bytelimits. Der aktuelle Lauf belegt nur einen
  übertiefen ValueTree und eine unvollständige Riesenlänge.

### Frische Prüfbelege der Revalidierung

Es wurde weiterhin nichts gebaut oder installiert.

| Prüfung | aktuelles Ergebnis |
|---|---|
| vorhandenes `EqCopSchemaTest.exe` (Release, Buildzeit 00:40) | Exit 0; 56/56; gemeinsame 60er-Hextabelle, 165 JSON- und 55 Binärfixtures |
| vorhandenes `EqCopStateMigrationTest.exe` (Release, Buildzeit 00:40) | Exit 0; 137/137; neue Präfix-/Trunkierungs-/State-Feldfälle grün |
| vorhandenes aktuelles Rust-Release-Artefakt `contract_cross_language-5f5cd0fed0086a10.exe --nocapture` (Buildzeit 00:36) | Exit 0; 8/8; 60 Hextabellenfälle, 165 JSON-, 55 Binärfixtures, 6/15 DTOs |
| älteres Rust-Debug-Artefakt (Buildzeit 00:33:03) | Exit 1 nur bei Roh-NUL-Fall; **stale/widerlegt**, weil `broker/src/vertrag.rs` erst 00:33:30 geändert wurde und das neuere Release-Artefakt denselben Fall besteht |
| `py -3 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung` | Exit 0; 211/211, 165 Fixtures; keine Definition/harter enum-/const-Pfad offen |
| `py -3 tools/eq-copilot/erzeuge_v3_fixtures.py --pruefen` | Exit 0; 167 Dateien bytegleich |
| `py -3.13 tools/eq-copilot/erzeuge_state_fixtures.py --pruefen` | Exit 0; 36 Dateien bytegleich |
| `git diff --check` auf den revalidierten Produktpfaden | keine Whitespacefehler; nur erwartete LF→CRLF-Hinweise |

### Revalidiertes Schlussurteil

VST-01 bis VST-04 sind in ihren ursprünglichen reproduzierbaren Kernfällen
geschlossen. Ein uneingeschränktes „0 Restpunkte“ wäre trotzdem nicht
source-ehrlich: **4 offene Residuals** bleiben — Marker-9/unknown-Variant-
Datenverlust, ungebremste Arrayverschachtelung, nicht vertraglich/read-only
abgebildete State-Caps und das nur in C++ vorhandene `INT_MAX`-Dokumentlimit.
Priorität vor einem neuen T3: zuerst RVST-01, dann RVST-02/RVST-03 gemeinsam;
RVST-04 kann spätestens mit dem verbindlichen SONDE-010-Envelope-Limit
geschlossen werden.

---

## Schlussrevalidierung nach Parser-Nachbesserung — 2026-08-27 01:10 CEST

Geprüft wurde ausschließlich der seit der ersten Revalidierung geänderte
State-Bytepfad samt Vertrag und Tests. Ergebnis: **RVST-01 und RVST-02 sind im
aktuellen Source geschlossen.** Von den vier zuvor genannten Residuals bleiben
zwei als Verhaltensthemen offen; hinzu kommen zwei Testlücken, die das grüne
141er-Ergebnis korrekt einordnen.

### Geschlossen

#### RVST-01 — Marker 9 / unbekannter Marker

**Status:** source-geschlossen. Confidence: sehr hoch.

- `BytePruefung` unterscheidet nun kaputt, verlustfrei und bekannte Wurzel mit
  nicht verlustfrei lesbarer Variante
  (`eq-copilot/plugin/state/NakamaState.cpp:48-68`).
- Der Vorleser erkennt ausschließlich die beiden eigenen Roottypen bereits an
  den Rohbytes (`:113-136`). Marker 9 und jeder unbekannte Marker setzen
  `nichtVerlustfrei`, werden aber weiterhin längengenau bis zum Variantenende
  geprüft (`:151-167,211-226`).
- Bei einer bekannten Wurzel geht dieser Fall **vor** dem JUCE-Decoder in
  `nurLesen`; `originalBytes` werden kopiert und `speichere` gibt sie
  unverändert zurück (`:664-683,729-739`). Damit kann JUCEs Marker→void-
  Normalisierung den gespeicherten Stand nicht mehr verändern.
- Der Statevertrag beschreibt diese Unterscheidung jetzt explizit
  (`eq-copilot/schemas/state/nakama-state-v2.md:133-141`).

Der ursprüngliche Datenverlustpfad ist damit auch für einen semantisch gültigen
Schema-2-Baum geschlossen. Eine Regressionstest-Schwäche bleibt separat unter
TL-01.

#### RVST-02 — ungebremste Arrayrekursion

**Status:** source-geschlossen. Confidence: sehr hoch.

`variante(int tiefe)` verwirft ab der festgelegten Variantentiefe und reicht
`tiefe + 1` ausschließlich beim Abstieg in Arrayelemente weiter
(`NakamaState.cpp:151-154,196-205`). Die ValueTree-Tiefe bleibt davon getrennt
begrenzt (`:229-246`). Ein manuell längenkorrekt aufgebauter 70-fach
verschachtelter Arraystream prüft im aktuellen Test den kontrollierten
`ignoriert`-Ausgang (`StateMigrationTestMain.cpp:232-257,845-848`). Der Vertrag
nennt beide 64er-Grenzen (`nakama-state-v2.md:133-136`). Die vorher ungebremste
rekursive Aufrufkette existiert damit nicht mehr.

### Wirklich offene Verhaltensresiduals

#### RVST-03W — dokumentierte State-Caps, aber Writer/Reader noch nicht symmetrisch

**Status:** offen, mittel. Confidence: hoch.

Der Teil „Grenzen sind nicht Vertragsbestandteil“ ist widerlegt/geschlossen:
16 MiB, beide Tiefen und beide Eintragsgrenzen stehen nun im Vertrag
(`nakama-state-v2.md:133-141`). Auch die Behandlung eines Inputs oberhalb der
Grenze als `ignoriert` ist jetzt eine explizite Formatentscheidung und nicht
mehr als unbekanntes-Major-Verlustproblem zu zählen.

Offen bleibt jedoch die inverse Richtung: `pruefeValueTreeBytes` lehnt jeden
Input oberhalb 16 MiB ab (`NakamaState.cpp:258-263`), während `speichere` ohne
dieselbe Grenze und ohne Fehlerkanal serialisiert (`:729-739`). Das ist nicht
nur eine frei erfundene Bibliotheksnutzung. Ein **gültig geladener** State knapp
unter 16 MiB darf eine große additive Binary-Property tragen; `synchronisiert`
erhält diese Eigenschaft und schreibt immer die typisierten Common-Felder,
einschließlich eines zuvor fehlenden oder später längeren Labels
(`:447-469`). Das Save kann dadurch über 16 MiB wachsen und wird vom eigenen
nächsten Load abgelehnt. Der Vertrag sagt selbst, dass ein fehlendes Label beim
Speichern ergänzt wird (`nakama-state-v2.md:32`).

**Noch fehlender Beleg/Fixrichtung:** writer→reader-Test mit einem gültigen
Schema-2-State unmittelbar unter der Bytegrenze, danach eine erlaubte
zustandsvergrößernde Änderung; das Ergebnis muss entweder innerhalb der
Vertragsgrenze bleiben oder `speichere` braucht einen ehrlichen Fehler-/
Fallbackpfad. Der aktuelle `winzig`-Test beweist nur, dass `16 MiB + 1` vor dem
ersten Bytezugriff abgelehnt wird
(`StateMigrationTestMain.cpp:850-853`), nicht Writer-Symmetrie.

#### RVST-04 — C++-exklusives `INT_MAX`-Dokumentlimit

**Status:** unverändert offen, mittel. Confidence: hoch.

C++ verwirft v3-/DTO-Dokumente oberhalb `INT_MAX`
(`eq-copilot/plugin/vertrag/NakamaVertrag.cpp:344-347`); Rust und Python haben
weiterhin kein gleiches Maximalurteil
(`broker/src/vertrag.rs:211-221`,
`tools/eq-copilot/pruefe_v3_vertrag.py:168-188`). Weder die State-
Nachbesserung noch der Statevertrag ändern diese §55-Abweichung. Sie kann durch
ein gemeinsames Dokumentmaximum oder spätestens durch ein verbindlich
kleineres SONDE-010-Envelope-Limit geschlossen werden.

### Offene Testlücken, aber kein verbleibender Sourcefehler

#### TL-01 — Die zwei Markerprüfungen unterscheiden den neuen Pfad nicht vom alten semantischen Fallback

`baumMitRohVariante` baut zwar eine bekannte `NakamaState`-Wurzel, aber weder
`schema=2` noch das Pflichtkind `Common`
(`StateMigrationTestMain.cpp:209-220`). Auch ohne die neue Markertriage würde
JUCE den Marker zu void lesen und `leseSchema2` denselben Baum anschließend
wegen des fehlenden Majors als read-only mit Originalbytes zurückgeben. Die
Assertions für Marker 9/10 prüfen nur `nurLesen` und Bytegleichheit, nicht den
spezifischen Grund (`:856-868`). Die beiden grünen Prüfungen sind daher für den
neuen Branch nicht trennscharf.

**Nachziehen:** Einen ansonsten gültigen Schema-2-State mit gültigem `Common`
und roher additiver Marker-9/10-Property verwenden oder mindestens exakt den
Grund `variant marker is not losslessly readable ...` behaupten. Der Source ist
auch ohne diesen Test eindeutig korrekt; offen ist der dauerhafte
Regressionsbeweis.

#### TL-02 — Nur Ablehnung bei 70, keine exakten positiven Grenzgoldens

Der Arraytest beweist eine endliche Begrenzung, aber nicht die dokumentierte
Kante: Es fehlen akzeptierte/abgelehnte Fälle unmittelbar um die 64er-Grenze.
Dasselbe gilt weiterhin für 16 MiB und die Eintragslimits. Wegen der Prüfung
`tiefe >= 64` vor jedem Variantenwert ist besonders festzulegen, ob „64
ineinander geschachtelte Arrays“ ein tiefstes leeres Array meint oder 64 Arrays
plus Nutzwert; derzeit kann die Textform je nach Zählkonvention anders gelesen
werden (`NakamaState.cpp:151-160,196-205`,
`nakama-state-v2.md:133-136`). Das ist eine Test-/Präzisionslücke, keine erneut
unbegrenzte Rekursion.

### Frischer Beleg

- Vorhandenes, nach Source und Test gebautes
  `EqCopStateMigrationTest.exe` (Buildzeit 01:10:17) direkt ausgeführt:
  **Exit 0, 141 Prüfungen ok, 0 Fehler**. Sichtbar grün waren insbesondere der
  70-fache Array-Gegenpfad, das frühe 16-MiB-Limit und Marker 9/10 read-only.
- `py -3.13 tools/eq-copilot/erzeuge_state_fixtures.py --pruefen`:
  **Exit 0, 36 Dateien bytegleich**, keine verwaiste Datei.
- `git diff --check` auf State-Source, State-Test und Statevertrag:
  keine Whitespacefehler; nur die bestehenden LF→CRLF-Hinweise.
- Es wurde nichts gebaut, installiert oder am Produkt geändert; geschrieben
  wurde ausschließlich dieses Paketartefakt.

### Endgültiger Reststand dieses Pakets

- **Direkte Parser-Residuals:** 0 offen (RVST-01/RVST-02 geschlossen).
- **Verhaltensresiduals:** 2 offen — Writer/16-MiB-Symmetrie (RVST-03W) und das
  sprachungleiche v3-/DTO-Dokumentmaximum (RVST-04).
- **Explizite Testlücken:** 2 — nicht trennscharfer Marker-Golden (TL-01) und
  fehlende exakte positive/negative Grenzgoldens (TL-02).

Damit ist der State-Parserkern gegenüber den zwei zuletzt gemeldeten direkten
Fehlern revalidiert. Ein Gesamturteil „0 offen“ wäre erst nach RVST-03W und
RVST-04 sowie den beiden gezielten Regressionsnachweisen belastbar.

---

## Revalidierung nach Writerreserve und exakten Grenztests — 2026-08-27 01:23 CEST

Diese Runde ersetzt den unmittelbar vorherigen Reststand. Geprüft wurden nur
die zuvor auditierten Vertrags-/Statepfade und ihre bestehenden Artefakte; es
wurde nichts gebaut oder am Produkt geändert.

### Geschlossen seit der letzten Runde

#### RVST-04 — gemeinsames v3-/DTO-Dokumentmaximum

**Status:** source-geschlossen. Confidence: sehr hoch.

Alle drei Bytegates verwenden jetzt dieselbe inklusive Obergrenze von 16 MiB:

- C++ exportiert `kMaxDokumentBytes = 16 * 1024 * 1024` und prüft vor jeder
  `size_t`→`int`-Konversion
  (`eq-copilot/plugin/vertrag/NakamaVertrag.h:26-29`,
  `NakamaVertrag.cpp:335-370`).
- Rust prüft dieselbe Konstante am Anfang des Slice-Gates
  (`broker/src/vertrag.rs:21-23,178-191`).
- Python prüft denselben Wert vor Decode/Parser
  (`tools/eq-copilot/pruefe_v3_vertrag.py:38-42,168-191`).
- Der kanonische v3-README nennt Grenze, engeren 256-KiB-Framer und direkte
  Datei-/DTO-Caller ausdrücklich (`eq-copilot/schemas/v3/README.md:78-83`).

Damit existiert die vorherige `INT_MAX`-Urteilsabweichung nicht mehr. C++ und
Python prüfen `16 MiB + 1` aktuell vor dem Bytezugriff
(`SchemaTestMain.cpp:611-616`, `pruefe_v3_vertrag.py:525-529`). Rust besitzt den
gleichen neuen Unit-Test im Source (`broker/src/vertrag.rs:892-896`); der
heutige Runtimebeleg dafür ist noch stale, siehe PB-01.

#### TL-01 — Marker-Golden ist jetzt trennscharf

**Status:** geschlossen.

`baumMitRohVariante` erzeugt nun einen semantisch gültigen Schema-2-Root samt
vollständigem `Common` und setzt erst dort die rohe additive Variante
(`StateMigrationTestMain.cpp:210-237`). Ohne die Markertriage würde dieser Baum
nach JUCE als schreibbar laden; die Marker-9/10-Assertions
(`:981-994`) decken deshalb jetzt tatsächlich den neuen read-only-Branch.

#### TL-02 — wesentliche Strukturgrenzen nachgezogen

**Status:** weitgehend geschlossen.

Die aktuelle Testsuite fährt jetzt beide Seiten der ValueTree- und Arraytiefe
sowie 65.536/65.537 Einträge je Sammlung
(`StateMigrationTestMain.cpp:899-937`). Der Source begrenzt Arrayrekursion und
ValueTree-Rekursion weiterhin getrennt
(`NakamaState.cpp:172-175,217-229,250-267`). Eine kleine Vertragspräzision und
zwei weniger zentrale Grenzgoldens bleiben unter PB-02.

### Bestätigtes offenes Verhaltensresidual

#### RVST-03R — Die 4-KiB-Reserve macht den ersten Save beim nächsten Load selbst read-only

**Status:** bestätigt, hoch. Confidence: sehr hoch. Frischer Runtime-Gegenbeleg.

Der neue Leser definiert:

- absolute Grenze: 16.777.216 Byte;
- schreibbarer Eingang: höchstens 16.773.120 Byte (= 16 MiB − 4 KiB);
- jeder bekannte Root zwischen beiden Grenzen wird ohne JUCE-Interpretation
  read-only gehalten
  (`eq-copilot/plugin/state/NakamaState.cpp:42-55,279-287,704-710`).

Der neue Test baut exakt 16.773.120 Byte, lädt ihn schreibbar, setzt die heute
maximal erlaubten Userfelder und speichert. Das Ergebnis hat in diesem frischen
Lauf **16.773.307 Byte**: unter der absoluten 16-MiB-Grenze, aber 187 Byte über
der schreibbaren Eingangsschwelle. Genau deshalb klassifiziert derselbe Leser
den eigenen Save beim unmittelbar folgenden Load als read-only statt
`geladen`; `StateMigrationTestMain.cpp:940-978` fordert zu Recht einen erneut
schreibbaren Load.

Der direkte Lauf ergab:

```text
FEHLER  Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773307]
FEHLER  == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
STATE-MIGRATION-TEST FEHLGESCHLAGEN - 147 Pruefungen ok, 2 Fehler
```

Das sind **nicht zwei unabhängige Defekte**: Die zweite Zeile ist die
Abschnittssumme nach dem ersten Fehler. Der konkrete Fehler ist die inverse
Writer/Reader-Asymmetrie.

Die Änderung verhindert zwar, dass der Save die absolute 16-MiB-Grenze
überschreitet; sie erfüllt aber nicht die Vertragsbehauptung, derselbe Leser
lehne den eigenen Save beim nächsten Start nicht ab/degradiere ihn nicht
(`eq-copilot/schemas/state/nakama-state-v2.md:138-146`). Eine pauschale
Rohlängenschwelle kann die Reserve nicht „verbrauchen“ und das Ergebnis
gleichzeitig wieder schreibbar erkennen.

**Fixrichtung:** Schreibbarkeit muss aus dem maximalen **synchronisierten**
Writerergebnis beziehungsweise dessen konservativer Worst-Case-Größe folgen,
nicht nur aus der Eingangsrohlänge. Alternativ braucht `speichere` einen
ehrlichen Ergebnis-/Fallbackpfad und muss die Originalbytes eines geladenen
Grenzstates weiter halten. Ein bloßes Verschieben zweier fester Schwellen löst
die Wiederlade-Invariante nicht allgemein, solange spätere erlaubte Änderungen
den State erneut vergrößern können.

### Verbleibende Proof-/Dokumentationsresiduen

#### PB-01 — Rust-Source geschlossen, vorhandenes Rust-Artefakt für den neuen Maximaltest stale

Die verfügbaren Rust-Release-Artefakte wurden um 01:14 gebaut, die aktuelle
`broker/src/vertrag.rs` um 01:16 geändert. Der ausgeführte 57er-Unit-Lauf listet
`textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python` nicht; auch der
8/8-Cross-Language-Lauf besitzt keine >16-MiB-Fixture. Beide Läufe sind grün,
belegen aber **nicht** die neue Rust-Grenze. Der Sourcezweig und sein Test sind
eindeutig vorhanden (`broker/src/vertrag.rs:178-191,892-896`); offen ist nur ein
frisch gebauter Rust-Beleg. Gemäß Paketauftrag wurde kein Build gestartet.

#### PB-02 — Grenzwortlaut und zwei Restkanten

- Der Vertrag sagt „64 Kindebenen“ und „64 ineinander geschachtelte
  Variantenarrays“ (`nakama-state-v2.md:133-136`). Source und Tests definieren
  tatsächlich **ValueTree-Tiefe 64 inklusive Wurzel** und **63 Arrays plus
  Blatt** (`NakamaState.cpp:172-175,250-267`,
  `StateMigrationTestMain.cpp:899-928`). Der Code ist begrenzt und die Tests
  sind trennscharf; der Vertrag sollte diese Zählweise wörtlich übernehmen oder
  die Implementierung auf die jetzige Formulierung anheben.
- Für `kMaxEintraegeGesamt = 262144` gibt es weiterhin keinen exakten
  N/N+1-Golden; geprüft wird nur das Limit je Sammlung
  (`NakamaState.cpp:160-169`, `StateMigrationTestMain.cpp:930-937`). Ebenso wird
  die absolute 16-MiB-Grenze oberhalb per frühem Dummy-Pointer geprüft, aber
  kein vollständiger bekannter State exakt bei 16 MiB auf read-only-
  Bytegleichheit gefahren (`StateMigrationTestMain.cpp:909-912,969-978`).

#### PB-03 — nach Grenzerweiterung stale Inline-Kommentare

Der kanonische README ist aktuell. Die API-/Funktionskommentare sind es nicht
vollständig: `NakamaVertrag.h:66-79` zählt weiterhin acht Regeln, nennt in
Regel 8 aber nur BOM/UTF-8 und nicht die neue Maximalgröße oder rohes NUL;
Rust/Python sprechen von drei Byte-Regeln, listen im Kommentar nur BOM und
kaputtes UTF-8 und haben durch das Maximum inzwischen ohnehin vier
(`broker/src/vertrag.rs:168-178`,
`pruefe_v3_vertrag.py:168-180`). Das ist stale Dokumentation, keine
Klassifikationsabweichung.

### Gezielte Prüfungen dieser Runde

| Prüfung | Ergebnis |
|---|---|
| aktuelles `EqCopStateMigrationTest.exe` (Buildzeit 01:20:10) | **Exit 1; 147 ok, 2 Fehlmeldungen; ein eindeutiger Reserve-/Reloaddefekt** |
| aktuelles `EqCopSchemaTest.exe` (Buildzeit 01:20:12) | Exit 0; 57/57; 16-MiB-Gegenpfad, 60 Hextabellen-, 165 JSON- und 55 Binärfixtures grün |
| vorhandenes Rust-Release-Unit-Artefakt | Exit 0; 57/57, aber vor aktuellem Source gebaut und ohne neuen Dokumentgrenztest |
| vorhandenes Rust-Cross-Language-Artefakt | Exit 0; 8/8, aber ohne Dokumentgrößenfall |
| `py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung` | Exit 0; 212/212, 165 Fixtures; neue 16-MiB-Prüfung grün |
| `py -3.13 tools/eq-copilot/erzeuge_v3_fixtures.py --pruefen` | Exit 0; 167 Dateien bytegleich |
| `py -3.13 tools/eq-copilot/erzeuge_state_fixtures.py --pruefen` | Exit 0; 36 Dateien bytegleich |

### Aktueller Reststand

- **Geschlossen:** VST-01 bis VST-04; RVST-01/RVST-02; gemeinsames
  v3-/DTO-Maximum RVST-04; Marker-Testlücke TL-01; wesentliche Tiefenkanten.
- **Offener Verhaltensfehler:** **1** — RVST-03R, Writerreserve/erneuter
  schreibbarer Load.
- **Offene Beweis-/Doku-Reste:** frischer Rust-Maximaltest, globale
  Eintrags-/absolute Stategrenze, exakter Grenzwortlaut und stale Inline-
  Kommentare.

Ein aktuelles S5-S7-Gesamtgrün ist damit nicht belegbar: Der Vertragskorpus ist
grün, aber der State-Gegenlauf ist reproduzierbar rot.

---

## Schlussrevalidierung nach zustandsabhängigem Writer-Headroom — 2026-08-27 01:33 CEST

Diese Runde supersediert ausdrücklich den roten 01:23-Zwischenlauf unmittelbar
darüber. Zwischen diesem Lauf und dem vorherigen Lauf wurden
`NakamaState.cpp`, `StateMigrationTestMain.cpp`, der Statevertrag und die beiden
C++-Testartefakte neu geschrieben beziehungsweise gebaut (Source 01:27:22/
01:27:38, Artefakte 01:27:51/01:27:53). Geprüft wurden ausschließlich die zuvor
auditierten Pfade. Es wurde nichts gebaut, installiert oder am Produktcode
geändert; geschrieben wurde nur dieses Paket.

### Geschlossen: RVST-03R — Writer/Reader-Symmetrie an 16 MiB

**Status:** geschlossen. Confidence: sehr hoch.

Die fehlerhafte feste 4-KiB-Eingangsschwelle existiert nicht mehr. Der
Byte-Riegel besitzt nur noch die absolute inklusive 16-MiB-Grenze
(`eq-copilot/plugin/state/NakamaState.cpp:42-52,263-267`). Nach vollständiger
semantischer Prüfung baut `hatWriterHeadroom` stattdessen über einer Kopie des
gehaltenen additiven Baums den größten heute über die Produkt-API erreichbaren
Folgezustand:

- `instance_id`, `label`, `pair_id` und `project_binding_id` werden mindestens
  auf ihre maximal erreichbare UTF-8-Größe gesetzt; für die 120/60-Zeichen-
  Felder verwendet die Prüfung tatsächlich U+10FFFF mit vier UTF-8-Bytes je
  Codepunkt (`NakamaState.cpp:510-554`);
- jede vom Bundle erlaubte Klasse/Position und die zugehörige
  `MainProject`-/`Parameters`-Matrix wird durchlaufen
  (`NakamaState.cpp:556-575`);
- jeder Kandidat wird mit demselben `synchronisiert`-Writer serialisiert und
  muss vollständig `<= 16 MiB` bleiben (`NakamaState.cpp:532-545`).

Die Prüfung läuft sowohl nach Schema-1-Migration als auch nach normalem
Schema-2-Load. Passt ein Kandidat nicht, hält der bestehende read-only-Pfad die
Originalbytes; andernfalls bleibt der State schreibbar
(`NakamaState.cpp:738-801`). Damit hängt die Entscheidung nicht mehr nur von
der Rohgröße des Inputs ab und das eigene Writerergebnis kann seine Reserve
nicht mehr selbst „verbrauchen“.

Der frische Gegenlauf trifft erneut exakt 16.773.120 Byte, speichert nach den
maximalen heute bekannten Userfeldern 16.773.307 Byte und lädt dieses Ergebnis
nun wieder als `geladen`. Ein State bei 16 MiB minus 64 Byte bleibt dagegen
read-only und wird bytegleich zurückgeschrieben
(`eq-copilot/plugin/tests/StateMigrationTestMain.cpp:940-979`). Das konkrete
frühere Gegenbeispiel ist damit in Source **und** Runtime geschlossen.

### Revalidierte Parser-/Sprachgrenzen

- Der ValueTree-Riegel erzwingt weiterhin vollständigen Verbrauch, getrennte
  Baum-/Variantentiefe und Eintragslimits
  (`NakamaState.cpp:145-169,156-231,234-267`). Marker 9 und unbekannte Marker
  auf einer bekannten Wurzel gehen vor JUCE in den read-only-Originalbytepfad
  (`NakamaState.cpp:219-228,753-757`); der Golden ist semantisch
  trennscharf (`StateMigrationTestMain.cpp:210-237,982-994`).
- C++, Rust und Python besitzen source-seitig weiter dasselbe inklusive
  16-MiB-Dokumentmaximum
  (`eq-copilot/plugin/vertrag/NakamaVertrag.h:26-29`,
  `broker/src/vertrag.rs:23,178-191`,
  `tools/eq-copilot/pruefe_v3_vertrag.py:42,169-191`). C++ und Python haben den
  `+1`-Gegenpfad in den frisch ausgeführten Tests bestanden. Für Rust ist der
  Source-Test vorhanden (`broker/src/vertrag.rs:893-896`), aber das vorhandene
  Artefakt ist älter; siehe PB-01.

### Noch offene konkrete Residuen

#### RB-01 — Grenztext und implementierte Zählweise sind nicht exakt identisch

**Status:** offen, niedrig; Vertrags-/Akzeptanzgrenze, kein ungebundener Parser.
Confidence: hoch.

Der Vertrag erlaubt wörtlich „höchstens 64 Kindebenen und 64 ineinander
geschachtelte Variantenarrays“
(`eq-copilot/schemas/state/nakama-state-v2.md:133-136`). Der Source prüft
dagegen `tiefe >= 64` bereits beim Eintritt in jeden Baum beziehungsweise jede
Variante (`NakamaState.cpp:156-159,234-237`). Die Tests definieren die Kante
daher als **64 ValueTree-Knoten inklusive Wurzel** und **63 Arrays plus
Skalarblatt**; Wurzel plus 64 Kinder beziehungsweise 64 Arrays plus Blatt
werden verworfen
(`StateMigrationTestMain.cpp:899-907,919-928`).

Das ist fail-closed und kein Rekursionsfehler, aber bei wörtlicher Lesart wird
ein vertraglich noch zulässiger Grenzstate als Müll ignoriert. Entscheidung
offen: Vertrag präzise auf „Tiefe 64 inklusive Wurzel/Blatt“ ändern oder die
beiden Implementierungsgrenzen um genau eine Ebene anheben. Ein Golden mit 64
leeren verschachtelten Arrays würde die Abweichung nicht entscheiden; nötig
ist ausdrücklich **64 Arrays plus ein Blatt**.

#### PB-01 — Rust-Maximaltest ist weiterhin nur source-, nicht runtime-belegt

**Status:** offene Beweislücke, kein bestätigter Sourcefehler.

`broker/src/vertrag.rs` ist von 01:16:13, die vorhandenen Release-Testartefakte
sind von 01:14:03/01:14:04. Der aktuelle Source enthält den 16-MiB-`+1`-Test
(`broker/src/vertrag.rs:893-896`), der ausgeführte Unit-Binärling listet jedoch
nur die alten 57 Tests und diesen Namen nicht. Der vorhandene Cross-Language-
Binärling prüft 8/8 bestehende Korpustests, besitzt aber ebenfalls keinen
Dokumentgrößenfall. Ein frischer Rust-Build wäre der fehlende Beleg; er wurde
gemäß dem Read-only-/Nicht-bauen-Auftrag nicht gestartet.

#### PB-02 — drei wichtige Grenzbehauptungen haben noch keinen exakten Golden

**Status:** Testlücken, Source derzeit plausibel/geschlossen.

1. `kMaxEintraegeGesamt = 262144` wird implementiert, aber nur 65.536/65.537
   je einzelner Sammlung wird exakt getestet
   (`NakamaState.cpp:45-46,145-153`,
   `StateMigrationTestMain.cpp:930-937`). Es fehlt Gesamt-N/N+1 über mehrere
   Sammlungen.
2. Oberhalb 16 MiB wird vor dem Bytezugriff getestet und ein State bei
   16 MiB minus 64 Byte wird read-only-bytegleich getestet; ein vollständiger,
   bekannter State **exakt** bei 16 MiB fehlt
   (`StateMigrationTestMain.cpp:909-912,970-979`).
3. Der neue Headroom berechnet den echten Vier-Byte-Worst-Case mit U+10FFFF,
   sein Laufzeittest mutiert `label`/`pair_id` jedoch nur mit ASCII `L`/`P`
   (`NakamaState.cpp:516-552`,
   `StateMigrationTestMain.cpp:953-968`). Ein direkter Grenztest mit 120/60
   Vier-Byte-Codepunkten würde die entscheidende UTF-8-Annahme dauerhaft
   beweisen.

#### PB-03 — Bytegate-Inlinekommentare zählen die neuen Regeln noch falsch

**Status:** stale Dokumentation, kein Urteilunterschied.

Der C++-Header nennt in seiner Byte-Regel weiterhin nur BOM/UTF-8, obwohl
rohes NUL und das Dokumentmaximum ebenfalls geprüft werden
(`eq-copilot/plugin/vertrag/NakamaVertrag.h:66-79`). Rust und Python schreiben
„Drei Regeln“, zeigen aber nur zwei Aufzählungspunkte; mit BOM, UTF-8, rohem
NUL und Maximum bestehen tatsächlich vier Byte-Regeln
(`broker/src/vertrag.rs:168-188`,
`tools/eq-copilot/pruefe_v3_vertrag.py:169-191`). Der kanonische v3-README ist
bereits korrekt; betroffen sind nur lokale Erklärtexte.

### Frische Prüfungen dieser Schlussrunde

| Prüfung | Ergebnis |
|---|---|
| aktuelles `EqCopStateMigrationTest.exe` (Build 01:27:51) | **Exit 0; 149 ok, 0 Fehler**; Writer-Reload, read-only-Bytegleichheit, Tiefe/Einträge und Marker grün |
| aktuelles `EqCopSchemaTest.exe` (Build 01:27:53) | **Exit 0; 57/57**; darunter 16-MiB-`+1`, 60 Textriegel-, 165 JSON- und 55 Binärfixtures |
| `py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung` | **Exit 0; 212/212**, 165 Fixtures |
| `py -3.13 tools/eq-copilot/erzeuge_state_fixtures.py --pruefen` | **Exit 0; 36 Dateien bytegleich**, keine verwaiste Datei |
| vorhandenes Rust-Release-Unit-Artefakt (Build 01:14:03) | Exit 0; 57/57, aber stale für den neuen Dokumentgrenztest |
| vorhandenes Rust-Cross-Language-Artefakt (Build 01:14:04) | Exit 0; 8/8; 60 Textriegel-, 165 JSON-, 55 Binärfixtures und DTO/JCS grün, aber ohne Größenfall |
| `git diff --check` auf den relevanten Vertrags-/Statequellen | Exit 0; nur bestehende LF→CRLF-Hinweise |

### Verbindlicher Reststand

- **Bestätigte offene Writer-/Save-Load-/Unknown-Major-Fehler:** 0.
- **Bestätigte offene C++/Rust/Python-Klassifikationsabweichungen im aktuellen
  Source:** 0.
- **Offene niedrige Vertrags-/Akzeptanzabweichung:** 1 (RB-01, exakte
  Tiefenzählung).
- **Offene Beweis-/Dokumentationskategorien:** 3 (PB-01 bis PB-03).

Damit ist der zuvor rote Statepfad aktuell grün und RVST-03R geschlossen. Ein
vollständiges „0 Restpunkte“ wäre erst nach Festlegung der 64er-Zählweise,
frischem Rust-Beleg und den genannten Grenzgoldens/dem Kommentarabgleich
ehrlich.

---

## Unabhängiger v2-IPC-Paritätsnachtrag — 2026-08-27 01:55 CEST

### Scope und Stand

Read-only verglichen wurden der laufende v2-Vertrag
`eq-copilot/schemas/eq-ipc.schema.json`, C++-Writer/-Reader in
`eq-copilot/plugin/src/PipeClient.{h,cpp}` sowie Rust-Deserialisierung und
Verbindungsautomat in `broker/src/{protokoll,server}.rs`. Für die beobachtbare
Folge wurden punktuell `broker/src/lib.rs` und der aktuelle C++-Lifecycle-Test
gelesen. Keine Produktdatei wurde geändert.

Während dieses Audits wurde der Parallelstand zweimal aktualisiert. Der erste
Wirebefund zu `hoermarkierung` ist im hier dokumentierten Endstand bereits
geschlossen: C++ schreibt das Feld (`PipeClient.cpp:108-112`), Rust hält es mit
abwärtskompatiblem Default (`protokoll.rs:97-108,417-447`) und der neue
Registerpfad sperrt fremde Evidenz bis zu einem nachweisbaren Reset
(`broker/src/lib.rs:218-325,386-417`). Die folgenden Befunde beziehen sich auf
den danach sichtbaren Stand von 01:54:54.

### Priorisierte bestätigte Abweichungen

#### V2P-01 — `additionalProperties:false` wird von keinem Reader durchgesetzt

**Schwere:** hoch/mittel. **Status:** bestätigt. Confidence: sehr hoch.

Der Vertrag schließt unbekannte Eigenschaften für alle sechs Nachrichten und
alle geschachtelten Objekte
(`eq-ipc.schema.json:17-21,27-42,51-79,84-103,124-152`). Keine der Rust-
Payloadstrukturen trägt jedoch `#[serde(deny_unknown_fields)]`
(`broker/src/protokoll.rs:22-159`). Serde ignoriert deshalb unbekannte Felder
auf Hello-, Sensor-, Audio-, Heartbeat-, Stats-, Measurement-, Fenster- und
Bye-Ebene. C++ macht dasselbe in Gegenrichtung: Welcome/Reject/ACK werden nur
über einzelne `getProperty`-Abfragen geprüft; eine exakte Schlüsselmenge wird
nicht verglichen (`PipeClient.cpp:306-335,385-397`).

Konkrete Urteilsdifferenz: Ein valides Hello oder ACK plus `"extra":1` fällt
am Schema, wird vom jeweiligen Code-Reader aber angenommen. Das ist nicht nur
Kosmetik: Ein Tippfehler in einem optionalen Feld wird still zum Default statt
als Vertragsbruch sichtbar.

**Fix/Test:** Alle Rust-Payloadstructs strikt deserialisieren und im C++-Reader
die exakten zulässigen Keysets für Welcome, Reject und ACK prüfen. Vor dem
Striktschalten müssen alle absichtlich additiven Felder (insbesondere das nun
vorhandene `hoermarkierung`) Teil der Structs sein. N/N+1-Fixtures für je ein
Extra auf Root- und jeder geschachtelten Ebene; C++-Fake-Peer mit Extra in
Welcome/ACK. Wenn additive Annahme gewollt ist, muss stattdessen der Vertrag an
allen betreffenden Stellen bewusst geöffnet werden.

#### V2P-02 — schemawidrige Messwerte werden angenommen; erkannte Fehler werden partiell verarbeitet

**Schwere:** hoch. **Status:** bestätigt. Confidence: sehr hoch.

Das Schema verlangt `aktiv_s >= 0` und `gesamt_s >= 0`
(`eq-ipc.schema.json:108-109`). `MessStand::ist_gueltig` prüft aktuell aber nur
LTAS-Länge und Zustandsenum (`broker/src/protokoll.rs:131-143`). Ein
Heartbeat mit `aktiv_s:-1` ist daher schema-ungültig, wird vom Broker aber als
gültiger Messstand gespeichert.

Auch für bereits erkannte Fehler ist das Verhalten nicht das versprochene
„ungültiges Paket verwerfen“: Bei falschem Zustand oder >512 LTAS-Werten setzt
der Server nur `messung=None`, verarbeitet Stats/Heartbeat weiter und sendet
weiterhin ein ACK (`broker/src/server.rs:643-691`). Da das Register einen
Messstand nur bei `Some(m)` ersetzt, bleibt ein früherer Stand intern erhalten;
ohne die nachgelagerte Sperrlogik kann ein alter Stand neben frischen
Heartbeats weiterleben. Der bestehende 600er-Test prüft nur den einfachen Fall
ohne vorherigen Messstand und fordert das ACK sogar ausdrücklich
(`server.rs:1064-1097`).

**Fix/Test:** `ist_gueltig` mindestens um Endlichkeit und Nichtnegativität von
`aktiv_s`/`gesamt_s` erweitern; alle weiteren numerischen Felder gegen ihre
Schemaaussage prüfen. Eine ungültige eingebettete Measurement macht nach JSON
Schema den ganzen Heartbeat ungültig: Verbindung eindeutig beenden oder einen
expliziten Fehler-ACK versionieren, nicht acklos hängen und nicht teilweise
übernehmen. Regression: erst gültigen Messstand speichern, dann je
`aktiv_s=-1`, `gesamt_s=-1`, falsches Enum und 513 LTAS-Werte senden; weder
Stats/Sequenz noch alter Messstand dürfen als frischer gültiger Gesamtstand
erscheinen.

#### V2P-03 — neue Code-Caps sind keine Vertrags-Caps

**Schwere:** mittel. **Status:** bestätigt. Confidence: sehr hoch.

Rust lehnt inzwischen Hello-Werte außerhalb folgender Grenzen ab:

- `sensor_id`: 1..64 Codepunkte;
- `plugin_version`: 1..64;
- `instance_nonce`: höchstens 64;
- `label`: höchstens 120;
- `pair_id`: höchstens 64;
- Samplerate: endlich, 0..768.000;
- Blockgröße: 0..65.536; Kanäle: 0..64
  (`broker/src/protokoll.rs:13-20,195-245`).

Der Vertrag kennt davon nur `label.maxLength=120` und die drei unteren
Audio-Grenzen (`eq-ipc.schema.json:25-46`). Damit sind zum Beispiel leere
`plugin_version`, 65 Zeichen `sensor_id` oder Samplerate 768.001 schema-gültig,
werden aber per Reject abgewiesen. In Gegenrichtung akzeptiert das Schema leere
oder beliebig lange Response-Texte; der C++-Reader verlangt jedoch
`reason` 1..512, `session_token` 1..256 und `broker_version` 1..128
(`PipeClient.cpp:47-55,313-330`).

**Fix/Test:** Die heute beabsichtigten Caps samt `minLength`/`maximum` in den
v2-Vertrag aufnehmen und aus einer gemeinsamen Tabelle mit N/N+1-Fällen
testen. Für `pair_id` die bewusste Lesergrenze 64 gegen die heutige
Plugin-Writergrenze 60 ausdrücklich benennen, statt beide still zu
vereinheitlichen.

#### V2P-04 — Bye-Required-Liste widerspricht dem Verbindungsautomaten

**Schwere:** mittel. **Status:** bestätigt. Confidence: sehr hoch.

Der Vertrag macht nur `type` zur Pflicht; `{"type":"bye"}` ist somit gültig
(`eq-ipc.schema.json:148-155`). Rust deserialisiert `session_token` passend dazu
als optional (`protokoll.rs:155-159`), der Server wertet ein fehlendes oder
abweichendes Token anschließend aber als Protokollfehler
(`server.rs:694-701`). Der neue Servertest bestätigt genau diese Ablehnung
(`server.rs:1123-1155`).

**Fix/Test:** Für den heutigen zustandsgebundenen Automaten
`session_token` in `bye.required` aufnehmen und `Bye` als nicht-optionalen
String modellieren. Fehlend, null, falsch und exakt richtig als gemeinsame
Schema-/Rust-Fälle fahren. Das Entfernen des gesamten Bye beim aktuellen
C++-Stop ist davon getrennt und zulässig: der Vertrag beschreibt den
Timeout-/Close-Fallback ausdrücklich.

#### V2P-05 — v1 kann entgegen der Kompatibilitätszusage Messdaten einspeisen

**Schwere:** mittel. **Status:** bestätigt. Confidence: hoch.

Schema- und Rust-Kommentar sagen, v1-Verbindungen lieferten keine Messdaten
(`eq-ipc.schema.json:5`, `protokoll.rs:4-6`). Im Server beeinflusst die
ausgehandelte Version aber nur, ob ein ACK geschrieben wird: Measurement wird
für v1 und v2 identisch validiert und an das Register gegeben
(`server.rs:584-595,643-691`). Ein v1-Hello gefolgt von einem Heartbeat mit
Measurement speichert daher einen Messstand, obwohl UI/Aggregat
`protokoll_version<=1` als „altes Plugin ohne Messdaten“ interpretieren.

**Fix/Test:** Bei `version < 2` Measurement (und gegebenenfalls v2-exklusive
Felder) verwerfen oder die Verbindung als Vertragsbruch beenden; E2E-Test mit
v1-Hello plus Measurement muss `messung=None` und weiterhin kein ACK zeigen.

#### V2P-06 — Null- und Integerbreiten bleiben schema-/codeverschieden

**Schwere:** niedrig. **Status:** bestätigt. Confidence: hoch.

- `Option<T>` akzeptiert bei Serde sowohl fehlend als auch explizit `null`.
  Rust nimmt daher `instance_nonce:null`, `stats:null` und `measurement:null`
  an (`protokoll.rs:39-70`), obwohl das Schema bei Anwesenheit String bzw.
  Objekt verlangt (`eq-ipc.schema.json:33,84-96`). Bei Bye wird `null` zunächst
  ebenfalls geparst und erst semantisch abgewiesen.
- Das Schema lässt Integer grundsätzlich unbeschränkt. Rust verwendet u32 für
  PID/Block/Kanäle/Fenstersprünge und u64/i64 für Sequenzen, Drops und
  Projektzeit (`protokoll.rs:30-86,146-177`). C++ serialisiert den u64-
  `frames_dropped` sogar als `juce::int64` und liest ACK-Sequenzen nur als
  int64 (`PipeClient.cpp:32-45,90,385-392`). Schema-gültige große Integer
  können deshalb je nach Bein schon am Typ fallen; ein Dropzähler oberhalb
  `INT64_MAX` würde C++ nicht mehr schema-konform positiv schreiben.

**Fix/Test:** Null entweder im Vertrag explizit erlauben oder über einen
„missing, aber nicht null“-Deserializer ablehnen. Für alle transportierten
Integer gemeinsame Min/Max-Grenzen festlegen; insbesondere Sequenz und Drops
auf eine von C++ und Rust identisch darstellbare Spanne begrenzen. Tests mit
`2.0`, u32+1, i64±1 und u64/INT64-Kante.

### Bereits geschlossen, aber noch ohne frischen Gesamtbeleg

`hoermarkierung` wird im aktuellen Source nicht mehr verloren. Der neu
hinzugekommene Registerpfad hält Markierung pro Verbindungs-Nonce, sperrt
fremde Messstände, blendet gesperrte Evidenz im Snapshot aus und verlangt nach
Ende einen messbaren Reset (`broker/src/lib.rs:104-124,248-325,392-417`). Für
diese umfangreiche Logik waren beim Schlusslesen noch keine gezielten
`hoermarkierung`-Tests im Registermodul vorhanden; das ist eine erhebliche
Proof-Lücke, kein erneut bestätigter Wirefehler. Notwendig sind mindestens:
Marker an/aus, fremde ID, gleiche ID mit zweiter Nonce, Disconnect ohne false,
Reconnect derselben/anderer Nonce, erster sauberer Stand nach Ende, tatsächlicher
Zählerrücklauf/Leerreset und Aggregat-/Paar-Sperre.

### Frische Belege und Testlücke

- Aktuelles `EqCopPipeClientTest.exe` (Build 01:40:34): **Exit 0, 0 Fehler**;
  Stop während Welcome/ACK, Reconnect-Generation, UTF-8/NUL und falsche
  ACK-Sequenz grün. Es enthält keine Extra-Property-, Response-Textgrenzen-
  oder vollständige Writer-Schema-Prüfung.
- `py -3.13 tools/eq-copilot/pruefe_v2_schemas.py`: **Exit 0, 5/5**. Das
  Skript prüft jedoch nur JSON, Metaschema und eingefrorene `$id`; keine einzige
  Nachrichteninstanz wird gegen das Schema oder gegen C++/Rust klassifiziert.
- Handvalidierung mit `jsonschema` bestätigte unter anderem:
  Extra-Hello/Extra-ACK und negative `aktiv_s` fallen am Schema; leere
  Pluginversion, Samplerate 768.001, Welcome mit leeren Strings und Bye ohne
  Token passieren das Schema. Die Codeurteile stehen jeweils oben dagegen.
- Rust-Source und Tests wurden während des Audits geändert, die vorhandenen
  Release-Testartefakte sind älter. Gemäß Read-only-Auftrag wurde kein Cargo-
  Build gestartet; ein frischer Rust-Gesamtlauf bleibt Beweisvoraussetzung.

### Kurzurteil v2

Der aktuelle Happy Path für Hello → Welcome → Heartbeat → ACK ist feldseitig
plausibel und der Frame-Cap 262.144 stimmt in Schema, C++ und Rust. Strikte
Schema-Parität besteht dennoch nicht: **6 bestätigte Abweichungskategorien**
(V2P-01 bis V2P-06), davon V2P-02 mit unmittelbarer Auswirkung auf gespeicherte
Messwahrheit. Der kleinste belastbare Abschluss ist ein gemeinsamer v2-
Instanzkorpus, den JSON Schema, Rust-Parser plus Semantik und C++-Peerpfade mit
demselben erwarteten Urteil fahren.
