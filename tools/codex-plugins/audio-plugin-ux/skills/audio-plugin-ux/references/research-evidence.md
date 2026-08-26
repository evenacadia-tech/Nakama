# Research evidence for audio-plugin UX

Use this reference when a repository contains a maintained knowledge archive or the user supplies research. It defines how to retrieve and transfer evidence; it deliberately does not duplicate the archive's findings.

## 1. Find the archive, then narrow the read

Prefer the repository's declared knowledge entrypoint over an ad hoc web search. Read its operating rules and index first, then only the active entries that bear on the current decision.

For Nakama, the route is:

1. `wissen/LIES-MICH.md` — purpose and single-source rule;
2. `wissen/AGENTS.md` — ingestion, authority, visual-evidence, and verification rules;
3. `wissen/INDEX.md` — current topic map and active-entry links;
4. the relevant topic router and active entries named by that index.

Do not hard-code today's dated entry filenames into this skill. A future entry becomes available through the index without a skill release. If an index entry is inactive, superseded, missing, or in conflict, report that state rather than silently treating it as current.

Choose entries by the decision at hand:

- interaction assumptions, mental models, recovery, and accessibility -> UX;
- hierarchy, controls, visual evidence, material, and motion -> UI and Motion plus UX;
- render cadence, JUCE painting, threading, and test method -> Engineering plus the human-facing UX consequence;
- migration, host lifecycle, automation, state restore, and compatibility -> FL Studio plus Engineering and UX;
- perceptual claims and audible evaluation -> DSP plus UX;
- product purpose, terminology, or ownership -> Product, then verify against the current product contract.

## 2. Keep four kinds of truth separate

Research answers what was observed or argued in a source and why it may matter. It does not own the current product.

1. **Source claim** — what the cited paper, talk, manual, or experiment actually supports, including population, context, method, limitations, and the archive's verification level.
2. **Current product contract** — what current code, tests, explicit user decisions, behavioral contracts, and approved visual sources require.
3. **Transfer inference** — why the source may apply to this product. This is a hypothesis until the contexts are shown to match or a product authority adopts it.
4. **Design decision** — the chosen consequence, its owner, and the observable test that could prove it wrong.

A section titled “Bedeutung für Nakama” is still a transfer analysis. It does not become binding merely because it lives in the repository. Confirm every volatile Nakama statement against the canonical source routed by `wissen/INDEX.md`.

When sources disagree, retain the disagreement in the ledger. Code and tests own implemented behavior; current explicit user decisions own product intent; current behavioral contracts own observable interaction; approved visual sources own their stated pixels; research supplies external evidence and human context.

## 3. Build the human causal chain

Do not translate a paper into a list of features or “best practices.” For each relevant finding, write:

**person and situation -> goal, risk, or uncertainty -> perceptual or interaction need -> design or engineering consequence -> observable human outcome**

Ask at each arrow:

- Did the source establish this link?
- Does the source population and context resemble the target users and host workflow?
- Is the proposed consequence the only plausible response, or one testable option?
- What would a user do, notice, understand, hear, avoid, or recover from if it worked?

Mark unsupported links **Inferred**. A familiar icon, “up means more,” a fashionable gesture, or another plugin's convention is not a universal law. Preserve an explicit user-decided mapping, but test comprehension and error rate in its actual context before calling it self-evident.

## 4. Treat visual examples as evidence, not templates

Every visual example used in reasoning needs:

- source and stable URL or repository path;
- page, figure, frame, or video timestamp;
- an objective observation of what is visible;
- the source's own claim, if any;
- a separate interpretation of the mechanism and why it may transfer;
- the mismatch risks between the example and the target plugin.

Do not infer hidden interaction, motion, hierarchy, or designer intent from one screenshot. Do not copy color, shape, material, or layout merely because the example is respected. Compare the human problem, sonic object model, available surface, and host state first.

## 5. Convert evidence into a falsifiable decision

Use a compact ledger:

| Entry ID/path | Verification level | Source claim | Current contract | Transfer inference | Proposed consequence | Observable proof |
|---|---|---|---|---|---|---|

The proof must match the claim:

- a visual golden or geometry check can prove pixels and stable loci, not audible continuity;
- a Web prototype can prove state, gesture, and motion logic, not VST3 lifecycle or DSP safety;
- a native host run can prove focus, automation, scale, multi-instance, and lifecycle behavior;
- an audio comparison can prove signal continuity or output equivalence under a defined tolerance;
- a user comprehension or task test can challenge a mapping, label, icon, or recovery assumption.

For performance claims, measure before prescribing a remedy. Record build type, OS, host, scale, instance count, the worst supported analyzer/visual load, UI frame/paint cost, audio-thread health, and the visible or audible symptom. Include idle and direct manipulation explicitly. Separate static and dynamic paint work, cache only after profiling, invalidate it deliberately, and check child-component behavior. Every performance recommendation must state both human consequences: prompt control feedback must let the user attribute the audible change to the current gesture and authoritative state, while idle editors must release host resources. The goal is not a low repaint count in isolation.

For a Web-to-native migration, preserve the current reference while replacing one bounded slice at a time. Keep UI state/motion contracts separate from parameter IDs, normalized ranges, preset/session state, automation gestures, audio behavior, and host lifecycle. Gate each slice with the proof appropriate to that boundary, including Windows Release builds in the target host, relevant scales, multiple instances, automation, save/reload, reopen, bypass, and audio continuity. A pixel match does not close those nonvisual contracts.

## 6. Stop conditions

If the archive has no evidence for a consequential claim, label it **Open** instead of filling the gap with folklore. If research and a current product contract conflict, do not silently overwrite either. If the task is only research ingestion, update the archive under its own rules; if the task is design, consume the archive through the index and leave the source knowledge in one place.
