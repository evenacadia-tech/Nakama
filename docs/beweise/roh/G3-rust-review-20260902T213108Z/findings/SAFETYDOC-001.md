---
id: SAFETYDOC-001
bug_class: safety-doc
title: Impersonation/SID gate `client_sid_authentisieren` carries 17 undocumented unsafe blocks, including a type-punned raw deref and an unaligned raw write
location: broker/src/transport/server_v3.rs:2115
function: client_sid_authentisieren
confidence: High
worker: worker-1
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified by rg over server_v3.rs:2070-2200: 18 unsafe occurrences and zero // SAFETY: comments in client_sid_authentisieren, while the rest of the file documents its unsafe blocks and the sibling TOKEN_USER deref in server.rs:167 carries the exact reasoning that is missing here; the invariants themselves currently hold."
severity: LOW
attack_vector: Local
exploitability: Theoretical
severity_rationale: "Currently-correct unsafe without a written proof: the alignment link (the unnamed const assert 88 lines earlier plus the Vec<u64> element type), the SID sub-authority bounds and the borrowed interior pointer's lifetime are all unstated in the crate's authentication gate, so the next edit can silently remove the proof."
---

## Description

`client_sid_authentisieren` is the broker's authentication gate: it is the
function that impersonates the connecting named-pipe client, reads the
caller's `TOKEN_USER`, and compares the caller SID against the SID the pipe
DACL was built from. Every connection — control line and telemetry line — runs
through it before any business logic, so its soundness is what stands between a
local unprivileged process and the broker's session state.

The whole function is written in `unsafe` and contains **17 `unsafe { … }`
blocks, none of which carries a `// SAFETY:` comment** (lines 2077, 2084,
2091, 2094, 2103, 2115, 2117, 2128, 2133, 2144, 2148, 2149, 2150, 2152, 2159,
2169, 2170, 2173). This is not a stylistic nit in this file: `server_v3.rs`
otherwise documents *every* `unsafe` block it contains (lines 340, 346, 362,
378, 392, 425, 462, 507, 515, 526, 532, 543, 549, 579, 630, 670, 677, 971,
1286, 1299, 1738, 1792, 3232, 3246, 3275, 3295, 3315). The one function with
the strongest safety obligations is the one exception.

Three of the undocumented blocks carry non-trivial, non-local preconditions
that a reader cannot verify without reconstructing the argument from scratch:

1. **Line 2115** reinterprets a `Vec<u64>` byte buffer as `TOKEN_USER` and
   dereferences it. Soundness depends on (a) `noetig >= size_of::<TOKEN_USER>()`
   — enforced at line 2097 — and (b) the buffer being aligned for `TOKEN_USER`
   — enforced only by the *unnamed* `const _: () = assert!(…)` at line 2027,
   88 lines earlier, plus the `Vec<u64>` element type chosen at line 2101.
   Neither link is stated at the site. The sibling implementation in
   `broker/src/server.rs:167` documents exactly this reasoning; here it is
   missing.
2. **Lines 2144–2150** do raw pointer arithmetic into a copied SID
   (`sid_bytes.add(1)`, `sid_bytes.add(8 + (anzahl - 1) * 4)`) followed by
   `ptr::read_unaligned` / `ptr::write_unaligned`. In-bounds-ness rests on the
   `laenge < 12` guard (2129), the SID header layout (`SubAuthorityCount` at
   byte offset 1, sub-authorities from byte 8), and the
   `8 + anzahl * 4 > laenge` guard (2145). None of that is written down.
3. **Line 2115's `sid`** is a borrowed interior pointer into `puffer`; it stays
   in use until `EqualSid` at line 2159 and is only valid while `puffer` is
   alive. That lifetime coupling is invisible at both use sites.

Per the cluster rule for this pass, a missing or pro-forma `// SAFETY:` on an
`unsafe` block in safe code is the finding: the invariants above are currently
upheld, but nothing at the call sites records *why*, so the next edit to the
buffer type, the `laenge` guard, or the `noetig` check silently removes the
proof. That risk is concentrated in the crate's authentication path.

## Code

```rust
// broker/src/transport/server_v3.rs:2090
    let mut noetig = 0u32;
    unsafe {
        GetTokenInformation(token.0, TokenUser, std::ptr::null_mut(), 0, &mut noetig);
    }
    let groessen_fehler = unsafe { GetLastError() };
    if cleanup.auth_fehler == V3AuthTestFehler::TokenGroesse
        || groessen_fehler != ERROR_INSUFFICIENT_BUFFER
        || noetig < std::mem::size_of::<TOKEN_USER>() as u32
    {
        return false;
    }
    let mut puffer = vec![0u64; (noetig as usize).div_ceil(std::mem::size_of::<u64>())];
    if cleanup.auth_fehler == V3AuthTestFehler::TokenInformation
        || unsafe {
            GetTokenInformation(token.0, TokenUser, puffer.as_mut_ptr().cast(), noetig, &mut noetig)
        } == 0
    {
        return false;
    }
    let sid = unsafe { (*puffer.as_ptr().cast::<TOKEN_USER>()).User.Sid };   // <-- no // SAFETY:
```

```rust
// broker/src/transport/server_v3.rs:2143 — raw SID arithmetic, also undocumented
        let sid_bytes = fremde_sid_speicher.as_mut_ptr().cast::<u8>();
        let anzahl = unsafe { *sid_bytes.add(1) } as usize;
        if anzahl == 0 || 8 + anzahl * 4 > laenge as usize {
            return false;
        }
        let letzte = unsafe { sid_bytes.add(8 + (anzahl - 1) * 4).cast::<u32>() };
        let wert = unsafe { std::ptr::read_unaligned(letzte) };
        unsafe { std::ptr::write_unaligned(letzte, wert.wrapping_add(1)) };
```

Contrast with the same operation in `broker/src/server.rs:167`, which does
carry the reasoning:

```rust
    // SAFETY: der Puffer beginnt laut API-Vertrag mit einer gültigen
    // TOKEN_USER-Struktur, das Alignment stimmt (u64-Puffer + Riegel oben),
    // und `User.Sid` zeigt in denselben, weiterhin lebenden Puffer.
    let sid_ptr = unsafe { (*puffer.as_ptr().cast::<TOKEN_USER>()).User.Sid };
```

## Data flow

- **Source:** any local process that opens the broker's named pipe; the caller's
  access token (and therefore `noetig`, the `TOKEN_USER` contents and the SID
  byte length) is chosen by the connecting party via
  `ImpersonateNamedPipeClient` at line 2077.
- **Sink:** `(*puffer.as_ptr().cast::<TOKEN_USER>()).User.Sid` at
  `broker/src/transport/server_v3.rs:2115`, plus `IsValidSid`/`EqualSid` on the
  derived interior pointer at 2117/2159.
- **Validation:** present but undocumented — `groessen_fehler ==
  ERROR_INSUFFICIENT_BUFFER` and `noetig >= size_of::<TOKEN_USER>()` (2095-2098),
  `Vec<u64>` element type (2101) plus the detached `const _` alignment lock
  (2027). No `// SAFETY:` comment ties any of them to the dereference.

## Reachability trace

`v3_server_starten` → acceptor thread (`eqcop-v3-acceptor`, line 1771) →
`verbindung_bedienen` (line 2252) → `client_sid_authentisieren(griff.h, …)`
(line 2358) → `unsafe { (*puffer.as_ptr().cast::<TOKEN_USER>()).User.Sid }`
(line 2115). Runs on every accepted connection before any envelope is parsed.

## Impact

No UB today: the alignment lock at line 2027, the `Vec<u64>` buffer and the
`noetig`/`laenge` guards do currently hold. The impact is loss of the
soundness audit trail on the crate's security gate. A future change that is
locally reasonable — switching `puffer` to `Vec<u8>` to avoid `div_ceil`,
relaxing the `noetig <` check, or moving the sub-authority arithmetic — turns
line 2115 into a misaligned read of a foreign token buffer or lines 2148-2150
into an out-of-bounds write, with no comment at the site to stop the reviewer.
The crate has no `[lints]` table and does not enable
`clippy::undocumented_unsafe_blocks` or `unsafe_op_in_unsafe_fn`, so nothing
mechanical catches the omission either.

## Mitigations checked

- `// SAFETY:` comment present and accurate: **absent** on all 17 blocks of
  this function (verified by adjacency scan of the 3 lines above each
  `unsafe {`); present on every other `unsafe` block in the same file.
- Compile-time alignment lock: present (`broker/src/transport/server_v3.rs:2027`
  and `broker/src/server.rs:119`) but not referenced from the use site.
- `debug_assert!` upstream: none on this path.
- MIRI / sanitizers: not run — `broker/` has no `rust-toolchain.toml` and
  `tools/beweise.ps1` runs no MIRI leg; the code is Win32-FFI-heavy and not
  MIRI-executable anyway.
- `#![deny(unsafe_op_in_unsafe_fn)]` / `clippy::pedantic` /
  `clippy::undocumented_unsafe_blocks`: none — `broker/Cargo.toml` has no
  `[lints]` table and `broker/src/lib.rs` has no crate-level lint attributes.
- `V3AuthTestFehler` fault seams are reachable only via the `#[doc(hidden)]`
  `v3_server_starten_fuer_security_vectors`; production entry points pass
  `V3SecurityTestOptionen::default()`.

## Recommendation

1. Add a `// SAFETY:` comment to each `unsafe` block in
   `client_sid_authentisieren`, naming the concrete enforcer. For line 2115,
   mirror the wording already used at `broker/src/server.rs:167` and cite the
   `const _: () = assert!(align_of::<TOKEN_USER>() <= align_of::<u64>())` lock
   at line 2027 plus the `noetig >= size_of::<TOKEN_USER>()` check at 2097.
   For lines 2144-2150, state the SID header layout and point at the
   `laenge < 12` and `8 + anzahl * 4 > laenge` guards.
2. Give the `const _` at line 2027 a named comment like the one at
   `broker/src/server.rs:117` so the two locks are greppable together.
3. Add a `[lints.clippy]` table to `broker/Cargo.toml` with
   `undocumented_unsafe_blocks = "deny"` (and `multiple_unsafe_ops_per_block =
   "warn"`) so the omission cannot recur silently; the crate is already clean
   everywhere else, so the gate costs nothing today.
