---
id: TOCTOU-001
bug_class: toctou
title: Store schema-guard checks the DB path with a separate read-only open before the read-write CREATE open
location: broker/src/store.rs:1259
function: store_vorbereiten
confidence: Medium
worker: worker-20
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified at store.rs:1259-1288: exists() then a separate read-only open in bekanntes_schema_readonly (whose connection is dropped) then a third, independent READ_WRITE|CREATE open, with create_dir_all in between and no handle or identity carried across — and the exists() pre-check makes the schema-downgrade gate fail open when the path is absent."
severity: LOW
attack_vector: Local
exploitability: Difficult
severity_rationale: "A genuine check/use split in a declared integrity gate, but the store lives in the broker's own user's LocalAppData, so the only party that can win the race is the user the broker already runs as — no privilege boundary is crossed and the residual is gate-bypass hygiene."
---

## Description

`store_vorbereiten` decides whether the broker may take ownership of the SQLite
event store by *name*, in three separate filesystem operations on the same path:

1. `konfiguration.db_pfad.exists()` (store.rs:1259) — if this returns `false`,
   the entire integrity gate below is skipped.
2. `bekanntes_schema_readonly(&konfiguration.db_pfad)` (store.rs:1260) opens a
   **second, independent** read-only connection to re-resolve the path and read
   `MAX(major) FROM schema_migrations`. Its result gates two refusals: a
   newer-than-known schema major ("kein Downgrade", store.rs:1261-1265) and an
   unreadable database ("DB kann nicht sicher gelesen werden", store.rs:1267-1271).
3. `Connection::open_with_flags(&konfiguration.db_pfad, READ_WRITE|CREATE|NO_MUTEX)`
   (store.rs:1279-1284) re-resolves the very same path a **third** time, then
   `migration_1` runs DDL/DML against whatever object that third resolution
   produced.

Nothing carries identity between steps 2 and 3 — no handle is kept open, no
`fstat`-equivalent is performed on the connection that was actually validated,
and there is no `SQLITE_OPEN_EXCLUSIVE`/`create_new` atomic creation. Every step
is a fresh name lookup, so the object the broker finally migrates and writes to
need not be the object it validated. The window is not microscopic: it spans a
full SQLite open, a `sqlite_master` query, a `MAX(major)` query, a connection
teardown, and a `create_dir_all` (store.rs:1275-1277).

The same "validate once by name, use repeatedly by name" pattern continues after
startup: `kurze_leseconnection` (store.rs:1431-1441) re-opens `db_pfad` by path
on every `session_state_lesen`, `command_event_lesen` and `outbox_lesen` call,
and `projektionen_neu_bauen` (store.rs:1384) opens it read-write again — none of
them re-run the schema gate.

Additionally, the `exists()` pre-check makes the gate *fail open*: an attacker who
wins the race in the "file absent" direction (path absent at 1259, present at
1279) gets the read-write `CREATE` open with the schema check never having run at
all.

## Code

```rust
fn store_vorbereiten(
    konfiguration: &StoreKonfiguration,
) -> Result<(Connection, Vec<ConflictGuard>), StoreFehler> {
    // ... remote-volume gate (see TOCTOU-002) ...

    if konfiguration.db_pfad.exists() {                       // CHECK 1 (name lookup)
        match bekanntes_schema_readonly(&konfiguration.db_pfad) {   // CHECK 2 (2nd open)
            Ok(Some(major)) if major > STORE_SCHEMA_MAJOR => {
                return Err(StoreFehler::Degradiert(format!(
                    "Schema-Major {major} ist neuer als {STORE_SCHEMA_MAJOR}; kein Downgrade"
                )));
            }
            Ok(_) => {}
            Err(e) => {
                return Err(StoreFehler::Degradiert(format!(
                    "DB kann nicht sicher gelesen werden: {e}"
                )));
            }
        }
    }

    if let Some(eltern) = konfiguration.db_pfad.parent() {
        std::fs::create_dir_all(eltern)                        // widens the window
            .map_err(|e| StoreFehler::Pfad(format!("{}: {e}", eltern.display())))?;
    }
    let mut conn = Connection::open_with_flags(                // USE (3rd open, RW+CREATE)
        &konfiguration.db_pfad,
        OpenFlags::SQLITE_OPEN_READ_WRITE
            | OpenFlags::SQLITE_OPEN_CREATE
            | OpenFlags::SQLITE_OPEN_NO_MUTEX,
    )?;
    pragmas_setzen(&conn)?;
    migration_1(&mut conn, konfiguration.test_haken.as_ref())?;  // DDL on the unvalidated object
```

`bekanntes_schema_readonly` deliberately opens its own short-lived connection and
drops it (store.rs:1334-1365):

```rust
fn bekanntes_schema_readonly(pfad: &Path) -> Result<Option<i64>, StoreFehler> {
    let conn = Connection::open_with_flags(
        pfad,
        OpenFlags::SQLITE_OPEN_READ_ONLY | OpenFlags::SQLITE_OPEN_NO_MUTEX,
    )?;
    // ... queries ... (conn dropped on return — the validated handle is thrown away)
}
```

## Data flow

- **Source:** the filesystem entry at `konfiguration.db_pfad`, in production
  `%LOCALAPPDATA%\evenacadia\nakama-broker\nakama-broker.sqlite3`
  (`STORE_RELATIVPFAD`/`STORE_DATEINAME`, store.rs:24-25, assembled by
  `store_pfad_unter`, store.rs:1469-1471). A local process running in the
  broker's own logon session can create, replace, delete or reparse-point that
  entry; the directory is created by `create_dir_all` (store.rs:1276) and
  therefore only carries the inherited `%LOCALAPPDATA%` ACL — the broker sets no
  DACL of its own on the store directory (no `SetNamedSecurityInfo`/`Dacl` call
  exists anywhere in `store.rs`).
- **Sink:** `Connection::open_with_flags(..., READ_WRITE|CREATE)` at
  store.rs:1279 followed by `migration_1` at store.rs:1287, plus every later
  `kurze_leseconnection`/`projektionen_neu_bauen` re-open by path.
- **Validation:** name-based only (`exists()` + a discarded read-only
  connection). No handle reuse, no `SQLITE_OPEN_EXCLUSIVE`, no `create_new`, no
  post-open re-verification of the schema major on the read-write connection.

## Reachability trace

`eqcop-broker.exe main()` (main.rs:50) -> `eqcop_broker::broker_starten`
(lib.rs:874) -> `store::StoreKonfiguration::standard()` (lib.rs:887,
store.rs:386-388) -> `store::StoreWriter::starten` (lib.rs:888, store.rs:621) ->
`store_vorbereiten` (store.rs:624, 1246) -> checks at 1259/1260 -> use at 1279.
This runs unconditionally on every broker start, before any pipe listener is
published.

## Impact

An attacker with write access to the store directory (a same-session local
process — precisely the adversary this crate defends against elsewhere with pipe
DACLs, SID impersonation checks and `FIRST_PIPE_INSTANCE`) can defeat the two
integrity refusals that this gate exists to enforce:

- **Downgrade guard bypass.** Present a schema-major-1 database for the
  read-only probe, then swap in the real newer-major database before line 1279.
  `migration_1` then runs against a database the broker explicitly refused to
  touch, corrupting an event log that the design treats as append-only.
- **Fail-open creation.** Keep the path absent at line 1259 (gate skipped
  entirely), then place a symlink/reparse point there before line 1279. The
  `CREATE` open follows it, and the broker's append-only event log, outbox and
  projections are created/migrated at an attacker-chosen location, where the
  attacker can read every persisted control command and session state and can
  feed forged rows back into `konflikt_guards_lesen` (store.rs:1289) and
  `restaurierte_guards`, which the coordinator restores into live session
  ownership state.
- **Degradation-based DoS.** Winning the race in the other direction forces the
  "DB kann nicht sicher gelesen werden" branch, leaving the broker in
  `degradiert` state, where no persistence-bound command gets a valid
  acknowledgement.

## Mitigations checked

- `// SAFETY:` comments: not applicable (safe code).
- Atomic primitive: **absent**. No `OpenOptions::create_new`, no
  `SQLITE_OPEN_EXCLUSIVE`, no open-then-verify on the surviving handle — exactly
  the three atomic patterns that would make this an FP.
- Directory ACL: **none set by the broker**. `store.rs` contains no
  security-descriptor code; the store directory is produced by plain
  `create_dir_all` (store.rs:1276) and inherits `%LOCALAPPDATA%`. A
  *different-user* unprivileged process is therefore blocked by the inherited
  ACL; a *same-session* process (e.g. a hostile plug-in loaded into the DAW, the
  documented squatting adversary) is not.
- Test coverage: `store_crash_matrix.rs` exercises crash points inside
  `migration_1` and the writer thread, but no test targets the
  check-to-use window (no test manipulates `db_pfad` between line 1260 and
  line 1279).
- MIRI / sanitizers: not run on this path (no `rust-toolchain.toml`, no
  fuzz/MIRI harness beyond the transport bit-flip tests).
- `debug_assertions`: no assertion re-checks the schema major after the
  read-write open.
- Note on residual risk: a same-session attacker can already write the store file
  directly, so this is an integrity-guard bypass rather than a privilege
  boundary crossing. The symlink/reparse variant is the part that reaches
  *outside* the store directory.

## Recommendation

Bind the decision to a handle, not to a name:

1. Open the database **once**, read-write, with `SQLITE_OPEN_EXCLUSIVE` on
   creation (or create the file first with
   `OpenOptions::new().write(true).create_new(true)` and hand the existing file
   to SQLite), so creation cannot land on an attacker-planted reparse point.
2. Run the schema-major probe on **that same connection** (`PRAGMA` /
   `SELECT MAX(major) FROM schema_migrations` inside the transaction that
   `migration_1` later uses) and abort the transaction on a newer major, instead
   of probing through a second connection that is then discarded.
3. Drop the `db_pfad.exists()` pre-check; let the single open distinguish
   "created fresh" from "opened existing" via its own result, so the gate cannot
   fail open.
4. On Windows, open with `FILE_FLAG_OPEN_REPARSE_POINT`-equivalent semantics or
   verify with `GetFileInformationByHandleEx(FileAttributeTagInfo)` that the
   opened object is not a reparse point before migrating it.
