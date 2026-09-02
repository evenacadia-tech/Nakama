---
id: RESDISC-001
bug_class: result-discarded
title: Automatic WAL checkpoint errors are discarded, so a stuck store never degrades and never reports
location: broker/src/store.rs:784
function: writer_lauf
confidence: High
worker: worker-12
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified at store.rs:784 and :894: both automatic checkpoints are let _ =, and the idle path sets idle_checkpoint_gelaufen = true even after a failure, while every sibling operation in the same loop routes its error into degradiere(); the finding's own attacker path is weaker than claimed because a reader-blocked PASSIVE checkpoint returns Ok(busy), not Err."
severity: LOW
attack_vector: Local
exploitability: Theoretical
severity_rationale: "Real loss of the store's only degradation signal plus a suppressed retry, but an Err needs SQLITE_IOERR/FULL/READONLY rather than anything a pipe client controls, so the attacker-facing impact only arrives indirectly via RESEXHAUST-004's disk growth."
---

## Description

`writer_lauf` is the single-writer thread of the broker's SQLite event store. It
triggers a `PRAGMA wal_checkpoint(PASSIVE)` from two places — the idle path
(line 784) and the WAL-size threshold path (line 894) — and discards the
`Result<CheckpointErgebnis, StoreFehler>` at both with `let _ =`.

Every other fallible store operation in the same loop routes its error into
`degradiere(&sicht, ...)`, which is the store's only mechanism for making a
persistence failure visible (`StoreWriter::ist_degradiert`, `StoreSicht.grund`,
and the `broker_store_sicht()` health surface at `broker/src/lib.rs:1074`):

- `kompaktieren(...)` → `if ergebnis.is_err() { degradiere(...) }`
- `konflikt_guard_schreiben(...)` → `degradiere(...)`
- `konflikt_guard_loeschen(...)` → `degradiere(...)`
- explicit `WriterBefehl::Checkpoint` → result returned to the caller

Only the two automatic checkpoints swallow the error, with no comment accepting
it. `checkpoint()` (`broker/src/store.rs:1207`) maps a real `rusqlite::Error` —
`SQLITE_BUSY` on the write lock, `SQLITE_IOERR`, `SQLITE_FULL`, `SQLITE_READONLY`
— into `StoreFehler`; a merely blocked passive checkpoint is *not* an error
(it comes back `Ok` with `busy = 1`), so an `Err` here always means the WAL
truly cannot be checkpointed.

Consequence: while `wal_groesse()` keeps growing past `WAL_SCHWELLE_BYTES`, the
loop retries, fails, discards, and reports the store as healthy. The
`idle_checkpoint_gelaufen` flag is even set to `true` after the discarded call
at line 784, so the idle path marks a *failed* checkpoint as done and will not
retry until the next commit resets the flag.

## Code

```rust
// broker/src/store.rs:781 — idle path
if checkpoint_ausloesen(
    wal_groesse(&wal_pfad),
    letztes_event.elapsed(),
    capture_aktiv.load(Ordering::SeqCst),
) && !idle_checkpoint_gelaufen
{
    let _ = checkpoint(&conn, false);   // <-- Err(StoreFehler) dropped
    idle_checkpoint_gelaufen = true;    // <-- marked done even on failure
}

// broker/src/store.rs:893 — WAL threshold path after a commit
if wal_groesse(&wal_pfad) >= WAL_SCHWELLE_BYTES {
    let _ = checkpoint(&conn, false);   // <-- Err(StoreFehler) dropped
}

// contrast, same loop, ~line 918:
let ergebnis = kompaktieren(&mut conn, &ziel, gedeckt_bis_event_ord, test_haken.as_ref());
if ergebnis.is_err() {
    degradiere(&sicht, ergebnis.as_ref().unwrap_err().to_string());
}
```

## Data flow

- **Source:** the on-disk SQLite database and its WAL sidecar, a local resource.
  A local unprivileged process that can open the broker's database file
  read-only and hold a long-lived read transaction (or simply fill the volume)
  keeps `wal_checkpoint` failing indefinitely.
- **Sink:** `checkpoint(&conn, false)` at `broker/src/store.rs:784` and `:894`;
  the returned `Result` is dropped by `let _ =`.
- **Validation:** none. The error never reaches `degradiere`, never increments a
  counter, is never logged, and is not visible through `StoreSicht`.

## Reachability trace

`BrokerLauf` start (`broker/src/lib.rs:887`) → `StoreWriter::starten` → writer
thread `writer_lauf` (`broker/src/store.rs:757`) → idle branch (`:784`) or
post-commit branch (`:894`) → `checkpoint()` → `Err` discarded. No caller,
health surface or test observes the failure.

## Impact

Silent loss of the store's integrity signal. The broker's own health view keeps
reporting a healthy, non-degraded store while the WAL cannot be checkpointed —
so the append-only event log, the projections and the outbox that the
persistence design treats as authoritative sit in an ever-growing WAL that is
never folded back. Practical effects for a local unprivileged adversary:
unbounded disk growth in the broker's `LocalAppData` store directory (a disk
exhaustion vector that produces no operator-visible warning), and a much larger
crash-recovery window than the store's own
`recovery_testgrenze_bestanden`/`STORE_RECOVERY_TEST_MAX_MS` budget assumes.
Because the idle path also sets `idle_checkpoint_gelaufen = true` after a failed
attempt, the store stops retrying until the next commit.

## Mitigations checked

- `degradiere(...)` exists and is used for every other fallible store operation
  in this loop — it is simply not called here.
- No comment at either site documents the discard as intentional.
- `unused_must_use` (rustc, on by default) does not fire on `let _ = expr`; the
  crate has no `[lints]` table and no `#![deny(unused_results)]`, so nothing
  flags this at build time.
- The store crash matrix (`broker/tests/store_crash_matrix.rs`) exercises commit
  and recovery paths, but a checkpoint that returns `Err` has no observable
  effect to assert on, so no test can catch this.

## Recommendation

Treat an automatic checkpoint failure exactly like every other store failure in
the same loop:

```rust
if let Err(fehler) = checkpoint(&conn, false) {
    degradiere(&sicht, format!("wal_checkpoint: {fehler}"));
} else {
    idle_checkpoint_gelaufen = true;
}
```

At minimum, count the failures in `StoreSicht` and do not set
`idle_checkpoint_gelaufen` unless the checkpoint actually succeeded, so the idle
path keeps retrying.
