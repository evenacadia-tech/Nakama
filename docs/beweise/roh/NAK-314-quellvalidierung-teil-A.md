# NAK-314 — Quellvalidierung Teil A (lesender Opus-Agent, HEAD 4ee4c407)

**Ticket:** NAK-314 · Planschritt S25m „Tiefenaudit 3 abarbeiten, Teil 6: Broker" · Etappe 0
„Quellvalidierung", **Teil A** (Store, Persistenzordnung, Loop-Wrap)
**HEAD:** `4ee4c40752845d98fb451f550ddf8743079153be` (Zweig `master`)
**Datum:** 26.09.2026 · **Modell:** Opus 5.5 (lesend)
**IDs (4):** T3-08-01 (W13); T3-08-02, T3-08-03 (W14); T3-08-05 (W28). Dazu die drei
Regelfragen dieses Teils: atomare Domänentransaktion (T3-08-01), Ordnung von Commit und
Publikation (T3-08-02, T3-08-03), Loop-Wrap (T3-08-05).
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (Phasenzeilen `:451`,
`:452`, `:453`, `:455`; Skeptikerzeilen Phase 10 `:539`, `:540`, `:541`, `:543`; Bündel B14
`:84`, B15 `:85`, B22 `:92`; Geordnete Abarbeitung `:127`, `:128`, `:136`; Arbeitsübersicht
W13 `:816`, W14 `:817`, W28 `:831`) und die Rohberichte unter
`docs/audits/2026-09-15-tiefenaudit/roh/`: `phase-08-broker.md` (`:30-34`, `:40-122`, `:140`,
`:172-173`, `:196`), `phase-10-skeptiker.md` (`:80-84`, `:108`, `:116-118`),
`phase-11-synthese.md` (`:74-75`, `:82`, `:163`, `:166`, `:180`, `:375-379`, `:406`, `:412`),
`phase-16-c-persistenz.md`, `phase-16-f-gegenpruefung-c.md`, `phase-16-abdeckung.md`. Die drei
Phase-16-Dateien behandeln keine der vier IDs; W13, W14 und W28 stehen dort als „Altbasis"
(`BEFUNDE.md:816`, `:817`, `:831`).
**Gate:** Schritt S25m in `docs/plan/plan.json:382-388` (Feld `text` in `:385`);
Registerzeilen `docs/offene-punkte.md:317` (NAK-314), `:184` (NAK-157), `:277` (NAK-269); dazu
gelesen `:193` (NAK-167), `:161` (NAK-125), `:304` (NAK-296).
**Muster:** `docs/beweise/roh/NAK-313-quellvalidierung-teil-A.md` (Form, Tiefe, Kopf) und
`docs/beweise/roh/NAK-313-quellvalidierung-teil-C.md` (Trennung Technik und Produkt).

**Gate-Text wörtlich** (`docs/plan/plan.json:385`, Feld `text`):

> „ANGELEGT 18.09.2026 (Register NAK-314; Arbeitsübersicht Phase 16 W13, W14, W15, W16, W17,
> W25, W28). INHALT: 10 Befunde im Broker — T3-08-01 (Domain-Atomarität am 64er-Batchrand;
> NAK-157), T3-08-02 und T3-08-03 (Rücknahme vor Erstpersistenz nach Restore wieder gültig,
> Commit- und Publikationsordnung; NAK-269), T3-15-12 und T3-15-13 (degradierter Store trennt
> Clients im Sekundentakt, transienter Lesefehler lässt Routing fail-closed ohne Anzeige),
> T3-16-02 (verspätetes Ende einer alten Telemetrieverbindung entkoppelt die angenommene
> Nachfolgerin), T3-15-15 und T3-14-03 (Klonkonflikt ohne Löser mit dauerhaften Sperren,
> Unknown nach reinem Sonde-Abbruch ohne Rückweg), T3-16-03 (erfolgreich geschriebene
> P1-Evidenz geht beim gewöhnlichen Linkende verloren), T3-08-05 (vollständiger Loop-Wrap
> invalidiert gültige Evidenz entgegen M-53). Produktentscheide als Karten: Klon-Identität und
> Sperrenende, Empfangs-ACK oder gezählter Verlust. GRENZE: ACK erst nach vollständigem Commit,
> Replay idempotent, keine Callbacks auf abgemeldeter Sitzung, Konfliktrestore fail-closed,
> keine Sicherheitsneubewertung (S25i). BEWEIS: docs/beweise/NAK-314.md mit Matrix,
> Rotbeweisen in A4, A4-SI (store_crash_matrix), A22, B8, vollem Kanon GRÜN und Laufzeit-Arm."

**Prüfform:** Quellenlektüre am HEAD. Kein Bau, kein `cargo`, kein Testlauf, kein Kanon, kein
FL Studio, kein Codex, keine Fehlerinjektion. Git nur lesend (`status`, `rev-parse`, `log`,
`diff`, `blame`, `grep`, `merge-base --is-ancestor`). **Eine Abweichung, offen benannt:** den
Transportstempel der Fixture
`eq-copilot/fixtures/v3/gueltig/evidence-snapshot-mit-ereignissen-und-stereo.json` habe ich mit
einem einzeiligen `py -3.13 -c` gelesen (nur `json.load` und Ausgabe, kein Produkt- oder
Testcode). Das ist formal ein Skriptaufruf; der Auftrag schloss Skripte aus. Das Ergebnis
(`project_sample_start 44108200`, `sample_count 512`, `transport_epoch 17`) ist nur für 2 d)
benutzt und lässt sich mit einem Blick in die Datei nachprüfen. Jede Zeilennummer unten ist am
HEAD `4ee4c407` selbst nachgelesen; wo eine Auditzeile gewandert ist, steht die neue Zahl.
Sicherheitsbefunde (T3-12-\*) sind nicht Gegenstand; Legacy-Bezeichner sind kein Befund; der
Designteil ist geparkt, unten steht kein Oberflächenvorschlag.

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn* (26.09.2026, 15:22:02 +0200):
die fremden Ordner `briefing-hub/` und `nimbalyst-local/` sowie die drei Auftragsdateien
`docs/beweise/roh/NAK-314-quellvalidierung-teil-{A,B,C}-auftrag.txt`, sonst nichts; HEAD
`4ee4c407`. *Unmittelbar vor dem Schreiben* (15:39:25 +0200): HEAD unverändert `4ee4c407`,
zusätzlich untracked `docs/beweise/NAK-314.md` — nicht von mir, nicht angefasst (Manifest des
Dirigenten). Der HEAD ist während des Laufs **nicht** gewandert; ein Abgleich der Zeilennummern
war nicht nötig. Diese Datei ist meine einzige Schreibaktion; `…-teil-B.md` und `…-teil-C.md`
gehören den Parallelagenten. *Nach dem Schreiben* (15:47:31 +0200, HEAD weiter `4ee4c407`)
zeigt `git status --short` zusätzlich diese Datei und `…-teil-B.md` des Parallelagenten, sonst
unverändert; die Datei hat LF-Zeilenenden (kein CR).

**Stand des Audits gegen den HEAD.** Phase 08 prüfte `979c4b94` (16.09.2026), Phase 16
`aff2d818` (18.09.2026). `git log aff2d818..HEAD -- broker/` zählt 21 Commits (NAK-309
Etappe 4, NAK-313 Etappen 4 bis 7, NAK-380 Etappen 2 bis 6). Für die vier IDs entscheidend:

- `git diff --stat aff2d818 HEAD` über `broker/src/store/`, `broker/src/coordinator/flush.rs`,
  `broker/src/transport/server_v3/queues.rs`, `broker/src/transport/warteschlange.rs`,
  `broker/src/zeit.rs` und `broker/src/aggregat.rs` ist **leer**. Writer, Flush und
  Ausgangsqueue stehen wörtlich wie im Audit.
- `invalidierung_verdrahtung.rs` änderte nur `f893c57e` (NAK-380, Signatur des
  Materialweges `:689-701`); `invalidierung.rs` änderten `2fa1e35a` und `f893c57e` (NAK-380,
  Materialurteil mit Messfassung `:219-296`). Transportbruch, `Bruchart` und
  `grund_fuer_bruch` sind unberührt.
- `evidenz.rs` und `befehl.rs` änderten `28839892` und `8fb8d46f` (NAK-313 Etappen 4 und 5a:
  Ganzzahlhelfer, P1-Weiche über den Wert). Der Ablauf Einfügen → Transportbruch → Ablage und
  der Append von Befehl plus Wirkung sind unverändert.
- `git blame` der tragenden Zeilen: `writer.rs:278` aus `c1fa08ac` (03.09.),
  `evidenz.rs:199-224` aus `fb929283`, `f6c66507`, `a3654f59`, `45f1948c` (alle 04.09.),
  `invalidierung_verdrahtung.rs:626-650` aus `725ab66c` und `f6c66507` (04.09.),
  `invalidierung.rs:146-154` aus `3203a028` (04.09.), `Zeit.h:519-522` aus `1412ae60` (09.09.),
  `store_crash_matrix.rs:1701-1721` aus `901598fc` (31.08.), `sonde013_verdrahtung.rs:3221-3230`
  aus `71194a7f` (04.09.), `SourcesModel.cpp:1625-1643` aus `14dea8c4` und `8ee5a59c`
  (04. und 08.09.). Alle sind Vorfahren von `aff2d818` (`git merge-base --is-ancestor`).
- Gewanderte Auditzeilen: `befehl.rs:409`/`:437`/`:189` → `:403-438`/`:439`/`:191-211`;
  `evidenz.rs:186`/`:197`/`:211`/`:450` → `:192-199`/`:210`/`:224`/`:462-479`;
  `experiment_verdrahtung.rs:1441` → `:1485-1508`; `SourcesModel.cpp:1505-1518` → `:1625-1643`;
  `Zeit.h:516` → `:519`; `Frame.h:221` → `:226`; `sonde013_verdrahtung.rs:3126`/`:1484`/`:1072`
  → `:3171`/`:1529`/`:1117`. Die übrigen Auditzeilen stehen am selben Ort.

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorievorschlag | Schwere | Rotbeweis fällt an | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-08-01 (W13) | BESTÄTIGT; dazu: auch ein SQLite-Fehler im zweiten Abschnitt führt ohne Absturz zum falschen `idempotent_wiederholt`, der Test B14 kann Teilung und Ganzheit nicht unterscheiden, und die Teilung ist eine SONDE-011-Bauregel | DEFEKT | high (Audit: critical) | A4-SI `store_crash_matrix.rs` neben `:2240`, A4 neben `:1701-1721`; Zusagezeile die Batchbildung `writer.rs:272-284` | nein — Technikregel (Regelfrage 1) |
| T3-08-02 (W14) | BESTÄTIGT; dazu: der serielle Fall tritt bei **jedem** Rücksprung mit neuer Epoche ein, also bei jedem Loop-Wrap, und dort irrt der Cache, nicht der Store | DEFEKT | medium (Audit: high) | A4 `sonde013_verdrahtung.rs` neben `:1503` und `:3325` (Neustart über `neuer_coordinator`); Zusagezeile die neue Ordnung um `evidenz.rs:192-224` | nein — Technikregel (Regelfrage 2) |
| T3-08-03 (W14) | **PRÄZISIERT**: Umordnung vor der Queue trägt; die Verbraucherwirkung ist global und fail-closed und heilt mit dem nächsten Snapshot; dazu eine zweite, umgekehrte Lücke (Erfassung vor Markierung, Commit danach), weshalb „nach `event_ord` sortieren" allein kein Fix ist | DEFEKT | medium (Audit: high) | A4 `tests_abonnement.rs` neben `:1039-1096` mit `SperreVorDemSender` (`:1118`); Zusagezeile die neue Sitzungsordnung zwischen Commit und Einreihen | nein — Technikregel (Regelfrage 2) |
| T3-08-05 (W28) | **PRÄZISIERT**: breiter als behauptet — **jeder** Loop-Wrap, mit oder ohne Passage, nimmt alle Evidenz aller Quellen der Sitzung vor dem alten Frameende zurück; die gebundene Passage kennt der Broker nur nach `experiment_begin`, das heute keinen Bedienweg hat | DEFEKT (Passagenfall gegen M-53, allgemeiner Loopfall gegen Entwurf §32.3) | high (Audit: medium) | A4 `sonde013_verdrahtung.rs` neben `:1117`; Zusagezeile die neue Bruchart-Entscheidung in `invalidierung_aus_transportbruch` `:617-657` | nein — Technikregel (Regelfrage 3) |

---

## 1. T3-08-01 — der Group-Commit teilt Befehl und Wirkung an der 64er-Grenze (W13)

*Befundzeile: `BEFUNDE.md:451` · Skeptikerzeile Phase 10: `:539` · Bündel B14: `:84` ·
Arbeitsübersicht W13: `:816` · Rohbeleg: `roh/phase-08-broker.md:40-56` · Register NAK-157:
`docs/offene-punkte.md:184`*

### a) Behauptung des Audits

Ein gemeinsamer Append macht Befehlsriegel und Domänenwirkung nicht atomar: der Group-Commit
kann einen Auftrag mit zwei Ereignissen an der gemeinsamen 64er-Grenze teilen, sodass nach
einem Absturz der Retry `idempotent_wiederholt` bekommt und die Wirkung dauerhaft fehlt;
`store_crash_matrix.rs:1701-1721` erwartet die Teilung 63+2 ausdrücklich (`BEFUNDE.md:451`).

### b) Quellkette am HEAD

**Station 1 — ein Befehl baut EINEN Auftrag aus Riegel und Wirkung.**
`broker/src/coordinator/befehl.rs:403-438` baut zuerst das Ereignis `command` (`:411-412`
`event.command_id = Some(command_id.to_owned()); event.event_type = "command".into();`), dann
je Domänenereignis ein weiteres (`:423-437`), und übergibt den Vektor in einem Aufruf:

```
        match store.append(ereignisse) {
            Ok(ausgaenge) => {
                ords.extend(ausgaenge.iter().map(|a| a.event_ord()));
                match ausgaenge.first()? {
                crate::store::AppendAusgang::Angewandt { .. } => {
                    Self::command_ack(command_id, "angewandt", revision, Some(&hash), None)
                }
```
(`befehl.rs:439-445`). Die Aufrufer mit Domänenereignissen: `preview_begin`, `preview_renew`,
`preview_end` (`:927-940`, Befehl plus `evidence_invalidate`), die vier Experimentfamilien
(`experiment_verdrahtung.rs:307-314`: Befehl plus gegebenenfalls `passage`, je verdrängtem
Versuch ein Terminal, `experiment`, gegebenenfalls die Materialrücknahme) und `user_verdict`
(`befehl.rs:644-660`, Befehl plus `user_verdict`). Alle anderen Store-Produzenten hängen genau
ein Ereignis an (`assistent.rs:197`, `evidenz.rs:479`, `flush.rs:158`,
`hypothese_verdrahtung.rs:678`, `intent.rs:752`, `invalidierung_verdrahtung.rs:389`,
`proposal_verdrahtung.rs:158`). Die Befehle einer Sitzung sind untereinander und mit den Flushes
derselben Sitzung über `session_flush_schloesser` serialisiert (`befehl.rs:172-176`, gehalten
bis zur Rückkehr), die übrigen Produzenten nicht.

**Station 2 — der Writer zählt Ereignisse über Aufträge hinweg und schneidet bei 64.**
`broker/src/store/mod.rs:50-51` (`COMMIT_FENSTER_MS: u64 = 50`, `COMMIT_BATCH_MAX: usize = 64`).
`broker/src/store/writer.rs:268-287`:

```
                    loop {
                        // Alles bereits Angenommene wird in Kanalreihenfolge
                        // bis zur 64er-Grenze in diesen Commit gezogen.
                        let mut index = 0usize;
                        while anzahl < COMMIT_BATCH_MAX && index < aktiv.len() {
                            let job = aktiv.get_mut(index).expect("Append-Index");
                            if job.rest.is_empty() {
                                index += 1;
                                continue;
                            }
                            let nehmen = (COMMIT_BATCH_MAX - anzahl).min(job.rest.len());
                            let events = job.rest.drain(..nehmen).collect::<Vec<_>>();
                            anzahl += events.len();
                            commit_jobs.push(CommitJob { events });
                            zuordnung.push(index);
                            index += 1;
                        }
                        if anzahl >= COMMIT_BATCH_MAX {
                            break;
                        }
```

`:278` nimmt aus einem Auftrag nur, was bis 64 noch passt; der Rest bleibt in `job.rest`. `:325`
committet die Gruppe als eine Transaktion (`append_gruppe`, `:417`
`transaction_with_behavior(TransactionBehavior::Immediate)`, `:429` `tx.commit()?`). Der Rest
des geteilten Auftrags läuft im nächsten Durchlauf der Schleife `:252`, der ein **neues**
Sammelfenster öffnet (`:262` `let start = Instant::now();`, `:289-293` bis zu 50 ms).

**Station 3 — die Antwort (und damit das ACK) kommt erst nach dem letzten Abschnitt.**
`writer.rs:252-256`:

```
                while !aktiv.is_empty() {
                    while aktiv.front().is_some_and(|job| job.rest.is_empty()) {
                        if let Some(job) = aktiv.pop_front() {
                            let _ = job.antwort.send(Ok(job.ergebnisse));
                        }
                    }
```

`StoreHandle::append` wartet auf genau diese Antwort (`broker/src/store/handle.rs:148-151`).
Das ACK entsteht in `befehl.rs:442-445` erst danach. **Die Gate-Grenze „ACK erst nach
vollständigem Commit" hält also am HEAD**; der erste Abschnitt ist aber vor dem ACK dauerhaft.

**Station 4 — Absturz oder Fehler zwischen den Abschnitten.** Stirbt der Broker zwischen `:429`
des ersten und des zweiten Commits, liegt der Befehlsriegel im Log, die Wirkung nicht, und es
ging kein ACK hinaus. Scheitert statt eines Absturzes der zweite `append_gruppe` (SQLite-Fehler),
degradiert der Writer und beantwortet alle offenen Aufträge mit `Err`, ohne den ersten Commit
zurückzunehmen (`writer.rs:345-352`); `persistenz_p0_intern` gibt dann `None` und trennt den
Link (`befehl.rs:469-472`, `flush.rs:311-317`).

**Station 5 — der Retry findet den Riegel und quittiert ohne Wirkung.** `befehl.rs:187-211`:

```
        match store.command_event_lesen(command_id) {
            Ok(Some(payload)) => {
                let (alt, revision, hash) = Self::persistierte_command_wirkung(&payload)?;
                return if alt == *wert {
                    Self::command_ack(
                        command_id,
                        "idempotent_wiederholt",
```

`command_event_lesen` öffnet eine eigene Leseverbindung und prüft den Degradationszustand nicht
(`handle.rs:277-287`, `:483-493`). Für die Experimentfamilien steht derselbe Riegel vor jeder
Fachprüfung (`experiment_verdrahtung.rs:60-62` über `bekannter_befehl`, `befehl.rs:85-108`).
**Eigene Ergänzung:** im Fehlerfall von Station 4 braucht es keinen Neustart — der Client
verbindet neu, wiederholt dieselbe `command_id` und bekommt im selben Brokerlauf
`idempotent_wiederholt` (hergeleitet aus `:191-211` und `handle.rs:277-287`, nicht gemessen).

**Wirkung je Familie.** Preview: der Riegel steht, die Rücknahme der Sitzung fehlt im Store
(Neustart: die zurückgenommenen Belege sind wieder gültig). `experiment_begin`: je nach
Schnittstelle fehlen Passage, Versuch, das Terminal eines Verdrängten oder die
Materialrücknahme, während der Befehl als bestätigt gilt. `user_verdict`: das Urteil fehlt,
der Client hat seinen Auftrag als Erfolg freigegeben (SONDE-011 O-01, `SONDE-011.md:560`).

**Zahlenränder** (Auftragslänge k, vorher gesammelte Ereignisse p, `writer.rs:272-285`):
p = 63, k = 1 → 64, ein Commit, ganz. p = 63, k = 2 → Schnitt 1 | 1. p = 62, k = 2 → 64,
ganz. p = 64 → der volle Commit geht zuerst, der Auftrag steht am Anfang des nächsten und bleibt
ganz. Allgemein wird geteilt, wenn p < 64 < p + k; ein Experimentbefehl mit vier oder fünf
Ereignissen wird schon bei p = 60 bis 63 geteilt. Ein Auftrag mit mehr als 64 Ereignissen wird
immer geteilt (`writer.rs:248-251`, gewollt) — kein Produktaufrufer erzeugt heute einen
solchen (Station 1). Das Fenster zwischen den Commits ist nicht mikroskopisch: bis zu 50 ms
Sammelfenster plus Commitzeit (`:289-293`).

**`event_ord` am Schnitt.** Die geteilten Ereignisse bleiben **benachbart**: das letzte des
ersten Commits trägt `n`, das erste des zweiten `n+1`, weil nur der Writer schreibt und die
Ereignisse in Kanalreihenfolge anhängt (`writer.rs:436-477`). Ein `event_ord`-Abstand von 1
beweist also keine gemeinsame Transaktion.

**Herkunft der Teilung.** SONDE-011 Phase B Nacharbeit Runde 1, Defekt 18 (`901598fc`,
31.08.2026), `docs/beweise/SONDE-011.md:1075`: „`broker/src/store.rs` — jeder logische
Appendjob wird geordnet in Transaktionen von höchstens 64 Events geteilt; `append_gruppe`
besitzt zusätzlich einen harten Cap-Riegel". Damals hatte jeder P0-Befehl ein einziges
Ereignis. SONDE-013 B14 (`71194a7f`, 04.09.) legte danach Befehl und Wirkung in einen Auftrag
und nahm die Ganzheit an, ohne den Writer zu ändern.

**Irreführende Kommentare** (alle am HEAD):
- `befehl.rs:414-421`: „Der Writer zieht bis zu `COMMIT_BATCH_MAX` Ereignisse in EINE
  Transaktion; ein Befehl mit seinen Domaenenereignissen liegt weit darunter und wird deshalb
  als Ganzes committet oder gar nicht." — falsch, die Grenze gilt der Summe aller Aufträge.
- `invalidierung_verdrahtung.rs:290-292`: „Damit liegen Befehlsriegel und Ruecknahme in
  derselben Transaktion. Ein Crash dazwischen kann es nicht mehr geben".
- `experiment_verdrahtung.rs:207-208`: „Befehl und Domaenenereignisse gehen in EINEN
  `store.append` — der Writer zieht sie in eine Transaktion."
- `broker/tests/sonde013_verdrahtung.rs:3227-3229`: „die Ruecknahme folgt dem Befehl
  UNMITTELBAR; dazwischen passt kein zweiter Commit und damit auch kein Absturz".
- `experiment_verdrahtung.rs:67-68`: „`persistenz_p0` schreibt den Befehl fest und antwortet
  `angewandt`, BEVOR die Wirkung laeuft." — beschreibt einen früheren Stand; heute läuft die
  Wirkung vorläufig davor und wird im selben Auftrag abgelegt (`:251-314`).
- `docs/plugin-wissen.md:2654-2656` („Befehl und Wirkung in EINEM Append") ist wörtlich wahr,
  wird aber als Atomarität gelesen.

### c) Zusage wörtlich

- Entwurf §53.9, `docs/FL-Nakama-Sonden-Design-Entwurf.md:4195-4198`: „Ein Killtest an jeder
  Outboxgrenze beweist at-least-once-Zustellung mit exactly-once-Wirkung: Entweder wird das
  Ereignis erneut gesendet oder als bestätigt rekonstruiert, nie still verloren oder wegen einer
  Wiederholung doppelt angewandt."
- SONDE-013 §13.4, `docs/beweise/SONDE-013.md:4203` (B14): „`preview_begin`, `preview_renew`
  und `preview_end` sowie der Materialwechsel nach `experiment_begin` tragen ihre Invalidierung
  als Domänenereignis im SELBEN `persistenz_p0_mit_domaene`." Die Regel dahinter steht im
  Testkommentar `sonde013_verdrahtung.rs:3171`: „B14 — eine Preview committet ihre Ruecknahme
  MIT sich, oder gar nicht."
- SONDE-011 K-02, `SONDE-011.md:597`: „Alle vier Storeteile gemeinsam abwesend oder committed";
  V-03 `:612`: „Interne `command_id`-Zuordnung, `event_log`-Append, Projektion und UPSERT der
  neuesten Snapshot-Schuld laufen in einer `BEGIN IMMEDIATE`-Transaktion"; S-02 `:534`: „Erst
  der Commit einer neuen `command_id` erlaubt `command_ack(ergebnis = angewandt)`; ein Retry
  derselben ID liest die haltbare Zuordnung, erzeugt kein zweites Event und antwortet
  `ergebnis = idempotent_wiederholt`."
- Gate S25m (`plan.json:385`): „ACK erst nach vollständigem Commit, Replay idempotent". Das ACK
  hält (Station 3). Der Replay ist formal idempotent, bestätigt aber eine Wirkung, die fehlt.
- Arbeitsübersicht W13 (`BEFUNDE.md:816`), zu erhaltende Invariante: „ACK erst nach gesamtem
  Commit, Replay idempotent, Backpressure begrenzt."
- Gegenlautende Bauregel: SONDE-011 Defekt 18 (`SONDE-011.md:1075`, oben zitiert). Sie ist
  eine Baubeschreibung, kein User-Entscheid; sie widerspricht B14 und §53.9 für Aufträge bis
  64 Ereignisse.
- Konvergenzentscheid SONDE-013 §14.1 (`SONDE-013.md:4340`) ordnete NAK-157 als „**HÄRTUNG**
  (Testform) — Killmatrix-Fall fehlt, Produktpfad ist atomar" ein. Die Prämisse „Produktpfad
  ist atomar" ist am HEAD falsch (Station 2).
- `CLAUDE.md:194-196` („State bleibt verlustfrei") betrifft den Plugin-State, nicht den
  Broker-Store; CLAUDE.md schweigt zum Store.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| A4 `store_crash_matrix.rs:1689-1722` (`jeder_group_commit_bleibt_bei_hoechstens_64_events`), Bein `tools/beweise.ps1:511-519` | 65 Ereignisse in einem Auftrag ergeben 2 Commits; dann Auftrag a mit 63 und b mit 2 Ereignissen: `sicht.commits == 2`, `groesster_commit == COMMIT_BATCH_MAX` (`:1720-1721`) | **Schreibt die Teilung fest:** mit 65 Ereignissen in 2 Commits ist 64 nur erreichbar, wenn b geteilt wird. Die Ereignisse sind `event(None, …)` ohne `command_id` (`:1712-1715`, Helfer `:118-131`), keine Befehl-Wirkung-Einheit |
| A4 `sonde013_verdrahtung.rs:3182-3249` (`preview_und_ruecknahme_liegen_in_einem_append`) | `event_ord`-Abstand Befehl zu Rücknahme ist 1 (`:3221-3230`), Retry `idempotent_wiederholt` (`:3239-3249`) | Abstand 1 entsteht auch beim Schnitt (b); kein Absturz, keine 63 fremden Ereignisse |
| A4 `sonde013_verdrahtung.rs:2221` (`experimentbefehl_und_wirkung_sind_ein_append`) | fertiger Zustand nach störungsfreiem Lauf (laut `phase-08-broker.md:52`) | kein Zwischencommit; nicht im Einzelnen gelesen |
| A4-SI `store_crash_matrix.rs:2214-2236` (K-01 bis K-03 mit echtem C++-Client) | Kill vor Persist, vor und nach Commit; Retry `angewandt` bzw. `idempotent_wiederholt` | Gegenstelle `PhaseBSenke::p0` hängt **ein** Ereignis an (`broker/src/bin/eqcop-store-crash-worker.rs:103-128`, `:115`); kein Mehrereignisauftrag, kein Coordinator |
| A4-SI `store_crash_matrix.rs:2240` (`kill_nach_store_commit_vor_snapshot_push`) | Kill nach dem ersten `nach_store_commit` | ein Ereignis; die Naht `StoreTestHaken` (`handle.rs:91-110`, `writer.rs:426-432`) könnte den Schnitt aber treffen |
| A4 `writer.rs:970-1019` (xSync-Zählung) | ein 64er-Commit hat so viele `xSync` wie einer mit einem Ereignis | Durchsatz, nicht Ganzheit |

Kein Fixture betrifft den Schnitt. Die Behauptungszeilen A4 (`tools/beweise.ps1:518`) und
A4-SI (`:527`) nennen „Single-Writer" und „Killmatrix", nicht die Ganzheit eines Auftrags.

### e) Urteil: BESTÄTIGT

Die Kette schließt am HEAD; kein Commit seit `aff2d818` hat Writer oder Befehlspfad
berührt (Stand oben). Ergänzungen: (1) Ein SQLite-Fehler im zweiten Abschnitt führt ohne
Absturz im selben Brokerlauf zum falschen `idempotent_wiederholt` (hergeleitet). (2) Das
Schnittfenster ist bis zu ein Sammelfenster lang, nicht mikroskopisch. (3) B14s Beleg
(`event_ord`-Abstand 1) kann Schnitt und Ganzheit nicht unterscheiden. (4) Die Teilung ist die
Bauregel SONDE-011 Defekt 18; der Widerspruch zu B14 gehört als Zeile nach
`docs/offene-punkte.md`. (5) Die Einordnung von NAK-157 als Härtung beruhte auf der falschen
Prämisse „Produktpfad ist atomar".

### f) Kategorie und Schwere

**DEFEKT** gegen Entwurf §53.9 (`:4195-4198`), SONDE-011 K-02 (`:597`) und die B14-Regel
(`SONDE-013.md:4203`). **Schwere high** (Audit: critical): der Verlust ist still und dauerhaft
(Userurteil, Versuchsbeginn oder Rücknahme fehlen, der Befehl gilt als bestätigt), braucht aber
einen Absturz oder SQLite-Fehler genau während ein Befehlsauftrag über der Batchgrenze liegt.

### g) Kleinster Fix

**Ort:** `writer_lauf`, `broker/src/store/writer.rs:268-287`, dazu `StoreHandle::append_einreihen`
(`handle.rs:156-171`).

1. **Ein Auftrag mit höchstens 64 Ereignissen wird nie geteilt.** Passt der nächste Auftrag
   nicht mehr in den laufenden Commit (`anzahl + job.rest.len() > COMMIT_BATCH_MAX` bei
   `anzahl > 0`), schließt die Sammlung und committet, was schon da ist; der Auftrag beginnt
   den nächsten Commit ganz. Die Grenze „höchstens 64 Ereignisse je Transaktion" (SONDE-011
   Defekt 18, `append_gruppe` `:411-416`) bleibt erhalten; die Kanalreihenfolge ebenfalls,
   weil ein späterer Auftrag nie an einem früheren vorbeigezogen wird (die Sammlung bricht ab,
   statt weiterzusuchen).
2. **Ein Auftrag mit mehr als 64 Ereignissen** wird an der Annahmekante abgewiesen
   (`Err`, kein Produktaufrufer erzeugt ihn), statt geteilt. Die Alternative „als eine
   Transaktion über 64 schreiben" hebt die Cap-Regel auf; die Abweisung ist kleiner.
3. Kommentare `befehl.rs:414-421`, `writer.rs:248-251`, `invalidierung_verdrahtung.rs:290-292`,
   `experiment_verdrahtung.rs:207-208`, `sonde013_verdrahtung.rs:3227-3229` nachziehen.

Keine Schemaänderung, keine Store-Migration (das Tabellenformat bleibt; ein Altstand mit
bereits geteiltem Befehl bleibt, wie er ist — ob ein Reparaturlauf alte Teilstände sucht, ist
eine eigene Frage, siehe „Nicht geprüft"). Kein neues Fixture. **Mitzuprüfende Beziehungen:**
Crash↔Retry (Kill zwischen den zwei möglichen Commits, danach Retry derselben `command_id`:
Riegel und Wirkung gemeinsam da oder gemeinsam weg, genau eine Wirkung), senden↔bestätigen
(ACK weiter erst nach dem Commit des ganzen Auftrags), Backpressure (`STORE_KANAL_CAP` 256
unverändert; ein früher geschlossener Commit erhöht die Commitzahl unter Last höchstens um
einen je Batch). Audio-Thread: nicht berührt. Pipe-Latenz: ein Befehl kann bis zu ein
Sammelfenster später committen als heute; das P0-ACK-Budget (A22, A24) mitmessen.

### h) Ort des Rotbeweises

**Bein A4-SI** (`store_crash_matrix`, `--ignored`, `tools/beweise.ps1:520-528`), neuer Fall
neben `kill_nach_store_commit_vor_snapshot_push` (`store_crash_matrix.rs:2240`): der
Crash-Worker (`eqcop-store-crash-worker.rs`, neue Aktion) reiht einen Auftrag mit 63
Einzelereignissen und danach `[command(id X), evidence_invalidate]` ein, Haken
`nach_store_commit` (`handle.rs:91-110`) hält nach dem ersten Commit, hart killen, dieselbe
DB öffnen. Assertion: `command` mit X vorhanden ⇔ `evidence_invalidate` vorhanden; danach ein
Coordinator auf derselben DB und der Retry `preview_begin` mit X: Rücknahme genau einmal im
Log, Evidenzzeilen tragen den Grund. **Heute rot:** Riegel da, Rücknahme fehlt, Retry
`idempotent_wiederholt` ohne Rücknahme.

**Bein A4**: `store_crash_matrix.rs:1701-1721` wird umgeschrieben — 63+2 ergibt zwei Commits
mit `groesster_commit == 63`, und b steht ganz im zweiten; dazu die Ränder 62+2 (ein Commit,
64), 63+1 (ein Commit, 64) und ein Auftrag mit 65 Ereignissen (abgewiesen). Das ist eine
Matrixänderung mit Beleg (SONDE-011 Defekt 18), kein stilles Umbiegen eines grünen Tests.

Die Zeile, die die Zusage trägt, ist die Batchbildung `writer.rs:272-284`: eine Mutation, die
`:278` wieder auf `(COMMIT_BATCH_MAX - anzahl).min(job.rest.len())` für Aufträge unter 64
zurückstellt, muss beide Fälle reißen.

---

## 2. T3-08-02 — ein im Cache ausgeschlossener Beleg wird ohne Ausschluss gespeichert (W14)

*Befundzeile: `BEFUNDE.md:452` · Skeptikerzeile Phase 10: `:540` · Bündel B15: `:85` ·
Arbeitsübersicht W14: `:817` · Rohbeleg: `roh/phase-08-broker.md:58-76` · Register NAK-167:
`docs/offene-punkte.md:193`*

### a) Behauptung des Audits

Ein im Coordinator bereits ausgeschlossener Beleg kann anschließend ohne Ausschluss gespeichert
werden und kehrt nach dem Wiederanlauf als gültig zurück; derselbe Unterschied zwischen Cache
und Store entsteht auch ohne Nebenläufigkeit beim Rücksprung der Projektzeit mit neuer Epoche
(`BEFUNDE.md:452`).

### b) Quellkette am HEAD

**Station 1 — der Beleg steht zuerst im Cache.** `broker/src/coordinator/evidenz.rs:188-199`
(unter dem Standlock):

```
            // Riegel UND Zaehlung UND Insert unter DEMSELBEN Lock (M-63).
            if !Self::evidence_dispatch_locked(&mut stand, &session) {
                return false;
            }
            let historie = stand.evidenz.entry(key.clone()).or_default();
            let mut eintrag = stand_neu;
            …
            eintrag.empfangsfolge = self.evidenz_folge.fetch_add(1, Ordering::SeqCst);
            let folge = eintrag.empfangsfolge;
            historie.push_back(eintrag);
```

Danach wird der Lock freigegeben (`:204`).

**Station 2 — der Transportbruch läuft VOR der Ablage.** `evidenz.rs:210`
`self.invalidierung_aus_transportbruch(&client_key);`, erst dann `:224`
`if !self.evidenz_persistieren(&client_key, &wert, empfangsfolge) {`. Die Ablage schreibt
ausschließlich ID, Empfangsfolge und den unveränderten Snapshot (`:462-466`) und wartet auf den
Group-Commit (`:479` `store.append(vec![event])`, also bis zu 50 ms plus Commitzeit).

**Station 3 — die Rücknahme markiert im Cache, der Store markiert nur vorhandene Zeilen.**
`invalidierung_verdrahtung.rs:156-183`: alle Quellen der Sitzung (`:156-161`), jeder noch
nicht ausgeschlossene Eintrag im Umfang bekommt den Grund (`:173-179`
`eintrag.ausschlussgrund = Some(invalidierung.grund.wort().to_string());`). Die Projektion im
Writer, `writer.rs:654-665`:

```
    let mut abfrage = tx.prepare(
        "SELECT evidence_id, state_jcs FROM evidence \
         WHERE project_binding_id=?1 AND session_epoch=?2",
    )?;
```

— eine Zeile, die es noch nicht gibt, kann sie nicht markieren (`:676-719`). Die spätere
Ablage des Belegs (Station 2) legt die Zeile ohne `ausschlussgrund` an; `projektionen_anwenden`
wendet frühere Rücknahmen nicht nach (`writer.rs:579-604`).

**Station 4 — der Restore glaubt dem Store.** `experiment_verdrahtung.rs:1504-1508`:

```
            // Der Ausschlussgrund steht NEBEN dem Snapshot: er ist eine
            // Aussage ueber den Beleg, nicht Teil der Wire-Wahrheit.
            if let Some(grund) = zeile.get("ausschlussgrund").and_then(Value::as_str) {
                eintrag.ausschlussgrund = Some(grund.to_owned());
            }
```

Ohne Grund in der Zeile ist der Beleg nach dem Neustart gültig.

**Weg A — nebenläufig (Audit).** Der P0- und der P1-Verbraucher eines Links laufen auf
getrennten Threads (`broker/src/transport/server_v3/verbindung.rs:593-594` und `:622-623`),
mehrere Links ohnehin. Zwischen Station 1 und dem Commit von Station 2 läuft ein
`preview_begin`: `befehl.rs:932-934` markiert die ganze Sitzung samt E im Cache
(`preview_invalidierung_vorbereiten`, `invalidierung_verdrahtung.rs:590-601`), `:939-940`
committet Befehl und Rücknahme — E steht noch nicht in SQLite. Dann legt P1 E ohne Grund ab.
Dasselbe gilt für jede andere Rücknahme im Fenster (Hörmarkerende `befehl.rs:877-886`,
Kanal- und Messpunktwechsel, Material). Das Fenster ist die Commitwartezeit von E, nicht ein
Mikrosekundenrennen: bei acht Sonden mit 4 Hz ist fast immer ein Beleg in diesem Zustand
(hergeleitet).

**Weg B — seriell, deterministisch (Audit).** Neuer Beleg E mit neuer `transport_epoch` und
kleinerem Projektstart als das Ende des alten Belegs. `invalidierung_aus_transportbruch`,
`invalidierung_verdrahtung.rs:641-651`:

```
        let umfang = match start {
            …
            Some(von) => Umfang::Bereich {
                von: 0,
                bis: von.saturating_add(anzahl as i64),
            },
```

Der Bereich `[0, altes Ende)` erfasst E, das seit Station 1 im Cache steht (Überlappung
`invalidierung.rs:96-105`). Die Rücknahme committet (`:389`) vor E (`evidenz.rs:224`). Cache:
E ausgeschlossen. Store: E gültig. **Das tritt bei jedem Rücksprung mit neuer Epoche ein,
also bei jedem Loop-Wrap** (Abschnitt 4). Auditbeispiel: alt `[48000,96000)`, neu `[0,48000)`
→ Bereich `[0,96000)` trifft neu im Cache.

**Wer irrt in Weg B.** Der Kommentar derselben Funktion sagt, was gewollt ist,
`invalidierung_verdrahtung.rs:613-616`: „Beide invalidieren ausdruecklich NUR den Bereich VOR
dem Bruch: was danach kommt, ist neues, gueltiges Material." E ist Material nach dem Bruch.
Im seriellen Weg irrt also der Cache, der Store liegt zufällig richtig; im nebenläufigen Weg
irrt der Store. Der Kommentar ist für Rücksprünge irreführend: der Bereich in Projektzeit
bildet „vor dem Bruch" nur bei Vorwärtssprüngen ab.

**Zahlenränder.** Bereich halboffen: ein neuer Beleg, der genau am alten Ende beginnt
(`start == alt.start + alt.count`), wird nicht erfasst, einer eine Sample davor schon
(`invalidierung.rs:104`). Ohne Projektzeit auf der alten Seite wird die ganze Sitzung
zurückgenommen (`invalidierung_verdrahtung.rs:652-654`), E ohne Projektzeit fällt bei `sample_range`
aus der Projektion (`writer.rs:695-702`) — auch dort weicht nur der Cache ab.

### c) Zusage wörtlich

- SONDE-013 M-52, `docs/beweise/SONDE-013.md:547`: „Bereits eingegangene Evidenz wird per ID
  oder Bereich invalidiert."
- Testregel `broker/tests/sonde013_verdrahtung.rs:1529`: „jede Zeile der Sitzung traegt den
  Grund - haltbar, nicht nur fluechtig".
- Entwurf §33.5, `FL-Nakama-Sonden-Design-Entwurf.md:2165`: „Kein Objekt darf gleichzeitig zwei
  still konkurrierende Wahrheiten besitzen"; `:2171`: Evidenz hat als autoritative Ablage den
  „lokale[n] SQLite-Experimentstore mit einem Writer".
- Entwurf §34.2, `:2227`: jede Evidenz besitzt einen „Ausschlussgrund, falls sie nicht mehr
  gültig ist."
- Code-Selbstaussage `experiment_verdrahtung.rs:209-211`: „Ein Speicher, der dem Log voraus
  ist, waere die zweite Wahrheit aus §33.5."
- Arbeitsübersicht W14 (`BEFUNDE.md:817`): „Rückgenommene Evidenz bleibt zurückgenommen; …
  Store und Snapshot widerspruchsfrei."
- Register NAK-167 (`offene-punkte.md:193`) kennt die Reihenfolge („`evidenz.rs:196`
  invalidiert und pusht, `:210` hängt an") nur für den gescheiterten Append und sagt: „Eine
  gemeinsame Transaktion ist nirgends zugesagt". Das stimmt; zugesagt ist aber, dass Cache und
  Store nicht auseinanderlaufen (§33.5).

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| A4 `sonde013_verdrahtung.rs:1503-1535` (`invalidierung_projiziert_ihren_grund_in_die_evidenzzeilen`) | drei **bereits gespeicherte** Belege tragen nach der Rücknahme den Grund | kein Beleg zwischen Cache und Store |
| A4 `:1117-1153` (`transportbruch_invalidiert_mit_seinem_grund`) | Zähler 1 und **irgendein** Beleg mit dem Grund (`:1142-1145` `.any(…)`) | Coordinator **ohne** Store (`coordinator()` `:86-90`); beide Belege tragen denselben Projektstart der Fixture (44 108 200, 512 Samples), der neue wird also mit ausgeschlossen, und `any` prüft nicht welcher |
| A4 `:3325-3382` (`gescheiterter_append_laesst_keinen_ausschluss_stehen`) | gescheiterter Append: Cache, Log und `neuer_coordinator()` (`:380-382`) gleich | gelingender Append; Muster für den Neustartvergleich ist aber da |
| A4 `:4168` (`evidenzreihenfolge_ueberdauert_den_neustart`) | Empfangsfolge nach Neustart | nicht im Einzelnen gelesen |

Kein Fixture. Keine Testnaht zwischen Cache-Einfügen und Ablage (`evidenz.rs:204-224`).

### e) Urteil: BESTÄTIGT

Beide Wege schließen am HEAD; NAK-313 hat nur das Lesen der Ganzzahlen geändert
(`evidenz.rs:272-288`), nicht den Ablauf. Ergänzungen: (1) Weg B tritt bei jedem Loop-Wrap
ein; dort irrt der Cache (Kommentar `:613-616`), und seine Ursache ist der Bereich des
Transportbruchs, der zu T3-08-05 gehört. (2) Weg A ist kein Mikrosekundenfenster, sondern die
Commitwartezeit der Ablage. (3) Betroffen ist jede Rücknahme ohne gemeinsame Ordnung mit der
Ablage, nicht nur die Preview.

### f) Kategorie und Schwere

**DEFEKT** gegen M-52 („haltbar", Testregel `:1529`) und Entwurf §33.5 (`:2165`). **Schwere
medium** (Audit: high): der Fehler ist still und nach einem Neustart dauerhaft, trifft aber je
Rücknahme nur die Belege im Ablagefenster (typisch einer je Quelle), und diese stammen meist
aus der Zeit vor dem Eingriff.

### g) Kleinster Fix

**Ort:** `evidence_snapshot_json_mit_minor_wert`, `evidenz.rs:135-262`, `evidenz_persistieren`
`:444-485`, `invalidierung_anwenden_mit_rueckweg` und `invalidierung_vorbereiten`
(`invalidierung_verdrahtung.rs:90-237`), Preview-Zweig `befehl.rs:927-940`.

Regel: **Store-Reihenfolge gleich Cache-Reihenfolge je Sitzung.** Eine Ordnungssperre je
Sitzung (die vorhandenen `session_flush_schloesser` oder eine eigene) umfasst (a) Cache-Einfügen
und `append_einreihen` des Belegs und (b) Markieren und `append_einreihen` jeder Rücknahme; das
**Warten** auf den Commit liegt außerhalb der Sperre (Muster R-283-1,
`hypothese_verdrahtung.rs:663-686`, und `handle.rs:153-171`). Der Writer committet in
Kanalreihenfolge (`writer.rs:268-284`), eine im selben Commit folgende Rücknahme sieht die
Zeile des Belegs (`append_gruppe` `:418-425` arbeitet die Aufträge der Reihe nach ab). Im
P1-Weg wird die Ablage des Belegs vor den Transportbruch gezogen (`evidenz.rs:210` hinter
`:224`); scheitert sie, bleibt der heutige Rückweg (`:224-235`). Im Preview-Weg wandert das
Markieren (`befehl.rs:932-934`) unter dieselbe Sperre wie der Append (`:172-176`). Der Bereich
des Transportbruchs (Weg B) wird in Abschnitt 4 g) korrigiert; ohne ihn wäre E danach in Cache
**und** Store ausgeschlossen — widerspruchsfrei, aber zu Unrecht.

Keine Schemaänderung, keine Migration. Altbestände mit fehlendem Grund bleiben, wie sie sind
(siehe „Nicht geprüft"). **Mitzuprüfende Beziehungen:** invalidieren↔wiederherstellen (Cache,
Evidenzzeile und `neuer_coordinator()` gleich), Crash↔Retry (Kill zwischen Beleg- und
Rücknahmecommit: beide oder keiner, oder der Beleg ohne Rücknahme **nur**, wenn die Rücknahme
danach nicht committet hat — dann steht sie auch nicht im Cache eines neuen Laufs), M-63 (Riegel,
Zählung und Einfügen bleiben unter dem Standlock). Audio-Thread: nicht berührt. Pipe-Latenz:
die Sperre darf das Warten nicht umfassen; sonst serialisieren 32 Sonden je Sitzung ihre
Commits zu je bis zu 50 ms (hergeleitet), und der P1-Verbraucher staut — das berührt T3-16-03
(Teil C).

### h) Ort des Rotbeweises

**Bein A4**, `sonde013_verdrahtung.rs`, `HarnischMitStore`:
- *Seriell*, neben `:1117`: Quelle mit Beleg alt `[48000,96000)` in Epoche 1, dann neu
  `[0,48000)` in Epoche 2 (eigene IDs, 48 kHz). Assertion: Ausschluss von neu in
  `evidenz_historie`, in der Zeile `evidence` und nach `neuer_coordinator()` gleich. **Heute
  rot** (Cache ausgeschlossen, Zeile und Neustart gültig).
- *Nebenläufig*, neben `:1503` und `:3325`: eine neue Testnaht (Muster `flush_test_haken`,
  `flush.rs:138-145`) hält P1 nach dem Einfügen und vor der Ablage; währenddessen
  `preview_begin`; freigeben; Cache, Zeile und `neuer_coordinator()` vergleichen. **Heute rot.**

Die Zeile, die die Zusage trägt, ist die neue Ordnung um `evidenz.rs:192-224` und
`befehl.rs:932-940`: wer die Sperre entfernt oder die Ablage wieder hinter den Transportbruch
setzt, muss den nebenläufigen bzw. den seriellen Fall reißen.

---

## 3. T3-08-03 — zwischen Commit und Einreihen fehlt die Ordnung (W14)

*Befundzeile: `BEFUNDE.md:453` · Skeptikerzeile Phase 10: `:541` · Bündel B15: `:85` ·
Arbeitsübersicht W14: `:817` · Rohbeleg: `roh/phase-08-broker.md:78-92` · Register NAK-269:
`docs/offene-punkte.md:277`*

### a) Behauptung des Audits

Zwischen Commit und Einreihen fehlt eine gemeinsame Ordnungssperre: Rücknahme N kann hinter
Vollsnapshot N+1 eingereiht werden; die reparierte Koaleszierung R-283-2 ordnet nur bereits
Eingereihtes; der C++-Verbraucher setzt den jüngeren Stand dann wieder invalid/stale
(`BEFUNDE.md:453`).

### b) Quellkette am HEAD

**Station 1 — die Rücknahme committet ohne Sitzungssperre und reiht danach ein.**
`invalidierung_verdrahtung.rs:96-113`: `invalidierung_vorbereiten` (`:96`, markiert unter dem
Standlock), `invalidierung_alleine_persistieren` (`:100`, Append `:389`), dann

```
            Ok(Some(event_ord)) => {
                self.invalidierung_zustellen(&wirkung, event_ord);
                self.paare_bei_bedarf_bilden();
                self.befunde_nach_ruecknahme_zustellen(session, betroffen);
```
(`:109-112`). Keiner dieser Schritte nimmt `session_flush_schloesser`. Die Zustellung,
`:426-429`:

```
            let geschrieben = self.push_ziel_noch_gueltig(&link_id, ziel)
                && push.as_ref().is_some_and(|push| {
                    push.snapshot_schreiben(&link_id, &ziel.object_key, event_ord, &payload)
                });
```

Im Preview-Weg committet der Befehl unter der Sitzungssperre (`befehl.rs:172-176`, bis zur
Rückkehr von `persistenz_p0_mit_domaene_und_ords`), zugestellt wird **nach** deren Freigabe
(`befehl.rs:939-945`, `:944` `self.invalidierung_zustellen(&w, ord);`).

**Station 2 — der Flush serialisiert nur bis zum Commit.** `flush.rs:58-63` nimmt die
Sitzungssperre, erfasst unter dem Standlock (`:80-136`), committet (`:147-171`), Marke =
`event_ord` (`:179-185`), dann:

```
        // Die Reihenfolge ist bis einschliesslich Store-/Outbox-Commit
        // serialisiert. Externe Pipe-Arbeit laeuft danach ohne dieses Schloss;
        // eine Senke darf den Coordinator reentrant beobachten, ohne dieselbe
        // Session zu deadlocken.
        drop(_flush_guard);
```
(`:187-191`), Push `:209-214`.

**Station 3 — Einreihen und Warten sind ein Aufruf.** `V3Sender::snapshot_schreiben`,
`broker/src/transport/server_v3/griff.rs:81-106`: Frame bauen, `snapshot_einreihen_mit_antwort`
(`:101`), dann `antwort.recv_timeout(SENKE_FRIST)` (`:105`). Wer das Einreihen ordnen will,
muss es vom Warten trennen.

**Station 4 — die Queue ordnet je Schlüssel, über Schlüssel nur nach Ankunft.**
`queues.rs:227-236`:

```
                let schluessel = objekt_schluessel.clone();
                let hochwasser = g.2.get(&schluessel).copied();
                …
                if hochwasser.is_some_and(|hoch| eintrag.marke < hoch) {
                    self.nachzuegler_verworfen.fetch_add(1, Ordering::SeqCst);
                    false
```

`session_snapshot` und `evidence_invalidate` haben getrennte Hochwasser; N ist kein Nachzügler
zu N+1 des anderen Schlüssels. Eine Ersetzung wandert ans Ende (`:237-253`, R-283-2), ein neuer
Schlüssel ebenso (`:257-258`); entnommen wird P0 zuerst, sonst von vorn (`:305-316`). Die Marke
reist nicht auf dem Draht (`broker/src/coordinator/mod.rs:145-146`: „Schluessel und Marke reisen
nie im Frame; die Wire-Form ist unberuehrt.").

**Station 5 — der Verbraucher kennt kein Alter und wirkt global.**
`eq-copilot/plugin/src/SourcesModel.cpp:1516` verlangt genau `type`, `grund`, `umfang`; kein
Ordnungsfeld (Schema `eq-ipc-v3.schema.json:1208-1212`, `additionalProperties: false`). Die
Wirkung, `SourcesModel.cpp:1625-1643`:

```
    std::lock_guard<std::mutex> l (mutex);
    for (auto& [_, e] : eintraege)
        if (e.zeile.messung != Messung::missing)
            e.zeile.messung = Messung::invalid;
    …
    setzeAlleBefundeStale();
```

`session_snapshot` ersetzt die Befunde (`:1450` `befunde = std::move (geleseneBefunde);`); die
Messachse wird durch P2 wieder `fresh` (`:1698-1711`). Folge N+1, N: alle Messungen invalid, alle
Befunde stale, obwohl N+1 jünger ist.

**Präzisierung 1 — Dauer und Richtung.** Die Wirkung ist fail-closed (zeigt weniger, nicht
mehr) und vorübergehend: im Weg `invalidierung_anwenden` folgt bei `betroffen > 0` sofort ein
Flush N+2 (`invalidierung_verdrahtung.rs:112`, `:444-453`), der die Befunde ersetzt; P2 setzt
die Messachse binnen eines Livetakts zurück. Im Preview-Weg (`befehl.rs:939-952`) folgt **kein**
Flush — die Befunde bleiben stale bis zum nächsten `session_snapshot` aus anderem Anlass. Bei
`betroffen == 0` wird die Rücknahme trotzdem zugestellt (`invalidierung_verdrahtung.rs:96-113`,
`invalidierung_vorbereiten` liefert `Some` mit 0), und der heilende Flush entfällt (`:445`).

**Präzisierung 2 — die umgekehrte Lücke.** Im Weg `invalidierung_anwenden` liegen Markieren
und Commit **nicht** unter der Sitzungssperre des Flushs. Möglich ist daher: Flush B erfasst S
unter dem Standlock (vor der Markierung), Rücknahme A markiert und committet N, B committet S als
N+1. Dann ist N+1 **älter im Inhalt** als N. Wird N vor N+1 zugestellt, zeigt Gen am Ende S —
Befunde `ready` auf zurückgenommener Evidenz — bis der Flush N+2 aus Präzisierung 1 kommt (nur
bei `betroffen > 0` und gelingendem Append). Hergeleitet aus `flush.rs:58-171` und
`invalidierung_verdrahtung.rs:96-113`, nicht gemessen. **Folge für den Fix:** „nach `event_ord`
sortieren" allein ordnet falsch; Markieren und Commit der Rücknahme gehören unter dieselbe
Sitzungssperre wie Erfassen und Commit des Flushs.

**Irreführender Kommentar.** `flush.rs:173-178` („Die Commit-Reihenfolge ist unter dem Schloss
serialisiert; die Marke traegt sie ueber das Schloss hinaus bis in die Writerqueue") gilt nur
für Flushes und P0-Befehle derselben Sitzung, nicht für Rücknahmen ohne Befehl.
`invalidierung_verdrahtung.rs:23-25` („erst der lokale Ausschluss, dann die Ablage, dann die
Zustellung") beschreibt die Reihenfolge innerhalb einer Rücknahme, nicht gegenüber Flushes.

### c) Zusage wörtlich

- R-283-2, `docs/beweise/NAK-283.md:112`: „Die Koaleszierung erhält die Entstehungsreihenfolge
  über Schlüssel: eine Ersetzung wandert ans Ende der Deque (Reihenfolge ist die Zeit der
  jüngsten Marke), sodass Vollsnapshot und Rücknahme desselben Objekts in ihrer
  Entstehungsreihenfolge zugestellt werden. Der Consumer-Altersriegel aus NAK-269 bleibt eigene
  Härtung und ersetzt die Ordnung nicht."
- R-D5, `docs/beweise/NAK-246.md:1234-1244`: „Die Zustellordnung reicht bis in die
  Writerqueue: jeder Snapshot trägt vom Commit an eine monotone Ordnungsmarke … Kein
  Schemawechsel in dieser Runde … der Consumer-Riegel nach Alter ist Härtung (Register) und
  wird nicht gebaut. Der Flush-Lock bleibt vor der Zustellung frei (Reentranz), die Ordnung
  entsteht durch die Marke." M-23 (`:635`) verlangt für zwei Flushes: „Der zuletzt angenommene
  Zustand am Empfänger … ist der neue."
- R-D9, `NAK-246.md:1370-1374`: „… sodass die Rücknahme nie von einem Vollsnapshot koalesziert
  wird".
- Arbeitsübersicht W14 (`BEFUNDE.md:817`): „keine Zukunftssicht durch alte Nachricht
  zurückrollen".
- Register NAK-269 (`offene-punkte.md:277`): der Consumer-Altersriegel bleibt Härtung.

Die Zusagen verlangen das Ergebnis „in Entstehungsreihenfolge zugestellt"; sie decken ihren
Mechanismus nur ab dem Einreihen.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| A4 `broker/src/transport/server_v3/tests_abonnement.rs:1039-1096` (NAK-246 M-27) | Rücknahme und Vollsnapshot in angehaltener Queue, Zustellung in Einreihfolge | wartet ausdrücklich, bis die Rücknahme eingereiht ist (`:1047-1050`), und startet den Vollsnapshot erst danach (`:1051-1052`) — das strittige Fenster fehlt |
| A4 `tests_abonnement.rs` M-23 (bis `:1028`) | zwei Flushes, älterer committet und hält vor dem Einreihen | gleicher Schlüssel; Hochwasser greift |
| A4 `queues.rs` `mod tests` (NAK-283 M-17 bis M-20, `NAK-283.md:623-626`) | Ersetzung wandert ans Ende, Atomarität der Ersetzung | nur Queue |
| B13 `EqCopSonde012SourcesModelTest` (`tools/beweise.ps1:750`) | Leser `evidence_invalidate` und Wirkung | Verbraucher allein, keine Folge N+1, N |

Werkzeug für den Rotbeweis liegt bereit: `SperreVorDemSender` (`tests_abonnement.rs:1118-1125`).

### e) Urteil: PRÄZISIERT

Der Mechanismus trägt am HEAD (Stationen 1 bis 5). Anders als behauptet: (1) die
Verbraucherwirkung ist global und fail-closed und heilt im Weg `invalidierung_anwenden` sofort
(bei `betroffen > 0`), im Preview-Weg erst mit dem nächsten Snapshot; (2) es gibt zusätzlich die
umgekehrte Lücke (Erfassung vor Markierung, Commit danach), die Inhalt und `event_ord`
entkoppelt — ein Fix über die Marke allein wäre falsch. Verbraucherzeilen gewandert
(`:1505-1518` → `:1625-1643`).

### f) Kategorie und Schwere

**DEFEKT** gegen das Ergebnis von R-283-2 („in ihrer Entstehungsreihenfolge zugestellt") und
W14. **Schwere medium** (Audit: high): falsche Anzeige in beiden Richtungen möglich, ohne
Datenverlust und meist nach einem Takt geheilt; im Preview-Weg bleibt sie bis zum nächsten
Snapshot stehen.

### g) Kleinster Fix

**Ort:** `invalidierung_anwenden_mit_rueckweg` und `invalidierung_zustellen`
(`invalidierung_verdrahtung.rs:90-130`, `:405-434`), `flush_session` (`flush.rs:58-221`),
Preview- und Experimentzweig (`befehl.rs:927-952`, `experiment_verdrahtung.rs:307-329`),
`V3Sender::snapshot_schreiben` und `SessionPush` (`griff.rs:81-106`,
`coordinator/mod.rs:131-152`).

Regel **„Commitordnung bis Publikation je Sitzung"**: Unter derselben Sitzungssperre liegen
(a) Markieren bzw. Erfassen, (b) Commit und (c) **Einreihen** in die Ausgangsqueue; das Warten
auf den Pipe-Write liegt außerhalb. Dafür wird `snapshot_schreiben` in Einreihen und Warten
geteilt (Muster `append_einreihen`/`append`, `handle.rs:148-171`). Der Queue-Eintrag trägt die
Marke wie heute; R-283-2 hält die Reihenfolge danach. Die Reentranzbedingung aus R-D5 bleibt
gewahrt, weil nur das Einreihen (nicht blockierend, `queues.rs:210-303`) unter der Sperre
läuft. Kommentare `flush.rs:173-178`, `:187-191` nachziehen.

**Keine v3-Vertragsänderung:** die Ordnung entsteht vollständig vor der Queue; `session_snapshot`
und `evidence_invalidate` bleiben bytegleich (A5, A8, B3c unverändert). Der Consumer-Altersriegel
aus NAK-269 bleibt Härtung. **Mitzuprüfende Beziehungen:** committen↔veröffentlichen (Folge an der
Queue monoton in `event_ord` je Sitzung, auch über Schlüssel), Reentranz (eine Senke, die den
Coordinator beobachtet, deadlockt nicht — bestehender Fall `coordinator_model.rs:1266`
`senke_haelt_den_standlock_nicht_ueber_den_push`), Outbox-Kompaktierung (weiter erst nach
geschriebenem Frame). Audio-Thread: nicht berührt. Pipe-Latenz: unverändert, der Write selbst
bleibt außerhalb.

### h) Ort des Rotbeweises

**Bein A4**, `tests_abonnement.rs` neben `:1039-1096`: `SperreVorDemSender` scharf stellen,
`invalidierung_wegen_messpunkt_fuer_link` starten — sie committet N und hält vor dem Sender;
währenddessen ein Flush (Descriptor setzen) N+1 committen und einreihen; Sperre lösen.
Assertion: die Zustellfolge an der Queue ist N, N+1 (oder N entfällt als verdrängt) — **heute
rot** (N+1, N). Zweiter Fall für Präzisierung 2 mit dem Flushhaken `flush.rs:138-145`: Flush
erfasst, Rücknahme markiert und committet, Flush committet; der letzte Stand am Empfänger trägt
den Ausschluss. **B13** ergänzt den Verbraucherfall N+1, N nur als Wirkungsbeleg, nicht als
Zusagezeile. Die Zeile, die die Zusage trägt, ist die neue Sitzungsordnung zwischen Commit und
Einreihen; entfernt man sie, muss der A4-Fall reißen.

---

## 4. T3-08-05 — der Loop-Wrap nimmt abgeschlossene Evidenz zurück (W28)

*Befundzeile: `BEFUNDE.md:455` · Skeptikerzeile Phase 10: `:543` · Bündel B22: `:92` ·
Arbeitsübersicht W28: `:831` · Rohbeleg: `roh/phase-08-broker.md:114-122` · Register NAK-296:
`docs/offene-punkte.md:304`*

### a) Behauptung des Audits

Auch ein Loop-Wrap genau an der gebundenen Passagengrenze invalidiert im Broker die
abgeschlossene Evidenz; die Ausnahme `LoopWrapAnGrenze` hat nur Testaufrufer, der produktive
Pfad wählt bei höherer `transport_epoch` bedingungslos den Epochwechsel, und der echte
Frame-Erzeuger erhöht beim Loop-Wrap die Epoche (`BEFUNDE.md:455`).

### b) Quellkette am HEAD

**Station 1 — der Erzeuger erkennt den Wrap und erhöht die Epoche.**
`eq-copilot/plugin/core/analysis/featureengine/Zeit.h:213-222`: bewegte Projektzeit, die nicht
anschließt, ist `istLoopWrap (alt, neu) ? Grenzgrund::loopWrap : Grenzgrund::zeitSprung`.
`istLoopWrap` (`:242-260`): Schleife aktiv und rückwärts; ohne gültige Grenzen
„`return true; // Schleife an, rueckwaerts: reicht`" (`:249-250`). `grenzeZiehen`, `:512-522`:

```
    if (grund == Grenzgrund::lokaleLuecke)
    {
        ++segmentInEpoche;          // §32.3: SEGMENT, nicht Epoche
        ++zSegmentwechsel;
    }
    else
    {
        ++transportEpoche;
        segmentInEpoche = 0;        // Segmente zaehlen INNERHALB einer Epoche
        ++zEpochenwechsel;
    }
```

Liegt das Schleifenende rechnerisch im Block, wird der Block vorher als möglicher Straddle
verworfen (`eq-copilot/plugin/core/analysis/FeatureEngine.h:888-895`,
`grenzeZiehen (Grenzgrund::moeglicherStraddle)`) — ebenfalls eine Epochengrenze. **Zahlenrand:**
ein Wrap kostet +1 Epoche (Schleifenende auf der Blockgrenze) oder +2 (Straddle, dann Wrap).

**Station 2 — der Grund bleibt lokal.** Der Stempel trägt nur Epoche und Segment
(`featureengine/Frame.h:226-227` `t.transport_epoch = transportEpoche;`
`t.continuity_segment = segmentInEpoche;`) und die rohe Schleife (`:300-319`: `cycle_active`,
PPQ-Grenzen, `cycle_derivation = Herleitung::unproven` „IMMER"). Das Feld für den Grund gibt es
nicht (`featureengine/Vertrag.h:122-164`); `Grenzgrund` wird nur gezählt (`FeatureEngine.h:1798`
`grundZaehler`). Der Draht (`eq-copilot/plugin/vertrag/NakamaEvidenz.cpp:195-197`, `:239-253`)
schreibt `transport_epoch`, `continuity_segment` und, wenn beide PPQ-Grenzen tragbar sind,
`cycle {active, bounds_valid, start_ppq, end_ppq}`; weder Projektposition in PPQ noch Tempo.

**Station 3 — der Broker entscheidet nur über Epoche und Segment.**
`invalidierung_verdrahtung.rs:626-635`:

```
            let neu = &historie[historie.len() - 1];
            let alt = &historie[historie.len() - 2];
            let grund = if neu.transport_epoch != alt.transport_epoch {
                Some(Grund::Epochwechsel)
            } else if neu.continuity_segment != alt.continuity_segment {
                Some(Grund::Sequenzluecke)
            } else {
                None
            };
```

Umfang `[0, alt.start + alt.count)` (`:648-651`), Aufruf ohne Rücksicht auf Passage oder
Schleife (`:656`). `Evidenzstand` hält die Schleife gar nicht (`evidenz.rs:104-110`). Die
Markierung trifft **alle Quellen der Sitzung** (`invalidierung_verdrahtung.rs:156-161`).
`!=` statt `>`: +1, +2 und eine kleinere Epoche lösen gleich aus.

**Station 4 — die Ausnahme existiert, aber nur für Tests.** `invalidierung.rs:146-155`:

```
pub fn grund_fuer_bruch(art: Bruchart) -> Option<Grund> {
    match art {
        // „Drop zählt als Segment, alles andere als Epoche" (SONDE-009).
        Bruchart::Drop => Some(Grund::Sequenzluecke),
        Bruchart::Seek | Bruchart::LoopWrapInnerhalb | Bruchart::AndererKontinuitaetsbruch => {
            Some(Grund::Epochwechsel)
        }
        Bruchart::LoopWrapAnGrenze => None,
    }
}
```

`git grep grund_fuer_bruch|Bruchart|LoopWrapAnGrenze` über `broker/`, `eq-copilot/`, `tools/`:
nur die Definition (`invalidierung.rs:128-155`) und `broker/tests/sonde013_taint.rs:20-21`,
`:109-139`.

**Station 5 — die gebundene Passage kennt der Broker nur nach `experiment_begin`.** Die
Passage (`projekt_von`, `projekt_bis` in Projektsamples, `transport_epoch`) reist nur in
`experiment_begin` (Schema `eq-ipc-v3.schema.json:1480-1512`, Gen schreibt sie in
`eq-copilot/plugin/src/prozessor/Analyse.cpp:918-919`). Einziger Absender ist `beginneVersuch`,
einziger Aufrufer `assistentVersuchStarten` (`Analyse.cpp:1312`), und diesen ruft kein
Bedienweg (`git grep` über `eq-copilot/plugin/src` und `…/sonde`; ebenso Teil A von NAK-313,
Abschnitt 4). Die Markierung `merkeManuellePassage` (`prozessor/State.cpp:438`) hat keinen
Produktaufrufer. **Der Passagenfall von M-53 ist heute also Stufe 2; das Verhalten „jeder
Loop-Wrap nimmt zurück" ist Stufe 1**, sobald Probeeq läuft und FL loopt — das Diagnoseprojekt
loopt bei 45,696 s (`docs/offene-punkte.md:304`, NAK-296).

**Folge je Wrap** (Loop `[von, bis)`, letzter Beleg `[x, x+n)` mit `x+n ≤ bis`, erster danach
`[von, …)`): Bereich `[0, x+n)` nimmt den ganzen abgeschlossenen Durchlauf aller Quellen der
Sitzung zurück, dazu im Cache den ersten Beleg nach dem Wrap (Abschnitt 2, Weg B). Beim nächsten
Wrap fällt der zweite Durchlauf. Mehr als ein Durchlauf Evidenz entsteht im Loopbetrieb nie.

**Irreführende Kommentare.** `invalidierung_verdrahtung.rs:606-609` („eine neue
`transport_epoch` heisst Seek oder Loop-Wrap → `epochwechsel`") nennt M-53 als Quelle, lässt
aber die Grenzausnahme aus; `:613-616` (nur „VOR dem Bruch") stimmt für Rücksprünge nicht.
`Zeit.h:237-241` („Loop-Wrap und Seek sind BEIDE Epochengrenzen … Sie unterscheidet nur die
Telemetrie") — die Unterscheidung erreicht die Telemetrie nicht (Station 2).

### c) Zusage wörtlich

- SONDE-013 M-53, `docs/beweise/SONDE-013.md:548`: „Ein Seek, ein Loop-Wrap **innerhalb** der
  Passage (Loop kürzer als die Passage) oder ein Drop während der laufenden Passagenmessung
  invalidiert diese Messung — jeweils mit dem oben je Auslöser festgelegten Grund — und die
  Messung beginnt mit der nächsten gültigen Epoche neu … Ein Loop-Wrap **an** der
  Passagengrenze (Loop deckt die Passage vollständig) beendet einen Durchlauf und beginnt den
  nächsten — das ist **keine** Invalidierung und **keine** Herabstufung (Abgrenzung zu M-23)."
- Entwurf §32.3, `FL-Nakama-Sonden-Design-Entwurf.md:1995-1999`: „Technisch hält deshalb jede
  Instanz mindestens den letzten Block beziehungsweise noch nicht abgeschlossene Fenster in einer
  Ein-Block-Quarantäne. Erst der monotone Beginn des Folgeblocks versiegelt sie zur
  Veröffentlichung. Stoppt der Callback vorher, bleiben sie unvollständig. So kann ein erst
  nachträglich sichtbarer Loop-Wrap keine bereits persistierte Evidenz kontaminieren; eine
  spätere Rücknahme ist nur zusätzlicher Crash-Fallback, nicht der Normalpfad."
- Entwurf §32.3, `:1955-1956`: „Ein Sprung, Stop/Start, Loop-Wrap, Sampleratewechsel oder
  Hostreset beginnt eine neue `transport_epoch`." (Der Erzeuger hält das.)
- Absicht im Code, `invalidierung.rs:142-145`: „Ein `Some(Epochwechsel)` an dieser Stelle nähme
  dem User bei jedem Loopdurchlauf seine Evidenz — und zwar genau bei der Arbeitsweise, für die
  Passagen gedacht sind."
- Schema `eq-ipc-v3.schema.json:1510-1511` (Passage): „Eine Passage bindet an GENAU EINE
  Transportepoche (Paragraph 32.4)."
- `CLAUDE.md:187-188` („Zeit ist aktive Musikzeit") schweigt zum Loop-Wrap.

Die Quellen **schweigen** zu: Loop ohne gebundene Passage, Loop länger als die Passage, und zum
Umfang einer Seek-Rücknahme (nur „diese Messung", nicht „alles davor").

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Warum sie den Befund nicht fängt |
|---|---|---|
| A4 `sonde013_taint.rs:105-144` (`epoch_change_invalidates_open_evidence`, M-53) | die reine Zuordnung `grund_fuer_bruch`, `:127` `LoopWrapAnGrenze → None` | kein Produktaufrufer; die Matrix M-53 nennt den Loopfall „gemessen", gemessen ist nur der Helfer |
| A4 `sonde013_verdrahtung.rs:1117-1153` (`transportbruch_invalidiert_mit_seinem_grund`) | jede neue Epoche löst genau eine Rücknahme aus | **schreibt „jede Epoche nimmt zurück" fest**; ohne Schleife, ohne Passage, ohne Store |
| B5 `EqCopAnalysisGoldenTest` (`tools/beweise.ps1:719`), G4 `AnalysisGoldenTestMain.cpp:3567-3584` | der Erzeuger zählt `loopWrap`, trennt jedes Fenster | Pluginseite; kein Broker |
| B23 `EqCopSonde013PassageStateTest` (`tools/beweise.ps1:759`) | Passage als Projektintent | kein Broker |
| A24 `pruefe_session_soak.py` (`tools/beweise.ps1:638`) | Dauerlauf mit Evidenzpfad | kein Loop (nicht im Einzelnen gelesen) |
| Laufzeit-Arm, Diagnoseprojekt | FL loopt bei 45,696 s (NAK-296) | ob ein Szenario die Rücknahmen je Wrap misst, habe ich nicht geprüft |

Kein Fixture mit `cycle.active = true` und neuer Epoche im Broker-Korpus gefunden (nicht
vollständig durchsucht, siehe „Nicht geprüft").

### e) Urteil: PRÄZISIERT

Die Kette trägt am HEAD (Zeilen gewandert: `Zeit.h:516` → `:519`, `Frame.h:221` → `:226`).
Anders und breiter als behauptet: (1) **Jeder** Loop-Wrap — mit oder ohne Passage — und jeder
Seek oder Stop/Start nimmt alle Evidenz **aller Quellen** der Sitzung vor dem alten Frameende
zurück, dazu im Cache den ersten Beleg danach. (2) Die gebundene Passage kennt der Broker nur
nach `experiment_begin`, das heute keinen Bedienweg hat; der wörtliche M-53-Fall ist Stufe 2,
die Wirkung im Loopbetrieb Stufe 1. (3) Ein Wrap kostet +1 oder +2 Epochen. (4) Neben M-53
bricht der allgemeine Loopfall Entwurf §32.3 (`:1998-1999`). Kein Commit seit `aff2d818` hat
den Pfad berührt.

### f) Kategorie und Schwere

**DEFEKT** gegen M-53 (Wrap an der gebundenen Passagengrenze) und gegen Entwurf §32.3
`:1998-1999` (Rücknahme nach einem Loop-Wrap ist kein Normalpfad). **LÜCKE, Technikregel** für
die Grenzfälle, zu denen die Quellen schweigen (Regelfrage 3). **Schwere high** (Audit: medium):
im gewöhnlichen Loopbetrieb (Stufe 1) entsteht nie Evidenz über mehr als einen Durchlauf, jeder
Wrap macht die Befunde stale; kein Audio- und kein Userdatenschaden.

### g) Kleinster Fix

**Ort:** `invalidierung_aus_transportbruch`, `invalidierung_verdrahtung.rs:617-657`; dazu das
interne `Evidenzstand` (`evidenz.rs:104-110`, gelesen in `evidenzstand_aus_wert` `:272-352`).

1. `Evidenzstand` bekommt `cycle_active` (aus `/transport/cycle/active` des schon gelesenen
   Wire-Werts). Rein intern, kein Vertragsfeld.
2. Der Bruch wird einer `Bruchart` zugeordnet und über `grund_fuer_bruch` entschieden: neues
   Segment bei gleicher Epoche → `Drop`; neue Epoche, neuer Beleg mit `cycle_active` und
   Rücksprung (`neu.start < alt.start`) → Loop-Wrap; sonst → `Seek` bzw.
   `AndererKontinuitaetsbruch`. Ein Loop-Wrap ist `LoopWrapAnGrenze`, wenn eine Passage der
   Sitzung (`stand.experimente`, Projektbindung der Sitzung) den neuen Start an `projekt_von`
   und das alte Ende an `projekt_bis` hat (je innerhalb einer Toleranz von einem Evidenzframe,
   weil Fenster an der Grenze verworfen werden und `derived_sample_bounds` heute immer
   `unproven` ist, `Frame.h:314-318`); `LoopWrapInnerhalb`, wenn der Sprung innerhalb einer
   Passage liegt. Ein Loop-Wrap ohne gebundene Passage oder außerhalb jeder Passage folgt der
   Technikregel aus Regelfrage 3 (keine Rücknahme).
3. Nimmt ein Bruch zurück, dann **nur die Belege der alten Epoche dieser Quelle**
   (`Umfang::Ids`, Wireform `evidence_ids`, schon im Vertrag und vom Verbraucher gelesen,
   `SourcesModel.cpp:1565-1582`), nicht `[0, altes Ende)` aller Quellen. Das behebt zugleich
   Weg B von T3-08-02.
4. Kommentare `invalidierung_verdrahtung.rs:606-616` und `Zeit.h:237-241` nachziehen.

**Keine Schemaänderung** nötig: Passage, Epoche, Projektstart, Samplezahl und `cycle.active`
reisen schon. Eine Grundangabe im Stempel wäre sauberer, bräuchte aber einen Fassungsschritt des
strikten `transportstempel` (`eq-ipc-v3.schema.json:244-259`, `additionalProperties: false`),
Fixtures (A5, A8, B3c, B16) und beide Leser — für den Fix nicht nötig. Keine Migration, kein neues
Fixture Pflicht; ein v3-Fixture mit `cycle` und Rücksprung wäre für A5 nützlich.
**Mitzuprüfende Beziehungen:** invalidieren↔wiederherstellen (kein Wrap-Beleg im Store als
ausgeschlossen, der im Cache gültig ist), Passagenbindung (M-25: Passage bindet eine Epoche; die
Ausnahme darf die Epochenbindung nicht verlangen, weil jeder Durchlauf eine neue Epoche hat).
Audio-Thread: nicht berührt (Broker). Pipe-Latenz: nicht berührt.

### h) Ort des Rotbeweises

**Bein A4**, `sonde013_verdrahtung.rs` neben `:1117`, mit `HarnischMitStore`: `experiment_begin`
mit Passage `[0, L)` (Helfer `experiment_begin_wert` `:189`), Belege über `[0, L)` in Epoche E,
dann ein Beleg ab 0 mit Epoche E+1 und `cycle.active = true`. Assertion: Rücknahmezähler
unverändert, kein Grund in Cache und Store. **Heute rot.** Gegenfälle derselben Tabelle: Epoche
E+2 (Straddle) — kein Bruch; Wrap auf `L/2` (Loop kürzer) — `epochwechsel` nur für die Belege
der alten Epoche; Rücksprung ohne `cycle.active` (Seek) — `epochwechsel`; Vorwärtssprung —
`epochwechsel`, der neue Beleg bleibt gültig; altes Ende genau `L`, `L−1` und `L+1`. Die Zeile,
die die Zusage trägt, ist die neue Bruchart-Entscheidung in `:626-635`: wer sie auf
`neu.transport_epoch != alt.transport_epoch → Epochwechsel` zurückstellt, muss den ersten Fall
reißen. `transportbruch_invalidiert_mit_seinem_grund` (`:1117-1153`) wird an die Regel
angepasst (Store, Ids-Umfang, Assertion je Beleg statt `any`). Integrationsbeleg:
Laufzeit-Arm auf dem Diagnoseprojekt (Loop 45,696 s) zählt die Rücknahmen je Wrap.

---

## Gemeinsame Ursachen

**Wurzel A — Ordnung und Ganzheit enden vor dem Ziel.** T3-08-01 (Ganzheit endet an der
Batchgrenze des Writers), T3-08-02 (Store-Reihenfolge ist nicht Cache-Reihenfolge), T3-08-03
(Commitreihenfolge ist nicht Einreihreihenfolge). Ein Grundsatz trägt alle drei: **was im
Coordinator in einer Reihenfolge und als Einheit geschieht, erreicht Store und Queue in
derselben Reihenfolge und als Einheit.** Die Änderungssätze bleiben getrennt:

- **Satz „Auftrag = Transaktion"** (T3-08-01): nur `writer.rs` und `handle.rs`, Tests in
  `store_crash_matrix.rs` und im Crash-Worker. Keine Coordinatoränderung.
- **Satz „Sitzungsordnung von der Markierung bis zum Einreihen"** (T3-08-02 und T3-08-03): eine
  Sperre je Sitzung, unter der Cache-Einfügen/Markieren, `append_einreihen` und
  Queue-Einreihen liegen, das Warten außerhalb. Beide IDs gehören in **einen** Satz: sie
  berühren dieselben Funktionen (`invalidierung_anwenden_mit_rueckweg`, `flush_session`,
  Preview-Zweig) und dieselbe Sperre; getrennt gebaut, würde der zweite Satz den ersten
  umbauen. Voraussetzung ist Satz 1: erst wenn ein Auftrag ganz committet, ist die
  Reihenfolge der Aufträge auch die Reihenfolge der Wirkungen.

**Wurzel B — die Bruchentscheidung kennt nur Zahlen, nicht Gründe.** T3-08-05 allein, dazu
Weg B von T3-08-02 (Bereich `[0, altes Ende)`). Eigener Satz in
`invalidierung_aus_transportbruch` und `Evidenzstand`; er setzt Satz 2 nicht voraus, sollte aber
danach landen, weil Satz 2 die Reihenfolge Ablage vor Transportbruch in `evidenz.rs` festlegt.

**Nur die Datei geteilt:** T3-08-01 und T3-08-02 berühren beide `befehl.rs`, aber verschiedene
Stellen (Append-Aufbau `:403-439` gegen Preview-Markierung `:927-940`).

**Berührung mit Teil B (T3-15-12).** T3-15-12 (degradierter Store trennt Clients im
Sekundentakt) nutzt denselben Writer und Flush: `writer.rs:345-352` (Degradation beantwortet alle
offenen Aufträge mit `Err`), `handle.rs:219-230` (Verweigerung bei Degradation),
`flush.rs:160-168` (Trennen des Verursachers). Der Fehlerfall von T3-08-01 (SQLite-Fehler im
zweiten Abschnitt) endet genau dort. Grenze: T3-08-01 ändert die Batchbildung
(`writer.rs:268-287`), T3-15-12 das Verhalten nach der Degradation; getrennte Sätze, gemeinsamer
Testharnisch (Crash-Worker, `append_naht`). Der Satz zu T3-08-01 darf die Degradationsantwort
nicht ändern.

**Berührung mit Teil C (T3-16-03).** T3-16-03 betrifft P1-Evidenz, die gelesen, aber beim
Linkende vor der Annahme verworfen wird (Eingang `queues.rs:10-92`, P1-Verbraucher
`verbindung.rs:622-623`). T3-08-02 beginnt **an** der Annahme: `evidence_snapshot_json_mit_minor_wert`
(`evidenz.rs:135`), Cache-Einfügen `:192-199`. Die Grenze ist der Eintritt in diese Funktion.
Kopplung: die Sitzungssperre aus Satz 2 darf das Commit-Warten nicht umfassen; sonst verlängert
sie die P1-Verarbeitung, der Eingang staut, und beim Linkende gehen mehr Frames verloren
(T3-16-03).

**Registerfolgen (für den Dirigenten).** NAK-157 (`offene-punkte.md:184`) ist kein
Testformbefund, sondern ein Defekt (Prämisse von `SONDE-013.md:4340` falsch). NAK-167
(`:193`) wird von Satz 2 berührt (Ablage vor Transportbruch). NAK-269 (`:277`) bleibt
Härtung; Satz 2 ersetzt den Consumer-Riegel nicht und braucht ihn nicht. Der Widerspruch
SONDE-011 Defekt 18 (`SONDE-011.md:1075`) gegen SONDE-013 B14 (`SONDE-013.md:4203`) gehört als
Zeile nach `docs/offene-punkte.md` (CLAUDE.md `:62-63`).

**Irreführende Texte in denselben Änderungssätzen:** Satz 1: `befehl.rs:414-421`,
`writer.rs:248-251`, `invalidierung_verdrahtung.rs:290-292`, `experiment_verdrahtung.rs:67-68`,
`:207-208`, `sonde013_verdrahtung.rs:3227-3229`, `docs/plugin-wissen.md:2654-2656`. Satz 2:
`flush.rs:173-178`, `:187-191`, `invalidierung_verdrahtung.rs:23-25`. Satz 3:
`invalidierung_verdrahtung.rs:606-616`, `Zeit.h:237-241`, die Belegspalte von M-53
(`SONDE-013.md:548`, „ein Loop-Wrap AN der Passagengrenze ist gar kein Bruch" als gemessen).

---

## Regelfragen

### 1. Atomare Domänentransaktion (T3-08-01)

**Welche Gruppen sind heute fachlich unteilbar.**
- *Befehl plus Wirkung:* `preview_*` (Befehl + `evidence_invalidate`), die Experimentfamilien
  (Befehl + Passage + Terminale Verdrängter + Versuch), `user_verdict` (Befehl + Urteil). Zugesagt
  in B14 (`SONDE-013.md:4203`), K-02 (`SONDE-011.md:597`), §53.9 (`Entwurf :4195-4198`),
  im Code `befehl.rs:8-14`.
- *Materialwechsel nach `experiment_begin`:* Teil desselben Befehlsauftrags
  (`experiment_verdrahtung.rs:300-314`), B14 nennt ihn ausdrücklich.
- *Evidenz plus Rücknahme:* **keine** zugesagte Einheit. Der Beleg und die von ihm ausgelöste
  Transportbruch-Rücknahme sind getrennte Aufträge (`evidenz.rs:210`, `:224`); NAK-167
  (`offene-punkte.md:193`): „Eine gemeinsame Transaktion ist nirgends zugesagt". Zugesagt ist
  nur Widerspruchsfreiheit (§33.5) — dafür reicht die Ordnung aus Regelfrage 2.
- *Sitzungsbefehle* (`confirm_join`, `unbind_probe`): Register und Wirkung flüchtig,
  `angewandt` auch ohne gelungenen Flush (`befehl.rs:612-617`); bekannt als NAK-125
  (`offene-punkte.md:161`), nicht Gegenstand.

**Wie der Group-Commit heute zählt.** Ereignisse, nicht Aufträge; über alle Aufträge im Kanal
in Kanalreihenfolge; Commit bei 64 Ereignissen oder nach 50 ms ab Fensterbeginn
(`writer.rs:262-293`, `mod.rs:50-51`); eine Barriere (Checkpoint, Riegel) beendet das Fenster
nicht (`:304-312`); ein Auftrag wird an der Grenze geteilt (`:278`).

**Wo der ACK-Zeitpunkt liegt.** Nach dem Commit des **letzten** Abschnitts des Auftrags
(`writer.rs:252-256` → `handle.rs:148-151` → `befehl.rs:442-445`). Vor dem ACK ist aber schon
ein Teil dauerhaft.

**Was das Gate wörtlich verlangt.** „ACK erst nach vollständigem Commit, Replay idempotent"
(`plan.json:385`). Beides hält wörtlich; gebrochen ist die Einheit, die das „vollständig"
voraussetzt, und die „exactly-once-Wirkung" aus §53.9.

**Technik oder Produkt: Technik.** Es gibt keine Variante, die ein User wählen würde: „ein
bestätigter Befehl darf seine Wirkung verlieren" widerspricht §53.9 und B14, und keine
Alternative ist für den User sichtbar verschieden — nur die Commitgrenze verschiebt sich um
höchstens ein Sammelfenster. **Kein Kartenentwurf.** Regel für den Dirigenten: *Ein
Store-Auftrag mit höchstens 64 Ereignissen wird in genau einer Transaktion geschrieben; ein
größerer wird abgewiesen. Das ACK folgt dem Commit dieser Transaktion.*

### 2. Ordnung von Commit und Publikation (T3-08-02, T3-08-03)

**Welche Ordnungsmarke existiert heute, und wie weit reicht sie.**

| Station | Marke | Beleg |
|---|---|---|
| Coordinator-Cache | `empfangsfolge` (nur Evidenz, unter dem Standlock) | `evidenz.rs:197` |
| Store | `event_ord` je Ereignis, Commitreihenfolge des einen Writers | `writer.rs:476` |
| Flush | Marke = `event_ord` des Commits (ohne Store `event_sequence`) | `flush.rs:179-185` |
| Rücknahme | Marke = `event_ord` ihres Appends | `invalidierung_verdrahtung.rs:392-393`, `befehl.rs:943` |
| Ausgangsqueue | Marke je Eintrag, Hochwasser **je Schlüssel**, Reihenfolge über Schlüssel = Ankunft | `queues.rs:148-155`, `:227-258` |
| Draht | keine Marke | `coordinator/mod.rs:145-146` |
| C++-Verbraucher | kein Ordnungsfeld in `session_snapshot` (`schema :1070-1073`) und `evidence_invalidate` (`:1208-1212`, `SourcesModel.cpp:1516`) | NAK-269 |

**Was R-283-2 abdeckt:** die Ordnung **in** der Queue — eine Ersetzung verliert ihren alten
Platz, die Reihenfolge ist die der jüngsten Marke (`queues.rs:237-253`). **Was nicht:** (a) die
Übergabe zwischen Commit und Einreihen (T3-08-03), (b) die Erfassung eines Flushs vor einer
Markierung, deren Commit danach liegt (Präzisierung 2 zu T3-08-03), (c) die Reihenfolge zwischen
Cache und Store (T3-08-02).

**Kommt eine Regel „Commitordnung bis Publikation" ohne v3-Vertragsänderung aus? Ja.** Die
Ordnung entsteht vor der Queue: Markieren/Erfassen, Commit und Einreihen einer Sitzung unter
einer Sperre, das Warten außerhalb; die Queue erhält die Reihenfolge schon heute (R-283-2); der
Verbraucher wendet in Ankunftsfolge an. Kein Feld in `session_snapshot` oder
`evidence_invalidate` nötig („Schemas sind Verträge", `CLAUDE.md:178-181`, bleibt unberührt).
Der Consumer-Altersriegel (NAK-269) bleibt, wie entschieden, Härtung; er bräuchte ein
versioniertes Feld und ist der Schutz für einen zweiten Sender, nicht für diesen.

**Technik oder Produkt: Technik.** Die Zusagen (M-52 „haltbar", §33.5 „keine zwei Wahrheiten",
R-283-2 „in Entstehungsreihenfolge zugestellt") legen das Ergebnis fest; die Wahl der Sperre ist
Umsetzung. **Kein Kartenentwurf.** Regel für den Dirigenten: *Je Sitzung liegen Markieren bzw.
Erfassen, Store-Einreihen, Commit und Queue-Einreihen jeder Evidenzablage, Rücknahme und jedes
Flushs in einer Ordnung; Store und Queue sehen sie in derselben Reihenfolge wie der Cache.
Gewartet wird außerhalb.*

### 3. Loop-Wrap (T3-08-05)

**Woher der Broker heute den Bruchgrund kennt:** gar nicht. Er vergleicht Epoche und Segment der
zwei jüngsten Belege einer Quelle (`invalidierung_verdrahtung.rs:626-635`). Im Frame stehen
`transport_epoch`, `continuity_segment`, `project_sample_start`, `sample_count` und, wenn
tragbar, `cycle {active, bounds_valid, start_ppq, end_ppq}` (`NakamaEvidenz.cpp:195-253`); keine
PPQ-Position, kein Tempo, kein Grund. Die Passage mit Projektsamplegrenzen kommt nur über
`experiment_begin` (Stufe 2, Abschnitt 4 b Station 5).

**Sendet der C++-Sender den Loopgrund?** Nein (`Vertrag.h:122-164`, Grund nur gezählt
`FeatureEngine.h:1798`). **Könnte er?** Ja, mit einem neuen Feld im strikten `transportstempel`
— das ist ein versionierter Fassungsschritt (kein Discriminator, keine Zieladresse, keine
Revision, keine Capability, also zulässig über die Fassungsleiter), aber kein Fix ohne
Vertragsarbeit.

**Ist die Bindung der Ausnahme an die gebundene Passage ohne Vertragsänderung möglich?** Ja,
aus vorhandenen Feldern: Passage (`projekt_von`, `projekt_bis`) im Brokerbestand, Rücksprung und
`cycle.active` aus den Frames (Fix 4 g). Die Genauigkeit ist ein Evidenzframe, weil
Samplegrenzen der Schleife nie bewiesen sind (`Frame.h:314-318`).

**Grenzfälle** (Passage `[von, bis)`, halboffen):

| Fall | Quellenlage | Vorschlag |
|---|---|---|
| Loop deckt die Passage exakt | M-53: keine Invalidierung | keine Rücknahme |
| Loop kürzer als die Passage (Wrap innerhalb) | M-53: invalidiert „diese Messung", Grund `epochwechsel` | Rücknahme der Belege der alten Epoche dieser Quelle |
| Loop länger als die Passage (Wrap hinter `bis`) | schweigt; die Passagenmessung ist beim Wrap schon abgeschlossen | keine Rücknahme (§32.3 `:1998-1999`) |
| Loop ohne gebundene Passage (heute der Normalfall) | M-53 schweigt; §32.3 `:1998-1999`: Rücknahme nach Loop-Wrap ist kein Normalpfad | keine Rücknahme; die Zeitreihe endet, Fenster sind pluginseitig getrennt |
| Off-by-one am Ende: altes Ende `bis`, `bis−1`, `bis+1` | schweigt | `bis` und bis zu einem Frame davor gelten als Grenze; `bis+1` heißt, der Beleg lag über der Passage — Wrap außerhalb |
| Epoche +1 gegen +2 (Straddle) | Erzeuger `Zeit.h:512-522`, `FeatureEngine.h:888-895` | beide gleich behandeln; nie „genau +1" verlangen |
| Seek statt Wrap | M-53: Seek → `epochwechsel`; ohne Grund im Frame ist ein Rücksprung bei aktiver Schleife vom Wrap nicht zu unterscheiden (so auch der Erzeuger, `Zeit.h:249-250`) | Rücksprung mit `cycle.active` zählt als Wrap; Rücksprung ohne Schleife und jeder Vorwärtssprung als Seek |
| Stop/Start innerhalb | M-53 „jeder andere Kontinuitätsbruch trägt `epochwechsel`" | Rücknahme wie Seek |

**Technik oder Produkt: Technik.** Der Passagenfall ist durch M-53 entschieden, der passagenlose
Loop durch Entwurf §32.3; der Code selbst nennt die Absicht (`invalidierung.rs:142-145`). Eine
Karte hätte eine einzige Seite, die ein Musiker wählen würde: dass Nakama beim Loopen eines Teils
die Messung jedes Durchlaufs behält. Die andere Seite — jeder Durchlauf löscht den vorigen — hat
keinen Nutzen und widerspricht der Quelle. **Kein Kartenentwurf.**
**Offen, aber nicht Gegenstand:** wie viel ein **Seek** zurücknimmt. Heute alles vor dem alten
Frameende aller Quellen (`:648-651`, `:156-161`); M-53 sagt nur „diese Messung". Der Vorschlag
4 g Punkt 3 (alte Epoche dieser Quelle) ist die enge Lesart; ob der Dirigent sie als Regel
festschreibt oder als eigene Registerzeile führt, ist seine Entscheidung.

**Trennung Technik und Produkt:**

| ID | Was der Dirigent als Technikregel entscheidet | Was nur der User entscheidet | Was unabhängig baubar ist |
|---|---|---|---|
| T3-08-01 | Auftrag = Transaktion bis 64, darüber Abweisung; Matrixumschrift SONDE-011 Defekt 18 | nichts | alles aus 1 g, 1 h |
| T3-08-02 | Sitzungsordnung Cache = Store; Ablage vor Transportbruch | nichts | alles aus 2 g, 2 h |
| T3-08-03 | Sitzungsordnung bis zum Einreihen; `snapshot_schreiben` in Einreihen und Warten teilen | nichts | alles aus 3 g, 3 h |
| T3-08-05 | Bruchart-Entscheidung, Toleranz, Umfang der Rücknahme, passagenloser Loop | nichts | alles aus 4 g, 4 h |

---

## Reihenfolge

Maßstab: die Arbeitsübersicht (`BEFUNDE.md:816-817`, `:831`: W13 „Vor Vertrauen in
Persistenz-/Replayheilung W12/W14", W14 „W13", W28 „W14 Rücknahmeordnung") und die geordnete
Abarbeitung (`:127` B14 ohne Abhängigkeit, `:128` B15 nach B14, `:136` B22 nach B15).

**Etappe A — T3-08-01 (Satz „Auftrag = Transaktion").** Kleinster Satz, keine Regelfrage
offen, Voraussetzung für die übrigen. `writer.rs`, `handle.rs`, Crash-Worker, Tests. Beine A4,
A4-SI; A22 und A24 für die ACK-Latenz.

**Etappe B — T3-08-02 und T3-08-03 (Satz „Sitzungsordnung").** Nach A. Innere Folge: (i)
`snapshot_schreiben` in Einreihen und Warten teilen (Trait intern); (ii) Sitzungssperre um
Markieren/Erfassen, Store-Einreihen und Queue-Einreihen in Flush, Rücknahme, Preview und
Experiment; (iii) P1: Ablage vor Transportbruch. Beine A4 (neue Nahtfälle), A4-SI, B13; Laufzeit
gegen T3-16-03 (Teil C) gegenlesen.

**Etappe C — T3-08-05 (Satz „Bruchart").** Nach B, weil B die Reihenfolge in `evidenz.rs`
festlegt. `Evidenzstand`, `invalidierung_aus_transportbruch`, Tests. Beine A4, B5 (unverändert
grün), Laufzeit-Arm auf dem Diagnoseprojekt.

**Sofort machbar, ohne Produktdiff:** die Registerzeile zum Widerspruch SONDE-011 Defekt 18 ↔
SONDE-013 B14 und die Umordnung von NAK-157 (Härtung → Defekt).

**Nachzuziehende Matrixzeilen:**
- **SONDE-011:** S-02 (`:534`), S-05 (`:537`), V-03 (`:612`), V-04 (`:613`), K-02/K-03
  (`:597-598`, dazu eine neue K-Zeile „Kill zwischen zwei Commits eines Befehlsauftrags"),
  Nacharbeit Defekt 18 (`:1075`: Aufträge bis 64 werden nicht geteilt).
- **SONDE-013:** §13.4 B14 (`:4203`, Beleg wird der Crashfall statt `event_ord`-Abstand 1),
  §14.1 B14-Test (`:4340`, Prämisse „Produktpfad ist atomar" als Erratum im NAK-314-Manifest),
  M-52 (`:547`, „haltbar" mit Neustartvergleich), M-53 (`:548`, Belegspalte: Produktpfad statt
  Helfer; Grenzfälle aus Regelfrage 3).
- **NAK-283:** R-283-2 (`:112`) und M-17 bis M-20 (`:623-626`) bleiben; eine neue Zeile für die
  Ordnung vor der Queue (Commit → Einreihen).
- **NAK-246:** M-23 (`:635`) und M-27 (`:644`, Test `tests_abonnement.rs:1039`) um die
  Schlüsselgrenze erweitern (Rücknahme hält, Flush überholt).

---

## Nicht geprüft

- **Kein Lauf.** Der Auftrag verbot Bau, Test, Kanon und Fehlerinjektion. Alles oben ist
  Quellenlektüre. Hergeleitet, nicht gemessen: der falsche `idempotent_wiederholt` nach
  SQLite-Fehler im selben Lauf (1 b), die Wahrscheinlichkeit des Ablagefensters (2 b), die
  umgekehrte Lücke (3 b Präzisierung 2), die Wirkung je Wrap auf dem Diagnoseprojekt (4 b).
- **Die eine Skriptausnahme** (Fixture-Stempel mit `py -3.13 -c`) steht unter „Prüfform".
- **Altbestände in echten Stores:** ob eine vorhandene Datenbank schon geteilte Befehle (Riegel
  ohne Wirkung) oder Evidenzzeilen ohne ihren Ausschluss enthält, ist aus dem Repo nicht
  bestimmbar. Ob ein Reparaturlauf nötig ist, entscheidet der Dirigent.
- **Größte Auftragslänge eines `experiment_begin`:** die Zahl verdrängter Versuche je Beginn
  habe ich nicht verfolgt (`experiment.rs:106` `N_PROJEKT = 8`, `:111` `N_GLOBAL = 32` gelesen,
  `beginne` nicht); für die Aussage „kein Produktaufrufer über 64" genügt, dass jeder Auftrag
  aus einem Befehl, höchstens einer Passage, den Verdrängten, einem Versuch und höchstens einer
  Rücknahme besteht.
- **`experimentbefehl_und_wirkung_sind_ein_append`** (`sonde013_verdrahtung.rs:2221`) und
  `evidenzreihenfolge_ueberdauert_den_neustart` (`:4168`) nur über den Rohbericht bzw. den Namen
  eingeordnet.
- **Wer den Sessionsnapshot eines P0-Befehls pusht** (Outbox-Schuld des Befehlsereignisses) und
  wann im Preview-Weg der nächste heilende Snapshot kommt, habe ich nicht verfolgt.
- **C++-Zustellfolge:** dass der ControlClient P1-Frames in Ankunftsfolge an `SourcesModel`
  gibt, habe ich nicht gelesen.
- **Outbox-Koaleszierung von Rücknahmen:** die Outbox hält je Ziel und Schlüssel nur die
  jüngste Schuld (`writer.rs:478-493`); zwei nicht zugestellte Rücknahmen mit verschiedenen
  Bereichen werden beim Nachspiel zu einer. NAK-246 M-28 (`NAK-246.md:645`) begründet das mit
  dem globalen Verbraucher; für die vier IDs trägt es nicht.
- **A24, B16, A22 und die Laufzeit-Szenarien** nur über ihre Behauptungszeilen
  (`tools/beweise.ps1:637-638`, `:767`) gelesen; ob ein Szenario im Diagnoseprojekt die
  Rücknahmen je Wrap sichtbar macht, nicht geprüft.
- **Broker-Fixtures** mit `cycle.active` und Epochensprung: nicht vollständig durchsucht.
- **Teil B und Teil C** sind nicht validiert; ihre IDs stehen oben nur als Berührung.
- **Sicherheit** (Pipe-Vertrauensgrenze, T3-12-\*) ist nicht Gegenstand.
- **Kein Dokuriegel gelaufen.** `tools/plan/dokuriegel.py` ist ein Skript; diese Datei ist ein
  Rohbeleg unter `docs/beweise/roh/`, kein Plandokument.

---

FERTIG Teil A, 2 bestätigt, 2 präzisiert, 0 widerlegt.
