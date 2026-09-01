---
version: alpha
authority:
  functionalSizing: "current"
  visualSystem: "historical-pre-2026-08-31"
functionalSizing:
  genStage:
    compactMinimum: "760x430 logical px"
    default: "950x538 logical px"
    fixedAspectRatio: "76:43"
    resizeBehavior: "workspace grows; graph/evidence first"
    uiScale: "independent"
colors:
  shell: "#1b1d1f"
  glass: "#0c0e10"
  gap: "#090a0b"
  text-primary: "#e7e9ea"
  text-secondary: "#858b90"
  selection: "#8d939a"
  signal-emission: "#7e3d3e"
  signal-field: "#521220"
  grid: "rgba(255, 255, 255, 0.055)"
typography:
  product:
    fontFamily: "Geist, Arial, sans-serif"
    fontSize: "9px"
    lineHeight: "1.2"
  telemetry:
    fontFamily: "Geist Mono, Consolas, monospace"
    fontSize: "8px"
    lineHeight: "1.25"
rounded:
  shell: "20px"
  glass: "12px"
  action: "7px"
  pill: "999px"
spacing:
  shell-inset: "7px"
  glass-inset: "2px"
  content-inset: "18px"
  control-gap: "8px"
components:
  stage:
    width: "760px"
    height: "430px"
  navigation:
    height: "28px"
  eq-field:
    width: "652px"
    height: "270px"
  focus-ring:
    color: "#e7e9ea"
    width: "1px"
---

# Nakama Gen Design

## Status after the functional restart

This file records the visual system of the pre-31-August-2026 simulator. It is
retained as historical implementation and comparison context, not as the
layout, color, material or position authority for the current functional UI
architecture. The current target is routed through
`design/LIES-MICH.md`, `design/docs/funktions-und-bedien-blueprint.md` and the
newer dated files under `design/abnahmen/`. A new visual system begins only
after an explicit user decision to enter that phase.

## Current functional sizing contract

Both Gen surfaces share one fixed-aspect, resizable editor stage. Its compact
minimum is 760×430 logical pixels; its default is 950×538, the integer-rounded
size at the same 76:43 aspect ratio. Resizing buys information area rather than
uniform zoom: the graph or acoustic evidence receives additional room first,
while established rails and control targets retain their logical geometry.
The 100/125/150/200-percent UI scale is a separate accessibility/rendering
mechanism and may not remove functionality. This decision does not set the
size of the compact Probeeq fallback surface. The binding wording is recorded
in `design/abnahmen/2026-09-01-gen-fenstergroesse-und-resize.md`.

## Historical overview

The prior Nakama Gen direction described a compact studio instrument rather
than a generic dashboard. Its former visual north star was the dark Figma
unibody and the two exports registered in
`design/prototyp/contract/asset-manifest.json`. The sections below document
that legacy simulator contract and must not silently supply the new design.

The legacy surface was a product register optimized for a producer working
inside FL Studio at a fixed 760×430 logical size. Product text is English;
surrounding prototype tooling is German.

## Colors

The palette follows the current Figma nodes `4090:685` and `6002:2`. Neutral values form the material and hierarchy. Wine red is reserved for musical focus, a pending signal or a real emitted-light state. White/grey selection remains distinct from red signal meaning. Error, stale, disconnected and protected states always include text, shape or pattern; color is never their only carrier.

Runtime ownership uses model B: Figma plus approved pixel goldens own the visual values; `design/prototyp/styles.css` mirrors those values as CSS custom properties. A token change is valid only when the Figma revision, this file, runtime CSS and visual goldens are reconciled together.

## Typography

Geist and Geist Mono are embedded locally from the official `geist@1.7.2` package under the SIL Open Font License. Geist carries product labels and prose; Geist Mono carries values, frequencies, revisions and compact status. Small type is deliberate because the target is a fixed desktop plug-in, but focus and state must remain legible through contrast and spacing rather than artificial boldness.

The NAKAMA GEN wordmark is a baked Figma export. It is not reconstructed as runtime text.

## Layout

The legacy logical editor was always 760×430. The 7px shell inset and the 2px glass inset define one shared coordinate system for both pages. Shell, wordmark, tabs and material lighting remain spatially fixed during page changes; only the glass content changes.

Overview uses one full-width probe field above one advisor workspace. EQ Center uses one 652×270 graph, a single precision parameter row and a compact bottom utility row. Repeated functions share one owner rather than gaining page-local variants. Up to 16 sources scroll horizontally in the existing source strip; Master remains a separate control.

The legacy simulator may scale to 100%, 125%, 150% and 200%, but all geometry remains defined in logical pixels. Each reference raster tier is derived directly from the four-times user golden, never enlarged from the 100% image.

## Elevation & Depth

Depth comes from believable material: the shell rim, inset glass, broad top sheen, edge assimilation, vignette and sparse emitted light. Content does not create elevation by adding generic panels. A draft card may lift because it is a transient object over the graph; ordinary data stays fused into the glass plane.

## Shapes

The shell radius is 20px and the flush glass radius is 12px. Action controls use the Figma 7px inset form. Pills are reserved for compact utility states such as Master, Global and Bypass. EQ handles remain visually small but receive larger invisible pointer targets.

## Components

Token mapping is direct and traceable:

| Design role | Runtime owner | Main consumers |
|---|---|---|
| Material and text colors | `design/prototyp/styles.css` `:root` variables | Shell, glass, controls, status |
| Motion values | `design/prototyp/contract/motion-tokens.json` and matching CSS variables | Page transition, state morph, feedback, measuring pulse |
| Product/domain states | `design/prototyp/contract/ui-contract.json` | `DemoAdapter`, renderers and tests |
| Exact static assets | `design/prototyp/contract/asset-manifest.json` | Shell grain, wordmark, EQ grid and Figma vector details |

Controls expose default, hover, focus-visible, pressed, selected, disabled/busy and unavailable-with-reason states. Direct manipulation has an immediate keyboard/numeric alternative. Planned controls remain visible only when the approved Figma design reserves them and always explain why they are unavailable.

Motion communicates state rather than decoration. Pointer drags are immediate; feedback is 100ms; ordinary state and curve changes are 140ms; page content changes are 180ms using `cubic-bezier(0.22, 1, 0.36, 1)`. Reduced motion removes transforms and keeps only short opacity feedback.

## Do's and Don'ts

- Do preserve the unibody shell, flush glass, fixed navigation and coherent lighting across both pages.
- Do draw text, statuses, heatmaps, spectrum, curves, bands, drafts and controls from state.
- Do distinguish confirmed, draft, audition, temporary lease, stale and disconnected states.
- Do keep every visible element either operational, truthful status/value, planned-with-reason or product identity.
- Do not replace the Figma material with generic cards, gradients chosen for novelty or oversized web typography.
- Do not animate static status, smooth direct drags, fake data cadence or let visual timing drive DSP timing.
- Do not present Mix as implemented before its versioned parameter contract exists.
