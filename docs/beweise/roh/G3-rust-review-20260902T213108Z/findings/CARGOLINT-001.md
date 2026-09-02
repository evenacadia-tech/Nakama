---
id: CARGOLINT-001
bug_class: cargo-lint-config
title: No [lints] table, no crate-level deny, and no clippy in the proof pipeline on a crate with 89 unsafe blocks
location: broker/Cargo.toml:1
function: (file-level)
confidence: High
worker: worker-18
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified on disk: broker/Cargo.toml has [package]/[lib]/[dependencies] only (no [lints], no [profile]), no clippy.toml or rust-toolchain anywhere, and rg counts 89 unsafe blocks against 60 // SAFETY: comments in non-generated src/ — the finding's numbers reproduce exactly."
severity: LOW
attack_vector: Local
exploitability: Theoretical
severity_rationale: "Latent hardening gap with no attacker data flow of its own; it is the missing detection control behind SAFETYDOC-001/002 and the two result-discarded findings, which is why it stays a real finding at LOW rather than an FP."
---

## Description

`eqcop-broker` is the local security boundary of Nakama: it owns the named-pipe
namespace, performs Win32 client impersonation plus a logon-SID check before any
business logic, and links against bundled SQLite through C FFI. That work is
carried by **89 `unsafe { }` blocks** in non-generated `broker/src/`
(`transport/server_v3.rs` 46, `server.rs` 28, `store.rs` 14, `persistenz.rs` 1),
against only **60 `// SAFETY:` comments** in the same files — roughly 29 unsafe
blocks carry no safety justification at all.

Despite that, the crate has **no lint configuration of any kind**:

- `broker/Cargo.toml` (43 lines, read in full) contains `[package]`, `[lib]` and
  `[dependencies]` only — there is **no `[lints]` table**.
- There is no workspace manifest (`broker/Cargo.toml` is the only non-vendored
  `Cargo.toml` in the repository), so no workspace `[lints]` can supply one.
- `broker/src/lib.rs` and `broker/src/main.rs` carry **no crate-level
  attributes at all** — the only `#![...]` in the whole crate are two
  `#![cfg(windows)]` lines in `tests/`. No `#![deny(...)]`, no
  `#![warn(clippy::...)]`.
- No `clippy.toml` and no `.cargo/config.toml` anywhere in the repository, so no
  `RUSTFLAGS = ["-Dwarnings"]` fallback either.
- The canonical proof runner `tools/beweise.ps1` never invokes clippy: every
  `Art = 'cargo'` entry is `test`/`build` (release build at line 774). There is
  no `.github/workflows/` directory. The runner *records* `rustc --version`
  (line 828) but gates nothing on it.

The specific security-relevant lints whose code condition holds here, and which
are consequently never evaluated:

| Lint | Default level | Concrete code condition in this crate |
|---|---|---|
| `clippy::undocumented_unsafe_blocks` | allow (restriction) | 89 `unsafe { }` blocks vs 60 `// SAFETY:` comments in non-generated `src/` |
| `clippy::missing_safety_doc` | warn — never escalated to `deny`, and clippy is never run, so even the warn is never emitted | crate contains `unsafe fn` / `unsafe extern "C" fn` items (`src/store.rs:1589`, `:1596`, `:1607`) |
| `clippy::cast_ptr_alignment` | allow (pedantic) | raw-pointer alignment casts in the SQLite VFS shim, e.g. `(datei as *mut u8).add(anhang_offset()).cast()` at `src/store.rs:1593` |

`clippy::cast_ptr_alignment` and `clippy::undocumented_unsafe_blocks` are not
hypothetical for this project: `docs/archiv/EQ-COPILOT-TECHNIK-UX-UEBERGABE.md:775`
records that both were once run by hand against `broker/Cargo.toml` with `-W`.
That check was never promoted into the manifest or the proof runner, so the
coverage was lost when the manual step stopped.

Per the finder's own gate, `unsafe_code` is deliberately **not** flagged: this
crate legitimately requires `unsafe`, so `deny(unsafe_code)` would be wrong here.

## Code

```toml
# broker/Cargo.toml — complete list of tables in the manifest (43 lines):
[package]
name = "eqcop-broker"
version = "0.1.0"
edition = "2021"
description = "EQ-Copilot-Broker (Nakama) — eigenständiger Named-Pipe-Server, ..."

[lib]
name = "eqcop_broker"
path = "src/lib.rs"

[dependencies]
# ... flatbuffers, serde, sha2, serde_json, serde_json_canonicalizer, uuid,
# rusqlite (=0.40.2, bundled), windows-sys 0.60 (Win32_Security, ...)
# <- no [lints] table, no [profile] section
```

```rust
// broker/src/store.rs:1589-1594 — an unsafe pointer-arithmetic + alignment cast
// that clippy::cast_ptr_alignment would flag, if clippy were ever run:
unsafe fn anhang(datei: *mut ffi::sqlite3_file) -> *mut DateiAnhang {
    // SAFETY: Der registrierte Wrapper vergroessert `szOsFile` exakt um
    // diesen ausgerichteten Anhang; SQLite reicht denselben Puffer an alle
    // I/O-Methoden weiter.
    unsafe { (datei as *mut u8).add(anhang_offset()).cast() }
}
```

## Data flow

N/A — file-level finding (no attacker-controlled data flow). The manifest is a
build-configuration defect: it removes the compiler-side backstop that would
otherwise flag new unjustified `unsafe { }` in the pipe-facing code paths.

## Reachability trace

N/A — file-level finding (no attacker-controlled data flow).

## Impact

Nothing in the build or the canonical proof run fails when a developer adds a new
`unsafe { }` block with no `// SAFETY:` justification, or an unaligned pointer
cast, to the code that parses attacker-controlled pipe envelopes
(`transport/server_v3.rs`, 46 unsafe blocks) or that performs impersonation and
SID validation (`server.rs`, 28 unsafe blocks). The existing gap of ~29
undocumented unsafe blocks is exactly what such a gate exists to prevent from
growing. Under the LOCAL_UNPRIVILEGED threat model, every one of those blocks
sits behind a pipe any local process can open, so the absent lint gate directly
weakens the review discipline protecting the crate's only trust boundary.

This is a defense-in-depth / hygiene defect: it does not by itself create a
memory-safety bug, it removes the automated detection of one.

## Mitigations checked

- Workspace `[lints]`: none — `broker/Cargo.toml` is the only non-vendored
  manifest in the repo; there is no workspace root manifest.
- Crate attributes: none. `#![deny(...)]` / `#![forbid(...)]` /
  `#![warn(clippy::...)]` do not appear in `broker/src/` at all.
- `clippy.toml`: not present anywhere in the repository.
- `.cargo/config.toml` / `RUSTFLAGS`: not present; no `-D warnings`.
- CI: no `.github/workflows/`. `tools/beweise.ps1` runs `cargo build --release`
  and `cargo test` only — no `cargo clippy` entry exists in the Beine list.
- Existing `#[allow(...)]` are narrow and justified (flatc-generated code in
  `src/generiert/`, one `#[allow(dead_code)]` at `transport/server_v3.rs:1498`,
  one `#[allow(clippy::too_many_arguments)]` at `:2251`) — they are not the
  problem; the absence of any escalation is.
- `#[cfg(test)]` scoping checked: the three `unsafe fn` items in `store.rs` are
  inside `#[cfg(test)] mod tests` (starts `src/store.rs:1559`), so
  `unsafe_op_in_unsafe_fn` is **not** claimed here — production code has no
  `unsafe fn`, and the test-only ones already use explicit inner `unsafe { }`
  blocks. Enabling it would be forward-looking hardening only.

## Recommendation

Add a `[lints]` table to `broker/Cargo.toml` escalating exactly the lints whose
code condition holds, and wire clippy into the canonical proof runner so the
table is actually evaluated:

```toml
[lints.clippy]
undocumented_unsafe_blocks = "deny"   # 89 unsafe blocks, ~29 without // SAFETY:
missing_safety_doc         = "deny"   # escalate from warn; crate has unsafe fn items
cast_ptr_alignment         = "deny"   # SQLite VFS pointer casts, src/store.rs:1593

[lints.rust]
unsafe_op_in_unsafe_fn = "deny"       # forward-looking: allow-by-default in edition 2021
# NOTE: do NOT add `unsafe_code = "deny"` — this crate legitimately needs unsafe.
```

Adopt it incrementally if the ~29 currently undocumented blocks cannot all be
annotated at once: start at `"warn"`, annotate, then flip to `"deny"` in the same
ticket that closes the SAFETYDOC findings. Add a `cargo clippy --manifest-path
broker/Cargo.toml --all-targets -- -D warnings` leg to `tools/beweise.ps1` so a
regression fails the proof run instead of passing silently. See
[Cargo `[lints]`](https://doc.rust-lang.org/cargo/reference/manifest.html#the-lints-section)
and the [Clippy lint index](https://doc.rust-lang.org/clippy/).

Separately (out of this pass's class, noted for the integer-overflow reviewer):
the manifest has **no `[profile.release]` section**, so the shipped binary is
built with `overflow-checks = false` and `debug-assertions = false`.
