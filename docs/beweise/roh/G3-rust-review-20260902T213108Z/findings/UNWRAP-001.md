---
id: UNWRAP-001
bug_class: unwrap-on-untrusted
title: Coordinator state lock uses expect() on the poison result, turning any single panic into a permanent whole-broker outage
location: src/coordinator.rs:3096
function: p2
confidence: Low
worker: worker-7
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: rg counts 54 .expect(\"Coordinator vergiftet\") sites in coordinator.rs, and p2 takes self.push poison-tolerantly at :3095 then self.stand poison-fatally two lines later at :3096, while the transport layer uses unwrap_or_else(|e| e.into_inner()) throughout — but the worker states honestly that no attacker-controlled first panic under the guard was found."
severity: LOW
attack_vector: Local
exploitability: Theoretical
severity_rationale: "Missing containment rather than a triggerable bug: no first panic under the lock is demonstrably attacker-reachable in release, so this is the amplifier that would turn DROPPANIC-001 or any future panic into a permanent whole-broker outage, not an independent DoS primitive."
---

## Description
The coordinator acquires its central state mutex with
`.lock().expect("Coordinator vergiftet")` at 41 production sites, including
every one of the three untrusted-message entry points (`p0_json_mit_minor`,
`p1_mit_minor`, `p2`) and every connection-lifecycle callback
(`control_verbunden`, `control_getrennt`, `telemetrie_gekoppelt`,
`telemetrie_getrennt`).

`Mutex::lock` returns `Err(PoisonError)` for the rest of the process lifetime
once **any** thread panics while holding the guard. Because the coordinator
propagates that error with `expect`, a single panic anywhere under the lock
does not degrade one connection — it converts every subsequent P0/P1/P2 message
and every connect/disconnect callback on **every** connection into a fresh
panic. The broker's business logic is then permanently dead while the process
keeps running and the pipe keeps accepting connections.

This is a deliberate divergence from the rest of the crate. The transport layer
consistently chose the poison-tolerant form for exactly this reason:

- `src/transport/server_v3.rs:2410` `kopplungen.lock().unwrap_or_else(|e| e.into_inner())`
- `src/transport/server_v3.rs:2549` `telemetrie_ausgaenge` ... `.lock().unwrap_or_else(|e| e.into_inner())`
- `src/transport/server_v3.rs:2571` `control_ausgaenge` ... `.lock().unwrap_or_else(|e| e.into_inner())`
- `src/coordinator.rs:2186` `self.session_flush_schloesser[..].lock().unwrap_or_else(|e| e.into_inner())`
- `src/coordinator.rs:3094` `self.push.lock().unwrap_or_else(|e| e.into_inner())`

Line 3094 and line 3096 sit two lines apart inside the same function: the push
handle is taken poison-tolerantly, the state lock two lines later is not.

The sink callbacks additionally run on detachable helper threads
(`Senkenruf::rufen` / `rufen_mit_ergebnis`, `src/transport/server_v3.rs:1361`),
so a panic inside a coordinator callback is *silently* absorbed by the joining
side (`join_mit_frist` just counts `senke_abgeloest`) — the poison it leaves
behind is the only lasting evidence, and it is not surfaced anywhere.

## Code
```rust
// src/coordinator.rs:3081 — the untrusted telemetry entry point
fn p2(&self, link_id: &str, payload: &[u8]) {
    let batch = match crate::telemetrie::fuer_broker(payload) {
        Ok(batch) => batch,
        Err(_) => { self.messframe_abweisen(link_id, P2RejectGrund::FeatureBatchUngueltig); return; }
    };
    if batch.frames.len() != 1 { /* ... */ return; }
    let frame = &batch.frames[0];
    let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();   // poison-tolerant
    {
        let mut stand = self.stand.lock().expect("Coordinator vergiftet");    // poison-fatal
```

```rust
// src/transport/server_v3.rs:2405 — how the same problem is handled one layer up
let mut k = kopplungen.lock().unwrap_or_else(|e| e.into_inner());
```

## Data flow
- **Source:** any P0/P1/P2 frame or connect/disconnect event on the pipe drives
  execution into a `self.stand.lock().expect(...)`; the `Result` being unwrapped
  is the mutex poison state, not the parsed message itself.
- **Sink:** `expect("Coordinator vergiftet")` at `src/coordinator.rs:3096`
  (and 40 sibling sites).
- **Validation:** none — the poisoned case is not handled anywhere; there is no
  `into_inner()` recovery, no restart of the coordinator state, no
  `catch_unwind` around the sink callbacks, and no counter that reports the
  condition.

## Reachability trace
`verbindung_bedienen` (`src/transport/server_v3.rs:2252`) → ingress consumer
thread → `senke.p2_mit_minor(&link, schema_minor, &payload)`
(`src/transport/server_v3.rs:2685`) → `Coordinator::p2`
(`src/coordinator.rs:3081`) → `self.stand.lock().expect(...)`
(`src/coordinator.rs:3096`).

Honest caveat: I did **not** find a proven attacker-controlled panic under the
lock in a release build — the reachable `expect`/`unwrap` sites inside the guard
(`stand.clients.get_mut(key).expect("Clientschluessel")` at
`src/coordinator.rs:1907`/`:1951`, `client.state_hash.clone().expect("oben geprueft")`
at `:2690`, `serde_json::from_slice(...).expect("interner Session-Snapshot ist JSON")`
at `:2696`) are all guarded by an immediately preceding check. What this
finding reports is that the *containment* is absent: the codebase has no
mechanism that keeps a first panic — from a future edit, a debug/`cargo test`
build where `debug_assert!`/overflow checks are live, or an
allocator/third-party panic under the guard — from escalating into a permanent
service outage. That escalation is the vulnerability; the FP judge should weigh
it as defence-in-depth versus a directly triggerable bug.

## Impact
One panic on any coordinator path permanently disables every session, every
telemetry route, every heartbeat ACK and every disconnect cleanup, while the
process stays alive and the acceptor keeps handing out connections. Clients see
an apparently healthy broker that answers nothing. Nothing recovers the state
short of killing the process; the broker's own idle-shutdown path
(`broker_idle_ende`, `src/lib.rs:1002`) also runs through locks and would panic
too. Because 41 sites share one panic message, an operator reading the log sees
only "Coordinator vergiftet" and no hint of the original fault.

## Mitigations checked
- No `catch_unwind` anywhere in `broker/src`; `Senkenruf` detaches slow callback
  threads but does not intercept panics.
- No `panic = "abort"` in `broker/Cargo.toml` (there is no `[profile]` section
  at all), so a panic unwinds and *does* poison rather than aborting the
  process — an abort would at least be a visible, restartable failure.
- No `[lints]` table, no `rust-toolchain.toml`, no clippy gate that would flag
  `expect` on a `LockResult`.
- `V3Statistik` counts `senke_abgeloest` / `lebenszyklus_abgeloest`, but a
  panicking callback is indistinguishable from a slow one in that counter.
- Poison-tolerant `unwrap_or_else(|e| e.into_inner())` is used consistently in
  the transport layer and for `self.push` / `session_flush_schloesser`, proving
  the pattern is known and intentional elsewhere.

## Recommendation
Use the same poison-tolerant form the transport layer already uses, so a first
panic costs one message instead of the whole service:

```rust
let mut stand = self.stand.lock().unwrap_or_else(|e| e.into_inner());
```

Apply it to all 41 `expect("Coordinator vergiftet")` sites (a small helper such
as `fn stand(&self) -> MutexGuard<'_, Stand>` makes this one change instead of
41). Where continuing on a possibly-inconsistent state is unacceptable, prefer
an explicit degraded mode — set the existing `routing_fail_closed` /
`intervention_state_unknown` flags and keep serving disconnects — over a panic
cascade. Additionally, wrap the sink callbacks in `catch_unwind` at the
`Senkenruf` boundary so a coordinator panic is recorded and contained to one
connection.
