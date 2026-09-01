# NAK-123 — Rohausgabe der E-5-Bruchproben (Stand 0f92c88)

Prueflistenzeile E-5 verlangt fuer jede neu benannte Pruefung einen absichtlich
erzeugten Rotlauf. Jeder Fall unten kippt GENAU EINE Produktstelle, faehrt die
benannten Tests, und nimmt die Kippung danach zurueck. Kein gekippter Stand
wurde committet; der Arbeitsbaum war vor und nach jedem Fall sauber.

Rust: `cargo test --manifest-path broker/Cargo.toml --test <ziel> -- --exact <tests>`.
C++: `cmake --build eq-copilot/build --config Release --target <ziel>` und danach
`eq-copilot/build/plugin/<ziel>_artefacts/Release/<ziel>.exe`.

Bei den C++-Laeufen sind die bestandenen `ok`-Zeilen zu einer Zahl je Binary
zusammengefasst; jede `FEHLER`-Zeile und jede Urteilszeile steht woertlich.
Die Rust-Ausgaben stehen vollstaendig.

## Uebersicht

| Fall | Matrix | Gekippte Stelle | Urteil |
|---|---|---|---|
| R1 | A-02, A-08 | ListenerInstanz::neu ohne FILE_FLAG_FIRST_PIPE_INSTANCE fuer die erste Instanz | ROT |
| R2 | A-01 | zweiter Listener wird erzeugt, aber nie bewaffnet (Zaehler meldet 1 statt 2) | ROT |
| R3 | A-01 | Startfehlerkante ZweiteInstanz gibt das erste Handle nicht per RAII frei | ROT |
| R4 | A-03 | Worker-Uebergabefenster liegt VOR der Armierung des Ersatzlisteners | ROT |
| R5 | A-04 | Ersatzlistener-Fehler wird verschluckt: kein listener_fehler, kein fail-closed | ROT |
| R6 | A-05 | Worker-Cap um 4 angehoben: der 97. Client erreicht einen Worker | ROT |
| R7 | A-07 | Broker besitzt einen anderen Pipenamen als den geforderten | ROT |
| R8 | A-09 | Besitzlistener fallen VOR Acceptor-, Wachhund- und Worker-Joins | ROT |
| R9 | A-06 | aktive_worker() meldet immer 0: ein verbundener Client verhindert das Selbstende nicht | ROT |
| R10 | B-01, B-02, B-03, B-04, B-05, B-08, B-09 | client_sid_authentisieren nimmt jede Verbindung ohne Impersonation, SID-Vergleich und Revert an | ROT |
| R11 | B-06 | Impersonation-/SID-Kette laeuft VOR dem Bootstrap-Read | ROT |
| R12 | B-07 | RevertToSelf-Fehler beendet den Prozess nicht mehr fail-fast | ROT |
| C1 | C-02, C-03, C-04, C-05, C-06, C-07, C-08, C-09, C-10 | namedPipeServerAuthentisieren meldet jeden Server sofort als verifiziert (kein PID-Pin, keine SID, kein Prozessbild, keine Dateiidentitaet, kein SHA-256, keine Signatur) | ROT |
| C2 | C-01 | fehlende Pipe (ERROR_FILE_NOT_FOUND) wird als Sicherheitsfehler belegtAberUnverifiziert gemeldet statt als nichtDa/pipeFehlt | ROT |
| C3 | C-11 | processBlock ruft brokerLifecycleSnapshot() und nimmt damit eine Lifecycle-Sperre im Audiothread | ROT |
| C4 | C-06 | TelemetryClient uebernimmt die Freigabe der Controlverbindung und fuehrt/zaehlt keine eigene Handlepruefung mehr | ROT |
| C5 | C-07 | BrokerLifecycle behandelt belegtAberUnverifiziert nicht mehr fail-closed, sondern faellt in den normalen Spawn-/Retryweg | ROT |
| C6 | C-07 | vorhandenePipeUebernehmen gilt bei blosser Erreichbarkeit sofort als uebernommen und gibt das Startmutex frei, ohne auf das Handleurteil zu warten | ROT |
| C7 | C-09 | PipeClient laesst eine einmal erteilte Serverfreigabe den Reconnect ueberleben und prueft den neuen Handle nicht erneut | ROT |

## Fall R1

```
# E-5 Rotlauf R1  Matrix A-02, A-08
# Kippung: ListenerInstanz::neu ohne FILE_FLAG_FIRST_PIPE_INSTANCE fuer die erste Instanz
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact start_belegter_name_scheitert_ohne_zweiten_broker neustart_besetzt_name_vor_bereit
# Erwartet ROT: start_belegter_name_scheitert_ohne_zweiten_broker, neustart_besetzt_name_vor_bereit


running 2 tests
test start_belegter_name_scheitert_ohne_zweiten_broker ... FAILED
test neustart_besetzt_name_vor_bereit ... FAILED

failures:

---- start_belegter_name_scheitert_ohne_zweiten_broker stdout ----

thread 'start_belegter_name_scheitert_ohne_zweiten_broker' (17964) panicked at tests\security_vectors.rs:262:5:
assertion failed: v3_server_starten_fuer_security_vectors(&pipe, senke,
        V3SecurityTestOptionen::default()).is_err()
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace

---- neustart_besetzt_name_vor_bereit stdout ----

thread 'neustart_besetzt_name_vor_bereit' (6840) panicked at tests\security_vectors.rs:386:5:
assertion failed: v3_server_starten_fuer_security_vectors(&pipe, senke,
        V3SecurityTestOptionen::default()).is_err()


failures:
    neustart_besetzt_name_vor_bereit
    start_belegter_name_scheitert_ohne_zweiten_broker

test result: FAILED. 0 passed; 2 failed; 0 ignored; 0 measured; 16 filtered out; finished in 0.00s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
warning: unused import: `FILE_FLAG_FIRST_PIPE_INSTANCE`
  --> src\transport\server_v3.rs:63:26
   |
63 |     ReadFile, WriteFile, FILE_FLAG_FIRST_PIPE_INSTANCE, FILE_FLAG_OVERLAPPED, PIPE_ACCESS_DUPLEX,
   |                          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
   |
   = note: `#[warn(unused_imports)]` (part of `#[warn(unused)]`) on by default

warning: `eqcop-broker` (lib) generated 1 warning (run `cargo fix --lib -p eqcop-broker` to apply 1 suggestion)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 1.95s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall R2

```
# E-5 Rotlauf R2  Matrix A-01
# Kippung: zweiter Listener wird erzeugt, aber nie bewaffnet (Zaehler meldet 1 statt 2)
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact start_freier_name_bewaffnet_zwei_listener_vor_veroeffentlichung
# Erwartet ROT: start_freier_name_bewaffnet_zwei_listener_vor_veroeffentlichung


running 1 test
test start_freier_name_bewaffnet_zwei_listener_vor_veroeffentlichung ... FAILED

failures:

---- start_freier_name_bewaffnet_zwei_listener_vor_veroeffentlichung stdout ----

thread 'start_freier_name_bewaffnet_zwei_listener_vor_veroeffentlichung' (7612) panicked at tests\security_vectors.rs:217:5:
assertion `left == right` failed
  left: 1
 right: 2
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    start_freier_name_bewaffnet_zwei_listener_vor_veroeffentlichung

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 17 filtered out; finished in 0.10s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
warning: variable does not need to be mutable
    --> src\transport\server_v3.rs:1665:9
     |
1665 |     let mut zweiter = ListenerInstanz::neu(&name_w, &attrs, false)
     |         ----^^^^^^^
     |         |
     |         help: remove this `mut`
     |
     = note: `#[warn(unused_mut)]` (part of `#[warn(unused)]`) on by default

warning: `eqcop-broker` (lib) generated 1 warning (run `cargo fix --lib -p eqcop-broker` to apply 1 suggestion)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 2.59s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall R3

```
# E-5 Rotlauf R3  Matrix A-01
# Kippung: Startfehlerkante ZweiteInstanz gibt das erste Handle nicht per RAII frei
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact startfehler_nach_erstem_handle_raeumt_handles_threads_und_namen_vor_publish
# Erwartet ROT: startfehler_nach_erstem_handle_raeumt_handles_threads_und_namen_vor_publish


running 1 test
test startfehler_nach_erstem_handle_raeumt_handles_threads_und_namen_vor_publish ... FAILED

failures:

---- startfehler_nach_erstem_handle_raeumt_handles_threads_und_namen_vor_publish stdout ----

thread 'startfehler_nach_erstem_handle_raeumt_handles_threads_und_namen_vor_publish' (20068) panicked at tests\security_vectors.rs:253:32:
Fehlerkante ZweiteInstanz liess den Namen besetzt
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    startfehler_nach_erstem_handle_raeumt_handles_threads_und_namen_vor_publish

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 17 filtered out; finished in 0.00s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 2.41s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall R4

```
# E-5 Rotlauf R4  Matrix A-03
# Kippung: Worker-Uebergabefenster liegt VOR der Armierung des Ersatzlisteners
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact accept_bewaffnet_nachfolger_vor_worker_uebergabe_bei_zwei_listenern
# Erwartet ROT: accept_bewaffnet_nachfolger_vor_worker_uebergabe_bei_zwei_listenern


running 1 test
test accept_bewaffnet_nachfolger_vor_worker_uebergabe_bei_zwei_listenern ... FAILED

failures:

---- accept_bewaffnet_nachfolger_vor_worker_uebergabe_bei_zwei_listenern stdout ----

thread 'accept_bewaffnet_nachfolger_vor_worker_uebergabe_bei_zwei_listenern' (18816) panicked at tests\security_vectors.rs:284:5:
assertion `left == right` failed
  left: 1
 right: 2
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    accept_bewaffnet_nachfolger_vor_worker_uebergabe_bei_zwei_listenern

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 17 filtered out; finished in 0.10s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 2.22s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall R5

```
# E-5 Rotlauf R5  Matrix A-04
# Kippung: Ersatzlistener-Fehler wird verschluckt: kein listener_fehler, kein fail-closed
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact ersatzlistener_fehler_behaelt_besitz_und_meldet_nicht_bereit
# Erwartet ROT: ersatzlistener_fehler_behaelt_besitz_und_meldet_nicht_bereit


running 1 test
test ersatzlistener_fehler_behaelt_besitz_und_meldet_nicht_bereit ... FAILED

failures:

---- ersatzlistener_fehler_behaelt_besitz_und_meldet_nicht_bereit stdout ----

thread 'ersatzlistener_fehler_behaelt_besitz_und_meldet_nicht_bereit' (8128) panicked at tests\security_vectors.rs:310:5:
assertion failed: warten(3000, || griff.statistik.listener_fehler.load(Ordering::SeqCst) == 1)
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    ersatzlistener_fehler_behaelt_besitz_und_meldet_nicht_bereit

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 17 filtered out; finished in 3.11s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 2.28s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall R6

```
# E-5 Rotlauf R6  Matrix A-05
# Kippung: Worker-Cap um 4 angehoben: der 97. Client erreicht einen Worker
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz
# Erwartet ROT: zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz


running 1 test
test zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz ... FAILED

failures:

---- zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz stdout ----

thread 'zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz' (132) panicked at tests\security_vectors.rs:341:5:
assertion failed: warten(3000,
    || griff.statistik.am_worker_cap_abgewiesen.load(Ordering::SeqCst) == 1)
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    zwei_listener_plus_96_worker_erhalten_cap_und_namensbesitz

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 17 filtered out; finished in 3.51s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 2.24s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall R7

```
# E-5 Rotlauf R7  Matrix A-07
# Kippung: Broker besitzt einen anderen Pipenamen als den geforderten
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact prozesskill_gibt_name_frei
# Erwartet ROT: prozesskill_gibt_name_frei


running 1 test
test prozesskill_gibt_name_frei ... FAILED

failures:

---- prozesskill_gibt_name_frei stdout ----

thread 'prozesskill_gibt_name_frei' (1848) panicked at tests\security_vectors.rs:373:5:
assertion failed: FirstInstance::nehmen(&pipe).is_none()
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    prozesskill_gibt_name_frei

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 17 filtered out; finished in 0.21s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 2.31s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall R8

```
# E-5 Rotlauf R8  Matrix A-09
# Kippung: Besitzlistener fallen VOR Acceptor-, Wachhund- und Worker-Joins
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact stoppen_schliesst_besitzlistener_zuletzt_und_neustart_ist_sofort_moeglich
# Erwartet ROT: stoppen_schliesst_besitzlistener_zuletzt_und_neustart_ist_sofort_moeglich


running 1 test
test stoppen_schliesst_besitzlistener_zuletzt_und_neustart_ist_sofort_moeglich ... FAILED

failures:

---- stoppen_schliesst_besitzlistener_zuletzt_und_neustart_ist_sofort_moeglich stdout ----

thread 'stoppen_schliesst_besitzlistener_zuletzt_und_neustart_ist_sofort_moeglich' (19764) panicked at tests\security_vectors.rs:171:13:
Reihenfolge ["close", "listeners_close"] in ["connect", "read", "impersonate", "sid", "revert", "self", "hello_accept", "listeners_close", "close", "listeners_close", "listeners_close"]
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    stoppen_schliesst_besitzlistener_zuletzt_und_neustart_ist_sofort_moeglich

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 17 filtered out; finished in 0.10s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 2.56s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall R9

```
# E-5 Rotlauf R9  Matrix A-06
# Kippung: aktive_worker() meldet immer 0: ein verbundener Client verhindert das Selbstende nicht
# Befehl: cargo test --manifest-path broker/Cargo.toml --test broker_idle -- --exact echter_brokerprozess_beendet_sich_nach_letztem_client_selbst
# Erwartet ROT: echter_brokerprozess_beendet_sich_nach_letztem_client_selbst


running 1 test
test echter_brokerprozess_beendet_sich_nach_letztem_client_selbst has been running for over 60 seconds
test echter_brokerprozess_beendet_sich_nach_letztem_client_selbst ... FAILED

failures:

---- echter_brokerprozess_beendet_sich_nach_letztem_client_selbst stdout ----

thread 'echter_brokerprozess_beendet_sich_nach_letztem_client_selbst' (12896) panicked at tests\broker_idle.rs:163:5:
assertion failed: dauer >= Duration::from_millis(BROKER_IDLE_ENDE_MS)
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    echter_brokerprozess_beendet_sich_nach_letztem_client_selbst

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 3 filtered out; finished in 60.37s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 1.83s
     Running tests\broker_idle.rs (broker\target\debug\deps\broker_idle-335b469e8f1e1607.exe)
error: test failed, to rerun pass `--test broker_idle`
```

## Fall R10

```
# E-5 Rotlauf R10  Matrix B-01, B-02, B-03, B-04, B-05, B-08, B-09
# Kippung: client_sid_authentisieren nimmt jede Verbindung ohne Impersonation, SID-Vergleich und Revert an
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact gleiche_sid_wird_erst_nach_revert_angenommen impersonation_fehlschlag_schliesst_ohne_serverkontext tokenabfragefehler_revertiert_und_schliesst_ohne_senke fremde_sid_revertiert_und_erreicht_keinen_coordinator vollstaendiges_abzulehnendes_hello_impersoniert_prueft_sid_und_revertiert_vor_reject control_und_telemetry_durchlaufen_die_identische_sid_kette nach_revert_bleiben_envelope_und_trennpfade_im_self_kontext
# Erwartet ROT: gleiche_sid_wird_erst_nach_revert_angenommen, impersonation_fehlschlag_schliesst_ohne_serverkontext, tokenabfragefehler_revertiert_und_schliesst_ohne_senke, fremde_sid_revertiert_und_erreicht_keinen_coordinator, vollstaendiges_abzulehnendes_hello_impersoniert_prueft_sid_und_revertiert_vor_reject, control_und_telemetry_durchlaufen_die_identische_sid_kette, nach_revert_bleiben_envelope_und_trennpfade_im_self_kontext


running 7 tests
test nach_revert_bleiben_envelope_und_trennpfade_im_self_kontext ... FAILED
test control_und_telemetry_durchlaufen_die_identische_sid_kette ... FAILED
test vollstaendiges_abzulehnendes_hello_impersoniert_prueft_sid_und_revertiert_vor_reject ... FAILED
test gleiche_sid_wird_erst_nach_revert_angenommen ... FAILED
test impersonation_fehlschlag_schliesst_ohne_serverkontext ... FAILED
test tokenabfragefehler_revertiert_und_schliesst_ohne_senke ... FAILED
test fremde_sid_revertiert_und_erreicht_keinen_coordinator ... FAILED

failures:

---- nach_revert_bleiben_envelope_und_trennpfade_im_self_kontext stdout ----

thread 'nach_revert_bleiben_envelope_und_trennpfade_im_self_kontext' (11908) panicked at tests\security_vectors.rs:692:5:
assertion `left == right` failed
  left: 0
 right: 1

---- control_und_telemetry_durchlaufen_die_identische_sid_kette stdout ----

thread 'control_und_telemetry_durchlaufen_die_identische_sid_kette' (11816) panicked at tests\security_vectors.rs:639:9:
assertion `left == right` failed: impersonate: ["connect", "read", "hello_accept", "connect", "read", "hello_accept"]
  left: 0
 right: 2

---- vollstaendiges_abzulehnendes_hello_impersoniert_prueft_sid_und_revertiert_vor_reject stdout ----

thread 'vollstaendiges_abzulehnendes_hello_impersoniert_prueft_sid_und_revertiert_vor_reject' (13692) panicked at tests\security_vectors.rs:163:28:
Schritt "impersonate" fehlt in ["connect", "read", "reject", "revert", "close"]
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace

---- gleiche_sid_wird_erst_nach_revert_angenommen stdout ----

thread 'gleiche_sid_wird_erst_nach_revert_angenommen' (5588) panicked at tests\security_vectors.rs:163:28:
Schritt "impersonate" fehlt in ["connect", "read", "hello_accept"]

---- impersonation_fehlschlag_schliesst_ohne_serverkontext stdout ----

thread 'impersonation_fehlschlag_schliesst_ohne_serverkontext' (17560) panicked at tests\security_vectors.rs:476:5:
assertion failed: warten(3000, || griff.sicherheits_spur().contains(&"close"))

---- tokenabfragefehler_revertiert_und_schliesst_ohne_senke stdout ----

thread 'tokenabfragefehler_revertiert_und_schliesst_ohne_senke' (1232) panicked at tests\security_vectors.rs:476:5:
assertion failed: warten(3000, || griff.sicherheits_spur().contains(&"close"))

---- fremde_sid_revertiert_und_erreicht_keinen_coordinator stdout ----

thread 'fremde_sid_revertiert_und_erreicht_keinen_coordinator' (18104) panicked at tests\security_vectors.rs:476:5:
assertion failed: warten(3000, || griff.sicherheits_spur().contains(&"close"))


failures:
    control_und_telemetry_durchlaufen_die_identische_sid_kette
    fremde_sid_revertiert_und_erreicht_keinen_coordinator
    gleiche_sid_wird_erst_nach_revert_angenommen
    impersonation_fehlschlag_schliesst_ohne_serverkontext
    nach_revert_bleiben_envelope_und_trennpfade_im_self_kontext
    tokenabfragefehler_revertiert_und_schliesst_ohne_senke
    vollstaendiges_abzulehnendes_hello_impersoniert_prueft_sid_und_revertiert_vor_reject

test result: FAILED. 0 passed; 7 failed; 0 ignored; 0 measured; 11 filtered out; finished in 3.01s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 2.58s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall R11

```
# E-5 Rotlauf R11  Matrix B-06
# Kippung: Impersonation-/SID-Kette laeuft VOR dem Bootstrap-Read
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact bootstrap_timeout_und_lesefehler_revertieren_jeweils_ohne_serverweiterlauf
# Erwartet ROT: bootstrap_timeout_und_lesefehler_revertieren_jeweils_ohne_serverweiterlauf


running 1 test
test bootstrap_timeout_und_lesefehler_revertieren_jeweils_ohne_serverweiterlauf ... FAILED

failures:

---- bootstrap_timeout_und_lesefehler_revertieren_jeweils_ohne_serverweiterlauf stdout ----

thread 'bootstrap_timeout_und_lesefehler_revertieren_jeweils_ohne_serverweiterlauf' (19852) panicked at tests\security_vectors.rs:567:9:
assertion failed: !spur.contains(&"impersonate")
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    bootstrap_timeout_und_lesefehler_revertieren_jeweils_ohne_serverweiterlauf

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 17 filtered out; finished in 0.10s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 3.17s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall R12

```
# E-5 Rotlauf R12  Matrix B-07
# Kippung: RevertToSelf-Fehler beendet den Prozess nicht mehr fail-fast
# Befehl: cargo test --manifest-path broker/Cargo.toml --test security_vectors -- --exact revert_fehlschlag_beendet_testbrokerprozess_fail_fast
# Erwartet ROT: revert_fehlschlag_beendet_testbrokerprozess_fail_fast


running 1 test
test revert_fehlschlag_beendet_testbrokerprozess_fail_fast ... FAILED

failures:

---- revert_fehlschlag_beendet_testbrokerprozess_fail_fast stdout ----

thread 'revert_fehlschlag_beendet_testbrokerprozess_fail_fast' (14928) panicked at tests\security_vectors.rs:602:9:
Revert-Fehler liess den Broker weiterlaufen
note: run with `RUST_BACKTRACE=1` environment variable to display a backtrace


failures:
    revert_fehlschlag_beendet_testbrokerprozess_fail_fast

test result: FAILED. 0 passed; 1 failed; 0 ignored; 0 measured; 17 filtered out; finished in 10.20s

   Compiling eqcop-broker v0.1.0 (C:\Users\phili\Projekte\Nakama\broker)
    Finished `test` profile [unoptimized + debuginfo] target(s) in 2.54s
     Running tests\security_vectors.rs (broker\target\debug\deps\security_vectors-4a68d58de2e43d5b.exe)
error: test failed, to rerun pass `--test security_vectors`
```

## Fall C1

```
# E-5 Rotlauf C1  Matrix C-02, C-03, C-04, C-05, C-06, C-07, C-08, C-09, C-10
# Kippung: namedPipeServerAuthentisieren meldet jeden Server sofort als verifiziert (kein PID-Pin, keine SID, kein Prozessbild, keine Dateiidentitaet, kein SHA-256, keine Signatur)
# Ziele: EqCopIpcTest, EqCopPipeClientTest, EqCopSonde012ProjectReloadTest


===== EqCopIpcTest  exit=1 =====
SONDE-010 | v3-Envelope, Pipetoken, Backpressure und die zwei Clients

== A · Envelope-Fixture-Korpus (dieselbe dritte Partei wie Rust) ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== B · CRC32C und Envelope-Rundlauf ==
  [... 10 bestandene ok-Zeilen ausgelassen ...]

== C · Fuzz (deterministisch): Invarianten des angenommenen Frames ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]

== D · v3-Pipename als reine Funktion ==
  [... 13 bestandene ok-Zeilen ausgelassen ...]

== E · Backpressure: P0 · P1 · P2 ==
  [... 31 bestandene ok-Zeilen ausgelassen ...]

== E2 · P2-Schleuse: vorallokiert, ohne Allokation, replace-oldest ==
  [... 20 bestandene ok-Zeilen ausgelassen ...]

== F · Ratengrenze ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G · ControlClient und TelemetryClient auf einer Testpipe ==
  [... 15 bestandene ok-Zeilen ausgelassen ...]

== G1a · JSON-Minor wird bis zum C++-Empfaenger getragen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G1b · Produktstatus sendet Heartbeat und State-Report ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G2 · ungekoppelter Telemetry-Connect wird geschlossen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G3 · Reconnect nach Serverneustart ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G4 · ein kaputter Envelope vom Server schliesst die Verbindung ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G5 · P0-Ueberlauf schliesst die Verbindung, statt zu verwerfen ==
  [... 7 bestandene ok-Zeilen ausgelassen ...]

== G6 · ein P1-Ereignis ueberlebt einen gescheiterten Write ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G7 · der TelemetryClient prueft sein welcome vollstaendig ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G8 · nicht endliche Audiofelder werden VOR der Wandlung verriegelt ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]

== G9 · ein P0-Befehl ueberlebt einen gescheiterten Write bei voller Queue ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== G10 · beide Clients pruefen das welcome gegen den VOLLSTAENDIGEN Vertrag ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== G11 · ein P2-Frame auf der Control-Verbindung wird abgewiesen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G12 · die Nachrichtenratengrenze gilt auch im Client ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G13 · die Telemetrie merkt im Leerlauf, dass ihre Kopplung fort ist ==
  [... 8 bestandene ok-Zeilen ausgelassen ...]

== G14 · was der TelemetryClient auf SEINER Verbindung annimmt ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G15 · zu grosse Nachrichten werden an der TUER abgewiesen ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G16 · ein P0-ACK kommt an, waehrend P1 rueckstaut ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G16b · der ACK trifft ein, WAEHREND der Write in seine Frist laeuft ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G17 · stop() kehrt in JEDEM Zustand zurueck ==
  [... 24 bestandene ok-Zeilen ausgelassen ...]

== G18 · die Telemetrie verliert die Verbindung mitten im Write ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== H · Bootstrapgrenze und JSON-Riegel ==
  [... 18 bestandene ok-Zeilen ausgelassen ...]

== I · Phase B: semantisches command_ack und In-Flight-Register ==
  [... 8 bestandene ok-Zeilen ausgelassen ...]

== J · Phase B: Broker-Autostart und Signaturkette ==
  [... 1 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  serverauth_gueltiger_pid_sid_datei_hash_und_signer_vor_hello
  FEHLER  serverauth_fremde_sid_und_tokenabfragefehler_senden_null_bytes
  FEHLER  serverauth_bild_dateiid_hash_signatur_und_signer_fail_closed
  FEHLER  waitnamedpipe_toctou_wird_am_verbundenen_handle_erkannt
  [... 4 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  spawn_pid_ersetzt_serverauth_nicht_und_fremdpipe_im_bereitfenster_faellt
  FEHLER  reconnect_prueft_pid_sid_und_bild_ohne_altfreigabe
  FEHLER  reconnect_sendet_erst_nach_neuer_serverauth
  FEHLER  serverauth_nach_brokercrash_verweigert_pidwechsel_und_fremdbild
  FEHLER  serverauth_nach_idle_exit_verweigert_fremde_pipe
  [... 20 bestandene ok-Zeilen ausgelassen ...]
BEREIT \\.\pipe\evenacadia.nakama.v3.probe.test.14780.79.foreign-broker-owner
{"abweisungen":0,"acceptor_wartet_auf_instanz":0,"angenommen":1,"besitzlistener_nach_stopp":0,"besitzlistener_vor_stopp":2,"control_getrennt":1,"control_verbindungen":1,"ende_grund":"stdin","geschlossen_bootstrap":0,"geschlossen_envelope":0,"geschlossen_familie":0,"geschlossen_kopplung":0,"geschlossen_p0_ueberlauf":0,"geschlossen_rate":0,"geschlossen_writer":0,"ingress_hoechststand":0,"ingress_p1_ueberlauf_trennt":0,"ingress_p2_verworfen":0,"letzter_grund":"","p0":0,"p0_beantwortet":0,"p1":0,"p2":0,"pipe":"\\\\.\\pipe\\evenacadia.nakama.v3.probe.test.14780.79.foreign-broker-owner","telemetrie_getrennt":0,"telemetrie_verbindungen":0}
  [... 3 bestandene ok-Zeilen ausgelassen ...]

FEHLER — 262 Pruefungen, 9 Fehler


===== EqCopPipeClientTest  exit=1 =====
PIPECLIENT-LIFECYCLE-TEST
  [... 8 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  pipeclient_sendet_vor_serverauth_keine_bytes
  [... 8 bestandene ok-Zeilen ausgelassen ...]
PIPECLIENT-LIFECYCLE-TEST FEHLGESCHLAGEN - 1 Fehler


===== EqCopSonde012ProjectReloadTest  exit=1 =====
  FEHLER  gefaelschtes_command_ack_vor_serverauth_mutiert_keinen_persistenten_projektzustand
  [... 9 bestandene ok-Zeilen ausgelassen ...]
SONDE-012 ProjectReload: 9/10 gruen
```

## Fall C2

```
# E-5 Rotlauf C2  Matrix C-01
# Kippung: fehlende Pipe (ERROR_FILE_NOT_FOUND) wird als Sicherheitsfehler belegtAberUnverifiziert gemeldet statt als nichtDa/pipeFehlt
# Ziele: EqCopIpcTest


===== EqCopIpcTest  exit=1 =====
SONDE-010 | v3-Envelope, Pipetoken, Backpressure und die zwei Clients

== A · Envelope-Fixture-Korpus (dieselbe dritte Partei wie Rust) ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== B · CRC32C und Envelope-Rundlauf ==
  [... 10 bestandene ok-Zeilen ausgelassen ...]

== C · Fuzz (deterministisch): Invarianten des angenommenen Frames ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]

== D · v3-Pipename als reine Funktion ==
  [... 13 bestandene ok-Zeilen ausgelassen ...]

== E · Backpressure: P0 · P1 · P2 ==
  [... 31 bestandene ok-Zeilen ausgelassen ...]

== E2 · P2-Schleuse: vorallokiert, ohne Allokation, replace-oldest ==
  [... 20 bestandene ok-Zeilen ausgelassen ...]

== F · Ratengrenze ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G · ControlClient und TelemetryClient auf einer Testpipe ==
  [... 15 bestandene ok-Zeilen ausgelassen ...]

== G1a · JSON-Minor wird bis zum C++-Empfaenger getragen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G1b · Produktstatus sendet Heartbeat und State-Report ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G2 · ungekoppelter Telemetry-Connect wird geschlossen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G3 · Reconnect nach Serverneustart ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G4 · ein kaputter Envelope vom Server schliesst die Verbindung ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G5 · P0-Ueberlauf schliesst die Verbindung, statt zu verwerfen ==
  [... 7 bestandene ok-Zeilen ausgelassen ...]

== G6 · ein P1-Ereignis ueberlebt einen gescheiterten Write ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G7 · der TelemetryClient prueft sein welcome vollstaendig ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G8 · nicht endliche Audiofelder werden VOR der Wandlung verriegelt ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]

== G9 · ein P0-Befehl ueberlebt einen gescheiterten Write bei voller Queue ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== G10 · beide Clients pruefen das welcome gegen den VOLLSTAENDIGEN Vertrag ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== G11 · ein P2-Frame auf der Control-Verbindung wird abgewiesen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G12 · die Nachrichtenratengrenze gilt auch im Client ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G13 · die Telemetrie merkt im Leerlauf, dass ihre Kopplung fort ist ==
  [... 8 bestandene ok-Zeilen ausgelassen ...]

== G14 · was der TelemetryClient auf SEINER Verbindung annimmt ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G15 · zu grosse Nachrichten werden an der TUER abgewiesen ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G16 · ein P0-ACK kommt an, waehrend P1 rueckstaut ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G16b · der ACK trifft ein, WAEHREND der Write in seine Frist laeuft ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G17 · stop() kehrt in JEDEM Zustand zurueck ==
  [... 24 bestandene ok-Zeilen ausgelassen ...]

== G18 · die Telemetrie verliert die Verbindung mitten im Write ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== H · Bootstrapgrenze und JSON-Riegel ==
  [... 18 bestandene ok-Zeilen ausgelassen ...]

== I · Phase B: semantisches command_ack und In-Flight-Register ==
  [... 8 bestandene ok-Zeilen ausgelassen ...]

== J · Phase B: Broker-Autostart und Signaturkette ==
  FEHLER  fehlender_broker_ist_einzige_startbedingung_auch_mit_serverauth
  [... 8 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  spawn_pid_ersetzt_serverauth_nicht_und_fremdpipe_im_bereitfenster_faellt
  FEHLER  reconnect_prueft_pid_sid_und_bild_ohne_altfreigabe
  FEHLER  reconnect_sendet_erst_nach_neuer_serverauth
  [... 7 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  fehlender_broker_ist_notwendige_startbedingung
  [... 14 bestandene ok-Zeilen ausgelassen ...]
BEREIT \\.\pipe\evenacadia.nakama.v3.probe.test.14876.79.foreign-broker-owner
{"abweisungen":0,"acceptor_wartet_auf_instanz":0,"angenommen":0,"besitzlistener_nach_stopp":0,"besitzlistener_vor_stopp":2,"control_getrennt":0,"control_verbindungen":0,"ende_grund":"stdin","geschlossen_bootstrap":0,"geschlossen_envelope":0,"geschlossen_familie":0,"geschlossen_kopplung":0,"geschlossen_p0_ueberlauf":0,"geschlossen_rate":0,"geschlossen_writer":0,"ingress_hoechststand":0,"ingress_p1_ueberlauf_trennt":0,"ingress_p2_verworfen":0,"letzter_grund":"","p0":0,"p0_beantwortet":0,"p1":0,"p2":0,"pipe":"\\\\.\\pipe\\evenacadia.nakama.v3.probe.test.14876.79.foreign-broker-owner","telemetrie_getrennt":0,"telemetrie_verbindungen":0}
  FEHLER  plugin_stoppt_keinen_fremden_brokerprozess
  [... 2 bestandene ok-Zeilen ausgelassen ...]

FEHLER — 262 Pruefungen, 6 Fehler
```

## Fall C3

```
# E-5 Rotlauf C3  Matrix C-11
# Kippung: processBlock ruft brokerLifecycleSnapshot() und nimmt damit eine Lifecycle-Sperre im Audiothread
# Ziele: EqCopLebenslaufTest


===== EqCopLebenslaufTest  exit=1 =====
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  [... 37 bestandene ok-Zeilen ausgelassen ...]

[2] Verdrahtet: EqCopilotProcessor (Gen)
  [... 30 bestandene ok-Zeilen ausgelassen ...]

[3] Phase B — Broker-Lifecycle-Gate und Audiothread-Nulltest
  [... 2 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  broker_lifecycle_aufrufe_im_audiothread_null
  FEHLER  brokerstop_nie_im_processblock
  [... 1 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  processblock_quellschnitt_dateioperationen_null
  FEHLER  processblock_quellschnitt_pipeoperationen_null
  FEHLER  processblock_quellschnitt_prozessoperationen_null
  FEHLER  processblock_quellschnitt_logoperationen_null
  FEHLER  processblock_quellschnitt_store_wait_null

LEBENSLAUF-TEST FEHLGESCHLAGEN - 70 ok, 7 Fehler
```

## Fall C4

```
# E-5 Rotlauf C4  Matrix C-06
# Kippung: TelemetryClient uebernimmt die Freigabe der Controlverbindung und fuehrt/zaehlt keine eigene Handlepruefung mehr
# Ziele: EqCopIpcTest


===== EqCopIpcTest  exit=1 =====
SONDE-010 | v3-Envelope, Pipetoken, Backpressure und die zwei Clients

== A · Envelope-Fixture-Korpus (dieselbe dritte Partei wie Rust) ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== B · CRC32C und Envelope-Rundlauf ==
  [... 10 bestandene ok-Zeilen ausgelassen ...]

== C · Fuzz (deterministisch): Invarianten des angenommenen Frames ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]

== D · v3-Pipename als reine Funktion ==
  [... 13 bestandene ok-Zeilen ausgelassen ...]

== E · Backpressure: P0 · P1 · P2 ==
  [... 31 bestandene ok-Zeilen ausgelassen ...]

== E2 · P2-Schleuse: vorallokiert, ohne Allokation, replace-oldest ==
  [... 20 bestandene ok-Zeilen ausgelassen ...]

== F · Ratengrenze ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G · ControlClient und TelemetryClient auf einer Testpipe ==
  [... 15 bestandene ok-Zeilen ausgelassen ...]

== G1a · JSON-Minor wird bis zum C++-Empfaenger getragen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G1b · Produktstatus sendet Heartbeat und State-Report ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G2 · ungekoppelter Telemetry-Connect wird geschlossen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G3 · Reconnect nach Serverneustart ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G4 · ein kaputter Envelope vom Server schliesst die Verbindung ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G5 · P0-Ueberlauf schliesst die Verbindung, statt zu verwerfen ==
  [... 7 bestandene ok-Zeilen ausgelassen ...]

== G6 · ein P1-Ereignis ueberlebt einen gescheiterten Write ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G7 · der TelemetryClient prueft sein welcome vollstaendig ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G8 · nicht endliche Audiofelder werden VOR der Wandlung verriegelt ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]

== G9 · ein P0-Befehl ueberlebt einen gescheiterten Write bei voller Queue ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== G10 · beide Clients pruefen das welcome gegen den VOLLSTAENDIGEN Vertrag ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== G11 · ein P2-Frame auf der Control-Verbindung wird abgewiesen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G12 · die Nachrichtenratengrenze gilt auch im Client ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G13 · die Telemetrie merkt im Leerlauf, dass ihre Kopplung fort ist ==
  [... 8 bestandene ok-Zeilen ausgelassen ...]

== G14 · was der TelemetryClient auf SEINER Verbindung annimmt ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G15 · zu grosse Nachrichten werden an der TUER abgewiesen ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G16 · ein P0-ACK kommt an, waehrend P1 rueckstaut ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G16b · der ACK trifft ein, WAEHREND der Write in seine Frist laeuft ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G17 · stop() kehrt in JEDEM Zustand zurueck ==
  [... 24 bestandene ok-Zeilen ausgelassen ...]

== G18 · die Telemetrie verliert die Verbindung mitten im Write ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== H · Bootstrapgrenze und JSON-Riegel ==
  [... 18 bestandene ok-Zeilen ausgelassen ...]

== I · Phase B: semantisches command_ack und In-Flight-Register ==
  [... 8 bestandene ok-Zeilen ausgelassen ...]

== J · Phase B: Broker-Autostart und Signaturkette ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  control_und_telemetry_authentisieren_jedes_handle
  [... 28 bestandene ok-Zeilen ausgelassen ...]
BEREIT \\.\pipe\evenacadia.nakama.v3.probe.test.10188.79.foreign-broker-owner
{"abweisungen":0,"acceptor_wartet_auf_instanz":0,"angenommen":1,"besitzlistener_nach_stopp":0,"besitzlistener_vor_stopp":2,"control_getrennt":1,"control_verbindungen":1,"ende_grund":"stdin","geschlossen_bootstrap":0,"geschlossen_envelope":0,"geschlossen_familie":0,"geschlossen_kopplung":0,"geschlossen_p0_ueberlauf":0,"geschlossen_rate":0,"geschlossen_writer":0,"ingress_hoechststand":0,"ingress_p1_ueberlauf_trennt":0,"ingress_p2_verworfen":0,"letzter_grund":"","p0":0,"p0_beantwortet":0,"p1":0,"p2":0,"pipe":"\\\\.\\pipe\\evenacadia.nakama.v3.probe.test.10188.79.foreign-broker-owner","telemetrie_getrennt":0,"telemetrie_verbindungen":0}
  [... 3 bestandene ok-Zeilen ausgelassen ...]

FEHLER — 262 Pruefungen, 1 Fehler
```

## Fall C5

```
# E-5 Rotlauf C5  Matrix C-07
# Kippung: BrokerLifecycle behandelt belegtAberUnverifiziert nicht mehr fail-closed, sondern faellt in den normalen Spawn-/Retryweg
# Ziele: EqCopIpcTest


===== EqCopIpcTest  exit=1 =====
SONDE-010 | v3-Envelope, Pipetoken, Backpressure und die zwei Clients

== A · Envelope-Fixture-Korpus (dieselbe dritte Partei wie Rust) ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== B · CRC32C und Envelope-Rundlauf ==
  [... 10 bestandene ok-Zeilen ausgelassen ...]

== C · Fuzz (deterministisch): Invarianten des angenommenen Frames ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]

== D · v3-Pipename als reine Funktion ==
  [... 13 bestandene ok-Zeilen ausgelassen ...]

== E · Backpressure: P0 · P1 · P2 ==
  [... 31 bestandene ok-Zeilen ausgelassen ...]

== E2 · P2-Schleuse: vorallokiert, ohne Allokation, replace-oldest ==
  [... 20 bestandene ok-Zeilen ausgelassen ...]

== F · Ratengrenze ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G · ControlClient und TelemetryClient auf einer Testpipe ==
  [... 15 bestandene ok-Zeilen ausgelassen ...]

== G1a · JSON-Minor wird bis zum C++-Empfaenger getragen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G1b · Produktstatus sendet Heartbeat und State-Report ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G2 · ungekoppelter Telemetry-Connect wird geschlossen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G3 · Reconnect nach Serverneustart ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G4 · ein kaputter Envelope vom Server schliesst die Verbindung ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G5 · P0-Ueberlauf schliesst die Verbindung, statt zu verwerfen ==
  [... 7 bestandene ok-Zeilen ausgelassen ...]

== G6 · ein P1-Ereignis ueberlebt einen gescheiterten Write ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G7 · der TelemetryClient prueft sein welcome vollstaendig ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G8 · nicht endliche Audiofelder werden VOR der Wandlung verriegelt ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]

== G9 · ein P0-Befehl ueberlebt einen gescheiterten Write bei voller Queue ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== G10 · beide Clients pruefen das welcome gegen den VOLLSTAENDIGEN Vertrag ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== G11 · ein P2-Frame auf der Control-Verbindung wird abgewiesen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G12 · die Nachrichtenratengrenze gilt auch im Client ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G13 · die Telemetrie merkt im Leerlauf, dass ihre Kopplung fort ist ==
  [... 8 bestandene ok-Zeilen ausgelassen ...]

== G14 · was der TelemetryClient auf SEINER Verbindung annimmt ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G15 · zu grosse Nachrichten werden an der TUER abgewiesen ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G16 · ein P0-ACK kommt an, waehrend P1 rueckstaut ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G16b · der ACK trifft ein, WAEHREND der Write in seine Frist laeuft ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G17 · stop() kehrt in JEDEM Zustand zurueck ==
  [... 24 bestandene ok-Zeilen ausgelassen ...]

== G18 · die Telemetrie verliert die Verbindung mitten im Write ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== H · Bootstrapgrenze und JSON-Riegel ==
  [... 18 bestandene ok-Zeilen ausgelassen ...]

== I · Phase B: semantisches command_ack und In-Flight-Register ==
  [... 8 bestandene ok-Zeilen ausgelassen ...]

== J · Phase B: Broker-Autostart und Signaturkette ==
  [... 7 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  unverifizierte_belegte_pipe_spawnt_und_reconnectet_nicht
  FEHLER  vorhandene_fremdpipe_wird_nicht_adoptiert
  FEHLER  spawn_pid_ersetzt_serverauth_nicht_und_fremdpipe_im_bereitfenster_faellt
  [... 24 bestandene ok-Zeilen ausgelassen ...]
BEREIT \\.\pipe\evenacadia.nakama.v3.probe.test.16812.79.foreign-broker-owner
{"abweisungen":0,"acceptor_wartet_auf_instanz":0,"angenommen":1,"besitzlistener_nach_stopp":0,"besitzlistener_vor_stopp":2,"control_getrennt":1,"control_verbindungen":1,"ende_grund":"stdin","geschlossen_bootstrap":0,"geschlossen_envelope":0,"geschlossen_familie":0,"geschlossen_kopplung":0,"geschlossen_p0_ueberlauf":0,"geschlossen_rate":0,"geschlossen_writer":0,"ingress_hoechststand":0,"ingress_p1_ueberlauf_trennt":0,"ingress_p2_verworfen":0,"letzter_grund":"","p0":0,"p0_beantwortet":0,"p1":0,"p2":0,"pipe":"\\\\.\\pipe\\evenacadia.nakama.v3.probe.test.16812.79.foreign-broker-owner","telemetrie_getrennt":0,"telemetrie_verbindungen":0}
  [... 3 bestandene ok-Zeilen ausgelassen ...]

FEHLER — 262 Pruefungen, 3 Fehler
```

## Fall C6

```
# E-5 Rotlauf C6  Matrix C-07
# Kippung: vorhandenePipeUebernehmen gilt bei blosser Erreichbarkeit sofort als uebernommen und gibt das Startmutex frei, ohne auf das Handleurteil zu warten
# Ziele: EqCopIpcTest


===== EqCopIpcTest  exit=1 =====
SONDE-010 | v3-Envelope, Pipetoken, Backpressure und die zwei Clients

== A · Envelope-Fixture-Korpus (dieselbe dritte Partei wie Rust) ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== B · CRC32C und Envelope-Rundlauf ==
  [... 10 bestandene ok-Zeilen ausgelassen ...]

== C · Fuzz (deterministisch): Invarianten des angenommenen Frames ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]

== D · v3-Pipename als reine Funktion ==
  [... 13 bestandene ok-Zeilen ausgelassen ...]

== E · Backpressure: P0 · P1 · P2 ==
  [... 31 bestandene ok-Zeilen ausgelassen ...]

== E2 · P2-Schleuse: vorallokiert, ohne Allokation, replace-oldest ==
  [... 20 bestandene ok-Zeilen ausgelassen ...]

== F · Ratengrenze ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G · ControlClient und TelemetryClient auf einer Testpipe ==
  [... 15 bestandene ok-Zeilen ausgelassen ...]

== G1a · JSON-Minor wird bis zum C++-Empfaenger getragen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G1b · Produktstatus sendet Heartbeat und State-Report ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G2 · ungekoppelter Telemetry-Connect wird geschlossen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G3 · Reconnect nach Serverneustart ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G4 · ein kaputter Envelope vom Server schliesst die Verbindung ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G5 · P0-Ueberlauf schliesst die Verbindung, statt zu verwerfen ==
  [... 7 bestandene ok-Zeilen ausgelassen ...]

== G6 · ein P1-Ereignis ueberlebt einen gescheiterten Write ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G7 · der TelemetryClient prueft sein welcome vollstaendig ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== G8 · nicht endliche Audiofelder werden VOR der Wandlung verriegelt ==
  [... 5 bestandene ok-Zeilen ausgelassen ...]

== G9 · ein P0-Befehl ueberlebt einen gescheiterten Write bei voller Queue ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== G10 · beide Clients pruefen das welcome gegen den VOLLSTAENDIGEN Vertrag ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]

== G11 · ein P2-Frame auf der Control-Verbindung wird abgewiesen ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G12 · die Nachrichtenratengrenze gilt auch im Client ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G13 · die Telemetrie merkt im Leerlauf, dass ihre Kopplung fort ist ==
  [... 8 bestandene ok-Zeilen ausgelassen ...]

== G14 · was der TelemetryClient auf SEINER Verbindung annimmt ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G15 · zu grosse Nachrichten werden an der TUER abgewiesen ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G16 · ein P0-ACK kommt an, waehrend P1 rueckstaut ==
  [... 3 bestandene ok-Zeilen ausgelassen ...]

== G16b · der ACK trifft ein, WAEHREND der Write in seine Frist laeuft ==
  [... 2 bestandene ok-Zeilen ausgelassen ...]

== G17 · stop() kehrt in JEDEM Zustand zurueck ==
  [... 24 bestandene ok-Zeilen ausgelassen ...]

== G18 · die Telemetrie verliert die Verbindung mitten im Write ==
  [... 4 bestandene ok-Zeilen ausgelassen ...]

== H · Bootstrapgrenze und JSON-Riegel ==
  [... 18 bestandene ok-Zeilen ausgelassen ...]

== I · Phase B: semantisches command_ack und In-Flight-Register ==
  [... 8 bestandene ok-Zeilen ausgelassen ...]

== J · Phase B: Broker-Autostart und Signaturkette ==
  [... 6 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  vorhandene_pipe_wird_nur_mit_dem_authentisierten_handle_uebernommen
  [... 27 bestandene ok-Zeilen ausgelassen ...]
BEREIT \\.\pipe\evenacadia.nakama.v3.probe.test.5480.79.foreign-broker-owner
{"abweisungen":0,"acceptor_wartet_auf_instanz":0,"angenommen":1,"besitzlistener_nach_stopp":0,"besitzlistener_vor_stopp":2,"control_getrennt":1,"control_verbindungen":1,"ende_grund":"stdin","geschlossen_bootstrap":0,"geschlossen_envelope":0,"geschlossen_familie":0,"geschlossen_kopplung":0,"geschlossen_p0_ueberlauf":0,"geschlossen_rate":0,"geschlossen_writer":0,"ingress_hoechststand":0,"ingress_p1_ueberlauf_trennt":0,"ingress_p2_verworfen":0,"letzter_grund":"","p0":0,"p0_beantwortet":0,"p1":0,"p2":0,"pipe":"\\\\.\\pipe\\evenacadia.nakama.v3.probe.test.5480.79.foreign-broker-owner","telemetrie_getrennt":0,"telemetrie_verbindungen":0}
  [... 3 bestandene ok-Zeilen ausgelassen ...]

FEHLER — 262 Pruefungen, 1 Fehler
```

## Fall C7

```
# E-5 Rotlauf C7  Matrix C-09
# Kippung: PipeClient laesst eine einmal erteilte Serverfreigabe den Reconnect ueberleben und prueft den neuen Handle nicht erneut
# Ziele: EqCopPipeClientTest


===== EqCopPipeClientTest  exit=1 =====
PIPECLIENT-LIFECYCLE-TEST
  [... 7 bestandene ok-Zeilen ausgelassen ...]
  FEHLER  pipeclient_reconnect_verwirft_serverfreigabe
  FEHLER  pipeclient_sendet_vor_serverauth_keine_bytes
  [... 8 bestandene ok-Zeilen ausgelassen ...]
PIPECLIENT-LIFECYCLE-TEST FEHLGESCHLAGEN - 2 Fehler
```
