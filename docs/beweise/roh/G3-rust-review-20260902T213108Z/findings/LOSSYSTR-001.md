---
id: LOSSYSTR-001
bug_class: lossy-str-conversion
title: Store path built with String::from_utf16_lossy silently rewrites the LocalAppData path it opens
location: broker/src/store.rs:1504
function: standard_store_pfad
confidence: Medium
worker: worker-12
fp_verdict: TRUE_POSITIVE
fp_rationale: "Code claim verified at store.rs:1504 (String::from_utf16_lossy on the raw SHGetKnownFolderPath buffer instead of OsString::from_wide, which the same file uses in the outbound direction at :1538); the security framing is weaker than claimed because LocalAppData belongs to the broker's own user."
severity: LOW
attack_vector: Local
exploitability: Theoretical
severity_rationale: "Genuine lossy OS-string conversion on the store path, but the postulated attacker is the same user whose profile the broker already writes to, so no privilege boundary is crossed; the residual is path confusion on an ill-formed-UTF-16 profile name."
---

## Description

`standard_store_pfad()` resolves `FOLDERID_LocalAppData` with
`SHGetKnownFolderPath`, which hands back a raw null-terminated UTF-16 buffer,
and then converts it with `String::from_utf16_lossy(...)` before wrapping it in
a `PathBuf`. `from_utf16_lossy` does not fail on ill-formed UTF-16 — it replaces
every unpaired surrogate with U+FFFD (`REPLACEMENT CHARACTER`). The resulting
`PathBuf` is therefore not guaranteed to be the path Windows returned; it is a
*different* path that merely looks similar.

Windows paths are sequences of UTF-16 code units, not well-formed Unicode: the
filesystem and the Win32 API accept unpaired surrogates in file and directory
names. That is exactly why Rust models Windows OS strings as WTF-8 and provides
the lossless conversion `std::os::windows::ffi::OsStringExt::from_wide`, which
round-trips any code-unit sequence. The crate already knows and uses that API
family in the opposite direction two functions below
(`store_pfad_ist_remote`, `kandidat.as_os_str().encode_wide()` at
`broker/src/store.rs:1538`) and in `broker/src/persistenz.rs:85` — only this
inbound conversion takes the lossy shortcut.

The value is not a log line: it becomes the *entire* base of the broker's
persistence location, `LocalAppData/evenacadia/nakama-broker/nakama-broker.sqlite3`
(`store_pfad_unter`, `broker/src/store.rs:1469`), which the store then creates
directories for and opens read-write, together with its `-wal` and `-shm`
sidecars.

Under a `LOCAL_UNPRIVILEGED` threat model the returned string is not trusted
input: the LocalAppData known folder is per-user redirectable (the `Local
AppData` value under the current user's `User Shell Folders` key /
`SHSetKnownFolderPath`), so an unprivileged user can make this API return a
string of their choosing, and independently of redirection any profile directory
name may legitimately contain an unpaired surrogate. In either case the broker
does not open the directory Windows named; it opens the U+FFFD-substituted
sibling — a path an unprivileged attacker can pre-create, populate with a
prepared `nakama-broker.sqlite3`, or point elsewhere via a directory junction,
while the operator inspecting the "real" LocalAppData path sees nothing.

## Code

```rust
// broker/src/store.rs:1493
let root = unsafe {
    let mut len = 0usize;
    while *roh.add(len) != 0 {
        len += 1;
    }
    let pfad = PathBuf::from(String::from_utf16_lossy(std::slice::from_raw_parts(
        roh, len,
    )));                                   // <-- unpaired surrogates -> U+FFFD
    CoTaskMemFree(roh.cast());
    pfad
};
Ok(store_pfad_unter(&root))

// broker/src/store.rs:1469 — what the value becomes
pub fn store_pfad_unter(local_app_data: &Path) -> PathBuf {
    local_app_data.join(STORE_RELATIVPFAD).join(STORE_DATEINAME)
}
```

## Data flow

- **Source:** `SHGetKnownFolderPath(FOLDERID_LocalAppData, ...)` — a per-user
  redirectable, unprivileged-writable configuration value returned as raw UTF-16
  code units.
- **Sink:** `String::from_utf16_lossy` at `broker/src/store.rs:1504`, whose
  output becomes the `PathBuf` that `StoreKonfiguration::standard()` passes to
  `StoreWriter::starten` for `create_dir_all` plus the SQLite open at
  `broker/src/store.rs:1277` and following.
- **Validation:** none. The conversion cannot fail, so no error path exists; the
  crate never checks that the reconstructed path equals the one Windows
  returned, and never falls back to `OsString::from_wide`.

## Reachability trace

`BrokerLauf` start → `store::StoreKonfiguration::standard()`
(`broker/src/lib.rs:887`) → `Self::fuer_pfad(standard_store_pfad()?)` →
`standard_store_pfad` (`broker/src/store.rs:1474`) → lossy conversion at `:1504`
→ `store_pfad_unter` → `StoreWriter::starten` opens/creates the database, WAL and
SHM files under that path.

## Impact

Path confusion on the broker's only persistent security artifact. When the
LocalAppData string contains any ill-formed UTF-16, the broker silently creates
and opens its append-only event store, projections and outbox at a path that is
not the one the OS designated — one that an unprivileged local user can create
in advance and fully control. Consequences range from an availability failure
attributed to the wrong directory (store degrades, operator inspects an empty
"correct" path) to the broker attaching to an attacker-prepared SQLite database
whose contents it then treats as its own event history. The failure is
completely silent because the lossy conversion has no error path.

## Mitigations checked

- No fallible alternative is attempted: there is no `from_utf16` /
  `OsString::from_wide` path and no post-conversion comparison.
- The surrounding `// SAFETY:` comments cover only the pointer/lifetime aspects
  of the FFI buffer, not the encoding conversion; the invariant they name
  ("null-terminated UTF-16 string") does not imply *well-formed* UTF-16.
- `store_pfad_ist_remote` (same file, `:1525`) and `persistenz.rs:85` already use
  `encode_wide`/`OsStrExt`, so the correct API is in the crate's dependency and
  idiom set.
- The store's degradation path (`degradiert_ohne_pfad`) only catches an outright
  `Err` from `standard_store_pfad`; a lossily-rewritten path returns `Ok` and is
  therefore never degraded.
- No MIRI run, no `[lints]` table, no clippy gate on this file; there is no test
  covering non-UTF-16-well-formed profile paths.

## Recommendation

Use the lossless Windows conversion and keep the value as an `OsString`/`PathBuf`
throughout:

```rust
use std::os::windows::ffi::OsStringExt;

let root = unsafe {
    let mut len = 0usize;
    while *roh.add(len) != 0 {
        len += 1;
    }
    let pfad = PathBuf::from(std::ffi::OsString::from_wide(
        std::slice::from_raw_parts(roh, len),
    ));
    CoTaskMemFree(roh.cast());
    pfad
};
```

If a strictly well-formed path is a requirement, use `String::from_utf16` and
return `StoreFehler::Pfad(...)` on `Err` so the store degrades visibly instead of
silently relocating.
