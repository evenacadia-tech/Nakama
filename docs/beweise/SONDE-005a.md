# Beweismanifest — S5 «`SONDE-005a`: v3-JSON-Schemas, Bandgitter, Cross-Language-Fixtures»

**Ticket-Gate im Wortlaut** (`docs/FL-Nakama-Sonden-Design-Entwurf.md` §65):

> `SONDE-005` | v3-Domain-/JSON-/FlatBuffers-Schemas, Bandgitter und
> Cross-Language-Fixtures | **C++/Rust validieren identisch; Codegen-Drift ist 0**

**Sessionschnitt** (`docs/bauaufteilung-sonden.md` §3, P1):

> S5 | `SONDE-005a` | **v3-JSON-Schemas + Bandgitter + gültige/ungültige Fixtures** | T1+T2

Die zweite Gate-Hälfte — *Codegen-Drift ist 0* — betrifft FlatBuffers und
gehört ausdrücklich **S6/`SONDE-005b`**. Sie wird hier weder erfüllt noch
behauptet.

---

## 0. Was diese Session ausdrücklich NICHT beweist

Damit kein Prüfer es erst herausfinden muss:

1. **Dass irgendein Produktionscode diese Schemas benutzt.** Kein Processor und
   kein Broker-Pfad spricht heute v3. Das ist Schnittgrenze, keine Lücke: der
   v3-Client kommt in `SONDE-010`, der State in `SONDE-006`. Was hier steht,
   ist der Vertrag — und er ist prüfbar, bevor es einen Sprecher gibt.
2. **Den 16-Byte-Envelope und CRC32C.** Entwurf §65 gibt den Parser
   `SONDE-010`. Fixtures ohne Implementierung wären toter Ballast; sie sind
   deshalb nicht angelegt.
3. **RFC-8785-Kanonisierung und `state_hash`.** Gehört zu `SONDE-006`, wo er
   erstmals verbraucht wird. Die ES6-Zahlenserialisierung ist eine eigene
   Beweisfläche.
4. **Die acht reservierten Nachrichtenfamilien.** Ihre Nutzlasten (Proposal,
   DSP-DTO, Experiment) sind erst ab P4 entschieden. Reserviert sind hier nur
   ihre **Namen** — und dass der Parser sie ablehnt.
5. **Dass die Capability-Liste vollständig ist.** Ob `capabilities` den
   richtigen Satz führt, entscheidet erst der Capabilityreport aus **S4**, und
   der hängt an den beiden FL-Terminen des Users. Siehe §7.
6. **Verletzungsmengen des Referenzbeins.** `jsonschema` meldet bei `oneOf` die
   Fehler *aller* Zweige, die beiden eigenen Engines steigen über den
   Discriminator in genau einen ab. Das Referenzbein prüft deshalb nur das
   **Urteil**. Beide Verhalten sind korrekt; sie sind nur nicht vergleichbar.

---

## 1. Behauptungen

| # | Behauptung | Befehl | Ergebnis | Rohe Ausgabe | Datum |
|---|---|---|---|---|---|
| 1 | Das feine Bandgitter hat **221 Bänder** nach IEC 61260-1 (G=2, b=24 gerade ⇒ Halbschritt, f_r=1000 Hz), Kanten monoton und lückenlos | `erzeuge_bandgitter.py --pruefen` | ✅ 24 Prüfungen, 0 rot | [↓ P1](#p1) | 2026-08-21 |
| 2 | Das grobe Gitter ist eine **exakte Partition** der 221 feinen Bänder in 64 Gruppen; seine Kanten sind **bitgleiche Kopien** | `erzeuge_bandgitter.py --pruefen` | ✅ Summe 221, Größen {3,4} | [↓ P1](#p1) | 2026-08-21 |
| 3 | Beide Gitterdateien sind **bytegleich** zur Neuerzeugung (Determinismus) | `erzeuge_bandgitter.py --pruefen` | ✅ SHA-256 identisch | [↓ P1](#p1) | 2026-08-21 |
| 4 | Der Quantisierungsvertrag trägt **61 Testvektoren** inkl. Sättigung, NaN/±inf und der Rundungsfalle | `erzeuge_quantisierung.py --pruefen` | ✅ 0 rot | [↓ P2](#p2) | 2026-08-21 |
| 5 | Der Fixture-Korpus hat **34 gültige + 97 ungültige** Fixtures und ist bytegleich zur Neuerzeugung | `erzeuge_v3_fixtures.py --pruefen` | ✅ 132 Dateien | [↓ P3](#p3) | 2026-08-21 |
| 6 | Das Schema ist gültiges draft 2020-12, hält die **Engine-Teilmenge** ein, und `jsonschema` klassifiziert **jedes** Fixture wie das Manifest | `pruefe_v3_vertrag.py --abdeckung` | ✅ 144 Prüfungen, 0 rot | [↓ P4](#p4) | 2026-08-21 |
| 7 | **Abdeckungsriegel:** jede der 41 Definitionen hat ≥1 Negativfixture; `enum` 16/16 und `const` 4/4 hart abgedeckt | `pruefe_v3_vertrag.py --abdeckung` | ✅ „keine" | [↓ P4](#p4) | 2026-08-21 |
| 8 | **Rust** klassifiziert den Korpus wie das Manifest — Urteil UND Verletzungsmenge — und liest Bandgitter + Quantisierung bitgleich | `cargo test` | ✅ 46 + 3 Tests | [↓ P5](#p5) | 2026-08-21 |
| 9 | **C++** ebenso, gegen dasselbe Manifest | `EqCopSchemaTest` | ✅ 33/33 | [↓ P6](#p6) | 2026-08-21 |
| 10 | **Die Riegel fallen nachweislich** — unbekanntes Schlüsselwort, unbekanntes Muster, falsche Manifesterwartung, mutiertes Fixture | siehe P7 | ✅ je Exit ≠ 0 mit benannter Ursache | [↓ P7](#p7) | 2026-08-21 |
| 11 | Der bestehende Kanon bleibt grün und **wächst von 7/7 auf 8/8** | `tools/beweise.ps1 -Bauen` | ✅ GRUEN | [↓ Kanon](#kanon-lauf---sonde-005a---v3-vertrag-bandgitter-cross-language-fixtures) | 2026-08-21 |
| 12 | Das Produktionsbundle bleibt hostfähig (kein Produktionsquelltext angefasst) | `pluginval --strictness-level 8` | ✅ SUCCESS, Exit 0 | [↓ P8](#p8) | 2026-08-21 |

---

## 2. Rohe Ausgaben

### P1 · Bandgitter <a id="p1"></a>

**Befehl:** `py -3.13 tools/eq-copilot/erzeuge_bandgitter.py --pruefen` · **Exitcode:** 0

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 24 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

**Warum 221 die Prüfsumme ist.** Der Entwurf nennt in §33.2 „volle 221 Bänder",
sagt aber nirgends, nach welcher Konvention. IEC 61260-1 / ANSI S1.11 kennen
zwei Fälle — `b` ungerade ⇒ `f_m = f_r·G^(x/b)`, `b` **gerade** ⇒
`f_m = f_r·G^((2x+1)/(2b))` — und zwei Basen (G = 2 oder G = 10^0,3). Von den
vier Kombinationen trifft **genau eine** die Zahl 221:

| Konvention | Bänder in 30 Hz … 18 kHz |
|---|---:|
| Basis-2, Halbschritt (**gewählt**) | **221** |
| Basis-2, ohne Halbschritt | 222 |
| Basis-10, Halbschritt | 222 |
| Basis-10, ohne Halbschritt | 222 |

Anschaulich heißt der Halbschritt: die **Kanten** liegen auf dem an 1 kHz
verankerten 1/24-Oktav-Gitter, die Mitten sind deren geometrische Mittel. So
ist das Gitter auch erzeugt — es gibt nur EIN Kantenarray, und „obere Kante
von Band n == untere Kante von Band n+1" ist deshalb nicht auf 1e-12 genau,
sondern **dasselbe Bit**.

### P2 · Quantisierungsvertrag <a id="p2"></a>

**Befehl:** `py -3.13 tools/eq-copilot/erzeuge_quantisierung.py --pruefen` · **Exitcode:** 0

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=a1aec4aaad39487aba044d0c46575ad6015ce896431ef3a5565efe132c4b45c1
```

### P3 · Fixture-Korpus <a id="p3"></a>

**Befehl:** `py -3.13 tools/eq-copilot/erzeuge_v3_fixtures.py --pruefen` · **Exitcode:** 0

```text
34 gueltige, 97 ungueltige Fixtures
  bytegleich: 132 Dateien, MANIFEST sha256=5f49894b9dab38bbd38099e04ffd3ded0217a1beda993b2021df4e286dfefce0
```

### P4 · Referenzbein (`jsonschema` 4.26) <a id="p4"></a>

**Befehl:** `py -3.13 tools/eq-copilot/pruefe_v3_vertrag.py --abdeckung` · **Exitcode:** 0

```text
jsonschema 4.26.0 (draft 2020-12)
41 Definitionen, 17 Nachrichtenfamilien, 131 Fixtures
Pruefungen: 144 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      5 / 35
  const                     4 / 4    HART
  enum                     16 / 16   HART
  exclusiveMinimum          2 / 2
  items                     1 / 3
  maxItems                  2 / 3
  maxLength                 2 / 14
  maxProperties             2 / 3
  maximum                   4 / 14
  minItems                  2 / 3
  minLength                 3 / 8
  minimum                   6 / 40
  oneOf                     2 / 2
  pattern                   1 / 1
  required                 26 / 149
  type                      8 / 123

$defs ohne Negativfixture: keine
```

**Zur Unterabdeckung, offen gesagt:** `required` ist mit 26 von 149 abgedeckt,
`type` mit 8 von 123. Der harte Riegel greift **je Definition** (alle 41 haben
ein Negativfixture) und **je Wertedomäne** (`enum` 16/16, `const` 4/4) — dort
ist ein falscher Wert ein echtes Produktrisiko. Ein Fixture je einzelnem
Pflichtfeld wären ~150 Dateien, die alle denselben Engine-Pfad prüfen. Die
Quoten stehen **vollständig** in der Ausgabe, damit die Entscheidung sichtbar
ist statt still.

### P5 · Rust-Bein <a id="p5"></a>

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` · **Exitcode:** 0

```text
running 46 tests
test result: ok. 46 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out

running 3 tests
test contract_cross_language::korpus_klassifiziert_wie_das_manifest ... ok
test contract_cross_language::bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test contract_cross_language::quantisierung_stimmt_mit_den_testvektoren ... ok
test result: ok. 3 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out
```

(36 der 46 Modultests standen vorher; 10 sind die Engine-Tests aus
`src/vertrag.rs`. Voller Lauf siehe Kanon-Abschnitt A4.)

### P6 · C++-Bein <a id="p6"></a>

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` · **Exitcode:** 0

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Korpus klassifiziert wie das Manifest (131 Fixtures)
[ok]   Korpus hat Substanz  [131 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
33 bestanden, 0 gescheitert
```

### P7 · Die Riegel fallen — gemessen, nicht behauptet <a id="p7"></a>

Ein grüner Test ist nichts wert, solange nicht gezeigt wurde, dass er
überhaupt fallen **kann**.

**a) Unbekanntes Schlüsselwort im Schema** (`multipleOf` an `heartbeat/sequence`):

```text
  ROT: nur implementierte Schluesselwoerter — #/$defs/heartbeat/properties/sequence/multipleOf
exit=2
```

Das ist der wichtigste Riegel des ganzen Entwurfs: JSON Schema **ignoriert
unbekannte Schlüsselwörter absichtlich**. Ohne diese Prüfung würde ein später
ergänztes `multipleOf` vom Referenzbein durchgesetzt und von beiden eigenen
Engines stillschweigend übergangen — die Einschränkung verschwände auf zwei
von drei Seiten, und nur ein Fixture, das zufällig genau darauf zielt, hätte
es gefunden.

**b) Unbekanntes Muster** (`^S-1-.*$` an `sid`):

```text
Pruefungen: 143 bestanden, 1 gescheitert
  ROT: nur Muster aus der Tabelle — #/$defs/sid: '^S-1-.*$'
```

**c) Falsche Erwartung im Manifest** (`capability-unknown` auf „gültig" gesetzt):

```text
Pruefungen: 141 bestanden, 3 gescheitert
  ROT: ungueltig/capability-unknown.json: jsonschema sagt ungueltig …
  ROT: Manifestzahlen stimmen
  ROT: gueltige Fixtures tragen keine Verletzungen
```

**d) Mutiertes Fixture gegen das C++-Bein** (gültiges `heartbeat` bekommt
`capability: "unknown"`):

```text
[ROT]  gueltig/heartbeat.json: Urteil gueltig erwartet, Engine sagt ungueltig {/capabilities/process_context | #/$defs/capability_wert/enum | enum}
[ROT]  Korpus klassifiziert wie das Manifest (131 Fixtures)  [1 Abweichungen]
32 bestanden, 1 gescheitert
exit=1
```

**e) Mutierte Gitterdatei** (eine Gruppengrenze von 3 auf 4 verbogen):

```text
  ROT: …\nakama_log64_v1.json weicht von der Neuerzeugung ab
exit=2
```

Dazu kommen **14 Riegelproben im C++-Test selbst** (siehe P6, erste Zeilen) und
**10 Modultests** auf der Rust-Seite — beide bringen jede Ladeprüfung und jede
Verhaltensregel einmal zum Fallen, ohne dass jemand von Hand mutieren muss.

### P8 · Echter VST3-Host <a id="p8"></a>

Kein Produktionsquelltext wurde angefasst — `git diff` über
`eq-copilot/plugin/src` ist leer, die einzige Änderung an `CMakeLists.txt` ist
das **neue** Ziel `EqCopSchemaTest`. Trotzdem gemessen, weil der Runner das
Bundle neu gebaut hat:

**Befehl:** `pluginval --strictness-level 8 --validate "…\EQ-Copilot.vst3" --skip-gui-tests` · **Exitcode:** 0

```text
Starting tests in: pluginval / Fuzz parameters...
Completed tests in pluginval / Fuzz parameters
SUCCESS
```

---

## 3. T1 — Selbstaudit je Commit

Liste aus `docs/bauaufteilung-sonden.md` §2:

1. **Numerische Ränder.** NaN, ±inf, ±0, Sättigung oben und unten und die
   Rundungs-Halbwerte stehen als Testvektoren im Quantisierungsvertrag; das
   Bandgitter wird auf `isfinite` und strenge Monotonie geprüft. Die
   Schema-Engine vergleicht Zahlen **numerisch** (3 == 3.0), weil `serde_json`
   und `juce::var` sie strukturell unterscheiden.
2. **Gegenpfad vorhanden?** Jeder Erzeuger hat `schreiben ↔ --pruefen`; jede
   Engine hat `laden ↔ pruefe`; jeder additive Objekttyp hat ein
   akzeptiertes-Unbekanntes- **und** ein `maxProperties`-Fixture; jede
   reservierte Familie hat ihr Eigentümerticket.
3. **Behauptungs-Integrität.** Jede in `schemas/v3/README.md` genannte Datei
   existiert: `tools/eq-copilot/pruefe_v3_vertrag.py`,
   `eq-copilot/plugin/vertrag/NakamaVertrag.{h,cpp}`,
   `broker/tests/contract_cross_language.rs`. Die Zahl 25 Nachrichtenfamilien
   wird **maschinell** gegen §33.3 geprüft (17 definiert + 8 reserviert,
   disjunkt).
4. **Lügt der UI-Text?** Keine UI in diesem Ticket.
5. **Anzeige-Pflichten.** Keine UI. Die Felder, die eine spätere UI zeigen
   muss (`frische.stale`, `konfidenz.klasse`, `capabilities`), sind im Vertrag
   Pflichtfelder — jedes mit Negativfixture.
6. **Audiothread.** Nicht berührt. `EqCopSchemaTest` ist eine Konsolen-App,
   `broker/src/vertrag.rs` läuft im Broker-Prozess.

---

## 4. Befunde aus dieser Session

Kein Befund verschwindet still.

| # | Befund | Ausgang |
|---|---|---|
| 1 | **Der `.gitattributes`-Riegel fehlte für den v3-Baum.** Der Gitter-Erzeuger vergleicht mit `--pruefen` **bytegleich**; mit `core.autocrlf=true` hätte eine frische Klonung CRLF bekommen und die Prüfung wäre auf dem Zweitrechner rot geworden, ohne dass sich ein Wert geändert hat. | gefixt (`eq-copilot/schemas/v3/** -text`, `fixtures/v3/** -text`); Index- und Arbeitskopie-SHA-256 als gleich gemessen |
| 2 | **Die eigene Rundungsregel war falsch.** `floor(a+0.5)` verfehlt `0.49999999999999994`; die naheliegende Nachbesserung `y - a > 0.5` greift **nie**, weil die Differenz auf exakt 0.5 rundet. | gefixt (Nachkommateil `a - floor(a)`, exakt); der eigene Testvektor hat es widerlegt, bevor eine Zeile C++ existierte |
| 3 | **14 Definitionen hatten kein einziges Negativfixture.** Mit 102 Fixtures war alles grün und der Korpus sah vollständig aus. | gefixt (29 Fixtures ergänzt, jetzt 131); der Abdeckungsriegel meldet sie **namentlich** |
| 4 | **Geschachtelter Discriminator wurde nicht gefunden.** `hello` ist ein Wrapper ohne eigene `properties`; die äußere Suche schaute nur auf `properties/type/const` und lehnte alle drei `hello`-Fixtures ab. | gefixt (Abstieg durch geschachtelte `oneOf`, Wert nur bei Einigkeit aller Untervarianten) |
| 5 | **Nicht-Objekt an der Wurzel meldete an `/type`** — ein Pfad, den es dort nicht gibt. | gefixt (Meldung an der Instanz selbst), im README festgehalten |
| 6 | **Eine handgeschriebene Erwartung war falsch** (`gitter-erfunden` zeigte auf `/baender` statt `/baender/gitter_id`). | gefixt; genau der Fehler, den ein aus einer Engine **erzeugtes** Manifest verdeckt hätte |
| 7 | **Panicpfad in der Rust-Engine**, bevor er auftreten konnte: der gewählte `oneOf`-Zweig wurde aus seinem Schemapfad zurückgerechnet — bei einem Zweig ohne `$ref` hätte das einen zu kurzen String zugeschnitten. | gefixt (Zweig wird mitgeführt) |
| 8 | **Gemessene Abweichung zwischen den Beinen:** ein Dokument mit skalarer Wurzel wird von JUCE (RFC 4627) schon im **Parser** abgelehnt, von `serde_json`/Python (RFC 8259) erst am **Schema**. | **kein Fix — dokumentiert.** Beide lehnen ab; nur die Verletzungsmengen sind nicht vergleichbar. Manifest-Feld `wurzel_skalar`, eigener Prüfzweig im C++-Bein, Abschnitt im README |
| 9 | **Der neue Quellort landete korrupt im Beweis-Runner** (`eq-copilot\plugin` + Vertical-Tab + `ertrag`), weil ein Skript einen Backslash verlor. Der Baustand-Scan hätte `vertrag/` nie gesehen und eine veraltete Binärdatei als frisch beglaubigt. | gefixt; im ganzen Skript ist kein 0x0B mehr. Gefunden durch **Nachlesen der Datei**, nicht durch die Erfolgsmeldung des Skripts |
| 10 | `pluginval.exe` liegt nur unter `%TEMP%` und kann von Windows aufgeräumt werden, obwohl der Beweiskanon es zitiert. | out of scope → `docs/offene-punkte.md` **NAK-26** |

---

## 5. Entscheidungen, die der Entwurf offen ließ

Beides fällt hier und steht ab jetzt im Vertrag:

1. **`session_epoch` ist ein beim Projekt-Öffnen neu gezogenes 128-Bit-Token**,
   kein Zähler im Projekt-State. Zweck ist laut §32.1, zwei gleichzeitig
   geöffnete Kopien derselben `.flp` zu trennen — ein Zähler würde in beiden
   denselben Wert lesen und den Zweck verfehlen.
2. **`nakama_log64_v1` ist keine zweite Frequenzachse, sondern eine exakte
   Partition** der 221 feinen Bänder. Damit ist das Livespektrum die lineare
   Energiesumme der Evidenzbänder (§33.1) — keine zweite Filterbank, keine
   Interpolation, kein Doppelzählen — und die groben Kanten sind bitgleiche
   Kopien feiner Kanten.

---

## 6. T2 — Frischkontext-Prüfer

**Offen.** Der `evaluator`-Subagent ist in dieser Session nicht gelaufen; der
Arbeitsmodus dieser Sitzung startet keine Subagenten ohne ausdrückliche
Anforderung. Nach `docs/bauaufteilung-sonden.md` §2 ist T2 **blockierend** —
dieses Ticket gilt bis dahin als *gebaut und selbstgeprüft*, **nicht** als
abgeschlossen.

Was der Prüfer bekommen muss: der Diff `5299037..HEAD` (alle SONDE-005a-Commits, beginnend bei `63d9c21`), dieses Manifest und
der Gate-Text aus §65 im Wortlaut — nicht meine Zusammenfassung.

---

## 7. Schließungsvorbehalt (Entwurf §65)

> „`SONDE-005` darf parallel zu den Hostmessungen **vorbereitet**, aber erst
> nach deren Capabilityentscheidung **geschlossen** werden."

Der Vertrag ist deshalb so gebaut, dass die Hostantwort seine **Form** nicht
mehr ändern kann:

* `capability_wert` kennt `supported` und `unsupported` — und **nichts sonst**
  (§54: „Kein `unknown, später prüfen` darf P1 passieren"). Ein ungültiges
  Fixture belegt, dass `"unknown"` abgelehnt wird.
* Jedes hostabhängige Feld hängt an einem eigenen Gültigkeitsbit
  (`validity.*`, `cycle.bounds_valid`, `derived_sample_bounds.derivation`).

Was **S4** liefert, sind damit **Werte**, keine Felder. Nachzuziehen bleibt
genau ein Punkt: ob der Satz in `capabilities` vollständig ist. Bringt ein
Termin eine Fähigkeit, die hier fehlt, ist das eine additive Minor-Erweiterung
mit eigenem Fixture — kein Umbau.

---

---

## Kanon-Lauf - SONDE-005a - Abnahmelauf auf sauberem Arbeitsbaum

**Lauf:** 2026-08-21 13:22 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 8/8 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 13:22:11 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | be370cd SONDE-005a: Beweismanifest + Doku nachgezogen (Kanon 8/8) |
| Commit (voll) | be370cd912d39d128d2c81ee1f5105f3f7b7478a |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-18 11:46:33 | `A6F71DAE47C7BE10` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-18 11:47:19 | `3456136DDD939B7F` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-20 21:28:44 | `AFEC07D04C9A80A6` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-21 13:13:44 | `FAB8D6374825D71F` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 13:13:42**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,05 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,42 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 5,67 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,41 s | [↓ A4](#a4) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
  ok      Latenz ist 0 Samples
  ok      Tail ist 0 s
  ok      1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)
  ok      kein falscher NaN-Alarm bei endlichem Signal
  ok      NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)
  ok      NaN/Inf wurde als Mangel registriert
  ok      Mono-Layout wird angenommen
  ok      5.1 wird klar abgelehnt (kein stilles Downmixen)
  ok      Save/Load: Sensor-ID bleibt stabil
  ok      Save/Load: Rolle und Paar-ID bleiben
NULLTEST OK
```

stderr:

_(leer)_

<a id="a2"></a>
#### A2 | EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,42 s

stdout:

```text
sinus-1k-minus12     32 ok, 0 Fehler
pink-minus20         39 ok, 0 Fehler
resonanz-116hz       41 ok, 0 Fehler
stille-mit-tick       7 ok, 0 Fehler
diag-scoop-mitte     10 ok, 0 Fehler
diag-mulm            12 ok, 0 Fehler
diag-haerte           8 ok, 0 Fehler
diag-mulm-halb       10 ok, 0 Fehler
diag-wander-ton      25 ok, 0 Fehler
stereo-antiphase     10 ok, 0 Fehler
stereo-nur-links      8 ok, 0 Fehler
stereo-nur-rechts     5 ok, 0 Fehler
stereo-dekorr         7 ok, 0 Fehler
nan-recovery          9 ok, 0 Fehler
nyquist-32k           4 ok, 0 Fehler
leicht-publikation    8 ok, 0 Fehler
konvergenz            4 ok, 0 Fehler

GOLDEN OK — 239 Pruefungen ok, 0 Fehler (metrics m4.1-2026-08-15 · diagnose m4.1-2026-08-15)
```

stderr:

_(leer)_

<a id="a3"></a>
#### A3 | EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 5,67 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,41 s

stdout:

```text

running 46 tests
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::hin_und_zurueck ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::stale_wird_sichtbar_aber_nie_entfernt ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::feindliches_ltas_array_wird_gezaehlt_verworfen ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 46 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 3 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 3 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.03s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-a60741d6aa4f32af.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-b91f868b743e27b1.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-c17ffc214d5eff74.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-65940c0c5ff0c0d4.exe)
   Doc-tests eqcop_broker
```

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: PLUGIN_CODE unveraendert  [Eqcp]
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE unveraendert  [Evna]
  ok      CMake-Quelle: PRODUCT_NAME unveraendert
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 1:22:17pm vs 21 Aug 2026 1:12:14pm]
  ok      moduleinfo.json ist nach dem Kommaputz parsebar
  ok      moduleinfo: Produktname wie im Manifest  [EQ-Copilot]
  ok      moduleinfo: Vendor wie im Manifest  [evenacadia]
  ok      moduleinfo: genau zwei Klassen (Component + Controller)  [2]
  ok      moduleinfo: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      moduleinfo: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      moduleinfo: Unterkategorien wie im Manifest
  ok      moduleinfo: keine der vier reservierten Sonden-CIDs im heutigen Bundle
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      CID nachgerechnet: active-probe Component (NkAc)  [ABCDEF019182FAEB45766E614E6B4163]
  ok      CID nachgerechnet: active-probe Controller (NkAc)  [ABCDEF011234ABCD45766E614E6B4163]
  ok      VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus  [5653544571637065712D636F70696C6F]
  ok      die echte CID stammt aus dem jucePluginId-Pfad
  ok      Golden-Ordner vorhanden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\identity]
  ok      Rolle 'sensor' geladen  [sensor]
  ok      Rolle 'sensor': Sensor-ID unveraendert uebernommen
  ok      Rolle 'sensor': Label uebernommen  [Klavier A]
  ok      Rolle 'sensor': Paar-ID uebernommen
  ok      Golden 'sensor' gelesen  [state-schema1-sensor.bin]
  ok      Schema-1-State 'sensor' ist bytegleich zum Golden  [128 Bytes]
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-State 'hub' ist bytegleich zum Golden  [125 Bytes]
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-State 'pre' ist bytegleich zum Golden  [133 Bytes]
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-State 'post' ist bytegleich zum Golden  [135 Bytes]
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 63 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
== A · Quellhash-Gate (JUCE 8.0.9) ==
  ok      JUCE-Wrapper liegt am gepinnten Ort: C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp
  ok      Patchdatei liegt im Repo: juce-8.0.9-nakama-vst3-bridge.patch
  ok      die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)
  ok      Wrapper liess sich lesen
  ok      Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: 6e5d4660d960836a…)
  ok      Wrapper ist NICHT der unberuehrte Auslieferungsstand
  ok      Patch: Bruecken-Header eingebunden
  ok      Patch: IAudioPresentationLatency als Basisklasse
  ok      Patch: Interface auch im queryInterface
  ok      Patch: Buslatenz-Methode umgesetzt
  ok      Patch: Blockbeginn verdrahtet
  ok      Patch: Context-Anwesenheit verdrahtet
  ok      Patch: fehlender Context wird ausdruecklich gemeldet
  ok      Patch: alle Parameterpunkte werden beobachtet
  ok      Patch: Uebergabe an die Senke verdrahtet
  ok      Patch: Bruecke als Wrapper-Member
  ok      Patch: Senke wird EINMAL beim Erzeugen aufgeloest
  ok      Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert
  ok      ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)
  ok      Patchdatei ist ein echter Unified Diff
  ok      Patch zielt auf den VST3-Wrapper
== B · Context-Anwesenheit und Gueltigkeitsbits ==
  ok      Senke wird genau einmal je Block gerufen
  ok      fehlender Context: processContextPresent == false
  ok      fehlender Context: Projektzeit ist UNGUELTIG, nicht 0
  ok      fehlender Context: Tempo und PPQ ungueltig
  ok      genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)
  ok      genullter Context: Projektzeit gueltig und 0 â eine Aussage, keine Vermutung
  ok      genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)
  ok      genullter Context: Tempo bleibt ungueltig
  ok      genullter Context: playing ist gueltig false â nicht 'unbekannt'
  ok      voller Context: anwesend
  ok      voller Context: projectTimeSamples 123456
  ok      voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)
  ok      voller Context: playing true
  ok      voller Context: recording true
  ok      voller Context: Tempo 128
  ok      voller Context: PPQ 8.25
  ok      voller Context: Taktbeginn 8.0 PPQ
  ok      voller Context: Cycle 4.0â12.0 PPQ, aktiv (PPQ, nicht Samples)
  ok      Teil-Context: playing true
  ok      Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)
  ok      Teil-Context: Cycle nicht aktiv
  ok      beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)
== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==
  ok      drei Punkte angekommen
  ok      Punkte sind nach Sample-Offset sortiert
  ok      sauberer Block: sampleAccurateAutomation == true
  ok      Blockgroesse wird mitgefuehrt
  ok      kleinerer Offset zuerst, auch wenn spaeter geliefert
  ok      gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)
  ok      Punkte aus mehreren Queues sind global aufsteigend
  ok      Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte
  ok      Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt
  ok      Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ueberlauf des Ereignisrings verwirft KEINEN Letztwert
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      NaN nach Ueberlauf: Zusicherung faellt
  ok      NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Letztwert-Tabelle: genau kMaxLetztwerte Parameter werden gefuehrt
  ok      Letztwert-Tabelle: die 3 ueberzaehligen Parameter sind gezaehlt
  ok      Letztwert-Ueberlauf laesst die Zusicherung ebenfalls fallen
  ok      Letztwert-Ueberlauf: der ueberzaehlige Parameter meldet ehrlich 'nichts'
  ok      ohne Ueberlauf: Rueckfallwert deckt sich mit dem letzten Ringeintrag (0.30)
  ok      ohne Ueberlauf: der Ring traegt denselben Wert am Ende
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      der Buszaehler steht im Blockbefund, ist also fuer die Senke sichtbar
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (7880 Byte)

HOSTKONTEXT OK — 91/91 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3b"></a>
#### B3b | EqCopHostProbeTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==
  ok      meldet 0 Samples Latenz
  ok      meldet 0 s Tail
  ok      200 Bloecke float: Ausgang bitgleich zum Eingang
  ok      double-Block: Ausgang bitgleich zum Eingang
  ok      double-Praezision wird dem Host angeboten
  ok      5.1 wird klar abgelehnt, nicht still gemischt
== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==
  ok      dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch
  ok      vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT
  ok      nach dem ersten Bruecken-Block meldet es: Bruecke liefert
== C - Context-Anwesenheit und Gueltigkeiten ==
  ok      acht Bloecke mit Context gezaehlt
  ok      der eine Block OHNE Context ist getrennt gezaehlt
  ok      neun verarbeitete Bloecke
  ok      neun Senke-Aufrufe - keine Asymmetrie ohne Grund
  ok      der Kontextverlust ist als Ereignis vermerkt
  ok      Tempo war in jedem Context gueltig
  ok      continousTimeSamples war NIE gueltig - und wird nicht erfunden
== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==
  ok      fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)
  ok      Seek vorwaerts wird als Sprung erkannt
  ok      die Sprungweite wird mitgemessen
  ok      Seek rueckwaerts wird als Sprung erkannt
  ok      nach Stop/Play wird der Positionswechsel NICHT als laufender Sprung gemeldet
  ok      er verschwindet aber auch nicht: eigener Zaehler 'ueber Stop/Play' (T2-Befund 21.08.)
  ok      und ein eigenes Ereignis, damit eine 0 bei den Spruengen nicht mehrdeutig ist
  ok      Stop und Play sind als Transportereignisse vermerkt
  ok      Loop-Wechsel erscheint als Rueckwaertssprung
  ok      Schleife-an ist genau einmal vermerkt
  ok      der Schleifenzustand wird gefuehrt
  ok      Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)
== E - Samplegenaue Automation: der Kernbeweis von Termin B ==
  ok      ohne Automation wird nichts behauptet
  ok      ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben
  ok      alle Punkte werden aufsummiert
  ok      der dichteste Block wird gemerkt
  ok      Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit
  ok      kleinster und groesster Offset werden gefuehrt
  ok      der erste Mehrpunkt-Block ist als Ereignis vermerkt
  ok      Mehrpunkt-Block OHNE Zusicherung zaehlt NICHT als samplegenau
  ok      er wird stattdessen getrennt gezaehlt - kein Befund verschwindet
  ok      Unplausibilitaeten und Ueberlaeufe der Bruecke werden AUFSUMMIERT uebernommen, nicht geglaettet
  ok      beide Bloecke ohne Zusicherung sind gezaehlt (der mit Offset- und der mit Wertfehler)
== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==
  ok      ohne Hostmeldung: 'nie gemeldet' - und KEIN Eintrag wird erfunden
  ok      Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten
  ok      die Gegenrichtung bleibt ungemeldet - es wird kein Wert erfunden
  ok      Ausgang Bus 0 = 1024 wird NEBEN dem Eingang gefuehrt, nicht verschluckt
  ok      auch ein dritter Bus kommt an
  ok      ein nicht gemeldeter Bus dazwischen bleibt ungemeldet
  ok      der zuerst gemeldete Latenzwert rastet ein
  ok      die spaetere Aenderung wird gezaehlt statt still verworfen
  ok      50 weitere Bloecke mit DEMSELBEN geaenderten Wert zaehlen NICHT weiter (Uebergaenge, nicht Bloecke)
  ok      ein dritter, wieder anderer Wert zaehlt als zweiter Uebergang
  ok      der Bericht listet genau die drei gemeldeten Eintraege - keinen erfundenen
  ok      und der Wertwechsel-Zaehler steht im Bericht - mit der Einheit im Schluesselnamen
== G - Senke ohne processBlock (Flush/Hostriegel) ==
  ok      Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt
  ok      der Block ohne Verarbeitung ist ausdruecklich vermerkt
== H - Gegenpfad: Bericht schreiben und wieder einlesen ==
  ok      der Bericht ist nicht leer
  ok      der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)
  ok      Schemaname steht im Bericht
  ok      der Bericht traegt die Kernaussage 'samplegenau belegt'
  ok      die Blockzahl im Bericht deckt sich mit der Messung
  ok      das Ereignisprotokoll liegt im Bericht
  ok      die Gueltigkeitsmaske steht im Bericht
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260821-132233.json
  ok      die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung
== I - Zuruecksetzen und Ringueberlauf ==
  ok      Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung
  ok      nach dem Zuruecksetzen wird die laufende Aufnahme wieder als aufnahme_an gemeldet
  ok      mehr Ereignisse als Ringplaetze wurden erzeugt
  ok      die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still
  ok      der Ring gibt genau seine Kapazitaet heraus
  ok      die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)
== I2 - Offline-Uebergang und negative Projektzeit ==
  ok      Offline- und Echtzeitbloecke getrennt gezaehlt
  ok      der Wechsel IN den Render ist vermerkt
  ok      der Wechsel ZURUECK in Echtzeit ist ebenfalls vermerkt - nicht nur der erste Block ueberhaupt
  ok      negative Projektzeit wird GEZAEHLT, nicht still uebersprungen
  ok      der erste negative Wert ist als Ereignis vermerkt
  ok      im negativen Bereich wird kein Scheinsprung erfunden
== J - Audiothread: keine Allokation ==
  ok      500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen
== J2 - Nebenlaeufig lesen, waehrend der Audiothread schreibt ==
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (20834 Bloecke)
  ok      200 nebenlaeufige Lesevorgaenge liefern durchweg plausible Eintraege und Zaehler
  ok      der letzte Lesevorgang hat Eintraege geliefert
  hinweis   Grenze: Rauchtest, kein Beweis der Tearing-Freiheit (der Wiederholpfad wird selten bis nie betreten)
== K - Anzeige: passt der Inhalt ueberhaupt ins Fenster? ==
  ok      der Editor laesst sich erzeugen
  ok      Fensterhoehe 850 deckt den Leerzustand (850 noetig)
  ok      Fensterhoehe deckt auch den Messzustand (850 noetig)
  ok      keine Textzeile liegt unter den Knoepfen (Text bis 760, Knopfstreifen ab 794)
  ok      keine Wertzeile ohne Beschriftung - und 27 Zeilen wurden dabei wirklich geprueft
  ok      hoechstens EINE unbeschriftete Hinweiszeile (gefunden: 1)
  ok      und der Riegel faellt: eine untergeschobene Wertzeile ohne Beschriftung wird erkannt

HOSTPROBE OK - 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3c"></a>
#### B3c | EqCopSchemaTest

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Korpus klassifiziert wie das Manifest (131 Fixtures)
[ok]   Korpus hat Substanz  [131 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
33 bestanden, 0 gescheitert
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 5,06 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostProbeTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_rc_lib.dir\Release\EqCopHostProbeTest_rc_lib.lib
  EqCopHostProbeTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSchemaTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_rc_lib.dir\Release\EqCopSchemaTest_rc_lib.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
```

</details>

