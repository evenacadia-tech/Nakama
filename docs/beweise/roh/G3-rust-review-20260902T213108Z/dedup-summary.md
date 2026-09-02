---
stage: dedup-judge
total_findings_in: 29
working_set_size: 29
unparseable_locations: 0
multi_locations: 0
tier1_merges: 0
tier2_merges: 0
tier3_merges: 1
primaries_after_dedup: 28
related_groups: 8
---

# Dedup Summary

Run: `20260902T213108Z` — threat model `LOCAL_UNPRIVILEGED`, scope root `broker/`,
phase gate G3 at `ad5b2d1b`. 21 workers, 29 raw findings, none carrying a prior
`merged_into` or `also_known_as` (this is a first dedup pass over this run).

## Location parse health
| Class | Count | Example IDs |
|-------|-------|-------------|
| parseable (`path:line`) | 29 | BUFFLUSH-001, CARGOLINT-001, OPAQUEPTR-001, TOCTOU-002, … |
| markdown-link (recovered) | 0 | — |
| multi-location (skipped Tier 1) | 0 | — |
| unparseable (skipped Tier 1) | 0 | — |

Three findings have no enclosing function (`(file-level)`) and are therefore
excluded from Tier 2/Tier 3 bucketing by rule: CARGOLINT-001, MSRV-001,
UNSAFESYNC-001.

### Path-prefix drift (affects bucketing — read this before assuming a missed merge)
Workers disagreed on the path root. 22 findings write repo-relative paths
(`broker/src/...`, `broker/Cargo.toml`); 7 findings from worker-5, worker-7 and
worker-19 write `finding_scope_root`-relative paths (`src/...`), which under
`finding_scope_root: broker` denote the **same files**:

| Scope-relative spelling | Findings | Same file as |
|---|---|---|
| `src/transport/server_v3.rs` | RAWFD-001, RAWFD-002, RESEXHAUST-002, UNSAFESYNC-001 | `broker/src/transport/server_v3.rs` |
| `src/coordinator.rs` | RESEXHAUST-004, UNWRAP-001 | `broker/src/coordinator.rs` |
| `src/lib.rs` | DROPSKIP-001 | `broker/src/lib.rs` |
| `src/telemetrie.rs`, `src/instance_alias.rs` | RESEXHAUST-001, RESEXHAUST-003 | `broker/src/…` (no counterpart filed) |

Path normalization is string-based by protocol (no resolution, no scope-root
rewriting), so these land in different buckets from their `broker/`-prefixed
twins and were **not** merged. That is the safe direction, but it means the
downstream judge must treat the cross-class related cluster below as one defect
family rather than four independent bugs. No merge was performed across these
spellings.

## Tier 1 — exact-location same-class merges (deterministic)
None. No `(path, line, bug_class)` bucket held more than one finding.

The only exact-location collision in the run is
`broker/src/transport/server_v3.rs:1979` shared by OPAQUEPTR-001
(`opaque-pointer`) and UAF-001 (`use-after-free`). Tier 1 is class-scoped by
construction and correctly declined it; it was handled in Tier 3 after a full
read. Note also `broker/Cargo.toml:1` (CARGOLINT-001) vs `broker/Cargo.toml:4`
(MSRV-001) — different lines *and* different classes; a missing `[lints]` table
is not a missing `rust-version`, and these stay separate.

## Tier 2 — same construct in same function (snippet-confirmed)
None. No `(path, function, bug_class)` bucket held more than one finding.

## Tier 3 — cross-class same-bug merges (LLM-confirmed)
| Primary | Merged IDs | Function | Merged classes | Rationale |
|---------|------------|----------|----------------|-----------|
| OPAQUEPTR-001 | UAF-001 | `v3_server_starten_intern` | use-after-free | Same construct: the `Err(_)` arm of the `thread::Builder::spawn` match at `broker/src/transport/server_v3.rs:1979`. Both describe closure-capture drop order releasing `griff` (→ `CloseHandle`, `:678`) before `handle_eintrag` (→ register `retain`, `:2229`), and both name the identical sink `CancelIoEx(*h as HANDLE)` in `alle_io_abbrechen` at `:1289`. Same attacker model (local unprivileged driving `spawn` failure), same root cause, same recommended fix (move deregistration into `Verbindungsgriff::drop`). One phrase: the register's raw `isize` handle (opaque pointer) is used via `CancelIoEx` after `CloseHandle` released it (use-after-free of a kernel handle) — one defect, two labels. |

Line delta 0; both confidence `Medium`, so no confidence raise was applied.
Primary chosen by the deterministic ordering (equal confidence → lexicographically
smallest id: `OPAQUEPTR-001` < `UAF-001`).

**Tier 3 candidate rejected:** bucket
`(src/transport/server_v3.rs, verbindung_bedienen)` = RAWFD-001
(`raw-fd-lifecycle`, line 2908, detached-writer handle strand at the
`join_mit_frist` teardown block) + RESEXHAUST-002 (`resource-exhaustion`,
line 2770, byte-unbounded ingress queue at `eingang.einreihen`). Different
constructs 138 lines apart, different sinks, different root causes — kept
separate.

## Tier 4 — Related (NOT merged — cross-reference only)
| Pattern | Finding IDs | Shared fix location |
|---------|-------------|---------------------|
| **Cross-class: HandleRegister drop-order family** — same spawn-failure defect filed under four classes across two path spellings and two functions. Not merged (differing `(path, function)` bucket keys; see path-prefix drift). Treat as one defect for severity. | OPAQUEPTR-001 (primary, already absorbed UAF-001), RAWFD-002, UNSAFESYNC-001, SAFETYDOC-002 | `broker/src/transport/server_v3.rs` — `Verbindungsgriff::drop` / `HandleEintrag::drop` / `alle_io_abbrechen` |
| `bufwriter-unflushed` — no `FlushFileBuffers` before `CloseHandle` on the v3 pipe (v2 path has it) | BUFFLUSH-001, BUFFLUSH-002 | `broker/src/transport/server_v3.rs` |
| `raw-fd-lifecycle` — pipe HANDLE vs cancel-register ownership on two distinct teardown paths | RAWFD-001, RAWFD-002 | `broker/src/transport/server_v3.rs` |
| `reentrancy-unsafe` — global `Coordinator::stand` guard held across `dyn SessionPush` callback loops | REENTRANT-001, REENTRANT-002 | `broker/src/coordinator.rs` |
| `result-discarded` — `let _ =` swallowing a failure signal (WAL checkpoint error; worker panic in join) | RESDISC-001, RESDISC-002 | `broker/src/store.rs`, `broker/src/transport/server_v3.rs` |
| `resource-exhaustion` — declared caps not enforced as control-flow bounds on attacker-scaled input | RESEXHAUST-001, RESEXHAUST-002, RESEXHAUST-003, RESEXHAUST-004 | `broker/src/telemetrie.rs`, `broker/src/transport/`, `broker/src/instance_alias.rs`, `broker/src/coordinator.rs` |
| `safety-doc` — `// SAFETY:` missing or asserting an unenforced invariant | SAFETYDOC-001, SAFETYDOC-002 | `broker/src/transport/server_v3.rs` |
| `toctou` — store gates decided by path name, never bound to the opened handle | TOCTOU-001, TOCTOU-002 | `broker/src/store.rs` |

Not grouped (single-instance classes): CARGOLINT-001, CHANSTARVE-001,
DROPPANIC-001, DROPSKIP-001, LOSSYSTR-001, MSRV-001, NONDET-001, STRCMP-001,
STRSLICE-001, UNWRAP-001.

## Bug-class counts (primaries only, after dedup)
| Bug class | Count |
|-----------|-------|
| resource-exhaustion | 4 |
| bufwriter-unflushed | 2 |
| raw-fd-lifecycle | 2 |
| reentrancy-unsafe | 2 |
| result-discarded | 2 |
| safety-doc | 2 |
| toctou | 2 |
| cargo-lint-config | 1 |
| channel-starvation | 1 |
| destructor-skip | 1 |
| drop-panic | 1 |
| lossy-str-conversion | 1 |
| msrv-mismatch | 1 |
| nondeterminism | 1 |
| opaque-pointer | 1 |
| str-slice-boundary | 1 |
| string-comparison | 1 |
| unsafe-sync-impl | 1 |
| unwrap-on-untrusted | 1 |
| use-after-free | 0 (merged into opaque-pointer) |
