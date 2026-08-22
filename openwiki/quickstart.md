---
type: Overview
title: Nakama quickstart
description: Orients contributors to the current plugin and broker runtime, prepared family contracts, delivery evidence, and collaboration workflows.
tags:
  - overview
  - architecture
  - navigation
sources:
  - id: openwiki-source-6d4b4e707b8d60b6ccfa3425
    resource: repo://.github/workflows/openwiki-update.yml
  - id: openwiki-source-8cd33e1d9dedf73d03c17132
    resource: repo://briefing-hub/.openai/hosting.json
  - id: openwiki-source-d9d84bd60a961e5bd3a44f28
    resource: repo://briefing-hub/db/index.ts
  - id: openwiki-source-8a90364cb31264aedf742997
    resource: repo://briefing-hub/package.json
  - id: openwiki-source-e583d5ab37a07999439f7776
    resource: repo://broker/Cargo.toml
  - id: openwiki-source-e8645ee57595da4a277f52d3
    resource: repo://broker/src/main.rs
  - id: openwiki-source-993537de92f3e3f3ef6c12f0
    resource: repo://broker/src/server.rs
  - id: openwiki-source-6d2cf17a390c97ede7b7186e
    resource: repo://design/LIES-MICH.md
  - id: openwiki-source-71e7d4e3896d39625d69a0a7
    resource: repo://eq-copilot/plugin/CMakeLists.txt
  - id: openwiki-source-f780b20513754b3fdca3ce27
    resource: repo://eq-copilot/plugin/src/EqCopilotIds.h
  - id: openwiki-source-b9bdb2eeb341991ef4dd964b
    resource: repo://eq-copilot/plugin/src/PipeClient.cpp
  - id: openwiki-source-5a8cd5b65a1c7fa70f8bf898
    resource: repo://eq-copilot/plugin/src/PluginProcessor.cpp
  - id: openwiki-source-837d41a393a054d97a0b2271
    resource: repo://eq-copilot/schemas/eq-ipc.schema.json
  - id: openwiki-source-354d48e4a519e2dbf1225ae6
    resource: repo://eq-copilot/schemas/v3/flatbuffers/README.md
  - id: openwiki-source-66e75bcd24466a9b101d850a
    resource: repo://eq-copilot/schemas/v3/README.md
  - id: openwiki-source-23775c3de52f3ab95a13cb8b
    resource: repo://README.md
  - id: openwiki-source-b9796d70c5f0967a458166b1
    resource: repo://tools/beweise.ps1
  - id: openwiki-source-62c36f02c52e1a4c49232f4f
    resource: repo://tools/hub/hub_sync.py
generated: {by: "claude-code", at: "2026-08-22T15:50:39.855Z"}
  by: claude-code
  at: "2026-08-22T15:50:39.855Z"
verified:
  - by: openwiki/0.3.3
    at: 2026-08-22T15:50:39.855Z
---
# Nakama quickstart

Nakama is an FL Studio mixing-advice plugin family: **Nakama Gen** is the main
application, **Nakama Probeeq** is the active full EQ, and **Nakama Suna** is
the passive probe. Together they form the planned Nakama Studio bundle.
Current code, bundle, and pipe identifiers still use the legacy EQ-Copilot and
`EqCop*` names.

## What exists today

The current native product build creates one `EqCopilot` VST3 analyzer. It
links `EqCopilotProcessor`, the current editor, `PipeClient`, the analysis
engine, and diagnosis logic. A standalone Rust broker owns the Windows named
pipe, live sensor register, derived sessions and pairs, profile bindings, and
aggregate export.

The repository also contains the deployed briefing application, the active
Figma-to-browser design workflow, delivery evidence, and versioned cross-language
contracts. These collaboration and contract systems support product work but
are not additional shipped audio plugins.

```mermaid
flowchart LR
    Host[FL Studio audio callback] --> Plugin[EqCopilot VST3]
    Plugin --> FIFO[Lock-free analysis handoff]
    FIFO --> Analysis[Analysis worker and snapshots]
    Analysis --> Editor[Current editor]
    Analysis --> Client[Compact v2 heartbeat]
    Client --> Pipe[Windows named pipe]
    Pipe --> Broker[Rust broker register]
    Broker --> Sessions[Sessions, pairs, bindings, aggregates]
```

The plugin observes host audio, preserves the dry path unless an audible
marking is explicitly authorized, and publishes analysis snapshots outside the
audio callback. `PipeClient` sends compact measurements using production JSON
protocol v2. The broker validates framing and messages before admitting them
to its in-memory register.

## Current versus prepared contracts

Production IPC is the framed JSON v2 exchange described in
[runtime protocol v2](contracts/runtime-protocol-v2.md). The strict v3 JSON
family in [family protocol v3](contracts/family-protocol-v3.md) and the
FlatBuffers [binary telemetry](contracts/binary-telemetry.md) surface are
prepared, independently validated contracts without production callers. Do
not infer runtime adoption from the existence of schemas, fixtures, generated
bindings, or validators.

The same distinction applies to host evidence. The wrapper bridge and
capability reports are real and validated, but the product processor does not
yet consume bridge payloads or the capability file. See
[host capabilities](delivery/host-capabilities.md).

## Find the right page

For plugin runtime work:

- [Plugin audio runtime](plugin/audio-runtime.md) — host callback, transparent
  audio, lock-free handoff, worker cadence, and audible marking.
- [Analysis engine](plugin/analysis-engine.md) — measurements, active-time
  windows, evaluation, snapshots, and deterministic diagnoses.
- [State and identity](plugin/state-and-identity.md) — state schema, migration,
  read-only preservation, runtime identity, host-dirty signaling, and the
  future parameter inventory.
- [Editor and diagnostics](plugin/editor-and-diagnostics.md) — current editor,
  visible failure states, snapshot export, and headless visual tools.

For broker work:

- [Broker service lifecycle](broker/service-lifecycle.md) — process startup,
  named-pipe security, framing, connection workers, and shutdown.
- [Sessions and aggregation](broker/sessions-and-aggregation.md) — live sensor
  ownership, duplicate conflicts, session/pair derivation, bindings, and
  aggregate persistence.

For shared contracts:

- [Runtime protocol v2](contracts/runtime-protocol-v2.md) — adopted JSON IPC,
  negotiation, runtime guards, failure handling, and compatibility limits.
- [Family protocol v3](contracts/family-protocol-v3.md) — contract-only JSON
  families, text guard, fixtures, canonical violations, and evolution rules.
- [Binary telemetry](contracts/binary-telemetry.md) — FlatBuffers wire model,
  field-ID history, cross-language readers, and generated-code drift.

For construction and evidence:

- [Build and proof](delivery/build-and-proof.md) — dependency pins, version
  gates, target map, validation commands, freshness, and evidence manifests.
- [Host capabilities](delivery/host-capabilities.md) — JUCE patch, realtime
  bridge, disposable probes, FL Studio reports, and adoption boundary.

For human and agent collaboration:

- [Design workflow](collaboration/design-workflow.md) — Figma evidence,
  acceptances, living browser sheets, edge cases, and design validation.
- [Briefing application](collaboration/briefing-application.md) — deployed
  vinext/React UI, D1 persistence, API routes, and trust boundary.
- [Briefing synchronization](collaboration/briefing-sync.md) — repository Hub
  authority, pull/incorporate/validate/send ordering, and consistency checks.
- [Session automation](collaboration/session-automation.md) — Claude Code
  primers, gates, reminders, stop handling, and generated handoffs.

## First validation route

Use focused commands from the owning page while iterating. For a repository
evidence run, the canonical entrypoint is the freshness-aware local runner in
`tools/beweise.ps1`; it can build the proof targets and append raw output,
environment provenance, review fields, and one verdict to a named manifest.
The generated OpenWiki workflow maintains these documentation pages and does
not replace product proof.

When changing a prepared v3 contract, run its independent C++, Rust, and
Python validation path before claiming compatibility. When changing deployed
briefing behavior, distinguish D1 application state from repository
`docs/hub/hub.json` and use the explicit synchronization workflow.
