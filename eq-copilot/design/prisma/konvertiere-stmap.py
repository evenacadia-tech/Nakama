# ST-Map-Konverter (NAK-16) — laeuft im Blender-Python (numpy gebuendelt,
# bpy liest 32f-EXR verlustfrei; der Browser-Bildpfad wuerde die Daten
# zerstoeren). Pipeline: docs/research/2026-08-19-stmap-live-refraktion.md.
#
#   blender -b --factory-startup -P konvertiere-stmap.py -- \
#       --exr renders/stmap/probe-stmap-s0.exr \
#       [--exr-rausch renders/stmap/probe-stmap-s7.exr] \
#       --glanz renders/stmap/probe-glanz.png \
#       [--out-js renders/stmap/stmap-daten.js] \
#       [--out-bin renders/stmap/probe-stmap.bin] \
#       [--meta renders/stmap/probe-stmap-meta.json]
#
# Was er rechnet:
#   1. u = R/B, v = G/B (Unpremultiply — B ist das Transmissionsgewicht),
#      Gueltigkeit = B > Schwelle (TIR/Fehlschuss = B 0 -> Gewicht 0).
#   2. Plate-UV -> Weltpunkt (Plate-Quad aus dem Bake-Skript) -> Projektion
#      mit der bewiesenen Blatt-Kamera -> Screen-UV (v nach unten).
#      OFFSET-Form: off = ziel - pixelmitte. Offsets clustern um 0 ->
#      Half-Float tragfaehig (Praezisionsrechnung im Report).
#   3. Eingebauter Beweis: Pixel AUSSERHALB der Prisma-Silhouette sehen die
#      Plate direkt — ihr Offset MUSS ~0 sein (prueft Emission-Kodierung,
#      Kameramathe und Flip-Konvention in einer Zahl).
#   4. Rauschmessung Seed-gegen-Seed, Half-Float-Quantisierungsfehler,
#      Out-of-Range-Anteil, B-Histogramm (Fresnel-Gewichtung ja/nein).
#
# Ausgabe: rohes Float16-Binaerformat (RGBA: offU, offV, Gewicht, 1;
# Zeile 0 = OBEN) + stmap-daten.js (Base64 + Glanz-PNG als data-URL —
# file://-Blaetter koennen kein fetch() auf lokale Dateien, und
# file://-Bilder tainten WebGL; data-URLs nicht).

import base64
import json
import math
import os
import sys

import bpy
import numpy as np

argv = sys.argv[sys.argv.index("--") + 1:] if "--" in sys.argv else []


def arg(name, standard):
    return argv[argv.index(name) + 1] if name in argv else standard


HIER = os.path.dirname(os.path.abspath(__file__))


def pfad(p):
    return p if os.path.isabs(p) else os.path.join(HIER, p)


EXR = pfad(arg("--exr", "renders/stmap/probe-stmap-s0.exr"))
EXR_RAUSCH = arg("--exr-rausch", "")
GLANZ = pfad(arg("--glanz", "renders/stmap/probe-glanz.png"))
OUT_JS = pfad(arg("--out-js", "renders/stmap/stmap-daten.js"))
OUT_BIN = pfad(arg("--out-bin", "renders/stmap/probe-stmap.bin"))
META = pfad(arg("--meta", "renders/stmap/probe-stmap-meta.json"))

# Kamera — IDENTISCH zur bewiesenen JS-Projektion (pruefe-projektion.html)
# und zum Bake-Aufruf. sensor_fit AUTO: F_PIX bezieht sich auf die
# GROESSERE Kante.
LENS, SENSOR = 65.0, 36.0
POS = np.array([float(x) for x in arg("--kamera", "-1.65,-3.53,0.95").split(",")])
ZIEL = np.array([float(x) for x in arg("--ziel", "0,0,0.8").split(",")])

# Plate-Quad — Konstanten aus prisma-material-still.py (Code = Wahrheit):
# lokal ((+-pw/2, 0, +-ph/2)), UV (0,0) unten links, Drehung -25 Grad um Z,
# Ort (1.24, 2.6, 0.95).
PW, PH = 6.2, 3.875
PLATE_ROT = math.radians(-25.0)
PLATE_ORT = np.array([1.24, 2.6, 0.95])
B_SCHWELLE = 0.05           # darunter: TIR oder Fehlschuss -> ungueltig


def lade_pixel(dateipfad):
    img = bpy.data.images.load(dateipfad)
    w, h = img.size
    puffer = np.empty(w * h * 4, dtype=np.float32)
    img.pixels.foreach_get(puffer)
    bpy.data.images.remove(img)
    # Blender liefert Zeile 0 = UNTEN; wir arbeiten durchgehend mit
    # Zeile 0 = OBEN (Canvas-/Screen-Konvention, v nach unten).
    return np.flipud(puffer.reshape(h, w, 4)), w, h


def berechne_offsets(exr_pfad):
    px, w, h = lade_pixel(exr_pfad)
    r, g, b = px[..., 0], px[..., 1], px[..., 2]
    gueltig = b > B_SCHWELLE
    # Unpremultiply: entfernt jede Pfadgewichtung; B selbst ist das
    # Transmissionsgewicht fuer den Composite.
    u = np.where(gueltig, r / np.maximum(b, 1e-9), 0.0)
    v = np.where(gueltig, g / np.maximum(b, 1e-9), 0.0)

    # Plate-UV -> Weltpunkt (Quad ist planar, bilinear = affin).
    c, s = math.cos(PLATE_ROT), math.sin(PLATE_ROT)
    def welt(lx, lz):
        return np.array([c * lx, s * lx, lz]) + PLATE_ORT
    c0 = welt(-PW / 2, -PH / 2)          # UV (0,0)
    du = welt(PW / 2, -PH / 2) - c0      # UV-u-Achse
    dv = welt(-PW / 2, PH / 2) - c0      # UV-v-Achse (Blender: v nach OBEN)
    wp = (c0[None, None, :]
          + u[..., None] * du[None, None, :]
          + v[..., None] * dv[None, None, :])

    # Kamerabasis wie to_track_quat("-Z","Y") / JS-KAM.
    f = ZIEL - POS
    f = f / np.linalg.norm(f)
    cam_z = -f
    up = np.array([0.0, 0.0, 1.0]) - f * f[2]
    cam_y = up / np.linalg.norm(up)
    cam_x = np.cross(cam_y, cam_z)
    f_pix = LENS / SENSOR * max(w, h)

    d = wp - POS[None, None, :]
    xc = d @ cam_x
    yc = d @ cam_y
    zc = d @ cam_z
    inv = 1.0 / (-zc)
    sx = w / 2 + f_pix * xc * inv        # Pixel, x nach rechts
    sy = h / 2 - f_pix * yc * inv        # Pixel, y nach UNTEN (Canvas)

    # Offset-Form gegen die Pixelmitte (Halbpixel!).
    spalte = (np.arange(w, dtype=np.float32) + 0.5) / w
    zeile = (np.arange(h, dtype=np.float32) + 0.5) / h
    ident_u = np.broadcast_to(spalte[None, :], (h, w))
    ident_v = np.broadcast_to(zeile[:, None], (h, w))
    off_u = np.where(gueltig, sx / w - ident_u, 0.0).astype(np.float32)
    off_v = np.where(gueltig, sy / h - ident_v, 0.0).astype(np.float32)
    gewicht = np.where(gueltig, np.clip(b, 0.0, 1.0), 0.0).astype(np.float32)
    return {
        "w": w, "h": h, "b_roh": b, "gueltig": gueltig,
        "off_u": off_u, "off_v": off_v, "gewicht": gewicht,
        "ident_u": ident_u, "ident_v": ident_v,
    }


# ---------------------------------------------------------- Dreh-Paket
# --dreh: 72-Frame-Sequenz -> EIN gezipptes Binaerpaket fuers Blatt.
# Groessenhebel (453 MB roh waeren unhaltbar): halbe Aufloesung
# (gewichtsgemitteltes 2x2-Box-Downsample — Offsets sind glatt, die
# weiche Kante wandert ins Gewicht) + Crop auf die Silhouetten-Union
# aller Frames + gzip. Glanz-Frames als WebP-Bytes im Paket (im Blatt
# per createImageBitmap(Blob) dekodiert — Blob ist same-origin, kein
# file://-Taint). Format: "NAKDREH1" | u32 metaLen | meta-JSON |
# je Frame u32+Karten-f16 und u32+Glanz-WebP.
if "--dreh" in argv:
    import gzip
    import struct

    N = int(arg("--dreh", "72"))
    STMAP_DIR = pfad(arg("--dreh-stmap", "renders/dreh-stmap"))
    GLANZ_DIR = pfad(arg("--dreh-glanz", "renders/dreh-glanz"))
    PAKET = pfad(arg("--paket", "renders/stmap/dreh-karten.bin.gz"))

    karten = []          # je Frame: dict(off_u, off_v, gewicht) halbaufgeloest
    glanz_bytes = []
    x0 = y0 = 10**9
    x1 = y1 = -1
    voll_w = voll_h = None
    for i in range(N):
        exr = os.path.join(STMAP_DIR, f"f{i:03d}.exr")
        webp = os.path.join(GLANZ_DIR, f"f{i:03d}.webp")
        if not os.path.exists(exr) or not os.path.exists(webp):
            raise SystemExit(f"Frame {i:03d} fehlt ({exr} / {webp})")
        k = berechne_offsets(exr)
        voll_w, voll_h = k["w"], k["h"]
        # Crop-Quelle ist die PRISMA-Silhouette (Glanz-Alpha) — die
        # Karten-Gueltigkeit umfasst auch die Direktsicht neben dem
        # Prisma und wuerde fast nichts beschneiden (gemessen: 103 MB).
        gpx, _, _ = lade_pixel(webp)
        gm = gpx[..., 3] > 0.5
        if gm.any():
            ys, xs = np.nonzero(gm)
            x0 = min(x0, int(xs.min())); x1 = max(x1, int(xs.max()))
            y0 = min(y0, int(ys.min())); y1 = max(y1, int(ys.max()))
        # gewichtsgemitteltes 2x2-Downsample (ungueltig zieht nicht mit)
        def halb(a):
            return (a[0::2, 0::2] + a[1::2, 0::2] + a[0::2, 1::2] + a[1::2, 1::2]) * 0.25
        gw = k["gewicht"]
        s = halb(gw)
        su = halb(k["off_u"] * gw)
        sv = halb(k["off_v"] * gw)
        sicher = np.maximum(s, 1e-6)
        karten.append({
            "off_u": (su / sicher).astype(np.float32),
            "off_v": (sv / sicher).astype(np.float32),
            "gewicht": s.astype(np.float32),
        })
        with open(webp, "rb") as f:
            glanz_bytes.append(f.read())
        print(f"gelesen {i + 1}/{N}", flush=True)

    # Crop auf gerade Koordinaten runden (Halbaufloesungs-Gitter) + Rand
    x0 = max(0, (x0 - 4) // 2 * 2); y0 = max(0, (y0 - 4) // 2 * 2)
    x1 = min(voll_w, ((x1 + 5) // 2) * 2); y1 = min(voll_h, ((y1 + 5) // 2) * 2)
    bw, bh = (x1 - x0) // 2, (y1 - y0) // 2

    meta = {
        "frames": N,
        "basisRotGrad": 28.0,
        "schrittGrad": 120.0 / N,
        "glas": {"w": voll_w, "h": voll_h},
        "karte": {"x": x0, "y": y0, "w": x1 - x0, "h": y1 - y0, "bw": bw, "bh": bh},
        "kanaele": "RGBA float16 (offU, offV, gewichtB, 1), Zeile 0 = oben, halbe Aufloesung",
        "glanzFormat": "webp",
    }
    teile = [b"NAKDREH1"]
    meta_roh = json.dumps(meta, ensure_ascii=False).encode("utf-8")
    teile.append(struct.pack("<I", len(meta_roh)))
    teile.append(meta_roh)
    for i in range(N):
        kk = karten[i]
        block = np.empty((bh, bw, 4), dtype=np.float16)
        sy, sx = y0 // 2, x0 // 2
        block[..., 0] = kk["off_u"][sy:sy + bh, sx:sx + bw]
        block[..., 1] = kk["off_v"][sy:sy + bh, sx:sx + bw]
        block[..., 2] = kk["gewicht"][sy:sy + bh, sx:sx + bw]
        block[..., 3] = 1.0
        roh = block.tobytes()
        teile.append(struct.pack("<I", len(roh)))
        teile.append(roh)
        teile.append(struct.pack("<I", len(glanz_bytes[i])))
        teile.append(glanz_bytes[i])
    paket_roh = b"".join(teile)
    os.makedirs(os.path.dirname(PAKET), exist_ok=True)
    with open(PAKET, "wb") as f:
        f.write(gzip.compress(paket_roh, 6))
    print(f"PAKET: {PAKET}")
    print(json.dumps({
        "frames": N, "crop": meta["karte"],
        "roh_mb": round(len(paket_roh) / 2**20, 1),
        "gz_mb": round(os.path.getsize(PAKET) / 2**20, 1),
    }, ensure_ascii=False))
    raise SystemExit(0)

k = berechne_offsets(EXR)
w, h = k["w"], k["h"]

# Coverage (Prisma-Silhouette) aus dem Glanz-Pass — Alpha ist nicht
# farbverwaltet, der Kanal ist also roh verwendbar.
glanz_px, gw, gh = lade_pixel(GLANZ)
if (gw, gh) != (w, h):
    raise SystemExit(f"Glanz-Groesse {gw}x{gh} passt nicht zur Karte {w}x{h}")
deckung = glanz_px[..., 3] > 0.5

px_norm = float(np.hypot(w, h))          # |uv|-Einheit -> Pixel: getrennt je Achse
def off_px(m):
    return np.hypot(k["off_u"][m] * w, k["off_v"][m] * h)

stats = {}

# 1. B-Kanal: Fresnel-Gewichtung ja/nein + TIR-Zonen (Probe-Checkliste).
b_glas = k["b_roh"][deckung]
stats["b_histogramm"] = {
    "anteil_fehl_oder_tir(B<0.05)": round(float(np.mean(b_glas < B_SCHWELLE)), 4),
    "anteil_zwischen(0.05-0.95)": round(float(np.mean((b_glas >= 0.05) & (b_glas <= 0.95))), 4),
    "anteil_voll(B>0.95)": round(float(np.mean(b_glas > 0.95)), 4),
    "min": round(float(b_glas.min()), 4), "max": round(float(b_glas.max()), 4),
    "mittel_gueltig": round(float(b_glas[b_glas > B_SCHWELLE].mean()), 4)
    if bool((b_glas > B_SCHWELLE).any()) else None,
}

# 2. Eingebauter Beweis: direkte Plate-Sicht neben dem Prisma -> Offset ~0.
aussen = (~deckung) & k["gueltig"]
stats["beweis_direktsicht_offset_px"] = {
    "pixel": int(aussen.sum()),
    "mittel": round(float(off_px(aussen).mean()), 4) if bool(aussen.any()) else None,
    "p99": round(float(np.percentile(off_px(aussen), 99)), 4) if bool(aussen.any()) else None,
    "max": round(float(off_px(aussen).max()), 4) if bool(aussen.any()) else None,
}

# 3. Offsets im Glas: Groessenordnung (Half-Float-Tragfaehigkeit) und
#    Out-of-Range-Anteil (Sampling faellt auf CLAMP_TO_EDGE).
innen = deckung & k["gueltig"]
ziel_u = k["ident_u"] + k["off_u"]
ziel_v = k["ident_v"] + k["off_v"]
ausserhalb = innen & ((ziel_u < 0) | (ziel_u > 1) | (ziel_v < 0) | (ziel_v > 1))
o = off_px(innen)
stats["offsets_im_glas_px"] = {
    "pixel": int(innen.sum()),
    "median": round(float(np.median(o)), 2),
    "p95": round(float(np.percentile(o, 95)), 2),
    "max": round(float(o.max()), 2),
    "max_uv": round(float(np.maximum(np.abs(k["off_u"][innen]).max(),
                                     np.abs(k["off_v"][innen]).max())), 4),
    "anteil_out_of_range": round(float(ausserhalb.sum() / max(1, innen.sum())), 4),
    "anteil_tir_im_glas": round(float(np.mean(k["b_roh"][deckung] < B_SCHWELLE)), 4),
}

# 3b. Ziel-UVs je Achse: dimensioniert den noetigen Hintergrund-Rand
# (Ebene 0 lebt im Blatt vollflaechig HINTER der Buehne — die Probe
# braucht denselben Ueberstand, sonst clampen 72 % der Glaspixel).
def achse_stats(zielwerte):
    zw = zielwerte[innen]
    return {"min": round(float(zw.min()), 3),
            "p1": round(float(np.percentile(zw, 1)), 3),
            "p99": round(float(np.percentile(zw, 99)), 3),
            "max": round(float(zw.max()), 3)}
stats["ziel_uv_bereich"] = {"u": achse_stats(ziel_u), "v": achse_stats(ziel_v)}

# 4. Half-Float-Quantisierung: float32 -> float16 -> Fehler in Pixeln.
f16u = k["off_u"].astype(np.float16).astype(np.float32)
f16v = k["off_v"].astype(np.float16).astype(np.float32)
qfehler = np.hypot((f16u - k["off_u"])[innen] * w, (f16v - k["off_v"])[innen] * h)
stats["half_float_quantisierung_px"] = {
    "mittel": round(float(qfehler.mean()), 4),
    "max": round(float(qfehler.max()), 4),
}

# 5. Rauschen Seed-gegen-Seed (Delta-Pfad: erwartet nur Subpixel-Jitter).
if EXR_RAUSCH:
    k2 = berechne_offsets(pfad(EXR_RAUSCH))
    beide = innen & k2["gueltig"]
    d_px = np.hypot((k["off_u"] - k2["off_u"])[beide] * w,
                    (k["off_v"] - k2["off_v"])[beide] * h)
    stats["rauschen_seed_vs_seed_px"] = {
        "pixel": int(beide.sum()),
        "rms": round(float(np.sqrt(np.mean(d_px ** 2))), 4),
        "p99": round(float(np.percentile(d_px, 99)), 4),
        "max": round(float(d_px.max()), 4),
        "gueltigkeit_uneins": int((innen ^ (deckung & k2["gueltig"])).sum()),
    }

# ---------------------------------------------------------------- Ausgabe
# RGBA-Float16, interleaved, Zeile 0 = OBEN: offU, offV, Gewicht, 1.
karte = np.empty((h, w, 4), dtype=np.float16)
karte[..., 0] = k["off_u"]
karte[..., 1] = k["off_v"]
karte[..., 2] = k["gewicht"]
karte[..., 3] = 1.0
roh = karte.tobytes()

os.makedirs(os.path.dirname(OUT_BIN), exist_ok=True)
with open(OUT_BIN, "wb") as f:
    f.write(roh)

with open(GLANZ, "rb") as f:
    glanz_b64 = base64.b64encode(f.read()).decode("ascii")

meta = {
    "quelle_exr": os.path.basename(EXR),
    "breite": w, "hoehe": h,
    "format": "RGBA float16 interleaved, Zeile 0 = oben, v nach unten",
    "kanaele": ["offU (uv-Einheiten)", "offV", "gewichtB (Transmission, 0=TIR/fehl)", "1"],
    "kamera": {"lens": LENS, "sensor": SENSOR, "pos": POS.tolist(),
               "ziel": ZIEL.tolist(), "fit": "AUTO (groessere Kante)"},
    "stats": stats,
}
with open(META, "w", encoding="utf-8") as f:
    json.dump(meta, f, ensure_ascii=False, indent=2)

with open(OUT_JS, "w", encoding="utf-8") as f:
    f.write("// GENERIERT von konvertiere-stmap.py — nicht von Hand editieren.\n")
    f.write("// Karte: RGBA float16 (offU, offV, gewichtB, 1), Zeile 0 = oben.\n")
    f.write("window.STMAP_DATEN = {\n")
    f.write(f"  breite: {w}, hoehe: {h},\n")
    f.write(f"  meta: {json.dumps(meta, ensure_ascii=False)},\n")
    f.write(f'  b64: "{base64.b64encode(roh).decode("ascii")}",\n')
    f.write(f'  glanzDataUrl: "data:image/png;base64,{glanz_b64}"\n')
    f.write("};\n")

print("KONVERTIERT:", OUT_BIN, f"({len(roh)} Bytes)")
print(json.dumps(stats, ensure_ascii=False, indent=2))
