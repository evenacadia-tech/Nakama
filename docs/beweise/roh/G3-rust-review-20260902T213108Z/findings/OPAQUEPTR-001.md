---
id: OPAQUEPTR-001
bug_class: opaque-pointer
title: Pipe HANDLE is closed before its entry leaves the CancelIoEx register on the thread-spawn failure path
location: broker/src/transport/server_v3.rs:1979
function: v3_server_starten_intern
confidence: Medium
worker: worker-16
also_known_as: [UAF-001]
locations:
  - broker/src/transport/server_v3.rs:1979
fp_verdict: LIKELY_TP
fp_rationale: "Both framings hold in source: the closure at server_v3.rs:1939-1979 captures griff (CloseHandle in Drop, :678) before handle_eintrag (register retain in Drop, :2229), closure-capture drop order is unspecified, and alle_io_abbrechen/io_abbrechen/watchdog read the bare isize with CancelIoEx under a mutex that never covers CloseHandle; reachability rests on forcing thread::Builder::spawn to Err, which I could not confirm as reliably forcible."
severity: MEDIUM
attack_vector: Local
exploitability: Difficult
severity_rationale: "Carrying framing is the use-after-close of a kernel handle (absorbed UAF-001): a recycled HANDLE value can take a CancelIoEx aimed at a listener's ConnectNamedPipe, another probe's read or a store file handle, i.e. local DoS of authenticated sessions from a connection rejected before authentication; reduced from the race adjustment, not reduced for the // SAFETY: comment because that comment states exactly the broken invariant. Same defect as RAWFD-002, UNSAFESYNC-001 and SAFETYDOC-002, which dedup left unmerged only because of path-prefix drift."
---

## Description

The v3 acceptor publishes every accepted pipe `HANDLE` into a process-wide
cancel register (`HandleRegister.offen: Vec<(u64, isize)>`) *before* the
worker thread that owns it exists (`server_v3.rs:1911-1914`). Two owners of
that same kernel handle are then moved into the worker closure:

* `griff: Arc<Verbindungsgriff>` — its `Drop` calls `CloseHandle(self.h)`
  (`server_v3.rs:675-681`), and
* `handle_eintrag: HandleEintrag` — its `Drop` removes `(id, h)` from the
  register (`server_v3.rs:2226-2233`).

Inside the worker (`verbindung_bedienen`) the ordering is sound: the register
entry is a body local (`let _handle_eintrag = handle_eintrag;`,
`server_v3.rs:2275`) and therefore drops *before* the `griff` function
parameter — unregister first, close second.

On the **spawn-failure** arm (`server_v3.rs:1979-1982`) that guarantee is
gone. The closure is never called; `std::thread::Builder::spawn` drops it, and
the two owners are dropped as *closure captures*. Rust leaves the drop order of
closure captures unspecified; rustc drops them in capture order, which here is
`griff` (captured at the `verbindung_bedienen(id, griff, ...)` call as
argument 2) **before** `handle_eintrag` (argument 19, captured last). So
`CloseHandle(h)` runs while `(id, h)` is still in the register.

During that window the register violates its own documented invariant ("der
Eintrag lebt nur, solange der besitzende Thread sein Handle haelt",
`server_v3.rs:1285-1287`), and `alle_io_abbrechen()` dereferences the stale
value with `CancelIoEx(*h as HANDLE, ...)`. Windows recycles handle *values*
aggressively, and this broker allocates new kernel objects on the very next
loop iteration (`CreateNamedPipeW` in `naechste_instanz`, `CreateEventW` in
`Ereignis::neu`, SQLite file handles in the store thread), so the recycled
value is likely to name a *different, live* object by the time the cancel is
issued.

## Code

```rust
// broker/src/transport/server_v3.rs:1897-1920
let verbundenes_handle = angenommen.handle_uebernehmen();
let griff = Arc::new(Verbindungsgriff {
    h: verbundenes_handle,
    sicherheits_spur: sicherheits_spur2.clone(),
});
// Das Handle geht ins Abbruchregister, BEVOR der Thread existiert. ...
{
    let mut r = handles2.lock().unwrap_or_else(|e| e.into_inner());
    r.offen.push((id, verbundenes_handle as isize));
}
// Der Eintrag gehoert ab hier dem Thread; scheitert `spawn`,
// faellt die Closure samt Eintrag und traegt ihn wieder aus.
let handle_eintrag = HandleEintrag { id, register: handles2.clone() };

// broker/src/transport/server_v3.rs:1971-1983
}) {
    Ok(j) => { /* ... */ }
    Err(_) => {
        // Closure-Drop gibt Workerplatz, Handle und Register
        // frei. Beide Listener sind bereits wieder bewaffnet.
    }
}
```

The comment asserts an ordering the language does not provide. The consumer of
the stale entry:

```rust
// broker/src/transport/server_v3.rs:1283-1293
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

- **Source:** a local unprivileged process connects to the v3 named pipe, is
  accepted at `server_v3.rs:1897`, and its handle is published to the register
  at `server_v3.rs:1913`; the attacker additionally drives the process toward
  thread/commit exhaustion so `thread::Builder::spawn` fails.
- **Sink:** `CancelIoEx(*h as HANDLE, null_mut())` at `server_v3.rs:1289`
  (also `server_v3.rs:1301` and the watchdog at `server_v3.rs:1740`), reading
  a handle value already passed to `CloseHandle` at `server_v3.rs:678`.
- **Validation:** none — the register stores a bare `isize`, is never nulled
  nor `take()`-n on close, and neither `alle_io_abbrechen` nor `io_abbrechen`
  re-validates the handle.

## Reachability trace

`v3_server_starten_intern` -> acceptor thread closure -> `WaitForMultipleObjects`
-> `angenommen.handle_uebernehmen()` -> register push -> `thread::Builder::spawn`
returns `Err` -> closure dropped -> `Verbindungsgriff::drop` -> `CloseHandle(h)`
-> *(register still holds h)* -> concurrent `V3Griff::stoppen()` ->
`alle_io_abbrechen` -> `CancelIoEx(stale h)`.

The concurrent reader is real: `V3Griff::stoppen()` calls
`alle_io_abbrechen(&self.handles)` (`server_v3.rs:1232`) *before* it joins the
acceptor thread (`server_v3.rs:1233-1235`), so it runs on a different thread
than the closure drop. `V3Griff::drop` invokes the same path on every broker
shutdown.

## Impact

A cancel issued against a recycled handle value aborts I/O on an unrelated
kernel object owned by the broker. The most likely victim is a listener
instance with a pending `ConnectNamedPipe`: `verbindung_fertig()` then returns
false, which the acceptor treats as fatal — `listener_fehler` is incremented,
`stop` is set, and the whole v3 endpoint is torn down fail-closed
(`server_v3.rs:1815-1822`). A store or WAL file handle is likewise in the
recycle pool. Net effect for a local unprivileged attacker who can push the
process into spawn failure: forced shutdown / aborted I/O of the control and
telemetry endpoint, i.e. denial of service against every probe instance, with
no authentication required (the handle is registered *before* the
impersonation/SID check ever runs). Because the register holds a bare integer
with no validity guard, the defect is also a latent use-after-close that any
future change to the cleanup order can widen.

## Mitigations checked

- The `// SAFETY:` comment at `server_v3.rs:1285-1287` names exactly the
  invariant that is violated here ("der Eintrag lebt nur, solange der
  besitzende Thread sein Handle haelt") — it is not upheld on the
  spawn-failure arm, so the comment is documentation, not proof.
- The register mutex serializes cancel against unregister, but not against
  `CloseHandle`, which happens outside the lock.
- NAK-123 `tests/security_vectors.rs` injects listener/watchdog/acceptor spawn
  failures (`V3StartTestFehler::WachhundSpawn`, `::AcceptorSpawn`) but has
  **no** seam for a failing *worker* spawn, so this arm is untested.
- No `#![deny(unsafe_op_in_unsafe_fn)]`, no `[lints]` table, no MIRI and no
  Application Verifier run on this path (MIRI cannot model Win32; Application
  Verifier handle checks would catch it).
- No `debug_assert!` guards the register contents.

## Recommendation

Do not depend on closure-capture drop order. Remove the register entry
explicitly before the handle can be closed, and make the stale handle
unrepresentable:

1. In the `Err(_)` arm, unregister first — keep the `HandleEintrag` outside the
   closure and hand the worker only what it needs, or call
   `handles2.lock().offen.retain(|(i, _)| *i != id)` in that arm before the
   closure is released.
2. Better, make the invariant structural: move the register entry *into*
   `Verbindungsgriff` so that `Verbindungsgriff::drop` unregisters and only
   then calls `CloseHandle`, which makes close-before-unregister impossible to
   express.
3. Store the handle as `Option<OwnedHandle>` / `Option<NonNull<c_void>>` in the
   register and `take()` it on close, so a stale value can never reach
   `CancelIoEx`.
