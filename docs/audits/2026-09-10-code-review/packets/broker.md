# Auditpaket Broker – 10.09.2026

Quellstand: `844b9c15935377e89d124ac33848e9cba1ecf4c2`, isolierter Snapshot `C:\na-audit-20260910-844b9c1`. Prüfung ausschließlich lesend; einzig dieses Berichtspaket wurde geschrieben. Kein Build, Cargo-, Clippy-, Server-, Host-, Last- oder Laufzeittest wurde ausgeführt. Statische Defekte sind keine gemessenen Performancefehler. Alle genannten Laufzeitbeweise: **NOT RUN**.

Methodik: AGENTS.md und CLAUDE.md gelesen, `nakama-plugin-quality`, dessen Qualitätsmodell und `sondenplan-audit` angewendet. Tiefenprofil für Transport, Ressourcen, Save/Restore und gekoppelte Lebenszyklen. Vorhandene Tests zur Gegenprüfung gelesen. Kein pauschales Qualitäts- oder Releaseurteil.

## B-01 – Neuere Sessionsnapshots können durch ältere ersetzt werden

**Status:** statisch bestätigter Defekt; Prioritätsvorschlag P1 für irreführende Zustands-/Befundanzeige. Kein Nachweis einer ungefragten Audioveränderung.

**Tragende Stellen:**

- `broker/src/coordinator/flush.rs:31–36`: Capture und Storecommit werden über ein Sessionshard serialisiert.
- `broker/src/coordinator/flush.rs:141–147`: Das Schloss wird vor dem Writer-Enqueue freigegeben; anschließend wird der bereits erfasste Payload gesendet.
- `broker/src/transport/server_v3/griff.rs:72–90`: Der Sender akzeptiert Payloadbytes ohne monotone Snapshotfolge; alle werden mit dem Schlüssel `session_snapshot` eingereiht.
- `broker/src/transport/server_v3/queues.rs:164–179`: Ein später eingereihter Snapshot ersetzt einen wartenden Snapshot desselben Schlüssels unabhängig von seinem Alter.
- `broker/src/coordinator/sicht.rs:576–585`: Der Snapshot trägt Session und Broker-Epoche, aber keine eigene monotone Revision.
- Gegenverbraucher `eq-copilot/plugin/src/SourcesModel.cpp:1152–1163` und `:1252–1255`: Nach Bindungs-/Sessionprüfung werden Mitglieder, Experimente, Paare und Befunde vollständig ersetzt. Der separate State-/IPC-Prüfer hat diese Gegenrichtung unabhängig bestätigt; sie wurde für diesen Befund zusätzlich direkt gelesen.

**Auslösekette:** A erfasst und committet S1, gibt das Shardschloss frei und wird vor dem Enqueue unterbrochen. B erfasst, committet und reiht S2 ein. A reiht danach S1 ein. S1 wird entweder nach S2 ausgeliefert oder ersetzt das noch wartende S2 direkt in der Queue. Gen akzeptiert S1 wieder vollständig. Der Store kann dabei korrekt auf S2 stehen und seine Outboxschuld durch den erfolgreichen S2-Write bereits getilgt haben.

**Schaden:** Eine Quelle kann wieder verbunden erscheinen, ein neueres Experimentresultat verschwinden oder ein inzwischen veralteter Befund wieder als aktuell dargestellt werden. Der reguläre nächste Snapshot kann das korrigieren; die Fehlanzeige ist dadurch weder verhindert noch zwingend sofort korrigiert.

**Gegenbeweisversuch:** `broker/tests/store_crash_matrix.rs:1155` (`snapshot_commit_bleibt_bei_konkurrierenden_flushes_monoton`) hält einen Flush vor dem Commit an und prüft danach ausschließlich die SQLite-Sessionprojektion. Das beweist die bereits vorhandene Commitordnung, nicht die Ordnung der ausgelieferten Payloads. Der Cleanup-Race-Test ab `:1210` prüft ungültig gewordene Adressaten, nicht zwei weiterhin gültige Snapshots. Der Grund für die Schlossfreigabe ist berechtigt: externe/reentrante Senken sollen nicht unter dem Sessionschloss laufen. Eine Reparatur muss diese Eigenschaft erhalten, etwa mit einer geordneten, begrenzten Zustellung außerhalb des Locks.

**Minimaler Verhaltenstest:** Ein `SessionPush` mit Barriere hält den alten Payload vor seiner Aufnahme fest. Parallel wird eine zweite Descriptoränderung bis zur Zustellung abgeschlossen; danach wird der alte Payload freigegeben. Empfangene Labels und anschließend das echte SourcesModel prüfen. Zusätzlich den wartenden Writerfall testen, in dem das alte Enqueue den neueren Queueeintrag ersetzt. Bestehende Tests müssen dabei weiterhin reentrante Senken erlauben.

## B-02 – Historische Daten umgehen die flüchtigen Ressourcenobergrenzen

**Status:** statisch bestätigtes unbeschränktes Wachstum und unbeschränkter Restore-Peak; P2. Ein konkretes RAM-, CPU- oder Startzeitbudget wurde nicht gemessen.

**Tragende Stellen:**

- `broker/src/store/handle.rs:301–318`: `domaene_lesen` liest globale Tabellen ohne LIMIT und sammelt alle Payloadbytes als `Vec<Vec<u8>>`.
- `broker/src/store/handle.rs:380–390`: Dasselbe gilt für die gesamte Experiment-Ereignishistorie.
- `broker/src/coordinator/experiment_verdrahtung.rs:1374–1419`: Passagen, Experimente und die Transitionshistorie werden vollständig in weitere Speicherstrukturen überführt.
- `broker/src/coordinator/experiment_verdrahtung.rs:1425–1458`: Sämtliche historischen Evidence-Zeilen werden gelesen; die Retention beschränkt nur die Länge je `ClientKey`, nicht die Anzahl historischer Keys.
- `broker/src/coordinator/experiment.rs:882–890`, `:904–905`: Wiederherstellung übernimmt alle Passagen, Experimente und Logeinträge ohne Speicherdeckel.
- `broker/src/coordinator/experiment.rs:1058–1096`: Ein reguläres neues Experiment fügt Passage, Experiment und Logereignisse hinzu.
- `broker/src/coordinator/experiment.rs:1105–1137`: `N_PROJEKT` und `N_GLOBAL` begrenzen ausschließlich **offene** Experimente; ein verdrängtes/geschlossenes Experiment bleibt im Bestand.
- `broker/src/coordinator/liveness.rs:99–109`: Evidence wird zusammen mit einem tatsächlich entfernten Client bereinigt. Die beim Start restaurierten historischen Keys haben zunächst keinen zugehörigen Eintrag in `stand.clients`; dieser Cleanup kann ihre Anzahl daher nicht begrenzen.
- `broker/src/coordinator/sicht.rs:609–618`: Die Grenze von 32 Experimenten beschränkt den versendeten Snapshot erst nach Sammlung/Sortierung, nicht den gehaltenen Bestand.

**Auslösekette:** Viele normale Projekte/Sitzungen oder lange Arbeit mit neuen Experimenten vergrößern dauerhaft den Store. Neustart liest die gesamte bisherige Historie zunächst als Bytevektoren und hält danach alle historischen Experimente/Passagen/Transitions sowie bis zu 32 Evidence-Zusammenfassungen für jeden jemals gespeicherten ClientKey. Der übliche Runtime-Deckel für 128 Clients wirkt auf diese restaurierten, verwaisten Keys nicht. Auch ohne Neustart bleiben geschlossene Experimente und das RAM-Log erhalten.

**Schaden:** Speicherverbrauch und Startzeit wachsen mit der bisherigen Nutzung statt mit den aktiven Instanzen. Aufnahmen werden unter anderem über `stand.evidenz.keys()` gesammelt (`hypothese_verdrahtung.rs:130–140`); historische Keys vergrößern damit auch spätere Verwaltungsarbeit. Die dauerhafte Aufbewahrung benannter Experimente im Store ist sinnvoll und ausdrücklich vorgesehen. Daraus folgt keine Notwendigkeit, alle historischen Daten dauerhaft im Broker-RAM vorzuhalten.

**Gegenbeweisversuch:** Evidence-Retentiontests `broker/tests/sonde013_verdrahtung.rs:414` und `:448` messen eine begrenzte Historie je Quelle. Experimentdeckeltests messen offene Versuche. Restoretests ab `:2055`, `:3556`, `:3630` prüfen Erhalt einzelner Versuche/Ereignisse/Reihenfolgen. Der Snapshotdeckeltest ab `:3061` prüft die Wireliste und kommentiert ausdrücklich den wachsenden terminalen Bestand. Keine dieser gelesenen Prüfungen deckelt Restore-RAM oder historisch restaurierte ClientKeys.

**Minimaler Beweisweg:** Einen Store mit vielen nacheinander entstandenen, inzwischen getrennten Sessions und mehreren abgeschlossenen Experimenten pro Session über die bestehenden Produktpfade erzeugen. Danach den Brokerbestand nach Restore sowie Peak/Steady-RAM messen. Die Anzahl aktiver Clients bleibt konstant. Zusätzlich Byte-/Zeilenzähler an der Restore-Grenze prüfen, damit ein nachträgliches Wegwerfen nach vollständigem Laden nicht als begrenzter Restore gilt. Reparaturrichtung: seitenweises Lesen und gezielte, begrenzte Caches; dauerhaftes Archiv weiter im Store erhalten.

## B-03 – Ereignisrücknahmen teilen sich den Queue-Schlüssel mit absoluten Snapshots

**Status:** statisch bestätigte Verdrängung zwischen verschiedenen Nachrichtenarten; P2. Persistent gespeicherte Schuld verhindert in vielen Fällen den endgültigen Verlust, verhindert aber nicht den falschen bzw. verspäteten Livezustand.

**Tragende Stellen:** `broker/src/coordinator/invalidierung_verdrahtung.rs:384–405` stellt `evidence_invalidate` über `SessionPush::snapshot_schreiben` zu. `broker/src/transport/server_v3/griff.rs:86` ordnet auch diesen Payload immer dem Schlüssel `session_snapshot` zu. `broker/src/transport/server_v3/queues.rs:164–179` ersetzt danach eine wartende Rücknahme durch einen neuen Sessionsnapshot. Der alte Aufrufer erhält `false` (`queues.rs:211–219`) und die Store-Schuld bleibt stehen. Der Wiederholungsweg läuft bei Resubscribe oder dem nächsten angenommenen Evidence-Takt (`subscription.rs:202`, `evidenz.rs:247`).

**Konkrete Wirkung im heutigen Gegenverbraucher:** Der State-/IPC-Prüfer hat bestätigt: `SourcesModel.cpp:1421–1424` invalidiert bei einer Rücknahme die Messachse; `:1435–1438` markiert Befunde stale und zählt/merkt die Rücknahme. Ein normaler Snapshot übernimmt bei gleicher Epoch/Nonce vorherige Messwerte (`:1171–1174`) und ersetzt diese Wirkung nicht allgemein. Ohne folgenden Evidence-Takt oder Resubscribe kann die Rücknahme daher liegenbleiben, obwohl ein gewöhnlicher Snapshot zugestellt wurde.

**Gegenbeweisversuch:** `invalidierung_wird_dem_abonnenten_wirklich_zugestellt` (`broker/tests/sonde013_verdrahtung.rs:2974`) misst einen Store plus unmittelbar annehmende `PushProbe`, nicht die echte Writerkoaleszierung. `sonde014_m75_ohne_zustellung_bleibt_die_schuld_stehen` ab `:6008` belegt die wichtige positive Gegenrichtung: Schuld wird nicht ohne erfolgreichen Write gelöscht und der nächste Evidence-Takt kann sie abtragen. Dieser Test stellt keine gleichzeitig eingereihten, verschiedenen Familien in die echte Queue.

**Minimaler Verhaltenstest:** Writer anhalten; eine gültige `evidence_invalidate` einreihen; danach einen `session_snapshot` derselben Verbindung einreihen; Writer freigeben. Prüfen, dass die Rücknahme mit ihrer eigenständigen Wirkung den Consumer erreicht oder dass eine ausdrücklich absolute Ersatznachricht diese Wirkung vollständig enthält. Eine Nachrichtenart darf nicht allein deshalb als Snapshot behandelt werden, weil sie denselben Byte-Push verwendet.

**Abgrenzung:** Auch die persistente Outbox verwendet für sämtliche Invalidierungen einer Zielinstanz nur `evidence_invalidate` (`invalidierung_verdrahtung.rs:219`, Writer-UPSERT `writer.rs:480–484`). Zwei disjunkte Scopes können dort einander ersetzen. Dies wird hier nicht als zusätzlicher bestätigter Gen-Schaden gezählt: Der heutige C++-Consumer invalidiert bei jeder Rücknahme konservativ global; eine zugestellte letzte Rücknahme reicht dort für den aktuellen Zustand. Der gesonderte Scope-/Replayvertrag verdient einen Test, ist aber kein zweiter Beweis derselben Wirkung.

## V-01 – Alte Hypothesenrechnung kann eine jüngere überholen

**Status:** konkreter Race-Verdacht, nicht ausgeführter Verhaltenstest; Priorität erst nach Gegenbeweis festlegen.

`hypothese_verdrahtung.rs:72–104` erfasst Eingänge, rechnet außerhalb des Standlocks und übernimmt das Ergebnis später. `befunde_eintragen` (`:443–475`) prüft dabei `ergebnis_ist_noch_gueltig` (`:961–996`). Diese Prüfung vergleicht die Intentgeneration und prüft nur, ob die benutzten Evidence-IDs noch nicht invalidiert wurden. Sie trägt keine monotone Eingangs-/Rechenfolge und vergleicht auch nicht den gesamten erfassten Client-, Leader-, Descriptor- oder Passagenstand. Der Sonderfall leerer Ergebnisse wird ohne weitere Prüfung akzeptiert (`:966–968`).

**Zu falsifizierende Kette:** A rechnet mit alter Evidenz. Währenddessen wird zusätzliche Evidenz angenommen und B veröffentlicht ein anderes, aktuelleres Ergebnis. Alte Evidence-IDs sind noch innerhalb der 32er-Retention, der Intent hat sich nicht geändert. A kehrt zurück und kann B ersetzen, weil der aktuelle Riegel die Eingangsänderung nicht erkennt. Auch ein Livenesswechsel ist ein Testziel: `liveness.rs:211–222` setzt stale, entfernt die alte Evidence aber erst bei der späteren Clienteviction. Damit können die IDs trotz geänderter Quellverfügbarkeit weiter als gültig gelten.

**Gegenargumente und Grenzen:** Die aktuelle Prüfung verhindert zwei wichtige reale Fälle bereits: Intentwechsel und explizite Evidence-Invalidierung. Die vorhandenen Tests `veraltetes_rechenergebnis_wird_nicht_veroeffentlicht` (`broker/tests/sonde014_verdrahtung.rs:530ff`, Haken `:575` und `:631`) provozieren genau diese Fälle. Neue Daten machen ältere Evidenz nicht automatisch ungültig; ein älterer fachlich weiterhin begründeter Befund ist deshalb nicht allein aufgrund seines Alters falsch. Für den Defektbeweis muss die zweite Rechnung eine konkrete neuere Aussage oder Enthaltung liefern, die die erste nicht mehr ersetzen darf. Diese vollständige inhaltliche Gegenprobe wurde in diesem Audit nicht ausgeführt.

**Minimaler Test:** Die vorhandene einmalige `rechen_test_haken_setzen`-Naht nutzen. Im Haken zusätzliche, konkret anders auswertbare Evidence über den Produktionspfad zuführen und deren Ergebnis erfassen. Nach Rückkehr des älteren Aufrufs prüfen, ob dieses Ergebnis wieder überschrieben wurde. Kontrollfälle: identische zusätzliche Evidence, tatsächlich invalidierte alte IDs, neue Intentgeneration. Kein Test einer kopierten Hypothesenimplementierung.

## Weitere Beobachtungen und bewusst nicht beförderte Kandidaten

- Wiederherstellung verwendet bei SQL-/Dekodierfehlern mehrfach `unwrap_or_default`/`filter_map(...ok())` (`experiment_verdrahtung.rs:1374–1408`, `:1425–1436`), während `Coordinator::mit_store` die Routingfreigabe bereits aus dem Writerstart ableitet (`coordinator/mod.rs:296ff`). Ein gezielt injizierter Restore-Lesefehler sollte belegen, ob und wie fehlende historische Daten sichtbar gemeldet werden. Ohne konkreten Fehlerfall hier als Nachweislücke, nicht als behaupteter Crashverlust.
- `vorschlaege_bilden` könnte bei fehlgeschlagener Persistierung mit `None` weiterhin in `draft_offer_zustellen` laufen (`proposal_verdrahtung.rs:84–86`). **Nicht als heutiger Defekt geführt:** Die echte `Proposallage` setzt `capability_vorhanden: false` (`:108`); die aktuelle Phase erzeugt dadurch kein echtes Draftangebot. Zukünftige SONDE-015-/Aktuationsarbeit wird nicht vorweg als defekt bewertet.
- Der bereits registrierte Rollback über verschiebbare Evidence-Indizes bleibt als Risiko sichtbar (`invalidierung_verdrahtung.rs:237–240`, `docs/offene-punkte.md` NAK-163). Er ist kein neuer Fund dieses Pakets und wird nicht doppelt gezählt.
- Rust-DSP-DTO-Zonenprüfung wurde für den C++-Gegenprüfer gelesen: `dto.rs:461–474` wandelt in i64 und weist anschließend außerhalb 0..8 ab; der C++-Kandidat `2147483648` hat hier keinen äquivalenten undefinierten Cast. Keine Rust-UB-Behauptung.
- Der Transport begrenzt Envelopebytes, Ingressplätze und Ingressbytes, verwendet getrennte P0-/Restverbraucher und stellt Pipe-I/O außerhalb des Coordinator-Standlocks. Diese Eigenschaften sind aus dem Code erkennbar; ihre Timing-/Mehrinstanzwirkung wurde nicht gemessen. Ein grüner Satz zu Audio-Deadlines oder 1 Gen plus 10/32 Probeeq wäre daher unzulässig.
- Bei der gelesenen Lockhierarchie wurde keine belegte zyklische Sperrfolge gefunden: Stand -> Alias und Stand -> Store-Sicht stehen hinter getrennten Besitzmodellen; die gelesenen Storepfade rufen nicht zurück in den Coordinator. Das ist kein vollständiger Deadlocknachweis für alle Brokerpfade.

## Coverage und Beweisgrenze

**Tief gelesene Produktionspfade:** V3-Envelopeprüfung und Writerformat, Serverqueue und V3Sender, vollständiger Verbindungsaufbau/Reader-/Writer-/Consumer-/Abbaupfad in `server_v3/verbindung.rs`; StoreHandle samt Restorelesern, Writerstart/-stop/-batch/-commit-/Outboxpfad; Coordinator-Capture/Flush/Push, Subscription/Replays, Evidenceannahme und Persistenzgrenze, Invalidierung samt Rückweg, Restore, Experimentbestand/Retention, Hypothesenerfassung/-übernahme/-Storehärtung, Liveness/Clienteviction, relevante SourcesModel-Gegenrichtung.

**Nur gezielt gescannt oder teilweise gelesen:** übrige Win32-Listener-/Handle-/Authhelfer; komplette Aliasregistrierung; allgemeiner Rust-Schema-/DTO-Prüfer; FlatBuffers-Telemetrieprüfer (Funktions-/Grenzen-/Finite-Scan, nicht gesamte Feld-für-Feld-Prüfung); DSP-/Hypothesenmathematik, Klassenwahl, Maskierung, PRE/POST, komplette Experimentkommandos, v2-Server und -Register. Insbesondere keine Zusage, sämtliche Rustzeilen einzeln geprüft zu haben.

**Gelesene Prüfbeispiele:** Snapshot-Commitmonotonie und Cleanup-Race aus `store_crash_matrix`; Evidence-Retention, Experiment-/Evidence-Restore, Rücknahmezustellung und Outboxschuld aus `sonde013_verdrahtung`; Intent-/Invalidierungs-Races aus `sonde014_verdrahtung`; Transportprioritäten und Queueeinheitstests. Ganze Testsuiten wurden weder vollständig semantisch auditiert noch ausgeführt.

**Auditbelege dieser Session:** Alle Quell-/Testfeststellungen stammen aus dem isolierten Snapshot. `git status --short` war vor dem Bericht ohne Ausgabe. Builds, reale Pipetests, Audiohost, Reconnect-/Suspend-/Soak, Speicherprofil und Performanceverteilungen bleiben **NOT RUN**. Root führt gemeinsame Preflight-/Inventarbelege und den integrierten Bericht.
