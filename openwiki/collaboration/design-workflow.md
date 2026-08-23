---
type: Workflow Guide
title: Design workflow
description: Explains how Figma evidence, user acceptances, living browser sheets, and validation tools become implementation-ready design truth.
tags: [collaboration, design, figma]
sources:
  - id: openwiki-source-63af4855f1326ea52bc527af
    resource: repo://design/abnahmen/2026-08-22-erfolgskriterium-exakt-wie-figma.md
  - id: openwiki-source-a3e7b9427467b6a1a7d95cb5
    resource: repo://design/abnahmen/2026-08-22-figma-depot.md
  - id: openwiki-source-1fec5dbb3c7e6508e5df036a
    resource: repo://design/abnahmen/LIES-MICH.md
  - id: openwiki-source-eecc525a3e21cd35616ab0cd
    resource: repo://design/assets/figma/LIES-MICH.md
  - id: openwiki-source-67cd3bebc4120709a33eba8e
    resource: repo://design/docs/arbeitsplan.md
  - id: openwiki-source-f7b501308aa49855daf474fe
    resource: repo://design/docs/interaktions-und-motion-spezifikation.md
  - id: openwiki-source-c2986483e5dcca20fd70a326
    resource: repo://design/docs/oberflaechen-spezifikation.md
  - id: openwiki-source-6d2cf17a390c97ede7b7186e
    resource: repo://design/LIES-MICH.md
  - id: openwiki-source-72fecea180aa55c1f523d84b
    resource: repo://design/prototyp/LIES-MICH.md
  - id: openwiki-source-897938d4b32c20613df314b4
    resource: repo://design/werkzeug/pruefung/pruefen.mjs
  - id: openwiki-source-45f03cf42412683cac56f96d
    resource: repo://design/werkzeug/pruefung/sondenprobe.mjs
generated: {by: "claude-code", at: "2026-08-23T10:03:23.427Z"}
verified:
  - by: openwiki/0.3.3
    at: 2026-08-23T10:03:23.427Z
---

# Design workflow

The `design/` tree translates user-owned Figma states into living browser
sheets. It does not own the plugin or broker implementation. Its job is to
preserve visual evidence, make hidden states and edge cases inspectable, and
turn explicit user judgments into constraints that downstream implementation
can safely follow.

## Sources and ownership

The external `Nakama-Design` Figma file is the current visual source. Only its
dark variants are binding today. Files under `design/assets/figma/` are dated
evidence snapshots, not a replacement source, and older snapshots remain as
history. Rejected directions and older visual material are reference evidence,
not current authority.

Figma defines visual direction and arrangement, not hidden technical behavior.
Specifications and accepted decisions supply state, semantics, and edge-case
requirements that a static image cannot express.

Repository responsibilities are deliberately separated:

- root `CLAUDE.md` owns product truth;
- `assets/figma/` preserves visual-source evidence;
- `abnahmen/` records binding user judgments;
- `docs/arbeitsplan.md` owns sequence and current work, while the surface and
  interaction specifications own content/state and gesture/motion details;
- `werkzeug/` contains measurement and falsification tools;
- `prototyp/` is the implementation surface for living design sheets.

An acceptance is binding only when it records the user's wording, the
constraint derived from it, and what remains unresolved. A provisional
assumption needs an explicit verification point and cannot be cited as an
accepted decision.

## Translation loop

Current Phase 1b proceeds in the order Gen, Probeeq, then Suna. Each app is
compared with its specification and accepted decisions, reproduced at 1:1
accepted dimensions, and captured for direct screenshot comparison with the
current Figma export. It is then given switchable states, stressed at named
edge cases, and accepted independently.

```mermaid
flowchart LR
    F[Figma state] --> C[Compare source and acceptances]
    C --> Q{Conflict or missing content?}
    Q -->|yes| U[Ask the user and record acceptance]
    Q -->|no| L[Build living sheet]
    U --> L
    L --> S[Switch states and stress edges]
    S --> V[Browser and semantic probes]
    V --> A[Per-app acceptance]
    A --> I[Later plugin implementation]
```

A new Figma state restarts the comparison at the source. When it conflicts
with a recorded acceptance, or required content is absent from the image, the
translator raises the mismatch rather than inventing a resolution.

## Interaction and motion handoff

`docs/interaktions-und-motion-spezifikation.md` complements the current Figma
states without changing their layout or visual language. It defines how the
visible controls and states react: direct band dragging, draft/audition/apply
transitions, Gen's measurement-to-advice flow, and Suna's compact status
behavior. It may not introduce measurements or product states that are absent
from the owning surface specification and technical contracts.

The visual presentation uses an approximately 140 ms ease-out without spring
or bounce. That number is not an audio contract. Parameter ramps and DSP
crossfades remain tied to measured technical behavior, while Figma variants or
motion demos show the intended visible transition. Before implementation
handoff, the required interaction sequences still need explicit visible start
and end states in the design evidence.

## Exactness boundary

The acceptance target is the result inside FL Studio, not an intermediate
gallery or browser approximation. Static shells, material, wordmarks, and
possibly glows may be exported per scale. Dynamic text, curves, cells,
controls, and states remain rendered.

Text rasterization can differ among Figma, browsers, and plugin graphics.
Position, size, spacing, and color remain measurable requirements. If zero
raster difference becomes a requirement, text may need to be baked into an
asset and accepted as such.

## Validation tools

Run the generic living-sheet check from the repository root:

```powershell
node design/werkzeug/pruefung/pruefen.mjs
node design/werkzeug/pruefung/pruefen.mjs --gegenprobe
```

The tool distinguishes a missing Playwright installation from a page failure.
It checks browser errors, silent or empty pages, visible failure bands,
screenshots, and four deliberate counterexamples.

The measurement-sheet probe adds semantic and interaction assertions:

```powershell
node design/werkzeug/pruefung/sondenprobe.mjs alles
```

It covers measured numbers, overflow, state matrices, reversible actions,
export truth, and failure-inducing counterprobes. When a living app sheet is
added under `prototyp/`, extend these probes with its accepted states and edge
cases rather than relying on screenshots alone.

## Change safely

For each new visual state, add a dated export and ledger entry, record each
new user judgment as its own acceptance, and update the current work sequence.
Do not treat hook gates as design authority; hook orchestration is documented
in [session automation](session-automation.md).
