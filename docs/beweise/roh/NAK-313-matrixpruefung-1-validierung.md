# NAK-313 — Validierung der Matrixprüfung 1 an der Quelle

| Merkmal | Wert |
|---|---|
| Ticket | NAK-313 (S25l), Etappe 1: Validierung des Rohurteils der Matrixprüfung 1 im Auftrag des Dirigenten (`docs/beweise/roh/NAK-313-matrixpruefung-1-validierung-auftrag.txt`) |
| HEAD | zu Beginn `de2731aa75d6b750305106831e65d9ec736def51` (23.09.2026, 22:25:44 +02:00); vor dem Schreiben `de2731aa75d6b750305106831e65d9ec736def51` (22:51:02 +02:00) |
| Datum | 23.09.2026 |
| Modell | Claude Opus 5.5 (`claude-opus-5-5`), frischer lesender Validierer; weder Bauer der Matrix noch Prüfer |
| Gegenstand | `docs/beweise/roh/NAK-313-matrixpruefung-1-urteil.md` (Codex `gpt-6-astra` xhigh, NEEDS_WORK, 14 DEFEKTE), Befunde D1 bis D14 in der Reihenfolge ihres Auftretens (Urteilsdatei Zeilen 12-52), gegen `docs/beweise/NAK-313.md` §5 bis §9 (Stand `ed6387cb`) mit §0 bis §4 und §10 als Gate-Bezug. `git diff ed6387cb HEAD -- docs/beweise/NAK-313.md` fügt nur §10 an (+62 Zeilen ab Zeile 2276); alle Zeilenangaben in NAK-313.md unten gelten deshalb für `ed6387cb` und HEAD gleich. Die Zeilenanker des Prüfers (`NAK-313.md:970`, `:972`, `:997`, `:1013`, `:1079`, `:1103`, `:1109`, `:1137`, `:1170`, `:1311-1317`, `:1323-1325`, `:1331-1335`, `:2025-2029`, `:2040-2043`) treffen alle die gemeinte Stelle. |
| Prüfform | Quellenlektüre mit Read/Grep, `git rev-parse`, `git status`, `git diff`, `git grep`, dazu lesend `sed`/`grep`/`ls` über Repo-Dateien, die JUCE- und FlatBuffers-Quellen unter `eq-copilot/build/_deps/` und die gepinnten Crate-Quellen unter `~/.cargo/registry/` (flatbuffers 25.12.19, serde_json 1.0.151, zmij 1.0.23). Kein Bau, kein Test, kein Kanon, kein FL Studio, kein Codex. **Grenzverstoß, offen benannt:** zwischen 22:25 und 22:51 Uhr (Zeitpunkt nicht einzeln gemessen) habe ich einen lesenden Python-Einzeiler gestartet (Auflisten der Wurzel-`oneOf` des v3-Schemas); er hing ohne Ausgabe bis zum Befehls-Timeout und wurde abgebrochen. Nichts daraus ist verwendet, keine Datei entstand (`git status --short` danach unverändert); dieselbe Tatsache ist unten per Grep/Read belegt (`eq-ipc-v3.schema.json:9-36`, `:838-845`). |
| Arbeitsbaum | `git status --short` zu Beginn und vor dem Schreiben identisch: `?? briefing-hub/`, `?? docs/beweise/roh/NAK-313-matrixpruefung-1-urteil.md`, `?? docs/beweise/roh/NAK-313-matrixpruefung-1-validierung-auftrag.txt`, `?? nimbalyst-local/`. Einzige Schreibaktion: diese Datei (neu, LF). Nichts gestagt, committet oder gepusht; fremde Ordner nicht angefasst. |
| Ergebnis | **12 BESTÄTIGT, 2 PRÄZISIERT (D5, D10), 0 WIDERLEGT.** Alle 14 sind DEFEKT; keiner fällt unter einen Ausschluss des Prüfauftrags. D5 und D10 reichen weiter als gemeldet. |

Schreibweise: „NAK-313.md:n" meint `docs/beweise/NAK-313.md`; wörtliche Zitate stehen in Anführungszeichen oder als Code, Auslassungen als `…`, Zeilenbereiche mit ASCII-Bindestrich.

---

## Teil 1 — die Befunde D1 bis D14

### D1 — M-09: bestehende Wache und neuer Aufrufzähler vermischt

**a) Beanstandet.** NAK-313.md:970 (M-09), Zellen wörtlich: Zustand „(a) frischer Gen, leeres Label; (b) Stand aus M-01" · Zusage „**Kein Aufruf, keine Mutation, keine Dirty-Meldung in (a) und (b)**" · Reihenfolge „erste Sperre: Unverändert-Prüfung im Panel; zweite: der Frühausstieg `if (neu == zustand.common) return false;` (`State.cpp:343-344`) bleibt innerhalb der Sperre" · Test „C++: B15 bestehend `312/M-37 ohne_aenderung_keine_mutation` (`ShotTestMain.cpp:480-485`), um Aufrufzähler und Fall (b) ergänzt" · Rotbeweis „**zwei Hälften.** (a) **Regressionswache**; einmal gebrochen: Unverändert-Prüfung entfernen → 1 Aufruf → rot. (b) **heute rot** wie M-01".

**b) Nachmessung.**
- Frischer Gen: `eq-copilot/plugin/src/PluginProcessor.cpp:268` `zustand = nakama::state::frisch (juce::Uuid().toString());`; `eq-copilot/plugin/state/NakamaState.cpp:2173-2174` `z.common.klasse = Klasse::legacy;` / `z.common.position = Messposition::insert;`.
- Das Schließen ruft ohne Vergleich: `eq-copilot/plugin/src/PluginEditor.cpp:784` `~MesspunktPanel() override { uebernehmen(); }` und `:816` `if (ed->processor.setzeBindung (rolle, label, paarId) && geaendert)`.
- Mutation und Dirty verhindert allein `eq-copilot/plugin/src/prozessor/State.cpp:343-344` `if (neu == zustand.common)` / `return false;   // keine Aenderung: kein Dirty, kein Reconnect-Geflacker`.
- Der bestehende Fall misst keinen Aufruf: `eq-copilot/plugin/tests/ShotTestMain.cpp:480-482` `const auto m37 = panelLauf (PanelFolge::panelVorEditor, false, nullptr);` / `panelPruefe (m37.geoeffnet && m37.panelWeg && m37.marke == 0 && m37.dirty == 0` / `&& m37.labelNachPanel == m37.labelVorher,`.

Reproduktion stimmt: am ZIEL gibt es in (a) genau einen Eintritt in `setzeBindung`; eine Unverändert-Prüfung im Panel existiert nicht, ihre Entfernung als Rotbeweis setzt Neubau voraus.

**c) Was bricht.** Etikettendefinition §6, NAK-313.md:903-905: „**Regressionswache**: heute grün und hält, was der Bau nicht ändern darf; sie wird einmal absichtlich gebrochen und ist kein Beleg". Die Zusagehälfte „Kein Aufruf" ist heute nicht grün (ein Aufruf, `PluginEditor.cpp:816`) und ohne Zähler nicht messbar; dafür gilt NAK-313.md:902-903: „**heute nicht messbar**: Mechanismus, Zähler oder Fixture entsteht erst im Bau". Gemessen wird R-313-3, NAK-313.md:127-128: „unverändert heißt: kein Aufruf, kein Dirty, kein Reconnect, kein gekürzter Text". Die Nachbarzeilen trennen richtig: M-08 (NAK-313.md:969) „**heute nicht messbar** (kein Aufrufzähler; heute 2 Aufrufe, der zweite ohne Dirty)", M-10 (:971) Rückgabe/Dirty gegen Reconnect-Zähler, M-13 (:974) „(b) **heute nicht messbar** (Aufrufzähler)".

**d) Urteil.** BESTÄTIGT · **DEFEKT** (Etikett widerspricht dem gemessenen Code am ZIEL). Kein Ausschluss: §6-Inhalt, weder Sicherheit noch U65, Sabotage oder Prüfliste.

**e) Schließende Regel.** M-09 (a) führt „kein Aufruf" als eigene Hälfte „heute nicht messbar" mit der Mutation an der neuen Unverändert-Prüfung und „keine Mutation, keine Dirty-Meldung" als Regressionswache, deren Einzelbruch an der Stelle liegt, die sie nach dem Bau allein trägt (Frühausstieg `State.cpp:343-344`, gebrochen in M-10). **Folgestellen:** nur M-09; M-08, M-10 und M-13 tragen die Trennung schon (Teil 2 Punkt 7).

### D2 — M-11: Paarname ohne Paarrolle

**a) Beanstandet.** NAK-313.md:972 (M-11): Zustand „frischer Gen, Popover offen" · Ereignis „in das leere Labelfeld 121-mal U+10FFFF über `insertTextAtCaret` einfügen (der Weg des Tippens und Einfügens), in das Paarfeld 61-mal" · Zusage „**Labelfeld hält 120 Codepunkte (480 UTF-8-Bytes), Paarfeld 60; nach dem Schließen genau 1 Aufruf, Label 120 und Paarname 60 Codepunkte, genau 1 Host-Dirty; nichts wird gekürzt, weil nichts Längeres entstehen kann**" · Rotbeweis „**heute rot** — das Feld nimmt 121 an, erst `substring (0, 120)` kürzt still. Nach dem Bau: `setInputRestrictions` am Labelfeld entfernen → 121 → rot".

**b) Nachmessung.**
- `eq-copilot/plugin/state/NakamaState.h:544-545` „`/** Frische Instanz (nie restauriert): legacy + insert, leeres Label. */`" / `Zustand frisch (const juce::String& instanceId);`; `NakamaState.cpp:2303-2311` (`v2Rolle`): `if (c.klasse == Klasse::main)` / `return "hub";` … `default:                 return "sensor";`.
- `PluginEditor.cpp:757-759`: `const auto r = proz.holeRolle();` / `rolleWahl.setSelectedId (r == "hub" ? 2 : r == "pre" ? 3 : r == "post" ? 4 : 1,` → id 1.
- `PluginEditor.cpp:788-790`: `const auto id = rolleWahl.getSelectedId();` / `const bool paar = (id == 3 || id == 4);` / `paarFeld.setEnabled (paar);`.
- `PluginEditor.cpp:809-811`: `const bool paar = (id == 3 || id == 4);` … `const auto paarId = paar ? paarFeld.getText().substring (0, 60) : juce::String();` — für id 1 wird ein leerer Paarname übergeben.
- §8.2, NAK-313.md:1553-1556: „`uebernehmen()` behält die Lebendprüfung als erste Anweisung, vergleicht danach Rolle, Label und — nur bei Paarrollen (id 3 und 4), sonst wird das deaktivierte Feld nicht gelesen — den Paarnamen mit den Merkwerten".
- Genauer als der Prüfer: das Einfügen scheitert nicht am deaktivierten Feld. `eq-copilot/build/_deps/juce-src/modules/juce_gui_basics/widgets/juce_TextEditor.cpp:1176-1191` (`insertTextAtCaret`) fragt `isReadOnly()` nicht ab, obwohl `:339-342` `return readOnly || ! isEnabled();` liefert. Der Text steht im Feld, wird aber weder heute (`:811`) noch nach §8.2 übernommen.

Reproduktion stimmt.

**c) Was bricht.** Die Zusage „Paarname 60 Codepunkte" ist mit dem Zustand der Zeile am Code nicht erreichbar (`PluginEditor.cpp:788-791`, `:811`) und widerspricht dem eigenen Bauplan §8.2 (NAK-313.md:1554-1556). §6 Spalten, NAK-313.md:889-890: „*Zustand und Vorbedingung* den Stand, den der Fall herstellt". Für das Paarfeld bleibt R-313-3 Satz 3 (NAK-313.md:129-130: „Keine stille Kürzung: die Felder tragen Eingabegrenzen 120/60") ungemessen.

**d) Urteil.** BESTÄTIGT · **DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** Jede Panelzeile, die den Paarnamen misst, stellt vorher eine Paarrolle (pre oder post) her, bevorzugt über denselben Ladeweg wie M-01, und misst Aufrufe, Reconnect-Anforderungen und Host-Dirty ab dem Stand nach dem Laden. **Folgestellen:** M-11 (eigene Paarmutation: `setInputRestrictions (60)` entfernen → 61 → rot); die Teilfälle M-13 (nur Label) bleiben; §8.2 `panelLauf`-Handlungsfolge „Einfügen" (NAK-313.md:1579-1583) und B15-Satz „die Felder begrenzen die Eingabe auf 120 und 60 Codepunkte" (:1594-1595). Einrichtungsweg: Teil 2 Punkt 7.

### D3 — M-18: Rotmutation `pair_id` bleibt über der Grenze

**a) Beanstandet.** NAK-313.md:997 (M-18): Zustand „Common mit 65 536 Einträgen mit `label`" · Zusage „**Load `nurLesen` bytegleich; `setzeBindung` verweigert ohne Host-Dirty; kein Folgezustand, den der nächste Load `ignoriert`**" · Reihenfolge „Kandidat 65 538 fällt am Riegel" · Rotbeweis „… Nach dem Bau: `pair_id` aus dem Kandidaten nehmen (`:1236`) → `geladen` → rot". Kopf §6.2, NAK-313.md:987-990: „der Headroomkandidat setzt `label`, einen `pair_id` und eine `project_binding_id` (`NakamaState.cpp:1235-1238`); ein Common mit `n` Einträgen ist genau dann schreibbar, wenn `n` + fehlende dieser drei ≤ 65 536."

**b) Nachmessung.**
- `NakamaState.cpp:1235-1238`: `kandidat.common.label = laenger (eingang.common.label, maximalerText (120));` / `kandidat.common.pairId = laenger (eingang.common.pairId, maximalerText (60));` / `kandidat.common.projectBindingId = laenger (` / `eingang.common.projectBindingId, "ffffffffffffffffffffffffffffffff");`.
- Writer `NakamaState.cpp:1099-1102`: `if (z.common.pairId.isNotEmpty()) common.setProperty (kPairId, z.common.pairId, nullptr);` und `if (z.common.projectBindingId.isNotEmpty()) common.setProperty (kBinding, z.common.projectBindingId, nullptr);` — fehlen beide, entstehen im Kandidaten zwei neue Einträge.
- Riegel `NakamaState.cpp:196` `if (! komprimierteZahl (wert) || wert < 0 || wert > kMaxEintraegeJeSammlung)` mit `:94` `constexpr int kMaxEintraegeJeSammlung = 65536;`.
- Rechnung am Vektor: 65 536 (mit `label`) + `pair_id` + `project_binding_id` = 65 538; ohne den `pair_id`-Zusatz 65 537 > 65 536. Der Riegel in `passt` lehnt weiter ab, die Zeile bleibt `nurLesen` und grün.

Reproduktion stimmt.

**c) Was bricht.** Matrixregel §6, NAK-313.md:911-912: „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt, nie an einem Nebeneffekt" (ebenso §2, NAK-313.md:106-107). Hier fällt er überhaupt nicht; die Zeile hat faktisch keinen Rotbeweis (DEFEKT-Klasse des Prüfauftrags: „eine Zeile nennt keinen … Rotbeweis").

**d) Urteil.** BESTÄTIGT · **DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** Eine Mutation, die einen Kandidatenzusatz entfernt, muss am Vektor der Zeile nachgerechnet den Kandidaten unter die Grenze bringen (≤ 65 536 je Sammlung, ≤ 262 144 im Baum); sonst sitzt der Rotbeweis am Riegelaufruf in `passt`. **Folgestellen:** nur M-18. Nachgerechnet und tragfähig: M-19 (65 535 mit `label`, Kandidat 65 537, ohne `project_binding_id` 65 536), M-21 (65 534 ohne die drei, Kandidat 65 537, ohne `pair_id` 65 536), M-17, M-20, M-24 (Riegelaufruf). Rotvektor: Teil 2 Punkt 7.

### D4 — M-34: nicht rekursive Nachzählung trifft einen Nebeneffekt

**a) Beanstandet.** NAK-313.md:1013 (M-34): Zustand „(a) Duplikat in einem Kindknoten von Common (Ebene 2); (b) Common mit 65 536 Einträgen, davon ein Name doppelt" · Zusage „**Beide `nurLesen` bytegleich; die Erkennung zählt, sie sammelt keine Namen: der Riegel zählt die Eigenschaftseinträge im vorhandenen Durchgang, die Nachzählung läuft rekursiv über den gelesenen Baum**" · Rotbeweis „**heute rot** (beide `geladen`). Nach dem Bau: die Nachzählung ohne Rekursion → (a) `geladen` → rot". §8.3, NAK-313.md:1630-1636: „Der Riegel zählt in seinem vorhandenen Gang die Eigenschaftseinträge aller Knoten … `lade` zählt nach `readFromData` (`:2245`) rekursiv `getNumProperties()` über den gelesenen Baum …; ist die Zahl kleiner, hat JUCE einen doppelten Namen zusammengelegt".

**b) Nachmessung.** Der Riegelgang läuft über alle Knoten, `NakamaState.cpp:287-299`: `int eigenschaften = 0;` / `if (! zaehler (eigenschaften))` … `for (int i = 0; i < kinder; ++i)` / `if (! baum (tiefe + 1))`. Die Vorzählung nach §8.3 enthält damit die Eigenschaften von `Common` (Kind der Wurzel, `synchronisiert` schreibt dort immer Eigenschaften, `:1094-1098`) und seiner Kinder. Eine Nachzählung ohne Rekursion zählt nur die Wurzel und ist für **jeden** realen Stand kleiner: jeder Stand wird `nurLesen`, (a) bleibt `nurLesen`, M-34 bleibt grün, rot wird M-33 (NAK-313.md:1012, Goldens und Roundtrip). Reproduktion stimmt; der Prüfer schreibt „allenfalls gültige Bäume werden zusätzlich abgewiesen" — es trifft sicher jeden Stand.

**c) Was bricht.** Matrixregel §6, NAK-313.md:911-912: der Rotbeweis fällt an einem Nebeneffekt (M-33), nicht an der Zusage von M-34.

**d) Urteil.** BESTÄTIGT · **DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** Die Mutation für den tiefen Fall beschneidet Vor- und Nachzählung symmetrisch auf dieselbe Tiefe, sodass eindeutige Stände gleich gezählt bleiben und allein das Duplikat unterhalb der Schnitttiefe unentdeckt bleibt. **Folgestellen:** M-34 (a); §8.3 Erkennungsabsatz (NAK-313.md:1630-1641) als Ort; M-33 bleibt die Nebeneffekt-Kontrolle. Mutation: Teil 2 Punkt 7.

### D5 — M-65: Mutation am Vergleich, Beobachtung am Deskriptor

**a) Beanstandet.** NAK-313.md:1079 (M-65): Zustand „Heartbeat mit `runtime`-Deskriptor, `host_mixer_index` `3.0`" · Ereignis „`heartbeat_kontakt`" · Zusage „**Der Deskriptor trägt Kanal 3 wie bei `3`**" · Reihenfolge „Ganzzahlhelfer statt `as_i64` (`broker/src/coordinator/liveness.rs:442-446`)" · Rotbeweis „**heute rot** — `as_i64` auf 3.0 ist `None`, der Kanal fehlt still. Nach dem Bau: `as_i64` zurück → rot".

**b) Nachmessung.**
- Vergleich, `broker/src/coordinator/liveness.rs:442-446`: `let kanal_von = |wert: Option<&Value>| -> Option<i64> {` / `wert.and_then(|d| d.get("host_mixer_index"))` / `.and_then(Value::as_i64)` / `.filter(|index| *index >= 1)` / `};` und `:454-461` `if alt_kanal != neu_kanal {` / `kanalwechsel = Some((`.
- Übernahme, `liveness.rs:752-756`: `for feld in ["host_bus_name", "host_mixer_index"] {` / `if let Some(wert) = runtime.get(feld) {` / `objekt.insert(feld.into(), wert.clone());` — der Rohwert; `descriptor_vertrag_erfuellt` (`:653-712`) prüft das Feld nicht.
- Speicherung, `liveness.rs:471-472`: `if runtime_gemeldet {` / `client.descriptor = descriptor;`.

Reproduktion stimmt: der Deskriptor trägt `3.0`, „der Kanal fehlt still" ist falsch; `as_i64` wirkt nur im Vergleich. **Über den Prüfer hinaus, am Code verfolgt:**
- Gen verwirft heute den ganzen Snapshot. `broker/src/coordinator/sicht.rs:545-557` kopiert `client.descriptor` in den `session_snapshot` (`mitglied_objekt.insert("probe_descriptor".into(), descriptor);`); serde_json 1.0.151 schreibt f64 über zmij (`serde_json-1.0.151/src/ser.rs:1720-1722`), das Ganzzahlen mit `.0` ausgibt (zmij 1.0.23 `src/lib.rs:1624` „`// 1234e7 -> 12340000000.0`"). Gen liest das Feld mit `nichtnegativeGanzzahl` (`eq-copilot/plugin/src/SourcesModel.cpp:310-314`), die nur `isInt()`/`isInt64()` annimmt (`:101-117`); JUCE liest `3.0` als double → `SourcesModel.cpp:1302-1309` `fehler = "probe_descriptor is invalid or disagrees with its member";` / `return SnapshotErgebnis::ungueltig;`.
- Ein zweiter Rust-Leser desselben gespeicherten Werts: `broker/src/coordinator/hypothese_verdrahtung.rs:296-305` (`.and_then(|d| d.get("host_mixer_index"))` / `.and_then(Value::as_i64)`); er steht nicht in den Ticketpfaden §8.5 (NAK-313.md:1831-1835). Der Zwilling des Vergleichs in `descriptor_setzen` (`liveness.rs:859-863`) hat laut `:852-858` keinen Produktaufrufer.

**c) Was bricht.** Matrixregel §6, NAK-313.md:911-912: die Mutation an `:442-446` verändert die gemessene Beobachtung (Deskriptorwert) nicht. R-313-5, NAK-313.md:160-162: „Rust aus dem geparsten Wert nach dem Textriegel an allen Produktstellen (Heartbeat, Evidenz, Bootstrap, typisierte Felder)" — die weiteren Stellen desselben Werts sind ohne Zeile.

**d) Urteil.** PRÄZISIERT · **DEFEKT**. Die Reproduktion trifft; der Befund reicht weiter (Gen verwirft heute den Snapshot; ein zweiter Rust-Leser fehlt im Plan). Kein Ausschluss.

**e) Schließende Regel.** M-65 misst die Wirkung, die der Helfer an seiner Stelle trägt (Kanalwechselreaktion bei `3` gefolgt von `3.0`), und jede weitere Stelle, die `host_mixer_index` aus dem gespeicherten Deskriptor liest, bekommt den Helfer oder schon beim Übernehmen die Ganzzahl, jeweils mit Zeile. **Folgestellen:** `liveness.rs:859-863`; `hypothese_verdrahtung.rs:296-305`; Gen `SourcesModel.cpp:310-314` (M-61, NAK-313.md:1075, hat keinen `host_mixer_index`-Vektor); §8.5 „Orte", NAK-313.md:1878-1879 („Laufzeitangaben über den Helfer (`liveness.rs:442-446`)"); M-73 (Eintrag `rust_p0`). Entscheidungsgrundlage: Teil 2 Punkt 1.

### D6 — M-89: Intentwriter im Testziel B10 nicht erreichbar

**a) Beanstandet.** NAK-313.md:1103 (M-89): Ereignis „`v3IntentUpdateJson`, dann Rust-Intentleser" · Zusage „**Der Broker übernimmt `bestand_revision` 9007199254740991 bytegleich**" · Test „C++: B10; Rust: A4 **NEU** `nak313_m89_intent_revision_am_rand`".

**b) Nachmessung.**
- `eq-copilot/plugin/CMakeLists.txt:977-978`: `juce_add_console_app(EqCopIpcTest PRODUCT_NAME "eqcop-ipctest")` / `target_sources(EqCopIpcTest PRIVATE tests/IpcTestMain.cpp core/ipc/PipeToken.cpp)`; `:983` `nakama_kern_anbinden(EqCopIpcTest)`. `NakamaKern` (`:198-220`) enthält `state/`, `vertrag/`, `core/ipc/…`, `core/diagnose/Briefkasten.cpp`, aber keine Datei unter `src/prozessor/`.
- `eq-copilot/plugin/src/prozessor/Ipc.cpp:548` `std::string EqCopilotProcessor::v3IntentUpdateJson (bool vollstaendig,` — Member des Prozessors; `:586` `+ ",\"bestand_revision\":" + std::to_string ((long long) revision)`.
- Die Prozessorquellen `CMakeLists.txt:111-117` (`src/prozessor/Ipc.cpp` in `:115`) übersetzt u. a. `EqCopSonde014IntentTest` (`:888-897`, `${NAKAMA_PROZESSOR_QUELLEN}` in `:891`), das Ziel von B27.
- Zugang `eq-copilot/plugin/src/PluginProcessor.h:366-367`: `std::string v3IntentUpdateFuerTest (bool vollstaendig) const` / `{ return v3IntentUpdateJson (vollstaendig, nullptr, nullptr, nullptr); }`; `git grep` findet keinen Aufrufer unter `eq-copilot/plugin/tests/` (nur die Auditprobe `docs/audits/2026-09-15-tiefenaudit/roh/phase-16-nachweise/c/state_probe.cpp:142`).

Reproduktion stimmt.

**c) Was bricht.** Die Testzelle „C++: B10" widerspricht dem Bauziel am ZIEL (`CMakeLists.txt:977-983`). R-313-4 Satz 4, NAK-313.md:143-144: „Sender geben die Zahl unverändert aus" — für den Intent-Sender ohne ausführbaren C++-Test. Gate BEWEIS (NAK-313.md:6): „identischen Positiv-/Negativvektoren durch echte C++- und Rust-Reader"; das Writer-Muster derselben Regelfamilie, R-313-7 (NAK-313.md:192-193): „der echte C++-Writer läuft durch den echten Rust-Empfänger (Byteinstanz)". Die vom Prüfer zitierte `AGENTS.md:53-54` („Writer, Reader, Migration, Fixtures und Cross-Language-Verbraucher im selben Änderungssatz berücksichtigen") ist Arbeitsregel, keine Gate-Quelle.

**d) Urteil.** BESTÄTIGT · **DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** Eine Zeile, die einen Produktwriter misst, nennt ein Bein, dessen Ziel die Writerquelle übersetzt, und bringt dessen Ausgabe über eine von Hand geschriebene Byteinstanz, gegen die der Writer bytegleich verglichen wird, zum Rust-Leser. **Folgestellen:** M-89; §8.5 Ticketpfade und Behauptung B27; §8.1 Ausnahmeliste der Handinstanzen (NAK-313.md:1494-1496 nennt nur `heartbeat-wire-v1.json`). Geprüft und tragfähig: M-88 (`Vertrag.cpp`, `Intern.h` im Kern), M-123 und M-124 (`Intern.h`). Weg: Teil 2 Punkt 2.

### D7 — M-95: ein Urteil für Vertragsreferenz und zustandsabhängigen Produktleser

**a) Beanstandet.** NAK-313.md:1109 (M-95): Zusage „**Dasselbe Urteil wie der C++-Client für jeden v2-Eintrag: `9223372036854775807` gültig am Bereich, `9223372036854775808`, `18446744073709551618`, `2e4294967296` und `2.0000000000000001` ungültig**". Dazu M-93 (NAK-313.md:1107): Zustand „verbunden, erster Heartbeat mit `aktuelleSeq` 0, zweiter mit 1", Zusage „(c) passiert den Riegel und endet mit „ungueltiges heartbeat_ack" (Sequenz passt nicht)"; §7.2 (NAK-313.md:1316-1317): „bei `v2` gilt die Stufe des C++-Clients, A11 vergleicht Urteil und Wert"; §8.5 (NAK-313.md:1943-1947): A11 „prüft sie gegen die Definition ihres Nachrichtentyps im v2-Vertrag … und vergleicht das Urteil".

**b) Nachmessung.**
- `eq-copilot/schemas/eq-ipc.schema.json:144`: `"seq": { "type": "integer", "minimum": 0, "maximum": 9223372036854775807, "description": "zitiert die seq des beantworteten Heartbeats" },`.
- `eq-copilot/plugin/src/PipeClient.cpp:600-611`: `const auto ack = juce::JSON::parse (ackJson);` … `|| ! ganzzahl64 (ack.getProperty ("seq", {}), ackSeq)` / `|| ackSeq != aktuelleSeq` … `zustand.letzterFehler = "ungueltiges heartbeat_ack";`. `9223372036854775807` ist `INT64_MAX`, JUCE liest ihn ohne Überlauf; `ackSeq` ≠ 0 bzw. 1 → ungültig; das Schema sagt gültig.

Reproduktion stimmt. Ergänzend die Gegenrichtung (heute kein Tabelleneintrag): `PipeClient.cpp:516-520` nimmt `min_protocol`/`max_protocol` über `ganzzahl64` ohne Bereich an, der Vertrag begrenzt beide auf 0 bis 4294967295 (`eq-ipc.schema.json:71-72`) — der Produktleser kann auch lockerer urteilen als der Vertrag.

**c) Was bricht.** M-95-Zusage und §7.2 („A11 vergleicht Urteil") gegen gemessenen Code und Vertrag. Die Grenze im Kopf, NAK-313.md:18: „derselbe Vektor durch den echten C++-Leser, den echten Rust-Leser und das Python-Referenzbein mit demselben Urteil und, bei Ganzzahlen, demselben Wert" ist für einen zustandsabhängigen Eintrag nicht erfüllbar, solange die Tabelle nur ein Urteil kennt.

**d) Urteil.** BESTÄTIGT · **DEFEKT**. Kein Ausschluss: E-313-4 (NAK-313.md:2311-2314) entscheidet, dass A11 v2-Einträge prüft, nicht die Urteilsform.

**e) Schließende Regel.** Der Korpus trägt je Eintrag das Vertragsurteil für A5 und A11 getrennt vom Produkturteil seines Eingangs; jede Abweichung ist mit Grund ausgewiesen und gezählt. **Folgestellen:** M-93 (c), M-95, §7.1 Feldliste (NAK-313.md:1263-1278), §7.2 (NAK-313.md:1305-1307, :1311-1319), §8.5 A11-Absatz (NAK-313.md:1942-1947). Form: Teil 2 Punkt 3.

### D8 — M-109: Meldung entsteht nicht auf dem Fehlerweg

**a) Beanstandet.** NAK-313.md:1137 (M-109): Zustand „`reason` mit (a) `\u0000`, (b) einsamem `\ud800`, (c) `\q`, (d) `\u00` (kurz)" · Zusage „**Alle vier ungültig: „reject haelt den Vertrag nicht", kein Teilgrund**" · Rotbeweis „**Regressionswache** im Urteil (heute fällt jeder Backslash); einmal gebrochen: im Dekodierer `\u0000` zulassen → (a) gültig → rot". §8.6, NAK-313.md:2023-2024: „einsame Surrogate und `\u0000` sind ungültig (Regeln 5 und 6), jedes andere Escape ebenso".

**b) Nachmessung.**
- `eq-copilot/plugin/core/ipc/IpcVerbindung.cpp:450-451`: `if (c == '\\' || c < 0x20)` / `return false;`; `flachesJsonObjekt` gibt dann false (`:482-483`, `:496-497`).
- `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:649-653`: `if (! flachesJsonObjekt (text, felder) || ! jsonText (felder, "type", typ))` … `zustand.letzterFehler = "welcome: kein flaches JSON-Objekt";`.
- `Verbindung.cpp:655-661`: `if (typ == "reject")` … `rejectHaeltVertrag (felder, grund) ? "Broker lehnt ab: " + grund` / `: "reject haelt den Vertrag nicht";` — nur nach erfolgreichem Lesen.
- Telemetrie gleich: `eq-copilot/plugin/core/ipc/TelemetryClient.cpp:671-675` und `:677-684`.

Reproduktion stimmt: heute und nach §8.6 lautet die Meldung „welcome: kein flaches JSON-Objekt". Zusätzlich: das Etikett „Regressionswache im Urteil" deckt nur die Urteilshälfte; die Meldungshälfte ist heute rot und bliebe es (dieselbe Wurzel wie D1).

**c) Was bricht.** Zusage gegen gemessenen Fehlerweg und gegen §8.6; Etikettendefinition §6 (NAK-313.md:903-905) für die Meldungshälfte. R-313-7 Satz 2 (NAK-313.md:186-190) verlangt die Ablehnung, nicht diese Meldung.

**d) Urteil.** BESTÄTIGT · **DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** Jede Meldungserwartung ist über den tatsächlichen Aufrufer bis zur Zuweisung an `letzterFehler` verfolgt: eine Ablehnung beim flachen Lesen meldet den Lesegrund, „reject haelt den Vertrag nicht" bleibt dem Vertragsbruch nach erfolgreichem Lesen. **Folgestellen:** M-109, M-111, M-113, §8.6 Handshake-Absatz (NAK-313.md:2020-2036), Behauptung B10 (:2077-2079). Kleinste Änderung: Teil 2 Punkt 5.

### D9 — M-127: Rotvektor `x = y` wird vom Riegel nicht erkannt

**a) Beanstandet.** NAK-313.md:1170 (M-127): Zusage „**Beleg `binary_telemetry`: `rohfeld` nennt SONDE-010 A21, A22 und das Urteil T2 PASS vom 30.08.2026 ohne `feld = wert`-Form; …**" · Rotbeweis „… Nach dem Bau: im `rohfeld` ein `x = y` einfügen → A13 sucht es in den Rohdaten → rot".

**b) Nachmessung.** `tools/eq-copilot/pruefe_host_capabilities.py:201`: `token = re.compile(r"(?P<kopf>busse\[\d+\])(?=\s*\()|(?P<pfad>[A-Za-z_]+(?:\.[A-Za-z_]+|\[\d+\])*)\s*=\s*(?P<wert>true|false|[-\d.]+)")` — rechts vom `=` nur `true`, `false` oder `[-\d.]+`. `y` bildet kein Token; die Rohfeldsuche `:211-231` läuft nicht, kein `pruefe(False, …)`. Reproduktion stimmt. `x = 1` bildet das Token, `loese` scheitert (`:219-225`) und `:226-227` meldet `pruefe(False, f"{bit}: Rohfeld {pfad} (Praefix '{praefix}') existiert nicht in {name}")`; Randbedingung `:207`: Segmente, die mit „nicht gemessen" oder „Beweis entsteht" beginnen, werden übersprungen. Nebenbeobachtung: M-127 verlangt SONDE-010 A21/A22 und T2 PASS im `rohfeld`; §8.7 (NAK-313.md:2110-2114) legt diesen Text in `kanonbeweis` und schreibt ins `rohfeld` „Build-Tatsache, kein Hostmesswert; Beleg im Kanon (kanonbeweis)".

**c) Was bricht.** Matrixregel §6 (NAK-313.md:911-912): der vorgeschriebene Rotbeweis fällt nicht; die Zeile hat faktisch keinen.

**d) Urteil.** BESTÄTIGT · **DEFEKT**. Kein Ausschluss: der Sabotage-Ausschluss betrifft Härtungen am Riegel; hier bleibt der Riegel, falsch ist der Rotvektor.

**e) Schließende Regel.** Ein Rotvektor gegen A13 hat die Form, die der Riegel erkennt (`feld = true|false|Zahl` in einem nicht übersprungenen Segment). **Folgestellen:** M-127; Angleichung M-127 ↔ §8.7 (welcher Text in `rohfeld`, welcher in `kanonbeweis`).

### D10 — §7.2: ein Stufenfeld für Leser mit verschiedenen Ketten

**a) Beanstandet.** §7.1, NAK-313.md:1272: „`stufe` · bei `ungueltig` die Ablehnungsstufe, bei `gueltig` `null`" (Tabellenzeile); §7.2, NAK-313.md:1303-1305: „**A5** fährt jeden v3-Eintrag (strenger Lauf, Textriegel, Schema, Feldregel, wo der Vertrag sie nennt) als Python-Referenz"; NAK-313.md:1311-1317: „Jedes Bein liest die Tabelle, filtert nach `eingang`, fährt jeden Eintrag als eigenen Fall mit der `id` im Prüfnamen, vergleicht Urteil, Stufe, bei Ganzzahlen den Wert und die Wirkung, … Kann ein Bein einen Eintrag nicht fahren, ist er rot, nie übersprungen. Die Stufe misst jeder Produktleser an seiner eigenen Stelle; bei `v2` gilt die Stufe des C++-Clients, A11 vergleicht Urteil und Wert."; §7.3, NAK-313.md:1334-1335: „die Negativen `1.5` (Stufe `schema`) und `9007199254740992` (Stufe `textriegel`, am flachen ACK-Leser `feldregel`)".

**b) Nachmessung.**
- A5-Kette: `tools/eq-copilot/pruefe_v3_vertrag.py:1845` `grund = textriegel_bytes(roh_bytes)` → `:1855` `daten = json_laden_strikt(roh_bytes.decode("utf-8"))` → `:1862` `gueltig = pruefer.is_valid(daten)`; 2^53 fällt am Riegel, `:182-184` „`Ganzzahl ausserhalb 2^53-1`".
- Flacher ACK-Leser, `eq-copilot/plugin/core/ipc/controlclient/Intern.h:211-217`: `if (! flachesJsonObjekt (text, felder)` … `|| ! jsonLiteral (felder, "state_revision", revision)` / `|| ! nichtnegativeJsonGanzzahl (revision))` / `return CommandAckArt::keinAck;` — kein Textriegel, kein Schema; `git grep -i textriegel` in `core/ipc/` findet nur Kommentare.

Reproduktion stimmt. **Genauer: der Widerspruch ist systematisch.**
- Die flachen Leser (Control, Telemetrie, ACK) kennen nur `parser` (`flachesJsonObjekt`) und `feldregel` (`welcomeHaeltVertrag`, `rejectHaeltVertrag`, `commandAckArtLesen`). A5 lehnt dieselben Bytes oft früher ab: `\u0000` (`pruefe_v3_vertrag.py:300-301`), einsames `\ud800` (`:321-322`), `\u00` (`:297-298`), `C3 28` (`:265-268`) am Textriegel, am flachen Leser am Parser (M-109 a, b, d; M-111); `broker_version` 65 × é und leer, `reason` 501 × é, `protocol` `4` und `3.5` am Schema, am flachen Leser an der Feldregel (M-106 b, M-110, M-107 b, M-60).
- Das Quellenmodell hat Textriegel (`SourcesModel.cpp:727-732`) und JUCE (`:733-738`), aber kein Schema (§1, NAK-313.md:65-66: „im C++-Produkt läuft die Schemaengine nirgends"): `1.5` und `-1` (M-61) fallen dort an der Feldregel, in A5 am Schema.
- Der Rust-Bootstrap hat bis Etappe 6 weder Textriegel noch Schema (D12).

**c) Was bricht.** §7.2 „vergleicht Urteil, Stufe" mit einem Stufenfeld (§7.1) gegen §7.3 (zwei Stufen für einen Eintrag) und gegen die gemessenen Leserketten; mit §7.2 „Kann ein Bein einen Eintrag nicht fahren, ist er rot" wird für diese Einträge A5 oder der Produktleser zwangsläufig rot.

**d) Urteil.** PRÄZISIERT · **DEFEKT** — nicht nur der ACK-Eintrag, sondern die Mehrzahl der Einträge an flachen C++-Lesern und alle Schemastufen-Einträge am Quellenmodell. Kein Ausschluss.

**e) Schließende Regel.** Der Korpus führt die Ablehnungsstufe je Leser (Vertragsreferenz und Produktleser des Eingangs getrennt), und jeder Leser erwartet nur Stufen, die seine Kette am Ende der Etappe besitzt. **Folgestellen:** §7.1, §7.2, §7.3 (Etappe 5), M-60, M-61, M-73, M-96, M-106, M-107, M-109, M-110, M-111, M-113; dazu M-49 als Kreuzzeile mit einer gemeinsamen Stufe (für ihre Etappe-4-Einträge hält die Annahme, weil dort alle Leser die Parserstufe haben). Form: Teil 2 Punkt 3.

### D11 — §7.3: v2-Parservektoren in Etappe 4, A11-Tabellenleser erst in Etappe 5

**a) Beanstandet.** §7.3, NAK-313.md:1323-1326: „**Etappe 4:** Datei, Erzeuger, Register und A8; die Einträge der Stufen `parser` und `duplikat` für `cpp_sources_snapshot`, `cpp_sources_ruecknahme`, `cpp_v2_client`, `rust_p0`, `rust_p1` und `rust_bootstrap` (M-39 bis M-46, M-49)"; §7.2, NAK-313.md:1305-1307: „**A11** fährt jeden v2-Eintrag gegen den v2-Vertrag, Zahlen als `decimal.Decimal` gelesen"; §8.4 Einzelbeine, NAK-313.md:1806: „B13, A4b, B3c, B16, A4, A5, A8" (kein A11); §8.5, NAK-313.md:1942-1944: „`pruefe_v2_schemas.py` liest die v2-Einträge der Tabelle, lädt ihre Bytes mit `json.loads (…, parse_float=decimal.Decimal)`" (kein Hook).

**b) Nachmessung.** `tools/eq-copilot/pruefe_v2_schemas.py:13-20` prüft nur Dateien („1. Jede Datei ist gueltiges JSON (UTF-8). 2. Jede Datei ist ein gueltiges JSON Schema … 3. `$schema` und `$id` sind vorhanden"), `:74` `schema = json.loads(roh.decode("utf-8"))` — kein Instanzleser, kein `object_pairs_hook`. Ohne Hook behält `json.loads` bei doppelten Namen den letzten Wert; ein v2-`welcome` mit doppeltem `type` oder `protocol_version` (M-42, NAK-313.md:1037) wäre für A11 schemagültig, genau der Fall, gegen den A5 in M-46 (NAK-313.md:1041) seinen Hook bekommt. Reproduktion stimmt.

**c) Was bricht.** §7.2 (NAK-313.md:1305-1306, :1314-1315): „A11 fährt jeden v2-Eintrag" und „Kann ein Bein einen Eintrag nicht fahren, ist er rot, nie übersprungen" — in Etappe 4 fährt A11 die v2-Einträge nicht; DEFEKT-Klasse des Prüfauftrags „der Bauplan §8 braucht in einer Etappe etwas, das erst eine spätere liefert"; R-313-6 (NAK-313.md:172) nennt für Python den `object_pairs_hook`; der Gate-Satz BEWEIS (NAK-313.md:6) nennt A11.

**d) Urteil.** BESTÄTIGT · **DEFEKT**. Kein Ausschluss (E-313-4 entscheidet das Ob von A11, dieser Befund Zeitpunkt und Hook).

**e) Schließende Regel.** Jedes Bein, das §7.2 einem Eintrag zuordnet, entsteht spätestens in der Etappe, die den Eintrag anlegt; A11 liest die Tabelle ab Etappe 4 mit Duplikat-Hook. **Folgestellen:** §8.4 (Ticketpfade, Einzelbeine, Behauptung A11), §8.5 A11-Absatz (Hook), M-42, M-95, E-313-4 (Zeitpunkt). Etappenbild: Teil 2 Punkt 4.

### D12 — §7.3: Bootstrap-Stufen `textriegel` und `schema` vor dem Torumbau

**a) Beanstandet.** §7.3, NAK-313.md:1331-1335: Etappe 5 „je Eingang Ganzzahleinträge in `.0`- und `e`-Form mit `wert` (… `rust_bootstrap`); die Negativen `1.5` (Stufe `schema`) und `9007199254740992` (Stufe `textriegel`, am flachen ACK-Leser `feldregel`)"; M-73 (NAK-313.md:1087): „**Dasselbe Urteil und bei Ganzzahlen derselbe Wert in C++ (flacher Leser, SourcesModel), Rust (P0, P1, Bootstrap) und Python (A5); `9007199254740992` fällt in allen am Textriegel beziehungsweise am Ganzzahlleser des flachen Lesers**"; §8.5 Orte (NAK-313.md:1884-1888): Bootstrap nur „`protocol` über den Helfer (`bootstrap.rs:242`); vor `from_value` ersetzt eine Normalisierung …"; §8.6 (NAK-313.md:1988-1998): Textriegel und Hello-Prüfung erst am Tor.

**b) Nachmessung.** `broker/src/transport/bootstrap.rs:219-289`: `std::str::from_utf8(…)` → `:220-221` `let wert: serde_json::Value = serde_json::from_str(roh)` → Protokollwahl `:226-250` → typisierte Übernahme `:254-255` bzw. `:276-277` (`serde_json::from_str(roh)`) → Feldprüfungen; kein Textriegel, kein Schema. In Etappe 5 (Stand nach §8.4 und §8.5) fällt `9007199254740992` im Helfer oder in der typisierten Übernahme, `1.5` in `from_value::<u32>`; die Stufen `textriegel` und `schema` gibt es dort nicht. Reproduktion stimmt.

**c) Was bricht.** §7.2 Stufenvergleich; DEFEKT-Klasse „braucht in einer Etappe etwas, das erst eine spätere liefert"; die M-73-Zusage für den Bootstrap ist in Etappe 5 nicht erfüllbar.

**d) Urteil.** BESTÄTIGT · **DEFEKT**. Kein Ausschluss (Reihenfolge am Tor ist Vertragsarbeit, Prüfauftrag Ausschluss 3).

**e) Schließende Regel.** Ein Eintrag entsteht erst in der Etappe, in der sein Produktleser die erwartete Stufe besitzt; Bootstrap-Einträge der Stufen `textriegel` und `schema` entstehen mit dem Torumbau in Etappe 6. **Folgestellen:** §7.3 Etappe 5 und 6, M-72 (Gegenfall `3.5`), M-73, M-96 (`NaN`, `Infinity`, `1e999` am Bootstrap fallen bis Etappe 6 am Parser). Empfehlung: Teil 2 Punkt 4.

### D13 — §8.6: UTF-8-Grund geht in der booleschen Funktion verloren

**a) Beanstandet.** M-111, NAK-313.md:1139: Zusage „**Beide ungültig mit UTF-8-Grund, bevor `flachesJsonObjekt` läuft**" · Reihenfolge „UTF-8-Prüfung des Payloads vor `flachesJsonObjekt` (`eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:646-653`)"; §8.6, NAK-313.md:2025-2029: „`flachesJsonObjekt` prüft vor dem ersten Zeichen die UTF-8-Gültigkeit des ganzen Texts mit `nakama::utf8::istGueltig` (…) das deckt Control-Welcome, Telemetrie-Welcome und den ACK-Weg."

**b) Nachmessung.** `eq-copilot/plugin/core/ipc/IpcVerbindung.h:263` / `IpcVerbindung.cpp:462`: `bool flachesJsonObjekt (const std::string& text, std::vector<JsonFeld>& felder)` — kein Grund-Ausgang. Aufrufer: `Verbindung.cpp:652` und `TelemetryClient.cpp:674` setzen „`welcome: kein flaches JSON-Objekt`", `Intern.h:211` (ACK) liefert `keinAck` ohne Meldung. In `core/ipc/` gibt es heute keinen UTF-8-Aufruf (`git grep "istGueltig\|utf8::"` leer) — M-111 „heute rot" stimmt. Prüfer `eq-copilot/plugin/vertrag/NakamaUtf8.h:11`: `inline bool istGueltig (const void* daten, size_t laenge) noexcept`. Reproduktion stimmt; M-111 („bevor … läuft") und §8.6 („in der Funktion vor dem ersten Zeichen") widersprechen sich zusätzlich im Ort.

**c) Was bricht.** M-111 gegen §8.6 und gegen den Fehlerweg. R-313-7 (NAK-313.md:186) „C++-Handshake: UTF-8-Prüfung vor dem flachen Lesen" wäre mit §8.6 erfüllt, die Grundmeldung der Zeile nicht.

**d) Urteil.** BESTÄTIGT · **DEFEKT**. Kein Ausschluss.

**e) Schließende Regel.** Die UTF-8-Ablehnung des flachen Lesers trägt ihren Grund bis in `letzterFehler`, und M-111 nennt den Ort, den §8.6 baut. **Folgestellen:** M-111, M-113, M-109 (dieselbe Wurzel), §8.6 Handshake-Absatz. Kleinste Änderung: Teil 2 Punkt 5.

### D14 — §8.6: Rust-Slotriegel über den gespeicherten Relativoffset statt über die VTable

**a) Beanstandet.** §8.6, NAK-313.md:2038-2046: „C++: `pruefeAdresse` liest nach der Verifikation die VTable der Adresstabelle (`flatbuffers::Table::GetVTable`, deren Größe) und meldet einmal `(…/quelle, adresse_zusatzfeld)`, sobald ein Slot mit Feld-ID ≥ 5 einen Offset ≠ 0 trägt (`GetOptionalFieldOffset`). Rust: `pruefe_adresse` prüft dieselben Slots mit `offset_nicht_null` und meldet denselben Namen; … die Regel hängt nicht an der Wahl der Funktion." R-313-8, NAK-313.md:196-198: „… dass die Adress-VTable jenseits von Feld-ID 4 keinen belegten Slot trägt (Offset ≠ 0; ein längerer Eintrag mit 0 bleibt zulässig)".

**b) Nachmessung.** `broker/src/telemetrie.rs:230-241`: `fn offset_nicht_null(tab: &::flatbuffers::Table, slot: ::flatbuffers::VOffsetT) -> bool {` / `let vo = tab.vtable().get(slot);` / `if vo == 0 {` / `return true; // Feld nicht vorhanden — das ist kein Selbstbezug.` … `u32::from_le_bytes([buf[pos], buf[pos + 1], buf[pos + 2], buf[pos + 3]]) != 0` — er prüft den gespeicherten Relativoffset eines vorhandenen Offsetfelds auf Selbstbezug (Zweck `:243-246`, Strukturriegel). Für einen fehlenden Slot 5 und für einen belegten `zusatz`-String liefert er beide Male `true`; M-114/M-115 und M-118 wären damit nicht unterscheidbar. Die C++-Anweisung misst richtig: `eq-copilot/build/_deps/flatbuffers-src/include/flatbuffers/table.h:36-44` (FlatBuffers 25.12.19, `base.h:142-144`) `voffset_t GetOptionalFieldOffset(voffset_t field) const {` … `return field < vtsize ? ReadScalar<voffset_t>(vtable + field) : 0;`. Reproduktion stimmt für die Rust-Hälfte.

**c) Was bricht.** Der Rust-Satz in §8.6 widerspricht R-313-8 („Offset ≠ 0" des VTable-Slots) und der gemessenen Semantik des Helfers; mit ihm bliebe der Zusatzslot in Rust unerkannt — `CLAUDE.md:180-181`: „Für v3 sind Discriminator, Zieladresse, Revision und Capability nicht additiv erweiterbar."

**d) Urteil.** BESTÄTIGT · **DEFEKT** (nur die Rust-Anweisung). Kein Ausschluss: Zieladresse ist Vertrag, keine Sicherheitsneubewertung.

**e) Schließende Regel.** Beide Leser prüfen die Belegung direkt an der VTable (je Slot ab Feld-ID 5 innerhalb der VTable-Länge: Slot-Eintrag ≠ 0), nie über den Inhalt des Feldes. **Folgestellen:** §8.6 Slotriegel und Fixture-Absatz (NAK-313.md:2048-2057), M-115, M-116, M-118, M-119. Weg und Fixtures: Teil 2 Punkt 6.

---

## Teil 2 — Entscheidungsgrundlagen

### 1. D5: `host_mixer_index` 3.0 heute und die Zusage von M-65

**Quelle.** Vergleich `liveness.rs:442-446` (Zitat in D5 b) und `:447-461` (`alt_kanal` aus dem gespeicherten, `neu_kanal` aus dem neuen Deskriptor, `kanalwechsel` bei Ungleichheit); Übernahme `:752-756` (`objekt.insert(feld.into(), wert.clone());`); Speicherung `:471-472`; Folge `:513-520` (`invalidierung_wegen_kanalwechsel` nach dem Lock) mit `broker/src/coordinator/invalidierung_verdrahtung.rs:517-537`: `if alt == neu {` / `return Ok(0);` … alle nicht ausgeschlossenen `evidence_id` der Quelle mit `grund: Grund::MesspunktWechsel`.

**Wirkung eines Heartbeats mit `3.0` heute:**
- (a) Vergleich: `as_i64` auf `3.0` ist `None`, das Feld gilt als nicht geliefert. Stand vorher `3` → `Some(3)` ≠ `None` → Kanalwechsel → alle gültigen Evidenzen der Quelle werden invalidiert; der nächste Heartbeat mit `3` löst von `None` nach `Some(3)` erneut aus. Ohne vorherigen Kanal keine Reaktion.
- (b) Gespeicherter Deskriptor: trägt `3.0` (Rohwert, `:752-756`), nicht „nichts".
- (c) Gen: der Snapshot trägt `"host_mixer_index":3.0` (`sicht.rs:545-557`, zmij `lib.rs:1624`); `nichtnegativeGanzzahl` lehnt ab (`SourcesModel.cpp:101-117`, `:310-314`), und Gen verwirft den **ganzen** `session_snapshot` (`:1302-1309`). Zusätzlich gruppiert `hypothese_verdrahtung.rs:296-305` die Quelle nicht nach Kanal.

**Welche Zeile misst R-313-5 dort deterministisch.** Der Ganzzahlhelfer an `:442-446` trägt allein die **Kanalwechselreaktion**. Deterministische Zeile (Form, kein Matrixtext): Quelle mit vorhandener, nicht ausgeschlossener Evidenz; Heartbeat mit `3`, danach mit `3.0` → keine Invalidierung (Ausschlussgründe und Invalidierungszähler unverändert). Heute rot (Invalidierung); nach dem Bau `as_i64` in `kanal_von` zurück → Invalidierung → rot. Der **Deskriptorwert** ist nur dann eine Zusage, wenn der Bau die Ganzzahl schon beim Übernehmen schreibt; der **Snapshotwert** an Gen ist die Zusage des C++-Lesers (Etappe 5, `wertAlsVar`, M-61) und braucht dort einen eigenen `cpp_sources_snapshot`-Eintrag mit `host_mixer_index` `3.0`.

**Vorlage für den Dirigenten (zwei Wege, gleiche Produktwirkung für vertragstreue Sender):**
- Weg H (Helfer je Leser): Helfer in `liveness.rs:442-446`, `:859-863` und `hypothese_verdrahtung.rs:296-305` (neuer Ticketpfad); der Deskriptor behält `3.0`, Gen wird in Etappe 5 über `wertAlsVar` heil. Drei Rust-Stellen, drei Zeilen.
- Weg N (Normalisierung beim Übernehmen): `liveness.rs:752-756` setzt den Helferwert statt `wert.clone()`. Eine Stelle; alle Leser danach sehen `3`, der Snapshot an Gen trägt `3`, `hypothese_verdrahtung.rs` bleibt außerhalb der Ticketpfade; eine Mutation (`wert.clone()` zurück) macht Deskriptorwert und Kanalwechselreaktion zugleich rot. `descriptor_setzen` (`:804` ff.) hat nach `:852-858` keinen Produktaufrufer und bliebe roh.
- Empfehlung: **Weg N** plus die Zeile zur Kanalwechselreaktion als Zusage von M-65 und der Gen-Eintrag in Etappe 5. Er deckt „an allen Produktstellen" mit einer Änderung, statt jede lesende Stelle einzeln zu verriegeln.

### 2. D6: welches Bein den echten Intentwriter aufruft

**Quelle.** `CMakeLists.txt:977-983` (EqCopIpcTest: `tests/IpcTestMain.cpp`, `core/ipc/PipeToken.cpp`, `nakama_kern_anbinden`), `:198-220` und `:240` (`add_library(NakamaKern STATIC ${NAKAMA_KERN_QUELLEN})`, ohne `src/prozessor/`), `:277-283` (`nakama_kern_anbinden` bindet nur Kern und JUCE-Module), `:888-897` und `:912` (EqCopSonde014IntentTest mit `${NAKAMA_PROZESSOR_QUELLEN}`), `:111-117` (`src/prozessor/Ipc.cpp`). Writer `Ipc.cpp:548-587`; er liefert nur für `main` mit gültiger Adresse Text (`:555-556` `if (h.pluginKind != "main" || ! nakama::ipc::adresseGueltig (h.adresse))` / `return {};`), die Adresse ist Laufzeit (`:465-472`: `v3LogonSid`, `v3SessionEpoch`, `alsHex32 (instanceNonce, "runtime_nonce")`). Zugang `PluginProcessor.h:366-367`.

**Antwort.**
- Bein: **B27** (`EqCopSonde014IntentTest`, `tests/Sonde014IntentTest.cpp`), das die Prozessorquellen übersetzt; Zugang `v3IntentUpdateFuerTest (true)`. B10 kann den Writer nicht linken.
- Weg zum Rust-Leser ohne ungeprüftes Hand-JSON: Byteinstanz nach dem Muster `eq-copilot/fixtures/v3/heartbeat-wire-v1.json` — von Hand geschrieben und damit Ausgabe keiner Implementierung (`:2-27`, `_kommentar`: „Stimmt der C++-Serialisierer mit ihr ueberein und der Rust-Leser nimmt sie an, stimmen beide transitiv miteinander ueberein."), mit `eingabe` (`:29-42`) und `faelle.*.wire` (`:43-59`); C++-Seite `MarkierungTestMain.cpp:1092-1116`, Rust-Seite `broker/tests/sonde013_verdrahtung.rs:4600-4637` (`heartbeat_wire_instanz`, `hello_aus_wire_instanz`). Für M-89: `eq-copilot/fixtures/v3/intent-wire-v1.json` (Zielpfad) mit `eingabe` (Bestand mit `intentBestandRevision` 9007199254740991, Adresse) und `wire` (genau der Writertext). B27 stellt den Bestand über die Produkt-API her, ruft den Zugang und vergleicht bytegleich — wegen der Laufzeitadresse mit ersetztem `adresse`-Objekt oder als Ausschnitt ohne Adresse (Muster B16, NAK-313.md:1790-1793) oder über einen Adresszugang im Testbau. A4 koppelt einen Link mit der Adresse aus `eingabe`, speist `wire` in die P1-Weiche und erwartet `bestand_revision` 9007199254740991 übernommen.
- §7.2: kein neuer Produkteingang (der Writer ist kein Leser); wahlweise derselbe `wire`-Text zusätzlich als `rust_p1`-Eintrag (`gueltig`, `feld` `/bestand_revision`, `wert` 9007199254740991), dann fährt ihn auch A5.
- §8.5: Ticketpfade um die Byteinstanz ergänzen; Testzelle M-89 „C++: B27"; Behauptung B27 („der Intentwriter schreibt `bestand_revision` 2^53−1 bytegleich zur Byteinstanz"); §8.1-Ausnahmeliste der Handinstanzen um die neue Datei erweitern (`-text` greift über `.gitattributes`, NAK-313.md:1243-1244).

### 3. D7 und D10: Vertragsurteil gegen Produkturteil, Stufe je Leser

**Quelle.** Vertrag v2 `eq-ipc.schema.json:144` (Zitat D7 b). Sequenzprüfung `PipeClient.cpp:606` `|| ackSeq != aktuelleSeq` (Kontext `:600-611`). Referenzbein `pruefe_v3_vertrag.py:1845` (Textriegel) → `:1855` (strenger Parser) → `:1862` (Schema); Riegelgrenze `:182-184`. Textriegel-Regel 2, `eq-copilot/schemas/v3/README.md:62-66`: „**Parser-sichere Dezimalpraezision:** mathematische Ganzzahlen nur innerhalb ±(2^53−1), unabhaengig davon, ob sie als `5`, `5.0`, `5e0` oder mit verschobenen Endnullen geschrieben sind."

**Einträge, die verschieden urteilen müssen.**
- Urteil verschieden (zustandsabhängige Feldregel): M-93 (c) — Vertrag gültig, Client ungültig (`aktuelleSeq`). Andere zustandsabhängige Produktregeln (ACK-`command_id`, erwartete Bindung und Session im Quellenmodell, Link-Adresse in Rust P0, `link_id`/`challenge` am Telemetrie-Welcome `TelemetryClient.cpp:690-691`) lassen sich über die Einspeisung (§7.2) gleichziehen und urteilen dann gleich; dass der Testserver `link_id` und `challenge` passend zu den Tabellenbytes vergeben kann, ist am Testserver nicht nachgeprüft.
- Stufe verschieden bei gleichem Urteil: ACK 2^53 (A5 `textriegel`, flacher Leser `feldregel`); ACK/Welcome `1.5`, `3.5`, `4` (A5 `schema`, flach `feldregel`); Handshake `\u0000`, `\ud800`, `\u00`, `C3 28`/`FF` (A5 `textriegel`, flach `parser`); `broker_version` 65 × é und leer, `reason` 501 × é (A5 `schema`, flach `feldregel`); Quellenmodell `1.5`, `-1` (A5 `schema`, Modell `feldregel`); `NaN`, `Infinity`, `1e999` (A5 `textriegel`, flach `parser`/`feldregel`, Bootstrap vor Etappe 6 `parser`); v2 M-91 (a) (A11 `schema` über `enum`, Client `feldregel`; A11 ist vom Stufenvergleich ausgenommen).

**Formen.**

| Form | Erzeuger | Beine in drei Sprachen | Zählprüfung | Gleichheitszusage vertragsfester Einträge | löst D7 | löst D10 |
|---|---|---|---|---|---|---|
| (A) je Eintrag `vertrag {urteil, stufe}` und `produkt {urteil, stufe}` des Eingangs, dazu `abweichung` (Pflicht, wenn die Urteile verschieden sind) | zwei Paare je Eintrag, meist gleiche Urteile; Selbstprüfung: Urteile gleich, außer mit `abweichung` | B10, B13, A4b, A4 lesen `produkt`; A5, A11, B16 lesen `vertrag` | unverändert je `eingang` und je `fassung`; neu: Zahl der Abweichungen = Zahl der `abweichung`-Einträge | bleibt und wird prüfbar: ohne `abweichung` erzwingt der Erzeuger dasselbe Urteil, alle Beine lesen dieselben Bytes | ja | ja |
| (B) ein `urteil`, `stufen` als Abbildung Leserklasse → Stufe | ein Urteil, eine Stufenabbildung je Eintrag | jedes Bein wählt seine Klasse | unverändert | bleibt, aber nur, weil ein zustandsabhängiger Eintrag nicht darstellbar ist | nein (M-93 c braucht zusätzlich C) | ja |
| (C) zustandsabhängige Einträge aus dem Korpus, nur als Modultest im Bein | ein Eintrag weniger | unverändert | unverändert | bleibt für den Rest | ja, durch Herausnahme | nein |

**Empfehlung: (A).** D10 verlangt ohnehin eine Stufe je Leser für die Mehrzahl der C++-Einträge; ist das zweite Feldpaar da, kostet das getrennte Urteil fast nichts. (A) behält M-93 (c) als ausgewiesene, gezählte Abweichung im Korpus, hält die Kopfgrenze „derselbe Vektor … mit demselben Urteil" (NAK-313.md:18) für alle vertragsfesten Einträge durch eine Erzeugerprüfung ein und ersetzt in §7.2 den Satz „bei `v2` gilt die Stufe des C++-Clients, A11 vergleicht Urteil und Wert" durch „Produktbeine vergleichen `produkt`, Referenzbeine `vertrag`". (B) allein lässt D7 offen, (C) allein D10.

### 4. D11 und D12: Etappenzuordnung

**Quelle.** §7.3 (NAK-313.md:1321-1348, Zitate in D11 a und D12 a). §8.4 Ticketpfade (NAK-313.md:1704-1726) ohne `pruefe_v2_schemas.py`; Bootstrap in Etappe 4 (NAK-313.md:1753-1756): „`bootstrap_lesen` ersetzt `from_str::<Value>` (`:220-221`) durch `json_streng` und liest die typisierten Hellos mit `serde_json::from_value (wert.clone())`". §8.5 (NAK-313.md:1835-1836 „`tools/eq-copilot/pruefe_v2_schemas.py` (A-4)"; :1884-1888 Bootstrap-Normalisierung; :1942-1947 A11 ohne Hook). §8.6 (NAK-313.md:1967-1968 „`broker/src/coordinator/schema.rs` (Zugriff des Tors auf das v3-Schema)"; :1988-2002 Reihenfolge am Tor).

**Abhängigkeit im Code.**
- Der Bootstrap hat heute weder Textriegel noch Schema (`bootstrap.rs:219-289`).
- Beide Bausteine liegen schon im Crate: `broker/src/vertrag.rs:340` `pub fn textriegel_bytes(roh: &[u8]) -> Result<(), String> {`, `:560` `pub fn laden(wurzel: Value) -> Result<Schema, String> {`, `:578` `pub fn gueltig(&self, daten: &Value) -> bool {`; die Wurzel des v3-Schemas deckt das Hello (`eq-ipc-v3.schema.json:10` `{ "$ref": "#/$defs/hello" },`, `:838-845` mit `hello_control` und `hello_telemetry`). Nur `v3_schema` ist modulprivat: `broker/src/coordinator/schema.rs:377` `pub(super) fn v3_schema(schema_minor: u8) -> Option<&'static crate::vertrag::Schema> {`.
- Der Torumbau hängt damit am einen strengen Lauf aus Etappe 4, nicht an Etappe 5.
- `pruefe_v2_schemas.py` hängt weder an C++ noch an Rust (liest heute nur die Schemadateien, `:59-98`); ein Tabellenleser braucht nur die Tabelle aus Etappe 4 und Pythons `json.loads (…, object_pairs_hook=…, parse_float=decimal.Decimal)`.

**Fahrbar je Etappe** (A11-Tabellenleser mit Hook in Etappe 4, Bootstrap-Einträge `textriegel`/`schema` in Etappe 6):

| Etappe | Eingänge und Einträge | Beine |
|---|---|---|
| 4 | `cpp_sources_snapshot`, `cpp_sources_ruecknahme`: `parser`, `duplikat`, Gegenfall zwei Objekte · `cpp_v2_client`: `parser`, `duplikat` · `rust_p0`, `rust_p1`, `rust_bootstrap`: `parser`, `duplikat` (Bootstrap mit `json_streng`; diese Stufen bleiben nach Etappe 6 gleich, weil die Einträge den Textriegel passieren) · Evidenz lokal (gültig, `null` am Schema) | B13; A4b und A11 (Hook); A4; A5 für alle v3; B16 |
| 5 | Ganzzahl-Positive (`.0`, `e`, `wert`) aller Eingänge einschließlich `rust_bootstrap` (Helfer und Normalisierung) · Negative `1.5`, 2^53 für `cpp_*`, `rust_p0`, `rust_p1` mit Stufe je Leser (Form A) · v2-Zahleinträge mit `wert` und `ub_bei_juce` · Bootstrap-Negative nur als Modultest ohne Stufe (`KeinHello`) | B10, B13, A4b, A4, A5, A11 (Decimal) |
| 6 | `rust_bootstrap`: Tor-Einträge (M-98 bis M-102, M-104) und die aus Etappe 5 verschobenen Negative (`1.5` und `3.5` → `schema`, 2^53, `NaN`, `1e999` → `textriegel`) · Handshake-Einträge beider flachen Leser | A4, B10, A5 |
| 7 | `rust_p0`: zwei gültige Heartbeats mit `binary_telemetry` | A4, A5 |

**Empfehlung: Vektoren verschieben, Tor nicht vorziehen.** Technisch ließe sich der Torumbau nach Etappe 4 oder 5 ziehen (alle Bausteine sind da, siehe oben). Er würde aber `bootstrap_lesen` in drei aufeinanderfolgenden Etappen umbauen — genau das Risiko, das der Prüfauftrag benennt („bootstrap.rs zwischen Etappe 4, 5 und 6") — und die Torarbeit mit ihrer eigenen Prüfregel (Kopf NAK-313.md:11: „reguliert Astra dort, prüft `gpt-5.6-sol` max diese eine Etappe") sowie die Übergabe an NAK-310 aus Etappe 6 herauslösen. Das Verschieben der Bootstrap-Einträge ändert nur §7.3, M-72, M-73 und M-96. Der A11-Tabellenleser gehört nach Etappe 4, weil er dort seine ersten Einträge bekommt und keine Codeabhängigkeit ihn später zwingt; Etappe 5 ergänzt nur `Decimal`, `wert` und die Zahleinträge.

### 5. D8 und D13: Fehlerweg im Handshake

**Quelle.** `Verbindung.cpp:646-669`: Lesen (`:649`), Meldung „`welcome: kein flaches JSON-Objekt`" (`:652`), `reject` mit „`Broker lehnt ab: `" + Grund oder „`reject haelt den Vertrag nicht`" (`:655-661`), sonst „`unerwartete Antwort auf hello`" (`:664-667`). `TelemetryClient.cpp:668-695`: dieselben drei Meldungen (`:674`, `:681-683`), am Ende „`unerwartete Antwort auf das Telemetry-Hello`" (`:690-694`). `letzterFehler` entsteht jeweils in diesen Zuweisungen unter `zustandMutex`. `IpcVerbindung.cpp:439-459` (`leseString`: Backslash und Steuerzeichen → false) und `:462-537` (`flachesJsonObjekt`, bool ohne Grund). Weitere Aufrufer: `Intern.h:211` (ACK), Tests `IpcTestMain.cpp:5751-5786`, `IpcLastMain.cpp:127`, `SessionSoakMain.cpp:756`.

**Kleinste Änderung.** `flachesJsonObjekt` bekommt einen optionalen Grund-Ausgang (`std::string* grund = nullptr`) und füllt ihn nur beim Scheitern: kein gültiges UTF-8 (Prüfung vor dem ersten Zeichen, wie §8.6), NUL-Escape, einsames Surrogat, unbekanntes Escape, kein vierstelliges `\u`-Escape, rohes Steuerzeichen, doppelter Name, Syntax. Nur die beiden welcome-Aufrufer (`Verbindung.cpp:649-653`, `TelemetryClient.cpp:671-675`) reichen ihn weiter und schreiben „`welcome: `" + Grund in `letzterFehler`. `commandAckArtLesen` und alle Testaufrufer übergeben nichts. Annahme- und Ablehnungsentscheid der Funktion bleiben gleich, gültige ACKs sind also unberührt; der ACK-Weg bleibt `keinAck` ohne Meldung. Die Alternative (getrennte `istGueltig`-Prüfung vor jedem der drei Aufrufer) braucht drei Stellen und liefert die Escape-Gründe trotzdem nicht.

**Erwartungen danach.**
- M-109: (a) bis (d) ungültig; `letzterFehler` = „`welcome: `" + Lesegrund (NUL-Escape; einsames Surrogat; unbekanntes Escape; kein vierstelliges `\u`-Escape); nie „`Broker lehnt ab: …`", kein Teilgrund. Etikett: Urteil Regressionswache, Meldung heute nicht messbar (heute „welcome: kein flaches JSON-Objekt").
- M-111: beide ungültig, `letzterFehler` = „`welcome: `" + UTF-8-Grund am Control-Leser, am Telemetrie-Leser dasselbe (M-113); der Ort heißt „am Anfang von `flachesJsonObjekt`", nicht „vor `flachesJsonObjekt`".
- „`reject haelt den Vertrag nicht`" bleibt den Vertragsbrüchen nach erfolgreichem Lesen (M-107 b); „`unerwartete Antwort auf hello`" bleibt M-106 b und M-110 (`welcome`).

### 6. D14: Belegung direkt an der VTable

**Quelle.** Rust: `telemetrie.rs:230-241` (`offset_nicht_null`, Zitat D14 b); `:247-259` (`strukturriegel` nutzt ihn für die 16 Offsetfelder); `:330-363` (`pruefe`: Dateikennung, dann `fb::root_as_feature_batch` `:350` → `verifier`, dann `strukturriegel` `:360`); `:386-387` (`let a = eintrag.quelle();` / `pruefe_adresse(&a, &format!("{p}/quelle"), &mut out);`); `:553-570` (`pruefe_adresse`, heute SID-Länge und hex32); `:408-419` (`fuer_broker` ruft `pruefe`). C++: `eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp:159-183` (`pruefeAdresse`), `:617-621` (`flatbuffers::Verifier pruefer (puffer, laenge);` / `if (! fb::VerifyFeatureBatchBuffer (pruefer))`), `:669-673` (Aufruf je Eintrag). Generiert: `eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h:225` `struct Adresse FLATBUFFERS_FINAL_CLASS : private ::flatbuffers::Table {`, `:228-234` (`VT_LOGON_SID = 4` bis `VT_RUNTIME_NONCE = 12`), `:250-264` (`Verify`: `VerifyTableStart`, fünf Pflichtfelder, `EndTable`); `broker/src/generiert/nakama_telemetry_v1_generated.rs:510-523` (`visit_table`, fünf `visit_field`, `finish`). Beide Verifier prüfen die VTable als Ganzes: C++ `verifier.h:185-199` (`VerifyTableStart`: Größenfeld, Ausrichtung, gerade Größe, `Verify(vtableo, vsize)`), Rust `flatbuffers-25.12.19/src/verifier.rs:371-378` (`deref_soffset`, `vtable_len`, `is_aligned`, `range_in_buffer`). Unbekannte Slots jenseits von Feld-ID 4 ignorieren beide.

**Direkte Prüfung, beide Sprachen, nach der Verifikation:**
- Feld-ID 5 liegt bei VTable-Byte 14 (4 + 2 × 5). Geprüft wird jeder Slot `s` = 14, 16, … mit `s + 2` ≤ VTable-Länge.
- C++: `GetVTable`/`GetOptionalFieldOffset` sind wegen der privaten Basis (`generated.h:225`) nur über einen Zeiger auf `flatbuffers::Table` erreichbar. VTable-Länge `ReadScalar<voffset_t>(GetVTable())`; belegt, wenn `GetOptionalFieldOffset (s) != 0` (`table.h:36-44`: außerhalb der VTable 0).
- Rust: `a._tab.vtable()` (`table.rs:47-51`), Länge `num_bytes()` (`vtable.rs:51-55`), belegt, wenn `get(s) != 0` (`vtable.rs:80-88`: `if byte_loc as usize + 2 > self.num_bytes() {` / `return 0;`). `_tab` ist öffentlich, `strukturriegel` nutzt es schon (`telemetrie.rs:253`). Genau das steht schon in der ersten Zeile von `offset_nicht_null` (`tab.vtable().get(slot)`); der Rest des Helfers ist für diese Regel falsch.
- Verstoß einmal je Eintrag: `(…/quelle, adresse_zusatzfeld)`; der Broker lehnt über `fuer_broker` ab, bevor Bytes weitergehen.

**Unterschied zum erlaubten Nullslot (M-118).** Die Länge allein ist kein Verstoß: eine VTable von 16 Byte mit Slot 14 = 0 ist gültig (`get` bzw. `GetOptionalFieldOffset` liefern 0). Verstoß ist nur ein Slot-Eintrag ≠ 0. Der Rotbeweis von M-118 („den Riegel auf die VTable-Länge prüfen lassen → ungültig") trifft genau diese Unterscheidung.

**Bytetransformationen.**
- Negativ (`ungueltig/adresse-zusatzfeld-id5.bin`): `.fbs` in einen Temporärordner kopieren, in `table Adresse` `zusatz:string (id: 5);` ergänzen, das JSON eines gültigen Batches mit `zusatz` im Eintrag 0 mit demselben gepinnten `flatc -b` übersetzen (Mechanik `erzeuge_fb_fixtures.py:1189-1206`). Die Adresse des Eintrags 0 bekommt eine VTable von 16 Byte mit Slot 14 ≠ 0 (Verweis auf den String); Einträge ohne `zusatz` behalten fünf Slots. Erwartete Menge `[("/eintraege/0/quelle", "adresse_zusatzfeld")]`. Den Stringinhalt prüft kein Verifier (unbekanntes Feld); der Riegel liest ihn nicht.
- Positiv (`gueltig/adresse-vtable-slot5-leer.bin`): an einen gültigen Puffer auf 2-Byte-Grenze eine Kopie der Adress-VTable des Eintrags 0 anhängen, deren Größenfeld um 2 erhöhen und einen Slot `0x0000` anhängen; das `soffset_t` am Anfang der Adresstabelle auf die Kopie umsetzen (`Tabellenposition − Kopieposition`, negativ zulässig: C++ `verifier.h:189-192` rechnet ohne Vorzeichen, Rust über `deref_soffset`). Die bekannten fünf Slots und die Objektgröße bleiben gleich; beide Verifier nehmen an. Dafür braucht der Erzeuger einen Weg für **gültige** Bytemutationen: `rohe_mutationen()` schreibt heute nur nach `ungueltig/` mit Urteil „ungueltig" (`erzeuge_fb_fixtures.py:1241-1246`).

### 7. D1 bis D4 und D9: Rotbeweise und Etiketten

- **D1 (M-09).** Neuer Aufrufnachweis in (a): Etikett **heute nicht messbar** (Zähler `testzugang::bindungsaufrufeFuerTest()` entsteht in Etappe 2; heute ein Aufruf über `PluginEditor.cpp:784` → `:816`); Mutation nach dem Bau: Unverändert-Prüfung in `uebernehmen()` entfernen → 1 Aufruf → rot. Bestehende Wache „keine Mutation, keine Dirty-Meldung": **Regressionswache**, heute getragen vom Frühausstieg `State.cpp:343-344` und gemessen von `312/M-37` (`ShotTestMain.cpp:480-485`, Marke, Dirty, Label). Nach dem Bau tragen sie zwei Sperren; ihr Einzelbruch liegt an der Stelle, die sie allein trägt: M-10 (Frühausstieg entfernen → 1 Host-Dirty und 1 Reconnect). Trennlinie im Bein: die Zählwerte `marke`/`dirty` aus `panelLauf` (`ShotTestMain.cpp:444-445`) bleiben die Wache, der neue Aufrufzähler ist der Nachweis.
- **D2 (M-11).** Etikett **heute rot** stimmt für beide Felder, sobald eine Paarrolle eingerichtet ist (heute nehmen beide Felder mehr an; erst `substring` kürzt, `PluginEditor.cpp:810-811`). Kleinster Einrichtungsweg: Rolle `pre` (id 3; `post`, id 4, gleichwertig) über denselben Zugang wie M-01 — Zustand `legacy`/`pre` mit leerem Label ohne `pair_id` über `nakama::state::speichere` erzeugen und mit `setStateInformation` laden. Das Laden ruft `setzeBindung` nicht und meldet kein Dirty (`eq-copilot/schemas/state/nakama-state-v2.md:315`: „**Laden und Migration melden nicht**"); zurückzusetzen ist nichts, die Zähler gelten ab dem Stand nach dem Laden. Wird die Rolle dagegen über `setzeBindung ("pre", "", "")` oder `rolleWahl.setSelectedId (3, juce::sendNotification)` gesetzt, entstehen je ein Aufruf, ein Reconnect und ein Host-Dirty; dann sind `bindungsaufrufeFuerTest`, `bindungsReconnectsFuerTest` und der `PanelDirty`-Zähler (`ShotTestMain.cpp:340-347`) vor dem Einfügen zurückzusetzen. Mutationen: `setInputRestrictions (120)` entfernen → 121; `setInputRestrictions (60)` entfernen → 61.
- **D3 (M-18).** Etikett **heute rot** stimmt (heute `geladen`; mit `pair_id` 65 537 → nächster Load `ignoriert`). Rotvektor, der den Kandidaten wirklich unter die Grenze bringt: Common mit 65 536 Einträgen samt `label` **und** `project_binding_id` (hex32). Der Leser nimmt die Bindung für `legacy` an (`NakamaState.cpp:1621-1626`), und der Stand ist über die Produkt-API erreichbar: `setzeBindung` behält eine Bindung beim Wechsel von `hub` zurück auf `sensor` (`State.cpp:332` kopiert den Zustand, `:340-342` erzeugt sie nur für `main`). Kandidat: nur `pair_id` kommt hinzu → 65 537 → `nurLesen`; Mutation `pair_id` aus dem Kandidaten (`:1236`) → 65 536 → `geladen` → rot. Alternative ohne neuen Vektor: Mutation am Riegelaufruf in `passt` wie M-17. Die Gesamtgrenze (262 144) setzt dieser Vektor so voraus wie M-22; nicht nachgerechnet.
- **D4 (M-34).** Etikett **heute rot** stimmt für (a) und (b) (heute keine Erkennung, beide `geladen`). Mutation, die genau den tiefen Fall aushebelt: Vorzählung im Riegel und Nachzählung in `lade` zählen beide nur die Ebenen 0 und 1 (Wurzel und ihre Kinder). Das Duplikat auf Ebene 2 fehlt auf beiden Seiten → gleiche Zahl → (a) `geladen` → rot. Eindeutige Stände bleiben auf beiden Seiten gleich → M-33 bleibt grün; (b) (Duplikat auf Ebene 1) bleibt erkannt.
- **D9 (M-127).** Etikett **heute rot** plausibel (die neue A13-Regel fällt gegen den heutigen Bericht). Rotvektor: `x = 1` (oder `x = true`) im `rohfeld` des Belegs `binary_telemetry`, in einem Segment, das nicht mit „nicht gemessen" oder „Beweis entsteht" beginnt → Token (`pruefe_host_capabilities.py:201`), Rohfeldsuche scheitert (`:219-225`), `:226-227` meldet „Rohfeld x … existiert nicht in B" → A13 rot.

---

## Übersicht

| Befund | Urteil | Einordnung | schließende Regel in einem Satz | Folgestellen |
|---|---|---|---|---|
| D1 M-09 | BESTÄTIGT | DEFEKT | Neuer Aufrufnachweis als Hälfte „heute nicht messbar", bestehende Dirty-/Mutationswache als Regressionswache mit Einzelbruch dort, wo sie nach dem Bau allein trägt. | nur M-09; Vorbild M-08, M-10, M-13 |
| D2 M-11 | BESTÄTIGT | DEFEKT | Panelzeilen mit Paarnamen stellen vorher eine Paarrolle über den Ladeweg von M-01 her und zählen ab dem Stand nach dem Laden. | M-11 (Paarmutation), §8.2 `panelLauf`, B15-Satz |
| D3 M-18 | BESTÄTIGT | DEFEKT | Eine Kandidatenmutation muss am Vektor nachgerechnet unter die Grenze führen, sonst sitzt der Rotbeweis am Riegelaufruf in `passt`. | nur M-18; M-19, M-21 nachgerechnet tragfähig |
| D4 M-34 | BESTÄTIGT | DEFEKT | Die Mutation beschneidet Vor- und Nachzählung symmetrisch, sodass nur das tiefe Duplikat unentdeckt bleibt. | M-34 (a), §8.3 Erkennungsabsatz |
| D5 M-65 | PRÄZISIERT | DEFEKT | M-65 misst die Kanalwechselreaktion am Helfer; jede weitere Stelle, die den gespeicherten Wert liest, bekommt den Helfer oder die Ganzzahl schon beim Übernehmen, mit Zeile. | `liveness.rs:859-863`, `hypothese_verdrahtung.rs:296-305`, Gen `SourcesModel.cpp:310-314` (M-61), §8.5 Orte, M-73 |
| D6 M-89 | BESTÄTIGT | DEFEKT | Writerzeilen nennen ein Bein, dessen Ziel die Writerquelle übersetzt, und führen die Ausgabe über eine Byteinstanz zum Rust-Leser. | M-89 (B27), §8.5, §8.1 Handinstanzen |
| D7 M-95 | BESTÄTIGT | DEFEKT | Vertragsurteil (A5/A11) und Produkturteil des Eingangs stehen getrennt, jede Abweichung ist begründet und gezählt. | M-93 (c), M-95, §7.1, §7.2, §8.5 A11 |
| D8 M-109 | BESTÄTIGT | DEFEKT | Meldungserwartungen sind bis zur Zuweisung an `letzterFehler` verfolgt; Leseablehnungen melden den Lesegrund. | M-109, M-111, M-113, §8.6 Handshake, B10-Satz |
| D9 M-127 | BESTÄTIGT | DEFEKT | Der Rotvektor hat die Form, die der Riegel erkennt (`feld = true`, `false` oder Zahl). | M-127; M-127 ↔ §8.7 (`rohfeld`/`kanonbeweis`) |
| D10 §7.2 | PRÄZISIERT | DEFEKT | Die Ablehnungsstufe steht je Leser, und jeder Leser erwartet nur Stufen seiner Kette am Etappenende. | §7.1, §7.2, §7.3, M-49, M-60, M-61, M-73, M-96, M-106, M-107, M-109 bis M-111, M-113 |
| D11 §7.3 | BESTÄTIGT | DEFEKT | Jedes zugeordnete Bein entsteht in der Etappe seiner Einträge; A11 liest ab Etappe 4 mit Duplikat-Hook. | §8.4, §8.5 A11-Absatz, M-42, M-95, E-313-4 (Zeitpunkt) |
| D12 §7.3 | BESTÄTIGT | DEFEKT | Einträge entstehen erst, wenn ihr Leser die Stufe hat; Bootstrap-`textriegel`/`schema` mit dem Torumbau in Etappe 6. | §7.3 Etappe 5 und 6, M-72, M-73, M-96 |
| D13 §8.6 | BESTÄTIGT | DEFEKT | Der UTF-8-Grund des flachen Lesers reicht bis `letzterFehler`, und M-111 nennt den Ort, den §8.6 baut. | M-111, M-113, M-109, §8.6 Handshake |
| D14 §8.6 | BESTÄTIGT | DEFEKT | Beide Leser prüfen je Slot ab Feld-ID 5 den VTable-Eintrag auf ≠ 0, nie den Feldinhalt. | §8.6 Slotriegel und Fixtures, M-115, M-116, M-118, M-119 (gültige Bytemutation im Erzeuger) |

---

## Gemeinsame Ursachen

1. **Rotmutation nicht bis zur Beobachtung nachgerechnet** — D3 (Kandidatenarithmetik), D4 (asymmetrische Zählung trifft M-33), D5 (Mutation am Vergleich, Beobachtung am Deskriptor), D9 (Rotvektor außerhalb der Riegelgrammatik). Die Mutationen sind aus der Absicht der Zeile geschrieben, nicht am Code durch den Vektor gerechnet.
2. **Etikett ohne Nachmessung je Zusagehälfte** — D1 (Regressionswache, obwohl heute ein Aufruf stattfindet), D8 (Regressionswache im Urteil, Meldungshälfte heute rot), D5 („der Kanal fehlt still" stimmt am Code nicht). Das Etikett ist je Zeile vergeben, nicht je Satz der Zusage am Basis-SHA gemessen.
3. **Korpus mit einem Urteil und einer Stufe für verschiedene Leser** — D7, D10, dazu der fehlende Hook in D11. Die Tabelle modelliert „das" Urteil eines Vektors; die Beine sind aber verschiedene Ketten (Vertragsreferenz gegen Produktleser, flacher Leser ohne Textriegel und Schema, zustandsabhängige Feldregeln).
4. **Etappen- und Bauzielzuordnung gegen Codeabhängigkeit** — D11 (A11 nach seinen Vektoren), D12 (Bootstrap-Stufen vor dem Tor), D6 (Writer einem Ziel zugeordnet, das ihn nicht übersetzt). Vektoren und Beine sind nach der Regel-Etappe verteilt, nicht danach, welche Stufe und welches Ziel am Ende der Etappe im Code existieren.
5. **Fehler- und Bedienweg nicht bis zur Beobachtung gedacht** — D8, D13 (boolesche Funktion verliert den Grund), D2 (deaktiviertes Paarfeld der Standardrolle), D5 (Gens Leser verwirft den ganzen Snapshot).

**Was ein Nacharbeitsauftrag deshalb als Ursache tragen muss, nicht als Punktkorrektur:**
- (a) je Zeile mit Rotbeweis eine am Code nachgerechnete Kette „Mutation → Vektor → beobachtete Zusage" mit Zahlen, Grammatik oder Aufrufpfad;
- (b) Etiketten je Zusagehälfte, am Basis-SHA gemessen, mit getrennten neuen Nachweisen und bestehenden Wachen;
- (c) die Korpusform mit Vertrags- und Produkturteil und Stufe je Leser (Teil 2 Punkt 3), rückwirkend über alle Einträge von §7.3;
- (d) je Etappe eine Fahrbarkeitsprüfung: jeder neue Eintrag hat am Etappenende sein Bein, dessen Bauziel die gemessene Funktion übersetzt, und seine Stufe;
- (e) jede Meldungs- und Wirkungserwartung bis zur Zuweisung (`letzterFehler`, Deskriptor, Snapshot, Zähler) verfolgt, einschließlich aller Leser desselben gespeicherten Werts.

Ohne (a) bis (e) sind dieselben Fehlerklassen in den nicht beanstandeten Zeilen zu erwarten; diese Validierung hat nur die 14 Befunde und ihre Folgestellen nachgemessen.

---

## Nicht geprüft

- Keine der 145 Matrixzeilen außerhalb der 14 Befunde und der genannten Folgestellen ist auf dieselben Ursachen durchgesehen; insbesondere nicht jede übrige Rotmutation und jedes übrige Etikett.
- Nichts ist gebaut oder gelaufen: alle Wirkungen (Invalidierung bei `3`/`3.0`, Snapshot-Verwurf an Gen, Riegelzahlen, Verifierannahme der Bytetransformation) sind aus der Quelle gelesen, nicht gemessen. Die Formatierung `3.0` durch serde_json ist an den Crate-Quellen (`ser.rs:1720-1722`, zmij `lib.rs:1624`) belegt, nicht an einem Lauf.
- Die Gesamtgrenze (262 144) des in Teil 2 Punkt 7 vorgeschlagenen D3-Vektors ist nicht nachgerechnet.
- Ob B27 die Laufzeitadresse (`v3LogonSid`, `v3SessionEpoch`, `instanceNonce`) im Testbau setzen kann, ist nicht geprüft; vorgeschlagen ist deshalb auch der Ausschnitt ohne Adresse.
- Die Zugänglichkeit von `GetOptionalFieldOffset` über einen Zeiger auf `flatbuffers::Table` (private Basis) ist nicht übersetzt, nur an den Headern gelesen.
- Auditwortlaut (`BEFUNDE.md`, Rohberichte), die drei Quellvalidierungen (nur Teil B, Zeilen 400-489, gezielt gelesen), §9 Prüfliste, Karte U65, Sicherheitsfragen am Tor und die Prioritätsangaben „P2" des Prüfers sind nicht Gegenstand und nicht bewertet.
- Die Frage, ob M-93 (c) als zustandsabhängiger Eintrag im Korpus bleiben soll (Form A) oder ein reiner Modultest wird (Form C), ist Entscheidung des Dirigenten; hier nur vorgelegt.

FERTIG Validierung Matrixprüfung 1, 12 bestätigt, 2 präzisiert, 0 widerlegt.
