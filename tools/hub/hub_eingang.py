#!/usr/bin/env python3
"""Holt Antworten und hochgeladene Dateien (Reviews, Bilder) aus einer gelesenen Hub-Seite ins Repo.

    py -3.13 tools/hub/hub_eingang.py <gelesene-hub-seite.html>

Ablauf für Claude (Sessionstart, wenn der Hub-Primer einen Eingang meldet
oder man es nicht weiß):
  1. Artifact(action='read', url=<artefakt_url aus hub.json>) → Datei auf der Platte
  2. dieses Skript auf die Datei → Reviews nach docs/reviews/JJJJ-MM-TT-<name>.md
     (hub.json: `reviews`, Status „eingegangen – noch nicht geprüft"), Bilder
     bytegleich nach docs/hub/eingang/JJJJ-MM-TT-<name> (hub.json: `uploads`,
     Status „eingegangen – noch nicht bearbeitet", mit der Notiz des Users)
  3. jeden Befund gegen die Quelldatei prüfen (Regel aus der Bauaufteilung §2 T3:
     bestätigt ⇒ gefixt oder NAK-Zeile · widerlegt ⇒ eine Zeile warum · nichts
     verschwindet still), Status in hub.json fortschreiben
  4. baue_hub.py → Artifact publish mit url → hub.json + docs/reviews/ committen

Idempotent: eine Datei, die schon bytegleich liegt, wird nicht erneut geschrieben
und nicht erneut eingetragen.
"""
from __future__ import annotations

import base64
import json
import re
import sys
from html import unescape
from pathlib import Path

HIER = Path(__file__).resolve().parent
REPO = HIER.parent.parent
HUB_JSON = REPO / "docs" / "hub" / "hub.json"
REVIEWS = REPO / "docs" / "reviews"
EINGANG = REPO / "docs" / "hub" / "eingang"


def state_aus(html: str) -> dict:
    m = re.search(r'<script type="application/json" id="hub-state">(.*?)</script>', html, re.S)
    if not m:
        raise SystemExit("Kein #hub-state in der Datei — ist das die gelesene Hub-Seite?")
    # Die Seite maskiert '</' als '<\/' — für JSON ist das ein gültiger Escape.
    return json.loads(m.group(1))


def antworten_holen(seite: dict, hub: dict) -> int:
    """Antworten des Users von der Seite nach hub.json — neu oder geändert ⇒ Status 'neu'."""
    alt = hub.setdefault("antworten", {})
    neu = 0
    for k, a in (seite.get("antworten") or {}).items():
        vorher = alt.get(k)
        gleich = vorher and vorher.get("wahl") == a.get("wahl") and vorher.get("text") == a.get("text") and vorher.get("datum") == a.get("datum")
        if gleich:
            continue
        alt[k] = {"wahl": a.get("wahl") or "", "text": a.get("text") or "", "datum": a.get("datum") or "", "status": "neu", "ergebnis": ""}
        neu += 1
        print(f"  Antwort {k}: [{a.get('wahl') or '—'}] {(a.get('text') or '').strip()[:160]!r}  ({a.get('datum')})")
    return neu


def sicherer_name(name: str, endung_weg: bool = True) -> str:
    stamm = re.sub(r"\.(md|markdown|txt)$", "", name, flags=re.I) if endung_weg else name
    stamm = re.sub(r"[^A-Za-z0-9äöüÄÖÜß._-]+", "-", stamm).strip("-.") or "datei"
    return stamm[:80]


def bild_bytes(src: str) -> tuple[bytes, str]:
    m = re.match(r"data:(image/(png|jpeg|webp|gif));base64,(.*)$", src, re.S)
    if not m:
        raise SystemExit("Bild-Upload ohne gültige data:-URL")
    endung = {"png": ".png", "jpeg": ".jpg", "webp": ".webp", "gif": ".gif"}[m.group(2)]
    return base64.b64decode(m.group(3)), endung


def main(argv: list[str]) -> int:
    for strom in (sys.stdout, sys.stderr):
        try:
            strom.reconfigure(encoding="utf-8", errors="replace")
        except Exception:
            pass
    if len(argv) != 2:
        print(__doc__)
        return 2
    quelle = Path(argv[1])
    html = quelle.read_text(encoding="utf-8", errors="replace")
    seite = state_aus(html)
    eintraege = list(seite.get("eingang") or [])
    hub = json.loads(HUB_JSON.read_text(encoding="utf-8"))
    n_ant = antworten_holen(seite, hub)
    if n_ant:
        print(f"{n_ant} Antwort(en) neu/geändert → hub.json (Status 'neu'). Jede Antwort ist User-Wort: mit Datum + Wortlaut ins Register/die Abnahmen, dann Status 'eingearbeitet' + ergebnis.")
    if not eintraege:
        if n_ant:
            HUB_JSON.write_text(json.dumps(hub, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n")
        else:
            print("Eingang leer, keine neuen Antworten — nichts zu holen.")
        return 0
    REVIEWS.mkdir(parents=True, exist_ok=True)
    EINGANG.mkdir(parents=True, exist_ok=True)
    bekannt = {r.get("datei") for r in hub.get("reviews", [])} | {u.get("datei") for u in hub.get("uploads", [])}
    neu = 0
    for e in eintraege:
        datum = (e.get("datum") or "")[:10] or "0000-00-00"
        if e.get("typ") == "bild":
            daten, endung = bild_bytes(e.get("src") or "")
            stamm = sicherer_name(e.get("name") or "bild", endung_weg=False)
            if not stamm.lower().endswith(endung):
                stamm = re.sub(r"\.[A-Za-z0-9]+$", "", stamm) + endung
            ziel = EINGANG / f"{datum}-{stamm}"
            n = 2
            while ziel.exists() and ziel.read_bytes() != daten:
                ziel = EINGANG / f"{datum}-{n}-{stamm}"
                n += 1
            rel = ziel.relative_to(REPO).as_posix()
            if ziel.exists() and rel in bekannt:
                print(f"  schon da: {rel}")
                continue
            if not ziel.exists():
                ziel.write_bytes(daten)
            if rel not in bekannt:
                hub.setdefault("uploads", []).insert(0, {
                    "datum": datum,
                    "name": e.get("name") or "",
                    "datei": rel,
                    "notiz": e.get("notiz") or "",
                    "status": "eingegangen – noch nicht bearbeitet",
                    "ergebnis": "",
                })
            neu += 1
            print(f"  geholt (Bild, {len(daten)//1024} KB): {rel}")
            continue
        text = e.get("text") or ""
        basis = f"{datum}-{sicherer_name(e.get('name') or 'review')}"
        ziel = REVIEWS / f"{basis}.md"
        n = 2
        while ziel.exists() and ziel.read_text(encoding="utf-8") != text:
            ziel = REVIEWS / f"{basis}-{n}.md"
            n += 1
        rel = ziel.relative_to(REPO).as_posix()
        if ziel.exists() and rel in bekannt:
            print(f"  schon da: {rel}")
            continue
        if not ziel.exists():
            ziel.write_text(text, encoding="utf-8", newline="\n")
        if rel not in bekannt:
            hub.setdefault("reviews", []).insert(0, {
                "datum": datum,
                "quelle": "Codex (Upload über den Hub)",
                "ziel": e.get("name") or "",
                "status": "eingegangen – noch nicht geprüft",
                "kurz": (f"Notiz: {e['notiz']} — " if e.get("notiz") else "") + f"{len(text)} Zeichen hochgeladen am {e.get('datum') or datum}. Jeden Befund gegen die Quelldatei prüfen, bevor gehandelt wird.",
                "datei": rel,
            })
        neu += 1
        print(f"  geholt: {rel}")
    hub["eingang"] = []
    HUB_JSON.write_text(json.dumps(hub, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n")
    print(f"{neu} neu. hub.json: Eingang geleert, Reviews/Uploads eingetragen. Jetzt bearbeiten (Status + ergebnis je Upload), dann baue_hub.py + veröffentlichen.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
