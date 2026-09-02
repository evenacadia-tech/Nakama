---
id: UNSAFESYNC-001
bug_class: unsafe-sync-impl
title: "`unsafe impl Send for HandleRegister` asserts an ownership invariant that the acceptor's spawn-failure path breaks: raw pipe handles stay cancellable after CloseHandle"
location: src/transport/server_v3.rs:975
function: (file-level)
confidence: Medium
worker: worker-5
fp_verdict: LIKELY_TP
fp_rationale: "Same defect as OPAQUEPTR-001 seen from the unsafe impl: HandleRegister launders a !Send HANDLE into an isize (server_v3.rs:1913) and the manual Send impl at :975 justifies it with an ownership invariant the spawn-failure arm at :1979 breaks; dedup kept it separate only because the finding is file-level and uses the src/... path spelling."
severity: MEDIUM
attack_vector: Local
exploitability: Difficult
severity_rationale: "Severity taken from the carrying framing (the handle use-after-close), not from the unsafe-impl label: the manual Send is what permits cross-thread sharing of the raw handle at all, and its stated justification is untrue on a reachable path. One defect with OPAQUEPTR-001/RAWFD-002/SAFETYDOC-002; count it once."
---

## Description

`HandleRegister` is the cross-thread cancellation register of the v3 pipe
server. It holds live connection handles so that `V3Griff::stoppen()`, the
watchdog thread and `V3Closer` can break blocking reads with `CancelIoEx`.

The raw `HANDLE` (`*mut c_void`, therefore `!Send`) is deliberately laundered
into an `isize` (`r.offen.push((id, verbundenes_handle as isize))`, line 1913)
and reconstituted as a pointer in *other* threads (`*h as HANDLE`, lines 1288,
1300, 1741). The manual `unsafe impl Send for HandleRegister {}` at line 975
documents the invariant that makes this sound:

> "Das Register haelt sie nur, solange der besitzende Thread seinen
> `Verbindungsgriff` noch nicht fallen gelassen hat"
> (the register holds them only as long as the owning thread has not yet
> dropped its `Verbindungsgriff`)

That invariant is **not enforced by the type system and is violated on one
path**: when `thread::Builder::spawn` for a connection worker fails, the
closure — which owns *both* the `Arc<Verbindungsgriff>` (whose `Drop` calls
`CloseHandle`, line 674) and the `HandleEintrag` (whose `Drop` removes the
register entry, line 2227) — is dropped inside `spawn`. The Rust Reference
(destructors, shipped with the local 1.93 toolchain) states verbatim:

> "The variables that a closure captures by move are dropped in an
> **unspecified** order."

So the release order of "close the handle" versus "remove it from the register"
is not defined by the language at all; with the current rustc capture order
(captures are laid out in first-use order, and `griff` is used before
`handle_eintrag` in the call to `verbindung_bedienen`) the handle is closed
**first** and deregistered afterwards. During that window a concurrent
`alle_io_abbrechen()` / watchdog pass calls `CancelIoEx()` on a handle value
that no longer belongs to this connection — and this very acceptor loop
allocates fresh pipe handles on every iteration (`naechste_instanz`), so the
Windows handle-table slot is a prime candidate for immediate reuse.

The normal (non-failure) path is correct and clearly deliberate:
`verbindung_bedienen` re-binds the entry to a *body local*
(`let _handle_eintrag = handle_eintrag;`, line 2271) which is dropped before
the `griff` function parameter, giving "deregister, then close". Only the
error arm at line 1979 relies on the unspecified closure-capture order.

## Code

```rust
// src/transport/server_v3.rs:965-975
#[derive(Default)]
struct HandleRegister {
    offen: Vec<(u64, isize)>,
}

// SAFETY: Win32-HANDLEs sind prozessweite Kernel-Referenzen ohne Thread-
// Affinitaet. Das Register haelt sie nur, solange der besitzende Thread seinen
// `Verbindungsgriff` noch nicht fallen gelassen hat; Eintragen und Austragen
// laufen unter demselben Mutex wie das Abbrechen.
unsafe impl Send for HandleRegister {}

// src/transport/server_v3.rs:1897-1920 (acceptor thread)
let verbundenes_handle = angenommen.handle_uebernehmen();
let griff = Arc::new(Verbindungsgriff {
    h: verbundenes_handle,                 // Drop => CloseHandle (line 674)
    sicherheits_spur: sicherheits_spur2.clone(),
});
{
    let mut r = handles2.lock().unwrap_or_else(|e| e.into_inner());
    r.offen.push((id, verbundenes_handle as isize));   // !Send laundered as isize
}
// Der Eintrag gehoert ab hier dem Thread; scheitert `spawn`,
// faellt die Closure samt Eintrag und traegt ihn wieder aus.
let handle_eintrag = HandleEintrag { id, register: handles2.clone() };

// src/transport/server_v3.rs:1939-1982
match std::thread::Builder::new()
    .name("eqcop-v3-conn".into())
    .spawn(move || {
        let _worker_platz = worker_platz;
        // ...
        verbindung_bedienen(id, griff, /* ... */, handle_eintrag);
    }) {
    Ok(j) => { /* ... */ }
    Err(_) => {
        // Closure-Drop gibt Workerplatz, Handle und Register
        // frei. Beide Listener sind bereits wieder bewaffnet.
    }   // <-- drop order of `griff` vs `handle_eintrag` is UNSPECIFIED
}

// src/transport/server_v3.rs:1283-1292 — the sink, on other threads
fn alle_io_abbrechen(handles: &Arc<Mutex<HandleRegister>>) {
    if let Ok(reg) = handles.lock() {
        for (_, h) in reg.offen.iter() {
            // SAFETY: der Eintrag lebt nur, solange der besitzende Thread sein
            // Handle haelt; Austragen und Abbrechen laufen unter diesem Mutex.
            unsafe {
                CancelIoEx(*h as HANDLE, std::ptr::null_mut());
            }
        }
    }
}
```

## Data flow

- **Source:** any local unprivileged process that can open the v3 pipe. It
  controls the connection rate and therefore the thread pressure on the broker
  (`MAX_VERBINDUNGEN = 96` workers, each spawning a writer plus two ingress
  consumer threads, plus one short-lived `eqcop-v3-senkenruf` thread per sink
  call) — i.e. it can drive `std::thread::Builder::spawn` at line 1939 into
  `Err`, which is the only trigger for the affected arm.
- **Sink:** `CancelIoEx(*h as HANDLE, null)` in `alle_io_abbrechen`
  (src/transport/server_v3.rs:1288), `io_abbrechen`
  (src/transport/server_v3.rs:1300) and the watchdog thread
  (src/transport/server_v3.rs:1741) — all executed on threads other than the
  one that closed the handle.
- **Validation:** none. The register stores only `(u64 id, isize handle)`; it
  holds no reference to the owning `Arc<Verbindungsgriff>` and there is no
  happens-before edge between `Verbindungsgriff::drop` (CloseHandle) and
  `HandleEintrag::drop` (register removal) on this path.

## Reachability trace

`v3_server_starten_intern` -> `eqcop-v3-acceptor` thread loop ->
`ListenerInstanz::handle_uebernehmen` -> register push (1913) ->
`thread::Builder::spawn(...) == Err` (1979) -> closure drop ->
`Arc<Verbindungsgriff>::drop` -> `CloseHandle` — *concurrently with* —
`V3Griff::stoppen` -> `alle_io_abbrechen` -> `CancelIoEx(*h)` (1288) or
`eqcop-v3-wachhund` -> `CancelIoEx(*h)` (1741), before
`HandleEintrag::drop` -> `register.offen.retain(...)` (2227) runs.

## Impact

Within the window the broker calls `CancelIoEx` on a handle value it no longer
owns. If the Windows handle-table slot has already been reused — and this
acceptor loop creates a new named-pipe instance per accepted connection, so
reuse of a just-freed slot is the common case, not an exotic one — the
cancellation lands on an unrelated object of the same process: another probe's
live pipe connection (its pending `ReadFile` returns `ERROR_OPERATION_ABORTED`,
which `ov_lesen` maps to `IoAusgang::Abgebrochen` and the connection is torn
down), a listener instance's pending `ConnectNamedPipe`, or the SQLite store
file handle. The observable effect is denial of service against *other*,
authenticated probe sessions triggered by an unprivileged local process that
never has to pass the SID/duplex handshake — the register entry it corrupts
belongs to a connection that was rejected before any authentication.

More fundamentally, the `unsafe impl Send` at line 975 is what allows a raw OS
handle to be shared across threads at all; its documented justification is
untrue for this path, so the "SAFETY" comment cannot be relied on by a future
reader who adds another `CancelIoEx`/`CloseHandle` user of the register.

## Mitigations checked

- `// SAFETY:` comments are present at both the impl (968-974) and the sink
  (1285-1287) and *both* state exactly the invariant that this path breaks —
  documentation, not enforcement (the register stores `isize`, so the compiler
  checks nothing).
- The happy path is correctly ordered (`_handle_eintrag` body local at 2271
  drops before the `griff` parameter), so no existing test can expose this.
- `broker/tests/security_vectors.rs` (NAK-123) covers listener/watchdog/
  acceptor start failures via `V3StartTestFehler` and the whole impersonation
  chain, but there is **no** injected failure for the connection-worker
  `spawn` — the arm at line 1979 has no test coverage at all.
- No MIRI run (and Win32 FFI could not run under MIRI anyway), no
  `rust-toolchain.toml` pinning a compiler, no `[lints]` table — nothing pins
  or checks the capture-drop order the code depends on.
- Registration deliberately happens *before* the spawn (comment at 1904-1911,
  T2 finding 6 of 2026-08-29), so simply moving the push into the thread would
  reintroduce the fixed bug and is not an available mitigation.

## Recommendation

Stop relying on closure-capture drop order and make the register entry
outlive-safe by construction: give `HandleEintrag` ownership of the handle's
keeper, e.g.

```rust
struct HandleEintrag {
    id: u64,
    register: Arc<Mutex<HandleRegister>>,
    /// Haelt das Handle offen, bis der Eintrag ausgetragen ist.
    _griff: Arc<Verbindungsgriff>,
}
```

and pass `griff.clone()` into it. `Drop for HandleEintrag` then removes the
entry *before* releasing its `Arc`, so no drop-order assumption is needed on
any path (spawn failure, normal return, or panic unwind), and the `// SAFETY:`
comment at line 975 becomes an invariant the type actually enforces.
Alternatively store `Arc<Verbindungsgriff>` (not `isize`) in
`HandleRegister::offen` and drop the manual `unsafe impl Send` entirely — the
struct is then auto-`Send` and the compiler, not a comment, guarantees that a
registered handle cannot be closed while it is still cancellable.
