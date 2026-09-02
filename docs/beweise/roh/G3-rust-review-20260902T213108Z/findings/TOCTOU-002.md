---
id: TOCTOU-002
bug_class: toctou
title: Remote-volume refusal is evaluated on the nearest existing ancestor, not on the DB path that is later created and opened
location: broker/src/store.rs:1530
function: store_pfad_ist_remote
confidence: Medium
worker: worker-20
fp_verdict: TRUE_POSITIVE
fp_rationale: "Confirmed in source and stronger than a race: store_pfad_ist_remote climbs 'while !kandidat.exists()' to the nearest existing ancestor (store.rs:1529-1536) and runs GetVolumePathNameW/GetDriveTypeW on that ancestor, while store_vorbereiten calls create_dir_all for the real parent only afterwards at :1274 — so on first start the guard never inspects the object it is meant to refuse."
severity: LOW
attack_vector: Local
exploitability: Difficult
severity_rationale: "The remote-volume refusal decides about a different object than the one opened, and the verdict is computed once per process while kurze_leseconnection and projektionen_neu_bauen re-open by name — a real logic defect in a security gate, but confined to the broker's own user's profile so no boundary is crossed."
---

## Description

`store_pfad_ist_remote` is the guard that makes the broker refuse to place its
SQLite database and WAL on a network volume ("Remote-Volume: DB und WAL werden
dort nicht geoeffnet", store.rs:1253-1257). The guard is a pure name-based
check, and it does not check the path it is asked about:

```rust
let mut kandidat = pfad;
while !kandidat.exists() {
    kandidat = kandidat.parent().ok_or_else(...)?;
}
```

When the target does not exist yet — the normal case on first start, and the
case the surrounding code explicitly plans for, since `create_dir_all` for the
parent runs *after* the guard (store.rs:1275-1277) — the loop climbs to whatever
ancestor happens to exist and calls `GetVolumePathNameW` + `GetDriveTypeW` on
**that ancestor**. The verdict for `…\evenacadia\nakama-broker\nakama-broker.sqlite3`
is thus derived from `…\AppData\Local` (or wherever the climb stops), and the
directory that will actually hold the database is only brought into existence
afterwards, at store.rs:1276.

This is the check/use split in its strongest form: the check does not merely
race the use, it never inspects the object the use operates on. Two concrete
consequences:

- **No race needed (first start).** The guard passes on a fixed local ancestor
  while the leaf components below it are created afterwards. Any component below
  the checked ancestor that is (or is made) a mount point, junction, or
  directory symlink is outside the guard's evidence. In particular a
  pre-created `%LOCALAPPDATA%\evenacadia\nakama-broker` reparse point is *never*
  examined, because the climb stops at it or above it and the volume verdict is
  taken from the string path, not from the opened object.
- **Classic race (subsequent starts).** Even when the file exists, the verdict
  is computed at store.rs:1543 and consumed at store.rs:1279 through a fresh
  name resolution, with `create_dir_all` in between; the path can be repointed in
  the window.

The verdict is also computed exactly once per process. Every later re-open by
name — `kurze_leseconnection` (store.rs:1431-1441), used by
`session_state_lesen`, `command_event_lesen` and `outbox_lesen`, and
`projektionen_neu_bauen` (store.rs:1384) — inherits a decision that was made
about a different object at a different time and is never revalidated.

## Code

```rust
#[cfg(windows)]
pub fn store_pfad_ist_remote(pfad: &Path) -> Result<bool, StoreFehler> {
    use std::os::windows::ffi::OsStrExt;
    use windows_sys::Win32::Storage::FileSystem::{GetDriveTypeW, GetVolumePathNameW};
    use windows_sys::Win32::System::WindowsProgramming::DRIVE_REMOTE;

    let mut kandidat = pfad;
    while !kandidat.exists() {          // CHECK is silently retargeted to an ancestor
        kandidat = kandidat.parent().ok_or_else(|| {
            StoreFehler::Pfad(format!(
                "kein existierender Vorfahr fuer {}",
                pfad.display()
            ))
        })?;
    }
    let mut wide: Vec<u16> = kandidat.as_os_str().encode_wide().collect();
    wide.push(0);
    let mut volume = vec![0u16; 32768];
    let ok = unsafe { GetVolumePathNameW(wide.as_ptr(), volume.as_mut_ptr(), volume.len() as u32) };
    // ...
    Ok(unsafe { GetDriveTypeW(volume.as_ptr()) } == DRIVE_REMOTE)
}
```

Consumed at the top of `store_vorbereiten`, before the directory even exists:

```rust
let remote = match konfiguration.remote_volume_override {
    Some(v) => v,
    None => store_pfad_ist_remote(&konfiguration.db_pfad)?,   // CHECK
};
if remote {
    return Err(StoreFehler::Pfad("Remote-Volume: DB und WAL werden dort nicht geoeffnet".into()));
}
// ... schema gate ...
if let Some(eltern) = konfiguration.db_pfad.parent() {
    std::fs::create_dir_all(eltern)?;                          // target dir created AFTER the check
}
let mut conn = Connection::open_with_flags(                    // USE
    &konfiguration.db_pfad,
    OpenFlags::SQLITE_OPEN_READ_WRITE | OpenFlags::SQLITE_OPEN_CREATE | OpenFlags::SQLITE_OPEN_NO_MUTEX,
)?;
```

## Data flow

- **Source:** the directory chain under `%LOCALAPPDATA%\evenacadia\nakama-broker\`
  (`STORE_RELATIVPFAD`, store.rs:24). It is created by plain `create_dir_all`
  (store.rs:1276) with no explicit DACL, so it carries only the inherited
  `%LOCALAPPDATA%` permissions; a process in the broker's own logon session can
  pre-create or replace those components before the broker's first start.
- **Sink:** `Connection::open_with_flags(..., READ_WRITE|CREATE)` at
  store.rs:1279, which creates `nakama-broker.sqlite3`, `-wal` and `-shm` on
  whatever volume the resolved path lands on, plus every later re-open through
  `kurze_leseconnection`/`projektionen_neu_bauen`.
- **Validation:** `GetDriveTypeW(GetVolumePathNameW(nearest_existing_ancestor))`
  — a string-path volume lookup on a *different* path than the sink, performed
  before the sink's directory exists, with no handle carried forward and no
  reparse-point inspection (`GetFileInformationByHandleEx`,
  `FILE_ATTRIBUTE_REPARSE_POINT`, `FILE_FLAG_OPEN_REPARSE_POINT` appear nowhere
  in the crate).

## Reachability trace

`eqcop-broker.exe main()` (main.rs:50) -> `broker_starten` (lib.rs:874) ->
`StoreKonfiguration::standard()` (lib.rs:887) -> `StoreWriter::starten`
(lib.rs:888, store.rs:621) -> `store_vorbereiten` (store.rs:1246) ->
`store_pfad_ist_remote` (store.rs:1251, 1524) -> ancestor climb (store.rs:1530)
-> verdict consumed (store.rs:1253) -> `Connection::open_with_flags`
(store.rs:1279). Unconditional on every production start.

## Impact

The guard can be made to certify "local, safe to open" for a database that is
then created on, or redirected to, a location the guard never examined:

- **Integrity loss of the append-only event store.** SQLite WAL on a network
  volume has the broken locking semantics this guard exists to avoid; a
  successfully bypassed guard reintroduces exactly the corruption/data-loss mode
  the code documents, against the store that holds the event log, session state
  and outbox.
- **Disclosure and forgery of persisted control data.** If the redirected
  location is attacker-readable/-writable, the attacker reads every persisted P0
  command payload (`event_log.payload_jcs`), session state and outbox entry, and
  can write rows that the broker reads back at start via
  `konflikt_guards_lesen` (store.rs:1289) into `restaurierte_guards`, which the
  coordinator restores as live session-ownership state.
- **Availability.** The reverse direction — making the guard see a remote
  ancestor — degrades the store at every start
  (`StoreWriter::degradiert_ohne_pfad`/`degradiere`), so no persistence-bound
  command receives a valid acknowledgement.

## Mitigations checked

- `// SAFETY:` comments at store.rs:1541-1542 and 1552 concern only the UTF-16
  buffer sizes for the Win32 calls; they say nothing about *which* path is being
  measured, and the invariant they document is unrelated to this bug.
- Atomic/handle-bound alternative: **not used**. The result of the volume lookup
  is never tied to a file handle, and the DB is subsequently opened by name.
- Reparse-point / mount-point awareness: **absent** crate-wide (no
  `symlink_metadata`, no `FILE_FLAG_OPEN_REPARSE_POINT`, no
  `GetFileInformationByHandleEx`).
- Store-directory hardening: **absent**. No DACL is set on the store directory
  (`store.rs` contains no security-descriptor code), unlike the pipe endpoints,
  which do get an explicit descriptor. Cross-user attackers are still blocked by
  the inherited `%LOCALAPPDATA%` ACL; same-session processes are not.
- Test coverage: **the real function is never executed by the test suite.**
  Every test and the crash worker set `remote_volume_override`
  (`store_crash_matrix.rs:134, 783, 858, 889, 974, 1575, 2250`;
  `eqcop-store-crash-worker.rs:143, 201`), which short-circuits
  `store_pfad_ist_remote` at store.rs:1249-1251. The ancestor climb therefore has
  no test at all.
- MIRI / sanitizers: not run on this path.

## Recommendation

Decide the volume question about the object that will actually be used, not
about an ancestor of its name:

1. Create the store directory **first** (`create_dir_all`), then evaluate the
   volume of the *created* directory, so the check and the use refer to the same
   existing object; drop the `while !kandidat.exists()` climb entirely and treat
   "no existing ancestor" as an error rather than as a licence to check
   something else.
2. Better, bind the verdict to a handle: open (or create) the database once, then
   call `GetVolumeInformationByHandleW` / `GetFileInformationByHandleEx` on that
   handle to classify the volume and to reject reparse points
   (`FILE_ATTRIBUTE_REPARSE_POINT`), closing the handle and degrading if the
   verdict is "remote". A handle-derived verdict cannot be invalidated by a
   later path swap.
3. Add a test that exercises `store_pfad_ist_remote` itself (currently every
   caller bypasses it via `remote_volume_override`), including the
   target-does-not-exist case, so the ancestor substitution is visible in CI.
