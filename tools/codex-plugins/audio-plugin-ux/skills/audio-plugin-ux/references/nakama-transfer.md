# Transfer to Nakama

Use this file as a repository router and constraint adapter for Nakama Gen and Probeeq. It is not a dated product snapshot. Do not cite a Nakama fact from this file without reading the adjacent repository source.

## Required preflight

Work from the Nakama workspace root: the directory containing `CLAUDE.md`, `design/`, `docs/`, and `tools/`.

Run the read-only contract check before Nakama-specific analysis or design:

```powershell
pwsh -File <directory-of-this-SKILL.md>/scripts/check-nakama-contract.ps1 -RepoRoot .
```

Replace `<directory-of-this-SKILL.md>` with the actual directory from which
this skill was loaded. Do not assume a user profile, plugin cache, or Nakama
checkout path.

Exit code `0` means the required source routes and contract anchors were found. Any non-zero exit means the Nakama adapter has drifted: report the failed checks, read the newer decisions, and update this adapter before giving product-specific layout advice.

Then read, in this order:

1. `CLAUDE.md` — compact current truth, authority rules, and design source.
2. `design/abnahmen/2026-08-25-rework-referenz-drei-designs.md` — newest binding visual authority and its three exact image paths.
3. `design/abnahmen/2026-08-23-gen-eq-zentrale.md` — binding user wording for the architecture cut.
4. `design/docs/ui-spielregeln-eq-zentrale.md` — numbered implementation constraints and proposal/assumption labels.
5. `design/abnahmen/2026-08-25-seitenverhaeltnis-bleibt-fest.md` — fixed-aspect information constraint.
6. `design/abnahmen/2026-08-24-seite2-groesse-keine-toten-elemente.md` — equal page size and the no-dead-elements law.
7. `design/abnahmen/2026-08-24-offene-figma-entscheide.md` — closed Undo/sidechain decisions; its Suna visual basis is superseded.
8. `docs/bauaufteilung-sonden.md` — current surface/build allocation; distinguish current sections from historical narrative.
9. `design/LIES-MICH.md` — current visual router and hard design rules.
10. `docs/PLAN-STAND.md` — generated current plan state; read only, never hand-edit.
11. `docs/review-2026-08-25-audio-plugin-ux.md` — failure analysis for this adapter, including the material law.

Read newer, more specific acceptance files when they affect the task. Never use `briefing-hub/`, `docs/archiv/`, historical Suna wording, old Figma nodes `25:444`/`28:994`/`6:2864`, or the plugin cache as current product authority.

## Authority and epistemic rules

- Current code and tests own implemented behavior.
- Binding user decisions require user wording in `design/abnahmen/` or an explicitly authorized register.
- `design/docs/ui-spielregeln-eq-zentrale.md` translates those decisions into technical UI constraints and labels unaccepted additions as **(Vorschlag)** or **(Annahme)**.
- The newest explicit user visual decision owns appearance and product identity. The current authority is the three user-supplied PNGs under `design/assets/rework-basis-2026-08-25/`; prior Figma nodes are history until a newer user decision promotes another source. Pro-Q may inform handling, never Nakama's style.
- Closed decisions stay closed unless a newer explicit user decision supersedes them. Do not turn an open item into a fact, and do not reopen a closed item as a design question.
- Attach a repository path to every volatile product claim. Do not add a date unless the cited source carries that date.

When sources conflict, name the conflict and prefer current source/tests for implementation truth and newer explicit user wording for product decisions. Ask only when the conflict would materially change the design and cannot be resolved from the routed sources.

## Current product contract to verify, not memorize

These statements summarize the routed sources and must be refreshed before use:

- The current visual truth in progress is exactly three images: `design/assets/rework-basis-2026-08-25/gen-page-1-overview.png`, `design/assets/rework-basis-2026-08-25/gen-page-2-eq-center.png`, and `design/assets/rework-basis-2026-08-25/probeeq.png`. They are the basis for the future large rework and supersede earlier Figma/Gen/Probeeq/Suna visuals (`design/abnahmen/2026-08-25-rework-referenz-drei-designs.md`).
- Nakama has two apps with one design identity: **Gen** is the hub/studio and EQ center; **Probeeq** is the single bus probe and executes its switchable EQ. Suna has been absorbed into Probeeq and is no longer an app name (`CLAUDE.md`; `design/abnahmen/2026-08-23-gen-eq-zentrale.md`).
- Gen page 2 centrally controls the probes and contains a full master EQ. The law is **nothing unasked**: processing occurs only when enabled; “Gen only advises” is obsolete (`CLAUDE.md`; `design/abnahmen/2026-08-23-gen-eq-zentrale.md`).
- The probe EQ runs on the buses, while Gen remotely controls it. Probeeq's compact current surface is the supplied `probeeq.png`, not the former Suna tile or old full Probeeq editor (`design/abnahmen/2026-08-25-rework-referenz-drei-designs.md`; `docs/bauaufteilung-sonden.md`).
- Gen pages 1 and 2 are designed at **760×430**. The editor may scale, but the product currently keeps a fixed aspect ratio: enlargement buys zoom, not more information area (`design/abnahmen/2026-08-25-seitenverhaeltnis-bleibt-fest.md`).
- Each EQ path has 8 band slots × 13 stored parameters and 5 global host parameters. `sidechain_source` exists with default `none` but is hidden in the first release; U5/NAK-33 is closed (`design/docs/ui-spielregeln-eq-zentrale.md`; `design/abnahmen/2026-08-24-offene-figma-entscheide.md`).
- The source switcher lives directly on Gen page 2. An optional second EQ trace shares the same graph, and the edit target must remain unambiguous (`design/abnahmen/2026-08-23-gen-eq-zentrale.md`; `design/docs/ui-spielregeln-eq-zentrale.md`).
- Advisor preview is **hold to audition** and must end on release; it is not a click-to-latch control. Apply/confirm/reject are separate states (`design/docs/ui-spielregeln-eq-zentrale.md`, rules 10–14 and 37).

## Where Pro-Q stops being the model

Pro-Q is useful for direct manipulation, selected-band ownership, contextual precision, mixed-value display, instance identity, and keeping the aggregate response visible. It does not solve Nakama's core arrangement:

- two independently owned EQ paths overlaid in one graph;
- one surface controlling remote probe instances plus Gen's master path;
- a one-click source switcher on the same EQ page;
- 8 × 13 band state plus global state on a 760×430 fixed-aspect information field.

Therefore do not copy Pro-Q's panel geometry and assume disclosure will fit. Pro-Q can gain information area as its work surface grows; Nakama currently cannot. Transfer the reasoning, then re-budget the surface.

Do not redesign the three current images during context loading. First inventory what they visibly establish, then use the generic reasoning only to expose unanswered transitions and future rework risks. “In progress” permits later revision by the user; it does not permit an agent to merge them with an older favorite.

## Nakama sonic-object and identity model

Model at least four nested scopes:

1. **System context:** Gen, broker/link truth, host transport, and current project.
2. **EQ path:** Gen master or a named Probeeq bus instance.
3. **Band or band selection:** one locus, several selected loci, or a mixed-value group.
4. **Listening transaction:** confirmed, draft, held preview, temporary apply, reject/revert, host-automated, stale, or unreachable.

The selected EQ path is not generic navigation metadata. It determines which remote DSP state a gesture changes. Keep the path name and edit ownership legible in resting, selected, comparison, lost-link, and return states. With two traces visible, distinguish all three facts without color alone: which trace belongs to which path, which trace is audible, and which trace will receive the next edit.

## Generate the first layout hypothesis at the real surface

Do not begin with 109 controls or a copied inspector. Produce one measured hypothesis:

1. Rank the listening tasks: switch source, orient in the aggregate result, select/edit a band, compare a second path, audition, apply/reject, bypass/undo, and recover from stale or lost link.
2. Reserve persistent pixels for the graph, active path identity, trace ownership/edit target, bypass/transaction truth, and scoped exceptions.
3. Put band precision into a contextual region owned by the selected band or group. Define mixed/indeterminate values before styling multiselection.
4. Reveal dynamics or other capability-specific controls only when meaningful, while leaving an active collapsed marker.
5. Keep the source switcher one action away on page 2; do not route through overview.
6. Account for both EQ-gain and analyzer-level scales if they share an edge. Unit, ticks, grid, and ownership must remain readable without color.
7. Build the worst truthful state at 760×430: eight active bands, longest labels/values, second trace visible, mixed selection, dynamic state active, and one scoped exception.
8. State which information is deliberately not simultaneous, then measure occlusion, clipping, hit targets, control-box stability, and graph area.

`design/werkzeug/sonde-messung.html` is useful as a measurement-method specimen and worst-state harness. Its embedded 700×420 and 8 × 12 inventory are historical; never copy those numbers into the current Gen design. `design/werkzeug/formfaktor.html` likewise supplies measurement technique, not authority over newer decisions.

## Two binding Nakama UI laws

### No dead elements

“Keine toten Elemente” means every visible element either performs a real action or reports an honest state at the correct scope. Decorative controls, duplicated status words, false affordances, and labels with no information role fail. Material such as shell, grain, and light edge is identity rather than a control, but must not pretend to be interactive (`CLAUDE.md`; `design/abnahmen/2026-08-24-seite2-groesse-keine-toten-elemente.md`).

For each visible element record one of:

- action -> real handler/command and reachable result;
- state -> real source, scope, refresh/failure behavior;
- material -> explicitly non-interactive visual identity.

Anything with no row is removed before polish.

### Material geometry is invariant across state

A button is a material object, not text that changes width when its label or state changes. Idle, hover, pressed, selected, armed, and disabled states keep the same outer width, height, and learned locus. Communicate state through fill, light, shadow, outline, or a contained inner transform rather than changing layout dimensions (`docs/review-2026-08-25-audio-plugin-ux.md`, binding user law recorded under B9).

In a running HTML/prototype check, capture each repeated control's outer `getBoundingClientRect()` in every state and compare `x`, `y`, `width`, and `height`. Use `offsetWidth`/`offsetHeight` as an independent layout-size check when transforms exist. A visual inner transform must not move the outer hit target or its neighbors.

## Nakama placement-proof additions

For each control also state:

- owning EQ path and whether it is master, probe, selected group, or global;
- current DSP, draft, audition, temporary apply, confirmed, comparison, or visualization-only state;
- remote-command authority, host-automation identity, stale/conflict behavior, and lost-link behavior;
- behavior during hold, release, apply, confirm, reject, undo, source switch, reload, and second-trace entry/exit;
- whether a collapsed entry still explains active sound;
- whether moving the control adds source-switch clicks, obscures trace ownership, or destabilizes a repeated motor path.

## Executable completion gate

A Nakama pre-Figma brief is complete only when:

1. `check-nakama-contract.ps1` exits `0` against the current workspace.
2. Every volatile fact in the brief has a current repository path.
3. The 760×430 worst truthful state is measured, not asserted.
4. Every visible element passes the action/state/material ledger.
5. Repeated control rectangles are invariant across interactive states.
6. Source identity, edit target, audible path, and second-trace ownership remain unambiguous without color alone.
7. Hold-to-audition ends on pointer/key release and cannot latch by click.
8. Closed decisions are not presented as questions; genuine open decisions are labeled **Open**.

If implementation exists, add the narrowest relevant proof to the repo's established evidence workflow. Read `tools/beweise.ps1` and the newest matching manifest in `docs/beweise/` rather than inventing a command or claiming a visual prototype proves host/audio behavior.
