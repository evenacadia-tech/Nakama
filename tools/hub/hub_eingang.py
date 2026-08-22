#!/usr/bin/env python3
"""Holt hochgeladene Review-Dateien aus einer gelesenen Hub-Seite ins Repo.

    py -3.13 tools/hub/hub_eingang.py <gelesene-hub-seite.html>

Ablauf für Claude (Sessionstart, wenn der Hub-Primer einen Eingang meldet
oder man es nicht weiß):
  1. Artifact(action='read', url=<artefakt_url aus hub.json>) → Datei auf der Platte
  2. dieses Skript auf die Datei → schreibt docs/reviews/JJJJ-MM-TT-<name>.md und
     trägt je Datei eine Zeile „eingegangen – noch nicht geprüft" in hub.json ein
  3. jeden Befund gegen die Quelldatei prüfen (Regel aus der Bauaufteilung §2 T3:
     bestätigt ⇒ gefixt oder NAK-Zeile · widerlegt ⇒ eine Zeile warum · nichts
     verschwindet still), Status in hub.json fortschreiben
  4. baue_hub.py → Artifact publish mit url → hub.json + docs/reviews/ committen

Idempotent: eine Datei, die schon bytegleich liegt, wird nicht erneut geschrieben
und nicht erneut eingetragen.
"""
from __future__ import annotations

import json
import re
import sys
from html import unescape
from pathlib import Path

HIER = Path(__file__).resolve().parent
REPO = HIER.parent.parent
HUB_JSON = REPO / "docs" / "hub" / "hub.json"
REVIEWS = REPO / "docs" / "reviews"


def eingang_aus(html: str) -> list[dict]:
    m = re.search(r'<script type="application/json" id="hub-state">(.*?)</script>', html, re.S)
    if not m:
        raise SystemExit("Kein #hub-state in der Datei — ist das die gelesene Hub-Seite?")
    roh = m.group(1)
    # Die Seite maskiert '</' als '<\/' — für JSON ist das ein gültiger Escape.
    state = json.loads(roh)
    return list(state.get("eingang") or [])


def sicherer_name(name: str) -> str:
    stamm = re.sub(r"\.(md|markdown|txt)$", "", name, flags=re.I)
    stamm = re.sub(r"[^A-Za-z0-9äöüÄÖÜß._-]+", "-", stamm).strip("-.") or "review"
    return stamm[:80]


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
    eintraege = eingang_aus(html)
    if not eintraege:
        print("Eingang leer — nichts zu holen.")
        return 0
    REVIEWS.mkdir(parents=True, exist_ok=True)
    hub = json.loads(HUB_JSON.read_text(encoding="utf-8"))
    bekannt = {r.get("datei") for r in hub.get("reviews", [])}
    neu = 0
    for e in eintraege:
        datum = (e.get("datum") or "")[:10] or "0000-00-00"
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
                "kurz": f"{len(text)} Zeichen hochgeladen am {e.get('datum') or datum}. Jeden Befund gegen die Quelldatei prüfen, bevor gehandelt wird.",
                "datei": rel,
            })
        neu += 1
        print(f"  geholt: {rel}")
    hub["eingang"] = []
    HUB_JSON.write_text(json.dumps(hub, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n")
    print(f"{neu} neu. hub.json: Eingang geleert, Reviews eingetragen. Jetzt prüfen, dann baue_hub.py + veröffentlichen.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
