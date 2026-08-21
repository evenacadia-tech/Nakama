#!/usr/bin/env python3
"""Build the original Nakama Phase display face.

The font is intentionally small: it is the interface voice for axes, headings,
short labels and identifiers, not a replacement for the readable Manrope body
face.  Its geometry shares one rule with Nakama's symbols: a load-bearing
stroke is interrupted by a narrow "breath cut" and resumes with a slightly
different weight.

Requires fontTools with WOFF2 support.  Run with Python 3.13 from this folder.
"""

from __future__ import annotations

import math
from pathlib import Path

from fontTools.fontBuilder import FontBuilder
from fontTools.pens.ttGlyphPen import TTGlyphPen
from fontTools.ttLib import TTFont


UNITS_PER_EM = 1000
ASCENDER = 820
DESCENDER = -180
CAP_HEIGHT = 720
X_HEIGHT = 520
FONT_TIMESTAMP = 3869683200  # 2026-08-16 00:00 UTC in the TrueType epoch.

LEFT = 104.0
MID = 300.0
RIGHT = 496.0
TOP = 710.0
MIDDLE = 370.0
BOTTOM = 42.0

STEM = 76.0
HAIR = 58.0
GAP = 54.0


def glyph_name(character: str) -> str:
    if character == " ":
        return "space"
    if "A" <= character <= "Z" or "0" <= character <= "9":
        return character
    names = {
        ".": "period",
        ",": "comma",
        ":": "colon",
        ";": "semicolon",
        "+": "plus",
        "-": "hyphen",
        "−": "minus",
        "/": "slash",
        "%": "percent",
        "·": "periodcentered",
        "…": "ellipsis",
        "(": "parenleft",
        ")": "parenright",
        "Ä": "Adieresis",
        "Ö": "Odieresis",
        "Ü": "Udieresis",
    }
    return names.get(character, f"uni{ord(character):04X}")


class PhaseGlyph:
    """A glyph assembled from tapered, cut-ended vector strokes."""

    def __init__(self, width: int = 600) -> None:
        self.width = width
        self.pen = TTGlyphPen(None)

    def polygon(self, points: list[tuple[float, float]]) -> None:
        self.pen.moveTo(points[0])
        for point in points[1:]:
            self.pen.lineTo(point)
        self.pen.closePath()

    def stroke(
        self,
        start: tuple[float, float],
        end: tuple[float, float],
        width: float = STEM,
        end_scale: float = 0.88,
        cut: float = 11.0,
    ) -> None:
        x1, y1 = start
        x2, y2 = end
        dx, dy = x2 - x1, y2 - y1
        length = math.hypot(dx, dy)
        if length == 0:
            return
        ux, uy = dx / length, dy / length
        nx, ny = -uy, ux
        w1 = width / 2
        w2 = width * end_scale / 2

        # The unequal caps are the font's small brush-like signature.
        points = [
            (x1 + nx * w1 - ux * cut, y1 + ny * w1 - uy * cut),
            (x1 - nx * w1 + ux * (cut * 0.55), y1 - ny * w1 + uy * (cut * 0.55)),
            (x2 - nx * w2 + ux * cut, y2 - ny * w2 + uy * cut),
            (x2 + nx * w2 - ux * (cut * 0.55), y2 + ny * w2 - uy * (cut * 0.55)),
        ]
        self.polygon(points)

    def split_stroke(
        self,
        start: tuple[float, float],
        end: tuple[float, float],
        width: float = STEM,
        at: float = 0.58,
        gap: float = GAP,
    ) -> None:
        x1, y1 = start
        x2, y2 = end
        dx, dy = x2 - x1, y2 - y1
        length = math.hypot(dx, dy)
        if length == 0:
            return
        half = gap / (2 * length)
        p_a = (x1 + dx * max(0, at - half), y1 + dy * max(0, at - half))
        p_b = (x1 + dx * min(1, at + half), y1 + dy * min(1, at + half))
        self.stroke(start, p_a, width, 0.82, 8)
        self.stroke(p_b, end, width * 0.88, 0.88, 8)

    def diamond(self, center: tuple[float, float], radius: float = 23.0) -> None:
        x, y = center
        self.polygon([(x, y + radius), (x + radius, y), (x, y - radius), (x - radius, y)])

    def dot(self, center: tuple[float, float], radius: float = 22.0) -> None:
        # An octagonal dot stays crisp at the font's intended micro sizes.
        x, y = center
        k = radius * 0.42
        self.polygon([
            (x - k, y + radius), (x + k, y + radius), (x + radius, y + k),
            (x + radius, y - k), (x + k, y - radius), (x - k, y - radius),
            (x - radius, y - k), (x - radius, y + k),
        ])

    def finish(self):
        return self.pen.glyph()


def add_segments(g: PhaseGlyph, segments: list[tuple]) -> None:
    for segment in segments:
        kind, start, end, *rest = segment
        width = rest[0] if rest else STEM
        if kind == "s":
            g.stroke(start, end, width)
        elif kind == "c":
            at = rest[1] if len(rest) > 1 else 0.58
            g.split_stroke(start, end, width, at=at)
        else:
            raise ValueError(f"Unknown segment kind: {kind}")


def octagon(g: PhaseGlyph, *, left=LEFT, right=RIGHT, bottom=BOTTOM, top=TOP, cut_side="ur") -> None:
    chamfer = 78.0
    points = {
        "tl": (left + chamfer, top), "tr": (right - chamfer, top),
        "ru": (right, top - chamfer), "rl": (right, bottom + chamfer),
        "br": (right - chamfer, bottom), "bl": (left + chamfer, bottom),
        "lb": (left, bottom + chamfer), "lt": (left, top - chamfer),
    }
    ordered = ["tl", "tr", "ru", "rl", "br", "bl", "lb", "lt", "tl"]
    for index, (a, b) in enumerate(zip(ordered, ordered[1:])):
        side = ["top", "ur", "right", "lr", "bottom", "ll", "left", "ul"][index]
        if side == cut_side:
            g.split_stroke(points[a], points[b], STEM, at=0.46, gap=48)
        else:
            g.stroke(points[a], points[b], STEM)


def build_letter(character: str) -> PhaseGlyph:
    g = PhaseGlyph()
    L, C, R, T, M, B = LEFT, MID, RIGHT, TOP, MIDDLE, BOTTOM
    U, D = TOP - 78, BOTTOM + 78
    patterns: dict[str, list[tuple]] = {
        "A": [("s", (L, B), (C, T)), ("s", (C, T), (R, B)), ("c", (L + 90, M), (R - 90, M), HAIR, .56)],
        "B": [("c", (L, B), (L, T), STEM, .56), ("s", (L, T), (R - 80, T)), ("s", (R - 80, T), (R, T - 82)), ("s", (R, T - 82), (R, M + 72)), ("s", (R, M + 72), (R - 74, M)), ("s", (L, M), (R - 74, M), HAIR), ("s", (R - 74, M), (R, M - 72)), ("s", (R, M - 72), (R, B + 78)), ("s", (R, B + 78), (R - 80, B)), ("s", (L, B), (R - 80, B))],
        "C": [("s", (R, T), (L + 74, T)), ("s", (L + 74, T), (L, T - 74)), ("c", (L, T - 74), (L, B + 74), STEM, .53), ("s", (L, B + 74), (L + 74, B)), ("s", (L + 74, B), (R, B))],
        "D": [("c", (L, B), (L, T), STEM, .56), ("s", (L, T), (R - 82, T)), ("s", (R - 82, T), (R, U)), ("s", (R, U), (R, D)), ("s", (R, D), (R - 82, B)), ("s", (R - 82, B), (L, B))],
        "E": [("c", (L, B), (L, T), STEM, .57), ("s", (L, T), (R, T)), ("s", (L, M), (R - 54, M), HAIR), ("s", (L, B), (R, B))],
        "F": [("c", (L, B), (L, T), STEM, .57), ("s", (L, T), (R, T)), ("s", (L, M), (R - 54, M), HAIR)],
        "G": [("s", (R, T), (L + 74, T)), ("s", (L + 74, T), (L, T - 74)), ("c", (L, T - 74), (L, B + 74), STEM, .53), ("s", (L, B + 74), (L + 74, B)), ("s", (L + 74, B), (R, B)), ("s", (R, B), (R, M)), ("s", (R, M), (C + 18, M), HAIR)],
        "H": [("c", (L, B), (L, T), STEM, .60), ("s", (R, B), (R, T)), ("c", (L, M), (R, M), HAIR, .56)],
        "I": [("s", (L + 48, T), (R - 48, T), HAIR), ("c", (C, B), (C, T), STEM, .53), ("s", (L + 48, B), (R - 48, B), HAIR)],
        "J": [("s", (L + 28, T), (R, T), HAIR), ("c", (R, T), (R, B + 82), STEM, .58), ("s", (R, B + 82), (R - 82, B)), ("s", (R - 82, B), (L + 42, B)), ("s", (L + 42, B), (L, B + 72), HAIR)],
        "K": [("c", (L, B), (L, T), STEM, .56), ("s", (L + 20, M), (R, T)), ("s", (L + 20, M), (R, B))],
        "L": [("c", (L, B), (L, T), STEM, .56), ("s", (L, B), (R, B))],
        "M": [("c", (L, B), (L, T), STEM, .55), ("s", (L, T), (C, M + 36)), ("s", (C, M + 36), (R, T)), ("s", (R, T), (R, B))],
        "N": [("s", (L, B), (L, T)), ("c", (L, T), (R, B), STEM, .55), ("s", (R, B), (R, T))],
        "P": [("c", (L, B), (L, T), STEM, .60), ("s", (L, T), (R - 76, T)), ("s", (R - 76, T), (R, T - 76)), ("s", (R, T - 76), (R, M + 76)), ("s", (R, M + 76), (R - 76, M)), ("s", (R - 76, M), (L, M), HAIR)],
        "R": [("c", (L, B), (L, T), STEM, .60), ("s", (L, T), (R - 76, T)), ("s", (R - 76, T), (R, T - 76)), ("s", (R, T - 76), (R, M + 76)), ("s", (R, M + 76), (R - 76, M)), ("s", (R - 76, M), (L, M), HAIR), ("s", (C + 24, M), (R, B))],
        "S": [("s", (R, T), (L + 74, T)), ("s", (L + 74, T), (L, T - 72)), ("s", (L, T - 72), (L, M + 58)), ("c", (L, M + 58), (R, M - 58), STEM, .51), ("s", (R, M - 58), (R, B + 72)), ("s", (R, B + 72), (R - 74, B)), ("s", (R - 74, B), (L, B))],
        "T": [("c", (L, T), (R, T), STEM, .57), ("c", (C, B), (C, T), STEM, .53)],
        "U": [("s", (L, T), (L, B + 78)), ("c", (L, B + 78), (L + 78, B), STEM, .47), ("s", (L + 78, B), (R - 78, B)), ("s", (R - 78, B), (R, B + 78)), ("s", (R, B + 78), (R, T))],
        "V": [("c", (L, T), (C, B), STEM, .56), ("s", (C, B), (R, T))],
        "W": [("s", (L, T), (L + 82, B)), ("s", (L + 82, B), (C, M - 60)), ("s", (C, M - 60), (R - 82, B)), ("c", (R - 82, B), (R, T), STEM, .45)],
        "X": [("c", (L, T), (R, B), STEM, .52), ("c", (L, B), (R, T), STEM, .48)],
        "Y": [("s", (L, T), (C, M)), ("s", (R, T), (C, M)), ("c", (C, M), (C, B), STEM, .52)],
        "Z": [("s", (L, T), (R, T)), ("c", (R, T), (L, B), STEM, .54), ("s", (L, B), (R, B))],
    }
    if character == "O" or character == "Q":
        octagon(g, cut_side="ur")
        if character == "Q":
            g.stroke((C + 26, M - 44), (R + 34, B - 32), HAIR)
    else:
        add_segments(g, patterns[character])
    return g


def build_digit(character: str) -> PhaseGlyph:
    g = PhaseGlyph(width=560)
    L, C, R, T, M, B = 92.0, 280.0, 468.0, TOP, MIDDLE, BOTTOM
    patterns: dict[str, list[tuple]] = {
        "1": [("s", (L + 70, T - 90), (C, T)), ("c", (C, T), (C, B), STEM, .56), ("s", (L + 54, B), (R - 18, B), HAIR)],
        "2": [("s", (L, T), (R - 72, T)), ("s", (R - 72, T), (R, T - 72)), ("s", (R, T - 72), (R, M + 56)), ("c", (R, M + 56), (L, B), STEM, .48), ("s", (L, B), (R, B))],
        "3": [("s", (L, T), (R - 72, T)), ("s", (R - 72, T), (R, T - 72)), ("c", (R, T - 72), (R, B + 72), STEM, .54), ("s", (R, B + 72), (R - 72, B)), ("s", (R - 72, B), (L, B)), ("s", (C - 20, M), (R, M), HAIR)],
        "4": [("s", (L, T), (L, M)), ("c", (L, M), (R, M), STEM, .58), ("s", (R, T), (R, B))],
        "5": [("s", (R, T), (L, T)), ("s", (L, T), (L, M)), ("s", (L, M), (R - 70, M)), ("s", (R - 70, M), (R, M - 70)), ("c", (R, M - 70), (R - 70, B), STEM, .51), ("s", (R - 70, B), (L, B))],
        "6": [("s", (R, T), (L + 70, T)), ("s", (L + 70, T), (L, T - 70)), ("c", (L, T - 70), (L, B + 70), STEM, .54), ("s", (L, B + 70), (L + 70, B)), ("s", (L + 70, B), (R - 70, B)), ("s", (R - 70, B), (R, B + 70)), ("s", (R, B + 70), (R, M)), ("s", (R, M), (L, M), HAIR)],
        "7": [("c", (L, T), (R, T), STEM, .58), ("c", (R, T), (L + 90, B), STEM, .52)],
        "9": [("s", (L + 70, T), (R - 70, T)), ("s", (R - 70, T), (R, T - 70)), ("c", (R, T - 70), (R, B + 70), STEM, .56), ("s", (L, T - 70), (L, M)), ("s", (L, M), (R, M), HAIR), ("s", (R, B + 70), (R - 70, B)), ("s", (R - 70, B), (L, B))],
    }
    if character in {"0", "8"}:
        if character == "0":
            octagon(g, left=L, right=R, cut_side="ur")
            g.stroke((L + 122, B + 95), (R - 112, T - 104), HAIR * .65)
        else:
            octagon(g, left=L, right=R, bottom=M - 20, top=T, cut_side="ur")
            octagon(g, left=L, right=R, bottom=B, top=M + 20, cut_side="ll")
    else:
        add_segments(g, patterns[character])
    return g


def build_punctuation(character: str) -> PhaseGlyph:
    widths = {".": 300, ",": 300, ":": 300, ";": 300, "(": 370, ")": 370}
    g = PhaseGlyph(width=widths.get(character, 500))
    if character == ".":
        g.diamond((150, 72), 24)
    elif character == ",":
        g.diamond((150, 86), 24)
        g.stroke((156, 62), (118, -34), 34)
    elif character == ":":
        g.diamond((150, 500), 22)
        g.diamond((150, 110), 22)
    elif character == ";":
        g.diamond((150, 500), 22)
        g.diamond((150, 120), 22)
        g.stroke((156, 98), (118, 4), 32)
    elif character in {"-", "−"}:
        g.split_stroke((70, MIDDLE), (430, MIDDLE), HAIR, at=.58, gap=46)
    elif character == "+":
        g.split_stroke((60, MIDDLE), (440, MIDDLE), HAIR, at=.58, gap=46)
        g.split_stroke((250, 180), (250, 560), HAIR, at=.42, gap=42)
    elif character == "/":
        g.split_stroke((88, BOTTOM - 20), (412, TOP + 20), HAIR, at=.55, gap=46)
    elif character == "%":
        g.dot((115, 590), 46)
        g.dot((385, 160), 46)
        g.split_stroke((100, 48), (400, 704), HAIR, at=.55, gap=48)
    elif character == "·":
        g.diamond((250, MIDDLE), 22)
    elif character == "…":
        g.diamond((108, 72), 21)
        g.diamond((250, 72), 21)
        g.diamond((392, 72), 21)
    elif character == "(":
        g.stroke((278, TOP), (118, TOP - 138), HAIR)
        g.split_stroke((118, TOP - 138), (118, BOTTOM + 138), HAIR, at=.53, gap=48)
        g.stroke((118, BOTTOM + 138), (278, BOTTOM), HAIR)
    elif character == ")":
        g.stroke((92, TOP), (252, TOP - 138), HAIR)
        g.split_stroke((252, TOP - 138), (252, BOTTOM + 138), HAIR, at=.53, gap=48)
        g.stroke((252, BOTTOM + 138), (92, BOTTOM), HAIR)
    return g


def build_dieresis(base: str) -> PhaseGlyph:
    g = build_letter(base)
    g.diamond((228, 794), 19)
    g.diamond((372, 794), 19)
    return g


def build_font(output_dir: Path) -> tuple[Path, Path]:
    output_dir.mkdir(parents=True, exist_ok=True)

    characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,:;+-−/%·…()ÄÖÜ"
    glyph_order = [".notdef", "space"]
    cmap: dict[int, str] = {ord(" "): "space"}
    glyphs = {}
    metrics = {}

    notdef = PhaseGlyph()
    octagon(notdef, left=150, right=450, bottom=120, top=650, cut_side="right")
    glyphs[".notdef"] = notdef.finish()
    metrics[".notdef"] = (600, 0)

    empty = PhaseGlyph(width=320)
    glyphs["space"] = empty.finish()
    metrics["space"] = (320, 0)

    for character in characters:
        name = glyph_name(character)
        if name in glyphs:
            cmap[ord(character)] = name
            continue
        if "A" <= character <= "Z":
            shape = build_letter(character)
        elif "0" <= character <= "9":
            shape = build_digit(character)
        elif character in {"Ä", "Ö", "Ü"}:
            shape = build_dieresis({"Ä": "A", "Ö": "O", "Ü": "U"}[character])
        else:
            shape = build_punctuation(character)
        glyphs[name] = shape.finish()
        metrics[name] = (shape.width, 0)
        cmap[ord(character)] = name

    # Lowercase deliberately resolves to the same small-cap drawing.  Text in
    # this face is short interface language; body copy stays in Manrope.
    for upper in "ABCDEFGHIJKLMNOPQRSTUVWXYZ":
        cmap[ord(upper.lower())] = glyph_name(upper)
    cmap[ord("ä")] = glyph_name("Ä")
    cmap[ord("ö")] = glyph_name("Ö")
    cmap[ord("ü")] = glyph_name("Ü")

    glyph_order.extend(name for name in glyphs if name not in glyph_order)

    builder = FontBuilder(UNITS_PER_EM, isTTF=True)
    builder.setupGlyphOrder(glyph_order)
    builder.setupCharacterMap(cmap)
    builder.setupGlyf(glyphs)
    builder.setupHorizontalMetrics(metrics)
    builder.setupHorizontalHeader(ascent=ASCENDER, descent=DESCENDER, lineGap=0)
    builder.setupNameTable({
        "familyName": "Nakama Phase",
        "styleName": "Regular",
        "uniqueFontIdentifier": "evenacadia:Nakama Phase:1.000",
        "fullName": "Nakama Phase Regular",
        "psName": "NakamaPhase-Regular",
        "version": "Version 1.000",
        "manufacturer": "evenacadia",
        "designer": "evenacadia / Codex",
        "description": "Original display alphabet for the Nakama spectral field interface.",
        "licenseDescription": "Project-owned original geometry for the private Nakama product.",
    })
    builder.setupOS2(
        sTypoAscender=ASCENDER,
        sTypoDescender=DESCENDER,
        sTypoLineGap=0,
        usWinAscent=860,
        usWinDescent=190,
        sxHeight=X_HEIGHT,
        sCapHeight=CAP_HEIGHT,
        usWeightClass=520,
        usWidthClass=4,
        fsSelection=0x40,
    )
    builder.setupPost(italicAngle=0, underlinePosition=-90, underlineThickness=45, isFixedPitch=0)
    builder.setupMaxp()
    builder.font.recalcTimestamp = False
    builder.font["head"].created = FONT_TIMESTAMP
    builder.font["head"].modified = FONT_TIMESTAMP

    ttf_path = output_dir / "NakamaPhase-Regular.ttf"
    woff2_path = output_dir / "NakamaPhase-Regular.woff2"
    builder.save(ttf_path)

    web_font = TTFont(ttf_path, recalcTimestamp=False)
    web_font.recalcTimestamp = False
    web_font.flavor = "woff2"
    web_font.save(woff2_path)
    return ttf_path, woff2_path


if __name__ == "__main__":
    ttf, woff2 = build_font(Path(__file__).resolve().parent)
    print(f"built {ttf.name} ({ttf.stat().st_size} bytes)")
    print(f"built {woff2.name} ({woff2.stat().st_size} bytes)")
