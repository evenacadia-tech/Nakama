---
id: DROPPANIC-001
bug_class: drop-panic
title: Drop of RegistrierteVerbindung panics on a poisoned Coordinator mutex, turning a single thread panic into a process abort
location: broker/src/server.rs:347
function: RegistrierteVerbindung::drop
confidence: High
worker: worker-12
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: RegistrierteVerbindung::drop (server.rs:342-349) guards its first lock poison-safely and then calls senke.getrennt, which reaches hoermarkierung_v2_getrennt's .expect(\"Coordinator vergiftet\") (coordinator.rs:2430) — one of 54 such sites — and Cargo.toml has no [profile], so panic=unwind is active."
severity: MEDIUM
attack_vector: Local
exploitability: Difficult
severity_rationale: "Panic inside Drop during cleanup: the second panic while unwinding is an unconditional process abort, turning a single-thread failure into loss of every session, the store writer and all NAK-123 teardown guarantees; held at the MEDIUM floor for drop-panics despite needing a prior poisoning panic."
---

## Description

`RegistrierteVerbindung` is the RAII teardown guard of every v2 pipe connection
thread. Its `drop` calls `senke.getrennt(...)`, which in production is
`Coordinator::hoermarkierung_v2_getrennt` (wired at `broker/src/lib.rs:896` via
`server_starten_mit_interventionssenke`). That function acquires the coordinator
state mutex with `.expect("Coordinator vergiftet")` — it **panics** when the
mutex is poisoned.

The first statement of the same `drop` is deliberately panic-free
(`if let Ok(mut r) = self.register.lock()`), so the author was aware that a
destructor must not unwrap a lock; the second statement then reaches a
panicking lock through a trait object.

The escalation path needs exactly one prior panic anywhere under the coordinator
lock:

1. Any thread panics while holding `Coordinator::stand` (e.g. the `expect`
   under the same lock at `broker/src/coordinator.rs:1907`,
   `stand.clients.get_mut(key).expect("Clientschluessel")`). The mutex is now
   poisoned for the rest of the process lifetime — nothing in the crate calls
   `clear_poison`.
2. A v2 connection thread is inside its heartbeat loop with `registrierung`
   alive on its stack and calls
   `senke.hoermarkierung(...)` (`broker/src/server.rs:810`) →
   `Coordinator::hoermarkierung_v2` (`broker/src/coordinator.rs:2411`) →
   `.expect("Coordinator vergiftet")` → **panic**, thread starts unwinding.
3. Unwinding drops `registrierung` → `RegistrierteVerbindung::drop` →
   `getrennt` → `.expect("Coordinator vergiftet")` → **panic during unwinding**.

A panic raised while a panic is already unwinding is an unconditional
`abort()` in the Rust runtime. `broker/Cargo.toml` declares no `[profile]`
section, so the release profile keeps the default `panic = "unwind"`; without a
drop panic the first panic would only have killed one connection thread and the
broker would keep serving every other probe.

## Code

```rust
// broker/src/server.rs:341
impl Drop for RegistrierteVerbindung {
    fn drop(&mut self) {
        if let Ok(mut r) = self.register.lock() {      // poison-safe on purpose
            r.sensor_trennen(&self.sensor_id, &self.nonce);
        }
        if let Some(senke) = &self.interventionssenke {
            senke.getrennt(&self.interventions_link_id); // <-- can panic
        }
    }
}

// broker/src/coordinator.rs:2429 — the sink behind that trait object
pub fn hoermarkierung_v2_getrennt(&self, link_id: &str) {
    let mut stand = self.stand.lock().expect("Coordinator vergiftet");
    stand
        .interventionen
        .retain(|_, intervention| intervention.link_id != link_id);
    stand.intervention_state_unknown = true;
}
```

## Data flow

- **Source:** any local unprivileged process that can open the v2 pipe and drive
  a connection to the point where a panic occurs under `Coordinator::stand`
  (poisoning step), then any further v2 connection that reaches
  `senke.hoermarkierung(...)` in its heartbeat loop.
- **Sink:** `Mutex::lock().expect(...)` at `broker/src/coordinator.rs:2430`,
  reached from `RegistrierteVerbindung::drop` at `broker/src/server.rs:347`.
- **Validation:** none — the poisoned state is never cleared, never queried, and
  the destructor has no panic guard (`std::thread::panicking()` is not checked
  and the call is not wrapped in `catch_unwind`).

## Reachability trace

`server_starten_mit_interventionssenke` (lib.rs:896) → per-connection thread
`verbindung_bedienen` (server.rs:~643) → `RegistrierteVerbindung` constructed at
`server.rs:720` → heartbeat loop calls `senke.hoermarkierung` (server.rs:810) →
panic on poisoned mutex → unwind → `RegistrierteVerbindung::drop`
(server.rs:342) → `senke.getrennt` (server.rs:347) →
`Coordinator::hoermarkierung_v2_getrennt` (coordinator.rs:2430) → second panic →
`abort()`.

## Impact

Whole-process termination of `eqcop-broker.exe`, not a single-connection
failure. Every probe session, the single-writer SQLite store thread, the v3
transport and its listeners die at once; unflushed WAL work and the outbox
in-flight state are lost, and `abort()` skips all remaining destructors, so the
graceful teardown paths that NAK-123 added (listener name ownership released
last, `RevertToSelf` cleanup, handle register drain) never run. Under
`LOCAL_UNPRIVILEGED` this converts any single reachable panic-under-lock into a
reliable denial of service against all users of the broker.

## Mitigations checked

- `// SAFETY:` comments: not applicable (safe code), and no comment documents an
  accepted panic in this destructor.
- Panic guard: `drop` does not check `std::thread::panicking()`, does not
  `catch_unwind` the sink call, and the crate never calls `Mutex::clear_poison`.
- `panic = "abort"` would make this moot — but `broker/Cargo.toml` has no
  `[profile]` table, so unwinding is active in both dev and release.
- The sibling statement in the same `drop` (`if let Ok(mut r) = ...lock()`) and
  the coordinator's own `unwrap_or_else(|e| e.into_inner())` style at
  `coordinator.rs:227/635/1078` show the poison-tolerant idiom is available and
  used elsewhere; only this destructor path uses the panicking variant.
- No MIRI run, no `[lints]` table, no `#![deny(...)]` in `broker/src/lib.rs`.
- `debug_assertions` is irrelevant here: `expect` panics in release too.

## Recommendation

Make the destructor panic-free, matching its first statement:

```rust
fn getrennt(&self, link_id: &str) {
    Coordinator::hoermarkierung_v2_getrennt(self, link_id);
}
// and in coordinator.rs
pub fn hoermarkierung_v2_getrennt(&self, link_id: &str) {
    let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
    ...
}
```

Switching the coordinator's `.expect("Coordinator vergiftet")` sites to
`unwrap_or_else(|e| e.into_inner())` (already the crate's idiom in
`push`/`sicht` locks) removes both the poisoning escalation and this abort. If
fail-fast on poison is intended policy, it must not be executed from a
destructor: guard the call with `if std::thread::panicking() { return; }` and
report the poisoned state through the register instead.
