# Beweismanifest — S6 «`SONDE-005b`: FlatBuffers, gepinntes flatc, Codegen-Drift»
<!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-21 nachgearbeitet -->
<!-- NAKAMA-URTEIL: T3 NEEDS_WORK 2026-08-24 offen -->
<!-- NAKAMA-URTEIL: T3 NEEDS_WORK 2026-08-24 nachgearbeitet -->
<!-- NAKAMA-URTEIL: T3 PASS 2026-08-28 -->

> **T3 / Gate G1, 24.08.2026 — NEEDS_WORK.** Zwei der vier Wire-Float-Traeger
> des `.fbs` (`start_ppq`, `end_ppq`, Zeilen 145-146) sind in **beiden**
> handgeschriebenen Lesern ungeprueft — Rust `telemetrie.rs` und C++
> `NakamaTelemetrie.cpp` haben dieselbe Auslassung, weshalb der Kreuzsprachtest
> uebereinstimmt: auf der falschen Antwort. Kein Binaerfixture deckt `ppq` ab
> (gezaehlt: 0), waehrend dieselbe Fehlerklasse fuer die anderen Traeger gedeckt
> ist. Vollstaendig: `docs/beweise/G1.md` §4.3, §7.1.


**Ticket-Gate im Wortlaut** (`docs/FL-Nakama-Sonden-Design-Entwurf.md` §65):

> `SONDE-005` | v3-Domain-/JSON-/FlatBuffers-Schemas, Bandgitter und
> Cross-Language-Fixtures | **C++/Rust validieren identisch; Codegen-Drift ist 0**

**Sessionschnitt** (`docs/bauaufteilung-sonden.md` §3, P1):

> S6 | `SONDE-005b` | **FlatBuffers (Feld-IDs!), gepinntes `flatc`, Codegen-Drift-Test** | T1+T2

Die erste Gate-Hälfte ist in `SONDE-005a` erfüllt (JSON-Vertrag, dort auch die
T2-Runde 1 mit 24 Befunden). Hier kommt die zweite dazu — und die erste musste
dabei grün bleiben.

Der wörtliche Auftrag aus Entwurf §53:

> „FlatBuffers-Felder erhalten explizite numerische `id`-Attribute. IDs werden
> nie wiederverwendet, auch nicht nach Entfernen eines Feldes. Eine gepinnte
> `flatc`-Version erzeugt C++ und Rust; die generierten Dateien werden
> committed und ein Drift-Test verlangt bitgleichen Neugenerierungsdiff."

---

## 0. Was diese Session ausdrücklich NICHT beweist

Damit kein Prüfer es erst herausfinden muss:

1. **Dass irgendein Produktionscode diesen Vertrag spricht.** Kein Processor
   und kein Broker-Pfad erzeugt heute einen `FeatureBatch`. Das ist
   Schnittgrenze, keine Lücke: der Erzeuger kommt in `SONDE-009`. Was hier
   steht, ist der Vertrag — und er ist prüfbar, bevor es einen Sprecher gibt.
2. **Den 16-Byte-Envelope und CRC32C.** Entwurf §65 gibt den Parser
   `SONDE-010`. Genau deshalb trägt der Payload auch **kein**
   `schema_major`/`schema_minor`: der Envelope trägt beide bereits, und zwei
   Wahrheiten über dieselbe Zahl in einem Frame widersprechen sich irgendwann.
3. **Dass JSON- und Binärdarstellung dieselben WERTE tragen.** Beide tragen
   dieselben Felder, dieselben Grenzen und dieselben sieben Gültigkeitsbits —
   aber es gibt heute keinen Konverter zwischen ihnen und deshalb auch keinen
   Roundtrip-Beweis. Ein solcher gehört zu dem Ticket, das den Konverter baut.
4. **Die Kadenzzahlen aus §33.2.** 10 Hz Live, 1–4 Hz Evidenz, „typisch vier
   bis fünf" Sonden je Bündel sind Design-Startwerte; der Entwurf sagt selbst,
   der Phasenplan müsse sie gegen CPU und Pipe-Last messen. Hier ist nur die
   **Obergrenze** 32 durchgesetzt (§53.9), nicht die Erwartung.
5. **Dass der Leser unter Last hält.** `pruefe()` wird heute ausschliesslich
   aus den beiden Testtreibern gerufen; Ratengrenzen, Backpressure und das
   Verhalten bei 32 Sonden × 10 Hz gehören zu `SONDE-010`. Was hier bewiesen
   ist: er urteilt korrekt und stürzt an keinem der 51 Fixtures ab — nicht,
   dass er schnell genug ist.
6. **Dass `--conform` gegen jede zukünftige Änderung schützt.** Es prüft
   Evolutionsregeln (Feld entfernt, Typ geändert, id umgehängt, Enumwert
   verschoben) — vier davon sind unten vorgeführt. Eine semantische Änderung
   bei gleichbleibender Struktur sieht es nicht; dagegen steht der
   Fixture-Korpus.
7. **Bedingte Feldpflichten** (`zeitbasis` ↔ `project_sample_start`,
   `bounds_valid` ↔ PPQ). Sie fehlen hier **genau wie im JSON-Vertrag** und
   werden dort als **NAK-29** geführt. Die beiden Darstellungen sollen EINE
   Aussage sein; eine Regel, die nur auf einer Seite gilt, wäre der Anfang der
   Drift.

---

## 1. Behauptungen

> **Alle Zahlen und Rohausgaben in §1 und §2 stammen aus EINEM Lauf am
> 21.08.2026, nach T2-Runde 4.** Das steht hier, weil das Dokument diesen Test
> vorher nicht bestand: nach Runde 2 war die Behauptungstabelle nachgezogen
> worden, die verlinkten Rohblöcke nicht — es widersprach seinen eigenen
> Belegen (Runde 3, Befund 2).

| # | Behauptung | Befehl | Ergebnis | Rohe Ausgabe | Datum |
|---|---|---|---|---|---|
| 1 | **Jedes** Tabellenfeld trägt ein explizites `id`; je Tabelle lückenlos ab 0, keine doppelt, keine verbrannte wieder in Gebrauch | `pruefe_fbs_feldids.py` | ✅ 8 Tabellen, 47 Felder, 0 ohne id, 15/15 Offsetfelder gedeckt | [↓ P1](#p1) | 2026-08-21 |
| 2 | Der Feld-ID-Riegel **fällt nachweislich** — auch bei der Mutation, die `flatc` selbst durchlässt, und bei `include` | 5 Mutationen | ✅ je Exit 2 mit benannter Ursache | [↓ P5](#p5) | 2026-08-21 |
| 3 | `flatc`, C++-Runtimeheader und die **aufgelöste** Rust-Crate tragen dieselbe Version, und der Pin ist ein **Commit** | `pruefe_flatc_drift.py` | ✅ 25.12.19 dreifach | [↓ P2](#p2) | 2026-08-21 |
| 4 | **Codegen-Drift ist 0**: die Neugenerierung aus dem `.fbs` ist bytegleich zum committeten C++- und Rust-Code | `pruefe_flatc_drift.py` | ✅ 0 Dateien | [↓ P2](#p2) | 2026-08-21 |
| 5 | Der Drift-Test **fällt nachweislich** bei einem einzigen geänderten Byte | 1 Byte in der `.h` | ✅ Exit 2 mit SHA-256-Paar | [↓ P6](#p6) | 2026-08-21 |
| 6 | `flatc --conform` ist **scharf** — vier schemabrechende Mutationen abgelehnt, der unveränderte Stand akzeptiert | `pruefe_flatc_drift.py --conform` | ✅ 4 abgelehnt + Gegenprobe | [↓ P3](#p3) | 2026-08-21 |
| 7 | Der Binärkorpus hat **51 Fixtures** mit handgeschriebenem Manifest und ist bytegleich zur Neuerzeugung | `erzeuge_fb_fixtures.py --pruefen` | ✅ 52 Dateien | [↓ P4](#p4) | 2026-08-21 |
| 8 | **Rust** klassifiziert jeden Binärfixture wie das Manifest — Urteil UND Verstoßmenge | `cargo test` | ✅ 6 Tests | [↓ P7](#p7) | 2026-08-21 |
| 9 | **C++** ebenso, gegen dasselbe Manifest | `EqCopSchemaTest` | ✅ 56/56, 51 Fixtures | [↓ P8](#p8) | 2026-08-21 |
| 10 | Der Cross-Language-Vergleich ist **nicht zeremoniell**: ein Byte bringt beide Beine zum Fallen, mit demselben Pfad und derselben Regel | 1 Byte in `live-64-band.bin` | ✅ identischer Verstoß | [↓ P9](#p9) | 2026-08-21 |
| 11 | **Kaputtes UTF-8 in einer Zeichenkette wird abgelehnt statt zu korrumpieren** — 200 Durchgänge, stabiles Urteil, kein Absturz | `EqCopSchemaTest` | ✅ stabil | [↓ P8](#p8) · [↓ P13](#p13) | 2026-08-21 |
| 12 | Beide Leser prüfen ihre einkompilierten Bandwertgrenzen **gegen die Vertragsdatei** | beide Beine | ✅ je 2 Proben | [↓ P7](#p7) · [↓ P8](#p8) | 2026-08-21 |
| 13 | Die Binärfixtures überleben einen `git`-Auscheckvorgang **bytegleich** | `git archive` + SHA-256 | ✅ 207 von 207 bytegleich | [↓ P10](#p10) | 2026-08-21 |
| 14 | Ein fehlendes oder falsches Werkzeug führt zu **Abbruch mit dem richtigen Exitcode** — 3 für „Voraussetzung fehlt", 2 für „Behauptung widerlegt" | vier Störfälle | ✅ 3 · 3 · 2 · 2 | [↓ P12](#p12) | 2026-08-21 |
| 15 | Der Kanon bleibt grün — im vereinten Stand mit der parallelen Kontext-Inventur **15/15**; das Produktionsbundle bleibt hostfähig | `beweise.ps1` · `pluginval -8` | ✅ GRUEN 15/15 · SUCCESS | [↓ P11](#p11) · Kanonlauf am Ende | 2026-08-21 |
| 16 | Ein `uoffset` **0** (Selbstbezug) wird in BEIDEN Beinen gleich klassifiziert — gemessen an der **Klasse**, nicht an einem Fixture | 6215 Byte-Mutanten durch beide Leser | ✅ **0** von 6215 (ohne Riegel: 143); drei weitere Zellwerte ebenfalls 0 | [↓ P14](#p14) | 2026-08-21 |
| 17 | Der Riegel gegen die **nächste** Lücke fällt nachweislich — neues Offsetfeld ohne Riegelzeile, und weggefallenes `required` an der Elementtabelle | 2 Mutationen des `.fbs` | ✅ je Exit 2 mit benannter Ursache | [↓ P14](#p14) | 2026-08-21 |

---

## 2. Rohe Ausgaben

### P1 · Feld-IDs <a id="p1"></a>

**Befehl:** `py -3.13 tools/eq-copilot/pruefe_fbs_feldids.py` · **Exitcode:** 0

```text
Schema:   eq-copilot\schemas\v3\flatbuffers\nakama_telemetry_v1.fbs
Tabellen: 8
Felder:   47
Felder ohne id: 0
Offsetfelder (string/Vektor/Tabelle) im Strukturriegel: 15
keine Luecke, keine Doppelung, keine Abweichung zur eingefrorenen Liste
Pruefungen: 0 rot
```

### P2 · Codegen-Drift <a id="p2"></a>

**Befehl:** `py -3.13 tools/eq-copilot/pruefe_flatc_drift.py` · **Exitcode:** 0

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (...\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

Die Zeile „Rust-Crate … (Cargo.lock; Anforderung …)" ist ein Befund aus
T2-Runde 3: vorher wurde nur die **Anforderung** aus `Cargo.toml` gelesen.
`flatbuffers = "25.12.19"` bedeutet in Cargo aber `^25.12.19` — nach einem
`cargo update` wäre der Riegel grün geblieben, während der erzeugte Code in
eine andere Laufzeit ruft. Die Wahrheit steht in `Cargo.lock`.

### P3 · `--conform` ist scharf <a id="p3"></a>

**Befehl:** `py -3.13 tools/eq-copilot/pruefe_flatc_drift.py --conform` · **Exitcode:** 0

```text
--conform (Evolutionsriegel):
  ok:  'Feld entfernt' abgelehnt (Exit 1) - schemas don't conform: field deleted: evenacadia.nakama.v3.Bandwerte.saturated
  ok:  'Feldtyp geaendert' abgelehnt (Exit 1) - schemas don't conform: types differ for field: evenacadia.nakama.v3.Transportstempel.sample_count
  ok:  'Feld-ID umgehaengt' abgelehnt (Exit 1) - schemas don't conform: offsets differ for field: evenacadia.nakama.v3.Frame.korrelation
  ok:  'Enumwert verschoben' abgelehnt (Exit 1) - schemas don't conform: values differ for enum: nakama_log64_v1
  ok:  unveraenderter Stand wird akzeptiert (Gegenprobe)

Pruefungen: 0 rot
```

Die **Gegenprobe** trennt den Riegel von einer Zeremonie: ohne sie meldete er
nur, dass `flatc` immer schimpft.

### P4 · Binärkorpus <a id="p4"></a>

**Befehl:** `py -3.13 tools/eq-copilot/erzeuge_fb_fixtures.py --pruefen` · **Exitcode:** 0

```text
9 gueltige, 42 ungueltige Fixtures
  bytegleich: 52 Dateien, MANIFEST sha256=15bcde347cfa559c0b2992c5f9f5f6e7583f64e7b4ba33347b4c46f808942a35
```

### P5 · Der Feld-ID-Riegel fällt <a id="p5"></a>

Mutationen einer Kopie des Schemas, gegen die eingefrorene ID-Liste gefahren.
**M4 ist die wichtige**: eine Tabelle ganz ohne `id`-Attribute — die Mutation,
die `flatc` selbst anstandslos übersetzt. **M5** kam aus T2-Runde 3 dazu.

```text
── m1 ── (ein Feld ohne id)
  ROT: Bandwerte: Felder ohne id: saturated
  ROT: Bandwerte: Schema und FELD-IDS.json weichen ab
   EXIT=2
── m2 ── (zwei Felder mit derselben id)
  ROT: Bandwerte: doppelte id(s) [0]
  ROT: Bandwerte: ids nicht lueckenlos ab 0 — [0, 0, 2, 3, 4, 5]
   EXIT=2
── m3 ── (zwei ids getauscht)
  ROT: Frame: Schema und FELD-IDS.json weichen ab — im Schema {'peak_db': 9, 'korrelation': 5}
   EXIT=2
── m4 ── (eine ganze Tabelle ohne ids — flatc laesst das durch)
  ROT: QuellenEintrag: Felder ohne id: quelle, frame
   EXIT=2
── m5 ── (include, T2-Runde 3)
  ROT: `include` im Vertrag: eine eingebundene Datei bringt Tabellen mit, die dieser Riegel nicht sieht.
   EXIT=2
```

Und die Gegenprobe zu m5: ein Blockkommentar mit einer Phantomtabelle
(`/* table Geist { a:int; b:int; } */`) ist jetzt **grün** — vorher meldete
der Riegel dort 9 Tabellen und 4× ROT, also rot aus dem falschen Grund.

### P6 · Der Drift-Test fällt <a id="p6"></a>

Ein einziges geändertes Byte im committeten Header (`19` → `18` in der
Versionszusicherung):

```text
  ROT: DRIFT in eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h
       committed  sha256=278db15f58eb530ec958c99b735ed61524cf5ead2659cb65c10eafb2eec916dc
       neu erzeugt sha256=a3454de8187a9cdc37e4d486f99510ae8ac21452a93c9c463068fb403f36bd30
Drift: 1 Datei(en)
DRIFT_EXIT=2
```

Nach dem Wiederherstellen: `Drift: 0 Dateien`, Exit 0. (Die SHA-Werte stammen
aus dem Stand vor der `process_context_present`-Ergänzung; die Aussage — ein
Byte genügt — ist davon unberührt.)

### P7 · Rust-Bein <a id="p7"></a>

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --test contract_cross_language` · **Exitcode:** 0

```text
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test quantisierung_stimmt_mit_den_testvektoren ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 6 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out
```

### P8 · C++-Bein <a id="p8"></a>

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` · **Exitcode:** 0

```text
[ok]   Binaerkorpus klassifiziert wie das Manifest (51 Fixtures)
[ok]   Binaerkorpus hat Substanz  [51 Fixtures]
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
-----------------------------------------
56 bestanden, 0 gescheitert
```

### P9 · Ein Byte bringt BEIDE Beine zum Fallen <a id="p9"></a>

`gueltig/live-64-band.bin`, ein Byte: `FF` → `75`. Der erste Bandwert wird
damit von `-123` zu `30085`, weit über der Obergrenze `240` von
`q_db_0p1_i16`.

```text
=== RUST ===
gueltig/live-64-band.bin: Urteil gueltig erwartet, Leser sagt ungueltig
  ([Verstoss { pfad: "/eintraege/0/frame/baender/werte_i16/0", regel: "bandwert_bereich" }])
test result: FAILED. 3 passed; 1 failed

=== C++ ===
[ROT]  gueltig/live-64-band.bin: Urteil gueltig erwartet, Leser sagt ungueltig
       {/eintraege/0/frame/baender/werte_i16/0 | bandwert_bereich}
```

**Derselbe Pfad, dieselbe Regel, aus zwei unabhängigen Implementierungen.**

### P10 · Der Korpus überlebt git <a id="p10"></a>

Dieses Projekt hat die Zeilenenden-Landmine zweimal bezahlt. Deshalb gemessen
statt geglaubt:

```text
$ git check-attr -a …/gueltig/live-64-band.bin
… binary: set   text: unset
$ git ls-files --eol …
i/-text w/-text attr/-text   …/live-64-band.bin

$ git archive HEAD eq-copilot/fixtures/v3 | tar -x -C <temp>
207 bytegleich, 0 abweichend  (ganzer v3-Korpus: JSON + Binaer + Falltabelle)
git ls-files zaehlt 207 Dateien im selben Unterbaum
P10 STIMMT
```

> Die **196** stand hier bis zum 21.08. und war seit drei Fixture-Runden falsch
> (T2-Runde 4, Befund B-1: der Prüfer maß 206, das 207. ist das Fixture dieser
> Runde). Sie ist nicht auf die neue Ziffer *korrigiert*, sondern **nachgemessen**
> worden — mit einer Gegenprobe gegen `git ls-files`, denn ein Auspacker, der
> still die Hälfte verliert, meldete sonst ebenfalls „0 abweichend".

### P11 · Echter VST3-Host <a id="p11"></a>

**Befehl:** `pluginval --strictness-level 8 --validate …\EQ-Copilot.vst3` · **Exitcode:** 0

```text
Starting tests in: pluginval / Fuzz parameters...
Completed tests in pluginval / Fuzz parameters
SUCCESS
```

### P12 · Die Vorbedingungen greifen — mit dem richtigen Exitcode <a id="p12"></a>

Ein Beweismanifest darf „das Werkzeug fehlt" nie mit „die Behauptung ist
widerlegt" verwechseln. Vier Störfälle, gemessen:

```text
(a) Zeiger zeigt ins Leere      -> VORAUSSETZUNG FEHLT: flatc nicht gefunden        EXIT=3
(b) Zeiger fehlt ganz           -> VORAUSSETZUNG FEHLT: flatc nicht gefunden        EXIT=3
(c) Cargo.lock loest auf 25.9.23 -> ROT: ... Der erzeugte Rust-Code ruft in eine
                                    Laufzeit, die er nicht kennt.                   EXIT=2
(d) WERKZEUG.json auf 99.9.9    -> ROT: flatc meldet 25.12.19, gepinnt ist 99.9.9   EXIT=2
```

(a) und (b) sind **3** — ohne Werkzeug ist nichts gemessen. (c) und (d) sind
**2**: das Werkzeug ist da, und was es meldet, widerspricht dem Pin. (d) war
vor T2-Runde 3 eine 3; ein manipulierter Steckbrief ist aber ein
Vertragsbruch, keine fehlende Voraussetzung.

### P13 · Kaputtes UTF-8 korrumpiert nichts mehr <a id="p13"></a>

Der schwerste Fund der Sitzung, aus T2-Runde 3. **Vorher**, gemessen vom
Prüfer an `gueltig/mit-schleife.bin` mit **einem** geänderten Byte (`0x80` in
der `logon_sid`):

```text
Rust : UNGUELTIG { | verifier}   — jedes Mal, kein Absturz
C++  : gueltig                   — und ab der ZWEITEN Verarbeitung:
       n=1 → Exit 1 · n=2 → 0xC0000374 (STATUS_HEAP_CORRUPTION)
       n=5 → 0xC0000409 (STATUS_STACK_BUFFER_OVERRUN) · n=20/120 → 0xC0000374
```

In einem 600er-Byte-Mutations-Fuzz über die gültigen Fixtures liefen **78 von
591** Mutanten auseinander und **neun stürzten ab**.

**Nachher**, gemessen an derselben Eingabe, die jetzt als
`ungueltig/sid-ungueltiges-utf8.bin` dauerhaft im Korpus liegt:

```text
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
```

Zehn vollständige Testläufe hintereinander: Exitcodes `0, 0, 0, 0, 0, 0, 0, 0, 0, 0`.

Ursache und Lehre stehen in `schemas/v3/flatbuffers/README.md`: **der
Verifier ist in den beiden Sprachen nicht gleich stark.** Rust prüft
String-Inhalte auf gültiges UTF-8, C++ nicht — und der Vertrag war darauf
gebaut, dass sich dieselbe Bibliothek in zwei Sprachen gleich verhält.

### P14 · Der Selbstbezug als Klasse <a id="p14"></a>

**Korpus** (reproduzierbar, nicht geerbt): jede 4-byte-ausgerichtete Zelle ab
Offset 4 aller **neun** gültigen Fixtures einzeln auf 0 gesetzt — genau die
Mutation, die aus einem Zeiger einen Selbstbezug macht. Zellen je Fixture =
Größe/4 − 1. Offset 0 ist ausgenommen: der Wurzeloffset ist eine eigene
Fehlerklasse und hat eigene Fixtures.

```text
  alle-validity-bits              656 Byte ->   163 Zellen  ok
  broker-buendel-5               2784 Byte ->   695 Zellen  ok
  evidenz-221-band                976 Byte ->   243 Zellen  ok
  float32-kreuzstatistik         1416 Byte ->   353 Zellen  ok
  grenze-32-eintraege           16496 Byte ->  4123 Zellen  ok
  live-64-band                    640 Byte ->   159 Zellen  ok
  mit-schleife                    736 Byte ->   183 Zellen  ok
  ohne-optionale-kennzahlen       600 Byte ->   149 Zellen  ok
  sid-ist-ein-nul                 592 Byte ->   147 Zellen  ok
6215 Mutanten aus 9 gueltigen Fixtures
```

**Beide Beine über denselben Korpus.** Rust: ein Helferbinary über
`telemetrie::pruefe`. C++: `EqCopSchemaTest` über einen **Schattenbaum** — der
Test sucht sein `MANIFEST.json` zuerst relativ zum Arbeitsverzeichnis und fällt
für alles andere auf das Repo zurück; die Erwartung im Schatten ist absichtlich
„ungültig mit LEERER Verstoßmenge", damit jedes einzelne Urteil aus dem
Testlauf herausfällt. Verglichen wird die **vollständige** Verstoßmenge, nicht
nur gültig/ungültig.

```text
$ sweep.py --basislinie      (Rust aus `git show HEAD:broker/src/telemetrie.rs`)
Rust: 6215 Urteile - C++: 6215 Urteile
ABWEICHUNGEN: 143

$ sweep.py --nachweis        (Rust aus dem Arbeitsbaum, mit Strukturriegel)
Rust: 6215 Urteile - C++: 6215 Urteile
ABWEICHUNGEN: 0
```

Die **143** ist dieselbe Zahl, die der Prüfer der Runde 4 gemeldet hat — aus
unabhängig gebautem Werkzeug reproduziert. Ohne diesen Gegenlauf hieße
„0 Abweichungen" auch dann 0, wenn das Messgerät blind wäre.

**Gegenprobe über die ganze Offsetklasse, nicht nur über die Null** (22.08.).
`VerifyOffset` lehnt auf der C++-Seite außerdem Offsets ab, die als `soffset`
negativ sind (`static_cast<SOffsetT>(o) >= 0`) oder aus dem Puffer zeigen. Ob
auch das eine Asymmetrie ist, steht hier nicht als Herleitung, sondern als
Messung — derselbe Korpus, dieselben zwei Beine, nur ein anderer Zellwert:

| Wert je Zelle | was er bedeutet | Abweichungen von 6215 |
|---|---|---:|
| `0x00000000` | Selbstbezug — die Klasse aus BL-A | **0** |
| `0x80000000` | als `soffset` negativ | **0** |
| `0xFFFFFFFF` | zeigt weit hinter den Puffer | **0** |
| `0x00000004` | kleiner, gültig **aussehender** Offset ins Falsche | **0** |

Zusammen **24 860** Puffer durch beide Leser, keine einzige Abweichung. Damit
ist nicht nur der eine Fund geschlossen, sondern die Frage dahinter: bei
Offsetzellen urteilen die beiden Beine gleich.

**Der Zwischenstand, der die Lehre trägt.** Die erste Fassung des Riegels trug
zusätzlich eine vtable-Plausibilitätsprüfung (`num_bytes() >= 4`). Sie schloss
dieselben 143 Lücken — und riss **61 neue auf, in der Gegenrichtung**:

```text
$ sweep.py --nachweis        (erste Riegelfassung)
ABWEICHUNGEN: 61
  alle-validity-bits__00332
    rust ungueltig ['|verifier']
    c++  ungueltig ['/eintraege/0/frame/transport/process_context_present|context_bit_fehlt',
                    '/eintraege/0/frame/transport/sample_rate|sample_rate_bereich',
                    '/eintraege/0/frame/transport/zeitbasis|enum_unbekannt']
```

Offset 332 ist in diesem Puffer der **Tabellenkopf** des `Transportstempel`
(nachgerechnet über den Puffer, nicht geraten), Offset 296 seine `vtable_len`;
beides macht die Tabelle feldlos. `Transportstempel`, `Schleife` und
`AbgeleiteteGrenzen` tragen **kein** `required`-Feld — für C++ ist eine
feldlose Tabelle dort strukturell in Ordnung, er liest lauter Defaults und
meldet deren semantische Folgen.

**Prüfung 7 beim Fallen** (`pruefe_fbs_feldids.py`, läuft im Kanon über A9):

```text
--- (a) Bandwerte.notiz:string — Offsetfeld ohne Riegelzeile ---
  ROT: Strukturriegel kennt Bandwerte::VT_NOTIZ nicht (Bandwerte.notiz: string) —
       ein Offsetfeld ohne Riegelzeile ist die Luecke aus T2-Runde 4.
Pruefungen: 1 rot
EXIT=2

--- (b) QuellenEintrag ohne `required` — die Begruendung faellt ---
  ROT: FeatureBatch.eintraege ist ein Vektor von QuellenEintrag, und QuellenEintrag
       traegt kein `required`-Feld mehr — damit faellt die Begruendung, warum die
       Elementoffsets keinen eigenen Riegel brauchen.
Pruefungen: 1 rot
EXIT=2

--- (c) `fn strukturriegel` ganz ausgebaut ---
  ROT: `fn strukturriegel` steht nicht mehr in …\p7\riegel_weg.rs — ohne ihn kennt
       das Rust-Bein C++' Regel 'May not point to itself' wieder nicht
       (T2-Runde 4, BL-A).
Pruefungen: 1 rot
EXIT=2
```

Die Mutationen sind so gewählt, dass die Prüfungen 1–4 **schweigen** — sonst
wäre nicht zu sehen, welche Prüfung wirklich angeschlagen hat. (c) läuft gegen
eine Kopie der Rust-Quelle; der dritte optionale Pfad dafür ist derselbe
Mechanismus, den der Feld-ID-Riegel schon für Schema und ID-Liste hatte.

Die Prüfung sieht dabei nur den **Rumpf** von `fn strukturriegel`, nicht die
ganze Datei — sonst genügte eine Erwähnung der Konstante in einem Kommentar,
um sie grün zu halten. Ein Riegel, der sich von Prosa beruhigen lässt, ist
keiner; das ist beim Selbstaudit dieser Runde aufgefallen und nachgezogen.

### P15 · Der tote Teilausdruck <a id="p15"></a>

**Befehl:** `b3.py` — beide Fassungen von `utf8Gueltig` aus
`NakamaTelemetrie.cpp` **herausgeschnitten** statt abgetippt (eine Abschrift
prüfte die Abschrift, nicht den Code), erschöpfend gegeneinander gefahren.

```text
geprueft: 4311810305 Bytefolgen (Laenge 0..4)
abweichungen: 0
TOT BESTAETIGT: `&& folge > 0` aendert kein einziges Urteil
```

Länge 0..4 ist für diese Frage vollständig: die längste UTF-8-Folge dieses
Lesers ist 4 Byte, und der Schleifenzustand hängt nur von (Restlänge, Bytes)
ab — damit ist jede Kombination aus Führungsbyte, Folgelänge und
Abschneidepunkt abgedeckt. Verglichen wurde Urteil **und** Codepunktzahl.

---

## 3. T1 — Selbstaudit je Commit

| Punkt | Befund |
|---|---|
| **Numerische Ränder** | NaN und ±inf werden auf der Leitung ABGELEHNT, nicht saniert (Fixtures `kennzahl-nan`, `bandwert-nicht-endlich`); Bandwertgrenzen je Kodierung aus `quantisierung-v1.json`; `sample_rate` 0 und 768001 je ein Negativfixture; `metrics_version` 0. |
| **Gegenpfad** | Erzeugen ↔ Prüfen: `erzeuge_fb_fixtures.py` schreibt, `--pruefen` liest bytegleich zurück. Codegen ↔ Drift-Test in derselben Datei. Beide Riegel sind zusätzlich beim FALLEN vorgeführt (P5, P6). |
| **Behauptungs-Integrität** | Jede in Code und Doku genannte Datei existiert. Ein Verweis auf ein `README-LESER.md`, das ich nie geschrieben hatte, ist beim Selbstaudit aufgefallen und auf die tatsächliche Datei berichtigt worden. |
| **Lügt der UI-Text?** | Keine UI angefasst. |
| **Audiothread** | Kein Produktionsquelltext im Audiopfad angefasst; `pluginval -8` SUCCESS. Die Leser laufen heute nur im Test. |

---

## 4. Befunde aus dieser Session

Drei meiner **handgeschriebenen** Erwartungen waren falsch — und der Korpus hat
sie gefunden, bevor irgendjemand v3 spricht. Genau dafür ist er da:

1. **Instanz-IDs ab 10 waren 64 Zeichen lang.** `str(nr) * 32` ergibt für
   `nr = 10` die Zeichenkette `"1010…"` mit 64 Zeichen — kein `hex32`. Die
   Fixtures `grenze-32-eintraege` und `eintraege-33` waren dadurch aus einem
   ganz anderen Grund ungültig als dem, den sie belegen sollten.
2. **Eine Erwartung widersprach ihrem eigenen Kommentar.** Bei
   `enum-unbekannt-gitter` stand in der Begründung ausdrücklich, die
   Bandzahlprüfung entfalle bei unbekanntem Gitter — und in der Erwartung stand
   sie trotzdem.
3. **Die Bitmap wird gegen die tatsächliche Werteanzahl geprüft**, nicht gegen
   die vom Gitter geforderte. Meine Erwartung für
   `bandzahl-passt-nicht-zum-gitter` verlangte zwei Verstöße; richtig ist
   einer, denn die Bitmap passte zu den 221 gelieferten Werten. Zwei Klagen
   über dieselbe Ursache wären kein schärferer Vertrag, sondern ein lauterer.

Dazu ein Fehlalarm im Kanon, den erst der Zusammenbau sichtbar gemacht hat:
**der JSON-Fixture-Generator beanspruchte `fixtures/v3/` ganz** und meldete das
Unterverzeichnis des Binärkorpus als „verwaiste Datei". Sein Verwaisten-Scan
deckt jetzt nur noch sein eigenes Revier (`gueltig/`, `ungueltig/`, sein
MANIFEST).

---

## 5. Entscheidungen, die der Entwurf offen ließ

| Frage | Entscheidung | Warum |
|---|---|---|
| Pin auf Tag oder Commit? | **Commit** | Der Upstream führt für 25.12.19 zwei Tags — eine Version wird dort nachträglich neu geschnitten. |
| `flatc` als Release-Binary laden oder bauen? | **bauen** | Das vorkompilierte Binary hängt an einem Release-Tag, die Header kämen aus dem Quell-Tarball: zwei Downloads, deren Zusammengehörigkeit nichts belegt außer dem Dateinamen. |
| Version im Payload? | **nein** | Der 16-Byte-Envelope trägt `schema_major`/`schema_minor` bereits (§33.1). Zwei Wahrheiten über dieselbe Zahl in einem Frame widersprechen sich irgendwann. |
| Union oder zwei optionale Vektoren für die Bandwerte? | **zwei Vektoren + explizites `encoding`** | Eine Union machte „genau einer" strukturell, aber die Skalierung muss laut §33.1 ohnehin explizit im Batch stehen — die Union hätte eine Indirektion gekostet und keine Prüfung erspart. |
| Wie entstehen die Binärfixtures? | **mit dem gepinnten `flatc` aus JSON** | Ein eigener Encoder wäre eine dritte Implementierung des Formats, die niemand prüft. |
| Enum ohne oder mit `unbekannt = 0`? | **mit** | FlatBuffers liefert für ein fehlendes Skalarfeld den Default 0. Wäre 0 eine gültige Bedeutung, ließe sich „weggelassen" nicht von „genau das gemeint" unterscheiden. |

---

## 6. T2 — Frischkontext-Prüfer

### 6.1 Runde 3 — gelaufen 21.08., Urteil **NEEDS_WORK**

Ein Prüfer ohne meinen Kontext, gegen `git diff c2ab1da..HEAD` (dieses Ticket
zum ersten Mal, dazu die Nacharbeit der 005a-Runde 2), dieses Manifest und den
Gate-Text aus §65 im Wortlaut.

Seine Hauptzielscheibe war richtig gewählt: **die zwei handgeschriebenen
Binärleser**. Er hat einen systematischen Byte-Mutations-Fuzz über die
gültigen Fixtures gefahren — 600 Mutanten, beide Beine über dieselbe Datei —
und damit gefunden, was der Fixture-Korpus allein nicht finden konnte.

### 6.2 Der schwerste Fund der ganzen Sitzung

**Ein Byte, und die C++-Seite urteilt nicht nur falsch, sondern stirbt.**

`gueltig/mit-schleife.bin`, Offset 689, `0x2D` → `0x80` (ein nacktes
UTF-8-Fortsetzungsbyte in der `logon_sid`):

| Bein | gemessen |
|---|---|
| Rust | `UNGUELTIG { \| verifier}` — jedes Mal, kein Absturz |
| C++ | **`gueltig`** — und ab der zweiten Verarbeitung stirbt der Prozess |

Exitcodes bei n Wiederholungen desselben Puffers in **einem** Prozess:
n=1 → 1 · n=2 → **0xC0000374 (STATUS_HEAP_CORRUPTION)** · n=5 →
**0xC0000409 (STATUS_STACK_BUFFER_OVERRUN)** · n=20/120 → 0xC0000374.
Über den ganzen Fuzz: **78 von 591** Mutanten unterschiedlich klassifiziert,
**9** Abstürze.

**Die Ursache ist eine Annahme, die ich in den Leser hineingeschrieben hatte.**
Dort stand:

> „Der Verifier. Ohne ihn ist jeder Feldzugriff auf einen manipulierten Puffer
> undefiniert — das ist der eine Punkt, an dem FlatBuffers wirklich hart ist,
> und er wird benutzt."

Das ist **auf einer Seite falsch**: der Rust-Verifier prüft String-Inhalte auf
gültiges UTF-8, der C++-Verifier tut das nicht. Die kaputten Bytes gingen
danach an `juce::String::fromUTF8` — ohne Länge, ohne Validierung. Für ein
nacktes Fortsetzungsbyte liefert `getAndAdvance()` Codepunkt 0, während
`isEmpty()` dasselbe Byte als „nicht Ende" liest; Längen- und Kopierdurchlauf
laufen verschieden weit.

> **Der Vertrag war darauf gebaut, dass sich DIESELBE Bibliothek in zwei
> Sprachen gleich verhält.** Das ist die Schwesterlehre zu der aus
> `SONDE-005a` („ein Riegel darf nie die Bibliothek befragen, gegen deren
> Verhalten er schützt") — eine Ebene höher und teurer, weil dieser Leser ab
> `SONDE-010` in einem VST3 im Hostprozess läuft.

### 6.3 Alle Befunde und ihr Ausgang

| # | Befund | Ausgang |
|---|---|---|
| BL-1 | Ein Byte: C++ sagt „gültig", Rust „ungültig", und der C++-Prozess stirbt an Heap-Korruption | **bestätigt · gefixt** — die C++-Seite prüft UTF-8 jetzt selbst, über die **echte Länge**, mit `str::from_utf8`-Semantik. Der Puffer liegt als `ungueltig/sid-ungueltiges-utf8.bin` dauerhaft im Korpus, dazu ein Riegel, der ihn **200×** in einem Prozess fährt — ein Korpuslauf sieht jedes Fixture nur einmal und hätte das nie bemerkt |
| BL-2 | 78 von 591 Byte-Mutanten unterschiedlich klassifiziert | **bestätigt · dieselbe Wurzel, mit BL-1 gefixt** |
| BL-3 | Eingebettete NUL: C++ las `c_str()`, Rust die volle Länge — Urteile in BEIDE Richtungen falsch | **bestätigt · gefixt** — alle Stringzugriffe längenbasiert. Zwei neue Negativfixtures (`hex32-nul-dahinter`, `sid-nul-dahinter`) und ein **gültiges** (`sid-ist-ein-nul`), das die Übereinstimmung beweist statt sie anzunehmen |
| BL-4 | Puffer unter 8 Byte: C++ meldet sauber, **Rust panickt** im flatbuffers-Crate | **bestätigt · gefixt** — derselbe Längenriegel auf beiden Seiten; drei Fixtures (0, 4, 7 Byte). Ein Panic im Broker beendet den Thread, der die Pipe bedient |
| 1 | `context_bit_fehlt` fehlte in der „geschlossenen" Regelliste des README | **bestätigt · gefixt** |
| 2 | Das Manifest widersprach seinen eigenen Rohausgaben (Behauptungstabelle nachgezogen, Rohblöcke nicht) | **bestätigt · gefixt** — §1 und §2 stammen jetzt aus EINEM Lauf, und §1 sagt das ausdrücklich |
| 3 | Das Referenzbein maß die Substanz am Metadatenfeld `anzahl` statt an der Liste | **bestätigt · gefixt** — plus eine Probe, dass beide übereinstimmen |
| 4 | Der Rust-Pin las die **Anforderung** aus `Cargo.toml`, nie die aufgelöste Version | **bestätigt · gefixt** — `"25.12.19"` heißt in Cargo `^25.12.19`; nach einem `cargo update` wäre der Riegel grün geblieben. Jetzt wird `Cargo.lock` gelesen, vorgeführt an einer manipulierten Auflösung |
| 5 | `pruefe_fbs_feldids.py` kannte keine Blockkommentare | **bestätigt · gefixt** — der Irrtum ging in die sichere Richtung, widerlegte aber den Anspruch des Parsers. Ein Riegel, der aus dem falschen Grund rot wird, ist beim nächsten Mal aus dem falschen Grund grün |
| 6 | `include` umging den Feld-ID-Riegel | **bestätigt · gefixt** — heute folgenlos, weil `flatc --cpp` für die eingebundene Datei keinen Code erzeugt; mit `--gen-all` wäre es ein echtes Loch |
| 7 | `--conform`-Anker könnte mehrfach passen | **bestätigt · gefixt** — Längenriegel gegen Mehrfachtreffer |
| 8 | `bandwert_bereich` verwies auf `quantisierung-v1.json`, die Grenzen standen aber nur im Quelltext — und `bereich_db` bedeutet den **Trägerumfang** (±32767) | **bestätigt · an der Wurzel gefixt** — der Vertrag trägt jetzt `plausibler_bereich_db`, und **beide** Leser vergleichen ihre einkompilierten Konstanten bei jedem Lauf damit. Aus der Quellenangabe ist eine Prüfung geworden |
| 9 | Der Textriegel hat keinen produktiven Aufrufer | **eingeordnet** — richtig und Absicht; steht jetzt als eigene Zeile in der Ausschlusstabelle mit Eigentümerticket `SONDE-010` |
| 10 | Manipulierter Steckbrief meldete 3 („Voraussetzung fehlt") statt 2 | **bestätigt · gefixt** — das Werkzeug ist da, was es meldet widerspricht dem Pin: das ist eine widerlegte Behauptung |
| H-1 | Mechanismus der Heap-Korruption (nicht im Debugger nachgestellt) | **zur Kenntnis** — der Fix umgeht `fromUTF8` auf unvalidierten Bytes vollständig, der genaue Mechanismus ist damit gegenstandslos |
| H-2 | `int`-Überlauf in `zahlPruefen` ab ~2,1 Mrd. Vorkommastellen | **zur Kenntnis** — nicht baubar (≈2 GB Literal); `juce::String` scheiterte vorher |

Zwei Dinge, die der Prüfer ausdrücklich **nicht** brechen konnte und die
deshalb hier stehen: der Zahlenriegel des Textriegels hielt über **1504**
Literale in allen drei Beinen ohne eine einzige Abweichung, und die gemeinsame
Falltabelle wird wirklich von allen drei Beinen gelesen — keine Kopie mehr im
Quelltext.

### 6.4 Was Runde 3 gekostet und gebracht hat

Alle Zahlen in diesem Lauf gemessen:

| | vor Runde 3 | nach Runde 3 |
|---|---:|---:|
| Binärfixtures | 40 | **50** |
| C++-Prüfungen | 53 | **56** |
| Rust-Integrationstests | 5 | **6** |
| Referenzbein-Prüfungen | 176 | **177** |
| Riegel im Drift-Prüfer | 4 | **5** (Cargo.lock kam dazu) |

### 6.5 Runde 4 — gelaufen 21.08., Urteil **NEEDS_WORK**; Befunde geschlossen

> **Was hier steht und was nicht.** Geschlossen sind die drei **Befunde**, je
> mit Messung und Rohausgabe. Das **Urteil** der Runde bleibt NEEDS_WORK: ein
> T2-Urteil vergibt ein Prüfer mit frischem Kontext, nicht der, der die
> Nacharbeit geschrieben hat. Eine Runde 5 gegen `git diff 4cf05b9..HEAD` ist
> der nächste Schritt dieses Verfahrens — sie ist nicht gelaufen.

Runde 4 hat die Zielscheibe noch einmal verschoben: nicht mehr der INHALT der
Puffer, sondern ihre **Struktur**. Der Prüfer hat systematisch Offsetzellen auf
0 gesetzt — die Mutation, die aus einem Zeiger einen Selbstbezug macht — und
gefunden, dass diesmal die **Rust**-Seite die schwächere ist.

| Bein (beide 25.12.19, derselbe Commit) | Regel |
|---|---|
| C++ `verifier.h:265-269`, `VerifyOffset` | `// May not point to itself.` → `if (!Check(o != 0)) return 0;` |
| Rust `verifier.rs:515-522`, `ForwardsUOffset::run_verifier` | `let next_pos = offset.saturating_add(pos);` — **kein `o != 0`** |

Rust folgt dem Selbstbezug, liest eine vtable der Länge 0 und damit eine
Tabelle OHNE JEDES FELD — lauter Defaults, die semantisch unauffällig sind.
Die Richtung ist die gefährliche: der Broker sitzt ZWISCHEN Sonde und Main. Er
hätte den Batch als gültig durchgereicht, und das Plugin hätte ihn danach
verworfen — die Validierung wäre keine **eine** Wahrheit mehr.
`VerifierOptions` kennt dafür keine Option; es gibt keinen Konfigurationsausweg.

#### Was offen war und was diese Runde daraus gemacht hat

Der Fix lag im Baum und war an **einem** Fixture belegt
(`ungueltig/offset-selbstbezug.bin`). Ein Fixture belegt einen Fall. Gemessen
ist jetzt die Klasse — Korpus, Werkzeug und Rohausgaben in [↑ P14](#p14):

| Stand | von 6215 Mutanten laufen auseinander |
|---|---:|
| ohne Strukturriegel (`git show HEAD:…/telemetrie.rs`) | **143** |
| erste Riegelfassung, mit `vtable_plausibel` | **61** — in der **Gegenrichtung** |
| Fassung im Baum: `offset_nicht_null` über alle 15 Offsetfelder | **0** |

> 🔑 **Ein Riegel, der STRENGER ist als das Bein, das er spiegelt, bricht
> denselben Vertrag wie einer, der schwächer ist.** Verlangt ist dieselbe
> Verstoßmenge, nicht dasselbe Vorzeichen des Urteils. Das ist die
> Schwesterlehre zu der aus Runde 3 („der Vertrag war darauf gebaut, dass sich
> dieselbe Bibliothek in zwei Sprachen gleich verhält") — und sie wäre an einem
> einzelnen Fixture **nicht** sichtbar geworden: das Fixture blieb die ganze
> Zeit grün, während 61 andere Puffer auseinanderliefen.

Die Vektorelemente von `eintraege` brauchen keine eigene Riegelzeile, weil
`QuellenEintrag` zwei `required`-Felder trägt und eine feldlose Tabelle deshalb
schon in BEIDEN Verifiern fällt. Das ist keine Herleitung, sondern Teil
derselben Messung — und weil eine Begründung still wegfallen kann, prüft
`pruefe_fbs_feldids.py` (Prüfung 7) sie mit.

#### Alle Befunde und ihr Ausgang

| # | Befund | Ausgang |
|---|---|---|
| BL-A | Rusts Verifier kennt „May not point to itself" nicht; 143 von 6215 Mutanten laufen auseinander | **bestätigt · gefixt · an der KLASSE belegt** — `strukturriegel` in `broker/src/telemetrie.rs` über alle 15 Offsetfelder; 0 von 6215. Dauerfixture `ungueltig/offset-selbstbezug.bin` (strukturell gesuchte Zelle, kein gezählter Offset). Dass der handgeschriebene Riegel beim nächsten Feld nicht still danebenläuft, hält **Prüfung 7** — beim Fallen vorgeführt |
| B-1 | Die **196** in P10 war stale; gemessen waren 206 | **bestätigt · gefixt** — nicht die Ziffer berichtigt, sondern nachgemessen: **207 bytegleich, 0 abweichend**, mit Gegenprobe gegen `git ls-files` (207). Das 207. ist das Fixture dieser Runde |
| B-3 | Toter Teilausdruck `&& folge > 0` in `utf8Gueltig` | **bestätigt · gefixt** — die Schleife läuft nur mit `i < laenge`, also ist `i + 0 >= laenge` nie wahr. Erschöpfend nachgemessen über **4.311.810.305** Bytefolgen der Länge 0..4, beide Fassungen aus der echten Quelle geschnitten: 0 Abweichungen in Urteil und Codepunktzahl ([↑ P15](#p15)) |

#### Selbstaudit dieser Runde

| Punkt | Befund |
|---|---|
| **Numerische Ränder** | Der Korpus lässt Offset 0 (Wurzeloffset) bewusst aus und sagt das; `offset_nicht_null` prüft `pos + 4 > buf.len()` vor dem Lesen, obwohl der Verifier das schon garantiert — der Riegel darf sich nicht auf die Seite verlassen, die er ergänzt. Der Denominator 6215 ist je Fixture gegen Größe/4 − 1 gegengerechnet, nicht übernommen |
| **Behauptungs-Integrität** | Beide Verifier-Zitate wörtlich gegen die Quellen geprüft (`_deps/flatbuffers-src/include/flatbuffers/verifier.h`, `~/.cargo/registry/…/flatbuffers-25.12.19/src/verifier.rs`); die Offsets 296/332 sind durch den Puffer **gelaufen**, nicht geraten; jede Zahl in §1 und §2 stammt aus dem Kanon-Lauf dieser Runde |
| **Gegenpfad** | Riegel ↔ Fixture ↔ Prüfung 7: jede der drei Hälften ist einmal beim Fallen vorgeführt (P5, P14 a/b) |
| **Audiothread** | Kein Produktionsquelltext im Audiopfad angefasst. Der Riegel läuft im Broker und im Test, nicht im `processBlock` |
| **Lügt der UI-Text?** | Keine UI angefasst |

---

## 7. Schließungsvorbehalt (Entwurf §65)

> „`SONDE-005` darf parallel zu den Hostmessungen **vorbereitet**, aber erst
> nach deren Capabilityentscheidung **geschlossen** werden."

Der binäre Vertrag hängt an keiner Hostmessung: seine Form folgt aus §33.1 und
§33.2, nicht aus dem, was FL antwortet. Was S4 liefern kann, sind Kadenzen
(10 Hz, 1–4 Hz) — und die sind hier bewusst **nicht** kodiert, sondern nur ihre
Obergrenze 32 aus §53.9. Der Vorbehalt bleibt trotzdem stehen, weil §65 das
Ticket als Ganzes meint; die JSON-Hälfte trägt ihn in
`docs/beweise/SONDE-005a.md` §7.

---

## Kanon-Lauf - SONDE-005b - Abnahmelauf

**Lauf:** 2026-08-21 17:28 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 14/14 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 17:28:23 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | e4a1c95 SONDE-005b: die beiden Binaerleser + Fixture-Korpus, Kanon auf 14 |
| Commit (voll) | e4a1c95ce74259331cccd78a216bfabf6d63e2fb |
| Arbeitsbaum | 3 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M broker/src/telemetrie.rs
?? docs/beweise/SONDE-005b.md
?? eq-copilot/schemas/v3/flatbuffers/README.md
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-18 11:46:33 | `A6F71DAE47C7BE10` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-18 11:47:19 | `3456136DDD939B7F` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-20 21:28:44 | `AFEC07D04C9A80A6` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-21 17:21:09 | `0AAEC7C023F4F8DC` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 17:20:59**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,36 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 5,61 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,45 s | [↓ A4](#a4) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 0,58 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,10 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,22 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,59 s | [↓ A10](#a10) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,14 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,36 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 5,61 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,45 s

stdout:

```text

running 53 tests
test framing::tests::hin_und_zurueck ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
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
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_ltas_array_wird_gezaehlt_verworfen ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::objektwertiges_const_vergleicht_wie_die_cpp_seite ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::textriegel_deckt_jede_gemessene_kante ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 53 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 4 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 4 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.08s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.01s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-1ae3914fdef4c778.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-d65c73b91b696f68.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-d2b910788f79aaeb.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-9c30b17f91172beb.exe)
   Doc-tests eqcop_broker
```

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 0,58 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
47 Definitionen, 17 Nachrichtenfamilien, 152 Fixtures
Pruefungen: 174 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 38  
  const                     4 / 4    HART
  enum                     14 / 14   HART
  exclusiveMinimum          2 / 2   
  items                     1 / 4   
  maxItems                  3 / 4   
  maxLength                 2 / 15  
  maxProperties             2 / 3   
  maximum                   4 / 14  
  minItems                  2 / 4   
  minLength                 4 / 9   
  minimum                   6 / 40  
  oneOf                     4 / 4   
  pattern                   1 / 1   
  required                 37 / 168 
  type                      8 / 133 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:544: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,10 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=a1aec4aaad39487aba044d0c46575ad6015ce896431ef3a5565efe132c4b45c1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
36 gueltige, 116 ungueltige Fixtures
  bytegleich: 153 Dateien, MANIFEST sha256=28e629a12d39d32297d8a774dd40cf1a850f29589579ae383ad14f531d0d16c1
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,22 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1171 Zeilen  sha256=a3454de8187a9cdc...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2000 Zeilen  sha256=226189c4fc77afba...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,59 s

stdout:

```text
8 gueltige, 31 ungueltige Fixtures
  bytegleich: 40 Dateien, MANIFEST sha256=0c9826b12da3bab488c063101f45a866d412db21795d46bbcf97bebb855200e7
```

stderr:

_(leer)_

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 5:28:28pm vs 21 Aug 2026 5:16:01pm]
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

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260821-172846.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (30090 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt jede gemessene Kante  [31 Faelle]
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (39 Fixtures)
[ok]   Binaerkorpus hat Substanz  [39 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [nicht endliche Zahl: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (152 Fixtures)
[ok]   Korpus hat Substanz  [152 Fixtures]
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
52 bestanden, 0 gescheitert
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 5,16 s

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
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
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


---

## Kanon-Lauf - SONDE-005b - Abnahmelauf nach T2-Runde 2

**Lauf:** 2026-08-21 17:43 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 14/14 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 17:43:58 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | e4a1c95 SONDE-005b: die beiden Binaerleser + Fixture-Korpus, Kanon auf 14 |
| Commit (voll) | e4a1c95ce74259331cccd78a216bfabf6d63e2fb |
| Arbeitsbaum | 100 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M CLAUDE.md
 M broker/src/generiert/nakama_telemetry_v1_generated.rs
 M broker/src/telemetrie.rs
 M broker/src/vertrag.rs
 M broker/tests/contract_cross_language.rs
 M docs/beweise/SONDE-005a.md
 M eq-copilot/fixtures/v3/MANIFEST.json
 M eq-copilot/fixtures/v3/flatbuffers/MANIFEST.json
 M eq-copilot/fixtures/v3/flatbuffers/gueltig/alle-validity-bits.bin
 M eq-copilot/fixtures/v3/flatbuffers/gueltig/broker-buendel-5.bin
 M eq-copilot/fixtures/v3/flatbuffers/gueltig/evidenz-221-band.bin
 M eq-copilot/fixtures/v3/flatbuffers/gueltig/float32-kreuzstatistik.bin
 M eq-copilot/fixtures/v3/flatbuffers/gueltig/grenze-32-eintraege.bin
 M eq-copilot/fixtures/v3/flatbuffers/gueltig/live-64-band.bin
 M eq-copilot/fixtures/v3/flatbuffers/gueltig/mit-schleife.bin
 M eq-copilot/fixtures/v3/flatbuffers/gueltig/ohne-optionale-kennzahlen.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/abgeschnitten.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/bandwert-ausser-bereich.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/bandwert-nicht-endlich.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/bandzahl-passt-nicht-zum-gitter.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/bitmap-fuellbits.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/bitmap-zu-kurz.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/bitmap-zu-lang.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/breite-negativ.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/dateikennung-falsch.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/eintraege-33.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/encoding-passt-nicht-zu-f32.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/encoding-passt-nicht-zu-i16.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/enum-unbekannt-encoding.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/enum-unbekannt-gitter.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/enum-unbekannt-zeitbasis.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/grenzen-verdreht.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/herleitung-unbekannt.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/hex32-grossbuchstabe.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/kein-traeger.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/kennzahl-nan.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/korrelation-ueber-1.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/metrics-version-null.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/quelle-doppelt.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/sample-count-zu-gross.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/sample-rate-null.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/sample-rate-zu-hoch.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/sid-leer.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/validity-unbekanntes-bit.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/wurzeloffset-kaputt.bin
 M eq-copilot/fixtures/v3/flatbuffers/ungueltig/zwei-traeger.bin
 M eq-copilot/fixtures/v3/gueltig/evidence-abdeckung-raender.json
 M eq-copilot/fixtures/v3/gueltig/evidence-cycle-nur-ppq.json
 M eq-copilot/fixtures/v3/gueltig/evidence-cycle-validiert.json
 M eq-copilot/fixtures/v3/gueltig/evidence-grobes-gitter.json
 M eq-copilot/fixtures/v3/gueltig/evidence-konfidenz-additiv.json
 M eq-copilot/fixtures/v3/gueltig/evidence-ohne-projektzeit.json
 M eq-copilot/fixtures/v3/gueltig/evidence-verteilung-additiv.json
 M eq-copilot/fixtures/v3/gueltig/evidence-volle-hostzeit.json
 M eq-copilot/fixtures/v3/gueltig/evidence_snapshot.json
 M eq-copilot/fixtures/v3/ungueltig/abdeckung-negativ.json
 M eq-copilot/fixtures/v3/ungueltig/abdeckung-ueber-1.json
 M eq-copilot/fixtures/v3/ungueltig/baender-leer.json
 M eq-copilot/fixtures/v3/ungueltig/baender-ohne-bitmap.json
 M eq-copilot/fixtures/v3/ungueltig/baender-ohne-encoding.json
 M eq-copilot/fixtures/v3/ungueltig/baender-zu-viele.json
 M eq-copilot/fixtures/v3/ungueltig/bandwert-als-string.json
 M eq-copilot/fixtures/v3/ungueltig/bandwerte-saturated-als-string.json
 M eq-copilot/fixtures/v3/ungueltig/bandzahl-passt-nicht-zum-gitter.json
 M eq-copilot/fixtures/v3/ungueltig/bitmap-laenge-passt-nicht.json
 M eq-copilot/fixtures/v3/ungueltig/continuity-segment-negativ.json
 M eq-copilot/fixtures/v3/ungueltig/cycle-ohne-active.json
 M eq-copilot/fixtures/v3/ungueltig/derivation-erfunden.json
 M eq-copilot/fixtures/v3/ungueltig/derived-bounds-ohne-ende.json
 M eq-copilot/fixtures/v3/ungueltig/encoding-erfunden.json
 M eq-copilot/fixtures/v3/ungueltig/evidence-ohne-verteilung.json
 M eq-copilot/fixtures/v3/ungueltig/gitter-erfunden.json
 M eq-copilot/fixtures/v3/ungueltig/konfidenz-klasse-erfunden.json
 M eq-copilot/fixtures/v3/ungueltig/konfidenz-ueber-maxproperties.json
 M eq-copilot/fixtures/v3/ungueltig/playing-als-zahl.json
 M eq-copilot/fixtures/v3/ungueltig/samplerate-negativ.json
 M eq-copilot/fixtures/v3/ungueltig/time-basis-erfunden.json
 M eq-copilot/fixtures/v3/ungueltig/transport-ohne-validity.json
 M eq-copilot/fixtures/v3/ungueltig/validity-ohne-continuous-time.json
 M eq-copilot/fixtures/v3/ungueltig/validity-ohne-latenzbit.json
 M eq-copilot/fixtures/v3/ungueltig/validity-ohne-record-state.json
 M eq-copilot/fixtures/v3/ungueltig/validity-unbekanntes-bit.json
 M eq-copilot/fixtures/v3/ungueltig/zahl-nicht-endlich.json
 M eq-copilot/plugin/tests/SchemaTestMain.cpp
 M eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp
 M eq-copilot/plugin/vertrag/NakamaVertrag.cpp
 M eq-copilot/plugin/vertrag/NakamaVertrag.h
 M eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h
 M eq-copilot/schemas/v3/README.md
 M eq-copilot/schemas/v3/eq-ipc-v3.schema.json
 M eq-copilot/schemas/v3/flatbuffers/FELD-IDS.json
 M eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs
 M tools/eq-copilot/erzeuge_fb_fixtures.py
 M tools/eq-copilot/erzeuge_v3_fixtures.py
 M tools/eq-copilot/pruefe_v3_vertrag.py
?? docs/beweise/SONDE-005b.md
?? eq-copilot/fixtures/v3/TEXTRIEGEL-FAELLE.json
?? eq-copilot/fixtures/v3/flatbuffers/ungueltig/transport-ohne-context-bit.bin
?? eq-copilot/fixtures/v3/ungueltig/transport-ohne-context-bit.json
?? eq-copilot/schemas/v3/flatbuffers/README.md
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-18 11:46:33 | `A6F71DAE47C7BE10` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-18 11:47:19 | `3456136DDD939B7F` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-20 21:28:44 | `AFEC07D04C9A80A6` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-21 17:44:04 | `AFAA54F520CC581E` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 17:40:46**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,05 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,30 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 5,63 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,49 s | [↓ A4](#a4) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 0,60 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,23 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,60 s | [↓ A10](#a10) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,15 s | [↓ B3c](#b3c) |
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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,30 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 5,63 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,49 s

stdout:

```text

running 51 tests
test framing::tests::hin_und_zurueck ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
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
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_ltas_array_wird_gezaehlt_verworfen ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 51 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 5 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 5 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.09s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-1ae3914fdef4c778.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-d65c73b91b696f68.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-d2b910788f79aaeb.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-9c30b17f91172beb.exe)
   Doc-tests eqcop_broker
```

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 0,60 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
47 Definitionen, 17 Nachrichtenfamilien, 153 Fixtures
Pruefungen: 176 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 38  
  const                     4 / 4    HART
  enum                     14 / 14   HART
  exclusiveMinimum          2 / 2   
  items                     1 / 4   
  maxItems                  3 / 4   
  maxLength                 2 / 15  
  maxProperties             2 / 3   
  maximum                   4 / 14  
  minItems                  2 / 4   
  minLength                 4 / 9   
  minimum                   6 / 40  
  oneOf                     4 / 4   
  pattern                   1 / 1   
  required                 38 / 169 
  type                      8 / 134 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:604: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=a1aec4aaad39487aba044d0c46575ad6015ce896431ef3a5565efe132c4b45c1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
36 gueltige, 117 ungueltige Fixtures
  bytegleich: 155 Dateien, MANIFEST sha256=bc72c521906af48592413ffa28064bdeb8feddee2955ec50093fbdda4a421b2c
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,60 s

stdout:

```text
8 gueltige, 32 ungueltige Fixtures
  bytegleich: 41 Dateien, MANIFEST sha256=7f94d8a04c93871748e254b0d45ce316b9302d605507f3152c5dce444b07e51b
```

stderr:

_(leer)_

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 5:44:05pm vs 21 Aug 2026 5:16:01pm]
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

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260821-174423.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (32297 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [59 Faelle]
[ok]   Falltabelle hat Substanz  [59 Faelle]
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (40 Fixtures)
[ok]   Binaerkorpus hat Substanz  [40 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (153 Fixtures)
[ok]   Korpus hat Substanz  [153 Fixtures]
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
53 bestanden, 0 gescheitert
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 6,77 s

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
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  SchemaTestMain.cpp
  NakamaVertrag.cpp
C:\Users\phili\Projekte\Nakama\eq-copilot\plugin\tests\SchemaTestMain.cpp(527,22): warning C4459: Deklaration von "fehler" blendet globale Deklaration aus [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest.vcxproj]
      C:\Users\phili\Projekte\Nakama\eq-copilot\plugin\tests\SchemaTestMain.cpp(28,5):
      Siehe Deklaration von "`anonymous-namespace'::fehler"
  
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


---

## Kanon-Lauf - SONDE-005b - Abnahmelauf nach T2-Runde 3

**Lauf:** 2026-08-21 18:31 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 14/14 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 18:31:09 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | e7e10e6 chore: GATES.md bleibt Sitzungsgeruest (lokal ausgeschlossen) |
| Commit (voll) | e7e10e6f9c2d60df521c9f67a098d249b6666815 |
| Arbeitsbaum | 21 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M broker/src/telemetrie.rs
 M broker/tests/contract_cross_language.rs
 M docs/beweise/SONDE-005b.md
 M eq-copilot/fixtures/v3/flatbuffers/MANIFEST.json
 M eq-copilot/plugin/tests/SchemaTestMain.cpp
 M eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp
 M eq-copilot/plugin/vertrag/NakamaTelemetrie.h
 M eq-copilot/schemas/v3/flatbuffers/README.md
 M eq-copilot/schemas/v3/quantisierung-v1.json
 M tools/eq-copilot/erzeuge_fb_fixtures.py
 M tools/eq-copilot/erzeuge_quantisierung.py
 M tools/eq-copilot/pruefe_fbs_feldids.py
 M tools/eq-copilot/pruefe_flatc_drift.py
 M tools/eq-copilot/pruefe_v3_vertrag.py
?? eq-copilot/fixtures/v3/flatbuffers/gueltig/sid-ist-ein-nul.bin
?? eq-copilot/fixtures/v3/flatbuffers/ungueltig/hex32-nul-dahinter.bin
?? eq-copilot/fixtures/v3/flatbuffers/ungueltig/puffer-leer.bin
?? eq-copilot/fixtures/v3/flatbuffers/ungueltig/puffer-sieben.bin
?? eq-copilot/fixtures/v3/flatbuffers/ungueltig/puffer-vier.bin
?? eq-copilot/fixtures/v3/flatbuffers/ungueltig/sid-nul-dahinter.bin
?? eq-copilot/fixtures/v3/flatbuffers/ungueltig/sid-ungueltiges-utf8.bin
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-18 11:46:33 | `A6F71DAE47C7BE10` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-18 11:47:19 | `3456136DDD939B7F` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-20 21:28:44 | `AFEC07D04C9A80A6` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-21 18:27:10 | `294B13711A8B7046` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 18:26:58**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,49 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 20,15 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 1,86 s | [↓ A4](#a4) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 0,67 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,12 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,26 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,67 s | [↓ A10](#a10) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,09 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,14 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,49 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 20,15 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 1,86 s

stdout:

```text

running 51 tests
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::hin_und_zurueck ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
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
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_ltas_array_wird_gezaehlt_verworfen ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 51 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 6 tests
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test quantisierung_stimmt_mit_den_testvektoren ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 6 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.09s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 1.15s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-1ae3914fdef4c778.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-d65c73b91b696f68.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-d2b910788f79aaeb.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-9c30b17f91172beb.exe)
   Doc-tests eqcop_broker
```

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 0,67 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
47 Definitionen, 17 Nachrichtenfamilien, 153 Fixtures
Pruefungen: 177 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 38  
  const                     4 / 4    HART
  enum                     14 / 14   HART
  exclusiveMinimum          2 / 2   
  items                     1 / 4   
  maxItems                  3 / 4   
  maxLength                 2 / 15  
  maxProperties             2 / 3   
  maximum                   4 / 14  
  minItems                  2 / 4   
  minLength                 4 / 9   
  minimum                   6 / 40  
  oneOf                     4 / 4   
  pattern                   1 / 1   
  required                 38 / 169 
  type                      8 / 134 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:610: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,12 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
36 gueltige, 117 ungueltige Fixtures
  bytegleich: 155 Dateien, MANIFEST sha256=bc72c521906af48592413ffa28064bdeb8feddee2955ec50093fbdda4a421b2c
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,26 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,67 s

stdout:

```text
9 gueltige, 38 ungueltige Fixtures
  bytegleich: 48 Dateien, MANIFEST sha256=2a94f144327ee02f309e5d363ef96acd9f22f24950f2123dc460fc99398915df
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 6:31:15pm vs 21 Aug 2026 5:16:01pm]
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

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,09 s

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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260821-183149.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (23608 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [59 Faelle]
[ok]   Falltabelle hat Substanz  [59 Faelle]
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (47 Fixtures)
[ok]   Binaerkorpus hat Substanz  [47 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (153 Fixtures)
[ok]   Korpus hat Substanz  [153 Fixtures]
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
56 bestanden, 0 gescheitert
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 5,32 s

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
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
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


---

## Kanon-Lauf - Abnahmelauf im vereinten Stand (mit dem A11-Bein der Kontext-Inventur)

**Lauf:** 2026-08-21 21:59 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 15/15 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 21:59:48 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 4f7182b SONDE-005b T2-Runde 3: der Verifier ist in beiden Sprachen NICHT gleich stark |
| Commit (voll) | 4f7182b4c8ea14d0ac3d59b2d4582555c3640737 |
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
| `EqCopNullTest` | 2026-08-21 21:10:27 | `C6A69FDEDBAE248A` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-21 21:10:32 | `AD3233A03C16F161` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-21 21:10:37 | `05A6C84BF99E686B` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-21 18:27:10 | `294B13711A8B7046` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 20:35:22**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,26 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 20,17 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,45 s | [↓ A4](#a4) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 0,64 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,22 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,67 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,24 s | [↓ A11](#a11) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,13 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,26 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 20,17 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,45 s

stdout:

```text

running 51 tests
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
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
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 51 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 6 tests
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test quantisierung_stimmt_mit_den_testvektoren ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 6 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.09s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-1ae3914fdef4c778.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-d65c73b91b696f68.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-d2b910788f79aaeb.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-9c30b17f91172beb.exe)
   Doc-tests eqcop_broker
```

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 0,64 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
47 Definitionen, 17 Nachrichtenfamilien, 153 Fixtures
Pruefungen: 177 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 38  
  const                     4 / 4    HART
  enum                     14 / 14   HART
  exclusiveMinimum          2 / 2   
  items                     1 / 4   
  maxItems                  3 / 4   
  maxLength                 2 / 15  
  maxProperties             2 / 3   
  maximum                   4 / 14  
  minItems                  2 / 4   
  minLength                 4 / 9   
  minimum                   6 / 40  
  oneOf                     4 / 4   
  pattern                   1 / 1   
  required                 38 / 169 
  type                      8 / 134 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:610: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
36 gueltige, 117 ungueltige Fixtures
  bytegleich: 155 Dateien, MANIFEST sha256=bc72c521906af48592413ffa28064bdeb8feddee2955ec50093fbdda4a421b2c
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,22 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,67 s

stdout:

```text
9 gueltige, 38 ungueltige Fixtures
  bytegleich: 48 Dateien, MANIFEST sha256=2a94f144327ee02f309e5d363ef96acd9f22f24950f2123dc460fc99398915df
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,24 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 9:59:54pm vs 21 Aug 2026 5:16:01pm]
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

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260821-220026.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (24578 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [59 Faelle]
[ok]   Falltabelle hat Substanz  [59 Faelle]
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (47 Fixtures)
[ok]   Binaerkorpus hat Substanz  [47 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (153 Fixtures)
[ok]   Korpus hat Substanz  [153 Fixtures]
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
56 bestanden, 0 gescheitert
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 5,27 s

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
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
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


---

## Kanon-Lauf - SONDE-005b T2-Runde 4

**Lauf:** 2026-08-21 23:50 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 15/15 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-21 23:50:36 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 68cdbb8 SONDE-005b T2-Runde 4, B-3: toter Teilausdruck in utf8Gueltig |
| Commit (voll) | 68cdbb822ac91d1925190c0acf03b49b15442c28 |
| Arbeitsbaum | 4 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
D docs/handoffs/auto-handoff-2026-08-18-2242-7c7cee38.md
 D docs/handoffs/auto-handoff-2026-08-19-1032-9338b4bb.md
?? docs/handoffs/auto-handoff-2026-08-21-2327-4ca90ff5.md
?? docs/handoffs/auto-handoff-2026-08-21-2333-60c295b3.md
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-21 21:10:27 | `C6A69FDEDBAE248A` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-21 21:10:32 | `AD3233A03C16F161` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-21 21:10:37 | `05A6C84BF99E686B` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-21 23:46:42 | `F68284E36B529CEB` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 23:46:31**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,07 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,88 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,15 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,59 s | [↓ A4](#a4) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 0,66 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,12 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,16 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,25 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,98 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,24 s | [↓ A11](#a11) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,10 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,16 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,88 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,15 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,59 s

stdout:

```text

running 51 tests
test framing::tests::abbruch_mitten_im_frame ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
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
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_ltas_array_wird_gezaehlt_verworfen ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 51 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.13s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 6 tests
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test quantisierung_stimmt_mit_den_testvektoren ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 6 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-1ae3914fdef4c778.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-d65c73b91b696f68.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-d2b910788f79aaeb.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-9c30b17f91172beb.exe)
   Doc-tests eqcop_broker
```

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 0,66 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
47 Definitionen, 17 Nachrichtenfamilien, 153 Fixtures
Pruefungen: 177 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 38  
  const                     4 / 4    HART
  enum                     14 / 14   HART
  exclusiveMinimum          2 / 2   
  items                     1 / 4   
  maxItems                  3 / 4   
  maxLength                 2 / 15  
  maxProperties             2 / 3   
  maximum                   4 / 14  
  minItems                  2 / 4   
  minLength                 4 / 9   
  minimum                   6 / 40  
  oneOf                     4 / 4   
  pattern                   1 / 1   
  required                 38 / 169 
  type                      8 / 134 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:610: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,12 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,16 s

stdout:

```text
36 gueltige, 117 ungueltige Fixtures
  bytegleich: 155 Dateien, MANIFEST sha256=bc72c521906af48592413ffa28064bdeb8feddee2955ec50093fbdda4a421b2c
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,98 s

stdout:

```text
9 gueltige, 42 ungueltige Fixtures
  bytegleich: 52 Dateien, MANIFEST sha256=15bcde347cfa559c0b2992c5f9f5f6e7583f64e7b4ba33347b4c46f808942a35
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,24 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,10 s

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [21 Aug 2026 11:50:41pm vs 21 Aug 2026 5:16:01pm]
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

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260821-235101.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (40366 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,16 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [59 Faelle]
[ok]   Falltabelle hat Substanz  [59 Faelle]
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (51 Fixtures)
[ok]   Binaerkorpus hat Substanz  [51 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (153 Fixtures)
[ok]   Korpus hat Substanz  [153 Fixtures]
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
56 bestanden, 0 gescheitert
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 5,55 s

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
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
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


---

## Kanon-Lauf - SONDE-005b T2-Runde 4 - Abnahmelauf

**Lauf:** 2026-08-22 00:02 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 15/15 Kanon-Laeufe bestanden | 5 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-22 00:02:50 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 90f09c9 S6-Manifest: T2-Runde 4 geschlossen, Abnahmelauf 15/15, P10 nachgemessen |
| Commit (voll) | 90f09c95b327858f7147fa54ba2e750277611d5f |
| Arbeitsbaum | 6 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M docs/beweise/SONDE-005b.md
 D docs/handoffs/auto-handoff-2026-08-18-2242-7c7cee38.md
 D docs/handoffs/auto-handoff-2026-08-19-1032-9338b4bb.md
 M tools/eq-copilot/pruefe_fbs_feldids.py
?? docs/handoffs/auto-handoff-2026-08-21-2327-4ca90ff5.md
?? docs/handoffs/auto-handoff-2026-08-21-2333-60c295b3.md
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-21 21:10:27 | `C6A69FDEDBAE248A` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-18 11:46:43 | `E01E176529ECCF6A` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-21 21:10:32 | `AD3233A03C16F161` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-21 21:10:37 | `05A6C84BF99E686B` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-21 23:46:42 | `F68284E36B529CEB` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`, `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-21 23:46:31**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,07 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,42 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,07 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,50 s | [↓ A4](#a4) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 0,62 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,23 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,97 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,23 s | [↓ A11](#a11) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B1](#b1) |
| B2 | State-Schema 2 laedt reine Schema-1-Staende ohne Verlust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [GEPLANT] geplant (ab P1) | - | - |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,13 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue haelt Blockgroessen-Stress ohne Allokation/Lock aus. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,07 s

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

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,50 s

stdout:

```text

running 51 tests
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
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
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_ltas_array_wird_gezaehlt_verworfen ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 51 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 6 tests
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test quantisierung_stimmt_mit_den_testvektoren ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 6 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.09s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-1ae3914fdef4c778.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-d65c73b91b696f68.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-d2b910788f79aaeb.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-9c30b17f91172beb.exe)
   Doc-tests eqcop_broker
```

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 0,62 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
47 Definitionen, 17 Nachrichtenfamilien, 153 Fixtures
Pruefungen: 177 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 38  
  const                     4 / 4    HART
  enum                     14 / 14   HART
  exclusiveMinimum          2 / 2   
  items                     1 / 4   
  maxItems                  3 / 4   
  maxLength                 2 / 15  
  maxProperties             2 / 3   
  maximum                   4 / 14  
  minItems                  2 / 4   
  minLength                 4 / 9   
  minimum                   6 / 40  
  oneOf                     4 / 4   
  pattern                   1 / 1   
  required                 38 / 169 
  type                      8 / 134 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:610: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
36 gueltige, 117 ungueltige Fixtures
  bytegleich: 155 Dateien, MANIFEST sha256=bc72c521906af48592413ffa28064bdeb8feddee2955ec50093fbdda4a421b2c
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,97 s

stdout:

```text
9 gueltige, 42 ungueltige Fixtures
  bytegleich: 52 Dateien, MANIFEST sha256=15bcde347cfa559c0b2992c5f9f5f6e7583f64e7b4ba33347b4c46f808942a35
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

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
  ok      moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [22 Aug 2026 12:02:55am vs 21 Aug 2026 5:16:01pm]
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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260822-000314.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (21320 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [59 Faelle]
[ok]   Falltabelle hat Substanz  [59 Faelle]
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (51 Fixtures)
[ok]   Binaerkorpus hat Substanz  [51 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (153 Fixtures)
[ok]   Korpus hat Substanz  [153 Fixtures]
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
56 bestanden, 0 gescheitert
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 5,36 s

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
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
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


---

## Nacharbeit zum Gate G1 — 24.08.2026

**Marke:** `T3 NEEDS_WORK 2026-08-24 nachgearbeitet`. Der G1-Befund §4.3 ist
geschlossen (`fa976fd`); das Urteil bleibt NEEDS_WORK, weil wer repariert, sich
nicht selbst freispricht.

`start_ppq` und `end_ppq` werden in **beiden** Lesern auf Endlichkeit geprüft,
dazu die Ordnungsregel `ppq_verdreht` (nur bei `bounds_valid`, weil §32.3 ohne
das Bit über die Grenzen nichts behauptet). Vier neue Binärfixtures, Korpus
51 → 55.

🔑 **Der tragende Teil des Fixes war nicht die Prüfung, sondern der Eintrag ins
MANIFEST.** Beide Leser hatten dieselbe Auslassung und waren sich deshalb
einig — der Kreuzsprachtest stimmte überein, auf der falschen Antwort.
`grep -c ppq MANIFEST.json` = 0 war der eigentliche Befund. Das Manifest ist die
dritte Partei: jeder Leser wird gegen *es* gemessen, nie gegen den anderen.

Vorgeführt: Prüfung **nur im Rust-Leser** entfernt →
`fb_korpus_klassifiziert_wie_das_manifest` FAILED mit drei Zeilen („Urteil
ungueltig erwartet, Leser sagt gueltig"). Damit ist die Auflage des Prüfers
erfüllt, dass die Prüfung schon in *einem* Leser allein rot wird. Rückstellung
byteweise statt per `git checkout` (NAK-73): SHA-256 vorher == nachher, `cmp`
bytegleich. Roh in `G1.md` §10.3.

---

## Kanon-Lauf - S6 Dirigent 2026-08-28 - Kanon auf 6c314e1 vor T3-Urteil

**Lauf:** 2026-08-28 18:29 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 29/29 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-28 18:29:19 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 6c314e1 S5 SONDE-005a: T3 PASS 2026-08-28 - drei frische Codex-Pruefungen, zwei Nacharbeitsrunden, Planstand |
| Commit (voll) | 6c314e1862520e8e72ac2034fb4d75cae16ea1da |
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
| `EqCopNullTest` | 2026-08-28 16:37:15 | `A8000FEF90307EEE` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-27 03:22:19 | `C89836581A6FFD48` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-28 16:37:19 | `08BFDD5F08117D89` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-27 02:25:05 | `ECA970E1DE7BA8CC` | frisch (Bau bestaetigt) |
| `EqCopSunaNullTest` | 2026-08-28 16:37:30 | `2EB04A41F6432C5F` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-28 16:37:32 | `C393C82F65A024A2` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-28 16:37:34 | `F12EA9D0EBF94F07` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-28 16:37:36 | `A5DF7B99893A3DD7` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-28 16:37:43 | `FCA232DB6568340A` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-28 16:37:45 | `FBD7BBAAD558392D` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-27 01:11:05 | `F56913243873A4FD` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-28 16:37:49 | `91F2C529928CB0FB` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-28 16:37:51 | `ED6BD5E4CAB0D9F5` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-28 16:30:50**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,07 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,64 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,48 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,95 s | [↓ A4](#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,06 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,44 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,18 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,25 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,81 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,24 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,20 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,39 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,83 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 29,02 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,18 s | [↓ A19](#a19) |
| A15 | Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | [OK] Exit 0 | 0,14 s | [↓ A15](#a15) |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,14 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,16 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,49 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,12 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,23 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,24 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,46 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,23 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,19 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,17 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,64 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,48 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: als Main klassifiziert (§53.5)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: als Main klassifiziert (§53.5)
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: als Main klassifiziert (§53.5)
  ok      T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: als Main klassifiziert (§53.5)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: als Main klassifiziert (§53.5)
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: beide Instanzen als Main klassifiziert (§53.5)
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
  ok      T11: als Main klassifiziert (§53.5)
  ok      T11: Auftrag gebaut
  ok      T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)
  ok      T11: und die Markierung meldet sich nicht hoerbar
  ok      T11: mit gestopptem Transport ebenfalls kein Sample
  ok      T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,95 s

stdout:

```text

running 90 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test dto::tests::as_f64_liefert_fuer_jede_number_einen_wert ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test dto::tests::bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test dto::tests::doppelter_schluessel_wird_nicht_still_ueberschrieben ... ok
test dto::tests::geschachtelter_doppelter_schluessel_faellt_ebenfalls ... ok
test aggregat::tests::schreiben_erzeugt_haltbare_datei ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::hin_und_zurueck ... ok
test bindung::tests::fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben ... ok
test aggregat::tests::alte_temp_restdatei_blockiert_snapshot_nicht ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow ... ok
test aggregat::tests::gleicher_zeitstempel_ueberschreibt_keinen_snapshot ... ok
test protokoll::tests::heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema ... ok
test protokoll::tests::gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::hello_enum_und_audiofelder_werden_semantisch_begrenzt ... ok
test protokoll::tests::hello_textfelder_haben_codepunktgenaue_harte_grenzen ... ok
test protokoll::tests::messstand_guard_prueft_minima_endlichkeit_und_textcap ... ok
test protokoll::tests::option_felder_akzeptieren_explizites_null_wie_das_schema ... ok
test protokoll::tests::unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test bindung::tests::wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset ... ok
test register_tests::owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test bindung::tests::parallele_schreiber_hinterlassen_genau_einen_intakten_stand ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id ... ok
test register_tests::stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge ... ok
test register_tests::unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test aggregat::tests::parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab ... ok
test server::tests::fremdes_session_token_beendet_established_statt_acklos_zu_haengen ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::ungelesenes_welcome_blockiert_connection_ende_nicht ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test vertrag::tests::textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python ... ok
test server::tests::sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung ... ok
test server::tests::sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten ... ok
test server::tests::v1_heartbeat_mit_measurement_ist_protokollbruch ... ok
test server::tests::zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen ... ok
test server::tests::zweites_hello_kann_die_established_bindung_nicht_umschreiben ... ok
test server::tests::bye_braucht_exakt_das_ausgegebene_session_token ... ok
test server::tests::ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz ... ok
test server::tests::connection_und_pipe_instanzzahl_bleiben_hart_begrenzt ... ok
test server::tests::ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist ... ok
test server::tests::acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup ... ok

test result: ok. 90 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.48s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 8 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test dto_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 8 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.13s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.07s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a4b"></a>
#### A4b | EqCopPipeClientTest

**Befehl:** `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | **Exitcode:** 0 | **Dauer:** 1,06 s

stdout:

```text
PIPECLIENT-LIFECYCLE-TEST
  ok      Fake-Pipe fuer Welcome-Stopp angelegt
  ok      stop() bricht blockiertes Welcome-Lesen begrenzt ab  [0 ms]
  ok      SQOS begrenzt den Peer auf SecurityIdentification  [1]
  ok      Fake-Pipe fuer ACK-Stopp angelegt
  ok      stop() bricht blockiertes ACK-Lesen begrenzt ab  [0 ms]
  ok      Reconnect-Fake-Pipe angelegt
  ok      Reconnect waehrend Welcome verliert keine Konfigurationsgeneration
  ok      ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt
  ok      Welcome mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ACK mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers  [326 ms]
  ok      u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze
  ok      200 konkurrierende start/stop-Paare ohne joinable-Leak
PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler
```

stderr:

_(leer)_

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 1,44 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
52 Definitionen, 17 Nachrichtenfamilien, 188 Fixtures
Pruefungen: 284 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 49  
  const                     8 / 8    HART
  enum                     10 / 10   HART
  exclusiveMinimum          2 / 2   
  items                     3 / 8   
  maxItems                  3 / 8   
  maxLength                 4 / 20  
  maxProperties             2 / 3   
  maximum                   6 / 18  
  minItems                  2 / 8   
  minLength                 5 / 14  
  minimum                   8 / 48  
  oneOf                     7 / 8   
  pattern                   4 / 9   
  required                 40 / 229 
  type                     11 / 165 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:933: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
42 gueltige, 146 ungueltige Fixtures
  bytegleich: 190 Dateien, MANIFEST sha256=bedc885d7e2ca10aebadee1d41b04187ed61b2792649804d04600d5dbb96cbee
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,81 s

stdout:

```text
9 gueltige, 46 ungueltige Fixtures
  bytegleich: 56 Dateien, MANIFEST sha256=dd794d59c27a62f66751ee95fc1f3f4eabd21c39a849c895ccdee02936befdcc
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,24 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,20 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,39 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (737942 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7105024 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      die Lib wurde mit der heutigen Definemenge gebaut (15 aus der Projektdatei)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.bundle = 'Nakama Probeeq.vst3' NICHT
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt active-probe.produktname = 'Nakama Probeeq' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.bundle = 'Nakama Suna.vst3' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT
  ok      NakamaKern.lib traegt passive-probe.produktname = 'Nakama Suna' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die 5 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

26 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 0,83 s

stdout:

```text
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag  [3 vs 3; identity=ok]
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID im Installer-Manifest
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest
  ok      faellt am verdorbenen Manifest: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt am verdorbenen Manifest: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt am verdorbenen Manifest: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt am verdorbenen Manifest: Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt am verdorbenen Manifest: kein Viercode, keine Class-ID im Installer-Manifest
  ok      faellt am verdorbenen Manifest: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt am verdorbenen Manifest: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt am verdorbenen Manifest: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt am verdorbenen Manifest: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt am verdorbenen Manifest: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt am verdorbenen Manifest: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt am verdorbenen Manifest: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[4] Auslieferungsstand
  hinweis nicht ausgeliefert - 4 Artefakt(e) ohne Hash: main, passive-probe, active-probe, eqcop-broker.exe
          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

38 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 29,02 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
  ok      kanonische VST3-Produktpolicy steht unveraendert im Original
  ok      kanonische Broker-Produktpolicy steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle
      - $erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
      + $erlaubteVst3Basis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-m6imwobb\ziel\VST3'
      - $erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
      + $erlaubteBrokerBasis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-m6imwobb\ziel\programme\evenacadia\Nakama'

[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
  ok      Bundle ohne erwartete innere Payload wird fail-closed verweigert  [Exit 1]
  ok      Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0: phili\AppData\Local\Temp\nakama-gegenpfad-m6imwobb\ziel\VST3\EQ-Copilot.vst3]
  ok      Nakama Suna installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-m6imwobb\ziel\VST3\Nakama Suna.vst3]
  ok      Nakama Probeeq installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-m6imwobb\ziel\VST3\Nakama Probeeq.vst3]
  ok      eqcop-broker.exe installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-m6imwobb\ziel\programme\evenacadia\Nakama\eqcop-broker.exe]
INSTALLATION OK
]
  ok      geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg
  ok      jede Sicherung ist relativ an backups/<transaktions-id> gebunden
  ok      Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet
  ok      beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM
  ok      wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      passive-probe: Bundle traegt moduleinfo.json  [Nakama Suna.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[2b] Aktiver Recovery-Anker wird nicht still superseded
  ok      geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie  [Exit 1]
  ok      VORBEREITET verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      VORBEREITET: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      KOMPENSATION verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      KOMPENSATION: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      ERROR_TEILSTAND verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      ERROR_TEILSTAND: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      RUECKWEG_AKTIV verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      RUECKWEG_AKTIV: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded  [Exit 0]
  ok      Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren
  ok      zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle vier Artefakte melden `aktuell`  [4x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben
  ok      frische Transaktion friert den nun bekannten historischen Stand ein
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht
  ok      passive-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Suna.vst3]
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      das im Journal belegte Broker-Ziel ist weg
  ok      unbelegte Elternverzeichnisse bleiben unangetastet
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert
  ok      spaeter Installationsfehler endet laut und nicht als Teilerfolg  [Exit 1]
  ok      Journal bestaetigt die vollstaendige Kompensation  [ERROR_RUECKGEROLLT]
  ok      Main ist nach der Kompensation bytegleich zum Vorzustand
  ok      passive-probe: kein Teilstand nach spaetem Fehler
  ok      active-probe: kein Teilstand nach spaetem Fehler
  ok      der fremde Blocker wurde nicht angetastet
  ok      vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei
  ok      ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden  [Exit 0]
  ok      nach Fehler-Supersede bleibt nur die aktive Backup-Generation

[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig
  ok      Ausgangsinstallation fuer den Hard-Crash-Zwischenstand
  ok      VORBEREITET + mutation_begonnen restauriert statt Drift zu melden  [Exit 0]
  ok      der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt
  ok      Ausgangsinstallation fuer einen unterbrochenen inversen Pfad
  ok      RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort  [Exit 0]
  ok      auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand

[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen
  ok      Ausgangsinstallation fuer Journal-Missbrauchstests
  ok      manipulierte Repo-Diagnose ist keine Rueckwegautoritaet  [Exit 0]
  ok      fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben  [Exit 1]
  ok      fremder Loeschpfad im Journal wird nicht entfernt  [Exit 1]
  ok      Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen  [Exit 1]

[11] Geschuetzte Autoritaet, Backup- und Zieldrift
  ok      oeffentliches Journal allein autorisiert keinen Rueckweg  [Exit 1]
  ok      Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash  [Exit 1]
  ok      frische Ausgangsinstallation fuer Zieldrift
  ok      veraendertes Installationsziel wird nicht ueberschrieben  [Exit 1]
  ok      Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet

[12] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2
  ok      Runtime verwirft eine Bundle-Zielkollision  [Exit 1]
  ok      Runtime bindet Identity typstreng an State-Schema 2  [Exit 1]
  ok      Runtime verwirft kanonisches Broker-Zieltraversal  [Exit 1]

80 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a19"></a>
#### A19 | erzeuge_bandgitter_header.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen (204 Zeilen).
    Evidenz nakama_1_24_oct_30_18k_v1: 221 Baender, 222 Kanten
    Live    nakama_log64_v1: 64 Gruppen, lueckenlose Partition der 221
```

stderr:

_(leer)_

<a id="a15"></a>
#### A15 | EqCopSunaNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse passive_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [passive_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [172 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [passive_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [172 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse active_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [active_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [3439 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [3439 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,16 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes Ziel im Manifest hat hier eine Zeile  [3 vs 3]
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest
  ok      CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle: COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle: der Viercode steht nicht literal im Zielblock  [Eqcp]
  ok      CMake-Quelle: der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      CMake-Quelle: der Produktname steht nicht literal im Zielblock
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden
  ok      CMake: Defineblock der Sondenfunktion gefunden
  ok      CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      passive-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkPr]
  ok      passive-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Suna]
  ok      passive-probe: NakamaSuna entsteht ueber nakama_sonde_ziel()
  ok      passive-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaSuna    passive-probe NAKAMA_SONDE_PASSIV)]
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 6:29:45pm vs 27 Aug 2026 2:27:21am]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      passive-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\Resources\moduleinfo.json]
  ok      passive-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 6:29:49pm vs 27 Aug 2026 2:27:21am]
  ok      passive-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      passive-probe: Produktname wie im Manifest  [Nakama Suna]
  ok      passive-probe: Vendor wie im Manifest  [evenacadia]
  ok      passive-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      passive-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: Unterkategorien wie im Manifest
  ok      passive-probe: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 6:29:50pm vs 27 Aug 2026 2:27:21am]
  ok      active-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      active-probe: Produktname wie im Manifest  [Nakama Probeeq]
  ok      active-probe: Vendor wie im Manifest  [evenacadia]
  ok      active-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      active-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B4163]
  ok      active-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B4163]
  ok      active-probe: Unterkategorien wie im Manifest
  ok      active-probe: keine fremde Ziel-CID im Bundle
  ok      Viercode ist vier Zeichen lang: main.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      Viercode ist vier Zeichen lang: passive-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      Viercode ist vier Zeichen lang: active-probe.plugin_code  [4 Zeichen]
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
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 114 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,49 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      16 Kombinationen aus Klasse x Messposition wie §2.2 - post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)  [16]
  ok      == Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle
  ok      21 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [21]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser
  ok      gueltiger Praefix mit Suffix wird vollstaendig ignoriert
  ok      abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen
  ok      deklarierte Riesenvariante wird vor JUCE-Allokation verworfen
  ok      zu tiefer ValueTree wird begrenzt und ignoriert
  ok      zu tief geschachteltes Variantenarray wird begrenzt und ignoriert
  ok      State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen
  ok      ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel
  ok      63 verschachtelte Arrays plus Blatt passieren und laden
  ok      65.536 Eintraege in einer Sammlung passieren den Byte-Riegel
  ok      65.537 Eintraege in einer Sammlung werden verworfen
  ok      262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel
  ok      262.145 Eintraege ueber mehrere Sammlungen werden verworfen
  ok      Teststate trifft die schreibbare Bytegrenze exakt  [16773120]
  ok      Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773848]
  ok      State ohne konkreten Writer-Headroom bleibt read-only bytegleich
  ok      vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [9]
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [10]
  ok      additive Binaer- und Array-Properties bleiben bytegleich lesbar
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [279734b3b1e445f0bcff826f78c5a112]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [5c847683930b4ab99456207343f561a4]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 153 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,12 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,23 s

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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260828-183045.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (34536 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,24 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [102 Faelle]
[ok]   Falltabelle hat Substanz  [102 Faelle]
[ok]   Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (55 Fixtures)
[ok]   Binaerkorpus hat Substanz  [55 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-auf-2hoch53.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740992.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-nahe-2hoch53-ab.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740991.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-unter-2hoch53-auf.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 4503599627370495.9]
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-kleiner-bruch-rundet-auf-eins.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 1.00000000000000001]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nan-token.json  [unbekanntes Literal NaN an Position 1090]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (188 Fixtures)
[ok]   Korpus hat Substanz  [188 Fixtures]
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
63 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 3,46 s

stdout:

```text
== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==
Produktionsstrom GenStrom: 1 Tap(s), 131072 Frames Ring, 2048 Deskriptoren, max 16384 Frames je Block.

== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==
  ok      drei Bloecke, drei Deskriptoren  [3]
  ok      jeder Frame kommt bitgleich und an derselben Stelle heraus
  ok      Stempel: lokaler Strom und Projektzeit stimmen je Block
  ok      nach dem Leeren ist die Queue leer
  ok      kein Verlust bei Normallast
== B - Wrap: der Ring laeuft mehrfach um ==
  ok      der Ring ist wirklich mehrfach umgelaufen  [11 Umlaeufe]
  ok      ueber jeden Wrap hinweg bitgleicher Inhalt
  ok      kein Frame verloren, keiner doppelt  [12000/12000]
== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==
  ok      vier Bloecke passen, danach ist Schluss  [4]
  ok      beide abgewiesenen Bloecke sind gezaehlt  [2]
  ok      verlorene Frames = 2 x 256 (keine Teilmenge)  [512]
  ok      ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock
  ok      der erste Block nach dem Verlust traegt kFlagLueckeDavor
  ok      und beginnt ein neues continuity_segment  [1]
== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==
  ok      genau `deskriptorPlaetze` Bloecke passen  [8]
  ok      die vier ueberzaehligen sind gezaehlt  [4]
  ok      und genau so viele stehen zum Lesen bereit  [8]
== E - Oversize: ein Block ueber der Slotkapazitaet ==
  ok      ein Block ueber maxBlockFrames wird als GANZES abgewiesen
  ok      und als Oversize gezaehlt, nicht als Ueberlauf  [1/0]
  ok      der Ueberlaufzaehler bleibt sauber
  ok      nichts Halbes im Ring
  ok      die verlorene Zeit ist vollstaendig gemeldet  [257]
  ok      genau maxBlockFrames passt noch (Kante, nicht Ausschluss)
== F - Flush (numSamples == 0) ist kein Verlust ==
  ok      ein Flush-Block wird nicht eingestellt
  ok      und zaehlt NICHT als Verlust - er traegt keine Audiozeit
  ok      der Folgeblock geht normal
  ok      und traegt keine Luecke
== G - Mono und Mehrfach-Tap ==
  ok      Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)
  ok      und der Deskriptor sagt kanaele == 1
  ok      Drei-Tap-Layout: Block angenommen
  ok      das Bitset meldet genau die belegten Taps  [3]
  ok      jeder Tap liegt getrennt und richtig; der inaktive liefert false
  ok      eine falsche Tapzahl wird abgewiesen statt halb kopiert
  ok      ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor
== H - Ein-Block-Quarantaene ==
  ok      5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene  [4]
  ok      und die Quarantaene haelt ihn wirklich
  ok      kein Bruch bei lueckenlosem Strom
  ok      der Seek ist als Bruch erkannt  [1]
  ok      genau EIN Block faellt an der Grenze - der Strom laeuft weiter
  ok      der Block NACH der Grenze ist schon wieder in Quarantaene
  ok      Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)
  ok      stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      Vorlauf ohne Bruch
  ok      Transportkante play->stop ist ein Bruch  [1]
  ok      ohne Hostkontext traegt der lokale monotone Strom (§53.6)
  ok      und versiegelt normal  [3]
  ok      Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung
  ok      negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)
== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==
  20 000 Bloecke, Groessen 1..288: angenommen 17809, Ueberlauf-Drops 0, Oversize-Drops 2191, groesster Block 256, versiegelt 15872, Brueche 1936
  ok      jeder gelesene Frame steht an der Stelle, die sein Stempel nennt
  ok      es gab nie einen Block mit sampleCount 0
  ok      die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt  [2191]
  ok      kein angenommener Block war je groesser als die Slotkapazitaet  [256]
  ok      es wurde wirklich gelesen  [2288127 Frames]
== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==
  ok      18 Blockgroessen von 1 bis 16 384: kein Sample veraendert
  ok      0 Samples Latenz  [0]
  ok      kein Tail
  angenommen 18, groesster Block 16384, Ueberlauf 0, Oversize 0
== K - verdrahtet: Oversize aendert das Audio nicht ==
  ok      ein Block ueber der Slotkapazitaet laeuft unveraendert durch
  ok      und ist als Oversize-Drop der ANALYSE gezaehlt  [1]
  ok      der abgewiesene Oversize-Block erscheint nicht im Projektfenster
  ok      der naechste angenommene Lueckenblock beginnt das Fenster exakt neu  [16385..16449]
  ok      der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben  [0]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3844 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
== M - verdrahtet: die Messung bekommt den Strom wirklich ==
  eingespeist 287744, analysiert 287232, Drops 0, versiegelt 561, Quarantaene-Verwuerfe 0, Brueche 0
  ok      bei Normallast kein einziger Drop
  ok      und kein Kontinuitaetsbruch (lueckenloser Transport)
  ok      die Engine sieht den ganzen Strom bis auf den Block in Quarantaene  [512 Frames offen]
  ok      Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)  [-8.684240]
  ok      framesDropped im Stats-Snapshot kommt aus derselben Quelle
== N - Gegenpfad: prepareToPlay mitten im Betrieb ==
  ok      auch der erste Block nach prepareToPlay alloziert nicht  [0]
  veraltete Bloecke (alter Anlauf, nicht analysiert): 40
  ok      ein Neuanlauf ist kein Oversize
== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==
  ok      der Anlauf steigt sofort, nicht erst beim naechsten Audioblock  [0 -> 1]
  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt 3, als aktuell durchgelassen 0
  ok      alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar  [3/3]
  ok      der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt
  ok      Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell
  ok      und er traegt die Luecke, die der Neuanlauf gerissen hat
  ok      der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut
  ok      zwei Neuanlaeufe hintereinander gehen beide nicht verloren  [1 -> 3]
  ok      zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration
  ok      und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste
  ok      Nullframe konsumiert die wartende Generation nicht
  ok      der naechste echte Block uebernimmt sie weiterhin
== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==
  ok      Seek um 10 s bei gestopptem Transport ist EIN Bruch  [1]
  ok      Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)  [0]
  ok      und bei laufendem Transport ist der Seek weiterhin ein Bruch  [1]
  ok      bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet  [0]
== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==
  ok      nach Start ist nur die leichte Publikation sofort faellig
  ok      10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus
  ok      vor 250 ms bleibt Gating/Kandidaten gesperrt
  ok      bei 250 ms ist genau die schwere Runde faellig
  ok      dieselbe Deadline kann nicht zweimal verbraucht werden
  ok      nach Pause genau eine Runde, kein Catch-up-Sturm
  ok      Reset/Generation startet die 250-ms-Deadline neu
== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==
  ok      Test hat echten Queue-Rueckstau hergestellt  [257 Bloecke bis zum ersten Drop]
  ok      unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms  [1 Runden in 600 ms]
  ok      neue Generation erbt keinen Batch-beschleunigten Schwertakt  [0 Runden in den ersten 100 ms]
  ok      Frame-Leser wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Reset-Steuerer wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Stop/Join bleibt auch mit Queue-Rest begrenzt  [0 ms]
== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==
  ok      Gegenprobe: erste Generation hat ihr eigenes Projektfenster
  ok      Testbarriere liegt deterministisch im laufenden Audiocallback
  ok      der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster  [10000..10064]
  ok      die Generationskante ist kein Seek innerhalb des alten Fensters  [0]
== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==
  ok      Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine
  ok      nichtendliche Hostrate wird fail-closed auf 0 gespiegelt
  ok      M1 verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      FeatureEngine verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung
  ok      eine folgende gueltige Generation aktiviert beide Engines wieder

114 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Feingitter: 10001 Bins a 0.01 LU ueber [-70.00, 30.01) LUFS.
Oberband:   3070 Bins a 1.00 LU ueber [30.01, 3100.01) LUFS - lautheit(DBL_MAX) = 3081,86.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: kein Block faellt durch das Raster  [0]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: kein Block faellt durch das Raster  [0]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): kein Block faellt durch das Raster  [0]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: kein Block faellt durch das Raster  [0]
  ok      Musik + echte Stille: LUFS-I innerhalb ±0,1 LU  [akku=-8.281644 ref=-8.281644 d=0.000000000]
  ok      Musik + echte Stille: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Musik + echte Stille: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000027]
  ok      Musik + echte Stille: Kurz-LUFS Gueltigkeit gleich
  ok      Musik + echte Stille: Kurz-LUFS BITGLEICH  [akku=-12.451912590513381 ref=-12.451912590513381]
== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==
  Sweep: 201 Laeufe, groesste Abweichung 2.781360 LU bei Faktor 1.000000, groesste gemeldete Schranke 2.781412 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen (Eingang aendert sich um 1e-5): 2.781403 LU
  ok      Sweep vollstaendig gefahren  [201/201]
  ok      Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)  [sonst pruefte D nichts; schranke=2.781412]
  ok      die selbstgemeldete Schranke deckt JEDEN Lauf  [201/201]
  ok      wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU  [76/76]
  ok      die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)  [maxD=2.781360 refSprung=2.781403]
  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)
== E - fixed memory: eine Million Zellen ==
  Akku:     0 Allokationen, LUFS-I -8.386510, Schranke 0.000000000 LU
  Referenz: 36 Allokationen, LUFS-I -8.386510
  ok      1 000 000 Zellen: 0 Allokationen im Akku  [0]
  ok      Gegenprobe: die alte Rechnung alloziert sehr wohl  [36]
  ok      1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU  [d=0.000000000]
  Fester Bedarf laut speicherBytes(): 157122 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      Kurz-LUFS meldet einen nichtendlichen Ring nie als gueltig
  ok      ueber dem Gitter: EIN Pegel bleibt wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      Gegenprobe: der Korpus liegt wirklich im Oberband  [197]
  ok      und kein Block faellt durch das Raster
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer
== G - Oberband: zwei Pegel ueber dem Feingitter ==
  G1: akku=49.302611 ref=49.302611 schranke=0.000000000 ueberGitter=1997
  ok      G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)  [1997]
  ok      zwei Pegel ueber dem Gitter (T2-1): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      zwei Pegel ueber dem Gitter (T2-1): kein Block faellt durch das Raster  [0]
  ok      zwei Pegel ueber dem Gitter (T2-1): LUFS-I innerhalb ±0,1 LU  [akku=49.302611 ref=49.302611 d=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS Gueltigkeit gleich
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS BITGLEICH  [akku=32.319299956639810 ref=32.319299956639810]
  ok      Naht, Gamma_r im Oberband: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Oberband: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Oberband: LUFS-I innerhalb ±0,1 LU  [akku=49.302481 ref=49.302481 d=0.000000000]
  ok      Naht, Gamma_r im Oberband: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Oberband: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  ok      Naht, Gamma_r im Feingitter: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Feingitter: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Feingitter: LUFS-I innerhalb ±0,1 LU  [akku=49.243363 ref=49.243363 d=0.000000000]
  ok      Naht, Gamma_r im Feingitter: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Feingitter: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  G3-Sweep: 201 Laeufe, groesste Abweichung 2.781577 LU bei Faktor 1.001000, groesste gemeldete Schranke 2.781577 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: 2.781401 LU
  ok      G3: Sweep vollstaendig gefahren  [201/201]
  ok      G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)  [1997]
  ok      G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)  [sonst pruefte G3 nichts; schranke=2.781577]
  ok      G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband  [201/201]
  ok      G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz  [maxD=2.781577 refSprung=2.781401]
  ok      G3: kein Block faellt durch das Raster  [0]
  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - 1000 Bloecke kippen gemeinsam die Gateseite.)
  G4: LUFS-I 3038.851, Schranke 0.000000000, ueberGitter 197, ueberOberband 0
  ok      G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert  [3038.851]
  ok      G4: nichts wird faelschlich als nicht-endlich gezaehlt  [0]
  ok      G4: auch am aeussersten Rand faellt kein Block durch das Raster  [0]
  ok      G4: und die Schranke bleibt endlich  [0.000000000]

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
109 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b5"></a>
#### B5 | EqCopAnalysisGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 2,19 s

stdout:

```text
== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==
Gate: "Drop/Seek/Loop trennt jedes offene Fenster."
Stufen: Bass 16384 (Hop 8192), Haupt 4096 (Hop 2048), Trennung bei 200 Hz.

== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==
[ok]   Evidenzgitter: band_anzahl == kEvidenzBaender  [221]
[ok]   Livegitter: band_anzahl == kLiveBaender  [64]
[ok]   Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)  [0 Abweichungen]
[ok]   Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)  [0 Abweichungen]
[ok]   Live-Kanten BITGLEICH zur Fixture (65 Werte)  [0 Abweichungen]
[ok]   Live-Mitten BITGLEICH zur Fixture (64 Werte)  [0 Abweichungen]
[ok]   Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal  [0 Abweichungen, Ende bei 221]
[ok]   Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank  [0 Abweichungen]

== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==
[ok]   q_db_0p1_i16: Skalierung stimmt mit der Fixture ueberein  [10]
[ok]   q_db_0p1_i16: alle 23 Vektoren treffen
[ok]   q_db_0p01_i16: Skalierung stimmt mit der Fixture ueberein  [100]
[ok]   q_db_0p01_i16: alle 23 Vektoren treffen
[ok]   float32: alle 15 Vektoren treffen bitgenau
[ok]   Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0
[ok]   halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl
[ok]   und negativ genauso (-0,05 -> -1)

== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==
[ok]   Band i sitzt in Byte i/8, Bit i%8 (LSB-first)  [Byte0=81 Byte1=1]
[ok]   ein Bit loeschen laesst die Nachbarn stehen
[ok]   Lesen und Schreiben sind dieselbe Adressierung
[ok]   221 Baender brauchen 28 Bitmapbytes  [28]
[ok]   die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)  [letztes Byte = 1f]

== D - FFT: gegen analytisch bekannte Ergebnisse ==
[ok]   Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin  [100.0000 %]
[ok]   Parseval haelt (Zeit- und Frequenzenergie gleich)  [rel. Fehler 0.0000000000000002]
[ok]   Gleichanteil sitzt vollstaendig in Bin 0  [Bin0 = 1048576.0, Rest = 0.00000000000000000000]

== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==
[ok]   RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander  [max 0.04312 dB bei 20000.0 Hz]
       (gemessene Groesstabweichung, zur Ablesung: 0.043120 dB)
[ok]   Hochpass sperrt den Gleichanteil vollstaendig  [|H(0)|Â² = 0.000000000000000000000000]

== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==
[ok]   die Sequenz zaehlt ab 1 aufwaerts  [6]
[ok]   mit gueltiger Projektzeit ist die Zeitbasis project_samples
[ok]   und project_sample_start ist gesetzt
[ok]   Gueltigkeitsbit project_time steht
[ok]   der Frame traegt seine Metrikversion  [20260823]
[ok]   und die Samplerate
[ok]   Frame-Stempel spannt alle integrierten Hostbloecke auf  [0 + 5120]
[ok]   Frame-Stempel umfasst den ueberlappenden FFT-Support  [2048 + 8192]
[ok]   ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck
[ok]   Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen
[ok]   fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden
[ok]   Evidenzframe vereinigt Haupt- und aelteren Basssupport  [0 + 30720, 196 Baender]
[ok]   stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen
[ok]   stehende FL-Teilstuecke erfinden kein Projektintervall
[ok]   ueberlaufende Host-Projektspanne wird konservativ lokal publiziert
[ok]   Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block  [104608 + 512]
[ok]   ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (Â§32.3)
[ok]   und dann steht WEDER das Bit NOCH ein Startwert da
[ok]   alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet  [0x7f]
[ok]   eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (Â§32.3)
[ok]   Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer
[ok]   continuous_time_samples kommt mit demselben Frameanker durch
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: NaN
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Inf
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Ende vor Start
[ok]   ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich

== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==
[ok]   G1 Drop (kFlagLueckeDavor): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G1 Drop (kFlagLueckeDavor): Ursache ist lokaleLuecke  [Zaehler 1]
[ok]   G1 Drop (kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche (§32.3)  [Epochen 0 Segmente 1]
[ok]   G2 Seek waehrend Wiedergabe: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G2 Seek waehrend Wiedergabe: Ursache ist zeitSprung  [Zaehler 1]
[ok]   G2 Seek waehrend Wiedergabe: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G3: bei gestopptem Transport laufen die Fenster normal weiter  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)  [0 Epochenwechsel]
[ok]   G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G3: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G3: und sie heisst zeitSprung, nicht lokaleLuecke
[ok]   G4: aktive Schleife allein trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4 Loop-Wrap: jedes Fenster getrennt  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G4: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G4: und die Ursache heisst loopWrap, nicht zeitSprung
[ok]   G5: Schleife ohne erreichbare Grenze trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, weil der Straddle-Block auch kein neues beginnt  [Bass=0 Haupt=0 Zelle=0 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G5: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet
[ok]   G5: mit eigener Ursache moeglicherStraddle
[ok]   G6 Transportkante (Play -> Stop): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G6 Transportkante (Play -> Stop): Ursache ist transportKante  [Zaehler 1]
[ok]   G6 Transportkante (Play -> Stop): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G7 Sampleratewechsel: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G7 Sampleratewechsel: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G7 Sampleratewechsel: Ursache ist sampleratewechsel  [Zaehler 1]
[ok]   G7 Sampleratewechsel: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G8 Neuanlauf (prepareToPlay): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G8 Neuanlauf (prepareToPlay): Ursache ist neuanlauf  [Zaehler 1]
[ok]   G8 Neuanlauf (prepareToPlay): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): Ursache ist beweislageWechsel  [Zaehler 1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G11: im Betrieb traegt die K-Kette einen Filterzustand  [1.392229]
[ok]   G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame  [290 / 290]
[ok]   G11: beide Laeufe liefern eine Kurzzeitlautheit
[ok]   G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - der Filternachklang ueberbrueckt nicht  [A=-60.034744263 B=-60.034744263]
[ok]   G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)  [0 Trennungen]
[ok]   G10: die Fenster laufen dabei weiter  [Bass=9224 Haupt=3080 Zelle=264 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G12: vor der Grenze tragen BEIDE Bandakkus den Ton  [Live 149 / Evidenz 149 Baender]
[ok]   G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber die Grenze weiter (Entscheid Â§10.1)  [liveSamples=0]
[ok]   G12: und der Frame traegt die NEUE Epoche  [1]
[ok]   G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird also wirklich geprueft
[ok]   G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)  [0 gesetzte Baender]
[ok]   G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter  [0 gesetzte Baender]
[ok]   G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit  [nicht gesetzt]
[ok]   G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden  [1192 Band-Fensterbeitraege]
[ok]   G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren  [120 von 120]
[ok]   G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor (Pruefer-Messung: 80 / 40 / 43)  [0 mit Live-Baendern, 0 mit Evidenz, 0 mit Aktivitaet]

== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) ==
[ok]   G13.0: zwei frische Frames sind gleich - der Vergleich sagt nicht pauschal ungleich
[ok]   G13.0: ein einziger geaenderter Skalar macht sie ungleich
[ok]   G13.0: ein einziger geaenderter Bandwert ebenso
[ok]   G13.0: und ein einziges Bitmapbit ebenso
[ok]   G13.0: und ein Feld im verschachtelten Transportstempel - der Vergleich reicht bis dorthin
[ok]   G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen vorgefuehrt
[ok]   G13a lokaleLuecke [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13a lokaleLuecke [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13a lokaleLuecke [Zwilling]: beide haben die Grenze lokaleLuecke gezogen  [A 1 / B 1]
[ok]   G13a lokaleLuecke [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13a lokaleLuecke [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13a lokaleLuecke [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13a lokaleLuecke [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13a lokaleLuecke [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13b zeitSprung [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13b zeitSprung [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13b zeitSprung [Zwilling]: beide haben die Grenze zeitSprung gezogen  [A 1 / B 1]
[ok]   G13b zeitSprung [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13b zeitSprung [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13b zeitSprung [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13b zeitSprung [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13b zeitSprung [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13c transportKante [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13c transportKante [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13c transportKante [Zwilling]: beide haben die Grenze transportKante gezogen  [A 1 / B 1]
[ok]   G13c transportKante [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13c transportKante [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13c transportKante [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13c transportKante [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13c transportKante [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13d sampleratewechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13d sampleratewechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13d sampleratewechsel [Zwilling]: beide haben die Grenze sampleratewechsel gezogen  [A 1 / B 1]
[ok]   G13d sampleratewechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13d sampleratewechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13d sampleratewechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13d sampleratewechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13d sampleratewechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13e neuanlauf [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13e neuanlauf [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13e neuanlauf [Zwilling]: beide haben die Grenze neuanlauf gezogen  [A 1 / B 1]
[ok]   G13e neuanlauf [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13e neuanlauf [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13e neuanlauf [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13e neuanlauf [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13e neuanlauf [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13f beweislageWechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13f beweislageWechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13f beweislageWechsel [Zwilling]: beide haben die Grenze beweislageWechsel gezogen  [A 1 / B 1]
[ok]   G13f beweislageWechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13f beweislageWechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13f beweislageWechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13f beweislageWechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13f beweislageWechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]

== H - NAK-29: bedingte Feldpflichten des Transportstempels ==
[ok]   ein sauberer Stempel kommt durch
[ok]   Fall 1: project_samples + Bit, aber KEIN project_sample_start
[ok]   Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu Â§32.3)
[ok]   Fall 3: bounds_valid ohne start_ppq/end_ppq
[ok]   Fall 4: derivation=validated_block_mapping ohne bounds_valid
[ok]   Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht
[ok]   Fall 6: continuous_time-Bit ohne continuous_time_samples
[ok]   Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus
[ok]   Fall 7: sample_count bleibt unter der Vertragsobergrenze
[ok]   Fall 7: der Feature-Erzeuger publiziert keinen Leerframe
[ok]   der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel entsteht im normalen, gedeckelten Betrieb  [0]
[ok]   abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich  [1]

== I - Ereignisse: kein Fluss ueber eine Grenze ==
[ok]   der spektrale Fluss hat im Betrieb einen Vorgaenger
[ok]   nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss einen Onset, den es nie gab
[ok]   jedes Ereignis traegt die Epoche, in der es gesehen wurde  [1 Ereignis(se)]
[ok]   der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, nicht behauptet  [64 im Ring, 1 verworfen, nach 1107 Bloecken]
[ok]   und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (Â§48.1: ein Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)  [64 / 64]
[ok]   drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste  [aeltestes vorher 38912, jetzt 47104]
[ok]   und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf
[ok]   Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock
[ok]   stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit  [1 Ereignis(se) nach 84 Bloecken]

== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==
[ok]   LUFS-S ist nach 4 s gesetzt
[ok]   LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (Â§39.3)  [gemessen -6.0620 erwartet -6.0620 d=0.00000]
[ok]   Peak trifft die Amplitude  [-6.021 dBFS]
[ok]   Crest eines Sinus ist 3,01 dB  [3.0091 dB]
[ok]   L==R ergibt Korrelation 1  [1.000000]
[ok]   und Breite 0 (kein Seitenanteil)  [0.000000000000]
[ok]   das Live-Band um 997 Hz ist gueltig  [Band 35]
[ok]   und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal

== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==
[ok]   die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen  [38 Bloecke]
[ok]   und mindestens einen Frame gebaut  [4 Frames]
[ok]   process_context_present kommt aus der BRUECKE durch (Â§32.3)
[ok]   recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen
[ok]   continuous_time_samples kommt durch
[ok]   die Schleifengrenzen kommen durch, samt `active`
[ok]   beide Presentation-Latencies kommen durch  [3924 / 4410]
[ok]   ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits  [0x7f]
[ok]   Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits  [0x3]
[ok]   insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`
[ok]   Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen
[ok]   Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt
[ok]   nichtendliche Sampleraten initialisieren keine Analyseengine

== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==
[ok]   L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt
[ok]   L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen  [0]
[ok]   L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, und zwar nur dort  [1 Abweichung(en), Zeile 26]
[ok]   L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'
[ok]   L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts  [0 gesetzte Baender]
[ok]   L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte  [3]
[ok]   L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung  [1, 3]
[ok]   L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'
[ok]   L6: ein echter Grund wird gezaehlt  [1]
[ok]   L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen  [gelesen: 0]
[ok]   L6: und `keine` ebenso - getrennt wird nie ohne Grund

== M - Auskunft ueber die Grenzen der Messung ==
[ok]   die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da  [erstes messbares Band 2 (32.6 Hz)]
[ok]   bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz  [erstes Band ueber der Kappe 221]
[ok]   bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab  [erstes Band ueber der Kappe 202 (10525.6 Hz)]
[ok]   und sie sitzt genau am ersten Band, dessen Oberkante 0,95Â·Nyquist ueberschreitet  [10374.7 .. 10678.7 Hz gegen 10473.8 Hz]
[ok]   kein Live-Band vollstaendig ueber der Kappe traegt einen Wert  [0 Baender]
[ok]   Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler

Ergebnis: 235 bestanden, 0 Fehler.
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,17 s

stdout:

```text
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  ok      frisch: unclassified  [unclassified]
  ok      frisch: neutral
  ok      frisch: keine Audio-Ausnahme (audio-neutral)
  ok      frisch: kein Brokerstart
  ok      Schema-1 'sensor' -> legacy  [legacy]
  ok      Schema-1 'sensor': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'pre' -> legacy  [legacy]
  ok      Schema-1 'pre': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'post' -> legacy  [legacy]
  ok      Schema-1 'post': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'hub' -> main  [main]
  ok      Schema-1 'hub': Audio-Ausnahme moeglich
  ok      Schema-2 main (Kind-Matrix bestaetigt) -> main
  ok      Schema-2 legacy -> legacy
  ok      Vorbereitung: erst main
  ok      unbekanntes Major danach -> zurueck auf unclassified
  ok      Muell (ignoriert) aendert nichts - main bleibt main
  ok      Muell auf einen frischen Automaten klassifiziert nicht
  ok      explizite Initialisierung OHNE Editor: abgelehnt
  ok      ... und der Automat bleibt neutral
  ok      der geoeffnete Editor allein klassifiziert NICHT
  ok      Editor offen + explizite Initialisierung: angenommen
  ok      ... -> main
  ok      main + offener Editor darf den Broker starten
  ok      Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)
  ok      main OHNE offenen Editor darf NICHT starten
  ok      Rueckweg: derselbe Akt fuehrt von main zurueck
  ok      ... -> legacy
  ok      legacy darf nie starten - auch mit Editor nicht
  ok      legacy ist wieder audio-neutral
  ok      Sonde (NkPr) frisch: neutral trotz fester Produktklasse
  ok      Sonde (NkPr): gueltiger eigener Stand -> passive_probe
  ok      Sonde (NkPr): kein Brokerstart
  ok      Sonde (NkPr): auch mit offenem Editor kein Brokerstart
  ok      Sonde (NkPr): kein Akt macht sie zum Main
  ok      Sonde (NkAc): gueltiger eigener Stand -> active_probe
  ok      Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main

[2] Verdrahtet: EqCopilotProcessor (Gen)
  ok      Scannerlauf: klassifiziert NICHT (53.5)  [unclassified]
  ok      Scannerlauf: kein Brokerstart
  ok      Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)
  ok      Scannerlauf: Markierung meldet sich nicht hoerbar
  ok      Altprojekt 'sensor' -> legacy  [legacy]
  ok      legacy mit offenem Editor: kein Brokerstart
  ok      legacy ist 'immer passiv' - kein Sample gefaerbt
  ok      Altprojekt 'hub' -> main  [main]
  ok      main, aber Editor zu: noch kein Brokerstart
  ok      main + offener Editor: Brokerstart erlaubt
  ok      main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)
  ok      Vorbereitung: Rollenwahl aendert den Stand
  ok      Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)  [unclassified]
  ok      der geoeffnete Editor allein holt es auch nicht nach
  ok      Vorbereitung: der User waehlt die Rolle im offenen Editor
  ok      frische Instanz: Editor offen + Rollenwahl 'hub' -> main
  ok      frische Instanz als main: faerbt
  ok      Rueckweg: 'sensor' klassifiziert zurueck auf legacy
  ok      Rueckweg: der Schnitt auf neutral greift sofort
  ok      Vorbereitung: main
  ok      der fremde Stand kommt read-only zurueck
  ok      read-only -> zurueck auf unclassified
  ok      read-only: kein Brokerstart
  ok      read-only: wieder audio-neutral

LEBENSLAUF-TEST OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 31,87 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopPipeClientTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_rc_lib.dir\Release\EqCopPipeClientTest_rc_lib.lib
  EqCopPipeClientTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSunaNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_rc_lib.dir\Release\EqCopSunaNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopSunaNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
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
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopAnalysisGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_rc_lib.dir\Release\EqCopAnalysisGoldenTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopAnalysisGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaSuna_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_rc_lib.dir\Release\NakamaSuna_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaSuna.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\Nakama Suna_SharedCode.lib
  NakamaSuna_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaSuna_vst3_helper.exe
  NakamaSuna_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\x86_64-win\Nakama Suna.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaSuna_artefacts/Release/VST3/Nakama Suna.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>


---

## Kanon-Lauf - S6 Dirigent 2026-08-28 - Kanon auf 3122103 nach Nacharbeit

**Lauf:** 2026-08-28 18:53 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 29/29 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-28 18:53:48 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 3122103 S6 Nacharbeit T3: Nichtendlich-Fixture je Float-Traeger, Drift-Test belegt den flatc-Commit |
| Commit (voll) | 3122103bebf1160bb514696443c522b8c028db7a |
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
| `EqCopNullTest` | 2026-08-28 16:37:15 | `A8000FEF90307EEE` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-27 03:22:19 | `C89836581A6FFD48` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-28 16:37:19 | `08BFDD5F08117D89` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-27 02:25:05 | `ECA970E1DE7BA8CC` | frisch (Bau bestaetigt) |
| `EqCopSunaNullTest` | 2026-08-28 16:37:30 | `2EB04A41F6432C5F` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-28 16:37:32 | `C393C82F65A024A2` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-28 16:37:34 | `F12EA9D0EBF94F07` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-28 16:37:36 | `A5DF7B99893A3DD7` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-28 18:51:57 | `69413AB1A66C7B5E` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-28 16:37:45 | `FBD7BBAAD558392D` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-27 01:11:05 | `F56913243873A4FD` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-28 16:37:49 | `91F2C529928CB0FB` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-28 16:37:51 | `ED6BD5E4CAB0D9F5` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-28 18:45:57**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,67 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,62 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,99 s | [↓ A4](#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,07 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,67 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,19 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,25 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,84 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,25 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,20 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,42 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,89 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 24,37 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A19](#a19) |
| A15 | Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | [OK] Exit 0 | 0,04 s | [↓ A15](#a15) |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,04 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,06 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,30 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,19 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,43 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 1,99 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,67 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,62 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: als Main klassifiziert (§53.5)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: als Main klassifiziert (§53.5)
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: als Main klassifiziert (§53.5)
  ok      T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: als Main klassifiziert (§53.5)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: als Main klassifiziert (§53.5)
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: beide Instanzen als Main klassifiziert (§53.5)
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
  ok      T11: als Main klassifiziert (§53.5)
  ok      T11: Auftrag gebaut
  ok      T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)
  ok      T11: und die Markierung meldet sich nicht hoerbar
  ok      T11: mit gestopptem Transport ebenfalls kein Sample
  ok      T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,99 s

stdout:

```text

running 90 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test dto::tests::as_f64_liefert_fuer_jede_number_einen_wert ... ok
test dto::tests::bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege ... ok
test aggregat::tests::schreiben_erzeugt_haltbare_datei ... ok
test dto::tests::doppelter_schluessel_wird_nicht_still_ueberschrieben ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test dto::tests::geschachtelter_doppelter_schluessel_faellt_ebenfalls ... ok
test aggregat::tests::alte_temp_restdatei_blockiert_snapshot_nicht ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::hin_und_zurueck ... ok
test bindung::tests::fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test aggregat::tests::gleicher_zeitstempel_ueberschreibt_keinen_snapshot ... ok
test protokoll::tests::fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow ... ok
test protokoll::tests::gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt ... ok
test protokoll::tests::heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::hello_enum_und_audiofelder_werden_semantisch_begrenzt ... ok
test protokoll::tests::hello_textfelder_haben_codepunktgenaue_harte_grenzen ... ok
test protokoll::tests::messstand_guard_prueft_minima_endlichkeit_und_textcap ... ok
test protokoll::tests::option_felder_akzeptieren_explizites_null_wie_das_schema ... ok
test protokoll::tests::unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt ... ok
test bindung::tests::wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test bindung::tests::parallele_schreiber_hinterlassen_genau_einen_intakten_stand ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset ... ok
test register_tests::owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test aggregat::tests::parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id ... ok
test register_tests::stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge ... ok
test register_tests::unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab ... ok
test server::tests::fremdes_session_token_beendet_established_statt_acklos_zu_haengen ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::ungelesenes_welcome_blockiert_connection_ende_nicht ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung ... ok
test server::tests::sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten ... ok
test server::tests::v1_heartbeat_mit_measurement_ist_protokollbruch ... ok
test server::tests::zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen ... ok
test server::tests::zweites_hello_kann_die_established_bindung_nicht_umschreiben ... ok
test server::tests::bye_braucht_exakt_das_ausgegebene_session_token ... ok
test server::tests::ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz ... ok
test server::tests::connection_und_pipe_instanzzahl_bleiben_hart_begrenzt ... ok
test server::tests::ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist ... ok
test server::tests::acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup ... ok

test result: ok. 90 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.48s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 8 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test dto_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 8 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.14s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a4b"></a>
#### A4b | EqCopPipeClientTest

**Befehl:** `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | **Exitcode:** 0 | **Dauer:** 1,07 s

stdout:

```text
PIPECLIENT-LIFECYCLE-TEST
  ok      Fake-Pipe fuer Welcome-Stopp angelegt
  ok      stop() bricht blockiertes Welcome-Lesen begrenzt ab  [0 ms]
  ok      SQOS begrenzt den Peer auf SecurityIdentification  [1]
  ok      Fake-Pipe fuer ACK-Stopp angelegt
  ok      stop() bricht blockiertes ACK-Lesen begrenzt ab  [0 ms]
  ok      Reconnect-Fake-Pipe angelegt
  ok      Reconnect waehrend Welcome verliert keine Konfigurationsgeneration
  ok      ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt
  ok      Welcome mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ACK mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers  [325 ms]
  ok      u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze
  ok      200 konkurrierende start/stop-Paare ohne joinable-Leak
PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler
```

stderr:

_(leer)_

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 1,67 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
52 Definitionen, 17 Nachrichtenfamilien, 188 Fixtures
Pruefungen: 284 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 49  
  const                     8 / 8    HART
  enum                     10 / 10   HART
  exclusiveMinimum          2 / 2   
  items                     3 / 8   
  maxItems                  3 / 8   
  maxLength                 4 / 20  
  maxProperties             2 / 3   
  maximum                   6 / 18  
  minItems                  2 / 8   
  minLength                 5 / 14  
  minimum                   8 / 48  
  oneOf                     7 / 8   
  pattern                   4 / 9   
  required                 40 / 229 
  type                     11 / 165 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:933: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,19 s

stdout:

```text
42 gueltige, 146 ungueltige Fixtures
  bytegleich: 190 Dateien, MANIFEST sha256=bedc885d7e2ca10aebadee1d41b04187ed61b2792649804d04600d5dbb96cbee
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  flatc-Commit: 7e163021e59cca4f8e1e35a7c828b5c6b7915953  (CMake-Beleg: C:\Users\phili\Projekte\Nakama\eq-copilot\build\nakama-flatc-commit-Release.txt)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,84 s

stdout:

```text
9 gueltige, 53 ungueltige Fixtures
  bytegleich: 63 Dateien, MANIFEST sha256=b6be5564a60ba6e524ea174807c253c7ad7ee0e810bef2260cc6eba52f1e8282
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,20 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,42 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (737942 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7105024 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      die Lib wurde mit der heutigen Definemenge gebaut (15 aus der Projektdatei)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.bundle = 'Nakama Probeeq.vst3' NICHT
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt active-probe.produktname = 'Nakama Probeeq' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.bundle = 'Nakama Suna.vst3' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT
  ok      NakamaKern.lib traegt passive-probe.produktname = 'Nakama Suna' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die 5 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

26 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 0,89 s

stdout:

```text
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag  [3 vs 3; identity=ok]
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID im Installer-Manifest
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest
  ok      faellt am verdorbenen Manifest: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt am verdorbenen Manifest: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt am verdorbenen Manifest: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt am verdorbenen Manifest: Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt am verdorbenen Manifest: kein Viercode, keine Class-ID im Installer-Manifest
  ok      faellt am verdorbenen Manifest: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt am verdorbenen Manifest: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt am verdorbenen Manifest: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt am verdorbenen Manifest: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt am verdorbenen Manifest: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt am verdorbenen Manifest: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt am verdorbenen Manifest: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[4] Auslieferungsstand
  hinweis nicht ausgeliefert - 4 Artefakt(e) ohne Hash: main, passive-probe, active-probe, eqcop-broker.exe
          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

38 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 24,37 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
  ok      kanonische VST3-Produktpolicy steht unveraendert im Original
  ok      kanonische Broker-Produktpolicy steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle
      - $erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
      + $erlaubteVst3Basis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-zci425wn\ziel\VST3'
      - $erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
      + $erlaubteBrokerBasis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-zci425wn\ziel\programme\evenacadia\Nakama'

[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
  ok      Bundle ohne erwartete innere Payload wird fail-closed verweigert  [Exit 1]
  ok      Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0: phili\AppData\Local\Temp\nakama-gegenpfad-zci425wn\ziel\VST3\EQ-Copilot.vst3]
  ok      Nakama Suna installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-zci425wn\ziel\VST3\Nakama Suna.vst3]
  ok      Nakama Probeeq installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-zci425wn\ziel\VST3\Nakama Probeeq.vst3]
  ok      eqcop-broker.exe installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-zci425wn\ziel\programme\evenacadia\Nakama\eqcop-broker.exe]
INSTALLATION OK
]
  ok      geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg
  ok      jede Sicherung ist relativ an backups/<transaktions-id> gebunden
  ok      Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet
  ok      beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM
  ok      wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      passive-probe: Bundle traegt moduleinfo.json  [Nakama Suna.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[2b] Aktiver Recovery-Anker wird nicht still superseded
  ok      geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie  [Exit 1]
  ok      VORBEREITET verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      VORBEREITET: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      KOMPENSATION verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      KOMPENSATION: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      ERROR_TEILSTAND verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      ERROR_TEILSTAND: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      RUECKWEG_AKTIV verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      RUECKWEG_AKTIV: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded  [Exit 0]
  ok      Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren
  ok      zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle vier Artefakte melden `aktuell`  [4x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben
  ok      frische Transaktion friert den nun bekannten historischen Stand ein
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht
  ok      passive-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Suna.vst3]
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      das im Journal belegte Broker-Ziel ist weg
  ok      unbelegte Elternverzeichnisse bleiben unangetastet
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert
  ok      spaeter Installationsfehler endet laut und nicht als Teilerfolg  [Exit 1]
  ok      Journal bestaetigt die vollstaendige Kompensation  [ERROR_RUECKGEROLLT]
  ok      Main ist nach der Kompensation bytegleich zum Vorzustand
  ok      passive-probe: kein Teilstand nach spaetem Fehler
  ok      active-probe: kein Teilstand nach spaetem Fehler
  ok      der fremde Blocker wurde nicht angetastet
  ok      vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei
  ok      ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden  [Exit 0]
  ok      nach Fehler-Supersede bleibt nur die aktive Backup-Generation

[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig
  ok      Ausgangsinstallation fuer den Hard-Crash-Zwischenstand
  ok      VORBEREITET + mutation_begonnen restauriert statt Drift zu melden  [Exit 0]
  ok      der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt
  ok      Ausgangsinstallation fuer einen unterbrochenen inversen Pfad
  ok      RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort  [Exit 0]
  ok      auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand

[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen
  ok      Ausgangsinstallation fuer Journal-Missbrauchstests
  ok      manipulierte Repo-Diagnose ist keine Rueckwegautoritaet  [Exit 0]
  ok      fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben  [Exit 1]
  ok      fremder Loeschpfad im Journal wird nicht entfernt  [Exit 1]
  ok      Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen  [Exit 1]

[11] Geschuetzte Autoritaet, Backup- und Zieldrift
  ok      oeffentliches Journal allein autorisiert keinen Rueckweg  [Exit 1]
  ok      Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash  [Exit 1]
  ok      frische Ausgangsinstallation fuer Zieldrift
  ok      veraendertes Installationsziel wird nicht ueberschrieben  [Exit 1]
  ok      Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet

[12] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2
  ok      Runtime verwirft eine Bundle-Zielkollision  [Exit 1]
  ok      Runtime bindet Identity typstreng an State-Schema 2  [Exit 1]
  ok      Runtime verwirft kanonisches Broker-Zieltraversal  [Exit 1]

80 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a19"></a>
#### A19 | erzeuge_bandgitter_header.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen (204 Zeilen).
    Evidenz nakama_1_24_oct_30_18k_v1: 221 Baender, 222 Kanten
    Live    nakama_log64_v1: 64 Gruppen, lueckenlose Partition der 221
```

stderr:

_(leer)_

<a id="a15"></a>
#### A15 | EqCopSunaNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse passive_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [passive_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [172 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [passive_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [172 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse active_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [active_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [3439 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [3439 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes Ziel im Manifest hat hier eine Zeile  [3 vs 3]
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest
  ok      CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle: COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle: der Viercode steht nicht literal im Zielblock  [Eqcp]
  ok      CMake-Quelle: der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      CMake-Quelle: der Produktname steht nicht literal im Zielblock
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden
  ok      CMake: Defineblock der Sondenfunktion gefunden
  ok      CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      passive-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkPr]
  ok      passive-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Suna]
  ok      passive-probe: NakamaSuna entsteht ueber nakama_sonde_ziel()
  ok      passive-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaSuna    passive-probe NAKAMA_SONDE_PASSIV)]
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 6:54:14pm vs 27 Aug 2026 2:27:21am]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      passive-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\Resources\moduleinfo.json]
  ok      passive-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 6:54:17pm vs 27 Aug 2026 2:27:21am]
  ok      passive-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      passive-probe: Produktname wie im Manifest  [Nakama Suna]
  ok      passive-probe: Vendor wie im Manifest  [evenacadia]
  ok      passive-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      passive-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: Unterkategorien wie im Manifest
  ok      passive-probe: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 6:54:19pm vs 27 Aug 2026 2:27:21am]
  ok      active-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      active-probe: Produktname wie im Manifest  [Nakama Probeeq]
  ok      active-probe: Vendor wie im Manifest  [evenacadia]
  ok      active-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      active-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B4163]
  ok      active-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B4163]
  ok      active-probe: Unterkategorien wie im Manifest
  ok      active-probe: keine fremde Ziel-CID im Bundle
  ok      Viercode ist vier Zeichen lang: main.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      Viercode ist vier Zeichen lang: passive-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      Viercode ist vier Zeichen lang: active-probe.plugin_code  [4 Zeichen]
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
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 114 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,30 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      16 Kombinationen aus Klasse x Messposition wie §2.2 - post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)  [16]
  ok      == Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle
  ok      21 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [21]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser
  ok      gueltiger Praefix mit Suffix wird vollstaendig ignoriert
  ok      abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen
  ok      deklarierte Riesenvariante wird vor JUCE-Allokation verworfen
  ok      zu tiefer ValueTree wird begrenzt und ignoriert
  ok      zu tief geschachteltes Variantenarray wird begrenzt und ignoriert
  ok      State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen
  ok      ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel
  ok      63 verschachtelte Arrays plus Blatt passieren und laden
  ok      65.536 Eintraege in einer Sammlung passieren den Byte-Riegel
  ok      65.537 Eintraege in einer Sammlung werden verworfen
  ok      262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel
  ok      262.145 Eintraege ueber mehrere Sammlungen werden verworfen
  ok      Teststate trifft die schreibbare Bytegrenze exakt  [16773120]
  ok      Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773848]
  ok      State ohne konkreten Writer-Headroom bleibt read-only bytegleich
  ok      vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [9]
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [10]
  ok      additive Binaer- und Array-Properties bleiben bytegleich lesbar
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [1513df53de9f48e69d65e84fd4129b44]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [4b9419f46d534e34ad316efa0f402c04]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 153 Pruefungen ok, 0 Fehler
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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260828-185509.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (15888 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,19 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [102 Faelle]
[ok]   Falltabelle hat Substanz  [102 Faelle]
[ok]   Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (62 Fixtures)
[ok]   Binaerkorpus hat Substanz  [62 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-auf-2hoch53.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740992.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-nahe-2hoch53-ab.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740991.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-unter-2hoch53-auf.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 4503599627370495.9]
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-kleiner-bruch-rundet-auf-eins.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 1.00000000000000001]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nan-token.json  [unbekanntes Literal NaN an Position 1090]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (188 Fixtures)
[ok]   Korpus hat Substanz  [188 Fixtures]
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
63 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 3,43 s

stdout:

```text
== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==
Produktionsstrom GenStrom: 1 Tap(s), 131072 Frames Ring, 2048 Deskriptoren, max 16384 Frames je Block.

== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==
  ok      drei Bloecke, drei Deskriptoren  [3]
  ok      jeder Frame kommt bitgleich und an derselben Stelle heraus
  ok      Stempel: lokaler Strom und Projektzeit stimmen je Block
  ok      nach dem Leeren ist die Queue leer
  ok      kein Verlust bei Normallast
== B - Wrap: der Ring laeuft mehrfach um ==
  ok      der Ring ist wirklich mehrfach umgelaufen  [11 Umlaeufe]
  ok      ueber jeden Wrap hinweg bitgleicher Inhalt
  ok      kein Frame verloren, keiner doppelt  [12000/12000]
== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==
  ok      vier Bloecke passen, danach ist Schluss  [4]
  ok      beide abgewiesenen Bloecke sind gezaehlt  [2]
  ok      verlorene Frames = 2 x 256 (keine Teilmenge)  [512]
  ok      ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock
  ok      der erste Block nach dem Verlust traegt kFlagLueckeDavor
  ok      und beginnt ein neues continuity_segment  [1]
== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==
  ok      genau `deskriptorPlaetze` Bloecke passen  [8]
  ok      die vier ueberzaehligen sind gezaehlt  [4]
  ok      und genau so viele stehen zum Lesen bereit  [8]
== E - Oversize: ein Block ueber der Slotkapazitaet ==
  ok      ein Block ueber maxBlockFrames wird als GANZES abgewiesen
  ok      und als Oversize gezaehlt, nicht als Ueberlauf  [1/0]
  ok      der Ueberlaufzaehler bleibt sauber
  ok      nichts Halbes im Ring
  ok      die verlorene Zeit ist vollstaendig gemeldet  [257]
  ok      genau maxBlockFrames passt noch (Kante, nicht Ausschluss)
== F - Flush (numSamples == 0) ist kein Verlust ==
  ok      ein Flush-Block wird nicht eingestellt
  ok      und zaehlt NICHT als Verlust - er traegt keine Audiozeit
  ok      der Folgeblock geht normal
  ok      und traegt keine Luecke
== G - Mono und Mehrfach-Tap ==
  ok      Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)
  ok      und der Deskriptor sagt kanaele == 1
  ok      Drei-Tap-Layout: Block angenommen
  ok      das Bitset meldet genau die belegten Taps  [3]
  ok      jeder Tap liegt getrennt und richtig; der inaktive liefert false
  ok      eine falsche Tapzahl wird abgewiesen statt halb kopiert
  ok      ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor
== H - Ein-Block-Quarantaene ==
  ok      5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene  [4]
  ok      und die Quarantaene haelt ihn wirklich
  ok      kein Bruch bei lueckenlosem Strom
  ok      der Seek ist als Bruch erkannt  [1]
  ok      genau EIN Block faellt an der Grenze - der Strom laeuft weiter
  ok      der Block NACH der Grenze ist schon wieder in Quarantaene
  ok      Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)
  ok      stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      Vorlauf ohne Bruch
  ok      Transportkante play->stop ist ein Bruch  [1]
  ok      ohne Hostkontext traegt der lokale monotone Strom (§53.6)
  ok      und versiegelt normal  [3]
  ok      Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung
  ok      negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)
== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==
  20 000 Bloecke, Groessen 1..288: angenommen 17809, Ueberlauf-Drops 0, Oversize-Drops 2191, groesster Block 256, versiegelt 15872, Brueche 1936
  ok      jeder gelesene Frame steht an der Stelle, die sein Stempel nennt
  ok      es gab nie einen Block mit sampleCount 0
  ok      die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt  [2191]
  ok      kein angenommener Block war je groesser als die Slotkapazitaet  [256]
  ok      es wurde wirklich gelesen  [2288127 Frames]
== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==
  ok      18 Blockgroessen von 1 bis 16 384: kein Sample veraendert
  ok      0 Samples Latenz  [0]
  ok      kein Tail
  angenommen 18, groesster Block 16384, Ueberlauf 0, Oversize 0
== K - verdrahtet: Oversize aendert das Audio nicht ==
  ok      ein Block ueber der Slotkapazitaet laeuft unveraendert durch
  ok      und ist als Oversize-Drop der ANALYSE gezaehlt  [1]
  ok      der abgewiesene Oversize-Block erscheint nicht im Projektfenster
  ok      der naechste angenommene Lueckenblock beginnt das Fenster exakt neu  [16385..16449]
  ok      der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben  [0]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3857 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
== M - verdrahtet: die Messung bekommt den Strom wirklich ==
  eingespeist 287744, analysiert 287232, Drops 0, versiegelt 561, Quarantaene-Verwuerfe 0, Brueche 0
  ok      bei Normallast kein einziger Drop
  ok      und kein Kontinuitaetsbruch (lueckenloser Transport)
  ok      die Engine sieht den ganzen Strom bis auf den Block in Quarantaene  [512 Frames offen]
  ok      Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)  [-8.684240]
  ok      framesDropped im Stats-Snapshot kommt aus derselben Quelle
== N - Gegenpfad: prepareToPlay mitten im Betrieb ==
  ok      auch der erste Block nach prepareToPlay alloziert nicht  [0]
  veraltete Bloecke (alter Anlauf, nicht analysiert): 40
  ok      ein Neuanlauf ist kein Oversize
== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==
  ok      der Anlauf steigt sofort, nicht erst beim naechsten Audioblock  [0 -> 1]
  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt 3, als aktuell durchgelassen 0
  ok      alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar  [3/3]
  ok      der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt
  ok      Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell
  ok      und er traegt die Luecke, die der Neuanlauf gerissen hat
  ok      der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut
  ok      zwei Neuanlaeufe hintereinander gehen beide nicht verloren  [1 -> 3]
  ok      zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration
  ok      und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste
  ok      Nullframe konsumiert die wartende Generation nicht
  ok      der naechste echte Block uebernimmt sie weiterhin
== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==
  ok      Seek um 10 s bei gestopptem Transport ist EIN Bruch  [1]
  ok      Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)  [0]
  ok      und bei laufendem Transport ist der Seek weiterhin ein Bruch  [1]
  ok      bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet  [0]
== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==
  ok      nach Start ist nur die leichte Publikation sofort faellig
  ok      10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus
  ok      vor 250 ms bleibt Gating/Kandidaten gesperrt
  ok      bei 250 ms ist genau die schwere Runde faellig
  ok      dieselbe Deadline kann nicht zweimal verbraucht werden
  ok      nach Pause genau eine Runde, kein Catch-up-Sturm
  ok      Reset/Generation startet die 250-ms-Deadline neu
== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==
  ok      Test hat echten Queue-Rueckstau hergestellt  [257 Bloecke bis zum ersten Drop]
  ok      unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms  [1 Runden in 600 ms]
  ok      neue Generation erbt keinen Batch-beschleunigten Schwertakt  [0 Runden in den ersten 100 ms]
  ok      Frame-Leser wird unter Rueckstau explizit vorgelassen  [2 ms]
  ok      Reset-Steuerer wird unter Rueckstau explizit vorgelassen  [2 ms]
  ok      Stop/Join bleibt auch mit Queue-Rest begrenzt  [0 ms]
== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==
  ok      Gegenprobe: erste Generation hat ihr eigenes Projektfenster
  ok      Testbarriere liegt deterministisch im laufenden Audiocallback
  ok      der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster  [10000..10064]
  ok      die Generationskante ist kein Seek innerhalb des alten Fensters  [0]
== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==
  ok      Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine
  ok      nichtendliche Hostrate wird fail-closed auf 0 gespiegelt
  ok      M1 verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      FeatureEngine verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung
  ok      eine folgende gueltige Generation aktiviert beide Engines wieder

114 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Feingitter: 10001 Bins a 0.01 LU ueber [-70.00, 30.01) LUFS.
Oberband:   3070 Bins a 1.00 LU ueber [30.01, 3100.01) LUFS - lautheit(DBL_MAX) = 3081,86.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: kein Block faellt durch das Raster  [0]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: kein Block faellt durch das Raster  [0]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): kein Block faellt durch das Raster  [0]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: kein Block faellt durch das Raster  [0]
  ok      Musik + echte Stille: LUFS-I innerhalb ±0,1 LU  [akku=-8.281644 ref=-8.281644 d=0.000000000]
  ok      Musik + echte Stille: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Musik + echte Stille: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000027]
  ok      Musik + echte Stille: Kurz-LUFS Gueltigkeit gleich
  ok      Musik + echte Stille: Kurz-LUFS BITGLEICH  [akku=-12.451912590513381 ref=-12.451912590513381]
== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==
  Sweep: 201 Laeufe, groesste Abweichung 2.781360 LU bei Faktor 1.000000, groesste gemeldete Schranke 2.781412 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen (Eingang aendert sich um 1e-5): 2.781403 LU
  ok      Sweep vollstaendig gefahren  [201/201]
  ok      Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)  [sonst pruefte D nichts; schranke=2.781412]
  ok      die selbstgemeldete Schranke deckt JEDEN Lauf  [201/201]
  ok      wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU  [76/76]
  ok      die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)  [maxD=2.781360 refSprung=2.781403]
  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)
== E - fixed memory: eine Million Zellen ==
  Akku:     0 Allokationen, LUFS-I -8.386510, Schranke 0.000000000 LU
  Referenz: 36 Allokationen, LUFS-I -8.386510
  ok      1 000 000 Zellen: 0 Allokationen im Akku  [0]
  ok      Gegenprobe: die alte Rechnung alloziert sehr wohl  [36]
  ok      1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU  [d=0.000000000]
  Fester Bedarf laut speicherBytes(): 157122 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      Kurz-LUFS meldet einen nichtendlichen Ring nie als gueltig
  ok      ueber dem Gitter: EIN Pegel bleibt wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      Gegenprobe: der Korpus liegt wirklich im Oberband  [197]
  ok      und kein Block faellt durch das Raster
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer
== G - Oberband: zwei Pegel ueber dem Feingitter ==
  G1: akku=49.302611 ref=49.302611 schranke=0.000000000 ueberGitter=1997
  ok      G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)  [1997]
  ok      zwei Pegel ueber dem Gitter (T2-1): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      zwei Pegel ueber dem Gitter (T2-1): kein Block faellt durch das Raster  [0]
  ok      zwei Pegel ueber dem Gitter (T2-1): LUFS-I innerhalb ±0,1 LU  [akku=49.302611 ref=49.302611 d=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS Gueltigkeit gleich
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS BITGLEICH  [akku=32.319299956639810 ref=32.319299956639810]
  ok      Naht, Gamma_r im Oberband: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Oberband: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Oberband: LUFS-I innerhalb ±0,1 LU  [akku=49.302481 ref=49.302481 d=0.000000000]
  ok      Naht, Gamma_r im Oberband: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Oberband: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  ok      Naht, Gamma_r im Feingitter: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Feingitter: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Feingitter: LUFS-I innerhalb ±0,1 LU  [akku=49.243363 ref=49.243363 d=0.000000000]
  ok      Naht, Gamma_r im Feingitter: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Feingitter: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  G3-Sweep: 201 Laeufe, groesste Abweichung 2.781577 LU bei Faktor 1.001000, groesste gemeldete Schranke 2.781577 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: 2.781401 LU
  ok      G3: Sweep vollstaendig gefahren  [201/201]
  ok      G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)  [1997]
  ok      G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)  [sonst pruefte G3 nichts; schranke=2.781577]
  ok      G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband  [201/201]
  ok      G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz  [maxD=2.781577 refSprung=2.781401]
  ok      G3: kein Block faellt durch das Raster  [0]
  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - 1000 Bloecke kippen gemeinsam die Gateseite.)
  G4: LUFS-I 3038.851, Schranke 0.000000000, ueberGitter 197, ueberOberband 0
  ok      G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert  [3038.851]
  ok      G4: nichts wird faelschlich als nicht-endlich gezaehlt  [0]
  ok      G4: auch am aeussersten Rand faellt kein Block durch das Raster  [0]
  ok      G4: und die Schranke bleibt endlich  [0.000000000]

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
109 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b5"></a>
#### B5 | EqCopAnalysisGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 1,99 s

stdout:

```text
== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==
Gate: "Drop/Seek/Loop trennt jedes offene Fenster."
Stufen: Bass 16384 (Hop 8192), Haupt 4096 (Hop 2048), Trennung bei 200 Hz.

== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==
[ok]   Evidenzgitter: band_anzahl == kEvidenzBaender  [221]
[ok]   Livegitter: band_anzahl == kLiveBaender  [64]
[ok]   Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)  [0 Abweichungen]
[ok]   Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)  [0 Abweichungen]
[ok]   Live-Kanten BITGLEICH zur Fixture (65 Werte)  [0 Abweichungen]
[ok]   Live-Mitten BITGLEICH zur Fixture (64 Werte)  [0 Abweichungen]
[ok]   Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal  [0 Abweichungen, Ende bei 221]
[ok]   Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank  [0 Abweichungen]

== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==
[ok]   q_db_0p1_i16: Skalierung stimmt mit der Fixture ueberein  [10]
[ok]   q_db_0p1_i16: alle 23 Vektoren treffen
[ok]   q_db_0p01_i16: Skalierung stimmt mit der Fixture ueberein  [100]
[ok]   q_db_0p01_i16: alle 23 Vektoren treffen
[ok]   float32: alle 15 Vektoren treffen bitgenau
[ok]   Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0
[ok]   halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl
[ok]   und negativ genauso (-0,05 -> -1)

== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==
[ok]   Band i sitzt in Byte i/8, Bit i%8 (LSB-first)  [Byte0=81 Byte1=1]
[ok]   ein Bit loeschen laesst die Nachbarn stehen
[ok]   Lesen und Schreiben sind dieselbe Adressierung
[ok]   221 Baender brauchen 28 Bitmapbytes  [28]
[ok]   die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)  [letztes Byte = 1f]

== D - FFT: gegen analytisch bekannte Ergebnisse ==
[ok]   Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin  [100.0000 %]
[ok]   Parseval haelt (Zeit- und Frequenzenergie gleich)  [rel. Fehler 0.0000000000000002]
[ok]   Gleichanteil sitzt vollstaendig in Bin 0  [Bin0 = 1048576.0, Rest = 0.00000000000000000000]

== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==
[ok]   RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander  [max 0.04312 dB bei 20000.0 Hz]
       (gemessene Groesstabweichung, zur Ablesung: 0.043120 dB)
[ok]   Hochpass sperrt den Gleichanteil vollstaendig  [|H(0)|Â² = 0.000000000000000000000000]

== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==
[ok]   die Sequenz zaehlt ab 1 aufwaerts  [6]
[ok]   mit gueltiger Projektzeit ist die Zeitbasis project_samples
[ok]   und project_sample_start ist gesetzt
[ok]   Gueltigkeitsbit project_time steht
[ok]   der Frame traegt seine Metrikversion  [20260823]
[ok]   und die Samplerate
[ok]   Frame-Stempel spannt alle integrierten Hostbloecke auf  [0 + 5120]
[ok]   Frame-Stempel umfasst den ueberlappenden FFT-Support  [2048 + 8192]
[ok]   ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck
[ok]   Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen
[ok]   fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden
[ok]   Evidenzframe vereinigt Haupt- und aelteren Basssupport  [0 + 30720, 196 Baender]
[ok]   stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen
[ok]   stehende FL-Teilstuecke erfinden kein Projektintervall
[ok]   ueberlaufende Host-Projektspanne wird konservativ lokal publiziert
[ok]   Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block  [104608 + 512]
[ok]   ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (Â§32.3)
[ok]   und dann steht WEDER das Bit NOCH ein Startwert da
[ok]   alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet  [0x7f]
[ok]   eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (Â§32.3)
[ok]   Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer
[ok]   continuous_time_samples kommt mit demselben Frameanker durch
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: NaN
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Inf
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Ende vor Start
[ok]   ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich

== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==
[ok]   G1 Drop (kFlagLueckeDavor): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G1 Drop (kFlagLueckeDavor): Ursache ist lokaleLuecke  [Zaehler 1]
[ok]   G1 Drop (kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche (§32.3)  [Epochen 0 Segmente 1]
[ok]   G2 Seek waehrend Wiedergabe: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G2 Seek waehrend Wiedergabe: Ursache ist zeitSprung  [Zaehler 1]
[ok]   G2 Seek waehrend Wiedergabe: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G3: bei gestopptem Transport laufen die Fenster normal weiter  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)  [0 Epochenwechsel]
[ok]   G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G3: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G3: und sie heisst zeitSprung, nicht lokaleLuecke
[ok]   G4: aktive Schleife allein trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4 Loop-Wrap: jedes Fenster getrennt  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G4: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G4: und die Ursache heisst loopWrap, nicht zeitSprung
[ok]   G5: Schleife ohne erreichbare Grenze trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, weil der Straddle-Block auch kein neues beginnt  [Bass=0 Haupt=0 Zelle=0 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G5: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet
[ok]   G5: mit eigener Ursache moeglicherStraddle
[ok]   G6 Transportkante (Play -> Stop): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G6 Transportkante (Play -> Stop): Ursache ist transportKante  [Zaehler 1]
[ok]   G6 Transportkante (Play -> Stop): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G7 Sampleratewechsel: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G7 Sampleratewechsel: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G7 Sampleratewechsel: Ursache ist sampleratewechsel  [Zaehler 1]
[ok]   G7 Sampleratewechsel: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G8 Neuanlauf (prepareToPlay): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G8 Neuanlauf (prepareToPlay): Ursache ist neuanlauf  [Zaehler 1]
[ok]   G8 Neuanlauf (prepareToPlay): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): Ursache ist beweislageWechsel  [Zaehler 1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G11: im Betrieb traegt die K-Kette einen Filterzustand  [1.392229]
[ok]   G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame  [290 / 290]
[ok]   G11: beide Laeufe liefern eine Kurzzeitlautheit
[ok]   G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - der Filternachklang ueberbrueckt nicht  [A=-60.034744263 B=-60.034744263]
[ok]   G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)  [0 Trennungen]
[ok]   G10: die Fenster laufen dabei weiter  [Bass=9224 Haupt=3080 Zelle=264 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G12: vor der Grenze tragen BEIDE Bandakkus den Ton  [Live 149 / Evidenz 149 Baender]
[ok]   G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber die Grenze weiter (Entscheid Â§10.1)  [liveSamples=0]
[ok]   G12: und der Frame traegt die NEUE Epoche  [1]
[ok]   G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird also wirklich geprueft
[ok]   G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)  [0 gesetzte Baender]
[ok]   G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter  [0 gesetzte Baender]
[ok]   G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit  [nicht gesetzt]
[ok]   G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden  [1192 Band-Fensterbeitraege]
[ok]   G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren  [120 von 120]
[ok]   G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor (Pruefer-Messung: 80 / 40 / 43)  [0 mit Live-Baendern, 0 mit Evidenz, 0 mit Aktivitaet]

== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) ==
[ok]   G13.0: zwei frische Frames sind gleich - der Vergleich sagt nicht pauschal ungleich
[ok]   G13.0: ein einziger geaenderter Skalar macht sie ungleich
[ok]   G13.0: ein einziger geaenderter Bandwert ebenso
[ok]   G13.0: und ein einziges Bitmapbit ebenso
[ok]   G13.0: und ein Feld im verschachtelten Transportstempel - der Vergleich reicht bis dorthin
[ok]   G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen vorgefuehrt
[ok]   G13a lokaleLuecke [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13a lokaleLuecke [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13a lokaleLuecke [Zwilling]: beide haben die Grenze lokaleLuecke gezogen  [A 1 / B 1]
[ok]   G13a lokaleLuecke [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13a lokaleLuecke [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13a lokaleLuecke [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13a lokaleLuecke [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13a lokaleLuecke [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13b zeitSprung [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13b zeitSprung [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13b zeitSprung [Zwilling]: beide haben die Grenze zeitSprung gezogen  [A 1 / B 1]
[ok]   G13b zeitSprung [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13b zeitSprung [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13b zeitSprung [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13b zeitSprung [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13b zeitSprung [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13c transportKante [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13c transportKante [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13c transportKante [Zwilling]: beide haben die Grenze transportKante gezogen  [A 1 / B 1]
[ok]   G13c transportKante [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13c transportKante [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13c transportKante [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13c transportKante [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13c transportKante [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13d sampleratewechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13d sampleratewechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13d sampleratewechsel [Zwilling]: beide haben die Grenze sampleratewechsel gezogen  [A 1 / B 1]
[ok]   G13d sampleratewechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13d sampleratewechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13d sampleratewechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13d sampleratewechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13d sampleratewechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13e neuanlauf [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13e neuanlauf [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13e neuanlauf [Zwilling]: beide haben die Grenze neuanlauf gezogen  [A 1 / B 1]
[ok]   G13e neuanlauf [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13e neuanlauf [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13e neuanlauf [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13e neuanlauf [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13e neuanlauf [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13f beweislageWechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13f beweislageWechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13f beweislageWechsel [Zwilling]: beide haben die Grenze beweislageWechsel gezogen  [A 1 / B 1]
[ok]   G13f beweislageWechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13f beweislageWechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13f beweislageWechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13f beweislageWechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13f beweislageWechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]

== H - NAK-29: bedingte Feldpflichten des Transportstempels ==
[ok]   ein sauberer Stempel kommt durch
[ok]   Fall 1: project_samples + Bit, aber KEIN project_sample_start
[ok]   Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu Â§32.3)
[ok]   Fall 3: bounds_valid ohne start_ppq/end_ppq
[ok]   Fall 4: derivation=validated_block_mapping ohne bounds_valid
[ok]   Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht
[ok]   Fall 6: continuous_time-Bit ohne continuous_time_samples
[ok]   Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus
[ok]   Fall 7: sample_count bleibt unter der Vertragsobergrenze
[ok]   Fall 7: der Feature-Erzeuger publiziert keinen Leerframe
[ok]   der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel entsteht im normalen, gedeckelten Betrieb  [0]
[ok]   abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich  [1]

== I - Ereignisse: kein Fluss ueber eine Grenze ==
[ok]   der spektrale Fluss hat im Betrieb einen Vorgaenger
[ok]   nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss einen Onset, den es nie gab
[ok]   jedes Ereignis traegt die Epoche, in der es gesehen wurde  [1 Ereignis(se)]
[ok]   der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, nicht behauptet  [64 im Ring, 1 verworfen, nach 1107 Bloecken]
[ok]   und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (Â§48.1: ein Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)  [64 / 64]
[ok]   drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste  [aeltestes vorher 38912, jetzt 47104]
[ok]   und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf
[ok]   Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock
[ok]   stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit  [1 Ereignis(se) nach 84 Bloecken]

== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==
[ok]   LUFS-S ist nach 4 s gesetzt
[ok]   LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (Â§39.3)  [gemessen -6.0620 erwartet -6.0620 d=0.00000]
[ok]   Peak trifft die Amplitude  [-6.021 dBFS]
[ok]   Crest eines Sinus ist 3,01 dB  [3.0091 dB]
[ok]   L==R ergibt Korrelation 1  [1.000000]
[ok]   und Breite 0 (kein Seitenanteil)  [0.000000000000]
[ok]   das Live-Band um 997 Hz ist gueltig  [Band 35]
[ok]   und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal

== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==
[ok]   die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen  [28 Bloecke]
[ok]   und mindestens einen Frame gebaut  [3 Frames]
[ok]   process_context_present kommt aus der BRUECKE durch (Â§32.3)
[ok]   recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen
[ok]   continuous_time_samples kommt durch
[ok]   die Schleifengrenzen kommen durch, samt `active`
[ok]   beide Presentation-Latencies kommen durch  [3924 / 4410]
[ok]   ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits  [0x7f]
[ok]   Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits  [0x3]
[ok]   insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`
[ok]   Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen
[ok]   Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt
[ok]   nichtendliche Sampleraten initialisieren keine Analyseengine

== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==
[ok]   L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt
[ok]   L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen  [0]
[ok]   L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, und zwar nur dort  [1 Abweichung(en), Zeile 26]
[ok]   L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'
[ok]   L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts  [0 gesetzte Baender]
[ok]   L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte  [3]
[ok]   L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung  [1, 3]
[ok]   L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'
[ok]   L6: ein echter Grund wird gezaehlt  [1]
[ok]   L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen  [gelesen: 0]
[ok]   L6: und `keine` ebenso - getrennt wird nie ohne Grund

== M - Auskunft ueber die Grenzen der Messung ==
[ok]   die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da  [erstes messbares Band 2 (32.6 Hz)]
[ok]   bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz  [erstes Band ueber der Kappe 221]
[ok]   bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab  [erstes Band ueber der Kappe 202 (10525.6 Hz)]
[ok]   und sie sitzt genau am ersten Band, dessen Oberkante 0,95Â·Nyquist ueberschreitet  [10374.7 .. 10678.7 Hz gegen 10473.8 Hz]
[ok]   kein Live-Band vollstaendig ueber der Kappe traegt einen Wert  [0 Baender]
[ok]   Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler

Ergebnis: 235 bestanden, 0 Fehler.
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  ok      frisch: unclassified  [unclassified]
  ok      frisch: neutral
  ok      frisch: keine Audio-Ausnahme (audio-neutral)
  ok      frisch: kein Brokerstart
  ok      Schema-1 'sensor' -> legacy  [legacy]
  ok      Schema-1 'sensor': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'pre' -> legacy  [legacy]
  ok      Schema-1 'pre': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'post' -> legacy  [legacy]
  ok      Schema-1 'post': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'hub' -> main  [main]
  ok      Schema-1 'hub': Audio-Ausnahme moeglich
  ok      Schema-2 main (Kind-Matrix bestaetigt) -> main
  ok      Schema-2 legacy -> legacy
  ok      Vorbereitung: erst main
  ok      unbekanntes Major danach -> zurueck auf unclassified
  ok      Muell (ignoriert) aendert nichts - main bleibt main
  ok      Muell auf einen frischen Automaten klassifiziert nicht
  ok      explizite Initialisierung OHNE Editor: abgelehnt
  ok      ... und der Automat bleibt neutral
  ok      der geoeffnete Editor allein klassifiziert NICHT
  ok      Editor offen + explizite Initialisierung: angenommen
  ok      ... -> main
  ok      main + offener Editor darf den Broker starten
  ok      Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)
  ok      main OHNE offenen Editor darf NICHT starten
  ok      Rueckweg: derselbe Akt fuehrt von main zurueck
  ok      ... -> legacy
  ok      legacy darf nie starten - auch mit Editor nicht
  ok      legacy ist wieder audio-neutral
  ok      Sonde (NkPr) frisch: neutral trotz fester Produktklasse
  ok      Sonde (NkPr): gueltiger eigener Stand -> passive_probe
  ok      Sonde (NkPr): kein Brokerstart
  ok      Sonde (NkPr): auch mit offenem Editor kein Brokerstart
  ok      Sonde (NkPr): kein Akt macht sie zum Main
  ok      Sonde (NkAc): gueltiger eigener Stand -> active_probe
  ok      Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main

[2] Verdrahtet: EqCopilotProcessor (Gen)
  ok      Scannerlauf: klassifiziert NICHT (53.5)  [unclassified]
  ok      Scannerlauf: kein Brokerstart
  ok      Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)
  ok      Scannerlauf: Markierung meldet sich nicht hoerbar
  ok      Altprojekt 'sensor' -> legacy  [legacy]
  ok      legacy mit offenem Editor: kein Brokerstart
  ok      legacy ist 'immer passiv' - kein Sample gefaerbt
  ok      Altprojekt 'hub' -> main  [main]
  ok      main, aber Editor zu: noch kein Brokerstart
  ok      main + offener Editor: Brokerstart erlaubt
  ok      main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)
  ok      Vorbereitung: Rollenwahl aendert den Stand
  ok      Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)  [unclassified]
  ok      der geoeffnete Editor allein holt es auch nicht nach
  ok      Vorbereitung: der User waehlt die Rolle im offenen Editor
  ok      frische Instanz: Editor offen + Rollenwahl 'hub' -> main
  ok      frische Instanz als main: faerbt
  ok      Rueckweg: 'sensor' klassifiziert zurueck auf legacy
  ok      Rueckweg: der Schnitt auf neutral greift sofort
  ok      Vorbereitung: main
  ok      der fremde Stand kommt read-only zurueck
  ok      read-only -> zurueck auf unclassified
  ok      read-only: kein Brokerstart
  ok      read-only: wieder audio-neutral

LEBENSLAUF-TEST OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 30,75 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopPipeClientTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_rc_lib.dir\Release\EqCopPipeClientTest_rc_lib.lib
  EqCopPipeClientTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSunaNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_rc_lib.dir\Release\EqCopSunaNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopSunaNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
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
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopAnalysisGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_rc_lib.dir\Release\EqCopAnalysisGoldenTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopAnalysisGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaSuna_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_rc_lib.dir\Release\NakamaSuna_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaSuna.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\Nakama Suna_SharedCode.lib
  NakamaSuna_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaSuna_vst3_helper.exe
  NakamaSuna_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\x86_64-win\Nakama Suna.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaSuna_artefacts/Release/VST3/Nakama Suna.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>


---

## Kanon-Lauf - S6 Dirigent 2026-08-28 - Kanon auf 0fdd2c3 nach Nacharbeit Runde 2

**Lauf:** 2026-08-28 19:25 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 29/29 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-28 19:25:35 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 0fdd2c3 S6 Nacharbeit T3 Runde 2: flatc-Beleg an das gebaute Binary gebunden (POST_BUILD, Commit + SHA-256) |
| Commit (voll) | 0fdd2c383cd86967b935b1b16d0b05da46f3a381 |
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
| `EqCopNullTest` | 2026-08-28 16:37:15 | `A8000FEF90307EEE` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-27 03:22:19 | `C89836581A6FFD48` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-28 16:37:19 | `08BFDD5F08117D89` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-27 02:25:05 | `ECA970E1DE7BA8CC` | frisch (Bau bestaetigt) |
| `EqCopSunaNullTest` | 2026-08-28 16:37:30 | `2EB04A41F6432C5F` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-28 16:37:32 | `C393C82F65A024A2` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-28 16:37:34 | `F12EA9D0EBF94F07` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-28 16:37:36 | `A5DF7B99893A3DD7` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-28 18:51:57 | `69413AB1A66C7B5E` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-28 16:37:45 | `FBD7BBAAD558392D` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-27 01:11:05 | `F56913243873A4FD` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-28 16:37:49 | `91F2C529928CB0FB` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-28 16:37:51 | `ED6BD5E4CAB0D9F5` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-28 19:16:04**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,15 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 10,45 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,67 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 1,59 s | [↓ A4](#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,15 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,48 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,21 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,26 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,86 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,25 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,25 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,21 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,38 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,82 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 29,36 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,16 s | [↓ A19](#a19) |
| A15 | Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | [OK] Exit 0 | 0,13 s | [↓ A15](#a15) |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,13 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,16 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,43 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,12 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,50 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,26 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,53 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,22 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,12 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,17 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,15 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 10,45 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 6,67 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: als Main klassifiziert (§53.5)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: als Main klassifiziert (§53.5)
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: als Main klassifiziert (§53.5)
  ok      T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: als Main klassifiziert (§53.5)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: als Main klassifiziert (§53.5)
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: beide Instanzen als Main klassifiziert (§53.5)
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
  ok      T11: als Main klassifiziert (§53.5)
  ok      T11: Auftrag gebaut
  ok      T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)
  ok      T11: und die Markierung meldet sich nicht hoerbar
  ok      T11: mit gestopptem Transport ebenfalls kein Sample
  ok      T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 1,59 s

stdout:

```text

running 90 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test dto::tests::as_f64_liefert_fuer_jede_number_einen_wert ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test dto::tests::bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege ... ok
test dto::tests::doppelter_schluessel_wird_nicht_still_ueberschrieben ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test dto::tests::geschachtelter_doppelter_schluessel_faellt_ebenfalls ... ok
test aggregat::tests::schreiben_erzeugt_haltbare_datei ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test aggregat::tests::alte_temp_restdatei_blockiert_snapshot_nicht ... ok
test bindung::tests::fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test aggregat::tests::gleicher_zeitstempel_ueberschreibt_keinen_snapshot ... ok
test protokoll::tests::fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow ... ok
test bindung::tests::wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand ... ok
test protokoll::tests::gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt ... ok
test protokoll::tests::heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::hello_enum_und_audiofelder_werden_semantisch_begrenzt ... ok
test protokoll::tests::hello_textfelder_haben_codepunktgenaue_harte_grenzen ... ok
test protokoll::tests::messstand_guard_prueft_minima_endlichkeit_und_textcap ... ok
test protokoll::tests::option_felder_akzeptieren_explizites_null_wie_das_schema ... ok
test protokoll::tests::unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset ... ok
test register_tests::owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test aggregat::tests::parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen ... ok
test bindung::tests::parallele_schreiber_hinterlassen_genau_einen_intakten_stand ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id ... ok
test register_tests::stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test register_tests::unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::fremdes_session_token_beendet_established_statt_acklos_zu_haengen ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::ungelesenes_welcome_blockiert_connection_ende_nicht ... ok
test server::tests::vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::v1_heartbeat_mit_measurement_ist_protokollbruch ... ok
test server::tests::sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten ... ok
test server::tests::sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung ... ok
test server::tests::zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen ... ok
test server::tests::zweites_hello_kann_die_established_bindung_nicht_umschreiben ... ok
test server::tests::bye_braucht_exakt_das_ausgegebene_session_token ... ok
test server::tests::ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz ... ok
test server::tests::connection_und_pipe_instanzzahl_bleiben_hart_begrenzt ... ok
test server::tests::ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist ... ok
test server::tests::acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup ... ok

test result: ok. 90 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 1.01s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 8 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test dto_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 8 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.13s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.04s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a4b"></a>
#### A4b | EqCopPipeClientTest

**Befehl:** `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | **Exitcode:** 0 | **Dauer:** 1,15 s

stdout:

```text
PIPECLIENT-LIFECYCLE-TEST
  ok      Fake-Pipe fuer Welcome-Stopp angelegt
  ok      stop() bricht blockiertes Welcome-Lesen begrenzt ab  [0 ms]
  ok      SQOS begrenzt den Peer auf SecurityIdentification  [1]
  ok      Fake-Pipe fuer ACK-Stopp angelegt
  ok      stop() bricht blockiertes ACK-Lesen begrenzt ab  [0 ms]
  ok      Reconnect-Fake-Pipe angelegt
  ok      Reconnect waehrend Welcome verliert keine Konfigurationsgeneration
  ok      ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt
  ok      Welcome mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ACK mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers  [322 ms]
  ok      u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze
  ok      200 konkurrierende start/stop-Paare ohne joinable-Leak
PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler
```

stderr:

_(leer)_

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 1,48 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
52 Definitionen, 17 Nachrichtenfamilien, 188 Fixtures
Pruefungen: 284 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 49  
  const                     8 / 8    HART
  enum                     10 / 10   HART
  exclusiveMinimum          2 / 2   
  items                     3 / 8   
  maxItems                  3 / 8   
  maxLength                 4 / 20  
  maxProperties             2 / 3   
  maximum                   6 / 18  
  minItems                  2 / 8   
  minLength                 5 / 14  
  minimum                   8 / 48  
  oneOf                     7 / 8   
  pattern                   4 / 9   
  required                 40 / 229 
  type                     11 / 165 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:933: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,21 s

stdout:

```text
42 gueltige, 146 ungueltige Fixtures
  bytegleich: 190 Dateien, MANIFEST sha256=bedc885d7e2ca10aebadee1d41b04187ed61b2792649804d04600d5dbb96cbee
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,26 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  flatc-Beleg: Commit 7e163021e59cca4f8e1e35a7c828b5c6b7915953, sha256=5ed5bdf97384c574f009a86a7fe8ba543a770c1e15e04f34495139e620ef0c1d  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\nakama-flatc-commit-Release.txt)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,86 s

stdout:

```text
9 gueltige, 53 ungueltige Fixtures
  bytegleich: 63 Dateien, MANIFEST sha256=b6be5564a60ba6e524ea174807c253c7ad7ee0e810bef2260cc6eba52f1e8282
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,21 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,38 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (737942 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7105024 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      die Lib wurde mit der heutigen Definemenge gebaut (15 aus der Projektdatei)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.bundle = 'Nakama Probeeq.vst3' NICHT
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt active-probe.produktname = 'Nakama Probeeq' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.bundle = 'Nakama Suna.vst3' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT
  ok      NakamaKern.lib traegt passive-probe.produktname = 'Nakama Suna' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die 5 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

26 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 0,82 s

stdout:

```text
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag  [3 vs 3; identity=ok]
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID im Installer-Manifest
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest
  ok      faellt am verdorbenen Manifest: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt am verdorbenen Manifest: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt am verdorbenen Manifest: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt am verdorbenen Manifest: Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt am verdorbenen Manifest: kein Viercode, keine Class-ID im Installer-Manifest
  ok      faellt am verdorbenen Manifest: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt am verdorbenen Manifest: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt am verdorbenen Manifest: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt am verdorbenen Manifest: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt am verdorbenen Manifest: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt am verdorbenen Manifest: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt am verdorbenen Manifest: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[4] Auslieferungsstand
  hinweis nicht ausgeliefert - 4 Artefakt(e) ohne Hash: main, passive-probe, active-probe, eqcop-broker.exe
          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

38 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 29,36 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
  ok      kanonische VST3-Produktpolicy steht unveraendert im Original
  ok      kanonische Broker-Produktpolicy steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle
      - $erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
      + $erlaubteVst3Basis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-drbar4jj\ziel\VST3'
      - $erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
      + $erlaubteBrokerBasis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-drbar4jj\ziel\programme\evenacadia\Nakama'

[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
  ok      Bundle ohne erwartete innere Payload wird fail-closed verweigert  [Exit 1]
  ok      Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0: phili\AppData\Local\Temp\nakama-gegenpfad-drbar4jj\ziel\VST3\EQ-Copilot.vst3]
  ok      Nakama Suna installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-drbar4jj\ziel\VST3\Nakama Suna.vst3]
  ok      Nakama Probeeq installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-drbar4jj\ziel\VST3\Nakama Probeeq.vst3]
  ok      eqcop-broker.exe installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-drbar4jj\ziel\programme\evenacadia\Nakama\eqcop-broker.exe]
INSTALLATION OK
]
  ok      geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg
  ok      jede Sicherung ist relativ an backups/<transaktions-id> gebunden
  ok      Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet
  ok      beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM
  ok      wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      passive-probe: Bundle traegt moduleinfo.json  [Nakama Suna.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[2b] Aktiver Recovery-Anker wird nicht still superseded
  ok      geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie  [Exit 1]
  ok      VORBEREITET verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      VORBEREITET: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      KOMPENSATION verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      KOMPENSATION: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      ERROR_TEILSTAND verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      ERROR_TEILSTAND: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      RUECKWEG_AKTIV verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      RUECKWEG_AKTIV: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded  [Exit 0]
  ok      Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren
  ok      zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle vier Artefakte melden `aktuell`  [4x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben
  ok      frische Transaktion friert den nun bekannten historischen Stand ein
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht
  ok      passive-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Suna.vst3]
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      das im Journal belegte Broker-Ziel ist weg
  ok      unbelegte Elternverzeichnisse bleiben unangetastet
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert
  ok      spaeter Installationsfehler endet laut und nicht als Teilerfolg  [Exit 1]
  ok      Journal bestaetigt die vollstaendige Kompensation  [ERROR_RUECKGEROLLT]
  ok      Main ist nach der Kompensation bytegleich zum Vorzustand
  ok      passive-probe: kein Teilstand nach spaetem Fehler
  ok      active-probe: kein Teilstand nach spaetem Fehler
  ok      der fremde Blocker wurde nicht angetastet
  ok      vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei
  ok      ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden  [Exit 0]
  ok      nach Fehler-Supersede bleibt nur die aktive Backup-Generation

[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig
  ok      Ausgangsinstallation fuer den Hard-Crash-Zwischenstand
  ok      VORBEREITET + mutation_begonnen restauriert statt Drift zu melden  [Exit 0]
  ok      der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt
  ok      Ausgangsinstallation fuer einen unterbrochenen inversen Pfad
  ok      RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort  [Exit 0]
  ok      auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand

[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen
  ok      Ausgangsinstallation fuer Journal-Missbrauchstests
  ok      manipulierte Repo-Diagnose ist keine Rueckwegautoritaet  [Exit 0]
  ok      fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben  [Exit 1]
  ok      fremder Loeschpfad im Journal wird nicht entfernt  [Exit 1]
  ok      Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen  [Exit 1]

[11] Geschuetzte Autoritaet, Backup- und Zieldrift
  ok      oeffentliches Journal allein autorisiert keinen Rueckweg  [Exit 1]
  ok      Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash  [Exit 1]
  ok      frische Ausgangsinstallation fuer Zieldrift
  ok      veraendertes Installationsziel wird nicht ueberschrieben  [Exit 1]
  ok      Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet

[12] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2
  ok      Runtime verwirft eine Bundle-Zielkollision  [Exit 1]
  ok      Runtime bindet Identity typstreng an State-Schema 2  [Exit 1]
  ok      Runtime verwirft kanonisches Broker-Zieltraversal  [Exit 1]

80 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a19"></a>
#### A19 | erzeuge_bandgitter_header.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,16 s

stdout:

```text
OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen (204 Zeilen).
    Evidenz nakama_1_24_oct_30_18k_v1: 221 Baender, 222 Kanten
    Live    nakama_log64_v1: 64 Gruppen, lueckenlose Partition der 221
```

stderr:

_(leer)_

<a id="a15"></a>
#### A15 | EqCopSunaNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse passive_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [passive_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [172 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [passive_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [172 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse active_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [active_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [3439 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [3439 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,16 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes Ziel im Manifest hat hier eine Zeile  [3 vs 3]
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest
  ok      CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle: COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle: der Viercode steht nicht literal im Zielblock  [Eqcp]
  ok      CMake-Quelle: der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      CMake-Quelle: der Produktname steht nicht literal im Zielblock
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden
  ok      CMake: Defineblock der Sondenfunktion gefunden
  ok      CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      passive-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkPr]
  ok      passive-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Suna]
  ok      passive-probe: NakamaSuna entsteht ueber nakama_sonde_ziel()
  ok      passive-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaSuna    passive-probe NAKAMA_SONDE_PASSIV)]
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 7:26:01pm vs 27 Aug 2026 2:27:21am]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      passive-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\Resources\moduleinfo.json]
  ok      passive-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 7:26:05pm vs 27 Aug 2026 2:27:21am]
  ok      passive-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      passive-probe: Produktname wie im Manifest  [Nakama Suna]
  ok      passive-probe: Vendor wie im Manifest  [evenacadia]
  ok      passive-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      passive-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: Unterkategorien wie im Manifest
  ok      passive-probe: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 7:26:07pm vs 27 Aug 2026 2:27:21am]
  ok      active-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      active-probe: Produktname wie im Manifest  [Nakama Probeeq]
  ok      active-probe: Vendor wie im Manifest  [evenacadia]
  ok      active-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      active-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B4163]
  ok      active-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B4163]
  ok      active-probe: Unterkategorien wie im Manifest
  ok      active-probe: keine fremde Ziel-CID im Bundle
  ok      Viercode ist vier Zeichen lang: main.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      Viercode ist vier Zeichen lang: passive-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      Viercode ist vier Zeichen lang: active-probe.plugin_code  [4 Zeichen]
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
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 114 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,43 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      16 Kombinationen aus Klasse x Messposition wie §2.2 - post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)  [16]
  ok      == Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle
  ok      21 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [21]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser
  ok      gueltiger Praefix mit Suffix wird vollstaendig ignoriert
  ok      abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen
  ok      deklarierte Riesenvariante wird vor JUCE-Allokation verworfen
  ok      zu tiefer ValueTree wird begrenzt und ignoriert
  ok      zu tief geschachteltes Variantenarray wird begrenzt und ignoriert
  ok      State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen
  ok      ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel
  ok      63 verschachtelte Arrays plus Blatt passieren und laden
  ok      65.536 Eintraege in einer Sammlung passieren den Byte-Riegel
  ok      65.537 Eintraege in einer Sammlung werden verworfen
  ok      262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel
  ok      262.145 Eintraege ueber mehrere Sammlungen werden verworfen
  ok      Teststate trifft die schreibbare Bytegrenze exakt  [16773120]
  ok      Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773848]
  ok      State ohne konkreten Writer-Headroom bleibt read-only bytegleich
  ok      vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [9]
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [10]
  ok      additive Binaer- und Array-Properties bleiben bytegleich lesbar
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [28ccc4f785824594b69a76867710fb3d]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [af62c4855a334db1a0297a3819e35c3b]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 153 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,12 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,50 s

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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260828-192703.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (80019 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,26 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [102 Faelle]
[ok]   Falltabelle hat Substanz  [102 Faelle]
[ok]   Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (62 Fixtures)
[ok]   Binaerkorpus hat Substanz  [62 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-auf-2hoch53.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740992.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-nahe-2hoch53-ab.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740991.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-unter-2hoch53-auf.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 4503599627370495.9]
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-kleiner-bruch-rundet-auf-eins.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 1.00000000000000001]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nan-token.json  [unbekanntes Literal NaN an Position 1090]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (188 Fixtures)
[ok]   Korpus hat Substanz  [188 Fixtures]
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
63 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 3,53 s

stdout:

```text
== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==
Produktionsstrom GenStrom: 1 Tap(s), 131072 Frames Ring, 2048 Deskriptoren, max 16384 Frames je Block.

== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==
  ok      drei Bloecke, drei Deskriptoren  [3]
  ok      jeder Frame kommt bitgleich und an derselben Stelle heraus
  ok      Stempel: lokaler Strom und Projektzeit stimmen je Block
  ok      nach dem Leeren ist die Queue leer
  ok      kein Verlust bei Normallast
== B - Wrap: der Ring laeuft mehrfach um ==
  ok      der Ring ist wirklich mehrfach umgelaufen  [11 Umlaeufe]
  ok      ueber jeden Wrap hinweg bitgleicher Inhalt
  ok      kein Frame verloren, keiner doppelt  [12000/12000]
== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==
  ok      vier Bloecke passen, danach ist Schluss  [4]
  ok      beide abgewiesenen Bloecke sind gezaehlt  [2]
  ok      verlorene Frames = 2 x 256 (keine Teilmenge)  [512]
  ok      ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock
  ok      der erste Block nach dem Verlust traegt kFlagLueckeDavor
  ok      und beginnt ein neues continuity_segment  [1]
== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==
  ok      genau `deskriptorPlaetze` Bloecke passen  [8]
  ok      die vier ueberzaehligen sind gezaehlt  [4]
  ok      und genau so viele stehen zum Lesen bereit  [8]
== E - Oversize: ein Block ueber der Slotkapazitaet ==
  ok      ein Block ueber maxBlockFrames wird als GANZES abgewiesen
  ok      und als Oversize gezaehlt, nicht als Ueberlauf  [1/0]
  ok      der Ueberlaufzaehler bleibt sauber
  ok      nichts Halbes im Ring
  ok      die verlorene Zeit ist vollstaendig gemeldet  [257]
  ok      genau maxBlockFrames passt noch (Kante, nicht Ausschluss)
== F - Flush (numSamples == 0) ist kein Verlust ==
  ok      ein Flush-Block wird nicht eingestellt
  ok      und zaehlt NICHT als Verlust - er traegt keine Audiozeit
  ok      der Folgeblock geht normal
  ok      und traegt keine Luecke
== G - Mono und Mehrfach-Tap ==
  ok      Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)
  ok      und der Deskriptor sagt kanaele == 1
  ok      Drei-Tap-Layout: Block angenommen
  ok      das Bitset meldet genau die belegten Taps  [3]
  ok      jeder Tap liegt getrennt und richtig; der inaktive liefert false
  ok      eine falsche Tapzahl wird abgewiesen statt halb kopiert
  ok      ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor
== H - Ein-Block-Quarantaene ==
  ok      5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene  [4]
  ok      und die Quarantaene haelt ihn wirklich
  ok      kein Bruch bei lueckenlosem Strom
  ok      der Seek ist als Bruch erkannt  [1]
  ok      genau EIN Block faellt an der Grenze - der Strom laeuft weiter
  ok      der Block NACH der Grenze ist schon wieder in Quarantaene
  ok      Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)
  ok      stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      Vorlauf ohne Bruch
  ok      Transportkante play->stop ist ein Bruch  [1]
  ok      ohne Hostkontext traegt der lokale monotone Strom (§53.6)
  ok      und versiegelt normal  [3]
  ok      Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung
  ok      negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)
== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==
  20 000 Bloecke, Groessen 1..288: angenommen 17809, Ueberlauf-Drops 0, Oversize-Drops 2191, groesster Block 256, versiegelt 15872, Brueche 1936
  ok      jeder gelesene Frame steht an der Stelle, die sein Stempel nennt
  ok      es gab nie einen Block mit sampleCount 0
  ok      die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt  [2191]
  ok      kein angenommener Block war je groesser als die Slotkapazitaet  [256]
  ok      es wurde wirklich gelesen  [2288127 Frames]
== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==
  ok      18 Blockgroessen von 1 bis 16 384: kein Sample veraendert
  ok      0 Samples Latenz  [0]
  ok      kein Tail
  angenommen 18, groesster Block 16384, Ueberlauf 0, Oversize 0
== K - verdrahtet: Oversize aendert das Audio nicht ==
  ok      ein Block ueber der Slotkapazitaet laeuft unveraendert durch
  ok      und ist als Oversize-Drop der ANALYSE gezaehlt  [1]
  ok      der abgewiesene Oversize-Block erscheint nicht im Projektfenster
  ok      der naechste angenommene Lueckenblock beginnt das Fenster exakt neu  [16385..16449]
  ok      der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben  [0]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3836 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
== M - verdrahtet: die Messung bekommt den Strom wirklich ==
  eingespeist 287744, analysiert 287232, Drops 0, versiegelt 561, Quarantaene-Verwuerfe 0, Brueche 0
  ok      bei Normallast kein einziger Drop
  ok      und kein Kontinuitaetsbruch (lueckenloser Transport)
  ok      die Engine sieht den ganzen Strom bis auf den Block in Quarantaene  [512 Frames offen]
  ok      Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)  [-8.684240]
  ok      framesDropped im Stats-Snapshot kommt aus derselben Quelle
== N - Gegenpfad: prepareToPlay mitten im Betrieb ==
  ok      auch der erste Block nach prepareToPlay alloziert nicht  [0]
  veraltete Bloecke (alter Anlauf, nicht analysiert): 40
  ok      ein Neuanlauf ist kein Oversize
== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==
  ok      der Anlauf steigt sofort, nicht erst beim naechsten Audioblock  [0 -> 1]
  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt 3, als aktuell durchgelassen 0
  ok      alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar  [3/3]
  ok      der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt
  ok      Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell
  ok      und er traegt die Luecke, die der Neuanlauf gerissen hat
  ok      der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut
  ok      zwei Neuanlaeufe hintereinander gehen beide nicht verloren  [1 -> 3]
  ok      zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration
  ok      und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste
  ok      Nullframe konsumiert die wartende Generation nicht
  ok      der naechste echte Block uebernimmt sie weiterhin
== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==
  ok      Seek um 10 s bei gestopptem Transport ist EIN Bruch  [1]
  ok      Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)  [0]
  ok      und bei laufendem Transport ist der Seek weiterhin ein Bruch  [1]
  ok      bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet  [0]
== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==
  ok      nach Start ist nur die leichte Publikation sofort faellig
  ok      10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus
  ok      vor 250 ms bleibt Gating/Kandidaten gesperrt
  ok      bei 250 ms ist genau die schwere Runde faellig
  ok      dieselbe Deadline kann nicht zweimal verbraucht werden
  ok      nach Pause genau eine Runde, kein Catch-up-Sturm
  ok      Reset/Generation startet die 250-ms-Deadline neu
== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==
  ok      Test hat echten Queue-Rueckstau hergestellt  [257 Bloecke bis zum ersten Drop]
  ok      unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms  [1 Runden in 600 ms]
  ok      neue Generation erbt keinen Batch-beschleunigten Schwertakt  [0 Runden in den ersten 100 ms]
  ok      Frame-Leser wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Reset-Steuerer wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Stop/Join bleibt auch mit Queue-Rest begrenzt  [0 ms]
== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==
  ok      Gegenprobe: erste Generation hat ihr eigenes Projektfenster
  ok      Testbarriere liegt deterministisch im laufenden Audiocallback
  ok      der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster  [10000..10064]
  ok      die Generationskante ist kein Seek innerhalb des alten Fensters  [0]
== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==
  ok      Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine
  ok      nichtendliche Hostrate wird fail-closed auf 0 gespiegelt
  ok      M1 verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      FeatureEngine verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung
  ok      eine folgende gueltige Generation aktiviert beide Engines wieder

114 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,22 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Feingitter: 10001 Bins a 0.01 LU ueber [-70.00, 30.01) LUFS.
Oberband:   3070 Bins a 1.00 LU ueber [30.01, 3100.01) LUFS - lautheit(DBL_MAX) = 3081,86.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: kein Block faellt durch das Raster  [0]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: kein Block faellt durch das Raster  [0]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): kein Block faellt durch das Raster  [0]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: kein Block faellt durch das Raster  [0]
  ok      Musik + echte Stille: LUFS-I innerhalb ±0,1 LU  [akku=-8.281644 ref=-8.281644 d=0.000000000]
  ok      Musik + echte Stille: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Musik + echte Stille: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000027]
  ok      Musik + echte Stille: Kurz-LUFS Gueltigkeit gleich
  ok      Musik + echte Stille: Kurz-LUFS BITGLEICH  [akku=-12.451912590513381 ref=-12.451912590513381]
== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==
  Sweep: 201 Laeufe, groesste Abweichung 2.781360 LU bei Faktor 1.000000, groesste gemeldete Schranke 2.781412 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen (Eingang aendert sich um 1e-5): 2.781403 LU
  ok      Sweep vollstaendig gefahren  [201/201]
  ok      Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)  [sonst pruefte D nichts; schranke=2.781412]
  ok      die selbstgemeldete Schranke deckt JEDEN Lauf  [201/201]
  ok      wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU  [76/76]
  ok      die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)  [maxD=2.781360 refSprung=2.781403]
  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)
== E - fixed memory: eine Million Zellen ==
  Akku:     0 Allokationen, LUFS-I -8.386510, Schranke 0.000000000 LU
  Referenz: 36 Allokationen, LUFS-I -8.386510
  ok      1 000 000 Zellen: 0 Allokationen im Akku  [0]
  ok      Gegenprobe: die alte Rechnung alloziert sehr wohl  [36]
  ok      1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU  [d=0.000000000]
  Fester Bedarf laut speicherBytes(): 157122 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      Kurz-LUFS meldet einen nichtendlichen Ring nie als gueltig
  ok      ueber dem Gitter: EIN Pegel bleibt wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      Gegenprobe: der Korpus liegt wirklich im Oberband  [197]
  ok      und kein Block faellt durch das Raster
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer
== G - Oberband: zwei Pegel ueber dem Feingitter ==
  G1: akku=49.302611 ref=49.302611 schranke=0.000000000 ueberGitter=1997
  ok      G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)  [1997]
  ok      zwei Pegel ueber dem Gitter (T2-1): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      zwei Pegel ueber dem Gitter (T2-1): kein Block faellt durch das Raster  [0]
  ok      zwei Pegel ueber dem Gitter (T2-1): LUFS-I innerhalb ±0,1 LU  [akku=49.302611 ref=49.302611 d=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS Gueltigkeit gleich
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS BITGLEICH  [akku=32.319299956639810 ref=32.319299956639810]
  ok      Naht, Gamma_r im Oberband: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Oberband: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Oberband: LUFS-I innerhalb ±0,1 LU  [akku=49.302481 ref=49.302481 d=0.000000000]
  ok      Naht, Gamma_r im Oberband: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Oberband: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  ok      Naht, Gamma_r im Feingitter: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Feingitter: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Feingitter: LUFS-I innerhalb ±0,1 LU  [akku=49.243363 ref=49.243363 d=0.000000000]
  ok      Naht, Gamma_r im Feingitter: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Feingitter: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  G3-Sweep: 201 Laeufe, groesste Abweichung 2.781577 LU bei Faktor 1.001000, groesste gemeldete Schranke 2.781577 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: 2.781401 LU
  ok      G3: Sweep vollstaendig gefahren  [201/201]
  ok      G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)  [1997]
  ok      G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)  [sonst pruefte G3 nichts; schranke=2.781577]
  ok      G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband  [201/201]
  ok      G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz  [maxD=2.781577 refSprung=2.781401]
  ok      G3: kein Block faellt durch das Raster  [0]
  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - 1000 Bloecke kippen gemeinsam die Gateseite.)
  G4: LUFS-I 3038.851, Schranke 0.000000000, ueberGitter 197, ueberOberband 0
  ok      G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert  [3038.851]
  ok      G4: nichts wird faelschlich als nicht-endlich gezaehlt  [0]
  ok      G4: auch am aeussersten Rand faellt kein Block durch das Raster  [0]
  ok      G4: und die Schranke bleibt endlich  [0.000000000]

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
109 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b5"></a>
#### B5 | EqCopAnalysisGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 2,12 s

stdout:

```text
== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==
Gate: "Drop/Seek/Loop trennt jedes offene Fenster."
Stufen: Bass 16384 (Hop 8192), Haupt 4096 (Hop 2048), Trennung bei 200 Hz.

== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==
[ok]   Evidenzgitter: band_anzahl == kEvidenzBaender  [221]
[ok]   Livegitter: band_anzahl == kLiveBaender  [64]
[ok]   Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)  [0 Abweichungen]
[ok]   Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)  [0 Abweichungen]
[ok]   Live-Kanten BITGLEICH zur Fixture (65 Werte)  [0 Abweichungen]
[ok]   Live-Mitten BITGLEICH zur Fixture (64 Werte)  [0 Abweichungen]
[ok]   Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal  [0 Abweichungen, Ende bei 221]
[ok]   Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank  [0 Abweichungen]

== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==
[ok]   q_db_0p1_i16: Skalierung stimmt mit der Fixture ueberein  [10]
[ok]   q_db_0p1_i16: alle 23 Vektoren treffen
[ok]   q_db_0p01_i16: Skalierung stimmt mit der Fixture ueberein  [100]
[ok]   q_db_0p01_i16: alle 23 Vektoren treffen
[ok]   float32: alle 15 Vektoren treffen bitgenau
[ok]   Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0
[ok]   halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl
[ok]   und negativ genauso (-0,05 -> -1)

== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==
[ok]   Band i sitzt in Byte i/8, Bit i%8 (LSB-first)  [Byte0=81 Byte1=1]
[ok]   ein Bit loeschen laesst die Nachbarn stehen
[ok]   Lesen und Schreiben sind dieselbe Adressierung
[ok]   221 Baender brauchen 28 Bitmapbytes  [28]
[ok]   die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)  [letztes Byte = 1f]

== D - FFT: gegen analytisch bekannte Ergebnisse ==
[ok]   Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin  [100.0000 %]
[ok]   Parseval haelt (Zeit- und Frequenzenergie gleich)  [rel. Fehler 0.0000000000000002]
[ok]   Gleichanteil sitzt vollstaendig in Bin 0  [Bin0 = 1048576.0, Rest = 0.00000000000000000000]

== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==
[ok]   RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander  [max 0.04312 dB bei 20000.0 Hz]
       (gemessene Groesstabweichung, zur Ablesung: 0.043120 dB)
[ok]   Hochpass sperrt den Gleichanteil vollstaendig  [|H(0)|Â² = 0.000000000000000000000000]

== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==
[ok]   die Sequenz zaehlt ab 1 aufwaerts  [6]
[ok]   mit gueltiger Projektzeit ist die Zeitbasis project_samples
[ok]   und project_sample_start ist gesetzt
[ok]   Gueltigkeitsbit project_time steht
[ok]   der Frame traegt seine Metrikversion  [20260823]
[ok]   und die Samplerate
[ok]   Frame-Stempel spannt alle integrierten Hostbloecke auf  [0 + 5120]
[ok]   Frame-Stempel umfasst den ueberlappenden FFT-Support  [2048 + 8192]
[ok]   ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck
[ok]   Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen
[ok]   fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden
[ok]   Evidenzframe vereinigt Haupt- und aelteren Basssupport  [0 + 30720, 196 Baender]
[ok]   stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen
[ok]   stehende FL-Teilstuecke erfinden kein Projektintervall
[ok]   ueberlaufende Host-Projektspanne wird konservativ lokal publiziert
[ok]   Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block  [104608 + 512]
[ok]   ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (Â§32.3)
[ok]   und dann steht WEDER das Bit NOCH ein Startwert da
[ok]   alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet  [0x7f]
[ok]   eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (Â§32.3)
[ok]   Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer
[ok]   continuous_time_samples kommt mit demselben Frameanker durch
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: NaN
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Inf
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Ende vor Start
[ok]   ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich

== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==
[ok]   G1 Drop (kFlagLueckeDavor): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G1 Drop (kFlagLueckeDavor): Ursache ist lokaleLuecke  [Zaehler 1]
[ok]   G1 Drop (kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche (§32.3)  [Epochen 0 Segmente 1]
[ok]   G2 Seek waehrend Wiedergabe: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G2 Seek waehrend Wiedergabe: Ursache ist zeitSprung  [Zaehler 1]
[ok]   G2 Seek waehrend Wiedergabe: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G3: bei gestopptem Transport laufen die Fenster normal weiter  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)  [0 Epochenwechsel]
[ok]   G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G3: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G3: und sie heisst zeitSprung, nicht lokaleLuecke
[ok]   G4: aktive Schleife allein trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4 Loop-Wrap: jedes Fenster getrennt  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G4: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G4: und die Ursache heisst loopWrap, nicht zeitSprung
[ok]   G5: Schleife ohne erreichbare Grenze trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, weil der Straddle-Block auch kein neues beginnt  [Bass=0 Haupt=0 Zelle=0 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G5: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet
[ok]   G5: mit eigener Ursache moeglicherStraddle
[ok]   G6 Transportkante (Play -> Stop): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G6 Transportkante (Play -> Stop): Ursache ist transportKante  [Zaehler 1]
[ok]   G6 Transportkante (Play -> Stop): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G7 Sampleratewechsel: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G7 Sampleratewechsel: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G7 Sampleratewechsel: Ursache ist sampleratewechsel  [Zaehler 1]
[ok]   G7 Sampleratewechsel: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G8 Neuanlauf (prepareToPlay): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G8 Neuanlauf (prepareToPlay): Ursache ist neuanlauf  [Zaehler 1]
[ok]   G8 Neuanlauf (prepareToPlay): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): Ursache ist beweislageWechsel  [Zaehler 1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G11: im Betrieb traegt die K-Kette einen Filterzustand  [1.392229]
[ok]   G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame  [290 / 290]
[ok]   G11: beide Laeufe liefern eine Kurzzeitlautheit
[ok]   G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - der Filternachklang ueberbrueckt nicht  [A=-60.034744263 B=-60.034744263]
[ok]   G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)  [0 Trennungen]
[ok]   G10: die Fenster laufen dabei weiter  [Bass=9224 Haupt=3080 Zelle=264 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G12: vor der Grenze tragen BEIDE Bandakkus den Ton  [Live 149 / Evidenz 149 Baender]
[ok]   G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber die Grenze weiter (Entscheid Â§10.1)  [liveSamples=0]
[ok]   G12: und der Frame traegt die NEUE Epoche  [1]
[ok]   G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird also wirklich geprueft
[ok]   G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)  [0 gesetzte Baender]
[ok]   G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter  [0 gesetzte Baender]
[ok]   G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit  [nicht gesetzt]
[ok]   G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden  [1192 Band-Fensterbeitraege]
[ok]   G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren  [120 von 120]
[ok]   G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor (Pruefer-Messung: 80 / 40 / 43)  [0 mit Live-Baendern, 0 mit Evidenz, 0 mit Aktivitaet]

== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) ==
[ok]   G13.0: zwei frische Frames sind gleich - der Vergleich sagt nicht pauschal ungleich
[ok]   G13.0: ein einziger geaenderter Skalar macht sie ungleich
[ok]   G13.0: ein einziger geaenderter Bandwert ebenso
[ok]   G13.0: und ein einziges Bitmapbit ebenso
[ok]   G13.0: und ein Feld im verschachtelten Transportstempel - der Vergleich reicht bis dorthin
[ok]   G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen vorgefuehrt
[ok]   G13a lokaleLuecke [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13a lokaleLuecke [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13a lokaleLuecke [Zwilling]: beide haben die Grenze lokaleLuecke gezogen  [A 1 / B 1]
[ok]   G13a lokaleLuecke [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13a lokaleLuecke [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13a lokaleLuecke [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13a lokaleLuecke [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13a lokaleLuecke [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13b zeitSprung [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13b zeitSprung [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13b zeitSprung [Zwilling]: beide haben die Grenze zeitSprung gezogen  [A 1 / B 1]
[ok]   G13b zeitSprung [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13b zeitSprung [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13b zeitSprung [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13b zeitSprung [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13b zeitSprung [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13c transportKante [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13c transportKante [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13c transportKante [Zwilling]: beide haben die Grenze transportKante gezogen  [A 1 / B 1]
[ok]   G13c transportKante [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13c transportKante [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13c transportKante [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13c transportKante [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13c transportKante [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13d sampleratewechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13d sampleratewechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13d sampleratewechsel [Zwilling]: beide haben die Grenze sampleratewechsel gezogen  [A 1 / B 1]
[ok]   G13d sampleratewechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13d sampleratewechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13d sampleratewechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13d sampleratewechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13d sampleratewechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13e neuanlauf [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13e neuanlauf [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13e neuanlauf [Zwilling]: beide haben die Grenze neuanlauf gezogen  [A 1 / B 1]
[ok]   G13e neuanlauf [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13e neuanlauf [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13e neuanlauf [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13e neuanlauf [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13e neuanlauf [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13f beweislageWechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13f beweislageWechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13f beweislageWechsel [Zwilling]: beide haben die Grenze beweislageWechsel gezogen  [A 1 / B 1]
[ok]   G13f beweislageWechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13f beweislageWechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13f beweislageWechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13f beweislageWechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13f beweislageWechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]

== H - NAK-29: bedingte Feldpflichten des Transportstempels ==
[ok]   ein sauberer Stempel kommt durch
[ok]   Fall 1: project_samples + Bit, aber KEIN project_sample_start
[ok]   Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu Â§32.3)
[ok]   Fall 3: bounds_valid ohne start_ppq/end_ppq
[ok]   Fall 4: derivation=validated_block_mapping ohne bounds_valid
[ok]   Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht
[ok]   Fall 6: continuous_time-Bit ohne continuous_time_samples
[ok]   Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus
[ok]   Fall 7: sample_count bleibt unter der Vertragsobergrenze
[ok]   Fall 7: der Feature-Erzeuger publiziert keinen Leerframe
[ok]   der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel entsteht im normalen, gedeckelten Betrieb  [0]
[ok]   abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich  [1]

== I - Ereignisse: kein Fluss ueber eine Grenze ==
[ok]   der spektrale Fluss hat im Betrieb einen Vorgaenger
[ok]   nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss einen Onset, den es nie gab
[ok]   jedes Ereignis traegt die Epoche, in der es gesehen wurde  [1 Ereignis(se)]
[ok]   der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, nicht behauptet  [64 im Ring, 1 verworfen, nach 1107 Bloecken]
[ok]   und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (Â§48.1: ein Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)  [64 / 64]
[ok]   drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste  [aeltestes vorher 38912, jetzt 47104]
[ok]   und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf
[ok]   Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock
[ok]   stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit  [1 Ereignis(se) nach 84 Bloecken]

== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==
[ok]   LUFS-S ist nach 4 s gesetzt
[ok]   LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (Â§39.3)  [gemessen -6.0620 erwartet -6.0620 d=0.00000]
[ok]   Peak trifft die Amplitude  [-6.021 dBFS]
[ok]   Crest eines Sinus ist 3,01 dB  [3.0091 dB]
[ok]   L==R ergibt Korrelation 1  [1.000000]
[ok]   und Breite 0 (kein Seitenanteil)  [0.000000000000]
[ok]   das Live-Band um 997 Hz ist gueltig  [Band 35]
[ok]   und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal

== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==
[ok]   die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen  [41 Bloecke]
[ok]   und mindestens einen Frame gebaut  [4 Frames]
[ok]   process_context_present kommt aus der BRUECKE durch (Â§32.3)
[ok]   recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen
[ok]   continuous_time_samples kommt durch
[ok]   die Schleifengrenzen kommen durch, samt `active`
[ok]   beide Presentation-Latencies kommen durch  [3924 / 4410]
[ok]   ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits  [0x7f]
[ok]   Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits  [0x3]
[ok]   insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`
[ok]   Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen
[ok]   Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt
[ok]   nichtendliche Sampleraten initialisieren keine Analyseengine

== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==
[ok]   L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt
[ok]   L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen  [0]
[ok]   L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, und zwar nur dort  [1 Abweichung(en), Zeile 26]
[ok]   L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'
[ok]   L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts  [0 gesetzte Baender]
[ok]   L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte  [3]
[ok]   L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung  [1, 3]
[ok]   L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'
[ok]   L6: ein echter Grund wird gezaehlt  [1]
[ok]   L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen  [gelesen: 0]
[ok]   L6: und `keine` ebenso - getrennt wird nie ohne Grund

== M - Auskunft ueber die Grenzen der Messung ==
[ok]   die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da  [erstes messbares Band 2 (32.6 Hz)]
[ok]   bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz  [erstes Band ueber der Kappe 221]
[ok]   bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab  [erstes Band ueber der Kappe 202 (10525.6 Hz)]
[ok]   und sie sitzt genau am ersten Band, dessen Oberkante 0,95Â·Nyquist ueberschreitet  [10374.7 .. 10678.7 Hz gegen 10473.8 Hz]
[ok]   kein Live-Band vollstaendig ueber der Kappe traegt einen Wert  [0 Baender]
[ok]   Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler

Ergebnis: 235 bestanden, 0 Fehler.
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,17 s

stdout:

```text
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  ok      frisch: unclassified  [unclassified]
  ok      frisch: neutral
  ok      frisch: keine Audio-Ausnahme (audio-neutral)
  ok      frisch: kein Brokerstart
  ok      Schema-1 'sensor' -> legacy  [legacy]
  ok      Schema-1 'sensor': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'pre' -> legacy  [legacy]
  ok      Schema-1 'pre': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'post' -> legacy  [legacy]
  ok      Schema-1 'post': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'hub' -> main  [main]
  ok      Schema-1 'hub': Audio-Ausnahme moeglich
  ok      Schema-2 main (Kind-Matrix bestaetigt) -> main
  ok      Schema-2 legacy -> legacy
  ok      Vorbereitung: erst main
  ok      unbekanntes Major danach -> zurueck auf unclassified
  ok      Muell (ignoriert) aendert nichts - main bleibt main
  ok      Muell auf einen frischen Automaten klassifiziert nicht
  ok      explizite Initialisierung OHNE Editor: abgelehnt
  ok      ... und der Automat bleibt neutral
  ok      der geoeffnete Editor allein klassifiziert NICHT
  ok      Editor offen + explizite Initialisierung: angenommen
  ok      ... -> main
  ok      main + offener Editor darf den Broker starten
  ok      Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)
  ok      main OHNE offenen Editor darf NICHT starten
  ok      Rueckweg: derselbe Akt fuehrt von main zurueck
  ok      ... -> legacy
  ok      legacy darf nie starten - auch mit Editor nicht
  ok      legacy ist wieder audio-neutral
  ok      Sonde (NkPr) frisch: neutral trotz fester Produktklasse
  ok      Sonde (NkPr): gueltiger eigener Stand -> passive_probe
  ok      Sonde (NkPr): kein Brokerstart
  ok      Sonde (NkPr): auch mit offenem Editor kein Brokerstart
  ok      Sonde (NkPr): kein Akt macht sie zum Main
  ok      Sonde (NkAc): gueltiger eigener Stand -> active_probe
  ok      Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main

[2] Verdrahtet: EqCopilotProcessor (Gen)
  ok      Scannerlauf: klassifiziert NICHT (53.5)  [unclassified]
  ok      Scannerlauf: kein Brokerstart
  ok      Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)
  ok      Scannerlauf: Markierung meldet sich nicht hoerbar
  ok      Altprojekt 'sensor' -> legacy  [legacy]
  ok      legacy mit offenem Editor: kein Brokerstart
  ok      legacy ist 'immer passiv' - kein Sample gefaerbt
  ok      Altprojekt 'hub' -> main  [main]
  ok      main, aber Editor zu: noch kein Brokerstart
  ok      main + offener Editor: Brokerstart erlaubt
  ok      main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)
  ok      Vorbereitung: Rollenwahl aendert den Stand
  ok      Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)  [unclassified]
  ok      der geoeffnete Editor allein holt es auch nicht nach
  ok      Vorbereitung: der User waehlt die Rolle im offenen Editor
  ok      frische Instanz: Editor offen + Rollenwahl 'hub' -> main
  ok      frische Instanz als main: faerbt
  ok      Rueckweg: 'sensor' klassifiziert zurueck auf legacy
  ok      Rueckweg: der Schnitt auf neutral greift sofort
  ok      Vorbereitung: main
  ok      der fremde Stand kommt read-only zurueck
  ok      read-only -> zurueck auf unclassified
  ok      read-only: kein Brokerstart
  ok      read-only: wieder audio-neutral

LEBENSLAUF-TEST OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 31,23 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopPipeClientTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_rc_lib.dir\Release\EqCopPipeClientTest_rc_lib.lib
  EqCopPipeClientTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSunaNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_rc_lib.dir\Release\EqCopSunaNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopSunaNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
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
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopAnalysisGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_rc_lib.dir\Release\EqCopAnalysisGoldenTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopAnalysisGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaSuna_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_rc_lib.dir\Release\NakamaSuna_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaSuna.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\Nakama Suna_SharedCode.lib
  NakamaSuna_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaSuna_vst3_helper.exe
  NakamaSuna_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\x86_64-win\Nakama Suna.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaSuna_artefacts/Release/VST3/Nakama Suna.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>


---

## Kanon-Lauf - S6 Dirigent 2026-08-28 - Kanon auf f208c45 nach Nacharbeit Runde 3

**Lauf:** 2026-08-28 19:49 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 29/29 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-28 19:49:34 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | f208c45 S6 Nacharbeit T3 Runde 3: flatc-Commit zur Bauzeit im POST_BUILD gemessen, schmutziger Checkout bricht ab |
| Commit (voll) | f208c4556be6e128cbd7d8ea367c2d1cc23f637c |
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
| `EqCopNullTest` | 2026-08-28 16:37:15 | `A8000FEF90307EEE` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-27 03:22:19 | `C89836581A6FFD48` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-28 16:37:19 | `08BFDD5F08117D89` | frisch (Bau bestaetigt) |
| `EqCopPipeClientTest` | 2026-08-27 02:25:05 | `ECA970E1DE7BA8CC` | frisch (Bau bestaetigt) |
| `EqCopSunaNullTest` | 2026-08-28 16:37:30 | `2EB04A41F6432C5F` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-28 16:37:32 | `C393C82F65A024A2` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-28 16:37:34 | `F12EA9D0EBF94F07` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-28 16:37:36 | `A5DF7B99893A3DD7` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-28 18:51:57 | `69413AB1A66C7B5E` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-28 16:37:45 | `FBD7BBAAD558392D` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-27 01:11:05 | `F56913243873A4FD` | frisch (Bau bestaetigt) |
| `EqCopAnalysisGoldenTest` | 2026-08-28 16:37:49 | `91F2C529928CB0FB` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-28 16:37:51 | `ED6BD5E4CAB0D9F5` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-28 19:45:01**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 10,24 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 20,41 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,95 s | [↓ A4](#a4) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 1,08 s | [↓ A4b](#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 1,52 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,14 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,18 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,25 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,88 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,24 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,24 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,20 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,37 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,82 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 26,07 s | [↓ A18](#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A19](#a19) |
| A15 | Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | [OK] Exit 0 | 0,05 s | [↓ A15](#a15) |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,04 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,33 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,04 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,19 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,66 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,14 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,09 s | [↓ B5](#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,07 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 10,24 s

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

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 20,41 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: als Main klassifiziert (§53.5)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: als Main klassifiziert (§53.5)
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: als Main klassifiziert (§53.5)
  ok      T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: als Main klassifiziert (§53.5)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: als Main klassifiziert (§53.5)
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: beide Instanzen als Main klassifiziert (§53.5)
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
  ok      T11: als Main klassifiziert (§53.5)
  ok      T11: Auftrag gebaut
  ok      T11: OHNE Playhead faerbt kein einziges Sample (U10, 22.08.)
  ok      T11: und die Markierung meldet sich nicht hoerbar
  ok      T11: mit gestopptem Transport ebenfalls kein Sample
  ok      T11: Gegenprobe - mit laufendem Transport faerbt genau dieser Aufbau
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,95 s

stdout:

```text

running 90 tests
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test dto::tests::as_f64_liefert_fuer_jede_number_einen_wert ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test dto::tests::bestand_kommt_aus_dem_vertrag_und_hat_109_eintraege ... ok
test dto::tests::doppelter_schluessel_wird_nicht_still_ueberschrieben ... ok
test aggregat::tests::schreiben_erzeugt_haltbare_datei ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test dto::tests::geschachtelter_doppelter_schluessel_faellt_ebenfalls ... ok
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::hin_und_zurueck ... ok
test bindung::tests::fremde_temp_restdatei_wird_weder_benutzt_noch_ueberschrieben ... ok
test aggregat::tests::alte_temp_restdatei_blockiert_snapshot_nicht ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::fehlende_hoermarkierung_bleibt_fuer_alte_v2_sender_false ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test protokoll::tests::gemeinsame_floatbreiten_akzeptieren_maximum_aber_keinen_overflow ... ok
test protokoll::tests::heartbeat_session_token_hat_dieselbe_codepunktgrenze_wie_das_schema ... ok
test protokoll::tests::gemeinsame_integerbreiten_urteilen_an_den_wire_kanten_exakt ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::hello_enum_und_audiofelder_werden_semantisch_begrenzt ... ok
test aggregat::tests::gleicher_zeitstempel_ueberschreibt_keinen_snapshot ... ok
test protokoll::tests::hello_textfelder_haben_codepunktgenaue_harte_grenzen ... ok
test protokoll::tests::messstand_guard_prueft_minima_endlichkeit_und_textcap ... ok
test protokoll::tests::option_felder_akzeptieren_explizites_null_wie_das_schema ... ok
test protokoll::tests::unbekannte_felder_werden_auf_jeder_vertragsebene_abgelehnt ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::hoermarkierung_sperrt_fremde_evidenz_bis_reset_nach_beobachtetem_false ... ok
test bindung::tests::wiederholtes_schreiben_ersetzt_den_vollstaendigen_stand ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::marker_disconnect_bleibt_fail_closed_bis_reconnect_false_und_neuem_reset ... ok
test register_tests::owner_wechsel_stellt_die_vollstaendigen_hello_metadaten_und_rechte_wieder_her ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test bindung::tests::parallele_schreiber_hinterlassen_genau_einen_intakten_stand ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::paarfenster_an_i64_raendern_bleiben_unklar_statt_ueberzulaufen ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::snapshot_sortiert_gleiche_labels_stabil_nach_sensor_id ... ok
test register_tests::stale_nutzt_monotone_zeit_und_ignoriert_wallclock_spruenge ... ok
test register_tests::unbestaetigte_marker_nonces_wachsen_nicht_unbegrenzt ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test aggregat::tests::parallele_snapshots_gleicher_millisekunde_bleiben_alle_erhalten ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok
test server::tests::stoppen_bricht_stille_connection_ohne_client_weckhilfe_ab ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::fremdes_session_token_beendet_established_statt_acklos_zu_haengen ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::ungelesenes_welcome_blockiert_connection_ende_nicht ... ok
test server::tests::vorhandene_leere_instance_nonce_wird_nicht_als_uuid_umgedeutet ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test server::tests::v1_heartbeat_mit_measurement_ist_protokollbruch ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test vertrag::tests::textriegel_hat_dieselbe_dokumentgrenze_wie_cpp_und_python ... ok
test server::tests::sequenzruecklauf_und_duplikat_beenden_jeweils_die_verbindung ... ok
test server::tests::sensor_register_hat_harte_grenze_aber_bekannte_id_darf_reconnecten ... ok
test server::tests::zweite_lebende_gleiche_nonce_wird_abgelehnt_ohne_die_erste_zu_trennen ... ok
test server::tests::zweites_hello_kann_die_established_bindung_nicht_umschreiben ... ok
test server::tests::bye_braucht_exakt_das_ausgegebene_session_token ... ok
test server::tests::ungueltige_messstaende_beenden_ohne_ack_und_entwerten_alte_evidenz ... ok
test server::tests::connection_und_pipe_instanzzahl_bleiben_hart_begrenzt ... ok
test server::tests::ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist ... ok
test server::tests::acceptor_stop_und_sofortiger_neustart_verlieren_keinen_wakeup ... ok

test result: ok. 90 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.48s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 8 tests
test quantisierung_stimmt_mit_den_testvektoren ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test dto_korpus_klassifiziert_wie_das_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 8 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.13s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.05s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a4b"></a>
#### A4b | EqCopPipeClientTest

**Befehl:** `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | **Exitcode:** 0 | **Dauer:** 1,08 s

stdout:

```text
PIPECLIENT-LIFECYCLE-TEST
  ok      Fake-Pipe fuer Welcome-Stopp angelegt
  ok      stop() bricht blockiertes Welcome-Lesen begrenzt ab  [0 ms]
  ok      SQOS begrenzt den Peer auf SecurityIdentification  [1]
  ok      Fake-Pipe fuer ACK-Stopp angelegt
  ok      stop() bricht blockiertes ACK-Lesen begrenzt ab  [0 ms]
  ok      Reconnect-Fake-Pipe angelegt
  ok      Reconnect waehrend Welcome verliert keine Konfigurationsgeneration
  ok      ungueltiges UTF-8 wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      rohes NUL wird vor der JUCE-Stringkonstruktion abgelehnt
  ok      ACK mit falscher Sequenz wird nicht als Bestaetigung gezaehlt
  ok      Welcome mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ACK mit schemafremdem Zusatzfeld wird abgelehnt
  ok      ein absolutes Zeitbudget gilt fuer Kopf, Body und Teiltransfers  [325 ms]
  ok      u64-Dropzaehler saettigt auf die gemeinsame i64-Wire-Grenze
  ok      200 konkurrierende start/stop-Paare ohne joinable-Leak
PIPECLIENT-LIFECYCLE-TEST OK - 0 Fehler
```

stderr:

_(leer)_

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 1,52 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
52 Definitionen, 17 Nachrichtenfamilien, 188 Fixtures
Pruefungen: 284 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 49  
  const                     8 / 8    HART
  enum                     10 / 10   HART
  exclusiveMinimum          2 / 2   
  items                     3 / 8   
  maxItems                  3 / 8   
  maxLength                 4 / 20  
  maxProperties             2 / 3   
  maximum                   6 / 18  
  minItems                  2 / 8   
  minLength                 5 / 14  
  minimum                   8 / 48  
  oneOf                     7 / 8   
  pattern                   4 / 9   
  required                 40 / 229 
  type                     11 / 165 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:933: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,13 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
42 gueltige, 146 ungueltige Fixtures
  bytegleich: 190 Dateien, MANIFEST sha256=bedc885d7e2ca10aebadee1d41b04187ed61b2792649804d04600d5dbb96cbee
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,25 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  flatc-Beleg: Commit 7e163021e59cca4f8e1e35a7c828b5c6b7915953, sha256=c4cf81a4f940f91a29c0d9f627654635bed88b5307f4e0f640ca26a63d9f3018  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\nakama-flatc-commit-Release.txt)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,88 s

stdout:

```text
9 gueltige, 53 ungueltige Fixtures
  bytegleich: 63 Dateien, MANIFEST sha256=b6be5564a60ba6e524ea174807c253c7ad7ee0e810bef2260cc6eba52f1e8282
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,24 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,24 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,20 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,37 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (737942 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7105024 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      die Lib wurde mit der heutigen Definemenge gebaut (15 aus der Projektdatei)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.bundle = 'Nakama Probeeq.vst3' NICHT
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt active-probe.produktname = 'Nakama Probeeq' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.bundle = 'Nakama Suna.vst3' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT
  ok      NakamaKern.lib traegt passive-probe.produktname = 'Nakama Suna' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die 5 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

26 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 0,82 s

stdout:

```text
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag  [3 vs 3; identity=ok]
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID im Installer-Manifest
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest
  ok      faellt am verdorbenen Manifest: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt am verdorbenen Manifest: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt am verdorbenen Manifest: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt am verdorbenen Manifest: Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt am verdorbenen Manifest: kein Viercode, keine Class-ID im Installer-Manifest
  ok      faellt am verdorbenen Manifest: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt am verdorbenen Manifest: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt am verdorbenen Manifest: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt am verdorbenen Manifest: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt am verdorbenen Manifest: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt am verdorbenen Manifest: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt am verdorbenen Manifest: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Adversariale Pfad- und Identitaetsgegenproben
  ok      faellt an einer Bundle-Zielkollision
  ok      faellt an Identity-state_schema 1
  ok      faellt an Identity-state_schema Text
  ok      faellt an Identity-state_schema fehlend
  ok      faellt an kanonischem Broker-Zieltraversal
  ok      faellt an cmake_ziel-Quelltraversal
  ok      faellt an benutzerbeschreibbaren Rueckweg-Backups

[4] Auslieferungsstand
  hinweis nicht ausgeliefert - 4 Artefakt(e) ohne Hash: main, passive-probe, active-probe, eqcop-broker.exe
          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).

[5] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

38 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 26,07 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
  ok      kanonische VST3-Produktpolicy steht unveraendert im Original
  ok      kanonische Broker-Produktpolicy steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle
      - $erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')
      + $erlaubteVst3Basis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-633n0pa2\ziel\VST3'
      - $erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\Nakama')
      + $erlaubteBrokerBasis = Kanonischer-Pfad 'C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-633n0pa2\ziel\programme\evenacadia\Nakama'

[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation
  ok      Bundle ohne erwartete innere Payload wird fail-closed verweigert  [Exit 1]
  ok      Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0: phili\AppData\Local\Temp\nakama-gegenpfad-633n0pa2\ziel\VST3\EQ-Copilot.vst3]
  ok      Nakama Suna installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-633n0pa2\ziel\VST3\Nakama Suna.vst3]
  ok      Nakama Probeeq installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-633n0pa2\ziel\VST3\Nakama Probeeq.vst3]
  ok      eqcop-broker.exe installiert  [C:\Users\phili\AppData\Local\Temp\nakama-gegenpfad-633n0pa2\ziel\programme\evenacadia\Nakama\eqcop-broker.exe]
INSTALLATION OK
]
  ok      geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg
  ok      jede Sicherung ist relativ an backups/<transaktions-id> gebunden
  ok      Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet
  ok      beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM
  ok      wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      passive-probe: Bundle traegt moduleinfo.json  [Nakama Suna.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[2b] Aktiver Recovery-Anker wird nicht still superseded
  ok      geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie  [Exit 1]
  ok      VORBEREITET verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      VORBEREITET: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      KOMPENSATION verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      KOMPENSATION: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      ERROR_TEILSTAND verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      ERROR_TEILSTAND: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      RUECKWEG_AKTIV verlangt Rueckweg statt neuem Journalstart  [Exit 1]
  ok      RUECKWEG_AKTIV: verweigerter Neustart laesst Anker und Ziele bytegleich
  ok      OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded  [Exit 0]
  ok      Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren
  ok      zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle vier Artefakte melden `aktuell`  [4x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben
  ok      frische Transaktion friert den nun bekannten historischen Stand ein
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht
  ok      passive-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Suna.vst3]
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      das im Journal belegte Broker-Ziel ist weg
  ok      unbelegte Elternverzeichnisse bleiben unangetastet
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert
  ok      spaeter Installationsfehler endet laut und nicht als Teilerfolg  [Exit 1]
  ok      Journal bestaetigt die vollstaendige Kompensation  [ERROR_RUECKGEROLLT]
  ok      Main ist nach der Kompensation bytegleich zum Vorzustand
  ok      passive-probe: kein Teilstand nach spaetem Fehler
  ok      active-probe: kein Teilstand nach spaetem Fehler
  ok      der fremde Blocker wurde nicht angetastet
  ok      vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei
  ok      ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden  [Exit 0]
  ok      nach Fehler-Supersede bleibt nur die aktive Backup-Generation

[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig
  ok      Ausgangsinstallation fuer den Hard-Crash-Zwischenstand
  ok      VORBEREITET + mutation_begonnen restauriert statt Drift zu melden  [Exit 0]
  ok      der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt
  ok      Ausgangsinstallation fuer einen unterbrochenen inversen Pfad
  ok      RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort  [Exit 0]
  ok      auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand

[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen
  ok      Ausgangsinstallation fuer Journal-Missbrauchstests
  ok      manipulierte Repo-Diagnose ist keine Rueckwegautoritaet  [Exit 0]
  ok      fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben  [Exit 1]
  ok      fremder Loeschpfad im Journal wird nicht entfernt  [Exit 1]
  ok      Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen  [Exit 1]

[11] Geschuetzte Autoritaet, Backup- und Zieldrift
  ok      oeffentliches Journal allein autorisiert keinen Rueckweg  [Exit 1]
  ok      Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash  [Exit 1]
  ok      frische Ausgangsinstallation fuer Zieldrift
  ok      veraendertes Installationsziel wird nicht ueberschrieben  [Exit 1]
  ok      Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet

[12] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2
  ok      Runtime verwirft eine Bundle-Zielkollision  [Exit 1]
  ok      Runtime bindet Identity typstreng an State-Schema 2  [Exit 1]
  ok      Runtime verwirft kanonisches Broker-Zieltraversal  [Exit 1]

80 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a19"></a>
#### A19 | erzeuge_bandgitter_header.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
OK: BandGridZahlen.h bytegleich zu den zwei Gitterfixturen (204 Zeilen).
    Evidenz nakama_1_24_oct_30_18k_v1: 221 Baender, 222 Kanten
    Live    nakama_log64_v1: 64 Gruppen, lueckenlose Partition der 221
```

stderr:

_(leer)_

<a id="a15"></a>
#### A15 | EqCopSunaNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse passive_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [passive_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [172 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [passive_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [172 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse active_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [active_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [3439 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [3439 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes Ziel im Manifest hat hier eine Zeile  [3 vs 3]
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest
  ok      CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle: COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle: der Viercode steht nicht literal im Zielblock  [Eqcp]
  ok      CMake-Quelle: der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      CMake-Quelle: der Produktname steht nicht literal im Zielblock
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden
  ok      CMake: Defineblock der Sondenfunktion gefunden
  ok      CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      passive-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkPr]
  ok      passive-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Suna]
  ok      passive-probe: NakamaSuna entsteht ueber nakama_sonde_ziel()
  ok      passive-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaSuna    passive-probe NAKAMA_SONDE_PASSIV)]
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 7:50:03pm vs 27 Aug 2026 2:27:21am]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      passive-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\Resources\moduleinfo.json]
  ok      passive-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 7:50:07pm vs 27 Aug 2026 2:27:21am]
  ok      passive-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      passive-probe: Produktname wie im Manifest  [Nakama Suna]
  ok      passive-probe: Vendor wie im Manifest  [evenacadia]
  ok      passive-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      passive-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: Unterkategorien wie im Manifest
  ok      passive-probe: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [28 Aug 2026 7:50:09pm vs 27 Aug 2026 2:27:21am]
  ok      active-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      active-probe: Produktname wie im Manifest  [Nakama Probeeq]
  ok      active-probe: Vendor wie im Manifest  [evenacadia]
  ok      active-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      active-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B4163]
  ok      active-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B4163]
  ok      active-probe: Unterkategorien wie im Manifest
  ok      active-probe: keine fremde Ziel-CID im Bundle
  ok      Viercode ist vier Zeichen lang: main.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      Viercode ist vier Zeichen lang: passive-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      Viercode ist vier Zeichen lang: active-probe.plugin_code  [4 Zeichen]
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
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 114 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,33 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      16 Kombinationen aus Klasse x Messposition wie §2.2 - post_fader_contribution fuer KEINE Klasse (contribution_aux unsupported)  [16]
  ok      == Positionsmatrix vollstaendig, jede Klasse in ihrem Bundle
  ok      21 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [21]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      ungueltiges UTF-8 im ValueTree faellt vor JUCEs tolerantem Stringleser
  ok      gueltiger Praefix mit Suffix wird vollstaendig ignoriert
  ok      abgeschnittener spaeter Kindbaum wird nicht als Teilstate uebernommen
  ok      deklarierte Riesenvariante wird vor JUCE-Allokation verworfen
  ok      zu tiefer ValueTree wird begrenzt und ignoriert
  ok      zu tief geschachteltes Variantenarray wird begrenzt und ignoriert
  ok      State oberhalb 16 MiB wird vor dem ersten Bytezugriff verworfen
  ok      ValueTree-Tiefe 64 inklusive Wurzel passiert den Byte-Riegel
  ok      63 verschachtelte Arrays plus Blatt passieren und laden
  ok      65.536 Eintraege in einer Sammlung passieren den Byte-Riegel
  ok      65.537 Eintraege in einer Sammlung werden verworfen
  ok      262.144 Eintraege ueber mehrere Sammlungen passieren den Byte-Riegel
  ok      262.145 Eintraege ueber mehrere Sammlungen werden verworfen
  ok      Teststate trifft die schreibbare Bytegrenze exakt  [16773120]
  ok      Writer bleibt mit maximalen bekannten Userfeldern innerhalb seiner Lesergrenze  [16773848]
  ok      State ohne konkreten Writer-Headroom bleibt read-only bytegleich
  ok      vollstaendiger bekannter State exakt bei 16 MiB bleibt read-only bytegleich
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [9]
  ok      nicht verlustfrei lesbarer Variantenmarker bleibt read-only bytegleich  [10]
  ok      additive Binaer- und Array-Properties bleiben bytegleich lesbar
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 21 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [5accb4f841c049f9ae65ec18896b9d71]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      Writer-API erzwingt 120/60-Zeichen-Grenzen ohne Dirty oder Teilmutation
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [2aa2b31b38d441f198c3aaa95ed470fa]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 153 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,04 s

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
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260828-195114.json
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
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (40655 Bloecke)
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

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,19 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [102 Faelle]
[ok]   Falltabelle hat Substanz  [102 Faelle]
[ok]   Textriegel lehnt oberhalb der gemeinsamen 16-MiB-Grenze vor dem Bytezugriff ab
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (62 Fixtures)
[ok]   Binaerkorpus hat Substanz  [62 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   JSON-Bandwertgrenzen stimmen mit quantisierung-v1.json
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-auf-2hoch53.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740992.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-nahe-2hoch53-ab.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 9007199254740991.1]
[ok]   Textriegel lehnt ab: ungueltig/zahl-bruch-rundet-unter-2hoch53-auf.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 4503599627370495.9]
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-kleiner-bruch-rundet-auf-eins.json  [Zahl mit mehr als 15 signifikanten Dezimalziffern: 1.00000000000000001]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nan-token.json  [unbekanntes Literal NaN an Position 1090]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (188 Fixtures)
[ok]   Korpus hat Substanz  [188 Fixtures]
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
63 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 3,66 s

stdout:

```text
== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==
Produktionsstrom GenStrom: 1 Tap(s), 131072 Frames Ring, 2048 Deskriptoren, max 16384 Frames je Block.

== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==
  ok      drei Bloecke, drei Deskriptoren  [3]
  ok      jeder Frame kommt bitgleich und an derselben Stelle heraus
  ok      Stempel: lokaler Strom und Projektzeit stimmen je Block
  ok      nach dem Leeren ist die Queue leer
  ok      kein Verlust bei Normallast
== B - Wrap: der Ring laeuft mehrfach um ==
  ok      der Ring ist wirklich mehrfach umgelaufen  [11 Umlaeufe]
  ok      ueber jeden Wrap hinweg bitgleicher Inhalt
  ok      kein Frame verloren, keiner doppelt  [12000/12000]
== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==
  ok      vier Bloecke passen, danach ist Schluss  [4]
  ok      beide abgewiesenen Bloecke sind gezaehlt  [2]
  ok      verlorene Frames = 2 x 256 (keine Teilmenge)  [512]
  ok      ein verworfener Block behauptet die noch ausstehende Luecke nicht selbst
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      die Produzentenrueckmeldung markiert genau den angenommenen Lueckenblock
  ok      der erste Block nach dem Verlust traegt kFlagLueckeDavor
  ok      und beginnt ein neues continuity_segment  [1]
== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==
  ok      genau `deskriptorPlaetze` Bloecke passen  [8]
  ok      die vier ueberzaehligen sind gezaehlt  [4]
  ok      und genau so viele stehen zum Lesen bereit  [8]
== E - Oversize: ein Block ueber der Slotkapazitaet ==
  ok      ein Block ueber maxBlockFrames wird als GANZES abgewiesen
  ok      und als Oversize gezaehlt, nicht als Ueberlauf  [1/0]
  ok      der Ueberlaufzaehler bleibt sauber
  ok      nichts Halbes im Ring
  ok      die verlorene Zeit ist vollstaendig gemeldet  [257]
  ok      genau maxBlockFrames passt noch (Kante, nicht Ausschluss)
== F - Flush (numSamples == 0) ist kein Verlust ==
  ok      ein Flush-Block wird nicht eingestellt
  ok      und zaehlt NICHT als Verlust - er traegt keine Audiozeit
  ok      der Folgeblock geht normal
  ok      und traegt keine Luecke
== G - Mono und Mehrfach-Tap ==
  ok      Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)
  ok      und der Deskriptor sagt kanaele == 1
  ok      Drei-Tap-Layout: Block angenommen
  ok      das Bitset meldet genau die belegten Taps  [3]
  ok      jeder Tap liegt getrennt und richtig; der inaktive liefert false
  ok      eine falsche Tapzahl wird abgewiesen statt halb kopiert
  ok      ein Block ohne aktiven Tap ist ein Verlust, kein leerer Deskriptor
== H - Ein-Block-Quarantaene ==
  ok      5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene  [4]
  ok      und die Quarantaene haelt ihn wirklich
  ok      kein Bruch bei lueckenlosem Strom
  ok      der Seek ist als Bruch erkannt  [1]
  ok      genau EIN Block faellt an der Grenze - der Strom laeuft weiter
  ok      der Block NACH der Grenze ist schon wieder in Quarantaene
  ok      Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)
  ok      stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      Vorlauf ohne Bruch
  ok      Transportkante play->stop ist ein Bruch  [1]
  ok      ohne Hostkontext traegt der lokale monotone Strom (§53.6)
  ok      und versiegelt normal  [3]
  ok      Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung
  ok      negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)
== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==
  20 000 Bloecke, Groessen 1..288: angenommen 17809, Ueberlauf-Drops 0, Oversize-Drops 2191, groesster Block 256, versiegelt 15872, Brueche 1936
  ok      jeder gelesene Frame steht an der Stelle, die sein Stempel nennt
  ok      es gab nie einen Block mit sampleCount 0
  ok      die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt  [2191]
  ok      kein angenommener Block war je groesser als die Slotkapazitaet  [256]
  ok      es wurde wirklich gelesen  [2288127 Frames]
== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==
  ok      18 Blockgroessen von 1 bis 16 384: kein Sample veraendert
  ok      0 Samples Latenz  [0]
  ok      kein Tail
  angenommen 18, groesster Block 16384, Ueberlauf 0, Oversize 0
== K - verdrahtet: Oversize aendert das Audio nicht ==
  ok      ein Block ueber der Slotkapazitaet laeuft unveraendert durch
  ok      und ist als Oversize-Drop der ANALYSE gezaehlt  [1]
  ok      der abgewiesene Oversize-Block erscheint nicht im Projektfenster
  ok      der naechste angenommene Lueckenblock beginnt das Fenster exakt neu  [16385..16449]
  ok      der Analyseverlust wird nicht als Seek im alten Fenster ausgegeben  [0]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3842 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
== M - verdrahtet: die Messung bekommt den Strom wirklich ==
  eingespeist 287744, analysiert 287232, Drops 0, versiegelt 561, Quarantaene-Verwuerfe 0, Brueche 0
  ok      bei Normallast kein einziger Drop
  ok      und kein Kontinuitaetsbruch (lueckenloser Transport)
  ok      die Engine sieht den ganzen Strom bis auf den Block in Quarantaene  [512 Frames offen]
  ok      Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)  [-8.684240]
  ok      framesDropped im Stats-Snapshot kommt aus derselben Quelle
== N - Gegenpfad: prepareToPlay mitten im Betrieb ==
  ok      auch der erste Block nach prepareToPlay alloziert nicht  [0]
  veraltete Bloecke (alter Anlauf, nicht analysiert): 40
  ok      ein Neuanlauf ist kein Oversize
== O - Anlaufwechsel: Altbloecke sind SOFORT erkennbar (T2-3) ==
  ok      der Anlauf steigt sofort, nicht erst beim naechsten Audioblock  [0 -> 1]
  Bloecke aus dem ALTEN Anlauf: als veraltet erkannt 3, als aktuell durchgelassen 0
  ok      alle drei Bloecke des alten Anlaufs sind als veraltet erkennbar  [3/3]
  ok      der Produzent meldet exakt den Zug, der den Neuanlauf uebernimmt
  ok      Gegenprobe: der erste Block nach dem Neuanlauf gilt als aktuell
  ok      und er traegt die Luecke, die der Neuanlauf gerissen hat
  ok      der Folgezug meldet keinen bereits verbrauchten Neuanlauf erneut
  ok      zwei Neuanlaeufe hintereinander gehen beide nicht verloren  [1 -> 3]
  ok      zwei zusammengefallene Anforderungen ergeben eine uebernommene Endgeneration
  ok      und der Produzent holt sich die ENDGUELTIGE Nummer, nicht die erste
  ok      Nullframe konsumiert die wartende Generation nicht
  ok      der naechste echte Block uebernimmt sie weiterhin
== P - Seek bei gestopptem Transport ist eine Grenze (T2-4) ==
  ok      Seek um 10 s bei gestopptem Transport ist EIN Bruch  [1]
  ok      Gegenprobe: stehende Zeit bei Stopp bleibt KEIN Bruch (§4.3 Punkt 2)  [0]
  ok      und bei laufendem Transport ist der Seek weiterhin ein Bruch  [1]
  ok      bei Stopp ist eine LUECKENLOS fortschreitende Zeit kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      ohne gueltiges „spielt“ bleibt eine wandernde Zeit unbewertet  [0]
== Q - Worker-Kadenz: monotone Deadlines statt Batchzaehler ==
  ok      nach Start ist nur die leichte Publikation sofort faellig
  ok      10 000 Workerzuege bei gleicher Wanduhr loesen keine Schwer-Auswertung aus
  ok      vor 250 ms bleibt Gating/Kandidaten gesperrt
  ok      bei 250 ms ist genau die schwere Runde faellig
  ok      dieselbe Deadline kann nicht zweimal verbraucht werden
  ok      nach Pause genau eine Runde, kein Catch-up-Sturm
  ok      Reset/Generation startet die 250-ms-Deadline neu
== R - verdrahtet: Rueckstau beschleunigt Auswertung nicht; Stop bleibt begrenzt ==
  ok      Test hat echten Queue-Rueckstau hergestellt  [257 Bloecke bis zum ersten Drop]
  ok      unter Rueckstau hoechstens ~1 Schwer-Auswertung je 250 ms  [1 Runden in 600 ms]
  ok      neue Generation erbt keinen Batch-beschleunigten Schwertakt  [0 Runden in den ersten 100 ms]
  ok      Frame-Leser wird unter Rueckstau explizit vorgelassen  [0 ms]
  ok      Reset-Steuerer wird unter Rueckstau explizit vorgelassen  [2 ms]
  ok      Stop/Join bleibt auch mit Queue-Rest begrenzt  [0 ms]
== S - Projektfenster folgt der tatsaechlich uebernommenen Queue-Generation ==
  ok      Gegenprobe: erste Generation hat ihr eigenes Projektfenster
  ok      Testbarriere liegt deterministisch im laufenden Audiocallback
  ok      der erste Block der neuen Queue-Generation beginnt ein frisches Projektfenster  [10000..10064]
  ok      die Generationskante ist kein Seek innerhalb des alten Fensters  [0]
== T - ungueltige Samplerate deaktiviert beide Analyse-Engines ==
  ok      Gegenprobe: mit 48 kHz laufen M1 und FeatureEngine
  ok      nichtendliche Hostrate wird fail-closed auf 0 gespiegelt
  ok      M1 verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      FeatureEngine verarbeitet unter ungueltiger Rate keinen Block  [0]
  ok      ohne gueltige Analyserate behauptet auch das Projektfenster keine Messung
  ok      eine folgende gueltige Generation aktiviert beide Engines wieder

114 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Feingitter: 10001 Bins a 0.01 LU ueber [-70.00, 30.01) LUFS.
Oberband:   3070 Bins a 1.00 LU ueber [30.01, 3100.01) LUFS - lautheit(DBL_MAX) = 3081,86.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: kein Block faellt durch das Raster  [0]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: kein Block faellt durch das Raster  [0]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: kein Block faellt durch das Raster  [0]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: kein Block faellt durch das Raster  [0]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): kein Block faellt durch das Raster  [0]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: kein Block faellt durch das Raster  [0]
  ok      Musik + echte Stille: LUFS-I innerhalb ±0,1 LU  [akku=-8.281644 ref=-8.281644 d=0.000000000]
  ok      Musik + echte Stille: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Musik + echte Stille: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000027]
  ok      Musik + echte Stille: Kurz-LUFS Gueltigkeit gleich
  ok      Musik + echte Stille: Kurz-LUFS BITGLEICH  [akku=-12.451912590513381 ref=-12.451912590513381]
== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==
  Sweep: 201 Laeufe, groesste Abweichung 2.781360 LU bei Faktor 1.000000, groesste gemeldete Schranke 2.781412 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen (Eingang aendert sich um 1e-5): 2.781403 LU
  ok      Sweep vollstaendig gefahren  [201/201]
  ok      Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)  [sonst pruefte D nichts; schranke=2.781412]
  ok      die selbstgemeldete Schranke deckt JEDEN Lauf  [201/201]
  ok      wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU  [76/76]
  ok      die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)  [maxD=2.781360 refSprung=2.781403]
  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)
== E - fixed memory: eine Million Zellen ==
  Akku:     0 Allokationen, LUFS-I -8.386510, Schranke 0.000000000 LU
  Referenz: 36 Allokationen, LUFS-I -8.386510
  ok      1 000 000 Zellen: 0 Allokationen im Akku  [0]
  ok      Gegenprobe: die alte Rechnung alloziert sehr wohl  [36]
  ok      1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU  [d=0.000000000]
  Fester Bedarf laut speicherBytes(): 157122 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      Kurz-LUFS meldet einen nichtendlichen Ring nie als gueltig
  ok      ueber dem Gitter: EIN Pegel bleibt wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      Gegenprobe: der Korpus liegt wirklich im Oberband  [197]
  ok      und kein Block faellt durch das Raster
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer
== G - Oberband: zwei Pegel ueber dem Feingitter ==
  G1: akku=49.302611 ref=49.302611 schranke=0.000000000 ueberGitter=1997
  ok      G1: alle 1997 Bloecke liegen im Oberband (Gegenprobe)  [1997]
  ok      zwei Pegel ueber dem Gitter (T2-1): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      zwei Pegel ueber dem Gitter (T2-1): kein Block faellt durch das Raster  [0]
  ok      zwei Pegel ueber dem Gitter (T2-1): LUFS-I innerhalb ±0,1 LU  [akku=49.302611 ref=49.302611 d=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS Gueltigkeit gleich
  ok      zwei Pegel ueber dem Gitter (T2-1): Kurz-LUFS BITGLEICH  [akku=32.319299956639810 ref=32.319299956639810]
  ok      Naht, Gamma_r im Oberband: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Oberband: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Oberband: LUFS-I innerhalb ±0,1 LU  [akku=49.302481 ref=49.302481 d=0.000000000]
  ok      Naht, Gamma_r im Oberband: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Oberband: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Oberband: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  ok      Naht, Gamma_r im Feingitter: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Naht, Gamma_r im Feingitter: kein Block faellt durch das Raster  [0]
  ok      Naht, Gamma_r im Feingitter: LUFS-I innerhalb ±0,1 LU  [akku=49.243363 ref=49.243363 d=0.000000000]
  ok      Naht, Gamma_r im Feingitter: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Naht, Gamma_r im Feingitter: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS Gueltigkeit gleich
  ok      Naht, Gamma_r im Feingitter: Kurz-LUFS BITGLEICH  [akku=49.308999999999997 ref=49.308999999999997]
  G3-Sweep: 201 Laeufe, groesste Abweichung 2.781577 LU bei Faktor 1.001000, groesste gemeldete Schranke 2.781577 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen: 2.781401 LU
  ok      G3: Sweep vollstaendig gefahren  [201/201]
  ok      G3: JEDER Lauf liegt wirklich im Oberband (Gegenprobe)  [1997]
  ok      G3: der Grenzbin des Oberbands wurde wirklich getroffen (Gegenprobe)  [sonst pruefte G3 nichts; schranke=2.781577]
  ok      G3: die selbstgemeldete Schranke deckt JEDEN Lauf auch im Oberband  [201/201]
  ok      G3: die Abweichung bleibt unter dem eigenen Sprung der Referenz  [maxD=2.781577 refSprung=2.781401]
  ok      G3: kein Block faellt durch das Raster  [0]
  (Wie §D misst G3 an der Unstetigkeit der Norm, nicht an 0,1 LU - 1000 Bloecke kippen gemeinsam die Gateseite.)
  G4: LUFS-I 3038.851, Schranke 0.000000000, ueberGitter 197, ueberOberband 0
  ok      G4: die lauteste darstellbare Blocklautheit liefert einen endlichen Wert  [3038.851]
  ok      G4: nichts wird faelschlich als nicht-endlich gezaehlt  [0]
  ok      G4: auch am aeussersten Rand faellt kein Block durch das Raster  [0]
  ok      G4: und die Schranke bleibt endlich  [0.000000000]

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
109 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b5"></a>
#### B5 | EqCopAnalysisGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 2,09 s

stdout:

```text
== Nakama SONDE-009 - FeatureEngine v2: Zeit, Validity, Events, Baender ==
Gate: "Drop/Seek/Loop trennt jedes offene Fenster."
Stufen: Bass 16384 (Hop 8192), Haupt 4096 (Hop 2048), Trennung bei 200 Hz.

== A - Bandgitter: die einkompilierten Zahlen gegen die Fixtures ==
[ok]   Evidenzgitter: band_anzahl == kEvidenzBaender  [221]
[ok]   Livegitter: band_anzahl == kLiveBaender  [64]
[ok]   Evidenz-Kanten BITGLEICH zur Fixture (222 Werte)  [0 Abweichungen]
[ok]   Evidenz-Mitten BITGLEICH zur Fixture (221 Werte)  [0 Abweichungen]
[ok]   Live-Kanten BITGLEICH zur Fixture (65 Werte)  [0 Abweichungen]
[ok]   Live-Mitten BITGLEICH zur Fixture (64 Werte)  [0 Abweichungen]
[ok]   Live-Gruppierung deckt die 221 Feinbaender lueckenlos und genau einmal  [0 Abweichungen, Ende bei 221]
[ok]   Live-Kanten sind bitgleiche KOPIEN feiner Kanten, keine zweite Bank  [0 Abweichungen]

== B - Quantisierung: alle Vektoren aus quantisierung-v1.json ==
[ok]   q_db_0p1_i16: Skalierung stimmt mit der Fixture ueberein  [10]
[ok]   q_db_0p1_i16: alle 23 Vektoren treffen
[ok]   q_db_0p01_i16: Skalierung stimmt mit der Fixture ueberein  [100]
[ok]   q_db_0p01_i16: alle 23 Vektoren treffen
[ok]   float32: alle 15 Vektoren treffen bitgenau
[ok]   Rundungsfalle: floor(|x|+0.5) waere hier 1, std::round ist 0
[ok]   halbe Werte VON NULL WEG (0,05 -> 1), nicht zur geraden Zahl
[ok]   und negativ genauso (-0,05 -> -1)

== C - Gueltigkeitsbitmap: LSB-first, Fuellbits 0 ==
[ok]   Band i sitzt in Byte i/8, Bit i%8 (LSB-first)  [Byte0=81 Byte1=1]
[ok]   ein Bit loeschen laesst die Nachbarn stehen
[ok]   Lesen und Schreiben sind dieselbe Adressierung
[ok]   221 Baender brauchen 28 Bitmapbytes  [28]
[ok]   die 3 Fuellbits des letzten Bytes bleiben 0 (sonst waere ein Bytevergleich sinnlos)  [letztes Byte = 1f]

== D - FFT: gegen analytisch bekannte Ergebnisse ==
[ok]   Sinus auf einer Binmitte legt >99,9 % der Leistung in genau diesen Bin  [100.0000 %]
[ok]   Parseval haelt (Zeit- und Frequenzenergie gleich)  [rel. Fehler 0.0000000000000002]
[ok]   Gleichanteil sitzt vollstaendig in Bin 0  [Bin0 = 1048576.0, Rest = 0.00000000000000000000]

== E - K-Gewichtung: Frequenzgang gegen die BS.1770-Referenzkette ==
[ok]   RBJ-Kette und BS.1770-Referenzkette liegen ueber 20 Hz..20 kHz unter 0,1 dB auseinander  [max 0.04312 dB bei 20000.0 Hz]
       (gemessene Groesstabweichung, zur Ablesung: 0.043120 dB)
[ok]   Hochpass sperrt den Gleichanteil vollstaendig  [|H(0)|Â² = 0.000000000000000000000000]

== F - Zeitvertrag: Epoche, Segment, Sequenz, Zeitbasis ==
[ok]   die Sequenz zaehlt ab 1 aufwaerts  [6]
[ok]   mit gueltiger Projektzeit ist die Zeitbasis project_samples
[ok]   und project_sample_start ist gesetzt
[ok]   Gueltigkeitsbit project_time steht
[ok]   der Frame traegt seine Metrikversion  [20260823]
[ok]   und die Samplerate
[ok]   Frame-Stempel spannt alle integrierten Hostbloecke auf  [0 + 5120]
[ok]   Frame-Stempel umfasst den ueberlappenden FFT-Support  [2048 + 8192]
[ok]   ein vorgezogener FFT-Start zieht die Continuous-Time gleich weit zurueck
[ok]   Continuous-Time-Unterlauf laesst nur die optionale Uhr fallen
[ok]   fehlende Continuous-Werte im FFT-Support werden nicht rueckwaerts erfunden
[ok]   Evidenzframe vereinigt Haupt- und aelteren Basssupport  [0 + 30720, 196 Baender]
[ok]   stille Evidenzluecke kann fehlende Continuous-Time nicht verbergen
[ok]   stehende FL-Teilstuecke erfinden kein Projektintervall
[ok]   ueberlaufende Host-Projektspanne wird konservativ lokal publiziert
[ok]   Frame direkt nach Seek beschreibt nur den Nachgrenzen-Block  [104608 + 512]
[ok]   ohne Zeitbeweis faellt die Zeitbasis auf local_monotonic (Â§32.3)
[ok]   und dann steht WEDER das Bit NOCH ein Startwert da
[ok]   alle SIEBEN Gueltigkeitsbits gesetzt, wenn der Host alle meldet  [0x7f]
[ok]   eine GEMELDETE 0 ist etwas anderes als 'nie gesagt' (Â§32.3)
[ok]   Schleifen-Samplegrenzen sind `unproven` - es gibt kein FL-Golden dafuer
[ok]   continuous_time_samples kommt mit demselben Frameanker durch
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: NaN
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Inf
[ok]   unbrauchbare Cycle-Bounds werden nicht publiziert: Ende vor Start
[ok]   ungueltiger Folge-Prepare deaktiviert; gueltiger Reprepare erholt sich

== G - DAS GATE: Drop/Seek/Loop trennt JEDES offene Fenster ==
[ok]   G1 Drop (kFlagLueckeDavor): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G1 Drop (kFlagLueckeDavor): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G1 Drop (kFlagLueckeDavor): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G1 Drop (kFlagLueckeDavor): Ursache ist lokaleLuecke  [Zaehler 1]
[ok]   G1 Drop (kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche (§32.3)  [Epochen 0 Segmente 1]
[ok]   G2 Seek waehrend Wiedergabe: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G2 Seek waehrend Wiedergabe: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G2 Seek waehrend Wiedergabe: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G2 Seek waehrend Wiedergabe: Ursache ist zeitSprung  [Zaehler 1]
[ok]   G2 Seek waehrend Wiedergabe: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G3: bei gestopptem Transport laufen die Fenster normal weiter  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G3: eine STEHENDE Zeit bei Stopp ist keine Grenze (sonst stuerbe das Vorhoeren)  [0 Epochenwechsel]
[ok]   G3: eine SPRINGENDE Zeit bei Stopp trennt jedes Fenster  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G3: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G3: und sie heisst zeitSprung, nicht lokaleLuecke
[ok]   G4: aktive Schleife allein trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G4 Loop-Wrap: jedes Fenster getrennt  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G4: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G4: und die Ursache heisst loopWrap, nicht zeitSprung
[ok]   G5: Schleife ohne erreichbare Grenze trennt nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5: und die Bandakkus tragen Inhalt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G5 moeglicher Straddle: jedes Fenster getrennt - und WIRKLICH leer, weil der Straddle-Block auch kein neues beginnt  [Bass=0 Haupt=0 Zelle=0 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0]
[ok]   G5: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G5: der Straddle-Block selbst wird verworfen, nicht halb verwendet
[ok]   G5: mit eigener Ursache moeglicherStraddle
[ok]   G6 Transportkante (Play -> Stop): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G6 Transportkante (Play -> Stop): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G6 Transportkante (Play -> Stop): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G6 Transportkante (Play -> Stop): Ursache ist transportKante  [Zaehler 1]
[ok]   G6 Transportkante (Play -> Stop): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G7 Sampleratewechsel: vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G7 Sampleratewechsel: NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G7 Sampleratewechsel: und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G7 Sampleratewechsel: Ursache ist sampleratewechsel  [Zaehler 1]
[ok]   G7 Sampleratewechsel: zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G8 Neuanlauf (prepareToPlay): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G8 Neuanlauf (prepareToPlay): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G8 Neuanlauf (prepareToPlay): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G8 Neuanlauf (prepareToPlay): Ursache ist neuanlauf  [Zaehler 1]
[ok]   G8 Neuanlauf (prepareToPlay): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): vor der Grenze sind alle Fenster offen  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus tragen Inhalt - sonst prueft der Rest nichts  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): NACH der Grenze traegt kein Fenster ein Sample von davor  [Bass=512 Haupt=512 Zelle=512 Kurz=0 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=0 (erwartet je 512)]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): und die Bandakkus wurden dabei WIRKLICH verworfen  [690 Band-Fensterbeitraege gefallen]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): Ursache ist beweislageWechsel  [Zaehler 1]
[ok]   G9 Beweislagewechsel (Kontext faellt weg): zaehlt als EPOCHE, nicht als Segment  [Epochen 1 Segmente 0]
[ok]   G11: im Betrieb traegt die K-Kette einen Filterzustand  [1.392229]
[ok]   G11: beide Laeufe brauchen gleich viele Bloecke bis zum ersten LUFS-Frame  [290 / 290]
[ok]   G11: beide Laeufe liefern eine Kurzzeitlautheit
[ok]   G11: LUFS-S nach der Grenze ist BITGLEICH zum frischen Lauf - der Filternachklang ueberbrueckt nicht  [A=-60.034744263 B=-60.034744263]
[ok]   G10: FL-Teilstuecke mit stehender Projektzeit sind KEINE Grenze (NAK-56)  [0 Trennungen]
[ok]   G10: die Fenster laufen dabei weiter  [Bass=9224 Haupt=3080 Zelle=264 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   G12: vor der Grenze tragen BEIDE Bandakkus den Ton  [Live 149 / Evidenz 149 Baender]
[ok]   G12: der Grenzblock macht den Frame faellig - die Uhr laeuft ueber die Grenze weiter (Entscheid Â§10.1)  [liveSamples=0]
[ok]   G12: und der Frame traegt die NEUE Epoche  [1]
[ok]   G12: der Evidenzsatz ist in genau diesem Frame faellig - er wird also wirklich geprueft
[ok]   G12: KEIN Live-Band im Frame - der Ton von vor der Grenze ist weg (T2-1)  [0 gesetzte Baender]
[ok]   G12: und KEIN Evidenzband - die langsamere Kadenz reicht nicht weiter  [0 gesetzte Baender]
[ok]   G12: auch keine Aktivitaet auf Stille - die fertigen Zellen fielen mit  [nicht gesetzt]
[ok]   G12: die Akkus wurden GELEERT, nicht bloss leer vorgefunden  [1192 Band-Fensterbeitraege]
[ok]   G12: der Sweep hat genug Grenzzeitpunkte wirklich bis zum Frame gefahren  [120 von 120]
[ok]   G12: ueber 120 Grenzzeitpunkte traegt KEIN Frame etwas von davor (Pruefer-Messung: 80 / 40 / 43)  [0 mit Live-Baendern, 0 mit Evidenz, 0 mit Aktivitaet]

== G13 - ZWILLINGSPROBE: strukturelle Deckung statt Feldliste (T2R2-1) ==
[ok]   G13.0: zwei frische Frames sind gleich - der Vergleich sagt nicht pauschal ungleich
[ok]   G13.0: ein einziger geaenderter Skalar macht sie ungleich
[ok]   G13.0: ein einziger geaenderter Bandwert ebenso
[ok]   G13.0: und ein einziges Bitmapbit ebenso
[ok]   G13.0: und ein Feld im verschachtelten Transportstempel - der Vergleich reicht bis dorthin
[ok]   G13.0: nach dem Zuruecksetzen wieder gleich - beide Richtungen vorgefuehrt
[ok]   G13a lokaleLuecke [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13a lokaleLuecke [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13a lokaleLuecke [Zwilling]: beide haben die Grenze lokaleLuecke gezogen  [A 1 / B 1]
[ok]   G13a lokaleLuecke [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13a lokaleLuecke [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13a lokaleLuecke [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13a lokaleLuecke [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13a lokaleLuecke [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13b zeitSprung [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13b zeitSprung [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13b zeitSprung [Zwilling]: beide haben die Grenze zeitSprung gezogen  [A 1 / B 1]
[ok]   G13b zeitSprung [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13b zeitSprung [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13b zeitSprung [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13b zeitSprung [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13b zeitSprung [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13c transportKante [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13c transportKante [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13c transportKante [Zwilling]: beide haben die Grenze transportKante gezogen  [A 1 / B 1]
[ok]   G13c transportKante [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13c transportKante [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13c transportKante [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13c transportKante [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13c transportKante [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13d sampleratewechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13d sampleratewechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13d sampleratewechsel [Zwilling]: beide haben die Grenze sampleratewechsel gezogen  [A 1 / B 1]
[ok]   G13d sampleratewechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13d sampleratewechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13d sampleratewechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13d sampleratewechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13d sampleratewechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13e neuanlauf [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13e neuanlauf [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13e neuanlauf [Zwilling]: beide haben die Grenze neuanlauf gezogen  [A 1 / B 1]
[ok]   G13e neuanlauf [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13e neuanlauf [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13e neuanlauf [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13e neuanlauf [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13e neuanlauf [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]
[ok]   G13f beweislageWechsel [Zwilling]: A traegt vor der Grenze auf JEDEM Traeger etwas  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=1 LiveAkku=149 EvidAkku=196 Breite=149B/4.162e-03 RZellen=1 kFilter=8.569e-01 RAktiv=1]
[ok]   G13f beweislageWechsel [Zwilling]: B traegt auf keinem - die zwei sind wirklich verschieden  [Bass=13824 Haupt=3584 Zelle=64 Kurz=30 Fluss=0 LiveAkku=0 EvidAkku=0 Breite=0B/0.000e+00 RZellen=1 kFilter=0.000e+00 RAktiv=0]
[ok]   G13f beweislageWechsel [Zwilling]: beide haben die Grenze beweislageWechsel gezogen  [A 1 / B 1]
[ok]   G13f beweislageWechsel [Zwilling]: beide bauen ihre Frames im selben Block - die Uhr haengt nicht am Inhalt (Entscheid §10.1)  [0 Bloecke auseinander]
[ok]   G13f beweislageWechsel [Zwilling]: es wurden ueberhaupt genug Frames verglichen  [14 Frames]
[ok]   G13f beweislageWechsel [Zwilling]: und sie tragen Messwerte - verglichen wird nicht zweimal Stille  [13 von 14 mit Live-Baendern]
[ok]   G13f beweislageWechsel [Zwilling]: nach der Grenze ist A von B in JEDEM FELD nicht zu unterscheiden - kein Traeger hat ueberbrueckt (T2R2-1)  [14 Frames feldgleich]
[ok]   G13f beweislageWechsel [Zwilling]: und keiner erfindet ein Ereignis in der neuen Epoche  [A 0 / B 0]

== H - NAK-29: bedingte Feldpflichten des Transportstempels ==
[ok]   ein sauberer Stempel kommt durch
[ok]   Fall 1: project_samples + Bit, aber KEIN project_sample_start
[ok]   Fall 2: local_monotonic MIT project_time-Bit (Widerspruch zu Â§32.3)
[ok]   Fall 3: bounds_valid ohne start_ppq/end_ppq
[ok]   Fall 4: derivation=validated_block_mapping ohne bounds_valid
[ok]   Fall 5: cycle_bounds-Bit ohne die PPQ-Werte, auf die es sich bezieht
[ok]   Fall 6: continuous_time-Bit ohne continuous_time_samples
[ok]   Fall 7: das Projektintervall laeuft nicht ueber int64 hinaus
[ok]   Fall 7: sample_count bleibt unter der Vertragsobergrenze
[ok]   Fall 7: der Feature-Erzeuger publiziert keinen Leerframe
[ok]   der ERZEUGER loest den Riegel nicht aus - kein verletzter Stempel entsteht im normalen, gedeckelten Betrieb  [0]
[ok]   abgelehnte faellige Evidenz wird konsumiert und der Erzeuger erholt sich  [1]

== I - Ereignisse: kein Fluss ueber eine Grenze ==
[ok]   der spektrale Fluss hat im Betrieb einen Vorgaenger
[ok]   nach einem Seek ist der Vorgaenger WEG - sonst meldete der Fluss einen Onset, den es nie gab
[ok]   jedes Ereignis traegt die Epoche, in der es gesehen wurde  [1 Ereignis(se)]
[ok]   der Ereignisring laeuft WIRKLICH ueber - der Deckel ist gemessen, nicht behauptet  [64 im Ring, 1 verworfen, nach 1107 Bloecken]
[ok]   und er waechst dabei NICHT ueber seine 64 Plaetze hinaus (Â§48.1: ein Strom, der bei Ueberlast waechst, waere ein unbegrenzter Vektor)  [64 / 64]
[ok]   drop-oldest: beim Ueberlauf faellt das AELTESTE, nicht das neueste  [aeltestes vorher 38912, jetzt 47104]
[ok]   und der Ring gibt sie weiter aeltestes-zuerst zurueck, auch nach dem Umlauf
[ok]   Eventzeit ist der 4096er-Fensteranfang und unabhaengig vom 512er Hostblock
[ok]   stehende Host-Zeit erzeugt keine extrapolierte FFT-Event-Projektzeit  [1 Ereignis(se) nach 84 Bloecken]

== J - EBU: LUFS-S gegen die analytisch gerechnete Erwartung ==
[ok]   LUFS-S ist nach 4 s gesetzt
[ok]   LUFS-S trifft die analytische Erwartung auf besser als 0,1 LU (Â§39.3)  [gemessen -6.0620 erwartet -6.0620 d=0.00000]
[ok]   Peak trifft die Amplitude  [-6.021 dBFS]
[ok]   Crest eines Sinus ist 3,01 dB  [3.0091 dB]
[ok]   L==R ergibt Korrelation 1  [1.000000]
[ok]   und Breite 0 (kein Seitenanteil)  [0.000000000000]
[ok]   das Live-Band um 997 Hz ist gueltig  [Band 35]
[ok]   und das unterste Band (30 Hz) ist es NICHT - dort ist kein Signal

== K - NAK-56 Werkbankhaelfte: nakamaBlockEmpfangen am echten Prozessor ==
[ok]   die FeatureEngine hat Bloecke aus dem echten Prozessorweg gesehen  [32 Bloecke]
[ok]   und mindestens einen Frame gebaut  [3 Frames]
[ok]   process_context_present kommt aus der BRUECKE durch (Â§32.3)
[ok]   recording: Bit gesetzt, Wert false - die Bruecke kann beides sagen
[ok]   continuous_time_samples kommt durch
[ok]   die Schleifengrenzen kommen durch, samt `active`
[ok]   beide Presentation-Latencies kommen durch  [3924 / 4410]
[ok]   ueber die Bruecke stehen ALLE SIEBEN Gueltigkeitsbits  [0x7f]
[ok]   Gegenprobe: der Playhead-Rueckfallweg erreicht NICHT alle sieben Bits  [0x3]
[ok]   insbesondere kennt JUCEs Playhead kein Gueltigkeitsbit fuer `recording`
[ok]   Projektblock ueber INT64_MAX wird als ungueltiges Fenster verworfen
[ok]   Abstand INT64_MIN zu INT64_MAX wird ohne Subtraktionsueberlauf als Sprung erkannt
[ok]   nichtendliche Sampleraten initialisieren keine Analyseengine

== L - Gegenproben: kann dieses Bein ueberhaupt rot werden? ==
[ok]   L1: OHNE Grenze meldet die Messgroesse FALSE - sie reagiert also ueberhaupt  [Bass=9216 Haupt=3072 Zelle=256 Kurz=30 Fluss=1 LiveAkku=196 EvidAkku=196 Breite=196B/2.660e-04 RZellen=1]
[ok]   L1: MIT Grenze meldet sie TRUE - beide Richtungen vorgefuehrt
[ok]   L2: der Gittervergleich meldet gegen die UNVERAENDERTE Zeile 0 Abweichungen  [0]
[ok]   L2: und gegen eine um EIN Bit gekippte Zeile genau 1 - er reagiert also, und zwar nur dort  [1 Abweichung(en), Zeile 26]
[ok]   L3: sieben Verletzungen ergeben sieben VERSCHIEDENE Nummern, nicht siebenmal 'nein'
[ok]   L4: auf Stille ist KEIN Band gueltig - es steht nicht '0 dB' da, sondern nichts  [0 gesetzte Baender]
[ok]   L5: drei Versuche verbrauchen drei Sequenznummern - auch der abgelehnte  [3]
[ok]   L5: der Empfaenger sieht 1 und 3 - die LUECKE bei 2 ist die Meldung  [1, 3]
[ok]   L5: und der Versuch nennt den FALL, nicht nur 'abgelehnt'
[ok]   L6: ein echter Grund wird gezaehlt  [1]
[ok]   L6: `anzahl` ist kein Grund und liefert 0, statt hinter das Array zu lesen  [gelesen: 0]
[ok]   L6: und `keine` ebenso - getrennt wird nie ohne Grund

== M - Auskunft ueber die Grenzen der Messung ==
[ok]   die unteren Baender sind bei dieser Aufloesung NICHT messbar - und das steht da  [erstes messbares Band 2 (32.6 Hz)]
[ok]   bei 48 kHz greift die Kappe nirgends - das oberste Band endet bei 17,96 kHz  [erstes Band ueber der Kappe 221]
[ok]   bei 22,05 kHz greift die Nyquist-Kappe WIRKLICH und schneidet oben ab  [erstes Band ueber der Kappe 202 (10525.6 Hz)]
[ok]   und sie sitzt genau am ersten Band, dessen Oberkante 0,95Â·Nyquist ueberschreitet  [10374.7 .. 10678.7 Hz gegen 10473.8 Hz]
[ok]   kein Live-Band vollstaendig ueber der Kappe traegt einen Wert  [0 Baender]
[ok]   Gegenpfad: neu vorbereiten setzt alles zurueck, auch die Zaehler

Ergebnis: 235 bestanden, 0 Fehler.
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,07 s

stdout:

```text
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  ok      frisch: unclassified  [unclassified]
  ok      frisch: neutral
  ok      frisch: keine Audio-Ausnahme (audio-neutral)
  ok      frisch: kein Brokerstart
  ok      Schema-1 'sensor' -> legacy  [legacy]
  ok      Schema-1 'sensor': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'pre' -> legacy  [legacy]
  ok      Schema-1 'pre': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'post' -> legacy  [legacy]
  ok      Schema-1 'post': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'hub' -> main  [main]
  ok      Schema-1 'hub': Audio-Ausnahme moeglich
  ok      Schema-2 main (Kind-Matrix bestaetigt) -> main
  ok      Schema-2 legacy -> legacy
  ok      Vorbereitung: erst main
  ok      unbekanntes Major danach -> zurueck auf unclassified
  ok      Muell (ignoriert) aendert nichts - main bleibt main
  ok      Muell auf einen frischen Automaten klassifiziert nicht
  ok      explizite Initialisierung OHNE Editor: abgelehnt
  ok      ... und der Automat bleibt neutral
  ok      der geoeffnete Editor allein klassifiziert NICHT
  ok      Editor offen + explizite Initialisierung: angenommen
  ok      ... -> main
  ok      main + offener Editor darf den Broker starten
  ok      Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)
  ok      main OHNE offenen Editor darf NICHT starten
  ok      Rueckweg: derselbe Akt fuehrt von main zurueck
  ok      ... -> legacy
  ok      legacy darf nie starten - auch mit Editor nicht
  ok      legacy ist wieder audio-neutral
  ok      Sonde (NkPr) frisch: neutral trotz fester Produktklasse
  ok      Sonde (NkPr): gueltiger eigener Stand -> passive_probe
  ok      Sonde (NkPr): kein Brokerstart
  ok      Sonde (NkPr): auch mit offenem Editor kein Brokerstart
  ok      Sonde (NkPr): kein Akt macht sie zum Main
  ok      Sonde (NkAc): gueltiger eigener Stand -> active_probe
  ok      Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main

[2] Verdrahtet: EqCopilotProcessor (Gen)
  ok      Scannerlauf: klassifiziert NICHT (53.5)  [unclassified]
  ok      Scannerlauf: kein Brokerstart
  ok      Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)
  ok      Scannerlauf: Markierung meldet sich nicht hoerbar
  ok      Altprojekt 'sensor' -> legacy  [legacy]
  ok      legacy mit offenem Editor: kein Brokerstart
  ok      legacy ist 'immer passiv' - kein Sample gefaerbt
  ok      Altprojekt 'hub' -> main  [main]
  ok      main, aber Editor zu: noch kein Brokerstart
  ok      main + offener Editor: Brokerstart erlaubt
  ok      main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)
  ok      Vorbereitung: Rollenwahl aendert den Stand
  ok      Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)  [unclassified]
  ok      der geoeffnete Editor allein holt es auch nicht nach
  ok      Vorbereitung: der User waehlt die Rolle im offenen Editor
  ok      frische Instanz: Editor offen + Rollenwahl 'hub' -> main
  ok      frische Instanz als main: faerbt
  ok      Rueckweg: 'sensor' klassifiziert zurueck auf legacy
  ok      Rueckweg: der Schnitt auf neutral greift sofort
  ok      Vorbereitung: main
  ok      der fremde Stand kommt read-only zurueck
  ok      read-only -> zurueck auf unclassified
  ok      read-only: kein Brokerstart
  ok      read-only: wieder audio-neutral

LEBENSLAUF-TEST OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 35,07 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopPipeClientTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_rc_lib.dir\Release\EqCopPipeClientTest_rc_lib.lib
  EqCopPipeClientTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSunaNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_rc_lib.dir\Release\EqCopSunaNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopSunaNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
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
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopAnalysisGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_rc_lib.dir\Release\EqCopAnalysisGoldenTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopAnalysisGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaSuna_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_rc_lib.dir\Release\NakamaSuna_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaSuna.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\Nakama Suna_SharedCode.lib
  NakamaSuna_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaSuna_vst3_helper.exe
  NakamaSuna_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\x86_64-win\Nakama Suna.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaSuna_artefacts/Release/VST3/Nakama Suna.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>


---

## Abschluss T3 — Dirigent 2026-08-28

**Marke:** `T3 PASS 2026-08-28`. Geprüfter Stand `f208c45`; Basis-SHA des Tickets `3c64b98`. Prüfer: Codex `gpt-5.6-sol`, Effort `xhigh` (Pflicht: Vertrag), vier frische Threads, jeder nur lesend; Nacharbeit jeweils im Thread des Prüfers, Commit durch den Dirigenten.

| Runde | Thread | Stand | Urteil | Befunde |
|---|---|---|---|---|
| 1 | `01a04934-4ae5-7a62-8da5-fe41015569ea` | `6c314e1` | NEEDS_WORK | (1) nur `lufs_s` hatte ein NaN-Fixture, obwohl beide Leser sieben Frame-Felder, `sample_rate` und die Schleifenfelder auf Endlichkeit prüfen — ein aus einem Leser entferntes Feld blieb unsichtbar (Klasse §4.3); (2) `pruefe_flatc_drift.py` prüfte nur `--version`, nicht den gepinnten Commit. → `3122103`: je Float-Träger (11) ein `nicht_endlich`-Fixture, Binärkorpus 55 → 62, `sample_rate` meldet `nicht_endlich` getrennt vom Bereich; CMake misst `git rev-parse HEAD` im FetchContent-Checkout und schreibt einen Sidecar, den der Drift-Test verlangt. |
| 2 | `01a0494a-b98c-77c1-be71-004e80e48c7c` | `3122103` | NEEDS_WORK | Sidecar entstand per `file(GENERATE)` beim Configure, nicht an `flatc.exe` gebunden. → `0fdd2c3`: POST_BUILD des flatc-Ziels (Projekt-Hook) schreibt `commit` + `sha256` des gebauten Binaries; der Prüfer hasht das ausgeführte flatc selbst; fehlend/abweichend = Exit 2. |
| 3 | `01a04967-d179-7c33-959e-a2129b51d921` | `0fdd2c3` | NEEDS_WORK | Commit war ein Configure-Literal im POST_BUILD. → `f208c45`: `NakamaFlatcBeleg.cmake` misst Commit und `git status --porcelain` zur Bauzeit, bricht bei Abweichung oder schmutzigem Checkout ab. Dritte Runde auf User-Wort „einfach immer weiter machen" (28.08.). |
| 4 | `01a0497d-c499-7be2-8370-1473cb70cd60` | `f208c45` | **PASS** | keine. Wortlaut: „Beide Leser prüfen sämtliche elf Float-/Double-Felder, einschließlich PPQ-Endlichkeit und bedingter Ordnungsregel; der Binärkorpus deckt jeden Träger einzeln ab. Feld-ID- und flatc-Provenienzriegel entsprechen dem Vertrag." |

**Tatsächlich gelaufene Beweise** (Dirigent, echter Rechner): Kanon `tools/beweise.ps1 -Bauen` GRÜN 29/29 auf `6c314e1`, `3122103`, `0fdd2c3` und `f208c45` (Anhänge oben); A9 `pruefe_flatc_drift.py` Drift 0 mit Commit- und SHA-256-Beleg; Rust `contract_cross_language` 8/8; C++ B3c Binärkorpus 62 wie das Manifest. **Einmalige Voraussetzung** nach dieser Änderung auf jedem Rechner: `cmake --build eq-copilot/build --config Release --target flatc` (der Sidecar entsteht nur beim Bau von flatc; ohne ihn ist A9 absichtlich ROT — so gemessen am 28.08., Exit 2 vor dem Neubau).

**Nicht geprüft:** die JSON-Hälfte des Gates (S5, eigenes PASS 28.08.). Die Codex-Sandbox konnte weder `py -3.13` noch CMake fahren; diese Beine hat der Dirigent gemessen.
