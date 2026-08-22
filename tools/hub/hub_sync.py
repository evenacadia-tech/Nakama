#!/usr/bin/env python3
"""Nakama-Briefing: docs/hub/hub.json <-> https://nakama-briefing.philipld.chatgpt.site

    py -3.13 tools/hub/hub_sync.py holen     # GET /api/hub: Antworten + neue Punkte -> hub.json
    py -3.13 tools/hub/hub_sync.py pruefen   # hub.json gegen die Regeln pruefen, nichts senden
    py -3.13 tools/hub/hub_sync.py senden    # pruefen, dann POST /api/state (Autor Claude), dann GET-Gegenprobe

Seit 22.08.2026 ist die Seite der einzige Hub (User: „das ist der neue und
einzige hub, alle anderen artefakte sind hiermit nichtmehr zu beachten").
Quelle des Projektstands bleibt docs/hub/hub.json; die Seite zeigt ihn
(GET /api/hub -> `hub`) und haelt dauerhaft, was dort entsteht: Antworten des
Users (`answers`, je Frage-ID) und neue Punkte (`items`, von Phil, Claude oder
Codex ueber „Neuen Punkt anlegen" bzw. POST /api/items).

Ablauf je Session: holen -> Antworten/Punkte einarbeiten (User-Wort mit Datum
und Wortlaut ins Register bzw. in die Abnahmen, Status `eingearbeitet` +
`ergebnis`) -> hub.json fortschreiben -> senden -> hub.json per Pathspec
committen. Quelle der Adresse: `hub_url` in hub.json (Umgebungsvariable
NAKAMA_HUB_URL ueberschreibt, z. B. fuer einen lokalen `vinext dev`).
"""
from __future__ import annotations

import datetime as dt
import json
import os
import re
import sys
import urllib.error
import urllib.request
from pathlib import Path

HIER = Path(__file__).resolve().parent
REPO = HIER.parent.parent
HUB_JSON = REPO / "docs" / "hub" / "hub.json"
BILDER = REPO / "docs" / "hub" / "bilder"
APPS = ("gen", "probeeq", "suna")
STATUS_ERLAUBT = {"erledigt", "gebaut", "naechster", "bei dir", "wartet", "offen"}
DRINGLICHKEIT_ERLAUBT = {"jetzt", "wenn du dazu kommst", "wissen", "später"}
ANTWORT_STATUS = ("neu", "gelesen", "eingearbeitet")
AUTOR = "Claude"
MAX_STATE_BYTES = 500_000  # Grenze der Seite (app/api/state/route.ts)
TIMEOUT = 30


def lade() -> dict:
    return json.loads(HUB_JSON.read_text(encoding="utf-8"))


def speichere(hub: dict) -> None:
    HUB_JSON.write_text(json.dumps(hub, ensure_ascii=False, indent=2) + "\n", encoding="utf-8", newline="\n")


def hub_url(hub: dict) -> str:
    url = os.environ.get("NAKAMA_HUB_URL") or hub.get("hub_url") or ""
    if not url:
        raise SystemExit("hub.json hat keine hub_url (und NAKAMA_HUB_URL ist nicht gesetzt).")
    return url.rstrip("/")


def http(url: str, daten: dict | None = None) -> dict:
    body = None
    # Eigener User-Agent ist Pflicht: Cloudflare vor der Seite antwortet auf
    # "Python-urllib/3.x" mit 403 (gemessen 22.08.2026).
    kopf = {"Accept": "application/json", "User-Agent": "nakama-hub-sync/1 (Claude)"}
    if daten is not None:
        body = json.dumps(daten, ensure_ascii=False).encode("utf-8")
        kopf["Content-Type"] = "application/json; charset=utf-8"
    anfrage = urllib.request.Request(url, data=body, headers=kopf, method="POST" if body else "GET")
    try:
        with urllib.request.urlopen(anfrage, timeout=TIMEOUT) as antwort:
            return json.loads(antwort.read().decode("utf-8"))
    except urllib.error.HTTPError as e:
        text = e.read().decode("utf-8", errors="replace")[:400]
        raise SystemExit(f"HTTP {e.code} von {url}: {text}")
    except urllib.error.URLError as e:
        raise SystemExit(f"Seite nicht erreichbar ({url}): {e.reason}")


# ---------------------------------------------------------------- pruefen
def pruefe(s: dict) -> list[str]:
    fehler: list[str] = []
    for k in ("stand", "stand_notiz", "hub_url", "naechster_schritt", "bei_dir", "plan", "design", "reviews", "eingang"):
        if k not in s:
            fehler.append(f"Schlüssel fehlt: {k}")
    if fehler:
        return fehler
    if "artefakt_url" in s:
        fehler.append("artefakt_url ist tot (User 22.08.: alle anderen Artefakte nicht mehr beachten) — Schlüssel entfernen")
    if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", s["stand"]):
        fehler.append("stand muss JJJJ-MM-TT sein")
    if not re.match(r"https://", s["hub_url"]):
        fehler.append("hub_url muss mit https:// beginnen")
    for k in ("technik", "design", "dich"):
        if not s["naechster_schritt"].get(k):
            fehler.append(f"naechster_schritt.{k} leer")
    ids: set[str] = set()
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
        if k not in ids and not k.startswith("item."):
            fehler.append(f"antworten: '{k}' gehört zu keiner Karte/keinem Punkt mehr — beim Einarbeiten nach docs/hub/antworten-archiv.md verschieben")
        if a.get("status") not in ANTWORT_STATUS:
            fehler.append(f"antworten {k}: Status '{a.get('status')}' unbekannt")
        if a.get("status") == "eingearbeitet" and not a.get("ergebnis"):
            fehler.append(f"antworten {k}: eingearbeitet ohne ergebnis")
    for e in s["eingang"]:
        if e.get("typ") == "punkt" and not (e.get("id") and e.get("titel") and e.get("autor")):
            fehler.append(f"eingang: Punkt ohne id/titel/autor: {e}")
    for u in s.get("uploads", []):
        for k in ("datum", "name", "datei", "status"):
            if not u.get(k):
                fehler.append(f"upload {u.get('name','?')}: {k} leer")
        if u.get("datei") and not (REPO / u["datei"]).exists():
            fehler.append(f"upload {u.get('name')}: Datei fehlt: {u['datei']}")
    zeilen = gates = naechste = 0
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
    groesse = len(json.dumps(s, ensure_ascii=False).encode("utf-8"))
    if groesse > MAX_STATE_BYTES:
        fehler.append(f"hub.json ist {groesse} Bytes — die Seite nimmt höchstens {MAX_STATE_BYTES} an")
    nb = sum(len(b.get("bilder", [])) for b in s["bei_dir"])
    npk = sum(len(b.get("punkte", [])) for b in s["bei_dir"])
    ant = s.get("antworten") or {}
    nneu = sum(1 for a in ant.values() if a.get("status") == "neu")
    print(f"Plan-Zeilen: {zeilen}  Gates: {gates}  Bei-dir: {len(s['bei_dir'])} (mit {nb} Bildern, {npk} Unterpunkten)  "
          f"Antworten: {len(ant)} ({nneu} neu)  Reviews: {len(s['reviews'])}  Uploads: {len(s.get('uploads', []))}  "
          f"Eingang: {len(s['eingang'])}  Größe: {groesse/1024:.0f} KB")
    return fehler


# ---------------------------------------------------------------- holen
def datum_aus(ms) -> str:
    try:
        return dt.datetime.fromtimestamp(int(ms) / 1000).strftime("%Y-%m-%d %H:%M")
    except Exception:
        return ""


def antworten_holen(seite: dict, hub: dict) -> int:
    """answers der Seite -> hub.json antworten. Neu oder geändert ⇒ Status 'neu'."""
    alt = hub.setdefault("antworten", {})
    n = 0
    for qid, a in (seite.get("answers") or {}).items():
        if a.get("author") and a["author"] != "Phil":
            continue  # nur User-Wort ist einzuarbeiten; Claude/Codex-Antworten sind keine Entscheide
        datum = datum_aus(a.get("updatedAt"))
        vorher = alt.get(qid)
        if vorher and vorher.get("wahl") == a.get("choice") and vorher.get("text") == (a.get("note") or "") and vorher.get("datum") == datum:
            continue
        alt[qid] = {"wahl": a.get("choice") or "", "text": a.get("note") or "", "datum": datum, "status": "neu", "ergebnis": ""}
        n += 1
        print(f"  Antwort {qid}: [{a.get('choice') or '—'}] {(a.get('note') or '').strip()[:160]!r}  ({datum})")
    return n


def punkte_holen(seite: dict, hub: dict) -> int:
    """items der Seite (neue Entscheidungen/Updates/Blocker) -> hub.json eingang, idempotent je id."""
    eingang = hub.setdefault("eingang", [])
    bekannt = {e.get("id") for e in eingang if e.get("typ") == "punkt"}
    bekannt |= {e.get("id") for e in hub.get("punkte_erledigt", [])}
    n = 0
    for it in seite.get("items") or []:
        pid = f"item.{it.get('id')}"
        if pid in bekannt:
            continue
        eingang.append({
            "typ": "punkt", "id": pid, "art": it.get("kind") or "", "titel": it.get("title") or "",
            "text": it.get("summary") or "", "warum": it.get("reason") or "", "dringlichkeit": it.get("urgency") or "",
            "autor": it.get("author") or "", "status_seite": it.get("status") or "", "datum": datum_aus(it.get("createdAt")),
        })
        n += 1
        print(f"  Punkt {pid} ({it.get('kind')}, {it.get('author')}): {it.get('title')!r}")
    return n


def holen(hub: dict) -> int:
    url = hub_url(hub)
    seite = http(url + "/api/hub")
    if seite.get("warning"):
        print("WARNUNG von der Seite:", seite["warning"])
    print(f"Seite: Stand {seite.get('hub', {}).get('stand')} · zuletzt gesetzt von {seite.get('stateUpdatedBy')} "
          f"({datum_aus(seite.get('stateUpdatedAt')) or 'Seed der Seite, noch kein /api/state'})")
    n_ant = antworten_holen(seite, hub)
    n_pkt = punkte_holen(seite, hub)
    if n_ant or n_pkt:
        speichere(hub)
        print(f"{n_ant} Antwort(en), {n_pkt} Punkt(e) neu → hub.json. Jede User-Antwort ist User-Wort: mit Datum + Wortlaut "
              "ins Register/die Abnahmen, dann Status 'eingearbeitet' + ergebnis; Punkte aus dem Eingang in Karten oder Plan überführen.")
    else:
        print("Nichts Neues auf der Seite.")
    return 0


# ---------------------------------------------------------------- senden
def senden(hub: dict) -> int:
    url = hub_url(hub)
    live = http(url + "/api/hub")
    fremd = {k: v for k, v in (live.get("answers") or {}).items()
             if v.get("author", "Phil") == "Phil" and k not in (hub.get("antworten") or {})}
    if fremd:
        print(f"ABBRUCH: {len(fremd)} Antwort(en) auf der Seite fehlen in hub.json ({', '.join(sorted(fremd))}) — erst `holen`, dann einarbeiten, dann senden.")
        return 2
    ergebnis = http(url + "/api/state", {"author": AUTOR, "hub": hub})
    if not ergebnis.get("ok"):
        print("FEHLER: Seite hat den Stand nicht angenommen:", ergebnis)
        return 1
    probe = http(url + "/api/hub")
    if probe.get("hub") != hub:
        print("FEHLER: Gegenprobe — GET /api/hub liefert nicht den gesendeten Stand.")
        return 1
    print(f"Gesendet und gegengeprüft: Stand {hub['stand']} steht auf {url} (gesetzt von {probe.get('stateUpdatedBy')}, "
          f"{datum_aus(probe.get('stateUpdatedAt'))}). Jetzt hub.json per Pathspec committen.")
    return 0


def main(argv: list[str]) -> int:
    for strom in (sys.stdout, sys.stderr):  # Windows-Konsole ist cp1252
        try:
            strom.reconfigure(encoding="utf-8", errors="replace")
        except Exception:
            pass
    if len(argv) != 2 or argv[1] not in ("holen", "pruefen", "senden"):
        print(__doc__)
        return 2
    hub = lade()
    if argv[1] == "holen":
        return holen(hub)
    fehler = pruefe(hub)
    if fehler:
        for f in fehler:
            print("FEHLER:", f, file=sys.stderr)
        return 1
    if argv[1] == "pruefen":
        print("hub.json in Ordnung.")
        return 0
    return senden(hub)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
