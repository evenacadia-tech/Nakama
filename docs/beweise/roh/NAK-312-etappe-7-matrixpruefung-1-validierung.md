# NAK-312 — Validierung der Matrixprüfung der Etappe 7, Runde 1, an der Quelle

## Kopf

| Merkmal | Wert |
|---|---|
| Ticket | NAK-312, Planschritt S25k, Etappe 7a (Matrixnachtrag M-94 bis M-131 in §46, Bauplan der Etappe 7b in §47), Matrixprüfung der Etappe 7, Runde 1 |
| HEAD | zu Beginn `6ea8ec526537ece6505b6d379516d435b4069316` (`Get-Date` 2026-09-23 00:52:48 +02:00); vor dem Schreiben `6ea8ec526537ece6505b6d379516d435b4069316` (`Get-Date` 2026-09-23 01:18:58 +02:00) |
| Arbeitsbaum | sichtbarer Checkout `C:\Users\phili\Projekte\Nakama`, Branch `master`; `git status --short` zu Beginn und vor dem Schreiben gleich: `?? briefing-hub/`, `?? docs/beweise/roh/NAK-312-etappe-7-matrixpruefung-1-urteil.md`, `?? docs/beweise/roh/NAK-312-etappe-7-matrixpruefung-1-validierung-auftrag.txt`, `?? nimbalyst-local/` |
| Modell | Opus 5.5 (`claude-opus-5-5`), Effort max |
| Gegenstand | Prüfbericht `docs/beweise/roh/NAK-312-etappe-7-matrixpruefung-1-urteil.md` (NEEDS_WORK, neun Defekte; hier D1 bis D9 in der Reihenfolge des Textes) über §46 und §47 von `docs/beweise/NAK-312.md` am ZIEL `ecef9008`; Gate-Bezug §45, §48, §49; Maßstab für Klassen und Ausschlüsse: `docs/beweise/roh/NAK-312-etappe-7-matrixpruefung-1-auftrag.txt` |
| Prüfform | lesend: Read, Grep, `git rev-parse`, `git status`, `git diff`; kein Bau, kein Test, kein Kanon, kein Skript, kein FL Studio, kein Python-Interpreter; einzige Schreibaktion diese Datei (LF) |
| Eigene Messungen | `git diff --stat 6ff660ed HEAD -- eq-copilot broker tools`: allein `eq-copilot/install/nakama-installer-v1.json` (3 Einfügungen, 3 Löschungen). `git diff ecef9008 6ea8ec52 -- docs/beweise/NAK-312.md`: Hunks `@@ -5,7 +5,7 @@` und `@@ -14,7 +14,7 @@` (je eine Kopfzeile ersetzt) und `@@ -10144,3 +10144,142 @@` (§49 angehängt) — §45 bis §48 stehen an ZIEL und HEAD auf denselben Zeilen, die Zeilenangaben des Prüfers gelten am HEAD. Die JUCE-Quelle liegt als Bauartefakt vor (`eq-copilot/build/_deps/juce-src/`) und ist an den zitierten Stellen gelesen |
| Ergebnis | 7 bestätigt, 2 präzisiert (D3, D6), 0 widerlegt; alle neun sind DEFEKT nach dem Maßstab des Prüfauftrags; keiner fällt unter einen Ausschluss |

Zeilenangaben ohne Pfad beziehen sich auf `docs/beweise/NAK-312.md`, sonst auf den genannten Pfad am HEAD. Aussagen „wird rot" oder „bleibt grün" sind aus dem Kontrollfluss des Codes abgeleitet, nicht gelaufen.

## Teil 1 — die neun Befunde

### D1 — Satz 1 zieht die bestehenden 32-Quellen-Tests (B13, A23) nicht nach

**a) Beanstandet.** `:9455-9456`: „`eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp` (B13): `312/M-122` bis `312/M-125` und `312/M-128` über echte Snapshots (`mitgliedJson`, `:97`)." — `:9465`: „**Beine einzeln:** B13, B15, B14, A3, danach A1 (`EqCopNullTest`)." — §47.5 Satz 1 (`:9820-9833`) nennt Behauptungszeilen für B13, B15, B14 und A3. Eine Suche über §45 bis §49 (Zeile 8648 bis Dateiende) nach `A23`, `SourcesLatency`, `visible_latency`, `16-/32`, `16 und 32` und `pruefe_sonde012` findet nichts.

**b) Nachmessung.**
- B13, `eq-copilot/plugin/tests/Sonde012SourcesModelTest.cpp:788-818`: `auto lauf = [] (int n)` legt n Mitglieder an (`q.id = hex (20 + (unsigned) i);`, `:795`), übernimmt sie mit `uebernehme (m, snapshot (quellen), a);` (`:806`) — `uebernehme` ist `return m.uebernehmeSessionSnapshot (json, t, grund) == Model::SnapshotErgebnis::uebernommen;` (`:205-210`), der echte Leser, kein Fixturezugang — und verlangt `if ((int) sicht.quellen.size() != n) return 1.0e9;` (`:809`); `const auto p16 = lauf (16), p32 = lauf (32);` (`:815`); `pruefe (p16 <= 300.0 && p32 <= 300.0, "visible_latency_16_and_32_sources", …)` (`:816-818`). Mit der Grenze trägt die Sicht bei n = 32 genau 20 Zeilen, `lauf (32)` liefert `1.0e9`, die Prüfung fällt.
- A23, `eq-copilot/plugin/tests/Sonde012SourcesLatencyTest.cpp`: jeder Snapshot des echten Rust-Coordinators geht in `model.uebernehmeSessionSnapshot (json, Uhr::now(), grund)` (`:118`). `starten()` wartet höchstens 25 s auf `sicht.quellen.size() == static_cast<std::size_t> (n)` samt Deskriptoren (`:206-219`) und gibt sonst `false` zurück (`:243`); `main` fährt `for (const int n : { 16, 32 })` und endet bei `! lauf.starten()` mit „FEHLER: … Quellen wurden nicht vollstaendig sichtbar" und `return 1;` (`:320-327`). Liefe der Start durch, fielen auch `soll = startAnzahl + … ((r + 1) * n)` (`:276`), `if (werte.size() != … (runden * n)) return 1.0e9;` (`:295`) — P2-Frames einer Quelle ohne Eintrag erzeugen keine Revision und damit keinen Latenzwert (`:156-158`; `eq-copilot/plugin/src/SourcesModel.cpp:1576-1579`) — und `erwartet = … (n * 4 * 3)` Anzeige-Revisionen (`:340-345`).
- A23 ist Kanonbein: `tools/beweise.ps1:668` (`Kuerzel='A23'; Name='pruefe_sonde012_sources_latency.py'; …; Behauptung='… Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters.'`), Bauziel `tools/beweise.ps1:834-838`; im letzten Kanon grün mit „32 Quellen / 2048 Samples p95 55.5564 ms <= 300 ms" (`docs/beweise/roh/NAK-312-9118b06-dirty.md:3446-3461`).
- Die Reproduktion stimmt.

**c) Was bricht.** Gate S25k, BEWEIS: „… vollem Kanon GRÜN und Laufzeit-Arm." (`docs/plan/plan.json:361`, wörtlich im Kopf `:6`) mit §47.7 „**Voller Kanon** nach allen drei Sätzen" (`:9895`): mit dem Bauplan, wie er steht, endet der Kanon nach Satz 1 rot an B13 und A23. §47.5: „Wer eine Zusage ändert, zieht Runnerkopf, Skriptkopf und den lebenden Manifestkopf nach" (`:9812-9813`) — die A23-Behauptung ändert sich mit der Grenze und fehlt. Auftrag 7a (`docs/beweise/roh/NAK-312-etappe-7a-auftrag.txt:43`): „… neue oder geänderte Tests mit Namen und Bein, welche Beine aus tools/beweise.ps1 einzeln zu fahren sind, … Behauptungszeilen, die sich ändern …". Klasse im Prüfauftrag (`…-auftrag.txt:18`): „ein Satz des Bauplans §47 widerspricht … dem gemessenen Code am ZIEL".

**d) Urteil.** BESTÄTIGT · DEFEKT. Kein Ausschluss: C-1 (E-312-21) wird nicht angegriffen, nur der fehlende Nachzug bestehender Erwartungen.

**e) Schließende Regel.** Satz 1 führt `visible_latency_16_and_32_sources` (B13) und A23 (`Sonde012SourcesLatencyTest.cpp`) als geänderte Tests mit der Erwartung „min(n, 20) angenommen, n − min(n, 20) nicht angenommen", A23 unter den Einzelbeinen und die Behauptungszeilen A23 und B13 in §47.5.

### D2 — die leere Mitgliederkopie leert die Sicht in legacy nicht

**a) Beanstandet.** M-110 (`:9081`), Zusage: „**In Legacy sind alle sieben unverändert gehalten und wirken nicht: das Quellenmodell zeigt 0 Zeilen, und die Handgriffe am Bestand … werden verweigert wie heute. …**". M-120 (`:9091`), Zusage: „… in Legacy keine Zeile im Quellenmodell; nach der Rückkehr zu Main ist X eine Zeile. …". §47.4 (`:9750-9753`): „in `setzeBindung` (`:305-381`) entfällt die Löschstelle `:339-340`; die Kopie fürs Quellenmodell (`:341`) trägt die Mitglieder nur bei `main`, sonst ist sie leer (in `legacy` zeigt das Modell wie heute nichts)".

**b) Nachmessung.**
- Publikation beim Rollenwechsel, `eq-copilot/plugin/src/prozessor/State.cpp:376-379`: `werteSourcesPublikationAus (sourcesModel.setzePersistenteMitglieder (mainMitglieder, generation, folge)); pipe.reconnect(); controlV3.reconnect();`.
- `eq-copilot/plugin/src/SourcesModel.cpp:419-441`: `persistenteMitglieder = std::move (neu); for (auto it = eintraege.begin(); it != eintraege.end();) { … if (p != persistenteMitglieder.end()) { … } else if (! it->second.fluechtigImSnapshot) { if (hauptziel == it->first) hauptziel.clear(); it = eintraege.erase (it); } else { it->second.zeile.mitgliedschaft = Mitgliedschaft::unclassified; it->second.zeile.userLabel = it->second.descriptorLabel; aktualisiereName (it->second); ++it; } }` — eine leere Kopie entfernt nur Einträge ohne `fluechtigImSnapshot`; jeder Snapshot-Eintrag bleibt als `unclassified` stehen.
- `fluechtigImSnapshot` setzt `uebernehmeSessionSnapshot` für jedes Sondenmitglied (`e.fluechtigImSnapshot = true;`, `:1279`), für gespeicherte Nur-Mitglieder `false` (`:1346`).
- `controlEnde` entfernt nichts (`:576-606`): `if (e.fluechtigImSnapshot) e.zeile.control = Control::getrennt;` (`:582-583`), `e.zeile.messung = Messung::stale;` (`:584-585`).
- In legacy wird nicht neu abonniert: `const bool darfAufbauUrteilMelden = hVoraus.pluginKind == "main" && …;` (`eq-copilot/plugin/src/prozessor/Ipc.cpp:914-915`), `if (! darfAufbauUrteilMelden) { telemetryV3.reconnect(); return; }` (`:991-995`), `beginneSubscription` erst hinter `if (h.pluginKind != "main" || …) return;` (`:1033-1037`). Kein Snapshot ersetzt danach die stehenden Einträge; `sicht()` gibt alle Einträge aus (`SourcesModel.cpp:1670-1671`).
- Im Produktweg ist die gebundene Quelle ein Live-Eintrag: „Bind source" wirkt auf die unklassifizierte Zeile des Aktionsziels (`eq-copilot/plugin/src/PluginEditor.cpp:179-190`), und `sendeSourcesCommand` verlangt deren `runtimeNonce` (`if (quelle == sicht.quellen.end() || ! nakama::ipc::istHex32 (quelle->runtimeNonce)) return false;`, `Ipc.cpp:1296-1299`), die nur ein Snapshot setzt.
- Aufbau RW (`:9062-9066`) füllt die Mitglieder über quittierte `confirmJoin` ohne Snapshot (Muster `Sonde012ProjectReloadTest.cpp:2654-2660`); darin gibt es keinen flüchtigen Eintrag, der geplante Test wäre grün, der Fall mit Live-Quellen bleibt ungeprüft.
- Die legacy-Fläche zeichnet die Liste nicht und liest die Sicht nicht: `return processor.holeRolle() == "hub" && ! processor.stateNurLesen();` (`PluginEditor.cpp:955-958`), `if (mainFlaecheAktiv) { paintMainFlaeche (g); return; }` (`:1540-1544`), `sourcesTick` und `sourcesSicht` nur `if (mainFlaecheAktiv)` (`:271-284`).
- Die Reproduktion stimmt mit der Bedingung des Prüfers („bei bereits empfangenen Live-Quellen"): jeder flüchtige Eintrag des letzten Snapshots bleibt in legacy eine Zeile des Modells.

**c) Was bricht.** Der Bauplansatz „(in `legacy` zeigt das Modell wie heute nichts)" (`:9752-9753`) widerspricht dem Code am ZIEL (`SourcesModel.cpp:430-441`). Die Zusagen „das Quellenmodell zeigt 0 Zeilen" (M-110) und „in Legacy keine Zeile im Quellenmodell" (M-120) stellt der geplante Mechanismus nicht her, und kein Test misst sie mit Snapshot-Einträgen: §6-Regel „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt, nie an einem Nebeneffekt." (`:657-658`, für §46 in Kraft nach `:8990-8993`). Eine Quelle der Zusage gibt es nicht: U49 „Was damit festliegt" (`design/abnahmen/2026-09-21-rollenwechsel-daten-bleiben-erhalten-u49.md:24-34`) spricht von den Beständen, nicht vom Quellenmodell; der Entwurf sagt „`legacy` bleibt passiv" (`docs/FL-Nakama-Sonden-Design-Entwurf.md:1873`) und „`MainProject` existiert nur im klassifizierten Main" (`:4077`), nichts über Zeilen des Modells; `docs/plugin-wissen.md` und `design/docs/funktions-und-bedien-blueprint.md` ohne Treffer. Die Zusage ist eine Annahme des Bauers. E-312-23 (das ACK wirkt einmal auf den ruhenden Bestand, `:10224-10228`) ist nicht berührt.

**d) Urteil.** BESTÄTIGT · DEFEKT. Kein Ausschluss. Die Abhilfe hat zwei Wege (Teil 2 Punkt 2); der Prüfer nennt nur die Stilllegung.

**e) Schließende Regel.** Entweder stellt Satz 3 die Live-Sicht beim Wechsel weg von `main` rollenabhängig still, oder M-110, M-120 und §47.4 sagen nur, dass der ruhende Bestand in legacy keine Zeile ist — in beiden Fällen gemessen mit echten Snapshot-Einträgen.

### D3 — der Tap-Inhalt der K-A-Hälfte in der Eintrittsblende

**a) Beanstandet.** M-96 (`:9045`), Zusage: „… **Inhalt, Weg K-B: der Tap `post_committed` ist in jedem Bypassblock bitgleich zu dem von R. Weg K-A: der Tap trägt den Eingang, wie im Hard-Bypass**". §47.3, Weg K-A (`:9552-9556`): „Der Kern behandelt den Hostbypass im Audiothread wie einen Wechsel des Committed-Pfades in den Passthrough: Blende 256 Samples, danach rechnet der Pfad nicht (Filter eingefroren), der Tap `post_committed` trägt den Eingang wie im Hard-Bypass …".

**b) Nachmessung.**
- `eq-copilot/plugin/dsp/DspKern.cpp:1317-1319`: `verarbeitePfad (Pfad::committed, eingL.data(), eingR.data(), numSamples, cVonL.data(), cVonR.data(), cNachL.data(), cNachR.data(), cAusL.data(), cAusR.data(), cGewicht.data());` — `:1342`: `std::memcpy (tapZeiger (Tap::postCommitted, k), (k == 0) ? cAusL.data() : cAusR.data(), bytes);`. Der Tap ist der gemischte Ausgang des Committed-Pfades.
- Ein Wechsel in den Passthrough (so wechselt der eigene Hard-Bypass: `return ! p.eqEngagiert || p.hardBypass;`, `:460-464`) mischt: Quelle ist die rechnende Bank (`verarbeiteBank (z, z.quelle, -1, 0, vonL, vonR, numSamples);`, `:1065-1071`), Ziel der Eingang (`std::memcpy (nachL, dryL.data(), bytes);`, `:1093-1096`), `ausL[i] = vonL[i] * (1.0 - t) + nachL[i] * t;` mit `const double t = 1.0 - (double) rest / (double) kFadeSamples;` (`:1109-1115`), danach `ausL[i] = nachL[i];` (`:1117-1120`). Während der 256 Samples trägt der Tap also die lineare Mischung aus verarbeitetem Ausgang und Eingang, erst danach den Eingang.
- Aufbau HB: `prepareToPlay (48000, 256)` (`:9024`), die Blende beginnt an Sample 0 des ersten Blocks mit dem neuen Eintritt (`:9005-9008`) — der erste Bypassblock ist genau die Blende. Mit Trim +6 dB liegt der verarbeitete Ausgang für den Gleichwert 0,25 bei rund 0,4988 (`:9027-9029`); der Tap ist in Block 41 nicht der Eingang.
- Präzisierung: M-96 sagt nicht wörtlich „auch im ersten Bypassblock"; die K-A-Klausel ist zeitlich unbestimmt. In der Lesart „jeder Bypassblock" (so setzt die K-B-Hälfte derselben Zeile den Umfang) widerspricht sie dem K-A-Mechanismus im ersten Bypassblock; in der Lesart „wie im Hard-Bypass samt dessen Blende" ist das Orakel für den ersten Bypassblock undefiniert.
- Nebenbeobachtung (nicht Teil des Prüfberichts): die Inhaltshälften von M-96 (K-B und K-A) haben keinen eigenen Rotbeweis; die einzige Mutation „im Eintritt `processBlockBypassed` die Analysekopie auslassen → 0 → rot" (`:9045`, Spalte Rotbeweis) trifft die Zählung.

**c) Was bricht.** §6, Spalte Zusage: „einen prüfbaren Satz mit Maß und Einheit" (`:638`) gegen den K-A-Satz aus §47.3 (`:9552-9556`) und den Code (`DspKern.cpp:1109-1120`, `:1342`); E-312-19: „in M-95, M-96 und M-100 bleiben bis dahin beide Hälften stehen, die Matrixprüfung prüft beide" (`:10193-10195`).

**d) Urteil.** PRÄZISIERT · DEFEKT. Ausschluss 2 greift nicht: er nimmt die Wahl K-A gegen K-B und den Widerspruch von K-B zum Kaltstartsatz aus, prüft aber ausdrücklich, „ob jede der beiden Hälften in sich am Code trägt" (`…-auftrag.txt:24`). Die K-A-Hälfte ist in sich unbestimmt; nicht nur ihre Wahl ist offen. Ihre Wirkung tritt erst ein, wenn U58 K-A ergibt.

**e) Schließende Regel.** Die K-A-Hälfte von M-96 legt den Tap-Inhalt des ersten Bypassblocks fest (während der Blende die lineare Mischung, danach der Eingang) und die Inhaltshälften bekommen einen eigenen Rotbeweis — oder der Dirigent erklärt die K-A-Hälften von M-95, M-96 und M-100 ausdrücklich zum Vorbehalt, der vor dem Bau von Satz 2 präzisiert und erneut geprüft wird.

### D4 — M-99 und M-107: die Analysehälfte ist heute rot

**a) Beanstandet.** M-99 (`:9048`), Zusage: „… die Analyse läuft in beiden Eintritten wie heute über `processBlock`"; Rotbeweis: „**Regressionswache** (hält heute, weil die JUCE-Basis nichts schreibt); einmal gebrochen: die Stufe auch hinter einem ruhenden Pfad den Eingang über `double` zurückschreiben lassen → unter FTZ und DAZ fallen die Subnormals, die Wachmarke kippt → rot". M-107 (`:9056`), Zusage: „… die Analyse verbucht ihn wie heute ohne Tap; nach `prepareToPlay` gilt M-106"; Rotbeweis: „**Regressionswache** (hält heute über die JUCE-Basis und `:1201`); einmal gebrochen: die Stufe vor der Prüfung `:1201` den Eingang über `double` zurückschreiben lassen → …". §46.6 zählt beide unter „**Regressionswache** vierzehn" (`:9172-9173`).

**b) Nachmessung.**
- `eq-copilot/build/_deps/juce-src/modules/juce_audio_processors/processors/juce_AudioProcessor.cpp:591-606`: `processBypassed` besteht aus `jassert (getLatencySamples() == 0);` und `for (int ch = getMainBusNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch) buffer.clear (ch, 0, buffer.getNumSamples());`; `processBlockBypassed` ruft nur `processBypassed`. Grep nach `processBlockBypassed` und `getBypassParameter` über `eq-copilot/plugin/`: kein Treffer. Der Wrapper wählt `processBlockBypassed`, wenn `pluginInstance->getBypassParameter() == nullptr && comPluginInstance->getBypassParameter()->getValue() >= 0.5f` (`…/juce_audio_plugin_client_VST3.cpp:3906-3909`).
- Die Analyse steht nur in `processBlock` (`eq-copilot/plugin/sonde/SondeProcessor.cpp`): Tapzweig `analyseQueue.veroeffentliche (&abgriff, 1, kanaele, samples, stempel);` (`:644`), ruhender Passthrough ohne Tap `analyseQueue.veroeffentliche (&abgriff, 1, kanaele, samples, stempel);` (`:646-654`), dritter Zustand `analyseQueue.verwirfOhneAudio (samples);` (`:655-666`).
- Fälle (a) EQ aus und (b) Hard-Bypass nehmen heute in `processBlock` den Tapzweig; Fall (c) unvorbereitet: `freigeben` setzt die Pfade in Ruhe und `tapGueltig = 0` (`DspKern.cpp:185`, `:193`), `committedRuhtImPassthrough()` ist wahr (`SondeProcessor.cpp:1776-1786`), die Analyse bekommt den rohen Eingang. Im Bypasseintritt läuft heute keiner der drei Zweige.
- Die Reproduktion stimmt: die Audiohälfte gilt heute, die Analysehälfte nicht; die Double-Mutation trifft nur die Audiohälfte.

**c) Was bricht.** §6, Etiketten: „**Regressionswache**: heute grün und hält, was der Bau nicht ändern darf" und „**zwei Hälften**: die Zeile trägt zwei Etiketten und sagt je Hälfte, welches gilt." (`:650-653`); §6-Regel „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt" (`:657-658`); Prüfauftrag: „ein Etikett … stimmt am Code am ZIEL nicht".

**d) Urteil.** BESTÄTIGT · DEFEKT. Kein Ausschluss.

**e) Schließende Regel.** M-99 und M-107 tragen zwei Hälften: Audio als Regressionswache mit der Double-Mutation, Analyse als „heute rot" mit der Gegenprobe „Blockzählung im Bypass steigt heute um 0" und einer Mutation am Analyseteil des Bypasseintritts.

### D5 — M-103: der Rotbeweis der Ruhegrenze fällt nicht zuverlässig

**a) Beanstandet.** M-103 (`:9052`), Ereignis: „40 Blöcke normal, 60 Hostbypass, 40 normal; davor und danach `getStateInformation`, `stateHashText()`, `stateRevision()`, `automationEpoche()`, Undo-Ring, Zahl der Host-Dirty-Meldungen"; Zusage: „RUHEGRENZE: der Hostbypass zählt verarbeitete Samples weiter; die Automationsepoche einer Geste schließt nach 0,25 s verarbeiteter Samples auch im Hostbypass, wie ohne ihn"; Rotbeweis: „Ruhegrenze: **heute rot** …; Mutation nach dem Bau: `verarbeiteteSamples` im Bypasseintritt nicht erhöhen → die Epoche bleibt offen → rot".

**b) Nachmessung.**
- Ruhegrenze, `SondeProcessor.cpp:1834-1839`: `if (transaktion->automation().laeuft && fs > 0.0 && verarbeiteteSamples.load (std::memory_order_relaxed) - samplesBeiLetzterAutomation >= (std::uint64_t) (nakama::transaktion::kAutomationsRuheSekunden * fs)) transaktion->automationRuht();` mit `inline constexpr double kAutomationsRuheSekunden = 0.25;` (`eq-copilot/plugin/state/NakamaTransaktion.h:62`) — 12 000 Samples bei 48 kHz. Den Bezugspunkt setzt der Takt beim Übernehmen eines Hostereignisses (`samplesBeiLetzterAutomation = verarbeiteteSamples.load (…);`, `:1824`).
- Aufbau HB mit Block 256: 40 + 40 normale Blöcke sind 20 480 Samples ≥ 12 000. Liegt der letzte Hostpunkt vor den ersten 40 Blöcken (die Zeile nennt „eine laufende Hostgeste", keinen Zeitpunkt), schließt auch der Mutant die Epoche in den folgenden normalen Blöcken, und die Lesung „danach" ist in beiden Fassungen „geschlossen".
- Der Worker startet im Konstruktor (`workerLaeuft.store (true); worker = std::thread ([this] { workerLauf(); });`, `:306-307`) und fährt `dspKontrollTakt()` im Abstand von mindestens 5 ms Wandzeit (`:753-757`). Die Zeile nennt weder `kontrollTaktFuerTest()` (`SondeProcessor.h:349-351`) noch die Taktsperre `mitAngehaltenemTaktFuerTest` (`:415-428`) noch den Zeitpunkt des letzten Hostpunkts; wann die Epoche im Lauf schließt, hängt damit am Takt des Workers.
- Die Reproduktion stimmt.

**c) Was bricht.** §6-Regel: „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt, nie an einem Nebeneffekt. Keine Zeile hängt an Wandzeit oder Zufall: Ladestart, Blockrand, Kontrolltakt und Destruktorfolge sind deterministisch ansteuerbar (`kontrollTaktFuerTest`, …)" (`:657-661`).

**d) Urteil.** BESTÄTIGT · DEFEKT. Kein Ausschluss.

**e) Schließende Regel.** Die Ruhegrenzenhälfte von M-103 legt den letzten Hostpunkt unmittelbar vor den Bypassabschnitt, fährt die Kontrolltakte ausdrücklich (Taktsperre oder Abstand von mindestens einem Block zur Grenze) und liest die Epoche vor und nach dem Überschreiten der 12 000 Samples innerhalb des Bypassabschnitts, bevor normale Blöcke folgen.

### D6 — der Gegenfall „Überlauf von Zählern" fehlt in Satz 2

**a) Beanstandet.** §46.1 (M-94 bis M-109, `:9043-9058`) enthält keine Zeile und keinen Satz zum Überlauf eines Zählers. In §46 stehen nur M-124 „kein Zähler läuft über (höchstens 64 flüchtige plus 64 gespeicherte Kandidaten)" (`:9113`) und M-121 „Revision am `int64`-Rand" (`:9092`), beide Satz 1 bzw. Satz 3. M-103 (`:9052`), Reihenfolge und Frist: „`verarbeiteteSamples` (`eq-copilot/plugin/sonde/SondeProcessor.cpp:611`) steigt in beiden Eintritten".

**b) Nachmessung.**
- `SondeProcessor.h:776`: `std::atomic<std::uint64_t> verarbeiteteSamples { 0 };`, `:777`: `std::uint64_t samplesBeiLetzterAutomation = 0;`; `SondeProcessor.cpp:611`: `verarbeiteteSamples.fetch_add ((std::uint64_t) samples, std::memory_order_relaxed);`; die Differenz an `:1837-1838` ist vorzeichenlos.
- Kein Testzugang setzt `verarbeiteteSamples` (Grep über `eq-copilot/plugin/`: nur `:611`, `:1824`, `:1837`).
- Weitere Zähler, die Satz 2 berührt: Queue-Zähler `std::uint64_t` (`eq-copilot/plugin/core/StampedAudioQueue.h:574-584`), Kernzähler `std::atomic<std::uint64_t>` (`DspKern.h:753-759`), die Hostereigniszähler `std::array<std::atomic<std::uint32_t>, …> hostEreignis` (`SondeProcessor.h:753`) und der Blockrandstand `std::uint32_t` (`DspKern.h:250`, `:260`), die der gemeinsame Rumpf nach §47.3 auch im Bypasseintritt liest („Blockrand der Hostmailbox (`:576-602`), Stempel, Analysekopie (`:613-666`) und `verarbeiteteSamples` (`:611`) laufen in beiden gleich", `:9587-9589`).
- Das Fehlen stimmt. Präzisierung: der vom Prüfer genannte Zähler läuft real nicht über — 2^64 Samples sind bei 48 kHz rund 1,2 · 10^7 Jahre —, und die vorzeichenlose Differenz bleibt über einen Wrap richtig, solange der Abstand unter 2^64 liegt. Erreichbar ist dagegen der Wrap der uint32-Hostereigniszähler (§21.11: „über 60 Tage Dauerautomation", `:3461-3467`), die Satz 2 in den Bypasseintritt holt; ihr Überlauf ist für `processBlock` gemessen (312/M-19 `:3200`, 312/M-82 `:3338`) mit dem vorhandenen Zugang `setzeHostZaehlerFuerTest` (`SondeProcessor.h:433-445`).

**c) Was bricht.** Auftrag 7a (`docs/beweise/roh/NAK-312-etappe-7a-auftrag.txt:41`): „Zahlenränder ehrlich: … Überlauf von Zählern, … stehen als eigene Zeilen, wo ein Gegenstand sie berührt." und die DEFEKT-Liste des Prüfauftrags (`…-auftrag.txt:18`: „… erste und letzte Samples der Blende, Zählerüberlauf, …"). Einen „deterministischen Wrap-Fall samt Rotbeweis" für den uint64-Zähler verlangt keine der beiden Stellen; verlangt ist die eigene Zeile.

**d) Urteil.** PRÄZISIERT · DEFEKT. Nicht unter Ausschluss 4 („theoretische Randfälle ohne Zusage"), weil der Auftrag 7a die Zeile verlangt; der erreichbare Teil ist der uint32-Wrap, nicht der vom Prüfer genannte uint64-Zähler.

**e) Schließende Regel.** Satz 2 bekommt eine Zeile „Zählerränder im Hostbypass", die jeden berührten Zähler mit Breite und Erreichbarkeit nennt, den erreichbaren uint32-Wrap der Hostereigniszähler im Bypasseintritt deterministisch mit Rotbeweis misst und den uint64-Samplezähler als nicht erreichbar und modulo-sicher benennt.

### D7 — M-104: die Offline-Mutation ist wirkungslos

**a) Beanstandet.** M-104 (`:9053`), Zusage: „**A und B sind bitgleich, 0 abweichende Samples: … der Wechsel nach offline stellt allein die Hörmatrix hart (Etappe 5) und lässt die Hostbypass-Blende weiterlaufen; `processBlockBypassed` setzt weder den Offline-Riegel noch das harte Schalten**"; Rotbeweis: „Entkopplung: **Regressionswache**; einmal gebrochen: im Bypasseintritt `setzeOfflineRiegel (true)` rufen oder die Hostbypass-Blende beim harten Schalten mitbeenden → B schneidet, A blendet → rot".

**b) Nachmessung.**
- `DspKern.h:340-346`: `void setzeOfflineRiegel (bool an) noexcept { if (an) { offlineRiegelAn.store (true, std::memory_order_release); hartSchalten.store (true, std::memory_order_release); } …`.
- `DspKern.cpp:1216-1217`: `const bool hart = hartSchalten.load (std::memory_order_acquire) && hartSchalten.exchange (false, std::memory_order_acq_rel);`; `:1225`: `if (offlineRiegelAn.load (std::memory_order_acquire)) wirksam = Hoermatrix::processed;`; `:1227-1236`: `if (hart) { hoerVorher = wirksam; hoerLaufend = wirksam; hoerFadeRest = 0; }` — beide wirken allein auf die Hörmatrix.
- Aufbau HB (`:9023-9026`) hat keine Vorschau; die Hörmatrix steht auf Processed (`bereiteVor`, `DspKern.cpp:146-150`). Die Mutation im Bypasseintritt läuft in A und B; B trägt den Riegel ohnehin (`setNonRealtime (true)` → `dspKern->setzeHoermatrix (nakama::dsp::Hoermatrix::processed); dspKern->setzeOfflineRiegel (true);`, `SondeProcessor.cpp:1590-1591`). Hörbar ändert sich in keinem der beiden etwas; A und B bleiben bitgleich.
- Nebenbeobachtung zur zweiten Alternative: „die Blende beim harten Schalten mitbeenden" wirkt nur im Teilfall „Wechsel nach offline mitten in einer laufenden Hostbypass-Blende". Bei Block 256 füllt die Blende genau den ersten Bypassblock (`:9005-9008`), und `setNonRealtime` wirkt nur vor einem Block (`pluginInstance->setNonRealtime (data.processMode == Vst::kOffline);`, `…/juce_audio_plugin_client_VST3.cpp:3892`; `hart` wird im nächsten Stück verbraucht, `DspKern.cpp:1216-1217`). Außerdem steht B laut Zeile „mit `setNonRealtime (true)` vor jedem Block" offline (`:9053`), also nicht erst mitten in der Blende. Der Teilfall ist im Aufbau HB nicht herstellbar.
- Die Reproduktion stimmt.

**c) Was bricht.** §6-Regel „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt" (`:657-658`); die Zusage „`processBlockBypassed` setzt weder den Offline-Riegel noch das harte Schalten" und die zweite Grenze aus §7.7 („die harte Hörmatrixumstellung der Etappe 5 wird **nicht** an `processBlockBypassed` gekoppelt", `:1868-1870`) bleiben ohne wirksamen Rotbeweis.

**d) Urteil.** BESTÄTIGT · DEFEKT. Kein Ausschluss.

**e) Schließende Regel.** Die Entkopplungshälfte von M-104 misst die Richtung „der Bypasseintritt setzt nichts" direkt am Echtzeitprozessor (Riegel und wirksame Hörmatrix nach Bypassblöcken unverändert) und die Richtung „das harte Schalten lässt die Stufe weiterlaufen" in einem Teilfall mit Blockgröße unter 256, in dem B erst mitten in der Blende offline wird.

### D8 — M-117: der Read-only-Mutant ist nicht beobachtbar

**a) Beanstandet.** M-117 (`:9088`), Zusage: „**Beide Wechsel werden verweigert, kein Bestand wandert, die Originalbytes kommen bytegleich zurück, 0 Host-Dirty**"; Reihenfolge: „die Wache `nurLesen` steht vor jeder Änderung (`eq-copilot/plugin/src/prozessor/State.cpp:323-324`)"; Test: „B2 bestehend (`StateMigrationTestMain.cpp:2359-2364`), um den Bestand ergänzt"; Rotbeweis: „**Regressionswache**; einmal gebrochen: die Wache `nurLesen` hinter die Klassenänderung ziehen → ein Host-Dirty entsteht → rot".

**b) Nachmessung.**
- `State.cpp:321-341`: `std::lock_guard<std::mutex> l (bindungMutex); if (zustand.nurLesen) return false; auto neu = zustand.common; neu.klasse = klasse; … if (neu == zustand.common) return false; zustand.common = neu; if (klasse != nakama::state::Klasse::main) zustand.mainProjectMitglieder.clear(); mainMitglieder = zustand.mainProjectMitglieder;` — die Publikation mit Dirty folgt erst hinter dem Sperrblock: `werteSourcesPublikationAus (sourcesModel.setzePersistenteMitglieder (mainMitglieder, generation, folge));` (`:376-377`). Dirty meldet allein `werteSourcesPublikationAus` (`meldeHostDirty();`, `Ipc.cpp:1488`).
- Jede Lage der Wache innerhalb des Sperrblocks (hinter `:338` oder hinter `:362`) kehrt vor `:376-377` mit `false` zurück: kein Dirty, keine Publikation, keine Revision. Ein Dirty entstünde nur, wenn die Wache ganz fehlt.
- `eq-copilot/plugin/state/NakamaState.cpp:2162-2169`: `if (z.nurLesen) { aus = z.originalBytes; return; }` — die Bytes bleiben gleich.
- Der Mutant hat aber `zustand.common` im Speicher geändert; lesbar über `holeRolle()`, `holeLabel()`, `holePaarId()` (`State.cpp:226-228`) und `holeZustandKopie()` (`:254-258`). Der bestehende Test prüft nur `! p->setzeBindung ("hub", "x", "") && ! p->neueSensorId() && dirty.nonParam == 2` (`eq-copilot/plugin/tests/StateMigrationTestMain.cpp:2364`), keine In-Memory-Bindung.
- Die Reproduktion stimmt.

**c) Was bricht.** §6-Regel „Ein Rotbeweis fällt an der Zeile, die die Zusage trägt" (`:657-658`). Die Zusage sichert `CLAUDE.md:193-195` („… werden read-only mit Originalbytes gehalten; jede persistente Änderung meldet dem Host Dirty-State.") und den Kopf, Grenze „read-only unverändert" (`:18`); der genannte Rotbeweis prüft das nicht.

**d) Urteil.** BESTÄTIGT · DEFEKT. Kein Ausschluss.

**e) Schließende Regel.** M-117 beobachtet nach jeder verweigerten `setzeBindung` zusätzlich die In-Memory-Bindung und den Bestand (Rolle, Label, Paar-ID, Klasse, Projektbindung, die sieben unverändert), sodass die Mutation „Wache hinter `zustand.common = neu`" an genau dieser Beobachtung fällt.

### D9 — M-127 und M-128: das Etikett „Regressionswache" stimmt nicht

**a) Beanstandet.** M-127 (`:9116`), Zustand: „Snapshots mit 20, 21, 32 und 64 Sonden über den echten Pfad, ohne Hauptziel"; Zusage: „… `sourcesZeilen().size()` ist gleich der Zahl der Zeilen der Sicht — keine angenommene Quelle ist ungezeichnet —, und jede gezeichnete Zeile wird per Klick Hauptziel"; Rotbeweis: „**Regressionswache** für das Ersatzziel (M-73 hält heute über die Fixture); einmal gebrochen: `kAnnahmeGrenze` auf 21 → …". M-128 (`:9117`), Zustand „wie M-123, X nicht angenommen"; Zusage: „**Keine Messung von X erreicht die Sicht: `uebernehmeP2` meldet für X-Frames keine Änderung, keine Zeile zählt einen Befund für X, die angenommenen Zeilen sind unberührt. …**"; Rotbeweis: „**Regressionswache** (für unbekannte IDs hält das Überspringen heute); …". §46.6 zählt beide als Regressionswache (`:9172-9173`); §47.2: „M-122, M-127, M-128, M-130 und die Bestandshälfte von M-126 als Regressionswache mit der Mutation ihrer Zeile" (`:9467-9469`).

**b) Nachmessung.**
- `SourcesModel.cpp:1670-1671`: `for (const auto& [_, e] : eintraege) s.quellen.push_back (e.zeile);` — heute ohne Grenze: 21, 32, 64 Sondenmitglieder ergeben 21, 32, 64 Sichtzeilen (Leserdeckel 64, `:1188-1189`).
- `PluginEditor.cpp:1144-1160`: `auto r = sourcesSpalte().reduced (8); r.removeFromTop (48); const int h = juce::jlimit (18, 34, r.getHeight() / juce::jmax (1, (int) sourcesAnzeige.quellen.size())); for (…) { if (r.getHeight() < h) break; aus.push_back (r.removeFromTop (h)); }` — bei 760×430 bleiben 430 − 16 − 48 = 366 px, ab 21 Zeilen ist h = 18, also 20 Rechtecke. M-127 heute: 21/32/64 Sichtzeilen gegen 20 Rechtecke — die Kapazitätshälfte ist rot.
- `SourcesModel.cpp:1576-1579`: `auto it = eintraege.find (frame.instanceId); if (it == eintraege.end() || ! it->second.fluechtigImSnapshot || it->second.zeile.runtimeNonce != frame.runtimeNonce) continue;` — übersprungen werden nur Frames ohne Eintrag. X aus dem 21er-Snapshot ist heute ein flüchtiger Eintrag (`:1279`); seine Frames ändern die Zeile (`geaendert = true;`, `:1623`; `return geaendert;`, `:1626`). `zaehleOffeneFindings`: `const auto it = eintraege.find (b.candidateSource); if (it == eintraege.end()) continue; … ++it->second.zeile.findingsOffen;` (`:1716-1721`) — X zählt heute einen Befund. M-128 ist heute rot.
- Heute grün ist die Ersatzzielhälfte: `stelleZielSicher` wählt die erste Quelle in Anzeigeordnung (`:1783-1798`), M-73 hält es über die Fixture.
- Die Reproduktion stimmt.

**c) Was bricht.** §6, Etiketten: „**heute rot**: am Basis-SHA widerspricht das Verhalten der Zusage nachweislich; die Zeile nennt die Gegenprobe …", „**Regressionswache**: heute grün …", „**zwei Hälften** …" (`:646-653`); §47.1: „„heute rot": der neue Test läuft vor der Reparatur einmal gegen den unveränderten Stand (Gegenprobe)" (`:9320-9322`); Prüfauftrag: „ein Etikett … stimmt am Code am ZIEL nicht".

**d) Urteil.** BESTÄTIGT · DEFEKT. Kein Ausschluss: C-1 und „die Geometrie ist die Prüfung" (E-312-21) bleiben unberührt.

**e) Schließende Regel.** M-127 trägt zwei Hälften (Ersatzziel Regressionswache, Kapazität „heute rot" mit der Gegenprobe 21/32/64 Sichtzeilen gegen 20 Rechtecke), M-128 ist „heute rot" mit der Gegenprobe am 21er-Snapshot, und §46.6 und §47.2 zählen neu.

## Teil 2 — Entscheidungsgrundlagen

### 1. Zu D1: welche Tests mehr als 20 Quellen einspeisen, und die kleinste Regel für Satz 1

Grep über `eq-copilot/plugin/tests/` nach Quellenzahlen (21 und mehr) und nach Snapshotaufbau (`uebernehmeSessionSnapshot`, `SourcesModel`):

| Fundstelle | Einspeisung | Bein | nach der Grenze 20 |
|---|---|---|---|
| `Sonde012SourcesModelTest.cpp:788-818`, `visible_latency_16_and_32_sources` | echte Snapshots (`uebernehme`, `:205-210`) | B13 (`tools/beweise.ps1:750`) | rot: `sicht.quellen.size() != n` bei n = 32 → `1.0e9` (`:809`) |
| `Sonde012SourcesLatencyTest.cpp:118`, `:206-219`, `:276`, `:295`, `:320-327`, `:340-345` | echter Coordinator, echte Snapshots | A23 (`tools/beweise.ps1:668`) | rot: `starten()` wartet auf 32 sichtbare Quellen, Abbruch nach 25 s mit `return 1` |
| `Sonde012ProjectReloadTest.cpp:2647-2726`, 312/M-76 (40 Mitglieder) | Mitglieder über quittierte `confirmJoin` (`:2656-2657`), Sicht über `setzeSourcesFixtureFuerTest` (`:2690`) | B14 | grün: geprüft werden `sourcesPersistenteMitgliederFuerTest().size() == 40` (`:2675`, `:2701`, `:2715`) und die State-Namen (`namen`, `:2531-2537`) — beides bleibt ungekürzt (`:9422-9423`) —, `gezeichnet == 20` über die Fixture |
| `ShotTestMain.cpp:161-164`, `:898-931`, `:937-951` (20, 21, 25, 32, 64) | `setzeSourcesFixtureFuerTest` (`:119`, `:850`, `:961`, `:1006`) | B15 | grün: der Fixturezugang bleibt ungedeckelt (`:9447-9451`) |
| `SessionSoakMain.cpp:620`, `:681`, `:910` | echte Snapshots mit `anzahl` Sonden | A24 mit `--sonden 16` (`tools/beweise.ps1:638`) | grün (16 Sonden) |
| `IpcLastMain.cpp:85` (32 Sonden) | kein `SourcesModel` (Grep ohne Treffer) | A22 | nicht betroffen |

A23 ist Bein des Kanons (`tools/beweise.ps1:668`, Bauziel `:834-838`, grün im letzten Kanon `docs/beweise/roh/NAK-312-9118b06-dirty.md:3446-3461`). `Sonde012SourcesLatencyTest.cpp` ist Ticketpfad über „`eq-copilot/plugin/tests/**` für Regressionen" (Kopf, Ticketpfade, `:17`); `tools/beweise.ps1` ist Ticketpfad „nur Behauptungszeilen", die A23-Behauptung darf sich also ändern. `tools/eq-copilot/pruefe_sonde012_sources_latency.py` ist kein Ticketpfad (weder Kopf `:17` noch E-312-24); die Regel muss ohne ihn auskommen, und sie kann es: er prüft nur Exitcode 0 und die sechs Schlüssel `p95_16_*`, `p95_32_*` mit ihren Grenzen (`:81-89`).

Kleinste Regel für Satz 1:
- B13 `lauf (n)`: Erwartung `sicht.quellen.size() == min (n, 20)` und `sicht.nichtAngenommen == n − min (n, 20)` (bei 32: 20 angenommen, 12 nicht angenommen); die Zeitgrenze p95 ≤ 300 ms bleibt.
- A23: `starten()` wartet auf min(n, 20) sichtbare Quellen mit Deskriptor und, nach dem Bau, auf `nichtAngenommen == n − 20` (`:206-219`); `messen()` rechnet `soll` und die Sollgröße von `werte` mit der Zahl der angenommenen Quellen statt n (`:276`, `:295`), ebenso `erwartet` (`:340`). Welche 20 angenommen sind, hängt bei A23 an der Beitrittsfolge (die Quellen starten nacheinander, `:174-204`; C-1 lässt angenommene Zeilen stehen, `:9411-9413`) — der Test liest deshalb die Zahl aus der Sicht, nicht die IDs 100 bis 119. Frames der 12 nicht angenommenen Quellen erzeugen keine Revision; das misst nebenbei „misst nicht mit" unter Last. Die JSON-Schlüssel bleiben (`p95_32_*` heißt dann „32 verbundene Quellen").
- Behauptungszeilen: A23 (`tools/beweise.ps1:668`) „Bei 16 und 32 verbundenen Quellen, davon höchstens 20 angenommen, …"; B13 (`:750`) der Satz zur 16-/32-Messung; Skriptkopf `Sonde012SourcesLatencyTest.cpp:1-2`; §47.5 Satz 1 um A23, §47.2 „Beine einzeln" um A23, §47.0 Ticketpfade von Satz 1 um `Sonde012SourcesLatencyTest.cpp`.

### 2. Zu D2: was das Quellenmodell in legacy heute zeigt, und zwei Wege

Heute (Zitate in D2 b): beim Wechsel `main` → `legacy` publiziert `setzeBindung` die geleerte Kopie (`State.cpp:339-341`, `:376-377`). `setzePersistenteMitglieder` (`SourcesModel.cpp:370-457`) prüft zuerst Generation (`:394-398`) und Folge (`:404-408`), löscht dann jeden Eintrag ohne `fluechtigImSnapshot` (`:430-434`) und lässt jeden Snapshot-Eintrag als `unclassified` stehen (`:435-441`). `uebernehmeSessionSnapshot` baut die Einträge je Snapshot neu (`std::map<std::string, Eintrag> neu;` `:1269`, Sonden mit `fluechtigImSnapshot = true` `:1279`, gespeicherte Nur-Mitglieder mit `false` und `Control::getrennt` `:1340-1355`, `eintraege = std::move (neu);` `:1356`). `controlEnde` (`:576-606`) setzt flüchtige Einträge nur auf `getrennt` und `stale` (`:580-586`). Ein neuer Snapshot kommt in legacy nicht, weil nicht abonniert wird (`Ipc.cpp:914-915`, `:991-995`, `:1033-1037`). Ergebnis: in legacy zeigt das Modell jeden flüchtigen Eintrag des letzten Snapshots weiter (unklassifiziert, getrennt, stale) und keinen gespeicherten Nur-Eintrag.

Flächenwechsel: `wechsleFlaecheWennNoetig` (`PluginEditor.cpp:960-1013`) schaltet mit `istMainFlaeche()` (`:955-958`, Rolle `hub` und nicht read-only); in legacy zeichnet `paint` die Legacy-Front statt `paintMainFlaeche` (`:1538-1555`), und der Timer ruft `sourcesTick` und `sourcesSicht` nur `if (mainFlaecheAktiv)` (`:271-284`). Die legacy-Fläche zeichnet die Quellenliste nicht.

Quelle der Zusage „in legacy keine Zeile im Quellenmodell": weder U49 (`…-u49.md:24-34`) noch der Entwurf (`:1873`, `:2953-2955`, `:4077-4080`) noch `docs/plugin-wissen.md` oder der Blueprint sagen etwas darüber. Annahme des Bauers.

**Weg A — rollenabhängige Stilllegung der Live-Sicht.**
- Wo: in der Publikation selbst. `setzeBindung` (`State.cpp:376-377`) und die Nachführungen (`Ipc.cpp:1534-1535`; `benenneSourcesHauptziel` `:1241`, `entferneSourcesHauptziel` `:1283`) geben dem Modell mit der Kopie die Klasse mit; bei `legacy` leert das Modell unter seinem `mutex`, im selben Schritt wie Generations- und Folgevergleich, Einträge, Hauptziel und Subscription und vergisst die erwartete Bindung und Sitzung (Muster `projektReload`, `SourcesModel.cpp:500-523`), damit ein verspäteter Snapshot oder P2-Frame des alten Links nichts mehr einträgt (`:1216-1222`, `:1257-1263`, `:1573-1575`). Sperrenordnung unverändert (Aufruf außerhalb `bindungMutex` wie heute). Nebenwirkung für den Bauer: ein verspäteter Snapshot fällt dann als ungültig aus und setzt die in legacy unsichtbare Diagnose `incompatible` (`Ipc.cpp:1098-1100`), die `beginneSubscription` bei der Rückkehr zurücksetzt (`SourcesModel.cpp:571`). Pfade: `SourcesModel.h`, `SourcesModel.cpp` (E-312-24), `State.cpp`, `Ipc.cpp` (Satz 3).
- Test: B14 mit echten Snapshot-Einträgen über `v3LinkFuerTest (true)` und `v3AntwortFuerTest (…)` (Muster `Sonde012ProjectReloadTest.cpp:135-143`, `:2879-2888`; Zugänge `PluginProcessor.h:540`, `:830-843`): vor dem Wechsel n Zeilen, nach `setzeBindung ("sensor", …)` 0 Zeilen und keine Subscription, ein verspäteter Snapshot der alten Sitzung trägt nichts ein, nach `setzeBindung ("hub", …)` die Mitglieder. Gegenprobe heute rot (`SourcesModel.cpp:435-441`); Mutation: Stilllegung auslassen → der flüchtige Eintrag bleibt → rot.
- Rückweg nach `main`: das Modell beginnt ohne angenommene Zeilen; die Annahmeregel vergibt die Plätze zuerst an gespeicherte Mitglieder in aufsteigender `instance_id` — dasselbe Ergebnis wie `projektReload`, das „ohne angenommene Zeilen" beginnt (`:9420-9421`). Live-Rückweg und gespeicherter Rückweg ergeben dieselben Zeilen; M-110, M-121 (d) und M-126 gelten wörtlich; M-110 und M-120 bleiben unverändert; §47.4 `:9752-9753` wird „die Live-Sicht wird stillgelegt".

**Weg B — die Zusage enger fassen.**
- M-110 (`:9081`): „das Quellenmodell zeigt 0 Zeilen" wird „kein Eintrag des Modells ist bestätigt, und keiner entsteht aus den gespeicherten Mitgliedern; flüchtige Einträge des letzten Snapshots bleiben unklassifiziert stehen (`SourcesModel.cpp:435-441`); die legacy-Fläche zeichnet die Liste nicht". M-120 (`:9091`): „in Legacy keine Zeile im Quellenmodell" wird „X wird in legacy keine bestätigte Zeile". §47.4 (`:9752-9753`): „(in `legacy` zeigt das Modell wie heute keine gespeicherten Mitglieder)". §47.5 B14 (`:9861-9862`) hat die enge Form schon („zeigt die Mitglieder in Legacy nicht"). §47.6 §1.4c (`:9883-9885`) bleibt. Der Test braucht trotzdem echte Snapshot-Einträge, sonst ist auch die enge Zusage ungemessen.
- Rückweg nach `main`: die stehenden flüchtigen Einträge sind nach C-1 weiter Kandidaten und behalten ihren Platz (`:9411-9413`). Bei mehr als 20 Kandidaten können sie nach der Rückkehr Plätze halten, die nach einem Neuladen gespeicherte Mitglieder bekämen; Live-Rückweg und gespeicherter Rückweg können sich dann in den Zeilen unterscheiden (nicht im Bestand). Bis 20 Kandidaten kein Unterschied; M-121 (d) und M-126 (ohne Live-Einträge) unberührt.

**Empfehlung: Weg A.** (1) In legacy frischt nichts die stehenden Einträge auf (`Ipc.cpp:991-995`, `:1033-1034`); sie sind veraltetes Wissen. (2) Nur A macht die Zeilen nach der Rückkehr unabhängig vom Live-Stand beim Wechsel und gleich denen nach einem Neuladen — U49 „live und über Speichern und Neuladen gleich" (`…-u49.md:24-26`). (3) M-110 und M-120 bleiben wörtlich. (4) Keine Produktwirkung, weil die legacy-Fläche die Liste weder zeichnet noch liest (`PluginEditor.cpp:271-284`, `:1538-1544`). Kosten: ein Klassenargument an der Publikation oder eine Modellfunktion und ein B14-Teilfall.

### 3. Zu D3: die konsistente K-A-Hälfte von M-96

Code (Zitate in D3 b): der Tap `postCommitted` ist `cAus` (`DspKern.cpp:1342`); beim Wechsel in einen Passthrough ist `cAus` während der Blende `vonL[i] * (1.0 - t) + nachL[i] * t` mit der rechnenden Bank als Quelle und dem Eingang als Ziel (`:1065-1071`, `:1093-1096`, `:1109-1115`), danach der Eingang (`:1117-1120`). §47.3 K-A (`:9552-9556`) sagt „Blende 256 Samples, danach … der Tap `post_committed` trägt den Eingang".

Konsistente Fassung: „Weg K-A: im ersten Bypassblock trägt der Tap `post_committed` während der 256 Samples der Blende die lineare Mischung aus dem Tap von R und dem Eingang, Sample n mit dem Gewicht n/256 für den Eingang (Sample 0: der Tap von R allein), ab Sample 256 und in jedem weiteren Bypassblock den Eingang, bitgleich zum Tap `pre_nakama` desselben Blocks (`DspKern.cpp:1341`)."

Messbar: ja. Am Kern (B6) über `tap (Tap::postCommitted, k)` und `tap (Tap::preNakama, k)` (`DspKern.h:359`), am Prozessor (A16) über `dspKernFuerTest()` (`SondeProcessor.h:347`), gegen R: nach der Blende bitgleich in `double`; in der Blende mit der E-31-Toleranz 2^−23 oder bitgleich, wenn der Test die Mischformel des Kerns nachrechnet (Kontraktion durch den Compiler nicht geprüft, deshalb die Toleranz als sichere Form). Rotbeweis der Inhaltshälfte: für K-B die Stufe vor die Taps legen → Tap weicht von R ab → rot; für K-A den Tap in der Blende sofort auf den Eingang setzen → Abweichung in Block 41 → rot.

Vorbehalt statt Präzisierung: vereinbar mit E-312-19 („die Matrixnacharbeit oder der Bauer streicht nach der Antwort die nicht gewählte", `:10195-10196`), aber E-312-19 macht beide Hälften ausdrücklich zum Prüfgegenstand (`:10194-10195`). Wählt der User K-A, müsste die Hälfte vor dem Bau von Satz 2 präzisiert und in einer eigenen Runde geprüft werden. Ein Satz jetzt ist kleiner. M-95 und M-100 sind in ihren K-A-Hälften konsistent: M-95 fordert Bitgleichheit erst „ab Sample 256" (`:9044`), M-100 setzt „der Hostbypass steht seit mehr als 256 Samples" voraus (`:9049`). Empfehlung: M-96 jetzt präzisieren.

### 4. Zu D4: welche Hälfte von M-99 und M-107 heute gilt

`juce_AudioProcessor.cpp:591-606` (Bauartefakt vorhanden; Zitat in D4 b): die JUCE-Basis prüft nur die Latenz per `jassert` und leert Ausgangskanäle oberhalb der Eingangskanäle; bei gleicher Busbelegung schreibt sie keinen Sample. `SondeProcessor.cpp:610-666`: Kern, Zähler `verarbeiteteSamples` und alle drei Analysezweige laufen nur in `processBlock`.
- Heute gilt: die Audiohälfte beider Zeilen (bytegleich, kein Sample geschrieben, Zähler für nicht endliche Eingänge und geheilte Zustände still) — im Bypasseintritt über die JUCE-Basis, in `processBlock` über `committedRuht` und `:1201`.
- Heute gilt nicht: die Analysehälfte (M-99 „die Analyse läuft in beiden Eintritten wie heute über `processBlock`", M-107 „die Analyse verbucht ihn wie heute ohne Tap") — im Bypasseintritt läuft kein Zweig von `:613-666`.
- Rotbeweis der Analysehälfte: Gegenprobe am Basisstand — die Blockzählung `bloeckeAngenommen() + dropsUeberlauf() + dropsOversize() + dropsOhneAudio()` (`StampedAudioQueue.h:574-584`; lesbar über `mitAngehaltenerAnalyseFuerTest`, `SondeProcessor.h:409-414`, und `analyseDrops…FuerTest`, `:386-398`) steigt über N Bypassblöcke um 0 statt um N → heute rot. Nach dem Bau die Mutation an der Zusagezeile: im Bypasseintritt den Analyseteil des gemeinsamen Rumpfs (die Entsprechung von `:613-666`) auslassen → 0 → rot; für Fall (c) den Zweig ohne Tap (Entsprechung von `:646-654`). Die Double-Mutation bleibt der Rotbeweis der Audiohälfte.

### 5. Zu D5: ein deterministischer Aufbau für die Ruhegrenze in M-103

Quellen: Ruhegrenze `SondeProcessor.cpp:1834-1839` und Zähler `SondeProcessor.h:776-777` (Zitate in D5 und D6 b). SONDE-015 M-81 (`docs/beweise/SONDE-015.md:1159`): „… Die Epoche wechselt einmal zu Beginn und einmal am Ende einer Hostgeste beziehungsweise nach einer definierten Ruhegrenze."; E4-11 (`:3951`): „… Die Epoche wechselt beim ersten Punkt und nach 0,25 s **verarbeiteten Audios** ohne neuen Punkt. …". Testzugänge: `kontrollTaktFuerTest()` (`SondeProcessor.h:349-351`, „Derselbe Code wie im Worker") und die Taktsperre `mitAngehaltenemTaktFuerTest` (`:415-428`: „der Worker kann in diesem Fenster also beweisbar nicht ticken"). Vorbild 312/M-24 (`eq-copilot/plugin/tests/TransactionTestMain.cpp:2350-2381`: 9 600 Samples „unter der Ruhegrenze", 12 480 „das Ende").

Aufbau:
1. Aufbau HB (Block 256, 48 kHz, Ruhegrenze 12 000 Samples), 40 Blöcke `processBlock`.
2. Innerhalb der Taktsperre: Hostpunkte auf `v1.global.output_trim_db` im Muster 312/M-24 (Hostwert schreiben, ein Block `processBlock`, `kontrollTaktFuerTest()`), Epoche e1 = e0 + 1; der letzte Punkt setzt den Bezugspunkt (`:1824`).
3. Unmittelbar danach nur `processBlockBypassed`: nach 46 Blöcken (11 776 Samples) `kontrollTaktFuerTest()` → Epoche e1 (offen); nach dem 47. Block (12 032 Samples) `kontrollTaktFuerTest()` → e1 + 1 (geschlossen).
4. Danach die übrigen 13 Bypassblöcke und 40 Blöcke `processBlock` für die Zustandshälfte (Bytes, Hash, Revision, Undo-Ring, Dirty).

Das Rot fällt an der Zusagezeile: am Basisstand (JUCE-Basis) und unter dem Mutanten „`verarbeiteteSamples` im Bypasseintritt nicht erhöhen" steht die Epoche nach Block 47 auf e1. Ohne Taktsperre genügt ein Abstand von einem Block zur Grenze (offen nach 45 Blöcken, 11 520 + 256 < 12 000; geschlossen nach 47, 12 032 ≥ 12 000), weil ein Workertakt vor dem letzten Block den Bezugspunkt höchstens einen Block früher legt. „Wie ohne ihn": derselbe Ablauf mit R nur über `processBlock` schließt am selben Block.

### 6. Zu D6: welche Zähler Satz 2 berührt, und wie eine ehrliche Zeile aussieht

| Zähler | Breite (Quelle) | durch Satz 2 berührt | Wrap erreichbar | Vergleich |
|---|---|---|---|---|
| `verarbeiteteSamples` | `std::atomic<std::uint64_t>` (`SondeProcessor.h:776`) | ja, zählt auch im Bypass (M-103, `:9052`) | nein: 2^64 Samples sind bei 48 kHz rund 1,2 · 10^7 Jahre, bei 192 kHz rund 3 · 10^6 Jahre | Differenz vorzeichenlos modulo 2^64 (`SondeProcessor.cpp:1837-1838`), über einen Wrap richtig, solange der Abstand unter 2^64 liegt |
| `samplesBeiLetzterAutomation` | `std::uint64_t` (`:777`) | mittelbar (Bezugspunkt) | wie oben | Subtrahend derselben Differenz |
| `hostEreignis`, Blockrandstand | `std::atomic<std::uint32_t>` (`SondeProcessor.h:753`), `std::uint32_t` (`DspKern.h:250`, `:260`) | ja, der gemeinsame Rumpf liest die Hostmailbox auch im Bypass (`:9587-9589`) | ja, bei Dauerautomation nach Wochen (§21.11: „über 60 Tage", `:3461-3467`) | nur Gleichheit (`SondeProcessor.cpp:584`, `:591`); über den Wrap gemessen für `processBlock` (312/M-19, 312/M-82) |
| Queue-Zähler | `std::uint64_t` (`StampedAudioQueue.h:574-584`) | ja (M-96) | nein | monoton, Telemetrie |
| Kernzähler | `std::atomic<std::uint64_t>` (`DspKern.h:753-759`) | ja (M-100, K-B) | nein | monoton |
| Blendenrest der Stufe | neu; nach dem Muster `int hoerFadeRest` (`DspKern.h:749`) mit Umkehr `std::max (0, kFadeSamples - hoerFadeRest - 1)` (`DspKern.cpp:1382`), begrenzt auf 0 bis 256 | neu | nein | begrenzt |
| Bypassblockzähler | im Bauplan nicht vorgesehen (§47.3: „Ziel, Restlänge, „ohne Verlauf"", `:9594-9595`) | — | — | nicht gefunden |

Testzugänge: für `verarbeiteteSamples` keiner (Grep); für die Hostereigniszähler `setzeHostZaehlerFuerTest (int index, std::uint32_t stand)` (`SondeProcessor.h:433-445`: „für den Zählerrand dicht unter dem Überlauf").

- **(A) Wrap per Testzugang.** Für die uint32-Hostzähler ohne neuen Zugang: Zähler auf 2^32 − 2, Hostpunkte über den Wrap, dazwischen Bypassblöcke; Zusage: der Blockrand des Bypasseintritts übernimmt jeden Punkt über den Wrap; Gegenprobe heute rot (im Bypass läuft kein Blockrand); Mutation an der Zusagezeile: Ordnungs- statt Ungleichheitsvergleich (`SondeProcessor.cpp:584`) → rot, wie 312/M-82. Für den uint64-Samplezähler bräuchte (A) einen neuen Testzugang in `SondeProcessor.h` (Ticketpfad von Satz 2); Mutation an der Ruhezeile etwa „Summe statt Differenz" (`verarbeiteteSamples >= samplesBeiLetzterAutomation + grenze`, die Addition läuft über, die Epoche schließt sofort) → rot.
- **(B) Feststellung ohne eigene Mutation.** Für den uint64-Samplezähler: Breite, Unerreichbarkeit, modulo-sichere Differenz, benannt. Vorbilder im Manifest: „benannt, nicht gebaut" für einen uint64-Rand (§21.11, `:3463-3467`) und „messend und trägt keine eigene Mutation" (E-312-12, `:2449-2452`).
- **Ehrlich am Code:** für den uint64-Samplezähler (B) — der Wrap tritt in keiner Sitzung ein, und ein Setter prüfte nur eine Arithmetik, die heute schon modulo-richtig ist; für die erreichbaren uint32-Hostzähler, die Satz 2 in den Bypasseintritt holt, (A) mit dem vorhandenen Zugang. Empfehlung: eine Zeile mit diesen zwei Hälften.

### 7. Zu D7: welche Mutation A und B auseinanderlaufen lässt

`DspKern.cpp:1216-1236` (Zitat in D7 b) und Aufbau HB (`:9023-9031`, Block 256, keine Vorschau, Hörmatrix Processed).

- **Wirksam für A ≠ B:** am harten Zweig `DspKern.cpp:1227-1236` (`if (hart) { hoerVorher = wirksam; hoerLaufend = wirksam; hoerFadeRest = 0; }`) zusätzlich die Blende der Hostbypass-Stufe beenden (Restlänge auf 0). `hart` verbraucht nur B (Wechsel nach offline, `SondeProcessor.cpp:1587-1591`; Verbrauch `DspKern.cpp:1216-1217`); A blendet weiter. Voraussetzung: Blockgröße unter 256 (etwa 64, die Blende läuft über vier Blöcke) und B bis zum ersten Bypassblock in Echtzeit, `setNonRealtime (true)` erst vor dem zweiten → unter der Mutation springt B ab Sample 0 dieses Blocks auf das Ziel der Stufe, A blendet weiter → rot; ohne Mutation bleiben beide bitgleich, weil das harte Schalten keine laufende Hörmatrixblende vorfindet.
- **Die Richtung „der Bypasseintritt setzt Riegel und hartes Schalten"** (`setzeOfflineRiegel (true)` im neuen Eintritt, Zeile: die Override `processBlockBypassed` in `SondeProcessor.cpp`) zeigt der Vergleich A gegen B nicht. Sie fällt an einer direkten Beobachtung am Echtzeitprozessor: nach Bypassblöcken `dspKernFuerTest().offlineRiegel() == false` (`DspKern.h:353`) und `wirksameHoermatrix()` gleich dem Wunsch (`DspKern.h:316`). Unter der Mutation bleibt der Riegel in A stehen, weil `setNonRealtime (false)` ohne Wechsel sofort zurückkehrt (`SondeProcessor.cpp:1573-1575`).

### 8. Zu D8: welche Beobachtung die Verweigerung messbar macht

`State.cpp:305-381` und `NakamaState.cpp:2165-2168` (Zitate in D8 b). Unter der Mutation „Wache hinter `zustand.common = neu` (`:338`)":

| Beobachtung | unter der Mutation | fällt |
|---|---|---|
| Rückgabewert von `setzeBindung` | `false` wie verlangt | nein |
| Dirty-Zähler | 0 (Rückkehr vor `:376-377`, `meldeHostDirty` nur in `Ipc.cpp:1488`) | nein |
| Publikation ans Modell | keine (`sourcesSicht().revision` unverändert) | nein |
| `getStateInformation` | `originalBytes` (`NakamaState.cpp:2165-2168`) | nein |
| In-Memory-Bindung | `zustand.common` geändert: `holeRolle()`, `holeLabel()`, `holePaarId()` (`State.cpp:226-228`), `holeZustandKopie().common` (`:254-258`) liefern die neuen Werte | ja |
| Klassifikation | unverändert (`spiegleKlassifikation` an `:362` wird nicht erreicht) | nein |

Messbar macht die Verweigerung also die In-Memory-Bindung (Rolle, Label, Paar-ID, Klasse, Projektbindung) zusammen mit den sieben Beständen aus `holeZustandKopie()`. An genau dieser Beobachtung scheitert die Mutation „Wache hinter `:338` ziehen" und jede andere Lage bis `:362`. Die Mutation „Wache `:323-324` entfernen" scheitert zusätzlich am Rückgabewert und am Dirty-Zähler.

### 9. Zu D9: grüne und rote Hälften von M-127 und M-128, und die Gegenprobe

- Heute grün: die Ersatzzielhälfte von M-127 (`stelleZielSicher`, `SourcesModel.cpp:1783-1798`; M-73 per Fixture) und die Klickhälfte für die 20 gezeichneten Zeilen; das Überspringen unbekannter IDs in `uebernehmeP2` (`:1576-1579`).
- Heute rot: die Kapazitätshälfte von M-127 (21/32/64 Sichtzeilen gegen 20 Rechtecke, `:1670-1671` gegen `PluginEditor.cpp:1144-1160`) und M-128 als Ganzes (X aus dem 21er-Snapshot ist heute ein Eintrag, `:1279`; Frames `:1576-1626`, Befund `:1716-1721`).
- Gegenprobe M-127 (B15, deterministisch, reine Geometrie ohne Uhr): echter Main (`setzeEditorOffen (true)`, `setzeBindung ("hub", …)`), `v3LinkFuerTest (true)`, `v3AntwortFuerTest (…)` mit einem Snapshot aus 21, 32 und 64 Sondenmitgliedern, adressiert mit `v3HelloFuerTest()` (Muster `Sonde012ProjectReloadTest.cpp:135-143`, `:2879-2888`); Editor 760×430, ein Tick → `sourcesSicht().quellen.size()` ist 21/32/64, `sourcesZeilenFuerTest().size()` ist 20 → die Zusage „gleich" fällt.
- Gegenprobe M-128 (B13): `beginneSubscription`, 21er-Snapshot über `uebernehmeSessionSnapshot` (`mitgliedJson`, `Sonde012SourcesModelTest.cpp:97-125`), ein P2-Frame für X → `uebernehmeP2` liefert `true`; ein Snapshot mit einem Befund, dessen `candidate_source` X ist (`befundJson`, `:131-148`) → die Zeile X trägt `findingsOffen == 1` → die Zusage fällt.

## Übersicht

| Befund | Urteil | Einordnung | schließende Regel in einem Satz |
|---|---|---|---|
| D1 | BESTÄTIGT | DEFEKT | Satz 1 stellt B13 `visible_latency_16_and_32_sources` und A23 auf „min(n, 20) angenommen, Rest nicht angenommen" um, führt A23 als Einzelbein und zieht die Behauptungszeilen A23 und B13 nach. |
| D2 | BESTÄTIGT | DEFEKT | Satz 3 legt die Live-Sicht beim Wechsel weg von main rollenabhängig still (empfohlen) oder fasst M-110, M-120 und §47.4 enger, in beiden Fällen gemessen mit echten Snapshot-Einträgen. |
| D3 | PRÄZISIERT | DEFEKT | Die K-A-Hälfte von M-96 legt den Tap-Inhalt des ersten Bypassblocks fest (Blende: lineare Mischung, danach der Eingang), und die Inhaltshälften bekommen einen eigenen Rotbeweis — oder K-A wird ausdrücklich Vorbehalt bis U58. |
| D4 | BESTÄTIGT | DEFEKT | M-99 und M-107 tragen zwei Hälften: Audio als Regressionswache, Analyse als „heute rot" mit Gegenprobe und Mutation am Analyseteil des Bypasseintritts. |
| D5 | BESTÄTIGT | DEFEKT | Die Ruhegrenzenhälfte von M-103 misst die Epoche mit festem letztem Hostpunkt und ausdrücklichen Kontrolltakten innerhalb des Bypassabschnitts vor und nach 12 000 Samples. |
| D6 | PRÄZISIERT | DEFEKT | Satz 2 bekommt eine Zeile „Zählerränder im Hostbypass": uint32-Hostzähler-Wrap deterministisch mit Rotbeweis, uint64-Samplezähler als unerreichbar und modulo-sicher benannt. |
| D7 | BESTÄTIGT | DEFEKT | M-104 misst „der Eintritt setzt nichts" direkt am Echtzeitprozessor und „hartes Schalten lässt die Stufe weiterlaufen" mit Blockgröße unter 256 und Offline-Wechsel mitten in der Blende. |
| D8 | BESTÄTIGT | DEFEKT | M-117 beobachtet nach jeder Verweigerung die In-Memory-Bindung und den Bestand, sodass die Mutation „Wache hinter `zustand.common = neu`" fällt. |
| D9 | BESTÄTIGT | DEFEKT | M-127 wird „zwei Hälften" (Kapazität heute rot mit Gegenprobe), M-128 wird „heute rot" mit Gegenprobe am 21er-Snapshot, §46.6 und §47.2 zählen neu. |

## Nicht geprüft

- Kein Test, kein Kanon, kein Build gefahren; jede Aussage „wird rot" oder „bleibt grün" ist aus dem Kontrollfluss abgeleitet.
- Geprüft sind die neun Befunde des Prüfberichts, keine eigene vollständige Matrixprüfung von M-94 bis M-131. Zwei Nebenbeobachtungen stehen bei D3 (M-96: Inhaltshälften ohne eigenen Rotbeweis) und D7 (M-104: der Teilfall „Wechsel nach offline mitten in der Blende" ist bei Block 256 nicht herstellbar); sie sind nicht als eigene Befunde eingeordnet.
- Ob die Mischformel der K-A-Blende im Test bitgleich nachrechenbar ist (Kontraktion durch MSVC), ist nicht geprüft; deshalb die Toleranzform in Teil 2 Punkt 3.
- Der JUCE-VST3-Wrapper ist nur an `:3884-3910` gelesen; wie JUCE einen Bypasswechsel innerhalb eines Blocks auf den Blockeintritt abbildet, nicht.
- FLs Verhalten beim Slot-Bypass (NAK-366), die Brokerseite nicht angenommener Quellen (NAK-367) und Gens Hostbypass (NAK-365): nicht Gegenstand.
- `tools/eq-copilot/pruefe_session_soak.py` nur an den Stellen zu `--sonden`; `SessionSoakMain.cpp` nur an den Stellen mit `anzahl` und `quellen.size()`.
- Die Sperrenfolge einer Stilllegung nach Weg A (Teil 2 Punkt 2) ist als Vorschlag beschrieben, nicht gegen alle Publikationswege nachgemessen.

FERTIG Validierung Matrixprüfung Etappe 7 Runde 1, 7 bestätigt, 2 präzisiert, 0 widerlegt
