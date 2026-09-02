---
id: RESEXHAUST-002
bug_class: resource-exhaustion
title: Ingress queue is capped by frame count (256) but not by bytes, allowing ~64 MiB per connection and ~6 GiB broker-wide
location: src/transport/server_v3.rs:2770
function: verbindung_bedienen
confidence: High
worker: worker-7
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified: CAP_INGRESS=256 is an entry count (warteschlange.rs:24, :354), MAX_PAYLOAD_BYTES=262128 applies per frame with no per-family budget, MAX_VERBINDUNGEN=96 for v3, and the P2 overflow branch drops the oldest P2 and enqueues instead of disconnecting, so the queue parks full."
severity: MEDIUM
attack_vector: Local
exploitability: Reliable
severity_rationale: "Local DoS: ~64 MiB resident per connection and ~6 GiB broker-wide held in steady state with well-formed frames that pass all 14 envelope rules, enough to push a workstation running FL Studio into swap; the abuser is never disconnected on the P2 path."
---

## Description
Every validated v3 frame is moved whole into the per-connection ingress queue.
The queue's only limit is a **count** of 256 entries
(`CAP_INGRESS`, `src/transport/warteschlange.rs:24`); nothing anywhere bounds
the *bytes* those 256 entries hold. Each entry owns a `Vec<u8>` that
`transport::v3::envelope_pruefen` allocated at exactly `payload_len`, and
`payload_len` may be as large as `MAX_PAYLOAD_BYTES = 262_128`
(`src/transport/v3.rs:41`) for **any** message family, because the envelope
parser checks size, CRC, flags and family/encoding agreement but never a
per-family size budget.

So one connection can hold `256 * 262_128 B = 64 MiB` of attacker-chosen bytes
resident, and the acceptor allows `MAX_VERBINDUNGEN = 96` concurrent
connections (`src/transport/server_v3.rs:87`), giving a broker-wide steady-state
ceiling of roughly **6.4 GiB** that is reached purely by sending large frames
faster than the sink drains them.

The P2 backpressure policy makes this *persistent* rather than transient: a P2
overflow drops the oldest P2 entry and enqueues the new one
(`IngressWarteschlange::einreihen`, `src/transport/warteschlange.rs:354`), so a
telemetry connection flooding 256 KiB P2 frames parks the queue at exactly 256
full-size entries indefinitely without ever being disconnected. (P0/P1 overflow
does disconnect — but only after the 64 MiB is already resident, and the
attacker simply reconnects.)

## Code
```rust
// src/transport/server_v3.rs:2770 — the whole payload is moved into the queue
match eingang.einreihen(familie, schema_minor, r.payload) {
    IngressErgebnis::Eingereiht => {}
    IngressErgebnis::P2Verworfen => { statistik.ingress_p2_verworfen.fetch_add(1, Ordering::SeqCst); }
    IngressErgebnis::ClientTrennen => { /* ... */ break 'lesen; }
    IngressErgebnis::Verworfen => { statistik.ingress_p2_verworfen.fetch_add(1, Ordering::SeqCst); }
}
```

```rust
// src/transport/warteschlange.rs:354 — the cap counts ENTRIES, not bytes
pub fn einreihen(&mut self, familie: Familie, wert: T) -> IngressErgebnis {
    if self.inhalt.len() < self.kapazitaet {          // kapazitaet == CAP_INGRESS == 256
        self.inhalt.push_back((familie, wert));
        return IngressErgebnis::Eingereiht;
    }
    if self.aeltesten_p2_verwerfen() {                // drop oldest P2, keep the queue full
        self.inhalt.push_back((familie, wert));
        return IngressErgebnis::P2Verworfen;
    }
    // ...
}
```

```rust
// src/transport/v3.rs:320 — each payload is an exact-size heap allocation
Ok(Rahmen {
    kopf: Kopf { /* ... */ },
    drahtlaenge,
    payload: payload.to_vec(),        // up to MAX_PAYLOAD_BYTES = 262_128
})
```

## Data flow
- **Source:** `payload_len` field of the 16-byte v3 envelope header, read from
  the pipe by any authenticated peer (`src/transport/v3.rs:246`), bounded only
  by `MAX_FRAME_BYTES = 262_144`.
- **Sink:** `Eingang::einreihen` → `IngressWarteschlange::einreihen`, whose
  residency limit is `CAP_INGRESS = 256` *entries*.
- **Validation:** per-frame size cap only. No per-connection byte budget, no
  global byte budget, no per-family size budget (a 256 KiB P0 control JSON is
  accepted by the envelope layer exactly like a 256 KiB P2 batch).

## Reachability trace
`v3_server_starten_intern` acceptor (`src/transport/server_v3.rs:1782`)
→ `verbindung_bedienen` (`src/transport/server_v3.rs:2252`)
→ bootstrap handshake → `'lesen` loop → `StromLeser::naechster`
→ `eingang.einreihen(familie, schema_minor, r.payload)`
(`src/transport/server_v3.rs:2770`). Entries leave only via the two consumer
threads calling into the sink; while `Coordinator::p1`/`p2` are busy (JSON
schema validation, FlatBuffers validation, SQLite writes) the queue stays full.

## Impact
A local unprivileged process can drive the broker's resident memory to several
gigabytes and hold it there, using only well-formed frames that pass every one
of the 14 envelope rules. On a workstation running FL Studio plus the plugin
family this is enough to push the machine into swap or trigger the OS
low-memory killer, taking the broker (and with it every probe's control path)
down. Unlike a flood that trips `IngressErgebnis::ClientTrennen`, the P2 path
never disconnects the abuser, so the condition is steady-state, not a spike.

## Mitigations checked
- `CAP_INGRESS = 256`, `CAP_WRITER = 256`, `CAP_P0 = 64`, `CAP_P1 = 128`,
  `CAP_P2_JE_SONDE = 2`: all count-based; none is byte-based.
- `MAX_VERBINDUNGEN = 96` plus `worker_reservieren` bound the connection count,
  which bounds the total but at ~6.4 GiB.
- `MAX_KOPPLUNGEN = 64` (`src/transport/bootstrap.rs:324`) bounds couplings,
  not per-connection buffering.
- `Ratengrenze::neu(RATE_PRO_SEKUNDE = 4000, 1000)` limits frames/second, not
  bytes/second, and 4000 x 256 KiB/s is far above what is needed to keep the
  queue saturated.
- `leser.offen() > MAX_FRAME_BYTES + 4` (`src/transport/server_v3.rs:2810`)
  bounds the *partial-frame* read buffer only, not the queue.
- No `try_reserve`, no allocator limit, no cgroup/job-object memory cap.

## Recommendation
Give the ingress queue a byte budget in addition to the entry budget: track a
running `bytes_im_ingang` and refuse/drop once it exceeds a documented value
(e.g. 4 MiB per connection, which still allows 16 full-size frames in flight).
Independently, enforce per-family payload maxima at the envelope layer — a P0
or P1 control JSON has no legitimate reason to approach 256 KiB, so a much
smaller family-specific `MAX_PAYLOAD_BYTES` closes most of the gap before the
frame is ever copied out of the read buffer.
