# Nakama Gen UX Contract

This is the durable observable-behavior contract for the web simulator and the later native editor. Visual intent lives in `DESIGN.md`; machine-readable names and invariants live in `design/prototyp/contract/ui-contract.json`. This contract is internal UI behavior, not a replacement for Nakama IPC, processor state or the 109-parameter host contract.

## Canonical UI map

| Capability | Canonical owner | Allowed variants | Verification |
|---|---|---|---|
| Page tabs | Shared shell tablist | Overview / EQ Center | keyboard + browser state test |
| Source switcher | Shared source-strip controller | 1–16 probes; separate Master | overflow + auto-scroll test |
| Band selection | EQ graph/controller | pointer, BAND control, keyboard | geometry + input-equivalence test |
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
| Switch page | OVERVIEW / EQ CENTER | 180ms glass-content change | selected page visible | none | selected tab |
| Select source | source name | immediate | graph/finding target changes | disconnected source is read-only with reason | selected source |
| Audition | HOLD TO AUDITION press | active only while held | release returns to confirmed sound | blocked by stopped/recording/unknown transport; connection loss ends safely | hold control |
| Send draft | Overview APPLY | DRAFT SENT / WAITING FOR PROBEEQ | EQ opens with an unconfirmed draft | disconnected target retains finding and offers recovery | originating action or EQ draft |
| Temporary apply | EQ APPLY | ten-second lease and visible confirmation action | second activation confirms and creates one revision | reject, expiry, invalid transport or connection loss reverts | Apply/Confirm control |
| Reject | REJECT | none | draft removed without revision | none | stable graph context |
| Undo | undo control | none | last confirmed source revision restored | disabled with reason when ring empty | undo control |
| External automation | inspection scenario | immediate authoritative value/curve change | confirmed state reflects host | open draft becomes stale; protected violation is shown, not hidden | current control unchanged |

## Hard invariants

- Overview APPLY sends a draft; it never confirms EQ.
- Audition and draft states never enter the revision ring or saved project state.
- EQ Apply is two-stage: temporary lease first, explicit confirmation second within ten seconds.
- Preview is permitted only while playing and never while recording or record state is unknown.
- Connection loss during preview/lease returns to the confirmed curve and explains the failed link segment.
- Protected ranges block pointer, keyboard and numeric Nakama edits at the boundary. Host automation may cross them and must reveal the violation.
- External automation is authoritative and makes an open draft stale.
- Undo is scoped to the selected probe; Master and each probe keep separate revision history.
- Master is not part of source wheel order.
- Mix remains `PLANNED` and explains the missing versioned parameter contract.
- Sidechain source remains absent until its placement decision is approved.
- No visible element is a dead decoration except material and product identity.

## Navigation and input

The two peer views use a real tablist. Arrow keys move between tabs and selection follows immediately because both panels are local. Source overflow scrolls horizontally and keeps the selected source visible. Every drag has a BAND selector, keyboard arrows and/or numeric entry alternative. Escape closes the topmost non-modal panel or cancels an in-progress numeric edit; it never silently rejects a draft.

The fixed plug-in stage does not reflow. The surrounding inspection tool does. At 200% the whole stage is intentionally larger, matching the plug-in scale tier rather than behaving like a responsive website.

## Feedback, offline and conflict

Critical conditions remain visible in the affected region instead of disappearing into transient notices. Broker offline, probe offline, stale, incomplete, transport stopped, recording blocked and automation conflict use distinct wording. Reconnection revalidates the authoritative confirmed state before controls unlock. Demo scenarios are explicitly marked outside the plug-in stage and never appear in visual-golden captures.

## Motion and data truth

Motion values come from `design/prototyp/contract/motion-tokens.json`. Pointer manipulation updates cursor, handle, number and curve in one frame. Measurement animation runs only while measuring. Spectrum input advances at 20Hz and display frames interpolate between samples. Reduced-motion mode removes transforms and keeps short opacity feedback. Audio ramps remain outside this contract.

## Approval gate

The native JUCE editor must not adopt this surface until both pages, required negative states and the named web release candidate have been visually accepted against the user goldens. That approval starts the VST3 transfer; it is not implied by passing automated tests.
