// Render-Werkzeug für die Kunstwerk-Studie (reine 2D-Zeichnung, kein WebGL).
//
//   node render-kunstwerk.mjs                     → Standardsatz nach vorentwurf-renders/
//   node render-kunstwerk.mjs schuss <name> <query> [breite] [hoehe]
//        z. B. node render-kunstwerk.mjs schuss probe "?welt=wesen&zeit=20" 1200 675
//
// Playwright kommt aus vendor-build/ (npm ci dort). Einzellauf, kein
// GPU-Batch (Arc-A770-Landmine); die Seite braucht nur den 2D-Rasterizer.
// Jeder Lauf schlägt fehl, wenn die Seite JS-Fehler wirft — das ist der
// „Blatt läuft"-Beweis.
import { createRequire } from "node:module";
import { fileURLToPath, pathToFileURL } from "node:url";
import { dirname, join } from "node:path";
import { mkdirSync } from "node:fs";

const require = createRequire(new URL("./vendor-build/", import.meta.url));
const { chromium } = require("playwright");

const hier = dirname(fileURLToPath(import.meta.url));
const blattUrl = pathToFileURL(join(hier, "nakama-kunstwerk-claude.html")).href;
const renderDir = join(hier, "vorentwurf-renders");
mkdirSync(renderDir, { recursive: true });

// k1-Serie: jede Welt in drei Song-Momenten (Intro ruhig · Chorus voll ·
// Kick-Schlag) plus Leerzustand — deterrministisch über ?zeit=.
const SATZ = [
  { name: "k1-01-garten-intro", query: "?welt=garten&zeit=4", width: 1200, height: 675 },
  { name: "k1-02-garten-chorus", query: "?welt=garten&zeit=20", width: 1200, height: 675 },
  { name: "k1-03-gewebe-chorus", query: "?welt=gewebe&zeit=20", width: 1200, height: 675 },
  { name: "k1-04-gewebe-frueh", query: "?welt=gewebe&zeit=8", width: 1200, height: 675 },
  { name: "k1-05-wesen-chorus", query: "?welt=wesen&zeit=20", width: 1200, height: 675 },
  { name: "k1-06-wesen-intro", query: "?welt=wesen&zeit=3.1", width: 1200, height: 675 },
  { name: "k1-07-garten-leer", query: "?welt=garten&zeit=20&aus=alle", width: 1200, height: 675 },
  { name: "k1-08-gross", query: "?welt=garten&zeit=20", width: 1920, height: 1080 }
];

async function schuss(seite, fehler, { name, query, width, height }) {
  await seite.setViewportSize({ width, height });
  const vorher = fehler.length;
  await seite.goto(blattUrl + query, { waitUntil: "load" });
  await seite.waitForTimeout(900);   // Fonts + erster Anstrich
  await seite.screenshot({ path: join(renderDir, name + ".png") });
  if (fehler.length > vorher)
    throw new Error(`Seitenfehler bei ${name}: ${fehler.slice(vorher).join(" | ")}`);
  console.log(`  ${name}.png (${width}×${height})`);
}

const browser = await chromium.launch();
const seite = await browser.newPage();
const fehler = [];
seite.on("pageerror", e => fehler.push(String(e)));

const modus = process.argv[2];
try {
  if (modus === "schuss") {
    const [, , , name, query, w, h] = process.argv;
    await schuss(seite, fehler, {
      name: name || "probe", query: query || "?zeit=20",
      width: parseInt(w || "1200", 10), height: parseInt(h || "675", 10)
    });
  } else {
    for (const bild of SATZ) await schuss(seite, fehler, bild);
  }
} finally {
  await browser.close();
}
