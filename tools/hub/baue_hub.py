#!/usr/bin/env python3
"""Baut den Nakama-Hub (Briefing-Artefakt) aus docs/hub/hub.json.

    py -3.13 tools/hub/baue_hub.py            # baut docs/hub/hub.html
    py -3.13 tools/hub/baue_hub.py --pruefen  # prüft nur hub.json, baut nichts

Die Seite ist ein Artefakt auf claude.ai. Veröffentlichen kann nur Claude
(Artifact-Werkzeug, mit `url` aus hub.json — dieselbe Adresse bleibt).
Bilder kommen aus der Design-Truhe (design/assets/figma/, neuester Stand je
App, halbiert auf 2×; seit 22.08. im Repo, vorher eigenes Repo Nakama-Design).
"""
from __future__ import annotations

import argparse
import base64
import datetime as dt
import io
import json
import os
import re
import subprocess
import sys
from pathlib import Path

HIER = Path(__file__).resolve().parent
REPO = HIER.parent.parent
HUB_JSON = REPO / "docs" / "hub" / "hub.json"
HUB_HTML = REPO / "docs" / "hub" / "hub.html"
VORLAGE = HIER / "seite.html"
DESIGN = Path(os.environ.get("NAKAMA_DESIGN", REPO / "design"))
TRUHE = DESIGN / "assets" / "figma"
APPS = ("gen", "probeeq", "suna")
BILDER = REPO / "docs" / "hub" / "bilder"      # Zeigebilder zu den Karten (committet)
EINGANG = REPO / "docs" / "hub" / "eingang"    # vom User über den Hub hochgeladene Bilder
MAX_ZEIGEBREITE = 1600
STATUS_ERLAUBT = {"erledigt", "gebaut", "naechster", "bei dir", "wartet", "offen"}
DRINGLICHKEIT_ERLAUBT = {"jetzt", "wenn du dazu kommst", "wissen", "später"}
MAX_BYTES = 16_000_000


def lade() -> dict:
    with HUB_JSON.open(encoding="utf-8") as f:
        return json.load(f)


def pruefe(s: dict) -> list[str]:
    fehler: list[str] = []
    for k in ("stand", "stand_notiz", "naechster_schritt", "bei_dir", "plan", "design", "reviews", "eingang"):
        if k not in s:
            fehler.append(f"Schlüssel fehlt: {k}")
    if fehler:
        return fehler
    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", s["stand"]):
        fehler.append("stand muss JJJJ-MM-TT sein")
    for k in ("technik", "design", "dich"):
        if not s["naechster_schritt"].get(k):
            fehler.append(f"naechster_schritt.{k} leer")
    ids = set()
    for b in s["bei_dir"]:
        for k in ("id", "dringlichkeit", "art", "titel", "was", "warum", "wo"):
            if not b.get(k):
                fehler.append(f"bei_dir {b.get('id','?')}: {k} leer")
        if b.get("dringlichkeit") not in DRINGLICHKEIT_ERLAUBT:
            fehler.append(f"bei_dir {b.get('id')}: Dringlichkeit '{b.get('dringlichkeit')}' unbekannt")
        if b.get("id") in ids:
            fehler.append(f"bei_dir: doppelte ID {b.get('id')}")
        ids.add(b.get("id"))
        for pk in b.get("punkte", []):
            for k in ("id", "titel", "entwurf", "alternative"):
                if not pk.get(k):
                    fehler.append(f"bei_dir {b.get('id')}: Punkt {pk.get('id','?')} ohne {k}")
            if not str(pk.get("id", "")).startswith(str(b.get("id")) + "."):
                fehler.append(f"bei_dir {b.get('id')}: Punkt-ID {pk.get('id')} muss mit '{b.get('id')}.' beginnen")
            if pk.get("id") in ids:
                fehler.append(f"bei_dir: doppelte ID {pk.get('id')}")
            ids.add(pk.get("id"))
        for bi in b.get("bilder", []):
            if not bi.get("datei") or not bi.get("text"):
                fehler.append(f"bei_dir {b.get('id')}: Bild braucht datei und text")
            elif not (BILDER / bi["datei"]).exists():
                fehler.append(f"bei_dir {b.get('id')}: Bild fehlt: docs/hub/bilder/{bi['datei']}")
    for k, a in (s.get("antworten") or {}).items():
        if k not in ids:
            fehler.append(f"antworten: '{k}' gehört zu keiner Karte/keinem Punkt mehr — beim Einarbeiten nach docs/hub/antworten-archiv.md verschieben")
        if a.get("status") not in ("neu", "gelesen", "eingearbeitet"):
            fehler.append(f"antworten {k}: Status '{a.get('status')}' unbekannt")
        if a.get("status") == "eingearbeitet" and not a.get("ergebnis"):
            fehler.append(f"antworten {k}: eingearbeitet ohne ergebnis")
    for u in s.get("uploads", []):
        for k in ("datum", "name", "datei", "status"):
            if not u.get(k):
                fehler.append(f"upload {u.get('name','?')}: {k} leer")
        if u.get("datei") and not (REPO / u["datei"]).exists():
            fehler.append(f"upload {u.get('name')}: Datei fehlt: {u['datei']}")
    zeilen = 0
    gates = 0
    naechste = 0
    for p in s["plan"]:
        for k in ("phase", "titel", "ziel", "zeilen"):
            if not p.get(k):
                fehler.append(f"plan {p.get('phase','?')}: {k} leer")
        for r in p.get("zeilen", []):
            zeilen += 1
            if r.get("ticket") == "Gate":
                gates += 1
            if r.get("status") not in STATUS_ERLAUBT:
                fehler.append(f"plan {r.get('id')}: Status '{r.get('status')}' unbekannt")
            if r.get("status") == "naechster":
                naechste += 1
            if r.get("status") == "erledigt" and not r.get("beleg"):
                fehler.append(f"plan {r.get('id')}: erledigt ohne Beleg")
            if r.get("status") == "erledigt" and not r.get("datum"):
                fehler.append(f"plan {r.get('id')}: erledigt ohne Datum")
            beleg = r.get("beleg") or ""
            if beleg and not (REPO / beleg).exists():
                fehler.append(f"plan {r.get('id')}: Beleg fehlt auf der Platte: {beleg}")
    if naechste != 1:
        fehler.append(f"genau eine Plan-Zeile muss 'naechster' sein (gefunden {naechste})")
    apps = {a.get("key") for a in s["design"].get("apps", [])}
    if apps != set(APPS):
        fehler.append(f"design.apps müssen genau {APPS} sein, sind {sorted(apps)}")
    for r in s["reviews"]:
        for k in ("datum", "quelle", "ziel", "status", "kurz", "datei"):
            if not r.get(k):
                fehler.append(f"review {r.get('datum','?')}: {k} leer")
    nb = sum(len(b.get("bilder", [])) for b in s["bei_dir"])
    npk = sum(len(b.get("punkte", [])) for b in s["bei_dir"])
    ant = s.get("antworten") or {}
    nneu = sum(1 for a in ant.values() if a.get("status") == "neu")
    print(f"Plan-Zeilen: {zeilen}  Gates: {gates}  Bei-dir: {len(s['bei_dir'])} (mit {nb} Bildern, {npk} Unterpunkten)  Antworten: {len(ant)} ({nneu} neu)  Reviews: {len(s['reviews'])}  Uploads: {len(s.get('uploads', []))}  Eingang: {len(s['eingang'])}")
    return fehler


def neuester_stand(app: str) -> Path | None:
    if not TRUHE.is_dir():
        return None
    kandidaten = sorted(TRUHE.glob(f"*-{app}.png"))
    return kandidaten[-1] if kandidaten else None


def auf_inhalt_zuschneiden(im, rand: int = 96):
    """Schneidet den einfarbigen Host-Grund weg (Suna-Export: Kachel 1040 px in
    3244 px Grund). Grundfarbe = häufigste Farbe des Bildrands; alles, was davon
    abweicht (auch der Schwebeschatten), bleibt samt `rand` Pixeln Luft."""
    from collections import Counter
    from PIL import ImageChops, Image

    w, h = im.size
    px = im.load()
    rand_pixel = [px[x, 0] for x in range(w)] + [px[x, h - 1] for x in range(w)] +                  [px[0, y] for y in range(h)] + [px[w - 1, y] for y in range(h)]
    grund = Counter(rand_pixel).most_common(1)[0][0]
    rgb = im.convert("RGB")
    diff = ImageChops.difference(rgb, Image.new("RGB", im.size, grund[:3])).convert("L")
    diff = diff.point(lambda v: 255 if v > 16 else 0)  # 16: Depot-Exporte tragen einen weichen Schatten über dem ganzen Grund (≤ 10)
    alpha = im.getchannel("A").point(lambda v: 255 if v > 0 else 0)
    box = ImageChops.multiply(diff, alpha).getbbox()  # durchsichtig zählt als Grund
    if not box:
        return im
    l, o, r, u = box
    return im.crop((max(0, l - rand), max(0, o - rand), min(w, r + rand), min(h, u + rand)))


def bild_einbetten(pfad: Path) -> dict:
    from PIL import Image  # Pillow 12 (geprüft 22.08.2026)

    with Image.open(pfad) as im:
        im = im.convert("RGBA")
        im = auf_inhalt_zuschneiden(im)
        w, h = im.size
        ziel = (w // 2, h // 2)  # Exporte sind 4×, die Seite zeigt 2×
        im = im.resize(ziel, Image.LANCZOS)
        buf = io.BytesIO()
        try:
            im.save(buf, format="WEBP", quality=88, method=6)
            mime = "image/webp"
        except Exception:  # Pillow ohne WebP — PNG als Rückfall
            buf = io.BytesIO()
            im.save(buf, format="PNG", optimize=True)
            mime = "image/png"
    daten = buf.getvalue()
    m = re.match(r"(\d{4}-\d{2}-\d{2})-", pfad.name)
    return {
        "src": f"data:{mime};base64," + base64.b64encode(daten).decode("ascii"),
        "w": ziel[0],
        "h": ziel[1],
        "bytes": len(daten),
        "datei": pfad.name,
        "datum": m.group(1) if m else "",
    }


def zeigebild(pfad: Path) -> dict:
    """Karten-/Upload-Bild: auf Zeigebreite verkleinert, kleine Bilder verlustfrei."""
    from PIL import Image

    with Image.open(pfad) as im:
        im = im.convert("RGBA")
        if im.width > MAX_ZEIGEBREITE:
            im = im.resize((MAX_ZEIGEBREITE, round(im.height * MAX_ZEIGEBREITE / im.width)), Image.LANCZOS)
        buf = io.BytesIO()
        try:
            if im.width < 400:
                im.save(buf, format="WEBP", lossless=True)
            else:
                im.save(buf, format="WEBP", quality=85, method=6)
            mime = "image/webp"
        except Exception:
            buf = io.BytesIO()
            im.save(buf, format="PNG", optimize=True)
            mime = "image/png"
        w, h = im.size
    daten = buf.getvalue()
    return {"src": f"data:{mime};base64," + base64.b64encode(daten).decode("ascii"), "w": w, "h": h, "bytes": len(daten)}


def git_kurz() -> str:
    try:
        return subprocess.run(["git", "-C", str(REPO), "rev-parse", "--short", "HEAD"],
                              capture_output=True, text=True, check=True).stdout.strip()
    except Exception:
        return ""


def json_in_script(o) -> str:
    return json.dumps(o, ensure_ascii=False, separators=(",", ":")).replace("</", "<\\/")


def baue(s: dict) -> None:
    bilder: dict[str, dict] = {}
    for app in APPS:
        p = neuester_stand(app)
        if p is None:
            print(f"  {app:8s} kein Export in {TRUHE}")
            bilder[app] = {}
            continue
        b = bild_einbetten(p)
        bilder[app] = b
        print(f"  {app:8s} {p.name}  {b['w']}×{b['h']}  {b['bytes']/1024:.0f} KB")
    gesamt = 0
    for b in s["bei_dir"]:
        for bi in b.get("bilder", []):
            z = zeigebild(BILDER / bi["datei"])
            gesamt += z["bytes"]
            bilder[f"karte:{bi['datei']}"] = z
    for u in s.get("uploads", []):
        if (REPO / u["datei"]).suffix.lower() in (".png", ".jpg", ".jpeg", ".webp", ".gif"):
            z = zeigebild(REPO / u["datei"])
            gesamt += z["bytes"]
            bilder[f"upload:{u['datei']}"] = z
    print(f"  Karten-/Upload-Bilder: {gesamt/1024:.0f} KB")
    s = dict(s)
    s["commit"] = git_kurz()
    s["gebaut_am"] = dt.datetime.now().strftime("%d.%m.%Y %H:%M")
    s["eingang"] = list(s.get("eingang") or [])
    s["antworten"] = dict(s.get("antworten") or {})
    vorlage = VORLAGE.read_text(encoding="utf-8")
    html = vorlage.replace("{{STATE_JSON}}", json_in_script(s)).replace("{{BILDER_JSON}}", json_in_script(bilder))
    HUB_HTML.write_text(html, encoding="utf-8", newline="\n")
    groesse = HUB_HTML.stat().st_size
    print(f"Geschrieben: {HUB_HTML}  ({groesse/1024/1024:.2f} MB, Kappe 16 MB)")
    if groesse > MAX_BYTES:
        print("FEHLER: Seite zu groß für ein Artefakt", file=sys.stderr)
        sys.exit(2)
    url = s.get("artefakt_url") or "<noch keine URL — erste Veröffentlichung ohne url, danach die URL in hub.json eintragen>"
    print("\nJetzt veröffentlichen (nur Claude kann das):")
    print(f"  Artifact(file_path='{HUB_HTML}', url='{url}', favicon='<Kompass-Emoji, wie bisher>', capabilities={{'artifact': {{}}}})")
    print("  Danach: hub.json per Pathspec committen und pushen.")


def main() -> int:
    for strom in (sys.stdout, sys.stderr):  # Windows-Konsole ist cp1252
        try:
            strom.reconfigure(encoding="utf-8", errors="replace")
        except Exception:
            pass
    ap = argparse.ArgumentParser()
    ap.add_argument("--pruefen", action="store_true", help="nur hub.json prüfen")
    a = ap.parse_args()
    s = lade()
    fehler = pruefe(s)
    if fehler:
        for f in fehler:
            print("FEHLER:", f, file=sys.stderr)
        return 1
    if a.pruefen:
        print("hub.json in Ordnung.")
        return 0
    baue(s)
    return 0


if __name__ == "__main__":
    sys.exit(main())
