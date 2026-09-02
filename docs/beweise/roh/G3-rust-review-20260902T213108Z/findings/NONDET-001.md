---
id: NONDET-001
bug_class: nondeterminism
title: P0 command target link chosen by HashMap iteration order when two links share one address
location: broker/src/coordinator.rs:2648
function: persistenz_p0
confidence: High
worker: worker-15
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: stand.links is a HashMap<String, LinkStand> (coordinator.rs:395), persistenz_p0 picks the P0 target with .iter().find() on address+session+!trennen only (:2648), and the displacement at :766 is skipped when the runtime nonce is identical, so two live links can share one address with trennen==false."
severity: MEDIUM
attack_vector: Local
exploitability: Difficult
severity_rationale: "A local process that holds a duplicate control link with an address it read from session_snapshot makes another instance's preview commands succeed or fail by hash order, and the append-only event log stops being reproducible — a targeted local DoS of the command path plus a durable-state integrity defect."
---

## Description

`persistenz_p0` resolves the target of a persistent P0 command (`preview_begin` /
`preview_renew` / `preview_end`) by scanning `stand.links` — a
`std::collections::HashMap<String, LinkStand>` — with `.iter().find(...)`. The
predicate matches on `link.adresse == ziel`, the session, and `!link.trennen`; it
does **not** match on the link identity. `HashMap` iteration order is randomized
per map instance (each `RandomState` is seeded independently), so when more than
one link satisfies the predicate, `find` returns an arbitrary one, and the choice
changes from run to run and from map to map inside the same process.

Two live links **can** carry the same `Adresse`. In
`control_hello_registrieren` the previous link of a known client is only
displaced when the runtime nonce differs (`broker/src/coordinator.rs:766`):

```rust
if alter_link != link_id && alt.current_nonce != adresse.runtime_nonce {
    link.verdraengt = true;
    link.trennen = true;
    ...
}
```

A second control connection that presents the *identical* address — same
`logon_sid`, `project_binding_id`, `session_epoch`, `instance_id` **and**
`runtime_nonce` — therefore leaves the old `LinkStand` in `stand.links` with
`trennen == false`, while `client.current_link` is repointed to the new link
(`stand.links.insert(...)` at `broker/src/coordinator.rs:834`, `ClientStand {
current_link: Some(link_id.to_owned()), .. }` at `:813`). The alias register does
not catch this either: `alias_besitzer` is `format!("{instance_id}:{runtime_nonce}")`,
so an identical-nonce duplicate registers as `Registrierung::BereitsEingetragen`,
not as a collision. Client caps do not fire either, because
`platz_schaffen_locked` is only entered for a *new* `ClientKey`
(`if !stand.clients.contains_key(&key)`, `:747`).

Same-runtime reconnects are an explicitly supported case (see the comment at
`broker/src/coordinator.rs:822`: "Auch ein Reconnect derselben Runtime muss den
laut E-M01 vorgeschriebenen ersten `heartbeat.runtime` erneut liefern"), so the
duplicate window opens whenever a plugin reconnects before the broker's reader
thread has observed the old pipe end (`control_schliesst` → `control_ende`), and
it stays open indefinitely for any local process that keeps such a duplicate
control connection alive.

With two matching links, the branch taken directly below the `find`
(`broker/src/coordinator.rs:2662`) flips on iteration order:

* `find` returns the current link → the command is authorised, an event is
  appended to the append-only `event_log` plus `outbox` and acked `angewandt`;
* `find` returns the stale twin → `client.current_link.as_deref() !=
  Some(ziel_link_id.as_str())` holds, the command is rejected with
  `unknown_target` and **no** event is persisted.

Identical client behaviour thus produces different durable state, and the
append-only log — the crate's replay/reconstruction source — is not reproducible.

## Code

```rust
// broker/src/coordinator.rs:2646
            if !sender_erlaubt {
                Err((base_revision, None, "abgelehnt", "unauthorized"))
            } else if let Some((ziel_link_id, ziel_link)) = stand.links.iter().find(|(_, link)| {
                link.adresse == ziel
                    && link.client_key.session() == sender_link.client_key.session()
                    && !link.trennen
            }) {
                // ...
                if client.current_link.as_deref() != Some(ziel_link_id.as_str())
                    || client.stale
                    || !self.dispatch_fuer_link_erlaubt_locked(&stand, ziel_link)
                {
                    Err((base_revision, None, "abgelehnt", "unknown_target"))
```

The success branch is what reaches durable storage:

```rust
// broker/src/coordinator.rs:2761
        event.snapshot_ziele = snapshot_ziele;
        match store.append(vec![event]) { ... }
```

## Data flow

- **Source:** P0 envelope on the v3 named pipe, fully attacker-controlled bytes;
  `kopf.ziel` is deserialized into `Adresse` at `broker/src/coordinator.rs:2576`.
  The second, duplicate control connection (the precondition) is also
  pipe-supplied: `HelloControl.adresse.runtime_nonce` is echoed straight back by
  the client, and every session member sees other members' full addresses —
  including `runtime_nonce` — in the `session_snapshot` payload built at
  `broker/src/coordinator.rs:2114`.
- **Sink:** `stand.links.iter().find(...)` at `broker/src/coordinator.rs:2648`;
  its result decides between `store.append(...)` (durable event + outbox row) and
  an `unknown_target` rejection.
- **Validation:** none for link uniqueness. The nonce-based displacement at
  `:766` is skipped for identical nonces; the alias register treats an identical
  `instance_id:runtime_nonce` owner as already-registered; no cap applies because
  the `ClientKey` already exists.

## Reachability trace

`CreateNamedPipeW` listener (`broker/src/transport/server_v3.rs:395`) →
control worker reads a v3 envelope → `Senke::p0_mit_minor`
(`broker/src/coordinator.rs:3057`) → `Coordinator::p0_json_mit_minor` →
`"preview_begin" | "preview_renew" | "preview_end"`
(`broker/src/coordinator.rs:2995`) → `persistenz_p0`
(`broker/src/coordinator.rs:2572`) → ambiguous
`stand.links.iter().find(...)` (`:2648`).

Precondition path: `Senke::control_verbunden`
(`broker/src/coordinator.rs:3014`) → `control_hello_registrieren` (`:716`) →
same-nonce branch at `:766` leaves the old link live.

## Impact

* Non-reproducible durable state: for the same sequence of client messages the
  broker either writes a `command` event (with its `command_id` idempotency key,
  `state_revision`, `state_hash` and outbox rows) or writes nothing. Replay,
  crash-recovery comparison and the cross-language contract proofs lose their
  "identical input → identical log" property.
* Denial of the main app's control path: while a duplicate link exists, roughly
  half of the leading main's `preview_*` commands to that probe fail with
  `unknown_target` even though the target is connected and healthy. A local
  unprivileged process that obtains one session address (a session member reads
  every member's `runtime_nonce` out of `session_snapshot`; a compromised or
  malicious plugin instance in the same project is such a member) can hold a
  duplicate control connection open and keep the failure permanent.
* Because the ambiguity is resolved by a randomized hash seed, the failure is
  neither reproducible in a proof run nor visible in the existing tests, which
  never construct two links with one address.

## Mitigations checked

- `dispatch_fuer_link_erlaubt_locked` (`:2486`) runs on the *selected* link only —
  it filters quarantined/conflicted links but cannot disambiguate two identical
  ones.
- The `client.current_link` comparison at `:2662` is a fail-closed guard for the
  wrong-link case; it converts the nondeterminism into a nondeterministic denial
  rather than a mis-delivery, but does not remove it.
- `conflict_guards` / `AliasRegister` quarantine only trigger on *different*
  owners claiming one wire `instance_id`; an identical `instance_id:runtime_nonce`
  duplicate is `BereitsEingetragen` (`broker/src/instance_alias.rs:147`).
- Session and global client caps are bypassed for this case because no new
  `ClientKey` is created (`broker/src/coordinator.rs:747`).
- Neighbouring code in the same file is deliberately determinism-hardened
  (`stales_opfer` tie-breaks on `effektive_adresse` at `:1436`,
  `fuehrung_neu_bewerten_locked` sorts and requires exactly one candidate at
  `:1868`, `snapshot_locked` sorts members at `:2163`), which shows the invariant
  is intended and simply missing here.
- No MIRI/sanitizer/fuzz coverage exercises a duplicate-address link set.

## Recommendation

Do not select the target link by scanning `links`. Resolve the client first
(`ClientKey::aus_adresse(&ziel)` → `stand.clients.get(&key)`) and then use
`client.current_link` as the only authoritative link id, looking it up with
`stand.links.get(...)`. If a scan must stay, make it total and deterministic:
collect all matches, reject when more than one matches (fail closed, like the
alias-collision quarantine), or order by an explicit tie-break key. Additionally,
close the stale twin in `control_hello_registrieren` by dropping the
`alt.current_nonce != adresse.runtime_nonce` condition at `:766` — any earlier
link of the same client that is not `link_id` should be marked `trennen` and
scheduled for closing, so that at most one live link per address exists.
