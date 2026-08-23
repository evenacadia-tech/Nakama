# Spielregeln-Umbau der Gen-Seiten — 23.08.2026 (Vorschlag, nicht abgenommen)

**Auftrag des Users (23.08.):** die zwei von ihm markierten Designs im Figma
`Probeeq-Nakama-Prototyp-Design` (Key `5TazApiPrImtLJZ2a71acV`) anpassen bzw.
umbauen, so dass sie die Spielregeln der EQ-Zentrale erfüllen
(`design/docs/ui-spielregeln-eq-zentrale.md`); die vorhandenen Designs gelten
visuell als Referenz; nur Gen Seite 1 + Seite 2, Zustände gesondert mit kurzer
Erklärung, Qualität vor Quantität.

**Vorgehen:** Beide markierten Frames wurden **geklont** und die Klone
umgebaut — die Originale (`4028:209` Main App / Product Screen 01,
`4028:4` Active Probe Editor 700×420) stehen unangetastet daneben und bleiben
die visuelle Referenz. Farb-, Schrift- und Materialwelt (Geist / Geist Mono,
Slate-Akzent, Signal-Rot, Glas/Chin) sind unverändert übernommen.

## Die vier Frames (Node-IDs im Prototyp-Figma)

| Frame | Node | Bild |
|---|---|---|
| Page 1 / Overview / 760×430 | `4082:3` | `gen-seite1-760x430.png` |
| Page 2 / EQ Center / 700×420 | `4082:139` | `gen-seite2-eq-zentrale-700x420.png` |
| Seite 1 — Zustände | `4085:2` | `gen-seite1-zustaende.png` |
| Seite 2 — Zustände | `4086:2` | `gen-seite2-zustaende.png` |

## Was geändert wurde — je Regel

**Seite 1 (Anpassung):**
- Nav-Element **EQ CENTER** mit Trennstrich — Seitenwechsel im selben Fenster (Regel 1)
- Geste am Befund: **HOLD TO AUDITION · APPLY · REJECT** statt „AUDITION / HOLD"
  und „DISCARD" (Regel 37)
- **OPEN IN EQ** daneben — der Sprung-Button in die betroffene Sonden-Ansicht (Regel 14)
- Befundzeile trägt **CONFIDENCE** (Regel 23)

**Seite 2 (Umbau des Working Designs):**
- Kopf: **Durchschalter mit reinen Bus-Namen** direkt auf der Seite, aktiver Bus
  mit Emission-Unterstrich (Regeln 19/36), **MASTER als eigenes Element** neben dem
  Rad (Regel 33), LIVE-Punkt der gezeigten Spur (Regel 23), **OVERVIEW** als
  Rückweg (Regel 1)
- Draft-Kasten: Wort **„DRAFT"**, **Limit-Angabe entfernt** (Regeln 6/44);
  Automat unverändert HOLD TO AUDITION · APPLY · REJECT (Regel 37)
- **Undo als kleines Symbol auf dem Display** (↶, je Sonde) statt großem
  Footer-Button (Regel 41)
- **GLOBAL einklappbar** mit kleinem Pfeil, Standard zu (Regel 43)
- **MIX-Knob unten rechts, Tag „PLANNED"** — zweistufig global↔Spur (Regel 42);
  der Parametervertrag trägt Mix noch nicht, darum nur geplanter Platz (Regel 32)
- Analyzer-Abgriff beschriftet: **SPECTRUM POST · COMMITTED** (Regel 26)

**Zustands-Tafeln (gesondert, je Element eine kurze deutsche Erklärung):**
- Draft-Automat in vier Zuständen: Ruhe · Halten (Preview) · Lease mit Restzeit ·
  Blockiert (Transport steht / Aufnahme) — Regeln 37/44/10/11/29
- Fernbedienung: DISCONNECTED (keine editierbare Geisterkurve) · COMMAND SENT
  (unterwegs) · Host-Automation gewinnt sichtbar — Regeln 15/16/17
- **Zwei Spuren in EINEM Graph** mit eindeutigem Bearbeitungsziel, Spektrum nur
  der bearbeiteten Spur — Regeln 22/38/25
- GLOBAL aufgeklappt (die vier Werte aus Regel 5) — Regel 43
- Durchschalter bei 16 Bussen (Rad, ohne Menü) — Regeln 20/21/33/36
- MIX zweistufig (GLOBAL ↔ Spur), beide PLANNED — Regeln 42/32
- Seite 1: Probe-Zeile LIVE · STALE · **GAP (Segment-Lücke) ≠ EPOCH (Host-Sprung)**
  — zwei verschiedene Zeichen, nie verschmolzen (Regeln 23/24)
- Geste am Befund: Halten · Lease-Countdown · Blockiert mit sichtbarem Grund
  (Regeln 37/10/11/29)

## Bewusst NICHT entschieden (offen, nicht gestaltet)

- Größe der Seite 2 — Working-Design-Maß 700×420 beibehalten (**NAK-65**)
- Undo-Form auf Seite 1: Verlauf oder Zähler (**U2.8**)
- Sichtbarkeit der Sidechain-Quelle je Band (**U5** / NAK-33)
- Lease-Restzeit „7 s" in den Tafeln ist Demo-Wert; Startwert des Users ist 10 s

**Status: Claude-Vorschlag auf Spielregeln-Basis — kein User-Urteil.** Die
Renders hier sind Verlauf/Doku; die Truhe (`design/assets/figma/`) hält weiter
nur die Stände des Users. Nimmt der User die Frames (ggf. nach eigenen Figma-
Änderungen) ab, wird der Stand regulär in die Truhe geholt und die Abnahme nach
`design/abnahmen/` geschrieben.
