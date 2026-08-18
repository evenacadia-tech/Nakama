#version 300 es
precision highp float;
precision highp int;

// NAKAMA · SCHLIEREN PROBE FIELD — Fragment-Shader fuer Unicorn Studio
// ====================================================================
// Der Befund wird NIE gezeichnet. Sichtbar ist ausschliesslich, wie er ein
// Prueffeld gerader Lichtlinien verbiegt (Schlieren-Verfahren).
//
// ABLESEGRAMMATIK — jede Groesse hat genau eine Bedeutung:
//   uOrt         Frequenzlage des Befundes         (Probe Center)
//   uStaerke     Ueber-/Fehlbetrag in dB           (Deflection)
//   uBreite      fVon..fBis                        (Zone Width)
//   uTiefe       Persistenz-Anteil                 (Vertical Extent)
//   uVorzeichen  >0.5 buendelt, <0.5 spreizt       (Convergence)
//   uLinien      Kammdichte = das Lineal           (Line Count, DESIGN)
//   uKoernung    Punktstreuung der Gravur          (Grain, DESIGN)
//   uLineColor   Bandfarbe Amber/Magenta/Eis       (Line Color)
//
// KALIBRIERT UND GEMESSEN (18.08., Export 1440x900, Line Count 24,
// Zone Width 30, Nennabstand ~79 px):
//   Deflection 100 → groesste Luecke 195 %, engster Abstand 77 %
//   Deflection  40 → groesste Luecke 134 %
//   Antwort strikt linear · Spiegelsymmetrie 0,0 px · Mittellinie steht
// Der Faktor 0.76 in `disp` IST diese Kalibrierung — nicht verstellen,
// sonst bedeutet dasselbe Bild an verschiedenen Tagen etwas anderes.

in vec2 vTextureCoord;
out vec4 fragColor;

uniform vec2  uResolution;
uniform float uTime;
uniform float uOrt;
uniform float uStaerke;
uniform float uBreite;
uniform float uTiefe;
uniform float uVorzeichen;
uniform float uLinien;
uniform float uKoernung;
uniform vec3  uLineColor;

// Canonical 2D PCG hash (unveraendert uebernommen)
uvec2 pcg2d(uvec2 v) {
  v = v * 1664525u + 1013904223u;
  v.x += v.y * v.y * 1664525u + 1013904223u;
  v.y += v.x * v.x * 1664525u + 1013904223u;
  v ^= v >> 16;
  v.x += v.y * v.y * 1664525u + 1013904223u;
  v.y += v.x * v.x * 1664525u + 1013904223u;
  return v;
}

float randFibo(vec2 p) {
  uvec2 v = floatBitsToUint(p);
  v = pcg2d(v);
  uint r = v.x ^ v.y;
  return float(r) / float(0xffffffffu);
}

void main() {
  vec2 uv = vTextureCoord;

  // Perspektive: SKALIERUNG um x = 0.5, niemals Verschiebung. Ein einseitiger
  // Scher wuerde dieselbe Abweichung links anders aussehen lassen als rechts.
  float persp = mix(1.03, 0.97, uv.y);
  vec2 pUv = vec2((uv.x - 0.5) * persp + 0.5, uv.y);

  // KERN: Die Ablenkung ist die ABLEITUNG der Abweichung, nicht die Abweichung
  // selbst. Deshalb ist die Verschiebung am Befundzentrum exakt null — die
  // Mittellinie steht, und man sieht nie die Glocke, nur was sie dem Licht antut.
  float w    = mix(0.03, 0.30, uBreite);
  float d    = (pUv.x - uOrt) / w;
  float bump = exp(-0.5 * d * d);
  float disp = -uStaerke * d * bump * w * 0.76;
  if (uVorzeichen < 0.5) {
    disp = -disp;                       // Fehlbetrag: das Feld spreizt
  }

  // Senkrechte Ausdehnung der Stoerung = Persistenz-Anteil, mittig.
  float h    = 0.5 * uTiefe;
  float prof = smoothstep(0.46 - h, 0.54 - h, pUv.y)
             * (1.0 - smoothstep(0.46 + h, 0.54 + h, pUv.y));
  float x = pUv.x + disp * prof;

  // Kamm — zugleich das Lineal: Versatz wird in Kamm-Einheiten abgelesen.
  float numLines = floor(mix(8.0, 48.0, uLinien) + 0.5);
  float spacing  = 1.0 / numLines;
  float lineIdx  = floor(x / spacing + 0.5);
  float distX    = abs(fract(x / spacing + 0.5) - 0.5) * spacing;

  // Punktkette (Innengravur). JEDE Linie bekommt eine EIGENE Phase — ohne sie
  // sitzen alle Punkte auf denselben Hoehen und es entstehen waagerechte
  // Baender (gemessen: Zeilen mit 17 von 18 Linien im Wechsel mit leeren).
  float numPoints = 150.0;
  float ptSpacing = 1.0 / numPoints;
  float phase     = randFibo(vec2(lineIdx * 3.71 + 5.3, 91.7));
  float yy        = uv.y + phase * ptSpacing;
  float ptIdx     = floor(yy / ptSpacing);
  float distY     = abs(yy - (ptIdx + 0.5) * ptSpacing);

  // Punktgroesse ABSOLUT in UV — nicht relativ zur Pixelgroesse. Die alte
  // Fassung teilte durch fwidth(); bei kleiner Darstellung wird fwidth gross,
  // der normierte Radius ueberall klein und der Effekt kippt in eine
  // VOLLFLAECHE (im Editor bei 90 % Zoom live beobachtet). Genau dieser Fall
  // ist der Ernstfall: im Plugin sitzt das Feld auf einer kleinen
  // Prismenflaeche. fwidth dient jetzt nur noch der Kantenglaettung, damit
  // die Punkte bei wenig Aufloesung DUENNER werden statt breiter.
  float halbBreite = 0.00075;          // halbe Linienbreite in x-UV
  float punktHalb  = 0.0021;           // halbe Punkthoehe in y-UV
  float r     = length(vec2(distX / halbBreite, distY / punktHalb));
  float aa    = max(fwidth(r), 1e-3);
  float punkt = 1.0 - smoothstep(1.0 - aa, 1.0 + aa, r);

  // Koernung: Helligkeit je Punkt aus Hash(Linie, Punkt), plus harte Aussetzer.
  float rnd = randFibo(vec2(lineIdx * 17.13 + 31.7, ptIdx * 43.19 + 71.3));
  float koern = clamp(uKoernung, 0.0, 1.0);
  float pointBrightness = mix(1.0, pow(rnd, 1.8), koern);
  if (rnd < koern * 0.45) {
    pointBrightness = 0.0;
  }

  // Tiefe wirkt entlang der PERSPEKTIVE (uv.y), nicht seitlich. Ein seitlicher
  // Verlauf wuerde denselben Befund links heller zeigen als rechts — das Bild
  // wuerde luegen, ohne dass man es merkt.
  float depthAlpha = mix(1.0, 0.75, uv.y);

  float finalAlpha = clamp(punkt * pointBrightness * depthAlpha, 0.0, 1.0);

  // RGB ist IMMER exakt die Linienfarbe. Nur Alpha traegt Tiefe und Koernung —
  // Helligkeit darf niemals Staerke bedeuten (das waere laut→hell).
  fragColor = vec4(uLineColor, finalAlpha);
}
