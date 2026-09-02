---
stage: fp-judge
threat_model: LOCAL_UNPRIVILEGED
primaries_evaluated: 28
true_positives: 22
likely_tp: 5
likely_fp: 1
false_positives: 0
out_of_scope: 0
---

# FP-Judge Summary

Run `20260902T213108Z` — threat model `LOCAL_UNPRIVILEGED`, severity filter
`medium`, scope root `broker/`, phase gate G3 at `ad5b2d1b`. Dedup delivered 28
primaries out of 29 raw findings (UAF-001 merged into OPAQUEPTR-001). Every
verdict below was rendered against the source read in this session; the
repository was treated as read-only (no cargo invocation, no pipe opened).

## Verdict counts (primaries)
| Verdict | Count |
|---------|-------|
| TRUE_POSITIVE | 22 |
| LIKELY_TP | 5 |
| LIKELY_FP | 1 |
| FALSE_POSITIVE | 0 |
| OUT_OF_SCOPE | 0 |

Severity lines on disk: 27 (= TRUE_POSITIVE + LIKELY_TP). Distribution:
CRITICAL 0, HIGH 0, MEDIUM 14, LOW 13.

## Threat-model calibration used
`context.md` declares the trust boundary explicitly: pipe clients are UNTRUSTED
until the duplex handshake proves possession of the session credentials, and the
adversary is a local unprivileged process doing impersonation, name squatting
and malformed input. The boundary in this review is therefore **authentication
at the pipe**, not an OS privilege step — the broker and the probe plugins run
as the same logon SID by design (`client_sid_authentisieren` compares the peer
token against `sicherheit.user_sid()`).

Consequences applied uniformly:

- An untrusted local pipe client producing a concrete adverse effect on the
  broker or on other sessions crosses that boundary. Under the local severity
  table the top DoS tier is **MEDIUM ("Local DoS")**, so no finding in this run
  reaches HIGH or CRITICAL — none of them yields other users' data, privileged
  file access, or privilege escalation.
- Real defects that no attacker drives (shutdown ordering, lossy path
  conversion, missing lint/MSRV config, undocumented-but-currently-correct
  `unsafe`, discarded diagnostics) are **TRUE_POSITIVE at LOW**, not FP. Their
  premise is correct; only the impact is small.
- `LIKELY_FP` is reserved for a finding whose premise survives but whose sink is
  provably unreachable for this attacker.

## Per-primary verdicts
| ID | Bug class | Verdict | Severity | Rationale |
|----|-----------|---------|----------|-----------|
| BUFFLUSH-001 | bufwriter-unflushed | TRUE_POSITIVE | MEDIUM | No `FlushFileBuffers` in v3 while `snapshot_schuld_kompaktieren` retires outbox debt on a merely buffered write |
| BUFFLUSH-002 | bufwriter-unflushed | TRUE_POSITIVE | LOW | Reject frame buffered then `CloseHandle`; peer loses the reason, no state consequence |
| CARGOLINT-001 | cargo-lint-config | TRUE_POSITIVE | LOW | Verified 89 `unsafe` vs 60 `// SAFETY:`, no `[lints]`, no clippy leg — hardening gap |
| CHANSTARVE-001 | channel-starvation | TRUE_POSITIVE | MEDIUM | `recv_timeout(2 s)` per subscriber on the single `nakama-coordinator-tick` thread; peer just stops reading |
| DROPPANIC-001 | drop-panic | TRUE_POSITIVE | MEDIUM | `Drop` calls a poison-fatal `expect` sink; double panic aborts the process (drop-panic floor) |
| DROPSKIP-001 | destructor-skip | TRUE_POSITIVE | LOW | `BrokerLauf.store` never stopped and never dropped from the `static OnceLock` |
| LOSSYSTR-001 | lossy-str-conversion | TRUE_POSITIVE | LOW | `String::from_utf16_lossy` on the store path; same-user profile, so no boundary crossed |
| MSRV-001 | msrv-mismatch | TRUE_POSITIVE | LOW | No `rust-version`, no toolchain pin; floor set by a caret dependency |
| NONDET-001 | nondeterminism | TRUE_POSITIVE | MEDIUM | P0 target chosen by `HashMap` order when a duplicate-nonce link stays live |
| OPAQUEPTR-001 | opaque-pointer | LIKELY_TP | MEDIUM | `CloseHandle` before deregistration on the spawn-failure arm; `CancelIoEx` on a recycled handle (absorbed UAF-001) |
| RAWFD-001 | raw-fd-lifecycle | LIKELY_TP | LOW | Detached writer keeps the handle after the register entry is removed; trigger not shown forcible |
| RAWFD-002 | raw-fd-lifecycle | LIKELY_TP | MEDIUM | Same defect as OPAQUEPTR-001, unmerged only due to path-prefix drift |
| REENTRANT-001 | reentrancy-unsafe | TRUE_POSITIVE | MEDIUM | Global `stand` guard held across the `dyn SessionPush` loop on the P2 ingress path |
| REENTRANT-002 | reentrancy-unsafe | TRUE_POSITIVE | MEDIUM | Same construct at subscriber attach; named guard lives to end of function |
| RESDISC-001 | result-discarded | TRUE_POSITIVE | LOW | Automatic WAL checkpoint errors dropped and marked done; `Err` needs an IO/disk fault, not peer input |
| RESDISC-002 | result-discarded | TRUE_POSITIVE | LOW | Worker join `Result` discarded — silent crash oracle, detection-control loss |
| RESEXHAUST-001 | resource-exhaustion | TRUE_POSITIVE | MEDIUM | `MAX_EINTRAEGE` recorded but never bounds the validation loop; ~1000x amplification per frame |
| RESEXHAUST-002 | resource-exhaustion | TRUE_POSITIVE | MEDIUM | `CAP_INGRESS` counts entries, not bytes; ~64 MiB/connection, ~6 GiB broker-wide, P2 never disconnects |
| RESEXHAUST-003 | resource-exhaustion | TRUE_POSITIVE | MEDIUM | `quarantaene` has `insert` but no `remove`, no cap, no expiry; address space attacker-chosen |
| RESEXHAUST-004 | resource-exhaustion | TRUE_POSITIVE | MEDIUM | Unbounded `conflict_guards` rows; only DELETE path is never called; survives restart |
| SAFETYDOC-001 | safety-doc | TRUE_POSITIVE | LOW | 18 `unsafe` occurrences, zero `// SAFETY:` in the authentication gate; invariants hold today |
| SAFETYDOC-002 | safety-doc | LIKELY_TP | MEDIUM | Same defect as OPAQUEPTR-001 seen at the `CancelIoEx` sink; the comment is false, not merely absent |
| STRCMP-001 | string-comparison | TRUE_POSITIVE | LOW | Probe-namespace prefix gate bypassable via a dot-dot segment in an un-normalized local-device path; interlock, not privilege |
| STRSLICE-001 | str-slice-boundary | LIKELY_FP | — | Unguarded byte-index slice is real, but the only producer is an `include_str!` compile-time schema |
| TOCTOU-001 | toctou | TRUE_POSITIVE | LOW | Three independent name resolutions; `exists()` pre-check makes the schema gate fail open |
| TOCTOU-002 | toctou | TRUE_POSITIVE | LOW | Remote-volume guard climbs to an existing ancestor, so it never inspects the object later opened |
| UNSAFESYNC-001 | unsafe-sync-impl | LIKELY_TP | MEDIUM | Same defect as OPAQUEPTR-001 seen at the manual `unsafe impl Send`; its justification is untrue on that path |
| UNWRAP-001 | unwrap-on-untrusted | TRUE_POSITIVE | LOW | 54 poison-fatal `expect` sites; no attacker-reachable first panic shown — missing containment |

(UAF-001 carries `merged_into: OPAQUEPTR-001` and correctly holds no verdict of
its own.)

## One defect filed four times — read this before counting MEDIUMs
OPAQUEPTR-001, RAWFD-002, UNSAFESYNC-001 and SAFETYDOC-002 are **one** defect:
the connection HANDLE is closed before its entry leaves the `CancelIoEx`
register on the `thread::Builder::spawn` failure arm
(`broker/src/transport/server_v3.rs:1979`). Dedup merged only UAF-001 into
OPAQUEPTR-001; the other three landed in different Tier-2/Tier-3 buckets because
worker-19 and worker-5 write scope-relative paths while worker-1 and worker-16
write repo-relative ones, and because two of the filings are anchored at the
sink (`:1289`) and at the `unsafe impl` (`:975`) instead of at the acceptor. All
four carry the same MEDIUM tier by the "same bug, same severity" rule. **The
report's MEDIUM count of 14 therefore covers 11 distinct defects.**

## Common FP patterns observed
- **Sink unreachable for the defined attacker (1 finding, STRSLICE-001)** — a
  genuine unguarded byte-index `str` slice whose only producer is a
  compile-time `include_str!` schema; the worker documented this honestly in its
  own data-flow section and explicitly flagged it for the FP judge.
- **Correct code claim, overstated security framing (LOSSYSTR-001, TOCTOU-001,
  TOCTOU-002, STRCMP-001)** — the defect is verifiably in the source, but the
  postulated attacker is the same user the broker already runs as (its own
  `LocalAppData`, its own argv), so no privilege boundary is crossed. Kept as
  TRUE_POSITIVE at LOW rather than downgraded to FP: the code defect is real,
  only the impact claim was too strong.
- **Attacker path asserted but mechanically wrong (RESDISC-001)** — the finding
  claims a local process can hold a read transaction to keep checkpoints
  failing, then itself notes that a reader-blocked PASSIVE checkpoint returns
  `Ok(busy)`, not `Err`. The defect (discarded error, suppressed retry) stands;
  the named trigger does not.
- **Path-prefix drift inflating the finding count** — see the section above.
  This is a dedup-input artefact, not a worker error.

No finding was rejected because a worker misread the code. Every code claim
spot-checked in this pass reproduced against the source, including the exact
`unsafe`/`// SAFETY:` counts (89/60) in CARGOLINT-001 and the zero-SAFETY
window in `client_sid_authentisieren` (18 `unsafe` occurrences, 0 comments in
lines 2070-2200).

## Areas that need deeper analysis
- **Can `thread::Builder::spawn` be driven to `Err` from a local unprivileged
  process?** This is the single reachability question that separates the
  handle-lifecycle family (4 filings) from a latent-only defect. It decides
  whether MEDIUM is generous or conservative, and NAK-123's
  `tests/security_vectors.rs` has no seam for a failing *worker* spawn — only
  for listener/watchdog/acceptor start failures.
- **RAWFD-001's detach precondition.** Whether a peer can make `CancelIoEx` fail
  to unblock `ov_schreiben` across the full 2 s `join_mit_frist` retry loop was
  not resolvable by reading. If it can, a permanent leak of one of
  `PIPE_INSTANZEN = 98` instances per event turns it into endpoint DoS and the
  LOW assigned here is too low.
- **The unbound `logon_sid`.** `adresse.logon_sid` from `hello_control` is
  shape-checked (1..184 chars) but never compared against the SID that
  `client_sid_authentisieren` obtained for the pipe peer — confirmed by grepping
  every `logon_sid` use in `broker/src`. That single omission is what makes
  RESEXHAUST-003 and RESEXHAUST-004 unbounded, and it may have further
  consequences for routing and for `effektive_adresse` beyond any one finding.
- **NONDET-001's stronger sibling.** The duplicate-nonce link that makes the P0
  target ambiguous also repoints `client.current_link` to the newest
  registration. Whether that lets an attacker who replays a victim's address
  (readable from `session_snapshot`) *become* the command target was not
  assessed — no worker filed it, and inventing findings is outside this stage's
  remit. It is worth a targeted look.
- **No fuzzing, no MIRI, no clippy on 89 `unsafe` blocks.** CARGOLINT-001 sits
  at LOW on its own, but it is the reason several of the questions above cannot
  be answered mechanically.
