# Transfer to Nakama

Use this file as a stable repository router and reasoning adapter for Nakama. It is not a product snapshot and must not become a second copy of current decisions or research findings.

## Required preflight

Work from the Nakama workspace root: the directory containing `CLAUDE.md`, `DESIGN.md`, `UX-CONTRACT.md`, `design/`, `docs/`, `wissen/`, and `tools/`.

Run the read-only structural check before Nakama-specific analysis or design:

```powershell
pwsh -File <directory-of-this-SKILL.md>/scripts/check-nakama-contract.ps1 -RepoRoot .
```

Replace `<directory-of-this-SKILL.md>` with the actual directory from which this skill was loaded. Do not assume a user profile, plugin cache, or checkout path.

Exit code `0` means the current routers, knowledge-index structure, and adapter anchors were found. It does not certify that every product statement is current. On a non-zero result, report the failed checks and repair the route or archive structure before giving Nakama-specific advice.

## Read current truth through routers

Start with the narrowest current owner instead of a dated list embedded here:

1. `CLAUDE.md` for product scope, explicit user decisions, and authority rules.
2. `DESIGN.md` and `design/LIES-MICH.md` for the current visual authority, design routes, and approved goldens or Figma sources.
3. `UX-CONTRACT.md` for approved observable interaction and state behavior.
4. The newest task-relevant file under `design/abnahmen/` and any implementation rule it explicitly routes to.
5. Current source and tests for implemented behavior, parameter identity, serialization, host automation, threading, and DSP truth.
6. `docs/PLAN-STAND.md` and the task-relevant technical router for implementation status. Treat generated plan files as read-only.
7. `wissen/LIES-MICH.md`, `wissen/AGENTS.md`, and `wissen/INDEX.md` for external evidence and its human why; then read only the active entries relevant to the decision.
8. `tools/beweise.ps1` and the newest matching evidence manifest when implementation proof is required.

Never use the plugin cache, this adapter, an old review, `docs/archiv/`, or historical app wording as current product authority. Old files may explain why a decision changed, but they do not silently regain ownership.

## Route knowledge by the decision

Use `wissen/INDEX.md` dynamically; do not memorize dated entry filenames.

- user goals, mental models, interaction assumptions, accessibility, failure, and recovery -> `wissen/ux/`;
- hierarchy, controls, material, lighting, motion, and visual fidelity -> `wissen/ui-motion/` plus UX;
- JUCE rendering, UI-thread behavior, performance, architecture, and test method -> `wissen/engineering/` plus the experienced consequence;
- FL Studio lifecycle, automation, scaling, state restore, compatibility, and migration continuity -> `wissen/fl-studio/` plus Engineering and UX;
- perception, measurement, audible comparison, and DSP claims -> `wissen/dsp/` plus UX;
- product purpose and terminology -> `wissen/produkt/`, then verify the current decision in the product owner routed by `wissen/INDEX.md`.

Record each used entry's ID, repository path, and `pruefstand`. A “Bedeutung für Nakama” section is a transfer hypothesis unless a canonical current source separately confirms it.

## Authority and conflict rules

- Code and tests own implemented behavior.
- Explicit current user decisions and their authorized registers own product intent.
- `UX-CONTRACT.md` owns approved observable UI behavior.
- The current visual sources routed by `DESIGN.md` and `design/LIES-MICH.md` own the pixels and editable design authority they explicitly claim.
- The indexed knowledge archive owns the collected external evidence and human reasoning, not Nakama's binding behavior.
- A Web prototype owns only the state, gesture, and motion behavior actually accepted from it. It does not own host, parameter, state-restore, DSP, or audio-thread truth.

Attach a current repository path to every volatile product claim. When sources conflict, name the exact conflict and resolve it by scope and recency. Prefer current code/tests for implementation truth and newer explicit user wording for product intent. Ask only when the unresolved difference would materially change the result.

## Derive the object and transaction model

Do not reuse the product model from memory. Read the current contracts and derive:

1. the system and host context;
2. each independently owned processing path or source;
3. the selected sonic object or group;
4. the listening and edit transaction states;
5. which authority can change each state: UI gesture, host automation, remote component, preset/session restore, or DSP.

For every visible control or status, create an action/state/material ledger:

- **action** -> real command, target scope, success, failure, cancel, and recovery;
- **state** -> authoritative source, freshness, unavailable behavior, and refresh path;
- **material** -> intentionally non-interactive product identity with no false affordance.

If a current Nakama contract imposes a stricter law such as no dead elements, fixed geometry across states, momentary release, protected ranges, or explicit confirmation, cite that current contract and add the corresponding transition test. Do not claim the law from this adapter alone.

## Build the human reasoning brief

Before changing Figma, the Web simulator, or native UI, write one compact chain for each primary job:

**person and situation -> listening goal or risk -> perceptual/interaction need -> proposed UI consequence -> observable proof**

Then add:

- task frequency and recovery cost;
- current object ownership and edit authority;
- persistent, contextual, transient, exceptional, unavailable, and pinned information;
- a measured information budget from the current host surface and scaling contract;
- the worst truthful state derived from current limits rather than old numbers;
- a source ledger that separates research claims, current contracts, user decisions, observations, and hypotheses.

Icons, directions, gestures, and competitor conventions remain contextual hypotheses. Preserve an explicit user decision, but do not call a mapping universal or obvious without evidence from the target workflow.

## Compare references without importing their product

When using Pro-Q or another plugin, compare only mechanisms that solve the same human problem under compatible constraints. Check sonic-object ownership, available surface, automation/state model, target users, and recovery cost before transferring contextual disclosure, direct manipulation, comparison traces, or selection behavior.

Every visual example needs a source path or URL plus page, figure, frame, or timestamp. Separate objective observation, source claim, and Nakama interpretation. A screenshot cannot establish hidden states, motion, audible behavior, or designer intent.

## Preserve continuity from Web reference to native VST3

Treat migration as several contracts, not a visual rewrite:

- **UI reference:** geometry, text, states, gestures, timing, visual hierarchy, and accepted motion;
- **parameter contract:** IDs, normalized ranges, defaults, units, gesture begin/end, automation, and ownership;
- **state contract:** presets, session serialization, migrations, undo, drafts, and recovery;
- **audio contract:** DSP output, bypass, ramps, latency, channel layouts, and real-time safety;
- **host contract:** FL Studio scan/open/close, focus, scaling, multiple instances, reopen, automation, and project reload.

Keep the accepted Web simulator and previous working native slice available as comparison evidence while replacing one bounded surface or interaction family at a time. For each slice:

1. freeze the explicit design acceptance, current Web contract, and approved image/golden relevant to that slice;
2. implement native rendering and input without changing parameter or audio contracts unless separately authorized;
3. compare pixels and motion against the accepted reference;
4. verify VST3 scan plus Windows Release behavior in FL Studio at every supported scale, with multiple instances, automation, preset and project save/reload, reopen, bypass, and interrupted transactions;
5. run the relevant state and audio proofs before removing the old slice.

Explicit design approval remains **User-decided**. Approved images and pixel diffs prove visual fidelity only; the Web RC can additionally prove its accepted interaction and motion behavior. Neither proves VST3 scan, host lifecycle, parameter identity, preset/project state, automation, or audio compatibility. Practice-report recommendations remain **Source claims**, and Nakama-specific transfers from them remain **Inferred** until a canonical current contract adopts them. Host lifecycle, parameter continuity, and sound require native host and audio evidence. Cache or repaint prescriptions require profiling under a named build, host, scale, instance count, worst supported analyzer/visual load, direct manipulation, idle state, and failure symptom; folklore is not a performance diagnosis.

## Executable completion gate

A Nakama pre-design brief is complete only when:

1. `check-nakama-contract.ps1` exits `0` against the current workspace.
2. Each volatile product claim cites its current canonical path.
3. Each research-derived consequence cites the indexed entry ID/path/verification level and labels unsupported transfer links as hypotheses.
4. The real target surface and worst truthful state are measured from current contracts.
5. Every visible element has an action, state, or material role and every primary action has a recovery route.
6. Source identity, edit target, audible state, automation authority, and stale/unavailable state remain truthful through the tested transitions.
7. Visual, Web-interaction, VST3 scan/native-host, preset/project state, automation, and audio claims are closed only by matching evidence.
8. Closed decisions are not presented as open questions; genuine conflicts and unknowns are labeled **Open**.

If implementation exists, add the narrowest proof to the repository's established evidence workflow. Do not invent a command or claim that one proof type establishes a different boundary.
