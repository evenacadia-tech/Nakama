# aufschliessen.py — macht die Assettruhe lesbar, ohne sie anzufassen.
#
# CLAUDE.md Regel 1: assets/ ist vor JEDER Gestaltungsentscheidung die erste
# Quelle. Gemessen lag darin am 2026-08-21 nichts, was eine Sitzung oeffnen
# kann: 110.sketch, 110_colorAdj.psd, 297*.fig, 301.fig. Genau die zwei
# Referenzen, die werkzeug/weltenwahl.html "direkte Uebersetzungen" nennt,
# konnte niemand nachsehen.
#
# Dieses Skript liest die Truhe NUR und schreibt ausschliesslich hierher.
# Vorbild ist werkzeug/textur/korn.png: herauskopiert, aufbereitet, die Truhe
# selbst unveraendert.
#
# AUFRUF   python werkzeug/truhe/aufschliessen.py
# BRAUCHT  Python mit Pillow (PIL). Sonst nichts — keine Build-Kette.
#
# Warum ein Skript und keine Handarbeit: werkzeug/PRUEFLISTE.md haelt den
# benannten Verlust fest — "die Skripte waren sitzungslokal und sind weg".
# Hier nicht noch einmal.

import io
import os
import re
import struct
import sys
import zipfile
import zlib

try:
    from PIL import Image
except ImportError:
    sys.exit("Pillow fehlt.  Abhilfe:  python -m pip install Pillow")

WURZEL = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TRUHE = os.path.join(WURZEL, "assets")
ZIEL = os.path.dirname(os.path.abspath(__file__))
PNG = b"\x89PNG\r\n\x1a\n"


def sag(*a):
    print(*a, flush=True)


def bild(daten, name, herkunft, mindest=200):
    """Speichert ein Bild, wenn es gross genug ist, um etwas zu zeigen."""
    try:
        im = Image.open(io.BytesIO(daten))
        im.load()
    except Exception:
        return None
    if max(im.size) < mindest:
        return None
    # Ein undurchsichtiges Bild braucht keinen Alphakanal. Das ist verlustfrei
    # und halbiert die Dateien — 301.png faellt von 4,6 auf 2,3 MB.
    if im.mode == "RGBA" and im.getchannel("A").getextrema() == (255, 255):
        im = im.convert("RGB")
    im.save(os.path.join(ZIEL, name), optimize=True)
    sag(f"  {name:<34} {im.size[0]}x{im.size[1]:<6} {im.mode:<5} <- {herkunft}")
    return im.size


def rtf_text(daten):
    """RTF auf lesbaren Text eindampfen. Reihenfolge ist entscheidend:
    erst Steuergruppen, dann Hex-Escapes, dann Steuerworte, dann Klammern."""
    t = daten.decode("cp1252", "replace")
    for _ in range(4):  # verschachtelte Gruppen, innen nach aussen
        t = re.sub(r"\{\\\*[^{}]*\}", "", t)
        t = re.sub(r"\{\\(?:fonttbl|colortbl|stylesheet|info)[^{}]*\}", "", t)
    t = re.sub(r"\\'([0-9a-fA-F]{2})", lambda m: bytes([int(m.group(1), 16)]).decode("cp1252", "replace"), t)
    t = t.replace("\\\n", "\n")
    t = re.sub(r"\\par\b", "\n", t)
    t = re.sub(r"\\[a-zA-Z]+-?\d*\s?", "", t)
    t = t.replace("{", "").replace("}", "").replace("\\", "\n")
    t = re.sub(r"[ \t]+", " ", t)
    return "\n".join(z.strip() for z in t.split("\n") if z.strip())


def aus_zip(pfad, eintrag):
    with zipfile.ZipFile(pfad) as z:
        return z.read(eintrag)


def fig_zip(roh, praefix):
    """Neue .fig-Dateien sind ZIPs mit thumbnail.png, meta.json und images/."""
    with zipfile.ZipFile(io.BytesIO(roh)) as z:
        namen = z.namelist()
        if "meta.json" in namen:
            sag(f"  meta: {z.read('meta.json').decode('utf-8', 'replace')[:200]}")
        for n in namen:
            if n.endswith("/") or z.getinfo(n).file_size == 0:
                continue
            if n == "thumbnail.png":
                bild(z.read(n), f"{praefix}-thumbnail.png", f"{praefix}.fig/{n}")
            elif n.startswith("images/"):
                bild(z.read(n), f"{praefix}-bild-{n.split('/')[-1][:8]}.png", f"{praefix}.fig/{n}")


def fig_kiwi(roh, praefix, hoechstens=8):
    """Alte .fig-Dateien: 8 B Magic + 4 B Version, dann laengenpraefigierte
    Deflate-Bloecke. Die Gestaltung selbst ist Vektor (kiwi-kodiert) und hier
    nicht zu holen — nur die eingebetteten Bitmaps."""
    off, nr, treffer = 12, 0, 0
    while off + 4 <= len(roh) and nr < 12 and treffer < hoechstens:
        (laenge,) = struct.unpack_from("<I", roh, off)
        off += 4
        block, off, nr = roh[off:off + laenge], off + laenge, nr + 1
        try:
            aus = zlib.decompress(block, -15)
        except Exception:
            continue
        i = aus.find(PNG)
        while i != -1 and treffer < hoechstens:
            e = aus.find(b"IEND", i)
            if e == -1:
                break
            if bild(aus[i:e + 8], f"{praefix}-bild{treffer + 1}.png", f"{praefix}.fig Block {nr}", mindest=300):
                treffer += 1
            i = aus.find(PNG, e)
    if treffer == 0:
        sag(f"  {praefix}: keine brauchbare Bitmap — die Gestaltung ist Vektor")


def main():
    fehlt = [f for f in ("110.zip", "297.zip", "301.zip") if not os.path.exists(os.path.join(TRUHE, f))]
    if fehlt:
        sys.exit(f"Truhe unvollstaendig, es fehlen: {', '.join(fehlt)}  (gesucht in {TRUHE})")

    sag("301.zip — 'Neumorphic Dark Knobs', die Vorlage der Welt 'Dunkles Geraet'")
    p301 = os.path.join(TRUHE, "301.zip")
    bild(aus_zip(p301, "301.png"), "301-vorschau.png", "301.zip/301.png")
    open(os.path.join(ZIEL, "301-readme.txt"), "w", encoding="utf-8").write(rtf_text(aus_zip(p301, "readme.rtf")))
    sag("  301-readme.txt")
    fig_zip(aus_zip(p301, "301.fig"), "301")

    sag("110.zip — die Vorlage der Welt 'Helles Objekt' (ohne readme)")
    p110 = os.path.join(TRUHE, "110.zip")
    sk = aus_zip(p110, "110.sketch")
    with zipfile.ZipFile(io.BytesIO(sk)) as z:  # .sketch ist selbst ein ZIP
        for n in z.namelist():
            if n.startswith("previews/"):
                bild(z.read(n), "110-vorschau.png", f"110.sketch/{n}")
            elif n.startswith("images/"):
                bild(z.read(n), f"110-bild-{n.split('/')[-1][:8]}.png", f"110.sketch/{n}")
    bild(aus_zip(p110, "110_colorAdj.psd"), "110-psd.png", "110.zip/110_colorAdj.psd (Composite)")

    sag("297.zip — 'Campfire' Dark Social UI-Kit")
    p297 = os.path.join(TRUHE, "297.zip")
    open(os.path.join(ZIEL, "297-readme.txt"), "w", encoding="utf-8").write(rtf_text(aus_zip(p297, "297/readme.rtf")))
    sag("  297-readme.txt")
    fig_kiwi(aus_zip(p297, "297/297 - Dark Social UI-Kit.fig"), "297")

    sag("\nTruhe unveraendert — dieses Skript oeffnet nur lesend.")


if __name__ == "__main__":
    main()
