---
id: RESEXHAUST-001
bug_class: resource-exhaustion
title: MAX_EINTRAEGE cap is recorded as a violation but never bounds the FeatureBatch validation loop, allowing ~1000x CPU/RAM amplification from one 256 KiB P2 frame
location: src/telemetrie.rs:316
function: pruefe
confidence: High
worker: worker-7
fp_verdict: TRUE_POSITIVE
fp_rationale: "Verified at telemetrie.rs:307-330: MAX_EINTRAEGE=32 only pushes a Verstoss and control flow falls straight into an unbounded for over eintraege, with the per-index format! path making every violation unique so kanonisch's BTreeSet dedups nothing; the only real bound is MAX_PAYLOAD_BYTES."
severity: MEDIUM
attack_vector: Local
exploitability: Reliable
severity_rationale: "Local DoS by amplification: one 256 KiB P2 frame from an authenticated-but-untrusted peer buys ~1000x CPU and hundreds of MB of transient allocation before any coordinator-side routing or address check runs, and 96 connections at RATE_PRO_SEKUNDE=4000 scale it across cores."
---

## Description
`telemetrie::pruefe` declares a hard protocol cap of 32 entries per
`FeatureBatch` (`pub const MAX_EINTRAEGE: usize = 32`, `src/telemetrie.rs:81`).
When a batch exceeds it the function *records a violation* and then keeps
going: the per-entry validation loop iterates over **every** element of the
attacker-supplied `eintraege` vector, with no `take(MAX_EINTRAEGE)`, no early
`return`, and no work budget.

The amplification comes from three properties that compound:

1. A FlatBuffers vector of table offsets is 4 bytes per element, so the
   262 128-byte payload cap allows roughly **65 500 entries**.
2. FlatBuffers permits many offsets to alias the **same** table. All 65 500
   entries can point at one `QuellenEintrag`, so the payload does not need to
   grow with the entry count. The verifier does not de-duplicate: its
   `max_tables` budget is 1 000 000 (flatbuffers 25.12.19,
   `src/verifier.rs:254`), well above the ~460 000 table visits this shape
   costs, so the buffer verifies successfully.
3. Per iteration the loop does *far* more than O(1) work: `pruefe_baender`
   walks up to `BAENDER_FEIN = 221` band values and `pruefe_band_stereo`
   another 64, and every violation allocates two heap `String`s
   (`Verstoss::neu(&format!("{p}/..."), "...")`). Because the JSON-pointer path
   `p = /eintraege/{i}` embeds the index, **every violation is unique**, so
   the `BTreeSet` in `kanonisch` (`src/telemetrie.rs:481`) dedups nothing.

A single maximally-invalid shared entry yields on the order of 25-30 distinct
violations per index, i.e. ~1.6 million `Verstoss` values (each two heap
`String`s) built from one 256 KiB frame, then sorted through a `BTreeSet` and
collected back into a `Vec` — both live at the same time.

## Code
```rust
// src/telemetrie.rs:302
let eintraege = batch.eintraege();

if eintraege.is_empty() {
    out.push(Verstoss::neu("/eintraege", "eintraege_leer"));
}
if eintraege.len() > MAX_EINTRAEGE {
    out.push(Verstoss::neu("/eintraege", "eintraege_zu_viele"));   // <-- only recorded
}

let mut gesehen: BTreeSet<String> = BTreeSet::new();

for (i, eintrag) in eintraege.iter().enumerate() {   // <-- NOT bounded by MAX_EINTRAEGE
    let p = format!("/eintraege/{i}");

    let a = eintrag.quelle();
    pruefe_adresse(&a, &format!("{p}/quelle"), &mut out);
    if !gesehen.insert(a.instance_id().to_string()) {
        out.push(Verstoss::neu(&format!("{p}/quelle/instance_id"), "quelle_doppelt"));
    }

    pruefe_frame(&eintrag.frame(), &format!("{p}/frame"), &mut out);
}

kanonisch(out)
```

```rust
// src/telemetrie.rs:481 — every path string is unique, so nothing is deduped;
// the source Vec and the BTreeSet are alive simultaneously.
fn kanonisch(v: Vec<Verstoss>) -> Vec<Verstoss> {
    let menge: BTreeSet<Verstoss> = v.into_iter().collect();
    menge.into_iter().collect()
}
```

## Data flow
- **Source:** P2 FlatBuffers payload of a v3 envelope on the telemetry pipe;
  `payload: payload.to_vec()` in `transport::v3::envelope_pruefen`
  (`src/transport/v3.rs:320`), bounded only by
  `MAX_PAYLOAD_BYTES = 262_128`.
- **Sink:** the unbounded `for (i, eintrag) in eintraege.iter().enumerate()`
  loop at `src/telemetrie.rs:316` and the `BTreeSet` sort in
  `kanonisch` at `src/telemetrie.rs:481`.
- **Validation:** `eintraege.len() > MAX_EINTRAEGE` is checked at
  `src/telemetrie.rs:307` but only pushes a `Verstoss`; control flow continues
  into the loop. The only real bound on `n` is the 256 KiB frame cap, i.e.
  ~2000x above the protocol's own declared cap of 32.

## Reachability trace
`verbindung_bedienen` read loop (`src/transport/server_v3.rs:2770`)
→ `Eingang::einreihen` → ingress consumer thread `eqcop-v3-ingress-rest`
(`src/transport/server_v3.rs:2680`) → `senke.p2_mit_minor(&link, schema_minor, &payload)`
(`src/transport/server_v3.rs:2685`)
→ `Coordinator::p2` (`src/coordinator.rs:3081`)
→ `crate::telemetrie::fuer_broker(payload)` (`src/coordinator.rs:3082`)
→ `telemetrie::pruefe(puffer)` (`src/telemetrie.rs:342`).

The expensive validation runs *before* any coordinator-side session, routing or
address check — those all live behind `fuer_broker` returning `Ok`. The only
prerequisite is a coupled control+telemetry pair, i.e. one bootstrap handshake
(`hello_control` with well-formed hex32 fields, then `hello_telemetry` echoing
`link_id`/`challenge`/`runtime_nonce`) from a process that passes the logon-SID
impersonation check — which any process of the same local user does.

## Impact
A local unprivileged process that can open the pipe turns each 256 KiB
telemetry frame into hundreds of megabytes of transient allocation plus tens of
millions of string comparisons in the sort. The `eqcop-v3-ingress-rest` thread
of that connection is saturated for a long time per frame; with the per-
connection rate limit at `RATE_PRO_SEKUNDE = 4000` and up to 96 concurrent
connections, the attacker can pin many cores and drive the broker's RSS into
multi-gigabyte territory, starving the P1/P2 path for every legitimate probe.
This is pure availability loss — no panic, so none of the existing
"connection is closed on violation" defences ever trigger; the frame is
rejected only *after* the work has been paid for.

## Mitigations checked
- `MAX_EINTRAEGE = 32`: present in the contract and reported as a violation,
  but never enforced as a control-flow bound. It is a *label*, not a *limit*.
- FlatBuffers verifier: `max_tables = 1_000_000`, `max_depth = 64`,
  `max_apparent_size = 1 << 31` (defaults, flatbuffers 25.12.19). None of them
  is hit by ~460 000 aliased table visits, so the hostile buffer verifies.
- `strukturriegel` (`src/telemetrie.rs:190`) rejects zero offsets only; it
  itself iterates the same unbounded `batch.eintraege()` vector.
- Envelope layer: `MAX_FRAME_BYTES = 262_144`, CRC32C, 14 rejection rules —
  all satisfied by a well-formed hostile batch.
- Ingress backpressure: P2 overflow drops frames (`IngressErgebnis::P2Verworfen`)
  but does not shorten the work already dequeued.
- No `debug_assert!`, no fuzzing harness over `pruefe`, no MIRI on this path.

## Recommendation
Enforce the cap before the loop, not after it:

```rust
if eintraege.len() > MAX_EINTRAEGE {
    out.push(Verstoss::neu("/eintraege", "eintraege_zu_viele"));
    return kanonisch(out);            // reject; do not validate 65k entries
}
```

The same guard belongs at the top of `strukturriegel`. If the contract must
report the full violation set for over-long batches, bound the reported set
instead (e.g. `for (i, eintrag) in eintraege.iter().take(MAX_EINTRAEGE).enumerate()`)
and additionally cap `out.len()` so that `kanonisch` can never sort an
attacker-scaled vector.
