# SONDE-011 — Phase A: v3-Vertragsrunde vor R0

| Merkmal | Wert |
|---|---|
| Ticket | S16–17 `SONDE-011` |
| Lieferumfang | Phase A — v3-Vertragsrunde vor R0 |
| Datum | 2026-08-31 |
| Basis | `b75ea06` |
| Prüfstufe | T1+T2 |
| Stand | Nacharbeit der Spezifikations- und Verhaltensmatrix; Dirigentenentscheide eingetragen, Umsetzung offen |

Dieses Manifest ist noch kein Umsetzungs- oder Abnahmeurteil. Die mit
**ZUSAGE** bezeichneten Regeln und die in §9 zusammengefassten
**ENTSCHIEDEN**-Festlegungen sind die bindende Referenz für Bauer und Prüfer.
Auftrag und Eigentum stehen in `docs/plan/plan.json:192-197`, insbesondere im
vollständigen S16–17-Text in `:195`.

## 1. Maß für Vertragsevolution und Beweis

`CLAUDE.md:123-125` setzt das Maß: Altstände müssen laden, Save und Load werden
gemeinsam geprüft, und Discriminator, Zieladresse, Revision und Capability sind
in v3 **nicht additiv** erweiterbar. Der v3-Vertrag ist standardmäßig strikt;
nur `zaehler`, `konfidenz` und `verteilung` sind ausdrücklich additive Objekte
(`eq-copilot/schemas/v3/README.md:298-315`). Deshalb bedeuten die Kürzel in den
Matrizen:

- **additiv**: alte Leser dürfen den neuen Anteil ignorieren und neue Leser den
  alten Anteil weiterhin lesen; ein neues FlatBuffers-Feld braucht trotzdem
  eine neue, nie wiederverwendete Feld-ID und eine höhere `schema_minor`.
- **Versionierung**: kein Minor-Nachtrag. Vor R0 darf der Dirigent die Änderung
  als neu gefassten anfänglichen v3-Vertrag abnehmen; nach R0 ist eine neue
  Major-Version nötig. „Vor R0 ersetzt“ heißt ausdrücklich nicht „additiv“.
- **keine Wire-Änderung**: bestehender Vertrag wird nur durch eine
  Lebenszyklusregel oder einen bereits vorhandenen Pfad präzisiert.

Die Vor-R0-Ausnahme ist hier real: Der C++-Client wartet noch auf Coordinator und
Landkarte (`eq-copilot/plugin/core/ipc/ControlClient.h:16-18`), und der heutige
v3-Server nennt den Coordinator ausdrücklich noch nicht gebaut
(`broker/src/transport/server_v3.rs:3-6,42`). Sie darf nach dem R0-Freeze nicht
als Begründung wiederverwendet werden.

### Verbindliche Beweisbein-Namen

| Bein | Was es misst |
|---|---|
| **A5** | Python-Referenz gegen JSON Schema, Engine-Teilmenge, Abdeckung und je Definition mindestens ein Negativfixture (`tools/beweise.ps1:307`). |
| **A8** | Fixture-Korpus und `MANIFEST.json` sind bytegleich regenerierbar und ohne verwaiste Datei (`tools/beweise.ps1:310`). |
| **B3c** | Der handgeschriebene C++-Leser `EqCopSchemaTest` klassifiziert Urteil **und** Verletzungsmenge wie das Manifest (`tools/beweise.ps1:422-424`). |
| **A4-Rust** | `cargo test`, insbesondere `broker/tests/contract_cross_language.rs`, klassifiziert denselben Korpus im Rust-Leser (`tools/beweise.ps1:285-290`; `eq-copilot/schemas/v3/flatbuffers/README.md:21`). |
| **A4-SI** | Neuer Rust-Server-Integrationstest unter A4 auf einem Probe-Pipenamen; misst Linkbesitz, Fremdadressablehnung und atomaren Cleanup. Diese Laufzeitbindung können A5/A8/B3c/A4-Rust-Fixtures nicht beweisen. |
| **A9/A10** | `flatc`-Codegen-/Driftwache sowie regenerierbarer FlatBuffers-Korpus (`tools/beweise.ps1:312-318`). |
| **B5** | Der bestehende FeatureEngine-Golden misst die sechs NAK-29-Erzeugergründe getrennt (`eq-copilot/plugin/core/analysis/FeatureEngine.h:185-246`). |
| **Q1** | Quelleninventar dieser Spezifikationsrunde: `plan.json`, `PR1.md`, Entwurf, Register und aktuelle Implementierung werden auf der Basis im Kopf zeilengenau gegeneinander gelesen. |

Ein Empfänger-Nachtest allein erfüllt keine Schema-Zusage. Jede aktive
JSON-Vertragsänderung muss in A5, A8, B3c und A4-Rust dieselbe Klassifikation
erreichen.

## 2. NAK-28 — Gegenpfad zu `subscribe_session`

Ausgangslage: §33.3 zählt derzeit 25 Familien
(`docs/FL-Nakama-Sonden-Design-Entwurf.md:1842-1859`), das Schema kennt
`subscribe_session` (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:739-748`),
aber keinen Gegenpfad (`docs/offene-punkte.md:54`). Im aktuellen Server gibt es
noch keine Subscribe-Semantik; die Control-Verbindung besitzt jedoch bereits
die Kopplung. Ihr Ende entfernt den Registereintrag und bricht die gekoppelte
Telemetrie ab (`broker/src/transport/server_v3.rs:1579-1588,1770-1793`).

| Entscheid | ZUSAGE und technische Abwägung | Vertrags-/Entwurfsstellen | Evolution | Test/Beweisbein |
|---|---|---|---|---|
| **28-B — ENTSCHIEDEN: Verbindungsende** | Die Lebensdauer einer Subscription ist exakt die Lebensdauer ihres erfolgreichen `subscribe_session` auf **demselben** Control-Link und auf dessen gebundener Session/Adresse. Ein Control-Link darf weder für eine fremde Session noch für eine fremde effektive Adresse abonnieren. Jedes Control-Ende — EOF, Protokollfehler, Timeout, Writefehler oder Serverstopp — entfernt nur die diesem Link gehörenden Subscriptions atomar **vor** einem weiteren Push; Reconnect verlangt ein neues Subscribe. Das kostet keine Wire-Nachricht und wahrt verbinden↔trennen als einen Lebenszyklus. Selektives Abmelden verlangt dafür einen Link-Neuaufbau. | Bindender Satz für Entwurf §33.3 (`:1842-1859`); effektive Adresse §32.1 (`:1620-1630`) und `eq-ipc-v3.schema.json:62-77`; heutiger Besitz-/Cleanup-Pfad `server_v3.rs:1572-1588,1770-1808`; gekoppelte Lebenszyklen `CLAUDE.md:68`. | **Keine Wire-Änderung** und keine Familie `unsubscribe_session`. Mit `reference_match` endet §33.3 bei **26** Familien: 17 definiert + 9 reserviert. | A4-SI `subscription_ist_an_eigenen_control_link_gebunden`: Control-Link A sendet `subscribe_session` für Session/Adresse B und wird abgelehnt; gültige Subscription A bleibt beim Ende von B erhalten und wird ausschließlich beim Ende von A genau einmal und vor jedem späteren Push entfernt. A4-SI `subscription_cleanup_vor_weiterem_push` parametrisiert EOF, Protokollfehler, Timeout, Writefehler und Serverstopp. Q1 prüft, dass Schema und Reservierung kein `unsubscribe_session` enthalten. **A5/A8/B3c/A4-Rust-Fixtures messen diese Linkbindung ausdrücklich nicht.** |
| 28-A — VERWORFEN | `unsubscribe_session` hätte selektives Abmelden ermöglicht, aber einen nicht additiven Top-Level-Discriminator, eine 27. Familie und einen zweiten Cleanup-Auslöser geschaffen. Es ist keine Bauoption mehr. | Entwurf §33.3 (`:1842-1859`); Root-`oneOf` `eq-ipc-v3.schema.json:1-27`. | Nicht übernommen. | Q1 `kein_unsubscribe_session_name` schützt den Entscheid; es entstehen keine Positivfixtures dieser Familie. |

## 3. NAK-29 — bedingte Transportpflichten im Schema

### 3.1 Schemaform

**ZUSAGE:** `$defs.transportstempel` wird über `time_basis` und `$defs.cycle`
über `bounds_valid` als strikte diskriminierte Union formuliert. Die sechs
untenstehenden Relationen werden im Schema selbst abgelehnt, nicht erst in
einem Empfänger. Positive Gegenstücke bleiben gültig. Die bestehenden
Erzeugerriegel 1–6 bleiben bestehen (`FeatureEngine.h:209-232`). Fall 7 des
Erzeugers ist nicht Teil dieser Wire-Zusage; siehe §10.

Die gemeinsame Engine akzeptiert als Discriminatorwert der Instanz heute nur
einen String (`eq-copilot/schemas/v3/README.md:228-235`), während
`cycle.bounds_valid` ein Boolean ist (`eq-ipc-v3.schema.json:199-214`). Der
Bauer muss die Auswertung in Python, C++ und Rust deshalb auf String **und
Boolean** erweitern; fehlend oder ein anderer Typ ergibt weiterhin genau eine
`oneOf`-Verletzung. Für die beiden Beziehungen von `validity` zu Geschwistern
im Transportobjekt bezeichnet ein mit `/` beginnender
`x-nakama-discriminator` einen RFC-6901-Pfad, konkret
`/validity/cycle_bounds` und `/validity/continuous_time`. Bestehende einfache
Eigenschaftsnamen behalten ihre Bedeutung. Verschachtelte, nötigenfalls
duplizierte strikte Zweige drücken dann die Beziehungen aus; ein nur
handgeschriebener Nachtest außerhalb des Schemas erfüllt die Zusage nicht.

Die Änderung schränkt Dokumente ein, die der heutige Vertrag annimmt
(`docs/offene-punkte.md:55`), und braucht deshalb in jeder Zeile
**Versionierung** beziehungsweise die ausdrückliche Vor-R0-Neufassung.

### 3.2 Eine Aussage in JSON und FlatBuffers

**ZUSAGE:** Jede der sechs Relationen gilt nicht nur für das JSON-Schema,
sondern ebenso für `Transportstempel`/`Schleife` im Binärvertrag und wird in
**beiden** handgeschriebenen Binärlesern durchgesetzt. Das ist keine zweite
Semantik: `flatbuffers/README.md:174` verlangt ausdrücklich, dass die beiden
Darstellungen eine Aussage sind. Die heutigen Prüfstellen
`NakamaTelemetrie.cpp:181-250` und `broker/src/telemetrie.rs:355-420` prüfen die
sechs Kopplungen noch nicht; der Änderungssatz ergänzt beide und friert jede
Relation als regenerierbares A10-Binärfixture mit gleichem Urteil und
Verletzungspfad in B3c/A4-Rust ein. Die Binärfelder stehen in
`nakama_telemetry_v1.fbs:139-181`.

| Fall | ZUSAGE | Vertrags-/Entwurfsstellen | Additiv/Version | Test/Beweisbein |
|---|---|---|---|---|
| 29-1 Projektzeit | `time_basis=project_samples` verlangt `validity.project_time=true` **und** `project_sample_start`; binär gilt dasselbe für `zeitbasis`, das `project_time`-Bit in `gueltigkeit` und `project_sample_start`. | Entwurf §32.3 `:1677-1717`; JSON-Schema `:216-256`; FBS `:150-181`; beide Binärleser `NakamaTelemetrie.cpp:181-250`, `telemetrie.rs:355-420`; Erzeugerriegel 1 `FeatureEngine.h:211-215`. | **Versionierung** im JSON-Vertrag; im bestehenden Binärfeldsatz semantische Verschärfung und deshalb dieselbe Vor-R0-Neufassung. | A5/A8/B3c/A4-Rust: `ungueltig/transport-project-samples-ohne-project-sample-start.json`, `ungueltig/transport-project-samples-ohne-project-time.json`, positiv `gueltig/transport-project-samples.json`; A10/B3c/A4-Rust: `ungueltig/transport-project-samples-ohne-project-sample-start.bin` und `ungueltig/transport-project-samples-ohne-project-time-bit.bin`; B5 Grund 1. |
| 29-2 lokale Zeit | `time_basis=local_monotonic` verlangt `validity.project_time=false`; `project_sample_start` ist in JSON und Binärzweig verboten. | Entwurf §32.3 `:1712-1717`; FBS `nakama_telemetry_v1.fbs:150-181`; Binärleser `NakamaTelemetrie.cpp:181-250`, `broker/src/telemetrie.rs:355-420`; Erzeugerriegel 2 `FeatureEngine.h:216-217`; heutiger Erzeuger `:1819-1836`. | **Versionierung**: JSON-Einschränkung und binäre Semantikverschärfung brauchen dieselbe Vor-R0-Neufassung; nach R0 neue Major-Version. | A5/A8/B3c/A4-Rust: `ungueltig/transport-local-monotonic-mit-project-time.json`, `ungueltig/transport-local-monotonic-mit-project-sample-start.json`, positiv `gueltig/transport-local-monotonic.json`; A10/B3c/A4-Rust: `ungueltig/transport-local-monotonic-mit-project-time-bit.bin` und `ungueltig/transport-local-monotonic-mit-project-sample-start.bin`; B5 Grund 2. |
| 29-3 gültige Cycle-Grenzen | `cycle.bounds_valid=true` verlangt sowohl `start_ppq` als auch `end_ppq`; binär gilt das identisch für `schleife.bounds_valid`. Eine halbe Grenze ist nie gültig. | Entwurf §32.3 `:1692-1701,1741-1751`; JSON-Schema `eq-ipc-v3.schema.json:199-214`; FBS `nakama_telemetry_v1.fbs:139-148`; Binärleser `NakamaTelemetrie.cpp:181-250`, `broker/src/telemetrie.rs:355-420`; Erzeugerriegel 3 `FeatureEngine.h:218-220`. | **Versionierung**: JSON-Einschränkung und binäre Semantikverschärfung brauchen dieselbe Vor-R0-Neufassung; nach R0 neue Major-Version. | A5/A8/B3c/A4-Rust: `ungueltig/cycle-bounds-valid-ohne-start-ppq.json`, `ungueltig/cycle-bounds-valid-ohne-end-ppq.json`, positiv `gueltig/evidence-cycle-validiert.json`; A10/B3c/A4-Rust: `ungueltig/schleife-bounds-valid-ohne-start-ppq.bin` und `ungueltig/schleife-bounds-valid-ohne-end-ppq.bin`; B5 Grund 3. |
| 29-4 unbewiesene Ableitung | `cycle.bounds_valid=false` darf keine `derived_sample_bounds.derivation=validated_block_mapping` behaupten; binär darf `schleife.bounds_valid=false` keine `abgeleitete_grenzen.herleitung=validated_block_mapping` behaupten. Rohes PPQ darf getrennt davon vorliegen. | Entwurf §32.3 `:1728-1751`; JSON-Schema `eq-ipc-v3.schema.json:187-214`; FBS `nakama_telemetry_v1.fbs:79-83,129-148`; Binärleser `NakamaTelemetrie.cpp:181-250`, `broker/src/telemetrie.rs:355-420`; Erzeugerriegel 4 `FeatureEngine.h:221-222`. | **Versionierung**: JSON-Einschränkung und binäre Semantikverschärfung brauchen dieselbe Vor-R0-Neufassung; nach R0 neue Major-Version. | A5/A8/B3c/A4-Rust: `ungueltig/cycle-bounds-invalid-mit-validated-block-mapping.json`, positiv `gueltig/evidence-cycle-nur-ppq.json`; A10/B3c/A4-Rust: `ungueltig/schleife-bounds-invalid-mit-validated-block-mapping.bin`; B5 Grund 4. |
| 29-5 PPQ-Gültigkeitsbit | `validity.cycle_bounds=true` verlangt `cycle.start_ppq` und `cycle.end_ppq`; binär verlangt das `cycle_bounds`-Bit in `gueltigkeit` dieselben beiden `schleife`-Werte. Das Bit bescheinigt rohe PPQ-Werte, nicht die Sampleableitung. | JSON-Schema `eq-ipc-v3.schema.json:160-185`; Entwurf §32.3 `:1728-1746`; FBS `nakama_telemetry_v1.fbs:85-112,139-181`; Binärleser `NakamaTelemetrie.cpp:181-250`, `broker/src/telemetrie.rs:355-420`; Erzeugerriegel 5 `FeatureEngine.h:224-230`. | **Versionierung**: JSON-Einschränkung und binäre Semantikverschärfung brauchen dieselbe Vor-R0-Neufassung; nach R0 neue Major-Version. | A5/A8/B3c/A4-Rust: `ungueltig/validity-cycle-bounds-ohne-start-ppq.json`, `ungueltig/validity-cycle-bounds-ohne-end-ppq.json`, positiv `gueltig/validity-cycle-bounds-mit-ppq.json`; A10/B3c/A4-Rust: `ungueltig/validity-cycle-bounds-ohne-start-ppq.bin` und `ungueltig/validity-cycle-bounds-ohne-end-ppq.bin`; B5 Grund 5. |
| 29-6 kontinuierliche Zeit | `validity.continuous_time=true` verlangt `continuous_time_samples`; binär verlangt das `continuous_time`-Bit in `gueltigkeit` dasselbe optionale Feld. | Entwurf §32.3 `:1728-1746`; JSON-Schema `eq-ipc-v3.schema.json:160-185,216-256`; FBS `nakama_telemetry_v1.fbs:85-112,157-181`; Binärleser `NakamaTelemetrie.cpp:181-250`, `broker/src/telemetrie.rs:355-420`; Erzeugerriegel 6 `FeatureEngine.h:231-232`. | **Versionierung**: JSON-Einschränkung und binäre Semantikverschärfung brauchen dieselbe Vor-R0-Neufassung; nach R0 neue Major-Version. | A5/A8/B3c/A4-Rust: `ungueltig/validity-continuous-time-ohne-wert.json`, positiv `gueltig/validity-continuous-time-mit-wert.json`; A10/B3c/A4-Rust: `ungueltig/validity-continuous-time-ohne-wert.bin`; B5 Grund 6. |
| 29-E Enginekante | `x-nakama-discriminator` wertet `true` und `false` als zwei verschiedene Const-Zweige; Boolean wird nicht zu String oder Zahl konvertiert. Ein RFC-6901-Pfad wird segmentweise aufgelöst; fehlendes Segment ist dieselbe eine `oneOf`-Verletzung wie ein fehlender direkter Discriminator. | Enginevertrag `README.md:228-235`; Cycle-Feld `eq-ipc-v3.schema.json:202-208`; drei Verbraucher laut `README.md:15-26`. | Boolean-/Pfad-Unterstützung der Engine ist additiv; ihre Verwendung in den Zeilen 29-1 bis 29-6 bleibt versionierungspflichtig. | Neue direkte Tests `discriminator_boolean_true_false`, `discriminator_boolean_falscher_typ`, `discriminator_boolean_fehlt`, `discriminator_json_pointer_boolean` und `discriminator_json_pointer_segment_fehlt` in A5, B3c und A4-Rust; A8 friert die Fälle im gemeinsamen Manifest ein. |

## 3a. NAK-114 — `plugin_kind` × `measurement_position`

| ID | ZUSAGE | Vertrags-/Entwurfsstellen | Additiv/Version | Test/Beweisbein |
|---|---|---|---|---|
| 114-1 Klassenmatrix | Der v3-Vertrag übernimmt exakt die State-v2-Matrix: `main` darf nur `insert`; `legacy`, `passive_probe` und `active_probe` dürfen `insert`, `pre` und `post`; `post_fader_contribution` darf heute **keine** der vier Klassen führen. Die äußere, über `measurement_position` diskriminierte Union und ihre vier strikten Objektzweige bleiben erhalten. Innerhalb der Engine-Teilmenge wird `plugin_kind` je Zweig inline gekoppelt: `insert` erhält `{"type":"string","enum":["main","passive_probe","active_probe","legacy"]}`; `pre` und `post` erhalten `{"type":"string","enum":["passive_probe","active_probe","legacy"]}`; der Beitragszweig erhält `{"type":"string","enum":["main","passive_probe","active_probe","legacy"],"maxLength":0}`. Weil alle vier eingefrorenen Namen nichtleer sind, ist dieser Zweig für den heutigen Wortschatz bewusst unerfüllbar, ohne einen fünften Namen zu erfinden. `allOf`, `not`, eine leere `enum` oder ein unbekanntes Schlüsselwort werden nicht benutzt. | Eigentum/Befund `docs/offene-punkte.md:17`; State-Matrix `nakama-state-v2.md:45-54`; heutige freien `$ref` in `eq-ipc-v3.schema.json:476-574`; Engine-Schlüsselwörter und nichtleere `enum` `README.md:162-198`; Entwurf §32.2 `:1632-1667`. | **Versionierung**: die erlaubte Kombination zweier Vertragsachsen und der strikte, diskriminierte Descriptor sind nicht additiv. Vor R0 ersetzt dies die v3-Baseline; nach R0 braucht es eine neue Major-Version. | Der heutige Korpus enthält laut NAK-114 nur `passive_probe`. A5/A8/B3c/A4-Rust ergänzen `ungueltig/beitrag-plugin-kind-main.json`, `ungueltig/beitrag-plugin-kind-legacy.json`, `ungueltig/beitrag-plugin-kind-passive-probe.json`, `ungueltig/beitrag-plugin-kind-active-probe.json`, jeweils ansonsten vollständig und mit `contribution_aux=supported`, damit genau der Beitrags-Klassenriegel fällt; `ungueltig/probe-descriptor-main-pre.json` und `ungueltig/probe-descriptor-main-post.json` messen den übrigen `main`-Riegel. Positive Korpuslücke schließen mit `gueltig/session-main-insert.json`, `gueltig/session-active-probe-pre.json`, `gueltig/session-legacy-post.json`; `gueltig/session_snapshot.json` bleibt der positive `passive_probe`-Fall. Das heutige `gueltig/session-beitragsklasse.json` wird als `ungueltig/beitrag-plugin-kind-passive-probe.json` umklassifiziert, nicht als Positivbeleg stehen gelassen. |
| 114-2 Beitragszweig und Riegel | `probe_descriptor_beitrag` wird **nicht entfernt**. Der berechtigte Kern des Prüferbefunds wird präzise umgesetzt: Alle vier heutigen `plugin_kind` fallen in **diesem** Zweig an der State-v2-Matrix, der Zweig selbst bleibt aber Ausdrucksmittel des Vertrags für `post_fader_contribution`/`beitrag` nach §32.2. Zusätzlich bleibt D1 unabhängig scharf: der Zweig verweist auf `capabilities_beitrag` und verlangt `contribution_aux=supported`. Auch beide State-Riegel **innerhalb** `positionErlaubt` bleiben zu: Riegel 1 ist die `false`-Konstante `kContributionAuxVerfuegbar`, Riegel 2 der für alle vier Klassen geschlossene Beitragspfad im Klassenswitch. Im Strukturprüfer wird die handgeschriebene Klassenmatrix ergänzt: Er verlangt genau vier Root-Zweige, ihre `additionalProperties:false`, die exakten branchenspezifischen `plugin_kind`-Schemas und den unerfüllbaren Beitragsriegel. Beim Gleichheitsvergleich wird zusätzlich `plugin_kind` aus dem gemeinsamen Rumpf genommen; alle übrigen Pflichtfelder/Feldschemas bleiben identisch. Die Gegenprobe muss sowohl eine verdorbene Klassenmenge als auch einen gelockerten Beitragsriegel erkennen. | Bindender Ticketwortlaut `docs/plan/plan.json:195`; Wortschatz bleibt ausdrücklich `nakama-state-v2.md:76-90`; Beitragszweig/Capability `eq-ipc-v3.schema.json:119-145,465-574`; heutiger Zweigriegel `pruefe_v3_vertrag.py:506-598`; State-Riegel `NakamaState.cpp:342-358,387-415`; Entwurf §32.2 `:1662-1675`. | Derselbe **nicht additive** Vor-R0-Vertrag wie 114-1. Das Beibehalten eines heute unerreichbaren Zweigs ist keine neue Annahme und keine Capability-Erweiterung. Eine künftig erlaubte Trägerklasse braucht einen eigenen dokumentierten Vertragsentscheid und Versionierung. | A5-Strukturtests `probe_descriptor_plugin_kind_matrix_ist_exakt`, `probe_descriptor_beitragszweig_bleibt_vorhanden`, `beitragszweig_hat_keine_heutige_traegerklasse`, `beitragszweig_verlangt_contribution_aux_supported` sowie zwei Gegenproben `plugin_kind_matrix_drift_faellt` und `beitragsriegel_gelockert_faellt`. A5/A8/B3c/A4-Rust behalten `ungueltig/beitrag-ohne-contribution-aux.json`; nach 114-1 erwartet dessen Manifest **beide** unabhängigen Verletzungen (Klassenriegel und D1), damit D1 nicht von der Unerreichbarkeit verdeckt wird. C++-State-Test `post_fader_contribution_bleibt_fuer_alle_sondenklassen_gesperrt` misst beide State-Riegel. |

## 4. NAK-40 — nicht-hex32-State-ID auf die v3-Adresse abbilden

Der Host-State bewahrt eine Schema-1-`sensor_id` als `instance_id` bytegleich,
auch wenn sie kein hex32 ist (`eq-copilot/schemas/state/nakama-state-v2.md:96-114`);
der bestehende Golden enthält etwa
`11111111-2222-3333-4444-555555555555`
(`eq-copilot/plugin/tests/StateMigrationTestMain.cpp:139,630`). Die v3-Adresse
fordert dagegen hex32 (`eq-ipc-v3.schema.json:31-37,62-77`), ebenso der
Rust-Bootstrap (`broker/src/transport/bootstrap.rs:144-163`) und der C++-Client
(`eq-copilot/plugin/core/ipc/ControlClient.h:35-55`;
`ControlClient.cpp:121-135,471-476`). Der Telemetrieclient verwendet dieselbe
Adresse und denselben Riegel (`TelemetryClient.h:36-42`;
`TelemetryClient.cpp:348-369`).

| Richtung/Fall | ENTSCHIEDENE ZUSAGE | Vertrags-/Entwurfsstellen | Additiv/Version | Test/Beweisbein |
|---|---|---|---|---|
| State → Wire, bereits hex32 | Entspricht der gespeicherte Wert exakt `[0-9a-f]{32}`, ist die Wire-`instance_id` bytegleich derselbe Wert. Keine Kleinschreibung oder sonstige Normalisierung. | State-Vertrag `nakama-state-v2.md:17,109`; Adresse `eq-ipc-v3.schema.json:31-37,74`. | **Versionierung**: Bedeutung der Zieladresse ist nicht additiv; vor R0 als v3-Baseline, danach neue Major-Version. | Neuer gemeinsamer Vektorkorpus `eq-copilot/fixtures/v3/instance-address-alias-v1.json`; Vektor `0123456789abcdef0123456789abcdef → identisch`; B3c und A4-Rust. |
| State → Wire, sonst | `alias = lower_hex(first_16_bytes(SHA-256(concat(UTF8("evenacadia.nakama.v3.instance-address.v1"), 0x00, u64be(len(exact_utf8(instance_id))), exact_utf8(instance_id)))))`. Keine Unicode-Normalisierung, kein Case-Folding. Eingefrorener Vektor: `11111111-2222-3333-4444-555555555555 → 63de6caeedaa39f91a6e35a64de7fd7d`. Ein leerer Schema-1-Wert nimmt weiter den vorhandenen Neu-UUID-Pfad und wird nicht gehasht. | NAK-40 `docs/offene-punkte.md:66`; bytegleiche Migration und Leerwertregel `nakama-state-v2.md:109-112`; C++-/Rust-Adressprüfer oben. | Versionierung wie vorige Zeile. Die Abbildung ändert **nicht** das State-Schema. | B3c/A4-Rust `instance_address_alias_vectors`; Vektoren für Bindestrich-UUID, Groß-/Kleinschreibung und nicht-ASCII sowie `instance_address_empty_uses_fresh_uuid`. A5/A8 prüfen weiterhin, dass auf dem Wire nur hex32 akzeptiert wird. |
| Wire → State/Instanz | Der Alias ist nicht reversibel und wird außerhalb der besitzenden Plugininstanz nicht als State-ID ausgegeben. Broker und Gen führen die Wireadresse; der Zielclient berechnet sie aus seiner eigenen gespeicherten `instance_id` erneut und akzeptiert nur die exakte Übereinstimmung. Niemand dekodiert oder errät eine Original-ID, und der Alias wird nie in den Host-State zurückgeschrieben. Das ist der dokumentierte Rückweg: Vergleich mit der lokalen Originalwahrheit, keine Hash-Umkehr. | Effektive Adresse Entwurf §32.1 `:1606-1630`; State-Lade-/Roundtrip-Regel `nakama-state-v2.md:10,109-114`; C++-Adressgrenze `ControlClient.cpp:121-135,471-476`. | Versionierung der Adresssemantik; Save+Load gemeinsam. | Neue Tests `instance_alias_target_recomputed_and_matched`, `instance_alias_unknown_fail_closed` und `legacy_instance_id_save_load_bytegleich`. Der C++-State-Test lädt, adressiert, speichert, lädt erneut und vergleicht die Originalbytes; A4-Rust beweist, dass ein unbekannter Alias keinen Dispatch erhält. |
| Kollision | Treffen native hex32 und Alias oder zwei verschiedene Originalwerte im selben Adressraum auf denselben Wirewert, wird **für beide Seiten keine Zuordnung gewählt**. Die Kollisionsregistrierung quarantänisiert unter demselben Register-Lock atomar sowohl den neu eintreffenden als auch den zuerst registrierten Eintrag, entfernt eine schon veröffentlichte Aliasauflösung vor Rückkehr aus dem Registrierungsschritt und sperrt Dispatch, Telemetrie und Session-Push für beide. Der erste Eintrag darf also nicht weiterlaufen, nur weil er zeitlich zuerst kam. Der sichtbare Konflikt bleibt bis zur expliziten Neu-ID-Auflösung bestehen; Reihenfolge und Ursprungstyp ändern das Urteil nicht. | Dirigentenentscheid dieser Matrixnacharbeit; Duplikatregel `nakama-state-v2.md:154-156`; bestehender Duplicate-Pfad `StateMigrationTestMain.cpp:1157-1172`; Adressform Entwurf §32.1 `:1606-1630`. | Keine weitere Formatänderung; Teil derselben **versionierten, fail-closed Adresssemantik**. | Deterministisch injizierte A4-Rust-Registertests `instance_alias_collision_native_dann_abgeleitet_quarantaenisiert_beide` und `instance_alias_collision_abgeleitet_dann_native_quarantaenisiert_beide`; zusätzlich parametrisiert `instance_alias_collision_zwei_ableitungen_quarantaenisiert_beide` für A→B und B→A. Jeder Test prüft nach dem zweiten Registerschritt: kein Aliasindexeintrag, beide Identitäten sichtbar quarantänisiert und kein Dispatch/Push an die zuerst oder zuletzt registrierte Seite. |

Die Original-ID bleibt damit die Save-/Load-Wahrheit; der hex32-Alias ist nur
eine abgeleitete Wireadresse. Ein zusätzlich persistiertes Aliasfeld wird nicht
eingeführt und kann daher nicht gegen die Original-ID driften.

## 5. NAK-59 — Band-Stereo

Die FeatureEngine liefert bereits den Seitenanteil `S/(M+S)` je Live-Band samt
Bitmap (`eq-copilot/plugin/core/analysis/FeatureEngine.h:343-366`), während
`Frame` nur die Felder mit IDs 0–9 besitzt
(`eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs:216-226`;
`FELD-IDS.json:74-87`). Die beiden handgeschriebenen Leser sind
`eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp` und
`broker/src/telemetrie.rs` (`flatbuffers/README.md:15-23`), konkret die
Frameprüfungen in `NakamaTelemetrie.cpp:253-384` und
`broker/src/telemetrie.rs:316-353`; der offene Vertragspunkt steht in
`docs/offene-punkte.md:43`.

| Entscheid | ZUSAGE und Abwägung | Vertrags-/Entwurfsstellen | Evolution | Test/Beweisbein |
|---|---|---|---|---|
| **59-A — ENTSCHIEDEN: Wirefeld** | Neues optionales Frame-Feld `band_stereo:Bandwerte (id: 10)`. Wenn vorhanden: Gitter `nakama_log64_v1`, Encoding `float32`, genau 64 Werte, passendes 8-Byte-Bitmap, kein `werte_i16`, jeder gesetzte Wert endlich und in `[0,1]`, `saturated=false`. Neue Sender senden es mit der erhöhten Vertrags-Minor-Version; neue Leser akzeptieren alte Frames ohne Feld. | Entwurf §31.2 `:1546-1554`, §56 `:3879-3898`; FBS `:60-70,189-205,216-226`; ID-Disziplin `flatbuffers/README.md:31-49`; FeatureEngine `:350-366`; Envelope-Schreiber `broker/src/transport/v3.rs:1-12,324-350`. | **Additiv**, weil optional am Tabellenende; `schema_minor` steigt, ID 10 wird in `FELD-IDS.json` eingefroren und nie umgewidmet. FBS, ID-Register, `flatc`-Codegen, **beide** handgeschriebenen Leser und Sender-Minor gehören in denselben Änderungssatz. | A9 `flatc`- und ID-Drift. A10 positiv: `gueltig/band-stereo-fehlt-altframe.bin`, `gueltig/band-stereo-bitmap-alle-werte-ungueltig.bin`, `gueltig/band-stereo-wert-0.bin`, `gueltig/band-stereo-wert-1.bin`; A10 negativ: `ungueltig/band-stereo-nan.bin`, `ungueltig/band-stereo-unter-0.bin`, `ungueltig/band-stereo-ueber-1.bin`, **`ungueltig/band-stereo-saturated-true.bin`**, `ungueltig/band-stereo-falsches-gitter.bin`, `ungueltig/band-stereo-falsches-encoding.bin`, `ungueltig/band-stereo-werte-i16-gesetzt.bin`, `ungueltig/band-stereo-laenge-falsch.bin`, `ungueltig/band-stereo-bitmap-laenge-falsch.bin`. B3c/A4-Rust liefern gleiches Urteil und Verletzungspfad. C++-Roundtrip `band_stereo_featureframe_to_flatbuffer`; Sender-Integrationstest `band_stereo_sender_emittiert_erhoehten_schema_minor` erzeugt über den echten Sendepfad einen Frame, liest im vollständigen Wireframe Offset 7 (Offset 3 des 16-Byte-Kopfs nach `frame_len`) zurück und vergleicht ihn mit der für ID 10 festgelegten Minor-Version — ein bloßer Direktaufruf von `envelope_schreiben` mit einem Testwert genügt nicht. |
| 59-B — VERWORFEN | Die Festhaltung „bleibt rein lokal“ hätte null Wirekosten, aber die bereits berechnete Größe Broker und Gen vorenthalten. Sie ist keine Bauoption mehr. | Entwurf §56 `:3889`; FeatureEngine `:350-364`; heutiger Frame `fbs:216-226`. | Nicht übernommen. | Q1 stellt sicher, dass §56 nach Umsetzung nicht „rein lokal“ behauptet; A9 muss ID 10 sehen. |

## 6. NAK-10 — `hoermarkierung` wird gelesen

Die ältere Entwurfsnotiz behauptet noch, Rust übernehme das Flag nicht
(`docs/FL-Nakama-Sonden-Design-Entwurf.md:1570-1572`). Der aktuelle Code hat
diesen Teil bereits überholt: `MessStand` des v2-Eingangs trägt
`hoermarkierung` (`broker/src/protokoll.rs:128-135`), und das Register wertet es
mit einer bis zum beobachteten Reset fail-closed bleibenden Sperre aus
(`broker/src/lib.rs:306-380`; bestehende Tests `:1150-1220,1247-1293`). Der
aktuelle Codebeleg geht der älteren beschreibenden Notiz und dem entsprechend
veralteten Registertext (`docs/offene-punkte.md:48`) vor.

| ID | ZUSAGE | Vertrags-/Entwurfsstellen | Additiv/Version | Test/Beweisbein |
|---|---|---|---|---|
| 10-1 | Der SONDE-011-Coordinator führt genau **einen** Interventionszustand: eine Menge sauber verfolgter aktiver `intervention_id` plus das sticky Bit `intervention_state_unknown`. Legacy-v2 `measurement.hoermarkierung=true/false` und v3 `audible_intervention_begin/end` mit `art=hoermarkierung` speisen ihn vor jedem Evidenzdispatch. Ein passendes End beendet nur seine eigene sauber verfolgte ID; bei Überlappung bleibt gesperrt, bis alle IDs beendet und ihre `tail_samples` abgelaufen sind. Ein End mit fremder ID ändert nichts. **Ring-Overflow, Control-Disconnect oder eine Eventsequenzlücke setzen sticky `intervention_state_unknown`**, quarantänisieren/invalidieren den betroffenen Timelinebereich und sperren starke Evidenz. Danach entsperren weder ein passend aussehendes End noch v2-`false` noch ein Session-Ende: ausschließlich ein **expliziter Neutral-/Sequenz-Resync** mit bestätigter neuer Sequenzbasis darf das Sticky-Bit löschen. Ein verlorenes Begin oder End kann so nie eine saubere Baseline vortäuschen. | Entwurf §31.2 `:1546-1572`, **§34.2 `:1968-1981`**, §56 `:3879-3898`; Heartbeat-Sticky-Semantik `eq-ipc-v3.schema.json:706-721`; v3 Begin/End, IDs, Sequenz und Nachlauf `:819-854`; heutiger v2-Pfad `protokoll.rs:128-135`, `lib.rs:306-380`. | **Keine neue Wire-Form**: v2-Flag, v3-Begin/End und das optionale Heartbeat-Bit existieren. Der explizite Resync ist eine Coordinator-Lebenszyklusoperation und darf nicht aus einem normalen End/`false` inferiert werden; diese Matrix erfindet dafür keine neue Familie. Eine spätere neue Wirefamilie oder ein neues striktes Objektfeld wäre versionierungspflichtig. | A5/A8/B3c/A4-Rust halten Begin/End samt Begin-`art` gültig und unbekannte Arten ungültig. A4-Rust: `hoermarkierung_v2_v3_gleicher_zustand`, `hoermarkierung_vor_evidence_dispatch`, `intervention_overflow_setzt_sticky_unknown`, `intervention_control_disconnect_setzt_sticky_unknown`, **`intervention_sequenzluecke_setzt_sticky_unknown`**, **`intervention_neutral_resync_entsperrt`**, **`intervention_ueberlappung_bleibt_bis_alle_end`**, **`intervention_falsche_end_id_bleibt_gesperrt`**, `hoermarkierung_tail_sperrt_bis_ende`. Jeder Sticky-Test sendet danach zusätzlich ein passendes End beziehungsweise v2-`false` und beweist, dass dies **nicht** entsperrt; nur der Resync-Test darf starke Evidenz wieder zulassen. |

## 7. Fünf Namensreservierungen ohne Nutzlast

Die bindende Reservierungsregel aus
`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json:5` lautet wörtlich:

> Ihre NAMEN sind trotzdem jetzt vergeben. Ein reservierter Discriminator wird vom v3-Parser ABGELEHNT (er steht nicht im oneOf des Schemas) und darf von keinem spaeteren Ticket fuer etwas anderes verwendet werden. Genau das ist der Vertragsanteil, der spaeter bricht, wenn man ihn heute offen laesst.

Heute sind 17 Familien definiert und 8 reserviert, Summe 25
(`reservierte-nachrichten-v1.json:4,16-77`). Die folgende Zählung ist kumulativ
in Gruppenfolge (a) bis (e). Feldnamen ändern die Familienzahl nicht;
`reference_match` erhöht die reservierten Familien auf 9 und die Summe auf 26.
Eine Reservierung fügt **keine** Nutzlast und keine Annahme ins aktive Schema
ein. A5 muss zusätzlich prüfen, dass jeder reservierte Feldname am strikten
Elternobjekt und jeder reservierte Familienname am Root heute abgelehnt wird.

| Gruppe / konkreter Name | ZUSAGE der Reservierung | Betroffene Stellen | Familienzahl nach Zeile | Additiv/Version bei späterer Einlösung | Riegel/Test |
|---|---|---|---:|---|---|
| (a) `session_snapshot.contribution_inputs` | Reservierter Feldname für später genau einen Deskriptor je diskretem Gen-Eingang. Er ist eine Gen-Eingangsachse, **kein** `probe_descriptor.measurement_position` einer Sondeninstanz. Empfänger ist Gen; Nutzlast und Aux-Bus gehören SONDE-018. Der bestehende `probe_descriptor_beitrag` bleibt daneben als Ausdrucksmittel des v3-Vertrags erhalten, ist nach NAK-114 aber von keiner heutigen Klasse führbar. | Plan S16–17 `plan.json:195`; Entwurf §32.2 `:1632-1675`; `session_snapshot` `eq-ipc-v3.schema.json:750-770`; NAK-79 `docs/offene-punkte.md:34`; NAK-114 §3a. Die Riegel bleiben zu: `capabilities_beitrag`, Capabilitykonstante `NakamaState.cpp:342-358` und Klassenmatrix `:387-415`. | 25 | Reservierung selbst ohne Wirewirkung. Einlösung im strikten `session_snapshot` ist **nicht additiv**: vor R0 einbauen oder neue Major-Version. Sie öffnet den Instanzzweig nicht. | A5/A8/B3c/A4-Rust: `ungueltig/reserviertes-feld-session-snapshot-contribution-inputs.json`. A5-Strukturriegel und die vier NAK-114-Negativfixtures beweisen, dass der Descriptorzweig vorhanden, aber für jede heutige Klasse geschlossen ist. C++-State-Test `post_fader_contribution_bleibt_fuer_alle_sondenklassen_gesperrt` prüft beide State-Riegel. |
| (b1) `state_report.dsp` | Reservierter Feldname für den bestätigten DSP samt Schutzgrenzen. Nur der Name; DTO/Nutzlast entsteht S26–28. | Entwurf §33.5 `:1892-1920`; heutiges striktes `state_report` `eq-ipc-v3.schema.json:1023-1046`; NAK-110 `docs/offene-punkte.md:23`; Plan `plan.json:195,280`. | 25 | Einlösung ist im strikten Objekt **nicht additiv** und gemeinsam mit DSP-State-Save/Load zu versionieren. | A5/A8/B3c/A4-Rust: `ungueltig/reserviertes-feld-state-report-dsp.json`; A5-Riegel `reserviertes_feld_hat_keine_nutzlast`. Späterer Eigentümertest: Reload/Reconnect liest erst `state_report.dsp`, nie blind Broker-Cache. |
| (b2) `command_ack.applied_dsp` | Reservierter Bestätigungsname für die **tatsächlich** angewandten Werte einer `apply_transaction`. Später nur in den Erfolgszweigen `angewandt` und `idempotent_wiederholt`; keine Nutzlast wird hier vorweggenommen. | Entwurf §33.4 `:1861-1890`; `command_ack`-Union `eq-ipc-v3.schema.json:951-1021`; reservierte Familie `apply_transaction` `reservierte-nachrichten-v1.json:46-50`; Plan `plan.json:195`. | 25 | Einlösung ändert die strikten, über `ergebnis` diskriminierten ACK-Zweige und ist **nicht additiv**; vor R0 oder neue Major-Version. | A5/A8/B3c/A4-Rust: `ungueltig/reserviertes-feld-command-ack-applied-dsp.json`; später positive Erfolgsfixtures und negative Ablehnungs-/Konfliktfixtures `command-ack-applied-dsp-bei-nichterfolg`. |
| (c) `state_report.eq_enabled` | Reservierter Feldname für den Betriebszustand des Sonden-EQ. Eigentümer S29–31. Der Name verspricht ausdrücklich **keine** Anzeige auf Gens Landkarte. | Plan S16–17 `plan.json:195`, Berichtigung S18–19 `:218`; heutiges `state_report` `eq-ipc-v3.schema.json:1023-1046`; NAK-107 `docs/offene-punkte.md:26`. | 25 | Einlösung im strikten Objekt ist **nicht additiv**; vor R0 oder neue Major-Version. | A5/A8/B3c/A4-Rust: `ungueltig/reserviertes-feld-state-report-eq-enabled.json`; A5 prüft Scope-Kollision mit `state_report.dsp`. Späterer Eigentümertest `eq_enabled_roundtrip_ohne_ui_zusage` misst nur Fernsteuerzustand. |
| (d1) `probe_descriptor.host_bus_name` | Reservierter optionaler Feldname. Fehlt er später, lautet die einzige Aussage „der Host liefert keinen Wert“; `label` bleibt gespeichertes, untrusted User-Wort. Typ, Länge und FL-Befüllung werden nicht vorweggenommen. | Plan S16–17 `plan.json:195`, S18–19 `:218`; strikte Descriptor-Union `eq-ipc-v3.schema.json:465-574`; NAK-106 `docs/offene-punkte.md:25`. | 25 | Die Reservierung verhindert Umwidmung, macht die spätere Einlösung aber **nicht additiv**: alle vier strikten Descriptorzweige und alle Leser brauchen eine versionierte Änderung. | A5/A8/B3c/A4-Rust: `ungueltig/reserviertes-feld-probe-descriptor-host-bus-name.json`; positives bestehendes Descriptorfixture ohne Hostfelder bleibt gültig. S18–19 misst separat `fl_track_properties_bus_name_present_or_absent`. |
| (d2) `probe_descriptor.host_mixer_index` | Reservierter optionaler Feldname. Fehlen heißt „der Host liefert keinen Wert“; daraus darf weder Verbindungsreihenfolge noch eine erfundene Mixernummer abgeleitet werden. Typ/Bereich und FL-Befüllung bleiben S18–19. | Dieselben Descriptor-/Planstellen wie (d1), insbesondere `plan.json:218`. | 25 | Wie (d1): Reservierung ja, spätere Einlösung **nicht additiv** und versionierungspflichtig. | A5/A8/B3c/A4-Rust: `ungueltig/reserviertes-feld-probe-descriptor-host-mixer-index.json`; positives Fixture ohne Wert; S18–19 `fl_track_properties_mixer_index_present_or_absent`. |
| (e) `reference_match` | Reservierter Top-Level-Familienname für Referenz-Nachbilden/EQ-Match. Nur Zweck und Name; kein Request-/Result-/Payloadmodell. Ausbau bleibt laut U19 hinten. | Plan S16–17 `plan.json:195`; Entwurf §47.5 `:3044-3059`; Root-Discriminator `eq-ipc-v3.schema.json:1-27`. | **26** = 17 definiert + 9 reserviert | Die Reservierung selbst bleibt abgelehnt. Spätere Aktivierung eines Top-Level-Discriminators ist **nicht additiv** und braucht eine neue Major-Version, sofern sie nicht vor R0 vollständig definiert wird. | A5 prüft disjunkte Namen und neue Summe 26; A8 erzeugt `ungueltig/reservierter-typ-reference-match.json`; B3c/A4-Rust müssen ihn mit `oneOf` ablehnen. Test `reservierter_name_reference_match_wird_nicht_umgewidmet`. |

Damit gilt nach dem Entscheid 28-B verbindlich: Das Verbindungsende ist der
Gegenpfad, `unsubscribe_session` entsteht nicht und die Gesamtsumme ist mit
`reference_match` **26**. Der bisherige A5-Riegel „Summe 25“
(`reservierte-nachrichten-v1.json:6`) muss bei Umsetzung auf 26 geändert werden;
eine bloße Änderung der Zahl ohne Disjunktheits-, Ablehnungs- und Fixturebeweis
genügt nicht.

## 8. Abschlussprüfung gegen PR1 und Plan

| Prüfung | Ergebnis/ZUSAGE | Fundstellen | Additiv/Version | Test/Beweisbein |
|---|---|---|---|---|
| Zusätzlicher zugewiesener Vertragspunkt | **NAK-114 gefunden und in §3a aufgenommen.** Das Register weist die fehlende Kopplung `plugin_kind` × `measurement_position` ausdrücklich S16–17 zu. Sie ist keine Begründung, `probe_descriptor_beitrag` zu entfernen, sondern schließt dessen heutige Trägerklassen fail-closed nach der bereits bindenden State-v2-Matrix. | `docs/offene-punkte.md:17`; `nakama-state-v2.md:45-54,76-90`; bindender Beitragswortlaut `docs/plan/plan.json:195`; Matrixgruppe §3a dieses Manifests. | **Versionierung**, weil der strikte diskriminierte Descriptor und die erlaubten Achsenkombinationen geändert werden. | Q1 plus sämtliche in §3a benannten A5/A8/B3c/A4-Rust-Struktur-, Positiv- und Negativbeine. |
| Weitere ausdrücklich dieser Runde zugewiesene Vertragspunkte | **Geprüft, nach Aufnahme von NAK-114 keine weiteren unbedingten Punkte.** PR1 weist NAK-10/28/29/40/59 der Runde zu und seine damalige Reservierungstabelle enthält (a) bis (d); der jüngere S16–17-Text ergänzt (e). Der laufende Überdeckungswert aus U16 ist **noch kein reservierbarer Vertragsname**: S23–25 entscheidet erst am Datenweg, ob überhaupt ein eigenes v3-Feld nötig ist. Nur falls dort „ja“ belegt wird, muss der Name vor R0 in diese Vertragsrunde zurückkehren; er darf nicht still später angefügt werden. | `docs/beweise/PR1.md:254-258,575-583,898-904`; `docs/plan/plan.json:195,254-259`, besonders die Bedingung in `:257`; Registerprüfung `docs/offene-punkte.md:11-29`. | Heute keine weitere Wire-Änderung. Ein später bejahter Name wird nach seinem Ort mit dem strikten Elternobjekt beziehungsweise Discriminator klassifiziert und ist nicht automatisch additiv. | Q1 auf Basis `b75ea06`. Kein A5/A8/B3c/A4-Rust-Fixture für U16, solange Quelle, Ort und Name nicht entschieden sind; ein erfundenes Fixture würde gerade die offene Datenwegentscheidung vorwegnehmen. |

## 9. Gefallene Dirigentenentscheidungen

1. **NAK-28 — ENTSCHIEDEN 28-B:** Das Verbindungsende ist der einzige
   Gegenpfad von `subscribe_session`; keine Familie `unsubscribe_session`.
   Linkbesitz wird im Server-Integrationstest bewiesen. Endsumme mit
   `reference_match`: **26**.
2. **NAK-59 — ENTSCHIEDEN 59-A:** `band_stereo` wird optionales
   FlatBuffers-Framefeld mit ID 10 und erhöhter Minor-Version. Schema,
   ID-Register, `flatc`-Codegen, beide Leser, Sender und Drift-/Binärbeweise
   bilden einen Änderungssatz.
3. **NAK-40 — ENTSCHIEDEN:** Gültiges hex32 bleibt identisch; sonst gilt die
   domain-separierte, längengebundene SHA-256/128-Abbildung ohne
   Normalisierung. Unbekanntes und jede Kollision sind fail-closed; eine
   Kollision quarantänisiert atomar **beide** Seiten, einschließlich der zuerst
   registrierten, in beiden Registrierungsreihenfolgen.
4. **Beitragszweig — ENTSCHIEDEN:** `probe_descriptor_beitrag` bleibt als
   Ausdrucksmittel des Vertrags. NAK-114 koppelt die vier heutigen
   `plugin_kind` an die State-v2-Positionsmatrix, sodass keine heutige Klasse
   `post_fader_contribution` führen kann; `capabilities_beitrag` und beide
   State-Riegel bleiben zusätzlich geschlossen.

## 10. Bewusst nicht aufgenommen

- **Broker-Autostart/Beenden/Installer-Rückweg** steht zwar im selben S16–17-Text
  (`plan.json:195`), ist aber Phase B und kein Vertragsmatrix-Anteil dieses
  ausdrücklich auf Phase A begrenzten Spezifikationsschritts.
- **Nutzlasten** der sieben konkreten Reservierungsnamen sind nicht erfunden:
  Contribution gehört SONDE-018, DSP S26–28/S29–31, Hostfelder S18–19,
  EQ-Match in den späteren Ausbau. Reservierung heißt bis dahin Ablehnung.
- **Das Entfernen von `probe_descriptor_beitrag`** ist bewusst nicht
  aufgenommen. `plan.json:195` hält das Instanzfeld fail-closed, und
  `nakama-state-v2.md:76-80` verlangt, dass der Vertrag die Position weiter
  ausdrücken kann. Die vier heutigen `plugin_kind` werden deshalb im
  Beitragszweig abgelehnt, ohne den Zweig zu löschen: Gesperrt ist die heutige
  Trägerbehauptung, nicht das Wort.
- **Eine künftige Trägerklasse für `post_fader_contribution`** wird nicht
  erfunden. Welche Klasse die Position bei real vorhandenem diskretem Bus führen
  darf, bleibt laut `nakama-state-v2.md:86-90` eine eigene Produktentscheidung
  und spätere Versionierung.
- **NAK-115** erhält keine eigene v3-Matrixzeile: Er betrifft den v2-Rollen-
  Rückweg in `NakamaState.cpp` und ist im Register primär S7/SONDE-006
  zugeordnet (`docs/offene-punkte.md:18`). Solange keine heutige Klasse den
  Beitragszweig führen darf, bleibt dieser Rückweg unerreichbar; ein späterer
  Trägerentscheid muss ihn mit Save+Load gemeinsam lösen.
- **Die verworfenen Wege** `unsubscribe_session` und „Band-Stereo bleibt rein
  lokal“ sind keine offenen Baualternativen und erhalten weder Nutzlast noch
  Positivfixture.
- **Ein Capabilityname für FL-Hostfelder** ist nicht ergänzt. S18–19 besitzt die
  echte Hostmessung und das Capability-Ergebnis (`plan.json:218`). Da Capability
  nicht additiv ist, muss ein dort nötiger Name vor R0 zur Abnahme zurück oder
  eine neue Major-Version erhalten; diese Matrix darf ihn ohne Messung nicht
  erfinden.
- **Ein v3-Feld für die Überdeckungsanzeige** ist aus dem in §8 genannten
  bedingten Grund nicht reserviert.
- **NAK-29-Erzeugerfall 7** (`sample_count==0`, über 1 048 576 oder
  `start+count`-Überlauf) bleibt ein FeatureEngine-Publikationsvertrag. Der
  allgemeine Wirevertrag kann laut Quellenkommentar einen leeren Frame
  darstellen (`FeatureEngine.h:234-244`); ihn hier zu verbieten wäre ein neuer,
  nicht zugewiesener Wireentscheid.
- **Produktcode, Schema, Fixtures und Reservierungsdatei** werden in diesem
  Spezifikationsschritt nicht geändert. Ihre späteren Änderungen sind genau die
  in jeder Matrixzeile benannten Bau- und Beweisaufträge.
