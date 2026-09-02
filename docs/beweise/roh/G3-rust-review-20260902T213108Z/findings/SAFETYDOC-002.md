---
id: SAFETYDOC-002
bug_class: safety-doc
title: "`CancelIoEx` SAFETY invariant \"entry lives only while the owning thread holds its handle\" is broken on the acceptor's spawn-failure path"
location: broker/src/transport/server_v3.rs:1289
function: alle_io_abbrechen
confidence: Medium
worker: worker-1
fp_verdict: LIKELY_TP
fp_rationale: "Same defect as OPAQUEPTR-001 seen from the sink: the // SAFETY: at server_v3.rs:1286 asserts 'der Eintrag lebt nur, solange der besitzende Thread sein Handle haelt', and the spawn-failure arm at :1979 breaks exactly that half while the mutex half holds; dedup left it unmerged because the bucket key is the sink function, not the acceptor."
severity: MEDIUM
attack_vector: Local
exploitability: Difficult
severity_rationale: "Carrying framing is the handle use-after-close, so this inherits OPAQUEPTR-001's tier rather than a documentation-only LOW: the SAFETY comment is not pro-forma but actively false on a reachable path, which is the no-reduction case. One defect with OPAQUEPTR-001/RAWFD-002/UNSAFESYNC-001; count it once."
---

## Description

`alle_io_abbrechen` (and its per-id twin `io_abbrechen`, line 1295) call
`CancelIoEx` on raw `HANDLE` values stored as `isize` in the shared
`HandleRegister`. The `// SAFETY:` comment at line 1286 names the invariant
that makes this sound:

> der Eintrag lebt nur, solange der besitzende Thread sein Handle haelt;
> Austragen und Abbrechen laufen unter diesem Mutex.
> *("the entry only lives as long as the owning thread holds its handle;
> de-registration and cancellation run under this mutex")*

The second half holds — `HandleEintrag::drop` (line 2226) and both cancel
functions take the same `Mutex<HandleRegister>`. **The first half does not hold
on the acceptor's `spawn`-failure path.**

The acceptor registers the accepted pipe handle at line 1913, *before* the
worker thread exists, then hands both the `Arc<Verbindungsgriff>` (which owns
the handle and closes it in `Drop`, line 675) and the `HandleEintrag` (which
de-registers in `Drop`, line 2226) to the connection closure. If
`Builder::spawn` fails, the closure is dropped and the code relies on that drop
to undo both actions:

```rust
Err(_) => {
    // Closure-Drop gibt Workerplatz, Handle und Register frei.
    // Beide Listener sind bereits wieder bewaffnet.
}
```

The correctness of that undo depends on the **relative drop order of two
captured variables**, which the language does not specify. The Rust Reference
(`types/closure.html`, "Drop order") only defines when *disjoint fields of one
captured composite* are dropped; it says nothing that pins the order of `griff`
against `handle_eintrag`. `rustc` lowers `move`-closure captures to struct
fields in capture order and drops struct fields in declaration order, and in
this closure `griff` is captured (line 1952) well before `handle_eintrag`
(line 1969). Under that lowering the sequence is:

1. `griff: Arc<Verbindungsgriff>` drops. Refcount is 1 here — no reader/writer
   thread was ever created — so `Verbindungsgriff::drop` runs
   `unsafe { CloseHandle(self.h) }` (line 678).
2. …the handle value is now closed but **still present in `HandleRegister`**…
3. `handle_eintrag: HandleEintrag` drops and finally runs
   `r.offen.retain(|(i, _)| *i != self.id)` (line 2229).

Between (1) and (3) any other thread that takes the register mutex —
`V3Griff::stoppen` → `alle_io_abbrechen`, the watchdog's bootstrap-deadline
`io_abbrechen`, or a writer thread's `io_abbrechen(&handles, id)` (line 2626) —
passes a closed handle value to `CancelIoEx`. Windows recycles handle values
aggressively within a process, so the value may already name a different kernel
object (another pipe instance created by `naechste_instanz` at line 1862, a
SQLite file handle from `store.rs`, an event, a thread) whose in-flight I/O is
then cancelled.

The normal (non-failing) path is ordered correctly and is *not* affected:
`verbindung_bedienen` rebinds the entry as its first statement
(`let _handle_eintrag = handle_eintrag;`, line 2275), so the local is dropped
before the `griff` parameter, i.e. de-registration strictly precedes
`CloseHandle`. The defect is that this ordering discipline is explicit in the
function body but only accidental in the closure.

## Code

```rust
// broker/src/transport/server_v3.rs:1283
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

```rust
// broker/src/transport/server_v3.rs:1897 — register first, then hand ownership to a closure
let verbundenes_handle = angenommen.handle_uebernehmen();
let griff = Arc::new(Verbindungsgriff { h: verbundenes_handle, sicherheits_spur: … });
{
    let mut r = handles2.lock().unwrap_or_else(|e| e.into_inner());
    r.offen.push((id, verbundenes_handle as isize));
}
// Der Eintrag gehoert ab hier dem Thread; scheitert `spawn`,
// faellt die Closure samt Eintrag und traegt ihn wieder aus.
let handle_eintrag = HandleEintrag { id, register: handles2.clone() };
…
match std::thread::Builder::new().name("eqcop-v3-conn".into()).spawn(move || {
        let _worker_platz = worker_platz;
        …
        verbindung_bedienen(id, griff, …, handle_eintrag);   // griff captured before handle_eintrag
    }) {
    Ok(j) => { … }
    Err(_) => {
        // Closure-Drop gibt Workerplatz, Handle und Register frei.
        // Beide Listener sind bereits wieder bewaffnet.
    }
}
```

```rust
// broker/src/transport/server_v3.rs:675 — the close, and 2226 — the de-registration
impl Drop for Verbindungsgriff {
    fn drop(&mut self) {
        // SAFETY: exklusiver Besitz ueber den Arc, genau einmal geschlossen.
        unsafe { CloseHandle(self.h) };
        self.sicherheits_spur.push("close");
    }
}

impl Drop for HandleEintrag {
    fn drop(&mut self) {
        if let Ok(mut r) = self.register.lock() {
            r.offen.retain(|(i, _)| *i != self.id);
        }
    }
}
```

## Data flow

- **Source:** a local unprivileged process connecting to the broker pipe drives
  the acceptor to `Builder::spawn`; the failure itself is triggered by thread /
  handle exhaustion, which the same adversary can approach by holding
  connections open up to `MAX_VERBINDUNGEN` while the process is under memory
  pressure.
- **Sink:** `unsafe { CancelIoEx(*h as HANDLE, null_mut()) }` at
  `broker/src/transport/server_v3.rs:1289` (and the identical call at 1301).
- **Validation:** none at the sink — the register is trusted to contain only
  open handles. The `HandleRegister` stores bare `isize` values, so there is no
  type-level ownership link back to the `Arc<Verbindungsgriff>` that closes them.

## Reachability trace

`v3_server_starten` → acceptor thread (line 1771) → `r.offen.push((id, h))`
(line 1913) → `Builder::spawn(...)` returns `Err` (line 1979) → closure drop →
`Verbindungsgriff::drop` → `CloseHandle` (line 678) → *window* →
`HandleEintrag::drop` → `retain` (line 2229).

Concurrently: `V3Griff::stoppen` (line 1226) or the watchdog thread →
`alle_io_abbrechen` / `io_abbrechen` → `CancelIoEx(<closed handle>)` (line 1289
/ 1301).

## Impact

The `// SAFETY:` justification printed at the `CancelIoEx` site is false on one
reachable path, so the operation runs with no upheld contract. Concretely, a
closed handle value that has been recycled by the kernel can receive a
`CancelIoEx`, aborting I/O on an unrelated object owned by the same process —
in this crate plausibly a freshly created listener instance (`naechste_instanz`
runs on the very next loop iteration and creates new pipe handles) or a
SQLite/WAL file handle. Effect ranges from a spurious
`ERROR_OPERATION_ABORTED` on an unrelated pipe (dropped connection, listener
restart) to an aborted store write. The window is short and requires
`Builder::spawn` to fail, so exploitation is opportunistic rather than
reliable; the audit defect — a documented invariant that is not enforced by
construction — is unconditional.

## Mitigations checked

- `// SAFETY:` comment present: yes, at line 1286 — but the invariant it names
  is not upheld on the `spawn`-failure path, so it is a proof-shaped comment
  rather than a proof.
- Mutex discipline: correct — cancellation and de-registration share
  `Arc<Mutex<HandleRegister>>`; the mutex does **not** cover `CloseHandle`,
  which is the gap.
- Explicit ordering elsewhere: `verbindung_bedienen` line 2275 deliberately
  rebinds `handle_eintrag` as the first local so it outlives the `griff`
  parameter; that discipline is absent in the closure.
- `debug_assert!`: none on this path.
- MIRI / sanitizers / Application Verifier handle checks: not run; the
  `tools/beweise.ps1` proof legs do not include a handle-verifier leg, and the
  NAK-123 `tests/security_vectors.rs` cases cover start-failure handle cleanup
  and process-kill name release, but not the acceptor `spawn`-failure ordering.
- `#![deny(unsafe_op_in_unsafe_fn)]` / `[lints]`: absent from
  `broker/Cargo.toml` and `broker/src/lib.rs`.

## Recommendation

Make the ordering structural instead of relying on closure capture layout:

1. Give `Verbindungsgriff` the register entry, so one owner performs
   de-registration and close in a guaranteed order — move the
   `r.offen.retain(...)` into `Verbindungsgriff::drop` *before*
   `CloseHandle(self.h)`, and drop `HandleEintrag` entirely. The register then
   cannot outlive the open handle by construction.
2. If the two types must stay separate, force the order explicitly on the
   failure path, e.g. bind the closure body as
   `let _handle_eintrag = handle_eintrag; let griff = griff;` at the top of the
   closure (mirroring line 2275), or handle `Err(_)` by explicitly
   `drop(handle_eintrag_clone_of_id)` before releasing the `Arc`.
3. Update the `// SAFETY:` comment at line 1286 to name the *enforcer*
   ("`Verbindungsgriff::drop` removes the entry before `CloseHandle`"), not the
   hoped-for property, so the next reader can check it in one place.
