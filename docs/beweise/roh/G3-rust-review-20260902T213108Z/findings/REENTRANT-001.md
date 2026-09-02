---
id: REENTRANT-001
bug_class: reentrancy-unsafe
title: Coordinator holds the global `stand` mutex across the `dyn SessionPush` callback loop on the P2 ingress path
location: broker/src/coordinator.rs:3191
function: p2
confidence: Medium
worker: worker-4
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: the stand guard taken at coordinator.rs:3096 is still alive at the push.messframe_schreiben loop (:3189-3191) and is only released when the block closes at :3194, contradicting the invariant flush_session documents and honours at :2267-2277."
severity: MEDIUM
attack_vector: Local
exploitability: Reliable
severity_rationale: "The broker's single global lock is held across subscriber-count x (allocate + CRC32C over up to 256 KiB + two further mutexes) per attacker-supplied P2 frame, with frame rate, payload size and subscriber count all peer-chosen — a local DoS by lock serialisation today, plus a latent global deadlock for any sink that takes the documented reentrancy permission."
---

## Description

`Coordinator::p2` — the P2 telemetry ingress entry point of the `Senke`
implementation — acquires `self.stand`, the broker's single global state mutex,
at `broker/src/coordinator.rs:3096` and keeps the guard alive until the block
closes at `:3194`. Inside that scope, at `:3189-3192`, it calls out through the
`Arc<dyn SessionPush>` trait object once per subscriber of the session.

That contradicts the invariant the same file states 900 lines earlier, in
`flush_session`, where the shard guard is explicitly dropped before any sink
call (`broker/src/coordinator.rs:2267-2271`):

> Externe Pipe-Arbeit laeuft danach ohne dieses Schloss; eine Senke darf den
> Coordinator **reentrant** beobachten, ohne dieselbe Session zu deadlocken.

`self.stand` is a `std::sync::Mutex`, which is not reentrant: a `SessionPush`
implementation that takes the coordinator up on the documented permission to
observe it reentrantly — any call into `sicht()`, `interventionssicht()`,
`dispatch_fuer_link_erlaubt()`, `main_darf_schreiben()`, `control_ende()`, … —
deadlocks the calling thread permanently, and because `stand` is the broker's
one global lock, every other connection thread, the store flush path and the
`nakama-coordinator-tick` liveness thread pile up behind it. The trait is `pub`
and `messframe_schreiben` even ships a default body
(`broker/src/coordinator.rs:210-212`), so the contract, not the current
implementation, is what constrains future sinks.

Even with today's `V3Sender`, which does not re-enter, the pattern has a direct
consequence: the global `stand` mutex is held while the coordinator performs
`envelope_schreiben` (allocation plus CRC over up to `MAX_PAYLOAD_BYTES`) and
two further mutex acquisitions (`V3Sender::telemetrie_ausgaenge`, then
`Ausgang::inhalt`) **for every subscriber**, on **every** P2 frame. A local
unprivileged peer controls all three multipliers: frames per second (up to
`RATE_PRO_SEKUNDE` = 4000 per connection), payload size (up to
`MAX_FRAME_BYTES` = 262144) and the number of subscriber links in the session
(up to `SESSION_CLIENT_CAP` = 64). The lock order established here,
`Coordinator::stand` → `V3Sender::telemetrie_ausgaenge` → `Ausgang::inhalt`,
also permanently forbids the reverse edge; the nearby teardown code at
`broker/src/transport/server_v3.rs:2876-2881` only avoids creating it because
the `control_ausgaenge` guard happens to be a statement-scoped temporary that
drops one line before `senke.control_schliesst(&link_id)`.

## Code

```rust
// broker/src/coordinator.rs:3094
let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
{
    let mut stand = self.stand.lock().expect("Coordinator vergiftet");   // <- held
    // ... routing checks, loudness bookkeeping, stand.messframes.insert(..) ...

    if let Some(push) = push.as_ref() {
        let session = key.session();
        let mut ziele = stand
            .subscriptions
            .iter()
            .filter(/* ... */)
            .map(|(ziel_link_id, _)| ziel_link_id.clone())
            .collect::<Vec<_>>();
        ziele.sort();
        for ziel_link_id in ziele {
            // dyn-dispatch into the transport layer WHILE `stand` is held.
            let _ =
                push.messframe_schreiben(&ziel_link_id, &key.instance_id, &batch.payload);
        }
    }
}   // <- `stand` released only here, at line 3194
```

Contrast with the invariant honoured on the snapshot path:

```rust
// broker/src/coordinator.rs:2271
drop(_flush_guard);
let push = self.push.lock().unwrap_or_else(|e| e.into_inner()).clone();
for (link_id, ziel) in ziele {
    let geschrieben = self.push_ziel_noch_gueltig(&link_id, &ziel)   // takes & releases `stand`
        && push.as_ref().is_some_and(|push| push.snapshot_schreiben(&link_id, &payload));
    ...
}
```

## Data flow

- **Source:** P2 telemetry frames from a local unprivileged peer on an
  authenticated v3 telemetry connection; the peer chooses frame rate, payload
  size and how many control links it subscribes to the same session.
- **Sink:** `push.messframe_schreiben(...)` at `broker/src/coordinator.rs:3191`,
  executed with the `MutexGuard` for `Coordinator::stand` (acquired at `:3096`)
  still alive.
- **Validation:** none — the loop has no bound on iterations and no check that
  the sink is safe to call under the lock; the `Senke` doc comment demanding
  short, I/O-free callbacks (`broker/src/transport/server_v3.rs:235-240`) is not
  enforced in this direction.

## Reachability trace

attacker P2 frame → `verbindung_bedienen` read loop
(`broker/src/transport/server_v3.rs:2755`) → `eingang.einreihen` →
`eqcop-v3-ingress-rest` consumer thread
(`broker/src/transport/server_v3.rs:2686`) → `senke.p2_mit_minor(...)` →
`Senke::p2` default (`broker/src/transport/server_v3.rs:258`) →
`Coordinator::p2` (`broker/src/coordinator.rs:3079`) → `stand` locked at `:3096`
→ `push.messframe_schreiben` at `:3191`.

## Impact

- Latent global deadlock: any `SessionPush` implementation that exercises the
  reentrant-observation permission the coordinator documents wedges the broker's
  single global mutex forever. `std::sync::Mutex` does not detect this — the
  thread never returns, and all control, heartbeat, command and liveness
  processing stops behind it.
- Present-day contention amplification: the broker's one global lock is held
  across `subscribers x (allocate + CRC + two mutexes)` of transport work per
  attacker-supplied telemetry frame, with all multipliers under local
  unprivileged control. This serialises the whole broker behind a
  peer-controlled workload, which is exactly what the `flush_session` comment
  was written to prevent.

## Mitigations checked

- `// SAFETY:` comments: none apply here (safe code).
- The documented invariant exists (`broker/src/coordinator.rs:2267-2271`) and is
  honoured at the other two push sites (`:1078-1082`, `:2272-2277`), but it is
  not enforced by any type, lint or `debug_assert!`.
- `Senkenruf`/`join_mit_frist` (`broker/src/transport/server_v3.rs:1330-1400`)
  bound *server → sink* calls, not this *coordinator → sink* call.
- `messframe_schreiben` is documented non-blocking, and `V3Sender`'s body is in
  fact non-blocking, so today the hold time is bounded — but that is a property
  of one implementation, not of the trait contract.
- No `[lints]` table, no `clippy::pedantic`, no MIRI/loom coverage in
  `broker/Cargo.toml`; nothing would catch a future reentrant sink.

## Recommendation

Collect the push targets under the lock, drop the guard, then push — the shape
`flush_session` already uses:

```rust
let ziele: Vec<String> = {
    let mut stand = self.stand.lock().expect("Coordinator vergiftet");
    /* ... existing bookkeeping and filtering ... */
    ziele
};                                  // stand released here
if let Some(push) = push.as_ref() {
    for ziel_link_id in ziele {
        let _ = push.messframe_schreiben(&ziel_link_id, &key.instance_id, &batch.payload);
    }
}
```

If the "target still valid at push time" guarantee matters, re-check it with the
existing `push_ziel_noch_gueltig`-style short lock inside the loop instead of
holding one long guard. Additionally, state the invariant on the `SessionPush`
trait itself ("no coordinator lock is held when this is called; the
implementation may call back into the `Coordinator`") so the contract is visible
at the definition site, not only in a comment inside one caller.
