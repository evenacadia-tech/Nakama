---
type: Architecture Guide
title: Plugin state and identity
description: Explains host persistence, migration, read-only preservation, identities, parameters, and canonical hashes.
tags: [plugin, state, identity]
sources:
  - id: openwiki-source-6bf0862b85fceb2943431cf0
    resource: repo://broker/tests/contract_cross_language.rs
  - id: openwiki-source-a116440ab220f7233740fbf3
    resource: repo://eq-copilot/cmake/NakamaIdentitaet.cmake
  - id: openwiki-source-6f2965f3eb9c224248634214
    resource: repo://eq-copilot/identity/plugin-identities-v1.json
  - id: openwiki-source-71e7d4e3896d39625d69a0a7
    resource: repo://eq-copilot/plugin/CMakeLists.txt
  - id: openwiki-source-241a5ddefd7551ffad5b4cd4
    resource: repo://eq-copilot/plugin/sonde/SondeProcessor.cpp
  - id: openwiki-source-5a8cd5b65a1c7fa70f8bf898
    resource: repo://eq-copilot/plugin/src/PluginProcessor.cpp
  - id: openwiki-source-83509902577386030dcb6537
    resource: repo://eq-copilot/plugin/state/NakamaKanon.cpp
  - id: openwiki-source-b93fd789cc6f3dbfff85c7e1
    resource: repo://eq-copilot/plugin/state/NakamaLebenslauf.cpp
  - id: openwiki-source-cc86afb122e93353cf462969
    resource: repo://eq-copilot/plugin/state/NakamaLebenslauf.h
  - id: openwiki-source-34fb35e4ab3e9d75c02a40f3
    resource: repo://eq-copilot/plugin/state/NakamaParameter.cpp
  - id: openwiki-source-9ec776bb925542fedf62d6f9
    resource: repo://eq-copilot/plugin/state/NakamaState.cpp
  - id: openwiki-source-b8f2dfd8cd8b6c6ed5a962a0
    resource: repo://eq-copilot/plugin/state/NakamaState.h
  - id: openwiki-source-a0a99c4dc2a7fccd852337ce
    resource: repo://eq-copilot/plugin/tests/IdentityTestMain.cpp
  - id: openwiki-source-fbf3ece1b64be989360b59d4
    resource: repo://eq-copilot/plugin/tests/LebenslaufTestMain.cpp
  - id: openwiki-source-0943c58ff1bce1e8bb1ba64d
    resource: repo://eq-copilot/plugin/tests/SondeNullTestMain.cpp
  - id: openwiki-source-78ba42cd6ffb895a08cd36be
    resource: repo://eq-copilot/plugin/tests/StateMigrationTestMain.cpp
generated: {by: "claude-code", at: "2026-08-23T10:03:23.427Z"}
---

# Plugin state and identity

`nakama::state` owns host-state parsing, migration, typed access, and byte
production. It is an all-or-read-only boundary and is not called from the audio
thread. `EqCopilotProcessor` integrates it with JUCE persistence, pipe
registration, duplicate repair, and host-dirty notifications.

## Schema-2 ownership

A readable `NakamaState` schema-2 tree has exactly one `Common` child and a
kind-specific child matrix. The bundle must accept the stored plugin kind, and
the measurement position must be valid for that kind. The Eqcp bundle accepts
main and legacy kinds, Suna accepts only `passive_probe`, and Probeeq accepts
only `active_probe`. Unknown children, duplicate children, and the
not-yet-readable `Dsp` or `Pairing` children force read-only handling instead
of partial interpretation.

```mermaid
stateDiagram-v2
    [*] --> Ignored: foreign tree or malformed bytes
    [*] --> Migrated: readable schema 1
    [*] --> Loaded: readable schema 2
    [*] --> ReadOnly: recognized but unsupported
    Migrated --> Writable
    Loaded --> Writable
    ReadOnly --> OriginalBytes
    Writable --> SynchronizedTree
```

Foreign roots and malformed bytes leave the processor's current state
unchanged. A recognized state whose version, child matrix, kind, or values
cannot be interpreted is kept read-only with its original bytes. Saving a
read-only state returns those bytes exactly. Saving writable state updates
known values in a copy of the loaded tree so additive unknown properties
survive.

When read-only state is loaded, the processor stops pipe registration because
it has no trusted identity to announce. A readable load or migration restarts
and reconnects the pipe. Loading and migration do not mark the host project
dirty. While read-only, both binding changes and persistent sensor-ID repair
are rejected without reconnecting or emitting a host-dirty notification.

## Migration and persistent changes

Schema-1 roles map into schema-2 plugin kind and measurement position. Migration
preserves sensor ID, label, and pair ID, does not invent a project binding, and
creates a UUID only for a broken legacy state with an empty sensor ID.

Binding edits validate the legacy v2 role bridge, reject read-only state, and
ignore no-op submissions. A real edit marks the host document dirty as a
non-parameter state change and reconnects the pipe. Duplicate plugin instances
can begin with the same persistent sensor ID but have different runtime nonces;
the visible repair action assigns this instance a new persistent ID, marks the
document dirty, and reconnects.

## Lifecycle classification

`nakama::state::Lebenslauf` separates a bundle's compiled product class from
what a particular instance is currently allowed to claim. Every fresh instance
starts `unclassified`. Garbage or a foreign tree is ignored without changing
that state; a recognized but unreadable state actively returns it to
`unclassified`; only a complete readable restore assigns a classification.

```mermaid
stateDiagram-v2
    [*] --> Unclassified
    Unclassified --> Main: restored main/hub state
    Unclassified --> Legacy: restored sensor/pre/post state
    Unclassified --> PassiveProbe: valid Suna state
    Unclassified --> ActiveProbe: valid Probeeq state
    Main --> Unclassified: read-only state
    Legacy --> Unclassified: read-only state
    PassiveProbe --> Unclassified: read-only state
    ActiveProbe --> Unclassified: read-only state
```

For the main bundle, migrated schema-1 `sensor`, `pre`, and `post` roles become
`legacy`; migrated `hub` and valid schema-2 main state become `main`. A never-
restored instance can become `main` only when the editor is open and the user
performs a real role change. Changing that role back to a sensor role also
returns the instance to `legacy`.

Suna and Probeeq have fixed passive and active product classes, respectively,
but remain neutral until their own valid state is restored and never become
`main`. The lifecycle predicate permits broker startup only for a
main-classified instance with an open editor. It is the required attachment
point for a future spawn path; it does not prevent ordinary pipe connection.

## Parameter and hash contract

The parameter table contains 109 entries: five global values and thirteen
fields for each of eight slots. Active-probe state carries this parameter set,
and a fresh Probeeq state initializes it with the valid neutral defaults rather
than zero-filled values. None of the three current product bundles exposes host
parameters, and Probeeq's EQ processing is not implemented yet.

DSP DTO parsing is intentionally strict. The text guard and canonical reader
reject malformed JSON, duplicate keys, wrong structure or schema version,
unknown or missing parameter IDs, wrong types, non-finite numbers, and values
outside contract ranges. Valid values are serialized to canonical UTF-8 and
hashed as lowercase SHA-256. C++, Python, and Rust tests consume the same
canonicalization and DTO fixtures, but the Rust broker hash path remains test
evidence rather than production broker behavior.

## Runtime and build identity

`plugin-identities-v1.json` now freezes all three built product identities:
Eqcp for the existing main bundle, NkPr for Suna, and NkAc for Probeeq. A CMake
reader supplies product name, vendor and plugin codes, and VST3 categories to
each thin target layer; missing or `null` identity fields stop configuration.
`EqCopIdentityTest` independently checks each built `moduleinfo.json` against
the same manifest. Configure-time equality between project and plugin versions
is a separate build concern documented in
[Build and proof](../delivery/build-and-proof.md).

## Safe change rules

- Additive properties within readable children are the compatible extension
  seam; new semantic children or incompatible values require version work.
- Update save and load behavior together, including byte-preservation tests.
- Change the parameter table, JSON contract, generated fixtures, C++ table, and
  cross-language hash tests as one contract change.
- Do not treat the 109 serialized parameters as current host controls or as
  evidence that Probeeq DSP already exists.

## Source map and focused validation

- State model: `eq-copilot/plugin/state/NakamaState.h`, `NakamaState.cpp`
- Parameter/DTO model: `NakamaParameter.h`, `NakamaParameter.cpp`
- Canonical JSON: `NakamaKanon.h`, `NakamaKanon.cpp`
- Lifecycle rules: `NakamaLebenslauf.h`, `NakamaLebenslauf.cpp`
- Processor integration: `plugin/src/PluginProcessor.cpp` —
  `getStateInformation`, `setStateInformation`, `setzeBindung`, `neueSensorId`
- Probe integration: `plugin/sonde/SondeProcessor.h`, `SondeProcessor.cpp`
- Contracts: `schemas/state/nakama-state-v2.md`,
  `schemas/state/nakama-parameter-v1.json`, `identity/plugin-identities-v1.json`
- Checks: `StateMigrationTestMain.cpp`, `IdentityTestMain.cpp`,
  `LebenslaufTestMain.cpp`, `SondeNullTestMain.cpp`,
  `tools/eq-copilot/erzeuge_state_fixtures.py`, and
  `broker/tests/contract_cross_language.rs`

The focused tests cover migration, idempotence, byte preservation, read-only
behavior, dirty/no-op rules, lifecycle transitions, all parameter metadata,
canonical hashes, and all three built identities. Artifact checks inspect the
actual `moduleinfo.json` files rather than accepting target declarations as
proof of host-visible identity.
