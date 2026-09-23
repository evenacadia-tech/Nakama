# NAK-310 — Quellvalidierung Teil B (lesender Opus-Agent, HEAD f02ae1b4)

**Ticket:** NAK-310 · Planschritt S25i „Tiefenaudit 3 abarbeiten, Teil 2: Pipe-Sicherheit" ·
Etappe 0 „Quellvalidierung", **Teil B (Broker-Pipe)**
**HEAD:** `f02ae1b481b5584b91ba6ff64521922f82ceeb29` (Zweig `master`)
**Datum:** 23.09.2026 · **Modell:** Opus 5.5 (lesend)
**IDs (3):** T3-12-02 (Bündel B04 „Authentisierte Verbindungsidentität"); T3-12-03 und
T3-12-05 (Bündel B05 „v2 darf v3 nicht stilllegen").
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (Phase 12 ab `:475`,
Befundzeilen `:482`, `:483`, `:485`, Zähler `:487`; Bündel `:74`, `:75`; Abarbeitungsplan
`:125`, `:126`; Skeptiker Phase 10 ab `:489`, Zeilen `:550`, `:551`, `:553`, Einordnung `:557`;
Arbeitsübersicht Phase 16 `:796`) und die Rohberichte unter
`docs/audits/2026-09-15-tiefenaudit/roh/`: `phase-12-pipesicherheit.md` (vollständig),
`phase-10-skeptiker.md` (`:30`, `:91`, `:92`, `:94`, `:120`), `phase-11-synthese.md` (`:64`,
`:65`, `:115`, `:116`, `:163`, `:164`, `:171`, `:185`, `:410`), `phase-16-b-verbindungen.md`,
`phase-16-e-last-lebenslauf.md`.
**Gate:** Schritt S25i, `docs/plan/plan.json:345` (Feld `text`); Registerzeile
`docs/offene-punkte.md:315`; Manifest `docs/beweise/NAK-310.md` (Kopf, §0).
**Muster:** `docs/beweise/roh/NAK-312-quellvalidierung-teil-A.md` (Form, Tiefe, Kopf) und
`docs/beweise/roh/NAK-312-quellvalidierung-teil-B.md` (Trennung Technik und Produkt).

**Gate-Text wörtlich** (`docs/plan/plan.json:345`, der hier einschlägige Teil):

> „… T3-12-02 (verifizierten Peer und zulässige Rolle gemeinsam binden; v2-Token schaltet keine
> fremde Instanzrolle frei), T3-12-03 und T3-12-05 (v2-Namenskollision vom v3-Start entkoppeln,
> v2-Handshake, Leerlauf und Rate begrenzen) … GRENZE: kein Angriffscode, kein Bruch der
> eingefrorenen Identität (NAK-30), Installer-Rückweg bleibt bytegleich, Probe-Pipe nie
> Produktions-Pipe. BEWEIS: docs/beweise/NAK-310.md mit Matrix, Rotbeweisen an A4
> security_vectors, A17/A18, B10, B30, vollem Kanon GRÜN und Laufzeit-Arm (Installation über
> die erhöhte Aufgabe nach der Reparatur)."

**Prüfform:** Quellenlektüre am HEAD. Kein Bau, kein `cargo`, kein Testlauf, kein Kanon, kein
FL Studio, kein Codex, keine Fehlerinjektion, keine Named Pipe, kein Angriffscode. Git nur
lesend. Jede Zeilennummer unten ist am HEAD `f02ae1b4` selbst nachgelesen. Drei
API-Aussagen stützen sich auf Microsoft Learn (abgerufen 23.09.2026):
`GetNamedPipeClientProcessId` („Retrieves the client process identifier for the specified named
pipe", Handle aus `CreateNamedPipe`), *Mandatory Integrity Control* („a principal with a low
integrity level cannot write to an object with a medium integrity level, even if that object's
DACL allows write access"; Objekte ohne Label gelten als medium) und `CryptProtectData`
(„Typically, only a user with the same logon credential as the user who encrypted the data can
decrypt the data"). Ein Lesebefehl enthielt versehentlich einen leeren `python3`-Aufruf ohne
Skriptinhalt; er hing und wurde abgebrochen. Er hat keinen Code ausgeführt und nichts
geschrieben (`git status --short` danach unverändert).

**Arbeitsbaum, geprüft mit `git status --short`.** Zu Beginn (08:16:54 Uhr) und unmittelbar
vor dem Schreiben (08:39:19 Uhr) identisch: nur die fremden untracked Ordner `briefing-hub/`
und `nimbalyst-local/`. HEAD beide Male `f02ae1b4`; er ist nicht gewandert. Diese Datei ist
die einzige Schreibaktion. `docs/beweise/roh/NAK-310-quellvalidierung-teil-A.md` gehört dem
parallelen Teil A und wurde nicht angefasst.

**Stand des Audits gegen den HEAD.**

- Phase 12 lief am HEAD `d20b8c15` (`BEFUNDE.md:475`, 16.09.2026 02:42). Seither hat
  `git diff --stat d20b8c15 HEAD -- broker` genau zwei Dateien: `broker/src/briefkasten.rs`
  (`3b2d182c`, NAK-309 Etappe 4, nur Testmodul) und `broker/tests/store_crash_matrix.rs`
  (`685158e9`, NAK-309 Etappe 4, NOT-RUN-Meldung). Keine Fundstelle der drei IDs ist
  verändert. Alle Zeilennummern des Audits treffen am HEAD noch; sie sind unten trotzdem
  einzeln neu gelesen.
- **Berichtigung des Auftragstexts:** Er nennt NAK-311 als Veränderer von
  `broker/src/briefkasten.rs`. `git log --grep=NAK-311 -- broker` ist leer; der Commit ist
  `3b2d182c` (NAK-309).
- **Berichtigung des Manifests:** `docs/beweise/NAK-310.md:5` spricht von „Bestätigung in
  Phase 16, Zeilen 549 bis 553", `:33` von „alle in Phase 16 als BESTÄTIGT geführt". Diese
  Zeilen stehen im Abschnitt **Phase 10** (Überschrift `BEFUNDE.md:489`). Phase 16 hat T3-12
  ausdrücklich ausgeklammert (`BEFUNDE.md:796`: „Sicherheitsbefunde T3-12-01 bis T3-12-05
  werden hier weder bearbeitet noch neu eingeordnet"; ebenso
  `roh/phase-16-nachweise/abschluss_pruefen.py:123`), ebenso Phase 15
  (`roh/phase-15-paket3-teilfehler.md:102`). Phase 10 prüfte Phase 12 „ausschließlich auf
  Quellenherleitung und Duplikate; Schwere und Kategorie werden dort unverändert übernommen"
  (`roh/phase-10-skeptiker.md:30`, `BEFUNDE.md:557`). **Diese Validierung ist damit die erste
  unabhängige Prüfung von Schwere und Kategorie der drei IDs.**

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorie | Schwere | Ort des Rotbeweises | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-12-02 (B04) | **PRÄZISIERT**: Kern bestätigt; im v2-Pfad schaltet nicht der Token frei, sondern das Hello selbst; am HEAD kein Audio- oder DSP-Weg | DEFEKT gegen das Threat-Model („Verwechslung und zufällige lokale Clients") | high (Audit: critical) | Bein A4, `broker/tests/security_vectors.rs` neben `:417-444` (B-01) und `:489-496` (B-04); tragende neue Zeile zwischen `verbindung.rs:259` und `:300` | **nein** |
| T3-12-03 (B05) | **BESTÄTIGT**, präzisiert: Kopplung seit `512790a2`, Ausfall im Produkt unsichtbar, auch nicht bösartige Auslöser, eine Quelltextwache schreibt das `?` fest | DEFEKT gegen Entwurf §48.3 und NAK-123 A-01 | high | Bein A4, neuer Kompositionstest auf Probe-/Testnamen; Mutation am `?` in `lebenslauf.rs:158` | **nein** (Variante A) |
| T3-12-05 (B05) | **BESTÄTIGT**, präzisiert: halbe Hellos begrenzt nur ein Broker ohne jeden anderen Client; Querwirkung auf v3 über die Hörmarkierungsbrücke | LÜCKE, Technikregel | medium | Bein A4, `broker/src/server.rs` neben `:1724-1755`, jeder Fall mit eigener Frist (NAK-241) | **nein** |

---

## 1. T3-12-02 — die Windows-Authentisierung bindet die behauptete Identität nicht (B04)

*Befundzeile `BEFUNDE.md:482` · Rohbeleg `roh/phase-12-pipesicherheit.md:19`, Schwere `:27` ·
Skeptiker `BEFUNDE.md:550` = `roh/phase-10-skeptiker.md:91` · Bündel `BEFUNDE.md:74` ·
Vorlauf NAK-152 (`docs/offene-punkte.md:179`), NAK-123 (`docs/beweise/NAK-123.md`)*

### a) Behauptung des Audits

Die erfolgreiche Windows-Authentisierung liefert nur ein `bool` und wird nicht an die im Hello
behauptete Logon-SID, Sensor-ID, Pluginart oder Host-PID gebunden; im v2-Pfad erhält jedes
syntaktisch gültige Hello denselben Sitzungstoken (`BEFUNDE.md:482`).

### b) Quellkette am HEAD

**Station 1 — das Authentisierungstor gibt nur ein bool zurück.**
`broker/src/transport/server_v3/auth.rs:45-49`:

```
pub(super) fn client_sid_authentisieren(
    h: HANDLE,
    sicherheit: &crate::server::Sicherheit,
    cleanup: &mut VerbindungsSicherheitsCleanup,
) -> bool {
```

Die SID des Clienttokens entsteht in `:103`
(`let sid = unsafe { (*puffer.as_ptr().cast::<TOKEN_USER>()).User.Sid };`), wird in `:165`
(`if unsafe { EqualSid(sid, erwartete_sid) } == 0 {`) gegen die DACL-SID verglichen, danach
fällt das Token (`:168` `drop(token);`, `:169` `cleanup.revertieren();`), und die Funktion endet
mit `:188` `true`. Die authentisierte SID verlässt die Funktion nicht. Der Funktionsname im Audit
(`authentifiziere_client`, `roh/phase-12-pipesicherheit.md:19`) existiert nicht; gemeint ist
`client_sid_authentisieren`.

**Station 2 — erst nach dem Tor wird das Hello gelesen, und nur syntaktisch.**
`broker/src/transport/server_v3/verbindung.rs:253-259`:

```
    if !client_sid_authentisieren(griff.h, &erwartete_sicherheit, &mut sicherheits_cleanup) {
        statistik
            .geschlossen_bootstrap
            .fetch_add(1, Ordering::SeqCst);
        return;
    }
    let (bs, verbraucht) = match bootstrap_lesen(&roh[..rahmen_laenge]) {
```

Die Adressprüfung ist `broker/src/transport/bootstrap.rs:149-164`:

```
fn adresse_pruefen(a: &Adresse) -> Result<(), String> {
    if a.logon_sid.is_empty() || a.logon_sid.chars().count() > 184 {
        return Err("logon_sid ausserhalb 1..184".into());
    }
```

gefolgt von reinen hex32-Prüfungen für `project_binding_id`, `session_epoch`, `instance_id`,
`runtime_nonce`. Die Pluginart prüft `:257` nur gegen die Liste `:142`
(`pub const PLUGIN_KIND_ERLAUBT: [&str; 4] = ["main", "passive_probe", "active_probe", "legacy"];`),
die Hostangabe `:175-188` nur auf Textlängen; `pid` ist ein beliebiges `u32` (`:92-98`).

**Station 3 — der Coordinator übernimmt die Behauptungen.** `verbindung.rs:300-306` meldet die
Kopplung mit der behaupteten `runtime_nonce` an, `:335-339` reicht das ganze behauptete Hello an
die Senke (`s.control_verbunden(&link_fuer_senke, &hello_fuer_senke)`), und
`broker/src/coordinator/senke.rs:10-15` ruft damit `control_hello_registrieren`.
`broker/src/coordinator/link.rs:86-89`:

```
        let wire_adresse = hello.adresse.clone();
        let host_pid = hello.host.as_ref().map(|h| h.pid);
        let mut adresse = wire_adresse.clone();
        let mut session_ungebunden = Self::ist_ungebundene_probe(hello);
```

Daraus folgen drei Entscheidungen allein aus Behauptungen:

1. **Sitzungsbeitritt über die Host-PID.** Eine ungebundene Probe tritt der Main-Sitzung bei,
   deren Main dieselbe behauptete PID, Logon-SID und Projektbindung meldet,
   `link.rs:42-46`:
   ```
               .filter(|(key, client)| {
                   client.plugin_kind == "main"
                       && client.host_pid == Some(host_pid)
                       && key.logon_sid == adresse.logon_sid
                       && key.project_binding_id == adresse.project_binding_id
   ```
   angewandt in `link.rs:91-97`.
2. **Schlüssel und Verdrängung.** `link.rs:99` bildet den `ClientKey` aus den behaupteten
   Feldern (`broker/src/coordinator/zustand.rs:9-25`); ein Hello mit einem schon lebenden
   Schlüssel verdrängt dessen Link, auch bei gleicher Nonce (`link.rs:145-155`,
   `link.verdraengt = true; link.trennen = true;`, Schließauftrag `:180`).
3. **Rolle.** `link.rs:238-241` speichert `plugin_kind: hello.plugin_kind.clone()` und
   `host_pid` ungeprüft; `broker/src/coordinator/mod.rs:457-462` baut die persistierte
   `effektive_adresse` aus der behaupteten `logon_sid`.

**Station 4 — was die behauptete Rolle am HEAD darf.** `session_command`
(`broker/src/coordinator/befehl.rs:474-525`) verlangt `client.plugin_kind == "main"` (`:497`),
Bestätigung und Führung, und vergleicht in `:518`
(`|| ziel.logon_sid != sender_link.adresse.logon_sid`) Behauptung gegen Behauptung. Die
P0-Weiche `p0_json_mit_minor` (`befehl.rs:665-954`) kennt `heartbeat` (`:673`),
`audible_intervention_begin/_end` (`:769`, `:806`), `session_command` mit `confirm_join` und
`unbind_probe` (`:863`, `:567-578`), `preview_*` nur als Persistenz mit Evidenzrücknahme
(`:864-921`), `experiment_*` (`:933-936`) und `user_verdict` (`:953`). **Es gibt am HEAD keinen
Weg, der einen DSP-Befehl an eine Sonde weiterreicht**; `SondeProcessor.cpp` hat keinen
P0-Eingang (Suche nach `p0`, `command_ack`, `apply` ohne Treffer). Die im Entwurf vorgesehene
HMAC-`control_capability` (`docs/FL-Nakama-Sonden-Design-Entwurf.md:3516-3524`, Ticket
SONDE-016 `:4498`) ist nicht gebaut (keine Fundstelle in `broker/src`, `eq-copilot/plugin`).

**Station 5 — die Telemetrie ist gebunden, nur das Control-Hello nicht.**
`bootstrap.rs:370-383` koppelt eine Telemetrieverbindung nur mit der im Welcome ausgegebenen
Challenge und derselben `runtime_nonce`
(`if k.challenge != h.challenge || k.runtime_nonce != h.adresse.runtime_nonce {`).

**Station 6 — der v2-Pfad: der Token ist kein Berechtigungsfaktor, das Hello ist es.**
v2 hat keine Impersonation (`ImpersonateNamedPipeClient` kommt in `broker/src` nur in
`server_v3` vor), nur die DACL (`broker/src/server.rs:222-246`, benutzt `:495-513`).
`hello_pruefen` prüft Syntax (`broker/src/protokoll.rs:242-305`) und antwortet
`protokoll.rs:307-311`:

```
    BrokerNachricht::Welcome {
        protocol_version: hello.protocol_version,
        broker_version: broker_version.to_string(),
        session_token: session_token.to_string(),
    }
```

Der Token ist einer je Brokerlauf (`broker/src/lebenslauf.rs:142`
`let session_token = uuid::Uuid::new_v4().to_string();`, weitergegeben `:156` und `:173`) und
wird nur auf derselben Verbindung verglichen (`server.rs:786`
`if hb.session_token != session_token {`, `:865` für `bye`). Keine andere Stelle nimmt ihn als
Berechtigung an; `briefkasten.rs:787-822` leitet aus ihm nur Diagnosekennungen ab (T3-12-04,
Teil A). **Die Wirkung auf fremde Instanzen entsteht durch das Hello:**

- `broker/src/register.rs:211-213`:
  ```
          // Das jüngste hello gewinnt die Schreibrechte und die Metadaten.
          eintrag.besitzer_nonce = nonce.to_string();
          hello_metadaten_uebernehmen(eintrag, hello);
  ```
  Ein Hello mit der `sensor_id` einer lebenden Gen-Instanz übernimmt deren Schreibrechte
  (Registerregel `register.rs:9-12`) und setzt `konflikt`; die echte Gen-Instanz zeigt dann
  „Kennung doppelt!" (`eq-copilot/plugin/src/PluginEditor.cpp:292`
  `konfliktKnopf.setVisible (processor.konfliktGemeldet());`, gespeist aus dem v2-Client,
  `PluginProcessor.h:177` `return pipe.snapshot().konflikt;`). Die `sensor_id` ist Gens
  persistente `instanceId` (`PluginProcessor.cpp:162`).
- Die Hörmarkierung eines v2-Heartbeats geht über die Brücke `server.rs:848-852` in den
  Coordinator (`coordinator/intervention.rs:27-35`, `:273-297`). Für eine v2-Link-ID gibt es
  keinen v3-Link; `intervention.rs:275` legt sie deshalb in den Platzhalter
  `SessionKey::unbekannt()` (`zustand.rs:41-53`), und dieser sperrt die Evidenz **aller**
  Sitzungen, `intervention.rs:634-640`:
  ```
          // Eine Sitzung ohne Eintrag ist sauber — aber der Platzhalter fuer
          // Ereignisse ohne zuordenbare Sitzung sperrt trotzdem.
          let erlaubt = erlaubt
              && stand
                  .taint
                  .get(&SessionKey::unbekannt())
                  .map_or(true, Taintstand::erlaubt);
  ```
  Beim Trennen setzt `:299-307` `unknown = true`, bis ein v3-Resync ihn löst (`:432`,
  `link.rs:470`). Diese Sperrwirkung ist Absicht (fail-closed, `server.rs:379-381`,
  `zustand.rs:42-47`); ungebunden ist, **wer** sie auslösen darf.

**Station 7 — wer die Kette erreicht.** Durch DACL und SID-Tor kommen nur Prozesse desselben
Benutzers (V-01, `roh/phase-12-pipesicherheit.md:35`). Prozesse niedriger Integrität schließt
die Mandatory Integrity Control aus (Microsoft Learn, s. Kopf). `GetNamedPipeClientProcessId`
kommt im Broker nicht vor; das Plugin prüft die Gegenrichtung längst so
(`eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:480` `GetNamedPipeServerProcessId`, Token
`:498-505`).

### c) Zusage wörtlich

- **Threat-Model**, `docs/FL-Nakama-Sonden-Design-Entwurf.md:3534-3536` = Gate NAK-123
  (`docs/beweise/NAK-123.md:15`): „Das schützt vor fremden Windows-Usern, Verwechslung und
  zufälligen lokalen Clients. Code, der bereits unter demselben User in FL injiziert wurde,
  liegt außerhalb des IPC-Threat-Models; dafür bleiben Signaturprüfung, Installationspfad und
  Betriebssystemschutz maßgeblich."
- **Entwurf §48.4**, `:3511-3513`: „PID und Prozesspfad sind nur Diagnose, keine Identität.
  Danach prüft der Server Clienttoken, Sitzung, Handshake, Nachrichtentyp, Tiefe, Länge und
  Rate;"
- **Entwurf §33.1**, `:2056-2057`: „Ein v2-Client nutzt weiter nur seine eine
  Heartbeat-Verbindung und erhält keine v3-Steuerfähigkeit."
- **v3-Vertrag**, `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:65`: „Windows-Logon-SID in
  S-R-I-S...-Form. Wird nie geparst, nur verglichen."
- **NAK-123 B-01**, `docs/beweise/NAK-123.md:53`: „… → `RevertToSelf` erfolgreich → erst dann
  Hello fachlich annehmen und gegebenenfalls `welcome` senden." Die Matrix B (Flächenzusage
  `:49`) regelt die Reihenfolge und die Gleichheit Token-SID ↔ DACL-SID, **keine** Bindung der
  Drahtfelder. Eine solche Zusage hat NAK-123 nie gegeben.
- **Gate S25i** (`plan.json:345`) ist die Planforderung des Tickets, kein Entscheid.

**Welcher Teil gebrochen ist.** „fremde Windows-User" hält (Station 7, `auth.rs:165`).
Gebrochen ist „Verwechslung und zufällige lokale Clients": ein lokaler Client desselben
Benutzers, der das Protokoll spricht, ist vom echten Plugin nicht unterscheidbar; eine falsche
Host-PID entscheidet über die Sitzung (Station 3.1), ein wiederholtes Hello verdrängt einen
lebenden Link (3.2). Der absichtlich handelnde, nicht in FL injizierte Prozess desselben
Benutzers ist im Wortlaut weder eingeschlossen („zufällig") noch ausgeschlossen („in FL
injiziert"). §33.1 ist nicht gebrochen: v2 erhält keine Befehlsgewalt, wohl aber die
sitzungsübergreifende Sperrwirkung aus Station 6; ob das „Steuerfähigkeit" ist, sagt der
Entwurf nicht.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Was sie nicht misst |
|---|---|---|
| A4 (`tools/beweise.ps1:511-519`, ganzer `cargo test`) mit `security_vectors.rs:417-444` (B-01) | Reihenfolge connect → read → impersonate → sid → revert → self → hello_accept; `adresse_json` (`:99-106`) sendet die erfundene SID `S-1-5-21-1-2-3-1001`, der Test verlangt `welcome` (`:423-424`) | Senke ist `ZaehlSenke` (`:154-159`), kein Coordinator; keine Bindung von Draht-SID, PID oder Rolle |
| `security_vectors.rs:602-662` (B-08) | dieselbe Kette für Control und Telemetry, wieder mit der erfundenen SID | dito |
| `security_vectors.rs:489-496` (B-04) | fremde **Token**-SID über die Testnaht `auth.rs:111-162` | keine fremde **Draht**-SID |
| Coordinatortests (`coordinator/mod.rs:496-500`, `broker/tests/coordinator_model.rs`) | Sitzungslogik mit direktem `control_hello_registrieren` | laufen am Tor vorbei |
| `server.rs:1573-1635` (v2) | Token auf derselben Verbindung, Bye | keine Aussage über Berechtigung |
| `server.rs:1270-1328`, `:1330-1403` (v2) | Übernahme und Konflikt sichtbar; Brücke setzt `aktive == 1` und sperrt brokerweit (`:1374-1375`) | schreiben das heutige Design fest, keine Bindung an den Prozess |

**Antwort auf die Auftragsfrage.** `gleiche_sid_wird_erst_nach_revert_angenommen` bestätigt den
Defekt nicht als Defekt und nennt ihn nicht als Zusage. Sein Ziel ist die Reihenfolge B-01.
Die erfundene SID ist ein Fixture-Kürzel; der Test verlangt dabei aber ein `welcome` für eine
Draht-SID, die nicht die Token-SID ist. **Er schreibt das heutige Verhalten damit
unbeabsichtigt fest** und wird unter einem ablehnenden Fix rot. Dasselbe Kürzel steht in
weiteren Clients, die den echten Listener erreichen: `server_v3/tests_hilfe.rs:174`, `:191`;
`broker/tests/broker_idle.rs:125`; `eq-copilot/plugin/tests/IpcLastMain.cpp:108` (A22);
`IpcTestMain.cpp:232` (B10); `SessionSoakMain.cpp:163`.

Die Behauptungszeile von A4 (`tools/beweise.ps1:518`) nennt weder `security_vectors` noch
Identitätsbindung. Kein Test prüft Host-PID gegen Peer-PID oder die behauptete Rolle.

### e) Urteil: PRÄZISIERT

1. **Kern bestätigt.** Stationen 1 bis 3 schließen am HEAD lückenlos; seit `d20b8c15` ist
   keine der Dateien verändert.
2. **Der v2-Teil ist anders gebaut, als die Befundzeile nahelegt.** Der gemeinsame Token ist
   vertragskonform (`eq-copilot/schemas/eq-ipc.schema.json:60`: „zufällig pro Broker-Lauf;
   Client zitiert es in jedem heartbeat") und berechtigt zu nichts außerhalb seiner Verbindung.
   Die Freischaltung einer fremden Instanzrolle geschieht durch das **Hello**: Übernahme der
   Schreibrechte einer lebenden `sensor_id` (`register.rs:211-213`) und die brokerweite
   Evidenzsperre über die Hörmarkierung (`intervention.rs:273-297`, `:634-640`). Der Gatesatz
   „v2-Token schaltet keine fremde Instanzrolle frei" hält im Wortlaut und bricht in der Wirkung.
3. **„Mess- oder Steuerdaten" ist am HEAD enger.** Steuerdaten sind
   Mitgliedschaftsbefehle, Vorschau-Persistenz, Experimente und Userurteile (Station 4). Einen
   Weg zu Audio oder DSP gibt es nicht; das Grundgesetz (`CLAUDE.md:28-32`) ist nicht berührt.
4. **Voraussetzung.** Für die Übernahme einer bestimmten Instanz braucht der Client deren
   hex32-Kennungen; für den Sitzungsbeitritt dazu die PID des FL-Prozesses. Beides sind
   Behauptungen, die niemand prüft.

### f) Kategorie und Schwere

**DEFEKT** gegen `docs/FL-Nakama-Sonden-Design-Entwurf.md:3534` (Teil „Verwechslung und
zufällige lokale Clients").

**Schwere high** (Audit: critical). Keine Benutzergrenze fällt, kein Audio- oder DSP-Weg ist am
HEAD erreichbar; getroffen sind Integrität und Verfügbarkeit von Analyse, Sitzung und Store.
**Mit der geplanten Fernsteuerung über den Broker (P7, SONDE-016) wird der Befund critical**,
weil dann die behauptete Rolle Audio bewegen könnte. Die Reparatur gehört deshalb vor P7.

### g) Kleinster Fix

**v3, zwei Bindungen am Tor, ohne Vertragsänderung.**

1. **Draht-SID gegen Token-SID.** `crate::server::Sicherheit` (`server.rs:61-66`) hält heute nur
   die binäre SID; `sicherheit_nur_user` (`:222-246`) hat die kanonische Zeichenkette in der
   Hand (`:223-224`) und wirft sie weg. Sie bleibt als Feld erhalten; in `verbindung_bedienen`
   wird nach `bootstrap_lesen` (`verbindung.rs:259`) und vor jeder Anmeldung (`:300`, `:407`)
   `adresse.logon_sid` auf Gleichheit mit ihr geprüft. Weil `EqualSid` in `auth.rs:165` schon
   bewiesen hat, dass die Token-SID genau diese SID ist, ist das der Vergleich mit der
   authentisierten SID. Ein Stringvergleich bleibt im Vertrag („nie geparst, nur verglichen").
   Das Plugin sendet dieselbe Form (`eq-copilot/plugin/core/ipc/PipeToken.cpp:147-171`,
   `ConvertSidToStringSidW` wie `server.rs:198`). Da nur exakt kanonische Werte passieren,
   bleiben NAK-152 (Kanonisierung mit Migration) und die persistierten Riegelschlüssel
   unberührt. Abweichung: kein Welcome, Schließen wie B-04, gezählt; will man antworten, dann mit
   dem vorhandenen Code `unauthorized` (`eq-ipc-v3.schema.json:820`).
2. **Host-PID gegen Peer-PID.** Auf demselben Handle `GetNamedPipeClientProcessId` rufen und
   mit `host.pid` vergleichen, wenn `host` gesendet ist (das Produkt sendet immer:
   `eq-copilot/plugin/src/prozessor/Ipc.cpp:476`, `SondeProcessor.cpp:1356`, beide `_getpid()`);
   Abweichung schließt. Ohne `host` gibt es schon heute keinen Beitritt (`link.rs:57`
   `host_pid?`). Gegen PID-Wiederverwendung das Prozesshandle mit
   `PROCESS_QUERY_LIMITED_INFORMATION` für die Dauer der Verbindung halten, spiegelbildlich zu
   NAK-123 C-02 (`NAK-123.md:70`). Die PID wird damit nicht Identität (§48.4 `:3511`), sondern
   Bedingung an ein Feld, über das der Coordinator entscheidet.
3. **Rolle.** `plugin_kind` ist ohne Berechtigungsnachweis nicht prüfbar. Kleinster Schritt:
   rollenabhängige Entscheidungen nur aus geprüften Merkmalen (Beitritt nach Punkt 2). Weiter
   gehend und **nicht** der kleinste Fix: einen lebenden Link (`link.rs:145-155`) nur aus
   demselben Peer-Prozess verdrängen, sonst den vorhandenen Kollisionsweg nehmen. Das berührt
   die Verdrängungsregeln C-10 (`docs/beweise/SONDE-011.md:516`) und H-10
   (`docs/beweise/NAK-121.md:224`; Kommentar `link.rs:145-150`) sowie die Aliasregeln
   (`broker/src/instance_alias.rs`) und braucht eigene Matrixzeilen.

**v2, ohne den v2-Vertrag zu ändern.**

4. **Übernahme nur im selben Prozess.** Die Peer-PID der v2-Verbindung festhalten; eine zweite
   Verbindung derselben `sensor_id` aus einem anderen Prozess bekommt `konflikt` wie heute
   (sichtbar, Plan §8.4), aber nicht die Schreibrechte (`register.rs:211-213`). Eine
   FL-Duplikation läuft im selben FL-Prozess und behält ihr heutiges Verhalten; „Eigentum
   wechselt vollständig oder gar nicht" (`register.rs:9`) bleibt wahr.
5. **Token nicht anfassen.** Er berechtigt zu nichts (e) 2); ein Token je Verbindung änderte
   den Satz „zufällig pro Broker-Lauf" im eingefrorenen v2-Vertrag und brächte keinen Schutz.
   Stattdessen eine Wache, dass keine Stelle den v2-Token außerhalb seiner Verbindung annimmt.
   Die Offenlegung in der Diagnoseantwort gehört Teil A (T3-12-04).

**Restlücke, bei jedem Weg.** Ein Prozess desselben Benutzers kann mit eigener PID beliebige
Rollen und frische Kennungen behaupten und, kennt er die Kennungen einer echten Instanz, deren
Schlüssel beanspruchen. Die Hörmarkierung über v2 bleibt eine brokerweite Sperrmöglichkeit
jedes verbundenen v2-Clients; T3-12-05 begrenzt sie zeitlich, nicht dem Grunde nach. Auch die
geplante Capability schützt nicht dagegen: sie liegt als Current-User-DPAPI-Blob im
Projektstate (`Entwurf :3522-3523`, `:4108-4109`), und DPAPI bindet an den Benutzer, nicht an
den Prozess (Microsoft Learn). Die Klasse „derselbe Benutzer" ist mit Produktmitteln nur zu
verkleinern, nicht zu schließen; das deckt sich mit dem Ausschluss eingeschleusten Codes im
Threat-Model.

**Mitzuprüfen:** verbinden↔trennen (Prozesshandle je Verbindung, Freigabe beim Schließen;
`control_anmelden` ↔ `kopplung_loesen`); Hello↔Bye (v2-Übernahme ↔ `sensor_trennen`,
`register.rs:379-416`); Token vergeben↔prüfen (unverändert); starten↔stoppen (SID-Zeichenkette
entsteht beim Start, `lebenslauf.rs:161`). Die C++-Seite braucht keine Änderung, die Fixtures
der Clients aus d) aber die echte SID.

### h) Ort des Rotbeweises

**Bein A4**, `broker/tests/security_vectors.rs`, neben B-01 (`:417-444`) und B-04 (`:489-496`),
auf Probe-Namen (`probe_pipe`, `:60-66`):

- **Fall „Draht-SID ≠ Token-SID":** Control-Hello mit der erfundenen SID → kein Welcome,
  Schließen, `assert_keine_fachlogik` (`:178-184`). **Heute rot** (Welcome kommt, `:423-424`).
  Die Zeile, die die Zusage trägt, ist der neue Vergleich zwischen `verbindung.rs:259` und
  `:300`; entfernt eine Mutation ihn, muss der Fall rot werden.
- **Gegenfall:** dasselbe Hello mit der echten SID → Welcome. `server` ist crateprivat
  (`broker/src/lib.rs:39`); der Test holt die SID über die Token-API oder einen öffentlichen
  Helfer.
- **Fall „host.pid ≠ Peer-PID":** fremde PID → Schließen; **Gegenfall** `std::process::id()` →
  Welcome.
- B-01 und B-08 wechseln auf die echte SID; ihr Prüfziel bleibt.
- **v2**, `broker/src/server.rs` neben `:1270-1328`: Übernahme einer lebenden `sensor_id` aus
  einem **zweiten Prozess** → `konflikt`, aber Schreibrechte bleiben; Gegenfall aus demselben
  Prozess wie heute. Dafür braucht der Test einen Client im Kindprozess.
- A4-Behauptung (`tools/beweise.ps1:518`) um die Bindung ergänzen; A22 (`:637`), B10 (`:738`)
  und das Soak-Bein laufen nach der Fixture-Umstellung unverändert grün.

---

## 2. T3-12-03 — der feste v2-Name blockiert den v3-Start (B05)

*Befundzeile `BEFUNDE.md:483` · Rohbeleg `roh/phase-12-pipesicherheit.md:20`, Schwere `:28`,
Vorlauf `:50` · Skeptiker `BEFUNDE.md:551` = `roh/phase-10-skeptiker.md:92` · Bündel
`BEFUNDE.md:75`*

### a) Behauptung des Audits

Der Brokerprozess startet zuerst die maschinenweit feste v2-Pipe mit
`FILE_FLAG_FIRST_PIPE_INSTANCE` und bricht bei fremd belegtem Namen bis `main` ab; die
SID-gebundene v3-Pipe entsteht dann trotz freien Namens nie (`BEFUNDE.md:483`).

### b) Quellkette am HEAD

**Station 1 — Prozessstart.** `broker/src/main.rs:50`
`match eqcop_broker::broker_starten(bindungen.clone()) {`, Fehlerzweig `:76-79`:

```
        Err(e) => {
            eprintln!("Broker-Start fehlgeschlagen: {e}");
            std::process::exit(1);
        }
```

**Station 2 — v2 vor v3, mit `?`.** `broker/src/lebenslauf.rs:41`
`pub const PIPE_NAME: &str = r"\\.\pipe\evenacadia.eq-copilot.v1";`, dann `:152-158`:

```
            let griff_v2 = server::server_starten_mit_interventionssenke(
                PIPE_NAME,
                register.clone(),
                broker_version(),
                session_token.clone(),
                coordinator.clone(),
            )?;
```

Erst danach `:161` SID, `:162` v3-Name, `:163-169` v3-Start. Store und Coordinator entstehen
schon vorher (`:143-151`).

**Station 3 — die erste v2-Instanz.** `broker/src/server.rs:494`
(„Erste Instanz synchron — der Belegt-Fehler gehört dem Aufrufer."), `:502-513` mit `:505`
`PIPE_ACCESS_DUPLEX | FILE_FLAG_FIRST_PIPE_INSTANCE,`, Fehlerrückgabe `:514-525` (Win32 5 →
„Pipe {name} ist schon belegt …").

**Station 4 — der Ausfall ist im Produkt unsichtbar.** Das Plugin startet den Broker in
`eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp:604-629` mit `CREATE_NO_WINDOW`, ohne
`STARTF_USESTDHANDLES`, erbt keine Handles (`FALSE`, `:619`) und schließt das Prozesshandle
sofort (`:624-625`). `eprintln!` geht ins Leere, der Exitcode wird nicht gelesen. Das Plugin
sieht nur „Pipe fehlt": Spawn, Wartefrist `SPAWN_BEREIT_TIMEOUT_MS = 10000`, Cooldown
`SPAWN_COOLDOWN_MS = 30000` (`BrokerLifecycle.h:23-24`; Zyklus `.cpp:816-856`, `:914-934`).
Solange der Name belegt ist, startet es also etwa alle 40 s einen Broker, der sofort endet.

**Station 5 — Auslöser.** Der Name ist nicht benutzergebunden; jeder Prozess, auch eines
anderen Benutzers, kann ihn zuerst belegen (`roh/phase-12-pipesicherheit.md:20`; allgemein
`wissen/engineering/2026-08-27-windows-pipes-sicherheit-und-lifecycle.md:52`). Es gibt auch
nicht bösartige Auslöser: die Fehlermeldung selbst nennt „zweite App-Instanz oder Probe neben
laufender App" (`server.rs:519-520`), und das v2-Probewerkzeug nimmt jeden Namen an, auch den
Produktionsnamen (`broker/src/bin/eqcop-broker-probe.rs:21-23`, `broker/src/probe.rs:22-23`
„auf einem FREI WÄHLBAREN Pipenamen"), anders als das v3-Werkzeug
(`eqcop-broker-v3probe.rs:38-44`, Erlaubnisprüfung).

**Station 6 — Herkunft.** Die v2-Zeile war vor v3 schon fatal (`901598fc` ersetzt
`server_starten(…)?` durch `server_starten_mit_interventionssenke(…)?`). Der v3-Start kam mit
`512790a2` (31.08.2026, SONDE-011 Phase B Bau Teil 1) **dahinter**; NAK-224 hat den Block
unverändert nach `lebenslauf.rs` verschoben (`d64d8399`; `docs/beweise/NAK-224.md:714`). Die
Kopplung ist nie entschieden worden.

### c) Zusage wörtlich

- **Entwurf §48.3**, `docs/FL-Nakama-Sonden-Design-Entwurf.md:3477-3480`: „Während der
  Migration kann der Broker zusätzlich den heutigen festen Namen `\\.\pipe\evenacadia.eq-copilot.v1`
  als strikt v2-/Heartbeat-only Listener mit User-DACL anbieten. Alle v3-Instanzen versuchen
  zuerst die SID-gebundene Pipe." — v2 ist ein **zusätzliches**, **optionales** Angebot. Es zur
  Vorbedingung von v3 zu machen, widerspricht dem Satz.
- **NAK-123 A-01**, `docs/beweise/NAK-123.md:37`: Zustand „Broker aus; v3-Name frei", Ereignis
  „Brokerstart", Zusage „Die erste RAII-besessene Instanz entsteht mit aktueller User-DACL,
  `PIPE_REJECT_REMOTE_CLIENTS` und `FILE_FLAG_FIRST_PIPE_INSTANCE`; eine zweite entsteht ohne
  First-Flag." Bei belegtem v2-Namen entsteht sie nicht.
- **Eingefrorene Identität** (`CLAUDE.md:197-198`): „Class-IDs, Hersteller-/Plugin-Codes und
  Replace-V2-Verhalten". Der Pipename steht nicht darin. Er steht an zwei Stellen
  (`lebenslauf.rs:38-41`, „Muss mit eqcop::kPipeName im Plugin übereinstimmen";
  `eq-copilot/plugin/src/EqCopilotIds.h:16-18`), aber nicht im v2-Vertrag
  (`eq-copilot/schemas/eq-ipc.schema.json:5` nennt nur „Windows Named Pipe (Byte-Modus), ACL nur
  aktueller User").

### d) Heutige Abdeckung

- `security_vectors.rs:206-225` (A-01) und `:258-269` (A-02) starten **nur** den v3-Listener
  (`v3_server_starten_fuer_security_vectors`). Sie widerlegen die Kopplung nicht
  (`roh/phase-10-skeptiker.md:92`).
- `server.rs:1221-1236` prüft den v2-Server allein: zweiter Start auf demselben Namen scheitert
  mit „belegt". Für den v2-Server bleibt das richtig.
- **Quelltextwache, die das `?` festschreibt:** `server.rs:1330-1339` liest `lebenslauf.rs` und
  verlangt wörtlich `"session_token.clone(), coordinator.clone(), )?;"` (`:1339`). Zweck ist die
  Verdrahtung mit dem Coordinator (NAK-224.md:552); unbeabsichtigt friert sie die fatale
  Weitergabe ein. Sie muss mit dem Fix angepasst werden.
- `broker_idle.rs:187-235` prüft die Stoppreihenfolge Supervisor < v2 < v3 < Store; davon
  unberührt.
- **Kein Test startet die Komposition.** `broker_starten` benutzt die Produktionsnamen
  (`lebenslauf.rs:152-163`) und eine prozessweite `OnceLock` (`:120`);
  `broker_idle.rs:182-186` begründet das. Die A4-Behauptung „produktiver v2+v3-Verdrahtung"
  (`tools/beweise.ps1:518`) misst nur die Quelltextwache.
- **Irreführende Kommentare:** `server.rs:420-428` („scheitert der Start SOFORT SICHTBAR") —
  sichtbar nur an einer Konsole, im Produkt unsichtbar (Station 4); `lebenslauf.rs:126-127`
  („Ein Fehler bleibt sichtbar im Status stehen — die App läuft weiter") — Stand der
  Hub-App, das eigenständige Programm endet (`main.rs:76-79`); `main.rs:52-54` meldet „läuft auf
  {PIPE_NAME}" und nennt nur den v2-Namen; `docs/plugin-wissen.md:1156-1159` („fremder Besitzer ⇒
  Start verweigert") mit veralteten Verweisen `lib.rs:29`, `server.rs:242`.

### e) Urteil: BESTÄTIGT

Die Kette schließt am HEAD vollständig; keine Datei ist seit `d20b8c15` geändert.
Präzisierungen: die Kopplung entstand ungeplant mit `512790a2` (b 6); der Ausfall ist für
Musiker und Plugin unsichtbar und erzeugt einen Spawnzyklus von etwa 40 s (b 4); neben einem
Angreifer genügen eine alte App-Instanz oder das v2-Probewerkzeug mit Produktionsnamen (b 5);
eine Quelltextwache schreibt das `?` fest (d).

### f) Kategorie und Schwere

**DEFEKT** gegen Entwurf §48.3 (`:3477-3480`) und NAK-123 A-01 (`NAK-123.md:37`).

**Schwere high** (wie Audit). Solange der Name belegt ist, fehlen Broker, Quellen, Befunde und
Advisor vollständig; Audio und State bleiben unberührt (`roh/phase-12-pipesicherheit.md:28`).

### g) Kleinster Fix

**Variante A (empfohlen): v3 zuerst, v2-Kollision nicht fatal.** In `broker_starten`
(`lebenslauf.rs:130-223`) SID, v3-Name und v3-Listener zuerst; danach v2. Scheitert v2, wird
der Fehler ins Register geschrieben (`fehler_merken`), `_griff_v2` bleibt `None` (die Hülle ist
schon `Mutex<Option<…>>`, `:87`), der Broker läuft weiter.

- **Einzelinstanz bleibt gewahrt:** ein zweiter Broker scheitert jetzt zuerst an v3 (A-02,
  `NAK-123.md:38`) und fasst v2 gar nicht an.
- **Ehrlicher Zustand:** `broker_status().pipe_name` (`lebenslauf.rs:388`) und die
  Startmeldung (`main.rs:52-54`) dürfen keinen v2-Dienst behaupten, den es nicht gibt;
  Registerfehler und Diagnose (Teil A) nennen die Belegung.
- **Folge für Altprojekte und Legacy-Rolle:** ohne Kollision keine. Mit Kollision findet Gens
  v2-Client den fremden Server, lehnt ihn ab (NAK-123 C-06, `NAK-123.md:74`;
  `eq-copilot/plugin/src/PipeClient.cpp:316-331` parkt im `authBlockiert`-Warten) und liefert
  keinen v2-Konflikthinweis mehr („Kennung doppelt!", `PluginEditor.cpp:292`); alles über v3
  läuft. Heute läuft in diesem Fall gar nichts.

**Variante B: Legacy-Name benutzergebunden.** Braucht dieselbe Änderung in `EqCopilotIds.h:18`
und `lebenslauf.rs:41` (beide liefern im selben Bundle aus; Vertrag und eingefrorene Identität
nennen den Namen nicht). Ältere v2-Clients außerhalb des Bundles, etwa eine alte Hub-App,
fänden den Broker nicht mehr. Gegen Belegung durch denselben Benutzer hilft es nicht (der Name
ist ableitbar wie der v3-Name, A-02 gilt dann auch dort). Nicht der kleinste Weg.

**Variante C: v2-Listener entfernen.** Beendet den Migrationspfad aus §48.3 und nimmt Gens
v2-Client den Server (`PluginProcessor.cpp:463` `pipe.start();`) samt Konflikthinweis. Das ist
eine Produktänderung und gehört nicht in NAK-310.

**Mitzuprüfen:** v2-Start↔v3-Start (Reihenfolge und Fehlerbehandlung je Listener);
starten↔stoppen (Stoppreihenfolge `lebenslauf.rs:266-278` mit `None` für v2, Wache
`broker_idle.rs:209-212`); Leerlauf (`lebenslauf.rs:316-323` zählt ohne v2-Server 0);
Status↔Wirklichkeit. Die Quelltextwache `server.rs:1339` ist mitzuändern. Nebenbefund außerhalb
der ID: das v2-Probewerkzeug braucht dieselbe Erlaubnisregel wie das v3-Werkzeug
(`CLAUDE.md:191`, `:225-226`).

### h) Ort des Rotbeweises

**Bein A4.** Voraussetzung ist eine Naht: die Startkomposition als Funktion mit übergebenen
Namen. Das Produkt ruft sie mit `PIPE_NAME` und `pipe_name_v3(SID)`, der Test mit einem
v2-Testnamen (Muster `server.rs:1049-1055`) und einem v3-Probenamen (`PROBE_PRAEFIX`,
`broker/src/transport/pipetoken.rs:28`, Erlaubnis `:38-43`). Wegen der `OnceLock` ruft der Test
die Naht, nicht `broker_starten`, oder fährt sie in einem Kindprozess wie
`broker_idle.rs:99-174`.

- **Fall:** v2-Testnamen vorher belegen (Muster `FirstInstance`, `security_vectors.rs:28-58`),
  Komposition starten. Erwartet: Start gelingt; `FirstInstance::nehmen(v3-Name)` liefert `None`
  (Broker besitzt v3); ein v3-Client erhält Welcome; das Register nennt die v2-Belegung.
  **Heute rot:** `Err` aus dem `?` in `lebenslauf.rs:158`, v3-Name bleibt frei.
- **Mutation:** das fatale `?` an der v2-Startzeile zurücksetzen → rot.
- **Gegenfälle:** beide Namen frei → beide bedient; zweite Komposition auf denselben Namen →
  scheitert an v3 (A-02), ohne v2 zu berühren.
- Behauptungszeile `tools/beweise.ps1:518` um die Entkopplung ergänzen.

---

## 3. T3-12-05 — der v2-Listener hat weder Fristen noch Rate (B05)

*Befundzeile `BEFUNDE.md:485` · Rohbeleg `roh/phase-12-pipesicherheit.md:22`, Schwere `:29`,
Vorlauf `:54` · Skeptiker `BEFUNDE.md:553` = `roh/phase-10-skeptiker.md:94` · Bündel
`BEFUNDE.md:75`*

### a) Behauptung des Audits

Der produktive v2-Listener hat weder absolute Lese- oder Inaktivitätsfrist noch Frame- oder
Byte-Ratengrenze; bis zu 64 halbe oder schweigende Verbindungen binden alle Worker, und die
60-s-Leerlaufentscheidung beendet einen Broker mit registriertem schweigendem Client nicht
(`BEFUNDE.md:485`).

### b) Quellkette am HEAD

**Station 1 — Annahme bis 64.** `broker/src/server.rs:45-48`:

```
/// Der Produktvertrag testet bis 32 Quellen. Der Faktor zwei lässt einen
/// kurzen Reconnect-/Duplikat-Überlapp zu, hält aber Threads, Kernelpuffer und
/// Handles hart begrenzt. Windows erlaubt höchstens 255 Pipe-Instanzen.
const MAX_VERBINDUNGEN: usize = 64;
```

Bei voller Belegung legt der Acceptor keine lauschende Instanz an (`:549-555`); jede
angenommene Verbindung bekommt einen eigenen Thread (`:620-634`).

**Station 2 — das Hello ohne Frist.** `server.rs:672-673`:

```
    // Erstes Paket muss ein gültiges hello sein — sonst Verbindung beenden.
    let erster = match frame_lesen(&mut datei) {
```

`broker/src/framing.rs:34-66` liest blockierend; geprüft wird nur die Länge
(`:53-55`, `MAX_FRAME_BYTES = 262_144`, `:9`). `:56` alloziert die angekündigte Länge vor dem
Lesen: je halbem Rahmen bis 256 KiB, bei 64 Verbindungen bis 16 MiB. Die Handles sind synchron
(`server.rs:505`, `:565` ohne `FILE_FLAG_OVERLAPPED`); abgebrochen wird nur beim Stopp
(`CancelSynchronousIo`, `:283-312`, aus `stoppen` `:395-411`).

**Station 3 — der Betrieb ohne Frist und Rate.** `server.rs:781-783`:

```
    let mut letzte_seq = None;
    while !stop.load(Ordering::SeqCst) {
        match frame_lesen(&mut datei) {
```

Jeder Heartbeat kostet Parser, Tokenvergleich, Registersperre (`:841-847`), ACK-Schreiben
(`:853-862`) und, sobald er einen Messblock trägt, einen Aufruf der Brücke in die
Coordinatorsperre (`:848-852` → `coordinator/intervention.rs:273-274`
`let mut stand = self.stand.lock()…`). Die Sequenz muss steigen (`:806`, `i64`); das begrenzt
Wiederholungen, nicht die Rate.

**Station 4 — Frische nur als Anzeige.** `broker/src/register.rs:23-25`:

```
/// Verbunden, aber länger als das hier ohne Heartbeat ⇒ stale. Sichtbar
/// markiert, nie still entfernt (Plan §11 M2-Abnahme).
pub const STALE_MS: u64 = 5000;
```

`stale` entsteht nur im Snapshot (`:482-489`); `verbunden` fällt erst mit dem Verbindungsende
(`:408-411`).

**Station 5 — die Leerlaufentscheidung.** `broker/src/lebenslauf.rs:316-324`:

```
    let aktive_v2 = lauf
        .register
        .lock()
        .expect("Register-Mutex")
        .sensoren
        .values()
        .filter(|sensor| sensor.verbunden)
        .count();
    let aktive = aktive_v2.saturating_add(aktive_v3);
```

mit `:280-282` (Ende nur bei `aktive_clients == 0` und 60 s, `BROKER_IDLE_ENDE_MS`, `:45`).
Ein registrierter, schweigender v2-Client hält den Broker unbegrenzt am Leben. Halbe Hellos
zählen nicht mit; sie fallen erst, wenn der Broker mangels **jedes** anderen Clients endet.
**Während einer echten Sitzung begrenzt sie nichts.**

**Station 6 — Wirkung.** Bei 64 gebundenen Plätzen erhält ein neuer v2-Client
`ERROR_PIPE_BUSY`; Gens `PipeClient` stuft nach 20 Versuchen `belegtNichtErreicht` ein
(`PipeClient.cpp:380-431`), der Konflikthinweis entfällt. v3 hat eigene Pipe und eigenen
Worker-Pool (`server_v3/mod.rs:143-149`) und wird nicht direkt ausgehungert. Querwirkungen auf v3
gibt es zwei: die geteilte Coordinatorsperre (Station 3) und die brokerweite Evidenzsperre,
die eine schweigende v2-Verbindung mit gemeldeter Hörmarkierung ohne jede Frist hält
(`intervention.rs:273-297`, `:634-640`; Verbindung zu T3-12-02).

**Gegenprobe v3, selbst nachgelesen.**

| Grenze | v3 am HEAD | v2 am HEAD |
|---|---|---|
| Bootstrapfrist | `BOOTSTRAP_FRIST` 5000 ms (`server_v3/mod.rs:151-154`), eingetragen `verbindung.rs:193-201`, Wachhund alle 100 ms (`listener.rs:369-409`, `WACHHUND_TAKT` `mod.rs:226`) | keine |
| Hellogröße | `MAX_BOOTSTRAP_BYTES` 16 KiB (`transport/v3.rs:47-49`), Rohpuffer `verbindung.rs:232-237` | 262144 Bytes (`framing.rs:9`) |
| Rate | `RATE_PRO_SEKUNDE` 4000 (`mod.rs:156-158`), `Ratengrenze` (`v3.rs:404-437`), angewandt `verbindung.rs:639`, Schließen `:657-661` | keine |
| Ingress | 256 Rahmen (`transport/warteschlange.rs:24`), 16 KiB je Rahmen (`:42`), 4 MiB je Verbindung (`:52`) | ein Rahmen je Thread, keine Warteschlange |
| Inaktivität | stale nach 2500 ms (`coordinator/mod.rs:103-106`), Tombstone 10000 ms (`:107`), `liveness_tick` (`liveness.rs:225-282`) liefert zu schließende Links, der Supervisor schließt (`lebenslauf.rs:178-187`) | nur Anzeige `stale` (Station 4) |
| Verbindungen | 96 Worker + 2 Listener (`mod.rs:143-149`) | 64 (`server.rs:48`) |

### c) Zusage wörtlich

- **v2-Vertrag** `eq-copilot/schemas/eq-ipc.schema.json:5`: „Harte Paketgrenze 262144 Bytes —
  größere Pakete beenden die Verbindung. … Der Client sendet als erstes Paket immer `hello`" —
  keine Frist, keine Rate. `:150` zu `bye`: „Geordneter Abschied (Plugin-Destruktor). Fehlt er,
  greift Last-Seen/Timeout im Broker." — „Last-Seen" ist gebaut (Station 4), „Timeout" ist
  nirgends festgelegt. Der Satz kündigt eine Frist an, ohne sie zu bestimmen.
- **Entwurf §33.1** `:2055-2057`: „Alle Pakete bleiben unter dem bestehenden Größenlimit;
  Parser erhalten Tiefen-, Längen- und Nachrichtenratenlimits. Ein v2-Client nutzt weiter nur
  seine eine Heartbeat-Verbindung …" — der Satz steht im Absatz über die zwei v3-Verbindungen;
  der Code zitiert ihn als Quelle der v3-Rate (`transport/v3.rs:404-405`). Ebenso §48.4
  `:3512-3513` („Danach prüft der Server … Länge und Rate"), der zur Impersonationskette gehört,
  die NAK-123 nur für v3 gebaut hat (`NAK-123.md:49`).
- **Entwurf §53.9** `:4116-4118`: „Die erste Nachricht jeder Verbindung ist ein höchstens
  16 KiB großes, wie heute nur mit `u32` längenpräfigiertes JSON-Hello. `protocol=2` bleibt
  danach vollständig im v2-Parser." — beschreibt einen gemeinsamen Bootstrap für beide
  Protokolle. Gebaut sind zwei Endpunkte; auf dem v3-Endpunkt wird ein v2-Hello abgelehnt
  (`verbindung.rs:284-299`), auf dem v2-Endpunkt gelten 262144 Bytes. Das ist ein Widerspruch
  Entwurf ↔ Code, über den Vertrag und Code stehen (`CLAUDE.md:61-62`); er gehört als Zeile nach
  `docs/offene-punkte.md`, nicht als Defekt hierher.
- **Entwurf §48.3** `:3487-3488` („beendet sich nach einer Leerlauffrist ohne Clients") und
  **NAK-123 A-06** (`NAK-123.md:42`, „Keine aktiven v2-/v3-Clients") — der HEAD hält beide: ein
  schweigender Client ist ein Client.
- **Technikregel ohne Zusagecharakter:** `wissen/engineering/2026-08-27-windows-pipes-sicherheit-und-lifecycle.md:55`
  („eine absolute Deadline über den ganzen Frame") und `:58` („harte Caps plus Bootstrap- und
  Idle-Deadline"); SONDE-010 liest §33.1 und §53.9 als Forderung nach Fristen und Raten
  (`docs/beweise/SONDE-010.md:118-121`) und hält zugleich „v2 bleibt v2" fest (`:311`).
  Registerpunkt NAK-07 („Pipe-Deadlines") gilt seit SONDE-010 als „vollständig geschlossen"
  (`docs/offene-punkte.md:473`) — gebaut wurden die Fristen nur für v3.

**Ergebnis:** für v2 schweigen die bindenden Quellen. Die Regel gibt es als Technikregel des
Projekts, angewandt nur auf v3.

### d) Heutige Abdeckung

- `server.rs:1724-1755` (`connection_und_pipe_instanzzahl_bleiben_hart_begrenzt`): zwei Clients
  „bleiben absichtlich schon vor hello stehen und binden damit genau die beiden zulässigen
  Connection-Threads" (`:1734-1735`). Der Test prüft die Grenze und beschreibt die Bindung durch
  schweigende Clients als erwartet; eine Frist prüft er nicht. Er endet mit `griff.stoppen()`
  ohne eigene Frist (`:1754`) und ist der Test, der 45 Minuten hing (NAK-241,
  `docs/offene-punkte.md:253`, offen; `docs/beweise/NAK-224.md:436-448`).
- `server.rs:1757-1776`: der Stopp beendet eine stille Verbindung binnen 2 s (Muster
  `recv_timeout`, `:1771-1773`).
- `server.rs:1792-1840`: Fristen nur für den Flush beim Schreiben.
- `framing.rs:90-101`: Längengrenze.
- `broker/tests/broker_idle.rs`: Leerlauf nur über den v3-Probe-Broker und die reine
  Zählfunktion; kein Test für den v2-Anteil `lebenslauf.rs:316-323`.
- A22 (`tools/beweise.ps1:637`) belastet nur v3.
- **Nicht gemessen:** v2-Bootstrapfrist, v2-Inaktivität, v2-Rate, 64 schweigende v2-Clients mit
  gleichzeitig erreichbarem v3, schweigender v2-Client gegen Leerlauf.
- **Irreführende Stellen:** `docs/offene-punkte.md:473` (NAK-07 „vollständig geschlossen",
  gebaut nur für v3); `docs/plugin-wissen.md:1121-1122` (Leerlauf „liest die aktiven Worker"
  nennt den v2-Anteil nicht); `server.rs:45-47` („hart begrenzt" stimmt für Threads und Handles,
  nicht für die Belegungsdauer).

### e) Urteil: BESTÄTIGT

Alle drei Teilaussagen tragen am HEAD. Präzisierungen: halbe Hellos begrenzt das Leerlaufende
nur, wenn der Broker sonst keinen Client hat (b 5); v3 wird nicht direkt ausgehungert, aber über
die geteilte Coordinatorsperre und die fristlose Evidenzsperre berührt (b 6); der v2-Vertrag
kündigt in `:150` einen Timeout an, den es nicht gibt (c).

### f) Kategorie und Schwere

**LÜCKE, Technikregel.** Keine bindende Quelle regelt v2-Fristen oder -Raten; der Musiker hat
hier keine Wahl zu treffen, denn keine Variante ändert, was er im Normalbetrieb hört oder sieht.
**Schwere medium** (wie Audit): erreichbar lokal, Wirkung Verfügbarkeit von v2, Lebensdauer des
Brokers und die fristlose Evidenzsperre, kein Audio- oder State-Schaden.

### g) Kleinster Fix

Alles in `server_starten_mit_grenzen` und `verbindung_bedienen` (`server.rs:464-661`,
`:663-911`), ohne Änderung am v2-Vertrag:

1. **Wachhund für v2** nach dem v3-Muster (`listener.rs:369-409`): Liste (Threadhandle, Frist);
   bei Fälligkeit `CancelSynchronousIo` auf den Verbindungsthread — dasselbe Mittel, das der Stopp
   schon benutzt (`server.rs:283-312`), passend zu den synchronen v2-Handles.
2. **Bootstrapfrist** 5 s wie v3, dazu **Hellogrenze 16 KiB**. Gültige v2-Hellos sind durch die
   Feldgrenzen des Vertrags (`eq-ipc.schema.json:25-36`) weit kleiner; die Grenze lehnt kein
   vertragsgemäßes Hello ohne künstliche Füllung ab.
3. **Inaktivitätsfrist** je empfangenem Rahmen erneuert, länger als `STALE_MS` (5 s), damit der
   sichtbare Zustand „stale" erhalten bleibt (`register.rs:23-24`); Richtwert wie v3 (2,5 s +
   10 s). Heartbeat-Takt des Plugins ist 1000 ms (`EqCopilotIds.h:23`). Das Schließen läuft über
   `RegistrierteVerbindung::drop` (`server.rs:352-372`): Sensor sichtbar getrennt, Brücke meldet
   `getrennt` → `unknown`, bis ein v3-Resync ihn löst (`intervention.rs:299-307`, `:432`).
4. **Rate je Verbindung** mit der vorhandenen `Ratengrenze` (`v3.rs:410-437`) und einer
   v2-gemäßen Schranke; Überschreitung schließt wie ein Protokollbruch.
5. **Leerlauf:** keine Änderung nötig; die Inaktivitätsfrist beendet `verbunden`.

**Mitzuprüfen:** verbinden↔trennen (Fristeintrag ↔ Austrag am Threadende, kein Abbruch auf
einem beendeten Thread); Hello↔Bye; Leerlauf↔Frist; starten↔stoppen (Wachhund vor den
Verbindungsthreads joinen); Hörmarkierung (Schließen einer Markerverbindung setzt `unknown`,
fail-closed wie gewollt).

### h) Ort des Rotbeweises

**Bein A4**, `broker/src/server.rs` neben `:1724-1755`, mit `server_starten_mit_grenzen` und
kleinen Grenzen; **jeder Fall mit eigener Frist** nach `:1766-1774`, sonst droht der
NAK-241-Hänger.

- **Fall 1:** zwei Clients vor dem Hello binden beide Plätze; nach Bootstrapfrist plus einem
  Wachhundtakt verbindet ein dritter. **Heute rot.** Mutation: Fristeintrag entfernen → rot.
- **Fall 2:** etablierter, schweigender Client ist nach der Inaktivitätsfrist `verbunden == false`;
  Gegenfall mit 1-Hz-Heartbeats bleibt verbunden.
- **Fall 3:** Flut über der Rate → Verbindung zu, `pakete_verworfen` gezählt; Gegenfall normale
  Rate.
- **Fall 4:** Hello mit 16 KiB + 1 → geschlossen, ohne 256 KiB zu allozieren.
- **Fall 5 (Leerlauf):** über die Naht aus 2 h): schweigender v2-Client verhindert das
  Leerlaufende nur bis zur Frist.

Weil die Quellen schweigen, trägt die neue Regel die Zusage; der Rotbeweis fällt an der
Fristprüfung im Wachhund, nicht an einem Nebeneffekt.

---

## 4. Gemeinsame Ursachen

**Wurzel 1 — Authentisierung ist nicht Autorisierung (T3-12-02, T3-12-04).** DACL und SID-Tor
beweisen den Benutzer, nicht die Instanz; alles dahinter vertraut Behauptungen. T3-12-04
(Teil A) ist dieselbe Wurzel am Diagnose-Briefkasten. **Schnittstelle:** der v2-Sitzungstoken
(`lebenslauf.rs:142` → `briefkasten.rs:787-822`). Weil er zu nichts berechtigt (1 e 2), erweitert
seine Offenlegung keine v2-Berechtigung (`roh/phase-10-skeptiker.md:120`). Ich empfehle, den Token
in NAK-310 **nicht** zu ändern; würde er je Verbindung vergeben, änderte sich die Ableitung der
Diagnosekennungen in Teil A. Die Maskierung entscheidet Teil A.

**Wurzel 2 — v2 ist ein ungebundener, unbegrenzter Nebeneingang, an dem v3 hängt (T3-12-03,
T3-12-05).** Beide berühren den Grundsatz „v2 ist zusätzlich" (§48.3) und dieselben Dateien, aber
verschiedene Funktionen: T3-12-03 `broker_starten` (`lebenslauf.rs`), T3-12-05 Acceptor und
Worker (`server.rs`). Getrennte Commits; gemeinsam ist die Kompositionsnaht aus 2 h), die
T3-12-05 für den Leerlauffall braucht.

**Kreuzung T3-12-02 × T3-12-05.** Eine unbelegte Behauptung (Hörmarkierung über v2) trifft auf
fehlende Fristen: eine schweigende v2-Verbindung hält die brokerweite Evidenzsperre unbegrenzt.
Die Frist aus T3-12-05 begrenzt das zeitlich, die Prozessbindung aus T3-12-02 begrenzt, wer es
darf. Beides gehört in dieselbe Matrix, nicht in denselben Commit.

**Was getrennt bleiben muss.** Der v3-Teil von T3-12-02 (Tor, Coordinator, Fixture-Umstellung in
A4, A22, B10, Soak) ist ein eigener Änderungssatz. T3-12-01 (Teil A) berührt keine
Broker-Datei. T3-12-04 berührt `briefkasten.rs`, nicht die drei Dateien hier.

---

## 5. Produktfragen

`roh/phase-12-pipesicherheit.md:58-60` nennt „Keine"; `BEFUNDE.md:487` zählt 0. Die sieben
Fragen der Phase 10 (`roh/phase-10-skeptiker.md:124-134`) betreffen T3-12 nicht.

Eigene Prüfung je ID:

| ID | Technik (Dirigent) | Produkt (User) |
|---|---|---|
| T3-12-02 | Vergleichsform, Schließen oder `unauthorized`, Prozesshandle, Fixture-Umstellung, Übernahmeregel über Prozessgrenzen | Nichts. Legitime Instanzen senden schon die geprüften Werte; sichtbar ändert sich nichts. |
| T3-12-03 | Variante A, Registertext, Naht | Nichts. §48.3 hat v2 als Zusatz entschieden; Variante A ist in jedem Fall besser als der Totalausfall. |
| T3-12-05 | Fristen, Rate, Hellogrenze, Wachhund | Nichts. Ein hängender v2-Client wird „getrennt" statt dauerhaft „stale"; das sieht nur die Diagnose. |

Eine echte Wahl entstünde erst mit **Variante C** (v2-Listener entfernen), weil dann Gens
Hinweis „Kennung doppelt!" seinen Weg verliert. Das gehört nicht in NAK-310. Falls der Dirigent
die Frage später stellt, ein Wortlautvorschlag:

> „Nakama spricht intern noch einen älteren Verbindungsweg, über den Gen dir meldet, wenn zwei
> Instanzen dieselbe Kennung tragen. Soll dieser alte Weg bleiben, bis es für den Hinweis einen
> neuen gibt (empfohlen), oder soll er jetzt wegfallen?"

---

## 6. Reihenfolge

1. **Etappe 1 — T3-12-03** (Variante A, Kompositionsnaht, ehrlicher Status, Quelltextwache
   `server.rs:1339` anpassen). Kleinster Eingriff, größte Wirkung auf die Verfügbarkeit, kein
   Vertrag berührt.
2. **Etappe 2 — T3-12-05** (v2-Wachhund: Bootstrapfrist, Hellogrenze, Inaktivität, Rate). Nutzt
   die Naht aus Etappe 1; NAK-241 (fehlende Testfrist in derselben Testdatei) im selben Schritt.
3. **Etappe 3 — T3-12-02 v3** (SID-Zeichenkette, Peer-PID, Fixture-Umstellung in A4, A22, B10,
   Soak). Größter Testumfang. Die Synthese stellt B04 hinter B12 (`BEFUNDE.md:125`); B12 liegt in
   NAK-313 (S25l, Manifest noch nicht angelegt, `docs/offene-punkte.md:317`). Für die
   Feldvergleiche ist das nicht blockierend; die Regel für doppelte JSON-Schlüssel (T3-03-09,
   P10-03) trifft aber auch das Hello und ist mit NAK-313 abzustimmen. **Muss vor P7
   (SONDE-016) fertig sein.**
4. **Etappe 4 — T3-12-02 v2** (Übernahme nur im selben Prozess, Wache gegen Tokenannahme
   außerhalb der Verbindung).

**Ausgeschlossen, weil sie Vertrag oder Migrationspfad berühren:** Token je Verbindung (Satz
„zufällig pro Broker-Lauf", `eq-ipc.schema.json:60`); Umbenennen oder Entfernen der v2-Pipe
(Varianten B, C; Plugin und Broker gemeinsam, Produktwirkung bei C); neue v3-Fehlercodes (der
vorhandene `unauthorized` reicht). Die eingefrorene Identität (NAK-30) berührt keine Etappe:
Class-IDs, Codes und Replace-V2 bleiben.

---

## 7. Nebenbefunde (außerhalb der drei IDs, zur Einordnung durch den Dirigenten)

- `docs/beweise/NAK-310.md:5` und `:33`: „Phase 16" statt Phase 10 (siehe Kopf).
- `broker/src/bin/eqcop-broker-probe.rs:21-23`, `broker/src/probe.rs:22-23`: v2-Probewerkzeug
  ohne Erlaubnisregel; kann die Produktions-Pipe belegen (`CLAUDE.md:191`, `:225-226`).
- `broker/src/transport/bootstrap.rs:8-9` („`protocol == 2` laeuft danach vollstaendig und
  unveraendert im v2-Parser weiter"): am v3-Endpunkt wird ein v2-Hello abgelehnt
  (`verbindung.rs:284-299`).
- Der Widerspruch Entwurf §53.9 ↔ Code (gemeinsamer Bootstrap gegen zwei Endpunkte, 3 c) gehört
  als Zeile nach `docs/offene-punkte.md`.

---

## 8. Nicht geprüft

- **Kein Lauf.** Alle Aussagen sind Quellherleitungen; Zeiten, Spawnzyklus, Sperrkonkurrenz und
  Speicher sind gerechnet, nicht gemessen.
- **Keine Belegung am Rechner geprüft.** Ob hier eine alte App oder ein anderer Prozess
  `evenacadia.eq-copilot.v1` hält, habe ich nicht untersucht; Prozesse und Pipes waren tabu.
- **FL-Brückenmodus.** Dass Peer-PID und `host.pid` auch bei einem in einem Brückenprozess
  geladenen Plugin übereinstimmen, folgt aus den Quellen (beide `_getpid()` bzw.
  `GetCurrentProcessId()` im ladenden Prozess), ist aber nicht gemessen.
- **serde und doppelte Schlüssel im Hello** nicht geprüft (gehört zu NAK-313).
- **C++-Reaktion** des `ControlClient` auf ein Schließen oder `unauthorized` im Bootstrap nicht
  gelesen.
- **Store** (`broker/src/store/**`) nicht gelesen; die Aussage zu persistierten Schlüsseln stützt
  sich auf `coordinator/mod.rs:457-462` und die Registerzeile NAK-152.
- **Teil A** (T3-12-01, T3-12-04) nicht geprüft; nur die Token-Schnittstelle benannt.
- **Windows-Interna** über die drei zitierten Microsoft-Learn-Seiten hinaus nicht untersucht.
- **Keine Dokumentänderung**, kein Dokuriegel (Skript); die Nebenbefunde in 7 sind nicht
  registriert, die veralteten Verweise in `docs/plugin-wissen.md` nicht berichtigt.
