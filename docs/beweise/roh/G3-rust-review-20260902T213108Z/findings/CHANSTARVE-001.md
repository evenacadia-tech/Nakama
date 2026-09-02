---
id: CHANSTARVE-001
bug_class: channel-starvation
title: Blocking snapshot-ack recv gated on untrusted peer pipe reads stalls the single liveness thread and ordered shutdown
location: broker/src/transport/server_v3.rs:1046
function: snapshot_schreiben
confidence: High
worker: worker-4
fp_verdict: TRUE_POSITIVE
fp_rationale: "Every link verified in source: snapshot_schreiben ends in antwort.recv_timeout(SENKE_FRIST=2000ms) (server_v3.rs:1046), flush_session calls it serially per subscriber, SESSION_CLIENT_CAP=64, the only driver is the single nakama-coordinator-tick thread (lib.rs:918-927), and BrokerSupervisor::drop joins it with a bare join()."
severity: MEDIUM
attack_vector: Local
exploitability: Reliable
severity_rationale: "Local DoS of the broker's only supervisory thread — an untrusted peer merely stops reading its pipes and stalls eviction, tombstoning and fail-closed link closing for up to ~128 s per tick, and hangs broker_geordnet_stoppen; no race and no resource exhaustion needed."
---

## Description

`V3Sender::snapshot_schreiben` enqueues a P1 session snapshot into the
per-connection writer queue and then **blocks on a `sync_channel(1)` receiver**
until the writer thread reports the frame as written, or `SENKE_FRIST` (2000 ms)
elapses. The sender half of that channel is only fired by the connection's
writer thread *after* `ov_schreiben` returns — and `ov_schreiben` waits
`WaitForSingleObject(e, INFINITE)` on the overlapped `WriteFile`
(`broker/src/transport/server_v3.rs:648`). A peer that stops draining its pipe
therefore parks the writer thread indefinitely (the named pipe out-buffer is
65536 bytes, `CreateNamedPipeW` at `broker/src/transport/server_v3.rs:400`),
and every `snapshot_schreiben` for that link burns the full 2 s timeout.

The wait is not per-broker but **per subscriber, serially**: `flush_session`
loops over every subscribed link of the session and calls `snapshot_schreiben`
for each (`broker/src/coordinator.rs:2273-2283`). The same loop is driven from
the process's **single** liveness thread `nakama-coordinator-tick`
(`broker/src/lib.rs:918-927`), which is the only thread that evicts stale
clients, applies tombstones and returns the link ids that
`V3Closer::link_schliessen` must close.

A local unprivileged process running as the same user (the pipe DACL and the
NAK-123 SID check admit exactly that principal) can open several v3 control
connections, subscribe them all to one session, and then simply never read from
its pipes. Each liveness tick then blocks `N * 2 s` inside `flush_session`. With
the session client cap of 64 (`SESSION_CLIENT_CAP`,
`broker/src/coordinator.rs:33`) that is up to ~128 s of dead time per tick on
the broker's only supervisory thread.

The same wait also stalls the ordered shutdown: `BrokerSupervisor::drop`
(`broker/src/lib.rs:830-837`) joins the tick thread **without a deadline**, so
`broker_geordnet_stoppen()` inherits the whole stall.

The blocking is inconsistent with the module's own rules. The sibling P2 path
is explicitly documented as never waiting on pipe I/O
(`SessionPush::messframe_schreiben`, `broker/src/coordinator.rs:206-211`:
"Der Broker wartet hier nie auf Pipe-I/O"), and the server side wraps every
lifecycle sink call in `Senkenruf` precisely so that a slow peer cannot pin a
broker thread (`broker/src/transport/server_v3.rs:1330-1355`). The
coordinator-to-sink direction has no such protection.

## Code

```rust
// broker/src/transport/server_v3.rs:1028
pub fn snapshot_schreiben(&self, link_id: &str, payload: &[u8]) -> bool {
    let frame = match envelope_schreiben(Familie::P1, P1_SCHEMA_MINOR, payload) {
        Ok(frame) => frame,
        Err(_) => return false,
    };
    let ausgang = self
        .control_ausgaenge
        .lock()
        .unwrap_or_else(|e| e.into_inner())
        .get(link_id)
        .cloned();
    let Some(ausgang) = ausgang else {
        return false;
    };
    let Some(antwort) = ausgang.snapshot_einreihen_mit_antwort("session_snapshot", frame)
    else {
        return false;
    };
    // The Sender lives inside the queued entry; it only fires after the writer
    // thread returns from ov_schreiben -> WaitForSingleObject(.., INFINITE).
    antwort.recv_timeout(SENKE_FRIST).unwrap_or(false)
}
```

```rust
// broker/src/coordinator.rs:2273 - the serial amplifier
for (link_id, ziel) in ziele {
    let geschrieben = self.push_ziel_noch_gueltig(&link_id, &ziel)
        && push
            .as_ref()
            .is_some_and(|push| push.snapshot_schreiben(&link_id, &payload));
    ...
}
```

```rust
// broker/src/lib.rs:918 - the single thread that pays for it
let supervisor_join = std::thread::Builder::new()
    .name("nakama-coordinator-tick".into())
    .spawn(move || {
        while !stop_fuer_thread.load(Ordering::SeqCst) {
            std::thread::sleep(Duration::from_millis(100));
            for link_id in coordinator_fuer_thread.liveness_tick() {
                closer.link_schliessen(&link_id);
            }
        }
    })
```

## Data flow

- **Source:** a local unprivileged process of the same logon SID opens N v3
  control pipes, completes the duplex handshake, sends `subscribe_session` for
  one shared session, and then stops issuing `ReadFile` on its pipe handles.
  Frame rate and read behaviour are fully peer-controlled.
- **Sink:** `antwort.recv_timeout(SENKE_FRIST)` at
  `broker/src/transport/server_v3.rs:1046`, reached once per subscribed link per
  session flush.
- **Validation:** none on the number of blocking waits per tick. The only bound
  is the per-call `SENKE_FRIST` (2 s); it is not a budget for the loop, for the
  tick, or for shutdown.

## Reachability trace

`nakama-coordinator-tick` thread → `Coordinator::liveness_tick`
(`broker/src/coordinator.rs:1502`) → `flush_session`
(`broker/src/coordinator.rs:1546`) → `push.snapshot_schreiben`
(`broker/src/coordinator.rs:2277`) → `V3Sender::snapshot_schreiben`
(`broker/src/transport/server_v3.rs:1028`) → `recv_timeout(SENKE_FRIST)`
(`:1046`), blocked until the attacker's writer thread leaves
`ov_schreiben`/`WaitForSingleObject(INFINITE)` (`:648`).

Shutdown variant: `broker_geordnet_stoppen` (`broker/src/lib.rs:969`) → drop of
`BrokerSupervisor` (`broker/src/lib.rs:830`) → unbounded `join()` on the same
stalled tick thread.

## Impact

A local unprivileged peer suspends the broker's liveness machinery for tens of
seconds to minutes at a time without sending a single malformed byte:

- stale-client detection, tombstoning and eviction stop running, so a client
  the coordinator has already flagged `trennen` keeps its session slot and its
  place under `SESSION_CLIENT_CAP` / `MAX_KOPPLUNGEN`;
- `V3Closer::link_schliessen` is never called for links the coordinator wants
  closed, i.e. the fail-closed disconnect path is disarmed for the duration;
- an ordered `broker_geordnet_stoppen()` hangs for the same period, so the
  supervised restart/shutdown path an operator or installer relies on becomes
  arbitrarily slow.

The same wait also runs on victim connections' P1 ingress threads (via
`subscribe_session` and `heartbeat_kontakt` → `flush_session`), so an attacker
can additionally stall an innocent client's control path.

## Mitigations checked

- `SENKE_FRIST` (2000 ms) bounds a *single* call, not the per-tick loop; there
  is no aggregate deadline, no per-tick budget and no skip-after-first-timeout.
- `Senkenruf` + `join_mit_frist` protect the server→sink direction only
  (`broker/src/transport/server_v3.rs:1330-1400`); the coordinator→sink
  direction used here is unwrapped.
- Snapshot coalescing in `Ausgang::einreihen_eintrag`
  (`broker/src/transport/server_v3.rs:872-905`) bounds queue growth but not the
  wait: the pending entry stays queued behind the parked writer, so the caller
  still consumes the full timeout.
- The rate limit `RATE_PRO_SEKUNDE` and `MAX_VERBINDUNGEN` bound ingress, not
  the number of subscribers a flush walks.
- `#![deny(unsafe_op_in_unsafe_fn)]`, `[lints]` table, MIRI or sanitizer
  coverage: none present in `broker/Cargo.toml`; irrelevant to this defect in
  any case.

## Recommendation

Do not let the coordinator block on peer-controlled pipe I/O from a shared
thread. Either

1. make the P1 snapshot push non-blocking like the P2 path — enqueue and treat
   "written" as the writer thread's later acknowledgement, driving the
   outbox/snapshot-debt compaction from the writer instead of from
   `flush_session`; or
2. keep the ack but give the *loop* a budget: track a per-flush deadline, and
   after the first `recv_timeout` expiry mark the link as back-pressured, skip
   its remaining pushes for this tick and let the existing
   `geschlossen_writer` / `trennen` machinery close it, exactly as
   `Senkenruf::stumm` already does on the server side.

Additionally, bound `BrokerSupervisor::drop` with a `join_mit_frist`-style
deadline so an ordered stop cannot inherit a peer-controlled stall.
