# Abdeckungskarte — welche Zusage hat einen Detektor?

**Kanonische Datei, überschrieben; Git-Historie ist das Archiv.** Erste
Fassung 12.09.2026, gerechnet am Stand `90b3bdec` als Prüfsession in
`nakama-d8` nach `docs/gesundheit/KONZEPT.md` §4.2: fünf lesende Agenten
haben je Befundklasse die Population gezählt (Objekte, Felder, Beine) und je
Objekt den Test genannt; drei Skeptiker haben jede gemeldete Lücke zu
widerlegen versucht. Von 19 gemeldeten Lücken sind **11 widerlegt, 5
teilweise, 3 bleiben**. Nur Überlebende stehen unten als Befund; die
Widerlegungen stehen mit Beleg dabei, damit der nächste Lauf sie nicht neu
findet.

Lesart der Stärke: ✔ Klassen-Detektor misst die nächste Instanz · ◐
Beispieltests und Prosa fangen die bekannten · ○ nur Prosa · — kein Besitzer.
Zeilenangaben gelten zum Stand `90b3bdec` und driften mit jedem Commit;
der Bezeichner ist der Anker.

## Kopf: Urteil, Befunde, Zähler

**Urteil.** Nakamas Kanon ist bei den bekannten Instanzen dichter, als die
erste Fassung des Konzepts annahm: K4 und K6 haben je einen echten
Laufzeit- oder Byte-Detektor, K3 hat auf der Broker-Seite eine generische
Schema-Engine. Klassen-Detektoren fehlen dort, wo eine **neue** Instanz
(neue Queue ohne Politik, neues Zustandsfeld ohne Fixture, neuer C++-Leser
ohne Grenze, neues CMake-Ziel ohne Bein) still durchliefe. Genau diese vier
Stellen beschreiben die Aufbauschritte 2 bis 4 des Konzepts.

**Befunde zur Weitergabe** (Klasse nach Dirigent §3.4; kein Befund bricht
eine messbare Zusage, deshalb keine Defekte. A-1 bis A-5 stehen seit
12.09.2026 als NAK-267 im Register, Commit `fdeef23c`; A-6 und A-7 sind
Inhalt von Aufbauschritt 2, NAK-260):

| Nr | K | Befund | Beleg | Klasse |
|---|---|---|---|---|
| A-1 | K1 | Verdrängung in `session_command_reihenfolge` (Kappe als Konstante ohne Absichtskommentar; Idempotenz nur aus der flüchtigen Map, kein Store-Rückfall) hat weder Test noch Kommentar noch Registerzeile | `broker/src/coordinator/befehl.rs` (Konstante in `broker/src/coordinator/mod.rs`, `session_command` in `befehl.rs`) | [Härtung/Struktur] |
| A-2 | K3 | Die C++-Generalprüfung `Schema::pruefe` (alle Schemagrenzen) ist Produktionscode in `NakamaKern`, wird aber nur in `SchemaTestMain` instanziiert; ihr Kopf verspricht „Ab SONDE-010 ist das die Eingangspruefung des C++-IPC-Clients" | `eq-copilot/plugin/vertrag/NakamaVertrag.h` Kopf, `eq-copilot/plugin/vertrag/NakamaVertrag.cpp` (`pruefeWert`), `eq-copilot/plugin/tests/SchemaTestMain.cpp` | [Härtung/Struktur]; Dirigent 12.09.2026: kein Defekt, der Gate-Text von S14–15 verlangt die Hüllenprüfung, nicht die Verdrahtung von `Schema::pruefe` im Produkt-Client; der Kopfkommentar wird mit NAK-262 berichtigt oder eingelöst |
| A-3 | K4 | Drei MainProject-Felder (`confirmed_members_v1`, `manual_passages_v1`, `assistant_step_v1`) haben Kanon-Roundtrips, aber kein eingefrorenes Fixture und keinen Feldmengen-Test | `tools/eq-copilot/erzeuge_state_fixtures.py` (Kommentar zu `main-intent-v1.bin`), `eq-copilot/fixtures/state/MANIFEST.json` | [Härtung · Test] |
| A-4 | K6 | Die Laufzeit-Wache des EQ-Kerns (`RtWache`: Sperren, Allokationen, Wanduhr je Callback) liegt nicht am Messpfad `processBlock` des Prozessors | `eq-copilot/plugin/dsp/DspRtWache.h`, Eintritt in `eq-copilot/plugin/dsp/DspKern.cpp`; `eq-copilot/plugin/src/PluginProcessor.cpp` (`processBlock`) ohne Wache | [Härtung/Struktur] |
| A-5 | K6 | Der Allokationszähler der Tests fängt nur `operator new`; `malloc`, `calloc`, `realloc` und `juce::HeapBlock` bleiben unsichtbar, ohne Kommentar oder Riegel | `eq-copilot/plugin/tests/QueueStressTestMain.cpp` (Zähler), Zwilling in `eq-copilot/plugin/tests/DspGoldenTestMain.cpp` | [Härtung · Test] |
| A-6 | K5 | Kein Werkzeug fährt `cargo clippy`, `cargo deny`, `cargo audit`, `osv-scanner`, `gitleaks`, `typos` oder `jscpd`; Clippy nur als Zählung in `gesundheit.py --clippy`, das Kanon-Bein A32 ruft ohne `--clippy` | `tools/beweise.ps1` (A32), `tools/plan/gesundheit.py` | Inhalt von Aufbauschritt 2 (Prüfgang), keine eigene Zeile |
| A-7 | K5 | Die Regel „jedes `EqCop*`-Ziel ist Bein oder stillgelegt" lebt nur als Prosa: fünf Ziele stehen dokumentiert außerhalb des Kanons (`docs/plugin-wissen.md` „Nicht im Kanon"), NAK-37 nennt das fehlende Bein für `EqCopAuxSpikeTest` | `eq-copilot/plugin/CMakeLists.txt`, `docs/plugin-wissen.md` | Vakuum-Wache in Aufbauschritt 2 mit Ausnahmeliste aus dieser Prosa |

**Zähler je Klasse** (Belege aus Prüfliste, Audit D1–D10 und Register, plus
diese Karte; die Zweitbefund-Regel aus `KONZEPT.md` §2 zählt hier):

| K | Klasse | Belege vor dieser Karte | Neu | Stärke | Antwort |
|---|---|---|---|---|---|
| K1 | Rückstau und Ordnung | Prüfliste A; D5, D9; NAK-165 | A-1 | ◐ | Lebenslauf-Audit (Schritt 3) |
| K2 | Lebenslauf und Besitz | Prüfliste B; D1, D2, D3; NAK-184 | keine bestätigt | ◐ | Lebenslauf-Audit (Schritt 3): Join-Fristen ungeprüft, siehe K2 |
| K3 | Vertrag und Zahlenränder | Prüfliste C; D7, D8; NAK-159 | A-2 | Rust ✔ · C++ ◐ | Vertragstreue-Audit (Schritt 4) |
| K4 | Zustand und Paarung | Prüfliste F; D6 | A-3 | ◐ (dicht) | Zustandstreue-Audit (Schritt 4), eng |
| K5 | Behauptung ≤ Messung | Prüfliste D, E; D4, D10; NAK-93, NAK-94, NAK-230 | A-6, A-7 | ◐ | Prüfgang (Schritt 2) |
| K6 | Echtzeit | Grundgesetz | A-4, A-5 | ◐ (EQ-Kern ✔) | Echtzeit-Audit (Schritt 7), kleiner als geplant |
| K7 | Größe und Kontext | NAK-223, NAK-235, NAK-236, NAK-255 | Skill-Riss ohne Zeile (12.09.2026) | ✔ | Wache W-Riss (Schritt 2) |
| K8 | Bedienehrlichkeit | User-Gesetze 24./25.08.2026 | keine | ○ | Bedienehrlichkeits-Audit (Schritt 9, wartet auf Karte A.5) |

---

## K1 Rückstau und Ordnung

**Population:** 30 Objekte (18 Rust, 12 C++), 28 mit Test.

| Objekt | Ort | Politik bei voll | Test |
|---|---|---|---|
| P0-, P1-, P2-Warteschlangen, Ingress (Slots, Bytebudget) | `broker/src/transport/warteschlange.rs` | abweisen · koaleszieren · ersetzen · ersetzen/abweisen | Inline-Tests derselben Datei; `broker/tests/transport_fuzz.rs` |
| Eingang, Ausgang (Writerqueue, Messframe-Ersetzung, Hochwassermarken, Antwortkanal) | `broker/src/transport/server_v3/queues.rs` | ersetzen/abweisen · ersetzen · abweisen · abweisen | Inline-Tests; `broker/src/transport/server_v3/tests_rueckstau.rs` |
| Store-Kanal (`sync_channel`), Antwortkanäle, `writer_lauf vorgemerkt` | `broker/src/store/writer.rs`, `broker/src/store/handle.rs` | abweisen (`KanalVoll`); Antwortkanäle gedeckelt über die Annahmekante | `broker/tests/store_crash_matrix.rs` |
| Ratengrenze, Bootstrap-Fristen, Evidenz-Historie | `broker/src/transport/v3.rs`, `broker/src/transport/server_v3/listener.rs`, `broker/src/coordinator/evidenz.rs` | abweisen · Fristen · ersetzen | `transport_fuzz.rs`, `tests_fristen.rs`, `broker/tests/sonde013_verdrahtung.rs` |
| `session_command_reihenfolge` | `broker/src/coordinator/befehl.rs` | ersetzen | **keiner** (A-1) |
| P0/P1/P2 im Plugin, Reservierung, Voranstellen, `berichteAelterAls` | `eq-copilot/plugin/core/ipc/IpcQueues.h` | wie Rust, beidseitig | `eq-copilot/plugin/tests/IpcTestMain.cpp` |
| Interventionsring, Sample- und Deskriptor-Ring, Blockquarantäne, inFlight-Register | `eq-copilot/plugin/core/ipc/InterventionsRing.h`, `eq-copilot/plugin/core/StampedAudioQueue.h`, `eq-copilot/plugin/core/ipc/controlclient/Nachrichten.cpp` | abweisen · ersetzen | `Sonde013InterventionRingTest.cpp`, `QueueStressTestMain.cpp`, `IpcTestMain.cpp` |

**Skeptiker:** „`writer_lauf vorgemerkt` ohne Test" widerlegt
(`store_crash_matrix.rs`, Test `checkpoint_oder_guard_loest_offenes_append_fenster_nicht_aus`;
Politik als Absicht in `writer.rs`). „Antwortkanäle unbegrenzt" teilweise:
gedeckelt über `STORE_KANAL_CAP`, aber keine Zeile formuliert die Politik
der Antwortkanäle selbst. A-1 bleibt.

**Klassenlücke:** Eine neue Queue ohne deklarierte Politik fällt niemandem
auf; die Politiken stehen je Objekt, nicht als Inventar mit Test. Das ist
der Auftrag des Lebenslauf-Audits.

## K2 Lebenslauf und Besitz

**Population:** 43 Objekte (Threads, Joins, Stop-Pfade, Callbacks,
Close-Flags), 42 mit Test. Besitzer: `broker/src/lebenslauf.rs`,
`broker/src/server.rs`, `broker/src/transport/server_v3/` (`griff.rs`,
`listener.rs`, `verbindung.rs`, `queues.rs`, `win_handles.rs`, `senke.rs`,
`trennung.rs`), `broker/src/store/writer.rs`, `broker/src/coordinator/`
(`flush.rs`, `mod.rs`); Plugin: `eq-copilot/plugin/core/ipc/ControlClient.cpp`,
`TelemetryClient.cpp`, `BrokerLifecycle.cpp`, `IpcVerbindung.cpp`,
`eq-copilot/plugin/src/PipeClient.cpp`, `PluginProcessor.cpp`. Tests:
`broker/tests/broker_idle.rs`, `store_crash_matrix.rs`, `security_vectors.rs`,
`tests_lebenszyklus.rs`, `tests_fristen.rs`, `tests_rueckstau.rs`,
`tests_kopplung.rs`, `tests_abonnement.rs`; Plugin `IpcTestMain.cpp`,
`LebenslaufTestMain.cpp`, `PipeClientLifecycleTestMain.cpp`,
`QueueStressTestMain.cpp`.

**Skeptiker:** alle vier vorgelegten Lücken widerlegt: v2-Antwort-Flushthread
hat einen Fristtest (`server.rs`, `ungelesenes_reject_haelt_den_flush_nur_bis_zur_harten_frist`);
der StoreWriter hat absichtlich kein Close-Flag (Shutdown im selben FIFO,
Absicht in `writer.rs`, geordneter Abbau in `broker_idle.rs`);
`session_push_setzen` ist `Arc<dyn SessionPush>` mit Lock-Freigabe vor dem
Schreiben (`flush.rs`); `thread_abbrechen_und_join` hält den Handle über den
ganzen Vorgang (Kommentar in `server.rs`).

**Ungeprüft und deshalb kein Befund:** Die Erstzählung markiert bei rund
zehn Rust-Threads „Join mit Frist: nein" (Tickthread in `lebenslauf.rs`,
Acceptor und Wachhund in `listener.rs`, `broker_geordnet_stoppen`). Prüfliste
B verlangt eine Frist je Join. Ob diese Joins absichtlich fristlos sind (etwa
weil das Stop-Flag vorher gesetzt wird und der Thread garantiert endet),
klärt das Lebenslauf-Audit an der Quelle; hier wird nichts behauptet.

## K3 Vertrag und Zahlenränder

**Population:** 26 Nachrichtentypen, 671 Felder (flach, `$ref` aufgelöst),
398 mit Grenze; Negativfixtures: 290 JSON (52 distinkte Schemapfade mit
Grenz-Negativfixture), 101 binär.

| Seite | Prüfung | Ort | Stärke |
|---|---|---|---|
| Broker (Rust) | generische Schema-Engine: min/max, min/maxLength, pattern, maxProperties, min/maxItems für alle Typen; Discriminator vor Inhalt; Envelope-Familie vor Payload | `broker/src/vertrag.rs`, `broker/src/coordinator/schema.rs`, `broker/src/transport/v3.rs` | ✔ |
| Plugin (C++), Produktionsleser | geschlossene Feldmengen und Enums (`exakteFelder`, `feldmengeGenau`), `type` zuerst; Zahlengrenzen handgeschrieben je Leser (`session_snapshot`: 14 Prüfstellen für 52 Grenzfelder); mehrere Typen ohne Plugin-Leser (Sender-Seite) | `eq-copilot/plugin/src/SourcesModel.cpp`, `eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp`, `Intern.h`, `eq-copilot/plugin/core/ipc/WireEnvelope.cpp` | ◐ |
| Plugin (C++), Generalprüfung | alle Grenzen mechanisch, aber nur im Test instanziiert (A-2) | `eq-copilot/plugin/vertrag/NakamaVertrag.cpp` | Test-only |
| beidseitig | Fixture-Korpus gegen dasselbe Manifest; `--abdeckung` verlangt je Schlüsselwort Negativfixtures (Exit 2 bei Lücke); Mengengleichheit Manifest↔Leserregeln | `tools/eq-copilot/pruefe_v3_vertrag.py`, `broker/tests/contract_cross_language.rs`, `eq-copilot/plugin/tests/SchemaTestMain.cpp` | ◐ |
| NaN/Inf | 16 `is_finite`-Stellen Rust (`broker/src/telemetrie.rs`, `vertrag.rs`), 15 `isfinite`-Stellen C++ (`eq-copilot/plugin/core/ipc/WireZahl.h`, `eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp`, `NakamaEvidenz.cpp`) | beide Seiten | ◐ |

**Skeptiker:** Ein Vertrauenssatz „das Plugin vertraut dem Broker" existiert
nicht; `eq-copilot/schemas/v3/README.md` weist Querfeldregeln namentlich dem
Empfänger zu. Kein Test vergleicht die zwei C++-Produktionsleser Feld für
Feld; das ist der Auftrag des Vertragstreue-Audits. A-2 bleibt.

## K4 Zustand und Paarung

**Population:** State v2 mit 136 persistenten Feldern (Common 7, MainProject
8, Parameters 114, Dsp 6, Wurzel 1), 133 mit eingefrorenem Fixture, 19 mit
Migrationszweig; Preset 4 Top-Level plus 119 Kennungen, alle mit Fixture;
79 Fixture-Dateien unter `eq-copilot/fixtures/state/`.

| Zusage | Besitzer | Stärke |
|---|---|---|
| Save↔Load bytegleich | `eq-copilot/plugin/state/NakamaState.cpp`; `eq-copilot/plugin/tests/StateMigrationTestMain.cpp` (Common gleich, 120 Parameterwerte bit-exakt, Preset `zurueck == vorher`) | ✔ für die Fixtures |
| unbekannte Major read-only mit Originalbytes | `NakamaState.cpp` (`nurLesen`); Test G9 in `StateMigrationTestMain.cpp`, zusätzlich `Sonde013PassageStateTest.cpp`; Broker-Store analog in `broker/src/store/migration.rs`, Test `store_crash_matrix.rs` | ✔ |
| Dirty-State an Host | `eq-copilot/plugin/src/prozessor/Hostbruecke.cpp`; alle Änderungspfade in `prozessor/State.cpp`, `Analyse.cpp`, `Ipc.cpp` rufen; Laden meldet absichtlich nicht; Reihenfolge Serialisieren-vor-Dirty ist begründet und gemessen (`Sonde012ProjectReloadTest.cpp`) | ✔ |
| `project_binding_id` nicht migriert | Absicht: Schema-Doku `eq-copilot/schemas/state/nakama-state-v2.md` §32.2 „wird nicht erfunden", Test je Rolle, vier Goldens bytegleich | widerlegt |
| Beziehungspaare (speichern↔laden, öffnen↔schließen, verbinden↔trennen, aktivieren↔abklingen, installieren↔Rückweg) | beide Seiten mit Test: `NakamaState.cpp`, `PluginEditor.cpp`, `PipeClient.cpp`, `NakamaTransaktion.cpp`, `eq-copilot/install/Install-Nakama.ps1` mit `tools/eq-copilot/pruefe_installer_gegenpfad.py` | ✔ |
| MainProject | Roundtrips im Kanon (B14, B23, B29), aber drei Felder ohne Fixture, kein Feldmengen-Test (A-3) | ◐ |

**Klassenlücke:** Es gibt keinen Test, der die Feldmenge eines
Zustandsobjekts abgleicht (kein `getNumProperties`-Vergleich in den Tests).
Ein neues MainProject-Feld ohne Fixture liefe durch. Das Zustandstreue-Audit
bleibt sinnvoll, ist aber enger als im Konzept angenommen: Paritäts- und
Migrationsgang sind für Common, Parameters, Dsp und Preset schon Kanon.

## K5 Behauptung ≤ Messung, Werkzeug-Ehrlichkeit

**Population:** 65 Beineinträge in `tools/beweise.ps1` (A1–A32, B1–B29,
A4-SI, A4b, B3b, B3c), einer stillgelegt (A15 seit 28.08.2026), einer nicht
blockierend (A32). Alle 22 `broker/tests/*.rs` laufen über A4 (`cargo test`
ohne Filter), `#[ignore]` nur in `store_crash_matrix.rs`, gedeckt durch
A4-SI.

| Wache | Ort | Stärke |
|---|---|---|
| Frische: Binary älter als Quelle ⇒ keine Beglaubigung (Exit 4) | `tools/beweise.ps1` (Quellorte, Vergleich, Urteil) | ✔ |
| Stillgelegt-Marke, Geplant-Marke | `tools/beweise.ps1` | ✔ |
| Golden-Ordner leer ⇒ rot (WAV-Zahl 0 im Runner; Golden-Tests selbst geben 2 bei fehlender Referenz) | `tools/beweise.ps1`, `eq-copilot/plugin/tests/GoldenTestMain.cpp`, `AnalysisGoldenTestMain.cpp` | ✔ |
| Korpus-Mindestzahl (100), Bytegleichheit und Verwaiste je Korpus, Schema-Anzahl, State-Fixtures `FEHLT` ⇒ Exit 2 | `tools/eq-copilot/pruefe_v3_vertrag.py`, `erzeuge_v3_fixtures.py`, `erzeuge_p4_korpus.py`, `erzeuge_p5_korpus.py`, `erzeuge_envelope_fixtures.py`, `erzeuge_fb_fixtures.py`, `pruefe_v2_schemas.py`, `erzeuge_state_fixtures.py` | ✔ |
| jedes CMake-Ziel ist Bein oder stillgelegt | nur Prosa in `docs/plugin-wissen.md` „Nicht im Kanon" (A-7) | ○ |
| Clippy, Supply-Chain, Geheimnisse, Tippfehler, Duplikate | nirgends (A-6) | — |
| jeder Riss hat eine Registerzeile | niemand (Skill-Riss 12.09.2026 ohne Zeile) | — |

**Skeptiker:** alle fünf Ziele außerhalb des Kanons sind als Werkzeug oder
Wegwerfware dokumentiert (`EqCopPipeProbe`, `EqCopPaintBench`, `EqCopAuxSpike`,
`EqCopAuxSpikeTest` mit NAK-37, `EqCopHostProbe` mit Selbsttest im Kanon).
Die Golden-Tests laufen bei leerem Ordner nicht leer grün. Die Prosa ist
wahr; die Wache dafür fehlt.

## K6 Echtzeit

**Population:** 6 Dateien im Aufrufgraph von `processBlock`
(`eq-copilot/plugin/src/PluginProcessor.cpp`, `HoerMarkierung.h`,
`eq-copilot/plugin/core/StampedAudioQueue.h`,
`eq-copilot/plugin/core/analysis/Vergleichspegel.h`,
`eq-copilot/plugin/core/ipc/InterventionsRing.h`, `PluginProcessor.h`);
157 Treffer verbotener Muster in diesen Dateien, **0 in Reichweite** von
`processBlock`, `lebenszeichen`, `nakamaBlockEmpfangen` und den erreichten
Funktionen (Sperren, `new`/`malloc`, Datei/Pipe/Netz, Logging).

| Zusage | Besitzer | Stärke |
|---|---|---|
| keine Allokation im `processBlock` | Thread-lokaler `operator new`-Zähler in `QueueStressTestMain.cpp` an vier Stellen (0 erwartet), Zwilling in `DspGoldenTestMain.cpp` | ◐ (A-5: `malloc`/`HeapBlock` unsichtbar) |
| keine Sperre, Wanduhr im Budget je Callback | `RtWache` als Produktionscode am EQ-Kern (`eq-copilot/plugin/dsp/DspRtWache.h`, Eintritt `DspKern.cpp`, Audiopfad über `eq-copilot/plugin/sonde/SondeProcessor.cpp`); gemessen in `DspGoldenTestMain.cpp` (`null_sperren_im_callback (M-47)`, Wanduhr p99/max in Prozent des Budgets) | ✔ für den EQ-Kern; Messpfad `processBlock` ohne Wache (A-4) |
| Nulltest bitidentisch, Latenz 0, Tail 0 | `NullTestMain.cpp`, `SondeNullTestMain.cpp`, `EqCopProbeeqNullTest` | ✔ |
| statischer Blick (verbotene Aufrufe in Reichweite) | niemand; Kalibrierung: 0 Treffer heute, das künftige Tor startet grün | — |

## K7 Größe und Kontext

Besitzer `tools/plan/gesundheit.py` (elf Maße, Klassen GRENZE/ZIEL,
Exit 4). Stand 12.09.2026: drei gerissene Grenzen (Funktionen über 200
Zeilen, Kommentar-Bezeichner 32 von 30, Dirigenten-Skill 38 448 von 36 864
Bytes); Registerzeilen NAK-235, NAK-236, NAK-255; für den Skill-Riss fand
`grep` keine Zeile. Der User bearbeitet die Risse am 12.09.2026 mit dem
Dirigenten. Stärke ✔; die Wache W-Riss (Riss ⇒ Zeile) fehlt.

## K8 Bedienehrlichkeit und Laufzeitbelege

Kein automatischer Test misst ein Layoutrechteck bei Zustandswechsel;
`design/werkzeug/pruefung/sondenprobe.mjs` prüft 16 Zustandskombinationen
über Zelltexte und die Umkehrung des Gesetzes („was nichts bewirkt, ist
abgeschaltet und zeigt —"), keine Rechtecke. Rechtecke stehen nur als
Sessionmessung in `design/abnahmen/2026-09-01-gen-fenstergroesse-und-resize.md`.
Kein Test „jedes sichtbare Element hat einen Handgriff". Laufzeitbelege
gegen FL Studio: Termin A und B vom 22.08.2026 (`docs/beweise/termin-a/`,
`docs/beweise/termin-b/`), Aufbau und Fahrt von Hand beziehungsweise als
protokollierte MCP-Einzelaufrufe; wiederholbar ist nur die Nachprüfung der
Rohdaten (`tools/eq-copilot/pruefe_host_capabilities.py`). Stärke ○.

---

## Scope-Beweis dieser Fassung

Fünf Leser: K1/K2 (31 Dateien gelesen, 18 gegrept, 0 fehlende Pfade); K3
(35 Dateien plus 402 Fixturenamen über das Manifest, 0 fehlend; Hinweis:
`broker/src/framing.rs`, `dto.rs`, `protokoll.rs` tragen v2, nicht v3); K4
(rund 32 Dateien; `NakamaKanon.*`, `NakamaKernRiegel.h`, `broker/src/store/pfad.rs`
nur gelistet); K5/K7 (15 Dateien vollständig oder kopfweise, 66 gegrept;
`.github/` existiert nicht); K6/K8 (28 Dateien, 6 Verzeichnislisten;
`AnalyseEngine.*` und `core/analysis/` außer `Vergleichspegel.h` nicht vom
Graph erreicht, nur gegrept). Drei Skeptiker: 25, 20 und 23 Dateien. Alle
Läufe auf `90b3bdec` in der Arbeitskopie, nie auf dem laufenden Worktree.

## Änderungsprotokoll

| Datum | Änderung | Anlass |
|---|---|---|
| 12.09.2026 | Erste Fassung, verifiziert (5 Leser, 3 Skeptiker); Befunde A-1 bis A-7 | `KONZEPT.md` §10 Schritt 1, Abnahme 12.09.2026 |
