---
type: Protocol Guide
title: Runtime protocol v2
description: Explains the production plugin-to-broker JSON exchange, framing, negotiation, runtime guards, and compatibility limits.
tags: [ipc, json, protocol-v2]
sources:
  - id: openwiki-source-57bb91b673883d82c7edc21f
    resource: repo://broker/src/framing.rs
  - id: openwiki-source-147d06d15258b01c3c7027da
    resource: repo://broker/src/lib.rs
  - id: openwiki-source-1ec6925550466f4ab20e1ec0
    resource: repo://broker/src/protokoll.rs
  - id: openwiki-source-993537de92f3e3f3ef6c12f0
    resource: repo://broker/src/server.rs
  - id: openwiki-source-7c625d989bae811077cd8c17
    resource: repo://eq-copilot/plugin/probe/PipeProbeMain.cpp
  - id: openwiki-source-f780b20513754b3fdca3ce27
    resource: repo://eq-copilot/plugin/src/EqCopilotIds.h
  - id: openwiki-source-b9bdb2eeb341991ef4dd964b
    resource: repo://eq-copilot/plugin/src/PipeClient.cpp
  - id: openwiki-source-329474248850d6e171eb413e
    resource: repo://eq-copilot/plugin/src/PipeClient.h
  - id: openwiki-source-5a8cd5b65a1c7fa70f8bf898
    resource: repo://eq-copilot/plugin/src/PluginProcessor.cpp
  - id: openwiki-source-837d41a393a054d97a0b2271
    resource: repo://eq-copilot/schemas/eq-ipc.schema.json
  - id: openwiki-source-dd243a8b8e494cd5837daeab
    resource: repo://tools/eq-copilot/pruefe_v2_schemas.py
generated: {by: "claude-code", at: "2026-08-22T15:50:39.855Z"}
verified:
  - by: openwiki/0.3.3
    at: 2026-08-23T10:24:07.287Z
---

# Runtime protocol v2

The production plugin and broker communicate with UTF-8 JSON protocol v2 over
a Windows named pipe. The broker still accepts protocol-v1 clients; the current
plugin requires a v2 welcome and is not a general downgrade client. The pipe
name retains a legacy `.v1` suffix, so negotiation—not the pipe string—is the
payload-version authority.

## Connection flow

`EqCopilotProcessor` constructs `PipeClient` with providers for persistent
sensor identity, volatile runtime nonce, audio configuration, runtime counters,
and compact measurement. The client connects, sends `hello`, and expects
`welcome` or `reject`. After a v2 welcome it sends a heartbeat roughly once per
second, waits for `heartbeat_ack`, and sends `bye` best-effort during teardown.

```mermaid
sequenceDiagram
    participant Plugin as PipeClient
    participant Broker as Connection worker
    participant Register as Live register
    Plugin->>Broker: u32 length + hello JSON
    Broker->>Broker: parse and negotiate v1..v2
    Broker->>Register: sensor_verbinden
    Broker-->>Plugin: welcome(v2) or reject
    loop approximately every second
        Plugin->>Broker: heartbeat(token, seq, stats, measurement?)
        Broker->>Register: validate and update
        Broker-->>Plugin: heartbeat_ack(seq, conflict)
    end
    Plugin->>Broker: bye (best effort)
```

Persistent identity comes from [Plugin state](../plugin/state-and-identity.md).
The runtime nonce distinguishes live copies of that state, and the broker's
session token belongs only to the current broker run. Duplicate live nonces for
one sensor ID create a visible conflict; the resolution path assigns one plugin
instance a new persistent ID and reconnects.

## Framing

Both sides use a four-byte little-endian unsigned length followed by exactly
that many UTF-8 JSON bytes. The maximum payload is 262,144 bytes. A zero-length,
oversized, truncated, or invalid UTF-8 frame fails before message dispatch.
Clean EOF before a new prefix is distinguished from a partial prefix or body.
Framing failure closes the affected connection rather than the broker process.

The broker's named-pipe ACL and same-name startup behavior are operational
properties documented in
[Broker service lifecycle](../broker/service-lifecycle.md).

## Declarative schema versus runtime enforcement

`eq-ipc.schema.json` declares strict shapes for hello, welcome, reject,
heartbeat, acknowledgement, and bye, plus measurement limits. Production code
does not load that JSON Schema. Rust `serde` deserialization and explicit
guards in `protokoll.rs` and `server.rs` are the actual runtime boundary.

Currently explicit measurement checks require a state and cap LTAS values at
512. A schema range or `additionalProperties` rule that has no equivalent
serde shape or explicit guard is a declarative contract expectation, not proof
of uniform runtime enforcement. `pruefe_v2_schemas.py` proves that the five v2
schema files are valid JSON Schemas with their frozen IDs; it does not prove
that the binaries enforce every keyword.

## Failure behavior

- Negotiation accepts versions 1 through 2 and returns a bounded version
  rejection otherwise.
- A heartbeat with the wrong session token increments the orphan-token counter,
  is logged, and is ignored.
- Invalid measurement is counted and omitted while the v2 connection remains
  alive and receives an acknowledgement.
- JSON parse failures are counted; frame failures end only that connection.
- V1 clients receive no acknowledgements and provide no v2 measurement.
- The current C++ client records ACK conflict state but does not verify that the
  echoed sequence equals the heartbeat sequence.

## Source map and validation

- Declarative contract: `eq-copilot/schemas/eq-ipc.schema.json`
- Client: `eq-copilot/plugin/src/PipeClient.h`, `PipeClient.cpp`
- Processor integration: `plugin/src/PluginProcessor.cpp`
- Framing: `broker/src/framing.rs`
- Messages/negotiation: `broker/src/protokoll.rs`
- Server dispatch: `broker/src/server.rs::verbindung_bedienen`
- Live state: [Broker sessions and aggregation](../broker/sessions-and-aggregation.md)

Focused validation combines:

```powershell
cargo test --manifest-path broker/Cargo.toml
py -3.13 tools/eq-copilot/pruefe_v2_schemas.py
```

With a broker running, `EqCopPipeProbe` exercises the real C++ client,
negotiated v2 ACKs, a compact measurement, and conflict creation/clearing.
There is no shared valid/invalid v2 corpus that proves full equivalence between
schema, C++ serialization, and Rust runtime checks.
