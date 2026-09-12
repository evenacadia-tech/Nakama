# NAK-283 Matrixprüfung 3 — Quellvalidierung des einen Restpunktes (lesend)

| Merkmal | Wert |
|---|---|
| Validierer | Lesender Opus-Agent, 12.09.2026, nur Quellen gelesen; nichts gebaut, nichts gefahren, Git nur lesend. |
| Gelesener Stand | Worktree an `e82572ef` (ein Commit nach dem geprüften `467e4534`). `git diff --stat 467e4534 e82572ef -- docs/beweise/NAK-283.md` = **13 +, 1 −**, betroffen nur die Kopfzeile „Etappe" und der angehängte §16. **§5 und §6 sind zeilengleich mit dem geprüften Stand**; alle Zeilennummern unten gelten für beide. Produktcode unverändert seit `ea6ddd57`. |
| Gegenstand | Punkt 1 der Matrixprüfung 3 (Codex `gpt-6-astra` max, Thread `01a095eb`): M-13, Rotbeweis nach bestandenem Frischevergleich. |
| Ergebnis | **BESTÄTIGT, DEFEKT** — siehe §5. |

---

## 1. Der Befund wörtlich

Quelle: `docs/beweise/roh/NAK-283-matrixpruefung-3-467e453.txt`, Zeilen 9 bis 10.

```text
9: - [P2] Erzwinge den M-13-Wettlauf nach bestandenem Frischevergleich — C:/Users/phili/Projekte/Nakama/docs/beweise/NAK-283.md:995-997
10:   Im festgelegten Ablauf von M-13/Fenster 2 persistiert B vollständig, bevor A den Wiedervergleich ausführt. A wird deshalb bereits dort verworfen: Die Mutation „Lock zwischen Vergleich und `append_einreihen` freigeben“ bleibt bei diesem Ablauf grün. Der hier und in §6.1 benannte Haken sitzt weiterhin vor `befund_persistieren` (`hypothese_verdrahtung.rs:478–480`) und kann das neue Fenster nicht erzwingen. Die Lockkopplung selbst schließt den ursprünglichen Fehler, aber ihr geforderter [deterministischer Rotbeweis](NAK-283.md#L518-L523) fehlt. Dafür braucht es einen zusätzlichen Ablauf nach bestandener Prüfung und ein passendes Erfolgskriterium: A darf zuerst angenommen werden, anschließend muss B die jüngere Projektion bestimmen.
```

## 2. Die geprüften Stellen wörtlich

### 2.1 Die Matrixzeile M-13 (`docs/beweise/NAK-283.md:616`, §5.2)

```text
616: | M-13 | F02 · R-283-1 | Rechnung A hat unter dem Standlock 12 Evidence-IDs eingesammelt (`aufnahmen_sammeln` nimmt den Standlock an `hypothese_verdrahtung.rs:130-131`) und rechnet ohne Lock; der bestehende Rechenhaken `rechen_test_haken_ausloesen` (`:91`) hält sie | **Zwei Fenster derselben Naht.** (1) Ein zweiter `eqcop-v3-conn`-Thread liefert Evidenz; Rechnung B sammelt 13 IDs, rechnet, trägt ein und wird zugestellt; danach wird A freigegeben und will eintragen. (2) A hat den Riegel genommen, steht bereits in `stand.befunde`, der Standlock ist gefallen (`:478`), und A hängt am **neuen** Persistenzhaken vor dem Persistenzlauf (`:479-481`); erst dann sammelt, rechnet, trägt ein und **persistiert** B vollständig; danach wird A freigegeben und nimmt den Standlock | **Die Frischeprüfung reicht bis zur Persistenz und Publikation, nicht nur bis zum Cache.** (1) `stand.befunde` trägt das Ergebnis von B (13 IDs); A ist verworfen und hat `befunde_neu_bilden` gesetzt. (2) **A schreibt nicht, und wer später einreiht, gewinnt:** das `event_log` trägt keinen `finding`-Event mit A's Payload, die Projektion (`findings.state_jcs`) trägt B's 13-ID-Payload mit der höheren Ordnungszahl, flüchtiger Stand und Projektion sind gleich, und A's unterlassener Schreibversuch ist gezählt | **Ein Riegel an zwei Stellen, beide unter dem Standlock.** (1) `ergebnis_ist_noch_gueltig` (`:961-1000`) vergleicht beim Rückschreiben zusätzlich die **Menge der beim Sammeln vorhandenen stabilen Evidence-IDs** gegen den aktuellen Stand, unter demselben Standlock wie `befunde_eintragen` (`:443-477`). (2) `befund_persistieren` (`:494-515`) vergleicht **und reiht unter derselben Sperre ein**: Standlock nehmen → Payload gegen den Standeintrag der Sitzung vergleichen (derselbe Vergleich wie `:465`) → `StoreHandle::append_einreihen` (`broker/src/store/handle.rs:156-171`) **am gehaltenen Guard** → Sperre freigeben → `recv` auf den Antwortkanal und dessen Fehlerbehandlung außerhalb. Die Annahme vergibt die Ordnungszahl: ein einziger Writer-Thread liest den FIFO (`store/writer.rs:176-193`), merkt weitere Aufträge mit `aktiv.push_back` vor (`:298`) und schreibt sie in derselben Reihenfolge (`:325-334`), sodass `last_event_ord = tx.last_insert_rowid()` (`:476`) der **Annahme**reihenfolge folgt und die Projektion die höhere behält (`:588-592`). **Kein Deadlock:** die Annahme blockiert nie — `senden` benutzt `SyncSender::try_send` und meldet bei vollem Kanal sofort `StoreFehler::KanalVoll` (`handle.rs:219-248`; `STORE_KANAL_CAP = 256`, `store/mod.rs:48`, Kanal an `store/writer.rs:53`) —, ihr Fehlerzweig zählt am gehaltenen Guard statt über ein zweites `lock()`, und der Lockpfad `:511-514`, der den Deadlock erzeugte, liegt hinter dem `recv` und damit hinter der Freigabe. Kein Pfad unter `broker/src/store/**` nimmt den Standlock — die Ordnung Standlock → Store-`sicht` (`handle.rs:234`, `:251`) kann sich nicht drehen. Frist: der Verwurf löst sofort einen Heilungstakt aus | `broker/tests/sonde014_verdrahtung.rs`, **NEU** `neue_evidenz_waehrend_der_rechnung_verwirft_das_aeltere_ergebnis` (Fenster 1) und **NEU** `aelterer_payload_wird_nach_dem_cacheeintrag_nicht_persistiert` (Fenster 2, gemessen an `event_log` **und** `findings`-Projektion), Bein **A4** | Je Fenster eine Mutation an der Zeile, die die Zusage trägt. (a) An `:961-1000`: den Vergleich der Eingangsmenge entfernen → A setzt den Bestand von B auf 12 IDs zurück. (b) In `befund_persistieren` die Sperre **zwischen Vergleich und `append_einreihen` freigeben** (die Fassung der Nacharbeit 1) und B in dieser Lücke laufen lassen → A besteht den Vergleich gegen den Stand vor B, reiht **nach** B ein, bekommt die höhere Ordnungszahl und gewinnt die Projektion (`store/writer.rs:476`, `:588-592`), weil `finding_id` (`coordinator/hypothese/befund.rs:353-372`) die Eingangsmenge nicht trägt. **ROT am Basis-SHA für (a)**; **(b) ist am Basis-SHA nicht deterministisch messbar**, weil Persistenzhaken und Wiedervergleich erst mit dieser Etappe entstehen — der Fehler selbst liegt dort heute offen | §2 R-283-1; §4 F02 (NAK-253); Audit-Korrekturziel `UEBERGABE-AN-CLAUDE.md:88` („bis zu Ergebnisübernahme, Persistenz und Publikation"); Matrixprüfung 1 Befund 3; Matrixprüfung 2 Punkt 1 (§13.1) | 3 |
```

### 2.2 §6.4, die Absätze zu `hypothese_verdrahtung.rs` (Zeilen 950 bis 997)

```text
950: **Änderungen je Datei, in Sätzen.**
951: 
952: - `hypothese_verdrahtung.rs` (Fenster 1, Cache): `aufnahmen_sammeln` gibt
953:   zusätzlich die **Menge der stabilen Evidence-IDs** zurück, die zum
954:   Sammelzeitpunkt gültig waren. `ergebnis_ist_noch_gueltig` vergleicht diese
955:   Menge beim Rückschreiben gegen den aktuellen Stand; weicht sie ab —
956:   hinzugekommen **oder** verworfen —, fällt das Ergebnis. Der Verwurf setzt
957:   `befunde_neu_bilden` in **jedem** Fall, nicht nur im bisherigen Zweig
958:   `:464`.
959: - `hypothese_verdrahtung.rs` (Fenster 2, Persistenz — M-13, R-283-1 reicht bis
960:   Persistenz und Publikation): `befund_persistieren` (`:494-515`) bekommt
961:   **Vergleich und Store-Annahme unter dieselbe Sperre**. Ablauf in dieser
962:   Reihenfolge: Standlock nehmen → prüfen, ob der eigene Payload noch der
963:   Standeintrag der Sitzung ist (derselbe Vergleich wie `:465`) → bei
964:   Ungleichheit nicht schreiben, eigenen Zähler erhöhen, `befunde_neu_bilden`
965:   gesetzt lassen → sonst `StoreHandle::append_einreihen`
966:   (`broker/src/store/handle.rs:156-171`) **am gehaltenen Guard** aufrufen und
967:   einen Annahmefehler direkt an diesem Guard zählen → Sperre freigeben → erst
968:   danach `recv` auf den Antwortkanal und dessen Fehlerbehandlung. Der heutige
969:   Aufruf `store.append` (`handle.rs:148-151`) ist genau die Summe aus beidem;
970:   er wird aufgetrennt, nicht geändert.
971:   **Warum das trägt, an der Quelle:** (a) *Die Annahme blockiert nie* —
972:   `senden` benutzt `SyncSender::try_send` und meldet bei vollem Kanal sofort
973:   `StoreFehler::KanalVoll` (`handle.rs:219-248`; `STORE_KANAL_CAP = 256`,
974:   `store/mod.rs:48`, Kanal angelegt in `store/writer.rs:53`). (b) *Kein
975:   Deadlock* — der Deadlock der Fassung aus Runde 1 entstand allein daraus,
976:   dass `befund_persistieren` den Standlock im Fehlerpfad **erneut** nimmt
977:   (`:511-514`, `std::sync::Mutex`, nicht reentrant); dieser Pfad liegt nach der
978:   Auftrennung hinter dem `recv` und damit hinter der Freigabe. Kein Pfad unter
979:   `broker/src/store/**` nimmt den Standlock — die Ordnung Standlock →
980:   Store-`sicht` (`handle.rs:234`, `:251`) kann sich nicht drehen. (c) *Die
981:   Annahme ordnet* — ein einziger Writer-Thread liest den FIFO
982:   (`store/writer.rs:176-193`), merkt weitere Aufträge mit `aktiv.push_back`
983:   vor (`:298`) und schreibt sie in derselben Reihenfolge (`:325-334`), sodass
984:   `last_event_ord = tx.last_insert_rowid()` (`:476`) der Annahmereihenfolge
985:   folgt und die Projektion die höhere behält (`:588-592`). Wer später einreiht,
986:   gewinnt — und unter der gemeinsamen Sperre reiht der später ein, der den
987:   Vergleich später besteht. (d) *Der Doc-Kommentar der Annahmekante verlangt
988:   genau diese Aufteilung*: „Der Aufrufer darf **zwischen diesen Schritten**
989:   insbesondere keinen Coordinator-Lock halten" (`handle.rs:153-155`) — gemeint
990:   ist das Warten, nicht die Annahme.
991:   **Was weiterhin nicht geht:** eine Ordnungsmarke im Event erreicht die
992:   Projektion nicht, weil `last_event_ord` die SQLite-`rowid` des Anhängens ist
993:   (`store/writer.rs:476`, verwendet in `:588-601`) und nicht die `sequence` des
994:   Events. Der Riegel bleibt deshalb im Coordinator, und `broker/src/store/**`
995:   bleibt unberührt. Zwischen Cache-Eintrag und Persistenzlauf (`:479-481`)
996:   entsteht ein Testhaken für das erzwungene Interleaving, im Produkt leer —
997:   Muster `rechen_test_haken_ausloesen` (`:91`).
```

### 2.3 §6.1 Rotbeweis-Ablauf, Schritt 2 mit der Hakenliste (Zeilen 796 bis 805)

```text
796: 1. **SHA-256 der unveränderten Quelldatei** notieren
797:    (`Get-FileHash -Algorithm SHA256`).
798: 2. **Mutation** genau an der Zeile, die die Zusage trägt — nie an einem
799:    Nebeneffekt. Bei Nebenläufigkeitszeilen (M-01 bis M-03, M-05, M-13, M-17,
800:    M-20 Hälfte 2, M-21, M-25, M-26, M-72) wird zusätzlich das Interleaving
801:    über den bestehenden bzw. den neuen Testhaken **erzwungen**; eine Zeile,
802:    die nur „manchmal" rot wird, ist kein Rotbeweis. Neue Haken dieser Runde:
803:    der Persistenzhaken in `hypothese_verdrahtung.rs` zwischen Cache-Eintrag
804:    und `befund_persistieren` (M-13) und der `cfg(test)`-Haken zwischen
805:    Entfernen und Anhängen in `queues.rs` (M-20).
```

### 2.4 §5.2 Matrixregel zum Rotbeweis (Zeilen 518 bis 523) — die Regel, die Codex zitiert

```text
518: - Ein Rotbeweis fällt an der Zeile, die die Zusage trägt, nie an einem
519:   Nebeneffekt. Nebenläufigkeits- und Lebensdauerbeweise fallen an einem
520:   **deterministisch erzwungenen** Interleaving (Testhaken, Barriere,
521:   Kanarienvogel), nie an einer Wahrscheinlichkeit; Persistenzbeweise am
522:   **geladenen** Zustand einer neuen Instanz; Ordnungsbeweise am **zuletzt
523:   angenommenen** Zustand des Empfängers; Zeitbeweise am **exakten Frame**.
```

### 2.5 §8.1 Feinheit 18, Absatz „Warum die Kopplung nötig ist" (Zeilen 1721 bis 1733)

```text
1721: **Warum die Kopplung nötig ist.** Ein Vergleich, der die Sperre vor dem Append
1722: wieder freigibt (die Fassung der Nacharbeit 1), lässt ein Fenster offen: A
1723: besteht den Vergleich, hält an, B rechnet, reiht ein und persistiert, danach
1724: reiht A ein — und gewinnt die Projektion, weil sie nach `last_event_ord`
1725: ordnet (`store/writer.rs:588-592`) und das die `rowid` des **Anhängens** ist
1726: (`:476` `tx.last_insert_rowid()`), nicht die `sequence` des Events; `finding_id`
1727: (`coordinator/hypothese/befund.rs:353-372`) trägt die Eingangsmenge nicht.
1728: Genau dieses Fenster hat die Matrixprüfung 2 gefunden (§13). Es schließt sich
1729: nur, wenn der Vergleich und die **Annahme** — nicht das Commit — dieselbe
1730: Sperre teilen: die Annahme vergibt die Ordnungszahl (ein Writer-Thread, FIFO,
1731: `store/writer.rs:176-193`, `:298`, `:325-334`), also gewinnt, wer später
1732: einreiht, und unter der gemeinsamen Sperre reiht der später ein, der den
1733: Vergleich später besteht.
```

### 2.6 §6.4 Ticketpfade der Etappe 3 (Zeilen 937 bis 948)

```text
937: **Ticketpfade** (aus §3, präzisiert):
938: `broker/src/coordinator/hypothese_verdrahtung.rs`
939: (`hypothesen_bilden` `:66-93`, `befunde_eintragen` `:443-477`, Persistenzlauf
940: `:479-481`, `befund_persistieren` `:494-515`,
941: `ergebnis_ist_noch_gueltig` `:961-1000`),
942: `broker/src/coordinator/invalidierung_verdrahtung.rs`
943: (`invalidierung_vorbereiten` `:139`, `:155-176`, Persistenzrahmen `:88-92`,
944: `invalidierung_ruecknehmen` `:230-259`),
945: `broker/src/coordinator/evidenz.rs` (Retention `:179-189`, Auslöser `:232`),
946: `broker/src/transport/server_v3/queues.rs` (`einreihen_eintrag` `:202-238`,
947: `mod tests`), `broker/tests/sonde013_verdrahtung.rs`,
948: `broker/tests/sonde014_verdrahtung.rs`.
```

---

## 3. Quellzitate (Code, Stand `ea6ddd57` = Worktree)

### 3.1 `broker/src/coordinator/hypothese_verdrahtung.rs` — der Lockrahmen und der Persistenzlauf

```rust
443:     fn befunde_eintragen(&self, session: &SessionKey, befunde: Vec<CauseHypothesis>) -> bool {
444:         let neue: Vec<CauseHypothesis> = {
445:             let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
446:             // 🔑 NR-03 (Nacharbeit 1, 07.09.2026): das Ergebnis wird UNTER
463:             if !Self::ergebnis_ist_noch_gueltig(&stand, session, &befunde) {
464:                 stand.befunde_neu_bilden = true;
465:                 return false;
466:             }
467:             let alt = stand.befunde.get(session);
468:             if alt.map(Vec::as_slice) == Some(befunde.as_slice()) {
469:                 return false;
470:             }
471:             if befunde.is_empty() {
472:                 stand.befunde.remove(session);
473:                 Vec::new()
474:             } else {
475:                 stand.befunde.insert(session.clone(), befunde.clone());
476:                 befunde
477:             }
478:         };
479:         for befund in &neue {
480:             self.befund_persistieren(session, befund);
481:         }
482:         // 🔑 NR-04: „beim Eintragen eines Befunds" — die zweite der beiden
483:         // Stellen, an denen M-28 die Existenz gegen den Store haelt.
484:         self.befunde_gegen_store_haerten(session);
485:         true
486:     }
```

### 3.2 `befund_persistieren` heute (`:494-515`) — ein `store.append`, kein Vergleich

```rust
494:     fn befund_persistieren(&self, session: &SessionKey, befund: &CauseHypothesis) {
495:         let Some(store) = self.store.as_ref() else {
496:             return;
497:         };
498:         let payload = Self::befund_json(befund);
499:         let Ok(payload_jcs) = serde_json_canonicalizer::to_vec(&payload) else {
500:             return;
501:         };
502:         let sequence = self.event_sequence.fetch_add(1, Ordering::SeqCst);
503:         let mut event = StoreEvent::session_snapshot(
504:             &session.project_binding_id,
505:             &session.session_epoch,
506:             &self.broker_epoch,
507:             sequence.min(i64::MAX as u64) as i64,
508:             payload_jcs,
509:         );
510:         event.event_type = "finding".into();
511:         if store.append(vec![event]).is_err() {
512:             let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
513:             stand.store_verweigerungen = stand.store_verweigerungen.saturating_add(1);
514:         }
515:     }
```

### 3.3 Der bestehende Rechenhaken (`:82-91`) — er läuft OHNE gehaltenen Standlock

```rust
82:         // 🔑 NR-03 (Nacharbeit 1, 07.09.2026): der Testhaken sitzt GENAU
83:         // hier — zwischen der Rechnung ohne Lock und der Eintragung unter
84:         // dem Lock.
85:         //
86:         // Er ist das einzige Mittel, mit dem sich das Interleaving aus EP-03
87:         // deterministisch messen laesst: waehrend der Rechnung steigt die
88:         // Intent-Revision oder eine Evidenz wird zurueckgenommen. Zwei
89:         // Threads mit Barriere maessen dasselbe, aber nicht reproduzierbar —
90:         // und eine Zusage, deren Beweis flackert, ist keine.
91:         self.rechen_test_haken_ausloesen();
92:         for (session, befunde) in ergebnisse {
93:             let geaendert = self.befunde_eintragen(&session, befunde);
```

### 3.4 Die Rendezvous-Naht, die es im Coordinator schon gibt (`broker/src/coordinator/mod.rs:164-209`)

```rust
164: /// Einmalige, schlaflose Testnaht direkt nach dem Snapshot-Capture. Produktion
165: /// setzt sie nie; sie macht die sonst mikroskopische Reihenfolge
166: /// "alt erfasst, neu committed, alt committed" deterministisch pruefbar.
167: #[doc(hidden)]
168: #[derive(Clone, Default)]
169: pub struct CoordinatorFlushTestHaken {
170:     stand: Arc<(Mutex<(bool, bool)>, Condvar)>,
171:     /// D12 der Nacharbeit Runde 1 (Abschlusspruefung 1, 03.09.2026): scharf
172:     /// gestellt panisiert der naechste Flush EINMAL, waehrend er den
173:     /// Standlock haelt. Ohne diese Naht laesst sich H-04 nicht messen:
174:     /// `modell_sicht` gibt nur Kopien zurueck und laesst seinen Guard fallen,
175:     /// bevor es zurueckkehrt - eine Panik danach vergiftet nichts.
176:     panik_unter_standlock: Arc<AtomicBool>,
177: }
178: 
179: impl CoordinatorFlushTestHaken {
180:     pub fn warten_bis_erfasst(&self) {
181:         let (schloss, signal) = &*self.stand;
182:         let mut stand = schloss.lock().unwrap_or_else(|e| e.into_inner());
183:         while !stand.0 {
184:             stand = signal.wait(stand).unwrap_or_else(|e| e.into_inner());
185:         }
186:     }
187: 
188:     pub fn freigeben(&self) {
189:         let (schloss, signal) = &*self.stand;
190:         let mut stand = schloss.lock().unwrap_or_else(|e| e.into_inner());
191:         stand.1 = true;
192:         signal.notify_all();
193:     }
194: 
195:     /// Stellt die Panik unter dem Standlock scharf. Sie faellt genau einmal.
196:     pub fn panik_unter_standlock_scharf(&self) {
197:         self.panik_unter_standlock.store(true, Ordering::SeqCst);
198:     }
199: 
200:     fn erreichen(&self) {
201:         let (schloss, signal) = &*self.stand;
202:         let mut stand = schloss.lock().unwrap_or_else(|e| e.into_inner());
203:         stand.0 = true;
204:         signal.notify_all();
205:         while !stand.1 {
206:             stand = signal.wait(stand).unwrap_or_else(|e| e.into_inner());
207:         }
208:     }
209: }
```

### 3.5 Die Annahmekante (`broker/src/store/handle.rs:148-171`)

```rust
148:     pub fn append(&self, events: Vec<StoreEvent>) -> Result<Vec<AppendAusgang>, StoreFehler> {
149:         let antwort_rx = self.append_einreihen(events)?;
150:         antwort_rx.recv().map_err(|_| StoreFehler::Beendet)?
151:     }
152: 
153:     /// Trennt die begrenzte Annahmekante vom spaeteren Commit-Warten. Der
154:     /// Aufrufer darf zwischen diesen Schritten insbesondere keinen
155:     /// Coordinator-Lock halten.
156:     pub fn append_einreihen(
157:         &self,
158:         events: Vec<StoreEvent>,
159:     ) -> Result<mpsc::Receiver<Result<Vec<AppendAusgang>, StoreFehler>>, StoreFehler> {
160:         if self.append_naht.load(Ordering::SeqCst) {
161:             return Err(StoreFehler::Degradiert(
162:                 "Testnaht: der Append wird verweigert".into(),
163:             ));
164:         }
165:         let (antwort_tx, antwort_rx) = mpsc::channel();
166:         self.senden(WriterBefehl::AppendBatch {
167:             events,
168:             antwort: antwort_tx,
169:         })?;
170:         Ok(antwort_rx)
171:     }
```

### 3.6 Ordnung und Projektion (`broker/src/store/writer.rs:476`, `:588-593`)

```rust
476:     let event_ord = tx.last_insert_rowid();
588:         let sql = format!(
589:             "INSERT INTO {tabelle}({id_feld},project_binding_id,session_epoch,last_event_ord,state_jcs) \
590:              VALUES(?1,?2,?3,?4,?5) ON CONFLICT({id_feld}) DO UPDATE SET \
591:              project_binding_id=excluded.project_binding_id,session_epoch=excluded.session_epoch,\
592:              last_event_ord=excluded.last_event_ord,state_jcs=excluded.state_jcs \
593:              WHERE excluded.last_event_ord > {tabelle}.last_event_ord"
```

### 3.7 `finding_id` trägt die Eingangsmenge nicht (`broker/src/coordinator/hypothese/befund.rs:353-373`)

```rust
353: pub fn finding_id(
354:     aufnahme: &Aufnahme,
355:     metrik: Zielmetrik,
356:     band: Bandintervall,
357:     quelle: &str,
358: ) -> String {
359:     use sha2::{Digest, Sha256};
360:     let mut hasher = Sha256::new();
361:     hasher.update(b"nakama.sonde014.finding.v1\0");
362:     hasher.update(aufnahme.session_epoch.as_bytes());
363:     hasher.update(b"\0");
364:     hasher.update(metrik.wire().as_bytes());
365:     hasher.update(b"\0");
366:     hasher.update(band.von.to_be_bytes());
367:     hasher.update(band.bis.to_be_bytes());
368:     hasher.update(quelle.as_bytes());
369:     hasher.update(b"\0");
370:     hasher.update(aufnahme.metrics_version.to_be_bytes());
371:     let digest = hasher.finalize();
372:     digest[..16].iter().map(|b| format!("{b:02x}")).collect()
373: }
```

### 3.8 `ergebnis_ist_noch_gueltig` heute (`:961-997`, zitiert `:980-997`) — Teilmengenprüfung, keine Eingangsmenge

```rust
980:         // Die gueltigen Belege DIESER Sitzung, aus derselben Historie, aus
981:         // der `aufnahmen_sammeln` sie genommen hat.
982:         let mut gueltig: BTreeSet<&str> = BTreeSet::new();
983:         for (key, historie) in stand.evidenz.iter() {
984:             if key.session() != *session {
985:                 continue;
986:             }
987:             for eintrag in historie.iter() {
988:                 if eintrag.ausschlussgrund.is_none() {
989:                     gueltig.insert(eintrag.evidence_id.as_str());
990:                 }
991:             }
992:         }
993:         befunde
994:             .iter()
995:             .flat_map(|b| b.evidence_ids.iter())
996:             .all(|id| gueltig.contains(id.as_str()))
997:     }
```

---

## 4. Prüfung des Befundes an der Quelle

### 4.1 Der festgelegte Ablauf verwirft A schon am Vergleich

M-13 legt Fenster 2 so fest (Zeile 616, Spalte *Ereignis*): A steht bereits in
`stand.befunde`, der Standlock ist gefallen (`:478`), A hängt am neuen
Persistenzhaken **vor dem Persistenzlauf** (`:479-481`); **erst dann** sammelt,
rechnet, trägt ein und persistiert B **vollständig**; danach wird A freigegeben
und nimmt den Standlock.

Gerechnet gegen die Fixfassung aus §6.4:959-970:

1. B trägt ein, also steht in `stand.befunde[session]` der Payload von B
   (13 IDs).
2. A wird freigegeben, betritt `befund_persistieren`, nimmt den Standlock,
   vergleicht seinen Payload (12 IDs) gegen den Standeintrag — **ungleich**,
   also schreibt A nicht.

Dieser Verwurf entsteht **allein aus dem Vergleich**. Ob die Sperre zwischen
Vergleich und `append_einreihen` gehalten oder freigegeben wird, ändert an
Schritt 2 nichts: A kommt gar nicht bis zur Annahme. Die Mutation (b) der
Rotbeweisspalte (Sperre zwischen Vergleich und `append_einreihen` freigeben und
B in dieser Lücke laufen lassen) bleibt in diesem Ablauf **grün**. Der Befund
ist damit an der Quelle nachvollzogen.

### 4.2 Der benannte Haken kann das nötige Fenster nicht erzeugen

Der einzige in §6.1:803-805 und §6.4:995-997 benannte neue Haken sitzt
„zwischen Cache-Eintrag und Persistenzlauf" (`:479-481`), also **außerhalb**
des Standlocks und **vor** dem Vergleich. Alles, was er anhalten kann, ist ein
A, das seinen Vergleich noch **nicht** bestanden hat. Gebraucht wird für die
Kopplung das Gegenteil: ein A, das den Vergleich **bestanden** hat und zwischen
Vergleich und Annahme steht. Diese Stelle existiert nur **innerhalb** des
Guards von `befund_persistieren` und hat heute keinen Haken.

### 4.3 Die Zusage, für die der Beweis fehlt, steht schon im Dokument

§8.1 Feinheit 18 (`:1721-1733`) beschreibt genau den fehlenden Ablauf in Prosa:
„A besteht den Vergleich, hält an, B rechnet, reiht ein und persistiert, danach
reiht A ein — und gewinnt die Projektion". Das ist eine **zweite, eigene
Zusage** (Annahmeordnung), und keine Matrixzeile misst sie. Die tragenden
Größen dafür sind an der Quelle bestätigt:

- `finding_id` (`befund.rs:353-373`) hasht Sitzung, Metrik, Band, Quelle und
  `metrics_version` — **nicht** die Evidenzmenge. A und B derselben Sitzung
  treffen also denselben Projektionsschlüssel mit verschiedenen Payloads.
- Die Projektion behält die höhere `last_event_ord` (`writer.rs:588-593`,
  `WHERE excluded.last_event_ord > …`), und `last_event_ord` ist die `rowid`
  des **Anhängens** (`writer.rs:476`), also die Annahmereihenfolge.
- `ergebnis_ist_noch_gueltig` prüft heute nur, ob die verwendeten
  `evidence_ids` noch gültig sind (`:993-996`, Teilmenge); eine
  **hinzugekommene** Evidenz fällt heute gar nicht auf. Das ist Fenster 1 und
  wird erst durch den Fix (§6.4:952-958) geschlossen.

### 4.4 Gegenprobe: ginge der Rotbeweis auch ohne neuen Haken?

Drei Alternativen geprüft, alle fallen aus:

- **Konkurrent ohne Coordinator** (der Test hängt selbst über
  `StoreHandle::append_einreihen` einen `finding`-Event mit derselben
  `finding_id` an): Dieser Konkurrent nimmt den Standlock nie, also ordnet ihn
  die Kopplung auch nicht. Der Lauf wäre **in beiden Fassungen rot** — kein
  Unterscheider, und er widerspräche der Zusage nicht, denn die Kopplung ordnet
  ausdrücklich nur Coordinator-interne Schreiber.
- **Gleicher Payload für A und B**: dann kehrt `befunde_eintragen` für B schon
  bei `:467-470` mit `false` zurück, ohne Eintrag und ohne Persistieren — kein
  Wettlauf.
- **Zwei Sitzungen**: verschiedene `session_epoch` ergeben verschiedene
  `finding_id` (`befund.rs:362`) und damit verschiedene Projektionszeilen,
  also keinen Konflikt.

Der Wettlauf muss also **im selben Sitzungsschlüssel über zwei
Coordinator-Rechnungen** stattfinden, und A muss den Vergleich bestehen. Genau
dafür braucht es einen zweiten Haken.

---

## 5. Status und Klasse

| Merkmal | Wert |
|---|---|
| Status | **BESTÄTIGT** |
| Einordnung | **DEFEKT** gegen die Matrixregel §5.2 `:518-523` (Nebenläufigkeitsbeweise an einem deterministisch erzwungenen Interleaving, Ordnungsbeweise am zuletzt angenommenen Zustand des Empfängers) und gegen §6.1 Schritt 2. |
| Klasse | **Belegdefekt der Matrix**, kein Rückbau: die Kopplung aus der Nacharbeit 2 ist richtig und bleibt. Es fehlt der Ablauf, der sie misst — eine Zusage (Annahmeordnung) ohne Rotbeweis, also derselbe Zeilenfehler wie bei M-73/M-74 in Runde 2 (§14.1 Punkt 3), eine Ebene tiefer. |
| Nicht bestätigt | Nichts. Der Befund trifft in jedem geprüften Teil zu; die Zeilenverweise von Codex (`:995-997`, `:961-989`, `:478-480`, `#L518-L523`) stimmen mit dem Dokument überein. |

---

## 6. Die schließende Änderung

### 6.1 Fenster 3 — der Ablauf

Neue Bühne in `broker/tests/sonde014_verdrahtung.rs` (Bein **A4**), ein
Coordinator **mit Store**, eine Sitzung, drei Fäden: **A** (die Rechnung),
**B** (der zweite `eqcop-v3-conn`-Faden) und der **Kontrollfaden** des Tests,
der den Standlock nie anfasst.

1. Bühne: Evidenzmenge mit 12 stabilen IDs, ein Befund je Sitzung.
2. **A** rechnet, betritt `befunde_eintragen`: die Frischeprüfung besteht
   (nichts hat sich geändert), `stand.befunde[session]` bekommt den Payload von
   A (12 IDs), der Guard fällt (`:478`).
3. **A** betritt `befund_persistieren`, nimmt den Standlock, vergleicht seinen
   Payload gegen den Standeintrag — **besteht**.
4. **Der neue Haken feuert hier**, zwischen bestandenem Vergleich und
   `append_einreihen`, **unter dem gehaltenen Guard**. A meldet „steht vor der
   Annahme" und wartet auf Freigabe.
5. Der Kontrollfaden startet **B**: 13. Evidenz zustellen, rechnen,
   `befunde_eintragen`, `befund_persistieren` — lauter Wege, die den Standlock
   brauchen. Er wartet mit **Frist** auf die Abschlussmarke von B.
6. Fixfassung: B kommt nicht durch, weil A den Guard hält; die Frist läuft ab,
   der Kontrollfaden gibt A frei; A reiht ein (Ordnungszahl *n*), gibt die
   Sperre frei und wartet außerhalb auf `recv`; danach läuft B durch, besteht
   seinen eigenen Vergleich und reiht ein (*n+1*).
7. Mutantenfassung (Sperre nach dem Vergleich freigegeben): A steht am Haken
   **ohne** Guard, B läuft vollständig durch und reiht ein (*n*), setzt die
   Abschlussmarke; der Kontrollfaden gibt A sofort frei; A reiht **nach** B ein
   (*n+1*) und gewinnt die Projektion mit dem 12-ID-Payload.

### 6.2 Der Haken: Ort, Form, und ob er unter gehaltenem Lock geht

**Ort.** In `befund_persistieren` (`hypothese_verdrahtung.rs:494-515`, nach dem
Umbau), **zwischen dem bestandenen Vergleich und `append_einreihen`, am
gehaltenen Guard**. Das ist genau die Kante, die die Zusage trägt, und kein
Nebeneffekt (§5.2 `:518`). Der Haken vor dem Persistenzlauf (`:479-481`)
bleibt, er trägt Fenster 2.

**Form.** Nicht das Closure-Muster `rechen_test_haken` (`mod.rs:244`,
`:357-378`): dessen Closure läuft auf **dem Faden von A** — die bestehenden
Fälle `sonde014_verdrahtung.rs:575` und `:631` rufen darin direkt wieder in den
Coordinator hinein. Unter gehaltenem Standlock wäre das ein
**Selbst-Deadlock** (`std::sync::Mutex` ist nicht reentrant, wie §6.4:976-977
selbst festhält). Richtig ist das Rendezvous-Muster, das der Coordinator schon
hat: `CoordinatorFlushTestHaken` (`mod.rs:164-209`; `erreichen` meldet und
wartet, `freigeben` löst) — einmalig, nur für Tests sichtbar, im Produkt nie
gesetzt und wie `flush_test_haken` beim Ziehen herausgenommen, damit er nur A
trifft und nicht auch B. Feld und Setter gehören nach
`broker/src/coordinator/mod.rs`.

**Geht ein Haken unter gehaltenem Lock deterministisch, ohne Deadlock?**
**Ja — unter genau einer Bedingung: der Faden, der ihn freigibt, darf den
Standlock nie brauchen.**

- Ein Haken, der auf **den Abschluss von B** wartet, ist in der Fixfassung ein
  garantierter Deadlock: B braucht den Standlock, den A hält (zirkuläres
  Warten, `std::sync::Mutex`, keine Frist, kein Ausweg über Vergiftung).
- Ein Haken, der auf die **Freigabe durch den Kontrollfaden** wartet,
  terminiert immer: der Kontrollfaden nimmt den Standlock nie und erreicht
  `freigeben` unabhängig davon, ob B vorankommt. Fortschritt ist damit in
  beiden Fassungen garantiert.
- Die Frist (Vorschlag: 500 ms) liegt deshalb **im Test, nicht im Produkt**:
  der Kontrollfaden wartet befristet auf die Abschlussmarke von B und
  entscheidet danach, wann er A freigibt. Der Produkt-Haken selbst bleibt ein
  schlafloses Rendezvous wie `CoordinatorFlushTestHaken`.
- Präzedenz für eine Naht unter dem Standlock gibt es schon: die D12-Naht in
  `flush.rs:85-91` feuert **unter** dem Guard (dort nicht blockierend, sondern
  panisierend).

**Nebenwirkung, benannt:** A hält den Standlock für die Dauer der Frist; jeder
andere Broker-Faden, der den Stand anfasst, wartet so lange. Im Test
unerheblich, im Produkt ausgeschlossen, weil der Haken dort nie gesetzt wird
(§6.4:996, „im Produkt leer").

### 6.3 Erfolgskriterium

Der neue Fall (Vorschlag `annahmeordnung_folgt_der_vergleichsordnung`,
`broker/tests/sonde014_verdrahtung.rs`, Bein **A4**) prüft:

1. **Beide** Payloads stehen im `event_log` — A schreibt hier, anders als in
   Fenster 2, ausdrücklich **mit** —, und der Event von A trägt die
   **kleinere** `event_ord`, der von B die größere.
2. Die Projektion `findings.state_jcs` zu dieser `finding_id` trägt den
   **13-ID-Payload von B**.
3. Projektion und flüchtiger Stand (`stand.befunde`) sind gleich.

Wörtlich das Kriterium von Codex: **A darf zuerst angenommen werden,
anschließend bestimmt B die jüngere Projektion.**

**Rot unter der Mutation** (Sperre zwischen Vergleich und `append_einreihen`
freigeben): der Event von A trägt die **höhere** `event_ord`, die Projektion
trägt den 12-ID-Payload, Projektion und Stand stehen auseinander — alle drei
Zusagen fallen.

### 6.4 Determinismus und seine einzige Grenze

Die Fixfassung ist **beweisbar** deterministisch: B kann zwischen Schritt 4 und
6 nicht vorankommen, weil jeder seiner Wege den von A gehaltenen Standlock
braucht; die Frist läuft also **immer** ab. Die Mutantenfassung ist
deterministisch, solange B innerhalb der Frist durchkommt; auf einer
überlasteten Maschine wäre ein Fristablauf denkbar, und der sähe wie grün aus.
Dagegen die Pflicht im Bauplan: der Test **protokolliert seinen Freigabegrund**
(`Signal` oder `Frist`) und macht ihn zur Zusicherung — der Fixlauf muss
`Frist` zeigen (zugleich der Beleg, dass B nicht durch die Lücke kam), der
Mutantenlauf muss `Signal` zeigen. Ein Mutantenlauf mit `Frist` gilt als
**nicht gemessen** und wird mit längerer Frist wiederholt, nie als grün
gezählt. Damit kippt das Urteil nie still, und die Forderung aus §6.1:801-802
(eine Zeile, die nur „manchmal" rot wird, ist kein Rotbeweis) bleibt gewahrt.

### 6.5 Welche Zeile ihn trägt: **neue Zeile M-75**

**Empfehlung: neue Zeile M-75; M-13 bleibt bei seinen zwei Fenstern.** Gründe:

- **Andere Zusage.** M-13 sagt zu: *die Frischeprüfung reicht bis Persistenz
  und Publikation* (A schreibt **nicht**). Fenster 3 sagt zu: *die
  Annahmeordnung folgt der Vergleichsordnung* (A schreibt, aber **vor** B).
  Beides in einer Zeile zu führen ist genau der Zeilenfehler, den die
  Matrixprüfung 2 an M-73 gerügt und die Nacharbeit 2 mit der Teilung in
  M-73/M-74 geschlossen hat (§14.1 Punkt 3).
- **Andere Klasse.** M-13 ist „rot in einer Hälfte, nicht messbar in der
  anderen" (§5.6:708). M-75 ist **ganz** „heute nicht messbar", weil am
  Basisstand weder Vergleich noch Kopplung existieren; rot wird es gegen die
  **Fixfassung** — dieselbe Bauart wie M-59 (§5.6:715-717).
- **Andere Erfolgsrichtung.** M-13 Fenster 2 misst ein **ausbleibendes**
  Schreiben, M-75 die **Reihenfolge** zweier tatsächlicher Schreibvorgänge.

Zeilenskizze M-75 (Spalten wie §5.2): *Zustand* A hat unter dem Standlock 12
IDs gesammelt und gerechnet, der Stand trägt den Payload von A · *Ereignis* A
besteht in `befund_persistieren` den Wiedervergleich und hängt am **neuen
Annahmehaken** zwischen Vergleich und `append_einreihen`; B liefert die 13.
Evidenz, rechnet, trägt ein und persistiert · *Zusage* Wer den Vergleich später
besteht, reiht später ein: der `finding`-Event von A trägt die kleinere
`event_ord`, der von B die größere, und die Projektion `findings.state_jcs`
trägt den 13-ID-Payload von B · *Reihenfolge und Frist* Standlock über
Vergleich **und** Annahme (`:494-515`), Freigabe vor dem `recv`; Ordnung aus
`store/writer.rs:476`, `:588-593` · *Test*
`broker/tests/sonde014_verdrahtung.rs`, **NEU**
`annahmeordnung_folgt_der_vergleichsordnung`, Bein **A4** · *Rotbeweis* Sperre
zwischen Vergleich und `append_einreihen` freigeben (die Fassung der Nacharbeit
1), B in der Lücke laufen lassen; **am Basis-SHA nicht messbar** (Vergleich und
Kopplung entstehen erst mit dieser Etappe), **rot gegen die Fixfassung** ·
*Quelle* §2 R-283-1, §8.1 Feinheit 18 (`:1721-1733`), Matrixprüfung 3 Punkt 1 ·
*Etappe* 3.

---

## 7. Was die Änderung mitziehen muss (Beziehungen im selben Satz)

1. **§5.2** — neue Zeile M-75 nach M-74; der Rotbeweis (b) von M-13 verliert
   seinen Anspruch, die Kopplung zu messen, und verweist auf M-75.
2. **§5.6** — Zählung neu aus den Zeilen: 74 auf **75** Zeilen, Baulücke 54 auf
   **55**, „heute nicht messbar" 7 auf **8** (M-16, M-63, M-65, M-66, M-68,
   M-71, M-72, **M-75**); die 20 Regressionswachen bleiben, Summe 55 + 20 = 75.
   In der Zuordnungstabelle bekommen F02 und R-283-1 die M-75.
3. **§6.1 Schritt 2** — die Hakenliste (`:802-805`) nennt bisher zwei neue
   Haken; der **dritte** (Annahmehaken unter dem Guard) gehört dazu, und die
   Aufzählung der Nebenläufigkeitszeilen (`:799-800`) bekommt M-75.
4. **§6.4** — der Absatz „Fenster 2" wird um Fenster 3 ergänzt (Haken, Frist im
   Test, Freigabegrund als Zusicherung); die Testtabelle (`:1020-1034`) bekommt
   die neue Zeile; „Reihenfolge" (`:1041-1047`) bekommt Fenster 3 nach Fenster
   2; „Risiken" (`:1049-1063`) bekommt den Satz, dass ein Haken unter dem Guard
   nur mit einem Freigeber ohne Standlock zulässig ist.
5. **§8.1 Feinheit 18** — der Absatz `:1721-1733` beschreibt den Ablauf schon
   und bekommt den Zeiger auf M-75, damit die Zusage eine messende Zeile hat.
6. **Nebenbefund, im selben Satz zu schließen:** die **Ticketpfade** der Etappe
   3 (`:937-948`) nennen `broker/src/coordinator/mod.rs` **nicht** — obwohl
   schon der in §6.4:995-997 angekündigte Fenster-2-Haken dort ein Feld und
   einen Setter braucht (Muster `rechen_test_haken`, `mod.rs:244`, `:357-378`;
   `flush_test_haken`, `mod.rs:223`, `:381-390`). Mit dem Annahmehaken gilt das
   doppelt. Ohne diese Ergänzung baut die Etappe 3 an einer Datei, die ihre
   eigene Pfadliste nicht führt.

---

## 8. Scope-Beweis

- **Geschrieben:** ausschließlich diese Datei
  (`scratchpad/NAK-283-matrixpruefung-3-validierung.md`).
- **Nicht geschrieben:** keine Repo-Datei, kein Commit, kein Push, kein Branch,
  kein Stash. `git status --short` war zu Beginn leer und ist es geblieben;
  `git` lief nur mit `status`, `log`, `rev-parse` und `diff`.
- **Nicht gebaut, nicht gefahren:** kein `cmake`, kein `cargo`, kein
  `tools/beweise.ps1`, kein Test, kein Skript, kein Werkzeug aus `tools/`.
- **Gelesene Quellen:** `docs/beweise/NAK-283.md` (§5.2, §5.6, §6.1, §6.4,
  §8.1, §13.1, §14), `docs/beweise/roh/NAK-283-matrixpruefung-3-467e453.txt`,
  `broker/src/coordinator/hypothese_verdrahtung.rs`,
  `broker/src/coordinator/mod.rs`, `broker/src/coordinator/flush.rs`,
  `broker/src/coordinator/hypothese/befund.rs`, `broker/src/store/handle.rs`,
  `broker/src/store/writer.rs`, `broker/tests/sonde014_verdrahtung.rs`.
- **Standbezug:** Worktree `e82572ef`; gegenüber dem geprüften `467e4534`
  unterscheiden sich nur die Kopfzeile „Etappe" und der angehängte §16
  (`git diff --stat`: 13 +, 1 −), §5 und §6 sind zeilengleich. Produktcode
  unverändert seit `ea6ddd57`.
