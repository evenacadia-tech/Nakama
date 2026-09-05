URTEIL: NEEDS_WORK

# G4 / P4 — eigener Falsifikationslauf des Dirigenten, 2026-09-05, HEAD `9f30008`

Prüfstufe T3, strikt lesend. Quellenstand unter `broker/`, `eq-copilot/`, `tools/`
ist mit dem Kanonstand `043e98e` identisch; die gefahrenen Exes sind aus diesem
Stand gebaut. Gate-Text: `/c-review` (DSP) + Codex, Falsifikation Gate 5 und 6
(`docs/FL-Nakama-Sonden-Design-Entwurf.md`:3556-3557), P4-Exit-Gate
(:4287-4289). Referenz für jede Zusage: Verhaltensmatrix M-01 bis M-86
(`docs/beweise/SONDE-013.md` §3).

---

## B1 — Gate 5: „Telemetrie steuert samplegenauen Gain oder erzeugt hörbares PRE/POST-Delta"

**kein Pfad gefunden.**

**Durchgespielte Wege: 10.**

1. **Inbound P0/P1 in Gen** — `ControlClient` liefert nur über zwei Callbacks
   herein (`eq-copilot/plugin/src/PluginProcessor.cpp`:106-111). Beide enden in
   Anzeige- und Statusobjekten: `v3Antwort()` (:1233-1271) verzweigt in
   `command_ack` → `bestaetigteSourcesCommands`, `evidence_invalidate` →
   `sourcesModel.uebernehmeEvidenzruecknahme`, sonst `uebernehmeSessionSnapshot`.
   Kein Zweig schreibt in einen Audiopuffer, eine Verstärkung oder einen Filter.
2. **`wendeBestaetigteSourcesCommandsAn()`** (:2278-2295 ff.) mutiert
   ausschließlich `zustand.mainProjectMitglieder` unter `bindungMutex` —
   Mitgliedschaft, kein DSP.
3. **Inbound P2 (Telemetrie-Flatbuffer)** — `v3Frame()` (:1273-1279) ruft nur
   `sourcesModel.uebernehmeP2(...)`.
4. **Match-Gain M-43** — der einzige Gain, den SONDE-013 kennt, liegt in
   `core/analysis/Vergleichspegel.h`. Im Audiothread läuft ausschließlich
   `speise()` (:92-126, reine Summation über zwei `const float*`); der Wert
   verlässt den Typ über `gainDb()` (:222) und erreicht nur den Wiretext
   (`PluginProcessor.cpp`:1937 `"match_gain_db"`) sowie `Blindvergleich.h`:95.
   Nirgends multipliziert ihn jemand auf den Puffer.
5. **Hörbares PRE/POST-Delta** — `deltaZustand()` (`Vergleichspegel.h`:374-385)
   plus `kCompareroutingInDieserPhase = false` (:390): in P4 ist der Raum aus
   drei Nachweisen an genau der Stelle gesperrt, an der er entstünde.
6. **Der Markierungsauftrag** — der einzige Produktweg in den Audiothread ist
   `HoerMarkierungDsp::reicheEin()`. Produktive Aufrufer sind nur
   `PluginEditor.cpp`:800 (`schalteMarkierung`, :754) und :821/:237
   (`markierungAus`); `schalteMarkierung` wird ausschließlich aus dem
   Klick-Callback der `BefundListe` gerufen (:509-514). Kein Wire-Weg.
7. **Testnaht** — `testForciereEchtzeit()` (`PluginProcessor.h`:420) hat
   außerhalb von `tests/` keinen Aufrufer (`rg` über `src/`, `sonde/`, `core/`).
8. **Die vier fail-closed Terme (M-33)** stehen vollständig in
   `PluginProcessor.cpp`:470-479: `istMainKlassifiziert` ∧ (`echtzeitOk` ∨ Test)
   ∧ `spielt` (gültig) ∧ `aufnahmeAus` (= `aufnahmeGueltig` ∧ ¬`aufnahmeAktiv`,
   ein **unbekannter** Aufnahmezustand blockiert) ∧ ¬`isNonRealtime()` ∧
   (`editorOffen` ∨ Test). Antwort auf die Einzelfrage: **nein** — ohne Editor,
   ohne gültiges `playing=true`, bei `recording=true` und ohne Echtzeitbeweis
   klingt nichts.
9. **Oversize (M-36)** — `HoerMarkierung.h`:339-354, :449-451: erzwungener
   Ausfade innerhalb `wetKapazitaet` mit verkürzter Rampe, danach
   `oversizeRiegel` bis `prepareToPlay`. Der Rest des Blocks bleibt wörtlich
   der Eingang (:466-467).
10. **Probeeq** — `sonde/SondeProcessor.cpp`:78-81 konstruiert `controlV3` und
    `telemetryV3` **ohne** Inbound-Handler (`{}`); `processBlock` (:155-224)
    schreibt in keinen Eingangskanal, nur `puffer.clear(k,…)` für
    überzählige Ausgangskanäle. Es gibt in Probeeq keinen Wireweg zu Audio.

**Messende Tests:** `EqCopMarkierungTest` (T4 Abgriff vor Färbung, T11 ohne
Playhead/gestoppt), `EqCopNullTest`/`EqCopProbeeqNullTest` (M-76),
`EqCopSonde013PrePostGoldenTest` (`audible_delta_locked_without_compare_routing`,
`match_gain_is_frozen_before_ab`), `EqCopQueueStressTest` (M-74/M-78 mit aktivem
Interventionsring).

**Gefahrene Exes:**
- `EqCopNullTest.exe` → `NULLTEST OK` (Exit 0)
- `EqCopProbeeqNullTest.exe` → `SONDE-NULLTEST OK - 87 Pruefungen ok, 0 Fehler`
- `EqCopMarkierungTest.exe` → `MARKIERUNGSTEST OK` (u. a. „T11: OHNE Playhead
  faerbt kein einziges Sample", „mit gestopptem Transport ebenfalls kein Sample")
- `EqCopSonde013PrePostGoldenTest.exe` → `30 bestanden, 0 gescheitert`
- `EqCopQueueStressTest.exe` → `116 Pruefungen, 0 Fehler. QUEUE-STRESSTEST OK`

---

## B2 — Gate 6: „Ein nicht vergleichbares Experiment erhält ein starkes Siegerurteil"

**kein Pfad gefunden.**

**Durchgespielte Wege: 10.**

1. **Wire-Vertrag** — `experiment_manual_result` (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json`:1630-1660)
   ist `additionalProperties: false` und trägt genau `type`, `kopf`,
   `experiment_id`, `hoerurteil`, `blindreihenfolge`, `notiz`, `werkzeug`.
   **Kein Feld `urteil` und kein Feld `vergleichbarkeit`.** Ein Wire-Client kann
   ein starkes Urteil nicht senden.
2. **Der Leser** — `ergebnis_anwenden_locked` (`broker/src/coordinator/experiment_verdrahtung.rs`:534-575)
   liest aus `wert` nur `hoerurteil`, `notiz`, `werkzeug`; die `messung` ist die
   brokereigene `Resultatmessung`.
3. **Herkunft der Vergleichbarkeit** — `resultatmessung` ruft
   `vergleichbarkeit::beurteile` (:1134-1153) auf zwei aus dem Store gebauten
   `Passagenbeleg`, nicht auf Wireangaben.
4. **Das Urteil** — `experiment::urteile` (`broker/src/coordinator/experiment.rs`:1302-1333)
   gibt bei `!vergleichbar` **zuerst** `VergleichNichtGueltig` zurück; der
   einzige Aufrufer ist `sicht.rs`:413-417 mit
   `vergleichbar = achsen.vergleichbarkeit.as_deref() == Some("stark")`.
5. **Store-Wiederherstellung** — `achsen_aus_gespeichertem` (:1663-1712) liest
   `vergleichbarkeit` aus dem **eigenen append-only Store**, nicht vom Draht.
6. **Fünf Belege statt Score** — `beurteile` (`vergleichbarkeit.rs`:280-367)
   ist ein Minimum: ein widersprochener Beleg ⇒ `Unvergleichbar` mit `return`
   (:333-342), erst danach entscheiden die drei Startgates 0,95/0,9/0,95
   (:345-353). Kein Hash hebt einen Widerspruch auf (M-28).
7. **NaN-Kanten der Gates** — `abdeckung` NaN ist ausdrücklich fail-closed
   (:298-304); `material_cosine` kann nicht NaN werden
   (`telemetrie.rs`:98-103 klemmt und ersetzt Nichtendliches durch 0,0);
   `jaccard` verdichtet zur Menge (:196-212, Befund B31); `ueberdeckung` rechnet
   sättigend (:168-186, M-17).
8. **Blindmodus M-44** — `aufgedeckte_reihenfolge()` (`experiment.rs`:603-608)
   gibt nur bei `Terminal::Ergebnis` etwas heraus; `sicht.rs`:392 ist der
   einzige Leser. Die Vorprüfung lehnt `reihenfolge_nicht_gebunden` und
   `blindreihenfolge_widerspruch` fail-closed ab
   (`experiment_verdrahtung.rs`:144-156). Antwort auf die Einzelfrage: **nein**,
   die Identität wird vor dem Urteil nicht aufgedeckt.
9. **Match-Gain M-43** — `!e.baseline.match_gain_db.is_finite()` ⇒
   `ohne_lautheitsabgleich` (:157-158); zusätzlich verlangt der Abschluss eine
   Resultatmessung (:160-179) und einen Kandidaten (`Abschlussfehler::OhneKandidat`).
10. **Abdeckung M-30** — `experiment_begin` lehnt `abdeckung < 0,5` mit
    `abdeckung_zu_gering` ab (:107); dasselbe Gate steht im Urteil
    (`GATE_ABDECKUNG`, `vergleichbarkeit.rs`:160).

**Messende Tests:** `broker/tests/sonde013_experiment.rs::gate6_incomparable_never_gets_a_strong_winner`,
`broker/tests/sonde013_passage.rs::comparability_needs_all_five_and_hash_alone_never_wins`,
`EqCopSonde013ExperimentUiTest` (M-43/M-44), `EqCopSonde013ExperimentGoldenTest`
(M-45 Bootstrap/FDR).

**Gefahrene Exes:**
- `EqCopSonde013ExperimentUiTest.exe` → `52 bestanden, 0 gescheitert`
- `EqCopSonde013ExperimentGoldenTest.exe` → `27 bestanden, 0 gescheitert`
- `EqCopSonde013PassageStateTest.exe` → `127 bestanden, 0 gescheitert`
(Die Rust-Beine A4 wurden **nicht** gefahren — `cargo test` hätte in `broker/target`
geschrieben und war damit außerhalb des Leseauftrags.)

---

## B3 — Exit-Gate Satz 2: „Kein unbekannter Zeitpfad erzeugt eine starke Cross-Probe- oder PRE/POST-Aussage"

**kein Pfad gefunden.**

**Durchgespielte Wege: 10.**

1. **Der Riegel selbst** — `prepost_verdrahtung.rs`:289-294 setzt
   `presentation_validiert: false` **hart** für jede Paarhälfte des
   Produktpfades. Es gibt keine Zuweisung `true` außerhalb von Tests.
2. **`frameschluessel`** (`prepost.rs`:885-916) gibt `Ausgerichtet` nur bei
   `presentation_validiert && klasse >= AudioAligned` zurück, sonst `Roh` mit
   `instance_id`/`transport_epoch` — also ohne Inhaltsgleichheitsbehauptung (M-21).
3. **`beurteile_paar`** (:1047-1079): der Kandidat `FeatureAligned` wird über
   denselben `frameschluessel` geprüft; scheitert er, kommt
   `KeinPresentationNachweis` in die Gründe und die Klasse fällt auf `Probable`.
   Damit ist `FeatureAligned` im Produktpfad **unerreichbar**.
4. **`dreifachergebnis`** (:783-846): `sicher = klasse >= AudioAligned`. Ohne
   `sicher` gibt es kein ausgerichtetes Delta, keinen Breitbandgain und keine
   interpretierte Wirkung (`wirkung = None`). Die starke Aussage entsteht
   schlicht nicht.
5. **Harte Ausschlüsse** (:947-977): getrennt, stale, nicht messbereit,
   Samplerate verschieden, keine Projektzeit, Fenster ≤ 0, **Sprung im Fenster**
   (Epochenbruch invalidiert statt herabzustufen, M-23) und keine Überlappung.
6. **Woher die Sprünge kommen** — `haelfte_aus_historie` zählt sowohl
   `transport_epoch`- als auch `continuity_segment`-Wechsel als Sprung
   (`prepost_verdrahtung.rs`:240-251); Drop und Loop-Wrap erreichen den Join
   also als Ausschluss.
7. **Zurückgenommene Evidenz** wird vor der Kurvenbildung ausgefiltert
   (`stand.ausschlussgrund.is_some()` → `continue`, :209-211).
8. **Kein Fenster ohne Projektzeit** — `projekt_fenster: None` statt „Fenster ab
   null" (:280-285).
9. **Plugin-Seite (M-06)** — `core/analysis/Konfidenz.h`:153-192: ohne
   Abdeckungsbit `unbrauchbar`; ein nicht-endliches Eingangssample verriegelt
   **vor** der Basis (:158-159); was der Erzeuger nicht beurteilen kann, deckelt
   auf `mittel` — eine Sonde erreicht `stark` deshalb strukturell nie.
10. **M-77/M-53** — Oversize verwirft nur die Analyse und startet ein neues
    `continuity_segment`; die Folge in der Engine ist eine Stromlücke, die jedes
    offene Fenster schließt (gemessen in B4 und B18).

**Messende Tests:** `broker/tests/sonde013_prepost.rs::unknown_time_path_can_never_reach_strong`
und `::distributed_pair_never_produces_transfer_function`,
`EqCopSonde013QualityClassTest` (M-06 über den ganzen Raum),
`EqCopSonde013DynamicsTest` (M-53/M-75/M-77 in der Engine),
`EqCopAnalysisGoldenTest` (B5, Grenztrennung).

**Gefahrene Exes:**
- `EqCopSonde013QualityClassTest.exe` → `24 bestanden, 0 gescheitert`
- `EqCopSonde013DynamicsTest.exe` → `46 bestanden, 0 gescheitert`
- `EqCopSonde013StereoGoldenTest.exe` → `57 bestanden, 0 gescheitert`
- `EqCopSonde013PrePostGoldenTest.exe` → `30 bestanden, 0 gescheitert`

---

## B4 — Exit-Gate Satz 3: „Hörmarker- und Experimentende schließen alle Taintintervalle; der reguläre Pfad nullt danach wieder"

**Pfad gefunden.** (Befund D-01, unten als DEFEKT geführt.)

**Durchgespielte Wege: 12.**

Sauber sind diese neun:

1. **Normales Markerende** — `HoerMarkierung.h`:492-516 meldet `endete` erst
   bei `fade == 0` exakt (nicht „unter 0,001"), mit `endeOffsetSamples`;
   `PluginProcessor.cpp`:594-612 stempelt `project_sample_end` sättigend und
   setzt `tail = 2·Dauer + fs/10`. Der Broker entfernt die ID, zieht
   `tail_samples_offen` hoch (`intervention.rs`:221-227) und **invalidiert den
   Bereich rückwirkend** (`befehl.rs`:641-663).
2. **Ende ohne Projektzeit** → ganze Sitzung invalidiert (`befehl.rs`:669-676).
3. **Nachlauf** — `tail_fortschritt_zeit` rechnet ab dem Tail-Start mit der
   Rate der meldenden Instanz und zieht bei nicht endlicher Rechnung **nichts**
   ab (`intervention.rs`:338-363).
4. **M-60** — `interventionssicht()`:536-552, `starke_evidenz_erlaubt =
   !unknown && aktive == 0 && tail == 0`; `evidence_dispatch_locked` (:577-592)
   liegt vor dem Commit und zählt unter demselben Lock.
5. **Ringüberlauf** — `InterventionsRing::schreibe` kennt kein `drop-oldest`
   (`core/ipc/InterventionsRing.h`:123-137), setzt sticky und zählt; der
   Rückgabewert wird ausgewertet (`PluginProcessor.cpp`:613-614).
6. **P0-Sendefehler** spiegelt dasselbe Sticky-Bit (:1083-1087).
7. **Experimentterminal** — `taint_intervalle_des_experiments_schliessen`
   (`intervention.rs`:437-449) räumt in **jeder** Sitzung, nicht nur in der des
   Aufrufers (Befund B4); `experiment_verdrahtung.rs`:337-342 ruft es für das
   Terminal und für jeden verdrängten Versuch.
8. **Abbruch ohne Terminalereignis (M-47/M-59)** — der Bestandsdeckel schreibt
   das terminale `aborted` und schließt darüber die Intervalle.
9. **Control-Disconnect** — `link.rs`:435-460 entfernt die Intervalle des
   sterbenden Links und setzt sticky `unknown`; eine Sitzung ohne Clients gibt
   ihren Taint ganz frei.

**Der brechende Weg (D-01):** der Rückweg aus dem Sticky-Zustand existiert im
Produkt nicht.

- `broker/src/coordinator/intervention.rs`:370-399 — `neutral_resync` ist der
  einzige Löscher von `taint.unknown`.
- `broker/src/coordinator/link.rs`:340-354 — `resync_bestaetigen` ist sein
  einziger Aufrufer.
- `broker/src/coordinator/befehl.rs`:550-575 — dessen einziger Produktaufrufer
  verlangt im Heartbeat **`Some(false)`**, also ein ausdrücklich vorhandenes
  Feld `"intervention_state_unknown": false`.
- `eq-copilot/plugin/core/ipc/ControlClient.cpp`:405-411 — der einzige
  Heartbeat-Erzeuger des Produkts **lässt das Feld weg, wenn es falsch ist**.
  `serde_json` liefert dann `None`, der Zweig ist `_ => {}` — es wird **nie**
  resynchronisiert.
- `eq-copilot/plugin/tests/MarkierungTestMain.cpp`:747-751 prüft die Auslassung
  ausdrücklich als Zusage („ein sauberer Zustand schreibt das Feld GAR NICHT").
  Beide Seiten sind je für sich grün und zusammen unverträglich.

Konkrete Eingabe, die den Zustand herstellt (Gen, Main, Marker hörbar):
`prepareToPlay` (Blockgrößen- oder Sampleratenwechsel des Hosts) ruft
`controlV3.reconnect()` (`PluginProcessor.cpp`:219) und setzt zugleich die
Markierung zurück (`markierung.vorbereiten`, :207 → `hartAus()` und
`warHoerbar = false`, `HoerMarkierung.h`:252-264) — der `endete`-Übergang
entfällt damit ersatzlos. Auf der Brokerseite läuft `control_ende`, entfernt das
Intervall und setzt `taint.unknown = true`, weil die Sitzung ihren Client
behält (`link.rs`:444-453). Danach ist `starke_evidenz_erlaubt` für diese
Sitzung dauerhaft `false`: `aktive` ist 0, `tail_samples_offen` ist 0, aber
`unknown` bleibt stehen, und **jeder** weitere `evidence_dispatch` wird gezählt
als `evidence_gesperrt`. Derselbe Endzustand entsteht bei Broker-Neustart, bei
`setzeBindung` (:1576) und bei jedem unfreiwilligen Pipeabbruch.

Zwei weitere, kleinere Wege derselben Familie, beide fail-closed und deshalb nur
als Härtung geführt (H-01, H-02, unten).

**Messende Tests:** `EqCopSonde013InterventionRingTest` (M-37 bis M-39, kein
`drop-oldest`, Sticky heilt nicht), `broker/tests/sonde013_taint.rs::only_neutral_resync_clears_sticky_unknown`,
`::inverse_path_returns_to_zero_after_every_intervention_kind`,
`broker/tests/sonde013_verdrahtung.rs::bestaetigter_resync_loest_das_sticky_unknown`
und `::erste_intervention_nach_resync_wird_angenommen`,
`EqCopSonde013PassageStateTest::r01Resync`, `EqCopMarkierungTest` B04.
**Keiner dieser Tests fährt den Handschlag `heartbeatAlsJson` → `p0_json_mit_minor`.**
Die Rust-Beine rufen `neutral_resync`/`resync_bestaetigen` direkt, das
Plugin-Bein B23 ruft `v3LinkFuerTest` direkt (NAK-156 hält genau diese
Testnaht bereits fest), und B04 prüft nur den JSON-Text, nicht seine Wirkung.

**Gefahrene Exes:**
- `EqCopSonde013InterventionRingTest.exe` → `22 bestanden, 0 gescheitert`
- `EqCopSonde013PassageStateTest.exe` → `127 bestanden, 0 gescheitert`
- `EqCopSonde013EventWireTest.exe` → `92 bestanden, 0 gescheitert`
- `EqCopMarkierungTest.exe` → `MARKIERUNGSTEST OK`
- `EqCopQueueStressTest.exe` → `116 Pruefungen, 0 Fehler`

---

## B5 — Exit-Gate Satz 1: Korpus und Beine gegen M-79 bis M-86

A25 (`tools/eq-copilot/erzeuge_p4_korpus.py`) und A26
(`tools/eq-copilot/pruefe_p4_korpus.py`) wurden als Quelltext gelesen, nicht
gefahren (kein Python im Auftrag). Zwei Eigenschaften von A26 sind für die
Bewertung tragend und stehen im Skriptkopf selbst: es **misst nichts nach**
(:16-20), und `_bein_bekannt` (:60-73) prüft nur, dass das genannte **Bein** im
Kanon-Runner steht — **nicht**, dass der genannte `fall` als Testfall existiert
oder besteht. `_passt` (:126-139) wertet die im Korpus selbst eingetragene
Wahrheit aus. A26 ist damit ein Buchhaltungsriegel über 22 Zeilen, kein
Messbein; das ist so gewollt und so beschrieben.

Zusage für Zusage:

| Zusage | Messender Test | Urteil |
|---|---|---|
| **M-79** vier Klassen, alle bestehen, Kennzahlen samt „falsche starke Behauptungen = 0" | `pruefe_p4_korpus.py` (A26), Gates :43-49 und :180-196; Pflichtklassen :191-194 | gemessen (als Buchhaltung über die vier Klassendateien) |
| **M-80** Korpus bytegleich reproduzierbar, MANIFEST und Waisen | `erzeuge_p4_korpus.py --pruefen` (A25), Runnerzeile `tools/beweise.ps1`:576 | gemessen |
| **M-81** Loudness/True Peak ±0,1 LU / ±0,1 dB gegen validierte Referenz | `Sonde013TruePeakGoldenTest.cpp`:121-231 (EBU Tech 3341 Fälle 15–19, analytische Referenz, 8×-Fehlerschranke −0,042 dB gegen 4× −0,169 dB, Sweep 44,1/48/88,2/96/192 kHz); `Sonde013DynamicsTest.cpp`:264-320 (PSR gegen dasselbe 3-s-Fenster), :496-520 (LRA unter 60 s nicht vorgetäuscht) | gemessen |
| **M-82** fünf Stereoklassen **über alle unterstützten Blockgrößen** | `Sonde013StereoGoldenTest.cpp`:160/207/255/294/430/495/692 misst die Klassen; die Blockgröße ist im Speiser fest `frames { 512 }` (:78) und wird **nirgends** variiert (einzige Variation: `sr = 22050.0` bei :607 für die Nyquist-Kappe) | Klassen gemessen, **Blockgrößen ungemessen** (L-01) |
| **M-83** PRE/POST-Lag innerhalb **eines** Feature-Hops; verteilter Pfad nicht gegen die 0,05-dB-Grenze | `broker/tests/sonde013_prepost.rs` (`unknown_time_path_can_never_reach_strong`, `distributed_pair_never_produces_transfer_function`), `Sonde013PrePostGoldenTest.cpp::nonlinear_chain_never_yields_static_eq_claim` | gemessen (Rust-Bein nicht gefahren) |
| **M-84** bekannte Impulsereignisse **über Blockgrößen und Sampleraten** zeitlich stabil | Grenztrennung: `AnalysisGoldenTestMain.cpp` (B5, Drop/Seek/Loop/Straddle/Sampleratewechsel), `QueueStressTestMain.cpp` (B4, 18 Blockgrößen 1…16384 für Passthrough). Ein Sweep über Blockgrößen **und** Sampleraten für die Zeitlage eines bekannten Impulses existiert nicht; der kurze Impuls wird bei einer Blockgröße gefahren (`AnalysisGoldenTestMain.cpp`:2123-2185) | Grenzhälfte gemessen, **Impuls-Stabilität über Blockgrößen/Sampleraten ungemessen** (L-02) |
| **M-85** adversarialer Distraktor ohne sicheren Auslöser; nicht vergleichbares Experiment ohne Siegerlabel; Insertprobe nie Mastersummenbeitrag | `broker/tests/sonde013_experiment.rs::gate6_incomparable_never_gets_a_strong_winner`, `broker/tests/sonde013_passage.rs::comparability_needs_all_five_and_hash_alone_never_wins`, `EqCopProbeeqNullTest` (Gate-7-Kette, gefahren: 87 Prüfungen ok) | gemessen |
| **M-86** zwei unabhängige Ereignisauslöser (Fluss und Peaksteigung) | `AnalysisGoldenTestMain.cpp`:2123-2185 (`short_impulse_triggers_peak_path_only`) | gemessen |

**Zusatzbefund zur Namensbindung:** von den 22 `fall`-Namen des Korpus
existieren **14 ausschließlich in `erzeuge_p4_korpus.py`** und in keinem Test:
alle fünf der Klasse `referenz`, drei von fünf der Klasse `zeitachse`, fünf von
sechs der Klasse `stereo` sowie `insert_probe_is_never_called_master_sum`. Die
zugrundeliegenden Tatsachen sind gemessen (siehe Tabelle), die Namen sind aber
Etiketten, keine geprüften Verweise — z. B. Korpus
`lra_below_sixty_seconds_is_unstable` gegen Test-Abschnitt
`M-04 lra_below_sixty_seconds_is_not_a_number`. Geführt als L-03.

**Gefahrene Exes für B5:**
- `EqCopSonde013TruePeakGoldenTest.exe` → `23 bestanden, 0 gescheitert`
- `EqCopSonde013DynamicsTest.exe` → `46 bestanden, 0 gescheitert`
- `EqCopSonde013StereoGoldenTest.exe` → `57 bestanden, 0 gescheitert`
- `EqCopSonde013FingerprintGoldenTest.exe` → `24 bestanden, 0 gescheitert`
- `EqCopProbeeqNullTest.exe` → `87 Pruefungen ok, 0 Fehler`

---

## Befunde

### D-01 · DEFEKT · Der bestätigte Resync hat im Produkt keinen Auslöser — nach dem ersten Control-Reconnect nullt der reguläre Pfad nie wieder

**Datei:Zeile (beide Seiten):**
- `eq-copilot/plugin/core/ipc/ControlClient.cpp`:405-411 — Feld wird bei
  `false` **weggelassen**.
- `broker/src/coordinator/befehl.rs`:550-575 — Resync verlangt **`Some(false)`**;
  ein fehlendes Feld fällt in `_ => {}`.
- `broker/src/coordinator/link.rs`:444-453 — `control_ende` setzt
  `taint.unknown = true`, solange die Sitzung noch einen Client führt.
- `broker/src/coordinator/link.rs`:340-354 und
  `broker/src/coordinator/intervention.rs`:370-399 — `resync_bestaetigen` →
  `neutral_resync` ist der einzige Löschweg.

**Eingabe / Reproduktion (aus der Quelle, ohne Prozessstart):** Gen ist Main und
verbunden; `prepareToPlay` (jeder Blockgrößen- oder Sampleratenwechsel des
Hosts) ruft `controlV3.reconnect()` (`PluginProcessor.cpp`:219). Der Broker
führt `control_ende` aus und setzt `taint[session].unknown = true`. Das Plugin
verbindet neu und sendet Heartbeats; da `status.interventionStateUnknown`
falsch ist, enthält der Heartbeat das Feld gar nicht, `resync_bestaetigen` läuft
nie. `interventionssicht_fuer_link(...)` liefert von da an dauerhaft
`starke_evidenz_erlaubt = false` bei `aktive = 0` und `tail_samples_offen = 0`;
`evidence_dispatch_locked` (`intervention.rs`:577-592) sperrt jede weitere
Evidenz dieser Sitzung. War beim `prepareToPlay` ein Marker hörbar, entfällt
zusätzlich sein `endete`-Übergang, weil `vorbereiten()`
(`HoerMarkierung.h`:252-264) `warHoerbar` und `fade` gemeinsam zurücksetzt —
das Markerende erreicht den Broker nie.

**Der Satz, der bricht:** Exit-Gate §58 (`docs/FL-Nakama-Sonden-Design-Entwurf.md`:4289)
— „Hörmarker- und Experimentende schließen alle Taintintervalle; **der reguläre
Pfad nullt danach wieder**." Und die Zeile, die „nullt" messbar macht, **M-60**:
„`interventionssicht().starke_evidenz_erlaubt` ist `true`, `aktive` ist 0,
`tail_samples_offen` ist 0 und `unknown` ist `false`; der nächste
`evidence_dispatch()` wird angenommen statt gesperrt." Genau dieser Zustand ist
nach dem ersten Reconnect unerreichbar. Zusätzlich betroffen ist **M-70** („Ein
Fixture ohne Verbraucher in beiden Sprachen ist ein Befund"): die
Heartbeat-Fixtures tragen `"intervention_state_unknown": false`
(`eq-copilot/fixtures/v3/gueltig/heartbeat.json`:29), der Produktsender erzeugt
diese Form nie, und der Rust-Verbraucher braucht genau sie.

**Richtung des Fehlers, damit sie nicht überbewertet wird:** fail-closed. Es
entsteht **keine** falsche starke Aussage; Gate 5 und Gate 6 bleiben unberührt.
Der Preis ist, dass der Evidenzpfad einer Sitzung nach dem ersten Reconnect
stumm bleibt — also das Gegenteil dessen, was das Ticket mit R01/B16
ausdrücklich reparieren wollte („ein sticky Bit ohne Gegenpfad ist dieselbe
Sorte Fehler wie ein fehlendes", `PluginProcessor.cpp`:1188-1189).

**Warum kein Bein fällt:** die Rust-Beine rufen `neutral_resync` und
`resync_bestaetigen` direkt (`broker/tests/sonde013_taint.rs`:560,
`broker/tests/sonde013_verdrahtung.rs`:637/684), B23 ruft `v3LinkFuerTest`
direkt, und `EqCopMarkierungTest` B04 (`MarkierungTestMain.cpp`:744-751) prüft
den JSON-Text beider Fälle, ohne ihn je durch `p0_json_mit_minor` zu schicken.

### H-01 · HÄRTUNG · Der Interventionsring wird bei nicht-Main-Klasse ohne Meldung geleert

`eq-copilot/plugin/src/PluginProcessor.cpp`:1057-1065: ist `pluginKind != "main"`
oder die Adresse ungültig, werden alle wartenden Ereignisse entnommen und
verworfen, **ohne** `interventionsRingUeberlauf` zu setzen. Erreichbar, wenn der
User die Rolle im Editor von `hub` auf `sensor` zurückstellt
(`setzeBindung` → `lebenslauf.expliziteInitialisierung`, :1569-1570,
`state/NakamaLebenslauf.h`:104-113), während ein Marker noch ausfadet: das
`end`-Ereignis entsteht danach und wird still verworfen. Kein Gatesatz verlangt
hier eine Meldung, und derselbe Aufruf löst über `controlV3.reconnect()` (:1576)
den Disconnectpfad aus, der das Intervall brokerseitig ohnehin entfernt.

### H-02 · HÄRTUNG · Ein Link ohne auffindbare Sitzung sperrt alle Sitzungen dauerhaft

`broker/src/coordinator/link.rs`:458-459 ruft bei unbekannter Sitzung
`alle_sitzungen_unbekannt`; `intervention.rs`:82-96 setzt zusätzlich den
Platzhalter. Das ist die ausdrücklich gewollte fail-closed-Antwort (§34.2), wird
durch D-01 aber ebenfalls unlösbar. Sobald D-01 behoben ist, löst
`neutral_resync` den Platzhalter mit (`intervention.rs`:398).

### L-01 · LÜCKE · M-82 „über alle unterstützten Blockgrößen" ist ungemessen

`eq-copilot/plugin/tests/Sonde013StereoGoldenTest.cpp`:78 — der Speiser läuft
fest auf 512 Frames; kein Fall variiert die Blockgröße. Die Runnerbehauptung von
B19 (`tools/beweise.ps1`:657) verspricht die Blockgrößen selbst nicht, die
Matrixzeile M-82 tut es. Verhalten ist nicht als falsch gezeigt — es ist an
dieser Achse nicht gemessen.

### L-02 · LÜCKE · M-84 „bekannte Impulsereignisse über Blockgrößen und Sampleraten zeitlich stabil" ist ungemessen

Gemessen ist die zweite Hälfte (jede Grenze trennt jedes offene Fenster, B5/B4).
Ein Sweep, der die **Zeitlage** eines bekannten Impulses über mehrere
Blockgrößen und Sampleraten vergleicht, existiert nicht;
`AnalysisGoldenTestMain.cpp`:2123-2185 fährt den kurzen Impuls bei einer
Blockgröße.

### L-03 · LÜCKE · Die 22 Fallnamen des P4-Korpus sind Etiketten, keine geprüften Verweise

`tools/eq-copilot/pruefe_p4_korpus.py`:60-73 prüft nur die Existenz des **Beins**
im Runner. 14 der 22 `fall`-Namen kommen ausschließlich in
`tools/eq-copilot/erzeuge_p4_korpus.py` vor. Eine Umbenennung oder Streichung
eines Testfalls würde A26 nicht rot machen. Weder Gate noch Matrix verlangen die
Namensbindung — deshalb Lücke, nicht Defekt.

---

## Geprüft

Vollständig gelesen und gegen die Matrix gehalten: `HoerMarkierung.h`,
`PluginProcessor.cpp` (Audiopfad, Erlaubnisterm, Interventionsring, Wire-JSON,
v3-Handshake, Klassifikation, Bindung), `PluginProcessor.h` (Testnähte, Felder),
`PluginEditor.cpp` (die einzigen Markierungsaufrufer), `SondeProcessor.cpp`
(Konstruktion, `prepareToPlay`, `processBlock`, Hostbrücke),
`core/analysis/Vergleichspegel.h`, `core/analysis/Konfidenz.h`,
`core/ipc/InterventionsRing.h`, `core/ipc/ControlClient.cpp` (Heartbeat-Erzeuger,
Laufzeitstruktur) und `.h`, `broker/src/coordinator/vergleichbarkeit.rs`,
`experiment.rs` (Urteil, Achsen, Terminal, Blindreihenfolge),
`experiment_verdrahtung.rs` (Vorprüfung, Append, Resultatmessung,
Store-Wiederherstellung), `prepost.rs` (Klassen, Ausschlüsse, Dreifachergebnis,
Frameschlüssel, `beurteile_paar`, `bilde_paare`), `prepost_verdrahtung.rs`,
`intervention.rs`, `link.rs` (Registrierung, `control_ende`,
`resync_bestaetigen`), `befehl.rs` (Heartbeat- und Interventionszweige),
`liveness.rs` (Eviction und Taint), `sicht.rs` (Urteilsserialisierung),
`telemetrie.rs`:89-158 (Fingerprintähnlichkeit, Grenzen),
`eq-ipc-v3.schema.json` (Experimentfamilien, Preview, `command_ack`),
`tools/eq-copilot/pruefe_p4_korpus.py`, die vier Korpusdateien und ihr MANIFEST,
`tools/beweise.ps1` (Beinbehauptungen A4, A16, A25, A26, B4, B5, B16 bis B26).

Gefahren (alle aus `eq-copilot/build/plugin/<Name>_artefacts/Release/`, alle
grün): `EqCopNullTest` (NULLTEST OK), `EqCopProbeeqNullTest` (87/0),
`EqCopMarkierungTest` (MARKIERUNGSTEST OK), `EqCopQueueStressTest` (116/0),
`EqCopSonde013InterventionRingTest` (22/0), `EqCopSonde013QualityClassTest`
(24/0), `EqCopSonde013PrePostGoldenTest` (30/0),
`EqCopSonde013ExperimentGoldenTest` (27/0), `EqCopSonde013PassageStateTest`
(127/0), `EqCopSonde013EventWireTest` (92/0), `EqCopSonde013StereoGoldenTest`
(57/0), `EqCopSonde013TruePeakGoldenTest` (23/0), `EqCopSonde013DynamicsTest`
(46/0), `EqCopSonde013FingerprintGoldenTest` (24/0),
`EqCopSonde013ExperimentUiTest` (52/0). Zusammen 15 Exes, 0 Fehler.

## Nicht geprüft

Die Rust-Beine (A4 `cargo test`) und die Python-Beine (A5, A8, A10, A25, A26)
wurden **nicht gefahren** — `cargo test` hätte in `broker/target/` geschrieben,
Python war im Auftrag ausgeschlossen; ihre Quellen sind gelesen. Nicht gefahren
wurden ferner alle Beine, die eine Named Pipe oder einen Broker brauchen (A22,
A24, `EqCopIpcTest`, `EqCopPipeClientTest`, `EqCopSessionSoak`) sowie
`EqCopShot` (Sichtprüfung). Nicht geprüft wurden `docs/**` als Befundfläche
(ausgeschlossen), die im Register datierten Punkte NAK-135 bis NAK-179
(ausgeschlossen; NAK-155/156 berühren die Umgebung von D-01, decken aber nur
Sequenz-Reset und Testnaht ab, nicht den fehlenden Heartbeat-Auslöser),
Same-user-Szenarien, FL-Termine und `tools/dirigent/pruefliste.md`. Nicht
geprüft wurde die Installer- und Identitätsfläche, die zu G4 nicht gehört.
Die Laufzeitmessung der Startbudgets aus §49.3 (p95-Zeiten, Soak) war ohne
Prozessstart nicht möglich und ist auch nicht Teil des P4-Exit-Gates.
