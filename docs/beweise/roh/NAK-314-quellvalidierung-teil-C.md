# NAK-314 — Quellvalidierung Teil C (lesender Opus-Agent, HEAD 4ee4c407)

**Ticket:** NAK-314 · Planschritt S25m „Tiefenaudit 3 abarbeiten, Teil 6: Broker" ·
Etappe 0 „Quellvalidierung", **Teil C** (Klonkonflikt, Unknown-Rückweg, P1-Annahme am
Linkende; Karte U50)
**HEAD:** `4ee4c40752845d98fb451f550ddf8743079153be` (Zweig `master`)
**Datum:** 26.09.2026 (Start 15:22:11, letzte Messung vor dem Schreiben 15:45:49, beides
`date` in der Session) · **Modell:** Opus 5.5 (lesend)
**IDs (3):** T3-15-15 und T3-14-03 (W17), T3-16-03 (W25). Dazu Karte U50
(`docs/plan/fragen.json:41-51`) und die vier Regelfragen des Auftrags.
**Quelle der Befunde:** `docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` — Phasenzeilen
`:590` (T3-14-03), `:646` (T3-15-15), `:722` (T3-16-03); Duplikatzeile P15-T-03 `:659`;
Präzisierung T3-15-15 / F18 / F19 `:740`; Abschlusszeilen F15 `:762`, F18 `:765`, F19
`:766`; Arbeitsübersicht W17 `:820`, W25 `:828`. Rohberichte unter
`docs/audits/2026-09-15-tiefenaudit/roh/`: `phase-14-blindspots.md:21`, `:32`, `:61-73`;
`phase-15-paket3-teilfehler.md:196-230` (P15-T-03), `:280-318` (P15-T-05), `:326-333`
(OF-01 bis OF-08); `phase-15-gegenpruefung-paket3.md:183-244` (K3), `:322-416` (K5),
`:426`, `:428`; `phase-16-b-verbindungen.md:43-55` (B-02), `:67-68` (F18);
`phase-16-e-last-lebenslauf.md:57`, `:77-79`; `phase-16-f-gegenpruefung.md:11`;
`phase-16-abdeckung.md:23`, `:26`, `:27`; `phase-16-nachweise/e/b02-gegenprobe.md:1-31`;
ergänzend `phase-16-c-persistenz.md:50-60` (F19).
**Kartenentwurf des Audits:** Der Abschnitt „Produktfragen als Kartenentwürfe"
(`BEFUNDE.md:203-329`, PF11-01 bis PF11-17) enthält **keinen** Entwurf zum Klonkonflikt und
keinen zur P1-Annahme — alle drei IDs stammen aus den Phasen 14 bis 16, die nach der
Synthese (Phase 11) liefen. Der einzige Kartenentwurf zum Klon ist die Karte U50 selbst.
Ebenso nennt „Geordnete Abarbeitung ab 18.09.2026" (`BEFUNDE.md:108-162`, Bündel B01 bis
B36 in `:115-150`) keine der drei IDs; die Zuordnung steht nur in der Arbeitsübersicht (`:820`, `:828`).
**Gate:** Schritt S25m, `docs/plan/plan.json:382-387` (Feld `text` in `:385`);
Registerzeilen `docs/offene-punkte.md:317` (NAK-314), `:210` (NAK-188), `:516` (NAK-95,
geschlossen), dazu `:169` (NAK-138), `:162` (NAK-126), `:522` (NAK-92, geschlossen).
**Muster:** `docs/beweise/roh/NAK-313-quellvalidierung-teil-A.md` (Form, Tiefe, Kopf) und
`docs/beweise/roh/NAK-313-quellvalidierung-teil-C.md` (Trennung Technik und Produkt,
Kartenprüfung mit Kartenentwurf).

**Gate-Text wörtlich** (`docs/plan/plan.json:385`, Feld `text`):

> „ANGELEGT 18.09.2026 (Register NAK-314; Arbeitsübersicht Phase 16 W13, W14, W15, W16,
> W17, W25, W28). INHALT: 10 Befunde im Broker — T3-08-01 (Domain-Atomarität am
> 64er-Batchrand; NAK-157), T3-08-02 und T3-08-03 (Rücknahme vor Erstpersistenz nach
> Restore wieder gültig, Commit- und Publikationsordnung; NAK-269), T3-15-12 und T3-15-13
> (degradierter Store trennt Clients im Sekundentakt, transienter Lesefehler lässt Routing
> fail-closed ohne Anzeige), T3-16-02 (verspätetes Ende einer alten Telemetrieverbindung
> entkoppelt die angenommene Nachfolgerin), T3-15-15 und T3-14-03 (Klonkonflikt ohne Löser
> mit dauerhaften Sperren, Unknown nach reinem Sonde-Abbruch ohne Rückweg), T3-16-03
> (erfolgreich geschriebene P1-Evidenz geht beim gewöhnlichen Linkende verloren), T3-08-05
> (vollständiger Loop-Wrap invalidiert gültige Evidenz entgegen M-53). Produktentscheide als
> Karten: Klon-Identität und Sperrenende, Empfangs-ACK oder gezählter Verlust. GRENZE: ACK
> erst nach vollständigem Commit, Replay idempotent, keine Callbacks auf abgemeldeter
> Sitzung, Konfliktrestore fail-closed, keine Sicherheitsneubewertung (S25i). BEWEIS:
> docs/beweise/NAK-314.md mit Matrix, Rotbeweisen in A4, A4-SI (store_crash_matrix), A22,
> B8, vollem Kanon GRÜN und Laufzeit-Arm."

**Prüfform:** ausschließlich Quellenlektüre am HEAD. Kein Bau, kein cargo, kein Testlauf,
kein Kanon, kein Skript, kein FL Studio, kein Codex, keine Fehlerinjektion. Git nur lesend
(`rev-parse`, `status`, `log`, `show`, `diff --stat`, `diff`, `ls-files`, `grep`). Jede
Zeilennummer unten ist am HEAD `4ee4c407` selbst nachgelesen, keine aus dem Audit
übernommen; wo eine Auditzeile gewandert ist, steht die neue Zahl. Sicherheitsbefunde
(T3-12-\*) sind nicht Gegenstand, und dieser Bericht bewertet keine Sicherheitsregel neu;
wo ein Fix eine Regel mit Sicherheitshintergrund berührt (NAK-40 / C-07, H-14), steht das
als Hinweis zur Delegation da. Legacy-Bezeichner (`EQ-Copilot`, `EqCop*`, `Eqcp`) sind
kein Befund. Der Designteil ist geparkt (12.09.2026): unten steht keine Oberfläche, nur
Diagnosezustand, Vertragsbefehl und Rückweg.

**Arbeitsbaum, geprüft mit `git status --short`.** *Zu Beginn des Laufs* (15:22:11): die
zwei fremden untracked Ordner `briefing-hub/` und `nimbalyst-local/` sowie die drei
Auftragsdateien `docs/beweise/roh/NAK-314-quellvalidierung-teil-A-auftrag.txt`,
`…-teil-B-auftrag.txt` und `…-teil-C-auftrag.txt`. *Unmittelbar vor dem Schreiben*
(15:45:49): dasselbe plus eine neue fremde untracked Datei `docs/beweise/NAK-314.md`
(Manifest des Dirigenten, nicht von mir, nicht gelesen, nicht angefasst). HEAD beide Male
`4ee4c407`; der HEAD ist **nicht** gewandert, ein Basisabgleich war nicht nötig. Diese
Datei ist die einzige Schreibaktion des Agenten; `…-teil-A.md` und `…-teil-B.md` gehören
den Parallelagenten. *Nach dem Schreiben* (15:53:26, HEAD weiter `4ee4c407`) zeigt
`git status --short` zusätzlich zu dieser Datei `…-teil-A.md` und `…-teil-B.md` der
Parallelagenten, sonst unverändert.

**Stand des Audits gegen den HEAD.** Phase 14 und 15 maßen an `e008811e` (16./17.09.),
Phase 16 an `aff2d818` (18.09.). `git log aff2d818..HEAD -- broker/` zählt 21 Commits.
`git diff --stat aff2d818 HEAD` über die tragenden Brokerdateien dieses Teils:
`link.rs`, `flush.rs`, `mitgliedschaft.rs`, `zustand.rs`, `intervention.rs`, `mod.rs`,
`sicht.rs`, `broker/src/instance_alias.rs`, `broker/src/store/`,
`transport/server_v3/queues.rs` und `transport/server_v3/verbindung.rs` sind **unverändert**.
Geändert sind nur `befehl.rs` (+58), `evidenz.rs` (+147), `liveness.rs` (+38), `senke.rs`
(+185) und `subscription.rs` (+10), alle aus NAK-313 Etappe 4 und 5a (`28839892`,
`8fb8d46f`): strenger JSON-Lauf, P1-Weiche über den geprüften Wert, Ganzzahlhelfer
(`crate::vertrag::ganzzahl`). Am Heartbeat-Pfad liest `befehl.rs:692-696` die `sequence`
jetzt **vor** den Seiteneffekten; die Resync-Regel selbst (`:744-768`) ist unverändert.
Auf der C++-Seite hat NAK-312 den Konstruktor der Probeeq umgebaut (Besitzschleuse,
`89432468`, `553b70ae`); der Aufbau des ControlClients ohne Antwort- und Linkrückruf ist in
der Sache gleich (`git show aff2d818`: `SondeProcessor.cpp:139-140` gegen HEAD
`:198-230`). **Kein Commit seit dem Auditstand hat einen der drei Mechanismen geändert.**

---

## Ergebnis in einer Zeile je ID

| ID | Urteil | Kategorievorschlag | Schwere | Rotbeweis fällt an | Produktentscheid nötig |
|---|---|---|---|---|---|
| T3-15-15 (W17) | **PRÄZISIERT**: Mechanik trägt vollständig; die Sperre überlebt auch den Brokerneustart (nicht nur „bis zum Brokerneustart"), endet erst mit einer neuen Sitzungsepoche; jeder Verdrängungswechsel setzt zusätzlich sticky Unknown für die ganze Sitzung (Kopplung an T3-14-03); der 512er-Deckel sperrt jedes neue Hello, auch Gen; eine späte Meldung eines verdrängten Links mit **gleicher** Nonce setzt ebenfalls einen Riegel (hergeleitet) | DEFEKT | high (Audit: medium) | A4 `coordinator_model.rs` neben `:916` und `:1058` (deterministisch, ManualClock); Lebensdauer der Guards in A4-SI `store_crash_matrix.rs` neben `:4011`; Probeeq-Seite B31; Zusagezeilen `flush.rs:289-298` (Riegel), `befehl.rs:570-576` (confirm_join) | **ja**, Karte U50 — mit präzisiertem Text; der Zusatz „ohne Antwort Weg 1" ist nicht haltbar |
| T3-14-03 (W17) | **BESTÄTIGT**; dazu: ein Sonden-Neuaufbau kann die Sperre nach Quelle nicht lösen (die Gegenprüfung Phase 15 K3 irrt hier), die Probeeq trägt gar keinen Interventionsstrom, gesperrte Evidenz geht ungezählt für den Absender verloren, und die Sperre entsteht schon bei gewöhnlichem Probeeq-Reconnect (prepareToPlay, setStateInformation) | LÜCKE, Technikregel | high (Audit: medium), vorbehaltlich Messung | A4 `coordinator_model.rs` neben `:367` (Produktvorgang statt direktem `neutral_resync`); Zusagezeile `link.rs:588-593` | nein |
| T3-16-03 (W25) | **BESTÄTIGT**; dazu: SONDE-010 hat die at-least-once-Zustellung der Clienthälfte ausdrücklich an SONDE-011 verschoben, SONDE-011 hat sie für die Richtung Sonde→Broker nie aufgenommen; die Begründung von A-IN-04 setzt eine Senderwiederholung voraus, die es nach erfolgreichem Write nicht gibt; der Verwurf am Ingress ist ungezählt | LÜCKE, Technikregel | medium | A4 `tests_rueckstau.rs` neben `:118` (Empfänger, ACK erst nach Commit, Dedupe), B10 `IpcTestMain.cpp` neben `:4929` (Senderhaltung), A4-SI mit echtem C++-Client (Kette) | **nein** — die Register-Karte „Empfangs-ACK oder gezählter Verlust" ist keine echte Wahl |

---

## 1. T3-15-15 — Probeeq-Klon: gegenseitige Verdrängung, dauerhafte Sperre ohne Löser (W17)

*Befundzeile: `BEFUNDE.md:646` · Präzisierung Phase 16: `:740` · Abschlusszeilen F18 `:765`,
F19 `:766` · Arbeitsübersicht W17: `:820` · Rohbelege: `roh/phase-15-paket3-teilfehler.md:280-318`
(P15-T-05), `roh/phase-15-gegenpruefung-paket3.md:322-416` (K5), `roh/phase-16-b-verbindungen.md:67-68`,
`roh/phase-16-c-persistenz.md:50-60` · Register: NAK-314 `docs/offene-punkte.md:317`,
NAK-138 `:169` (Testlücke derselben Kette), NAK-126 `:162` (verdrängte Pipe bleibt eingabefähig)*

### a) Behauptung des Audits

Zwei Probeeq-Instanzen mit derselben `instance_id` (FL-Klon samt Zustand) verdrängen sich
dauerhaft gegenseitig, sind nach dem ersten Konflikt beide gesperrt, für die Probeeq gibt
es keinen produktiven Auflösungsweg, und Gen bietet einen Handgriff an, der positiv
quittiert und nichts löst (`BEFUNDE.md:646`); nach 512 beendeten Kollisionen verweigert der
flüchtige Aliasdeckel auch ein unbeteiligtes Hello, der Store trägt nach drei Sitzungen
2/4/6 Riegel (`:740`).

### b) Quellkette am HEAD

**Station 1 — gleiche Kennung, verschiedene Nonce.** Die Nonce entsteht je
Prozessorinstanz, `eq-copilot/plugin/sonde/SondeProcessor.cpp:187`
`v3RuntimeNonce (uuidHex32()),`; die Kennung kommt aus dem geladenen Zustand, `:1276`
`zustand = std::move (geladen);`, und geht unverändert ins Hello, `:1375`
`h.adresse.instanceId = zustand.common.instanceId.toStdString();`. Der Vertrag will das so:
`eq-copilot/schemas/state/nakama-state-v2.md:327` „Zwei Instanzen aus denselben Bytes
tragen dieselbe `instance_id` (Absicht: der State IST der Messpunkt) und verschiedene
`runtime_nonce`." Nach jedem Laden verbindet die Probeeq neu (`:1358-1359`
`controlV3.reconnect(); telemetryV3.reconnect();`).

**Station 2 — jedes Hello verdrängt, und löscht die Messwahrheit.**
`broker/src/coordinator/link.rs:131-143`:

```
        let geerbt = stand.clients.get(&key).cloned();
        if let Some(alt) = &geerbt {
            if alt.current_nonce != adresse.runtime_nonce {
                ...
                stand.messframes.remove(&key);
                stand.messfehler.remove(&key);
                stand.lautheit.remove(&key);
                ...
                stand.evidenz.remove(&key);
            }
```

und `:151-155`, `:179-180`:

```
                if alter_link != link_id {
                    let gleiche_nonce = alt.current_nonce == adresse.runtime_nonce;
                    if let Some(link) = stand.links.get_mut(alter_link) {
                        link.verdraengt = true;
                        link.trennen = true;
                    ...
                    Self::subscription_entfernen_locked(&mut stand, alter_link);
                    schliessen.push(alter_link.to_owned());
```

Der `ClientKey` trägt keine Nonce (`broker/src/coordinator/zustand.rs:10-15`:
`logon_sid`, `project_binding_id`, `session_epoch`, `instance_id`). Der verdrängte
C++-Client verbindet nach 500 ms neu, weil ein zuvor stehender Link den Backoff
zurücksetzt (`eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:451-452`
`if (stand) backoffMs = kBackoffStartMs;`, `IpcVerbindung.h:34` `kBackoffStartMs = 500`),
und verdrängt zurück.

**Station 3 — die Meldung des verdrängten Links erzeugt den Konflikt.**
`broker/src/coordinator/liveness.rs:337` `if link.verdraengt {`, `:351-355` registriert den
verdrängten Besitzer erneut, `:360-366`:

```
                let am_deckel = match kollisionsriegel_setzen_locked(
                    &mut stand,
                    registrierung,
                    &effective,
                    &ids,
                    true,
                ) {
```

`broker/src/instance_alias.rs:181-196` quarantänisiert beide Besitzer
(`stand.quarantaene.insert(erster_besitzer); stand.quarantaene.insert(besitzer);
return Registrierung::KollisionBeideQuarantaenisiert;`);
`broker/src/coordinator/zustand.rs:509-518` trägt je `derived_id` einen Riegel ein;
`liveness.rs:387-392` setzt `client.bestaetigt = false` und trennt den meldenden Link;
`:510` `self.guards_persistieren(guards);` schreibt sie in den Store
(`broker/src/store/writer.rs:752-788`). Die effektive Adresse des Riegels trägt **keine**
Nonce, `broker/src/coordinator/mod.rs:457-462`:

```
fn effektive_adresse(adresse: &Adresse) -> String {
    format!(
        "{}|{}|{}|{}",
        adresse.logon_sid, adresse.project_binding_id, adresse.session_epoch, adresse.instance_id
    )
}
```

**Station 4 — danach ist alles gesperrt, die Verdrängung läuft weiter.**
`broker/src/coordinator/flush.rs:289-298` (`dispatch_fuer_link_erlaubt_locked`: `!stand.guard_gesetzt(...)`
und `alias_register.dispatch_erlaubt(...)`), Auto-Join `mitgliedschaft.rs:17`
(`if stand.guard_gesetzt(&effective) || !stand.routing_bereit { return; }`). Ein
quarantänisierter Besitzer wird weiter angenommen: `instance_alias.rs:156-158`
(`if stand.quarantaene.contains(&besitzer) { return Registrierung::Ungueltig; }`) führt über
`zustand.rs:492-494` (`if !riegel_eintragen { return Deckelausgang::Frei(Vec::new()); }`)
zu Welcome und erneuter Verdrängung.

**Station 5 — eigene Ergänzung: jeder Verdrängungswechsel sperrt die Evidenz der ganzen
Sitzung.** Jeder geschlossene Link läuft über `broker/src/transport/server_v3/verbindung.rs:821`
`senke.control_schliesst(&link_id);` → `broker/src/coordinator/senke.rs:23-24`
`self.control_ende(link_id);` → `link.rs:588-593`:

```
                if hat_clients {
                    let taint = Self::taint_mut(&mut stand, &session);
                    taint
                        .interventionen
                        .retain(|_, intervention| intervention.link_id != link_id);
                    taint.unknown = true;
```

Bei einem Klon hat die Sitzung immer Clients (der verdrängende ist ja da). Damit setzt jede
Runde des Wechsels sticky Unknown — **alle** Sonden der Sitzung verlieren ihre starke
Evidenz, und nur ein neuer Gen-Link löst das (siehe T3-14-03). Das steht in keiner
Auditzeile zu T3-15-15.

**Station 6 — Gen sieht eine Beitrittsfrage statt eines Konflikts.** Der Snapshot bildet
ein Mitglied je `ClientKey` (`broker/src/coordinator/sicht.rs:519-525`, `.filter(|(key, _)| &key.session() == session)`
über `stand.clients`), die zwei Nonces fallen in **eine** Zeile; Gens Duplikaterkennung
`eq-copilot/plugin/src/SourcesModel.cpp:1333-1334`
(`if (! ids.insert (gm.instanceId).second) doppelteId = true;`) greift deshalb nie. Der
Riegel erreicht Gen nur als `beitritt_bestaetigung_noetig`, `mitgliedschaft.rs:238-243`:

```
        if clients.iter().any(|client| {
            (client.join_kandidat && !client.bestaetigt)
                || stand.guard_gesetzt(&effektive_adresse(&client.adresse))
        }) {
            return true;
        }
```

Gen zeigt daraus `SourcesModel.cpp:1468-1472` `Diagnose::confirmationRequired` und
`eq-copilot/plugin/src/PluginEditor.cpp:1525-1528`:

```
        if (sourcesAnzeige.diagnose == SourcesModel::Diagnose::confirmationRequired)
            diagnoseText += sourcesAnzeige.diagnoseHatHandgriff
                              ? " - choose a source and bind it"
                              : " - only the leading Main can resolve it";
```

**Station 7 — der Handgriff quittiert ohne Wirkung.** `broker/src/coordinator/befehl.rs:551-568`
findet das Ziel (der Client existiert, er ist nur gesperrt), `:570-576` ruft
`beitritt_bestaetigen_locked` (`mitgliedschaft.rs:119-131`, `client.bestaetigt = true;`),
`:617` antwortet `angewandt`. Der Riegel bleibt, `flush.rs:292` sperrt weiter, der nächste
Snapshot trägt weiter `beitritt_bestaetigung_noetig: true`.

**Station 8 — es gibt keinen Löser im Produkt.** `flush.rs:247-279`
(`konflikt_guard_aufloesen`) räumt Store-Riegel, Coordinator-Riegel und Quarantäne
gemeinsam, hat aber nur Testaufrufer: `git grep` über `broker/src` trifft nur die
Definitionen (`flush.rs:247`, `store/handle.rs:182`, `instance_alias.rs:302`, `:336`),
Aufrufer stehen nur in `broker/tests/store_crash_matrix.rs:2144`, `:2171`, `:4043`. Die
Probeeq hat keinen Editor (`eq-copilot/plugin/sonde/SondeProcessor.h:219-220`
`createEditor() override { return nullptr; }`, `hasEditor() … { return false; }`) und keine
Neu-ID-Operation; `neueSensorId` gibt es nur in Gen
(`eq-copilot/plugin/src/prozessor/State.cpp:54-67`). Das Brokersignal hat keinen Leser:
`befehl.rs:772-778` sendet `duplicate_instance_id`, die Probeeq baut ihren ControlClient
ohne Antwortrückruf (`SondeProcessor.cpp:204` `v3PipeName, {},` — drittes Argument
`beiAntwort` leer, Signatur `eq-copilot/plugin/core/ipc/ControlClient.h:356-363`), und
`git grep duplicate_instance_id` trifft in `eq-copilot/plugin` nur
`tests/V3TestServer.h:550`, `:572`, `:705`.

**Station 9 — die Sperre überlebt jedes Ende außer einer neuen Sitzungsepoche.**
Quarantäne: `instance_alias.rs:220-230` („Quarantaene dagegen bleibt bis zur expliziten
Neu-ID-Aufloesung bestehen; ein Disconnect darf den sichtbaren Konflikt nicht waschen";
`if stand.quarantaene.contains(&besitzer) { return; }`). Eviction entfernt den Client, nicht
den Riegel (`liveness.rs:99-209`; Test `coordinator_model.rs:1058-1096`). Brokerneustart:
die Quarantäne ist flüchtig und weg, der Riegel kommt vor jedem Routing zurück,
`mod.rs:316-320`:

```
        stand.routing_bereit = !store_writer.ist_degradiert();
        if stand.routing_bereit {
            for guard in store_writer.restaurierte_guards() {
                stand.guard_eintragen(&guard.effective_address.clone(), &guard.derived_id.clone());
```

Weil der Riegel die `session_epoch` enthält und diese nicht im State liegt
(`nakama-state-v2.md:331` „`runtime_nonce`, `session_epoch`, … (flüchtig, §32.1)"), endet er
praktisch erst, wenn Gen das Projekt neu öffnet. Existieren beide Klone weiter, kehrt der
Konflikt in der neuen Epoche zurück — mit zwei neuen Riegelzeilen.

**Station 10 — Zahlenränder.**
- **512er-Deckel:** `instance_alias.rs:103` `pub const MAX_QUARANTAENE: usize = 1024;`,
  jede Kollision belegt zwei Plätze (`:194-195`), der Deckel reserviert zwei
  (`:177-179` `if stand.quarantaene.len() + 2 > MAX_QUARANTAENE { return Registrierung::DeckelErreicht; }`).
  Nach 512 Kollisionen ist er voll. Die Vorabprüfung im Hello läuft für **jedes** Hello,
  bevor feststeht, ob überhaupt eine Kollision entsteht, `link.rs:126-129`:

  ```
        if self.alias_register.deckel_wuerde_reissen() {
            stand.cap_abweisungen = stand.cap_abweisungen.saturating_add(1);
            return ControlRegistrierung::abgewiesen("alias_quarantaene_deckel");
        }
  ```

  Auch `registriere_wire_zuordnung` prüft den Deckel vor der Kollisionserkennung
  (`:177` vor `:181`). Folge: ein unbeteiligtes Hello — auch das von Gen — wird bis zum
  Brokerneustart abgewiesen. Einträge fallen produktiv nie (Station 8).
- **Guards 2/4/6:** je Kollision zwei Zeilen (`instance_id:nonce` beider Besitzer,
  `link.rs:217-222` bzw. `liveness.rs:356-359`), je neuer Sitzungsepoche mit neuen Nonces
  zwei weitere. Gelöscht wird produktiv nie. `broker/src/store/mod.rs:62`
  `MAX_KONFLIKT_GUARDS: usize = crate::instance_alias::MAX_QUARANTAENE;` (1024): nach 512
  solchen Sitzungen weist `zustand.rs:505-508` jede neue Kollision mit
  `konfliktriegel_deckel` ab, und zwar über Brokerneustarts hinweg. Die Zeilen alter Epochen
  können nie mehr treffen; sie sind toter Bestand. Stimmen Store und Coordinator nicht
  überein, degradiert der Writer den ganzen Store (`writer.rs:769-775` mit `:372-376`).
- **Tombstone-Frist:** `mod.rs:107` `TOMBSTONE_MS: u64 = 10_000;` (`liveness.rs:252-255`,
  `>= TOMBSTONE_MS`). Sie räumt Client, Link und Aliasindex, nie Riegel oder Quarantäne.

**Station 11 — eigene Ergänzung, hergeleitet und nicht gemessen: ein Riegel ohne Klon.**
Seit H-10 verdrängt auch ein Hello mit **gleicher** Nonce den älteren Link (`link.rs:144-150`).
Verarbeitet der P0-Verbraucher des alten Links danach noch einen eingereihten Heartbeat
(Fenster zwischen dem neuen Hello und dem Schließen des alten Ingress), läuft
`liveness.rs:337-366` mit `riegel_eintragen = true` unabhängig vom Registrierungsergebnis;
`BereitsEingetragen` führt dort zu einem Riegel mit `ids = [besitzer, besitzer]`
(`zustand.rs:509-518` trägt ihn einmal ein). Eine einzelne Probeeq, die nach einem
Brokerstau neu verbindet, kann sich so selbst für die Sitzung sperren. C-10 spricht nur von
der **verdrängten Nonce** (`docs/beweise/SONDE-011.md:516`); zum gleichen Nonce-Fall schweigt
der Vertrag (so auch der Kommentar `link.rs:146-148`). Für Weg 1 der Karte ist das
entscheidend: eine automatische Neu-ID auf ein falsch-positives Konfliktsignal nähme einer
einzelnen Probeeq ihre Identität.

**Erreichbarkeit: Stufe 1.** Jede Kopie einer Probeeq samt Zustand erzeugt den Fall; F19
(`BEFUNDE.md:766`) hat den Stateklon am echten Prozessor gemessen, der FL-Presetweg ist
ungemessen.

### c) Zusage wörtlich

- `eq-copilot/schemas/state/nakama-state-v2.md:327` (§7 Duplicate): „Der Broker meldet die
  zweite lebende Verbindung (`heartbeat_ack{konflikt}`), der User löst sichtbar auf
  (`neueSensorId` ⇒ neue ID, Label/Paar bleiben, Host-Dirty)." (Auditstand `:269`.)
- `docs/FL-Nakama-Sonden-Design-Entwurf.md:1854` (§32.1): „`instance_id` | im
  Plugin-State persistent | stabiler Messpunkt; Duplikate werden sichtbar aufgelöst"
  (Auditstand `:1841`); `:1870-1872`: „Neue Sonden treten nur automatisch bei, wenn genau
  eine eindeutige Main-Sitzung im selben Host existiert. Bei Bridge, zwei offenen Projekten
  oder duplizierten IDs ist eine kurze sichtbare Bestätigung Pflicht."
- `docs/beweise/SONDE-011.md:513` (C-07): „Kollision quarantänisiert atomar beide Seiten
  und sperrt Dispatch, Telemetrie sowie Session-Push; Registrierungsreihenfolge ändert das
  Urteil nicht. … überlebt Eviction und Brokerneustart … Auflösung ausschließlich über
  explizite Neu-ID im Phase-A-Weg." Spalte: „keine zeitbasierte Freigabe".
- `docs/beweise/SONDE-011.md:516` (C-10): „Meldet die verdrängte Nonce danach erneut,
  entsteht der fail-closed-Duplikatkonflikt: beide Seiten sind bis zur expliziten Auflösung
  nach C-03/C-07 gesperrt." Gleichlautend L-03 `:624`.
- `docs/beweise/SONDE-011.md:524` (E-03): „Konfliktwahrheit bleibt clientunabhängig in
  `conflict_guards` und wird durch Eviction weder gelöscht noch routbar."
- `docs/beweise/NAK-121.md:228` (H-14): „Beide Speicher sind gedeckelt. Ist der Deckel
  erreicht, wird die Verbindung abgewiesen … Unveraendert bleibt C-07 im Kern: keine
  zeitbasierte Freigabe, Aufloesung ausschliesslich ueber die explizite Neu-ID". Ereignis
  dieser Zeile: „Weitere Kollisionen desselben Angreifers" — der Deckel ist eine
  Schutzmaßnahme, keine Lebensdauerregel.
- `eq-copilot/schemas/v3/eq-ipc-v3.schema.json:949-952` (`heartbeat_ack.duplicate_instance_id`,
  `$comment`): „Duplikate entstehen durch Kopieren einer Plugininstanz im Host. Sie werden
  SICHTBAR aufgeloest, nie heuristisch (§32.1)."
- `CLAUDE.md:44-45`: „jedes sichtbare Element führt einen Handgriff aus oder meldet ehrlich
  einen Zustand." (Auditstand `:51`.) Getroffen von Station 6 und 7.
- `docs/offene-punkte.md:169` (NAK-138): „Die Duplicate-Kette hat keinen Ende-zu-Ende-Test.
  … Regel: ein A4-SI-Test über die Probe-Pipe mit zwei lebenden Nonces derselben
  `instance_id`."

**Irreführende Kommentare und Texte, die der Befund berührt:**
`flush.rs:262-263` („die Aufloesung geschieht ausschliesslich ueber diese explizite
Neu-ID") — die Methode vergibt keine Neu-ID und hat keinen Produktaufrufer; der Test
`store_crash_matrix.rs:4011-4055` lässt nach ihr **dieselbe** ID wieder routen.
`instance_alias.rs:99-102` („damit er im gesunden Betrieb nie greift") — ohne produktives
Entfernen greift er nach 512 beendeten Kollisionen. `instance_alias.rs:220-223` verweist auf
eine „explizite Neu-ID-Aufloesung", die es im Produkt nicht gibt. `PluginEditor.cpp:1527`
(„ - choose a source and bind it") verspricht einen Handgriff, der nicht löst. Der
`$comment` `eq-ipc-v3.schema.json:950` („SICHTBAR aufgeloest") beschreibt keinen gebauten
Weg.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Was nicht |
|---|---|---|
| A4 `broker/tests/coordinator_model.rs:902-914` (`neue_nonce_verdraengt_alte_sofort`), `:916-925` (`verdraengte_nonce_report_sperrt_beide`) | eine Verdrängung; der Report des Verdrängten sperrt beide | kein Reconnect des Verdrängten, kein Wechsel, keine Auflösung, kein Snapshot, keine Main-Klasse `active_probe` (beide Tests mit `"main"`) |
| A4 `coordinator_model.rs:1058-1096` (`eviction_haelt_phase_a_riegel`, dreimal aufgerufen als `:1098`, `:1103`, `:1108`) | nach Eviction bleibt die alte ID gesperrt (`:1084-1086`), eine **neue** ID routet (`:1087-1095`) | schreibt den heutigen Zustand als gewollt fest: Auflösung nur über neue ID, ohne dass ein Produktweg sie erzeugt |
| A4-SI/A4 `store_crash_matrix.rs:4011-4055` (`guard_aufloesung_wirkt_auch_ohne_lebende_links`) | die Test-API räumt Riegel und Quarantäne gemeinsam | ruft `konflikt_guard_aufloesen` direkt; kein Produktaufrufer |
| `store_crash_matrix.rs:3671` (`quarantaene_deckel_weist_ab_statt_zu_wachsen`), `:3802` (`alias_deckel_weist_das_hello_ab_statt_ein_welcome_zu_senden`) | der Deckel weist ab, statt zu wachsen | schreibt fest, dass auch beendete Kollisionen den Deckel füllen; kein unbeteiligtes Hello als Gegenfall |
| `store_crash_matrix.rs:2968` (`nonce_verdraengung_cleanup_vor_altem_report`) | Cleanup vor altem Report | kein fortlaufender Wechsel |
| B2 `eq-copilot/plugin/tests/StateMigrationTestMain.cpp:3709-3729` (G10) | Gen: Duplikat hat gleiche ID und verschiedene Nonce, `neueSensorId` löst mit genau einem Host-Dirty | nur Gen; für die Probeeq gibt es kein Gegenstück |
| B13 `EqCopSonde012SourcesModelTest` | Gens Diagnosen allgemein | kein Fall „Riegel gesetzt" mit dem Text aus `PluginEditor.cpp:1527`; kein `confirm_join` auf ein gesperrtes Ziel |

Kein Test fährt den Wechsel zweier lebender Nonces, keiner die Sitzungssperre aus Station 5,
keiner ein unbeteiligtes Hello nach 512 beendeten Kollisionen, keiner den Fall aus
Station 11. NAK-138 (`:169`) ist unverändert offen.

### e) Urteil: PRÄZISIERT

Alle Glieder der Befundzeile tragen am HEAD: Verdrängung samt Löschen der Messwahrheit
(Station 2), Konflikt durch den Report des verdrängten Links (3), Sperre von Dispatch,
Push, Auto-Join und P2 (4), Beitrittsanzeige und wirkungsloses `angewandt` (6, 7), kein
Produktlöser, kein Leser des Duplikatflags, keine Neu-ID in der Probeeq (8). Kein Commit
seit `aff2d818` hat die Stellen verändert. Präzisiert wird:

1. **Reichweite in der Zeit.** „Bis zum Brokerneustart bleibende Sperre" (Folgebeweis in
   `:646`) ist zu kurz: der Neustart räumt nur die Quarantäne; der Riegel kommt vor dem
   Routing zurück (`mod.rs:316-320`) und sperrt, solange Gen dieselbe Epoche führt. Die
   Sperre endet erst mit dem Neuöffnen des Projekts und kehrt dort zurück, solange beide
   Klone existieren (so schon K5, `roh/phase-15-gegenpruefung-paket3.md:394-403`).
2. **Reichweite in der Sitzung.** Jeder Wechsel setzt sticky Unknown für die ganze Sitzung
   (Station 5); ein Klon legt damit die Evidenz **aller** Sonden still. Das koppelt
   T3-15-15 an T3-14-03.
3. **Der 512er-Deckel trifft jedes Hello** (`link.rs:126-129`), nicht nur beteiligte;
   betroffen ist auch Gen.
4. **Hergeleitete Nebenkante:** der späte Report eines verdrängten Links mit gleicher Nonce
   setzt ebenfalls einen Riegel (Station 11).

### f) Kategorie und Schwere

**DEFEKT** gegen `nakama-state-v2.md:327`, Entwurf `:1854`, C-07/C-10 (bis zur „expliziten
Auflösung" gesperrt, ohne dass eine Auflösung existiert) und `CLAUDE.md:44-45`
(Gens Handgriff quittiert ohne Wirkung). **Schwere high** (Audit: medium): ein gewöhnlicher
Handgriff in FL macht beide Sonden stumm und nicht fernsteuerbar, der fortlaufende Wechsel
löscht alle 0,5 bis 1 s die Messwahrheit der Kennung und sperrt die Evidenz der ganzen
Sitzung, Gen führt den User zu einem wirkungslosen Handgriff, und innerhalb der Sitzung
gibt es keinen Ausweg. Audio bleibt unberührt.

### g) Kleinster Fix

**Kartenunabhängig (vor der Antwort auf U50 baubar):**

1. **Ehrlicher Konfliktzustand.** Der Snapshot führt den Konflikt als Konflikt, nicht als
   Beitrittsfrage: ein optionales, nur positiv reisendes Merkmal je Mitglied (Muster
   `store_degraded`, `eq-ipc-v3.schema.json:1098-1102`). `session_mitglied` ist strikt
   (`:782` `additionalProperties: false`) — das ist eine neue Fassung
   (`wire_envelope_schema_minor`) mit Vertragstext, Rust- und C++-Leser, Fixtures
   (`tools/eq-copilot/erzeuge_v3_fixtures.py`, A5, A8, B3c). Gen zeigt daraus einen
   Diagnosezustand statt „choose a source and bind it"; `SourcesModel.cpp:1468-1472`,
   `PluginEditor.cpp:1525-1528`. Keine Oberfläche, nur Zustandswort.
2. **Kein positiver ACK ohne Wirkung.** `befehl.rs:551-576`: `confirm_join` auf ein Ziel mit
   gesetztem Riegel antwortet `abgelehnt` mit einem benennenden Code statt `angewandt`. Ein
   neuer `fehlercode` ist laut `eq-ipc-v3.schema.json:815` („Ein neuer Code ist eine
   Minor-Aenderung") ebenfalls ein Fassungsschritt; ohne neuen Code bleibt nur ein
   vorhandener (`unknown_target` ist falsch, `revision_conflict` ebenso) — deshalb neuer
   Code im selben Schritt wie 1.
3. **Wechsel begrenzen.** Ein Hello eines quarantänisierten Besitzers
   (`instance_alias.rs:156-158` → `Ungueltig`) verdrängt keinen lebenden Link mehr
   (`link.rs:144-192` erst nach dem Registrierungsurteil). Damit enden das Löschen der
   Messwahrheit und die Sitzungssperre aus Station 5; beide Links bleiben verbunden und
   gesperrt.
4. **Lebensdauer der Sperren** (Regelfrage 1, „Sperrenende"): die Vorabprüfung
   `link.rs:126-129` und der Deckel `instance_alias.rs:177-179` gelten nur für ein Hello,
   das tatsächlich eine Kollision einträgt; Quarantäneeinträge einer evictierten Nonce
   fallen mit der Eviction (der nonce-freie Riegel trägt die Konfliktwahrheit weiter, E-03
   bleibt wahr); die Auflösung räumt die Riegel **aller** Epochen derselben
   `logon_sid + project_binding_id + instance_id`. Diese Punkte berühren H-14 (Schutz gegen
   wiederholte Kollisionen eines Angreifers): ihre Sicherheitswirkung ist nicht Gegenstand
   dieses Tickets und vor dem Bau an einen Prüfer zu delegieren (`CLAUDE.md:121-123`).
5. **Riegel nur für eine echte zweite Nonce.** `liveness.rs:360-366` trägt einen Riegel nur,
   wenn `registrierung == KollisionBeideQuarantaenisiert` oder der gemeldete Besitzer vom
   aktuellen verschieden ist; ein später Heartbeat desselben Besitzers auf dem verdrängten
   Link bleibt ein getrennter Link ohne Riegel (Station 11).
6. **Auflösungsprimitive ohne Auslöserpolitik.** Probeeq: eine Neu-ID-Operation wie Gens
   `State.cpp:54-67` (neue `instance_id`, Label/Paar bleiben, genau ein Host-Dirty über
   `updateHostDisplay`, wie `SondeProcessor.cpp:1524-1525`, dann `reconnect`); dazu ein
   Leser des Konfliktsignals im ControlClient der Probeeq (heute `beiAntwort` leer,
   `SondeProcessor.cpp:204`). Broker: der produktive Aufruf von `konflikt_guard_aufloesen`
   (`flush.rs:247-279`) für die verbleibende Instanz, sobald die andere mit neuer ID
   verbunden ist. **Wer** die Neu-ID auslöst und **welche** Instanz sie bekommt, ist U50.

**Wartet auf U50:** die Auslöserpolitik (automatisch nach Reihenfolge = Weg 1, oder nach
Wahl des Users = Weg 2), der Signalweg Broker→Probeeq dafür (ein Flag für beide reicht
nicht, `befehl.rs:772` meldet beiden `true`) und die Vertragstexte
`nakama-state-v2.md:327`, Entwurf `:1854`, `:1870-1872`, C-07.

**Schema und State:** Die Kennung liegt schon im State (`common.instance_id`); eine neue
ID ist eine persistente Änderung mit Host-Dirty, keine neue State-Version
(`nakama-state-v2.md:323` nennt `neueSensorId` ausdrücklich). v3: Snapshotmerkmal, neuer
Fehlercode und jede neue Signalform an die Probeeq sind Fassungsschritte; Discriminator
(`session_command.command`) und Zieladresse sind nicht additiv (`CLAUDE.md:178-181`) —
ein dritter `command`-Wert wäre ebenfalls nur als neue Fassung zulässig.

**Mitzuprüfende Beziehungen:** Riegel setzen↔Riegel räumen (Store, Coordinator,
Quarantäne in einem Zug, auch ohne lebende Links); klonen↔laden (Klon mit neuer ID, danach
Speichern und Neuladen: zwei verschiedene IDs, kein Konflikt; ohne Speichern: Konflikt
kehrt zurück); verbinden↔trennen (Gegenfall Reload derselben Probeeq: alter Link tot,
neue Nonce, kein Riegel; Gegenfall gleiche Nonce, Station 11); sperren↔freigeben (die
Sitzungssperre aus Station 5 endet nach T3-14-03-Regel, nicht durch die Auflösung).

**Audio und Latenz:** kein Audiothread-Bezug (Neu-ID auf dem Nachrichtenthread,
Broker ohne Audio); die Begrenzung des Wechsels senkt die Pipe-Last.

### h) Ort des Rotbeweises

**Folgebeweis des Audits, deterministisch in A4** (`cargo test`, `tools/beweise.ps1:512-519`),
`broker/tests/coordinator_model.rs` neben `:916` und `:1058`, mit `ManualClock` ohne Pipes,
Klasse `active_probe` unter einem bestätigten Main mit erstem neutralem Heartbeat:
Hello X/a → Hello X/b → Report von Link a → `control_ende(a)` → Hello X/a → Hello X/b →
Snapshot lesen → `confirm_join` für X/b → `control_ende(b)` → Uhr auf `TOMBSTONE_MS - 1`
und `TOMBSTONE_MS` → Tick. **Heute** (aus der Quelle hergeleitet): Trennlisten wechseln bei
jedem Hello, `konfliktriegel_gesetzt` true, Snapshot `beitritt_bestaetigung_noetig: true`
ohne Konfliktmerkmal, `confirm_join` → `angewandt` und danach
`dispatch_fuer_link_erlaubt` false, `interventionssicht_fuer_link(main).unknown` true nach
jedem Linkende, Sperre über die Tombstone-Frist hinaus. **Nach dem kartenunabhängigen Fix:**
Konfliktmerkmal im Snapshot, `confirm_join` abgelehnt, kein zweites Verdrängen durch den
quarantänisierten Besitzer, keine Sitzungssperre durch den Wechsel.

**Zusagezeilen und Mutationen:** (i) `befehl.rs:570-576` — die neue Riegelprüfung vor
`beitritt_bestaetigen_locked` entfernen → `confirm_join` wird wieder `angewandt` → rot.
(ii) Die neue Verdrängungsbedingung in `link.rs:144-192` entfernen → der Wechsel und die
Sitzungssperre kehren zurück → rot. (iii) `link.rs:126-129` auf jedes Hello zurücksetzen →
der Gegenfall „unbeteiligtes Hello nach 512 beendeten Kollisionen wird angenommen" → rot.
(iv) `liveness.rs:365` wieder bedingungslos `true` → der Gleich-Nonce-Fall setzt einen Riegel
→ rot.

**Persistenz, nur mit Store:** A4-SI (`tools/beweise.ps1:521-528`, `store_crash_matrix`
`--ignored`) bzw. die nicht ignorierten Store-Fälle in A4, neben `:4011`: drei Sitzungen mit
Klon zählen heute 2/4/6 Zeilen; nach dem Fix räumt eine Auflösung die Zeilen aller Epochen;
Brokerneustart mit restauriertem Riegel bleibt fail-closed (E-06), der Aufruf der Auflösung
nach dem Neustart gibt frei.

**Probeeq-Seite:** B31 `EqCopSondeLebenslaufTest` (`tools/beweise.ps1:749`): zwei
`SondeProcessor` aus denselben Bytes, gleiche ID, verschiedene Nonce; die Neu-ID-Operation
vergibt 32 Hex, Label/Paar bleiben, genau ein Host-Dirty (Muster B2 G10
`StateMigrationTestMain.cpp:3709-3729`). Gens Anzeige: B13 mit einem Snapshot, der das
Konfliktmerkmal trägt.

**Nur als Integrationslauf:** der echte Wechsel zweier lebender C++-Clients mit 500-ms-Backoff
ist zeitabhängig und nicht deterministisch; er gehört in A22 (`pruefe_ipc_last.py`,
`tools/beweise.ps1:637`) oder einen A24-artigen Lauf mit echten `SondeProcessor`
(`:638`). B8 (`:779`) berührt den Weg nicht (Lebenslaufklassifikation und Brokerstart).

---

## 2. T3-14-03 — Probeeq endet, Gen bleibt: sticky Unknown ohne Rückweg (W17)

*Befundzeile: `BEFUNDE.md:590` · Phase-14-Urteil: `roh/phase-14-blindspots.md:21`, `:32`,
`:61-73` · verwandte Duplikatzeile P15-T-03: `BEFUNDE.md:659`, Gegenprüfung K3
`roh/phase-15-gegenpruefung-paket3.md:183-244` · Arbeitsübersicht W17: `:820` ·
Register: NAK-188 `docs/offene-punkte.md:210` (Platzhalterfall, offen)*

### a) Behauptung des Audits

Für einen Sonde-Abbruch bei weiter verbundenem Gen fehlt ein belegter produktiver Weg zur
Wiederfreigabe oder zum verständlichen Hinweis auf die sitzungsweite Evidenzsperre: jedes
Control-Ende setzt Unknown, normale Heartbeats und ein Sonden-Neuaufbau lösen nicht, der
Heartbeat-ACK meldet nichts, und der Join-Reconnect-Test entsperrt direkt über
`neutral_resync` (`BEFUNDE.md:590`).

### b) Quellkette am HEAD

**Station 1 — Eingang: jedes Linkende.** `verbindung.rs:810-821` (`ende.setzen();`,
`eingang.schliessen();`, `senke.control_schliesst(&link_id);`) → `senke.rs:23-24` →
`link.rs:499` `pub fn control_ende(&self, link_id: &str)`. Auslöser auf der Probeeq sind
nicht nur Abbruch und Entfernen: `SondeProcessor.cpp:428-429` (`prepareToPlay`) und
`:1358-1359` (`setStateInformation`) rufen `controlV3.reconnect()`, jeder Reconnect beendet
den alten Link.

**Station 2 — Wirkung: sticky Unknown für die Sitzung.** `link.rs:571-593`:

```
        // C-08 gilt fuer jeden Control-Disconnect, auch wenn der interne
        // Joinpfad den Reconnect angefordert hat. Nur neutral_resync loest.
        //
        // M-62: das trifft die SITZUNG dieses Links, nicht den ganzen Broker.
        match sterbende_session {
            Some(session) => {
                ...
                if hat_clients {
                    let taint = Self::taint_mut(&mut stand, &session);
                    ...
                    taint.unknown = true;
```

Der sterbende Client bleibt als Tombstone Mitglied (`:557-563`), `hat_clients` ist also
auch dann wahr, wenn nur Gen und die gehende Sonde da waren. Ohne auffindbare Sitzung
greift `:606` `None => Self::alle_sitzungen_unbekannt(&mut stand),` — der NAK-188-Fall
(`intervention.rs:88-101`).

**Station 3 — Sperre der Evidenz, auch fremder Sonden.** `evidenz.rs:189`
`if !Self::evidence_dispatch_locked(&mut stand, &session) {` → `intervention.rs:632-647`
(`let erlaubt = stand.taint.get(session).map_or(true, Taintstand::erlaubt);` plus
Platzhalter). Die Ablehnung zählt `evidence_gesperrt`; dieser Zähler hat im Produkt keinen
Leser (`git grep evidence_gesperrt -- broker/src` trifft nur Erhöhungen und die
Felddefinition `zustand.rs:313`). Die P1-Weiche verwirft das Ergebnis still,
`senke.rs:117` `let _ = self.evidence_snapshot_json_mit_minor_wert(link_id, wert);` — der
Absender erfährt nichts, die Evidenz ist für ihn verloren (dieselbe Klasse wie T3-16-03).

**Station 4 — der einzige Löser ist Gens neuer Link.** `befehl.rs:704-770`:

```
                let ist_erster_heartbeat = self.ersten_heartbeat_markieren(link_id);
                match wert
                    .get("intervention_state_unknown")
                    .and_then(Value::as_bool)
                {
                    Some(true) => { ... }
                    ...
                    Some(false) if ist_erster_heartbeat => {
                        let _ = self.resync_bestaetigen(link_id, 0);
                    }
                    ...
                    Some(false) if self.nachbericht_abschliessen_und_bestaetigen(link_id) => {}
                    _ => {}
                }
```

`resync_bestaetigen` (`link.rs:480-494`) → `neutral_resync` (`intervention.rs:401-434`),
der auch den Platzhalter löst (`:432`). Das `false` reist nur im ersten Heartbeat nach
einem bestätigt neutralen Aufbau, `eq-copilot/plugin/core/ipc/controlclient/Vertrag.cpp:185-189`:

```
         + (status.interventionStateUnknown
                ? std::string (",\"intervention_state_unknown\":true")
                : (bestaetigtNeutral
                       ? std::string (",\"intervention_state_unknown\":false")
                       : std::string()))
```

Normale Heartbeats lassen das Feld weg (dreiwertige Regel, `ControlClient.h:171-186`), ein
späteres `false` löst nie (`befehl.rs:729-733`).

**Station 5 — nur Gen darf Neutralität erklären; die Probeeq nie.**
`eq-copilot/plugin/src/prozessor/Ipc.cpp:914-915`:

```
    const bool darfAufbauUrteilMelden =
        hVoraus.pluginKind == "main" && nakama::ipc::adresseGueltig (hVoraus.adresse);
```

und `:991-997` (ohne Recht nur `telemetryV3.reconnect(); return;`, sonst
`controlV3.meldeAufbauUrteil (neutral)`), Neutralität aus `:956-959` (Ring leer, kein
offenes Begin, kein offenes End, kein hörbarer Marker). `SondeProcessor.cpp` ruft
`meldeAufbauUrteil` nicht (`git grep` in `eq-copilot/plugin` außerhalb der Tests: nur
`Ipc.cpp:457`, `:997`); ihr Statusrückruf liefert `interventionStateUnknown` in der Vorgabe
`false`, und sie hat weder Interventionsring noch Begin/End (`git grep audible_intervention`
im Produktcode trifft nur `src/prozessor/Ipc.cpp:87`). **Ein Sonden-Neuaufbau kann die
Sperre deshalb nicht lösen.** Die Gegenprüfung Phase 15 (K3,
`roh/phase-15-gegenpruefung-paket3.md:215-216`: „auch der erste neutrale Heartbeat einer neu
verbundenen Probeeq entsperrt") irrt für das Produkt; richtig ist nur, dass der Broker ein
solches `false` von jeder Klasse annähme (`befehl.rs:744` prüft keine Klasse).

**Station 6 — kein Hinweis.** Heartbeat-ACK: `befehl.rs:772-778` trägt nur `sequence` und
`duplicate_instance_id`. Snapshot: `session_snapshot` hat kein Feld dafür
(`eq-ipc-v3.schema.json:1070-1117`; Aufbau `sicht.rs:576-585`). Gen: `SourcesModel.cpp:1468-1470`
kennt nur `storeDegraded`, `confirmationRequired`, `keine`. Gens eigener Reconnect ist
möglich (`Ipc.cpp:1046`, `:1122`; `Hostbruecke.cpp:162` in `prepareToPlay`;
`State.cpp:65`, `:199`, `:227`), wird aber von keinem fremden Sondenende ausgelöst.

**Zahlenränder:** keine Frist — die Sperre steht, bis Gen neu verbindet; `TOMBSTONE_MS`
(10 000 ms) räumt den Client der gegangenen Sonde, nicht das Unknown (C-08, E-05); nur
wenn danach **kein** Client der Sitzung mehr existiert und der Taint sauber ist, fällt der
Eintrag (`liveness.rs:183-191`) — mit lebendem Gen nie.

**Erreichbarkeit: Stufe 1**, und breiter als „reiner Sonde-Abbruch": jedes Entfernen einer
Probeeq, jedes Laden eines Zustands in eine Probeeq und jedes `prepareToPlay` einer Probeeq,
das nach Gens erstem neutralem Heartbeat verarbeitet wird, sperrt die Sitzung. Ob das beim
Projektladen in FL regelmäßig passiert, hängt an der Aufrufreihenfolge des Hosts und ist
nicht gemessen (siehe „Nicht geprüft").

### c) Zusage wörtlich

- `docs/beweise/SONDE-011.md:514` (C-08): „Overflow/Lücke/Disconnect setzen sticky
  unknown; End, v2-`false`, Sessionende, stale oder Eviction löschen es nicht. Nur
  bestätigter Neutral-/Sequenz-Resync entsperrt; Überlappung und Tail laufen vollständig
  aus."
- `docs/beweise/SONDE-011.md:202` (10-1): „… ausschließlich ein **expliziter
  Neutral-/Sequenz-Resync** mit bestätigter neuer Sequenzbasis darf das Sticky-Bit löschen.
  Ein verlorenes Begin oder End kann so nie eine saubere Baseline vortäuschen." Spalte
  Wire: „Der explizite Resync ist eine Coordinator-Lebenszyklusoperation und darf nicht aus
  einem normalen End/`false` inferiert werden; diese Matrix erfindet dafür keine neue
  Familie. Eine spätere neue Wirefamilie oder ein neues striktes Objektfeld wäre
  versionierungspflichtig."
- `docs/beweise/SONDE-013.md:524` (M-39): „Overflow, Control-Disconnect oder Sequenzlücke
  setzen sticky `intervention_state_unknown`. … starke Evidenz bleibt bis zu einem
  expliziten Neutral-/Sequenz-Resync gesperrt."
- `docs/beweise/SONDE-013.md:561` (M-61): „Nur `neutral_resync` löst es. … eine
  ausdrückliche Coordinator-Lebenszyklusoperation, keine neue Wirefamilie und kein normales
  End."
- `docs/beweise/SONDE-013.md:562` (M-62): „Die Taint-Logik ist **sitzungsweit**, nicht
  instanzlokal. … Bei unbekanntem Routing ist der Umfang die ganze Sitzung."
- Entwurf §34.2, `docs/FL-Nakama-Sonden-Design-Entwurf.md:2249-2254`: „Dieser Ereignisring
  verwendet für P0 **kein** `drop-oldest`. Overflow, Control-Disconnect oder eine Lücke in
  der Eventsequenz setzt ein sticky `intervention_state_unknown`. … Ein verlorenes Begin
  **oder** End darf damit niemals eine scheinbar saubere Baseline erzeugen."
- `docs/beweise/NAK-180.md:306` (N-11): „Instanz ist **nicht** als `main` klassifiziert
  (Sonden-/Probe-Rolle) | Control-Link kommt hoch | Es reist **keine
  Neutralitätsbehauptung**".
- `docs/beweise/SONDE-011.md:1210` (Nacharbeit R3 Nr. 2): „`control_ende` räumt einen
  internen Join-Reconnect-Marker weiterhin auf, setzt danach aber ausnahmslos
  `intervention_state_unknown`".
- `docs/beweise/SONDE-013.md:1041`: „Ein gesetzter Zähler ohne Leser ist dasselbe wie kein
  Zähler." (M-63 `:563`: „Der Zähler `evidence_gesperrt` wird ausgewertet, nicht
  ignoriert.")

**Die Quellen schweigen** zur Wiederfreigabe bei unverändert stehendem Gen-Link und zu
einem Hinweis an Gen. Sie legen aber fest, was eine Lösung nicht darf: kein normaler
Heartbeat, kein End, keine Eviction, keine Sondenaussage, keine neue Wireform ohne
Versionierung.

**Irreführende Texte:** `docs/beweise/SONDE-013.md:1038` führt M-39/M-61 unter „Puffer mit
Abflussweg ohne Reconnect" („`neutral_resync` ist eine Coordinator-Lebenszyklusoperation,
kein Reconnect") — sein einziger Produktauslöser **ist** Gens Link-Neuaufbau. Register
NAK-188 (`docs/offene-punkte.md:210`, „Löst sich mit NAK-180, weil `neutral_resync` den
Platzhalter mitnimmt (`intervention.rs:398`)") — die Zeile steht heute bei `:432`, und die
Lösung setzt einen Gen-Neuaufbau voraus (so auch `BEFUNDE.md:659`).

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Was nicht |
|---|---|---|
| A4 `coordinator_model.rs:367-384` (`join_reconnect_disconnect_setzt_sticky_unknown_bis_resync`) | Sondenende setzt Unknown (`:378-380`), v2-`false` löst nicht | entsperrt direkt mit `c.neutral_resync("main", 0)` (`:383`) — kein Produktvorgang; schreibt fest, dass ein Sondenende die Sitzung sperrt |
| A4 `broker/tests/sonde013_verdrahtung.rs:4692` (`erster_heartbeat_mit_false_loest_das_sticky_unknown`) | R1-Weg über die Byteinstanz | nur Main, kein vorausgehendes Sondenende |
| A4 `sonde013_verdrahtung.rs:4733` (`spaeteres_false_loest_das_sticky_unknown_nicht`) | späteres `false` löst nicht | — (gewollt) |
| A4 `sonde013_verdrahtung.rs:5209` (`sondenhello_aendert_den_taint_nicht`) | ein Sonden-Hello ändert nichts | ohne vorausgehendes Sondenende bei stehendem Gen |
| B23 `eq-copilot/plugin/tests/Sonde013PassageStateTest.cpp:1614`, `:1892` (N-11) | eine Nicht-Main-Instanz behauptet nichts | Wirkung auf den Broker |
| A24/A22 | Last und Soak mit echten Clients | keine Messung von `evidence_gesperrt` nach einem Sondenende |

Kein Test fährt „Gen und zwei Sonden, nur Sonde A endet, Sonde B sendet weiter" mit
Produktvorgängen.

### e) Urteil: BESTÄTIGT

Alle Sätze der Befundzeile tragen am HEAD (Stationen 1 bis 6); kein Commit seit dem
Auditstand hat `link.rs` oder `intervention.rs` verändert, `befehl.rs` nur die
Lesereihenfolge der `sequence`. Ergänzt:

1. Die Probeeq kann die Sperre nach Quelle **nie** lösen (N-11, kein
   `meldeAufbauUrteil`); die gegenteilige Aussage der Gegenprüfung Phase 15 K3 trifft nicht zu.
2. Die Probeeq ist keine Interventionsquelle: ihr Linkende kann kein Begin und kein End
   verlieren — genau das, wogegen §34.2 die Sperre setzt. Die Sperre entsteht dennoch
   ausnahmslos (so gewollt seit SONDE-011 R3 Nr. 2).
3. Gesperrte Evidenz ist für den Absender still verloren; der Sperrzähler hat keinen
   Produktleser.
4. Auslöser sind auch `prepareToPlay` und `setStateInformation` der Probeeq, nicht nur ihr
   Abbruch.
5. Die Variante „alter Gen-Link endet nach dem neutralen ersten Heartbeat des neuen"
   (`roh/phase-14-blindspots.md:73`) trägt am HEAD ebenso: `control_ende` des alten Links
   setzt `unknown` erneut, und der neue Link hat seinen ersten Heartbeat schon verbraucht.

### f) Kategorie und Schwere

**LÜCKE, Technikregel.** Die Sperre selbst ist zugesagt (C-08, M-39, M-61, M-62); zu ihrem
Ende bei stehendem Gen schweigen die Quellen. Der User hat hier keine Wahl: ob eine Sitzung
nach einem Sondenende wieder messen darf, entscheidet allein die Frage, ob eine Intervention
verloren sein kann — und die beantwortet Gen selbst (Station 5). Ein Hinweis ist ein
Diagnosezustand, keine Oberfläche. **Schwere high** (Audit: medium), vorbehaltlich der
Messung: gewöhnliche Vorgänge an einer einzigen Probeeq legen still die Evidenz aller Sonden
der Sitzung still, ohne Frist und ohne Anzeige; Audio bleibt unberührt. Sollte die Messung
zeigen, dass beim Projektladen Gens Neuaufbau zuverlässig zuletzt kommt, bleibt medium.

### g) Kleinster Fix

**Regel (Technik, vom Dirigenten zu setzen), zwei Teile:**

1. **Ein Ende sperrt nur, was es verlieren kann.** `control_ende` (`link.rs:575-607`) setzt
   sticky Unknown nur, wenn der sterbende Link eine Interventionsquelle war (Klasse `main`,
   oder ein Link, der je eine Ereignissequenz gemeldet hat, `LinkStand.letzte_event_sequence`),
   **und** nur, wenn kein Nachfolger derselben Instanz und Nonce seit dem Aufbau des
   sterbenden Links schon einen bestätigt neutralen Aufbau gemeldet hat (sonst hat dessen
   Neutralitätsurteil den alten Link mit abgedeckt, `Ipc.cpp:956-959`). Ein Probeeq-Ende
   ohne Ereignisstrom sperrt nicht. Das ändert C-08 (`SONDE-011.md:514`), 10-1 (`:202`),
   M-39 (`SONDE-013.md:524`) und die Entscheidung R3 Nr. 2 (`SONDE-011.md:1210`, „ausnahmslos")
   im Text; die Absicht von §34.2 bleibt, solange die Probeeq keine Marker erzeugt (NAK-180
   `:77`, §7.1 E-08). Wird die Probeeq je Interventionsquelle, fällt die Ausnahme automatisch,
   weil sie am Ereignisstrom hängt. Der ungebundene Platzhalterfall (`link.rs:614-616`,
   NAK-188) bleibt fail-closed.
2. **Wo die Sperre bleibt, wird sie sichtbar und von Gen selbst gelöst.** Der Snapshot trägt
   ein optionales, nur positiv reisendes Merkmal „Interventionszustand unbekannt"
   (Fassungsschritt wie `store_degraded`, Rust- und C++-Leser, Fixtures). Gen reagiert
   **ohne Userhandgriff** mit seinem eigenen kontrollierten Neuaufbau (`controlV3.reconnect()`),
   aber nur, wenn sein Neutralitätsprädikat (`Ipc.cpp:956-959`) gilt; sonst zeigt es den
   Diagnosezustand „Analyse angehalten, Hörzustand unbekannt". Eine neue ausdrückliche
   Neutralitätserklärung auf dem stehenden Link wäre die Alternative, ist aber eine neue
   Wireform (10-1: versionierungspflichtig) und nicht kleiner.

Normale Heartbeats geben weiter nie frei (M-61 bleibt wörtlich). Ein erzwungener
Gen-Reconnect durch den Broker (Link auf `trennen`) ist technisch möglich, aber wegen der
Wechselwirkung mit T3-15-14 (Replay vor dem ersten Heartbeat) und T3-16-02
(Telemetrienachfolge) nicht der kleinste Weg.

**Dateien:** `link.rs:499-617` (`control_ende`), `befehl.rs:704-778` (unverändert in der
Regel), `sicht.rs:576-585`, `eq-ipc-v3.schema.json:1070-1117`, `SourcesModel.cpp:1468-1472`,
`Ipc.cpp:851-1000` (Gens Reaktion). Kein State-Bezug.

**Mitzuprüfende Beziehungen:** sperren↔freigeben (Main-Linkende sperrt weiter; Sondenende
ohne Ereignisstrom sperrt nicht; offene Intervention oder Nachlauf bleibt gesperrt, M-58);
verbinden↔trennen in beiden Reihenfolgen (alter Gen-Link endet vor/nach dem neutralen
ersten Heartbeat des neuen); der Platzhalterfall NAK-188.

**Audio und Latenz:** kein Audiothread-Bezug; Gens Reaktion läuft auf dem Clientthread.

### h) Ort des Rotbeweises

**A4, deterministisch**, `coordinator_model.rs` neben `:367`: Main bestätigt über den
**Produktvorgang** (erster Heartbeat mit `false` über `Senke::p0`, Muster
`sonde013_verdrahtung.rs:4692`) statt `c.neutral_resync`; Sonden A und B bestätigt; nur A
`control_ende`; danach Evidenz von B. **Heute rot** gegen die Regel: `evidence_gesperrt`
steigt, `unknown` true. Gegenfälle: Main-Linkende sperrt; offene Intervention bleibt
gesperrt; alter Gen-Link endet nach dem neutralen ersten Heartbeat des neuen → nach der
Regel kein erneutes Unknown. **Zusagezeile:** `link.rs:588-593`; eine Mutation, die das
`taint.unknown = true` wieder bedingungslos setzt, muss den Fall „Sonde A endet" reißen.

**Echte Clients:** der Folgebeweis aus `roh/phase-14-blindspots.md:73` (Gen + A + B über die
Probe-Pipe) gehört in A24 (`pruefe_session_soak.py`, echte `SondeProcessor` und echter
Coordinator) oder A22; B23 hält N-11. Gens Reaktion auf das Snapshotmerkmal: B13
(`EqCopSonde012SourcesModelTest`) für die Diagnose, B10 für den Reconnect mit neutralem
ersten Heartbeat.

---

## 3. T3-16-03 — geschriebene P1-Evidenz geht beim gewöhnlichen Linkende verloren (W25)

*Befundzeile: `BEFUNDE.md:722` · Abschlusszeile F15: `:762` · Arbeitsübersicht W25: `:828` ·
Rohbelege: `roh/phase-16-b-verbindungen.md:43-55` (B-02), `roh/phase-16-nachweise/e/b02-gegenprobe.md:1-31`,
`roh/phase-16-f-gegenpruefung.md:11`, `roh/phase-16-abdeckung.md:23`, Vorläufer OF-03
`roh/phase-15-paket3-teilfehler.md:328`*

### a) Behauptung des Audits

Bereits erfolgreich geschriebene, nicht koaleszierbare P1-Evidenz kann beim gewöhnlichen
Linkende ungesehen verloren gehen: der Sender hält nur fehlgeschlagene Writes zur
Wiederholung, das Schließen der Brokerqueue verwirft eingelesene Einträge; es fehlt eine
Regel für bestätigte Annahme oder gezählten Verlust (`BEFUNDE.md:722`; gemessen: vier
Belege 1000 bis 1003, einer angenommen, drei eingelesen und verworfen, Highwater 3,
Overflow 0).

### b) Quellkette am HEAD

**Station 1 — Producerentnahme in der Probeeq.** `SondeProcessor.cpp:1081-1089`:

```
        // Leerer Koaleszenzschluessel: zwei Snapshots derselben Quelle sind
        // ZWEI Belege mit eigener evidence_id, nicht zweimal derselbe Blick
        // auf ein Objekt. ...
        const auto ergebnis = controlV3.sendeP1 ({}, json);
        ...
        uebergeben = nakama::ipc::p1Uebergeben (ergebnis);
```

`p1Uebergeben` heißt „lokal eingereiht", `eq-copilot/plugin/core/ipc/IpcQueues.h:309-314`
(`eingereiht`, `koalesziert`, `zurWiederholung`). Danach werden die im Beleg enthaltenen
Ereignisse entnommen, `SondeProcessor.cpp:1127-1131`
(`merkmale.ereignisseEntnommen(); letzteEreignisverluste = verworfen;`). Die Probeeq behält
keine Kopie; der nächste Snapshot ist ein neues Fenster mit neuer `evidence_id`
(`:1033` `kopf.evidenceId = uuidHex32();`).

**Station 2 — Writerreservierung im ControlClient.** `Verbindung.cpp:901-904` entnimmt
(`etwasGesendet = istP0 || p1.entnehmen (schluessel, nachricht);`, Platz reserviert, A-P1-08);
scheitert der Write, legt `:952` zurück (`p1.zuruecklegen (schluessel, std::move (nachricht));`,
A-P1-09); gelingt er, gibt `:1012` frei (`p1.bestaetigen();`). Der Kommentar `:982-983` sagt
den Umfang ehrlich: „erst JETZT gibt die Queue den reservierten Platz frei (§53.9 "nichts
verwerfen" gilt bis zum Write-Commit)". Für P1 gibt es kein In-Flight-Register; das gibt es
nur für persistenzpflichtige P0 (`:1024-1033` `inFlightNachWireWrite`).

**Station 3 — Brokerannahme.** Der Leser reiht in den Ingress (Cap 256,
`broker/src/transport/warteschlange.rs:24`), ein eigener Verbraucher entnimmt,
`verbindung.rs:616-625`:

```
    let verbraucher_rest = {
        ...
                while let Some((familie, schema_minor, payload)) = eingang.entnehmen_ohne_p0() {
                    match familie {
                        Familie::P1 => senke.p1_mit_minor(&link, schema_minor, &payload),
```

Fachlich angenommen ist ein Beleg erst in `evidenz.rs:153-199` (Sperrprüfung `:189`,
Einfügen `:191-198`) und nach gelungener Ablage (`:221-236` nimmt ihn sonst zurück;
`evidenz_persistieren` `:445-485`).

**Station 4 — Schließen verwirft den Rest, ungezählt.** `verbindung.rs:810-814`
(`ende.setzen(); … eingang.schliessen();`), `queues.rs:55-58`:

```
    pub(super) fn entnehmen_nach(&self, p0: bool) -> Option<(Familie, u8, Vec<u8>)> {
        let mut g = self.inhalt.lock().unwrap_or_else(|x| x.into_inner());
        loop {
            if g.1 {
                return None;
```

Der Kommentar `queues.rs:37-41` begründet das: „Vorher lief der Verbraucher nach dem
Schliessen noch durch den Restbestand — und rief P0/P1 fuer eine Verbindung, deren
Kopplung schon abgemeldet war. Die Frames gehoeren zu einer Sitzung, die es nicht mehr gibt;
sie fallen mit ihr". Einen Zähler für die so verworfenen Einträge gibt es nicht (`git grep`
über `queues.rs` und `warteschlange.rs`: nur `nachzuegler_verworfen` am Ausgang und
`p2_verworfen`).

**Station 5 — Reconnect holt nichts zurück.** Der Client wiederholt nur, was im
Wiederholpuffer lag (`Verbindung.cpp:746` `p1.nachReconnectWiederholen();`, A-P1-11); der
Broker kennt keinen Rückruf an den Absender (die P1-Weiche verwirft jedes Ergebnis,
`senke.rs:117`).

**Station 6 — heute gäbe es keine Deduplikation.** Käme ein Beleg zweimal, hängte
`evidenz.rs:191-198` ihn zweimal an die Historie (Retention `EVIDENZ_RETENTION = 32`,
`:39`), `evidence_angenommen` zählte doppelt, und `evidenz_persistieren` schriebe ein
zweites `event_log`-Ereignis; nur die Projektion wäre über den Primärschlüssel idempotent
(`broker/src/store/migration.rs:77-78` `evidence_id TEXT PRIMARY KEY`,
`writer.rs:586-592` `ON CONFLICT(…) DO UPDATE … WHERE excluded.last_event_ord > …`).

**Zahlenränder:** Ingress-Cap 256 (Überlauf ohne P2 trennt, A-IN-04), Client-P1 128 +
Wiederholpuffer 128 (`SONDE-010.md:114`), Producerschwelle `kEvidenzP1Schwelle = 4`
(`SondeProcessor.h:717`; über 4 wartenden Einträgen entsteht gar kein Beleg,
`SondeProcessor.cpp:993-1013`); Wiederholungsgrenze für geschriebene Belege: keine;
Deduplikationsfenster: keines. Der Auditlauf (Highwater 3, Overflow 0, IDs 1000 bis 1004)
ist mit diesen Stellen vereinbar; nachgerechnet habe ich ihn nicht.

**Erreichbarkeit: Stufe 1** bei jedem Linkende mit gefülltem Ingress — Reconnect aus
`prepareToPlay`/`setStateInformation` (`SondeProcessor.cpp:428-429`, `:1358-1359`),
Verdrängung (T3-15-15), Brokerstau.

### c) Zusage wörtlich

- Entwurf §53.9, `docs/FL-Nakama-Sonden-Design-Entwurf.md:4162`: „| P1 Zustand/Evidenz |
  128 | Snapshots nach Objektschlüssel koaleszieren; nicht koaleszierbare Events bei Überlauf
  über Reconnect/Outbox wiederholen |" (Auditstand `:4132`).
- Entwurf §53.9, `:4195-4198`: „Ein Killtest an jeder Outboxgrenze beweist
  at-least-once-Zustellung mit exactly-once-Wirkung: Entweder wird das Ereignis erneut
  gesendet oder als bestätigt rekonstruiert, nie still verloren oder wegen einer
  Wiederholung doppelt angewandt."
- `docs/beweise/SONDE-010.md:114`: „der Wiederholpuffer 128 ist **(Festlegung Worker)** —
  die Clienthälfte des Outboxgedankens"; `:273-275`: „Der Wiederholpuffer hier ist die
  Clienthälfte des Outboxgedankens; die Outbox selbst und die at-least-once-Zustellung mit
  dem Killtest an jeder Outboxgrenze (§53.9) gehören dorthin" (nach SONDE-011).
- `docs/beweise/SONDE-010.md:159` (A-P1-09): „reserviert, Ereignis | Write scheitert oder
  Verbindungsverlust | zurück an seine Position (vorn)". `:158` (A-P1-08): „der Platz bleibt
  reserviert (`inFlug`)".
- `docs/beweise/SONDE-010.md:191` (A-IN-04): „die Verbindung wird GETRENNT wie bei P0; NIE
  fällt eine P1-Nachricht still, während die Verbindung gesund wirkt — der Sender könnte sie
  sonst nicht wiederholen (§53.9 P1 „über Reconnect/Outbox wiederholen") … die
  Ende-zu-Ende-Wiederholung NACH der Trennung ist Outbox und Coordinator (`SONDE-011`) und
  steht unter „bewusst nicht festgelegt"".
- `docs/beweise/SONDE-011.md:561` (O-02): absoluter `session_snapshot` an Subscriber, „Ein
  Pipe-Write bleibt keine Zustell-/Wirkungswahrheit." `:563` (O-04): „Nicht koaleszierbare
  Events bleiben interne Store-Wahrheit: In Phase B besitzen sie keinen Produkt-Consumer und
  keine Wire-Zustellzusage" — beides gilt der Richtung Broker→Main, nicht Sonde→Broker.
- Register NAK-95 (`docs/offene-punkte.md:516`, geschlossen): (1) Abfluss des
  Wiederholpuffers, (2) Schlüssel im Wiederholpuffer, (3) „die Rust-P1-Queue trägt noch den
  in C++ geschlossenen Fehler (`pop_front` verdrängt Akzeptiertes …)"; Konvergenzentscheid
  „(3) … DEFEKT (§53.9 … Wiederholen statt Verwerfen)". Keiner betrifft einen geschriebenen,
  nicht angenommenen Beleg.
- Register NAK-92 (`docs/offene-punkte.md:522`, geschlossen), Nr. 4: „`entnehmen()` liefert
  nach dem Schließen zuerst noch Einträge … abgelöste Verbraucher rufen P0/P1-Callbacks nach
  der Abmeldung" — Grund des heutigen Verwerfens.
- `tools/dirigent/pruefliste.md:14-17` (Abschnitt A): „nie stillschweigend Akzeptiertes
  löschen" und „Ein Puffer, der „später wiederholt", hat einen Abflussweg **ohne**
  Reconnect"; `:36-37` (Abschnitt B): „Nach dem Schließen einer Queue wird nichts mehr
  geliefert; das Schließflag wird **vor** dem Inhalt geprüft." Die Prüfliste ist laut
  `:7-10` keine Anforderungsquelle für Prüfer, wohl aber die Abhakliste der Worker.

**Was daraus folgt:** Eine wörtliche Zusage für den Fall „geschrieben, dann gewöhnliches
Linkende" gibt es nicht. Die Absicht ist aber belegt und verwaist: SONDE-010 hat die
at-least-once-Zustellung der Clienthälfte ausdrücklich an SONDE-011 übergeben, SONDE-011 hat
für die Richtung Sonde→Broker keine Zeile dafür (O-01 ist nur P0, O-02 bis O-04 gelten dem
Subscriberweg). Am Brokeringress gilt beides zugleich: B verlangt das Verwerfen, A verbietet
das stille Löschen von Angenommenem — heute ist der Verwurf still.

**Irreführende Texte:** A-IN-04 (`SONDE-010.md:191`) begründet das Trennen mit einer
Senderwiederholung, die es nach erfolgreichem Write nicht gibt. `Verbindung.cpp:921-925`
(„trotz Reconnect-Vertrag (§53.9)") und `SondeProcessor.cpp:989-992` („der
P1-Wiederholpuffer wirft ausdruecklich NICHTS weg („Was einmal angenommen wurde, bleibt
angenommen")") lesen sich wie eine Ende-zu-Ende-Zusage; sie gelten nur bis zum Write.

### d) Heutige Abdeckung

| Prüfstelle | Was sie misst | Was nicht |
|---|---|---|
| A4 `broker/src/transport/server_v3/tests_rueckstau.rs:118-140` (`geschlossener_eingang_liefert_nichts_mehr`) | nach dem Schließen liefert der Ingress nichts mehr | schreibt den stillen Verwurf als gewollt fest; zählt ihn nicht |
| A4 `warteschlange.rs:546` (`p1_haelt_ereignisse_fuer_den_reconnect_vor`), `:570` (`p1_wiederholpuffer_fliesst_ohne_reconnect_ab`) | Rust-P1-Politik | keine Linkgrenze |
| B10 `IpcTestMain.cpp:4929` („ein gescheiterter Write legt das Ereignis an seinen Platz zurueck") | A-P1-09 | kein erfolgreicher Write mit nachfolgendem Brokerverwurf |
| A4 `coordinator_model` (Evidenzempfang, SONDE-013 M-05, `SONDE-013.md:474`) | Annahme, Sperre, fremde Adresse | kein Linkende zwischen Write und Annahme |
| A22 (`tools/beweise.ps1:637`) | P0 geht unter P2-Flut nicht verloren | P1-Evidenz über ein Linkende |
| A4-SI `store_crash_matrix` (echter C++-Client, `tools/beweise.ps1:527`) | P0-Wiederholung über Brokerkills mit derselben `command_id` | P1-Evidenz |

### e) Urteil: BESTÄTIGT

Die Kette schließt am HEAD (Stationen 1 bis 5); keine der Stellen ist seit `aff2d818`
verändert (`queues.rs`, `verbindung.rs` unverändert; `senke.rs`/`evidenz.rs` nur die
P1-Weiche und Ganzzahlhelfer aus NAK-313). Die Einordnung als LÜCKE hält. Ergänzt:

1. Die Absicht zur at-least-once-Zustellung existiert (Entwurf `:4195-4198`, SONDE-010
   `:114`, `:273-275`) und ist zwischen SONDE-010 und SONDE-011 liegen geblieben — die
   Auditformel „keine eindeutige Zusage" ist richtig, „keine Absicht" wäre falsch.
2. Der Verwurf am Ingress ist ungezählt; das berührt Prüfliste A.
3. Eine Wiederholung würde heute doppelt zählen (Station 6); „Replay idempotent" aus dem
   Gate ist also ein Bauteil des Fixes, kein Nebenaspekt.
4. Dieselbe stille Verlustklasse entsteht bei der Sperre aus T3-14-03 (`senke.rs:117`).

### f) Kategorie und Schwere

**LÜCKE, Technikregel** (Begründung Regelfrage 3). **Schwere medium**: je Linkende gehen
bis zu einige Sekunden Evidenz einer Sonde verloren (Kadenz 1 bis 4 Hz, `SONDE-013.md:474`),
darunter Transientenereignisse, die kein späterer Snapshot wiederbringt; betroffen sind
Befunde und Resultatmessungen von Versuchen. Kein Audio, kein State.

### g) Kleinster Fix

**Regel in Sätzen (Technik):**

1. Ein nicht koaleszierbarer P1-Beleg gilt beim Sender als erledigt, wenn der Broker ihn
   **nach vollständigem Commit** (Sperrprüfung, Historie, Ablage im Store; der Punkt, den
   T3-08-02 in Teil A festlegt) bestätigt oder ausdrücklich endgültig abgelehnt hat — nicht
   schon nach dem Write.
2. Bis dahin hält der ControlClient der Probeeq ihn in einem kleinen, begrenzten
   In-Flight-Bestand je Link (Deckel an `kEvidenzP1Schwelle` gekoppelt, damit das Budget aus
   `SondeProcessor.cpp:974-979` hält) und sendet ihn nach einem Reconnect erneut, vor
   Neuzugängen, mit derselben `evidence_id`.
3. Der Broker nimmt eine bereits bekannte `evidence_id` idempotent: bestätigt erneut, hängt
   nichts an, zählt nichts doppelt, schreibt kein zweites `event_log`-Ereignis
   (Deduplikationsfenster: Historie je Client, `EVIDENZ_RETENTION = 32`, plus
   Primärschlüssel im Store).
4. Nach dem Schließen eines Links läuft kein Rückruf mehr gegen dessen Sitzung
   (`queues.rs:55-58` bleibt); der Ingressrest wird **gezählt** verworfen und durch die
   Senderwiederholung ersetzt.
5. Eine endgültige Ablehnung (Sperre nach T3-14-03, `beeinflusst`, Schemaverstoß) wird nicht
   wiederholt, aber beim Sender gezählt. Läuft der In-Flight-Bestand über oder übersteht ein
   Beleg eine feste Zahl von Reconnects nicht, wird er als gezählter Verlust verworfen und
   als Messlücke der Quelle gemeldet — das ist der ehrliche Restzustand, nicht die Regel.
6. Die Bestätigung reist auf der Antwortseite, blockiert keinen P0 (Prüfliste A, C-LS-07)
   und läuft nie im Audiothread.

**Dateien:** `SondeProcessor.cpp:1076-1131` (Entnahme erst bei Bestätigung oder bewusst
weiter nach Write — die Ereignisentnahme darf bleiben, weil der gehaltene Beleg sie
enthält), `controlclient/Verbindung.cpp:893-1044` und `IpcQueues.h` (In-Flight je Beleg),
`senke.rs:92-117` und `evidenz.rs:153-236`, `:445-485` (Bestätigung nach Commit, Dedupe),
`queues.rs:85-91` (Zähler beim Schließen). **Vertrag:** eine Bestätigung an den Sender ist
eine neue Wireform — neue Familie oder ein neues Feld in einem strikten Objekt, beides nur
als neue Fassung mit Vertragstext, Rust- und C++-Leser, Fixtures (`CLAUDE.md:178-181`,
10-1 „versionierungspflichtig"). Die Probeeq braucht dafür einen Antwortleser (heute leer,
`SondeProcessor.cpp:204`). Kein State-Bezug.

**Mitzuprüfende Beziehungen:** reservieren↔bestätigen (Write-Commit gibt den Queueplatz,
erst die Brokerbestätigung den Beleg frei); verbinden↔trennen (Reconnect wiederholt, alter
Link ruft nichts mehr); Überlauf gegen gewöhnliches Linkende getrennt; P0/P2-Vorrang bleibt.

**Audio und Latenz:** kein Audiothread-Bezug (Worker- und Clientthread); die Bestätigung
fügt je Sonde 1 bis 4 kleine Antworten je Sekunde hinzu.

### h) Ort des Rotbeweises

**Folgebeweis des Audits, deterministisch in A4** (Barriere wie im Auditlauf
`roh/phase-16-nachweise/b/wire.rs`, echter Listener und Coordinator, Probe-Pipe), neben
`tests_rueckstau.rs:118`: vier Belege 1000 bis 1003 schreiben, Senkenrückkehr nach 1000
halten, Link schließen, freigeben, Reconnect, 1004 senden. **Heute:** nur 1000 und 1004 in
der Historie, Overflow 0, kein Verwurfszähler. **Nach dem Fix:** mit Rust-Sender, der die
Regel spiegelt, 1001 bis 1003 genau einmal, Verwurfszähler 3 auf dem alten Link, kein
Rückruf nach der Abmeldung (bestehender Fall bleibt grün), zweite Zustellung derselben
`evidence_id` ohne Doppelzählung.

**Senderhälfte:** B10 (`EqCopIpcTest`, `tools/beweise.ps1:738`) gegen `V3TestServer`:
erfolgreich geschriebener, unbestätigter Beleg wird nach Reconnect mit derselben
`evidence_id` erneut gesendet; bestätigter nicht; endgültig abgelehnter nicht, aber gezählt.

**Kette mit echtem C++-Client:** A4-SI (`store_crash_matrix`, echter ControlClient nach dem
Muster der Kill-Fälle) oder A22 mit Evidenz statt P2.

**Zusagezeile und Mutation:** die neue Freigabe erst bei Brokerbestätigung in
`Verbindung.cpp` (heute `:1012`). Eine Mutation, die wieder beim Write freigibt, lässt die
IDs 1001 bis 1003 fehlen → rot; eine Mutation, die die Deduplikation in `evidenz.rs` entfernt,
zählt 1001 doppelt → rot.

---

## Gemeinsame Ursachen

**Eine Lebenslaufregel, drei Stellen.** Alle drei Befunde entstehen dort, wo ein Linkende
oder ein Linkwechsel mehr oder weniger wirkt, als dem Link gehört:

- T3-14-03: das Ende eines Links ohne Interventionsstrom sperrt die ganze Sitzung, und das
  späte Ende eines alten Gen-Links hebt die Aussage seines Nachfolgers auf.
- T3-15-15: der Wechsel zweier Nonces wirkt über jede Runde auf die Sitzung (Station 5), und
  der Konflikt überlebt jedes Ende, weil es keinen Löser gibt.
- T3-16-03: das Ende verwirft, was dem Link schon übergeben war, ohne dass der Absender es
  erfährt.

**Die gemeinsame Regel:** *Ein Ende räumt genau den Zustand, den dieser Link (diese
Generation) trägt; es setzt Sperren nur im Umfang dessen, was es verlieren konnte; ein
spätes Ende hebt keine Wirkung eines Nachfolgers auf; was ein Ende verwirft, wird gezählt
und — wo der Absender es wiederholen kann — ihm nicht als erledigt gemeldet.* Die Regel ist
dieselbe wie in C-LS-06 (`SONDE-010.md:256`, jeder Trenncallback gehört zu seiner Kopplung)
und trägt auch T3-16-02 aus Teil B.

**Gleiche Datei, verschiedene Wurzel:** `link.rs` trägt T3-15-15 (Verdrängung,
Registrierung, Deckel) und T3-14-03 (`control_ende`); getrennte Änderungssätze, weil die
Klonregel auf U50 wartet und die Unknown-Regel nicht.

**Was zusammengehört:**
- T3-15-15 Punkt 3 (kein Verdrängen durch quarantänisierte Besitzer) und die
  T3-14-03-Regel: beide nehmen die Sitzungssperre aus dem Klonwechsel. Die T3-14-03-Regel
  zuerst; dann entfällt die Kopplung auch ohne T3-15-15.
- T3-16-03 und T3-14-03: die Senderwiederholung aus T3-16-03 ist wertlos, solange jeder
  Probeeq-Reconnect die Sitzung sperrt — die wiederholten Belege würden alle gesperrt. Die
  Unknown-Regel muss vorher stehen.

**Grenzen zu den anderen Teilen (Regelfrage 4):**
- **T3-16-03 gegen T3-08-02 (Teil A):** T3-16-03 ist der Raum **vor** der Annahme (Write
  gelungen, Coordinator nie erreicht); T3-08-02 (`BEFUNDE.md:452`) der Raum **nach** der
  vorläufigen Annahme (ausgeschlossen, bevor persistiert; Restore macht ihn wieder gültig).
  Beide treffen sich im Commitpunkt `evidenz.rs:189-236`. Das Gate verlangt „ACK erst nach
  vollständigem Commit" — also setzt der ACK von T3-16-03 genau dort an, wo T3-08-02 den
  Commit (einschließlich Ausschluss) konsistent macht. Reihenfolge: T3-08-02 vor dem ACK,
  getrennte Änderungssätze.
- **T3-16-03 gegen T3-15-14 (nicht in diesem Ticket, S25n):** T3-15-14 ist das
  P0-Gegenstück (Replay vor dem ersten Heartbeat endgültig `unauthorized`). Der P1-Replay
  aus T3-16-03 darf denselben Fehler nicht wiederholen: ein wiederholter Beleg nach einem
  Reconnect darf nicht endgültig abgelehnt werden, nur weil der Link noch nicht bestätigt
  ist. Heute prüft die Evidenzannahme keine Bestätigung, aber die Sperre aus T3-14-03 —
  darum wieder: Unknown-Regel zuerst.
- **T3-14-03 gegen T3-16-02 (Teil B):** T3-16-02 (`BEFUNDE.md:721`) entfernt beim späten
  Ende einer alten Telemetrieverbindung die Kopplung der Nachfolgerin
  (`senke.rs:73-78` entfernt per Control-`link_id` ohne Besitzvergleich); T3-14-03 setzt
  beim Ende eines Control-Links ohne eigenen Interventionsstrom die Sitzungssperre. Beide
  tragen die obige Lebenslaufregel; verschiedene Dateien, getrennte Sätze, ein gemeinsamer
  Testtyp („spätes altes Ende nach Bestätigung des Nachfolgers").

---

## Regelfragen

### Frage 1 — Karte U50: Weg 1, Weg 2, Zusagen, Kennungsvergabe, Laden, Reload, Zusatz, Sperrenende

**Beide Wege sind technisch baubar.** Beide brauchen dieselben Primitive (g-Punkt 6 zu
T3-15-15): eine Neu-ID-Operation in der Probeeq mit Host-Dirty, einen Leser des Signals in
der Probeeq, die produktive Riegelauflösung im Broker. Sie unterscheiden sich nur darin,
**wer** entscheidet, welche Instanz die Kennung behält.

**Wer vergibt bei Weg 1 die neue Kennung, und wo liegt sie?** Nur die Probeeq kann sie
persistieren: die Kennung ist `common.instance_id` in ihrem State
(`SondeProcessor.cpp:1276`, `:1375`); der Broker schreibt keinen Pluginzustand, Gen auch
nicht. Also entscheidet der Broker, **welche** Instanz eine neue Kennung braucht, und sagt es
genau dieser; sie vergibt die ID, meldet genau ein Host-Dirty (`nakama-state-v2.md:323`) und
verbindet neu; danach räumt der Broker den Riegel der verbleibenden. Das heutige Signal
genügt nicht: `befehl.rs:772` meldet `duplicate_instance_id` beiden Seiten gleich.

**Nächstes Laden des Projekts.** Die neue Kennung reist nur mit dem nächsten Speichern. Wird
gespeichert, laden beide Instanzen danach verschiedene Kennungen — kein Konflikt. Wird nicht
gespeichert, lädt der Klon seine alte Kennung erneut; der Konflikt kehrt zurück, und die
Auflösung läuft noch einmal.

**Echter Klon gegen Reload.** Weg 1 greift nur bei zwei **lebenden** Nonces (C-10: die
verdrängte meldet erneut). Ein Reload derselben Probeeq beendet den alten Link, bevor der
neue kommt, und erzeugt keinen Konflikt. Zwei Grenzen: (i) ein später Report des alten Links
kann nach Quelle einen Riegel setzen, auch bei gleicher Nonce (T3-15-15 Station 11) — ein
automatischer Weg 1 würde dann einer einzelnen Probeeq ihre Identität nehmen; Fix-Punkt 5
muss vor Weg 1 stehen. (ii) „Die zuerst verbundene" ist nur im laufenden Betrieb
eindeutig (Original läuft längst, Kopie kommt neu). Laden beide zugleich — etwa ein Projekt,
in dem die Kopie gespeichert wurde, während Gen nicht lief (der Broker startet nur bei Gen mit
offenem Editor, B8 `tools/beweise.ps1:779`) —, entscheidet der Zufall der Ladereihenfolge.
Die Instanz, die die Kennung behält, behält auch Gens gespeicherte Mitgliedschaft
(`nakama-state-v2.md:24` `confirmed_members_v1`) und alles, was an der Kennung hängt; landet
das auf der falschen Spur, wandert Gens Wissen über die Quelle mit.

**Vereinbarkeit mit den Zusagen:**

| Zusage | Weg 1 (zuerst verbundene bleibt, Klon automatisch neu) | Weg 2 (beide gesperrt, User wählt in Gen) |
|---|---|---|
| C-07 „Auflösung ausschließlich über explizite Neu-ID" (`SONDE-011.md:513`) | vereinbar, wenn die Neu-ID ein ausdrücklicher Schritt ist (keine Freigabe über Zeit) | vereinbar |
| C-07 „Registrierungsreihenfolge ändert das Urteil nicht" | **Spannung:** Weg 1 macht die Reihenfolge zum Kriterium der Auflösung. Die Sperre selbst bliebe reihenfolgefrei, aber der Satz hat einen Sicherheitshintergrund (NAK-40, H-14 „desselben Angreifers"); seine Bewertung ist nicht Gegenstand und vor dem Bau zu delegieren | vereinbar |
| `nakama-state-v2.md:327` „der User löst sichtbar auf (`neueSensorId` ⇒ neue ID, Label/Paar bleiben, Host-Dirty)" | **unvereinbar** im Teil „der User löst"; vereinbar in „neue ID, Label/Paar bleiben, Host-Dirty". Verlangt eine Textänderung auf ein datiertes Userwort | vereinbar |
| Entwurf `:1854` „Duplikate werden sichtbar aufgelöst" | vereinbar, wenn Gen die automatische Auflösung ehrlich meldet (Diagnosezustand) | vereinbar |
| Entwurf `:1870-1872` „Bei … duplizierten IDs ist eine kurze sichtbare Bestätigung Pflicht" | **Spannung:** die Kopie tritt mit neuer ID automatisch bei (eindeutiges Main, `mitgliedschaft.rs:30-55`). Auflösbar, indem die Kopie als Beitrittskandidat mit Bestätigung erscheint (vorhandener `confirm_join`) | vereinbar |
| `CLAUDE.md:44-45` ehrliche Anzeige | vereinbar mit Diagnosezustand | braucht eine Wahl zwischen zwei in Gen unterscheidbaren Einträgen — heute fällt der Klon in **eine** Zeile (`sicht.rs:519-525`), und beide tragen denselben Namen; unterscheidbar wären sie nur über den Mixerkanal, und den liefert FL ungemessen (U26, `docs/plan/fragen.json:30-40`) |
| Designparken (`CLAUDE.md:34`) | Diagnosezustand genügt | die sichtbare Wahl ist Bedienung in Gen; über einen Diagnosezustand hinaus geparkt |

**Ist der Zusatz „Ohne Antwort baut der Dirigent Weg 1" haltbar?** Nein, nicht als stiller
Rückfall. Weg 1 ändert den Vertragstext `nakama-state-v2.md:327` und den Entwurf
`:1870-1872`; nach `CLAUDE.md:53-55` macht nur Datum plus Userzitat eine Entscheidung
verbindlich, und nach `:61-63` wird der Widerspruch registriert, nicht durch Bau kaschiert.
Dazu steht Weg 1 in Spannung zu einem Satz von C-07 mit Sicherheitshintergrund, den dieses
Ticket nicht neu bewerten darf (Gate: „keine Sicherheitsneubewertung"). Haltbar ist:
**zuerst** der kartenunabhängige Teil (T3-15-15 g-Punkte 1 bis 6: ehrlicher Konfliktzustand
im Snapshot und in Gens Diagnose, kein positiver ACK ohne Wirkung, Wechsel begrenzt,
512er-Deckel nur für echte Kollisionen, Quarantäne endet mit der Eviction der Nonce, Guards
2/4/6 werden bei Auflösung über alle Epochen geräumt, Riegel nur bei echter zweiter Nonce,
Auflösungsprimitive samt Leser des Duplikatsignals). **Danach** die Wegwahl. Ohne Antwort
bleibt es beim Vertragstext — fail-closed, jetzt aber ehrlich angezeigt.

**Verspricht die heutige Karte etwas, das die Technik nicht hält?** Ja, an zwei Stellen:
(1) „heute sperren sie sich gegenseitig, bis eine verschwindet" — falsch: nach Quelle
bleibt die Sperre auch dann, wenn eine Instanz gelöscht wird, bis das Projekt neu geöffnet
wird (Station 9; C-07/E-03 verbieten die Freigabe über Zeit ausdrücklich). Dazu stehen
während des Konflikts auch die Messungen der **übrigen** Sonden (Station 5). (2) Weg 1
verspricht „die zuerst verbundene bleibt", ohne die Ladegrenze zu nennen. Präzisierter
Kartentext:

```json
{
 "id": "U50",
 "dringlichkeit": "wenn du dazu kommst",
 "art": "Produktentscheid (Verhalten)",
 "titel": "Probeeq samt Einstellungen kopiert: welche Kopie bleibt die bekannte Quelle?",
 "was": "Kopierst du in FL eine Probeeq mit ihren Einstellungen (Spur klonen, Plugin kopieren), tragen beide dieselbe Kennung. Heute blockieren sie sich dann gegenseitig: Gen bekommt von beiden keine Messung mehr, die EQ-Fernsteuerung beider ist gesperrt, und solange beide laufen, pausiert auch die Auswertung der übrigen Quellen. Das Audio läuft unverändert weiter. Die Sperre bleibt, auch wenn du eine der beiden wieder löschst – erst beim nächsten Öffnen des Projekts ist sie weg, und sie kommt zurück, solange beide existieren. Gen zeigt dabei „Join confirmation required“, und „Bind source“ hilft nicht. Das wird repariert: der Konflikt wird künftig als Konflikt angezeigt, und die übrigen Quellen messen weiter. Offen ist nur, wer die Kennung behält. (1) Empfohlen: die Probeeq, die schon lief, bleibt die bekannte Quelle mit allem, was Gen über sie weiß; die neu hinzugekommene bekommt automatisch eine neue Kennung und erscheint als weitere Quelle mit gleichem Namen, den du ändern kannst. Das Projekt gilt danach als geändert; beim nächsten Speichern ist es dauerhaft gelöst. Grenze: werden beide gleichzeitig geladen – etwa weil du die Kopie gespeichert hast, während Gen nicht offen war –, kann Nakama das Original nicht erkennen; dann behält eine der beiden zufällig die Kennung. (2) Beide bleiben gesperrt, bis du in Gen auswählst, welche die bekannte ist. Grenze: beide tragen denselben Namen, und welche Spur welche ist, kann Gen erst zeigen, wenn FL die Busnamen herausgibt (Karte U26); bis dahin wäre die Wahl blind.",
 "warum": "Audit-Befund T3-15-15 (Arbeitsübersicht W17), an der Quelle bestätigt und präzisiert (Validierung NAK-314 Teil C). Weg 1 ändert die heutige Vertragsregel „der User löst sichtbar auf“ und braucht deshalb deine Antwort. Ohne Antwort bleibt der Konflikt gesperrt, wird aber ehrlich angezeigt, und die übrigen Quellen messen weiter (Planschritt S25m, Ticket NAK-314).",
 "wo": "docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md T3-15-15, Arbeitsübersicht W17; docs/beweise/roh/NAK-314-quellvalidierung-teil-C.md; docs/offene-punkte.md NAK-314",
 "seit": "18.09.",
 "status": "offen"
}
```

Empfehlung bleibt Weg 1: er braucht keine Bedienung in Gen, ist im häufigen Fall (Kopie im
laufenden Betrieb) richtig und blind nur im seltenen Ladefall; Weg 2 ist bis zu U26 blind in
jedem Fall. Die Sicherheitsbewertung des Reihenfolgekriteriums (C-07) geht vor dem Bau an
einen Prüfer. `docs/ZIELBILD.md:349-350` („Ohne Antwort: die zuerst verbundene; der Klon wird
eine neue Quelle") zieht mit der Karte nach.

**„Sperrenende" (Register NAK-314) — welche Sperre endet bei welchem Ereignis:**

| Sperre (Ort) | Link-Ende | Tombstone-Frist (10 000 ms) | Brokerneustart | Auflösung (Neu-ID) | heute |
|---|---|---|---|---|---|
| Linkverdrängung `trennen`/`verdraengt` (`link.rs:151-155`) | endet (Link weg) | — | — | — | so |
| Alias-Quarantäne je `instance_id:nonce` (`instance_alias.rs:194-195`) | bleibt (C-07) | **soll enden** für die evictierte Nonce; der Riegel trägt die Konfliktwahrheit (E-03 bleibt wahr) — H-14-Wirkung vor Bau delegiert prüfen | endet (flüchtig) | endet | endet nur mit Neustart oder Test-API |
| Persistenter Riegel je effektiver Adresse (`zustand.rs:509-518`, Store) | bleibt | bleibt (C-07, E-03) | bleibt, vor Routing restauriert (E-06) | **endet**, und zwar für alle Epochen derselben Kennung | endet nie produktiv; Zeilen toter Epochen bleiben bis zum Deckel 1024 |
| `bestaetigt = false` des Konfliktclients (`liveness.rs:387-389`) | bleibt am Client | fällt mit dem Client | fällt | Beitritt der verbleibenden Instanz gilt wieder | heute durch `confirm_join` setzbar, ohne Wirkung |
| Globaler Aliasdeckel 512 (`link.rs:126-129`) | — | — | endet | — | soll nur echte Kollisionen treffen |
| Sitzungs-Unknown aus dem Wechsel (`link.rs:588-593`) | entsteht bei jedem Ende | bleibt (C-08) | Taint ist flüchtig | nicht durch die Auflösung | nach T3-14-03-Regel entsteht es beim Probeeq-Ende nicht mehr |

### Frage 2 — Unknown nach Sonde-Abbruch (T3-14-03)

**Quellenlage:** C-08 (`SONDE-011.md:514`), 10-1 (`:202`), M-39 (`SONDE-013.md:524`), M-61
(`:561`), M-62 (`:562`), Entwurf §34.2 (`:2249-2254`) — wörtlich unter 2c. Sie verlangen die
sticky Sperre bei jedem Control-Disconnect und lassen als einzigen Löser den bestätigten
Neutral-/Sequenz-Resync zu, ausdrücklich als Coordinator-Lebenszyklusoperation ohne neue
Wirefamilie; eine neue Wireform wäre versionierungspflichtig. NAK-180 N-11
(`docs/beweise/NAK-180.md:306`) verbietet der Probeeq jede Neutralitätsaussage.

**Welche Wege die Quellen zulassen:**
- **Gen-Neutralitätserklärung über Gens eigenen Neuaufbau** (R1/R2, `befehl.rs:744-768`,
  `Ipc.cpp:956-997`): zulässig und vorhanden, nur ohne Auslöser. Ein Auslöser aus einem
  sichtbaren Sperrmerkmal ist mit allen Zeilen vereinbar.
- **`neutral_resync` direkt:** ist die Coordinator-Operation selbst; ein Aufruf ohne
  bestätigte Neutralität wäre die verbotene Selbstheilung.
- **Neuer ausdrücklicher Befehl** (Gen erklärt auf stehendem Link neu): zulässig nur als neue
  Fassung (10-1).
- **Normale Heartbeats:** dürfen nie freigeben (M-61; `befehl.rs:729-733`; Test
  `sonde013_verdrahtung.rs:4733`).

**Technik oder Produkt: Technik.** Kein sichtbarer Handgriff ist nötig: ob eine Intervention
verloren sein kann, weiß allein Gen (Ring, Marker, offene Enden), und die Probeeq trägt keine
Interventionen. Die Regel steht in 2g: ein Ende sperrt nur, was es verlieren kann (C-08 und
M-39 werden im Text präzisiert); wo die Sperre bleibt, trägt der Snapshot ein Merkmal, Gen
löst durch eigenen kontrollierten Neuaufbau, wenn es neutral ist, sonst zeigt es einen
Diagnosezustand. Eine Karte entfällt.

### Frage 3 — Empfangs-ACK oder gezählter Verlust (T3-16-03)

**Keine echte Produktwahl.** Der Musiker merkt den Unterschied nur als fehlende oder
vorhandene Messsekunden einer Quelle; einen Grund, den Verlust zu wollen, hat er nicht, und
das Gate legt die Form schon fest („ACK erst nach vollständigem Commit, Replay idempotent,
keine Callbacks auf abgemeldeter Sitzung"). Unter dieser Grenze ist at-least-once mit
Deduplikation die einzige tragfähige Lösung; der gezählte Verlust ist nur der ehrliche
Restzustand, wenn der begrenzte Senderbestand überläuft oder ein Beleg endgültig abgelehnt
wird. **Die Register-Karte „Empfangs-ACK oder gezählter Verlust" (`docs/offene-punkte.md:317`)
wird nicht gebraucht;** `docs/plan/fragen.json` trägt zu Recht keine.

**Zusagen heute:** Entwurf §53.9 `:4162` (Wiederholung **bei Überlauf**) und `:4195-4198`
(at-least-once an jeder Outboxgrenze); SONDE-010 A-P1-08 (`:158`), A-P1-09 (`:159`),
A-IN-04 (`:191`), `:114`, `:273-275`; SONDE-011 O-02 (`:561`), O-04 (`:563`); NAK-95
(`docs/offene-punkte.md:516`) und NAK-92 Nr. 4 (`:522`) — wörtlich unter 3c. Prüfliste A
(`tools/dirigent/pruefliste.md:14-17`) und B (`:36-37`) stehen am Ingress gegeneinander;
auflösen lässt sich das nur, indem der Verwurf gezählt und durch Senderwiederholung ersetzt
wird.

**Zusammenhang heute:** Producerentnahme nach lokaler Einreihung
(`SondeProcessor.cpp:1085-1131`) → Writerreservierung, Rücklegen nur bei gescheitertem Write,
Freigabe beim Write-Commit (`Verbindung.cpp:901-904`, `:952`, `:1012`) → Brokeringress,
Verbraucher, Annahme im Coordinator (`verbindung.rs:616-625`, `evidenz.rs:153-236`) →
Schließen verwirft den Rest ungezählt (`verbindung.rs:814`, `queues.rs:55-58`) → Reconnect
wiederholt nur Ungeschriebenes (`Verbindung.cpp:746`). Zwischen Write-Commit und Annahme hält
niemand den Beleg.

**Die Regel in Sätzen:** siehe 3g, Punkte 1 bis 6.

### Frage 4 — Grenzen der drei Befunde

Siehe „Gemeinsame Ursachen", Absatz „Grenzen zu den anderen Teilen": T3-16-03 liegt vor der
Annahme, T3-08-02 nach ihr, beide am Commitpunkt `evidenz.rs:189-236`; T3-15-14 ist das
P0-Gegenstück in S25n; T3-14-03 und T3-16-02 tragen dieselbe Lebenslaufregel („ein Ende
wirkt nur auf das, was ihm gehört, und hebt keinen Nachfolger auf"), an `link.rs` bzw.
`senke.rs`.

---

## Reihenfolge

1. **Messen, ohne Produktdiff (Etappe 1).** Den Laufzeit-Arm um eine Messung ergänzen, ob
   nach dem Laden des Diagnoseprojekts und nach einem `prepareToPlay` aller Instanzen die
   Sitzung gesperrt bleibt (`evidence_gesperrt` gegen `evidence_angenommen`); davon hängt die
   Schwere von T3-14-03 ab. Die drei deterministischen Rotbeweise in A4 anlegen
   (T3-15-15-Folgebeweis, T3-14-03 mit Produktvorgang, T3-16-03-Barrierelauf) — heute rot
   gegen die neuen Regeln.
2. **T3-14-03-Regel (Etappe 2).** Zuerst, weil der Klonwechsel (T3-15-15 Station 5) und die
   Senderwiederholung (T3-16-03) davon abhängen. Textänderung C-08/10-1/M-39 mit Regelnummer,
   `control_ende`, Snapshotmerkmal als Fassungsschritt, Gens Reaktion. Kein Kartenbezug.
3. **T3-15-15, kartenunabhängiger Teil (Etappe 3).** g-Punkte 1 bis 6; Fassungsschritt für
   Konfliktmerkmal und Fehlercode gemeinsam mit dem Merkmal aus Etappe 2, wenn zeitlich
   möglich (ein Schritt statt zwei). Punkt 4 erst nach delegierter Prüfung der H-14-Wirkung.
   Karte U50 mit präzisiertem Text vorlegen (Zeitregel des Dirigenten).
4. **T3-16-03 (Etappe 4)** nach T3-08-02 (Teil A, Commitpunkt) und nach Etappe 2:
   In-Flight je Beleg, Bestätigung nach Commit, Dedupe, gezählter Verwurf, neue Fassung.
5. **U50-Wegwahl (Etappe 5)** nach der Antwort: Auslöserpolitik, Signal an genau eine
   Probeeq, Texte `nakama-state-v2.md:327`, Entwurf `:1854`, `:1870-1872`, C-07; bei Weg 1
   vorher die delegierte Bewertung des Reihenfolgekriteriums.

**Matrixzeilen, die nachgezogen werden müssen:**
- SONDE-011: C-07 (`:513`, Auflösungsweg und Lebensdauer), C-08 (`:514`, Umfang des
  Disconnects), C-10 (`:516`, gleiche Nonce; kein Verdrängen durch quarantänisierte
  Besitzer), L-03 (`:624`), E-03 (`:524`) und E-05 (`:526`, Quarantäne der evictierten Nonce),
  10-1 (`:202`), O-01 bis O-04 (`:560-563`, neue Zeile für Sonde→Broker-P1), R3 Nr. 2
  (`:1210`, Verweis).
- SONDE-013: M-39 (`:524`), M-61 (`:561`, Auslöser), M-62 (`:562`, unverändert im Umfang),
  M-63 (`:563`, Produktleser des Sperrzählers), M-05 (`:474`, Transport mit Bestätigung), die
  Prüflistenzeile `:1038`.
- SONDE-010: A-IN-04 (`:191`, Begründung), A-P1-08/A-P1-09 (`:158-159`) plus neue Zeile
  „geschrieben, nicht bestätigt", „Was die Matrix bewusst nicht festlegt" (`:273-275`,
  Verweis auf die neue Zeile).
- NAK-121 H-14 (`:228`, falls Quarantänelebensdauer und Deckelumfang sich ändern).
- Register: NAK-188 (`docs/offene-punkte.md:210`, Lösungsannahme berichtigen), NAK-138
  (`:169`, durch Etappe 1 und 3 gedeckt), NAK-314 (`:317`, Karte „Empfangs-ACK" entfällt).

---

## Nicht geprüft

- **Keine Messung, kein Lauf.** Bau, Test, Kanon, Skript und FL waren verboten; alle
  Aussagen sind Quellenherleitungen. Die Auditläufe (Phase 16 B/C/E) habe ich nicht
  wiederholt; ihre Zahlen (512, 2/4/6, 1000 bis 1004, Highwater 3) sind mit den gelesenen
  Stellen vereinbar, nachgerechnet sind nur die Deckel aus den Konstanten.
- **Station 11 von T3-15-15** (Riegel bei gleicher Nonce) ist hergeleitet: ob das Fenster
  zwischen neuem Hello und dem Schließen des alten Ingress in der Praxis getroffen wird, ist
  nicht gemessen. Sollte der Dirigent sie als eigenen Befund führen, ist sie noch nicht im
  Register.
- **Die Aufrufreihenfolge von `prepareToPlay` und `setStateInformation`** über Gen und viele
  Probeeq-Instanzen beim Projektladen in FL ist unbekannt; davon hängt ab, ob T3-14-03
  routinemäßig eintritt (Schwerevorbehalt).
- **Der FL-Hostnachweis aus F19** (isoliertes FL-Preset in zweiter Instanz,
  `BEFUNDE.md:766`) ist nicht Gegenstand; welche FL-Handgriffe genau einen Stateklon erzeugen,
  ist ungemessen (OF-08, `roh/phase-15-paket3-teilfehler.md:333`).
- **Sicherheitswirkung** der vorgeschlagenen Lebensdauerregeln (Quarantäne bei Eviction,
  Deckel nur für echte Kollisionen) und des Reihenfolgekriteriums von Weg 1 (C-07, NAK-40,
  H-14): nicht bewertet, vor dem Bau zu delegieren. Ebenso nur beobachtet, nicht bewertet:
  der Broker nimmt ein neutrales erstes `false` von jeder Klasse an (`befehl.rs:744`); nur die
  C++-Seite verhindert es (N-11).
- **NAK-126** (verdrängte Control-Pipe bleibt eingabefähig, `docs/offene-punkte.md:162`) und
  sein Zusammenspiel mit dem Klonwechsel habe ich nicht vertieft.
- **Fremde Datei:** `docs/beweise/NAK-314.md` erschien während des Laufs untracked; nicht
  gelesen, nicht angefasst.
- **Die IDs aus Teil A und B** waren nicht Gegenstand; ihre Grenzen sind nur aus meiner Seite
  gelesen (T3-08-02 `BEFUNDE.md:452`, T3-16-02 `:721`).

FERTIG Teil C, 2 bestätigt, 1 präzisiert, 0 widerlegt.
