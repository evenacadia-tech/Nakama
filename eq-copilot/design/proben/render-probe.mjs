// Render-Werkzeug für die Stil-Proben-Serie (Geschmacks-Kalibrierung).
//
//   node render-probe.mjs <probe.html> <name> [query] [breite] [hoehe]
//   z. B.: node render-probe.mjs probe-01-perkolation.html p01-chorus "?zeit=20"
//
// Playwright aus ../vendor-build. Einzellauf, Software-GL (SwiftShader) —
// Arc-A770-Landmine: keine GPU-Batch-Loops. Seitenfehler = Abbruch.
import { createRequire } from "node:module";
import { fileURLToPath, pathToFileURL } from "node:url";
import { dirname, join } from "node:path";
import { mkdirSync } from "node:fs";

const require = createRequire(new URL("../vendor-build/", import.meta.url));
const { chromium } = require("playwright");

const hier = dirname(fileURLToPath(import.meta.url));
const renderDir = join(hier, "renders");
mkdirSync(renderDir, { recursive: true });

const [, , blatt, name, query = "?zeit=20", w = "1200", h = "675", warte = "1500"] = process.argv;
if (!blatt || !name) {
  console.error("Aufruf: node render-probe.mjs <probe.html> <name> [query] [breite] [hoehe]");
  process.exit(1);
}

const browser = await chromium.launch({
  args: ["--use-angle=swiftshader", "--enable-unsafe-swiftshader"]
});
const seite = await browser.newPage();
const fehler = [];
seite.on("pageerror", e => fehler.push(String(e)));

try {
  await seite.setViewportSize({ width: parseInt(w, 10), height: parseInt(h, 10) });
  await seite.goto(pathToFileURL(join(hier, blatt)).href + query, { waitUntil: "load" });
  await seite.waitForTimeout(parseInt(warte, 10));   // Fonts + Einschwingen + erster Frame
  await seite.screenshot({ path: join(renderDir, name + ".png") });
  if (fehler.length) throw new Error("Seitenfehler: " + fehler.join(" | "));
  console.log(`  ${name}.png (${w}×${h})`);
} finally {
  await browser.close();
}
