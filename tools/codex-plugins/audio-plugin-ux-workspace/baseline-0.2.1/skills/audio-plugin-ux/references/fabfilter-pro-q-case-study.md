# FabFilter Pro-Q case study: focus, placement, and disclosure

Use this as a studied interaction case, not a visual template. It is based on the two user-supplied Pro-Q interface states and FabFilter's Pro-Q 4 documentation. Where FabFilter documents behavior but not design intent, the rationale below is explicitly an inference.

## What the two supplied states show

### Resting state

The display is almost entirely the acoustic work surface. A yellow aggregate response curve remains prominent over a subdued spectrum. Frequency occupies the horizontal axis, gain the vertical axis, and output level remains at the right edge. Global navigation and modes sit in thin top and bottom rails. No band-precision panel competes with the curve.

**Focus:** the current total EQ result in relation to the incoming signal.

### Selected/editing state

Individual bands become distinct colored objects with nodes and filled influence regions. A compact control panel appears at the bottom, spatially under the selected band group. Frequency, gain, Q, filter shape/slope, dynamics, routing, navigation, bypass, and deletion become available for the current selection. The total response and spectrum remain visible behind them.

Three quieter details matter beyond the main panel:

- `Bell` with `(multiple)` marks a multiselection whose members do not reduce to one ordinary value.
- The right edge carries two co-located measurement scales: EQ gain for the response curve and analyzer level for the spectrum.
- `● Guitar` in the lower rail preserves track/instance context while the user edits an object inside it.

**Focus:** the selected band as part of the total result, not a detached parameter record.

FabFilter documents that the large interactive display fills the window and that the floating band controls appear under selected bands. Clicking the background deselects and hides those controls. See [Overview](https://www.fabfilter.com/help/pro-q/using/overview), [Display and workflow](https://www.fabfilter.com/help/pro-q/using/eqdisplay), and [Band controls](https://www.fabfilter.com/help/pro-q/using/bandcontrols).

## What can appear, disappear, dim, or persist

| Element | Trigger | Behavior | Reason supported by the workflow |
|---|---|---|---|
| Floating band controls | Select one or more bands | Appear under the selection; hide on deselect | Precision is local to an owned object and absent from the resting view |
| Band parameter display | Move near a band | Appears next to the node; can be disabled | Exact values are transient evidence, not permanent labels |
| Curve preview | Hover the display | Shows the curve that a click/drag would create | Teaches the action before commitment without adding permanent chrome |
| Selected band shapes | Select bands | Highlight while the total curve remains | Separates object focus from whole-result context |
| Mixed-value marker | Select bands whose parameter values differ | Shows `(multiple)` rather than inventing one shared value | Multiselection remains truthful and can be edited without misreading one member as the group |
| Dynamic controls | Give a supported band dynamic range | Additional controls become available; the detailed panel expands explicitly | Complexity follows capability; ordinary static EQ work is not burdened |
| Analyzer panel | Hover its bottom-bar entry | Pops up; click can pin it | Quick access for short inspection, persistence for a longer analyzer task |
| Output panel | Hover the output entry | Pops up; click can pin it | Global controls remain at the global edge without occupying the canvas by default |
| Spectrum Grab | Dwell/hold in the spectrum | Spectrum freezes, existing bands dim, peaks become targets; normal state returns after the action | The temporary task changes from editing bands to selecting spectral evidence |
| Solo/band listen | Hold solo near a band | Other bands and total response dim; heard region and affected range are exposed | Visual and auditory attention share the same selected region |
| Output meter | User option | Can be shown or hidden | Metering is useful evidence but not mandatory for every EQ task |
| Track/instance identity | Work in an instance-aware context | `● Guitar` remains in the lower rail | The user retains which processing context owns the selected bands while local controls change |
| EQ-gain and analyzer-level scales | Response curve and spectrum share the display | Two differently ranged scales occupy the same edge | Canvas is conserved, but unit and visualization ownership must remain distinguishable |
| Output pan and mode | Stereo context | Visible only where the channel layout makes them meaningful | Avoids controls that promise an impossible action |
| Q or gain-related controls | Filter-shape/slope context | Unavailable where the DSP meaning does not exist | Capability and control visibility remain truthful |
| Undo/redo and A/B | Always in top rail, enabled when meaningful | Stable recovery/comparison locus | Reversibility and comparison are part of expert iteration |

Documented behaviors come from [Band controls](https://www.fabfilter.com/help/pro-q/using/bandcontrols), [Dynamic EQ](https://www.fabfilter.com/help/pro-q/using/dynamic-eq), [Spectrum analyzer](https://www.fabfilter.com/help/pro-q/using/analyzer), [Spectrum Grab](https://www.fabfilter.com/help/pro-q/using/spectrumgrab), [Solo](https://www.fabfilter.com/help/pro-q/using/solo), [Output options](https://www.fabfilter.com/help/pro-q/using/output), and [Undo/A-B](https://www.fabfilter.com/help/pro-q/using/undoredo).

## Why the main controls are where they are

The locations are documented; the explanations in this section are **inferred** from the interaction and human-factors evidence.

### The band node is the first control

Dragging the represented band changes frequency and gain directly; the mouse wheel or modified gesture changes bandwidth. The object, control, and result share the same coordinates. That removes a translation step between "the resonance near here" and a remote form field.

### Frequency, gain, and Q form the central triad

- **Frequency** answers where in the spectrum the band acts and corresponds to horizontal position.
- **Gain** answers how strongly and in which direction it acts and corresponds to vertical displacement. It also carries the dynamic-range ring, so it receives greater visual/motor capacity.
- **Q** answers how wide the affected region is and corresponds to the visible width of the curve.

Their grouping is not merely conventional. They jointly define the selected band's most common audible geometry and can be checked continuously against the curve.

### Filter shape and slope sit before the triad

They define what kind of object the band is and which of the core dimensions are meaningful. Placing identity/geometry before the continuous triad makes the panel read from object definition into fine adjustment. This is a medium-confidence inference, not a quoted FabFilter rationale.

### Stereo placement sits after band shaping

Routing answers who receives the band rather than what the band shape is. Keeping it on the other side of the core triad preserves a scope boundary inside the local panel.

### Bypass, delete, and band navigation occupy the panel's perimeter

These act on the selected object as a whole. They are accessible without looking like another continuous sound-shaping parameter. Delete is separated from the high-frequency knobs, and undo remains at a stable global locus.

### The floating panel is under the selected object, not in a permanent inspector

This likely reduces pointer and eye travel, strengthens ownership, and preserves most of the spectral canvas. Anchoring it near the bottom avoids covering the selected node in many ordinary cases. Edge selection, multiple selection, and occlusion still require explicit testing; proximity is not automatically correct.

### Multiselection has an indeterminate value state

`(multiple)` is not decoration and not a third filter type. It prevents the inspector from claiming that different selected objects share one value. A target plugin borrowing multiselection must define this state control by control, including what happens when the user drags, enters a number, resets, or selects objects for which a capability is only partly available.

### Instance identity stays outside band precision

`● Guitar` is observed as a persistent context label in the lower rail. It is the closest visible analogue in the supplied state to an instance or source switcher: it answers which larger processing context owns the current local edit. This does not prove that another product should copy the locus or mechanism; it proves that object focus does not remove the need for context identity.

### Two scales share one edge

The response curve and analyzer spectrum reuse the right edge but do not use the same unit range. This is an information-density tactic with a reading risk. A transfer must preserve unit, tick, grid, and visualization ownership without relying on color alone, and must test the smallest supported surface where the scales are most likely to collapse into visual noise.

## Why hiding works here without becoming mystery meat

Pro-Q combines disclosure with multiple discoverability aids:

- the sonic objects remain visible as nodes and curves;
- hover previews show what can be created;
- nearby value displays expose exact state;
- controls light or highlight on approach;
- common tasks have both direct gestures and precision controls;
- panels can be pinned for longer work;
- undo and A/B make exploration recoverable;
- collapsed entries retain active-state markers;
- unsupported controls are not presented as if they could work.

The tradeoff is real: hidden features are less discoverable. Pro-Q pays that cost to preserve the work surface and mitigates it with consistent triggers, previews, shortcuts, and a layered learning path. Do not copy the hiding pattern unless the target can provide equivalent signposting and state truth.

## The critical lesson

The interface is simple in the resting state, not simple in capability. Complexity is attached to the object and moment where it becomes relevant. The central display remains a continuous representation of the audible operation; secondary controls orbit that representation.

FabFilter also cautions that analyzer collision indications are not definitive and that the engineer should decide by ear. That boundary matters: visualization supports listening rather than replacing it. See [Spectrum analyzer](https://www.fabfilter.com/help/pro-q/using/analyzer).

## Confidence ledger

- **High:** documented show/hide triggers, direct gestures, panel locations, pinning, contextual availability, undo/A-B behavior.
- **High:** observed resting versus selected focus in the supplied images.
- **Medium:** explanations for exact left-to-right knob order and the panel's bottom anchor.
- **High:** observed mixed-value marker, track/instance context, and co-located scales in the supplied selected state.
- **Low unless user-tested:** whether the same pattern is right for a different plugin, user population, or constrained host surface.
