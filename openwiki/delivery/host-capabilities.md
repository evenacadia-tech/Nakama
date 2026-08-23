---
type: Architecture Guide
title: Host capabilities
description: Explains the hash-gated JUCE bridge, disposable host probes, measured FL Studio evidence, and the current production-adoption boundary.
tags: [delivery, host, vst3]
sources:
  - id: openwiki-source-518f7da97942d6ec789ebab2
    resource: repo://docs/beweise/termin-a/aux-spike-20260822-001701.json
  - id: openwiki-source-d6b7b1ed3ed9e95aed3a894f
    resource: repo://docs/beweise/termin-a/aux-spike-20260822-002722.json
  - id: openwiki-source-2be3e2244ba4cd3dfbb5646c
    resource: repo://docs/beweise/termin-a/LIES-MICH.md
  - id: openwiki-source-2c7a838f12f94ff9a23bd1e7
    resource: repo://docs/beweise/termin-b/AKTIONSPROTOKOLL.md
  - id: openwiki-source-3f48c527454df707e6b06bd9
    resource: repo://docs/beweise/termin-b/host-probe-20260822-132644.json
  - id: openwiki-source-2fabb80afe10a8768663d718
    resource: repo://docs/beweise/termin-b/LIES-MICH.md
  - id: openwiki-source-a03415bfddc1b134b2d92620
    resource: repo://eq-copilot/cmake/NakamaBruecke.cmake
  - id: openwiki-source-c3029ad49119415f8327accf
    resource: repo://eq-copilot/CMakeLists.txt
  - id: openwiki-source-e6a84631099ee5eb51aed575
    resource: repo://eq-copilot/identity/host-capabilities-fl-v1.json
  - id: openwiki-source-71e7d4e3896d39625d69a0a7
    resource: repo://eq-copilot/plugin/CMakeLists.txt
  - id: openwiki-source-be5fa74a1aa8f6e17a6620a8
    resource: repo://eq-copilot/plugin/hostbridge/NakamaHostBridge.h
  - id: openwiki-source-fcf25e3725e3cd3eddd6cdb6
    resource: repo://eq-copilot/plugin/hostprobe/HostProbeEditor.cpp
  - id: openwiki-source-0bb864a274cca024648ee43d
    resource: repo://eq-copilot/plugin/hostprobe/HostProbeProcessor.cpp
  - id: openwiki-source-407aa5ab78cb49492c2abc25
    resource: repo://eq-copilot/plugin/hostprobe/HostProbeProcessor.h
  - id: openwiki-source-98d83c7b083f1306161d6a26
    resource: repo://eq-copilot/plugin/spike/AuxSpikeProcessor.cpp
  - id: openwiki-source-f8b06632565274a9d46feb9c
    resource: repo://eq-copilot/plugin/spike/AuxSpikeProcessor.h
  - id: openwiki-source-4cceb4b96a9e3ee42df1f62b
    resource: repo://eq-copilot/plugin/src/PluginProcessor.h
  - id: openwiki-source-9043c3635e77962304034589
    resource: repo://eq-copilot/plugin/tests/HostContextTestMain.cpp
  - id: openwiki-source-adbd755bfff32253f3e5d7e8
    resource: repo://eq-copilot/schemas/v3/eq-ipc-v3.schema.json
  - id: openwiki-source-26a7709f19465fc37ff64535
    resource: repo://third_party/patches/juce-8.0.9-nakama-vst3-bridge.patch
  - id: openwiki-source-b453891c83ed97f9eb07489e
    resource: repo://tools/eq-copilot/pruefe_host_capabilities.py
generated: {by: "claude-code", at: "2026-08-22T15:50:39.855Z"}
---

# Host capabilities

Nakama measures host behavior through a thin JUCE VST3 wrapper patch,
fixed-capacity bridge, and disposable probe plugins. Raw reports and operator
observations are reconciled into a strict capability file. That evidence does
not yet drive the product processor at runtime.

## Bridge installation and semantics

JUCE's public processor API does not expose the distinctions Nakama needs for
context presence, every automation point, and per-bus presentation latency.
`NakamaHostBridge.h` supplies SDK-free repository logic; the JUCE patch only
forwards wrapper observations.

Patch installation normalizes line endings before hashing. An already patched
wrapper is a no-op, pristine JUCE `8.0.9` is patched and rehashed, and any
unknown source state fails configuration. A JUCE update therefore requires an
explicitly re-proved patch and new accepted hashes.

The patch observes every parameter-queue point, distinguishes an absent process
context from a present one, forwards `IAudioPresentationLatency`, and hands the
payload off immediately before `processBlock`. JUCE's existing last-point-wins
parameter behavior remains unchanged.

Bridge storage is fixed-capacity: 512 automation events, 128 remembered last
values, and 16 buses. Missing data remains distinct from zero. Overflow keeps
the final host value, revokes sample-accuracy assurance, and increments
explicit counters. Only processors implementing `hostbruecke::Senke` receive
payloads, including flush-style callbacks for which JUCE may not call
`processBlock`.

## Measurement flow

```mermaid
flowchart LR
    Host[FL Studio VST3 calls] --> Patch[Hash-gated JUCE patch]
    Patch --> Bridge[Fixed-capacity bridge]
    Bridge -->|Senke implemented| Probe[Disposable HostProbe]
    Host --> Aux[Disposable AuxSpike]
    Probe --> Raw1[HostProbe JSON]
    Aux --> Raw2[AuxSpike JSON]
    Raw1 --> Check[Capability validator]
    Raw2 --> Check
    Notes[Operator log and limitations] --> Check
    Check --> Caps[host-capabilities-fl-v1.json]
    Caps -. explicit future adoption .-> Product[EqCopilotProcessor]
```

HostProbe is passive, bit-identical, zero-latency, and disposable. It supports
float and double paths and reports bridge callbacks separately from JUCE audio
processing. It promotes multi-point automation to sample-accurate only when
delivery has no overflow or ordering failure. Reports are timestamped JSON
under the user's `evenacadia/nakama/spike` application-data directory;
directory or write failure is returned as an empty path and shown by the
editor.

AuxSpike is a separate passive bus-routing probe with main plus two default-off
auxiliary inputs. It observes impulse offsets without changing main audio and
explicitly rejects continuous-signal contribution measurement.

## What the checked-in evidence proves

The Termin B HostProbe report records bridge/context delivery and project
sample time in one controlled FL Studio measurement. Automation never exceeded
one point per queue, so sample-accurate automation remains unproved. Input and
output presentation latency were reported, but no impulse golden connected the
reported numbers to measured alignment and one later value change was not
retained.

The two Termin A AuxSpike reports show activation and routing for sidechain and
compare buses, with impulses observed at offset zero. They do not prove PDC,
source or channel attribution, or auxiliary contribution because the run used
indistinguishable stimuli and no latency-bearing load.

These are measurements of a particular setup, not general guarantees inferred
from an FL Studio version.

## Capability contract and validation

`identity/host-capabilities-fl-v1.json` is a strict ten-key adoption contract.
Every value is either `supported` or `unsupported`; additional keys are not
accepted. Only `host_context_presence` and `project_time_samples` are currently
supported. Each other capability carries a measured limitation, fixed
fallback, or explicit unmeasured boundary. A value changes only after a new
measurement, never from host-version inference.

The eight unsupported bits and their present dispositions are:

- `sample_accurate_automation` is measured unsupported: FL delivered at most
  one point per queue, always at offset zero. Use a block ramp and disable
  topology automation.
- `float64_processing` is measured unsupported: the run delivered float
  callbacks and no double callbacks. Advertise only the proved float ability.
- `presentation_latency` has a reported value but an incomplete impulse
  golden. Do not perform subtractive cross-probe alignment.
- `aux_compare_pre` was observed without the required PDC and channel-identity
  golden. Permit state A/B only, not a local audio delta.
- `aux_priority_sidechain` has the same incomplete PDC/channel proof. Disable
  dynamic actuation.
- `contribution_aux` is unmeasured. Fall back to association rather than exact
  attribution.
- `binary_telemetry` is not yet adopted. Use reduced JSON cadence without P0
  loss.
- `remote_control` is not yet adopted. Keep the active probe locally
  operable.

Validate the capability file against schemas, raw fields, evidence prose,
fallbacks, and the expected support distribution:

```powershell
py -3.13 tools/eq-copilot/pruefe_host_capabilities.py
```

Missing `jsonschema` exits 3, an evidence or schema mismatch exits 2, and a
successful reconciliation exits 0. Upgrading a capability requires a new
controlled host run, immutable raw report, action log and limitations,
capability update, and successful reconciliation. Unit tests alone cannot
upgrade measured host support.

## Production boundary

The product VST3 compiles the patched JUCE wrapper but `EqCopilotProcessor`
does not implement `hostbruecke::Senke` and does not load the capability JSON.
Today the bridge is inert for the product and the capability document is a
validated evidence/adoption artifact, not runtime configuration.

A future adopter must implement the sink explicitly, define reset and
lifecycle behavior, and map every unsupported capability to its documented
fallback. New bridge fields need fixed-capacity storage, validity and overflow
semantics, patch anchors, serialization, and focused host-context tests.
