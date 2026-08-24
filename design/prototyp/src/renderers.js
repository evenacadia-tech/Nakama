(function registerRenderers(root) {
  "use strict";

  const N = root.Nakama;
  if (!N) throw new Error("contract.js must be loaded before renderers.js");

  const GRAPH_WIDTH = 652;
  const GRAPH_HEIGHT = 270;
  const OVERVIEW_ROW_HEIGHT = 31;

  function byId(id) {
    return document.getElementById(id);
  }

  function sourceById(state, id) {
    return state.sources.find((source) => source.id === id);
  }

  function targetName(state, id) {
    if (id === "master") return "MASTER";
    return sourceById(state, id)?.name || "SOURCE";
  }

  function bandsFor(state, id) {
    return state.eqBands[id || state.probeEq.targetId] || [];
  }

  function sourceConnected(state, sourceId) {
    return state.connections.broker === "connected" && state.connections.probes[sourceId] === "connected";
  }

  function responseAtFrequency(bands, frequency) {
    let total = 0;
    for (const band of bands) {
      if (!band.enabled) continue;
      const ratio = Math.max(0.0001, frequency / band.frequency);
      const logDistance = Math.log2(ratio);
      switch (band.type) {
        case "LOW SHELF":
          total += band.gain / (1 + Math.pow(ratio, 3.2));
          break;
        case "HIGH SHELF":
          total += band.gain / (1 + Math.pow(1 / ratio, 3.2));
          break;
        case "LOW CUT":
          total += -12 / (1 + 3 * Math.pow(ratio, 5));
          break;
        case "HIGH CUT":
          total += -12 / (1 + 3 * Math.pow(1 / ratio, 5));
          break;
        case "NOTCH":
          total += -Math.max(3, Math.abs(band.gain)) * Math.exp(-0.5 * Math.pow(logDistance * band.q * 2.1, 2));
          break;
        default:
          total += band.gain * Math.exp(-0.5 * Math.pow(logDistance * band.q * 1.45, 2));
          break;
      }
    }
    return N.clamp(total, -12, 12);
  }

  function curvePath(bands) {
    const points = [];
    for (let index = 0; index <= 180; index += 1) {
      const x = (index / 180) * GRAPH_WIDTH;
      const frequency = N.xToFrequency(x, GRAPH_WIDTH);
      const y = N.gainToY(responseAtFrequency(bands, frequency), GRAPH_HEIGHT);
      points.push(`${index === 0 ? "M" : "L"}${x.toFixed(2)},${y.toFixed(2)}`);
    }
    return points.join(" ");
  }

  function withDraft(state, sourceId) {
    const result = N.deepClone(bandsFor(state, sourceId));
    if (state.draft.state === "none" || state.draft.sourceId !== sourceId) return result;
    const target = result.find((band) => band.id === state.draft.bandId);
    if (target) Object.assign(target, state.draft.values);
    return result;
  }

  function withDynamicReduction(bands) {
    return bands.map((band) => ({ ...band, gain: band.gain + (band.dynamic ? band.reduction || 0 : 0) }));
  }

  function renderPage(state) {
    const stage = byId("pluginStage");
    const overviewPanel = byId("panel-overview");
    const eqPanel = byId("panel-eq-center");
    const overviewActive = state.view.page === "overview";

    stage.dataset.page = state.view.page;
    overviewPanel.hidden = false;
    eqPanel.hidden = false;
    overviewPanel.classList.toggle("is-active", overviewActive);
    eqPanel.classList.toggle("is-active", !overviewActive);
    overviewPanel.setAttribute("aria-hidden", String(!overviewActive));
    eqPanel.setAttribute("aria-hidden", String(overviewActive));

    for (const tab of document.querySelectorAll("[data-page-target]")) {
      const selected = tab.dataset.pageTarget === state.view.page;
      tab.classList.toggle("is-selected", selected);
      tab.setAttribute("aria-selected", String(selected));
      tab.tabIndex = selected ? 0 : -1;
    }

    for (const item of document.querySelectorAll("[data-lifecycle]")) {
      item.classList.toggle("is-current", item.dataset.lifecycle === state.view.lifecycle);
    }
  }

  function renderSourceSwitcher(state) {
    const track = byId("sourceSwitcherTrack");
    const signature = state.sources.map((source) => `${source.id}:${source.name}`).join("|");
    if (track.dataset.signature !== signature) {
      track.dataset.signature = signature;
      track.replaceChildren(
        ...state.sources.map((source) => {
          const button = document.createElement("button");
          button.type = "button";
          button.className = "source-choice";
          button.dataset.sourceId = source.id;
          button.setAttribute("role", "radio");
          button.textContent = source.name;
          return button;
        }),
      );
    }

    for (const choice of track.querySelectorAll(".source-choice")) {
      const selected = choice.dataset.sourceId === state.probeEq.targetId;
      choice.setAttribute("aria-checked", String(selected));
      choice.tabIndex = selected ? 0 : -1;
    }

    const selectedChoice = track.querySelector('[aria-checked="true"]');
    const switcher = byId("sourceSwitcher");
    if (selectedChoice) {
      const maximum = Math.max(0, track.scrollWidth - switcher.clientWidth);
      const desired = N.clamp(
        selectedChoice.offsetLeft + selectedChoice.offsetWidth / 2 - switcher.clientWidth / 2,
        0,
        maximum,
      );
      track.style.transform = `translateX(${-desired}px)`;
    }

    const masterSelected = state.probeEq.targetId === "master";
    byId("masterSwitch").setAttribute("aria-pressed", String(masterSelected));
  }

  function overviewContentHeight(sourceCount) {
    const count = Math.max(0, Number(sourceCount) || 0);
    if (count <= 5) return 149;
    return (count - 1) * OVERVIEW_ROW_HEIGHT + 25;
  }

  function renderOverviewRows(state) {
    const rows = byId("probeRows");
    const sources = state.sources;
    rows.classList.toggle("is-scrollable", sources.length > 5);
    const signature = sources.map((source) => `${source.id}:${source.name}`).join("|");
    if (rows.dataset.signature !== signature) {
      rows.dataset.signature = signature;
      rows.querySelectorAll(".probe-row").forEach((row) => row.remove());
      sources.forEach((source, index) => {
        const button = document.createElement("button");
        button.type = "button";
        button.className = "probe-row";
        button.dataset.sourceId = source.id;
        button.style.top = `${index * OVERVIEW_ROW_HEIGHT}px`;
        const indexLabel = document.createElement("span");
        indexLabel.className = "probe-row-index";
        indexLabel.textContent = String(source.order).padStart(2, "0");
        const nameLabel = document.createElement("span");
        nameLabel.className = "probe-row-name";
        const stateLabel = document.createElement("span");
        stateLabel.className = "probe-row-state";
        button.append(indexLabel, nameLabel, stateLabel);
        rows.append(button);
      });
    }

    for (const button of rows.querySelectorAll(".probe-row")) {
      const source = sourceById(state, button.dataset.sourceId);
      const selected = source.id === state.view.selectedSourceId;
      const status = !sourceConnected(state, source.id)
        ? "OFFLINE"
        : selected
          ? "FOCUS"
          : source.freshness === "stale"
            ? "STALE"
            : source.freshness === "incomplete"
              ? "INCOMPLETE"
              : "LIVE";
      button.classList.toggle("is-selected", selected);
      button.classList.toggle("is-stale", source.freshness === "stale");
      button.querySelector(".probe-row-name").textContent = source.name;
      button.querySelector(".probe-row-state").textContent = status;
      button.setAttribute("aria-pressed", String(selected));
      button.setAttribute("aria-label", `${source.name}, ${status}`);
    }

    if (rows.dataset.selectedSourceId !== state.view.selectedSourceId) {
      rows.dataset.selectedSourceId = state.view.selectedSourceId;
      const selectedRow = rows.querySelector(`[data-source-id="${state.view.selectedSourceId}"]`);
      if (selectedRow) {
        const rowTop = selectedRow.offsetTop;
        const rowBottom = rowTop + selectedRow.offsetHeight;
        if (rowTop < rows.scrollTop) rows.scrollTop = rowTop;
        else if (rowBottom > rows.scrollTop + rows.clientHeight) rows.scrollTop = rowBottom - rows.clientHeight;
      }
    }
  }

  function setupCanvas(canvas, cssWidth, cssHeight) {
    const ratio = Math.max(1, Math.min(4, root.devicePixelRatio || 1));
    const width = Math.round(cssWidth * ratio);
    const height = Math.round(cssHeight * ratio);
    if (canvas.width !== width || canvas.height !== height) {
      canvas.width = width;
      canvas.height = height;
    }
    const context = canvas.getContext("2d");
    context.setTransform(ratio, 0, 0, ratio, 0, 0);
    return context;
  }

  function drawHeatmap(state) {
    const canvas = byId("heatmapCanvas");
    const sources = state.sources;
    const contentHeight = overviewContentHeight(sources.length);
    canvas.style.height = `${contentHeight}px`;
    const context = setupCanvas(canvas, 704, contentHeight);
    context.clearRect(0, 0, 704, contentHeight);

    sources.forEach((source, rowIndex) => {
      const y = rowIndex * OVERVIEW_ROW_HEIGHT + 3;
      context.fillStyle = "rgba(9, 10, 11, 0.70)";
      context.fillRect(112, y, 592, 19);
      source.heatmap.forEach((level, bandIndex) => {
        const selected = source.id === state.view.selectedSourceId && (bandIndex === 2 || bandIndex === 3);
        const stale = source.freshness === "stale";
        const x = 112 + bandIndex * 66;
        if (stale) {
          context.strokeStyle = "rgba(133, 139, 144, 0.22)";
          context.setLineDash([3, 3]);
          context.strokeRect(x + 0.5, y + 0.5, 62, 18);
          context.setLineDash([]);
        } else {
          const value = selected ? "rgba(82, 18, 32, 0.98)" : `rgba(${level}, ${level + 3}, ${level + 5}, 0.96)`;
          context.fillStyle = value;
          context.fillRect(x, y, 63, 19);
          if (selected) {
            context.strokeStyle = "rgba(141, 147, 154, 0.95)";
            context.strokeRect(x + 0.5, y + 0.5, 62, 18);
          }
        }
      });
    });
  }

  function measurementText(state) {
    const measurement = state.measurement;
    if (state.connections.broker !== "connected") return "MEASUREMENT DISCONNECTED";
    if (measurement.state === "measuring") return `MEASURING ${Math.round(measurement.progress)}%`;
    if (measurement.state === "fresh") return "MEASUREMENT FRESH";
    if (measurement.state === "stale") return "MEASUREMENT STALE";
    if (measurement.state === "incomplete") return "MEASUREMENT INCOMPLETE";
    return "MEASUREMENT UNAVAILABLE";
  }

  function renderOverview(state) {
    renderOverviewRows(state);
    drawHeatmap(state);
    const linked = state.connections.broker === "connected"
      ? state.sources.filter((source) => state.connections.probes[source.id] === "connected").length
      : 0;
    const stale = state.sources.filter((source) => source.freshness === "stale").length;
    const selected = sourceById(state, state.view.selectedSourceId) || state.sources[0];

    byId("probeCount").textContent = `${state.sources.length} PROBES`;
    byId("staleCount").textContent = `${stale} STALE`;
    byId("measurementLabel").textContent = measurementText(state);
    byId("linkedReadout").textContent = `${linked} / ${state.sources.length} LINKED`;
    byId("focusReadout").textContent = `FOCUS  /  ${selected.name}  /  180–260 HZ`;
    byId("findingMeta").textContent = `FINDING ${state.finding.index} OF ${state.finding.total} · CONFIDENCE ${state.finding.confidence.toUpperCase()}`;
    byId("findingScore").textContent = `${linked} / ${state.sources.length}`;
    byId("findingPriority").textContent = state.finding.title;
    byId("findingCause").textContent = state.finding.cause;
    byId("findingTest").textContent = state.finding.test;
    byId("findingListen").textContent = state.finding.listenFor;
    byId("findingThen").textContent = state.finding.then;
    byId("testReadyText").textContent = `TEST READY  /  ${targetName(state, state.finding.targetSourceId)}  −2.0 DB  @  220 HZ  /  Q 1.2`;

    const auditioning = state.draft.state === "auditioning";
    document.querySelectorAll("[data-hold-audition]").forEach((button) => {
      button.classList.toggle("is-active", auditioning);
      button.setAttribute("aria-pressed", String(auditioning));
      button.disabled = state.draft.state === "none" || state.draft.state === "lease";
    });
  }

  function renderHandles(state) {
    const container = byId("bandHandles");
    const bands = bandsFor(state);
    const targetId = state.probeEq.targetId;
    const connected = targetId === "master" || (
      state.connections.broker === "connected" && state.connections.probes[targetId] === "connected"
    );
    const signature = bands.map((band) => band.id).join("|");
    if (container.dataset.signature !== signature) {
      container.dataset.signature = signature;
      container.replaceChildren(
        ...bands.map((band) => {
          const button = document.createElement("button");
          button.type = "button";
          button.className = "band-handle";
          button.dataset.bandId = String(band.id);
          const visual = document.createElement("span");
          visual.className = "handle-visual";
          visual.textContent = String(band.id);
          button.append(visual);
          return button;
        }),
      );
    }

    for (const button of container.querySelectorAll(".band-handle")) {
      const band = bands.find((candidate) => candidate.id === Number(button.dataset.bandId));
      const selected = band.id === state.view.selectedBandId;
      button.style.left = `${N.frequencyToX(band.frequency, GRAPH_WIDTH)}px`;
      button.style.top = `${N.gainToY(band.gain, GRAPH_HEIGHT)}px`;
      button.classList.toggle("is-selected", selected);
      button.classList.toggle("is-disabled", !band.enabled);
      button.disabled = !connected;
      button.setAttribute("aria-pressed", String(selected));
      button.setAttribute(
        "aria-label",
        `Band ${band.id}, ${band.enabled ? "enabled" : "disabled"}, ${N.formatFrequency(band.frequency)}, ${N.formatDb(band.gain, 1)}, Q ${band.q.toFixed(2)}`,
      );
      button.tabIndex = selected ? 0 : -1;
    }

    let draftTarget = container.querySelector(".draft-target");
    if (state.draft.state !== "none" && state.draft.sourceId === state.probeEq.targetId) {
      if (!draftTarget) {
        draftTarget = document.createElement("div");
        draftTarget.className = "draft-target";
        container.append(draftTarget);
      }
      const confirmed = bands.find((band) => band.id === state.draft.bandId);
      const draftX = N.frequencyToX(state.draft.values.frequency, GRAPH_WIDTH);
      const draftY = N.gainToY(state.draft.values.gain, GRAPH_HEIGHT);
      const confirmedY = N.gainToY(confirmed.gain, GRAPH_HEIGHT);
      draftTarget.style.left = `${draftX}px`;
      draftTarget.style.top = `${draftY}px`;
      draftTarget.style.setProperty("--connector-height", `${Math.max(4, Math.abs(draftY - confirmedY) - 5)}px`);
      draftTarget.hidden = false;
    } else if (draftTarget) draftTarget.hidden = true;
  }

  function renderCurves(state) {
    const targetId = state.probeEq.targetId;
    const confirmedBands = bandsFor(state, targetId);
    const confirmed = curvePath(confirmedBands);
    byId("confirmedPath").setAttribute("d", confirmed);
    byId("confirmedFillPath").setAttribute("d", `M0,135 ${confirmed.replace(/^M/, "L")} L652,135 Z`);
    byId("dynamicPath").setAttribute("d", curvePath(withDynamicReduction(confirmedBands)));

    const bandCurves = byId("bandCurves");
    if (bandCurves.children.length !== confirmedBands.length) {
      bandCurves.replaceChildren(
        ...confirmedBands.map(() => {
          const path = document.createElementNS("http://www.w3.org/2000/svg", "path");
          path.setAttribute("class", "individual-band-curve");
          return path;
        }),
      );
    }
    confirmedBands.forEach((band, index) => {
      bandCurves.children[index].setAttribute("d", curvePath([band]));
      bandCurves.children[index].classList.toggle("is-disabled", !band.enabled);
    });

    const draftVisible = state.draft.state !== "none" && state.draft.sourceId === targetId;
    byId("draftPath").setAttribute("d", draftVisible ? curvePath(withDraft(state, targetId)) : "");
    byId("draftPath").style.opacity = draftVisible ? "1" : "0";

    const comparisonPath = byId("comparisonPath");
    const comparisonVisible = Boolean(state.view.comparisonSourceId && state.view.comparisonSourceId !== targetId);
    comparisonPath.classList.toggle("is-visible", comparisonVisible);
    comparisonPath.setAttribute(
      "d",
      comparisonVisible ? curvePath(bandsFor(state, state.view.comparisonSourceId)) : "",
    );

    byId("spectrumLabel").textContent = state.draft.state === "auditioning" || state.draft.state === "lease"
      ? "SPECTRUM POST · CANDIDATE"
      : "SPECTRUM POST · COMMITTED";
  }

  function setInputValue(input, value) {
    if (document.activeElement !== input) input.value = String(value);
  }

  function renderParameters(state) {
    const selected = bandsFor(state).find((band) => band.id === state.view.selectedBandId) || bandsFor(state)[0];
    byId("bandSelect").value = String(selected.id);
    byId("paramType").value = selected.type;
    setInputValue(byId("paramFrequency"), Math.round(selected.frequency));
    setInputValue(byId("paramGain"), N.formatDb(selected.gain, 1).replace(" dB", ""));
    setInputValue(byId("paramQ"), selected.q.toFixed(2));
    byId("paramMode").value = selected.mode;
    byId("paramDynamic").value = selected.dynamic ? "ON" : "OFF";
    setInputValue(byId("paramThreshold"), N.formatDb(selected.threshold, 0).replace(" dB", ""));
    setInputValue(byId("paramRange"), N.formatDb(selected.range, 0).replace(" dB", ""));
    setInputValue(byId("paramAttack"), selected.attack);
    setInputValue(byId("paramHold"), selected.hold);
    setInputValue(byId("paramRelease"), selected.release);
    byId("bandPower").setAttribute("aria-pressed", String(selected.enabled));
    byId("bandPower").setAttribute("aria-label", `${selected.enabled ? "Disable" : "Enable"} band ${selected.id}`);

    const connected = state.probeEq.targetId === "master" || (
      state.connections.broker === "connected" &&
      state.connections.probes[state.probeEq.targetId] === "connected"
    );
    for (const control of byId("parameterForm").querySelectorAll("input, select, button")) {
      control.disabled = !connected || (control.dataset.param && ["threshold", "range", "attack", "hold", "release"].includes(control.dataset.param) && !selected.dynamic);
    }
  }

  function renderDraft(state) {
    const card = byId("draftCard");
    const draft = state.draft;
    const targetMatches = draft.sourceId === state.probeEq.targetId;
    const visible = draft.state !== "none" && targetMatches;
    const hadFocus = card.contains(document.activeElement);
    card.classList.toggle("is-empty", !visible);
    card.classList.toggle("is-stale", draft.state === "stale");
    card.setAttribute("aria-hidden", String(!visible));
    card.inert = !visible;

    if (!visible) {
      byId("eqApply").textContent = "APPLY";
      for (const button of card.querySelectorAll("button")) button.disabled = true;
      if (hadFocus) {
        root.requestAnimationFrame(() => selectedBandHandle(state)?.focus());
      }
      return;
    }
    for (const button of card.querySelectorAll("button")) button.disabled = false;
    const confirmed = bandsFor(state, draft.sourceId).find((band) => band.id === draft.bandId);
    byId("draftTitle").textContent = draft.state === "stale" ? "DRAFT · STALE" : "DRAFT";
    byId("draftDescription").textContent = draft.state === "stale"
      ? draft.staleReason
      : `Band ${draft.bandId} · ${N.formatFrequency(draft.values.frequency)} · ${N.formatDb(confirmed.gain, 1).replace(" dB", "")} → ${N.formatDb(draft.values.gain, 1)} · Q ${draft.values.q.toFixed(1)}`;

    const apply = byId("eqApply");
    if (draft.state === "lease") {
      const remaining = Math.max(0, (draft.leaseExpiresAt - state.meta.nowMs) / 1000);
      apply.textContent = `CONFIRM ${remaining.toFixed(1)} S`;
    } else apply.textContent = "APPLY";
    apply.disabled = draft.state === "stale";
    byId("eqReject").disabled = false;
  }

  function selectedBandHandle(state) {
    return document.querySelector(`.band-handle[data-band-id="${state.view.selectedBandId}"]`);
  }

  function renderConnectionAndUtilities(state) {
    const targetId = state.probeEq.targetId;
    const connected = targetId === "master" || (
      state.connections.broker === "connected" && state.connections.probes[targetId] === "connected"
    );
    const aperture = byId("connectionAperture");
    aperture.classList.toggle("is-offline", !connected);
    aperture.setAttribute("aria-label", connected ? `${targetName(state, targetId)} connected` : `${targetName(state, targetId)} unavailable`);

    const bypass = Boolean(state.probeEq.bypassByTarget[targetId]);
    byId("bypassButton").setAttribute("aria-pressed", String(bypass));
    byId("pluginStage").dataset.bypass = String(bypass);
    const globalOpen = state.view.openPanel === "global";
    byId("globalButton").setAttribute("aria-expanded", String(globalOpen));
    byId("globalPanel").hidden = !globalOpen;

    const globals = state.probeEq.globalsByTarget[targetId];
    setInputValue(byId("globalInput"), globals.inputTrim.toFixed(1));
    setInputValue(byId("globalOutput"), globals.outputTrim.toFixed(1));
    setInputValue(byId("globalWidth"), globals.width.toFixed(2));
    setInputValue(byId("globalMonoBass"), Math.round(globals.monoBass));
    for (const input of byId("globalPanel").querySelectorAll("input[data-global-param]")) input.disabled = !connected;
  }

  function renderComparisonOptions(state) {
    const select = byId("comparisonSelect");
    const signature = state.sources.map((source) => `${source.id}:${source.name}`).join("|");
    if (select.dataset.signature !== signature) {
      select.dataset.signature = signature;
      select.replaceChildren(new Option("Keine zweite Spur", ""));
      for (const source of state.sources) select.add(new Option(source.name, source.id));
    }
    select.value = state.view.comparisonSourceId || "";
  }

  function renderEq(state) {
    renderSourceSwitcher(state);
    renderHandles(state);
    renderCurves(state);
    renderParameters(state);
    renderDraft(state);
    renderConnectionAndUtilities(state);
    renderComparisonOptions(state);
    byId("protectedRange").classList.toggle("is-violated", Boolean(state.protectionViolation));
  }

  function drawSpectrum(state, interpolation) {
    const canvas = byId("spectrumCanvas");
    const context = setupCanvas(canvas, GRAPH_WIDTH, GRAPH_HEIGHT);
    context.clearRect(0, 0, GRAPH_WIDTH, GRAPH_HEIGHT);

    if (!["measuring", "fresh", "stale"].includes(state.measurement.state)) return;
    const t = N.clamp(interpolation, 0, 1);
    const values = state.spectrum.values.map((value, index) => {
      const previous = state.spectrum.previousValues[index] ?? value;
      return previous + (value - previous) * t;
    });

    const gradient = context.createLinearGradient(0, 80, 0, GRAPH_HEIGHT);
    gradient.addColorStop(0, "rgba(113, 134, 146, 0.16)");
    gradient.addColorStop(1, "rgba(61, 72, 80, 0.01)");
    context.beginPath();
    values.forEach((value, index) => {
      const x = (index / (values.length - 1)) * GRAPH_WIDTH;
      const shape = value + 0.12 * Math.exp(-Math.pow((index - 14) / 7, 2)) + 0.08 * Math.exp(-Math.pow((index - 42) / 11, 2));
      const y = GRAPH_HEIGHT - 18 - N.clamp(shape, 0, 0.92) * 185;
      if (index === 0) context.moveTo(x, y);
      else context.lineTo(x, y);
    });
    context.lineTo(GRAPH_WIDTH, GRAPH_HEIGHT - 18);
    context.lineTo(0, GRAPH_HEIGHT - 18);
    context.closePath();
    context.fillStyle = gradient;
    context.fill();

    context.beginPath();
    values.forEach((value, index) => {
      const x = (index / (values.length - 1)) * GRAPH_WIDTH;
      const shape = value + 0.12 * Math.exp(-Math.pow((index - 14) / 7, 2)) + 0.08 * Math.exp(-Math.pow((index - 42) / 11, 2));
      const y = GRAPH_HEIGHT - 18 - N.clamp(shape, 0, 0.92) * 185;
      if (index === 0) context.moveTo(x, y);
      else context.lineTo(x, y);
    });
    context.strokeStyle = state.measurement.state === "stale" ? "rgba(120,126,133,0.28)" : "rgba(126,151,164,0.36)";
    context.lineWidth = 0.8;
    context.stroke();
  }

  function renderInspectorState(state) {
    byId("inspectorStatus").textContent = state.meta.notice || "Bereit.";
    byId("fixtureSelect").value = state.meta.fixture;
    byId("transportSelect").value = state.transport.recordState;
    byId("measurementSelect").value = state.measurement.state;
    if (state.connections.broker !== "connected") byId("connectionSelect").value = "broker-offline";
    else if (state.probeEq.targetId !== "master" && state.connections.probes[state.probeEq.targetId] !== "connected") byId("connectionSelect").value = "probe-offline";
    else byId("connectionSelect").value = "connected";
  }

  function render(state) {
    const stage = byId("pluginStage");
    const measuring = (
      state.measurement.state === "measuring" &&
      state.transport.recordState === "playing" &&
      state.connections.broker === "connected"
    );
    stage.dataset.measuring = String(measuring);
    stage.dataset.dragging = String(Boolean(state.view.drag));
    document.title = `${state.view.page === "overview" ? "Overview" : "EQ Center"} · Nakama Gen Simulator`;
    renderPage(state);
    renderOverview(state);
    renderEq(state);
    renderInspectorState(state);
    byId("stateProbe").textContent = JSON.stringify(state);
  }

  N.renderSimulator = render;
  N.drawSpectrum = drawSpectrum;
  N.curvePath = curvePath;
  N.responseAtFrequency = responseAtFrequency;
  N.overviewContentHeight = overviewContentHeight;
})(globalThis);
