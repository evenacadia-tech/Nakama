---
id: RESDISC-002
bug_class: result-discarded
title: v3 connection-worker join results are discarded, hiding every panicking worker thread
location: broker/src/transport/server_v3.rs:1426
function: fertige_ernten
confidence: High
worker: worker-12
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified at server_v3.rs:1426: fertige_ernten drops the thread::Result with let _ = j.join(), and the v2 side's join_auswerten (server.rs:265) proves the crate knows the counter-pattern; V3Statistik has no panic counter."
severity: LOW
attack_vector: Local
exploitability: Reliable
severity_rationale: "Loss of a detection control rather than a vulnerability: a panicking worker on the fully attacker-facing parse path leaves no trace anywhere, giving a silent crash oracle, but no direct availability or integrity effect beyond the one connection that already died."
---

## Description

`fertige_ernten` is the steady-state reaper for v3 connection worker threads: it
is called by the acceptor on every accept round, pulls all finished
`JoinHandle<()>`s out of the shared vector and joins them. The join result —
`std::thread::Result<()>`, i.e. `Err(Box<dyn Any>)` exactly when that worker
died by panic — is dropped with `let _ = j.join();`.

Every v3 worker thread runs the fully attacker-facing code: envelope framing and
CRC checks, the bootstrap handshake, JSON/FlatBuffers parsing and the queue
handoff, all fed by bytes from a local unprivileged pipe client. A panic in any
of those paths is the primary signal that untrusted input reached an
unsatisfied invariant. Here that signal is thrown away without a counter, a log
line, or a statistic: `V3Statistik` has fields for `lebenszyklus_uebersprungen`,
`lebenszyklus_abgeloest`, `listener_fehler` and `bewaffnete_listener`, but
nothing for "worker ended by panic". After the join, the thread is simply gone
from the vector and the broker reports normal operation.

The v2 side of the same crate does the opposite and shows the intended
contract — `join_auswerten` (`broker/src/server.rs:265`) inspects the identical
`Result` and records `"{art} ist mit Panic beendet"` in the register:

```rust
fn join_auswerten(join: JoinHandle<()>, register: &Arc<Mutex<Register>>, art: &str) {
    if join.join().is_err() {
        fehler_merken(register, format!("{art} ist mit Panic beendet"));
    }
}
```

The v3 transport lost that. The same discard is repeated on the shutdown path in
`V3Griff::stoppen` (`:1234`, `:1237`, `:1253`) and in `join_mit_frist`
(`:1323`), but `fertige_ernten` is the one on the hot, always-running path where
the diagnostic actually matters.

## Code

```rust
// broker/src/transport/server_v3.rs:1409
fn fertige_ernten(verbindungen: &Arc<Mutex<Vec<JoinHandle<()>>>>) {
    let fertig: Vec<JoinHandle<()>> = {
        let mut v = verbindungen.lock().unwrap_or_else(|e| e.into_inner());
        let mut raus = Vec::new();
        let mut i = 0;
        while i < v.len() {
            if v[i].is_finished() {
                raus.push(v.remove(i));
            } else {
                i += 1;
            }
        }
        raus
    };
    for j in fertig {
        let _ = j.join();   // <-- Err(..) == "this worker panicked" is dropped
    }
}
```

## Data flow

- **Source:** envelope bytes from any local unprivileged process connected to the
  v3 pipe; they are parsed inside the worker thread that is later joined here.
- **Sink:** `JoinHandle::join()` at `broker/src/transport/server_v3.rs:1426`,
  whose `Err` variant carries the worker's panic payload and is discarded.
- **Validation:** none — the result is neither matched, counted, nor logged, and
  no statistic distinguishes a clean worker exit from a panicking one.

## Reachability trace

`v3_server_starten` → acceptor thread loop → `fertige_ernten(&verbindungen)` on
each round → `j.join()` for every finished `verbindung_bedienen` worker →
panic payload dropped. `WorkerPlatz::drop` still decrements the active-worker
counter during the unwind, so even the counter looks normal.

## Impact

A local unprivileged attacker who finds any input that panics a v3 worker gets
an entirely silent crash oracle: each malformed envelope kills one connection
thread with no trace anywhere in the broker's health surface, statistics, or
register. Operators cannot distinguish "the probe disconnected" from "the parser
crashed on crafted input", so a working memory-safety or logic probe can be
iterated indefinitely without detection, and the eventual reliability symptom
(dropped telemetry, missing sessions) has no attributable cause. This is the
loss of the detection control for the exact input surface the NAK-123 hardening
was written for.

## Mitigations checked

- `join_auswerten` in `broker/src/server.rs:265` proves the crate knows how to
  handle this `Result` — the v3 path just does not use it.
- No `catch_unwind` inside `verbindung_bedienen`, so a panicking worker leaves no
  other artifact.
- `unused_must_use` does not fire on `let _ = expr`; there is no `[lints]` table
  and no `#![deny(unused_results)]` in the crate.
- `V3Statistik` was reviewed: no panic/abnormal-exit counter exists.
- `broker/tests/security_vectors.rs` asserts handle/thread/name cleanup on
  failure paths, but nothing asserts that a panicking worker is reported.

## Recommendation

Mirror the v2 behaviour and make a panicking worker observable:

```rust
for j in fertig {
    if j.join().is_err() {
        statistik.worker_panics.fetch_add(1, Ordering::SeqCst);
    }
}
```

Add a `worker_panics: AtomicU64` to `V3Statistik`, surface it next to
`listener_fehler` in the health view, and apply the same treatment to the
shutdown joins at `:1234`, `:1237`, `:1253` and to `join_mit_frist` at `:1323`.
