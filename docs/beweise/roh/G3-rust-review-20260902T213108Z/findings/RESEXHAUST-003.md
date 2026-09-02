---
id: RESEXHAUST-003
bug_class: resource-exhaustion
title: Alias quarantine set grows without bound or eviction on attacker-chosen address spaces
location: src/instance_alias.rs:163
function: registriere_wire_zuordnung
confidence: High
worker: worker-7
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: instance_alias.rs has insert on quarantaene at :163/:164/:180 and contains/checks elsewhere but no remove anywhere in the crate, entferne refuses quarantined owners at :197, and rg confirms the hello's adresse.logon_sid is never compared against the SID that client_sid_authentisieren obtains, so the address space is attacker-chosen and unbounded."
severity: MEDIUM
attack_vector: Local
exploitability: Reliable
severity_rationale: "Local DoS by unbounded memory growth: each provoked collision permanently adds two BesitzerSchluessel with no eviction, cap or expiry, the attacker never repeats a key, and the growing set is consulted on every routing decision — ending in broker OOM with no operator signal."
---

## Description
`AliasRegister::registriere_wire_zuordnung` reacts to an alias collision by
inserting both owners into a permanent quarantine set:

```rust
stand.quarantaene.insert(erster_besitzer);
stand.quarantaene.insert(besitzer);
```

`quarantaene` is a `HashSet<BesitzerSchluessel>` where
`BesitzerSchluessel = (Sitzungsadressraum, String)` and
`Sitzungsadressraum = { logon_sid, project_binding_id, session_epoch }`
(`src/instance_alias.rs:71-95`). There is **no `remove` on `quarantaene`
anywhere in the crate** — the module documents this deliberately
("Quarantaene bleibt dagegen bis zur expliziten Neu-ID-Aufloesung bestehen; ein
Disconnect darf den sichtbaren Konflikt nicht waschen",
`src/instance_alias.rs:190`) — and `entferne` explicitly returns early for a
quarantined owner (`src/instance_alias.rs:197`). There is also no cap on the
set's size.

Every one of the four key components is a field of the attacker's
`hello_control` message. `bootstrap::adresse_pruefen` only checks *shape*:
`logon_sid` must be 1..=184 characters of anything, and
`project_binding_id`/`session_epoch`/`instance_id`/`runtime_nonce` must be
hex32. Crucially, the claimed `adresse.logon_sid` is **never compared against
the impersonated caller's real logon SID** — `client_sid_authentisieren`
(`src/transport/server_v3.rs:2070`) validates the pipe peer's token SID against
the broker's expected security descriptor, but nothing ties that result back to
the `logon_sid` string in the hello. So the attacker freely picks an unlimited
number of distinct address spaces, and each one supports a fresh collision.

Provoking a collision needs no privilege: connect once as
`(space S, instance_id I, runtime_nonce N1)`, then again as
`(space S, instance_id I, runtime_nonce N2)`. `alias_besitzer` is
`format!("{}:{}", adresse.instance_id, adresse.runtime_nonce)`
(`src/coordinator.rs:741`), so the two connections are different owners of the
same wire value — exactly `Registrierung::KollisionBeideQuarantaenisiert`.
Two permanent entries are added, then the attacker moves to space S+1.

## Code
```rust
// src/instance_alias.rs:150 — the collision branch
if let Some(erster) = stand.nach_wire.get(&wire_schluessel).cloned() {
    if erster == original {
        return Registrierung::BereitsEingetragen;
    }

    let erster_besitzer = Self::besitzer_schluessel(adressraum, &erster);
    stand.nach_wire.remove(&wire_schluessel);
    stand.nach_original.remove(&erster_besitzer);
    stand.nach_original.remove(&besitzer);
    stand.quarantaene.insert(erster_besitzer);   // never removed, never capped
    stand.quarantaene.insert(besitzer);          // never removed, never capped
    return Registrierung::KollisionBeideQuarantaenisiert;
}
```

```rust
// src/coordinator.rs:736 — every component comes straight from the hello
let alias_adressraum = Sitzungsadressraum::neu(
    &adresse.logon_sid,
    &adresse.project_binding_id,
    &adresse.session_epoch,
);
let alias_besitzer = format!("{}:{}", adresse.instance_id, adresse.runtime_nonce);
```

```rust
// src/transport/bootstrap.rs:149 — the only validation of those fields
fn adresse_pruefen(a: &Adresse) -> Result<(), String> {
    if a.logon_sid.is_empty() || a.logon_sid.chars().count() > 184 {
        return Err("logon_sid ausserhalb 1..184".into());
    }
    for (name, wert) in [ /* project_binding_id, session_epoch, instance_id, runtime_nonce */ ] {
        if !ist_hex32(wert) {
            return Err(format!("{name} ist kein hex32"));
        }
    }
    Ok(())
}
```

## Data flow
- **Source:** `adresse.{logon_sid, project_binding_id, session_epoch,
  instance_id, runtime_nonce}` in the `hello_control` bootstrap JSON, parsed by
  `bootstrap_lesen` (`src/transport/bootstrap.rs:203`).
- **Sink:** `stand.quarantaene.insert(...)` at `src/instance_alias.rs:163` and
  `:164` (and `:180` on the identity-rewrite path) — a `HashSet` with no cap
  and no removal path.
- **Validation:** length/hex32 shape only; no uniqueness cap, no cross-check of
  `logon_sid` against the impersonated caller SID, no size limit on the set,
  no time-based eviction.

## Reachability trace
Local process opens the v3 pipe → `verbindung_bedienen`
(`src/transport/server_v3.rs:2252`) → `client_sid_authentisieren` (passes for
any process of the same user) → `bootstrap_lesen` → `Bootstrap::V3Control`
→ `senkenruf.rufen_mit_ergebnis(|s| s.control_verbunden(&link, &hello))`
(`src/transport/server_v3.rs:2442`) → `Coordinator::control_verbunden`
→ `Coordinator::control_hello_registrieren` (`src/coordinator.rs:716`)
→ `self.alias_register.registriere_wire_zuordnung(...)`
(`src/coordinator.rs:782`).

Note this happens *before* any per-session or per-client cap matters:
`platz_schaffen_locked` bounds `stand.clients` to `GLOBAL_CLIENT_CAP = 128`
and evicts stale entries, but eviction never touches `quarantaene`.

## Impact
A local unprivileged process can permanently leak roughly 1 KiB of broker heap
per connect pair (two `BesitzerSchluessel` values, each holding four `String`s
totalling up to ~313 bytes of text plus `HashSet` overhead), with no upper
bound and no way to reclaim it short of restarting the broker. Sustained at a
modest connect rate this is a slow but unbounded memory leak that ends in
broker OOM. Because the address space is attacker-chosen, the usual "one
misbehaving probe" reasoning does not apply — the attacker never revisits the
same key, so nothing is ever a duplicate insert.

Secondary correctness impact: the quarantine set is consulted by
`dispatch_erlaubt`/`session_push_erlaubt`/`ist_quarantaenisiert`, so the
growing set also makes every routing decision progressively more expensive.

## Mitigations checked
- `MAX_KOPPLUNGEN = 64` and `MAX_VERBINDUNGEN = 96` bound *concurrent*
  connections, not the cumulative number of registrations.
- `GLOBAL_CLIENT_CAP = 128` / `SESSION_CLIENT_CAP = 64` with stale eviction
  bound `stand.clients`; `client_eviktieren_locked` (`src/coordinator.rs:1441`)
  removes `messframes`/`messfehler`/`lautheit` but not quarantine entries.
- `AliasRegister::entferne` exists as the documented counter-path for the
  *collision-free* case and explicitly refuses to act on quarantined owners
  (`src/instance_alias.rs:197`), so the connect/disconnect pair is not
  symmetric here.
- The logon-SID impersonation chain added by NAK-123 validates the *pipe
  peer's* token but does not constrain the `logon_sid` string carried in the
  hello, so it does not limit the address-space cardinality.

## Recommendation
Bound the quarantine and tie it to a real identity:

1. Reject a `hello_control` whose `adresse.logon_sid` does not equal the SID
   obtained from the impersonated pipe peer in `client_sid_authentisieren`.
   That alone collapses the address space to the caller's own SID.
2. Give `AliasStand::quarantaene` a hard cap with LRU/oldest-first eviction (or
   a per-`Sitzungsadressraum` entry cap), and record an overflow counter so the
   drop is visible instead of silent.
3. Add a time-based expiry keyed on last observation, so a quarantine entry for
   an address space with no live client is reclaimed after a documented
   interval; the "a disconnect must not wash the conflict" rule is preserved by
   making the expiry much longer than a reconnect cycle rather than infinite.
