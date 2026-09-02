---
id: RAWFD-002
bug_class: raw-fd-lifecycle
title: Thread-spawn failure closes the connection HANDLE before deregistering it, leaving CancelIoEx to hit a recycled handle
location: src/transport/server_v3.rs:1979
function: v3_server_starten_intern
confidence: Medium
worker: worker-19
fp_verdict: LIKELY_TP
fp_rationale: "Same defect as OPAQUEPTR-001 (dedup left it unmerged only because worker-19 spells the path src/... instead of broker/src/...); the capture order griff-before-handle_eintrag at server_v3.rs:1939-1979 and the bare-isize CancelIoEx sinks were verified in source."
severity: MEDIUM
attack_vector: Local
exploitability: Difficult
severity_rationale: "Identical severity to OPAQUEPTR-001 by construction — one defect, four filings: CloseHandle before deregistration leaves a recycled handle value cancellable, giving a local unprivileged caller cross-connection I/O aborts and a fail-closed v3 teardown. Not an independent bug; count it once."
---

## Description

In the acceptor loop the freshly accepted pipe handle is published to the
cancel register *before* the connection thread exists, and the register entry
(`HandleEintrag`) plus the owning `Arc<Verbindungsgriff>` are both moved into
the spawn closure. The comment states the intent for the failure case:

```rust
Err(_) => {
    // Closure-Drop gibt Workerplatz, Handle und Register
    // frei. Beide Listener sind bereits wieder bewaffnet.
}
```

but the closure does not control the order in which those two are released.
Closure captures are stored as struct fields in capture order (first mention in
the closure body) and dropped in field-declaration order. In the closure body
`griff` is mentioned at server_v3.rs:1953 and `handle_eintrag` at
server_v3.rs:1969, so on `spawn` failure the drop sequence is:

1. `griff: Arc<Verbindungsgriff>` -> `Verbindungsgriff::drop` -> `CloseHandle(h)`
   (server_v3.rs:675-681)
2. ... the remaining captures ...
3. `handle_eintrag: HandleEintrag` -> takes the register mutex and removes
   `(id, h)` (server_v3.rs:2226-2232)

Between step 1 and step 3 the register contains an **already-closed** Windows
`HANDLE` value. Windows recycles handle values immediately and aggressively
within a process, and this broker allocates handles constantly on other threads
(`ListenerInstanz::neu` -> `CreateNamedPipeW`, `Ereignis::neu`/`EndeSignal::neu`
-> `CreateEventW`, SQLite file handles, thread handles). Any concurrent
`alle_io_abbrechen` therefore issues `CancelIoEx` against a handle number that
may already name an unrelated kernel object.

This is precisely the invariant the register documents and relies on:

```rust
// Das Register haelt sie nur, solange der besitzende Thread seinen
// `Verbindungsgriff` noch nicht fallen gelassen hat; Eintragen und Austragen
// laufen unter demselben Mutex wie das Abbrechen.
unsafe impl Send for HandleRegister {}
```

The mutex only serializes register access; it does not order `CloseHandle`
against deregistration, because `CloseHandle` happens outside the lock.

Note that the normal path is safe: in `verbindung_bedienen` the register entry
is a body-local (`let _handle_eintrag = handle_eintrag;`, server_v3.rs:2275) and
`griff` is a parameter, and Rust drops body locals *before* parameters — so
there the order is deregister-then-close. Only the spawn-failure path inverts it.

## Code

```rust
// src/transport/server_v3.rs:1897-1983 (abridged) — acceptor handover
let verbundenes_handle = angenommen.handle_uebernehmen();
let griff = Arc::new(Verbindungsgriff {
    h: verbundenes_handle,
    sicherheits_spur: sicherheits_spur2.clone(),
});

// Das Handle geht ins Abbruchregister, BEVOR der Thread existiert.
{
    let mut r = handles2.lock().unwrap_or_else(|e| e.into_inner());
    r.offen.push((id, verbundenes_handle as isize));
}
// Der Eintrag gehoert ab hier dem Thread; scheitert `spawn`,
// faellt die Closure samt Eintrag und traegt ihn wieder aus.
let handle_eintrag = HandleEintrag { id, register: handles2.clone() };
...
match std::thread::Builder::new()
    .name("eqcop-v3-conn".into())
    .spawn(move || {
        ...
        verbindung_bedienen(
            id, griff, senke, ... , sicherheits_spur,
            handle_eintrag,          // captured AFTER `griff`
        );
    }) {
    Ok(j) => { ... }
    Err(_) => {
        // Closure-Drop gibt Workerplatz, Handle und Register
        // frei. Beide Listener sind bereits wieder bewaffnet.
        //  -> `griff` drops first: CloseHandle(h)
        //  -> `handle_eintrag` drops later: register.retain(...)
    }
}
```

```rust
// src/transport/server_v3.rs:1283-1293 — the concurrent reader of the register
fn alle_io_abbrechen(handles: &Arc<Mutex<HandleRegister>>) {
    if let Ok(reg) = handles.lock() {
        for (_, h) in reg.offen.iter() {
            // SAFETY: der Eintrag lebt nur, solange der besitzende Thread sein
            // Handle haelt; Austragen und Abbrechen laufen unter diesem Mutex.
            unsafe { CancelIoEx(*h as HANDLE, std::ptr::null_mut()); }
        }
    }
}
```

## Data flow

- **Source:** a local unprivileged process drives the broker to its concurrency
  ceiling (up to `MAX_VERBINDUNGEN = 96` connections, each spawning a connection
  thread, a writer thread and two ingress consumer threads, plus short-lived
  `Senkenruf` threads), making `std::thread::Builder::spawn` fail with a
  resource error at server_v3.rs:1979.
- **Sink:** `CancelIoEx(*h as HANDLE, null)` in `alle_io_abbrechen`
  (server_v3.rs:1288) / `io_abbrechen` (server_v3.rs:1300) executed against the
  stale `(id, h)` entry that `griff`'s `CloseHandle` has already invalidated.
- **Validation:** none. The register stores a bare `isize`; there is no
  generation counter, no re-validation and no ordering constraint between
  `CloseHandle` and the `retain` that removes the entry.

## Reachability trace

`main` -> `broker_starten` -> `v3_server_starten_intern` -> acceptor thread
`eqcop-v3-acceptor` -> `worker_reservieren` OK -> `naechste_instanz` OK ->
`handles2.offen.push((id, h))` -> `thread::Builder::spawn` returns `Err` ->
closure drop -> `Verbindungsgriff::drop` -> `CloseHandle(h)` ... concurrently
`main` -> `broker_geordnet_stoppen` -> `V3Griff::stoppen` ->
`alle_io_abbrechen` (also re-run every 5 ms inside the join loop at
server_v3.rs:1250-1252) -> `CancelIoEx` on the closed/recycled handle ->
`HandleEintrag::drop` finally removes the entry.

## Impact

Operating on a released kernel handle (CWE-672). Because Windows reuses handle
values immediately, the `CancelIoEx` can land on an unrelated object opened in
the meantime by another broker thread — for example another client's connected
pipe instance (aborting that client's in-flight read/write with
`ERROR_OPERATION_ABORTED` and tearing its session down), a listener instance's
pending `ConnectNamedPipe`, or the SQLite database/WAL file handle held by the
`nakama-store-writer` thread. A local unprivileged attacker who can push the
process into thread-spawn failure while a shutdown or a fatal-listener cancel
sweep is running gets cross-connection I/O aborts on sessions it does not own.
The window is short but it is re-entered on every failed spawn, and
`alle_io_abbrechen` is executed in a tight 5 ms loop during `stoppen()`.

## Mitigations checked

- `HandleRegister`'s `// SAFETY:` comment asserts the exact invariant that this
  path breaks; it is a claim, not an enforcement.
- The register mutex is held across `CancelIoEx` and across the `retain`, but
  `CloseHandle` runs outside it in `Verbindungsgriff::drop`, so the lock gives
  no ordering guarantee here.
- `Err(_)` arm carries an explanatory comment asserting the cleanup is correct;
  it does not `drop(handle_eintrag)` explicitly and therefore relies on
  unstated closure-capture drop order.
- No `debug_assert!`, no MIRI, no sanitizer on this path. `tests/security_vectors.rs`
  covers handle/thread/name cleanup on *listener start* failure paths, not on
  connection-thread spawn failure.

## Recommendation

Make the ordering explicit and ownership single. Minimal fix at the `Err(_)`
arm: recover the closure's state and drop the register entry before the handle,
e.g. by not moving `handle_eintrag` into the closure but keeping it in an
`Option` the acceptor owns, or by an explicit
`drop(handle_eintrag); drop(griff);` sequence on the failure path. The robust
fix is to make deregistration part of `Verbindungsgriff::drop` itself (store the
`(id, Arc<Mutex<HandleRegister>>)` inside `Verbindungsgriff` and `retain` before
`CloseHandle`), so that no ordering can ever place a closed handle in the
register regardless of how the value is dropped.
