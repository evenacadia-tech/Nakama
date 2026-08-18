// Render-Werkzeug fürs Tusche-Musterblatt (reine 2D-Zeichnung, kein WebGL).
//
//   node render-blatt.mjs                       → Standardsatz nach vorentwurf-renders/
//   node render-blatt.mjs pruefe                → Semantik + Stop/Start prüfen
//   node render-blatt.mjs schuss <name> <query> → Einzelbild, z.B.
//        node render-blatt.mjs schuss probe "?welt=nebel-nacht&zeit=20" 1200 675
//
// Playwright kommt aus vendor-build/ (npm ci dort). Einzellauf, kein
// GPU-Batch (Arc-A770-Landmine); die Seite braucht nur den 2D-Rasterizer.
import { createRequire } from "node:module";
import { fileURLToPath, pathToFileURL } from "node:url";
import { dirname, join } from "node:path";
import { mkdirSync } from "node:fs";

const require = createRequire(new URL("./vendor-build/", import.meta.url));
const { chromium } = require("playwright");

const hier = dirname(fileURLToPath(import.meta.url));
const blattUrl = pathToFileURL(join(hier, "nakama-spectral-field-claude.html")).href;
const renderDir = join(hier, "vorentwurf-renders");
mkdirSync(renderDir, { recursive: true });

// ?zeit=14 friert die Simulation deterministisch ein (Stand-Bild).
// Chips = BEFUND-DATEN (aus=resonanz,mulm,loch,haerte,maskierung|alle);
// ?kali=schluessel:wert setzt Regler. Nur dunkle Welten (17.08.).
// n4-Serie: semantische Zustände plus die drei vertraglichen Referenzgrößen.
const SATZ = [
  { name: "n4-01-frueh-nebel", query: "?welt=nebel-nacht&zeit=4", width: 1200, height: 675 },
  { name: "n4-02-belastbar-licht", query: "?welt=nebel-nacht&zeit=30&nur=reso-low", width: 1200, height: 675 },
  { name: "n4-03-breite-fehlstelle", query: "?welt=nebel-nacht&zeit=24&nur=mid-hole", width: 1200, height: 675 },
  { name: "n4-04-transient", query: "?welt=nebel-nacht&zeit=13.2&nur=harsh", width: 1200, height: 675 },
  { name: "n4-05-ueberlagerung", query: "?welt=nebel-nacht&zeit=20", width: 1200, height: 675 },
  { name: "n4-06-kein-befund", query: "?welt=nebel-nacht&zeit=20&aus=alle", width: 1200, height: 675 },
  { name: "n4-07-kompakt", query: "?welt=nebel-nacht&zeit=20", width: 720, height: 450 },
  { name: "n4-08-mittel", query: "?welt=nebel-nacht&zeit=20", width: 1200, height: 675 },
  { name: "n4-09-gross", query: "?welt=nebel-nacht&zeit=20", width: 1920, height: 1080 },
  { name: "n4-10-tusche", query: "?welt=tusche-dunkel&zeit=20", width: 1200, height: 675 }
];

async function screenshotSchreiben(seite, path) {
  // Windows hält eine gerade betrachtete PNG gelegentlich noch einen Moment
  // offen. Der Render ist deterministisch; nur der Dateischreibweg braucht
  // dann einen kurzen Gegenversuch.
  for (let versuch = 0; versuch < 3; versuch++) {
    try {
      await seite.screenshot({ path });
      return;
    } catch (fehler) {
      if (versuch === 2) throw fehler;
      await seite.waitForTimeout(250 * (versuch + 1));
    }
  }
}

async function schuss(seite, fehler, { name, query, width, height }) {
  await seite.setViewportSize({ width, height });
  const fehlerVorher = fehler.length;
  await seite.goto(blattUrl + query, { waitUntil: "load" });
  await seite.waitForTimeout(900);   // Fonts + erster Anstrich
  await screenshotSchreiben(seite, join(renderDir, name + ".png"));
  const neueFehler = fehler.slice(fehlerVorher);
  if (neueFehler.length) {
    console.error("SEITENFEHLER bei " + name + ":\n  " + neueFehler.join("\n  "));
    process.exit(1);
  }
  console.log(`  ${name}.png (${width}×${height})`);
}

function segmentzahl(path) {
  return path === "M0,0" ? 0 : (path.match(/M/g) || []).length;
}

async function feldProbe(seite, query, width = 1200, height = 675) {
  await seite.setViewportSize({ width, height });
  await seite.goto(blattUrl + query, { waitUntil: "load" });
  const roh = await seite.evaluate(() => {
    const d = id => document.getElementById(id).getAttribute("d") || "";
    return {
      nebel: d("nebelFern"), licht: d("tiefenLicht"), basis: d("diagnosticBasis"),
      leicht: d("feldLeicht"), mittel: d("feldMittel"), dicht: d("feldDicht"),
      marker: [...document.querySelectorAll(".issue-marker")]
        .filter(node => getComputedStyle(node).display !== "none").length
    };
  });
  for (const key of ["nebel", "licht", "basis", "leicht", "mittel", "dicht"])
    roh[key] = segmentzahl(roh[key]);
  return roh;
}

async function semantikPruefen(seite, fehler) {
  const frueh = await feldProbe(seite, "?welt=nebel-nacht&zeit=4&nur=reso-low");
  const spaet = await feldProbe(seite, "?welt=nebel-nacht&zeit=30&nur=reso-low");
  const leer = await feldProbe(seite, "?welt=nebel-nacht&zeit=20&aus=alle");
  const kompakt = await feldProbe(seite, "?welt=nebel-nacht&zeit=20", 720, 450);
  const gross = await feldProbe(seite, "?welt=nebel-nacht&zeit=20", 1920, 1080);

  await seite.setViewportSize({ width: 1200, height: 675 });
  await seite.goto(blattUrl, { waitUntil: "load" });
  await seite.waitForTimeout(180);
  await seite.locator("#playButton").click();
  const zustand = () => seite.evaluate(() => ({
    kurve: document.getElementById("inkStroke").getAttribute("d"),
    zeit: document.getElementById("reportTime").textContent,
    laeuft: document.body.dataset.running
  }));
  const stopA = await zustand();
  await seite.waitForTimeout(400);
  const stopB = await zustand();
  await seite.locator("#playButton").click();
  await seite.waitForTimeout(400);
  const neu = await zustand();

  const pruefungen = {
    "früh: Nebel > Licht": frueh.nebel > frueh.licht,
    "belastbar: Licht > Nebel": spaet.licht > spaet.nebel,
    "Leerzustand ohne Diagnosezeichen": leer.nebel === 0 && leer.licht === 0
      && leer.basis === 0 && leer.marker === 0,
    "dB-Klassen bei Resize stabil": ["leicht", "mittel", "dicht"]
      .every(key => kompakt[key] === gross[key]),
    "Stop friert Kurve und Zeit": stopA.kurve === stopB.kurve
      && stopA.zeit === stopB.zeit && stopB.laeuft === "false",
    "Neustart bewegt wieder": neu.kurve !== stopB.kurve && neu.laeuft === "true"
  };
  for (const [name, ok] of Object.entries(pruefungen))
    console.log(`  ${ok ? "OK" : "FEHLER"} · ${name}`);
  if (fehler.length) throw new Error("Seitenfehler:\n" + fehler.join("\n"));
  const gescheitert = Object.entries(pruefungen).filter(([, ok]) => !ok).map(([name]) => name);
  if (gescheitert.length) throw new Error("Semantikprüfung fehlgeschlagen: " + gescheitert.join(", "));
}

const [, , modus, name, query, breite, hoehe] = process.argv;
const browser = await chromium.launch();
const seite = await browser.newPage({ viewport: { width: 1200, height: 675 } });
const fehler = [];
seite.on("pageerror", e => fehler.push(String(e)));

if (modus === "pruefe") {
  await semantikPruefen(seite, fehler);
} else if (modus === "schuss") {
  await schuss(seite, fehler, {
    name,
    query: query || "",
    width: Number(breite) || 1200,
    height: Number(hoehe) || 675
  });
} else {
  for (const szenario of SATZ) await schuss(seite, fehler, szenario);
}
await browser.close();
