---
id: RAWFD-001
bug_class: raw-fd-lifecycle
title: Detached writer thread strands a connected named-pipe HANDLE that is deregistered from the cancel register and never closed
location: src/transport/server_v3.rs:2908
function: verbindung_bedienen
confidence: Medium
worker: worker-19
fp_verdict: LIKELY_TP
fp_rationale: "Verified: join_mit_frist returns false by dropping the JoinHandle (server_v3.rs:1314-1326), the detached writer keeps its Arc<Verbindungsgriff>, and _handle_eintrag (body local at :2275) then removes the register entry on return, so no later CancelIoEx can reach that handle; unconfirmed is whether an attacker can make the 2 s of repeated io_abbrechen fail to unblock the write."
severity: LOW
attack_vector: Local
exploitability: Theoretical
severity_rationale: "Inverted ownership on the detach path leaks a thread, a HANDLE and one of PIPE_INSTANZEN=98 pipe instances permanently when it fires, which would escalate to endpoint DoS if repeatable — but the precondition is CancelIoEx failing across the whole SENKE_FRIST retry loop, which I could not show is attacker-forcible."
---

## Description

`verbindung_bedienen` owns one connected named-pipe instance through
`Arc<Verbindungsgriff>`; `Verbindungsgriff::drop` is the *only* `CloseHandle`
for that handle (server_v3.rs:675-681). Three threads hold clones of that
`Arc`, and the writer thread (`eqcop-v3-writer`) is the one that blocks
indefinitely on peer-controlled I/O (`ov_schreiben` waits `INFINITE`,
server_v3.rs:627-663).

The only mechanism that can unblock that writer is a `CancelIoEx` issued
through `HandleRegister`, and the register entry is owned by
`HandleEintrag` — a **body-local of `verbindung_bedienen`**
(`let _handle_eintrag = handle_eintrag;`, server_v3.rs:2275).

At teardown the writer join is deliberately time-boxed:

```rust
if let Some(j) = schreiber {
    if !join_mit_frist(j, SENKE_FRIST, || io_abbrechen(&handles, id)) {
        statistik.schreiber_abgeloest.fetch_add(1, Ordering::SeqCst);
    }
}
```

`join_mit_frist` returning `false` means the thread was **detached, not
joined** (server_v3.rs:1314-1330, "das `JoinHandle` faellt, der Thread haelt
nur noch seine `Arc`s"). The detached writer still holds its
`Arc<Verbindungsgriff>` clone and is still blocked inside `WriteFile` on the
handle.

`verbindung_bedienen` then returns. Rust drops body locals before parameters,
so `_handle_eintrag` runs first and executes
`r.offen.retain(|(i, _)| *i != self.id)` (server_v3.rs:2226-2232), **removing
the handle from the register while a live thread still owns it**. That is
exactly the inverse of the invariant asserted on the register itself:

```rust
// Das Register haelt sie nur, solange der besitzende Thread seinen
// `Verbindungsgriff` noch nicht fallen gelassen hat; ...
unsafe impl Send for HandleRegister {}
```

From that moment on nothing in the process can reach the handle:
`alle_io_abbrechen` (used by `V3Griff::stoppen()` in a 5 ms retry loop) and
`io_abbrechen` both iterate `reg.offen` only, and the watchdog thread does the
same. The detached writer therefore blocks forever, its `Arc` never drops, and
`CloseHandle` is never called — a permanent handle + pipe-instance leak for
the life of the process.

## Code

```rust
// src/transport/server_v3.rs:2896-2911 — connection teardown
    ausgang.schliessen();
    io_abbrechen(&handles, id);
    // Alle Joins haben eine FRIST. Steht ein Verbraucher in einem
    // Senkenaufruf oder der Schreiber in einem Write, den `CancelIoEx` nicht
    // loest, wird der Thread abgeloest statt gejoint [...]
    for j in [verbraucher_p0, verbraucher_rest].into_iter().flatten() {
        if !join_mit_frist(j, SENKE_FRIST, || {}) {
            statistik.senke_abgeloest.fetch_add(1, Ordering::SeqCst);
        }
    }
    if let Some(j) = schreiber {
        if !join_mit_frist(j, SENKE_FRIST, || io_abbrechen(&handles, id)) {
            statistik.schreiber_abgeloest.fetch_add(1, Ordering::SeqCst);
        }
    }
    // ... function returns; `_handle_eintrag` (declared at :2275) drops here and
    // removes (id, handle) from HandleRegister while the detached writer still
    // holds an Arc<Verbindungsgriff> for that very handle.
```

```rust
// src/transport/server_v3.rs:675-681 — the only closer
impl Drop for Verbindungsgriff {
    fn drop(&mut self) {
        // SAFETY: exklusiver Besitz ueber den Arc, genau einmal geschlossen.
        unsafe { CloseHandle(self.h) };
        self.sicherheits_spur.push("close");
    }
}
```

## Data flow

- **Source:** a local unprivileged process connects to the v3 pipe, completes
  the duplex handshake, then stops reading its side of the pipe so that the
  broker's outbound `WriteFile` blocks with a full 64 KiB pipe buffer
  (`ov_schreiben`, `WaitForSingleObject(e, INFINITE)`, server_v3.rs:648).
- **Sink:** `HandleEintrag::drop` at server_v3.rs:2226-2232 removes the handle
  from `HandleRegister` while `Arc<Verbindungsgriff>` is still held by the
  detached `eqcop-v3-writer` thread; `Verbindungsgriff::drop` (the sole
  `CloseHandle`) therefore never runs.
- **Validation:** none. `join_mit_frist` only counts the detachment
  (`statistik.schreiber_abgeloest`); it does not keep the register entry alive,
  does not re-arm a cancellation path, and does not transfer handle ownership
  to any surviving owner.

## Reachability trace

`v3_server_starten_intern` (acceptor) -> `verbindung_bedienen` -> writer thread
`eqcop-v3-writer` blocked in `ov_schreiben` -> teardown
`join_mit_frist(schreiber, SENKE_FRIST, io_abbrechen)` returns `false` ->
`verbindung_bedienen` returns -> `_handle_eintrag::drop` deregisters ->
`V3Griff::stoppen()` -> `alle_io_abbrechen` iterates a register that no longer
contains the handle -> writer stays blocked, handle stays open.

## Impact

Each occurrence permanently consumes one of the `PIPE_INSTANZEN = MAX_VERBINDUNGEN + 2 = 98`
instances configured in `CreateNamedPipeW` (server_v3.rs:399), plus one leaked
thread, one leaked `Ereignis` event handle and one leaked `EndeSignal`. The
leaked instances are **not** bounded by the worker cap: `WorkerPlatz` is
released when the connection thread exits, not when the detached writer exits.

Once enough instances are stranded, `naechste_instanz` (server_v3.rs:1438-1481)
receives `ERROR_PIPE_BUSY` on every `CreateNamedPipeW`, only increments
`acceptor_wartet_auf_instanz` and loops forever — the broker accepts no further
connections at all. Because the process only self-terminates after 60 s with
zero active workers (`BROKER_IDLE_ENDE_MS`, lib.rs:39), an attacker holding a
single legitimate connection keeps the broker alive in that wedged state
indefinitely: a persistent local denial of service against every probe/plugin
instance that needs the broker.

## Mitigations checked

- `// SAFETY:` on `HandleRegister` claims exactly the invariant that is broken
  here ("Das Register haelt sie nur, solange der besitzende Thread seinen
  `Verbindungsgriff` noch nicht fallen gelassen hat"); the claim is not upheld
  on the detach path.
- `io_abbrechen` is retried every 2 ms during the 2 s `SENKE_FRIST`
  (`join_mit_frist`), which makes the detach uncommon but does not remove it —
  the authors themselves document a write "den `CancelIoEx` nicht loest" as the
  reason the detach path exists, and ship a counter for it.
- `debug_assertions` are stripped in release; no assertion guards this.
- No MIRI/sanitizer coverage on this path; `tests/security_vectors.rs` covers
  listener-name ownership and the impersonation chain, not writer detachment.
- `V3Griff::stoppen()` does not join detached writers (they are not in
  `verbindungen`), so shutdown does not repair the leak either.

## Recommendation

Keep the handle reachable for as long as any owner survives. Move the
`HandleEintrag` into the same ownership unit as the `Verbindungsgriff` (e.g.
store the register entry inside `Verbindungsgriff` so deregistration happens in
`Verbindungsgriff::drop`, after the last `Arc` goes), or hand the register entry
to the writer thread when `join_mit_frist` reports a detach so
`alle_io_abbrechen`/`stoppen()` can still cancel it. Additionally, refuse to
return from `verbindung_bedienen` with a detached writer without either
(a) escalating to `DisconnectNamedPipe` on the handle before deregistering — which
forcibly completes the peer-blocked write — or (b) parking the detached
`JoinHandle` in a process-wide list that `V3Griff::stoppen()` drains.
