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

Was der Prüfer bekommen muss: der Diff `3582ae4^..HEAD`, dieses Manifest und
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
