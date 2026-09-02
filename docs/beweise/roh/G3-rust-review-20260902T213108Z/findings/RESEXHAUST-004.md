---
id: RESEXHAUST-004
bug_class: resource-exhaustion
title: Attacker-triggered alias collisions append unbounded rows to the persistent conflict_guards table (disk exhaustion, reloaded into RAM at startup)
location: src/coordinator.rs:799
function: control_hello_registrieren
confidence: High
worker: worker-7
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: the collision branch at coordinator.rs:787 queues a ConflictGuard whose primary key is built entirely from hello fields (:3206), store.rs:1181 is INSERT OR IGNORE which dedups only identical pairs, the sole DELETE (:1200) is reachable only from konflikt_guard_aufloesen which no disconnect, eviction or compaction path calls, and store.rs:1370 reloads the whole table at startup."
severity: MEDIUM
attack_vector: Local
exploitability: Reliable
severity_rationale: "Local DoS with persistence: unbounded attacker-keyed rows grow the SQLite file until the user-writable volume is full, which denies service to every other component on that volume, survives restart, and reappears as startup RAM because the table is fully reloaded."
---

## Description
When `registriere_wire_zuordnung` reports
`Registrierung::KollisionBeideQuarantaenisiert`, the coordinator does two
things with attacker-chosen strings: it inserts them into the in-memory
`conflict_guards: HashMap<String, HashSet<String>>` and it queues a
`ConflictGuard` for **persistence into SQLite**.

```rust
guards_zu_persistieren.push(ConflictGuard { effective_address: effective.clone(), ... });
```

`guards_persistieren` (`src/coordinator.rs:2298`) then writes each one with
`INSERT OR IGNORE INTO conflict_guards(effective_address, derived_id,
created_utc_ms)` (`src/store.rs:1181`). Both key columns are attacker-supplied:

- `effective_address = format!("{}|{}|{}|{}", logon_sid, project_binding_id,
  session_epoch, instance_id)` (`src/coordinator.rs:3203`) — `logon_sid` is any
  1..184-character string from the hello, the other three are attacker-chosen
  hex32.
- `derived_id = format!("{}:{}", instance_id, runtime_nonce)`
  (`src/coordinator.rs:741`).

`INSERT OR IGNORE` deduplicates only *identical* pairs; the attacker simply
varies `logon_sid`/`project_binding_id`/`session_epoch` and every collision
writes a brand-new row. The only deletion path is
`konflikt_guard_aufloesen` (`src/coordinator.rs:2310` →
`DELETE FROM conflict_guards WHERE effective_address=?1 AND derived_id=?2`,
`src/store.rs:1200`), an explicit resolution operation that no automatic
disconnect, eviction, compaction or idle path ever calls. There is no row cap,
no retention window, and `kompaktieren` covers the event log, not this table.

At startup the whole table is read back into memory
(`SELECT effective_address, derived_id, created_utc_ms FROM conflict_guards`,
`src/store.rs:1370`) and loaded into `stand.conflict_guards`
(`src/coordinator.rs:605`), so unbounded disk growth becomes unbounded startup
RAM growth as well.

## Code
```rust
// src/coordinator.rs:787 — reached on every collision the attacker provokes
if registrierung == Registrierung::KollisionBeideQuarantaenisiert {
    let mut ids = vec![alias_besitzer.clone()];
    if let Some(alt) = &geerbt {
        ids.push(format!("{}:{}", alt.adresse.instance_id, alt.current_nonce));
    }
    for derived_id in ids {
        if stand
            .conflict_guards
            .entry(effective.clone())      // unbounded HashMap, attacker-keyed
            .or_default()
            .insert(derived_id.clone())
        {
            guards_zu_persistieren.push(ConflictGuard {
                effective_address: effective.clone(),
                // ... persisted below
```

```rust
// src/store.rs:1181 — one permanent row per distinct (address, derived_id)
"INSERT OR IGNORE INTO conflict_guards(effective_address,derived_id,created_utc_ms)\
 ...
```

```rust
// src/coordinator.rs:3202 — the primary key is built entirely from hello fields
fn effektive_adresse(adresse: &Adresse) -> String {
    format!(
        "{}|{}|{}|{}",
        adresse.logon_sid, adresse.project_binding_id, adresse.session_epoch, adresse.instance_id
    )
}
```

## Data flow
- **Source:** `hello_control.adresse` fields on the v3 control pipe
  (`src/transport/bootstrap.rs:203`), shape-validated only.
- **Sink:** `conflict_guards` SQLite table via
  `StoreHandle::konflikt_guard_persistieren` (`src/store.rs:453`) and the
  in-memory `stand.conflict_guards` map (`src/coordinator.rs:415`).
- **Validation:** `logon_sid` length 1..184 characters (any content), four
  hex32 fields. No cap on distinct address spaces, no row cap, no retention
  policy, no cross-check of `logon_sid` against the impersonated caller SID.

## Reachability trace
Local process opens the v3 pipe → `verbindung_bedienen`
(`src/transport/server_v3.rs:2252`) → `client_sid_authentisieren` passes for any
process of the same user → `bootstrap_lesen` → `Bootstrap::V3Control`
→ `senkenruf.rufen_mit_ergebnis(|s| s.control_verbunden(&link, &hello))`
(`src/transport/server_v3.rs:2442`) → `Coordinator::control_verbunden`
→ `Coordinator::control_hello_registrieren` (`src/coordinator.rs:716`)
→ collision branch at `src/coordinator.rs:787` → `guards_persistieren`
(`src/coordinator.rs:2298`) → `INSERT OR IGNORE INTO conflict_guards`.

Two connect attempts with the same `(logon_sid, project_binding_id,
session_epoch, instance_id)` but different `runtime_nonce` are enough to force
the collision; incrementing any address-space field yields a fresh, never-
deduplicated row.

## Impact
A local unprivileged process can grow the broker's SQLite database without
bound — roughly 300-800 bytes of row plus index per provoked collision, at a
rate limited only by how fast it can complete bootstrap handshakes. Unlike the
in-memory leak this survives a restart: the file keeps growing until the volume
is full, at which point every store write degrades
(`degradiere` → `StoreFehler::Degradiert`) and the broker's persistence path
fails closed for legitimate clients. Because the whole table is re-read into
`stand.conflict_guards` at startup, a large table also makes startup slow and
memory-hungry, and `dispatch_fuer_link_erlaubt_locked` consults that map on
every routing decision.

Filling a user-writable volume is also a classic lever for the local
unprivileged threat model: it denies service to every other component that
needs to write on that volume, not just to the broker.

## Mitigations checked
- `INSERT OR IGNORE`: deduplicates identical pairs only; the attacker never
  repeats a pair.
- `konflikt_guard_aufloesen` (`src/coordinator.rs:2310`) is the only delete
  path and is never invoked from disconnect, eviction, idle-shutdown or
  compaction.
- `kompaktieren` / WAL checkpointing (`src/store.rs:1224`, `:1229`) manage the
  event log and WAL size, not `conflict_guards`.
- `GLOBAL_CLIENT_CAP = 128` bounds live clients, not persisted guard rows;
  `client_eviktieren_locked` does not delete guards.
- Store single-writer discipline and the append-only trigger on `event_log`
  protect integrity, not growth.
- No disk-quota check, no row-count ceiling, no `created_utc_ms`-based retention
  despite that column existing.

## Recommendation
1. Reject a `hello_control` whose `adresse.logon_sid` does not match the SID
   obtained by `client_sid_authentisieren` for the pipe peer; this collapses the
   attacker's address-space cardinality to one value.
2. Cap the `conflict_guards` table: enforce a maximum row count (and/or a
   maximum per `effective_address`) at insert time, and prune the oldest rows by
   `created_utc_ms` — the column is already there.
3. Rate-limit collision-driven persistence per link/per address space so a
   connect loop cannot translate one-to-one into durable writes, and surface an
   overflow counter so the pruning is observable rather than silent.
