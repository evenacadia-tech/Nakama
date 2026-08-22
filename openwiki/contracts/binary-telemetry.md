---
type: Protocol Guide
title: Binary telemetry
description: Explains the contract-only FlatBuffers batch, semantic readers, field evolution, generated code, and cross-language evidence.
tags: [ipc, flatbuffers, telemetry]
sources:
  - id: openwiki-source-a1bfb386c47c8b27b3cbb6ff
    resource: repo://broker/src/telemetrie.rs
  - id: openwiki-source-6bf0862b85fceb2943431cf0
    resource: repo://broker/tests/contract_cross_language.rs
  - id: openwiki-source-f5d558bf608bb78fa425f80f
    resource: repo://eq-copilot/plugin/tests/SchemaTestMain.cpp
  - id: openwiki-source-4f45a0a768ecf1feb60d7d2b
    resource: repo://eq-copilot/plugin/vertrag/NakamaTelemetrie.cpp
  - id: openwiki-source-b4aa6d342c833b846b203596
    resource: repo://eq-copilot/schemas/v3/flatbuffers/FELD-IDS.json
  - id: openwiki-source-05ada47b7815ab9b9a80dd21
    resource: repo://eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs
  - id: openwiki-source-354d48e4a519e2dbf1225ae6
    resource: repo://eq-copilot/schemas/v3/flatbuffers/README.md
  - id: openwiki-source-bb8011d65e3d1ca7e008a187
    resource: repo://eq-copilot/schemas/v3/flatbuffers/WERKZEUG.json
  - id: openwiki-source-341631043a49666a9a225d12
    resource: repo://tools/eq-copilot/pruefe_fbs_feldids.py
  - id: openwiki-source-6800e857bf3a2264beb57f17
    resource: repo://tools/eq-copilot/pruefe_flatc_drift.py
generated: {by: "claude-code", at: "2026-08-22T15:50:39.855Z"}
verified:
  - by: openwiki/0.3.3
    at: 2026-08-22T15:50:39.855Z
---

# Binary telemetry

Binary telemetry is the P2 FlatBuffers side of the v3 family contract. It owns
the `.fbs` schema, frozen field-ID history, tool pin, committed C++ and Rust
bindings, semantic readers, and a shared binary corpus. It currently has no
production producer, consumer, 16-byte outer envelope, CRC32C parser,
persistence owner, or live backpressure behavior.

## Wire model

The root is `FeatureBatch` with file identifier `NKT3`. Every field has an
explicit numeric ID, and zero is reserved as the unknown/default value for
enums. Schema-major and schema-minor fields are intentionally left to a future
outer envelope rather than duplicated in the FlatBuffers payload.

A batch carries source identity, timing and derivation data, transport stamps,
feature frames, and band values. Each band must choose exactly one carrier,
declare an encoding and grid, provide the matching value count and validity
bitmap, and leave bitmap padding bits clear. One batch may contain several
sources, but duplicate `instance_id` values are invalid.

## Reader pipeline

Both language readers follow the same stages:

1. reject buffers shorter than the file-identifier precondition or without
   `NKT3`;
2. run the language's FlatBuffers verifier;
3. apply a structural supplement needed to equalize verifier behavior; and
4. apply semantic checks and return a sorted, duplicate-free violation set.

The C++ verifier does not validate string UTF-8 as strictly as Rust, so the C++
reader adds an explicit UTF-8 pass. The Rust verifier does not enforce every
offset property checked by C++, so the Rust reader adds `strukturriegel`.
These are compatibility corrections, not optional validation layers.

Semantic checks cover batch/source limits, strict identities, duplicate source
IDs, finite metrics, time and context relations, enum ranges, known validity
bits, derived bounds, carrier exclusivity, grid lengths, bitmap padding, and
quantization ranges.

## Evolution and generated code

Field IDs are the wire identity. `FELD-IDS.json` freezes active and burned IDs,
so removed IDs cannot be reused. `pruefe_fbs_feldids.py` enforces explicit and
contiguous IDs, history agreement, burned IDs, restricted schema constructs,
and structural-guard coverage for offset-bearing fields.

`WERKZEUG.json` is the common source for the FlatBuffers compiler
version/commit, C++ header and Rust crate expectations, generation arguments,
and target files. `pruefe_flatc_drift.py` locates the configured compiler,
verifies versions and IDs, regenerates both languages, and requires byte-for-byte
agreement with committed bindings. Generated files are evidence artifacts, not
hand-maintained implementations.

## Adoption boundary and unknowns

The validators are reached from tests only. Therefore the repository provides
no evidence for live ownership, queueing, backpressure, rate limiting,
persistence, or recovery of telemetry batches. Several conditional relations
are also explicitly deferred. Do not infer runtime guarantees from the schema
comments until a producer, consumer, and focused load/failure tests exist.

## Source map and validation

- Schema and evolution: `eq-copilot/schemas/v3/flatbuffers/`
- Generated bindings: `plugin/vertrag/generiert/` and `broker/src/generiert/`
- C++ reader: `plugin/vertrag/NakamaTelemetrie.*`
- Rust reader: `broker/src/telemetrie.rs`
- Binary corpus: `eq-copilot/fixtures/v3/flatbuffers/MANIFEST.json`
- Tooling: `pruefe_fbs_feldids.py`, `pruefe_flatc_drift.py`,
  `erzeuge_fb_fixtures.py`

`EqCopSchemaTest` and Rust `contract_cross_language` consume the same
handwritten manifest and compare both verdict and complete violation set. The
field-ID, code-generation drift, and fixture-regeneration tools close the
evolution loop. These checks establish cross-language contract agreement, not
production use.
