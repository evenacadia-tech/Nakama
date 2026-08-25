# Audio Plugin UX benchmark

## Target card

- Target: Codex plugin and skill
- True source: `C:/Users/phili/plugins/audio-plugin-ux`
- Durable baseline snapshot: `C:/Users/phili/plugins/audio-plugin-ux-workspace/baseline-0.2.1`
- Mode: fix
- Must-read files: plugin manifest, skill instructions, three references, agent metadata
- Evaluation classes: first-response/routing, runtime proof, plugin metadata
- Provider: deterministic current-versus-candidate assertions
- Constraint: no subagents; the active session forbids delegation unless the user asks for it

The review in `C:/Users/phili/Projekte/Nakama/docs/reviews/2026-08-25-audio-plugin-ux.md` is the first RED baseline. A byte-for-byte content snapshot of the installed 0.2.1 source before the Voger refactor is the durable baseline for this iteration; it lives in the benchmark workspace so plugin cache cleanup cannot invalidate later runs. The benchmark does not re-grade accepted generic research; it targets documented regressions and concrete newly identified gaps.

## Eval cases

1. **E1 — Current routes:** Nakama analysis names the current primary files instead of saying only “refresh the repo.”
2. **E2 — No stale product:** the adapter states two apps, 760×430, 8 × 13, and the closed sidechain decision; it rejects the old Suna/700×420/12-parameter/advisory-only claims.
3. **E3 — Generative fixed surface:** the generic reasoning derives and measures a layout hypothesis before audit/polish.
4. **E4 — Screenshot omissions:** the Pro-Q study models mixed values, instance/track identity, and co-located scales.
5. **E5 — User laws and proof:** Nakama names no-dead-elements, material geometry invariance, rectangle proof, contract check, and release-bound audition.
6. **E6 — Executable drift check:** the plugin ships a read-only contract checker for the critical Nakama anchors.
7. **E7 — Current visual authority:** the adapter names the three exact 2026-08-25 reference images and the acceptance decision that gives them priority.
8. **E8 — Visual contract check:** the executable checker guards both the new image set and the updated design router.
9. **E9 — Access and response contract:** the entrypoint requires alternative input paths, assistive semantics, response under visual load, and interruption discipline.
10. **E10 — Executable review prompts:** the reasoning reference provides concrete input-equivalence, response-under-load, and interruption tests without misapplying a web pixel threshold as a native-plugin law.

Run:

```powershell
pwsh -File C:/Users/phili/plugins/audio-plugin-ux-workspace/benchmark.ps1
```

## Results

### Iteration 0 — stale Nakama adapter to 0.2.1

- Installed 0.1.0 baseline: **0/8**
- Source 0.2.1 candidate: **8/8**
- Nakama repository contract check: **30/30 checks passed**

The baseline score does not mean its accepted generic analysis is worthless; these eight cases deliberately target the review's missing and unsafe behavior plus the newly declared visual truth.

### Iteration 1 — Voger source review

- Installed 0.2.1 baseline: **8/10**
- Source 0.2.2 candidate: **10/10**
- Value-add: E9 and E10 now pass; E1–E8 did not regress.

The Voger article's simplicity, consistency, feedback, and screen-size points were already covered. The refactor adds only the gaps it exposed: alternative/assistive operation, response truth under visual load, and interruption/focus discipline. Its named plugin examples remain editorial examples rather than causal evidence.
