# Audio-plugin design reasoning

Use this reference before judging or changing the layout of a desktop audio plugin.

## 1. Design the listening loop, not a parameter page

The central unit of work is an auditory decision. A useful control shortens or clarifies the loop between an intention, a gesture, the changed sound, the visible consequence, and a reversible judgment.

Website conventions start from navigation, content hierarchy, completion, and page transitions. An audio plugin starts from a live process whose state may change continuously and externally. A fixed sidebar, dashboard grid, or complete form can be appropriate, but it needs an audio-task reason; familiarity from web design is not enough.

Use six interacting maps:

1. **Sonic map**: source -> processor/stage -> band/module -> output.
2. **Perceptual map**: what the engineer listens for and what visual evidence supports or risks biasing that judgment.
3. **Control map**: gesture direction, range, precision, reset, fine mode, text entry, and audible feedback.
4. **State map**: selection, engagement, automation, bypass, comparison, signal validity, and host context.
5. **Access map**: pointer, keyboard, focus order, text entry, alternative routes, assistive semantics, and scaling.
6. **Response map**: input acknowledgment, visible update cadence, audible state, host automation, and behavior under maximum visual load.

A layout is strong when these maps agree. A control that belongs to one band should not look global. A global output control should not sit inside one band's visual territory. A horizontal frequency gesture should not produce a conceptually unrelated change.

## 1A. Generate a layout hypothesis from constraints

An inventory is not a layout. Before drawing a component tree, convert the inventory into one testable arrangement:

1. Rank listening tasks by frequency, time pressure, and recovery cost.
2. Name the sonic object that owns each task and separate per-object state from global, host, and diagnostic state.
3. Classify every item as persistent, contextual, transient, conditional, exceptional, pinned, or unavailable.
4. Measure the real surface: target width and height, fixed or free aspect ratio, reflow behavior, reserved chrome, minimum hit targets, and the pixels left for acoustic evidence.
5. Reserve the persistent work surface first. Place contextual precision beside its owner only after the result, identity, safety, and recovery truths have space.
6. Write one primary layout hypothesis and its deliberate omissions. If everything is claimed to be simultaneously visible, the hierarchy has not been designed.
7. Build the worst truthful states at the target size: maximum active objects, longest values, mixed selection, advanced capability active, exception visible, and comparison mode.
8. Measure occlusion, clipping, target stability, and eye/pointer travel. Revise until the stated constraints pass, then validate the listening transitions in a host.

Scaling is not automatically more information. A fixed-aspect interface that enlarges every element buys zoom; a reflowing or freely growing work surface may buy information area. State which behavior the product has before borrowing disclosure logic from a larger reference interface.

## 1B. Accessibility, response, and interruption

Accessibility is part of control truth, not a final contrast pass. For each primary action, distinguish the direct gesture from its outcome. A freehand curve may genuinely depend on a pointer path; band selection, numeric gain entry, bypass, reset, apply, reject, and source switching usually have endpoint meanings that can support keyboard or text alternatives. Define the focus order from sonic-object ownership and task sequence, and return focus to the invoking object after a temporary panel or dialog closes.

Custom-rendered controls also need programmatic semantics. JUCE's `AccessibilityHandler` exposes a control's role, title/description/help, current state, supported actions, and value interface. Use that native route for JUCE custom components, or the equivalent API in another framework; a painted knob with a tooltip is not sufficient evidence that assistive technology can identify or operate it.

Target size must be evaluated in the real editor coordinate system and at the actual OS/DAW scale. WCAG 2.2 uses **24 CSS pixels** as a web minimum with spacing and equivalent-control exceptions. Do not transplant that number as a literal native audio-plugin law. Use it as a documented risk comparator, then record the plugin's logical units, physical scaling, target spacing, precision mode, and equivalent routes and test them with the intended users and host sizes.

Response is temporal state truth. Under maximum analyzers, objects, overlays, scaling, and incoming automation, the gesture needs prompt acknowledgment and the displayed value must remain attributable to the audible or host-authoritative state. Reduce analyzer frame rate or decorative animation before allowing a control readout to drift ambiguously. If evidence becomes stale, report that state instead of presenting an old visualization as current.

Interruptions have scope. Noncritical marketing, update, onboarding, and informational pop-ups should not steal focus during dragging, audition, comparison, or text entry. A clipping, link-loss, or invalid-state exception may displace lower-priority content when continued work would be false or unsafe; preserve the interrupted transaction where possible and return focus predictably.

## 2. Focus layers

### Resting focus

Show what answers "what is the plugin doing to the sound now?" The answer is often a result curve, gain reduction history, transfer function, modulation field, impulse shape, or another domain representation. It is not automatically a hero illustration.

### Object focus

After selection, make ownership explicit. Keep enough surrounding context to compare the active object with the whole result. Local precision controls may appear near the object if they do not cover the evidence being judged.

### Auditory focus

Solo, audition, band-listen, or delta modes temporarily change what is heard. The UI must show both that the listening contract changed and what region/object is being exposed. Momentary controls should release immediately and visibly.

### Exception focus

Clipping, missing signal, stale analysis, lost pairing, invalid automation, or an unavailable channel layout may need to interrupt the normal hierarchy. The exception should speak at the scope where it exists: band, source, plugin, or host connection.

### Instance and context identity

A shared editor may control several tracks, sources, plugin instances, or processing paths. The current target is then part of the sonic object, not navigation metadata. Keep its identity visible in the resting and editing states, give switching a stable locus, and make comparison overlays distinguishable from the edit target. A user must be able to answer both “what am I hearing?” and “which instance will this gesture change?” before acting.

### Mixed values in multiselection

When selected objects disagree, the UI has a real third value state: mixed or indeterminate. Show that state explicitly per control. A new gesture may set all selected objects to one value; reset should restore the documented baseline or each object's prior value according to the product contract. Never display one selected object's value as if it represented the group. Preserve partial capability differences too: a control may be mixed, unavailable for part of the selection, or capable only after narrowing the selection.

## 3. Visibility classes

Classify each element by behavior, not by component type.

| Class | Purpose | Typical treatment |
|---|---|---|
| Persistent | Orientation, current result, safety, or global truth | Always visible but not necessarily loud |
| Contextual | Precision for the selected sonic object | Appears on selection near its owner |
| Transient | Preview, value readout, shortcut hint, or momentary evidence | Appears on hover, gesture, or hold |
| Conditional | Exists only when a capability/state exists | Reveal after activation; preserve a collapsed active indicator |
| Exceptional | Failure, stale data, conflict, or risk | Appears at the correct scope and displaces lower-priority content if necessary |
| Pinned | User promotes an on-demand panel for a longer task | Persists by explicit choice and remains dismissible |
| Unavailable | Not meaningful in the current DSP/channel state | Hide when absence is unambiguous; otherwise disable with a reason |

Do not confuse "not currently needed" with "not currently active." Hiding an active process without a persistent marker makes the plugin lie by omission.

## 4. Why position is not arbitrary

Evaluate placement in this order:

### Ownership

Put a local control inside the selected object's territory or make the relationship visually explicit. Distance weakens perceived ownership and requires mental recoding.

### Natural mapping

Align movement and display consequences where the domain supports it. In a parametric EQ, horizontal position naturally represents frequency and vertical position represents gain. Width represents bandwidth. Direct manipulation makes these relationships inspectable and reversible.

### Workflow order

Neighbor controls that are commonly used in one listening decision, but do not force a sequence that the audio task does not have. Frequency, gain, and bandwidth form a band-shaping triad; routing and global output belong to different scopes.

### Signal topology

Input, per-object processing, and output may benefit from spatial order when it clarifies signal flow. Do not use signal-flow order merely as decoration if the actual workflow jumps between stages.

### Motor path and stability

Frequently repeated adjustments need reachable targets and stable locations. Pointing time generally rises with distance and falls with target size; contextual proximity can therefore matter. Do not let a popover chase the cursor unpredictably or reorder controls when state changes.

### Occlusion and attention

Local controls save travel but can cover curves, meters, or neighboring objects. Place them in a reserved or low-information region, reposition only by a stable rule, and verify edge cases.

When one physical edge carries two measurement systems, such as EQ gain and analyzer level, treat it as a co-located-scale problem. Different units, ranges, grid emphasis, and labels must remain attributable without color alone. Test whether either scale can be read incorrectly as governing the other visualization, especially at the smallest size.

### Material and geometry stability

State styling should not silently change a control's layout box. Hover, pressed, selected, bypassed, and armed states may change fill, light, shadow, outline, or a contained transform, but repeated targets should keep a stable locus and footprint unless resizing is itself the explicit interaction. Measure rectangles across states instead of trusting visual similarity.

### Risk

Separate destructive actions from high-frequency gestures, but keep recovery nearby and legible. A/B, undo, bypass, and reset are part of the listening loop, not generic application chrome.

The display-control compatibility literature reports faster and more accurate operation when spatial, movement, and conceptual relationships match operator expectations. Direct-manipulation research emphasizes continuous object representation, physical action, incremental reversible operations, and immediate feedback. These findings support the mechanism, not any one visual style.

## 5. Placement-proof template

Use one row per control group:

| Control/group | Listening job | Owner/scope | Frequency | Proposed locus | Mapping | Visible states | Move/swap cost | Evidence/confidence |
|---|---|---|---|---|---|---|---|---|

Do not accept "balanced", "modern", "standard", or "looks cleaner" as the only placement reason.

## 6. State-matrix template

Use actual product states; this is a starting set:

| Element | Idle | Hover | Selected | Gesture | Audition | Advanced active/collapsed | Bypassed | No signal/degraded |
|---|---|---|---|---|---|---|---|---|

For each cell use an explicit behavior: visible, emphasized, normal, dimmed, hidden, disabled-with-reason, or replaced. Add transitions and release behavior where timing matters.

## 7. Adversarial checks

- **Swap test**: exchange neighboring groups. What task cost changes?
- **Blank-label test**: does position/mapping still communicate partial meaning without relying entirely on text?
- **Scope test**: can a user mistake band-local state for global state or the reverse?
- **Hidden-active test**: collapse every panel while its process stays active. Is the sound still explained?
- **Automation test**: change a value from the host. Does the UI update without inventing an undo event or stale selection?
- **Identity test**: switch instances or sources and enter comparison mode. Is the edit target still unambiguous before the next gesture?
- **Mixed-value test**: select objects with different values and capabilities. Does every control show disagreement or partial availability honestly?
- **Co-located-scale test**: temporarily remove color and trace each label/grid to the visualization and unit it governs.
- **Geometry-invariance test**: record each repeated control's bounding rectangle in idle, hover, pressed, selected, and disabled states. Do width, height, and learned locus stay stable?
- **Repetition test**: simulate the same adjustment one hundred times. Does movement stay short and the target stable?
- **Edge test**: select objects near every window edge and at the smallest supported size.
- **Ears-first test**: can the engineer make and compare the auditory decision without being forced to read or navigate during the critical moment?
- **Failure test**: remove audio, connection, or valid analysis. Does the interface stop making claims the system cannot support?
- **Input-equivalence test**: operate every endpoint action without a pointer, inspect focus order and focus return, and verify name/role/value/state/action semantics with the framework accessibility client.
- **Response-under-load test**: enable the maximum visual load and incoming automation while dragging or entering a value. Does acknowledgment stay prompt, and can the user tell which state is audible and authoritative?
- **Interruption test**: deliver a noncritical notice during drag, audition, comparison, and text entry. Does the transaction finish without focus theft, while real safety or connection failures still appear at the correct scope?

## 8. Common failure modes

- A permanent wall of every parameter, justified only by completeness.
- Decorative knobs or meters that imply control or measurement they do not provide.
- A web-dashboard layout that turns the sonic object into one card among many.
- Several simultaneous accents answering the same question.
- A hidden advanced process with no collapsed active marker.
- Selection that erases the context needed to judge the selection.
- Controls that move or reorder when modes change, breaking learned motor paths.
- A pressed or selected style that changes a button's width or height and shifts neighboring targets.
- Multiselection that displays one member's value as the group's value.
- A shared editor that hides which instance or source the next gesture will change.
- Two visual scales sharing an edge without clear unit and ownership cues.
- Color-only state or low-contrast "disabled" state that is indistinguishable from absence.
- A visual analyzer treated as ground truth instead of evidence to check by ear.
- Pointer-only endpoint actions, custom controls with no assistive semantics, or a focus order unrelated to sonic-object ownership.
- Visual feedback that falls behind audible or host state under analyzer load without declaring itself stale.
- A noncritical pop-up that steals focus or ends an audition, drag, comparison, or value-entry transaction.

## Sources

- Ben Shneiderman's publication index summarizes the direct-manipulation properties used here: [University of Maryland](https://www.cs.umd.edu/~ben/publications.html).
- Tsang, Ho, and Chan review spatial, movement, and conceptual display-control compatibility: [Measurement and Control, 2015](https://doi.org/10.1177/0020294015569264).
- Fitts' pointing research grounds the distance/target-size tradeoff: [Journal of Experimental Psychology, 1954](https://doi.org/10.1037/h0055392).
- A recent layered-interface experiment reports potential learnability and consistency benefits plus discoverability tradeoffs: [Designing for Learnability, 2025](https://doi.org/10.1177/10648046241273291).
- McGregor studies alternate audio-plugin interfaces with producers and sound designers in their working environments: [Knobs and Nodes, Massey University](http://hdl.handle.net/10179/16620).
- Voger Design's short practitioner article groups simplicity, consistency, feedback, accessibility, performance, interruption, and screen-size adaptation as UX concerns. Treat its named plugin examples as editorial examples rather than causal evidence: [Enhance Your Audio Plugin UX](https://vogerdesign.com/blog/make-audio-plugin-with-great-ux/).
- W3C documents keyboard operability, meaningful focus order, target-size exceptions, and non-focus-stealing status messages for the web. Use these as explicit accessibility evidence and carefully translated review prompts, not automatic native-plugin compliance claims: [Keyboard](https://www.w3.org/WAI/WCAG22/Understanding/keyboard.html), [Focus Order](https://www.w3.org/WAI/WCAG22/Understanding/focus-order.html), [Target Size](https://www.w3.org/WAI/WCAG22/Understanding/target-size-minimum.html), and [Status Messages](https://www.w3.org/WAI/WCAG22/Understanding/status-messages.html).
- JUCE documents the roles, state, actions, and value interfaces available to native custom components: [`AccessibilityHandler`](https://docs.juce.com/master/classAccessibilityHandler.html).
