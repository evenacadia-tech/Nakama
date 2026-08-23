---
type: Architecture Guide
title: Analysis engine
description: Explains measurement ownership, publication cadences, derived findings, and numerical failure semantics.
tags: [plugin, analysis, diagnostics]
sources:
  - id: openwiki-source-e210bd27e224e2cb1ed8b94d
    resource: repo://eq-copilot/plugin/src/AnalyseEngine.cpp
  - id: openwiki-source-e0c81eaf4ac4d859fae75152
    resource: repo://eq-copilot/plugin/src/AnalyseEngine.h
  - id: openwiki-source-f4a6ee23e1805858effa3fe0
    resource: repo://eq-copilot/plugin/src/Diagnose.cpp
  - id: openwiki-source-5a8cd5b65a1c7fa70f8bf898
    resource: repo://eq-copilot/plugin/src/PluginProcessor.cpp
  - id: openwiki-source-9a52c3877c1658a9868e49c2
    resource: repo://eq-copilot/plugin/tests/GoldenTestMain.cpp
generated: {by: "claude-code", at: "2026-08-23T16:26:37.107Z"}
---

# Analysis engine

`AnalyseEngine` is independent of the editor and is mutated by exactly one
thread: `EqCopilotProcessor::workerLauf`. Consumers receive copied
`MessSnapshot` values. Snapshot acquisition is the only engine operation
designed for arbitrary threads, so editor paint and broker reporting never
inspect mutable accumulators.

## Measurement lifecycle

Preparation builds four Welch resolutions for different frequency ranges. A
sample-rate change resets accumulated measurement data; repeated preparation at
the same rate is a no-op. The usable LTAS range ends at the lower of 18 kHz or
95 percent of Nyquist. Bands beyond that cap stay invalid instead of receiving
fabricated edge values.

Ingestion performs the ongoing work:

- non-finite analysis samples become zero and increment `nanErsetzt`;
- peak, loudness, mid/side, correlation, and spectral accumulators advance;
- mono input duplicates left into the analysis right channel while stereo-only
  metrics retain their validity distinction; and
- zone persistence advances once per second of active audio, so pauses do not
  repeatedly count the last visible spectrum.

`MessSnapshot` is the versioned publication contract. It carries measurement
state, active and total time, spectral curves, loudness and stereo metrics,
resonance candidates, zone history, convergence information, revision,
processed-sample count, and replaced non-finite count.

## Light and heavy publication

The light path publishes live curves and fast scalar values at the worker
cadence, but only after new samples. It sets readiness to no data, collecting,
or ready; readiness begins after 15 seconds of active material. The heavy path
finalizes loudness, LTAS, coverage, percentiles, resonances, zones, and
convergence, then replaces the complete guarded snapshot and increments its
revision.

LTAS gaps are interpolated in linear power before conversion to decibels. The
multi-resolution curves are blended only within their intended seams, and the
Nyquist cap remains invalid throughout finalization.

## Findings and confidence

Resonance discovery runs only on ready, valid LTAS. A candidate must pass local
excess, minimum separation, maximum bandwidth, persistence, and reliable
coverage gates. At most six candidates enter the measurement snapshot.

`diagnose` is a deterministic pure consumer of a snapshot. It emits nothing
before readiness, turns at most the two strongest resonances into cards, adds
other supported spectral findings, applies stable priority, and marks no more
than three non-low-confidence findings as top-ranked. The editor and snapshot
export call the same function, preventing two competing diagnosis paths.

This engine is not the only measurement path in the tree. The newer
[Measurement core](measurement-core.md) serves the versioned family contracts
with its own window bookkeeping and boundary rules; the two currently coexist,
and this one remains what the editor and the offline reference use.

## Persistence and failure behavior

Accumulation lives in memory and resets through the engine API. Host state does
not contain analysis history. A user-triggered export in
[Editor and diagnostics](editor-and-diagnostics.md) is a separate JSON
artifact built from one copied snapshot.

Invalid or unavailable evidence is represented explicitly:

- non-finite input is counted and isolated from accumulators;
- unmeasurable frequency bands remain NaN in memory and become `null` at JSON
  export;
- findings are suppressed before sufficient material; and
- idle publication does not fabricate new revisions.

## Change surfaces

- Add accumulators in `AnalyseEngine::verarbeite` without moving work into the
  host callback.
- Put cheap live fields in `auswertenLeicht` and expensive coherent derivations
  in `auswerten`.
- Extend `MessSnapshot` when a new immutable consumer-facing measurement is
  required.
- Change shared spectral semantics through `ZonenRegeln.h` and bump the
  corresponding metrics/diagnosis contract when meaning changes.

## Source map and focused validation

- Types and ownership: `eq-copilot/plugin/src/AnalyseEngine.h`
- Accumulation/finalization: `AnalyseEngine.cpp` — `verarbeite`,
  `finalisiereLtas`, `auswertenLeicht`, `auswerten`, `findeResonanzen`
- Findings: `Diagnose.h`, `Diagnose.cpp` — `diagnose`
- Shared zone rules: `ZonenRegeln.h`
- Upstream owner: `PluginProcessor.cpp::workerLauf`
- Main evidence: `tests/GoldenTestMain.cpp`, `fixtures/golden-referenz.json`,
  `tools/eq-copilot/erzeuge_fixtures.py`

`EqCopGoldenTest` exercises deterministic signal anchors, false-positive
behavior, Nyquist invalidation, exact non-finite counts, zone timing, light
revision publication, and idle non-publication. The fixtures provide tested
anchors and tolerances, not a proof of numerical correctness for every signal
or sample rate. Full reference regeneration can also depend on an analyzer that
is not stored in this repository.

