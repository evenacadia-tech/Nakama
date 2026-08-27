# Paket 08 — Finaler Contract-Adversary

Stand: 2026-08-27. Read-only-Integrationsreview des gemeinsamen Arbeitsbaums;
keine Produktdatei wurde durch dieses Paket geändert. Scope: Broker,
JSON-/Binärverträge, Host-State und Installer. FeatureEngine, Queue,
PluginProcessor sowie Design-/Kontextdateien blieben ausdrücklich außerhalb.

## Ergebnis

Zwei reale Gegenpfade wurden reproduziert:

| ID | Schwere | Endstatus | Urteil |
|---|---:|---|---|
| A18-RB-01 | hoch | **geschlossen** | Ein vorhandenes VST3-Bundle ohne erwartete innere Payload hätte zuerst einen nicht rückwegfähigen Journalzustand erzeugt. Der integrierte Installer verweigert es nun vor Transaktion und Zielmutation; A18 belegt den Gegenpfad. |
| V3-NUM-01 | hoch | **geschlossen** | Lexikalisch nichtganzzahlige Dezimalwerte konnten in binary64 zu Ganzzahlen runden und dadurch ein JSON-Schema-`integer` passieren. Zusätzlich wurde ein echter Rust-/Python-Wertdrift bei gleicher Bytefolge gemessen. Der v3-Schemaweg verwirft die gefährliche Präzisionsklasse nun vor allen Parsern. |

Im übrigen ergab die erneute adversariale Lesung der geänderten Broker-,
State-, Persistenz- und Rollbackpfade keinen weiteren reproduzierten
High-/Medium-Fehler. Die gezielte Schlussprüfung ist grün; verbleibende
Auslieferungs-/Beweisgrenzen stehen am Ende dieses Pakets.

## A18-RB-01 — unvollständiges Fremdbundle war nicht rückwegfähig

### Reproduktion vor der Korrektur

Ein am Ziel vorhandenes `.vst3`-Verzeichnis hatte einen gültigen Ordnerhash,
aber nicht `Contents/x86_64-win/<erwarteter Bundle-Name>`. Der Installationspfad
konnte daher einen Backup-/Journaleintrag mit `vorher_sha256_innen = null`
erzeugen. Der Rückweg verlangt für VST3-Sicherungen dagegen zwingend einen
beweisbaren inneren Hash und verweigerte die Wiederherstellung selbst mit
`-Erzwingen`. Damit konnte die Installation einen Vorzustand mutieren, den ihr
eigener inverse Pfad nicht mehr restaurieren durfte.

### Integrierter Endstand

`eq-copilot/install/Install-Nakama.ps1:917-930` prüft nun jedes vorhandene
VST3-Ziel vor Anlage einer neuen Transaktion. Fehlt die erwartete innere
Payload, endet der Lauf mit Exit 1 und
`Installation bleibt unveraendert`. Eine zweite Prüfung unmittelbar vor dem
Sicherungszug (`:948-955`) schließt die relevante Änderung zwischen Preflight
und Backup fail-closed.

Der A18-Gegenfall
`tools/eq-copilot/pruefe_installer_gegenpfad.py:197-208` baut genau ein solches
Fremdbundle. Der frische Lauf meldete:

```text
[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
Exit 1; Fremdbundle unveraendert; kein aktiver Anker
```

Die Policy kollidiert nicht mit vollständigen historischen/Legacy-Bundles:
diese besitzen den erwarteten inneren Payload und bleiben hashbar. Abgelehnt
werden nur gleichnamige, strukturell fremde oder unvollständige Bundles, deren
Vorzustand der bestehende NAK-41-Rückweg nicht beweisen könnte.

## V3-NUM-01 — nichtganzzahliges Literal wurde zum Schema-Integer

### Exakte Ganzzahlformen

Der ursprüngliche Textriegel band `2^53-1` nur an Literale ohne Punkt und
Exponent. Deshalb passierten unter anderem
`9007199254740992.0`, `90071992547409920e-1` und
`0.9007199254740992e16`. Die integrierte Zwischenkorrektur berechnet den Wert
sprachneutral als `digits * 10^scale` und erkennt damit mathematisch
ganzzahlige `.0`-/Exponentformen ohne Gleitkommaoperation. Legale negative Null
bleibt erlaubt; `00`, `01`, `-00` und `00.0` fallen bereits an der JSON-Grammatik.

### Adversarial reproduzierter Bypass

Auch ein mathematisch **nicht** ganzzahliges Literal kann beim Parsen exakt auf
eine binary64-Ganzzahl runden. Vier temporäre, echte Heartbeat-Fixtures setzten
jeweils das Schema-Integer `sequence` auf:

| Literal | Python `json` | Rust, normale Dependencies | Schemaurteil C++ / Rust / Python |
|---|---:|---:|---|
| `9007199254740991.1` | `9007199254740991.0` | `9007199254740990.0` | gültig / gültig / gültig |
| `9007199254740992.1` | `9007199254740992.0` | `9007199254740992.0` | gültig / gültig / gültig |
| `4503599627370495.9` | `4503599627370496.0` | `4503599627370496.0` | gültig / gültig / gültig |
| `1.00000000000000001` | `1.0` | `1.0` | gültig / gültig / gültig |

Messwege:

- Python: `json.loads` plus `Draft202012Validator` gegen das echte v3-Schema;
- Rust: temporäres Cargo-Programm mit normaler Path-Dependency auf den
  aktuellen Broker, `textriegel_bytes`, `serde_json` und
  `Schema::pruefe`; alle vier Fälle ergaben `gate=Ok`, `valid=true`,
  `violations=[]`;
- C++: Release-`EqCopSchemaTest.exe` gegen eine ausschließlich unter `%TEMP%`
  erweiterte Kopie des echten Schema-/Fixturebaums; alle vier temporären
  Fixtures waren als gültig klassifiziert (`169 Fixtures`, insgesamt 57/57).

Der erste Fall beweist zusätzlich Wertdrift, nicht nur einen Typ-Bypass: der
normale Rust-Dependencygraph und Python erzeugen aus denselben Bytes
verschiedene Zahlen. Ein separat direkt gegen den vorhandenen
`float_roundtrip`-Build gelinkter Rust-Parser ergab dagegen
`9007199254740991.0`; damit kann der Test-Dependencygraph den Produktionspfad
hier sogar unbeabsichtigt verändern.

### Integrierte sprachneutrale Schlussregel

Eine rein mathematische Integerprüfung reicht deshalb nicht. Der integrierte
v3-Schemaweg verwendet folgende parserunabhängige Eingangsmenge:

1. Exakte Null akzeptieren.
2. Mathematisch ganzzahlige Dezimal-/Exponentformen normalisieren und wie
   bisher exakt gegen `abs(x) <= 9007199254740991` prüfen.
3. Mathematisch nichtganzzahlige Formen nur mit höchstens 15 signifikanten
   Dezimalziffern akzeptieren (führende und wertneutrale abschließende Nullen
   ignorieren; der Exponent ändert die Signifikanz nicht).
4. Die bestehende exakte Exponent-/`abs(x) < 1e308`-Prüfung bleibt davor aktiv,
   sodass weder Integerarithmetik überläuft noch riesige Strings allokiert
   werden.

Die drei symmetrischen Implementierungen stehen in
`broker/src/vertrag.rs:129-208`,
`eq-copilot/plugin/vertrag/NakamaVertrag.cpp:288-398` und
`tools/eq-copilot/pruefe_v3_vertrag.py:123-202`; die gemeinsame, nicht aus
einer Engine abgeleitete Matrix steht in
`eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json`.

Die 15-Ziffern-Regel gilt nur für **nichtganzzahlige** Werte; die sichere
16-stellige Ganzzahlgrenze bleibt ausdrückbar. Minimale Matrix:

- akzeptieren: `9007199254740991`, `9007199254740991.0`,
  `90071992547409910e-1`, `0.9007199254740991e16`, `-0`, `-0.0`,
  `90071992547409.1`, `0.0000000000000000001`,
  `1.23456789012345e-100`;
- ablehnen: `9007199254740992.0`, `90071992547409920e-1`,
  `0.9007199254740992e16`, `9007199254740991.1`,
  `9007199254740992.1`, `4503599627370495.9`,
  `1.00000000000000001` sowie die grammatisch illegalen
  `00`, `01`, `-00`, `00.0`.

Die Policy ist absichtlich auf den binary64-basierten **v3-Schemaweg**
begrenzt. Der DSP-DTO-/JCS-Pfad besitzt einen eigenen korrekt gerundeten
Zahlenleser mit feldgenauer Typ-/Bereichsprüfung und ruft denselben Byte-/
Grammatikriegel mit deaktivierter Schema-Ganzzahlsicherung auf:

- C++ nur in `NakamaParameter.cpp:190-215` mit explizitem `false`;
- Rust über die crate-private Funktion
  `textriegel_bytes_fuer_exakten_zahlenleser` in `vertrag.rs:229-254`, deren
  einziger Produktcaller `dto.rs:269-286` ist;
- Python nur im State-Fixture-Referenzvalidator
  `erzeuge_state_fixtures.py:234-265` mit benanntem Keyword.

Alle normalen v3-Aufrufe behalten die sichere Defaultpolicy. Damit bleibt der
bereits eingefrorene DTO-/JCS-Wert `q = 0.7071067811865476` samt Kanonhash
gültig, während derselbe 16-stellige echte Bruch im v3-Schemaweg bewusst vor
binary64 fällt. Äquivalente Endnullformen
`90071992547409.1000` und `1.2300000000000000` passieren die v3-Regel; die
gemeinsame Falltabelle belegt, dass wertneutrale Endnullen nicht als zusätzliche
Präzision zählen.

## Weitere adversariale Lesung

- Brokerzahlen und IPC-v2: NaN/Inf, negative Zähler, Text-/Framegrenzen und
  Projektfenster werden vor Zustandsübernahme verworfen. Der v1-Handshake ist
  eine dokumentierte Kompatibilitätskante, kein v2-Schemaurteil.
- Brokerpersistenz: Tempdatei mit `create_new`, vollständigem Write,
  `sync_all` und atomarer/verzögerter Windows-Veröffentlichung; Snapshotnamen
  sind kollisionsfest und ersetzen keine vorhandene Datei.
- State: Bytegate, vollständiger Verbrauch, Struktur-/Typmatrix und
  Read-only-Originalbytes schließen die früheren Suffix-/Trunkierungs- und
  Save/Load-Verlustpfade. In der erneuten Matrixlesung wurde kein weiterer
  reproduzierter High-/Medium-Verlustpfad gefunden.
- Installer: Journalautoritat, Ziel-/Backup-Containment, Reparsepoint-Riegel,
  Zieldrift-Vorprüfung, partielle Hin-/Rückwegzustände und geschützter Lock
  wurden erneut gegeneinander gelesen. Kein weiterer reproduzierter
  High-/Medium-Pfad blieb offen.

## Frische Schlussprüfungen

- C++ Release-Build der Targets `EqCopSchemaTest` und
  `EqCopStateMigrationTest`: Exit 0.
- `EqCopSchemaTest.exe`: **57/57**, gemeinsame Textriegelfalltabelle
  **90/90**, JSON-Korpus **165 Fixtures**, FlatBuffers-Korpus **55 Fixtures**.
- `EqCopStateMigrationTest.exe`: **153/153**; darunter 6/6 DTO-Hashes,
  15/15 negative DTOs, 21 read-only-Fälle, vollständige Bytegrenzen und
  Save/Load-Bytegleichheit.
- Python-v3-Referenz: **212/212**; State-Fixture-Referenz: **36 bytegleich**,
  einschließlich `q = 0.7071067811865476`.
- Broker: **90 Unit- plus 8 Cross-Language-Tests** grün;
  `cargo clippy --all-targets -- -D warnings` Exit 0.
- A18-Installer-Sandbox: **80/80**; der neue Fall `[0b]` endet vor Mutation,
  lässt das Fremdbundle bytegleich und erzeugt keinen aktiven Anker.
- Zusätzlicher nicht persistierter Heartbeat-Gegenkorpus mit den vier oben
  genannten `sequence`-Literalen: C++ **61/61** bei **169 Fixtures**; Rust und
  Python meldeten für alle vier bereits am Textriegel
  `mehr als 15 signifikante Dezimalziffern`. Der Schema-Integer wird nicht mehr
  erreicht.

Es wurde keine echte Installation gestartet. A18 lief ausschließlich unter
`%TEMP%`; die C++-/Rust-Gegenkorpora lagen ebenfalls nur in eindeutig benannten
Tempordnern.

## Verbleibende Risiken und Beweisgrenzen

- Die Installer-Artefakthashes sind weiterhin bewusst `null`, ein
  Codesigning-Endbeleg und ein echter Install-/Scan-/Load-Lauf fehlen. Das ist
  eine Auslieferungssperre, kein durch dieses Paket neu gefundener Codepfad.
- Crashrecovery wurde durch reproduzierte Journalzwischenstände belegt, nicht
  durch einen realen Stromverlust oder Prozess-Kill während eines Dateisystem-
  Flushs. Power-loss-Dauerhaftigkeit bleibt deshalb eine Plattformbeweisgrenze.
- Niedrig: Der Rust-v2-Deserializer nimmt `bye.session_token` weiterhin als
  optional/null an, während das v2-Schema einen nichtleeren String verlangt.
  Das steht in `broker/src/protokoll.rs:213-218` gegen
  `eq-copilot/schemas/eq-ipc.schema.json:148-155`; der Server verwirft und
  schließt die Nachricht in `broker/src/server.rs:736-745` vor Zustandswirkung.
  Es ist Parser-/Schema-Drift, aber kein offener Autorisierungs- oder Statepfad.
- Die 15-Ziffern-Regel ist nun Teil des v3-Textvertrags. Neue v3-Produzenten
  müssen hochpräzise echte Brüche entsprechend kanonisieren; der getrennte
  DTO-/JCS-Vertrag ist davon ausdrücklich ausgenommen.

## Parent-Nachintegration

Die vier im Adversary zunächst nur unter `%TEMP%` aufgebauten Heartbeat-Fälle
sind anschließend über `rohtext_faelle()` des kanonischen Generators dauerhaft
in `fixtures/v3/ungueltig/` übernommen worden. Der Endkorpus umfasst damit
**169 JSON-Fixtures**. Frisch auf der persistenten Fassung: Python **216/216**,
C++ **61/61** (einschließlich aller vier benannten Ablehnungen), Rust
**90 Unit- plus 8 Cross-Language-Tests** und der kanonische Gesamtbeweis
**29/29**. Die frühere Formulierung „nicht persistiert“ beschreibt nur den
Reproduktionszeitpunkt dieses Pakets, nicht den ausgelieferten Auditstand.
