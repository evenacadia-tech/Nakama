---
id: BUFFLUSH-002
bug_class: bufwriter-unflushed
title: Bootstrap reject frame written and abandoned — no flush before the connection handle closes (v2 flush-on-answer path not carried over to v3)
location: broker/src/transport/server_v3.rs:2217
function: bootstrap_reject_schreiben
confidence: Medium
worker: worker-13
fp_verdict: TRUE_POSITIVE
fp_rationale: "Same verified mechanism as BUFFLUSH-001 at the reject path: ov_schreiben buffers, the sole Arc<Verbindungsgriff> drops immediately and CloseHandle discards unread bytes; v2's pipe_nach_antwort_schliessen flush was not carried into v3."
severity: LOW
attack_vector: Local
exploitability: Reliable
severity_rationale: "Impact is limited to a rejected peer losing the protocol-level reason and seeing ERROR_BROKEN_PIPE instead; no state, session or store consequence, so negligible impact even though the loss is easy to provoke."
---

## Description

`bootstrap_reject_schreiben` is the v3 server's "tell the peer why it was
rejected, then hang up" path. It builds the `reject` frame, pushes it into the
pipe with `ov_schreiben`, discards the result, and returns; every caller
`return`s immediately afterwards, so the sole `Arc<Verbindungsgriff>` drops and
`CloseHandle` runs (`broker/src/transport/server_v3.rs:678`) microseconds later.

`ov_schreiben` completes once the bytes are in the pipe instance's 64 KiB output
buffer (`CreateNamedPipeW`, `server_v3.rs:400`); it does not wait for the client
to read. Nothing between the write and the close waits for the peer: no
`FlushFileBuffers` (not imported in this module, imports at `server_v3.rs:62-68`),
no read-to-EOF drain, no delay. A peer that is not already blocked in a read at
that instant — the realistic case for a poll-driven client, and for the C++
probe whose frame pump is not necessarily parked in `ReadFile` — can lose the
answer and observe only a broken pipe.

This is a regression against the same project's v2 server, which treats exactly
this "answer, then close" case as needing a flush and implements it with a
deadline so a silent peer cannot block the broker
(`broker/src/server.rs:874-903`, `pipe_nach_antwort_schliessen`:
"FlushFileBuffers ist bei Named Pipes synchron und wartet auf den Client.
Deshalb läuft nur der seltene Reject-Flush in einem eigenen Thread und wird nach
einer harten Frist über dessen stabiles Threadhandle abgebrochen."). The v3
rewrite (NAK-123) reproduced the write but not the flush.

## Code

```rust
// broker/src/transport/server_v3.rs:2202-2219
fn bootstrap_reject_schreiben(
    h: HANDLE,
    ereignis: HANDLE,
    grund: &str,
    cleanup: &VerbindungsSicherheitsCleanup,
) {
    let payload = serde_json::json!({
        "type": "reject",
        "code": "protocol_mismatch",
        "reason": grund.chars().take(500).collect::<String>()
    });
    if let Ok(payload) = serde_json::to_vec(&payload) {
        let mut rahmen = (payload.len() as u32).to_le_bytes().to_vec();
        rahmen.extend_from_slice(&payload);
        cleanup.spur("reject");
        let _ = ov_schreiben(h, ereignis, &rahmen);   // buffered only
    }
}                                                     // caller returns -> CloseHandle
```

Same shape for the second reject variant, written inline and abandoned:

```rust
// broker/src/transport/server_v3.rs:2464-2470
sicherheits_cleanup.spur("reject");
let _ = ov_schreiben(griff.h, leseereignis.roh(), &rahmen);
statistik.geschlossen_bootstrap.fetch_add(1, Ordering::SeqCst);
return;
```

## Data flow

- **Source:** any local process that opens the pipe and sends a bootstrap frame
  the broker rejects (bad JSON, wrong `type`, wrong `kind`, protocol != 3,
  oversized bootstrap, v2 framing on the v3 endpoint) — the rejection reasons
  enumerated at `server_v3.rs:2430-2470` and exercised by
  `broker/tests/security_vectors.rs:496-520`.
- **Sink:** `ov_schreiben(h, ereignis, &rahmen)` at
  `broker/src/transport/server_v3.rs:2217`, immediately followed by the
  unflushed `CloseHandle` in `Verbindungsgriff::drop`.
- **Validation:** none — the write result is explicitly discarded (`let _ =`)
  and no drainage is awaited.

## Reachability trace

client connects → `verbindung_bedienen` → bootstrap read/parse fails
(`server_v3.rs:2430`) → `bootstrap_reject_schreiben` (`server_v3.rs:2217`)
→ `return` → last `Arc<Verbindungsgriff>` drops → `CloseHandle`
(`server_v3.rs:678`) destroys the pipe instance with the reject possibly still
unread in its output buffer.

## Impact

The documented protocol-level failure signal is best-effort in a place where the
contract says it must be explicit: §33.3 "klarer Kompatibilitaetsfehler" is the
stated reason the v2-on-v3 case sends a v2-framed `reject` at all. When the
frame is lost, a legitimate probe/app sees an indistinguishable
ERROR_BROKEN_PIPE for "your protocol is wrong", "you were rate limited"
(`server_v3.rs:2455-2461`) and "the broker died / something is squatting the
pipe name" — the exact discrimination a client needs to decide between "stop and
report a version mismatch" and "retry the connection". The practical outcome is
a reconnect loop against a broker that keeps rejecting, with no diagnosable
reason on either side, plus loss of the only channel that carries the
`rate_limited` back-off hint.

## Mitigations checked

- No `FlushFileBuffers` in `broker/src/transport/` at all; the only occurrences
  in the crate are the legacy v2 path (`broker/src/server.rs:36,876,890`).
- The write result is deliberately dropped (`let _ =`), so even a hard
  `ERROR_NO_DATA`/broken-pipe failure of the reject write is invisible; only
  `geschlossen_bootstrap` is counted, and that counter is incremented on both
  the success and the failure branch.
- `broker/tests/security_vectors.rs:496-520`
  (`vollstaendiges_abzulehnendes_hello_impersoniert_prueft_sid_und_revertiert_vor_reject`)
  does assert the client reads the reject, but the client calls `read_exact`
  immediately after `write_all` (`security_vectors.rs:509-511`), i.e. the read is
  already pending when the server writes — the best-case timing. It does not
  cover a peer that is not parked in a read at that moment, which is the losing
  case.
- The `sicherheits_spur` order check only proves `reject` was *attempted* before
  `close`, not that it was delivered.

## Recommendation

Give the v3 reject path the same guarantee the v2 path already has: after
`ov_schreiben`, call `FlushFileBuffers` on the connection handle behind a hard
deadline (the `pipe_nach_antwort_schliessen` pattern in
`broker/src/server.rs:874-903`) before returning, so the handle close cannot
race the peer's read; a hostile peer that never reads is bounded by the same
timeout. Also stop discarding the write result — count a failed reject write
into a dedicated statistic instead of `let _ =`, so an undeliverable rejection
is observable.
