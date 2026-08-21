// ARCHIV — ALTE DESIGN-RICHTUNG. Symbol-Sync ins Codex-Blatt (Feld-Alphabet): alt. User 21.08.2026: „alle alten sind alt“. Keine Vorgabe, nur Verlauf. Aktive Design-Quelle: Figma (User) → Projekte/Nakama-Design. Einzelheiten: LIES-MICH.md in diesem Ordner.
import { readFile, writeFile } from "node:fs/promises";
import { dirname, join } from "node:path";
import { fileURLToPath } from "node:url";

const here = dirname(fileURLToPath(import.meta.url));
const assetPath = join(here, "assets", "nakama-symbols.svg");
const previewPath = join(here, "nakama-spectral-field-vorentwurf.html");
const atlasPath = join(here, "concepts", "nakama-field-alphabet-v2.html");

const startMarker = "      <!-- NAKAMA_SYMBOLS_START · generated from assets/nakama-symbols.svg -->";
const endMarker = "      <!-- NAKAMA_SYMBOLS_END -->";

const asset = await readFile(assetPath, "utf8");
const firstSymbol = asset.indexOf("  <symbol ");
const svgEnd = asset.lastIndexOf("</svg>");
if (firstSymbol < 0 || svgEnd < 0 || firstSymbol >= svgEnd) {
  throw new Error("assets/nakama-symbols.svg enthält keinen gültigen Symbolblock");
}

const symbols = asset.slice(firstSymbol, svgEnd).trimEnd()
  .split(/\r?\n/)
  .map((line) => line.trimEnd())
  .map((line) => line.length ? `    ${line}` : "")
  .join("\n");
const generated = `${startMarker}\n${symbols}\n${endMarker}\n\n`;

async function syncFile(path, allowLegacy = false) {
  let document = await readFile(path, "utf8");
  const markedStart = document.indexOf(startMarker);
  const markedEnd = document.indexOf(endMarker);

  if (markedStart >= 0 && markedEnd > markedStart) {
    document = document.slice(0, markedStart)
      + generated
      + document.slice(markedEnd + endMarker.length).replace(/^\r?\n\r?\n?/, "");
  } else if (allowLegacy) {
    const legacyStart = document.indexOf("      <!-- Inline mirror of assets/nakama-symbols.svg for direct file previews. -->");
    const nextDefinition = document.indexOf("      <linearGradient id=\"materialFill\"", legacyStart);
    if (legacyStart < 0 || nextDefinition < 0) {
      throw new Error("Inline-Symbolblock im Vorentwurf nicht gefunden");
    }
    document = document.slice(0, legacyStart) + generated + document.slice(nextDefinition);
  } else {
    throw new Error(`Symbolmarker fehlen in ${path}`);
  }

  await writeFile(path, document, "utf8");
}

await syncFile(previewPath, true);
await syncFile(atlasPath);
console.log("Nakama-Symbolalphabet in Vorentwurf und Atlas gespiegelt.");
