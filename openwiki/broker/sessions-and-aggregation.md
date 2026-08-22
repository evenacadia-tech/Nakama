---
type: Architecture Guide
title: Broker sessions and aggregation
description: Explains register ownership, duplicate handling, profile bindings, pair alignment, and aggregate exports.
tags: [broker, sessions, persistence]
sources:
  - id: openwiki-source-28ed3266a194d52e2b1cbae3
    resource: repo://broker/src/aggregat.rs
  - id: openwiki-source-d10bf7912f9667f1d9a3f151
    resource: repo://broker/src/bindung.rs
  - id: openwiki-source-147d06d15258b01c3c7027da
    resource: repo://broker/src/lib.rs
  - id: openwiki-source-1ec6925550466f4ab20e1ec0
    resource: repo://broker/src/protokoll.rs
  - id: openwiki-source-993537de92f3e3f3ef6c12f0
    resource: repo://broker/src/server.rs
  - id: openwiki-source-1dac3d9e043830bbeb20a2c8
    resource: repo://eq-copilot/schemas/eq-aggregat.schema.json
generated: {by: "claude-code", at: "2026-08-22T15:50:39.855Z"}
verified:
  - by: openwiki/0.3.3
    at: 2026-08-22T15:50:39.855Z
---

# Broker sessions and aggregation

`Register` is the broker's in-memory source for sensor records, the persistent
sensor-to-profile binding map, discarded-packet count, and visible errors. The
pipe server mutates it after validating connection traffic; status, session,
pair, binding, and aggregate APIs read or update the same state.

## Live sensor ownership

An accepted hello calls `sensor_verbinden`. The register restores any existing
profile binding, tracks the connection's runtime nonce, and lets the newest
hello own sensor metadata and subsequent measurement writes. More than one
live nonce for the same persistent sensor ID sets the conflict flag.

Every recognized heartbeat refreshes liveness and increments the cumulative
heartbeat count. Only the owner nonce may replace statistics or measurement.
Disconnect removes its nonce, transfers ownership to a surviving connection,
and retains a visible disconnected sensor record after the last connection
leaves. Snapshots compute staleness at read time, keep stale and disconnected
records, and sort connected records first and then by label.

The server discards an invalid measurement rather than storing a truncated or
partially trusted value. It counts that packet while allowing the connection
and heartbeat acknowledgement to continue.

## Derived sessions and pairs

Sessions are derived, not persisted. `sessions_bilden` groups sensors by host
process ID and summarizes total and connected sensors plus distinct positive
sample rates. `broker_status` derives sensors, sessions, and pair evaluations
from one sensor snapshot so the views share a point-in-time input.

PRE and POST sensors group by non-empty `pair_id`. Missing or duplicate roles
produce an incomplete pair with a reason. A complete pair becomes unclear for
disconnection, staleness, sample-rate mismatch, absent or unready measurement,
missing project-time information, invalid windows, or no overlap. Jumps, low
overlap, different host processes, or materially different active duration
produce a probable rather than fully aligned result.

## Profile-binding persistence

This page is the sole owner of binding durability. `bindung.rs` stores a
schema-tagged JSON object. A missing file means an empty first start; unreadable,
malformed, or wrong-schema data returns an explicit error. Writing creates the
parent directory, writes a process-specific temporary file, and renames it into
place, removing the temporary file when replacement fails.

Startup loads the configured map into `Register`. Load failure does not prevent
broker startup; it leaves an empty in-memory map and appends a visible error.
The error list is capped at 50 entries.

`profil_binden` updates the in-memory map and any existing live sensor before
attempting to write the file. Consequently, a persistence failure is returned
and recorded but leaves memory ahead of disk. With no configured path, binding
is intentionally memory-only. A later connection restores the map's binding
for that sensor ID.

## Aggregate export

`aggregat_schreiben` requires a running broker, snapshots current sensors, and
applies exact optional profile and host-process filters. `aggregat_bauen`
intersects the available project-time windows and evaluates pairs on the
filtered subset. Missing measurement, absent windows, disconnection, staleness,
conflict, old protocol clients, collecting state, jumps, or non-overlap remain
visible as warnings. If the common window cannot be established, the document
uses `null` instead of inventing alignment.

The output contains schema and creation metadata, filters, the common window,
warnings, sensor snapshots, and pair assessments. Files use timestamped names
under `%LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots`. Missing environment
state and directory, serialization, write, or rename failures abort file
creation and return an error.

No production in-repository caller currently invokes the public binding or
aggregate-export APIs; current plugin traffic contains hello, heartbeat, and
bye only. Treat these functions as implemented library seams, not as exposed
plugin controls.

## Source map and focused validation

- Register and derivation: `broker/src/lib.rs` — `Register`,
  `sessions_bilden`, `paare_auswerten`, `profil_binden`, `aggregat_schreiben`
- Upstream mutation: `broker/src/server.rs::verbindung_bedienen`
- Message inputs: `broker/src/protokoll.rs` — `Hello`, `Heartbeat`,
  `MessStand`, `ProjektFenster`
- Binding durability: `broker/src/bindung.rs`
- Aggregate construction: `broker/src/aggregat.rs`
- Contract: `eq-copilot/schemas/eq-aggregat.schema.json`

Focused validation is:

```powershell
cargo test --manifest-path broker/Cargo.toml bindung::tests
cargo test --manifest-path broker/Cargo.toml register_tests
cargo test --manifest-path broker/Cargo.toml aggregat::tests
py -3.13 tools/eq-copilot/pruefe_v2_schemas.py
```

The tests cover file round-trip and corruption, conflict ownership, stale
visibility, session grouping, reconnect binding, pair downgrades, filters, and
aggregate warnings. They do not cover the memory/disk divergence after a
failed binding write or validate every built aggregate instance against the
JSON Schema.

