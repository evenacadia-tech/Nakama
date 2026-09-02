---
id: REENTRANT-002
bug_class: reentrancy-unsafe
title: Coordinator holds the global `stand` mutex across the `dyn SessionPush` callback loop when replaying held P2 frames to a subscriber
location: broker/src/coordinator.rs:1124
function: messframes_an_subscriber_push
confidence: Medium
worker: worker-4
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: messframes_an_subscriber_push binds the stand guard to a named local at coordinator.rs:1097, so it is still held across the per-frame push.messframe_schreiben loop at :1123-1125 — the same construct as REENTRANT-001 at a second, independently reachable site."
severity: MEDIUM
attack_vector: Local
exploitability: Reliable
severity_rationale: "Same mechanism and same magnitude as REENTRANT-001 (up to SESSION_CLIENT_CAP=64 held frames of up to 256 KiB CRC-framed under the global lock per subscriber attach), and an attacker controls the attach rate, so it gets the same MEDIUM local-DoS tier for consistency."
---

## Description

`Coordinator::messframes_an_subscriber_push` binds the guard for the broker's
single global state mutex to a named local at `broker/src/coordinator.rs:1097`
(`let stand = self.stand.lock()...`). Because it is a named binding rather than
a scoped block, the guard lives to the end of the function body — including the
loop at `:1123-1125` that calls out through the `Arc<dyn SessionPush>` trait
object once per held live frame.

This is the same defect as REENTRANT-001 at a second, independently reachable
site (subscriber attach rather than telemetry ingress), and it likewise
contradicts the invariant `flush_session` documents at
`broker/src/coordinator.rs:2267-2271` ("Externe Pipe-Arbeit laeuft danach ohne
dieses Schloss; eine Senke darf den Coordinator **reentrant** beobachten").

`self.stand` is a non-reentrant `std::sync::Mutex`. A `SessionPush`
implementation that acts on the documented permission to observe the coordinator
from inside the callback self-deadlocks the calling thread and, because `stand`
is the one global lock, blocks every other broker thread behind it. The trait is
`pub` and `messframe_schreiben` has a default body
(`broker/src/coordinator.rs:210-212`), so the contract — not the current
`V3Sender` — is what governs.

The iteration count is peer-influenced: `frames` is built from
`stand.messframes` filtered to the session, i.e. one entry per live probe
instance in the session, bounded only by `SESSION_CLIENT_CAP` = 64
(`broker/src/coordinator.rs:33`), and each payload is a peer-supplied P2 batch
of up to `MAX_FRAME_BYTES` = 262144 bytes that is cloned under the lock at
`:1120` and then CRC-framed by `envelope_schreiben` inside the callback — all
while the global mutex is held.

## Code

```rust
// broker/src/coordinator.rs:1094
fn messframes_an_subscriber_push(&self, link_id: &str) {
    let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
    let Some(push) = push else { return };
    let stand = self.stand.lock().expect("Coordinator vergiftet");   // <- named guard,
    let Some(sub) = stand.subscriptions.get(link_id) else {          //    lives to the
        return;                                                     //    end of the fn
    };
    let Some(ziel_link) = stand.links.get(link_id) else {
        return;
    };
    if ziel_link.trennen
        || !stand.routing_bereit
        || ziel_link.adresse != sub.adresse
        || !self.alias_register.session_push_erlaubt(/* ... */)
    {
        return;
    }
    let session = ziel_link.client_key.session();
    let mut frames = stand
        .messframes
        .iter()
        .filter(|(key, _)| key.session() == session)
        .map(|(key, frame)| (key.instance_id.clone(), frame.payload.clone()))
        .collect::<Vec<_>>();
    frames.sort_by(|a, b| a.0.cmp(&b.0));
    for (instance_id, payload) in frames {
        // dyn-dispatch into the transport layer WHILE `stand` is held.
        let _ = push.messframe_schreiben(link_id, &instance_id, &payload);
    }
}
```

The `frames` vector is already an owned snapshot at `:1121`; nothing after that
line reads `stand`, so the guard is held purely by accident of binding scope.

## Data flow

- **Source:** a local unprivileged peer triggers the call by coupling a
  telemetry connection (`telemetrie_gekoppelt`) or by (re)subscribing a control
  link (`subscribe_session` → `resubscribe_snapshot_push`). The replayed
  payloads are earlier peer-supplied P2 batches held in `stand.messframes`.
- **Sink:** `push.messframe_schreiben(link_id, &instance_id, &payload)` at
  `broker/src/coordinator.rs:1124`, executed with the `Coordinator::stand`
  guard acquired at `:1097` still alive.
- **Validation:** none — no bound on loop iterations under the lock, and no
  enforcement that the sink is safe to call reentrantly-free.

## Reachability trace

Path A (couple): attacker telemetry bootstrap →
`verbindung_bedienen` → `senkenruf.rufen(|s| s.telemetrie_gekoppelt(..))`
(`broker/src/transport/server_v3.rs:2556`) → `Coordinator::telemetrie_gekoppelt`
(`broker/src/coordinator.rs:3032`) → `messframes_an_subscriber_push`
(`:3043`) → `stand` locked at `:1097` → callback at `:1124`.

Path B (subscribe): attacker P1 `subscribe_session` →
`Coordinator::subscribe_json_mit_minor` (`broker/src/coordinator.rs:934`) →
`resubscribe_snapshot_push` (`:1119` call at `:985`) →
`messframes_an_subscriber_push` (`:1087`) → same sink.

## Impact

- Latent global deadlock: a `SessionPush` implementation that observes the
  coordinator inside `messframe_schreiben` — permitted by the coordinator's own
  documented contract — wedges the broker's single global mutex permanently.
  All connection threads, the store flush path and the `nakama-coordinator-tick`
  liveness thread then block forever; the process must be killed.
- Present-day contention: the global lock is held across up to 64 iterations of
  allocate + CRC + two further mutex acquisitions, repeated on every
  subscribe/couple event, which a local peer can issue at will within the
  connection cap.

## Mitigations checked

- The invariant is documented (`broker/src/coordinator.rs:2267-2271`) and
  honoured at the snapshot push sites (`:1078-1082`, `:2272-2277`), but it is
  not expressed as a type, lint or assertion, and it is violated here and in
  `Coordinator::p2`.
- `Senkenruf`/`join_mit_frist` (`broker/src/transport/server_v3.rs:1330-1400`)
  bound only the *server → sink* direction; this call is *coordinator → sink*.
- `V3Sender::messframe_schreiben` happens to be non-blocking, which bounds the
  hold time today, but that is a property of one implementation of a `pub`
  trait with a defaulted method, not of the contract.
- `broker/Cargo.toml` has no `[lints]` table and the project runs no MIRI or
  loom coverage; nothing mechanical would catch a future reentrant sink.

## Recommendation

Release the guard before the callback loop — the owned `frames` vector already
makes this a one-line change:

```rust
let (frames, ) = {
    let stand = self.stand.lock().expect("Coordinator vergiftet");
    /* existing checks and collection */
    (frames, )
};                                   // stand released here
for (instance_id, payload) in frames {
    let _ = push.messframe_schreiben(link_id, &instance_id, &payload);
}
```

Apply the same change at `Coordinator::p2` (see REENTRANT-001) and record the
"no coordinator lock is held across a `SessionPush` call" rule on the
`SessionPush` trait definition itself, so it is visible where implementations
are written rather than only inside one caller's comment.
