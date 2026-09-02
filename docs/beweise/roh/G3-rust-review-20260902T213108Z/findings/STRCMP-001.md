---
id: STRCMP-001
bug_class: string-comparison
title: Probe-namespace gate is a raw prefix match on an un-normalized Win32 pipe path
location: broker/src/transport/pipetoken.rs:38
function: ist_probe_pipename
confidence: Medium
worker: worker-15
fp_verdict: TRUE_POSITIVE
fp_rationale: "The gate really is a byte-exact prefix test with an unconstrained remainder that is then handed unchanged to CreateNamedPipeW, and \\\\.\\ device paths are normalized by Win32 (unlike \\\\?\\), so a \\..\\ remainder resolves to the production endpoint; the case-insensitivity mismatch against NPFS is a second real gap."
severity: LOW
attack_vector: Local
exploitability: Reliable
severity_rationale: "Broken containment interlock rather than a privilege gain: the caller who supplies argv already runs the probe binary as themselves and could open any pipe directly, so the real cost is a test broker landing on the production pipe name — exactly the failure the project's own operating rule forbids."
---

## Description

`ist_probe_pipename` is the only containment gate that keeps the probe/proof
binaries off the production v3 endpoint. It decides purely on a byte-exact string
prefix and then accepts *any* non-empty remainder:

```rust
pub fn ist_probe_pipename(name: &str) -> bool {
    match name.strip_prefix(PROBE_PRAEFIX) {
        Some(rest) => !rest.is_empty(),
        None => false,
    }
}
```

The doc comment states the invariant the gate is supposed to carry
(`broker/src/transport/pipetoken.rs:25`): *"Alles darunter gehoert Tests und
Beweislaeufen; nichts darunter kann je ein produktiver v3-Endpunkt sein, weil ein
SID-Token ein 26-stelliges Base32-Wort ist und nie mit `probe.` beginnt."* That
argument only holds for remainders that are plain name tokens. The gate never
constrains the remainder's character set, and the *same, unmodified* string is
handed to `CreateNamedPipeW` (`broker/src/transport/server_v3.rs:395`), i.e. to
the Win32 path parser. For a `\\.\` local-device path the parser performs normal
path normalization — the documented difference to `\\?\`, which is the prefix
that disables parsing. A remainder that contains `\..\` therefore denotes a
different pipe object than the string the check inspected:

```
\\.\pipe\evenacadia.nakama.v3.probe.x\..\evenacadia.nakama.v3.<TOKEN26>
```

passes `ist_probe_pipename` (prefix matches, remainder non-empty) and resolves to
the production endpoint `\\.\pipe\evenacadia.nakama.v3.<TOKEN26>` produced by
`pipe_name_v3` (`broker/src/transport/pipetoken.rs:78`). The check is a statement
about a *string*, not about the *pipe object* that string names.

The same location carries a second, independent case-sensitivity mismatch: NPFS
compares pipe names case-insensitively, while `strip_prefix` is byte-exact. The
`to_uppercase()` normalization applied to the SID in `pipe_token`
(`broker/src/transport/pipetoken.rs:72`) shows the module is aware that pipe
identity is case-insensitive, but the namespace gate is not normalized at all.
That direction happens to fail closed for the allow-list, yet it confirms the
gate reasons over a form that is not the form the kernel compares.

## Code

```rust
// broker/src/transport/pipetoken.rs:28
pub const PROBE_PRAEFIX: &str = r"\\.\pipe\evenacadia.nakama.v3.probe.";

// broker/src/transport/pipetoken.rs:38
pub fn ist_probe_pipename(name: &str) -> bool {
    match name.strip_prefix(PROBE_PRAEFIX) {
        Some(rest) => !rest.is_empty(),
        None => false,
    }
}
```

Call site — the argument goes from the command line through the gate into a
real listener without any further validation:

```rust
// broker/src/bin/eqcop-broker-v3probe.rs:34
    let pipe = args[1].clone();
    if !ist_probe_pipename(&pipe) {
        eprintln!("VERWEIGERT: {pipe} liegt nicht im Probe-Namensraum. ...");
        std::process::exit(3);
    }
    ...
    v3_server_starten(&pipe, senke.clone(), "v3probe".into())
```

## Data flow

- **Source:** `std::env::args()[1]` of `eqcop-broker-v3probe`
  (`broker/src/bin/eqcop-broker-v3probe.rs:34`) and of
  `eqcop-broker-sonde012-probe` (`broker/src/bin/eqcop-broker-sonde012-probe.rs:25`)
  — fully controlled by whoever starts the binary (local unprivileged user, proof
  runner, or a script).
- **Sink:** `CreateNamedPipeW(name_w.as_ptr(), ...)` at
  `broker/src/transport/server_v3.rs:395`, reached via `v3_server_starten` →
  `v3_server_starten_mit_epoch` → `ListenerInstanz::neu`.
- **Validation:** only the prefix predicate at
  `broker/src/transport/pipetoken.rs:38`. The remainder is not restricted to a
  name-token character set; `\`, `/`, `..` and case variants are all accepted, and
  no canonicalization happens before or after the check.

## Reachability trace

`main` (`broker/src/bin/eqcop-broker-v3probe.rs:28`) → `ist_probe_pipename`
(`broker/src/transport/pipetoken.rs:38`) → `v3_server_starten`
(`broker/src/transport/server_v3.rs:1569`) → `v3_server_starten_mit_epoch` →
`ListenerInstanz::neu` (`broker/src/transport/server_v3.rs:384`) →
`CreateNamedPipeW` (`:395`).

## Impact

A local unprivileged process that can start one of the probe binaries — they are
built into `broker/target/release/` and are driven by the proof runner — can make
that binary create the **production** v3 endpoint instead of a probe endpoint,
which is exactly the outcome the gate was introduced to prevent (T2-Befund 7,
2026-08-29). Consequences:

- Name squatting: the first instance is created with
  `FILE_FLAG_FIRST_PIPE_INSTANCE` (`broker/src/transport/server_v3.rs:388`), so
  the squatter owns the production name and the real broker's later start fails.
- Interception: plugin instances connect to the squatting listener and perform
  their control/telemetry handshake against it, exposing addresses,
  `runtime_nonce`s and telemetry to a process that never had to defeat the pipe
  DACL.
- Accidental destruction of a production session by a mistyped proof-run
  argument, which is the failure mode the allow-list was meant to make
  impossible.

## Mitigations checked

- The gate is an allow-list rather than the earlier deny-list, which fixed the
  previously reported bypass but did not change the fact that the decision is
  made on the un-normalized string.
- The pipe DACL / `Sicherheit` descriptor restricts *who may connect*, not *which
  name this process may create*; it does not stop a same-user squatter.
- `FILE_FLAG_FIRST_PIPE_INSTANCE` makes squatting detectable to the broker (its
  start fails) but does not prevent it, and gives no protection once the squatter
  is first.
- The unit test `probe_namensraum_laesst_nur_probe_namen_durch`
  (`broker/src/transport/pipetoken.rs:102`) only covers plain token names; no
  vector contains `\`, `/`, `..` or a case variant.
- No `debug_assert!`, no additional charset check, no canonicalization anywhere on
  this path.

Not verified by execution: the review is read-only and forbids starting any
process that opens a named pipe, so the Win32 `\\.\` normalization step is
asserted from the documented `\\.\` (parsed) vs `\\?\` (parsing disabled)
distinction rather than measured. The Rust-side gap — an unconstrained remainder
handed unchanged to the OS — is directly visible in the source regardless of how
the parser resolves `..`.

## Recommendation

Make the gate a statement about the pipe object, not about the raw string:
restrict the remainder to a name-token character set and reject anything that
could re-parse, e.g.

```rust
pub fn ist_probe_pipename(name: &str) -> bool {
    match name.strip_prefix(PROBE_PRAEFIX) {
        Some(rest) => {
            !rest.is_empty()
                && rest.len() <= 64
                && rest
                    .bytes()
                    .all(|b| b.is_ascii_alphanumeric() || matches!(b, b'.' | b'_' | b'-'))
        }
        None => false,
    }
}
```

This rejects `\`, `/`, `..`, and every other separator, so the checked string and
the created object cannot diverge. Additionally compare the prefix
case-insensitively (`rest`/prefix via `eq_ignore_ascii_case` on the fixed-length
prefix) so that the gate speaks the same case semantics as NPFS, and add negative
test vectors for `...probe.x\..\evenacadia.nakama.v3.<token>` and for a
mixed-case probe prefix.
