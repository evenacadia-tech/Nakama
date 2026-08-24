(function registerDemoAdapter(root) {
  "use strict";

  const N = root.Nakama;
  if (!N) throw new Error("contract.js must be loaded before demo-adapter.js");

  const SOURCE_LIBRARY = [
    "KICK",
    "SNARE",
    "BASS",
    "PIANO",
    "PADS",
    "LEAD VOX",
    "GUITAR",
    "PERCUSSION",
    "SYNTH",
    "STRINGS",
    "BRASS",
    "FX RETURN",
    "ROOM",
    "CHOIR",
    "SUB",
    "MUSIC BUS",
  ];

  const OVERVIEW_GOLDEN_NAMES = ["LEAD VOX", "DRUMS", "BASS", "PIANO", "SYNTH PAD"];
  const EQ_GOLDEN_NAMES = ["KICK", "SNARE", "BASS", "PIANO", "PADS"];

  function createBands() {
    return [
      { id: 1, enabled: true, type: "BELL", frequency: 36, gain: 0.6, q: 0.8, mode: "STEREO", dynamic: false, threshold: -24, range: -3, attack: 12, hold: 0, release: 180, reduction: 0 },
      { id: 2, enabled: true, type: "BELL", frequency: 102, gain: -1.0, q: 1.0, mode: "STEREO", dynamic: false, threshold: -24, range: -3, attack: 12, hold: 0, release: 180, reduction: 0 },
      { id: 3, enabled: true, type: "BELL", frequency: 240, gain: -1.5, q: 1.2, mode: "STEREO", dynamic: true, threshold: -24, range: -6, attack: 12, hold: 0, release: 180, reduction: -0.45 },
      { id: 4, enabled: true, type: "BELL", frequency: 580, gain: 1.6, q: 1.1, mode: "STEREO", dynamic: false, threshold: -24, range: -3, attack: 12, hold: 0, release: 180, reduction: 0 },
      { id: 5, enabled: true, type: "BELL", frequency: 3100, gain: -2.0, q: 1.5, mode: "STEREO", dynamic: true, threshold: -22, range: -4, attack: 18, hold: 0, release: 220, reduction: -0.25 },
      { id: 6, enabled: true, type: "BELL", frequency: 5900, gain: 2.2, q: 1.35, mode: "STEREO", dynamic: false, threshold: -24, range: -3, attack: 12, hold: 0, release: 180, reduction: 0 },
      { id: 7, enabled: false, type: "NOTCH", frequency: 9000, gain: 2.8, q: 3.2, mode: "STEREO", dynamic: false, threshold: -24, range: -3, attack: 12, hold: 0, release: 180, reduction: 0 },
      { id: 8, enabled: true, type: "HIGH SHELF", frequency: 13200, gain: 1.6, q: 0.7, mode: "STEREO", dynamic: false, threshold: -24, range: -3, attack: 12, hold: 0, release: 180, reduction: 0 },
    ];
  }

  function heatmapFor(index) {
    const templates = [
      [12, 18, 23, 32, 26, 26, 22, 18, 12],
      [26, 32, 22, 18, 18, 22, 26, 32, 22],
      [32, 32, 31, 30, 15, 15, 12, 8, 8],
      [12, 22, 32, 26, 22, 22, 18, 18, 12],
      [18, 18, 24, 30, 26, 22, 20, 16, 12],
    ];
    return templates[index % templates.length].slice();
  }

  function createSources(names, count) {
    const result = [];
    for (let index = 0; index < count; index += 1) {
      const name = names[index] || SOURCE_LIBRARY[index] || `SOURCE ${String(index + 1).padStart(2, "0")}`;
      result.push({
        id: `source-${index + 1}`,
        order: index + 1,
        name,
        connected: true,
        freshness: index === 4 && count === 5 ? "stale" : "fresh",
        confidence: index === 2 ? "high" : "medium",
        heatmap: heatmapFor(index),
      });
    }
    return result;
  }

  function sourceBands(sources) {
    const result = {};
    for (const source of sources) result[source.id] = createBands();
    result.master = createBands();
    return result;
  }

  function createState(fixtureName, nowMs) {
    const fixture = fixtureName || "overview-golden";
    const count = fixture === "sixteen-sources" ? 16 : 5;
    const names = fixture === "overview-golden" ? OVERVIEW_GOLDEN_NAMES : EQ_GOLDEN_NAMES;
    const sources = createSources(names, count);
    const selectedSourceId = sources[Math.min(2, sources.length - 1)].id;
    const time = Number.isFinite(nowMs) ? nowMs : 0;

    const state = {
      meta: {
        contractVersion: N.CONTRACT_VERSION,
        fixture,
        demo: true,
        nowMs: time,
        spectrumFrame: 0,
        lastSpectrumAt: time,
        notice: "",
      },
      sources,
      connections: {
        broker: "connected",
        probes: Object.fromEntries(sources.map((source) => [source.id, "connected"])),
      },
      measurement: {
        state: "measuring",
        progress: 68,
        freshness: "fresh",
        gapKind: null,
      },
      finding: {
        index: 1,
        total: 3,
        confidence: "high",
        targetSourceId: selectedSourceId,
        title: "Bass against piano, 180–260 Hz",
        cause: "Bass carries 3.4 dB too much there",
        test: "Bass −2 dB at 220 Hz, Q 1.2",
        listenFor: "Piano left hand, bars 33–41",
        then: "Re-check the same passage",
      },
      transport: {
        recordState: "playing",
      },
      probeEq: {
        targetId: selectedSourceId,
        bypassByTarget: Object.fromEntries([...sources.map((source) => [source.id, false]), ["master", false]]),
        globalsByTarget: Object.fromEntries(
          [...sources.map((source) => source.id), "master"].map((id) => [id, { inputTrim: 0, outputTrim: 0, width: 1, monoBass: 0 }]),
        ),
      },
      spectrum: {
        cadenceHz: 20,
        seed: 17,
        values: Array.from({ length: 64 }, (_, index) => 0.22 + 0.18 * Math.sin(index * 0.34) + 0.08 * Math.sin(index * 0.93)),
        previousValues: Array(64).fill(0.2),
      },
      eqBands: sourceBands(sources),
      draft: {
        state: "ready",
        sourceId: selectedSourceId,
        bandId: 3,
        values: { frequency: 240, gain: -3, q: 1.6 },
        leaseExpiresAt: null,
        staleReason: null,
      },
      revisions: Object.fromEntries([...sources.map((source) => source.id), "master"].map((id) => [id, []])),
      protectedRanges: [
        { id: "presence-protection", startHz: 3000, endHz: 6000, label: "PROTECTED" },
      ],
      protectionViolation: null,
      comparison: {
        sourceId: null,
      },
      view: {
        page: fixture === "eq-golden" ? "eq-center" : "overview",
        selectedSourceId,
        selectedBandId: 3,
        comparisonSourceId: null,
        openPanel: null,
        hoveredControl: null,
        heldControl: null,
        drag: null,
        lifecycle: "advise",
      },
    };

    if (fixture === "disconnected") {
      state.connections.broker = "broker-offline";
      state.meta.notice = "BROKER OFFLINE · CONFIRMED EQ REMAINS ACTIVE";
    }
    if (fixture === "recording-gate") {
      state.transport.recordState = "recording";
      state.meta.notice = "AUDITION BLOCKED · RECORDING IS ACTIVE";
    }
    if (fixture === "stale-automation") {
      state.draft.state = "stale";
      state.draft.staleReason = "EXTERNAL AUTOMATION CHANGED BAND 3";
      state.meta.notice = state.draft.staleReason;
    }
    return state;
  }

  function selectedTarget(state) {
    return state.probeEq.targetId || state.view.selectedSourceId;
  }

  function bandsFor(state, targetId) {
    return state.eqBands[targetId || selectedTarget(state)];
  }

  function findBand(state, bandId, targetId) {
    return bandsFor(state, targetId).find((band) => band.id === Number(bandId));
  }

  function connectedFor(state, sourceId) {
    return state.connections.broker === "connected" && state.connections.probes[sourceId] === "connected";
  }

  function previewBlockReason(state) {
    if (!connectedFor(state, state.draft.sourceId)) return "AUDITION BLOCKED · CONNECTION UNAVAILABLE";
    if (state.transport.recordState === "recording") return "AUDITION BLOCKED · RECORDING IS ACTIVE";
    if (state.transport.recordState === "unknown") return "AUDITION BLOCKED · RECORD STATE UNKNOWN";
    if (state.transport.recordState !== "playing") return "AUDITION BLOCKED · TRANSPORT STOPPED";
    if (state.draft.state === "stale") return "AUDITION BLOCKED · DRAFT IS STALE";
    if (state.draft.state === "none") return "NO DRAFT TO AUDITION";
    return "";
  }

  function closeTransientOnInvalidation(state, reason) {
    if (["auditioning", "lease"].includes(state.draft.state)) {
      state.draft.state = "none";
      state.draft.leaseExpiresAt = null;
      state.view.heldControl = null;
      state.meta.notice = reason;
    }
  }

  function applyParameter(state, action, allowProtectedBypass) {
    const targetId = action.targetId || selectedTarget(state);
    const band = findBand(state, action.bandId || state.view.selectedBandId, targetId);
    if (!band) return;
    const parameter = action.parameter;
    let value = Number(action.value);
    if (!Number.isFinite(value)) return;

    if (parameter === "frequency") {
      if (allowProtectedBypass) {
        value = N.clamp(value, 20, 20000);
        const violated = state.protectedRanges.find((range) => value > range.startHz && value < range.endHz);
        state.protectionViolation = violated
          ? { rangeId: violated.id, sourceId: targetId, bandId: band.id, origin: "external-automation" }
          : null;
      } else {
        const protectedResult = N.applyProtectedFrequency(band.frequency, value, state.protectedRanges);
        value = protectedResult.frequency;
        state.protectionViolation = protectedResult.collision
          ? { rangeId: protectedResult.collision, sourceId: targetId, bandId: band.id, origin: "nakama-control" }
          : null;
      }
    } else if (parameter === "gain") value = N.clamp(value, -12, 12);
    else if (parameter === "q") value = N.clamp(value, 0.15, 24);
    else if (parameter === "threshold") value = N.clamp(value, -60, 0);
    else if (parameter === "range") value = N.clamp(value, -12, 12);
    else if (parameter === "attack") value = N.clamp(value, 0.1, 500);
    else if (parameter === "hold") value = N.clamp(value, 0, 500);
    else if (parameter === "release") value = N.clamp(value, 5, 5000);
    else return;

    band[parameter] = value;
  }

  function reduce(state, action, nowMs) {
    const next = N.deepClone(state);
    const now = Number.isFinite(nowMs) ? nowMs : next.meta.nowMs;
    next.meta.nowMs = now;
    const A = N.ACTION;

    switch (action.type) {
      case A.NAVIGATE_PAGE:
        if (["overview", "eq-center"].includes(action.page)) next.view.page = action.page;
        next.view.openPanel = null;
        break;

      case A.SELECT_SOURCE:
        if (action.sourceId === "master" || next.sources.some((source) => source.id === action.sourceId)) {
          next.view.selectedSourceId = action.sourceId;
          next.probeEq.targetId = action.sourceId;
          next.view.selectedBandId = 3;
          next.meta.notice = action.sourceId === "master" ? "MASTER EQ SELECTED" : "";
        }
        break;

      case A.SELECT_COMPARISON_SOURCE:
        next.view.comparisonSourceId = action.sourceId || null;
        next.comparison.sourceId = action.sourceId || null;
        break;

      case A.SELECT_BAND:
        if (findBand(next, action.bandId)) next.view.selectedBandId = Number(action.bandId);
        break;

      case A.SET_BAND_ENABLED: {
        const band = findBand(next, action.bandId || next.view.selectedBandId);
        if (band) band.enabled = Boolean(action.enabled);
        break;
      }

      case A.SET_BAND_PARAMETER:
        applyParameter(next, action, false);
        break;

      case A.DRAG_BAND:
        applyParameter(next, { ...action, parameter: "frequency", value: action.frequency }, false);
        applyParameter(next, { ...action, parameter: "gain", value: action.gain }, false);
        break;

      case A.PREVIEW_BEGIN: {
        const reason = previewBlockReason(next);
        if (reason) {
          next.meta.notice = reason;
        } else {
          next.draft.state = "auditioning";
          next.view.heldControl = "audition";
          next.view.lifecycle = "audition";
          next.meta.notice = "AUDITION ACTIVE · RELEASE TO RETURN";
        }
        break;
      }

      case A.PREVIEW_END:
        if (next.draft.state === "auditioning") next.draft.state = "ready";
        next.view.heldControl = null;
        next.view.lifecycle = "advise";
        next.meta.notice = next.draft.state === "ready" ? "AUDITION ENDED · CONFIRMED EQ RESTORED" : next.meta.notice;
        break;

      case A.SEND_DRAFT:
        if (connectedFor(next, next.finding.targetSourceId)) {
          next.draft = {
            state: "ready",
            sourceId: next.finding.targetSourceId,
            bandId: 3,
            values: { frequency: 220, gain: -2, q: 1.2 },
            leaseExpiresAt: null,
            staleReason: null,
          };
          next.meta.notice = "DRAFT SENT · WAITING FOR PROBEEQ";
          next.view.lifecycle = "apply";
        } else {
          next.meta.notice = "DRAFT NOT SENT · TARGET PROBE OFFLINE";
        }
        break;

      case A.TEMPORARY_APPLY: {
        const reason = previewBlockReason(next);
        if (reason) next.meta.notice = reason.replace("AUDITION", "APPLY");
        else {
          next.draft.state = "lease";
          next.draft.leaseExpiresAt = now + 10000;
          next.view.heldControl = null;
          next.meta.notice = "TEMPORARY APPLY · CONFIRM WITHIN 10.0 S";
        }
        break;
      }

      case A.CONFIRM_APPLY:
        if (next.draft.state === "lease" && next.draft.leaseExpiresAt > now) {
          const targetId = next.draft.sourceId;
          const band = findBand(next, next.draft.bandId, targetId);
          next.revisions[targetId].push({
            id: `revision-${next.revisions[targetId].length + 1}`,
            bandId: band.id,
            before: { frequency: band.frequency, gain: band.gain, q: band.q },
            confirmedAt: now,
          });
          Object.assign(band, next.draft.values);
          next.draft.state = "none";
          next.draft.leaseExpiresAt = null;
          next.view.lifecycle = "observe";
          next.measurement.state = "measuring";
          next.measurement.progress = 0;
          next.meta.notice = "EQ CONFIRMED · REVISION CREATED";
        }
        break;

      case A.REJECT_DRAFT:
        next.draft.state = "none";
        next.draft.leaseExpiresAt = null;
        next.view.heldControl = null;
        next.meta.notice = "DRAFT REJECTED · CONFIRMED EQ UNCHANGED";
        break;

      case A.UNDO: {
        const targetId = selectedTarget(next);
        const history = next.revisions[targetId];
        const revision = history.pop();
        if (revision) {
          Object.assign(findBand(next, revision.bandId, targetId), revision.before);
          next.meta.notice = "LAST CONFIRMED STEP UNDONE";
        } else next.meta.notice = "UNDO UNAVAILABLE · NO CONFIRMED REVISION";
        break;
      }

      case A.SET_BYPASS: {
        const targetId = action.targetId || selectedTarget(next);
        next.probeEq.bypassByTarget[targetId] = Boolean(action.enabled);
        next.meta.notice = next.probeEq.bypassByTarget[targetId] ? "BYPASS ACTIVE" : "BYPASS OFF";
        break;
      }

      case A.TOGGLE_PANEL:
        next.view.openPanel = next.view.openPanel === action.panel ? null : action.panel;
        break;

      case A.SET_CONNECTION:
        if (action.segment === "broker") next.connections.broker = action.state;
        if (action.segment === "probe" && action.sourceId) next.connections.probes[action.sourceId] = action.state;
        closeTransientOnInvalidation(next, `${String(action.state).toUpperCase().replace(/-/g, " ")} · PREVIEW REVERTED`);
        break;

      case A.SET_TRANSPORT:
        next.transport.recordState = action.recordState;
        if (action.recordState !== "playing") closeTransientOnInvalidation(next, "TRANSPORT INVALIDATED PREVIEW · CONFIRMED EQ RESTORED");
        break;

      case A.SET_MEASUREMENT:
        next.measurement.state = action.state;
        if (Number.isFinite(action.progress)) next.measurement.progress = N.clamp(action.progress, 0, 100);
        next.measurement.gapKind = action.gapKind || null;
        break;

      case A.EXTERNAL_AUTOMATION:
        applyParameter(next, action, true);
        if (next.draft.state !== "none") {
          next.draft.state = "stale";
          next.draft.leaseExpiresAt = null;
          next.draft.staleReason = "EXTERNAL AUTOMATION CHANGED THE TARGET";
        }
        next.meta.notice = next.protectionViolation
          ? "AUTOMATION CROSSED A PROTECTED RANGE · HOST VALUE IS AUTHORITATIVE"
          : "EXTERNAL AUTOMATION UPDATED THE CONFIRMED CURVE";
        break;

      case A.TICK: {
        if (next.draft.state === "lease" && next.draft.leaseExpiresAt <= now) {
          next.draft.state = "none";
          next.draft.leaseExpiresAt = null;
          next.meta.notice = "TEMPORARY APPLY EXPIRED · CONFIRMED EQ RESTORED";
        }
        const spectrumPeriod = 1000 / next.spectrum.cadenceHz;
        if (
          next.measurement.state === "measuring" &&
          next.transport.recordState === "playing" &&
          next.connections.broker === "connected" &&
          now - next.meta.lastSpectrumAt >= spectrumPeriod
        ) {
          next.spectrum.previousValues = next.spectrum.values.slice();
          next.meta.spectrumFrame += 1;
          const frame = next.meta.spectrumFrame;
          next.spectrum.values = next.spectrum.values.map((_, index) => {
            const value = 0.24 + 0.15 * Math.sin(index * 0.31 + frame * 0.17) + 0.07 * Math.sin(index * 0.91 - frame * 0.11);
            return N.clamp(value, 0.04, 0.72);
          });
          next.meta.lastSpectrumAt = now;
          next.measurement.progress = Math.min(100, next.measurement.progress + 1);
          if (next.measurement.progress >= 100) next.measurement.state = "fresh";
        }
        break;
      }

      default:
        break;
    }

    return next;
  }

  class DemoAdapter {
    constructor(options) {
      const config = options || {};
      this.clock = config.clock || { now: () => performance.now() };
      this.state = createState(config.fixture || "overview-golden", this.clock.now());
      this.listeners = new Set();
    }

    getSnapshot() {
      return N.deepClone(this.state);
    }

    dispatch(action) {
      if (!action || typeof action.type !== "string") throw new TypeError("UiAction.type is required");
      if (action.type === N.ACTION.LOAD_FIXTURE) {
        this.state = createState(action.fixture, this.clock.now());
      } else {
        this.state = reduce(this.state, action, this.clock.now());
      }
      for (const listener of this.listeners) listener(this.getSnapshot(), action);
      return this.getSnapshot();
    }

    subscribe(listener) {
      this.listeners.add(listener);
      return () => this.listeners.delete(listener);
    }

    setClock(clock) {
      if (!clock || typeof clock.now !== "function") throw new TypeError("Clock must expose now()");
      this.clock = clock;
    }

    needsTick() {
      return (
        this.state.draft.state === "lease" ||
        (this.state.measurement.state === "measuring" &&
          this.state.transport.recordState === "playing" &&
          this.state.connections.broker === "connected")
      );
    }
  }

  N.createDemoState = createState;
  N.reduceDemoState = reduce;
  N.DemoAdapter = DemoAdapter;
  N.previewBlockReason = previewBlockReason;
})(globalThis);
