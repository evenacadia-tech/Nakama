---
id: DROPSKIP-001
bug_class: destructor-skip
title: Ordered shutdown never drops StoreWriter, so the SQLite writer thread is killed by process::exit without shutdown or sqlite3_close
location: src/lib.rs:997
function: broker_geordnet_stoppen
confidence: High
worker: worker-19
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: BrokerLauf.store is a bare StoreWriter field in a static OnceLock (lib.rs:847), broker_geordnet_stoppen takes and drops only _supervisor/_griff_v2/_griff_v3, and rg over lib.rs finds no store.stoppen() — so StoreWriter::drop and sqlite3_close never run."
severity: LOW
attack_vector: Local
exploitability: Theoretical
severity_rationale: "Real omission in the crate's own documented replacement for static drop, but the impact is a durability/shutdown-hygiene gap (undrained writer queue, un-truncated WAL with wal_autocheckpoint=0) on a path no attacker capability is needed for and that SQLite recovery largely repairs."
---

## Description

`BrokerLauf` is held in `static BROKER: OnceLock<Result<BrokerLauf, String>>`
(lib.rs:863). Rust never runs destructors for `static` values, so no field of
`BrokerLauf` is ever dropped implicitly. The codebase knows this — it is the
stated reason `broker_geordnet_stoppen()` exists:

> Statische `OnceLock`-Werte werden beim Rueckweg aus `main` nicht gedroppt;
> ohne diese Funktion bliebe die Reihenfolge allein der OS-Prozessbereinigung
> ueberlassen.

That function then explicitly `take()`s and drops `_supervisor`, `_griff_v2`
and `_griff_v3` — and stops there. The fourth resource-owning field,

```rust
#[cfg(windows)]
store: store::StoreWriter,     // src/lib.rs:847
```

is a bare field (not `Mutex<Option<_>>`), so it cannot be taken, and no code
anywhere calls `lauf.store.stoppen()`. `rg -n "store\.stoppen|\.store\b" src/lib.rs`
finds only `broker_store_sicht()` reading `lauf.store.handle().sicht()`.

`StoreWriter::drop` is therefore unreachable in the broker binary:

```rust
impl Drop for StoreWriter {          // src/store.rs:715
    fn drop(&mut self) { self.stoppen(); }
}
pub fn stoppen(&mut self) {          // src/store.rs:704
    if let Some(sender) = &self.handle.sender {
        let _ = sender.send(WriterBefehl::Shutdown);
    }
    if let Some(join) = self.join.take() { let _ = join.join(); }
    self.handle.sender = None;
}
```

`main` calls `broker_geordnet_stoppen()` and then either falls out of `main` or
executes `std::process::exit(1)` (main.rs:73). In both cases the process dies
while the `nakama-store-writer` thread is still running: it never receives
`WriterBefehl::Shutdown`, never reaches `WriterBefehl::Shutdown => break`
(store.rs:953), never drops its `rusqlite::Connection`, and therefore
`sqlite3_close` is never called on the event/outbox database.

## Code

```rust
// src/lib.rs:970-999 — the documented replacement for the missing static drop
pub fn broker_geordnet_stoppen() {
    let Some(Ok(lauf)) = BROKER.get() else { return; };
    if lauf.beendet.swap(true, Ordering::SeqCst) { return; }

    #[cfg(windows)]
    {
        let supervisor = lauf._supervisor.lock().unwrap_or_else(|e| e.into_inner()).take();
        drop(supervisor);
        let griff_v2 = lauf._griff_v2.lock().unwrap_or_else(|e| e.into_inner()).take();
        drop(griff_v2);
        let griff_v3 = lauf._griff_v3.lock().unwrap_or_else(|e| e.into_inner()).take();
        drop(griff_v3);
        // `lauf.store` (StoreWriter, src/lib.rs:847) is NEVER stopped here,
        // and being inside a `static OnceLock` it is never dropped either.
    }
}
```

```rust
// src/main.rs:71-74 — the skip site
            eqcop_broker::broker_geordnet_stoppen();
            if fataler_listenerfehler {
                std::process::exit(1);
            }
```

```rust
// src/store.rs:1291-1302 — why sqlite3_close matters here
fn pragmas_setzen(conn: &Connection) -> Result<(), StoreFehler> {
    conn.pragma_update(None, "foreign_keys", "ON")?;
    let modus: String = conn.query_row("PRAGMA journal_mode=WAL", [], |row| row.get(0))?;
    ...
    conn.pragma_update(None, "synchronous", "FULL")?;
    conn.busy_timeout(Duration::from_millis(BUSY_TIMEOUT_MS))?;
    conn.pragma_update(None, "wal_autocheckpoint", 0)?;   // auto-checkpoint DISABLED
    Ok(())
}
```

## Data flow

- **Source:** any local unprivileged process can drive the broker to exit —
  by staying away long enough for the 60 s idle self-exit
  (`BROKER_IDLE_ENDE_MS`, lib.rs:39, `broker_soll_idle_enden`), or by forcing
  the fatal-listener path that reaches `std::process::exit(1)` at main.rs:73.
- **Sink:** `std::process::exit(1)` (main.rs:73) and the implicit `exit(0)` on
  `main` return, both executed while `BrokerLauf.store: StoreWriter` (owning a
  live thread and an open `rusqlite::Connection`) is alive and undropped.
- **Validation:** `broker_geordnet_stoppen()` is the designated cleanup and it
  covers three of the four owning fields; the store is omitted, and nothing
  else compensates.

## Reachability trace

`main` -> `broker_starten` (constructs `BrokerLauf { store, ... }` into
`static BROKER`) -> supervision loop -> `broker_soll_idle_enden()` or
`broker_hat_fatalen_v3_listenerfehler()` -> `broker_geordnet_stoppen()`
(drops supervisor/v2/v3 only) -> `std::process::exit(1)` (main.rs:73) or `main`
return -> process terminates with `nakama-store-writer` still inside
`writer_lauf`, its `Connection` never closed.

## Impact

The append-only event store, its projections and the outbox are the broker's
durable record of session ownership and control traffic. Skipping
`StoreWriter::drop` means every broker exit is an abrupt kill of the storage
writer rather than an ordered shutdown:

- No `sqlite3_close`, so SQLite's close-time checkpoint never runs. Combined
  with `wal_autocheckpoint = 0` (store.rs:1301), the WAL is only ever trimmed by
  the size threshold (`WAL_SCHWELLE_BYTES`) and the one-shot idle checkpoint, so
  the `-wal`/`-shm` sidecars are consistently left dirty across restarts and
  grow unboundedly under a workload that stays below both heuristics — a local
  unprivileged process can shape its traffic to sit exactly there.
- The writer thread can be terminated mid-`append_gruppe` transaction or
  mid-`PRAGMA wal_checkpoint`, so the last in-flight batch is silently rolled
  back with no shutdown handshake and no record that it was lost.
- The shutdown ordering contract the function's own doc comment claims to
  establish ("ohne diese Funktion bliebe die Reihenfolge allein der
  OS-Prozessbereinigung ueberlassen") is not actually established for the one
  component that owns on-disk state: for the store, the order *is* left to OS
  process teardown.

This is the audit/telemetry-durability half of the broker losing its guaranteed
flush point on a path an unprivileged local process can trigger at will.

## Mitigations checked

- `synchronous = FULL` in WAL mode fsyncs each commit, so already-committed
  transactions survive the abrupt exit — this bounds the damage to the
  uncommitted tail and the missing close-checkpoint, it does not remove it.
- Append callers use blocking `recv()` (store.rs:435/459/473/487/496), so no
  caller is told "committed" for a batch lost to the exit; the loss is silent
  rather than incorrect-ack.
- `beendet: AtomicBool` makes `broker_geordnet_stoppen()` idempotent but does
  not add the store to it.
- No `// SAFETY:`-style comment documents the store as intentionally exempt;
  the doc comment enumerates supervisor, v2 and v3 and simply does not mention
  the store, so this reads as an omission, not a documented handoff.
- `tests/store_crash_matrix.rs` exercises crash recovery, i.e. it proves SQLite
  survives this; it does not assert that the ordered stop path shuts the writer
  down.

## Recommendation

Make the store part of the ordered stop. Change `BrokerLauf.store` to
`Mutex<Option<store::StoreWriter>>` (matching `_supervisor`/`_griff_v2`/`_griff_v3`)
and, in `broker_geordnet_stoppen()`, `take()` and `drop()` it **after**
`_griff_v3` — so all v2/v3 workers and the coordinator tick thread are joined
first and no further `AppendBatch` can be submitted, and only then the writer
receives `WriterBefehl::Shutdown`, is joined, and its `Connection` is closed
(giving SQLite its close-time checkpoint). Add an assertion or proof step to
`tools/beweise.ps1` that the `nakama-store-writer` thread has exited before the
process returns from `broker_geordnet_stoppen()`.
