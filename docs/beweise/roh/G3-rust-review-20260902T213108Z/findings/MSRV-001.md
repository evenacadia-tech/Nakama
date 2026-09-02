---
id: MSRV-001
bug_class: msrv-mismatch
title: No rust-version and no pinned toolchain, while the locked dependency graph already requires Rust 1.85
location: broker/Cargo.toml:4
function: (file-level)
confidence: High
worker: worker-18
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: broker/Cargo.toml declares edition 2021 with no rust-version, no rust-toolchain file and no .cargo/config.toml exist in the repo, and Cargo.lock does pin uuid 1.24.1 via the caret requirement, so the effective floor is set by a dependency rather than by a decision."
severity: LOW
attack_vector: Local
exploitability: Theoretical
severity_rationale: "Supply-chain/reproducibility hardening gap with no attacker data flow; it matters because tools/beweise.ps1 records rustc --version without gating on it, so a routine cargo update can move the floor silently."
---

## Description

`broker/Cargo.toml` declares `edition = "2021"` but **no `rust-version` field**
(the whole 43-line manifest was read: `[package]` has `name`, `version`,
`edition`, `description` only). Nothing else in the repository supplies a floor
either:

- no `rust-toolchain.toml` / `rust-toolchain` anywhere in the repository,
- no `.cargo/config.toml`,
- no `build.rs` in `broker/` (so no `rustc_version`-style compile-time gate),
- no `.github/workflows/` and no `cargo +<msrv> check` leg in the canonical
  proof runner `tools/beweise.ps1`. The runner records `rustc --version` into the
  proof manifest (line 828) but never compares it against a required minimum.

Meanwhile the checked-in `broker/Cargo.lock` already resolves to dependencies
that declare a floor far above anything the broker states. Read from the local
registry copies of the exact locked versions:

| Locked dependency | declared `rust-version` |
|---|---|
| `uuid 1.24.1` | **1.85.0** |
| `getrandom 0.4.3` | **1.85** |
| `serde_json 1.0.151` | 1.71 |
| `libc 0.2.189`, `once_cell 1.21.4` | 1.65 |
| `windows-sys 0.60.2` | 1.60 |
| `rusqlite 0.40.2`, `libsqlite3-sys 0.38.2`, `sha2 0.10.9`, `serde_json_canonicalizer 0.3.2` | (none declared) |

So the crate's *actual* minimum today is **Rust 1.85**, set accidentally by a
caret requirement (`uuid = { version = "1", features = ["v4"] }`) rather than by
a decision. The broker's own source only needs ~1.70 (`let ... else`,
`Option::is_some_and`, `std::sync::OnceLock` are the newest constructs found;
no `LazyLock`, no `c"…"` literals, no 2024-edition-only APIs).

Because `rust-version` is absent, Cargo has no MSRV to honour when it selects
dependency versions, so the floor can move again on any routine `cargo update`
without anyone noticing — the failure surfaces as a compile error deep inside a
third-party crate rather than as a clear "requires rustc 1.x" message.

## Code

```toml
# broker/Cargo.toml:1-9 — the complete [package]/[lib] section
[package]
name = "eqcop-broker"
version = "0.1.0"
edition = "2021"
# <- no `rust-version` here

description = "EQ-Copilot-Broker (Nakama) — eigenständiger Named-Pipe-Server, ..."

[lib]
name = "eqcop_broker"
path = "src/lib.rs"
```

```toml
# broker/Cargo.toml:27 — the caret requirement that silently set the real floor
uuid = { version = "1", features = ["v4"] }
# resolves (broker/Cargo.lock) to uuid 1.24.1, whose own manifest says:
#   rust-version = "1.85.0"
```

## Data flow

N/A — file-level finding (no attacker-controlled data flow).

## Reachability trace

N/A — file-level finding (no attacker-controlled data flow).

## Impact

The security properties of the shipped `eqcop-broker.exe` become a property of
whichever toolchain happened to be installed on the build machine, and neither
the manifest nor the proof run constrains or checks it. For this crate that
matters more than usual: the binary is the local trust boundary (named-pipe ACL,
client impersonation, logon-SID check, 89 `unsafe` blocks over Win32 and bundled
SQLite FFI), and standard-library security fixes are delivered by toolchain
version — e.g. the `std::fs::remove_dir_all` TOCTOU fix (CVE-2022-21658, Rust
1.58.1) and the Windows `std::process::Command` argument-escaping fixes
(CVE-2024-24576, Rust 1.77.2; CVE-2024-43402, Rust 1.81.0). Those two examples
are **illustrative, not this crate's current exposure**: I verified that
`Command::new` appears only in `broker/tests/*` and that every `remove_dir_all`
call in `src/` sits inside `#[cfg(test)] mod tests` (`src/bindung.rs:46`,
`src/aggregat.rs:176`). The defect is that no declared floor exists to keep it
that way, or to keep any future use of a version-gated std API safe.

Concrete, non-hypothetical consequences today:

1. A build on a toolchain older than 1.85 fails inside `uuid`/`getrandom` with a
   confusing error instead of a clear MSRV diagnostic — and the phase-gate proof
   run would report a build failure of unclear provenance.
2. Cargo cannot apply MSRV-aware version selection, so the floor drifts upward
   silently on `cargo update`, undermining the reproducibility boundary the
   manifest explicitly claims to maintain elsewhere (`rusqlite = "=0.40.2"` is
   pinned precisely because "die exakte Version … ist eine geprüfte
   Reproduzierbarkeitsgrenze der Matrix" — the toolchain gets no such treatment).

## Mitigations checked

- `rust-toolchain.toml` / `rust-toolchain`: absent (searched repo-wide,
  excluding `target/`).
- `[package] rust-version`: absent (manifest read in full, 43 lines).
- `build.rs` version gate: `broker/build.rs` does not exist.
- CI pin: no `.github/workflows/`; `tools/beweise.ps1` has no `cargo +<msrv>`
  leg and no minimum-version assertion — `rustc --version` is only *recorded*.
- `Cargo.lock` is committed, which pins the *dependency* versions and therefore
  keeps today's floor at 1.85 stable for as long as nobody runs `cargo update`.
  This is a partial mitigation for consequence (2) only; it does not declare or
  enforce a toolchain minimum.

## Recommendation

Declare the floor and enforce it:

```toml
[package]
name = "eqcop-broker"
version = "0.1.0"
edition = "2021"
rust-version = "1.85"   # set by uuid 1.24.1 / getrandom 0.4.3; source needs 1.70
```

Then pin the toolchain for reproducible proof runs by adding a
`rust-toolchain.toml` next to `broker/Cargo.toml`:

```toml
[toolchain]
channel = "1.85.0"
components = ["clippy", "rustfmt"]
```

and add a `cargo +1.85.0 check --manifest-path broker/Cargo.toml --all-targets`
leg to `tools/beweise.ps1` so a toolchain or dependency drift below the declared
floor fails the phase gate instead of passing silently. If staying on 1.85 is
not desired, pin `uuid` to the newest version compatible with the intended floor
instead of the open `version = "1"` requirement.
