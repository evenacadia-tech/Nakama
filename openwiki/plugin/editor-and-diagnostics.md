---
type: Architecture Guide
title: Editor and diagnostics
description: Explains the current JUCE editor, visible failure states, marking controls, and snapshot export.
tags: [plugin, ui, diagnostics]
sources:
  - id: openwiki-source-6d2cf17a390c97ede7b7186e
    resource: repo://design/LIES-MICH.md
  - id: openwiki-source-1c70526be5b18770605eb2fa
    resource: repo://eq-copilot/plugin/src/PluginEditor.cpp
  - id: openwiki-source-d0ecbf97882d0c9c6a3699ee
    resource: repo://eq-copilot/plugin/src/PluginEditor.h
  - id: openwiki-source-5a8cd5b65a1c7fa70f8bf898
    resource: repo://eq-copilot/plugin/src/PluginProcessor.cpp
  - id: openwiki-source-ceaaacbb11099235777aaced
    resource: repo://eq-copilot/plugin/tests/PaintBenchMain.cpp
  - id: openwiki-source-54e8631ccef9272699b25214
    resource: repo://eq-copilot/plugin/tests/ShotTestMain.cpp
generated: {by: "claude-code", at: "2026-08-22T15:50:39.855Z"}
verified:
  - by: openwiki/0.3.3
    at: 2026-08-22T15:50:39.855Z
---

# Editor and diagnostics

`EqCopilotEditor` is the current Material-based JUCE surface. It reads copied
measurement, pipe, and state snapshots; it does not own mutable analysis or
persistence. Persistent actions delegate to `EqCopilotProcessor`, while
display choices and marking latches remain transient editor state.

## Snapshot-driven UI

A 30 Hz timer acquires one measurement snapshot per tick, derives display state
from that copy, samples pipe and read-only status, and runs diagnosis at 1 Hz.
The editor repaints only when a measurement revision, UI toggle, pipe state, or
visible message changes. Spectrum smoothing is presentation-only: invalid bands
remain gaps, interpolated display segments do not create new peaks, and static
chrome is cached separately from changing data.

The editor enforces a fixed aspect ratio within its supported resize bounds.
Connection state, measurement readiness, analysis FIFO drops, and non-finite
sample counts are rendered as visible diagnostics rather than hidden logs.

## Identity, state, and marking controls

Duplicate sensor identity opens a dedicated panel that lets the user give this
instance a new persistent ID. Read-only state blocks binding edits and explains
why the original state is being preserved. Binding fields cap labels and pair
IDs, submit only actual changes, and rely on
[State and identity](state-and-identity.md) for dirty and reconnect behavior.

Audible marking freezes the selected finding's parameters at engagement. Pulse
depth is limited using the measured true peak, and invalid finding values are
rejected before an order reaches the processor. Editor close disables marking;
deadman, sample-rate, realtime, transport, and offline-render rules provide
additional termination or authorization gates. The audio-side rules are
documented in [Plugin audio runtime](audio-runtime.md).

## Snapshot export

Export refuses when no measurement exists. A successful file contains snapshot
format 3, measurement and sensor fields, and diagnoses produced from the same
copied `MessSnapshot` used elsewhere. Non-finite numeric values become JSON
`null`, and `raw_audio` is always `null`.

Files are written beneath the Windows local application-data
`evenacadia/EQ-Copilot/snapshots` directory with timestamped, sanitized names.
Directory and write failures return text to the editor. The schema fixes the
machine-readable shape, but the schema checker validates the schema document;
it does not validate each newly exported file at runtime.

## Current UI versus design authority

This page describes the editor that exists in code. The repository's active
design workflow declares user Figma exports as the visual source for the three
future Nakama applications. Therefore Material assets and the current JUCE
surface are not the authority for future appearance. See
[Design workflow](../collaboration/design-workflow.md).

## Source map and validation

- Editor state and controls: `eq-copilot/plugin/src/PluginEditor.h`,
  `PluginEditor.cpp`
- Processor actions and export: `PluginProcessor.cpp::schreibeSnapshotDatei`
- Current assets: `EqCopilotAssetKit.h`, `LeitstandTokens.h`
- Export contract: `eq-copilot/schemas/eq-snapshot.schema.json`
- Visual tools: `tests/ShotTestMain.cpp`, `tests/PaintBenchMain.cpp`

`EqCopShot` feeds the real processor and proves that an offscreen PNG can be
written, including a read-only-state scenario. It does not compare pixels.
`EqCopPaintBench` reports average, percentile, and extreme paint timing but has
no pass/fail budget. Neither tool establishes interactive DAW behavior,
accessibility, or fidelity to the approved Figma design.

