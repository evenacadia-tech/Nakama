---
stage: final-report
threat_model: LOCAL_UNPRIVILEGED
severity_filter: medium
total_primaries: 28
reported_findings: 14
---

# Rust Security Review — Final Report

**Scope:** `broker/` (crate `eqcop_broker`), phase gate G3 at commit `ad5b2d1b`
**Threat Model:** LOCAL_UNPRIVILEGED
**Severity Filter:** medium (LOW findings are recorded in `findings/` but not reproduced here)
**Primaries (after dedup):** 28
**Reported:** 14 (after FP verdict and severity filter)

## Severity distribution (reported)
| Severity | Count |
|----------|-------|
| CRITICAL | 0 |
| HIGH     | 0 |
| MEDIUM   | 14 |
| LOW      | 13 (filtered out) |

Of the 28 primaries, 1 was judged LIKELY_FP (STRSLICE-001) and 13 were real but
LOW-impact under this threat model. There were no FALSE_POSITIVE and no
OUT_OF_SCOPE verdicts. Per-finding reasoning is in `fp-summary.md`.

## Why nothing is HIGH or CRITICAL
The review's own threat model puts the trust boundary at the named pipe: clients
are untrusted until the duplex handshake proves possession of the session
credentials, and the adversary is a local unprivileged process. The broker and
the probe plugins run under the same logon SID by design, so no defect found in
this run can yield another user's data, privileged file access or privilege
escalation. Under the local severity table the ceiling for what these defects do
achieve — denial of service and integrity divergence against the broker and
against other probe sessions — is MEDIUM. Read the absence of HIGH as "the
privilege model holds", not as "the surface is clean": 14 of 28 primaries are
attacker-reachable, and four of them need nothing more exotic than a client that
stops reading its own pipe.

## Read this before counting the MEDIUMs
Four of the 14 reported findings are **one** defect filed four times
(OPAQUEPTR-001, RAWFD-002, UNSAFESYNC-001, SAFETYDOC-002). Dedup merged only
UAF-001 into OPAQUEPTR-001; the other three escaped merging because two workers
wrote scope-relative paths and two wrote repo-relative ones, and because two
filings are anchored at the sink and at the `unsafe impl` rather than at the
acceptor. They share one severity by rule and are presented below as a single
grouped entry. **The 14 reported findings are 11 distinct defects.**

---

## MEDIUM (14)

### CHANSTARVE-001 — Blocking snapshot ack on the broker's only supervisory thread
- **Location:** `broker/src/transport/server_v3.rs:1046` (`snapshot_schreiben`)
- **Attack vector:** Local
- **Exploitability:** Reliable — no race, no resource exhaustion, no malformed byte
- **Also affects:** — (standalone primary)
- **FP verdict:** TRUE_POSITIVE — every link verified in source: the blocking `recv_timeout(SENKE_FRIST)`, the serial per-subscriber loop in `flush_session`, `SESSION_CLIENT_CAP = 64`, the single-threaded tick driver, and the deadline-free `join()` in `BrokerSupervisor::drop`.
- **Severity rationale:** Local DoS of the broker's only supervisory thread; eviction, tombstoning and fail-closed link closing stop for up to ~128 s per tick, and an ordered stop inherits the stall.

`V3Sender::snapshot_schreiben` enqueues a P1 session snapshot and then blocks on
a `sync_channel(1)` receiver until the connection's writer thread returns from
`ov_schreiben`, which itself waits `WaitForSingleObject(e, INFINITE)` on the
overlapped `WriteFile`. A peer that stops draining its pipe parks that writer
once the 64 KiB pipe output buffer is full, so every `snapshot_schreiben` for
that link burns the full `SENKE_FRIST = 2000 ms`.

The wait is serial per subscriber. `Coordinator::flush_session`
(`broker/src/coordinator.rs:2273-2283`) loops over every subscribed link of the
session, and the only driver is the process's single liveness thread
`nakama-coordinator-tick` (`broker/src/lib.rs:918-927`) — the same thread that
evicts stale clients, applies tombstones and returns the link ids that
`V3Closer::link_schliessen` must close. With `SESSION_CLIENT_CAP = 64` that is
up to ~128 s of dead time per tick.

The blocking contradicts the module's own rule. The sibling P2 path is
documented as never waiting on pipe I/O (`SessionPush::messframe_schreiben`,
`broker/src/coordinator.rs:206-211`), and the server-to-sink direction is
wrapped in `Senkenruf` precisely so a slow peer cannot pin a broker thread. The
coordinator-to-sink direction has no such protection.

Shutdown inherits it: `BrokerSupervisor::drop` (`broker/src/lib.rs:830-837`)
joins the tick thread with a bare `join()`, so `broker_geordnet_stoppen()` hangs
for as long as the attacker keeps its pipes unread.

**Recommendation.** Make the P1 push non-blocking like the P2 path, or give the
loop a budget: after the first `recv_timeout` expiry mark the link
back-pressured, skip its remaining pushes for this tick and let the existing
`geschlossen_writer`/`trennen` machinery close it. Additionally bound
`BrokerSupervisor::drop` with a `join_mit_frist`-style deadline.

Full finding: `findings/CHANSTARVE-001.md`

---

### RESEXHAUST-001 — Declared batch cap is recorded as a violation but never bounds the loop
- **Location:** `broker/src/telemetrie.rs:316` (`pruefe`)
- **Attack vector:** Local
- **Exploitability:** Reliable
- **Also affects:** family with RESEXHAUST-002/003/004 (same root pattern: a declared cap that is not a control-flow bound)
- **FP verdict:** TRUE_POSITIVE — verified at `telemetrie.rs:307-330`: the `MAX_EINTRAEGE` check only pushes a `Verstoss` and execution falls straight into an unbounded `for` over `eintraege`.
- **Severity rationale:** Local DoS by amplification, reachable before any coordinator-side routing or address check.

`MAX_EINTRAEGE = 32` is the protocol cap for entries per `FeatureBatch`. When a
batch exceeds it the function records a violation and keeps going: the per-entry
validation loop iterates every element of the attacker-supplied vector, with no
`take(MAX_EINTRAEGE)`, no early return and no work budget.

Three properties compound. A FlatBuffers vector of table offsets costs 4 bytes
per element, so the 262 128-byte payload cap allows roughly 65 500 entries. Many
offsets may alias the *same* table, so the payload does not grow with the entry
count, and the verifier's `max_tables` budget of 1 000 000 is not reached. Per
iteration the loop does far more than O(1) work (`pruefe_baender` walks up to
221 band values, `pruefe_band_stereo` another 64), and every violation allocates
two heap `String`s whose JSON-pointer path embeds the index — so every violation
is unique and the `BTreeSet` in `kanonisch` dedups nothing.

The expensive validation runs *before* session, routing and address checks,
which all live behind `fuer_broker` returning `Ok`. The only prerequisite is one
completed bootstrap handshake.

**Recommendation.** Enforce the cap as a bound, not a label: return `kanonisch(out)`
immediately when `eintraege.len() > MAX_EINTRAEGE`, add the same guard at the top
of `strukturriegel`, and cap `out.len()` so `kanonisch` can never sort an
attacker-scaled vector.

Full finding: `findings/RESEXHAUST-001.md`

---

### RESEXHAUST-002 — Ingress queue is capped by frame count, never by bytes
- **Location:** `broker/src/transport/server_v3.rs:2770` (`verbindung_bedienen`)
- **Attack vector:** Local
- **Exploitability:** Reliable
- **Also affects:** family with RESEXHAUST-001/003/004
- **FP verdict:** TRUE_POSITIVE — verified: `CAP_INGRESS = 256` is an entry count (`warteschlange.rs:24`, `:354`), `MAX_PAYLOAD_BYTES = 262128` applies per frame with no per-family budget, `MAX_VERBINDUNGEN = 96` for v3.
- **Severity rationale:** ~64 MiB resident per connection and ~6.4 GiB broker-wide held in steady state using only well-formed frames.

Every validated v3 frame is moved whole into the per-connection ingress queue,
whose only limit is a count of 256 entries. Each entry owns a `Vec<u8>` sized at
exactly `payload_len`, and the envelope parser checks size, CRC, flags and
family/encoding agreement but never a per-family size budget — a 256 KiB P0
control JSON is accepted exactly like a 256 KiB P2 batch.

The P2 backpressure policy makes the condition persistent rather than transient:
on overflow `IngressWarteschlange::einreihen` drops the oldest P2 entry and
enqueues the new one, so a telemetry connection flooding full-size P2 frames
parks the queue at 256 full entries indefinitely and is never disconnected.
(P0/P1 overflow does disconnect — but only after the 64 MiB is already resident,
and the attacker simply reconnects.)

On a workstation running FL Studio plus the plugin family this is enough to push
the machine into swap and take the broker down with every probe's control path.

**Recommendation.** Give the queue a byte budget alongside the entry budget
(e.g. 4 MiB per connection, still 16 full-size frames in flight), and enforce
per-family payload maxima at the envelope layer so a control JSON can never
approach 256 KiB.

Full finding: `findings/RESEXHAUST-002.md`

---

### RESEXHAUST-003 — Alias quarantine set grows without bound or eviction
- **Location:** `broker/src/instance_alias.rs:163` (`registriere_wire_zuordnung`)
- **Attack vector:** Local
- **Exploitability:** Reliable
- **Also affects:** family with RESEXHAUST-001/002/004; shares its root cause (the unbound `logon_sid`) with RESEXHAUST-004
- **FP verdict:** TRUE_POSITIVE — verified: `quarantaene` has `insert` at `:163`, `:164`, `:180` and membership tests elsewhere, but no `remove` anywhere in the crate; `entferne` explicitly refuses quarantined owners at `:197`.
- **Severity rationale:** Unbounded permanent memory growth ending in broker OOM, with the growing set consulted on every routing decision.

An alias collision inserts both owners into a permanent quarantine set keyed by
`(Sitzungsadressraum, String)`, where the address space is
`{logon_sid, project_binding_id, session_epoch}`. The module documents the
no-removal policy deliberately (a disconnect must not wash the visible
conflict), but there is also no cap, no expiry and no overflow counter.

Every key component comes from the attacker hello. `bootstrap::adresse_pruefen`
checks shape only: `logon_sid` is 1..=184 characters of anything, the other
fields are hex32. Crucially — and I confirmed this by grepping every
`logon_sid` use in `broker/src` — the claimed `adresse.logon_sid` is never
compared against the impersonated caller real logon SID.
`client_sid_authentisieren` validates the pipe peer token against the broker own
user SID; nothing ties that result back to the string in the hello. The attacker
therefore picks an unlimited number of distinct address spaces, and each
supports a fresh collision.

Provoking a collision needs no privilege: connect as
`(space S, instance_id I, nonce N1)`, then again as `(space S, I, N2)`. The two
connections are different owners of the same wire value, both are quarantined
permanently, and the attacker moves to space S+1 — never revisiting a key, so
nothing is ever a duplicate insert. `GLOBAL_CLIENT_CAP` eviction does not touch
the quarantine.

**Recommendation.** Reject a `hello_control` whose `adresse.logon_sid` does not
equal the SID obtained from the impersonated peer — that alone collapses the
address space to one value. Then cap the quarantine with oldest-first eviction
and a visible overflow counter, and add a time-based expiry much longer than a
reconnect cycle rather than an infinite one.

Full finding: `findings/RESEXHAUST-003.md`

---

### RESEXHAUST-004 — Attacker-triggered collisions append unbounded rows to a persistent table
- **Location:** `broker/src/coordinator.rs:799` (`control_hello_registrieren`)
- **Attack vector:** Local
- **Exploitability:** Reliable
- **Also affects:** family with RESEXHAUST-001/002/003; same root cause as RESEXHAUST-003
- **FP verdict:** TRUE_POSITIVE — verified: the collision branch queues a `ConflictGuard` whose primary key is built entirely from hello fields (`coordinator.rs:3206`); `store.rs:1181` is `INSERT OR IGNORE`, which dedups only identical pairs; the sole DELETE (`store.rs:1200`) is reachable only from `konflikt_guard_aufloesen`, which no disconnect, eviction, idle or compaction path calls; `store.rs:1370` reloads the whole table at startup.
- **Severity rationale:** Local DoS that persists across restarts and denies the whole volume, not just the broker.

Each provoked collision writes a new row of roughly 300-800 bytes plus index, at
whatever rate the attacker can complete bootstrap handshakes. Since
`effective_address` embeds the free-form `logon_sid` and three hex32 fields, the
attacker never repeats a pair and `INSERT OR IGNORE` never suppresses anything.
`kompaktieren` and WAL checkpointing manage the event log and the WAL, not this
table; there is no row cap and no retention window despite the `created_utc_ms`
column existing.

Unlike the in-memory leak this survives a restart. The file grows until the
user-writable volume is full — which denies service to every other component
that needs to write there, a classic local-unprivileged lever — and at that
point every store write degrades to `StoreFehler::Degradiert`. Because the whole
table is re-read into `stand.conflict_guards` at startup, a large table also
makes startup slow and memory-hungry, and that map is consulted on every routing
decision.

**Recommendation.** Bind `logon_sid` to the impersonated peer SID (see
RESEXHAUST-003), cap the table by row count and prune oldest-first by
`created_utc_ms`, and rate-limit collision-driven persistence per link so a
connect loop cannot translate one-to-one into durable writes.

Full finding: `findings/RESEXHAUST-004.md`

---

### REENTRANT-001 — Global state mutex held across the sink callback loop on the P2 ingress path
- **Location:** `broker/src/coordinator.rs:3191` (`p2`)
- **Attack vector:** Local
- **Exploitability:** Reliable
- **Also affects:** REENTRANT-002 (same construct, subscriber-attach path)
- **FP verdict:** TRUE_POSITIVE — verified: the guard taken at `:3096` is still alive at the `push.messframe_schreiben` loop (`:3189-3191`) and is released only when the block closes at `:3194`.
- **Severity rationale:** The broker single global lock is held across subscriber-count times (allocate + CRC32C over up to 256 KiB + two further mutexes) per attacker-supplied frame, with every multiplier peer-chosen.

`Coordinator::p2` acquires `self.stand`, the broker one global state mutex, and
keeps the guard alive while calling out through the `Arc<dyn SessionPush>` trait
object once per subscriber. That contradicts the invariant the same file states
900 lines earlier in `flush_session` (`:2267-2271`), where the shard guard is
explicitly dropped before any sink call because a sink may observe the
coordinator reentrantly.

Two consequences. **Today:** the global lock serialises the whole broker behind
a peer-controlled workload — frames per second (up to `RATE_PRO_SEKUNDE = 4000`
per connection), payload size (up to 256 KiB) and subscriber count (up to
`SESSION_CLIENT_CAP = 64`) are all attacker-chosen, which is exactly what the
`flush_session` comment was written to prevent. **Latent:** `std::sync::Mutex`
is not reentrant, and `SessionPush` is a `pub` trait with a default body, so any
future sink that takes the documented reentrancy permission wedges the global
lock forever — control, heartbeat, command and liveness processing all stop
behind it, and `std::sync::Mutex` does not detect the self-deadlock.

The lock order established here (`Coordinator::stand`, then
`V3Sender::telemetrie_ausgaenge`, then `Ausgang::inhalt`) also permanently
forbids the reverse edge.

**Recommendation.** Collect the push targets under the lock, drop the guard,
then push — the shape `flush_session` already uses. Re-check target validity
with a short lock inside the loop (`push_ziel_noch_gueltig` already exists) if
that guarantee matters, and state the invariant on the `SessionPush` trait
itself rather than in a comment inside one caller.

Full finding: `findings/REENTRANT-001.md`

---

### REENTRANT-002 — Same construct when replaying held P2 frames to a subscriber
- **Location:** `broker/src/coordinator.rs:1124` (`messframes_an_subscriber_push`)
- **Attack vector:** Local
- **Exploitability:** Reliable
- **Also affects:** REENTRANT-001 (same construct, P2 ingress path)
- **FP verdict:** TRUE_POSITIVE — verified: the guard is bound to a named local at `:1097` rather than to a block, so it lives to the end of the function, including the per-frame push loop at `:1123-1125`.
- **Severity rationale:** Same mechanism and magnitude as REENTRANT-001 (up to 64 held frames of up to 256 KiB CRC-framed under the global lock per subscriber attach), and the attacker controls the attach rate.

This is REENTRANT-001 at a second, independently reachable site: subscriber
attach rather than telemetry ingress. `frames` is built from `stand.messframes`
filtered to the session — one entry per live probe instance, bounded only by
`SESSION_CLIENT_CAP = 64` — each payload is a peer-supplied P2 batch cloned
under the lock and then CRC-framed by `envelope_schreiben` inside the callback,
all with the global mutex held.

Both sites need the same fix; they are listed separately here only because they
are separate code changes.

**Recommendation.** As REENTRANT-001: hoist the frame collection into a scoped
block, drop the guard, then push.

Full finding: `findings/REENTRANT-002.md`

---

### DROPPANIC-001 — Panic in a destructor turns one thread failure into a process abort
- **Location:** `broker/src/server.rs:347` (`RegistrierteVerbindung::drop`)
- **Attack vector:** Local
- **Exploitability:** Difficult — needs one prior panic under the coordinator lock
- **Also affects:** UNWRAP-001 (LOW, filtered out) is the containment gap that makes this escalate
- **FP verdict:** TRUE_POSITIVE — verified: the destructor guards its first lock poison-safely (`if let Ok(mut r) = self.register.lock()`) and then calls `senke.getrennt`, which reaches `hoermarkierung_v2_getrennt` and its poison-fatal `expect` at `coordinator.rs:2430` — one of 54 such sites in that file. `broker/Cargo.toml` has no `[profile]` section, so `panic = "unwind"` is active in release.
- **Severity rationale:** Panic inside Drop during cleanup is held at the MEDIUM floor: a second panic while unwinding is an unconditional `abort()`.

`RegistrierteVerbindung` is the RAII teardown guard of every v2 pipe connection
thread. The escalation needs exactly one prior panic anywhere under
`Coordinator::stand`: the mutex is then poisoned for the rest of the process
lifetime (nothing in the crate calls `clear_poison`), the next v2 connection
thread panics on the poison-fatal lock in its heartbeat loop, unwinding drops
`registrierung`, and the destructor panics again on the same poison.

The result is whole-process termination of `eqcop-broker.exe`, not a single
connection failure. Every probe session, the single-writer SQLite store thread,
the v3 transport and its listeners die at once, and `abort()` skips all
remaining destructors — so exactly the graceful teardown guarantees NAK-123 was
written to establish (listener name ownership released last, `RevertToSelf`
cleanup, handle-register drain) never run.

The author was aware a destructor must not unwrap a lock: the first statement of
the same `drop` is deliberately poison-safe. Only the second one, which reaches
a panicking lock through a trait object, is not.

**Recommendation.** Make the destructor panic-free to match its first statement:
switch `hoermarkierung_v2_getrennt` (and ideally all 54
`expect("Coordinator vergiftet")` sites) to `unwrap_or_else(|e| e.into_inner())`,
which the transport layer already uses everywhere. Both halves of the fix belong
in one change set with UNWRAP-001.

Full finding: `findings/DROPPANIC-001.md`

---

### NONDET-001 — P0 command target chosen by hash-map iteration order
- **Location:** `broker/src/coordinator.rs:2648` (`persistenz_p0`)
- **Attack vector:** Local
- **Exploitability:** Difficult — needs a duplicate-nonce control link held open
- **Also affects:** — (standalone primary)
- **FP verdict:** TRUE_POSITIVE — verified: `stand.links` is a `HashMap<String, LinkStand>` (`coordinator.rs:395`), the target is resolved with `.iter().find()` on address plus session plus `!trennen` only, and the displacement at `:766` is skipped when the runtime nonce is identical, so two live links can share one address with `trennen == false`.
- **Severity rationale:** Targeted local DoS of another instance command path plus a durable-state integrity defect: identical client behaviour produces different persisted state.

The predicate does not match on link identity, and `HashMap` iteration order is
randomized per map instance, so when more than one link satisfies it `find`
returns an arbitrary one and the choice changes from run to run.

The precondition is reachable. A second control connection presenting the
identical address — same `logon_sid`, `project_binding_id`, `session_epoch`,
`instance_id` and `runtime_nonce` — is not displaced (the displacement branch
requires a differing nonce), so the old `LinkStand` stays in `stand.links` with
`trennen == false` while `client.current_link` is repointed to the new link. The
alias register does not catch it either: an identical `instance_id:runtime_nonce`
owner registers as `BereitsEingetragen`, not as a collision. Client caps do not
fire because `platz_schaffen_locked` is only entered for a new `ClientKey`. Every
session member can read other members full addresses, including `runtime_nonce`,
from the `session_snapshot` payload.

With two matching links the branch below the `find` flips on iteration order:
either the command is authorised and an event is appended to the append-only
`event_log` plus `outbox` and acked, or it is rejected as `unknown_target` with
nothing persisted. The append-only log — the reconstruction source of the whole
persistence design — is therefore not reproducible.

**Recommendation.** Resolve the target by link identity, not by address match:
look the target up through `client.current_link` and verify the address, instead
of scanning `stand.links` for the first address match. Independently, make a
duplicate-nonce control hello displace the older link the same way a
differing-nonce hello does, so two live links can never share one address.

Full finding: `findings/NONDET-001.md`

---

### BUFFLUSH-001 — Outbox debt retired on a write that only reached a kernel buffer
- **Location:** `broker/src/transport/server_v3.rs:678` (`Verbindungsgriff::drop`)
- **Attack vector:** Local
- **Exploitability:** Difficult — the attacker must hit the window between write and close
- **Also affects:** BUFFLUSH-002 (LOW, filtered out) is the same missing flush on the reject path
- **FP verdict:** TRUE_POSITIVE — verified: `FlushFileBuffers` appears nowhere in `broker/src/transport/` (the only hits in the crate are the legacy v2 path at `server.rs:876` and `:890`), `Verbindungsgriff::drop` closes the handle bare, and `coordinator.rs:2274-2281` calls `snapshot_schuld_kompaktieren` on the strength of the writer bool.
- **Severity rationale:** Silent integrity divergence on a path the broker has already recorded as delivered, with no counter and no log line.

On Windows a `WriteFile` against the server end of a named pipe completes as
soon as the payload is copied into the pipe instance output buffer (64 KiB
here); it does not wait for the client to read. `FlushFileBuffers` on a pipe
server handle is the documented mechanism that waits for drainage, and Microsoft
pipe-server guidance places it before `DisconnectNamedPipe`/`CloseHandle`. This
crate own legacy v2 server does exactly that in `pipe_nach_antwort_schliessen`,
with a deadline so a silent peer cannot block the broker. The v3 rewrite dropped
it; the symbol is not even imported in `server_v3.rs`.

The consequence is not cosmetic, because the writer result is treated as proof
of delivery and propagated upward: writer thread bool, then answer channel, then
`V3Sender::snapshot_schreiben`, then `Coordinator::flush_session`, which reacts
to `geschrieben == true` by calling `store.snapshot_schuld_kompaktieren(ziel, ord)`
and deleting the outbox debt for that target up to `event_ord`. So the broker
permanently retires its obligation to re-deliver a session snapshot on the
strength of a write that only reached a kernel buffer, and then closes that
buffer owner without draining it. There is no application-level receipt:
`bestaetigt` in the coordinator is session-join confirmation, not snapshot
receipt.

Under this threat model it is attacker-influenceable: a local process can drive a
control hello that makes the coordinator evict the legitimate link at a moment
of its choosing, forcing teardown of a connection whose last snapshot frame is
still unread. Repeated, that is a targeted "the app silently misses committed
state updates" primitive rather than a visible outage. The same window also
covers P0 command answers and the welcome frame.

**Recommendation.** Make "written" mean "read by the peer" before it is allowed
to retire store state. Call `FlushFileBuffers` on the server end with a hard
deadline before the handle closes — reuse the v2 pattern (own thread plus
timeout) and place it in `Verbindungsgriff::drop` so every exit path is covered
— then `DisconnectNamedPipe`, then `CloseHandle`. Only propagate
`geschrieben = true` for entries carrying an answer channel after that flush
confirms drainage, or gate `snapshot_schuld_kompaktieren` on a peer-side
acknowledgement. Add a `flush_abgelaufen` counter beside `geschlossen_writer` so
a lost delivery is observable.

Full finding: `findings/BUFFLUSH-001.md`

---

### OPAQUEPTR-001 + RAWFD-002 + UNSAFESYNC-001 + SAFETYDOC-002 — one defect: pipe HANDLE closed before it leaves the cancel register
- **Location:** `broker/src/transport/server_v3.rs:1979` (`v3_server_starten_intern`); sink at `:1289` (`alle_io_abbrechen`); `unsafe impl Send` at `:975`
- **Attack vector:** Local
- **Exploitability:** Difficult — requires forcing `thread::Builder::spawn` to fail and then winning a race
- **Also known as:** UAF-001 (merged into OPAQUEPTR-001 by dedup)
- **Also affects:** RAWFD-002, UNSAFESYNC-001 and SAFETYDOC-002 are the same defect filed from three other angles; dedup left them unmerged because of path-prefix drift and differing anchor lines. **Fix once.**
- **FP verdict:** LIKELY_TP (all four) — every mechanical claim reproduced in source; only the reachability of a failing worker `spawn` is unproven.
- **Severity rationale:** Carrying framing is the use-after-close of a kernel handle. A recycled HANDLE value can absorb a `CancelIoEx` aimed at a listener pending `ConnectNamedPipe`, another probe live read, or a store file handle — local DoS of authenticated sessions, triggered from a connection that was rejected before authentication ever ran. Reduced one level for the race; not reduced for the SAFETY comment, because that comment states precisely the invariant being broken.

The v3 acceptor publishes every accepted pipe HANDLE into a process-wide cancel
register (`HandleRegister.offen: Vec<(u64, isize)>`) *before* the worker thread
that owns it exists (`:1911-1914`) — deliberately, to fix an earlier bug where a
stop between spawn and registration left a thread blocked with no watchdog. Two
owners of that same kernel handle are then moved into the worker closure:
`griff: Arc<Verbindungsgriff>`, whose `Drop` calls `CloseHandle` (`:678`), and
`handle_eintrag: HandleEintrag`, whose `Drop` removes the entry (`:2229`).

Inside the worker the ordering is sound and explicitly engineered:
`verbindung_bedienen` rebinds the entry as a body local
(`let _handle_eintrag = handle_eintrag;`, `:2275`) so it drops before the `griff`
parameter — deregister first, close second.

On the spawn-failure arm (`:1979`) that guarantee is gone. The closure is never
called; `spawn` drops it, and the two owners are dropped as closure captures.
Rust leaves the drop order of `move`-closure captures **unspecified**; rustc
lowers captures to struct fields in first-use order and drops them in
declaration order, and in this closure `griff` is used at the
`verbindung_bedienen` call well before `handle_eintrag` (the last argument). So
`CloseHandle(h)` runs while `(id, h)` is still published in the register. The
comment in that arm ("closure drop releases worker slot, handle and register")
asserts an ordering the language does not provide.

During that window the register violates the invariant printed at both the
`unsafe impl Send` (`:971-974`) and the `CancelIoEx` sink (`:1285-1287`): "the
entry lives only as long as the owning thread holds its handle". The mutex half
of that claim does hold — deregistration and cancellation share the same lock —
but `CloseHandle` happens outside the lock, so the lock gives no ordering here.

The concurrent reader is real and runs on other threads: `V3Griff::stoppen`
calls `alle_io_abbrechen` before joining the acceptor, and re-runs it in a 5 ms
polling loop, which *increases* the chance of sampling the window; the watchdog
thread and `io_abbrechen` reach the same values. Windows recycles handle values
aggressively, and this broker mints new kernel objects continuously on other
threads (`CreateNamedPipeW` in `naechste_instanz` on the very next loop
iteration, `CreateEventW` per connection, SQLite file handles), so a recycled
value naming a different live object is the common case rather than an exotic
one.

Note the authentication ordering: the handle is registered *before*
`client_sid_authentisieren` ever runs, so the corrupted entry belongs to a
connection that never had to pass the SID or duplex handshake, while the damage
lands on connections that did.

**Coverage gap.** NAK-123 `tests/security_vectors.rs` injects listener, watchdog
and acceptor spawn failures via `V3StartTestFehler`, but has no seam for a
failing *connection-worker* spawn, so this arm has no test coverage at all. No
MIRI (it cannot model Win32 anyway), no sanitizer, no Application Verifier
handle-checking leg in `tools/beweise.ps1`, and no `debug_assert!` on the
register contents.

**Recommendation.** Do not rely on closure-capture drop order for a safety
invariant; make the ordering structural so it cannot be expressed wrongly on any
path (spawn failure, normal return, or panic unwind). Two good shapes:

1. Move deregistration into `Verbindungsgriff::drop` — `retain` the entry out
   first, then `CloseHandle` — and delete `HandleEintrag` entirely. Close-before-
   unregister then becomes inexpressible.
2. Or give `HandleEintrag` ownership of the keeper (`_griff: Arc<Verbindungsgriff>`)
   and pass `griff.clone()` into it, so the entry always outlives the open
   handle. Storing `Arc<Verbindungsgriff>` rather than a bare `isize` in
   `HandleRegister::offen` additionally lets the manual `unsafe impl Send` be
   deleted, making the compiler rather than a comment the guarantor.

Either way, update the SAFETY comments at `:975` and `:1286` to name the
*enforcer* rather than the hoped-for property, and align the poison handling:
`HandleEintrag::drop` silently skips removal on a poisoned mutex
(`if let Ok(...)`) while the acceptor push recovers with
`unwrap_or_else(|e| e.into_inner())`, so the two halves of the same invariant do
not even agree on poisoning.

Full findings: `findings/OPAQUEPTR-001.md` (primary, absorbed `findings/UAF-001.md`),
`findings/RAWFD-002.md`, `findings/UNSAFESYNC-001.md`, `findings/SAFETYDOC-002.md`

---

## Filtered out at LOW (13) — recorded, not reproduced above
The `medium` filter drops these. They keep their `severity: LOW` in
`findings/*.md` for traceability. Three clusters are worth a maintenance ticket
even though none is a vulnerability under this threat model:

| ID | One line |
|----|----------|
| CARGOLINT-001 | No `[lints]` table, no crate-level deny, no clippy leg, on a crate with 89 `unsafe` blocks and 60 `// SAFETY:` comments |
| MSRV-001 | No `rust-version` and no pinned toolchain; the real floor is set by a caret dependency in `Cargo.lock` |
| SAFETYDOC-001 | 18 `unsafe` occurrences and zero `// SAFETY:` comments inside `client_sid_authentisieren`, the authentication gate itself |
| UNWRAP-001 | 54 poison-fatal `expect` sites on the coordinator state lock; the missing containment behind DROPPANIC-001 |
| RESDISC-001 | Automatic WAL checkpoint errors discarded and the idle path marked done anyway |
| RESDISC-002 | Worker join `Result` discarded, so a panicking worker on the attacker-facing parse path leaves no trace |
| DROPSKIP-001 | `BrokerLauf.store` is neither stopped by `broker_geordnet_stoppen` nor dropped from the `static OnceLock` |
| BUFFLUSH-002 | Bootstrap reject frame buffered and abandoned; the peer sees a broken pipe instead of the reason |
| RAWFD-001 | Detached writer keeps the handle after its register entry is removed; leak is permanent when it fires |
| TOCTOU-001 | Store schema gate validated through a separate read-only open, and the `exists()` pre-check makes it fail open |
| TOCTOU-002 | Remote-volume refusal evaluated on the nearest existing ancestor, never on the path that is created and opened |
| LOSSYSTR-001 | `String::from_utf16_lossy` on the store path instead of `OsString::from_wide` |
| STRCMP-001 | Probe-namespace gate is a raw prefix match on an un-normalized local-device pipe path |

The two hygiene findings (CARGOLINT-001, MSRV-001) are cheap and unblock
mechanical answers to several open questions below; SAFETYDOC-001 and UNWRAP-001
are the ones a reviewer should read alongside the MEDIUM entries they enable.

## Scope notes
- **Read-only run.** No `cargo build/test/check/clippy` was executed and no
  named pipe was opened, per the run constraint (a soak test held
  `broker/target/release/`). Every verdict rests on reading `broker/src` and on
  `rg`/`grep` searches; nothing was validated dynamically.
- **Four filings, one defect.** See the note above the MEDIUM section. Dedup
  reported the cause honestly: 22 findings use repo-relative paths and 7 use
  scope-root-relative ones, and normalization is string-based by protocol.
- **Claimed NAK-123 mitigations were checked, not assumed.** The impersonation
  chain, the listener name-ownership discipline and the handle/thread cleanup on
  *start-failure* paths hold as described. The gap is the connection-worker
  `spawn` failure arm, which `tests/security_vectors.rs` does not reach.
- **Not assessed.** Whether an attacker who replays a victim address (readable
  from `session_snapshot`) can become the P0 command target, rather than merely
  making the choice nondeterministic, was outside every filed finding; see
  `fp-summary.md` for why it deserves a targeted look.
- **The one recurring root cause worth naming.** `adresse.logon_sid` from
  `hello_control` is shape-checked but never bound to the SID that
  `client_sid_authentisieren` obtained for the pipe peer. That single omission
  is what makes RESEXHAUST-003 and RESEXHAUST-004 unbounded. Binding it is the
  highest-leverage change in this report.

## Artifacts
- `findings/*.md` — individual finding files; frontmatter carries `fp_verdict`,
  `fp_rationale`, `severity`, `attack_vector`, `exploitability`,
  `severity_rationale`, plus `merged_into` / `also_known_as`
- `fp-summary.md` — FP-judge summary, verdict counts and FP patterns
- `dedup-summary.md` — dedup summary (28 primaries from 29 raw findings)
- `findings-index.txt` — canonical manifest
- `REPORT.sarif` — SARIF 2.1.0 export of the same reported set
