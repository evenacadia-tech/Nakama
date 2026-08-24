(function startNakamaSimulator(root) {
  "use strict";

  const N = root.Nakama;
  if (!N || !N.DemoAdapter || !N.renderSimulator) {
    throw new Error("Nakama simulator modules must be loaded before app.js");
  }

  const GRAPH_WIDTH = 652;
  const GRAPH_HEIGHT = 270;
  const FIXTURES = new Set([
    "overview-golden",
    "eq-golden",
    "sixteen-sources",
    "disconnected",
    "recording-gate",
    "stale-automation",
  ]);
  const SCALES = new Set([1, 1.25, 1.5, 2]);
  const query = new URLSearchParams(root.location.search);
  const captureMode = ["1", "true"].includes(query.get("capture"));
  const requestedFixture = FIXTURES.has(query.get("fixture")) ? query.get("fixture") : "overview-golden";

  const controlledClock = {
    value: 0,
    now() {
      return this.value;
    },
    advance(milliseconds) {
      this.value += Math.max(0, Number(milliseconds) || 0);
    },
  };
  const liveClock = { now: () => performance.now() };
  const adapter = new N.DemoAdapter({ fixture: requestedFixture, clock: captureMode ? controlledClock : liveClock });
  const audio = new N.DemoAudioLoop();

  const stage = document.getElementById("pluginStage");
  const stageHolder = document.getElementById("stageHolder");
  const explanation = document.getElementById("contextExplanation");
  let explanationTimer = null;
  let state = adapter.getSnapshot();
  let tickTimer = null;
  let spectrumFrame = null;
  let dragPointer = null;
  let auditionPointer = null;
  let auditionKey = null;

  if (captureMode) document.body.classList.add("capture-mode");

  function safeStoredScale() {
    if (captureMode) return null;
    try {
      return root.localStorage.getItem("nakama-gen-prototype-scale");
    } catch {
      return null;
    }
  }

  function initialScale() {
    const queryScale = Number(query.get("scale"));
    if (SCALES.has(queryScale)) return queryScale;
    const storedScale = Number(safeStoredScale());
    return SCALES.has(storedScale) ? storedScale : 1;
  }

  function setScale(value, persist = true) {
    const scale = SCALES.has(Number(value)) ? Number(value) : 1;
    document.documentElement.style.setProperty("--preview-scale", String(scale));
    document.getElementById("scaleSelect").value = String(scale);
    if (captureMode) {
      document.body.style.width = `${Math.round(760 * scale)}px`;
      document.body.style.height = `${Math.round(430 * scale)}px`;
    }
    if (persist && !captureMode) {
      try {
        root.localStorage.setItem("nakama-gen-prototype-scale", String(scale));
      } catch {
        // A blocked storage policy must not make the simulator unusable.
      }
    }
  }

  function shouldAnimateSpectrum(snapshot) {
    const targetId = snapshot.probeEq.targetId;
    const connected = targetId === "master" || (
      snapshot.connections.broker === "connected" && snapshot.connections.probes[targetId] === "connected"
    );
    return (
      !captureMode &&
      snapshot.view.page === "eq-center" &&
      snapshot.measurement.state === "measuring" &&
      snapshot.transport.recordState === "playing" &&
      connected
    );
  }

  function paintSpectrumFrame(now) {
    spectrumFrame = null;
    const period = 1000 / state.spectrum.cadenceHz;
    const interpolation = N.clamp((now - state.meta.lastSpectrumAt) / period, 0, 1);
    N.drawSpectrum(state, interpolation);
    if (shouldAnimateSpectrum(state)) spectrumFrame = root.requestAnimationFrame(paintSpectrumFrame);
  }

  function syncActivity() {
    const shouldTick = !captureMode && adapter.needsTick();
    if (shouldTick && tickTimer === null) {
      tickTimer = root.setInterval(() => adapter.dispatch({ type: N.ACTION.TICK }), 50);
    } else if (!shouldTick && tickTimer !== null) {
      root.clearInterval(tickTimer);
      tickTimer = null;
    }

    const animateSpectrum = shouldAnimateSpectrum(state);
    if (animateSpectrum && spectrumFrame === null) spectrumFrame = root.requestAnimationFrame(paintSpectrumFrame);
    if (!animateSpectrum && spectrumFrame !== null) {
      root.cancelAnimationFrame(spectrumFrame);
      spectrumFrame = null;
    }
  }

  function render(snapshot) {
    state = snapshot;
    N.renderSimulator(snapshot);
    N.drawSpectrum(snapshot, captureMode ? 1 : N.clamp((performance.now() - snapshot.meta.lastSpectrumAt) / 50, 0, 1));
    audio.setAudition(snapshot.draft.state === "auditioning");
    syncActivity();
  }

  function dispatch(action) {
    return adapter.dispatch(action);
  }

  function numericInputValue(value) {
    const normalized = String(value).trim().replace(/−/g, "-").replace(/,/g, ".");
    return normalized === "" ? Number.NaN : Number(normalized);
  }

  function hideExplanation() {
    if (explanationTimer !== null) root.clearTimeout(explanationTimer);
    explanationTimer = null;
    explanation.hidden = true;
    explanation.classList.remove("is-warning");
  }

  function showExplanation(message, warning = false) {
    if (explanationTimer !== null) root.clearTimeout(explanationTimer);
    explanation.textContent = message;
    explanation.classList.toggle("is-warning", warning);
    explanation.hidden = false;
    explanationTimer = root.setTimeout(hideExplanation, 6000);
  }

  function releaseAudition() {
    if (auditionPointer === null && auditionKey === null) return;
    auditionPointer = null;
    auditionKey = null;
    dispatch({ type: N.ACTION.PREVIEW_END });
  }

  function targetConnectionText() {
    const targetId = state.probeEq.targetId;
    if (state.connections.broker !== "connected") {
      return "BROKER OFFLINE · confirmed EQ remains active; preview and apply are unavailable.";
    }
    if (targetId !== "master" && state.connections.probes[targetId] !== "connected") {
      return "TARGET PROBE OFFLINE · confirmed EQ remains active; preview and apply are unavailable.";
    }
    return `${targetId === "master" ? "MASTER" : "TARGET PROBE"} CONNECTED · live controls are available.`;
  }

  function selectRelativeSource(delta) {
    const sourceIds = state.sources.map((source) => source.id);
    let index = sourceIds.indexOf(state.probeEq.targetId);
    if (index < 0) index = 0;
    index = N.clamp(index + delta, 0, sourceIds.length - 1);
    dispatch({ type: N.ACTION.SELECT_SOURCE, sourceId: sourceIds[index] });
  }

  function moveBandFromPointer(event) {
    if (!dragPointer || dragPointer.pointerId !== event.pointerId) return;
    const graph = document.getElementById("eqGraph");
    const rect = graph.getBoundingClientRect();
    const x = N.clamp((event.clientX - rect.left) * (GRAPH_WIDTH / rect.width), 0, GRAPH_WIDTH);
    const y = N.clamp((event.clientY - rect.top) * (GRAPH_HEIGHT / rect.height), 0, GRAPH_HEIGHT);
    dispatch({
      type: N.ACTION.DRAG_BAND,
      phase: "move",
      bandId: dragPointer.bandId,
      pointerId: event.pointerId,
      frequency: N.xToFrequency(x, GRAPH_WIDTH),
      gain: N.yToGain(y, GRAPH_HEIGHT),
    });
  }

  function endBandDrag(event) {
    if (!dragPointer || dragPointer.pointerId !== event.pointerId) return;
    if (event.type === "pointerup") moveBandFromPointer(event);
    dispatch({
      type: N.ACTION.DRAG_BAND,
      phase: "end",
      bandId: dragPointer.bandId,
      pointerId: event.pointerId,
    });
    dragPointer = null;
  }

  adapter.subscribe((snapshot) => render(snapshot));
  setScale(initialScale(), false);
  render(state);

  document.addEventListener("pointerdown", (event) => {
    const audition = event.target.closest("[data-hold-audition]");
    if (audition && !audition.disabled && event.button === 0) {
      event.preventDefault();
      auditionPointer = event.pointerId;
      audition.setPointerCapture?.(event.pointerId);
      dispatch({ type: N.ACTION.PREVIEW_BEGIN });
      return;
    }

    const handle = event.target.closest(".band-handle");
    if (handle && !handle.disabled && event.button === 0) {
      event.preventDefault();
      const bandId = Number(handle.dataset.bandId);
      dragPointer = { pointerId: event.pointerId, bandId };
      handle.setPointerCapture?.(event.pointerId);
      dispatch({ type: N.ACTION.DRAG_BAND, phase: "start", bandId, pointerId: event.pointerId });
    }
  });

  document.addEventListener("pointermove", (event) => {
    if (dragPointer) {
      event.preventDefault();
      moveBandFromPointer(event);
    }
  });

  document.addEventListener("pointerup", (event) => {
    endBandDrag(event);
    if (auditionPointer === event.pointerId) releaseAudition();
  });

  document.addEventListener("pointercancel", (event) => {
    endBandDrag(event);
    if (auditionPointer === event.pointerId) releaseAudition();
  });

  document.addEventListener("click", (event) => {
    const tab = event.target.closest("[data-page-target]");
    if (tab) {
      dispatch({ type: N.ACTION.NAVIGATE_PAGE, page: tab.dataset.pageTarget });
      return;
    }

    const source = event.target.closest("[data-source-id]");
    if (source) {
      dispatch({ type: N.ACTION.SELECT_SOURCE, sourceId: source.dataset.sourceId });
      return;
    }

    const handle = event.target.closest(".band-handle");
    if (handle) {
      dispatch({ type: N.ACTION.SELECT_BAND, bandId: Number(handle.dataset.bandId) });
      return;
    }

    if (event.target.closest("[data-hold-audition]")) {
      event.preventDefault();
      return;
    }

    const id = event.target.closest("button, input")?.id;
    if (id === "masterSwitch") dispatch({ type: N.ACTION.SELECT_SOURCE, sourceId: "master" });
    else if (id === "overviewApply") dispatch({ type: N.ACTION.SEND_DRAFT });
    else if (id === "overviewReject" || id === "eqReject") dispatch({ type: N.ACTION.REJECT_DRAFT });
    else if (id === "openInEq") {
      dispatch({ type: N.ACTION.SELECT_SOURCE, sourceId: state.finding.targetSourceId });
      dispatch({ type: N.ACTION.SELECT_BAND, bandId: state.draft.bandId || 3 });
      dispatch({ type: N.ACTION.NAVIGATE_PAGE, page: "eq-center" });
    } else if (id === "eqApply") {
      dispatch({ type: state.draft.state === "lease" ? N.ACTION.CONFIRM_APPLY : N.ACTION.TEMPORARY_APPLY });
    } else if (id === "undoButton") dispatch({ type: N.ACTION.UNDO });
    else if (id === "bandPower") {
      const selected = state.eqBands[state.probeEq.targetId].find((band) => band.id === state.view.selectedBandId);
      dispatch({ type: N.ACTION.SET_BAND_ENABLED, bandId: selected.id, enabled: !selected.enabled });
    } else if (id === "globalButton") dispatch({ type: N.ACTION.TOGGLE_PANEL, panel: "global" });
    else if (id === "bypassButton") {
      const targetId = state.probeEq.targetId;
      dispatch({ type: N.ACTION.SET_BYPASS, targetId, enabled: !state.probeEq.bypassByTarget[targetId] });
    } else if (id === "widthUnavailable" || id === "globalWidth") {
      showExplanation("WIDTH UNAVAILABLE · the current host capability contract exposes no versioned Width control.");
    } else if (id === "mixPlanned") {
      showExplanation("MIX · PLANNED · it becomes editable only after the versioned parameter contract exists.");
    } else if (id === "connectionAperture") showExplanation(targetConnectionText(), state.connections.broker !== "connected");

    if (
      state.view.openPanel === "global" &&
      !event.target.closest("#globalPanel") &&
      !event.target.closest("#globalButton")
    ) {
      dispatch({ type: N.ACTION.TOGGLE_PANEL, panel: "global" });
    }
  });

  document.addEventListener("keydown", (event) => {
    const hold = event.target.closest("[data-hold-audition]");
    if (hold && [" ", "Enter"].includes(event.key) && !event.repeat) {
      event.preventDefault();
      auditionKey = event.key;
      dispatch({ type: N.ACTION.PREVIEW_BEGIN });
      return;
    }

    const tab = event.target.closest("[data-page-target]");
    if (tab && ["ArrowLeft", "ArrowRight", "Home", "End"].includes(event.key)) {
      event.preventDefault();
      const page = event.key === "ArrowLeft" || event.key === "Home" ? "overview" : "eq-center";
      dispatch({ type: N.ACTION.NAVIGATE_PAGE, page });
      document.querySelector(`[data-page-target="${page}"]`).focus();
      return;
    }

    const source = event.target.closest(".source-choice");
    if (source && ["ArrowLeft", "ArrowRight", "Home", "End"].includes(event.key)) {
      event.preventDefault();
      if (event.key === "Home") dispatch({ type: N.ACTION.SELECT_SOURCE, sourceId: state.sources[0].id });
      else if (event.key === "End") dispatch({ type: N.ACTION.SELECT_SOURCE, sourceId: state.sources.at(-1).id });
      else selectRelativeSource(event.key === "ArrowRight" ? 1 : -1);
      root.requestAnimationFrame(() => document.querySelector('.source-choice[aria-checked="true"]')?.focus());
      return;
    }

    const handle = event.target.closest(".band-handle");
    if (handle && ["ArrowLeft", "ArrowRight", "ArrowUp", "ArrowDown"].includes(event.key)) {
      event.preventDefault();
      const bandId = Number(handle.dataset.bandId);
      const band = state.eqBands[state.probeEq.targetId].find((candidate) => candidate.id === bandId);
      const fine = event.shiftKey;
      if (event.key === "ArrowLeft" || event.key === "ArrowRight") {
        const ratio = fine ? 1.01 : 1.06;
        dispatch({
          type: N.ACTION.SET_BAND_PARAMETER,
          bandId,
          parameter: "frequency",
          value: event.key === "ArrowRight" ? band.frequency * ratio : band.frequency / ratio,
        });
      } else {
        dispatch({
          type: N.ACTION.SET_BAND_PARAMETER,
          bandId,
          parameter: "gain",
          value: band.gain + (event.key === "ArrowUp" ? 1 : -1) * (fine ? 0.1 : 0.5),
        });
      }
      return;
    }

    if (event.key === "Escape") {
      hideExplanation();
      releaseAudition();
      if (state.view.openPanel) dispatch({ type: N.ACTION.TOGGLE_PANEL, panel: state.view.openPanel });
    }
  });

  document.addEventListener("keyup", (event) => {
    if (auditionKey === event.key) {
      event.preventDefault();
      releaseAudition();
    }
  });

  document.addEventListener("pointerover", (event) => {
    const control = event.target.closest("button, input, select");
    if (!control || control.contains(event.relatedTarget)) return;
    dispatch({
      type: N.ACTION.SET_HOVERED_CONTROL,
      controlId: control.id || control.dataset.sourceId || control.dataset.bandId || control.dataset.param || null,
    });
  });

  document.addEventListener("pointerout", (event) => {
    const control = event.target.closest("button, input, select");
    if (!control || control.contains(event.relatedTarget)) return;
    dispatch({ type: N.ACTION.SET_HOVERED_CONTROL, controlId: null });
  });

  document.getElementById("sourceSwitcher").addEventListener("wheel", (event) => {
    event.preventDefault();
    selectRelativeSource((event.deltaY || event.deltaX) > 0 ? 1 : -1);
  }, { passive: false });

  document.getElementById("parameterForm").addEventListener("input", (event) => {
    const input = event.target.closest("input[data-param]");
    if (!input) return;
    dispatch({
      type: N.ACTION.SET_BAND_PARAMETER,
      bandId: state.view.selectedBandId,
      parameter: input.dataset.param,
      value: numericInputValue(input.value),
    });
  });

  document.getElementById("parameterForm").addEventListener("change", (event) => {
    if (event.target.id === "bandSelect") {
      dispatch({ type: N.ACTION.SELECT_BAND, bandId: Number(event.target.value) });
      return;
    }
    const control = event.target.closest("[data-param]");
    if (!control) return;
    dispatch({
      type: N.ACTION.SET_BAND_PARAMETER,
      bandId: state.view.selectedBandId,
      parameter: control.dataset.param,
      value: control.tagName === "INPUT" ? numericInputValue(control.value) : control.value,
    });
  });

  document.getElementById("globalPanel").addEventListener("input", (event) => {
    const control = event.target.closest("[data-global-param]");
    if (!control) return;
    dispatch({ type: N.ACTION.SET_GLOBAL_PARAMETER, parameter: control.dataset.globalParam, value: numericInputValue(control.value) });
  });

  document.getElementById("parameterForm").addEventListener("focusout", () => {
    root.setTimeout(() => render(state), 0);
  });

  document.getElementById("globalPanel").addEventListener("focusout", () => {
    root.setTimeout(() => render(state), 0);
  });

  document.getElementById("globalWidth").addEventListener("focus", () => {
    showExplanation("WIDTH UNAVAILABLE · the current host capability contract exposes no versioned Width control.");
  });

  document.getElementById("fixtureSelect").addEventListener("change", (event) => {
    dispatch({ type: N.ACTION.LOAD_FIXTURE, fixture: event.target.value });
  });
  document.getElementById("scaleSelect").addEventListener("change", (event) => setScale(event.target.value));
  document.getElementById("transportSelect").addEventListener("change", (event) => {
    dispatch({ type: N.ACTION.SET_TRANSPORT, recordState: event.target.value });
  });
  document.getElementById("measurementSelect").addEventListener("change", (event) => {
    const progress = event.target.value === "measuring" ? 68 : event.target.value === "incomplete" ? 44 : 100;
    dispatch({ type: N.ACTION.SET_MEASUREMENT, state: event.target.value, progress });
  });
  document.getElementById("comparisonSelect").addEventListener("change", (event) => {
    dispatch({ type: N.ACTION.SELECT_COMPARISON_SOURCE, sourceId: event.target.value || null });
  });
  document.getElementById("connectionSelect").addEventListener("change", (event) => {
    const mode = event.target.value;
    const targetId = state.probeEq.targetId;
    if (mode === "broker-offline") {
      dispatch({ type: N.ACTION.SET_CONNECTION, segment: "broker", state: "broker-offline" });
      return;
    }
    dispatch({ type: N.ACTION.SET_CONNECTION, segment: "broker", state: "connected" });
    if (targetId !== "master") {
      dispatch({
        type: N.ACTION.SET_CONNECTION,
        segment: "probe",
        sourceId: targetId,
        state: mode === "probe-offline" ? "probe-offline" : "connected",
      });
    } else if (mode === "probe-offline") {
      showExplanation("MASTER has no source Probe to disconnect. Select a source target first.");
    }
  });
  document.getElementById("automationButton").addEventListener("click", () => {
    dispatch({ type: N.ACTION.NAVIGATE_PAGE, page: "eq-center" });
    dispatch({
      type: N.ACTION.EXTERNAL_AUTOMATION,
      bandId: 5,
      parameter: "frequency",
      value: 4200,
    });
  });
  document.getElementById("audioButton").addEventListener("click", async (event) => {
    const button = event.currentTarget;
    button.disabled = true;
    try {
      const running = await audio.toggle();
      button.setAttribute("aria-pressed", String(running));
      button.textContent = running ? "Demo-Loop stoppen" : "Demo-Loop starten";
      document.getElementById("inspectorStatus").textContent = running
        ? "Interner lizenzfreier Demo-Loop aktiv · Bedien-Demo, keine DSP-Referenz."
        : "Demo-Loop gestoppt.";
    } catch (error) {
      document.getElementById("inspectorStatus").textContent = `Demo-Audio nicht verfügbar: ${error.message}`;
    } finally {
      button.disabled = false;
    }
  });

  root.addEventListener("blur", () => {
    releaseAudition();
    if (dragPointer) {
      dispatch({ type: N.ACTION.DRAG_BAND, phase: "end", bandId: dragPointer.bandId, pointerId: dragPointer.pointerId });
      dragPointer = null;
    }
  });
  document.addEventListener("visibilitychange", () => {
    if (document.hidden) releaseAudition();
  });
  root.addEventListener("beforeunload", () => {
    if (audio.running) void audio.stop();
  });

  async function waitForVisualReadiness() {
    await document.fonts.ready;
    await Promise.all(
      Array.from(document.images, (image) => image.decode?.().catch(() => undefined) || Promise.resolve()),
    );
    await new Promise((resolve) => root.requestAnimationFrame(() => root.requestAnimationFrame(resolve)));
    document.body.dataset.ready = "true";
    return true;
  }

  const ready = waitForVisualReadiness();
  root.__nakama = {
    adapter,
    audio,
    ready,
    getSnapshot: () => adapter.getSnapshot(),
    dispatch,
    setScale,
    step(milliseconds) {
      if (!captureMode) throw new Error("step() is available only with ?capture=1");
      controlledClock.advance(milliseconds);
      return dispatch({ type: N.ACTION.TICK });
    },
    freezeMotionAt(progress) {
      const normalized = N.clamp(progress, 0, 1);
      for (const animation of stage.getAnimations({ subtree: true })) {
        const duration = Number(animation.effect?.getTiming().duration);
        if (!Number.isFinite(duration) || duration <= 0) continue;
        animation.pause();
        animation.currentTime = duration * normalized;
      }
    },
    resumeMotion() {
      for (const animation of stage.getAnimations({ subtree: true })) animation.play();
    },
    stage,
    stageHolder,
  };
})(globalThis);
