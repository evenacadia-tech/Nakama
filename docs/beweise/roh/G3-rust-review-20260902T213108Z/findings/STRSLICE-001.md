---
id: STRSLICE-001
bug_class: str-slice-boundary
title: Unguarded byte-index str slice in $ref resolution (aufloesen) panics on any $ref shorter than 8 bytes or with a multi-byte char at byte 8
location: broker/src/vertrag.rs:612
function: aufloesen
confidence: Low
worker: worker-8
fp_verdict: LIKELY_FP
fp_rationale: "The unguarded &r[8..] at vertrag.rs:612 is real, but the worker's own data-flow section establishes the only in-crate producers build the schema from include_str! of a compile-time constant, so no pipe byte, CLI argument or file reaches the slice; eqcop_broker is an internal lib with no external consumers, leaving the panic unreachable for the defined local attacker."
---

## Description

`aufloesen()` strips the `#/$defs/` prefix off a JSON-Schema `$ref` by slicing
the `&str` at the constant byte index `"#/$defs/".len()` (= 8) **without any
prior boundary or length proof**:

```rust
let name = &r["#/$defs/".len()..];
```

`r` is whatever string sits under the `$ref` key of the schema node. Slicing a
`&str` at byte index 8 panics in two distinct ways:

* `byte index 8 is out of bounds of ...` when `r.len() < 8` (e.g. `"#"`,
  `"#/$defs"`, `""`).
* `byte index 8 is not a char boundary; it is inside 'X' (bytes 6..9) of ...`
  when byte 8 lands inside a multi-byte UTF-8 sequence (e.g. `"#/$de\u{20AC}fs/x"`).

The sibling call site 43 lines earlier (`pruefe_teilschema`,
`broker/src/vertrag.rs:559-573`) performs exactly the same slice but **is**
guarded by `if !r.starts_with("#/$defs/")`. `aufloesen()` — the function the
whole validation walk funnels through — has no such guard. The asymmetry is
not deliberate: the comment on lines 615-616 (`kein Panic im Bibliothekspfad`
— "no panic in the library path") claims panic-freedom for the *dangling
reference* case immediately below the unguarded slice, and a second comment in
`pruefe_wert` (lines 762-764) explicitly reasons about avoiding "ein Panic im
Bibliothekspfad" from back-cutting a schema path. The authors were aware of
this exact panic class and still left line 612 unguarded.

`vertrag::Schema` is a `pub` type with a `pub fn laden(wurzel: Value)`
constructor, so the crate's public API accepts an arbitrary caller-supplied
schema.

## Code

```rust
// broker/src/vertrag.rs:608
fn aufloesen<'a>(wurzel: &'a Value, knoten: &'a Value, pfad: &str) -> (&'a Value, String) {
    let Some(r) = knoten.get("$ref").and_then(|v| v.as_str()) else {
        return (knoten, pfad.to_string());
    };
    let name = &r["#/$defs/".len()..];      // <-- no starts_with / is_char_boundary / get(..)
    match wurzel.get("$defs").and_then(|d| d.get(name)) {
        Some(ziel) => (ziel, format!("#/$defs/{name}")),
        // Kann nach der Ladepruefung nicht mehr vorkommen; kein Panic im
        // Bibliothekspfad.
        None => (knoten, pfad.to_string()),
    }
}
```

Guarded sibling for contrast (`pruefe_teilschema`, same file):

```rust
// broker/src/vertrag.rs:559
if let Some(r) = obj.get("$ref").and_then(|v| v.as_str()) {
    if !r.starts_with("#/$defs/") {
        fehler.push(format!("nicht-lokale Referenz bei {pfad}: {r}"));
    } else {
        let name = &r["#/$defs/".len()..];   // <-- guarded here
        ...
    }
}
```

## Data flow

- **Source:** the `$ref` string of a schema node passed to
  `vertrag::Schema::laden()` / reached during `Schema::pruefe()`. Today the only
  in-crate producers are `coordinator::v3_schema_wurzel()` and
  `coordinator::v3_schema_minor_0_wurzel()`
  (`broker/src/coordinator.rs:44,102,106`), which build the schema from
  `include_str!("../../eq-copilot/schemas/v3/eq-ipc-v3.schema.json")` — a
  compile-time constant, **not** attacker-controlled.
- **Sink:** `&r[8..]` at `broker/src/vertrag.rs:612`.
- **Validation:** none at the sink. `Schema::laden()` runs `pruefe_teilschema`,
  which rejects a `$ref` without the `#/$defs/` prefix — but only at nodes it
  recurses into (`properties/*`, `$defs/*`, `items`, `oneOf/*`). `aufloesen()`
  is additionally called from `diskriminator_schema()`
  (`broker/src/vertrag.rs:659,662`), `diskriminatorwert()`
  (`broker/src/vertrag.rs:674`) and `pruefe_wert()`
  (`broker/src/vertrag.rs:745,767`), which follow RFC-6901 pointer segments and
  `oneOf` chains; a `$ref` reached only through those paths is never
  prefix-checked at load time.

## Reachability trace

`Coordinator::…::v3_schema(minor) → vertrag::Schema::laden(<frozen schema>) →
Schema::pruefe(<pipe JSON>) → pruefe_wert → aufloesen → &r[8..]`

**Honest reachability statement (this is why confidence is Low):** the *instance
document* on this path is fully attacker-controlled (v3 JSON control messages
off the named pipe), but the *schema* is not. All 162 `"$ref"` values in
`eq-copilot/schemas/v3/eq-ipc-v3.schema.json` begin with `#/$defs/` (verified by
enumerating them), so a local unprivileged pipe client cannot reach the panic
today. The exposure is (a) the crate's public `Schema::laden` API for any future
non-frozen schema source, and (b) a future edit to the frozen schema
introducing a short `$ref` such as `"#"` — which `pruefe_teilschema` would only
catch at nodes it visits, and which `aufloesen` would turn into a process-level
panic rather than the intended `Err` from `laden`.

## Impact

An unguarded `str` byte-index slice on the single hot path of the v3 contract
validator. If ever fed a schema whose `$ref` is shorter than 8 bytes or has a
multi-byte character at byte offset 8, the broker panics inside the message
validation path instead of returning the `Err(String)` that `Schema::laden` is
documented to produce. On the message path that panic would abort the
per-connection worker thread; in `v3_schema()` it would abort before the
`OnceLock` is populated, poisoning schema initialization for the process. The
concrete security value today is defense-in-depth: the function's own doc
comment promises "kein Panic im Bibliothekspfad" while the line above it is the
only unguarded `str` range-slice in the crate.

## Mitigations checked

- `pruefe_teilschema` prefix check: present at `vertrag.rs:560`, but it is a
  *load-time* check over a partial node walk, not a check at the `aufloesen`
  sink; nodes reached only via `diskriminator_schema`/`diskriminatorwert`
  pointer walks bypass it.
- `Schema::laden` returns `Err` on a non-empty `fehler` list
  (`vertrag.rs:470-479`), so a schema with a *visited* bad `$ref` never reaches
  `pruefe`. Callers `.expect()` that Err, so a bad frozen schema is a startup
  abort, not a silent pass.
- Frozen schema: all 162 `$ref` values use the `#/$defs/` prefix — verified, so
  no panic fires with the shipped schema.
- No `#![deny(clippy::indexing_slicing)]`, no `[lints]` table in
  `broker/Cargo.toml`, no `rust-toolchain.toml`; no MIRI or fuzzing coverage on
  `vertrag.rs`.
- `panic = "abort"` is **not** set (no `[profile]` section in
  `broker/Cargo.toml`), so a panic here unwinds the thread rather than killing
  the process.

## Recommendation

Make the sink self-guarding rather than relying on a load-time check that does
not cover every `aufloesen` caller:

```rust
let Some(name) = r.strip_prefix("#/$defs/") else {
    return (knoten, pfad.to_string());   // same fallback as a dangling ref
};
```

`strip_prefix` is total: it cannot panic on a short string or on a multi-byte
boundary, and it makes the "non-local reference" case take the documented
non-panicking fallback path. Apply the same change to the guarded sibling at
`vertrag.rs:569` so both sites express the invariant once, in the code rather
than in a comment.
