# rust-review run summary — Nakama G3 (broker)

Base commit: `ad5b2d1b4729e09ea0cd58b6f22dba3f10624bed` (2026-09-02/03).
Predecessor run: `.rust-review-results/20260901T104834Z/` (G2, 2026-09-01).

## Resolved parameters

- threat_model: LOCAL_UNPRIVILEGED · severity_filter: medium
- finding_scope_root: broker · context_roots: .
- Capability flags: has_unsafe=true, has_ffi=true, has_concurrency=true,
  has_async=false, has_packed_repr=false, has_fs_io=true
- Cargo manifest: single-crate (broker/Cargo.toml); no workspace, no
  `rust-toolchain.toml`, no `build.rs`
- worker_model: opus · plan: 21 workers (build_run_plan.py, max 4 passes/worker)
- Plugin version: **1.1.0** (G2 ran on 1.0.0)
- Cache primer: ran foreground, returned the expected abort line with 0 tool
  calls; both waves spawned with `run_in_background` omitted.
- Waves: M=21 exceeds the per-message cap, so workers ran as two balanced
  waves (1–11, then 12–21), each wave one assistant message. Wave 2 started
  immediately after wave 1 returned, so the primer prefix was still warm.

## Worker outcomes (all 21 completed, validate_artifacts.py OK, exit 0)

| worker | cluster | findings | shard lines | coverage | status |
|---|---|---:|---:|---|---|
| 1 | unsafe-boundary | 2 | 2 | coverage/worker-1.md | completed |
| 2 | memory-safety-1 | 1 | 1 | coverage/worker-2.md | completed |
| 3 | memory-safety-2 | 0 | 0 | coverage/worker-3.md | completed |
| 4 | concurrency-locking | 3 | 3 | coverage/worker-4.md | completed |
| 5 | concurrency-data-race-1 | 1 | 1 | coverage/worker-5.md | completed |
| 6 | concurrency-data-race-2 | 0 | 0 | coverage/worker-6.md | completed |
| 7 | panic-dos-1 | 5 | 5 | coverage/worker-7.md | completed |
| 8 | panic-dos-2 | 1 | 1 | coverage/worker-8.md | completed |
| 9 | recursion-dos-1 | 0 | 0 | coverage/worker-9.md | completed |
| 10 | recursion-dos-2 | 0 | 0 | coverage/worker-10.md | completed |
| 11 | recursion-dos-3 | 0 | 0 | coverage/worker-11.md | completed |
| 12 | error-handling-1 | 4 | 4 | coverage/worker-12.md | completed |
| 13 | error-handling-2 | 2 | 2 | coverage/worker-13.md | completed |
| 14 | logic-correctness-1 | 0 | 0 | coverage/worker-14.md | completed |
| 15 | logic-correctness-2 | 2 | 2 | coverage/worker-15.md | completed |
| 16 | ffi-cross-language-1 | 1 | 1 | coverage/worker-16.md | completed |
| 17 | ffi-cross-language-2 | 0 | 0 | coverage/worker-17.md | completed |
| 18 | static-hygiene | 2 | 2 | coverage/worker-18.md | completed |
| 19 | resource-handling | 3 | 3 | coverage/worker-19.md | completed |
| 20 | input-os-safety | 2 | 2 | coverage/worker-20.md | completed |
| 21 | info-disclosure | 0 | 0 | coverage/worker-21.md | completed |

**No cluster task is incomplete.** No aborts, no retries, no worker returned
the `truncated at hard cap` token, and no coverage file contains a
`NOT SEARCHED` row (checked by grep over `coverage/*.md`).

- findings-index.txt: 29 lines, exactly matching the sum of the 21 worker
  claims (29).
- Orphan reconcile: none — every finding basename on disk appears in a shard.

## Bookkeeping notes

- **Shard path spelling (same defect as G2).** Seven workers (1, 2, 7, 8, 16,
  18, 19) wrote their index shards with the Git-Bash `/c/...` spelling, which
  `validate_artifacts.py` resolves to the nonexistent `C:\c\Users\...`. The
  shards were normalized to `C:/...` with `sed 's#^/c/#C:/#'` before
  validation; **no finding content was touched**. First validator run failed
  only on that spelling; the run after normalization returned
  `validate_artifacts: OK` with a real exit code of 0 (measured without a
  pipe, since a pipe masks the exit code).
- **Prompt encoding.** `build_run_plan.py` writes the worker prompts in the
  Windows ANSI code page (cp1252, CRLF), so the em-dash separators arrive as
  byte `0x97`. The orchestrator passed the prompts through with a real em-dash
  in those four punctuation positions; every field the worker self-check reads
  was passed verbatim, and all 21 workers passed their self-check.
- **Read-only discipline.** The run constraints (no `cargo build/test/run/
  check/clippy`, no writes to tracked files, no named-pipe processes) were
  carried into every worker prompt through `context.md`, which the planner
  embeds verbatim. Several workers explicitly confirmed a clean
  `git status`. HEAD stayed at `ad5b2d1b`, the worktree stayed clean, and the
  three binaries under `broker/target/release/` used by the running soak test
  were unchanged in size and mtime before and after the run.
- **Task ledger.** The orchestrator tracked all 21 clusters with
  TaskCreate/TaskUpdate; every task is `completed`.

## Judges

- dedup-judge: **complete** — 29 findings to 28 primaries (0 tier-1, 0 tier-2,
  1 tier-3 cross-class merge, 8 related groups). The merge folded `UAF-001`
  (`use-after-free`) into `OPAQUEPTR-001` (`opaque-pointer`): both describe the
  same `Err(_)` arm of the `thread::Builder::spawn` match at
  `broker/src/transport/server_v3.rs:1979`, where closure-capture drop order
  releases the handle before the cancel register is cleaned.
- **Path-prefix drift (worth acting on).** Workers 5, 7 and 19 wrote their
  finding locations relative to `finding_scope_root` (`src/transport/...`),
  every other worker wrote them repo-relative (`broker/src/...`). Under
  `finding_scope_root: broker` these are the same files, but the dedup
  protocol normalizes paths as strings, so twins landed in different buckets
  and were deliberately not merged. Consequence recorded by the judge:
  OPAQUEPTR-001, RAWFD-002, UNSAFESYNC-001 and SAFETYDOC-002 describe one
  defect - the HandleRegister spawn-failure drop-order window - filed under
  four bug classes across two path spellings and two functions. Weigh it as
  one work item, not four.
- fp-judge: **complete** — 28 primaries judged: 22 TRUE_POSITIVE, 5 LIKELY_TP,
  1 LIKELY_FP, 0 FALSE_POSITIVE, 0 OUT_OF_SCOPE. Severities: 14 MEDIUM,
  13 LOW, no HIGH and no CRITICAL; 14 reported under the `medium` filter.
  The judge repaired a self-inflicted frontmatter error mid-run (its first
  edit pass consumed each file's closing `---`, which made the SARIF generator
  skip all 28); it detected this from the generator's stderr, repaired all 28
  files and re-ran. The final SARIF reports 0 skipped.
- SARIF: **complete** — `generate_sarif.py` was re-run unconditionally as the
  Phase-8b safety net, exit code 0, no `WARNING: skipped` line, no dropped
  findings. `REPORT.md` (35 693 bytes) is judge-authored and was NOT
  overwritten by the orchestrator.
- delta-vs-20260901T104834Z.md: **written** — per-primary comparison against
  the G2 run, plus a per-case verdict (disappeared through NAK-123 with diff
  evidence, versus not rediscovered) for every G2 primary missing here.

## Result

28 primaries — 20 known from G2, 8 new. 14 reported (all MEDIUM), 13 LOW and
1 LIKELY_FP below the filter. Of the 25 G2 primaries, 17 were rediscovered,
2 are provably gone through NAK-123 (RAWPTR-001, RAWFD-001), and 6 were not
rediscovered although the defect still exists in the current source. See
`delta-vs-20260901T104834Z.md`.
