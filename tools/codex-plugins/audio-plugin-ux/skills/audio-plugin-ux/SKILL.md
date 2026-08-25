---
name: audio-plugin-ux
description: Analyze, audit, or design desktop audio-plugin UX from the engineer's listening workflow, sonic object model, control mapping, state, and host constraints. Use for VST, AU, AAX, JUCE, EQ, dynamics, effects, plugin Figma work, control placement, focus, progressive disclosure, or explaining why a knob belongs in a particular place. Do not use for ordinary websites or DSP-only implementation with no interface decision.
---

# Audio Plugin UX

Treat audio-plugin design as a domain to investigate, not an intuition Codex already owns. A plausible-looking panel is not evidence of a usable instrument.

## Epistemic contract

Tag important claims as one of:

- **Observed**: visible in a supplied interface, recording, or running build.
- **Documented**: stated by a manual, product contract, source, or test.
- **Inferred**: a reasoned explanation that has not been confirmed by the designer or users.
- **User-decided**: an explicit current decision by the user.
- **Open**: evidence is insufficient or sources conflict.

Never turn an inference into the designer's intent. Replace words such as "intuitive", "clean", or "obvious" with a concrete mechanism and cost.

## Load the relevant model

- Before evaluating or changing an audio-plugin layout, read [references/design-reasoning.md](references/design-reasoning.md).
- For equalizers, FabFilter Pro-Q comparisons, contextual controls, or show/hide behavior, also read [references/fabfilter-pro-q-case-study.md](references/fabfilter-pro-q-case-study.md).
- For Nakama Gen, Probeeq, or the historical Suna surface, also read [references/nakama-transfer.md](references/nakama-transfer.md). Follow its explicit repository routes and run its contract check before using Nakama-specific facts.

## The primary loop

Model the work as:

**listen -> orient -> choose a sonic object -> manipulate -> hear and see the consequence -> compare -> keep, revise, or undo**

The visual hierarchy must protect this loop. Navigation, settings, branding, and parameter inventory are supporting systems. Do not let them become the main surface merely because they are easy to arrange.

An audio plugin differs from a web page in ways that change layout decisions:

- It is a repeatedly played instrument inside a host, often in a small fixed window.
- The user's attention is split between ears, plugin, DAW, transport, and other tracks.
- Many operations are continuous, fine-grained, reversible gestures rather than page navigation.
- Parameters may be automated externally while the window is open; state identity and feedback must remain truthful.
- The processed signal, selected object, and audible result are more important than a complete on-screen parameter catalog.
- Expert repetition makes motor path, stable mapping, fast comparison, and low visual interruption consequential.

## Required preflight before a new layout or substantial redesign

Do not edit Figma or production UI until the following compact reasoning brief exists:

1. **Task frequency**: the engineer's common, occasional, and recovery jobs.
2. **Sonic object map**: what is manipulated, what it affects, and what is global versus local.
3. **Focus statement**: the one object or decision that owns the resting state.
4. **Visibility matrix**: persistent, contextual, transient, exceptional, unavailable, and user-pinned elements across real states.
5. **Placement proofs** for every primary control or control group.
6. **Source ledger**: observed, documented, inferred, user-decided, conflicting, and open claims.
7. **Information budget**: the real target width, height, aspect/reflow behavior, reserved chrome, minimum targets, and evidence density before controls are arranged.
8. **Host-size and audio-state checks**: smallest supported size, no-signal, playback, bypass, automation, single selection, multiselection with mixed values, instance/context switching, and error/degraded states.
9. **Access and response contract**: alternative input routes, focus behavior, assistive semantics, interaction feedback, interruption policy, and response under the worst visual load.

For a new design, do not stop at critique. Derive one falsifiable layout hypothesis from the task ranking, object ownership, visibility classes, and measured surface budget. State what remains persistent, what is selected into view, what can collapse, and what the design deliberately refuses to show simultaneously. Then measure the hypothesis at the real host surface before polishing it.

If the user only requests analysis, stop at the brief. If implementation is requested, use the brief as the design contract and verify the implemented transitions, not just a resting screenshot.

## Access and response contract

- Document a pointer, keyboard, fine-adjustment, and text-entry route for each primary action whose meaning does not inherently depend on a freehand path. Include reset, cancel, undo, and focus return. Do not force a path-dependent gesture into a misleading keyboard clone; provide an equivalent endpoint operation where practical.
- Expose each interactive custom control's semantic name, role, value, state, and actions through the native framework's accessibility API. Tooltips and visible labels do not by themselves make an unlabeled custom widget operable by assistive technology.
- Define the immediate acknowledgment and authoritative state under the worst visual load: maximum analyzers, active objects, overlays, automation, and scaling. Throttling decorative or analytical redraw is acceptable; letting the visible parameter value lag ambiguously behind audible or host state is not.
- Defer noncritical pop-ups, update notices, upsells, and onboarding while the user is dragging, auditioning, comparing, or entering a value. A scoped safety or connection failure may interrupt when continued work is impossible or unsafe; restore focus to a predictable owner afterward.
- Measure targets and spacing at the smallest supported editor size and relevant OS/DAW scale. Treat web accessibility thresholds as comparison evidence, not as unexamined native-plugin pixel laws.

## Placement proof

For every important knob, node, slider, toggle, or panel, answer:

1. What listening decision or recovery action does it serve?
2. Which sonic object owns it: band, source, stage, plugin output, or host/session?
3. Is it primary, conditional, global, destructive, or diagnostic?
4. What conceptual, spatial, movement, or signal-flow mapping justifies its position?
5. In which states must it appear, remain visible, dim, disable, or disappear?
6. What concrete cost appears if it moves elsewhere: longer motor path, broken ownership, extra eye travel, occlusion, false signal order, unstable motor memory, or accidental activation?
7. What evidence supports that answer, and how strong is it?

Run the **swap test**: exchange it with the neighboring group. If no task, mapping, or error cost gets worse, the original position is not yet justified.

## Disclosure and focus rules

- Preserve the sonic object and audible consequence continuously where the task requires orientation.
- Reveal precision controls next to the selected object's locus when that improves ownership and reach without hiding essential evidence.
- Reveal capability-specific controls only after the capability exists. Hidden inactive complexity is different from hidden active state.
- Keep global safety and truth signals discoverable even while their detailed settings are collapsed.
- Dim unrelated content only for a specific focus operation, and never so far that the user loses the context needed for judgment.
- Keep active hidden states legible at their collapsed entry point.
- Keep the currently edited instance, source, track, or processing path legible whenever several similar objects can be controlled from one surface.
- Represent mixed values honestly in multiselection; do not replace disagreement with an arbitrary member's value.
- Allow expert persistence where useful, such as pinning an on-demand panel, without making the expanded state the default for everyone.
- Do not use color as the only carrier of selection, bypass, stale data, or danger.

## Validation

Inspect and test state transitions, not only static polish:

- idle -> hover/preview -> selected -> dragging/fine adjustment -> deselected
- normal -> audition/solo -> release
- static -> dynamic/advanced -> collapsed while still active
- one object -> multiselection -> object navigation
- one instance/source -> another -> optional comparison overlay -> return
- enabled -> bypassed -> restored
- internal edit -> host automation or preset change -> undo/A/B
- signal -> no signal -> stale/degraded/error
- nominal size -> smallest supported host size and scaling
- pointer/direct gesture -> alternative input route -> predictable focus return
- nominal visual load -> worst visual load -> host automation or state change
- uninterrupted listening gesture -> noncritical notice arrives -> gesture completes without focus theft

Run the interface with audio in the real host when possible. A visual-only prototype cannot prove whether focus supports listening, whether a gesture masks the audible comparison, or whether momentary audition releases correctly.

## Output

Lead with the actual focus and visibility conclusion. Then provide the placement proofs, state matrix, tradeoffs, and open questions. Separate what the evidence proves from what remains a design hypothesis.

Do not imitate FabFilter's colors, shapes, or arrangement as a style kit. Transfer the interaction reasoning only when the target's sonic objects, jobs, and constraints support the same conclusion.
