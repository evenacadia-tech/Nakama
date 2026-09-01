# Nakama Gen UX Contract

This is the durable observable-behavior contract for the web simulator and the later native editor. Visual intent lives in `DESIGN.md`; machine-readable names and invariants live in `design/prototyp/contract/ui-contract.json`. This contract is internal UI behavior, not a replacement for Nakama IPC, processor state or the 109-parameter host contract.

The checked-in simulator predates the functional restart of 31 August 2026.
Where it still exposes the former peer-page tabs or another superseded layout,
the current target behavior is owned by
`design/docs/funktions-und-bedien-blueprint.md` and the newer dated acceptance
files. The old simulator remains comparison evidence until its bounded
replacement; it is not allowed to reopen those decisions.

## Canonical UI map

| Capability | Canonical owner | Allowed variants | Verification |
|---|---|---|---|
| Forward transition to Gen EQ | Gen Sources `SEND DRAFT` | only with a valid unconfirmed draft | state + focus test |
| Source switcher | Shared source-strip controller | 1–16 probes; separate Master | overflow + auto-scroll test |
| Gen editor sizing | Shared Gen stage across both surfaces | compact 760×430; default 950×538; fixed 76:43 resize | compact/default geometry + native-host resize test |
| Band selection | EQ graph/controller | pointer, BAND control, keyboard | geometry + input-equivalence test |
| Band precision panel | selected EQ band | object-anchored Enabled/Type/Channel/Frequency/Gain/Q/Dynamic views plus separate Remove | disclosure + focus + graph-edge + rebind test |
| Parameter input | Shared parameter-field behavior | drag, numeric input, native named select | range + protected-zone tests |
| Status feedback | Shared in-glass status/live region | neutral, warning, error | state + accessibility test |
| Scrollbar | Global prototype stylesheet | stable-gutter exceptions only | computed-style/browser check |
| Draft lifecycle | `DemoAdapter` reducer | ready, audition, lease, stale | deterministic clock tests |
| Undo | Per-source confirmed revision ring | one-step action | revision-isolation test |

Native `<select>` ownership is intentional for the compact precision row and the external inspection panel. On the initial Windows/FL Studio target, the platform popup is accepted; the closed control remains part of the authored visual contract. Direct manipulation never depends on a popup.

## State and action ownership

`DomainSnapshot` carries sources, connections, measurement, findings, transport, Probeeq state, spectrum, bands, draft, revisions and protected ranges. `ViewState` carries the active page, selection, comparison, panels and transient pointer state. Only a `UiAdapter` may change domain state in response to `UiAction` values.

The browser uses a deterministic `DemoAdapter`. The future VST3 editor will use a native adapter to Processor, Broker and Probeeq. They share vocabulary, fixtures, asset hashes, geometry and motion values, not implementation source.

## Behavior ledger

| Operation | Trigger | Pending/held state | Success | Failure/recovery | Focus outcome |
|---|---|---|---|---|---|
| Select source | source name | immediate | graph/finding target changes | disconnected source is read-only with reason | selected source |
| Audition | HOLD TO AUDITION press | active only while held | release returns to confirmed sound | blocked by stopped/recording/unknown transport; connection loss ends safely | hold control |
| Send draft and open Gen EQ | SEND DRAFT | DRAFT SENT / WAITING FOR PROBEEQ | Gen EQ opens with the same unconfirmed draft | disconnected target retains finding and offers recovery | originating action or EQ draft |
| Temporary apply | EQ APPLY | ten-second lease and visible confirmation action | second activation confirms and creates one revision | reject, expiry, invalid transport or connection loss reverts | Apply/Confirm control |
| Reject | REJECT | none | draft removed without revision | none | stable graph context |
| Undo | undo control | none | last confirmed source revision restored | disabled with reason when ring empty | undo control |
| External automation | inspection scenario | immediate authoritative value/curve change | confirmed state reflects host | open draft becomes stale; protected violation is shown, not hidden | current control unchanged |
| Resize Gen editor | host-supported editor edge | view-only transaction; no domain or audio change | both Gen surfaces keep 76:43; graph/evidence receives added area first | clamp at compact minimum or available host/display work area | currently focused sonic object/control remains focused |
| Rebind an open band panel | single-click another occupied band point | owner changes atomically; no values move | same panel anchors to the new point and shows its basic view | no stale writes to the previous slot | newly selected band point for pointer; first basic control when invoked by keyboard |
| Change band channel mode | compact current-mode control in the panel head | only `channel_mode` changes at the block edge | same panel returns from its five-option view to Frequency/Gain/Q | unavailable modes are omitted with an explicit reason; no implicit Stereo fallback | current-mode control |
| Disable or enable a band | stable `ON/OFF` control in every panel view | only `enabled` changes at the block edge | values, slot ID, channel mode and Dynamic state remain; disabled point stays reachable | unavailable/offline is explained without hiding the recovery control | `ON/OFF` control |
| Open Dynamic for an inactive band | `DYN · OFF` | `dynamic_enabled` changes at the block edge | the same panel opens its Dynamic view; stored values remain | unavailable/offline is explained without a false open state | Range field |
| Disable Dynamic | leading state control in the Dynamic view | only `dynamic_enabled` changes | the same panel returns to Frequency/Gain/Q | five Dynamic values are retained | `DYN · OFF` |
| Remove band | separate `Remove Band` action at the panel perimeter | atomic slot-release transaction; not band bypass | point disappears, panel closes, other band IDs remain stable | Undo restores the complete band under the same ID; native action unavailable until slot/undo contract exists | stable Undo recovery locus |
| Close band precision panel | explicit close control; Escape outside a dirty numeric edit | none | panel closes without parameter change | first Escape restores a dirty numeric field instead | owning band point |

## Hard invariants

- `SEND DRAFT` sends an unconfirmed draft; it never confirms EQ.
- `SEND DRAFT` is the only product action that opens Gen EQ from Gen Sources.
- Audition and draft states never enter the revision ring or saved project state.
- EQ Apply is two-stage: temporary lease first, explicit confirmation second within ten seconds.
- Preview is permitted only while playing and never while recording or record state is unknown.
- Connection loss during preview/lease returns to the confirmed curve and explains the failed link segment.
- Protected ranges block pointer, keyboard and numeric Nakama edits at the boundary. Host automation may cross them and must reveal the violation.
- External automation is authoritative and makes an open draft stale.
- Undo is scoped to the selected probe; Master and each probe keep separate revision history.
- Master is not part of source wheel order.
- Both Gen surfaces share the same editor size and fixed 76:43 aspect ratio.
- 760×430 is the compact minimum and 950×538 the integer-rounded default.
- Window resizing and UI scaling are separate: extra window area grows the
  graph/evidence first and never becomes a second path to otherwise missing
  functionality.
- Mix remains `PLANNED` and explains the missing versioned parameter contract.
- Sidechain source remains absent until its placement decision is approved.
- Opening, leaving or closing the Dynamic view never disables an already active
  Dynamic band. Only its explicit state control does.
- A single click on empty EQ graph space does not close the band precision
  panel or change its owning band.
- Active Dynamic remains evident without text or color through a second contour
  at the configured band position.
- Band `enabled`, `dynamic_enabled`, channel mode, Remove and panel Close are
  separate actions. None may silently stand in for another.
- `enabled = false` never frees an automation slot. A disabled occupied band
  stays visible, selectable and recoverable without relying on color.
- Removing a band never renumbers another band. Restore returns the full state
  to the same fixed band ID.
- No visible element is a dead decoration except material and product identity.

## Navigation and input

Gen Sources has no direct product tab or second navigation action for opening
Gen EQ. `SEND DRAFT` is the sole forward transition and carries the same
unconfirmed draft into the EQ surface. A surface switcher in an external
inspection or sketch tool is tooling, not product UI. The product return path
from Gen EQ remains open and must not be invented from the legacy tablist.
Source overflow keeps the selected source visible. Every drag has a BAND
selector, keyboard arrows and/or numeric entry alternative. In a dirty numeric
field, the first Escape restores the entry value and keeps the panel open;
otherwise Escape closes the topmost non-modal panel and returns focus to its
owning object. It never silently rejects a draft. The EQ band panel also has an
explicit close control; an empty-graph single click is not a close gesture.
While the panel is open, selecting a different occupied point atomically moves
the panel owner and resets only its transient subview to the basic controls.
The compact current-channel control opens `Stereo`, `Left`, `Right`, `Mid` and
`Side` inside the same panel body. `ON/OFF` remains at a stable header position
through every subview. Remove is a separate named action and requires an
immediate Undo route; modifier gestures may supplement, but never replace,
these accessible controls.

The two Gen surfaces use one fixed-aspect, resizable plug-in stage. Its compact
minimum is 760×430 logical pixels; its default is 950×538, rounded from the
same 76:43 ratio. Resizing reallocates information area without changing the
surface hierarchy: fixed rails and established targets retain their logical
size, while the EQ graph on Gen surface 2 and the acoustic evidence on Gen
surface 1 receive additional room first. Every function remains reachable at
the compact minimum. UI scale at 100/125/150/200 percent remains a separate
whole-interface accessibility/render tier. The surrounding inspection tool
may reflow, but its controls are not product UI. Probeeq sizing is outside this
Gen decision. Native resize negotiation, reopen behavior and bounds still
require proof in the Windows Release VST3 inside FL Studio.

## Feedback, offline and conflict

Critical conditions remain visible in the affected region instead of disappearing into transient notices. Broker offline, probe offline, stale, incomplete, transport stopped, recording blocked and automation conflict use distinct wording. Reconnection revalidates the authoritative confirmed state before controls unlock. Demo scenarios are explicitly marked outside the plug-in stage and never appear in visual-golden captures.

## Motion and data truth

Motion values come from `design/prototyp/contract/motion-tokens.json`. Pointer manipulation updates cursor, handle, number and curve in one frame. Measurement animation runs only while measuring. Spectrum input advances at 20Hz and display frames interpolate between samples. Reduced-motion mode removes transforms and keeps short opacity feedback. Audio ramps remain outside this contract.

For Dynamic EQ, the outer band contour remains at configured gain as the stable
state marker and hit target. The inner point and its response-curve segment may
follow actual per-band gain excursion only when an authoritative runtime field
with usable ID and payload exists. `Frame.band_dynamic_gain_db` is reserved for
S26–28, but its field ID and runtime payload do not exist yet. Probeeq will
later carry it in the feature frame; Gen Master provides it locally without
IPC. Production UI must not synthesize continuous motion from Range or
Threshold. The technical target sketch may use a finite, explicitly labeled
activation demonstration, with no loop and no transform in reduced-motion
mode.

## Approval gate

The native JUCE editor must not adopt this surface until both pages, required
negative states, compact 760×430, default 950×538 and the named web release
candidate have been visually accepted against the current functional target.
That approval starts the VST3 transfer; it is not implied by passing automated
tests. Native host proof must additionally cover resize, reopen, focus
retention and all UI-scale tiers.
