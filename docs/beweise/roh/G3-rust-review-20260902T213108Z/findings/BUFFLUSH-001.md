---
id: BUFFLUSH-001
bug_class: bufwriter-unflushed
title: v3 pipe connection handle closed without FlushFileBuffers — buffered frames discarded while the store already compacted their re-delivery debt
location: broker/src/transport/server_v3.rs:678
function: Verbindungsgriff::drop
confidence: High
worker: worker-13
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: no FlushFileBuffers/DisconnectNamedPipe anywhere in src/transport/ (only legacy v2 server.rs:876/890), Verbindungsgriff::drop closes the handle bare, and coordinator.rs:2274-2281 retires persistent outbox debt via snapshot_schuld_kompaktieren on the buffered-write bool."
severity: MEDIUM
attack_vector: Local
exploitability: Difficult
severity_rationale: "Untrusted local pipe client can time a teardown so a snapshot recorded as delivered is discarded from the pipe buffer; silent integrity divergence between store and app with no counter, but it needs the attacker to hit the window between write and close."
---

## Description

`Verbindungsgriff` owns the server end of a named-pipe instance (created by
`CreateNamedPipeW` with a 64 KiB output buffer, `broker/src/transport/server_v3.rs:400`).
Its `Drop` closes the handle with `CloseHandle` and nothing else — the v3
transport never calls `FlushFileBuffers`; the symbol is not even imported
(imports at `broker/src/transport/server_v3.rs:62-68`).

On Windows, `WriteFile` against the *server* end of a named pipe completes as
soon as the payload has been copied into the pipe instance's output buffer; it
does not wait for the client to read. `FlushFileBuffers` on a pipe server
handle is the documented mechanism that does not return until the client has
read all buffered data, and it is what Microsoft's own pipe-server guidance
puts *before* `DisconnectNamedPipe`/`CloseHandle`. This broker's own legacy v2
server implements exactly that (`broker/src/server.rs:874-890`,
`pipe_nach_antwort_schliessen`, with the comment
"FlushFileBuffers ist bei Named Pipes synchron und wartet auf den Client").
The v3 rewrite dropped it.

The writer thread nevertheless treats a completed `ov_schreiben` as proof of
delivery and reports it upward:

- `broker/src/transport/server_v3.rs:626-627` documents `ov_schreiben` as
  "Blockiert, solange der Peer nicht abholt" — that is only true once the
  64 KiB buffer is full; for a normal frame with an idle peer the call returns
  immediately after buffering.
- `broker/src/transport/server_v3.rs:2618-2621`: `geschrieben` is sent back
  through the entry's answer channel.
- `V3Sender::snapshot_schreiben` (`broker/src/transport/server_v3.rs:1028-1047`)
  returns that bool to the coordinator.
- `Coordinator::flush_session` (`broker/src/coordinator.rs:2274-2281`) and
  `coordinator.rs:1079-1086` react to `geschrieben == true` by calling
  `store.snapshot_schuld_kompaktieren(ziel, ord)`, which deletes the outbox
  debt for that target up to `event_ord` (`broker/src/store.rs:476-488`,
  `WriterBefehl::CompactSnapshotDebt`).

So the broker permanently retires its obligation to re-deliver a session
snapshot on the strength of a write that only reached a kernel buffer, and then
closes that buffer's owner without draining it. There is no application-level
receipt: the only `bestaetigt`/confirmation state in the coordinator is session
join confirmation, not snapshot receipt.

## Code

```rust
// broker/src/transport/server_v3.rs:675-681
impl Drop for Verbindungsgriff {
    fn drop(&mut self) {
        // SAFETY: exklusiver Besitz ueber den Arc, genau einmal geschlossen.
        unsafe { CloseHandle(self.h) };
        self.sicherheits_spur.push("close");
    }
}

// broker/src/transport/server_v3.rs:2616-2622 (writer thread)
let geschrieben =
    !erzwungen && ov_schreiben(griff.h, ereignis.roh(), &eintrag.frame);
if let Some(antwort) = eintrag.geschrieben {
    let _ = antwort.send(geschrieben);   // "written" == "buffered", not "read"
}

// broker/src/coordinator.rs:2274-2281
let geschrieben = self.push_ziel_noch_gueltig(&link_id, &ziel)
    && push.as_ref().is_some_and(|push| push.snapshot_schreiben(&link_id, &payload));
if geschrieben {
    if let (Some(store), Some(ord)) = (&self.store, event_ord) {
        let _ = store.snapshot_schuld_kompaktieren(ziel, ord);  // debt deleted
    }
}
```

For contrast, the same project's v2 path:

```rust
// broker/src/server.rs:874-890
fn pipe_nach_antwort_schliessen(datei: &File, register: &Arc<Mutex<Register>>) {
    // FlushFileBuffers ist bei Named Pipes synchron und wartet auf den Client.
    ...
        .spawn(move || unsafe {
            FlushFileBuffers(flush_datei.as_raw_handle() as HANDLE);
        }) {
```

## Data flow

- **Source:** connection-teardown timing, controllable by any local process
  that can open the pipe — bootstrap-deadline expiry, protocol reject, coordinator
  eviction of an older link (`anmeldung.zu_schliessende_links` →
  `kopplung_loesen(..., true)`, `broker/src/transport/server_v3.rs:2466-2470`),
  `stoppen()`/`io_abbrechen`.
- **Sink:** `CloseHandle(self.h)` at `broker/src/transport/server_v3.rs:678`,
  with no `FlushFileBuffers` and no post-write drain anywhere in the module.
- **Validation:** none. No flush, no peer-side ack, no re-send after the debt is
  compacted. `Ausgang::schliessen` (`server_v3.rs:945-957`) only reports `false`
  for entries still *queued*; entries already handed to `ov_schreiben` are
  counted as written.

## Reachability trace

`Coordinator::flush_session` → `V3Sender::snapshot_schreiben` (server_v3.rs:1028)
→ `Ausgang::snapshot_einreihen_mit_antwort` → writer thread `ov_schreiben`
(server_v3.rs:2618, frame lands in the 64 KiB pipe buffer) → `antwort.send(true)`
→ `Coordinator` calls `store.snapshot_schuld_kompaktieren` (coordinator.rs:2276)
→ connection teardown (`ende.setzen()` → `ausgang.schliessen()` →
`io_abbrechen` → join, server_v3.rs:2869-2911) → last `Arc<Verbindungsgriff>`
drops → `CloseHandle` at server_v3.rs:678 destroys the instance together with
any bytes the peer had not yet read.

## Impact

Silent, unrecoverable loss of integrity-relevant protocol output on a path the
broker has already recorded as delivered: the session snapshot (P1) is dropped
from the store's outbox debt, so it is never re-sent, while the peer never saw
it. The result is a divergence between the broker's persisted view ("subscriber
is covered up to event_ord N") and the app's actual state, with no error
counter incremented and no log entry — `geschlossen_writer` is only bumped when
`ov_schreiben` itself fails.

Under the LOCAL_UNPRIVILEGED model this is attacker-influenceable: a local
process that can open the pipe can drive a control hello that makes the
coordinator evict the legitimate link (`zu_schliessende_links`) at a moment of
the attacker's choosing, forcing the teardown-and-close of a connection whose
last snapshot frame is still sitting unread in the pipe buffer. Repeated, this
is a targeted "the app silently misses committed state updates" primitive
rather than a visible denial of service. The same loss window also covers P0
command answers and the `welcome` frame, where the peer sees only
ERROR_BROKEN_PIPE instead of the protocol-level answer.

## Mitigations checked

- `// SAFETY:` comment at `server_v3.rs:677` only claims the handle is closed
  exactly once; it makes no claim about pipe drainage — the invariant that is
  actually missing.
- No `FlushFileBuffers` anywhere in `broker/src/transport/` (`rg` over
  `broker/src`: the only hits are `broker/src/server.rs:36,876,890`, i.e. the
  legacy v2 path). Not imported in `server_v3.rs`.
- No `DisconnectNamedPipe` in the connection-teardown path either
  (`server_v3.rs:495` is the *listener* instance, not the served connection).
- No application-level snapshot acknowledgement exists; `bestaetigt` in the
  coordinator is session-join confirmation, unrelated.
- Tests: `broker/src/transport/server_v3.rs` in-file tests read `welcome` while
  the connection stays open; `broker/tests/security_vectors.rs` reads the reject
  with a read posted before the server writes. No test asserts that a peer
  without a pending read still receives the last frame after teardown.
- No MIRI / sanitizer / fuzzing coverage of this path; no `[lints]` table.

## Recommendation

Make "written" mean "read by the peer" before it is allowed to retire store
state:

1. Before the connection handle is closed, call `FlushFileBuffers(h)` on the
   server end with a hard deadline — reuse the v2 pattern in
   `broker/src/server.rs:874-903` (own thread + `ANTWORT_FLUSH_TIMEOUT`, so a
   silent peer cannot hold the broker), then `DisconnectNamedPipe`, then
   `CloseHandle`. Placing it in `Verbindungsgriff::drop` covers every exit path.
2. Only propagate `geschrieben = true` to the coordinator after that flush has
   confirmed drainage for entries that carry an answer channel
   (`AusgangEintrag::geschrieben`), or gate `snapshot_schuld_kompaktieren` on a
   peer-side acknowledgement instead of on the write result.
3. Count and surface flush timeouts (e.g. a `flush_abgelaufen` counter beside
   `geschlossen_writer`) so a lost delivery is observable rather than silent.
