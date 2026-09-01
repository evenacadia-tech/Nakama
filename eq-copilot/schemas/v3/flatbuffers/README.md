# Der binäre Teil des v3-Vertrags (SONDE-005b)

Entwurf §33.1 teilt IPC v3 in zwei Nutzlastklassen:

> „kanonisches UTF-8-JSON für Handshake, Session, Zustand, Evidenz und
> Transaktionen; ein versioniertes FlatBuffers-`FeatureBatch` mit CRC32C für
> die häufigen P2-Featureframes."

Das JSON liegt eine Ebene höher (`../eq-ipc-v3.schema.json`), hier liegt das
Binäre.

| Datei | Was |
|---|---|
| `nakama_telemetry_v1.fbs` | Das Schema. Jedes Tabellenfeld trägt ein explizites `id`. |
| `FELD-IDS.json` | Die eingefrorene Name→ID-Zuordnung. Handgeschrieben. |
| `WERKZEUG.json` | Der `flatc`-Pin: Version, Repo, **Commit**, Codegen-Aufrufe. |

| Bein | Wo | Prüft |
|---|---|---|
| C++ | `EqCopSchemaTest` (`plugin/vertrag/NakamaTelemetrie.*`) | Urteil **und** Verstoßmenge |
| Rust | `broker/tests/contract_cross_language.rs` (`broker/src/telemetrie.rs`) | Urteil **und** Verstoßmenge |
| Drift | `tools/eq-copilot/pruefe_flatc_drift.py` | Neugenerierung ist bytegleich |

Beide Leser messen gegen dasselbe **handgeschriebene**
`../../../fixtures/v3/flatbuffers/MANIFEST.json`. Dieselbe Konstruktion wie
beim JSON-Korpus: das Manifest ist die dritte Partei; stimmen beide Leser je
mit ihm überein, stimmen sie transitiv miteinander überein.

---

## Warum jedes Feld ein `id` trägt

FlatBuffers erlaubt Schemaevolution normalerweise nur durch **Anhängen** am
Tabellenende — die Feldreihenfolge IST dann die Wire-Identität. Trägt jedes
Feld ein explizites `id`, ist die Reihenfolge im Text bedeutungslos und die
Zahl ist die Identität. Erst damit ist „ein Feld einsortieren" keine stille
Formatänderung mehr.

**`flatc` erzwingt davon nur die Hälfte.** Benutzt eine Tabelle ids, müssen
alle ihre Felder welche haben und sie müssen bei 0 lückenlos sein. Eine später
ergänzte Tabelle **ganz ohne** ids übersetzt anstandslos und fällt still auf
„Reihenfolge ist Identität" zurück. Genau diese Lücke schließt
`tools/eq-copilot/pruefe_fbs_feldids.py` — vorgeführt an vier Mutationen,
darunter dieser Fall.

Den zweiten Satz aus §53 — „IDs werden nie wiederverwendet, auch nicht nach
Entfernen eines Feldes" — kann `flatc` grundsätzlich nicht prüfen: eine Datei
kennt ihre eigene Vergangenheit nicht. Deshalb liegt die Vergangenheit als
`FELD-IDS.json` daneben, mit einer `verbrannt`-Liste je Tabelle.

---

## Warum der Pin ein Commit ist und kein Tag

Gemessen am 21.08.2026 über die GitHub-Tags-API führt der FlatBuffers-Upstream
für Version 25.12.19 **zwei** Tags (`v25.12.19` und
`v25.12.19-2026-02-06-03fffb2`) — der Beweis, dass eine Version nachträglich
neu geschnitten wird. Ein `GIT_TAG`-Pin auf den Namen wäre ein Pin auf einen
beweglichen Ref, und „Codegen-Drift ist 0" wäre eine Aussage über ein
bewegliches Ziel.

Drei Dinge müssen dieselbe Version tragen, sonst ruft der erzeugte Code in
eine Laufzeit, die er nicht kennt:

| | woher | wer prüft |
|---|---|---|
| `flatc` | aus dem gepinnten Commit **gebaut** | `NakamaFlatBuffers.cmake` (Bauabbruch bei anderer Version) |
| C++-Laufzeitheader | derselbe Commit | zusätzlich ein `static_assert` **im erzeugten Header selbst** |
| Rust-Crate | `broker/Cargo.lock` (die AUFGELÖSTE Version, nicht die Anforderung aus `Cargo.toml` — `"25.12.19"` heißt in Cargo `^25.12.19`) | `pruefe_flatc_drift.py` |

`flatc` wird gebaut und nicht als Release-Binary geladen: das vorkompilierte
Windows-Binary hängt an einem Release-Tag, die Header kämen aus dem
Quell-Tarball — zwei Downloads, deren Zusammengehörigkeit außer dem
Dateinamen nichts belegt. Ein Commit belegt sie.

---

## Was der Leser prüft — und warum es ihn überhaupt gibt

FlatBuffers prüft beim Verifizieren die **Struktur**: Offsets zeigen in den
Puffer, `required`-Felder sind da, Vektorlängen passen zum Puffer. Beide Leser
benutzen das, statt es nachzubauen.

Es prüft **nicht**:

* ob ein Enumwert im deklarierten Bereich liegt — ein `ubyte` 99 kommt
  unbeanstandet durch `Verify`, und erst der Leser sieht es;
* ob ein Bitflagfeld nur bekannte Bits trägt;
* **irgendeine** Beziehung zwischen zwei Feldern.

Genau dort liegen aber die Regeln aus §33.1. Ein Vertrag, der sie nur in
seinen Kommentaren trägt, ist keiner.

### Und der Verifier ist in den beiden Sprachen NICHT gleich stark

Das ist die teuerste Lehre dieses Tickets, gefunden in T2-Runde 3:

> **Der Rust-Verifier prüft String-Inhalte auf gültiges UTF-8. Der
> C++-Verifier tut das nicht.**

Ein einziges Byte (`0x80` in der `logon_sid`) reichte: die Rust-Seite lehnte
ab, die C++-Seite sagte „gültig" — und ab der **zweiten** Verarbeitung starb
der Prozess mit `STATUS_HEAP_CORRUPTION`. Ursache war
`juce::String::fromUTF8` auf unvalidierten Bytes: für ein nacktes
Fortsetzungsbyte liefert `getAndAdvance()` Codepunkt 0, während `isEmpty()`
dasselbe Byte als „nicht Ende" liest — Längen- und Kopierdurchlauf laufen
verschieden weit. In einem systematischen Byte-Fuzz über die gültigen
Fixtures liefen **78 von 591** Mutanten auseinander, neun stürzten ab.

Der Vertrag war darauf gebaut, dass sich **dieselbe Bibliothek in zwei
Sprachen gleich verhält** — dieselbe Fehlerklasse wie das Vertrauen auf
JUCEs Zahlenleser eine Ebene höher. Die C++-Seite prüft die UTF-8-Gültigkeit
jetzt selbst, über die **echte Länge** (FlatBuffers-Strings dürfen ein NUL
enthalten), mit `str::from_utf8`-Semantik: Überlangkodierungen, Surrogate und
alles über U+10FFFF sind ungültig. Der gemeldete Verstoß ist absichtlich
derselbe wie auf der Rust-Seite (`""` / `verifier`) — dort fällt der Puffer im
Verifier, hier im Nachschlag, aber es ist dieselbe Aussage an derselben Stelle
der Verstoßmenge.

Und: die kurze Gegenrichtung. Bei einem Puffer unter 8 Byte **panickte die
Rust-Seite** im `flatbuffers`-Crate, während C++ sauber `dateikennung`
meldete. Beide tragen jetzt denselben Längenriegel. Ein Panic im Broker
beendet den Thread, der die Pipe bedient — auch das ist keine Klassifikation.

### Die geschlossene Regelliste

Ein Verstoß ist ein Paar `(pfad, regel)`. `pfad` ist ein JSON-Pointer in den
Batch (`""` ist der Batch selbst), `regel` einer der folgenden Namen. Beide
Leser bilden denselben Text und sortieren die Menge kanonisch nach
`(pfad, regel)`, damit der Vergleich nicht von der Auswertungsreihenfolge
abhängt.

| Regel | Bedeutung |
|---|---|
| `dateikennung` | Die vier Bytes an Offset 4 sind nicht `NKT3`. Das Erste, was ein Leser sehen kann — ein fremder Puffer fällt hier, **vor** jedem Feldzugriff. |
| `verifier` | Der FlatBuffers-Verifier lehnt den Puffer ab (abgeschnitten, Offset ins Leere). |
| `eintraege_leer` | Ein Batch ohne Einträge trägt nichts. |
| `eintraege_zu_viele` | Mehr als 32 (§53.9: „bis zu 32 Sonden"). |
| `quelle_doppelt` | Zwei Einträge mit derselben `instance_id`. §33.1: „Ein Batch trägt nie mehrere Frames derselben Quelle." **Genau diese Regel erspart dem Format die zweite Wrapper-Ebene** — sie kann kein Schema ausdrücken. |
| `hex32` | Eines der vier Token ist nicht 32 Kleinbuchstaben-Hexziffern. |
| `sid_laenge` | `logon_sid` ist leer oder länger als 184 Zeichen. |
| `enum_unbekannt` | Ein Enumfeld trägt `unbekannt` (= 0, der Default eines fehlenden Skalarfelds) oder einen Wert außerhalb der Deklaration. |
| `validity_unbekanntes_bit` | Ein gesetztes Bit außerhalb der sieben aus §32.3. |
| `context_bit_fehlt` | `process_context_present` ist nicht gesetzt. §32.3 wörtlich: „Die Wrapper-Bridge liefert deshalb `process_context_present` und unabhängige Validity-Bits." Es ist ein **Optional** und kein `bool` mit Default — sonst wäre „der Sender hat es weggelassen" ununterscheidbar von „der Host hat keinen Context angelegt", und das sind zwei verschiedene Konfidenzaussagen. |
| `sample_count_bereich` · `sample_rate_bereich` | Dieselben Grenzen wie im JSON-Vertrag. |
| `metrics_version` | Kleiner als 1 — eine Kennzahl ohne Version ist später nicht vergleichbar. |
| `zwei_traeger` · `kein_traeger` | Beide Wertevektoren gesetzt (zwei Wahrheiten über dieselbe Messung) bzw. keiner (ein Bandsatz ohne Bänder). |
| `encoding_passt_nicht` | Der gesetzte Träger passt nicht zur Kodierung. §33.1 wörtlich: „Empfänger raten die Skalierung nie aus dem Nachrichtentyp." |
| `bandzahl` | Die Werteanzahl folgt nicht der `gitter`-Angabe (221 bzw. 64). Die Gitter sind als Zahlen **eingefroren**. |
| `bitmap_laenge` | Nicht `ceil(n/8)` Bytes — gemessen an der **tatsächlichen** Werteanzahl, nicht an der vom Gitter geforderten; sonst wäre es eine zweite Klage über dieselbe Ursache. |
| `bitmap_fuellbits` | Die ungenutzten Bits des letzten Bytes sind nicht 0. Sonst erzeugen zwei Sender für **dieselbe** Messung zwei verschiedene Puffer, und ein Bytevergleich wäre keine Aussage mehr. |
| `bandwert_bereich` | Ein i16-Bandwert außerhalb von **`plausibler_bereich_db`** in `quantisierung-v1.json` (−144…+24 dB, in Trägerwerte umgerechnet). **Nicht** `bereich_db` — das ist der Trägerumfang (±32767) und wäre als Regel wirkungslos. Beide Leser vergleichen ihre einkompilierten Konstanten bei jedem Testlauf mit der Vertragsdatei; vorher stand die Zahl nur im Quelltext, während hier `bereich_db` als Quelle genannt war. |
| `nicht_endlich` | NaN oder ±inf auf der Leitung. `quantisierung-v1.json` legt fest, dass Nichtendliches beim **Erzeugen** zu Wert 0 mit `gueltig=false` wird — auf der Leitung ist es ein Senderfehler und wird abgelehnt, nicht saniert. |
| `korrelation_bereich` · `breite_negativ` | Korrelation außerhalb [-1, 1]; negative Breite. |
| `grenzen_verdreht` | Abgeleitete Schleifengrenzen mit Ende vor Anfang. |
| `ppq_verdreht` | Rohe PPQ-Schleifengrenzen mit Ende vor Anfang. |
| `project_time_bit_fehlt` · `project_sample_start_fehlt` | `zeitbasis=project_samples` verlangt das Project-Time-Bit und `project_sample_start`. |
| `local_project_time_bit` · `local_project_sample_start` | `zeitbasis=local_monotonic` verbietet Project-Time-Bit und `project_sample_start`. |
| `cycle_start_ppq_fehlt` · `cycle_end_ppq_fehlt` | `schleife.bounds_valid=true` verlangt beide PPQ-Grenzen. |
| `validated_mapping_ohne_bounds` | `bounds_valid=false` darf keine `validated_block_mapping`-Ableitung behaupten. |
| `cycle_bounds_start_ppq_fehlt` · `cycle_bounds_end_ppq_fehlt` | Das `cycle_bounds`-Validity-Bit verlangt beide rohen PPQ-Grenzen. |
| `continuous_time_samples_fehlt` | Das `continuous_time`-Validity-Bit verlangt `continuous_time_samples`. |
| `band_stereo_gitter` · `band_stereo_encoding` · `band_stereo_werte_i16` | Das optionale ID-10-Feld benutzt ausschließlich `nakama_log64_v1`, `float32` und keinen i16-Träger. |
| `band_stereo_bandzahl` · `band_stereo_bitmap_laenge` | Stereo trägt genau 64 Werte und ein passendes 8-Byte-Bitmap. |
| `band_stereo_bereich` · `band_stereo_saturated` | Jeder im Bitmap gesetzte Stereoanteil ist endlich und in [0,1]; `saturated` bleibt false. `nicht_endlich` benennt wie bei allen float32-Werten NaN/±inf. |
| `lufs_i_paar` | `lufs_i` und `lufs_i_unsicherheit_lu` sind entweder beide präsent und endlich oder beide abwesend. Ein halbes oder nichtendliches Paar wird verworfen; andere Framefelder bleiben prüfbar. |
| `lufs_i_status` | Ein präsenter Status ist genau 1 (`collecting`) oder 2 (`gated`). |
| `lufs_i_status_mit_paar` | `lufs_i_status` reist nie zusammen mit einem gültigen endlichen Lautheitspaar. |

Die sechs Transportrelationen sind damit in JSON und FlatBuffers dieselbe
Aussage. Jede Relation besitzt ein regenerierbares Binär-Negativfixture; beide
handgeschriebenen Leser müssen dieselbe vollständige Verstoßmenge liefern.
`band_stereo` ist als optionales Tabellenendfeld `(id: 10)` additiv, erhöht
aber den tatsächlich gesendeten Envelope-`schema_minor` auf 1. Alte Frames
ohne das Feld bleiben gültig.

### Warum `unbekannt = 0` in jedem Enum steht

FlatBuffers liefert für ein fehlendes Skalarfeld den Default, und der ist 0.
Wäre 0 eine gültige Bedeutung, ließe sich „der Sender hat das Feld
weggelassen" nicht mehr von „der Sender meinte genau das" unterscheiden.
§33.1 verlangt für Discriminator und Capability ausdrücklich Ablehnung bei
unbekannter Bedeutung — `unbekannt = 0` macht das Weglassen zu genau so einem
Fall.

---

## Was hier bewusst NICHT steht

| Fehlt | Eigentümer | Warum nicht hier |
|---|---|---|
| `schema_major` / `schema_minor` im Payload | `SONDE-010` | Der 16-Byte-Envelope aus §33.1 trägt beide bereits. Sie hier zu wiederholen hieße, zwei Wahrheiten über dieselbe Zahl in denselben Frame zu legen. Die Formatidentität trägt `file_identifier`; die Feldevolution trägt die id-Disziplin. |
| Der 16-Byte-Envelope und CRC32C | `SONDE-010` | Entwurf §65 gibt den Parser diesem Ticket. Fixtures ohne Implementierung wären toter Ballast. |
| Ein produktiver **Aufrufer** der Leser | `SONDE-010` | `pruefe()` wird heute ausschließlich aus den beiden Testtreibern gerufen. Wo der Leser einmal steht — als Eingangsprüfung des IPC-Clients — ist damit noch nichts über sein Laufzeitverhalten unter Last gesagt; Ratengrenzen und Backpressure gehören dorthin. |
