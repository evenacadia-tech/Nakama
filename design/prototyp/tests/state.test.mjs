import assert from "node:assert/strict";
import fs from "node:fs";
import path from "node:path";
import vm from "node:vm";
import { fileURLToPath } from "node:url";

const here = path.dirname(fileURLToPath(import.meta.url));
const prototypeRoot = path.resolve(here, "..");

for (const script of ["src/contract.js", "src/demo-adapter.js"]) {
  const source = fs.readFileSync(path.join(prototypeRoot, script), "utf8");
  vm.runInThisContext(source, { filename: script });
}

const N = globalThis.Nakama;

function clock(start = 1000) {
  return {
    value: start,
    now() {
      return this.value;
    },
    advance(milliseconds) {
      this.value += milliseconds;
    },
  };
}

function adapter(fixture = "overview-golden") {
  const time = clock();
  return { time, ui: new N.DemoAdapter({ fixture, clock: time }) };
}

function band(snapshot, sourceId = snapshot.probeEq.targetId, bandId = 3) {
  return snapshot.eqBands[sourceId].find((candidate) => candidate.id === bandId);
}

{
  const { ui } = adapter();
  const state = ui.getSnapshot();
  assert.equal(state.meta.contractVersion, 1);
  assert.equal(state.sources.length, 5);
  assert.equal(state.view.page, "overview");
  assert.equal(state.draft.state, "ready");
  assert.equal(state.revisions[state.draft.sourceId].length, 0);
}

{
  const { ui } = adapter();
  ui.dispatch({ type: N.ACTION.PREVIEW_BEGIN });
  let state = ui.getSnapshot();
  assert.equal(state.draft.state, "auditioning");
  assert.equal(state.view.heldControl, "audition");
  assert.equal(state.revisions[state.draft.sourceId].length, 0, "preview must not create a revision");

  ui.dispatch({ type: N.ACTION.PREVIEW_END });
  state = ui.getSnapshot();
  assert.equal(state.draft.state, "ready");
  assert.equal(state.view.heldControl, null);
}

{
  const { ui } = adapter("recording-gate");
  ui.dispatch({ type: N.ACTION.PREVIEW_BEGIN });
  const state = ui.getSnapshot();
  assert.equal(state.draft.state, "ready");
  assert.match(state.meta.notice, /RECORDING/);
}

{
  const { ui } = adapter();
  const before = band(ui.getSnapshot()).gain;
  ui.dispatch({ type: N.ACTION.SEND_DRAFT });
  const after = ui.getSnapshot();
  assert.equal(band(after).gain, before, "sending a draft must not change confirmed EQ");
  assert.equal(after.revisions[after.draft.sourceId].length, 0);
  assert.equal(after.draft.state, "ready");
}

{
  const { ui, time } = adapter();
  const before = band(ui.getSnapshot()).gain;
  ui.dispatch({ type: N.ACTION.TEMPORARY_APPLY });
  assert.equal(ui.getSnapshot().draft.state, "lease");
  time.advance(10001);
  ui.dispatch({ type: N.ACTION.TICK });
  const expired = ui.getSnapshot();
  assert.equal(expired.draft.state, "none");
  assert.equal(band(expired).gain, before, "lease expiry must restore confirmed EQ");
  assert.equal(expired.revisions[expired.probeEq.targetId].length, 0);
}

{
  const { ui, time } = adapter();
  const sourceId = ui.getSnapshot().draft.sourceId;
  const before = { ...band(ui.getSnapshot(), sourceId) };
  ui.dispatch({ type: N.ACTION.TEMPORARY_APPLY });
  time.advance(2500);
  ui.dispatch({ type: N.ACTION.CONFIRM_APPLY });
  let confirmed = ui.getSnapshot();
  assert.equal(confirmed.draft.state, "none");
  assert.equal(band(confirmed, sourceId).gain, -3);
  assert.equal(confirmed.revisions[sourceId].length, 1);

  ui.dispatch({ type: N.ACTION.UNDO });
  confirmed = ui.getSnapshot();
  assert.equal(band(confirmed, sourceId).gain, before.gain);
  assert.equal(confirmed.revisions[sourceId].length, 0);
}

{
  const { ui } = adapter();
  const sourceId = ui.getSnapshot().draft.sourceId;
  ui.dispatch({ type: N.ACTION.PREVIEW_BEGIN });
  ui.dispatch({ type: N.ACTION.SET_CONNECTION, segment: "probe", sourceId, state: "probe-offline" });
  const disconnected = ui.getSnapshot();
  assert.equal(disconnected.draft.state, "none");
  assert.equal(disconnected.revisions[sourceId].length, 0);
  assert.match(disconnected.meta.notice, /PREVIEW REVERTED/);
}

{
  const { ui } = adapter("eq-golden");
  ui.dispatch({
    type: N.ACTION.SET_BAND_PARAMETER,
    bandId: 5,
    parameter: "frequency",
    value: 4200,
  });
  let state = ui.getSnapshot();
  assert.equal(band(state, state.probeEq.targetId, 5).frequency, 3000);
  assert.equal(state.protectionViolation.origin, "nakama-control");

  ui.dispatch({
    type: N.ACTION.EXTERNAL_AUTOMATION,
    bandId: 5,
    parameter: "frequency",
    value: 4200,
  });
  state = ui.getSnapshot();
  assert.equal(band(state, state.probeEq.targetId, 5).frequency, 4200);
  assert.equal(state.protectionViolation.origin, "external-automation");
  assert.equal(state.draft.state, "stale");
}

{
  const { ui, time } = adapter("eq-golden");
  const firstSource = ui.getSnapshot().view.selectedSourceId;
  ui.dispatch({ type: N.ACTION.TEMPORARY_APPLY });
  time.advance(1000);
  ui.dispatch({ type: N.ACTION.CONFIRM_APPLY });
  ui.dispatch({ type: N.ACTION.SELECT_SOURCE, sourceId: "source-1" });
  ui.dispatch({ type: N.ACTION.UNDO });
  const state = ui.getSnapshot();
  assert.equal(state.revisions[firstSource].length, 1, "undo on another source must not consume this source's history");
  assert.match(state.meta.notice, /UNDO UNAVAILABLE/);
}

{
  const { ui } = adapter("sixteen-sources");
  const state = ui.getSnapshot();
  assert.equal(state.sources.length, 16);
  assert.equal(state.sources.at(-1).name, "MUSIC BUS");
  assert.ok(!state.sources.some((source) => source.id === "master"), "Master must stay outside source order");
}

{
  const { ui } = adapter("eq-golden");
  ui.dispatch({ type: N.ACTION.SELECT_SOURCE, sourceId: "master" });
  const state = ui.getSnapshot();
  assert.equal(state.probeEq.targetId, "master");
  assert.ok(state.eqBands.master);
}

console.log("State tests passed: 11 lifecycle, protection, automation and source-order cases.");

