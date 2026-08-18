// Erzeugt plugin/src/LeitstandTokens.h aus tokens.json — die eine maschinen-
// lesbare Token-Quelle des EQ-Copilot (Plan §7.6 / M0). Aufruf:
//   node eq-copilot/design/gen-tokens.mjs
// Der Header wird eingecheckt; dieser Generator ist der einzige legale Weg,
// ihn zu ändern.
import { readFileSync, writeFileSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, join } from "node:path";

const hier = dirname(fileURLToPath(import.meta.url));
const tokens = JSON.parse(readFileSync(join(hier, "tokens.json"), "utf8"));

const zuArgb = (hex) => {
  const h = hex.replace("#", "");
  if (!/^[0-9a-fA-F]{6}$/.test(h)) throw new Error(`kein 6-stelliges Hex: ${hex}`);
  return `0xff${h.toLowerCase()}`;
};

let konstanten = "";
for (const [gruppe, werte] of Object.entries(tokens)) {
  if (typeof werte !== "object") continue;
  for (const [name, wert] of Object.entries(werte)) {
    if (typeof wert !== "string" || !wert.startsWith("#")) continue;
    const cname = `${gruppe}_${name}`.replace(/[^a-zA-Z0-9_]/g, "_");
    konstanten += `inline constexpr juce::uint32 ${cname} = ${zuArgb(wert)};\n`;
  }
}

const header = `// GENERIERT aus eq-copilot/design/tokens.json via gen-tokens.mjs — NICHT von
// Hand editieren. Quelle der Farbwahrheit: tokens.json; die hellen Leitstand-
// Gruppen spiegeln plugin-hub-app/src/lib/leitstand.css, copilot ist eigenständig.
#pragma once
#include <juce_graphics/juce_graphics.h>

namespace leitstand
{
${konstanten}
inline juce::Colour farbe (juce::uint32 argb) { return juce::Colour (argb); }
} // namespace leitstand
`;

writeFileSync(join(hier, "..", "plugin", "src", "LeitstandTokens.h"), header);
console.log("LeitstandTokens.h geschrieben.");
