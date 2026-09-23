# NAK-313 — Quellvalidierung Teil B (lesender Opus-Agent, HEAD 43549a0e)

**Ticket:** NAK-313 · Planschritt S25l „Tiefenaudit 3 abarbeiten, Teil 5: State und
Verträge" · Etappe 0 „Quellvalidierung", **Teil B** (W30: JSON- und FlatBuffers-Vertrag
an jedem Produkteingang)
**HEAD:** `43549a0eab4d810cbe9e148e7c8962bc47190042` (Zweig `master`)
**Datum:** 23.09.2026, Lauf 19:11 bis 19:48 (+0200, beide Zeiten mit `date` gemessen) ·
**Modell:** Opus 5.5 (lesend)
**IDs (8):** T3-03-01, T3-03-02, T3-03-04, T3-03-05, T3-03-06, T3-03-07, T3-03-08,
T3-03-09 (Arbeitsübersicht W30; Bündel B12 und B13).
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (Phasenzeilen
`:378-379`, `:381-386`; Skeptikerzeilen Phase 10 `:512-513`, `:515-520`; Bündel B12 `:82`,
B13 `:83`; Kartenentwurf PF11-14 `:302-307`; Arbeitsübersicht W30 `:833`) und die
Rohberichte unter `docs/audits/2026-09-15-tiefenaudit/roh/`: `phase-03-vertrag.md`,
`phase-08-broker.md` (nur Vertragsstellen), `phase-10-skeptiker.md`, `phase-11-synthese.md`,
`phase-16-b-verbindungen.md`, `phase-16-f-gegenpruefung.md`, `phase-16-abdeckung.md`.
**Gate:** Schritt S25l in `docs/plan/plan.json:366-371` (Feld `text` in `:369`);
Registerzeilen `docs/offene-punkte.md:317` (NAK-313) und `:498` (NAK-29).
**Muster:** `docs/beweise/roh/NAK-312-quellvalidierung-teil-A.md` (Form, Kopf) und
`docs/beweise/roh/NAK-312-quellvalidierung-teil-B.md` (Trennung Technik und Produkt).

**Gate-Text wörtlich** (`docs/plan/plan.json:369`, Feld `text`):

> „ANGELEGT 18.09.2026 (Register NAK-313; Arbeitsübersicht Phase 16 W10, W11, W30, W31,
> W33). INHALT: 16 Befunde an State und Verträgen — T3-02-02, T3-02-04, T3-02-05 (Writer
> macht grenzgroßen State unlesbar, Panel-Abbau verliert Label/Paar-ID, doppelte
> ValueTree-Properties), T3-02-01, T3-02-03, T3-03-03 (Revisionsbereich, Überlauf,
> ACK-Revision 0), T3-03-01, T3-03-02, T3-03-04 bis T3-03-09 (JSON- und
> FlatBuffers-Vertrag an jedem Produkteingang: null-Startsample, 1.0/1e0, lockere Syntax,
> Option/null/Text, UTF-8-Längen, zusätzliche FlatBuffer-Slots, v2-Integerüberlauf,
> doppelte Schlüssel), T3-03-10 (binary_telemetry), T3-14-04 (Exportkollision in
> derselben Sekunde). GRENZE: Schemas sind Verträge — versionieren, Altstände laden,
> unbekannte Felder erhalten, Save und Load gemeinsam, Discriminator/Zieladresse/
> Revision/Capability nicht additiv; Fixtures bytegleich mit .gitattributes;
> Cross-Language-Gleichheit C++/Rust; keine stillen Rundungen. Produktentscheide als
> Karten nur, wo der Vertrag schweigt (Duplikatregel, Archiv- oder Überschreibregel beim
> Export). BEWEIS: docs/beweise/NAK-313.md mit Matrix, identischen Positiv-/
> Negativvektoren durch echte C++- und Rust-Reader (A5, A8, A11, A12, B2, B3c, B10, B13,
> A4), vollem Kanon GRÜN und Laufzeit-Arm."

Registerzeile NAK-313 (`docs/offene-punkte.md:317`), der hier einschlägige Teil: „T3-03-01,
T3-03-02, T3-03-04 bis T3-03-09 (W30: JSON- und FlatBuffers-Vertrag an jedem
Produkteingang; B12, B13) … Schemas sind Verträge: versionieren, Save und Load gemeinsam,
Fixtures bytegleich, Cross-Language-Gleichheit. Produktentscheide als Karten nur, wo der
Vertrag schweigt (Duplikatregel W10, Archiv- oder Überschreibregel W33)."

Registerzeile NAK-29 (`docs/offene-punkte.md:498`), die zwei Schließungen: „**GESCHLOSSEN
23.08. mit `SONDE-009`** … `nak29Verstoss()` … setzt alle vier genannten Faelle durch" und
„**NACHTRAG 02.09.2026 — GESCHLOSSEN** durch `SONDE-011` Phase A (`01f6194`): die bedingten
Transportpflichten sind als strikte diskriminierte Union im Schema selbst formuliert".
Geschlossen ist damit die bedingte Feldpflicht im Erzeuger und im Schema; T3-03-01 betrifft
die JSON-Abbildung dazwischen.

**Prüfform:** Quellenlektüre am HEAD. Kein Bau, kein Testlauf, kein Kanon, kein FL Studio,
kein Codex, keine Fehlerinjektion; Git nur lesend (`rev-parse`, `status`, `log`, `diff`,
`show`, `check-attr`, `ls-files`). Offen benannt: einmal lief ein lesender
Python-Einzeiler, der nur die obersten Schlüssel von
`eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json` ausgab; er hat nichts geschrieben und nichts
geprüft, alle Aussagen unten stützen sich auf Lektüre. Die zwei externen Normaussagen
(JSON Schema 2020-12, RFC 8259) sind an der Primärquelle nachgelesen (json-schema.org,
rfc-editor.org). Fremdcode ist lokal gelesen: `serde_json` 1.0.151, `serde`,
`serde_core` und `serde_derive` 1.0.229, `flatbuffers` 25.12.19 aus dem Cargo-Register
(Versionen laut `broker/Cargo.lock:104-105`, `:281-282`, `:291-292`, `:300-301`,
`:311-312`), JUCE und die FlatBuffers-C++-Header
aus dem Bauartefakt `eq-copilot/build/_deps/`, `jsonschema` 4.25.1 unter `C:\Python314`.
Jede Zeilennummer unten ist am HEAD `43549a0e` selbst nachgelesen, keine aus dem Audit
übernommen. Sicherheitsbefunde (T3-12-\*) und die Authentisierung am Tor sind nicht
Gegenstand (W30, `BEFUNDE.md:833`: „keine Sicherheitsneubewertung"); Legacy-Bezeichner
sind kein Befund; der Designteil ist geparkt, unten steht keine Oberfläche.

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn* (19:11): die fremden Ordner
`briefing-hub/` und `nimbalyst-local/` sowie die drei Auftragsdateien
`docs/beweise/roh/NAK-313-quellvalidierung-teil-{A,B,C}-auftrag.txt`. *Unmittelbar vor dem
Schreiben* (19:48): dasselbe plus `docs/beweise/roh/NAK-313-quellvalidierung-teil-A.md` und
`…-teil-C.md` der parallelen Validierer; beide habe ich weder gelesen noch angefasst. HEAD
beide Male `43549a0e`; er ist nicht gewandert, ein Zeilenabgleich war nicht nötig. Diese
Datei ist die einzige Schreibaktion dieses Agenten.

**Stand des Audits gegen den HEAD.** Phase 03 maß an `8f7c6f5f`, Phase 10 an `2c3a4b0a`,
Phase 16 an `aff2d818`. Zwei `git diff --stat 8f7c6f5f HEAD` über die unten zitierten Pfade
nennen zusammen 20 geänderte Dateien. Für die acht IDs:

- **Unverändert seit `8f7c6f5f`:** der Broker bis auf `broker/src/briefkasten.rs` (NAK-309
  Etappe 4, `3b2d182c`, Diagnosebriefkasten ohne Bezug zu den IDs) — also
  `broker/src/transport/`, `coordinator/*`, `telemetrie.rs`, `vertrag.rs`, `protokoll.rs`,
  `dto.rs`, `generiert/`; dazu `NakamaEvidenz.cpp`, `NakamaVertrag.cpp`,
  `NakamaTelemetrie.cpp`, `state/NakamaKanon.*`, `core/analysis/*`, der generierte
  C++-Header, `core/ipc/*` samt `controlclient/*`, der FlatBuffers-Korpus samt Erzeuger,
  `TEXTRIEGEL-FAELLE.json`, `evidenz-zahlen-wire-v1.json`, `pruefe_v3_vertrag.py`,
  `pruefe_v2_schemas.py`. Die Broker-Zeilen des Audits (Anhang C in
  `roh/phase-03-vertrag.md:1180` ff.) treffen deshalb zeilengleich; ich habe die tragenden
  selbst nachgelesen.
- **Geänderte Testdateien, am HEAD gelesen:** `PipeClientLifecycleTestMain.cpp` und
  `tests/V3TestServer.h` (NAK-309 Etappe 3 und 4), `Sonde012SourcesModelTest.cpp`,
  `StateMigrationTestMain.cpp`, `erzeuge_state_fixtures.py` (NAK-312), `tools/beweise.ps1`
  (Behauptungszeilen am HEAD zitiert).
- **Verschoben, nicht geändert:** `SourcesModel.cpp` (NAK-312 Etappe 6b/7b; Parseeinstieg
  heute `:728`/`:734` statt `:640`/`:646`, Rücknahme `:1478`/`:1484` statt `:1387`; der Text
  ist bytegleich, verglichen mit `git show 8f7c6f5f:…`), `SondeProcessor.cpp` (NAK-311/312;
  der Diff enthält keine Zeile aus `evidenzSnapshotSenden`, `evidenceSnapshotAlsJson`,
  `sendeP1` oder `ereignisseEntnommen`), `IpcTestMain.cpp` (NAK-309 Etappe 4, +128 Zeilen:
  Escape-Test heute `:5780` statt `:5652`), `SchemaTestMain.cpp` (NAK-311: 1.0-Golden heute
  `:1127-1128` statt `:1114-1115`), `PipeClient.cpp` (NAK-309: nur die Pipenamenwahl
  `:218-244`; Zahlen- und JSON-Weg unberührt).
- **Inhaltlich berührt, ohne Bezug zu den IDs:** `eq-ipc-v3.schema.json`, `MANIFEST.json`
  und `erzeuge_v3_fixtures.py` (NAK-311: nur `$comment` und `warum` von `auto_gain_db`).
- **NAK-310** hat seit dem Audit keinen Produktcode geändert (seine Codeetappen haben nicht
  begonnen, das Ticket ruht, `docs/offene-punkte.md:315`).

Kein Befund ist durch NAK-310, NAK-311 oder NAK-312 erledigt oder verändert.

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorie | Schwere (Audit) | Rotbeweis fällt an | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-03-01 | BESTÄTIGT, präzisiert: der Verlust ist still (kein Zähler am Brokerausgang), die Ereignisse des Fensters sind danach auch beim Sender entnommen; erreichbar bei jedem Evidenzframe ohne laufenden Transport | DEFEKT | high (high) | B16 `Sonde013EventWireTest.cpp` Abschnitt A mit Speiser ohne Projektzeit; A4 über eine Byteinstanz; Mutation `NakamaEvidenz.cpp:207-208` | nein |
| T3-03-02 | BESTÄTIGT, präzisiert: vier verschiedene Wirkungen je Eingang (Ablehnung, stiller Ersatzwert, Teilmutation ohne ACK, stilles Verwerfen); kein eigener Writer schreibt heute `1.0` | DEFEKT | medium (high) | A4 `befehl.rs:759`, `evidenz.rs:265-268`, `bootstrap.rs:242`; B13 `SourcesModel.cpp:101-118`; B10 `Intern.h:81-91`, `Vertrag.cpp:210` | nein |
| T3-03-04 | BESTÄTIGT, präzisiert: auch das C++-Vertragsbein B3c liest den Korpus mit JUCE; ein strenger C++-Leser liegt schon im Plugin (`nakama::kanon::lies`) | DEFEKT | medium (high) | B13 neben `Sonde012SourcesModelTest.cpp:1434`; Mutation am Parseeinstieg `SourcesModel.cpp:734` und `:1484` | nein |
| T3-03-05 | BESTÄTIGT, präzisiert: einzige gefundene Folgewirkung ist die Samplerate im Nachlauf-Countdown (`intervention.rs:258-265`, `:376-377`); die Untergrenze 1e-308 steht in Code und Fixtures, nicht im README-Wortlaut | DEFEKT | medium (high) | A4 `bootstrap.rs` neben `:492`; Mutation: der neue Textriegel- beziehungsweise Schemaaufruf vor `bootstrap.rs:254` entfällt | nein |
| T3-03-06 | BESTÄTIGT, präzisiert: kein eigener Rust-Writer erzeugt heute solche Texte; der v2-Client desselben Plugins zählt schon Codepunkte, und das gemeinsame Falltabellen-Positiv (`\"`, `\\`) widerspricht dem flachen Leser | DEFEKT | low (medium) | B10 G10 `IpcTestMain.cpp:4927` ff. mit `V3TestServer.h:416-417`; Mutation `Vertrag.cpp:213`, `:228`, `IpcVerbindung.cpp:450` | nein |
| T3-03-07 | BESTÄTIGT, präzisiert: der Broker reicht die Originalbytes an Main weiter (`senke.rs:212`, `:254`); der zusätzliche Slot passiert beide Leser hintereinander | DEFEKT | medium (high) | B3c und A4 über ein neues FB-Fixture aus `erzeuge_fb_fixtures.py` (A10); Mutation am neuen Slotriegel in `NakamaTelemetrie.cpp:600-684` und `telemetrie.rs:247-324` | nein |
| T3-03-08 | BESTÄTIGT, präzisiert: zweiter Überlaufpfad über den Exponenten; nach dem im README gemessenen Umklappverhalten liest der Client `18446744073709551618` als Protokollversion 2 und nimmt das `welcome` an | DEFEKT | medium (high) | A4b `PipeClientLifecycleTestMain.cpp` neben `:397`; neuer Zahlriegel vor `PipeClient.cpp:504`/`:600` | nein |
| T3-03-09 | **PRÄZISIERT**: typisierte Rust-Structs lehnen nur ab, wenn sie aus dem Rohtext lesen; aus einem schon geparsten `Value` (`evidenz.rs:129`, `befehl.rs:674`) sehen sie nur den letzten Wert; die DTO-Kante hat die Ablehnungsregel bereits in drei Sprachen | LÜCKE (Technikregel) | medium (medium) | A4 `schema.rs:422-426`, B13, A5; NAK-310 M-77 als Wache | nein — Technik; die Karte des Registers gehört W10 |

---

## Beine, die diese Wege berühren (`tools/beweise.ps1`)

| Bein | Zeile | Fährt | Behauptung (wörtlich, einschlägiger Teil) |
|---|---|---|---|
| A4 | `:511-519` | `cargo test --manifest-path broker/Cargo.toml` (alle Broker-Unit- und Integrationstests, darunter `vertrag.rs`, `bootstrap.rs`, `schema.rs`, `broker/tests/contract_cross_language.rs`) | „Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, …" |
| A4b | `:529-536` | `EqCopPipeClientTest` = `tests/PipeClientLifecycleTestMain.cpp` + `src/PipeClient.cpp` (`eq-copilot/plugin/CMakeLists.txt:622-626`) | „C++-PipeClient: … Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. …" |
| A5 | `:544` | `pruefe_v3_vertrag.py --abdeckung` | „Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. …" |
| A8 | `:547` | `erzeuge_v3_fixtures.py --pruefen` | „Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei." |
| A9 | `:554` | `pruefe_flatc_drift.py` | „Codegen-Drift ist 0: … jedes Tabellenfeld traegt eine explizite Feld-ID, keine ist verbrannt oder wiederverwendet, und jedes Offsetfeld steht im Rust-Strukturriegel." |
| A10 | `:555` | `erzeuge_fb_fixtures.py --pruefen` | „Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 … sind in beiden Lesern gedeckt." |
| A11 | `:562` | `pruefe_v2_schemas.py` | „Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren." |
| B3c | `:702` | `EqCopSchemaTest` = `tests/SchemaTestMain.cpp` (`CMakeLists.txt:1263-1270`) | „v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; …" |
| B10 | `:738` | `EqCopIpcTest` = `tests/IpcTestMain.cpp` (`CMakeLists.txt:977-978`) | „C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei …" |
| B13 | `:750` | `EqCopSonde012SourcesModelTest` = `tests/Sonde012SourcesModelTest.cpp` + `src/SourcesModel.cpp` (`CMakeLists.txt:511-515`) | „Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; …" |
| B16 | `:767` | `EqCopSonde013EventWireTest` = `tests/Sonde013EventWireTest.cpp` | „Evidenzpfad (§33.2): der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert Textriegel und dieselbe Vertragsengine wie B3c; …" |

Wichtig für alle acht IDs: A5, A8, A10, B3c und der Korpusteil von A4
(`contract_cross_language.rs:44-143`, `:176` ff.) messen **Vertragsleser**, keine
Produktleser. Einen Produktaufrufer der C++-Schemaengine gibt es nicht (Suche nach
`vertrag::Schema`, `Schema::laden`, `.gueltig (` außerhalb `NakamaVertrag.*` und der Tests:
kein Treffer). Die Synthese sagt das selbst: „SchemaTest allein prüft nicht die daneben
laufenden Produktparser" (`roh/phase-11-synthese.md:165`).

---

## 1. T3-03-01 — der lokale Evidenzwriter schreibt `project_sample_start: null`

*Befundzeile `BEFUNDE.md:378` · Skeptiker Phase 10 `:512` · B12 `:82` · W30 `:833` ·
Rohbeleg `roh/phase-03-vertrag.md:20`, `:84`, `:160`*

### a) Behauptung des Audits

Der C++-Evidenzwriter macht aus einem gültigen lokalen Transportstempel einen
schemawidrigen Snapshot, weil er `project_sample_start` ausdrücklich als `null` schreibt;
die P1-Übergabe gelingt, der Broker verwirft die Evidenz (`BEFUNDE.md:378`, bestätigt
`:512`).

### b) Quellkette am HEAD

**Station 1 — der Erzeuger kennt „kein Projektstart".**
`eq-copilot/plugin/core/analysis/featureengine/Frame.h:261-272`:

```
    if (gesamt.projektGueltig)
    {
        t.zeitbasis = Zeitbasis::project_samples;
        t.project_sample_start_gesetzt = true;
        t.project_sample_start = gesamt.projektStart;
        t.gueltigkeit |= kGProjectTime;
    }
    else
    {
        t.zeitbasis = Zeitbasis::local_monotonic;
        // KEIN `project_time`-Bit und KEIN `project_sample_start`.
    }
```

Wann das eintritt: `featureengine/Zeit.h:524-530` verlangt für die Projektspanne eines
Blocks `kFlagZeitGueltig | kFlagSpieltGueltig | kFlagSpielt` („if ((b.flags & muss) != muss)
return false;"); `Spektrum.h:334-342` legt dieses Bit je Block im Ring ab, und die
Frame-Projektzeit entsteht aus diesen Stützen (`Frame.h:226-232`, vereinigt über
`Zeit.h:77-92`). Ohne laufenden Transport gibt es nach dieser Kette keine Projektzeit
(gelesen, nicht gelaufen). Ob ein Evidenzframe
entsteht, hängt dagegen nur an der Samplemenge (`Frame.h:45-46`: „f.evidenzFrisch = evidenzS
>= evidenzIntervallS;"), und der Versand filtert nicht nach Zeitbasis
(`sonde/SondeProcessor.cpp:916-917`: „if (frame.evidenzFrisch) evidenzSnapshotSenden
(frame);"). Jeder Evidenzframe bei angehaltenem FL-Transport ist damit `local_monotonic`.

**Station 2 — der NAK-29-Riegel lässt ihn zu.** `featureengine/Vertrag.h:199-201`:

```
    if (t.zeitbasis == Zeitbasis::local_monotonic
        && (zeitBitAn || t.project_sample_start_gesetzt))
        return 2;
```

Ohne Startwert ist das Ergebnis 0; `vertrag/NakamaEvidenz.cpp:446` („if
(nakama::analyse::nak29Verstoss (frame.transport) != 0) return false;") passiert.

**Station 3 — die Abbildung schreibt `null`.** `vertrag/NakamaEvidenz.cpp:204-208`:

```
    aus += "\",\"project_sample_start\":";
    // `null` heisst hier nicht "fehlt", sondern "diese Zeitbasis kennt keinen
    // Projektstart" — der Zweig `local_monotonic` verlangt genau das.
    aus += t.project_sample_start_gesetzt
         ? std::to_string (t.project_sample_start) : std::string ("null");
```

Derselbe Writer lässt ein fehlendes optionales Feld sonst weg (`:215-219`, „if
(t.continuous_time_samples_gesetzt) { … }").

**Station 4 — die lokale Übergabe gelingt.** `core/ipc/controlclient/Nachrichten.cpp:402-421`
(`sendeP1`) prüft nur die Größe (`:405`) und reiht ein (`:413`). Danach
`SondeProcessor.cpp:1085` „const auto ergebnis = controlV3.sendeP1 ({}, json);", `:1089`
„uebergeben = nakama::ipc::p1Uebergeben (ergebnis);" und `:1127-1131`:

```
    if (uebergeben)
    {
        merkmale.ereignisseEntnommen();
        letzteEreignisverluste = verworfen;
    }
```

**Station 5 — das Schema lässt nur Abwesenheit zu.** `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:306-307`
im lokalen Zweig und `:360` als gemeinsame Eigenschaft:

```
            "time_basis": { "const": "local_monotonic" },
            "project_sample_start": { "type": "null" },
```
```
        "project_sample_start": { "type": "integer" },
```

`eq-copilot/schemas/v3/README.md:254-258`: „Der gewaehlte Zweig ersetzt nicht seine
Geschwisterregeln. Nach der Zweigpruefung werden `required`, `properties`, Grenzen und
weitere diskriminierte Unionen desselben Knotens ebenfalls ausgewertet." Der Zweig weist
jede Zahl ab, die gemeinsame Eigenschaft weist `null` ab; wirksam ist nur „fehlt". Das
Positivfixture `gueltig/evidence-ohne-projektzeit.json` (Transportblock Zeilen 11-30) lässt
das Feld weg; das Negativfixture `ungueltig/transport-local-monotonic-mit-project-sample-start.json`
trägt eine Zahl (`MANIFEST.json:3845-3855`, Verletzung
`#/$defs/transportstempel/oneOf/1/properties/project_sample_start/type`). Ein Fixture mit
`null` gibt es im v3-Korpus nicht.

**Station 6 — der Broker verwirft still.** `broker/src/coordinator/senke.rs:106-108`:

```
            Some("evidence_snapshot") => {
                let _ = self.evidence_snapshot_json_mit_minor(link_id, payload, schema_minor);
            }
```

`broker/src/coordinator/evidenz.rs:125-128`:

```
        let Some(wert) = v3_nachricht_lesen_mit_minor(payload, "evidence_snapshot", schema_minor)
        else {
            return false;
        };
```

mit `coordinator/schema.rs:422-426` (Textriegel, `serde_json::from_slice`,
`v3_schema(schema_minor)?.gueltig(&wert)`). Die Zähler des Evidenzpfads
(`coordinator/zustand.rs:312-313`, `:326`: `evidence_angenommen`, `evidence_gesperrt`,
`evidence_beeinflusst`) berührt dieser Ausgang nicht; einen Zähler für einen am Schema
gescheiterten Snapshot habe ich nicht gefunden.

**Beide Seiten für den Vektor `"time_basis":"local_monotonic","project_sample_start":null`:**
der C++-Writer erzeugt ihn; die C++-Schemaengine lehnt ihn wegen der Geschwisterregel ab
(hergeleitet, kein Bein fährt es, siehe d); die Rust-Schemaengine lehnt ab, der Snapshot ist
verloren; das Python-Referenzbein (jsonschema wertet `properties` und `oneOf` unabhängig) lehnt
ab. Die drei Vertragsleser sind sich einig, nur der Writer weicht ab. Erreichbarkeit
**Stufe 1**.

### c) Zusage wörtlich

- `CLAUDE.md:178-181`: „**Schemas sind Verträge.** Neue persistente Felder zuerst
  versionieren; Altstände laden; unbekannte Felder dürfen alte Consumer nicht zerstören;
  Save und Load gemeinsam testen."
- Schema `:360` (gemeinsam `integer`) mit dem Zweig `:307`; dieselbe Aussage binär in
  `eq-copilot/schemas/v3/flatbuffers/README.md:158`: „`zeitbasis=local_monotonic` verbietet
  Project-Time-Bit und `project_sample_start`."
- Registerzeile NAK-29 (`docs/offene-punkte.md:498`): die bedingten Transportpflichten sind
  „als strikte diskriminierte Union im Schema selbst formuliert". Der Writer bricht genau
  diese Union.
- Der Absichtssatz des Senders, `SondeProcessor.cpp:1120-1124`: „Ein abgewiesener Snapshot
  (P1-Puffer voll, zu gross, oder `evidenceSnapshotAlsJson` fail-closed) loeschte damit die
  Ereignisse, die er gerade NICHT transportiert hat — ein stiller Verlust ohne Zaehler." Der
  Kommentar beschreibt diese Verlustart als behoben; er deckt nur die lokalen Ausgänge.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| B16 `Sonde013EventWireTest.cpp:462-497` (Abschnitt A) | echter FeatureFrame → `evidenceSnapshotAlsJson` → Textriegel → Schemaengine | Der Speiser setzt immer Projektzeit (`:108-109`: „b.flags = rt::kFlagKontextAnwesend \| rt::kFlagSpieltGueltig \| rt::kFlagSampleRateGueltig \| rt::kFlagSpielt \| rt::kFlagZeitGueltig;"). `local_monotonic` läuft nie durch Writer und Schema. |
| B3c `SchemaTestMain.cpp:314-323`, B10 `IpcTestMain.cpp:4275-4285` | NAK-29-Fälle am Binärencoder und am Sender | prüfen `nak29Verstoss`, nicht die JSON-Abbildung |
| B3c/A4/A5 Korpus | `evidence-ohne-projektzeit.json` gültig | prüft den Vertrag, nicht den Writer |
| A4 Evidenzempfänger | Coordinator mit handgebauten Snapshots | kein vom C++-Writer erzeugter lokaler Snapshot |

Die B16-Behauptung („der aus einem echten FeatureFrame erzeugte evidence_snapshot passiert
Textriegel und dieselbe Vertragsengine wie B3c", `tools/beweise.ps1:767`) ist damit weiter,
als das Bein misst: sie gilt nur für Frames mit Projektzeit.

### e) Urteil: BESTÄTIGT

Die Kette schließt am HEAD ohne Lücke. Präzisierungen gegenüber der Befundzeile:

1. Der Verlust ist **still**: am Brokerausgang `evidenz.rs:125-128` steigt kein Zähler.
2. Die Ereignisse des Fensters sind beim Sender **schon entnommen** (`SondeProcessor.cpp:1127-1131`),
   weil die lokale Übergabe gelang.
3. Erreichbar ist der Fall bei **jedem** Evidenzframe ohne laufenden Transport
   (`Zeit.h:526-530`), nicht nur in einem Randfall.
4. Der Kommentar `NakamaEvidenz.cpp:205-206` ist **irreführend**: der lokale Zweig
   „verlangt" kein `null`, die gemeinsame Eigenschaft verbietet es.

NAK-310, NAK-311 und NAK-312 haben an dieser Kette nichts geändert.

### f) Kategorie und Schwere

**DEFEKT, high.** Ein Stufe-1-Weg im Normalbetrieb (FL angehalten, Audio läuft) erzeugt zu
100 % schemawidrige Snapshots, die ohne Zähler auf einer der beiden Seiten verloren gehen,
samt ihrer Ereignisse.

### g) Kleinster Fix

`NakamaEvidenz.cpp:204-208` in `transportJson` schreibt `project_sample_start` nur bei
`project_sample_start_gesetzt`, nach dem Muster von `:215-219`; der Kommentar `:205-206` wird
berichtigt. Schema, FlatBuffers-README und Positivfixture sagen übereinstimmend „fehlt"; ein
Schemawechsel ist nicht nötig. `erzeuge_v3_fixtures.py` bekommt ein Negativfixture mit
`null` im lokalen Zweig (erwartete Verletzung
`#/$defs/transportstempel/properties/project_sample_start/type`), A8 hält es bytegleich,
`.gitattributes:36` (`eq-copilot/fixtures/v3/** -text`) deckt es. Mitzuprüfende Beziehung:
**schreiben↔lesen** — der echte C++-Writer muss durch den echten Rust-Empfänger, nicht nur
durch die C++-Engine; **Positiv↔Negativ** — fehlt (gültig), `null` (ungültig), Zahl
(ungültig). Außerhalb dieses Fixes, nur benannt: der Kommentar `senke.rs:103-105` („ein
abgelehnter P1 schliesst die Verbindung nicht, er wird gezaehlt") trifft für den
Schemaausgang des Evidenzpfads nicht zu.

### h) Ort des Rotbeweises

- **C++, B16** (`Sonde013EventWireTest.cpp`, neben Abschnitt A `:462-497`): ein zweiter
  Speiser ohne `kFlagZeitGueltig`/`kFlagSpielt` liefert einen `local_monotonic`-Frame; der
  Snapshot muss Textriegel und Schema passieren und das Feld darf nicht vorkommen. Heute rot
  mit der Verletzung an `/transport/project_sample_start`. Mutation, die die Zusage bricht:
  in `NakamaEvidenz.cpp:207-208` wieder `null` schreiben.
- **Rust, A4:** eine von Hand geschriebene Byteinstanz des lokalen Snapshots nach dem Muster
  `eq-copilot/fixtures/v3/heartbeat-wire-v1.json` (dort `_kommentar`: „Stimmt der
  C++-Serialisierer mit ihr ueberein und der Rust-Leser nimmt sie an, stimmen beide
  transitiv miteinander ueberein"). B16 vergleicht den Transportblock des Writers mit ihr,
  A4 (`broker/tests/sonde013_verdrahtung.rs`, Nachbar der Evidenzempfänger-Tests) fährt sie
  durch `evidence_snapshot_json_mit_minor` und verlangt Annahme und
  `evidence_angenommen + 1`.
- **Python, A5:** das neue `null`-Fixture wird als ungültig klassifiziert.

---

## 2. T3-03-02 — schema-gültige Integer in Dezimal- oder Exponentialform

*Befundzeile `BEFUNDE.md:379` · Skeptiker `:513` · B12 `:82` · Rohbeleg
`roh/phase-03-vertrag.md:21`, `:85-110`, `:162`, Anhang C `:1180` ff.*

### a) Behauptung des Audits

Integer wie `1.0` und `1e0` sind nach dem Schema gültig, scheitern aber in beiden
Produktlesern; Rust ersetzt einzelne Evidenzwerte über `unwrap_or` durch 0 oder „fehlt"; das
Golden „integer akzeptiert 1.0" ist der Gegenbeleg (`BEFUNDE.md:379`, `:513`).

### b) Quellkette am HEAD

**Die drei Vertragsleser nehmen `1.0` an.**

- C++, `vertrag/NakamaVertrag.cpp:240-248`:
  ```
      if (name == "integer")
      {
          // draft 2020-12: jede Zahl OHNE Nachkommateil ist ein integer, also
          // auch 1.0. Nicht "ist ein int-var" - das waere enger als der Standard.
          if (! istZahl (w))
              return false;
          const auto d = static_cast<double> (w);
          return std::isfinite (d) && std::trunc (d) == d;
      }
  ```
- Rust, `broker/src/vertrag.rs:803-808`:
  ```
          // draft 2020-12: jede Zahl OHNE Nachkommateil ist ein integer,
          // also auch 1.0. Nicht `is_i64()` — das waere enger als der Standard.
          "integer" => wert
              .as_f64()
              .map(|f| f.is_finite() && f.fract() == 0.0)
              .unwrap_or(false),
  ```
- Python: `jsonschema/_types.py:195-201` (lokal 4.25.1) definiert ab Draft 6 „integer" als
  `is_integer(checker, instance) or (isinstance(instance, float) and instance.is_integer())`;
  2020-12 erbt es (`:202-204`).
- Der Textriegel lässt beide Formen durch (Regel 2, README `:62-66`; `NakamaVertrag.cpp:371-374`:
  „JSON Schema meint mit `integer` den mathematischen Wert, nicht nur ein Literal ohne Punkt:
  auch 5.0 und 5e0 sind ganze Zahlen.").

**Die Produktleser in C++ verlangen die Darstellung.**

- `src/SourcesModel.cpp:101-118`:
  ```
  bool nichtnegativeGanzzahl (const juce::var& v, std::uint64_t& aus)
  {
      if (v.isInt())
      { … }
      if (v.isInt64())
      { … return aus <= 9007199254740991ULL; }
      return false;
  }
  ```
  JUCE liest ein Literal mit `.` oder `e` als double
  (`eq-copilot/build/_deps/juce-src/modules/juce_core/json/juce_JSON.cpp:267-271`, „if (c ==
  'e' || c == 'E' || c == '.') … readDoubleValue"). Aufrufer: `:172` (`letzter_kontakt_ms`),
  `:312` (`host_mixer_index`), `:331` (`zaehler`), `:1029-1030` (Band), `:1201-1202`
  (`band_von`/`band_bis`), `:1225` (`intent_revision`), `:1563-1564`
  (`sample_start`/`sample_end`). Wirkung am Beispiel `letzter_kontakt_ms`: `frische`
  (`:166-175`) liefert `false`, und `:1294-1297` macht den ganzen Snapshot `ungueltig`
  („session member kind or freshness is invalid"); die übrigen Aufrufer habe ich nicht
  einzeln bis zum Ergebnis verfolgt.
- `core/ipc/controlclient/Intern.h:81-91` (`nichtnegativeJsonGanzzahl`: nur Ziffern, keine
  führende Null), benutzt für `command_ack.state_revision` in `:215-216` → `keinAck`, der
  Auftrag bleibt im In-Flight-Register.
- `core/ipc/controlclient/Vertrag.cpp:209-211`:
  ```
      // `protocol` ist im Schema eine ZAHL mit dem Wert 3, kein String "3".
      if (! jsonLiteral (felder, "protocol", protokoll) || protokoll != "3")
          return false;
  ```
  → `Verbindung.cpp:664-668` „unerwartete Antwort auf hello", keine Verbindung.

**Die Produktleser in Rust verlangen den Zahlentyp.** `serde_json` 1.0.151 `src/number.rs:151-156`:
`as_u64` liefert für `N::Float` `None`. Vier verschiedene Wirkungen:

1. *Ablehnung:* `transport/bootstrap.rs:242-250` (`protocol` über `as_u64` → „weder protocol
   noch protocol_version"); dazu die typisierten `u32`-Felder `protocol` (`:106`),
   `block_size`/`channels` (`:86-87`), `pid` (`:93`): `serde_core` 1.0.229
   `src/de/impls.rs:428-434` kennt für `u32` nur Ganzzahl-Besucher, eine Gleitkommazahl ist
   ein Typfehler. Ebenso `coordinator/intent.rs:333-335` („as_i64().unwrap_or(-1); if
   revision < 0 { return Err(IntentAbweisung::Vertrag); }"), `:388-392`,
   `coordinator/assistent.rs:102-105`.
2. *Stiller Ersatzwert:* `coordinator/evidenz.rs:265-268` (`sequence`), `:277-288`
   (`ereignisse_verloren`, `verteilung_fenster`, `samples_nicht_endlich`), `:316-323`
   (`transport_epoch`, `continuity_segment`), `:327-330` (`sample_count`) jeweils
   `.and_then(Value::as_u64).unwrap_or(0)`; `:324-326` `project_sample_start` wird `None`.
3. *Teilmutation ohne ACK:* `coordinator/befehl.rs:671` nimmt den Heartbeat am Schema an,
   `:692` ruft `ersten_heartbeat_markieren` (`coordinator/link.rs:384-393` setzt
   `link.erster_heartbeat_gesehen = true`), `:732-734` bestätigt bei `false` im ersten
   Heartbeat den Neuaufbau (`resync_bestaetigen`) — und erst `:759` „let sequence =
   wert.get("sequence")?.as_u64()?;" bricht ab: kein `heartbeat_ack`, kein
   `heartbeat_kontakt` (`:760`). Das Positivfixture `gueltig/heartbeat-integer-als-1punkt0.json`
   trägt genau diese Kombination (`"sequence": 91.0` in Zeile 10,
   `"intervention_state_unknown": false` in Zeile 29).
4. *Stilles Verwerfen:* `befehl.rs:795`, `:808`, `:813` (`event_sequence`, `tail_samples`
   der Interventionsgrenzen mit `?`) — die Intervention wird nicht registriert; `:167` und
   `coordinator/experiment_verdrahtung.rs:43`, `:219` (`base_revision`).

Die weiteren Stellen stehen zeilengleich in Anhang C des Audits (`roh/phase-03-vertrag.md:1180`
ff.); ich habe die oben genannten am HEAD nachgelesen und die Broker-Dateien sind seit
`8f7c6f5f` unverändert.

**Die eigenen Writer schreiben heute nur Zifferntext.** C++: `Vertrag.cpp:150-153`,
`:162-168`, `NakamaEvidenz.cpp:194-199`, `Intern.h:442-443` (`std::to_string`). Rust:
`befehl.rs:52` (`Value::from(state_revision)` aus `u64`), `:764` (`heartbeat_ack` per
`format!` aus `u64`), `coordinator/liveness.rs:796` (`Value::from(index)`). Der Befund ist
damit heute latent: er trifft jede andere vertragstreue Quelle, die `1.0` schreibt, nicht
die eigenen.

**Vektor `heartbeat.sequence = 91.0`:** C++-Vertragsleser gültig, Rust-Vertragsleser
gültig, Python gültig; Rust-Produktleser: Teilmutation, keine Antwort.
**Vektor `session_snapshot … letzter_kontakt_ms = 10.0`:** Vertrag gültig; C++-Produktleser
ungültig. **Vektor `command_ack.state_revision = 7.0`:** Vertrag gültig; C++-Produktleser
`keinAck`. **Vektor `hello.protocol = 3.0`:** Vertrag gültig; Rust-Bootstrap lehnt ab.

### c) Zusage wörtlich

- `eq-copilot/schemas/v3/README.md:208-210`: „`integer` folgt draft 2020-12: **jede Zahl
  ohne Nachkommateil** ist ein Integer, auch `1.0`. `number` schliesst Integer ein. `null`
  ist ein eigener Typ, nicht „fehlt"."
- README `:62-66` (Regel 2): „mathematische Ganzzahlen nur innerhalb ±(2^53−1),
  unabhaengig davon, ob sie als `5`, `5.0`, `5e0` oder mit verschobenen Endnullen geschrieben
  sind."
- `eq-copilot/fixtures/v3/MANIFEST.json:337-340`: `gueltig/heartbeat-integer-als-1punkt0.json`,
  „urteil": „gueltig", „warum": „draft 2020-12: jede Zahl ohne Nachkommateil ist ein
  integer".
- JSON Schema Validation 2020-12 §6.1.1: „'integer' which matches any number with a zero
  fractional part."
- README `:3-5`: „Drei Sprachen lesen dieselben Dateien; wo sie sich uneinig sind, ist eine
  von ihnen falsch — nie der Vertrag." Gate `plan.json:369`: „Cross-Language-Gleichheit
  C++/Rust; keine stillen Rundungen." W30 `BEFUNDE.md:833`: „ungültige Nachricht ohne
  Teilmutation".

### d) Heutige Abdeckung

- B3c `SchemaTestMain.cpp:1127-1128` („integer akzeptiert 1.0 (draft 2020-12)") und A4
  `vertrag.rs:1122-1125` (`integer_akzeptiert_1punkt0`) messen die Engines.
- Das Fixture mit `91.0` läuft nur durch Vertragsleser: B3c (`SchemaTestMain.cpp:512-579`),
  A4 (`contract_cross_language.rs:44-143`), A5. Kein Produktleser-Test mit einer `.0`- oder
  `e`-Form (Suche in `broker/` und `eq-copilot/plugin/tests/` nach dem Fixturenamen und nach
  `state_revision` mit Nachkomma: kein Treffer).
- B13 baut seine Snapshots über `juce::JSON::toString`; Ganzzahlen bleiben dort Ganzzahlen,
  eine `1.0`-Form entsteht nicht.
- Kein vorhandener Test schreibt die engere Produktregel fest; es fehlt schlicht der Weg.

### e) Urteil: BESTÄTIGT

Präzisierungen: (1) die Wirkung ist je Eingang verschieden — Ablehnung in allen C++-Lesern
und am Rust-Bootstrap, stiller Ersatzwert im Rust-Evidenzaufbau, Teilmutation ohne Antwort
am Rust-Heartbeat, stilles Verwerfen an den Interventionsgrenzen; (2) kein eigener Writer
erzeugt heute die Form; (3) der v2-Client desselben Plugins folgt bereits der Wertregel
(`src/PipeClient.cpp:54-67`, Kommentar `:56`: „JSON Schema bewertet auch 2.0 als
integer.") — zwei C++-Clients, zwei Zahlenregeln.

### f) Kategorie und Schwere

**DEFEKT, medium** (Audit: high). Die Zusage ist ausdrücklich (README, Manifest, Golden),
und der stille Ersatzwert ist die schlechteste Fehlerform; es fehlt heute ein
Produkterzeuger, der die Form schreibt.

### g) Kleinster Fix

- **C++, flacher Leser:** ein lexikalischer Ganzzahlleser in `Intern.h`, der die
  Normalisierung des Textriegels benutzt (`NakamaVertrag.cpp:371-417`: Wert = Ziffern ×
  10^Skala, ganzzahlig nur bei abgeschnittenen Nullen, Betrag ≤ 2^53−1), ersetzt
  `nichtnegativeJsonGanzzahl` in `:215-216` und den Textvergleich in `Vertrag.cpp:210`. Er
  liefert Wert oder „ungültig", nie 0 im Fehlerfall — dieselbe Funktion schließt damit den
  `stoull`-Weg `Intern.h:240-250` aus T3-03-03 (Teil A). Die Ganzzahlgrenze gehört als
  Parameter in die Funktion; T3-03-08 braucht sie mit `INT64_MAX`.
- **C++, SourcesModel:** `nichtnegativeGanzzahl` (`SourcesModel.cpp:101-118`) nimmt auch
  einen ganzzahligen Wert aus einer Dezimal- oder Exponentialform an. Den Wert nicht aus
  JUCEs `readDoubleValue` gewinnen („Ein Riegel darf nie die Bibliothek befragen, gegen deren
  Verhalten er schuetzt.", README `:113-114`), sondern aus dem strengen Leser, den T3-03-04
  ohnehin braucht (`nakama::kanon::lies`, Zahlen über `std::from_chars`,
  `state/NakamaKanon.h:79-80`).
- **Rust:** ein Wert-Helfer (`u64`/`i64` aus `PosInt`, `NegInt` oder `Float` mit `fract() ==
  0` im Bereich ±(2^53−1); der Textriegel in `schema.rs:423` läuft vorher, das f64 ist
  exakt) an allen Produktstellen aus b); am Bootstrap die Protokollwahl `:233`, `:242` und die
  typisierten `u32`-Felder über denselben Helfer. Im Heartbeat wandert die Feldauswertung
  (`befehl.rs:759`) **vor** die Seiteneffekte `:692` und `:732-734`.
- **Python:** keine Änderung. **Writer:** keine Änderung — Zifferntext bleibt die
  kanonische Teilmenge.
- **Fixtures:** Positivvektoren `1.0`, `1e0`, `91e0`, `9007199254740991e0` je
  Produkteingang, Negativ `1.5` und `9007199254740992.0`, über `erzeuge_v3_fixtures.py` (A8).
- **Beziehungen:** schreiben↔lesen (C++-Zifferntext ↔ Rust-Wertleser und umgekehrt),
  senden↔bestätigen (Heartbeat `91.0` → `heartbeat_ack` mit `91`), Positiv↔Negativ.

### h) Ort des Rotbeweises

- **Rust, A4:** Unit-Test in `befehl.rs` (oder `broker/tests/sonde013_verdrahtung.rs`):
  `p0_json` mit den Bytes von `heartbeat-integer-als-1punkt0.json` über einen Link mit
  derselben Adresse → `heartbeat_ack` mit `"sequence":91`; heute `None`. Mutation:
  `befehl.rs:759` zurück auf `as_u64()`. Zweiter Fall: `evidence_snapshot` mit
  `"sequence": 8241.0` → `Evidenzstand.sequence == 8241`; heute 0; Mutation
  `evidenz.rs:265-268`. Dritter Fall: `bootstrap_lesen` mit `"protocol": 3.0` →
  `V3Control`; Mutation `bootstrap.rs:242`.
- **C++, B13:** Snapshot mit `"letzter_kontakt_ms": 10.0` → `uebernommen`, Alter 10;
  Mutation: der neue Zweig in `SourcesModel.cpp:101-118` entfällt.
- **C++, B10:** `command_ack` mit `"state_revision": 7.0` → Art `angewandt`, Revision 7;
  `welcome` mit `"protocol": 3.0` über einen neuen Schalter in `tests/V3TestServer.h` →
  verbunden. Mutation `Intern.h:216`, `Vertrag.cpp:210`.
- **Python, A5:** klassifiziert alle Vektoren als vertragsgültig.

---

## 3. T3-03-04 — der C++-Produktleser übernimmt syntaktisch ungültiges JSON

*Befundzeile `BEFUNDE.md:381` · Skeptiker `:515` · B12 `:82` · Rohbeleg
`roh/phase-03-vertrag.md:23`, `:112`, `:166`*

### a) Behauptung des Audits

`SourcesModel` übernimmt nachgestellte Dokumente, Schlusskommas und unbekannte Escapes, die
Rust ablehnt; der Textriegel prüft diese Grammatik nicht (`BEFUNDE.md:381`, `:515`).

### b) Quellkette am HEAD

**Eingang.** `SourcesModel.cpp:727-738`:

```
    juce::String riegel;
    if (! nakama::vertrag::textriegelBytes (json.data(), json.size(), riegel))
    { … return SnapshotErgebnis::ungueltig; }
    juce::var root;
    const auto parse = juce::JSON::parse (juce::String::fromUTF8 (json.data(),
                                                                  static_cast<int> (json.size())), root);
    const auto* o = objekt (root);
    if (parse.failed() || o == nullptr)
        return SnapshotErgebnis::ignoriert;
```

Dasselbe für `evidence_invalidate` in `:1477-1488`.

**Der Textriegel ist ein Tokenprüfer, kein Parser.** In `NakamaVertrag.cpp:498-679` prüft
die Schleife Zeichenketten, Literale und Zahlen; jedes andere Zeichen, also `{ } [ ] , :`,
fällt auf `++i;` (`:678`). Ein Escape außer `\u` wird übersprungen (`:563-564`, „j += 2;
continue;"). Rust (`broker/src/vertrag.rs:432`) und Python
(`tools/eq-copilot/pruefe_v3_vertrag.py:312`) tun an derselben Stelle dasselbe. Die drei
Textriegel sind sich einig; unterschiedlich ist, was danach kommt.

**JUCE ist nachsichtig.** `juce_JSON.cpp`:
- `:82-91` `parseObjectOrArray` kehrt nach dem ersten Objekt zurück; `JSON::parse`
  (`:572-583`) prüft danach kein Dokumentende → ein angehängtes `[]` oder ein zweites Objekt
  fällt weg.
- `:327` „if (matchIf (',')) continue;" und `:303` „if (c == '}') break;" am Schleifenanfang
  → `{"a":1,}` wird angenommen.
- `:173-189` Escape-Schalter mit `default: break;` (`:189`) → `\q` wird `q`; dazu nimmt JUCE
  die Nicht-JSON-Escapes `\'` (`:176`) und `\a` (`:180`) an.

**Beide Seiten.** Rust: `schema.rs:424` („let wert: Value = serde_json::from_slice(payload).ok()?;")
lehnt Nachspann, Schlusskomma und unbekanntes Escape ab (`serde_json` 1.0.151 `src/de.rs:146-150`
`TrailingCharacters` am Dokumentende, `:2011` `TrailingComma` im Objekt, `src/read.rs:891`
`InvalidEscape`). Python: `json.loads` in
`pruefe_v3_vertrag.py:141` lehnt dieselben drei Formen ab. C++: übernimmt, sobald das erste
Objekt die eigenen Feldprüfungen besteht.

**Auch das C++-Vertragsbein liest mit JUCE.** `SchemaTestMain.cpp:413` („const auto
ergebnis = juce::JSON::parse (datei.loadFileAsString(), wert);") — B3c würde ein
Negativfixture dieser Klasse ebenfalls als wohlgeformt lesen und die Verletzungsmenge des
ersten Objekts melden.

**Einen strengen C++-Leser gibt es schon.** `state/NakamaKanon.h:72-86` beschreibt
`nakama::kanon::lies` als „den EIGENEN Leser des Kanons"; `state/NakamaKanon.cpp:414-424`
prüft das Dokumentende („fehler = "Text nach dem Dokument""), `:357` verlangt nach einem
Komma einen Schlüssel, `:285` meldet „unbekanntes Escape", `:364-365` „doppelter
Schluessel". Er bedient heute nur den DTO-/`state_hash`-Weg.

**Erreichbarkeit.** Der einzige Erzeuger von `session_snapshot` und `evidence_invalidate` ist
der Rust-Broker, der mit `serde_json` schreibt und seine Ausgabe vor dem Senden selbst prüft
(`coordinator/subscription.rs:201`, `:217`, `:352`: `v3_nachricht_lesen(…)`). Ein eigener
Writer erzeugt die Formen also nicht; der Befund greift bei einer nicht vertragstreuen
Gegenstelle. Deren Sicherheitsseite ist nicht Gegenstand (W30).

### c) Zusage wörtlich

- Entwurf `docs/FL-Nakama-Sonden-Design-Entwurf.md:2012` (§33.1): „kanonisches UTF-8-JSON
  für Handshake, Session, Zustand, Evidenz und Transaktionen".
- README `:3-5` (siehe oben) und `:35-38`: der Textriegel ist „eine Pruefung des
  **Rohtexts**, bevor ihn ein Parser sieht" — die Grammatik bleibt dem Parser überlassen.
  README `:296-307` duldet eine Parserabweichung ausdrücklich nur, wo „beide Wege … eine
  Ablehnung" sind.
- RFC 8259 §7 (nachgelesen): die erlaubten Zwei-Zeichen-Escapes sind `\" \\ \/ \b \f \n \r
  \t`; die Grammatik kennt weder Schlusskomma noch Text nach dem Wert.
- `CLAUDE.md:178-181`; Gate `plan.json:369` („Cross-Language-Gleichheit C++/Rust").

### d) Heutige Abdeckung

- B13 enthält keinen Syntaxfall (Suche nach Nachspann, Schlusskomma, `\q` in
  `Sonde012SourcesModelTest.cpp`: kein Treffer).
- B10 `IpcTestMain.cpp:5784-5785` („Text hinter dem Objekt wird abgelehnt") trifft nur den
  flachen Leser.
- `TEXTRIEGEL-FAELLE.json` (102 Fälle, `:6`) enthält keine Struktur- und keine
  Escape-Grammatikfälle außer gültigen Escapes (Fall 57 `:400-405`, Fall 70 `:491-496`, Fall
  71 `:498-503`).
- Der v3-Korpus (402 Einträge) hat kein Negativfixture für Nachspann, Schlusskomma oder
  unbekanntes Escape; seine Rohtextfälle (`rohtext_faelle`, `erzeuge_v3_fixtures.py:3291-3406`, etwa
  `zahl-fuehrende-null`, `zahl-nan-token`, `nul-escape-im-label`) fallen alle schon am
  Textriegel.

### e) Urteil: BESTÄTIGT

Präzisierungen: (1) Das C++-Vertragsbein B3c teilt den nachsichtigen Parser; ein
Grammatik-Negativfixture wäre dort heute rot. (2) Ein vorhandener strenger C++-Leser
(`kanon::lies`) macht eine neue Parserbibliothek überflüssig. (3) Heute erzeugt kein eigener
Writer diese Formen. Die Zeilen des Audits sind durch NAK-312 verschoben (`:640`/`:646` →
`:728`/`:734`, `:1387` → `:1478`/`:1484`), der Code ist bytegleich.

### f) Kategorie und Schwere

**DEFEKT, medium** (Audit: high). Die Zusage ist ausdrücklich, die Abweichung trifft zwei
Produkteingänge und das C++-Vertragsbein; eine eigene Quelle für die Formen gibt es nicht.

### g) Kleinster Fix

`SourcesModel.cpp:734` und `:1484`: vor der JUCE-Übernahme (oder statt ihrer) läuft
`nakama::kanon::lies` über denselben Text; scheitert er, ist das Ergebnis `ungueltig` mit dem
Grund des Lesers. Keine neue Bibliothek, kein Textriegelumbau — der Textriegel bleibt, was
README `:35-38` sagt. Derselbe strenge Einstieg gehört in `SchemaTestMain.cpp:413`, damit
das Vertragsbein wie Rust und Python urteilt. Fixtures: eine neue Klasse „der Parser lehnt
ab" im Erzeuger (Muster `textriegel_lehnt_ab` in `erzeuge_v3_fixtures.py:3492-3506` und
`:3527-3531`), ohne Verletzungsmenge, weil das Schema nie erreicht wird. Beziehung:
**Positiv↔Negativ** — dasselbe Dokument ohne Nachspann bleibt gültig.

### h) Ort des Rotbeweises

- **C++, B13** (`Sonde012SourcesModelTest.cpp`, neben dem N-28-Block `:1434-1461`): ein
  gültiger Snapshot mit angehängtem `[]`, derselbe mit Schlusskomma, ein Label mit `\q` →
  jeweils `ungueltig`, heute `uebernommen`. Mutation: den strengen Einstieg vor `:734`
  entfernen. Dasselbe für die Rücknahme (`:1484`).
- **Rust, A4:** dieselben Bytes durch `v3_nachricht_lesen` → `None` (hält heute, sichert die
  Gleichheit).
- **Python, A5:** dieselben Bytes als Parserablehnung klassifiziert.

---

## 4. T3-03-05 — der Rust-Bootstrap umgeht Text- und Schemakante

*Befundzeile `BEFUNDE.md:382` · Skeptiker `:516` · B12 `:82` · Rohbeleg
`roh/phase-03-vertrag.md:24`, `:113-117`, `:168`*

### a) Behauptung des Audits

Der Bootstrap nimmt `host:null`, `host.name:null` und `host.version:null` als Abwesenheit
an; `plugin_version` mit NUL-Escape und `samplerate` 1e-308 passieren (`BEFUNDE.md:382`,
`:516`).

### b) Quellkette am HEAD

**Eingang.** `broker/src/transport/server_v3/verbindung.rs:253-259`: nach
`client_sid_authentisieren` (`:253`) folgt direkt `bootstrap_lesen(&roh[..rahmen_laenge])`
(`:259`), ohne Textriegel und ohne Schema.

**Leser.** `transport/bootstrap.rs:219-221`:

```
    let roh = std::str::from_utf8(&daten[4..ende]).map_err(|_| BootstrapFehler::KeinUtf8)?;
    let wert: serde_json::Value = serde_json::from_str(roh)
        .map_err(|e| BootstrapFehler::KeinJson(e.to_string()))?;
```

danach die Protokollwahl (`:233`, `:242`) und `:254` „let h: HelloControl =
serde_json::from_str(roh)". Die Typen `:92-98` und `:110-111`:

```
pub struct HostAngabe {
    pub pid: u32,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub name: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub version: Option<String>,
}
```
```
    #[serde(default, skip_serializing_if = "Option::is_none")]
    pub host: Option<HostAngabe>,
```

`serde_json` 1.0.151 `src/de.rs:1673-1685` bildet ein JSON-`null` für ein `Option` auf
`visit_none()` ab — `null` und „fehlt" werden dasselbe.

**Prüfer danach.** `host_pruefen` (`:175-188`) beginnt mit „let Some(h) = h else { return
Ok(()) };"; `plugin_version_pruefen` (`:190-196`) zählt nur Zeichen (1..64), ein
dekodiertes NUL ist ein Zeichen; `:265-272` prüft die Samplerate auf „is_finite() && … > 0.0
&& … <= 768_000.0".

**Der Textriegel würde zwei der fünf Vektoren ablehnen.** NUL-Escape: `vertrag.rs:414-416`
(„NUL-Escape in Zeichenkette"), C++ `NakamaVertrag.cpp:538-541`. 1e-308:
`vertrag.rs:272-273` („if dez >= DEZ_GRENZE || dez <= -DEZ_GRENZE"), C++
`NakamaVertrag.cpp:364-369`, Python `pruefe_v3_vertrag.py:205-206`; für `1e-308` ist
`dez = -308`. Die Kommentare `pruefe_v3_vertrag.py:123` („|x| < 1e308") und README `:67`
nennen nur die Obergrenze.

**Folgewirkung im Broker.** `host.name`, `host.version` und `plugin_version` werden nach dem
Bootstrap nicht weiter benutzt (Suche in `coordinator/` und `server_v3/`: nur `host.pid` in
`coordinator/link.rs:87`). Die Samplerate schon: `link.rs:242` („abtastrate:
hello.audio.samplerate,"), dann `coordinator/intervention.rs:258-265` als Nachlaufrate der
Sitzung (Filter nur `is_finite() && *r > 0.0`) und `:376-377`:

```
            let sekunden = jetzt.saturating_sub(seit).as_secs_f64();
            let samples = (sekunden * taint.abtastrate).floor();
```

Bei 1e-308 ergibt das dauerhaft 0 Samples je Takt; der Nachlauf einer hörbaren Intervention
dieser Instanz läuft nie ab und die Evidenzsperre der Sitzung bleibt stehen (hergeleitet,
nicht gelaufen). Dieselbe Wirkung hätte allerdings auch eine vertragsgültige Winzrate wie
`1e-300` (Textriegel: `dez = -300`, Schema `audio_lage.samplerate` `:163`
„exclusiveMinimum": 0) — das ist eine eigene Lücke außerhalb dieser ID, hier nur benannt.

**Beide Seiten.** Der C++-Writer erzeugt keinen der Vektoren: `helloAlsJson`
(`controlclient/Vertrag.cpp:149-150`) schreibt `host` nur als `{"pid":N}` oder gar nicht,
`jsonString` (`Intern.h:299-311`) verwirft Steuerzeichen, `audioGueltig`
(`Vertrag.cpp:249-254`) verbindet bei einer vom Textriegel verweigerten Samplerate gar
nicht. Die C++- und Rust-Schemaengine und Python würden alle fünf ablehnen (Schema `:859`,
`:882`, `:887-888`; Textriegel Regel 5 und 3). Nur der Bootstrap nimmt an.

### c) Zusage wörtlich

- README `:209-210`: „`null` ist ein eigener Typ, nicht „fehlt"." Dieselbe Regel hat das
  Plugin für sich schon festgeschrieben: `Sonde012SourcesModelTest.cpp:1476-1488` („ein
  VORHANDENES `null` ist ungueltig — nicht „wie fehlend".", NAK-181 EP-01/NR-01).
- Schema `hello_control.host` `:859` (`$ref host_angabe`, dort `"type": "object"` `:882`),
  `host_angabe.name`/`version` `:887-888` (`"type": "string"`).
- README `:72-74` (Regel 5): „**Kein NUL-Escape in einer Zeichenkette**".
- Untergrenze der Zahlen: README Regel 3 (`:67`) sagt nur „Zahlen betragsmaessig unter
  1e308"; die Untergrenze tragen der Code (oben) und die gemeinsamen Tabellen:
  `TEXTRIEGEL-FAELLE.json:267-273` (Fall 38, `1e-400`, „wird_abgelehnt": true) und
  `evidenz-zahlen-wire-v1.json:18` („betrag": „dez > -308 und dez < 308 am GERUNDETEN
  Wiretext") mit `:130-133` (`eins_e_minus_308`, „genau an der unteren Kante",
  „angenommen": false).
- Entwurf `:4116-4118` (§53.9): „Die erste Nachricht jeder Verbindung ist ein höchstens 16 KiB
  großes, wie heute nur mit `u32` längenpräfigiertes JSON-Hello" — sie ist Teil des
  v3-Vertrags (Schema `hello`, `:838-845`).

### d) Heutige Abdeckung

- A4, `bootstrap.rs:492-565` (`host_haelt_die_laengen_des_vertrags`): Längen, fehlendes
  `host`, Umlaute zählen als Zeichen (`:542-543`: „Die Grenze zaehlt ZEICHEN, nicht Bytes —
  genau wie `maxLength` im Schema."). Kein `null`, kein NUL, keine Grenzrate.
- Kein Test fährt Hello-Fixtures des Korpus durch `bootstrap_lesen` (Suche in
  `broker/tests/` und `server_v3/tests_*.rs`: kein Treffer). Negativfixtures für die fünf
  Vektoren gibt es nicht (`ungueltig/hello-*` kennt Connection-Kind, Plugin-Kind und
  Telemetriefelder).

### e) Urteil: BESTÄTIGT

Alle fünf Vektoren passieren am HEAD. Präzisierungen: (1) die einzige gefundene
Folgewirkung ist die Nachlaufrate (`intervention.rs:258-265`, `:376-377`); `null` beim Host
wirkt wie ein fehlender Host, NUL in der Version bleibt folgenlos; (2) die Grenze 1e-308
steht in Code und Tabellen, nicht im README-Wortlaut (`:67`) — die Vertragsdoku ist dort
unvollständig; (3) der Kommentar `bootstrap.rs:166-169` („`pid` deckt `u32` bereits ab
(`minimum: 0`, und serde weist alles ab, was nicht in `u32` passt)") beschreibt eine engere
Regel als das Schema (`:886`, nur `minimum: 0`) — das ist die offene Karte PF11-05
(`roh/phase-11-synthese.md:229-234`), nicht Teil dieser ID.

### f) Kategorie und Schwere

**DEFEKT, medium** (Audit: high). Die Verbindungskante nimmt vertragswidrige Hellos an; eine
Folgewirkung ist belegt, ein eigener Writer löst sie nicht aus.

### g) Kleinster Fix

In `bootstrap_lesen` (`bootstrap.rs:204-289`) nach der Protokollwahl für v3: erst
`crate::vertrag::textriegel_bytes` über die Hello-Bytes, dann **ein** strenger Parselauf (der
Duplikate ablehnende Leser aus T3-03-09), dann die Hello-Prüfung des v3-Schemas
(`coordinator/schema.rs:377` ff. stellt die Engine bereit), erst danach die typisierte
Übernahme aus **demselben** Wert mit wertbasierten Ganzzahlen (T3-03-02). `null` fällt damit
am Schema, NUL und 1e-308 am Textriegel. v2-Hellos bleiben unangetastet (`:230-232`: „Ein
v2-Hello darf hier nie an v3-Regeln scheitern und umgekehrt."). README Regel 3 (`:67`) wird um
die Untergrenze ergänzt. Diese Reihenfolge ist zugleich die Voraussetzung für NAK-310 M-77
(„ein Parselauf je Hello", `docs/beweise/NAK-310.md:1189`). Beziehung: **schreiben↔lesen** —
die Samplerategrenze des C++-Writers (`audioGueltig`) und die des Bootstraps werden
dieselbe; **Positiv↔Negativ** — `host` fehlt (gültig) gegen `host: null` (ungültig),
`1e-307` (gültig, `evidenz-zahlen-wire-v1.json`, Klasse `eins_e_minus_307`) gegen `1e-308`.

### h) Ort des Rotbeweises

**Rust, A4**, `bootstrap.rs` neben `host_haelt_die_laengen_des_vertrags` (`:492`): `host:
null`, `host.name: null`, `host.version: null`, `plugin_version: "a\u0000b"`,
`samplerate: 1e-308` → jeweils `Err(KeinHello)`, heute `Ok`. Gegenfälle: ohne `host`
(vorhanden, `:495-501`), `samplerate: 1e-307`, 120 Umlaute (vorhanden, `:544-552`) bleiben
`Ok`. Mutation, die die Zusage bricht: den Textriegelaufruf entfernen (NUL und 1e-308 rot)
beziehungsweise die Schemaprüfung vor `:254` entfernen (die drei `null`-Fälle rot). C++ und
Python brauchen keinen neuen Fall; A5 klassifiziert die fünf Vektoren als ungültig, sobald
sie im Korpus stehen.

---

## 5. T3-03-06 — der C++-Handshake zählt Bytes und verbietet jedes Escape

*Befundzeile `BEFUNDE.md:383` · Skeptiker `:517` · B12 `:82` · Rohbeleg
`roh/phase-03-vertrag.md:25`, `:118-119`, `:170`*

### a) Behauptung des Audits

Der C++-Handshake begrenzt `broker_version` auf 64 und `reject.reason` auf 500 Bytes, der
Vertrag zählt Codepunkte; zusätzlich wird jedes Backslash-Escape pauschal verworfen
(`BEFUNDE.md:383`, `:517`).

### b) Quellkette am HEAD

**Eingang.** `controlclient/Verbindung.cpp:646-669` (Control) und
`core/ipc/TelemetryClient.cpp:668-690` (Telemetrie): Payload → `flachesJsonObjekt` →
`reject` oder `welcomeHaeltVertrag`. Eine Textriegel- oder UTF-8-Prüfung des Payloads gibt es
auf diesem Weg nicht (Suche in `WireEnvelope.*`: kein UTF-8-Bezug).

**Escape.** `core/ipc/IpcVerbindung.cpp:439-451`:

```
/// Liest einen JSON-String OHNE Escapes. Ein Backslash fuehrt zur Ablehnung —
/// nicht zur Interpretation.
bool leseString (const std::string& t, std::size_t& i, std::string& ziel)
{
    …
        if (c == '\\' || c < 0x20)
            return false;
```

Jeder String mit `\n`, `\\` oder `\"` macht das ganze Objekt unlesbar → `Verbindung.cpp:649-653`
„welcome: kein flaches JSON-Objekt".

**Längen.** `controlclient/Vertrag.cpp:212-214`:

```
    if (! jsonText (felder, "broker_version", brokerVersion)
        || brokerVersion.empty() || brokerVersion.size() > 64)
        return false;
```

und `:227-228`: „return jsonText (felder, "code", code) && ! code.empty() && jsonText
(felder, "reason", grund) && grund.size() <= 500;". `std::string::size` zählt Bytes: 33 × „é"
sind 33 Codepunkte und 66 Bytes, 251 × „é" sind 502 Bytes. Wirkung: ein solches `welcome`
führt zu „unerwartete Antwort auf hello" und keiner Verbindung; ein solcher `reject` zu
„reject haelt den Vertrag nicht" statt „Broker lehnt ab: <Grund>".

**Im selben Plugin gibt es die richtige Zählung schon.** `Intern.h:347-379`
(`utf8CodepointsBis`, benutzt für das Laufzeitlabel `:424`) und der v2-Client
`src/PipeClient.cpp:105-113` („return (darfLeerSein || aus.isNotEmpty()) && aus.length() <=
maxZeichen;" — `juce::String::length` zählt Codepunkte).

**Beide Seiten.** Der Rust-Writer kürzt Gründe nach Codepunkten
(`server_v3/verbindung.rs:350`: „"reason": grund.chars().take(500).collect::<String>()") und
escapt per `serde_json`. Die heute tatsächlich gesendeten Texte sind aber reines ASCII ohne
escapepflichtige Zeichen: `broker_version` ist `env!("CARGO_PKG_VERSION")`
(`broker/src/lebenslauf.rs:123`, `broker/Cargo.toml:3`: „0.1.0"); die v3-gerahmten
Coordinator-Gründe sind `"alias_quarantaene_deckel"` (`coordinator/zustand.rs:490`),
`"konfliktriegel_deckel"` (`:507`) und „Session-Globalcap erreicht; Neuzugang abgewiesen"
(`coordinator/liveness.rs:49`). Die Bootstrap-Ablehnung mit ihrem Debug-Text
(`verbindung.rs:265-270`, `format!("bootstrap: {e:?}")`) geht über
`bootstrap_reject_schreiben` (`:111-128`) nur `u32`-gerahmt hinaus; der C++-Client liest sie
als Envelopeverstoß (`Verbindung.cpp:631-636`) und erreicht den flachen Leser nie. Python
zählt `len(str)`, also Codepunkte (`jsonschema/_keywords.py:237-238`).

### c) Zusage wörtlich

- Schema `welcome.broker_version` `:900` (`"minLength": 1, "maxLength": 64`),
  `reject.reason` `:918` (`"maxLength": 500`).
- JSON Schema Validation 2020-12 §6.3.1 (nachgelesen): „The length of a string instance is
  defined as the number of its characters as defined by RFC 8259."
- Beide eigenen Engines zählen Codepunkte: `NakamaVertrag.cpp:1106-1108` („JUCEs
  String::length() zaehlt CODEPUNKTE, nicht Bytes - genau die JSON-Schema-Regel und dieselbe
  wie Rusts chars().count().") und `vertrag.rs:812-817` („Zeichenlaenge nach
  JSON-Schema-Regel: CODEPUNKTE, nicht Bytes und nicht UTF-16-Einheiten."); Tests
  `SchemaTestMain.cpp:1209-1211`, `vertrag.rs:1227-1231`.
- Escapes sind vertragsgültig: RFC 8259 §7; gemeinsame Falltabelle Fall 70 (`\"`,
  „wird_abgelehnt": false, `TEXTRIEGEL-FAELLE.json:491-496`) und Fall 71 (`\\`, `:498-503`).
- Der Dateikopf `Vertrag.cpp:27-28` verspricht selbst: „jedes Textfeld mit Schema-Laenge wird
  gegen genau diese Laenge geprueft, Zeichen gegen Bytes benannt".

### d) Heutige Abdeckung

- B10 G10 `IpcTestMain.cpp:4927-4961` prüft „`broker_version` ueber 64 Zeichen" mit 65 × `v`
  (`tests/V3TestServer.h:416-417`): ASCII, Bytes und Codepunkte fallen zusammen; der Test
  bliebe nach einer Codepunktkorrektur grün und unterscheidet die Regeln nicht.
- B10 `IpcTestMain.cpp:5780-5781` („ein Escape wird abgelehnt statt interpretiert")
  **schreibt die engere Implementierungsregel fest**, nicht den Vertrag.
- In B10 kein Fall mit mehrbyteigen Handshaketexten und kein `reject`-Fall (Suche nach
  `reject` in `tests/V3TestServer.h`, nach `reason` und nach
  `welcomeHaeltVertrag`/`rejectHaeltVertrag` in `IpcTestMain.cpp`: kein Treffer). Andere
  Beine mit echten Clients (A22, A24) habe ich darauf nicht durchsucht.

### e) Urteil: BESTÄTIGT

Präzisierungen: (1) heute erzeugt kein eigener Rust-Writer einen auslösenden Text; (2) der
v2-Client und `utf8CodepointsBis` zeigen, dass die richtige Regel im Plugin schon existiert;
(3) irreführend sind die Kommentare `Vertrag.cpp:220-221` („`reason` hoechstens 500
Zeichen", der Code zählt in `:228` Bytes) und `Vertrag.cpp:27-28` (Zeichen und Bytes werden
in `:213` und `:228` gerade nicht benannt); (4) der Handshake prüft kein UTF-8 — rohe
ungültige Bytes ≥ 0x20 in `broker_version` oder `reason` passieren `leseString`.

### f) Kategorie und Schwere

**DEFEKT, low** (Audit: medium). Die Abweichung ist enger als der Vertrag und fällt damit
geschlossen aus (keine Verbindung beziehungsweise kein angezeigter Grund); kein eigener
Writer löst sie heute aus.

### g) Kleinster Fix

`leseString` (`IpcVerbindung.cpp:441-459`) dekodiert die RFC-8259-Escapes einschließlich
`\uXXXX` mit Surrogatpaar und lehnt wie der Textriegel NUL-Escape und einsame Surrogate ab
(Regeln 4-6, README `:68-76`); vor dem flachen Lesen läuft die UTF-8-Prüfung
(`nakama::utf8::istGueltig`, wie in `PipeClient.cpp:700-701`). `Vertrag.cpp:213` und `:228`
zählen mit `utf8CodepointsBis` (`Intern.h:347-379`) auf 64 beziehungsweise 500. Eine
Sonderregel „Schlüssel ohne Escape" wird damit unnötig; die Duplikatprüfung `:515-518`
vergleicht dann dekodierte Namen. Keine Schemaänderung. Der Test `IpcTestMain.cpp:5780-5781`
wird auf den Vertrag umgestellt. Beziehung: **senden↔lesen** — die Codepunktkürzung des
Rust-Writers (`verbindung.rs:350`) und die Codepunktgrenze des Lesers sind dieselbe Zahl;
**Positiv↔Negativ** — 64/500 Codepunkte gültig, 65/501 ungültig, leerer `reason` gültig
(Schema `:918` ohne `minLength`), leere `broker_version` ungültig (`:900`).

### h) Ort des Rotbeweises

**C++, B10** im Block G10 (`IpcTestMain.cpp:4927` ff.) mit neuen Schaltern in
`tests/V3TestServer.h` (neben `:416-417`): `broker_version` aus 64 × „é" (128 Bytes) →
verbunden, heute nicht; 65 × „é" → nicht verbunden; `reject` mit 500 × „é" → Grund wird
gemeldet; `reason` „a\nb" → Grund „a⏎b" gemeldet; `reason` mit `\u0000` → abgelehnt.
Mutationen, die die Zusage brechen: `Vertrag.cpp:213` zurück auf `.size() > 64`;
`IpcVerbindung.cpp:450` wieder jedes `\\` ablehnen. Rust und Python brauchen keinen neuen
Fall (beide zählen Codepunkte, `bootstrap.rs:542-552`, jsonschema `len`).

---

## 6. T3-03-07 — beide FlatBuffers-Leser nehmen zusätzliche Adress-Slots an

*Befundzeile `BEFUNDE.md:384` · Skeptiker `:518` · B13 `:83` · Rohbeleg
`roh/phase-03-vertrag.md:26`, `:122`, `:172`*

### a) Behauptung des Audits

Eine Adresse mit den fünf gültigen Pflichtstrings plus einem belegten Feld mit ID 5 bleibt
in C++ und Rust gültig, obwohl die Zieladresse ausdrücklich geschlossen ist
(`BEFUNDE.md:384`, `:518`).

### b) Quellkette am HEAD

**Struktur.** Die VTable einer Tabelle ist `[vtable_len u16][table_len u16][Slot 0 …]`, der
Slot für ID k liegt bei Byte 4 + 2k; ID 5 ist die Position 14, `vtable_len` wird 16. Die
generierten Konstanten enden bei `VT_RUNTIME_NONCE = 12`
(`vertrag/generiert/nakama_telemetry_v1_generated.h:229-233`).

**Verifier C++.** `generiert/nakama_telemetry_v1_generated.h:251-263` besucht genau
`VT_LOGON_SID` bis `VT_RUNTIME_NONCE`, dann `verifier.EndTable()`;
`build/_deps/flatbuffers-src/include/flatbuffers/verifier.h:198-199` prüft die VTable nur auf
„Check((vsize & 1) == 0) && Verify(vtableo, vsize)".

**Verifier Rust.** `broker/src/generiert/nakama_telemetry_v1_generated.rs:510-523` besucht
dieselben fünf Felder; `flatbuffers-25.12.19/src/verifier.rs:371-388` (`visit_table`) prüft
`vtable_len` auf Geradzahligkeit (`:377`) und Puffergrenze (`:378`).

**Semantische Leser.** C++ `vertrag/NakamaTelemetrie.cpp:159-183` (`pruefeAdresse`: SID-Länge,
vier hex32), `:600-684` (`pruefe`: Dateikennung, Verifier, UTF-8 der fünf Strings `:639-655`,
Eintragszahl, doppelte Quelle, Frame) — kein Blick auf weitere Slots. Rust
`broker/src/telemetrie.rs:247-324` (`strukturriegel`: Selbstbezug nur der fünf bekannten
Adress-Slots `:260-270`), `:330-399` (`pruefe`), `:553-570` (`pruefe_adresse`).

**Weg durch das Produkt.** Sonde → Broker: `coordinator/senke.rs:125-137` →
`telemetrie::fuer_broker` (`telemetrie.rs:408-419`, übernimmt `payload: puffer.to_vec()`);
`senke.rs:157` vergleicht nur die fünf Felder mit der Linkadresse, `:212` speichert und
`:254` schreibt **dieselben Bytes** an jeden Main-Abonnenten. Broker → Main:
`SourcesModel.cpp:1635` → `nakama::telemetrie::lese` (`NakamaTelemetrie.cpp:686-759`), das
nur die fünf Felder kopiert (`:706-709`). Ein zusätzlicher Slot läuft damit durch beide Leser
hintereinander, ohne dass einer ihn sieht.

**Beide Seiten.** C++ gültig, Rust gültig; Python hat kein FlatBuffers-Bein. Ein Writer, der
Slot 5 belegt, existiert nicht: `TelemetryClient.cpp:81` baut die Adresse über den
generierten `fb::CreateAdresse (b, sid, projekt, epoche, instanz, nonce)`.

### c) Zusage wörtlich

- `CLAUDE.md:180-181`: „Für v3 sind Discriminator, Zieladresse, Revision und Capability
  nicht additiv erweiterbar."
- Entwurf `:2040-2042` (§33.1): „Discriminator, Zieladresse, Revision, Capability und
  sicherheitsrelevante Felder sind **nicht** additiv und erzwingen bei unbekannter Bedeutung
  Ablehnung."
- `eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs:116-117`: „Effektive
  Steueradresse (§32.1). KEIN Feld hier ist additiv: eine unbekannte Eigenschaft waere eine
  unbekannte Zieladresse."
- README v3 `:262-264` zitiert §33.1 für den JSON-Teil; dort setzt `additionalProperties:
  false` die Regel durch (Schema `:72-75`). Für FlatBuffers gibt es kein Gegenstück: die
  geschlossene Regelliste `flatbuffers/README.md:125-168` enthält keine Mengenregel für die
  Adresse, und `FELD-IDS.json:35-43` führt nur Namen, IDs und `verbrannt`.

### d) Heutige Abdeckung

- B3c und A4 (`contract_cross_language.rs:176` ff.) fahren den FB-Korpus (128 Einträge, Suche
  im FB-`MANIFEST.json` nach Zusatzfeld/Slot: kein Adressfall).
- A9 prüft Drift und Feld-IDs (`pruefe_fbs_feldids.py`), nicht fremde Slots am Eingang.
- Der Erzeuger kann den Fall heute nicht bauen: `erzeuge_fb_fixtures.py:16-26` erzeugt über
  den gepinnten `flatc` aus JSON (das Schema kennt ID 5 nicht) oder per Bytemutation
  (`ROHE_MUTATIONEN`), die keine VTable verlängert.

### e) Urteil: BESTÄTIGT

Präzisierungen: (1) der Broker leitet die Originalbytes weiter, der Slot erreicht Main
unverändert; (2) der `.fbs`-Kommentar `:116-118` behauptet die Geschlossenheit, und sein
Nachsatz („Alle fuenf sind `required` — das prueft schon der FlatBuffers-Verifier, nicht erst
unser Leser.") legt nahe, der Verifier sichere sie — er sichert nur die Anwesenheit.

### f) Kategorie und Schwere

**DEFEKT, medium** (Audit: high). Eine ausdrückliche Invariante wird in beiden Lesern nicht
durchgesetzt; der Schaden tritt erst mit einem Sender auf, der die Adresse erweitert, und
den gibt es heute nicht.

### g) Kleinster Fix

Nach der strukturellen Verifikation in beiden Lesern je Eintrag prüfen, dass die
Adress-VTable jenseits von ID 4 **keinen belegten** Slot trägt (Offset ≠ 0); ein längerer
VTable-Eintrag mit 0 bleibt zulässig, weil er „fehlt" bedeutet. C++ in `pruefe`
(`NakamaTelemetrie.cpp`, neben `:672-673`), Rust in `strukturriegel` oder `pruefe_adresse`
(`telemetrie.rs:247-324` / `:553-570`). Beide melden denselben neuen Verstoß, etwa
`("/eintraege/<i>/quelle", "adresse_zusatzfeld")`; die Regelliste
`flatbuffers/README.md:133-168` bekommt die Zeile, der `.fbs`-Kommentar `:116-118` wird
berichtigt. Fixture: `erzeuge_fb_fixtures.py` übersetzt in einem Temporärordner eine
Schemavariante mit `zusatz:string (id: 5)` in `Adresse` mit demselben gepinnten `flatc` und
legt `ungueltig/adresse-zusatzfeld-id5.bin` samt Manifestzeile ab; A10 hält es bytegleich,
`.gitattributes:36` und `*.bin binary` (`:5`) decken es. Beziehung: **Positiv↔Negativ** —
der ganze vorhandene Gültigkorpus bleibt gültig; **senden↔weiterreichen** — der Broker lehnt
ab, bevor er Bytes weiterreicht.

### h) Ort des Rotbeweises

B3c (FB-Korpusschleife in `SchemaTestMain.cpp`) und A4 (`contract_cross_language.rs:176`
ff.) klassifizieren das neue Fixture mit derselben Verstoßmenge; heute beide „gültig", also
rot. Mutation: der neue Slotriegel entfällt in einem der beiden Leser. Zusätzlich B13:
`SourcesModel::uebernehmeP2` (`SourcesModel.cpp:1623-1640`) mit dem Fixture → `false`; A4
über `senke.rs:125-131` → `messframe_abweisen(… FeatureBatchUngueltig)`.

---

## 7. T3-03-08 — der v2-Client liest Zahlen erst nach JUCEs Akkumulation

*Befundzeile `BEFUNDE.md:385` · Skeptiker `:519` · B12 `:82` · Rohbeleg
`roh/phase-03-vertrag.md:27`, `:123`, `:174`*

### a) Behauptung des Audits

Große dezimale `protocol_version`-, `seq`- und Reject-Grenzwerte überlaufen JUCEs `int64`
(signed overflow, UB), bevor `ganzzahl64` sie sieht (`BEFUNDE.md:385`, `:519`).

### b) Quellkette am HEAD

**Eingang.** `src/PipeClient.cpp:669-709` (`empfange`) prüft Länge (`:678`), rohes NUL
(`:688-692`), BOM (`:693-699`) und UTF-8 (`:700-705`), keine Zahl. Danach `:504` „const auto
v = juce::JSON::parse (antwort);" und `:600` für das ACK.

**Erster Überlaufpfad.** `juce_JSON.cpp:263` „intValue = intValue * 10 + digit;" ohne
Schranke; `:286-287` gibt `int` oder `int64` zurück.

**Zweiter Überlaufpfad, im Audit nicht genannt.** Ein Literal mit `e` geht über `:267-271` an
`CharacterFunctions::readDoubleValue`, dessen Exponent ebenfalls ungeprüft in einem `int`
wächst (README `:99-111`; gemessen: „`1e4294967296` | liest **1.0**"; ebenso der Kommentar
`NakamaVertrag.cpp:305-311`).

**Übernahme.** `PipeClient.cpp:42-69` (`ganzzahl64`) nimmt `int`, `int64` und ganzzahlige
`double` an. Verwendet für `protocol_version` (`:536-537`, Vergleich mit
`kProtocolVersion`), `min_protocol`/`max_protocol` (`:515-520`) und `seq` (`:605-606`,
Vergleich mit `aktuelleSeq`).

**Folgewert, hergeleitet aus einer Messung am selben Parser.** README `:44-46` misst
„`18446744073709552016` | liest **400**" — also den Rest modulo 2^64. Nach demselben
Verhalten wird `18446744073709551618` (= 2^64 + 2) zu 2, `2 == kProtocolVersion`, und das
`welcome` passiert die Prüfung `:533-544`. `2e4294967296` wird nach demselben Mechanismus wie
README `:109` („`1e4294967296` | liest **1.0**") zu `2.0`, das `ganzzahl64` ebenfalls als 2
übernimmt. Ein `heartbeat_ack` mit `seq = aktuelleSeq + 2^64`
bestätigt den laufenden Heartbeat. Formal bleibt das UB; ich behaupte keinen Lauf.

**Beide Seiten.** Rust-v2 liest typisiert (`broker/src/protokoll.rs:34`
`protocol_version: u32`, `:70` `seq: i64`; `server.rs:683`, `:784`
„serde_json::from_str::<ClientNachricht>") und lehnt beide Überbereiche ab. `INT64_MAX`
(`9223372036854775807`) selbst läuft auf beiden Seiten ohne Überlauf durch, `…808` kippt in
C++. Python hat kein v2-Instanzbein (A11 prüft nur die Schemas, `pruefe_v2_schemas.py:92-93`).

**Erreichbarkeit.** Gegenstelle ist der Rust-Broker, der diese Felder aus `u32`/`i64`
schreibt; ein eigener Writer erzeugt die Literale nicht. Eine fremde Belegung des v2-Namens ist
Sicherheitsarbeit (T3-12-03/-05) und nicht Gegenstand.

### c) Zusage wörtlich

- `eq-copilot/schemas/eq-ipc.schema.json:58` („"protocol_version": { "type": "integer",
  "enum": [1, 2] }"), `:144` („"seq": { "type": "integer", "minimum": 0, "maximum":
  9223372036854775807 …"), `:71-72` (`min_protocol`/`max_protocol` bis 4294967295).
- `CLAUDE.md:178-181`. Der Grund, warum v3 einen Textriegel hat, steht wörtlich in README
  `:54-57`: „der Wert ist beim Ankommen bereits verfaelscht. … Der einzige Ort, an dem alle
  drei Beine dasselbe sehen koennen, ist der Text." Für v2 fehlt diese Stufe.

### d) Heutige Abdeckung

A4b `PipeClientLifecycleTestMain.cpp:397-432` (ACK mit falscher Sequenz), `:456-466`
(Zusatzfeld), `:584-604` (Sättigung der eigenen Zähler beim Senden). Kein übergroßes Literal,
kein Exponent im empfangenen `welcome`, `reject` oder ACK (Suche nach `9223372036854775808`,
`18446744073709551`: kein Treffer). Die v3-Textriegelfälle zum großen Exponenten
(`TEXTRIEGEL-FAELLE.json` Fall 44/45, `:309-322`) schützen diesen Weg nicht.

### e) Urteil: BESTÄTIGT

Präzisierungen: (1) ein zweiter Überlaufpfad über den Exponenten; (2) nach dem gemessenen
Umklappverhalten wird ein vertragswidriges `welcome` als gültige Version 2 angenommen — die
Wirkung ist eine falsche Annahme, nicht nur ein unbestimmter Wert; (3) der Kommentar
`PipeClient.cpp:56-58` ist richtig, betrifft aber nur den späteren Cast, nicht die
Akkumulation davor.

### f) Kategorie und Schwere

**DEFEKT, medium** (Audit: high). UB in einem Produktparser auf fremden Prozessbytes, mit
belegbarer Fehlannahme; die eigene Gegenstelle erzeugt die Literale nicht, die fremde ist
Sicherheitsarbeit außerhalb dieses Teils.

### g) Kleinster Fix

Vor `juce::JSON::parse` (in `empfange` oder unmittelbar vor `:504` und `:600`) eine
lexikalische Zahlprüfung über alle Zahlliterale außerhalb von Zeichenketten: keine führende
Null, höchstens drei Exponentziffern und `|dez| < 308` wie Textriegel-Regel 3, ganzzahliger
Betrag höchstens `INT64_MAX` als Ziffernvergleich. Das ist `zahlPruefen`
(`NakamaVertrag.cpp:323-434`) mit der v2-Ganzzahlgrenze als Parameter statt `sichereGanzzahl`
(`NakamaVertrag.h:51`) — die v3-Grenze 2^53−1 darf v2 nicht beschneiden (Audit-Vorschlag
`BEFUNDE.md:385`). Keine Schemaänderung. Beziehung: **senden↔bestätigen** — `seq` bleibt über
den ganzen v2-Bereich bestätigbar; **Positiv↔Negativ** — `INT64_MAX` gültig, `INT64_MAX + 1`
ungültig, `2` und `2.0` gültig (v2-Schema ist draft 2020-12, `eq-ipc.schema.json:2`).

### h) Ort des Rotbeweises

**C++, A4b**, `PipeClientLifecycleTestMain.cpp` neben dem ACK-Sequenzfall `:397-432`: ein
Testserver sendet `welcome` mit `"protocol_version": 18446744073709551618`, dann mit
`2e4294967296`, dann ein ACK mit `seq = aktuelleSeq + 2^64`; erwartet wird „nicht verbunden"
beziehungsweise „ungueltiges heartbeat_ack" **und** ein `letzterFehler`, der den neuen
Zahlriegel nennt. Die Assertion auf den Riegelgrund macht den Rotbeweis unabhängig vom
UB-Wert. Dazu eine Vektortabelle direkt an der Riegelfunktion mit `9223372036854775807`
(gültig) und `9223372036854775808` (ungültig). Mutation: der Riegel vor `:504` entfällt.

---

## 8. T3-03-09 — doppelte JSON-Schlüssel ohne einheitliche Wire-Regel

*Befundzeile `BEFUNDE.md:386` · Skeptiker `:520` · B12 `:82` · Kartenentwurf PF11-14
`:302-307` · Rohbeleg `roh/phase-03-vertrag.md:28`, `:124`, `:176`*

### a) Behauptung des Audits

Der flache C++-Leser und die typisierten Rust-Leser lehnen doppelte Schlüssel ab, der
generische Rust-`Value` und der JUCE-Objektpfad behalten den letzten Wert; das betrifft auch
Discriminator und Adressfelder (`BEFUNDE.md:386`; Phase 10 `:520`: „JUCE/serde Value
ersetzen, der flache C++-Parser und typisierte Rust-Structs lehnen ab").

### b) Quellkette am HEAD

| Eingang | Stelle | Verhalten bei `{"k":A, …, "k":B}` |
|---|---|---|
| C++ flacher Leser (Handshake, ACK) | `IpcVerbindung.cpp:515-518` („Doppelter Schluessel ist eine Ablehnung, keine "letzter gewinnt"-Regel.") | Ablehnung |
| C++ JUCE (SourcesModel `:734`, `:1484`; v2-Client `PipeClient.cpp:504`, `:600`) | `juce_JSON.cpp:324` „resultProperties.set (propertyName, parseAny());" → `juce_NamedValueSet.cpp:180-189` ersetzt den Wert | **B** |
| Rust generischer v3-Weg (alle P0/P1 vom Plugin) | `schema.rs:424` → `serde_json` 1.0.151 `src/value/de.rs:137-142` (`values.insert(key, value)`), `src/map.rs:127-128`; `preserve_order` ist nicht aktiv (`Cargo.lock:311-321` ohne `indexmap`) | **B** |
| Rust P1-Weiche | `senke.rs:93-95` liest `type` aus einem eigenen `Value` | **B** |
| Rust typisiert **aus dem Rohtext** (Hello, v2) | `bootstrap.rs:254`, `:276`; `server.rs:683`, `:784`; `serde_derive-1.0.229/src/de/struct_.rs:269` (`duplicate_field`), für das v2-Tag `serde-1.0.229/src/private/de.rs:872` | Ablehnung |
| Rust typisiert **aus einem Value** | `evidenz.rs:129`, `befehl.rs:674`, `:770`, `:807` (`serde_json::from_value::<Adresse>(wert["adresse"].clone())`), `befehl.rs:167` (`kopf.ziel`) | **B** — der Value hat das Duplikat schon verschluckt |
| Python-Referenzbein | `pruefe_v3_vertrag.py:141` (`json.loads` ohne `object_pairs_hook`) → `json/decoder.py:216` („pairs = dict(pairs)") | **B** |

**Discriminator und Adresse.** `{"type":"state_report",…,"type":"evidence_snapshot"}` wird
im Broker in `senke.rs:93-95` als Evidenz geroutet und am Schema als Evidenz geprüft — beide
sehen denselben letzten Wert. Im Snapshot nimmt `SourcesModel` ein `session_epoch`, das
zuerst fremd und zuletzt passend ist (`:774-783` prüft nur den verbliebenen Wert).

**Am Hello heute.** Die Protokollwahl liest aus dem `Value` (`bootstrap.rs:220-252`, letzter
Wert), die Übernahme typisiert aus dem Rohtext (`:254`) — ein Duplikat eines Hello-Felds fällt
dort als `duplicate field`. Ein doppeltes `protocol_version` mit letztem Wert 2 geht an den
v2-Weg, dessen typisierter Leser (`server.rs:683`) es ebenfalls ablehnt. Am HEAD wird damit
jedes Hello mit doppeltem Schlüssel abgelehnt, über zwei Parseläufe.

**Die Regel existiert im Projekt schon — an der DTO-Kante, in drei Sprachen.** Rust
`broker/src/dto.rs:232-305` (`StrengerWert`, Kommentar `:237-242`: „`serde_json::Value` kann
das nicht: seine `Map` ist eine Map, sie UEBERSCHREIBT den ersten Wert still — genau wie
Pythons `json.loads` ohne `object_pairs_hook`. Der C++-Leser (`kanon::lies`) meldet den
doppelten Schluessel als eigenen Fehler."), C++ `state/NakamaKanon.cpp:364-365` (samt
Escape-Alias, Test `StateMigrationTestMain.cpp:933-934`), Python
`tools/eq-copilot/erzeuge_state_fixtures.py:402-416` (`object_pairs_hook`), gemeinsames
Fixture `eq-copilot/fixtures/state/dto/ungueltig/doppelter-schluessel.json` (erzeugt
`erzeuge_state_fixtures.py:870`).

### c) Zusage wörtlich

- Für den Wire-Vertrag: **keine ausdrückliche Regel gefunden** — weder im v3-Schema noch im
  README noch in SONDE-005a (Suche nach „doppelt"/„Duplikat": nur fremde Treffer).
- Allgemein: README `:3-5` (Uneinigkeit heißt, ein Bein ist falsch); Gate `plan.json:369`
  („Cross-Language-Gleichheit C++/Rust").
- Benachbarte Regeln: Entwurf `:4104-4106` (§53.8, DSP-DTO): „Nichtendliche Zahlen, doppelte
  Schlüssel und unbekannte Pflichtfelder werden vor dem Hash abgelehnt."; Entwurf `:3521`:
  RFC-8785-Testvektoren „decken NaN/Inf, doppelte Schlüssel und Unicode ab" (Signierweg).
- RFC 8259 §4 (nachgelesen): „The names within an object SHOULD be unique." und „When the
  names within an object are not unique, the behavior of software that receives such an
  object is unpredictable. Many implementations report the last name/value pair only."
- Kartenentwurf PF11-14 (`BEFUNDE.md:307`): „Default: Wire-JSON abweisen; binären State vor
  mutierbarer Übernahme abweisen beziehungsweise Originalbytes read-only erhalten, niemals
  still einen Wert verlieren. Technik legt den kompatiblen Weg fest."
- NAK-310 M-77 (`docs/beweise/NAK-310.md:1189`): „Tor und Coordinator lesen denselben einmal
  geparsten Wert: entweder wird das Hello wegen des doppelten Schlüssels abgelehnt, oder beide
  sehen dieselbe SID … Die Regel für doppelte Schlüssel selbst setzt NAK-313"; dazu
  `:2000-2006`.

### d) Heutige Abdeckung

- B10 `IpcTestMain.cpp:5782-5783` („doppelter Schluessel ist eine Ablehnung, keine 'letzter
  gewinnt'-Regel") — nur der flache Leser.
- A12/A4 DTO-Korpus (`contract_cross_language.rs:525` ff.) — nur die DTO-Kante.
- Kein v3-Fixture mit doppeltem Schlüssel, kein B13- oder A4-Fall am generischen Weg.

### e) Urteil: PRÄZISIERT

Die Kernaussage — keine einheitliche Regel, auch Discriminator und Adresse betroffen — trägt.
Zu weit ist „typisierte Rust-Structs lehnen ab" (Phase 10, `:520`; im Auftrag übernommen):
das gilt nur für den Parselauf aus dem Rohtext (Hello, v2). Die typisierten Adressen im
Coordinator entstehen aus einem schon geparsten `Value` und sehen nur den letzten Wert. Die
Phase-03-Fassung (`:386`, „typisierter Rust-Hello-Leser") ist genau. Für die Reparatur ist der
Unterschied wesentlich: „auf typisierte Structs umstellen" hilft nur, wenn aus dem Rohtext
gelesen wird. Neu gegenüber dem Audit: das Python-Referenzbein behält ebenfalls den letzten
Wert, und die DTO-Kante hat die Ablehnungsregel bereits vollständig.

### f) Kategorie und Schwere

**LÜCKE, Technikregel, medium.** Die Wire-Quellen schweigen; die Gleichheitszusage und die
Nachbarregel der DTO-Kante machen die Regel zu einer technischen Festlegung ohne
Produktwirkung (Begründung in „Regelfragen", Frage 1). medium, weil Discriminator und
Adressfelder betroffen sind und die Beine heute verschieden urteilen; ein eigener Writer
erzeugt keine Duplikate.

### g) Kleinster Fix

Regel „doppelter dekodierter Schlüssel im selben Objekt ist ungültig", an jedem Eingang vor
dem Aufbau eines schlüsseleindeutigen DOM, mit den vorhandenen Bausteinen: Rust `StrengerWert`
aus `dto.rs:245-305` crateweit nutzbar machen und in `schema.rs:424` sowie für die Weiche
`senke.rs:93-95` (oder: die Weiche liest aus demselben geprüften Wert) einsetzen; am
Bootstrap ein strenger Parselauf, aus dem Protokollwahl und typisierte Übernahme lesen (M-77).
C++ `kanon::lies` vor `SourcesModel.cpp:734`/`:1484` (derselbe Einstieg wie T3-03-04) und vor
`PipeClient.cpp:504`/`:600`. Python `pruefe_v3_vertrag.py:141` mit `object_pairs_hook` nach
`erzeuge_state_fixtures.py:405-416`. Fixtures: Duplikat des Discriminators `type`, eines
Adressfelds, beide Reihenfolgen, Escape-Alias (`"type"` gegen `"typ\u0065"`), dazu der
Gegenfall „gleicher Schlüssel in zwei verschiedenen Objekten" (gültig); im Manifest mit
eigenem Merker ohne Verletzungsmenge (Muster `erzeuge_v3_fixtures.py:3527-3531`), A8
bytegleich. Beziehung: **Positiv↔Negativ**, **Tor↔Coordinator** (M-77).

### h) Ort des Rotbeweises

- **Rust, A4:** `schema.rs`-Tests (neben `:454` ff.) — Heartbeat mit doppeltem `type` und
  Evidenz mit doppeltem `adresse.session_epoch` → `v3_nachricht_lesen` `None`, heute `Some`.
  Mutation: `schema.rs:424` zurück auf `Value`. Am Hello bleibt M-77
  (`doppelte_sid_schluessel_ein_wert`, beide Reihenfolgen) die Wache.
- **C++, B13:** Snapshot mit `session_epoch` zuerst fremd, zuletzt passend → `ungueltig`,
  heute `uebernommen`. Mutation: den strengen Einstieg vor `:734` entfernen.
- **Python, A5:** die Duplikatfixtures werden als ungültig klassifiziert; heute gültig.

---

## 9. Trennung Technik und Produkt

| ID | Was der Dirigent als Technikregel entscheidet | Was nur der User entscheidet | Was sofort baubar ist |
|---|---|---|---|
| T3-03-01 | Abwesenheit statt `null`, Byteinstanz, Testform. | Nichts. Schema, FB-README und Positivfixture sagen übereinstimmend „fehlt". | alles unter 1 g) und 1 h) |
| T3-03-02 | Helfer, Orte, Reihenfolge im Heartbeat, Fixtures. | Nichts. README `:208-210` und `:62-66` entscheiden die Wertregel. | alles unter 2 g) und 2 h) |
| T3-03-04 | Strenger Einstieg (`kanon::lies`), Manifestmerker. | Nichts. | alles unter 3 g) und 3 h) |
| T3-03-05 | Reihenfolge Textriegel → Parselauf → Schema → Übernahme; Abstimmung mit NAK-310 Etappe 4. | Nichts. README `:209-210` und der Textriegel entscheiden. | alles unter 4 g) und 4 h); die Winzraten-Lücke (`1e-300`) nur benennen |
| T3-03-06 | Escape-Dekodierung, Codepunktzählung, UTF-8-Prüfung, Testumstellung. | Nichts. JSON Schema §6.3.1 und beide Engines entscheiden. | alles unter 5 g) und 5 h) |
| T3-03-07 | Regelname, Ort, Fixturebau über Schemavariante. | Nichts. `CLAUDE.md:180-181` entscheidet. | alles unter 6 g) und 6 h) |
| T3-03-08 | v2-Grenze als Parameter, Ort des Riegels, Testform. | Nichts. Das v2-Schema entscheidet die Bereiche. | alles unter 7 g) und 7 h) |
| T3-03-09 | Die Regel selbst: „ablehnen" (Begründung Frage 1). | Nichts für den Wire. Die Duplikatkarte des Registers (`docs/offene-punkte.md:317`: „Duplikatregel W10") betrifft den binären State (T3-02-05), wo fremde Projektbytes verlustfrei zu halten sind — nicht Teil B. | alles unter 8 g) und 8 h) |

Keine der acht IDs braucht eine Produktkarte. Der Gate-Satz „Produktentscheide als Karten
nur, wo der Vertrag schweigt (Duplikatregel, …)" (`plan.json:369`) schließt eine Karte für
W30 nicht aus; ich empfehle keine, weil keine der beiden Regeln für vertragstreue Sender eine
sichtbare Wirkung hat.

---

## 10. Gemeinsame Ursachen

**Wurzel 1 — Produktleser prüfen die Darstellung statt des Werts.** T3-03-02 (Zifferntext,
JUCE-Typtag, `as_u64`), T3-03-06 (Bytes statt Codepunkte, Escape-Darstellung statt
dekodiertem Text), T3-03-08 (der Wert wird aus der Darstellung ungeprüft gebildet). Die
Vertragsleser beider Sprachen tun das Richtige und sagen es in ihren Kommentaren
(`NakamaVertrag.cpp:242-243`, `vertrag.rs:803-804`, `:812-814`, `NakamaVertrag.cpp:1106-1107`);
die Produktleser daneben nicht. **Ein Änderungssatz** für die C++-Ganzzahlen des flachen
Lesers zusammen mit Teil A (siehe unten); **getrennt** der Textsatz T3-03-06 (anderer Code,
keine Zahl) und der v2-Riegel T3-03-08 (anderer Client, andere Grenze).

**Wurzel 2 — der Textriegel ist kein Parser, und in C++ folgt ihm kein strenger.** T3-03-04 und
T3-03-09. In Rust und Python folgt dem Textriegel ein RFC-8259-Parser, in C++ JUCE. Der
strenge C++-Leser liegt im Plugin (`kanon::lies`), der duplikatfeste Rust-Leser im Broker
(`StrengerWert`), der Python-Riegel im State-Erzeuger. **Ein Änderungssatz** für beide IDs,
weil derselbe strenge Einstieg beide schließt.

**Wurzel 3 — Kanten neben dem gemeinsamen Vertragsweg.** T3-03-05 (Bootstrap ohne Textriegel
und Schema), T3-03-06 (Handshake ohne Textriegel und UTF-8), T3-03-08 (v2 ohne Zahlriegel),
T3-03-01 (Writer ohne Selbstprüfung). Im C++-Produkt läuft die Schemaengine nirgends; sie lebt
nur in Tests. Das erklärt, warum ein falscher Writer und nachsichtige Leser unbemerkt blieben.

**Wurzel 4 — `null` wird vor der Kante zu „fehlt".** T3-03-05 (serde `Option`) und, gespiegelt,
T3-03-01 (der Writer schreibt `null`, wo „fehlt" gemeint ist). Die C++-Seite hat die Regel
gelernt (`Sonde012SourcesModelTest.cpp:1476-1488`), der Bootstrap nicht.

**Wurzel 5 — Geschlossenheit wird nicht als Menge geprüft.** T3-03-07 (welche Felder
existieren) und T3-03-09 (wie oft ein Schlüssel vorkommt). Feldweise Leser sehen beides nicht.

**Berührung mit Teil A.** T3-03-03 (ACK-Grenze) und T3-02-03 (Revisionsbereich) hängen an
derselben Zahlenregel wie T3-03-02: README Regel 2 legt Form (`1.0` gültig) und Bereich
(±(2^53−1)) in einem Satz fest. Der flache ACK-Leser verletzt beides — er lehnt `7.0` ab und
nimmt `9007199254740992` an, und sein `stoull` macht aus einem Überlauf 0
(`Intern.h:215-216`, `:240-250`). Ein lexikalischer Ganzzahlleser mit Wert und Bereich
schließt T3-03-02 und T3-03-03 am flachen Leser zugleich; beide gehören in einen
Änderungssatz. T3-02-03 ist die Writer-Hälfte desselben Bereichs (`Intern.h:381-384`
`jsonSafe` kappt still, was der Gate-Satz „keine stillen Rundungen" berührt) — Teil A
entscheidet dort. T3-03-08 folgt derselben Rohzahlidee, aber mit der v2-Grenze `INT64_MAX`;
eine gemeinsame Funktion muss die Grenze als Parameter tragen.

**Berührung mit NAK-310 M-77.** Am HEAD lehnt der Bootstrap jedes Hello mit doppeltem Schlüssel
ab, allerdings über zwei Parseläufe (`bootstrap.rs:220-221` und `:254`). Die Reparatur von
T3-03-05 und T3-03-09 legt genau **einen** strengen Parselauf fest, aus dem Protokollwahl,
Tor und Coordinator lesen. Damit ist M-77 in der Alternative „abgelehnt" erfüllt und bleibt
als Wache stehen (`NAK-310.md:2005-2006`: „Die Regel setzt NAK-313; ändert sie das Parsen,
bleibt M-77 als Wache."). Beide Tickets ändern `bootstrap.rs:204-289` und
`server_v3/verbindung.rs:253-273`; NAK-313 sollte seinen Parselauf vor dem Wiederanlauf von
NAK-310 Etappe 4 liefern oder ihn dort ausdrücklich übergeben.

---

## 11. Regelfragen

### Frage 1 — Duplikatregel an jedem Produkteingang: Technik oder Produkt?

**Quellenlage.** Der Wire-Vertrag schweigt (8 c). RFC 8259 §4 empfiehlt Eindeutigkeit und nennt
das Verhalten bei Verstoß unvorhersehbar. Die DTO-Kante desselben Projekts lehnt ab (Entwurf
`:4104-4106`, drei Implementierungen und ein gemeinsames Fixture, 8 b). PF11-14 schlägt
„Wire-JSON abweisen" vor. M-77 lässt zwei Alternativen und überträgt die Wahl an NAK-313.

**Heute festgeschrieben.** Ablehnung: flacher C++-Leser (`IpcVerbindung.cpp:515-518`, Test
`IpcTestMain.cpp:5782-5783`, `docs/plugin-wissen.md:430-431`: „kein doppelter Schlüssel —
alles andere wird abgelehnt statt geraten"), Rust typisiert aus Rohtext, die ganze DTO-Kante.
Letzter Wert: JUCE, Rust `Value`, Python-Referenzbein.

**Technik oder Produkt: Technik.** Kein eigener Writer erzeugt Duplikate; betroffen sind nur
nicht vertragstreue Nachrichten, die ohnehin verworfen oder angenommen werden — der Musiker
merkt davon nichts. Anders als beim binären State (W10) liegen am Wire keine Projektdaten des
Users, die zu erhalten wären.

**Folgen der Wahl.**
- *Ablehnen* (empfohlen): Writer unverändert. Leser: C++ `kanon::lies` vor den JUCE-Eingängen,
  Rust `StrengerWert` am generischen Weg und am Bootstrap, Python `object_pairs_hook`.
  Fixtures: neue Negativklasse ohne Verletzungsmenge. Gleichheit: alle drei urteilen „ungültig"
  vor dem Schema; die Verletzungsmengen bleiben vergleichbar, weil keine entsteht. M-77 nimmt
  die Alternative „abgelehnt".
- *Letzter Wert*: die ablehnenden Leser müssten gelockert werden (flacher C++-Leser, serde
  derive, DTO-Kante), die DTO-Kante bekäme eine andere Regel als der Wire, und alle drei
  müssten Escape-Aliasse identisch auflösen. Ein „zuerst fremd, zuletzt passend"-Discriminator
  bliebe still annehmbar. Das ist technisch schwächer.

### Frage 2 — Zahlenformen: mathematischer Integer oder Zifferntext?

**Quellenlage.** Der Vertrag entscheidet ausdrücklich für den **mathematischen Integer**: README
`:208-210` und Regel 2 `:62-66`, Positivfixture `MANIFEST.json:337-340`, Goldens
`SchemaTestMain.cpp:1127-1128` und `vertrag.rs:1122-1125`, JSON Schema 2020-12 §6.1.1, jsonschema
`_types.py:195-201`. Die Gegenfrage ist damit keine offene Regelfrage, sondern ein Vertrag, von
dem die Produktleser abweichen.

**Heute festgeschrieben.** Wertregel: beide Schemaengines, Python, Textriegel, der C++-v2-Client
(`PipeClient.cpp:54-67`). Zifferntext beziehungsweise Integer-Token: C++ flacher v3-Leser
(`Intern.h:81-91`, `Vertrag.cpp:210`), `SourcesModel` (`:101-118`), Rust-Produktleser (`as_u64`,
`as_i64`, typisierte `u32`), Rust-v2 typisiert (`protokoll.rs:34`, `:70` — enger als das v2-Schema,
das draft 2020-12 nennt, `eq-ipc.schema.json:2`). Alle eigenen Writer schreiben Zifferntext
(2 b).

**Technik oder Produkt: Technik.** Keine sichtbare Wirkung; der Vertrag steht.

**Folgen der Wahl.**
- *Wertregel halten* (empfohlen): Writer unverändert (Zifferntext bleibt die kanonische
  Teilmenge). Leser: je Sprache ein Helfer, der Wert **und** Bereich prüft (C++ lexikalisch am
  flachen Leser, am SourcesModel aus dem strengen Leser; Rust aus `Value` nach dem Textriegel);
  Python unverändert. Fixtures: Positivvektoren in `.0`- und `e`-Form je Produkteingang.
  Gleichheit: alle Beine sagen für dieselben Bytes denselben Wert.
- *Auf Zifferntext verengen*: Vertragsänderung in README (`:62-66`, `:208-210`), Manifest
  (Positiv wird negativ), zwei Goldens und das Python-Bein (jsonschema kennt keine
  Zifferntextregel; es bräuchte einen eigenen Typprüfer). Weil der Textriegel das Schema nicht
  kennt, müsste die Regel feldweise außerhalb der Engine-Teilmenge (README `:164-173`) laufen —
  in drei Sprachen. Für Discriminator (`protocol` const 3) und Revisionen wäre es eine nicht
  additive Verengung. Das ist teurer und widerspricht dem Typbegriff des Schemas.
- Für alle Wege gilt der Bereich aus Regel 2 (±(2^53−1)) in v3 und die v2-Bereiche in v2;
  T3-03-03 und T3-02-03 (Teil A) sind die Bereichshälfte derselben Regel.

### Frage 3 — Text und Längen: Codepunkte oder Bytes, Escapes?

**Quellenlage und Wortlaut.** Längen: JSON Schema 2020-12 §6.3.1 („the number of its characters
as defined by RFC 8259"), beide Engines und ihre Tests (5 c), der Rust-Bootstrap
(`bootstrap.rs:542-543`), das SourcesModel (`Sonde012SourcesModelTest.cpp:1459`: „gezaehlt werden
CODEPOINTS - ein Vier-Byte-Zeichen zaehlt als eins"), der v2-Client (`PipeClient.cpp:112`).
Gezählt wird der dekodierte Text. Escapes: RFC 8259 §7; der Textriegel verbietet nur
`\u`-Escapes ohne vier Hexziffern, das NUL-Escape und einsame Surrogate (Regeln 4-6, README
`:68-76`) und führt `\"`, `\\`, `\u00e4` ausdrücklich als gültig (Falltabelle Fälle 57, 70,
71). Byteebene: gültiges UTF-8, kein BOM, kein rohes NUL (Regel 9, `:85-90`; RFC 8259 §8.1).

**Heute festgeschrieben.** Codepunkte überall außer im C++-v3-Handshake (`Vertrag.cpp:213`,
`:228`) und in der eigenen Adressprüfung `adresseGueltig` (`Vertrag.cpp:283`, SID in Bytes);
für die SID leitet das Audit daraus keinen Befund ab, weil Windows-SIDs ASCII sind
(`roh/phase-03-vertrag.md:120`). „Kein Escape" nur im flachen C++-Leser (`IpcVerbindung.cpp:439-451`, Test
`IpcTestMain.cpp:5780-5781`, `docs/plugin-wissen.md:430-431`) und im Tokenleser für eigene
Aufträge (`Intern.h:132-135`, dort mit der Begründung „weder ein Schluessel noch hex32 darf laut
Vertrag ein Escape benoetigen"). „Benötigen" stimmt; „enthalten dürfen" sie es nach dem Vertrag
trotzdem.

**Technik oder Produkt: Technik.** Der Vertrag spricht; die Folge der Korrektur ist ein
ehrlicher Fehlertext, keine neue Oberfläche.

**Folgen.** Writer: unverändert (`serde_json` escapt, kürzt nach `chars()`). Leser: der
C++-Handshake dekodiert, prüft UTF-8 und zählt Codepunkte; der ACK-Leser teilt denselben
`leseString` und bleibt für gültige ACKs unverändert. Fixtures: 64/65 und 500/501 Codepunkte aus
Zweibyte-Zeichen, `\n`/`\\`/`\"` positiv, `\u0000` und einsames Surrogat negativ, leerer
`reason` positiv, leere `broker_version` negativ. Gleichheit: C++, Rust und Python zählen
denselben dekodierten Text.

---

## 12. Reihenfolge

Maßstab: erst was eigene Writer im Normalbetrieb auslösen, dann die gemeinsame
Parserinfrastruktur, von der andere Reparaturen abhängen, dann die engen Einzelkanten.

1. **Matrix, gemeinsamer Vektorkorpus und Rotbeweise vorab** (ohne Produktänderung): die
   Rotbeweise aus den Abschnitten h) werden geschrieben und laufen rot.
2. **T3-03-01** — Writer, klein, unabhängig, als einziger heute im Normalbetrieb erreichbar.
3. **T3-03-04 und T3-03-09 zusammen** — strenger Einstieg in C++ (`kanon::lies`), Rust
   (`StrengerWert`) und Python (`object_pairs_hook`), neue Manifestklasse. Liefert zugleich den
   einen Parselauf, den M-77 braucht.
4. **T3-03-02 zusammen mit Teil A T3-03-03** — ein Ganzzahlleser je Sprache mit Wert und
   Bereich; T3-02-03 (Teil A) im selben Takt abstimmen. Nach Schritt 3, weil der
   SourcesModel-Wert aus dem strengen Leser kommen soll.
5. **T3-03-05** — Bootstrap: Textriegel → Parselauf aus 3 → Schema → Übernahme mit Zahlen aus 4.
   Vor dem Wiederanlauf von NAK-310 Etappe 4, mit Übergabe an dessen Tor.
6. **T3-03-06** — Handshaketext; unabhängig, kann neben 2 laufen.
7. **T3-03-07** — FlatBuffers-Geschlossenheit; unabhängig, kann neben 2 laufen (Bündel B13 hat
   laut `roh/phase-11-synthese.md:114` „Keine zusätzliche Bündelabhängigkeit").
8. **T3-03-08** — v2-Zahlriegel; unabhängig, nutzt den parametrisierten Zahlprüfer aus 4.

**Was der gemeinsame Vektorkorpus braucht.** Die Synthese verlangt „eine Matrix je realem
Eintritt, keine einzige Sammelassertion" (`roh/phase-11-synthese.md:56`). Konkret:

- **Eine Tabelle, hex-kodiert**, nach dem Muster `TEXTRIEGEL-FAELLE.json` (`:5`: „Als Text waere
  sie von keinem JSON-Leser unveraendert transportierbar"), weil Nachspann, Duplikate,
  NUL-Escapes und BOM darin stehen müssen. Erzeugt von `erzeuge_v3_fixtures.py`, A8 hält sie
  bytegleich, `.gitattributes:36` hält sie `-text`.
- **Je Eintrag:** der Produkteingang (flacher C++-Leser Control/Telemetrie, SourcesModel
  Snapshot/Rücknahme, v2-Client, Rust-Bootstrap, Rust P0, Rust P1, FB-Leser), die Bytes, das
  von Hand geschriebene Vertragsurteil, bei Ganzzahlfeldern der **erwartete Wert** (sonst fällt
  ein stiller Ersatzwert nicht auf) und die Ablehnungsstufe (Textriegel, Parser, Duplikat,
  Schema, Feldregel).
- **Leser:** C++ in B10, B13, B16 und A4b, Rust in A4, Python in A5 (Vertragsurteil über
  jsonschema, strengen Parser und Duplikatriegel). Die FlatBuffers-Vektoren bleiben im
  FB-Korpus (A10, B3c, A4).
- **Zahlenränder:** `1.0`, `1e0`, `91.0`, `9007199254740991`, `9007199254740991e0`,
  `9007199254740992`, `9007199254740992.0`, `INT64_MAX`, `INT64_MAX + 1`,
  `18446744073709551618`, `2e4294967296` (v2), `1e-307` (gültig), `1e-308` (ungültig).
- **Text:** NUL-Escape, rohes NUL, BOM, 64/65 und 500/501 Codepunkte aus „é", leerer `reason`,
  leere `broker_version`, `\n`, `\\`, `\"`, einsames Surrogat.
- **Struktur:** Nachspann `[]`, zweites Objekt, Schlusskomma, `\q`, Duplikat von `type`, von
  `session_epoch` und `logon_sid` in beiden Reihenfolgen, Escape-Alias.
- **Hello:** `host: null`, `host.name: null`, `host.version: null`, `host` fehlt (gültig).
- **Evidenz:** `local_monotonic` mit Startwert fehlt (gültig), `null` (ungültig), Zahl
  (ungültig).
- **Wirkungsassertionen außerhalb der Tabelle**, im jeweiligen Bein: keine Teilmutation bei
  Ablehnung (etwa `erster_heartbeat_gesehen` unverändert), keine Weitergabe eines abgelehnten
  FeatureBatch, kein freigegebener In-Flight-Auftrag bei ungültigem ACK.

---

## 13. Nicht geprüft

- **Kein Lauf.** Bau, Test, Kanon, Skript, FL Studio und Fehlerinjektion waren verboten. Alle
  Urteile sind Quellherleitungen. Drei Wirkungen sind ausdrücklich nur hergeleitet: der
  Umklappwert 2 in T3-03-08 (aus der README-Messung `:44-46`, `:109`), der stehende Nachlauf bei
  Winzraten in T3-03-05 und die Ablehnung des `null`-Starts durch die C++-Engine in T3-03-01.
- **Der Python-Einzeiler** (siehe Prüfform) hat nur Schlüssel ausgegeben; er ist keine Prüfung.
- **Fremdquellen nicht gegen den Bau verifiziert.** JUCE und die FlatBuffers-C++-Header stammen
  aus `eq-copilot/build/_deps/` (Bauartefakt); dass sie genau dem Pin entsprechen, habe ich nicht
  geprüft. Die Rust-Crates passen nach Versionsnummer zu `broker/Cargo.lock`. Das Python-Bein
  läuft laut README `:19` unter `py -3.13` mit `jsonschema` 4.26; gelesen habe ich 4.25.1 unter
  `C:\Python314`; die zitierte Integerregel ist seit Draft 6 dieselbe.
- **Die Anzeige von `letzterFehler`** (wo und wie der Handshakegrund im Produkt erscheint) habe
  ich nicht verfolgt; T3-03-06 sagt nur, welcher Text entsteht.
- **Nicht jede Stelle aus Anhang C** des Audits einzeln: die Speicherleser im hinteren Teil von
  `experiment_verdrahtung.rs` (`:1284` ff.) lesen gespeicherte Stände, nicht den Draht; ich habe
  sie nicht bewertet.
- **Sicherheit:** fremde Prozesse auf den Pipes, Tor-Authentisierung und T3-12-\* sind
  ausgeklammert (W30). Wo ein Befund nur mit einer nicht vertragstreuen Gegenstelle greift, steht
  das als Erreichbarkeit, nicht als Sicherheitsurteil.
- **Die Winzraten-Lücke** (`audio_lage.samplerate` ohne sinnvolle Untergrenze, `:163`) und der
  fehlende Zähler für am Schema gescheiterte P1-Snapshots (`senke.rs:103-105` gegen
  `evidenz.rs:125-128`) sind Beobachtungen außerhalb der acht IDs, ohne Einordnung.
- **Dokudrift, nur benannt:** `docs/plugin-wissen.md:1806-1808` nennt „acht Regeln" und 59 Fälle;
  README `:59` sagt „Neun Regeln", die Falltabelle trägt 102 (`TEXTRIEGEL-FAELLE.json:6`).
- **Teil A und Teil C** (andere IDs von NAK-313) sind nicht Gegenstand; ihre Berichte habe ich
  nicht gelesen. Die Berührungen in Abschnitt 10 stützen sich auf Befundzeilen und Code.
- **Kein Dokuriegel.** `tools/plan/dokuriegel.py` ist ein Skript; diese Datei ist ein Rohbeleg,
  kein Plandokument.

---

FERTIG Teil B, 7 bestätigt, 1 präzisiert, 0 widerlegt.
