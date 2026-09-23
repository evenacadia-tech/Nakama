# NAK-310 — Validierung der Matrixprüfung 1 (lesender Opus-5.5-Thread, Effort max)

**HEAD vorher:** `ac69a9bdcd219f71d6e73564d64c0988bb46abe3` (23.09.2026, 12:18:01, `date`) · **HEAD nachher:** `e8c6f079f4331755f4a901c2715ce6df2324420a` (12:32:04, `date`).
Dazwischen zwei Dirigenten-Commits: `8de878a1` (12:22:40, §10, Abnahme U59) und `e8c6f079` (12:25:07, Planstand). `git diff --stat ac69a9bd HEAD -- broker eq-copilot tools` ist leer; im Manifest ersetzen sie nur die Zeilen 8, 9 und 169 und hängen §10 an. Die Zeilennummern des Prüfers (822-825, 1190-1192, 1328-1332) gelten damit unverändert.
Prüfgegenstand gelesen über `git show ac69a9bd:docs/beweise/NAK-310.md`; Code am Arbeitsbaum (gleich `ac69a9bd`). Nur Lesebefehle; kein Bau, kein Test, kein Python, kein FL, keine Pipe; geschrieben nur diese Datei.

| Befund | Urteil | Klassenvorschlag |
|---|---|---|
| V-1 Broker-Marke an die Diagnose-Sitzung binden (`:1328-1332`, M-85) | **PRÄZISIERT** | DEFEKT |
| V-2 Etappe-4-Satz zur Hörmarkierung (`:822-825`, M-71, M-72) | **BESTÄTIGT** | DEFEKT (Text) |
| V-3 A4-Behauptung für Etappe 3 und 4 (`:1190-1192`) | **BESTÄTIGT** | DEFEKT (Text) |

Zeilenangaben ohne Datei meinen `docs/beweise/NAK-310.md` an `ac69a9bd`; „Teil A" und „Teil B" meinen `docs/beweise/roh/NAK-310-quellvalidierung-teil-A.md` und `…-teil-B.md`.

---

## V-1 — Broker-Marke an die Diagnose-Sitzung binden

### 1. Quelle

- `:1328-1332` (§7.4 „Risiken"), Zeilen stimmen: „Ein Broker aus einem früheren FL-Lauf ohne Marke antwortet nicht; M-85 erwartet ihn dann nicht. Ein Broker aus einem früheren markierten Lauf, der noch im Leerlauf lebt, hat einen anderen Elternprozess; seine Antwort ist nicht erwartet — der Bauer prüft, dass der Runner eine unerwartete Rolle als Notiz führt und nicht als Fehler."
- M-85 (`:754`): (a) Broker als Kind des Diagnose-FL → Rolle `broker` erwartet; (b) anderer Elternprozess → keine Erwartung, Notiz „Broker läuft ohne Marke (Elternprozess ist nicht das Diagnose-FL)"; (c) kein Broker wie heute. Geregelt wird nur die Erwartung des Runners (`tools/fl/szenario.py:668-688`). Stimmt.
- R-310-2 Satz 3 (`:144-145`, wiederholt §5.6 `:519-520`): „Der Briefkasten antwortet nur in Sitzungen, die die Automatik selbst gestartet hat, solange U59 nicht anders entscheidet." Das Zitat des Prüfers ist wörtlich.
- `AGENTS.md:50-52` existiert: „Gekoppelte Lebenszyklen gemeinsam behandeln: speichern/laden, starten/stoppen, öffnen/schließen, verbinden/trennen, aktivieren/abklingen und installieren/Rückweg." Gleichlautend `CLAUDE.md:77-79`, im Manifest als Zusage für alle Gegenstände zitiert (§5.0 `:231-233`).

### 2. Ablauf

**Woran die Matrix die Marke bindet.**

- Marke = Umgebungsvariable `NAKAMA_DIAGNOSE_BRIEFKASTEN`, Wert genau `1` (§6 `:588-589`, M-82 `:751`). Gesetzt nur von `Starte-FL` in die Umgebung des gestarteten FL, auch über `Frischer-Start` (M-83 `:752`, §7.4 `:1297-1301`).
- Gen und Probeeq prüfen sie im Konstruktor (M-79 `:748`). Ihr Briefkasten lebt im FL-Prozess und endet mit ihm, ist also an die Sitzung gebunden.
- Der Broker prüft sie in `starten_mit` vor `localappdata()` (M-80 `:749`, §7.4 `:1305-1307`). `starten_mit` läuft genau einmal je Brokerprozess (`broker/src/lebenslauf.rs:131` `BROKER.get_or_init`, `:173` `briefkasten_starten`; `broker/src/briefkasten.rs:787-795`). Die Marke kommt über die geerbte Umgebung (`eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:618-621`, Umgebungsblock `nullptr`; M-84 `:753`). **Beim Broker hängt die Marke also am Start des Brokerprozesses, nicht an der FL-Sitzung.**

**Wer den Broker startet und beendet (Code).**

- Start nur durch Gen als `main` mit offenem Editor (`eq-copilot/plugin/state/NakamaLebenslauf.h:117-127`, Aufruf `eq-copilot/plugin/src/PluginProcessor.cpp:247`, `:256`); der Laufzeit-Arm startet ihn nie (`tools/fl/laufzeit.ps1:45`; NAK-286 T-10 `docs/beweise/NAK-286.md:952`).
- Der Broker endet nur von selbst, nach 60 s ohne aktiven v2- oder v3-Client (`lebenslauf.rs:45`, `:280-295`, `:299-327`; `broker/src/main.rs:60-71`). `lebenslauf.rs:297-298`: „Das Plugin beendet nie einen Prozess und besitzt deshalb keinen Stop-Pfad". Der Laufzeit-Arm beendet nur FL-Prozesse (`laufzeit.ps1:265-269` mit den Namen `FL64`, `FL`, `FL Studio`; `:545-553`; `-Beenden` `:1076`).
- Folge: Ein Broker, den ein Diagnose-FL gestartet hat, lebt nach dessen Ende mindestens 60 s weiter, und unbegrenzt, solange irgendein Client verbunden ist. Jede Instanz eines danach geöffneten FL verbindet sich mit ihm (ein Broker je Benutzer, `lebenslauf.rs:43`).

**Die beiden Fälle des Prüfers, nachgerechnet.**

- (i) Ein markierter Broker überlebt das Ende seiner Sitzung, und ein normal geöffnetes FL verbindet sich binnen 60 s. Dann bleibt der Briefkasten des Brokers offen. Er beantwortet Anfragen während der eigenen Sitzung des Users, mit dem Aggregat über alle angemeldeten Sensoren (`briefkasten.rs:682-700`). **Am Bauplan nachvollziehbar:** Keine Zeile schließt den Briefkasten des Brokers, wenn die markierende Sitzung endet.
- (ii) Ein unmarkierter Broker aus einer eigenen Sitzung ist noch im Leerlauf, und das Diagnose-FL startet. Dann startet Gen keinen zweiten Broker, und der Briefkasten des Brokers bleibt zu. Ebenfalls nachvollziehbar. Das schlägt sicher fehl (fail-closed), und §7.4 `:1328-1329` nimmt es bewusst hin.

**Am HEAD erreichbar?**

- Im Diagnoseprojekt ist Gen `legacy` (`eq-copilot/fixtures/fl/LIES-MICH.md:16`) und startet keinen Broker. Belege: NAK-286 T-10 (`:952`) und M-57 (`:602`); `tools/fl/LIES-MICH.md:208-209`: „Im Diagnoseprojekt antwortet kein Broker, solange Gen dort `legacy` ist." Aus dem Laufzeit-Arm entsteht am HEAD also kein markierter Broker.
- Die Matrix plant aber genau für diesen Broker: M-84 („ein von Gen gestarteter Broker erbt die Umgebung … und antwortet"), M-85 (a) und §7.4 `:1329-1330`.
- Einen zweiten Weg öffnet der Produktentscheid selbst: die „eigene Verknüpfung" für eine gewünschte Messung einer User-Sitzung (Karte U59; Abnahme `design/abnahmen/2026-09-23-messdaten-weg-nur-automatik-sitzungen-u59.md:30-32`). Sie startet ein markiertes FL mit einem Projekt des Users. Ist Gen dort `main` mit offenem Editor, entsteht ein markierter Broker, der die nächste, normal geöffnete Sitzung bedient.

**Weitere Befunde.**

- Eine Matrixzeile für „markierende Sitzung endet → Marke verfällt" oder für „Broker lebt über die Sitzung hinaus" gibt es nicht. Die Beziehungsliste (§6 `:629-634`) nennt für den Briefkasten nur M-88 (Plugin, `stoppe()` ohne Marke). M-38 und M-40 (Etappe 3) berücksichtigen die Marke nicht.
- Nebenfund am selben Ort: Der Notiztext von M-85 (b), „Broker läuft ohne Marke", ist genau für den Fall aus `:1329-1330` falsch: Ein markierter Broker eines früheren Laufs hat ebenfalls einen anderen Elternprozess.

### 3. Zusagenlage

- **R-310-2 Satz 3** (`:144-145`) deckt Fall (i) ab: „Der Briefkasten antwortet nur in Sitzungen, die die Automatik selbst gestartet hat". Seit 23.09.2026, 12:12 Uhr ist das ein Produktentscheid: „Nur Automatik-Sitzungen (Empfohlen)" (Abnahme `:21`). Dort steht in `:28-29`: „In Sitzungen, die der User selbst öffnet, ist er zu."
- **`CLAUDE.md:77-79`** (§5.0 `:231-233`): „starten↔stoppen … aktivieren↔abklingen … gehören in denselben Änderungssatz". Die Marke aktiviert den Briefkasten des Brokers beim Prozessstart; in Etappe 5 fehlt das Gegenstück zum Ende der markierenden Sitzung.
- **§7.4 Restlücke** (`:1324-1326`, übernommen aus Teil A `:687-688`): „Während eines Laufzeit-Arm-Laufs kann jeder Prozess des Kontos anfragen; das Diagnose-FL trägt das Testprojekt, keine Musik des Users". Fall (i) macht diese Eingrenzung falsch: Das Fenster reicht über den Lauf hinaus und in ein Projekt des Users hinein.
- **Teil A schweigt zur Lebensdauer des Brokers.** Der Vorschlag in `:677-683` übernimmt die geerbte Umgebung ohne Ende. „Mitzuprüfen: starten↔stoppen" (`:700-701`) nennt nur „ohne Marke kein Start". §1 übernimmt das in `:110-111` („eine Marke in dessen Prozessumgebung erreicht Plugins und Broker"). Die Lücke ist also aus Teil A geerbt.
- Fall (ii) verletzt keine Zusage: „nur" ist eine Obergrenze, und M-84 verspricht eine Antwort nur für „ein von Gen gestarteter Broker".

### 4. Urteil: PRÄZISIERT

Der Kern stimmt: Die Marke hängt am Start des Brokerprozesses, und der Broker überlebt die markierende Sitzung. M-85 und §7.4 regeln nur die Erwartung des Runners. In Fall (i) ist R-310-2 Satz 3 verletzt.

Abweichungen:

1. Aus dem Laufzeit-Arm ist der Fall am HEAD nicht erreichbar, weil Gen im Diagnoseprojekt `legacy` ist. Real wird er, sobald das Diagnose-FL einen Broker startet (das sehen M-84 und M-85 (a) vor) oder sobald es die Verknüpfung aus U59 gibt.
2. Die Umkehrung (ii) verletzt keine Zusage; sie kostet nur die Messung des Brokers und steht bewusst in §7.4.
3. Hinzu kommt der falsche Notiztext in M-85 (b).

**Klassenvorschlag: DEFEKT.**

- §7.4 `:1328-1332` und M-85 widersprechen R-310-2 Satz 3 und dem Produktentscheid U59.
- Für den zeitlichen Teil von R-310-2 Satz 3 misst keine Zeile.
- Die Beziehung aktivieren↔abklingen der Marke fehlt (`CLAUDE.md:77-79`).

Prüfbare Fixrichtung: eine Zeile „markierende Sitzung endet → Briefkasten des Brokers zu", mit Gegenfall und Rotbeweis, dazu der berichtigte Notiztext in M-85 (b). Randbedingungen für den Weg: Der Runner startet nie einen Broker (NAK-286 T-10), und das Plugin hat keinen Stopp-Pfad (`lebenslauf.rs:297-298`).

---

## V-2 — Etappe-4-Satz zur Hörmarkierung

### 1. Quelle

- `:822-825` steht in §7.0 „Für alle Etappen" (Absatz „Kreuzung T3-12-02 × T3-12-05"), nicht in §7.3. Die Zeilen stimmen. Wortlaut: „Eine schweigende v2-Verbindung mit Hörmarkierung hält die brokerweite Evidenzsperre. Etappe 3 begrenzt die Dauer (M-53), Etappe 4 begrenzt, wer sie setzen kann (M-71, M-72): dieselbe Matrix, getrennte Commits."
- `broker/src/transport/server_v3/register.rs` existiert nicht. `git grep -n hoermarkierung broker/src` führt auf **`broker/src/register.rs`**. Dort in dieser Reihenfolge:
  - `:265` bestimmt `ist_besitzer`;
  - der Kommentar `:267-270` sagt: „auch eine duplizierte, nicht besitzende Instanz kann hörbar färben";
  - `:271-288` setzt die Markierung je Nonce, `:290-292` ruft `fremde_messungen_sperren`;
  - erst `:298-300` prüft den Besitz (`if !ist_besitzer { return; }`).

  Die vom Prüfer zitierte Stelle (`register.rs:267-299`) sagt also, was er behauptet.
- **`broker/src/server.rs`:** `:840` liest `hoermarkierung`, `:843` schreibt ins Register, `:848-852` ruft `senke.hoermarkierung(&registrierung.interventions_link_id, aktiv)` ohne Blick auf den Besitz; die Link-ID gilt je Verbindung (`:766`). **Ja:** Die Markierung wird vor der Besitzprüfung gesetzt und an die Interventionssenke weitergeleitet.
- **Teil B `:382-383`** (unter `:380` „Restlücke, bei jedem Weg"): „Die Hörmarkierung über v2 bleibt eine brokerweite Sperrmöglichkeit jedes verbundenen v2-Clients; T3-12-05 begrenzt sie zeitlich, nicht dem Grunde nach." Das ist wörtlich die Restlücke, die der Prüfer nennt.

### 2. Ablauf

- M-71 (`:735`), Fall (a): Ein Hello mit der eigenen PID bekommt `welcome`. M-72 (`:736`): Die zweite Verbindung von S aus P2 wird registriert, „beide sehen `konflikt: true`; Schreibrechte und Metadaten bleiben bei P1". Beide Zeilen lassen also das Hello einer nicht besitzenden Verbindung mit wahrer PID zu. Eine Verbindung mit frischer Sensor-ID besitzt ihren Eintrag ohnehin.
- §7.3 ändert für v2 nur `server.rs:715-745` und `register.rs:209-214` (`:1258-1265`). `register.rs:262-300` und `server.rs:840-852` bleiben, wie sie sind.
- **Nachgerechneter Weg eines Heartbeats mit `hoermarkierung: true`:**
  1. `register.rs:271-292` setzt die Markierung und sperrt fremde Messungen, noch vor der Besitzprüfung.
  2. `server.rs:848-852` reicht den Wert weiter an `coordinator/intervention.rs:273-297`.
  3. Zu einer v2-Link-ID gibt es keine Sitzung; `:275` legt den Eintrag deshalb unter `SessionKey::unbekannt()` ab.
  4. `:632-647` sperrt daraufhin die Evidenz aller Sitzungen.
- Ergebnis: Auch nach Etappe 4 setzt jede verbundene v2-Verbindung desselben Kontos die brokerweite Sperre wie heute. Etappe 4 begrenzt nicht, wer sie setzen kann.
- Der Satz widerspricht auch dem Manifest selbst: §7.3 `:1270-1271` sagt „Nach Etappe 4 kann sich ein Prozess desselben Kontos weiter als eigene, neue Instanz mit seiner wahren PID anmelden."

### 3. Zusagenlage

- Die Begrenzung sagt nur der Satz `:822-825` zu, sonst nichts:
  - §1 T3-12-02 (`:95-98`) nennt für v2 nur „Übernahme nur im selben Prozess, Wache gegen Tokenannahme außerhalb der Verbindung";
  - R-310-5 (`:154-159`), §5.5 und M-71 bis M-74 sagen nichts zur Hörmarkierung;
  - M-53 (`:712`, Etappe 3) begrenzt nur die Dauer.
- Der Satz stammt aus Teil B §4 `:833-836`: „Die Frist aus T3-12-05 begrenzt das zeitlich, die Prozessbindung aus T3-12-02 begrenzt, wer es darf." Teil B widerspricht sich damit selbst (§4 gegen die Restlücke in `:380-388`). Der Code und der kleinste Fix (`:370-378`) stützen die Restlücke. `:386-388` ordnet sie der Klasse „derselbe Benutzer" zu, die laut Threat-Model nur zu verkleinern ist.
- Teil B `:315-318` zählt die Sperre zwar zu der Wirkung, mit der das Hello eine fremde Instanzrolle freischaltet. Die Fixrichtung in §1 nimmt sie aber nicht auf, und nach dem Prüfauftrag ist dieser Entscheid des Dirigenten nicht Prüfgegenstand.

### 4. Urteil: BESTÄTIGT

Zeilen, Code und Restlücke stimmen so, wie der Prüfer sie beschreibt. Ergänzend: Der Satz stammt aus Teil B §4, und die Datei heißt `broker/src/register.rs`.

**Klassenvorschlag: DEFEKT (Text).**

- Der Satz im Bauplan widerspricht der Quellvalidierung (`:380-388`), dem Code und §7.3 `:1270-1271`.
- Keine Zusage verlangt die Begrenzung; eine neue Matrixzeile ist deshalb nicht nötig.
- Nacharbeit: Satz `:822-825` berichtigen (Etappe 4 begrenzt nicht, wer die Sperre setzt) und die Restlücke aus Teil B `:382-383` in §7.3 unter „Restlücke im Threat-Model" nennen.
- Das Kriterium „T3-12-02 ohne Zeile für seine Fixrichtung" greift nicht: M-72 und M-74 decken die v2-Fixrichtung aus §1.
- Die Markierung an den Besitz zu binden, wäre HÄRTUNG und kollidiert mit `register.rs:267-270`: Nicht besitzende FL-Duplikate müssen färben und sperren dürfen.

---

## V-3 — A4-Behauptung für Etappe 3 und 4

### 1. Quelle

- `:1190-1192` (§7.2), Zeilen stimmen: „Tests: A4 (Rust) für M-34 bis M-57 außer M-42, A4b für M-42, `git diff` für M-58; Einzelbein A4 mit Testfilter, Rotbeweise je Zeile; voller Kanon am Ende."
- Teil B `:595` (T3-12-03, Ort des Rotbeweises): „Behauptungszeile `tools/beweise.ps1:518` um die Entkopplung ergänzen."
- Teil B `:414-415` (T3-12-02): „A4-Behauptung (`tools/beweise.ps1:518`) um die Bindung ergänzen; A22 (`:637`), B10 (`:738`) und das Soak-Bein laufen nach der Fixture-Umstellung unverändert grün."
- Beide Zitate sind wörtlich so, wie der Prüfer sie wiedergibt. Teil A verlangt für A4 keine Änderung der Behauptung, nur für B30 (`:624-629`, umgesetzt in M-32).
- Heutige A4-Behauptung, `tools/beweise.ps1:512-518` (2395 Zeichen), Anfang: „Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, … produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. …" Aus diesem Themenfeld nennt sie nur „produktiver v2+v3-Verdrahtung". `security_vectors`, SID- oder Peer-Bindung, v3 vor v2, v2-Fristen und Briefkasten kommen nicht vor.

### 2. Ablauf

- `git show ac69a9bd:docs/beweise/NAK-310.md | grep -n beweise.ps1` trifft nur in §5 (`:323`, `:370`, `:396`, `:502`), in §6 (`:595` Spaltendefinition, `:663` M-19, `:681` M-32) und in §7.0 (`:860` „hat keinen Beinfilter").
- §7.2 (`:1121-1205`), §7.3 (`:1207-1289`) und §7.5 (`:1337-1353`) planen keinen Edit an `tools/beweise.ps1`.
- Änderungen an einer Behauptung plant der Bauplan nur für B30: M-32 (`:681`) und §7.4 `:1316` („Die Behauptung von B30 nennt danach „nur mit Marke"").
- Die Lücke hat das Manifest in §5 selbst festgestellt:
  - §5.3 `:396-398`: „… produktiver v2+v3-Verdrahtung …" — gemessen wird heute nur die Quelltextwache;
  - §5.5 `:502-503`: „A4 … nennt `security_vectors` und eine Identitätsbindung nicht".
- **Tests und Rotbeweise der Regeln in A4.** A4 fährt den ganzen `cargo test` (`beweise.ps1:515`); jede neue Zeile hat einen Rotbeweis.
  - **R-310-3**, M-34 bis M-41 (§6.8 `:776`), A4 **NEU**:
    - `komposition_beide_frei_bedient_beide`, `komposition_v2_belegt_v3_laeuft`, `komposition_status_nennt_v2_belegung` samt Quelltextwache auf `main.rs`;
    - `zweite_komposition_scheitert_an_v3`, `komposition_stopp_gibt_beide_namen_frei`, `komposition_leerlauf_ohne_v2`, `komposition_namensrand`;
    - dazu die umgestellte Wache `produktiver_v2_server_speist_den_gemeinsamen_coordinator_interventionsriegel` (M-39) und M-93 `v2_name_und_token_je_lauf_unveraendert`.
  - **R-310-5**, M-59 bis M-77:
    - A4 **NEU** in `broker/tests/security_vectors.rs`: `fremde_draht_sid_wird_unauthorized_abgelehnt`, `sid_nicht_kanonisch_wird_abgelehnt`, `host_pid_ungleich_peer_wird_abgelehnt`, `peer_prozess_nicht_ermittelbar_schliesst`, `peer_prozessgriff_lebt_mit_der_verbindung`, `telemetrie_aus_anderem_prozess_abgelehnt`, `beitritt_nur_mit_verifizierter_host_pid`, `verdraengung_nur_aus_demselben_peer`, `ablehnungscode_nach_grund`;
    - v2-Seite: `v2_host_pid_ungleich_peer_wird_abgelehnt`, `v2_uebernahme_nur_im_selben_prozess`, `v2_token_wirkt_nur_auf_seiner_verbindung`, Wache `session_token_nur_in_verbindung_bedienen_verglichen`, `doppelte_sid_schluessel_ein_wert`;
    - umgestellt: `gleiche_sid_wird_erst_nach_revert_angenommen`, `control_und_telemetry_durchlaufen_die_identische_sid_kette`.

### 3. Zusagenlage

- Den Befund stützt der Gate-Bezug (d): Teil B `:414-415` und `:595` verlangen die Änderung ausdrücklich.
- Das Gate verlangt unter BEWEIS („Rotbeweisen an A4 security_vectors … vollem Kanon GRÜN") Rotbeweise, keinen Behauptungstext. Die Matrix liefert diese Rotbeweise.
- Mildernd: §7.0 `:867-870` bindet jede Etappe an `tools/dirigent/pruefliste.md` A bis F. Dort verlangt E `:82-85`, dass jemand, der eine Zusage ändert, den Runnerkopf (`tools/beweise.ps1`) nachzieht. §8 `:1382` ordnet diese Regel aber nur M-25 und M-32 zu, und die Prüfliste ist laut Prüfauftrag keine Anforderungsquelle.
- **Beide Regeln haben messende Zeilen mit Test und Rotbeweis.** Das Kriterium „von keiner Zeile mit messendem Test und Rotbeweis gedeckt" trifft auf R-310-3 und R-310-5 also nicht zu. Unvollständig ist nur der Behauptungstext des Kanons.

### 4. Urteil: BESTÄTIGT

Zitate, Zeilen und die fehlenden Edits in §7.2 und §7.3 stimmen. Der Prüfer behauptet keine fehlende Testdeckung, sondern eine fehlende Behauptung, und das trifft zu.

**Klassenvorschlag: DEFEKT (Text, geringes Gewicht).**

- Für Etappe 3 und 4 lässt der Bauplan eine ausdrückliche Forderung der Quellvalidierung weg (Gate-Bezug d), obwohl er dieselbe Art Änderung für B30 vorsieht.
- Das Kriterium „Regel ohne messende Zeile" greift nicht.
- Nacharbeit: je ein Satz in §7.2 (Änderungssatz A: Entkopplung) und §7.3 (Commit 1: Bindung am Tor; Commit 2: v2-Übernahme), jeweils mit Textfund vorher und nachher.

---

## Übersicht

| Befund | Urteil | Klassenvorschlag | tragender Satz |
|---|---|---|---|
| V-1 (§7.4 `:1328-1332`, M-85) | PRÄZISIERT | DEFEKT | R-310-2 Satz 3 (`:144-145`): „Der Briefkasten antwortet nur in Sitzungen, die die Automatik selbst gestartet hat"; Produktentscheid U59, 12:12 Uhr: „In Sitzungen, die der User selbst öffnet, ist er zu."; `CLAUDE.md:77-79`: starten↔stoppen, aktivieren↔abklingen |
| V-2 (§7.0 `:822-825`, M-71, M-72) | BESTÄTIGT | DEFEKT (Text) | Teil B `:382-383`: „Die Hörmarkierung über v2 bleibt eine brokerweite Sperrmöglichkeit jedes verbundenen v2-Clients; T3-12-05 begrenzt sie zeitlich, nicht dem Grunde nach."; Code `broker/src/register.rs:267-300`, `broker/src/server.rs:848-852` |
| V-3 (§7.2 `:1190-1192`, §7.3) | BESTÄTIGT | DEFEKT (Text) | Teil B `:595`: „Behauptungszeile `tools/beweise.ps1:518` um die Entkopplung ergänzen."; Teil B `:414`: „A4-Behauptung (`tools/beweise.ps1:518`) um die Bindung ergänzen" |

## Gemeinsame Ursache

Alle drei Befunde haben dieselbe Ursache im Bauplan. §7 übernimmt Sätze aus den Zusammenfassungen der Quellvalidierung, etwa das Markendesign aus Teil A `:677-683` und die Kreuzung aus Teil B §4 `:833-836`. Ebenso übernimmt er Forderungen aus deren Abschnitten „Ort des Rotbeweises" und „Mitzuprüfen". Dabei ordnet er nicht jeden dieser Sätze einer Matrixzeile oder einem Satz einer Etappe zu. Pro Etappe fehlen deshalb genau die Gegenstücke:

- **V-1:** Zur Aktivierung fehlt die Abklingzeile. Die Marke wirkt ab dem Brokerstart, endet aber nicht mit der Sitzung.
- **V-3:** Zur neuen Zusage fehlt die Behauptungszeile, hier A4 für Etappe 3 und 4. Die Prüflistenregel „drei Stellen" ist in §8 `:1382` nur M-25 und M-32 zugeordnet.
- **V-2:** Einer Behauptung in der Übersicht fehlt die Zeile, die sie trägt („Etappe 4 begrenzt, wer sie setzen kann").

V-1 und V-2 haben zusätzlich denselben Sachverhalt. Ein einzelner Teilnehmer löst einen brokerweiten Zustand aus, nämlich den offenen Briefkasten oder die Evidenzsperre. Dieser Zustand ist weder an seine Sitzung noch an seine Berechtigung gebunden. Die Durchsicht der Etappe 1 (§8, Zeile F, `:1384`: „Beziehungen im selben Änderungssatz — erfüllt (geplant)") hat den Brokerprozess nicht als eigenen Lebenszyklus neben FL geprüft.

## Nicht geprüft

- Kein Lauf; alle Aussagen sind aus den Quellen hergeleitet. Nicht gemessen ist, ob ein markierter Broker tatsächlich eine folgende Sitzung bedient.
- Den Inhalt von `aggregat_bauen` (`broker/src/aggregat.rs:16`) habe ich nicht gelesen. Die Aussage „Aggregat über alle angemeldeten Sensoren" stützt sich auf `briefkasten.rs:682-700`.
- Wie `szenario.py` heute mit einer unerwarteten Rolle umgeht, habe ich nicht nachgelesen.
- §10 des Manifests liegt nach `ac69a9bd` und ist nicht Prüfgegenstand. `docs/PLAN-STAND.md` und fremde Ordner habe ich nicht angefasst.

FERTIG Validierung Matrixprüfung 1 NAK-310, 2 bestätigt, 1 präzisiert, 0 widerlegt
