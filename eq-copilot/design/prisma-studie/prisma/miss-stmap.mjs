// PRISMA-STUDIE — User-Idee, geparkt (User 21.08.2026: „Familie; Prisma nur Studie“ · „Alles nur Studie“ · Herkunft: „Meine Idee“). Kein Produktteil, kein Bauplan, kein Vokabular für die Plugin-UI. Statusblatt: ../STATUS.md. 
// Mess-Runner fuer die ST-Map-Probe (NAK-16).
//
//   node miss-stmap.mjs [--unicorn]
//
// Liefert den Erfolgsmassstab der Kreativ-Freigabe 19.08.: zwei
// Screenshots mit verschiedenem Hintergrund-Stand muessen sich IM Glas
// unterscheiden ("Aehnlichkeit zum alten Frame" ist als Metrik verboten).
// Dazu die Mess-Seite (?messung=1): In-Glas-Differenz aus readPixels,
// Konsistenz ausserhalb, RGBA16F-Upload-Pfad, Upload-Benchmark.
// Playwright aus ../../vendor-build (Studien-Umzug 21.08.2026), Software-GL (SwiftShader), Einzellauf —
// Arc-A770-Landmine: keine GPU-Batch-Loops.
import { createRequire } from "node:module";
import { fileURLToPath, pathToFileURL } from "node:url";
import { dirname, join } from "node:path";
import { writeFileSync } from "node:fs";

const require = createRequire(new URL("../../vendor-build/", import.meta.url));
const { chromium } = require("playwright");

const hier = dirname(fileURLToPath(import.meta.url));
const blattUrl = pathToFileURL(join(hier, "stmap-probe.html")).href;
const ausDir = join(hier, "renders", "stmap");
const MIT_UNICORN = process.argv.includes("--unicorn");

const browser = await chromium.launch({
  args: ["--use-angle=swiftshader", "--enable-unsafe-swiftshader"]
});
const seite = await browser.newPage();
const fehler = [];
seite.on("pageerror", e => fehler.push(String(e)));

async function lade(query, warteAufMessung) {
  await seite.goto(blattUrl + query, { waitUntil: "load" });
  await seite.waitForFunction(
    warteAufMessung ? "window.MESSUNG !== undefined" : "window.BEREIT === true",
    null, { timeout: 30000 });
  await seite.waitForTimeout(400);          // letzter Frame steht
}

try {
  await seite.setViewportSize({ width: 1980, height: 1360 });

  // Erfolgsmassstab-Paar: zwei Hintergrund-Staende, echtes Blatt.
  await lade("?bgzeit=3", false);
  await seite.locator("#comp").screenshot({ path: join(ausDir, "probe-live-a.png") });
  await lade("?bgzeit=9", false);
  await seite.locator("#comp").screenshot({ path: join(ausDir, "probe-live-b.png") });

  // Messung (readPixels-Differenz im Glas + Checkliste).
  await lade("?messung=1", true);
  const messung = await seite.evaluate("window.MESSUNG");
  await seite.screenshot({ path: join(ausDir, "probe-messung.png") });
  writeFileSync(join(ausDir, "messung.json"), JSON.stringify(messung, null, 2));
  console.log(JSON.stringify(messung, null, 2));

  // Lebende Unicorn-Ebene 0 (braucht Internet/CDN) — nur auf Wunsch.
  if (MIT_UNICORN) {
    await seite.goto(blattUrl + "?bg=unicorn", { waitUntil: "load" });
    await seite.waitForFunction("window.BEREIT === true", null, { timeout: 30000 });
    await seite.waitForTimeout(6000);       // Szene laedt + schwingt ein
    await seite.locator("#comp").screenshot({ path: join(ausDir, "probe-live-unicorn.png") });
    console.log("unicorn: " + await seite.evaluate(
      "window.__uc = !!document.querySelector('#ebene0 canvas'), window.__uc ? 'EBENE 0 LIVE' : 'CDN/Szene nicht geladen — Testmuster'"));
  }

  if (fehler.length) throw new Error("Seitenfehler: " + fehler.join(" | "));
  console.log("Screenshots: probe-live-a/b.png, probe-messung.png"
    + (MIT_UNICORN ? ", probe-live-unicorn.png" : ""));
} finally {
  await browser.close();
}
