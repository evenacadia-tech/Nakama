# Tiefenaudit 3 · Phase 15 · Gegenprüfung Paket 3 („Erholung nach Teilfehlern")

> **Rolle dieses Berichts:** Widerlegungsprüfung zu Paket 3 im Tiefenaudit 3, Phase 15
> (17.09.2026, Quellstand `e008811efa7650f3e0cc2332cc21f9e268cdd125`). Ein anderer
> lesender Prüfer als der Finder hat jeden Kandidaten gezielt zu widerlegen versucht; sein
> Urteil ist für Kategorie und Schwere verbindlich, soweit
> [phase-15-zentral.md](phase-15-zentral.md) nichts anderes begründet. Kandidat `K1` bis
> `K5` entspricht `P15-T-01` bis `P15-T-05` in
> [phase-15-paket3-teilfehler.md](phase-15-paket3-teilfehler.md); `K3` ist hier als
> Duplikat zu NAK-188 eingeordnet und damit nicht aufgenommen.


Rolle: Skeptiker. Auftrag war Widerlegung, nicht Bestätigung. HEAD
`e008811efa7650f3e0cc2332cc21f9e268cdd125`. Nur gelesen; kein Bau, kein Lauf,
kein Broker-/FL-Start. Sicherheits- und Designthemen ausgeklammert.

---

## K1 — Degradierter Store: jeder Heartbeat trennt seinen Absender

**URTEIL: BESTÄTIGT** · **Kategorie: DEFEKT** · **Schwere: high** (K2/K8)

### Stärkstes gefundenes Gegenargument
Die Verursachertrennung ist **ausdrücklich zugesagt und durch einen eigenen Test
festgeschrieben**. S-03 Satz 1 (`docs/beweise/SONDE-011.md:535`) lautet wörtlich
„Bei vollem Kanal oder degradiertem Store wird die Annahme persistenzpflichtiger
Ereignisse verweigert **und der Verursacher getrennt**"; O-01 (`:560`) sagt
dasselbe für P0. Und es gibt genau dafür ein Bein:
`broker/tests/store_crash_matrix.rs:1558-1576`
`degradierter_store_verweigert_annahme_und_tick_trennt_verursacher` — degradierter
Writer, ein Report, `store_verweigerungen() == 1`,
`verbindung_soll_trennen("verursacher")`, `liveness_tick() == ["verursacher"]`.
Der Ablauf, den K1 beanstandet, ist also gewollt, benannt und gemessen. Ein
abgeleiteter Snapshot-Flush IST persistenzpflichtig: `flush.rs:149-158` baut einen
echten `StoreEvent::session_snapshot` und reicht ihn ein.

### Warum der Kandidat trotzdem überlebt
Der Test misst **genau einen** Report und hört auf. Er misst nicht die zweite
Runde, nicht P2 und nicht den sichtbaren Zustand. Genau dort bricht die
**zweite Hälfte derselben Zusage**: S-03 Satz 2 „sichtbarer gezählter Betrieb ohne
Historie, **P2 läuft weiter**", L-08 (`:629`) und O-01 „P2 bleibt aktiv". Beide
Hälften können nicht gleichzeitig gelten, wenn der Verursacher der **1-Hz-
Heartbeat** ist — dann trennt sich jede Sitzung an ihrem eigenen
Lebenszeichenkontakt, und es gibt keinen „Betrieb", in dem P2 weiterlaufen
könnte. Das ist der Defekt: nicht die Trennregel, sondern dass der
Heartbeat-Flush sie zum Dauerzustand macht.

Selbst geprüfte Kette (jede Stelle gelesen):

* Heartbeat wird im 1-Hz-Takt gesendet, auch ohne Änderung:
  `eq-copilot/plugin/core/ipc/controlclient/Verbindung.cpp:757`, `:851-874`
  (`kHeartbeatTaktMs = 1000`, `eq-copilot/plugin/core/ipc/ControlClient.h:42`).
* Sitzung wird bei **jedem** Heartbeat dirty markiert — unbedingt, ohne
  Änderungsvergleich: `broker/src/coordinator/liveness.rs:505-506`; Flush mit
  Verursacherlink `:535-537`.
* `flush_session` nimmt die Dirty-Marke (`flush.rs:92`), baut den Store-Event und
  trennt im Fehlerzweig den Verursacher: `flush.rs:158-169`.
* Degradierter Handle verweigert jeden Auftrag: `broker/src/store/handle.rs:219-223`
  (über `append` → `append_einreihen` → `senden`).
* Der Tick schließt genau die Links mit `trennen`:
  `broker/src/lebenslauf.rs:178-187`, `liveness.rs:264-269`.
* **Kein Weg hebt die Degradation auf.** `degradiere` setzt nur
  (`store/handle.rs:495-501`); kein `degradiert = false` außer der Initialisierung
  `store/mod.rs:220`. Laufzeitdegradation entsteht an vier Stellen
  (`store/writer.rs:346`, `:368`, `:375`, `:386`), Startdegradation an
  `lebenslauf.rs:143-146`.
* **Kein Weg beendet den Broker.** `broker_soll_idle_enden`
  (`lebenslauf.rs:299-327`) braucht 0 aktive Clients über 60 s; die Clients
  verbinden sich ständig neu, also nie.

Zwei zusätzliche, selbst gefundene Verschärfungen, die K1 nicht nennt:

1. **Die ehrliche Anzeige erreicht Gen nie.** Der Client sendet P0 strikt vor P1
   (`Verbindung.cpp:884-895`: `istP0 = p0.entnehmen(...)`,
   `etwasGesendet = istP0 || p1.entnehmen(...)`). Der erste Heartbeat ist P0, das
   `subscribe_session` wird im Link-Callback als P1 eingereiht (`:729` →
   `eq-copilot/plugin/src/prozessor/Ipc.cpp:1041`). Der Heartbeat trifft den
   Broker also **vor** dem Subscribe, setzt `trennen`, und
   `resubscribe_snapshot_push` fällt danach an `subscription.rs:93` aus. Damit
   wird `subscriptionAktiv` nie wahr, `Diagnose::storeDegraded`
   (`eq-copilot/plugin/src/SourcesModel.cpp:1356`) wird nie gesetzt, und der User
   sieht ausschließlich `authenticating` (`SourcesModel.cpp:593-596`) bzw.
   `brokerUnavailable` (`:577-583`) — obwohl der Broker läuft. Die
   Degradations-Ausnahmen in `subscription.rs:94` und `:427` sind vorhanden und
   laufen genau deshalb leer.
2. **Kein Dämpfer.** Der Reconnect-Backoff wird nach jedem erfolgreichen Welcome
   zurückgesetzt (`Verbindung.cpp:451-452`, `kBackoffStartMs = 500`,
   `eq-copilot/plugin/core/ipc/IpcVerbindung.h:34`). Es gibt also ~2
   Verbindungsaufbauten je Sekunde statt einer abklingenden Folge.

### Start- gegen Laufzeitvariante
* **Start degradiert** (`lebenslauf.rs:143-146` → `mod.rs:316`
  `routing_bereit = !ist_degradiert()`): zusätzlich kein Riegelrestore
  (`mod.rs:317-333` steht im `if routing_bereit`-Block) und `routing_bereit`
  bleibt für immer falsch — einzige Setzer sind `mod.rs:316`, `zustand.rs:624`
  (Default) und `flush.rs:33` (nur nach false). Folge: P2 wird schon am Ingress
  abgewiesen (`senke.rs:149-155` über `dispatch_fuer_link_erlaubt_locked`,
  `flush.rs:289-298`), Bind/Remove `abgelehnt/unauthorized`
  (`befehl.rs:493-514`). Das bricht zusätzlich S-01 (`:533`) „P2 darf
  weiterlaufen".
* **Laufzeitdegradation**: `routing_bereit` bleibt wahr, P2 wäre erlaubt — aber
  der Link trägt nach dem ersten Heartbeat `trennen`, also greift
  `!link.trennen` in `dispatch_fuer_link_erlaubt_locked` und der Frame fällt
  ebenso. Die Trenn-/Reconnectschleife ist in beiden Varianten identisch.

### Duplikat?
Nein. NAK-125 (`docs/offene-punkte.md:162`, offen) beschreibt das **umgekehrte**
Problem bei degradiertem Store (`session_command` quittiert „angewandt" ohne
Persistenz); NAK-274 E5-2 (`:284`) ein Ordinal-Randfenster an der Degradation.
Kein T3-Befund berührt die Degradation (Suche in
`docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` ohne Treffer).

### Kategorie und Schwere
DEFEKT ist richtig: S-03, L-08, O-01 und S-01 sind ausdrückliche Matrixzusagen.
high ist vertretbar — im zugesagten Weiterbetriebsfall ist das Produkt
unbenutzbar, und der User bekommt keine ehrliche Ursache. Nicht critical: der
Auslöser ist selbst ein Fehlerfall, Audio bleibt unberührt (Grundgesetz und
`AUDIO_THREAD_STORE_WAIT_MS_MAX = 0` intakt), kein Datenverlust.

---

## K2 — Ein Store-Lesefehler beim Resubscribe schaltet Routing brokerweit ab

**URTEIL: HERABGESTUFT** · **Kategorie: HÄRTUNG** (vorgeschlagen war LÜCKE) ·
**Schwere: low** (K2/K8)

### Stärkstes gefundenes Gegenargument
Es gibt ein Bein, das **genau diesen Ablauf als gewollt festschreibt**:
`broker/tests/store_crash_matrix.rs:1156-1172`
`projektionslesefehler_haelt_subscription_sichtbar_fail_closed` — Writer stoppen,
DB-Datei löschen, subscriben, dann `assert!(!coordinator.routing_bereit())`,
`assert!(coordinator.verbindung_soll_trennen("main"))`,
`assert!(push.snapshots().is_empty())`. Die Sperre ist zusätzlich durch C-07
(`SONDE-011.md:513`) „Misslingt Persistenz oder Restore, bleibt Routing
fail-closed", S-01 (`:533`) und die Matrixzeile (`:1061`) gedeckt, und „keine
zeitbasierte Freigabe" ist dort ausdrücklich Teil der Zusage. Der fehlende
automatische Wiederanlauf ist damit Absicht, nicht Versehen.

**Die Erreichbarkeit ist deutlich schmaler als behauptet.** Der realistische
Dauerfall (DB gelöscht, Platte tot, Pfad weg) degradiert innerhalb eines
Heartbeats auch den **Writer** (`store/writer.rs:346`, `:368`, `:375`, `:386`).
Dann ist `store_degradiert()` wahr, und genau dafür existieren die
Ausnahmezweige `subscription.rs:94` (`!routing_bereit && !store_degradiert`) und
`:427` (`routing_bereit || store_degradiert`): Livestand mit
`MARKE_OHNE_ORDINAL` wird zugestellt, und Gen zeigt ehrlich
`Diagnose::storeDegraded` (`SourcesModel.cpp:1356`). Der von K2 beschriebene
Zustand „Routing aus, Store gesund, Anzeige `authenticating` für immer" braucht
einen **Lesefehler, der nie zu einem Schreibfehler wird** — also ein transientes
`SQLITE_BUSY` jenseits `BUSY_TIMEOUT_MS = 2000` oder eine kurze Fremdsperre auf
der Datei (`store/handle.rs:483-493`, `Connection::open_with_flags` READ_ONLY +
`busy_timeout`). Das ist konstruiert, nicht Normalbetrieb.

### Was trotz Widerlegungsversuch stehen bleibt
* Die Latch ist **einschüssig und brokerweit**: ein einziger transienter
  Lesefehler setzt `routing_bereit = false` und `trennen` für **alle** Links
  **aller** Sitzungen (`flush.rs:31-37`), und kein Setzer macht es je wieder wahr
  (vollständige Setzerliste geprüft: `mod.rs:316`, `zustand.rs:624`, `flush.rs:33`).
  Ein neues Hello wird weiterhin angenommen (`link.rs:100-338` prüft
  `routing_bereit` nicht) und `subscribe_json` ebenfalls
  (`subscription.rs:34-72`) — der Client kommt also durch und bekommt dann
  nichts.
* Das Wort „sichtbar" im Testnamen ist auf der Produktfläche **nicht gemessen**:
  der Test prüft nur Trennmarke und leere Pushliste. Produktseitig bleibt
  `subscriptionAktiv` falsch, also `Diagnose::authenticating`
  (`SourcesModel.cpp:539-553`, `:593-596`) ohne Handgriff
  (`diagnoseHatHandgriff` wird dort nicht gesetzt). Gegen `CLAUDE.md:51-52`
  („meldet ehrlich einen Zustand") ist genau dieser Rest ein Befund — nicht die
  Sperre, nicht die brokerweite Reichweite, nicht der fehlende Zeitablauf.
* P2 wird dabei mit `RoutingNichtFreigegeben` abgewiesen (`senke.rs:149-155`),
  Bind/Remove mit `abgelehnt/unauthorized` (`befehl.rs:493-514`), und der negative
  ACK verschwindet still (`prozessor/Ipc.cpp:1069-1077`).

### Kategorie und Schwere
HÄRTUNG, nicht LÜCKE: der beanstandete Kern (Sperre, Reichweite, kein
Zeitablauf) ist durch C-07/S-01 und ein Bein gedeckt; übrig bleibt ein
Robustheits-/Detektorthema plus die unehrliche Anzeige. low, weil der Auslöser
ein enges transientes Fenster ist und der häufige Dauerfall über die
Degradationsausnahmen ehrlich landet. Wer den Befund behalten will, sollte ihn
auf „ehrlicher Zustand bei fail-closed Routing mit gesundem Store" verengen.

---

## K3 — Jedes v2-Verbindungsende sperrt Evidenz brokerweit

**URTEIL: DUPLIKAT** (NAK-188), Restanteil **HERABGESTUFT** ·
**Kategorie: HÄRTUNG** · **Schwere: low** (K2)

### Mechanik stimmt
* Jede v2-Verbindung erhält die Senke und die ID `v2:<len>:<sensor>:<nonce>`:
  `broker/src/server.rs:761-767`; `Drop` ruft `senke.getrennt`: `:352-371`.
* `hoermarkierung_v2_getrennt` (`coordinator/intervention.rs:299-307`) findet die
  v2-ID nie in `stand.links` (dort liegen nur v3-Links, `link.rs:288-304`), nimmt
  `SessionKey::unbekannt()` und setzt `unknown = true` **bedingungslos** — auch
  ohne je aktiven Marker.
* Der Platzhalter sperrt Evidenz in **jeder** Sitzung:
  `intervention.rs:632-647` (`erlaubt && taint.get(&SessionKey::unbekannt())`).
* Nur Gen hat einen v2-Client (`eq-copilot/plugin/src/PluginProcessor.h:1909`
  `PipeClient pipe`, Pipename `EqCopilotIds.h:18`). **Probeeq hat keinen**:
  `eq-copilot/plugin/sonde/SondeProcessor.h:523` führt ausschließlich
  `nakama::ipc::ControlClient`. K3s eigene Frage ist damit beantwortet.

### Stärkstes gefundenes Gegenargument — es ist bereits registriert, und der Löser existiert
`docs/offene-punkte.md:212` (**NAK-188, offen**) ist wörtlich dieser Befund:
„Ein Link ohne auffindbare Sitzung sperrt alle Sitzungen — heute ohne
Produktlöser … gewollt fail-closed (§34.2) … Löst sich mit NAK-180, weil
`neutral_resync` den Platzhalter mitnimmt". NAK-180 ist geschlossen
(`docs/offene-punkte.md:484`, Abschnitt „Geschlossen" ab `:342`), und der Löser
ist heute im Produktweg vorhanden — K3 unterschätzt ihn:

* `befehl.rs:732-734`: `Some(false) if ist_erster_heartbeat => resync_bestaetigen`
  → `link.rs:480-494` → `intervention.rs:401-434`, und `:432` löscht den
  Platzhalter ausdrücklich.
* Zweiter Weg `link.rs:431-472` (`nachbericht_abschliessen_und_bestaetigen`),
  `:470` löscht den Platzhalter ebenfalls.
* Das gilt für **jeden** neuen v3-Link, nicht nur Gen — auch der erste neutrale
  Heartbeat einer neu verbundenen Probeeq entsperrt.

**Und jeder produktive Auslöser des v2-Drops ist im selben Rumpf mit einem
v3-Reconnect gepaart**: `eq-copilot/plugin/src/prozessor/State.cpp:64-65`
(`neueSensorId`: `pipe.reconnect(); controlV3.reconnect();`) und `:218-221`
(Projektreload: `pipe.start(); pipe.reconnect(); … controlV3.reconnect();`). Im
Reload- und Kennungswechselfall heilt sich der Platzhalter also über den ersten
Heartbeat des neuen Links.

### Rest, der übrig bleibt
Nur zwei schmale Fälle: (1) die **Reihenfolge-Race** v2-Drop gegen v3-Erst-
Heartbeat (landet der Drop später, bleibt der Platzhalter bis zum nächsten
Linkaufbau); (2) Gens **Destruktor**, in dem `controlV3.stop()`
(`PluginProcessor.cpp:464`) **vor** `pipe.stop()` (`:475`) läuft — dann gibt es
keinen eigenen v3-Neuaufbau mehr, und wenn eine andere Instanz (zweites Gen,
Probeeq) ihren Link stehend hält, bleibt deren Evidenz gesperrt, bis irgendein
Link neu aufbaut. Kein ACK- und kein Snapshotfeld nennt den Zustand.

Dieser Rest ist zudem in Wirkung und Abhilfe deckungsgleich mit
**T3-14-03** (`BEFUNDE.md:590`, „fehlt ein belegter produktiver Weg zur
Wiederfreigabe oder zum verständlichen Hinweis auf die sitzungsweite
Evidenzsperre") — andere Kante, gleiche Sanierung.

### Kategorie und Schwere
Als eigener Befund nicht tragfähig: DUPLIKAT zu NAK-188, Restanteil
Reihenfolge/Destruktor höchstens HÄRTUNG low. Der Vorschlag „LÜCKE medium" ist
zu hoch, weil der Produktlöser existiert und im Normalfall greift. Sinnvolle
Verwertung: NAK-188 um die v2-Kante und die Annahme „löst sich mit NAK-180"
korrigieren, statt eine neue Zeile zu führen.

---

## K4 — Replay vor dem ersten Heartbeat wird nach Brokerneustart endgültig `unauthorized`

**URTEIL: BESTÄTIGT** (mit Duplikatanteil) · **Kategorie: DEFEKT** ·
**Schwere: medium** (K1/K2)

### Reihenfolge selbst nachgezogen — sie stimmt
* `Verbindung.cpp:739` `inFlightNachReconnect(dieseWireGeneration)` läuft **vor**
  der Sendeschleife; es hängt mit `p0.einreihen` **hinten** an
  (`Nachrichten.cpp:258-282`, Zeile `:268`) — nicht `voranstellen`.
* Die Schleife entnimmt **genau einen** Eintrag je Runde und P0 strikt vor P1:
  `Verbindung.cpp:892-895`. Der erste Heartbeat wird erst in Runde 1 bei `:872`
  per `sendeP0` angehängt (`heartbeatSchritt` → `sendeP0`, `:272`) — also
  **hinter** allen Replays. Das `subscribe_session` (P1) kommt noch später.
* Nach Brokerneustart gibt es keinen geerbten Client:
  `link.rs:250` `bestaetigt: geerbt.as_ref().is_some_and(|c| c.bestaetigt)` →
  false; `bestaetigt` setzen nur `auto_join_locked`
  (`mitgliedschaft.rs:9-55`, gerufen aus `liveness.rs:481`) und
  `beitritt_bestaetigen_locked` (`mitgliedschaft.rs:119-133`). Führung entsteht
  erst über `fuehrung_neu_bewerten_locked` (`mitgliedschaft.rs:57-93`).
* `session_command` prüft **zuerst** Autorisierung, nicht Idempotenz:
  `befehl.rs:486-514` (`plugin_kind == "main"` + `bestaetigt` + führendes Main +
  `dispatch_fuer_link_erlaubt_locked`) → sonst `abgelehnt/unauthorized`. Der
  Store-Idempotenzblock aus `persistenz_p0` (`befehl.rs:185-215`) und der
  Vorbedingungsriegel (`:239-262`) liegen auf einem **anderen** Pfad und greifen
  hier nicht; das In-Memory-Register `session_commands` (`:529`) kommt erst nach
  der Abweisung.
* Gen sendet Bind/Remove genau so: `prozessor/Ipc.cpp:1330`/`:1351`
  (`sendePersistenzP0`, also im In-Flight-Register) und `:1650`.
* Client: `abgelehnt` gibt endgültig ohne Erfolg frei
  (`Nachrichten.cpp:362-363`, `:395-399`), und Gen löscht den ausstehenden
  Befehl **ohne jede Anzeige**: `prozessor/Ipc.cpp:1069-1077`.

### Stärkste Gegenargumente — und warum sie nicht reichen
1. **(c) Der Beweis existiert, ist aber blind.** Die K-01/K-02-Zusage
   (`SONDE-011.md:596-597`: „Broker-Kill trennt den echten C++-Client; er reiht
   dieselbe `command_id` wieder ein … antwortet `ergebnis = angewandt`") wird von
   `store_crash_matrix.rs:2211-2237` über `echte_cpp_client_crashrunde` belegt,
   und die Rohläufe zeigen `result=angewandt`
   (`docs/beweise/roh/G3-SOAK-7386741.md:702`). Nur: der dortige Befehl ist
   `preview_begin` (`eq-copilot/plugin/tests/IpcTestMain.cpp:215-227`
   `persistenzBefehl`), läuft also durch `persistenz_p0` mit Store-Idempotenz
   VOR jeder Link-/Rechtelage — und der Testclient setzt **keinen
   `statusProvider`** (`IpcTestMain.cpp:939-949`), sendet also nie einen
   Heartbeat. Das Bein kann den `session_command`-Riegel konstruktiv nicht sehen.
2. **(b) Andere Lesart.** O-01 (`:560`) erlaubt `abgelehnt` ausdrücklich als
   „endgültig beantwortet, aber nicht erfolgreich". Das entschärft die
   Antwortklasse, nicht den Fall: K-01/K-02 benennen für **genau diesen
   Killpunkt** `angewandt` als Erwartung. Und der harte Rest ist die
   **Stille**: der Auftrag verschwindet ohne Modell- oder UI-Wirkung.
3. **(d) Fenster.** Es braucht einen Brokerneustart oder eine Eviction
   (`liveness.rs:249-273`, `STALE_NACH_MS = 2500` + `TOMBSTONE_MS = 10000`, also
   ≥ 12,5 s Ausfall) mit einem `session_command` in Flight. Eng, aber nicht
   konstruiert — der Brokerkill ist genau das Szenario, für das die Matrix K-01
   bis K-03 baut. Die Knopfsperre bei getrennter Verbindung
   (`SourcesModel.cpp:555-585` setzt `Control::getrennt` und
   `brokerUnavailable`) verhindert **neue** Klicks, nicht das Replay eines
   vorher abgesetzten Befehls.
4. **(e) Duplikat, teilweise.** Die stille Verwurfshälfte ist buchstäblich
   T3-07-04 (`BEFUNDE.md:439`, `:536`) — dieselbe Zeile `Ipc.cpp:1073`, dieselbe
   Wirkung. Neu ist die Ursache: nicht eine veraltete Zielnonce, sondern der
   fehlende `bestaetigt`-/Führungszustand vor dem ersten Heartbeat.
   Empfehlung: als eigener Befund führen, mit ausdrücklichem Überlappungsvermerk
   zu T3-07-04 (gemeinsame Abhilfe: abgelehnte Sources-Aktionen ans Modell
   zurückgeben). NAK-120 (`docs/offene-punkte.md:113`) passt nicht — dort geht es
   um den fehlenden `MainProjectState`-Ingress für die Führungsrekonstruktion,
   nicht um das Replay-Fenster.

### Kategorie und Schwere
DEFEKT gegen K-01/K-02 bleibt. medium ist richtig: verlorener Bedienhandgriff
ohne Rückmeldung, aber kein Datenverlust, und der User kann nach dem nächsten
Heartbeat erneut drücken (bis er es merkt).

---

## K5 — Duplizierte Probeeq: Verdrängung, danach Sperre ohne Auflösungsweg

**URTEIL: BESTÄTIGT** (mit einer Korrektur an der Formulierung) ·
**Kategorie: DEFEKT** · **Schwere: medium** (K2/K8)

### Kette selbst nachgezogen
* Verdrängung samt Löschen von Messframes/Evidenz: `link.rs:131-194` (`:137-142`
  die Maps, `:151-192` Verdrängung des alten Links, Aufnahme in
  `zu_schliessende_links`).
* Kollision beim Report des verdrängten Links, Quarantäne beider Seiten,
  persistierter Riegel: `liveness.rs:337-399` (`kollisionsriegel_setzen_locked`,
  `guards_persistieren`), `link.rs:196-236`, `flush.rs:235-245`.
* Riegel wird beim Start **vor** jedem Routing restauriert: `mod.rs:316-320`.
  `effektive_adresse` ist `logon_sid + project_binding_id + session_epoch +
  instance_id` (`mod.rs:457-462`) — **ohne** `runtime_nonce`, überlebt also jeden
  Brokerneustart, wie C-07 zusagt.
* Danach gesperrt: Dispatch/P2 (`flush.rs:289-298`, `senke.rs:149-155`), Push
  (`flush.rs:112-121`, `subscription.rs:418-442`), Auto-Join
  (`mitgliedschaft.rs:17`).
* Quarantäne überlebt das Verbindungsende ausdrücklich:
  `broker/src/instance_alias.rs:221-228`. Einzige Löser
  `quarantaene_aufloesen` (`:302`) und `quarantaene_aufloesen_ueberall` (`:336`),
  erreichbar nur aus `konflikt_guard_aufloesen` (`flush.rs:247-279`) — und dessen
  **einzige Aufrufer sind Tests**: `store_crash_matrix.rs:2144`, `:2171`, `:3962`
  (Suche über `broker/src` und `eq-copilot` ohne Produkttreffer).
* Probeeq kann nichts auflösen: `eq-copilot/plugin/sonde/SondeProcessor.h:158-159`
  (`createEditor() → nullptr`, `hasEditor() → false`); `neueSensorId` existiert nur
  am Gen-Prozessor (`eq-copilot/plugin/src/PluginProcessor.h:148`,
  `prozessor/State.cpp:54-67`, Knopf `PluginEditor.cpp:591`).
* Das Brokersignal ist da, **hat aber keinen Verbraucher**:
  `heartbeat_ack.duplicate_instance_id` (`befehl.rs:760-766`) kommt im
  Produktcode nirgends an — einzige Treffer liegen in
  `eq-copilot/plugin/tests/V3TestServer.h:484`, `:506`, `:636`.
* Reproduzierbarkeit der Ursache: die `instance_id` kommt aus dem gespeicherten
  Zustand (`sonde/SondeProcessor.cpp:1087` liest `zustand.common.instanceId`,
  Persistenz `:990-994` / `:1017-1039`), und §32.1 nennt das Absicht („der State
  IST der Messpunkt"). Ein FL-Klon einer Probeeq erzeugt also zwangsläufig zwei
  gleiche IDs.

### Gebrochene Zusage — sie ist ausdrücklich
* `eq-copilot/schemas/state/nakama-state-v2.md:269` §7: „Der Broker meldet die
  zweite lebende Verbindung (`heartbeat_ack{konflikt}`), **der User löst sichtbar
  auf** (`neueSensorId` ⇒ neue ID, Label/Paar bleiben, Host-Dirty)."
* `docs/FL-Nakama-Sonden-Design-Entwurf.md:1841`: „`instance_id` … Duplikate
  werden sichtbar aufgelöst"; `:1857-1859`: „Bei Bridge, zwei offenen Projekten
  oder duplizierten IDs ist eine kurze sichtbare Bestätigung Pflicht."

Für die Probeeq-Rolle existiert **keine** der beiden Hälften: kein Melder, kein
Handgriff. C-07/C-10 (`SONDE-011.md:513`, `:516`) sind kein Gegenargument — sie
sagen die sticky Sperre zu und benennen „Auflösung ausschließlich über explizite
Neu-ID im Phase-A-Weg" als den Weg, den es hier nicht gibt. Beanstandet ist also
nicht die Sperre, sondern der fehlende zugesagte Rückweg.

**Verschärfung, selbst gefunden:** Gens `confirm_join` auf die quarantänisierte
Sonde quittiert `angewandt` und setzt `bestaetigt = true`, **ohne** Riegel oder
Quarantäne zu prüfen — `befehl.rs:549-579` findet das Ziel (der Client existiert
weiter, er ist nur gesperrt), `mitgliedschaft.rs:119-133` schaltet ihn scharf,
`befehl.rs:615` antwortet `angewandt`. Dispatch und Push bleiben gesperrt. Das
ist ein sichtbarer Handgriff, der weder wirkt noch ehrlich meldet — genau
`CLAUDE.md:51-52`. Die Anzeige „Join confirmation required — choose a source and
bind it" (`SourcesModel.cpp:1356-1360`, `PluginEditor.cpp:1208-1211`) führt den
User dazu hin.

### Korrektur an K5s Formulierung (ehrlich gegen den Kandidaten)
1. „**endlose Verdrängung**" ist zu stark: den verdrängten Link schließt schon
   das Hello über `ControlAnmeldung.zu_schliessende_links`
   (`senke.rs:15-20`, `link.rs:180`/`:317-320`), nicht erst der 100-ms-Tick — das
   Reportfenster je Runde ist klein. Weil aber der Client-Backoff nach jedem
   erfolgreichen Welcome zurückgesetzt wird (`Verbindung.cpp:451-452`, 500 ms),
   wiederholen sich die Runden unbegrenzt und die Kollision wird über die Zeit
   praktisch sicher. Der Befund hängt nicht an „endlos", sondern am fehlenden
   Rückweg.
2. „**danach dauerhaft**" ist zu pauschal: der persistierte Riegel schlüsselt über
   `session_epoch`, und die ist flüchtig pro geöffneter Projektkopie
   (`Entwurf:1841` Tabelle; in `nakama-state-v2.md` §8 ausdrücklich **nicht** im
   State). Ein Schließen und erneutes Öffnen des Projekts erzeugt eine neue
   Epoche, der Riegelschlüssel passt nicht mehr, und die In-Memory-Quarantäne ist
   ohnehin weg. Die Ursache bleibt aber (beide Klone tragen weiter dieselbe
   `instance_id`), also kehrt die Sperre zurück, sobald der verdrängte Link
   wieder meldet. Richtige Fassung: **innerhalb einer Sitzung unauflösbar, je
   Sitzung wiederkehrend** — kein dokumentierter, kein auffindbarer und kein
   sichtbarer Handgriff.

### Duplikat?
Nein. NAK-138 (`docs/offene-punkte.md:170`) ist eine **Test**lücke („Die
Duplicate-Kette hat keinen Ende-zu-Ende-Test"), nicht der fehlende
Auflösungsweg. NAK-121 ist geschlossen (`:477`) und hat nur Riegel und Alias
gleichlaufen lassen (H-14, Kommentar `flush.rs:258-278`), ohne einen
Produktaufrufer zu schaffen. Kein T3-Befund berührt Alias/Quarantäne.

### Kategorie und Schwere
DEFEKT gegen `nakama-state-v2.md:269` §7 und Entwurf `:1841`/`:1857-1859`.
medium: eine duplizierte Probeeq ist stumm und nicht wieder in Betrieb zu nehmen,
zusätzlich lügt ein Gen-Handgriff; aber Audio bleibt unberührt, kein
Datenverlust, und ein Projektneuöffnen bringt die Sonde vorübergehend zurück.

---

## Ergebnistabelle

| Kandidat | Urteil | Kategorie | Schwere | Kernbegründung |
|---|---|---|---|---|
| K1 | BESTÄTIGT | DEFEKT | high | Trennregel und Bein (`store_crash_matrix.rs:1558`) decken S-03 Satz 1; weil der Verursacher der 1-Hz-Heartbeat ist (`liveness.rs:505-506`, `:535-537`), steht der Betrieb dauerhaft und S-03 Satz 2 / L-08 / O-01 („P2 läuft weiter") sind nicht erfüllbar. Zusätzlich erreicht die ehrliche Degradationsanzeige Gen nie, weil P0-Heartbeat vor P1-Subscribe geht (`Verbindung.cpp:892-895`, `subscription.rs:93`). |
| K2 | HERABGESTUFT | HÄRTUNG | low | Sperre, brokerweite Reichweite und fehlender Zeitablauf sind durch C-07/S-01 und ein Bein (`store_crash_matrix.rs:1156`) gedeckt; der Dauerfall degradiert den Writer und landet über die Ausnahmen `subscription.rs:94`/`:427` ehrlich als `storeDegraded`. Rest ist nur der ehrliche Zustand bei gesundem Store und transientem Lesefehler. |
| K3 | DUPLIKAT | HÄRTUNG | low | NAK-188 (`docs/offene-punkte.md:212`) führt den Befund wörtlich; der Löser existiert heute produktiv (`befehl.rs:732-734` → `intervention.rs:432`, `link.rs:470`) und jeder v2-Drop ist im selben Rumpf mit einem v3-Reconnect gepaart (`State.cpp:64-65`, `:218-221`). Rest: Race und Gen-Destruktor, deckungsgleich mit T3-14-03. |
| K4 | BESTÄTIGT | DEFEKT | medium | Replay steht per FIFO vor dem ersten Heartbeat (`Verbindung.cpp:739`/`:892-895`, `Nachrichten.cpp:268`), `session_command` prüft Autorisierung vor allem anderen (`befehl.rs:486-514`), `bestaetigt` entsteht erst im Heartbeat (`liveness.rs:481`, `link.rs:250`). K-01/K-02 sagen `angewandt` zu; das belegende Bein nutzt `preview_begin` und einen Client ohne Heartbeat und ist blind. Stille Verwurfshälfte überlappt T3-07-04. |
| K5 | BESTÄTIGT | DEFEKT | medium | `konflikt_guard_aufloesen` hat nur Testaufrufer (`store_crash_matrix.rs:2144`, `:2171`, `:3962`), Quarantäne überlebt jedes Verbindungsende (`instance_alias.rs:221-228`), Probeeq hat keinen Editor (`SondeProcessor.h:158-159`) und kein `neueSensorId`, `duplicate_instance_id` hat keinen Produktverbraucher. Bricht `nakama-state-v2.md:269` §7 und Entwurf `:1841`/`:1857-1859`. Gens `confirm_join` quittiert dabei positiv ohne Wirkung (`befehl.rs:549-615`). |

## Gelesener Scope

Kontext: `CLAUDE.md`, `AGENTS.md`.

Broker (`broker/src/`): `coordinator/flush.rs` (ganz), `coordinator/liveness.rs`
(ganz), `coordinator/intervention.rs` (ganz), `coordinator/subscription.rs`
(ganz), `coordinator/senke.rs` (ganz), `coordinator/link.rs:100-630`,
`coordinator/mitgliedschaft.rs:9-160`, `coordinator/befehl.rs:150-270`,
`:470-620`, `:700-770`, `coordinator/mod.rs:230-470`, `coordinator/zustand.rs`
(Setzerstellen über Suche), `lebenslauf.rs` (ganz), `server.rs:330-390`,
`:735-785`, `store/handle.rs:150-300`, `:483-508`, `store/writer.rs`
(Degradationsstellen), `instance_alias.rs` (Quarantänestellen).

Plugin (`eq-copilot/plugin/`): `core/ipc/controlclient/Verbindung.cpp:215-380`,
`:440-490`, `:690-975`, `core/ipc/controlclient/Nachrichten.cpp:240-415`,
`core/ipc/ControlClient.h` (Konstanten), `core/ipc/IpcVerbindung.h` (Backoff),
`src/PluginProcessor.cpp:420-485`, `src/prozessor/State.cpp:40-80`, `:190-230`,
`src/prozessor/Ipc.cpp:1040-1135`, `:1330-1360` (Suche), `src/SourcesModel.cpp:530-610`,
`:1345-1370`, `src/PipeClient.h`/`.cpp` (Lebenszyklus, Suche),
`src/EqCopilotIds.h:18`, `sonde/SondeProcessor.h:150-170`, `:520-525`,
`sonde/SondeProcessor.cpp:130-270`, `:960-1090`,
`tests/IpcTestMain.cpp:215-227`, `:855-975`.

Tests: `broker/tests/store_crash_matrix.rs:1140-1180`, `:1500-1650`,
`:2205-2250`; Testinventar über `docs/audits/2026-09-15-tiefenaudit/roh/phase-09-beweisluecken.md:1108`.

Zusagen und Register: `docs/beweise/SONDE-011.md` B.2/B.3/B.4-Matrizen
(C-01 bis C-10, E-01 bis E-06, S-01 bis S-07), B.5 (O-01), K-01 bis K-05,
Matrixzeile `:1061`; `docs/FL-Nakama-Sonden-Design-Entwurf.md:1835-1862`;
`eq-copilot/schemas/state/nakama-state-v2.md:262-276`;
`docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md` (T3-Tabellen, gezielte Suche);
`docs/offene-punkte.md` (NAK-120, 121, 125, 126, 138, 164, 165, 180, 188, 274 und
Abschnittsgrenzen `:106`/`:342`); `docs/beweise/roh/G3-SOAK-7386741.md:700-705`.

Nicht gelesen (außerhalb): Sicherheitsthemen (Pipe-ACL, Impersonation, Token,
v2-Fristen), `design/**`, `broker/src/transport/server_v3/**` bis auf die
Suche nach `verbindung_soll_trennen` (kein Aufrufer im Transport — Durchsetzung
allein über den 100-ms-Tick).
