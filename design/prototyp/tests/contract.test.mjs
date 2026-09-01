import assert from "node:assert/strict";
import crypto from "node:crypto";
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

const here = path.dirname(fileURLToPath(import.meta.url));
const repo = path.resolve(here, "../../..");
const contractRoot = path.resolve(here, "../contract");
const uiContract = JSON.parse(fs.readFileSync(path.join(contractRoot, "ui-contract.json"), "utf8"));
const motion = JSON.parse(fs.readFileSync(path.join(contractRoot, "motion-tokens.json"), "utf8"));
const assets = JSON.parse(fs.readFileSync(path.join(contractRoot, "asset-manifest.json"), "utf8"));

assert.equal(uiContract.schemaVersion, 2);
assert.equal(uiContract.contractId, "nakama-gen-ui-v2");
assert.equal(assets.schemaVersion, 1);
assert.equal(motion.schemaVersion, 1);
assert.deepEqual(assets.logicalStage, { width: 760, height: 430, unit: "css-px" });
assert.equal(assets.sourceStatus, "historical-snapshot");
assert.equal(assets.currentDesignSource.fileKey, "NPCQYSkoZEd4Av0NlKxBOd");
assert.equal(assets.currentDesignSource.nodes.overviewArtboard, "25:444");
assert.equal(assets.currentDesignSource.nodes.eqCenterWrapper, "28:993");
assert.equal(assets.currentDesignSource.nodes.eqCenterArtboard, "28:994");
assert.equal(assets.currentDesignSource.nodes.probeeqFallbackArtboard, "6:2864");
assert.deepEqual(assets.currentDesignSource.artboards.overview, { width: 760, height: 430 });
assert.deepEqual(assets.currentDesignSource.artboards.eqCenter, { width: 760, height: 430 });
assert.deepEqual(assets.currentDesignSource.artboards.probeeqFallback, { width: 260, height: 84 });
assert.deepEqual(assets.currentDesignSource.inventory, {
  nodeCount: 554,
  reactionCount: 0,
  componentLikeCount: 0,
});
assert.equal(assets.figma.nodes.overview, "4090:685");
assert.equal(assets.figma.nodes.eqCenter, "6002:2");
assert.equal(assets.figma.motion.overviewAnimatedNodes, 0);
assert.equal(assets.figma.motion.eqCenterAnimatedNodes, 0);
assert.equal(motion.directManipulationMs, 0);
assert.equal(motion.stateChangeMs, 140);
assert.equal(motion.pageChangeMs, 180);
assert.equal(motion.spectrumDataHz, 20);
assert.equal(motion.easing, "cubic-bezier(0.22, 1, 0.36, 1)");
assert.ok(uiContract.invariants.some((rule) => rule.includes("ten-second temporary lease")));
assert.ok(uiContract.invariants.some((rule) => rule.includes("Mix remains planned")));
assert.deepEqual(uiContract.surfaceSizing.gen.surfaces, ["overview", "eq-center"]);
assert.deepEqual(uiContract.surfaceSizing.gen.compactMinimum, {
  width: 760,
  height: 430,
  unit: "logical-px",
});
assert.deepEqual(uiContract.surfaceSizing.gen.default, {
  width: 950,
  height: 538,
  unit: "logical-px",
});
assert.deepEqual(uiContract.surfaceSizing.gen.fixedAspectRatio, { width: 76, height: 43 });
assert.equal(uiContract.surfaceSizing.gen.resizeBehavior, "fixed-aspect-graph-evidence-first");
assert.equal(uiContract.surfaceSizing.gen.uiScaleIndependent, true);
assert.equal(uiContract.surfaceSizing.gen.functionParityRequired, true);
assert.equal(uiContract.surfaceSizing.gen.nativeHostVerification, "required");
assert.ok(uiContract.invariants.some((rule) => rule.includes("compact minimum")));

for (const [surfaceName, surface] of Object.entries(assets.surfaces)) {
  assert.equal(surface.figmaStructureControl.isPixelGolden, false);
  assert.equal(surface.pixelGolden4x.width, 3040);
  assert.equal(surface.pixelGolden4x.height, 1720);
  for (const tier of Object.values(surface.qualityTiers)) {
    assert.equal(tier.derivedDirectlyFrom, "pixelGolden4x");
  }
  for (const record of [surface.userOriginal, surface.pixelGolden4x, surface.figmaStructureControl]) {
    const bytes = fs.readFileSync(path.join(repo, record.path));
    const hash = crypto.createHash("sha256").update(bytes).digest("hex");
    assert.equal(hash, record.sha256, `${surfaceName}: ${record.path}`);
  }
}

const actionNames = new Set(uiContract.types.UiAction.names);
for (const required of [
  "PREVIEW_BEGIN",
  "PREVIEW_END",
  "SEND_DRAFT",
  "SET_GLOBAL_PARAMETER",
  "TEMPORARY_APPLY",
  "CONFIRM_APPLY",
  "REJECT_DRAFT",
  "UNDO",
  "EXTERNAL_AUTOMATION",
]) {
  assert.ok(actionNames.has(required), `Missing action ${required}`);
}

console.log("Contract tests passed: manifests, motion tokens, action vocabulary and golden hashes agree.");
