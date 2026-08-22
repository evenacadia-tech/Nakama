#!/usr/bin/env python3
"""Erzeugt das Planblatt fuer Nimbalyst aus docs/hub/hub.json.

EINE Wahrheit, ein Ort: hub.json ist die Quelle, dieses Blatt ist NUR eine
Ansicht. Es wird nie von Hand editiert — jeder Lauf ueberschreibt es
vollstaendig. Wer den Planstand aendern will, aendert hub.json (und schickt
ihn mit hub_sync.py an die Briefing-Seite).

Aufruf vom Repo-Root:
    py -3.13 tools/hub/plan_blatt.py

Schreibt docs/PLAN-STAND.md — Nimbalyst rendert das Mermaid-Diagramm darin
live. Exitcode 0 = geschrieben, 3 = Quelle fehlt/unlesbar.
"""
from __future__ import annotations

import json
import pathlib
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
QUELLE = WURZEL / "docs" / "hub" / "hub.json"
ZIEL = WURZEL / "docs" / "PLAN-STAND.md"

# hub.json fuehrt Status als Klartext. Alles, was hier nicht steht, gilt als
# offen — lieber zu wenig gruen melden als Fortschritt behaupten, den es
# nicht gibt (Projektregel: Fortschritt nur mit Beleg).
FERTIG = {"erledigt", "fertig", "gebaut", "abgeschlossen"}
LAEUFT = {"laeuft", "läuft", "in arbeit", "begonnen", "offen_teilweise"}


def klasse(status: str) -> str:
    s = (status or "").strip().lower()
    if s in FERTIG:
        return "fertig"
    if s in LAEUFT:
        return "laeuft"
    return "offen"


ZEICHEN = {"fertig": "■", "laeuft": "▨", "offen": "□"}
WORT = {"fertig": "fertig", "laeuft": "läuft", "offen": "offen"}


def alsText(wert) -> str:
    """hub.json fuehrt manche Felder mal als Text, mal als Objekt. Nie raten:
    bekannte Textfelder in sinnvoller Reihenfolge nehmen, sonst leer lassen —
    lieber nichts anzeigen als eine rohe Datenstruktur ins Blatt kippen."""
    if isinstance(wert, str):
        return wert.strip()
    if isinstance(wert, dict):
        for k in ("text", "titel", "kurz", "beschreibung", "id"):
            v = wert.get(k)
            if isinstance(v, str) and v.strip():
                rest = wert.get("text") if k != "text" else None
                if k != "text" and isinstance(rest, str) and rest.strip():
                    return f"{v.strip()} — {rest.strip()}"
                return v.strip()
    if isinstance(wert, list):
        teile = [alsText(x) for x in wert]
        return " · ".join(t for t in teile if t)
    return ""


def balken(fertig: int, gesamt: int, breite: int = 24) -> str:
    if gesamt <= 0:
        return "—"
    voll = round(breite * fertig / gesamt)
    return "█" * voll + "░" * (breite - voll)


def main() -> int:
    try:
        daten = json.loads(QUELLE.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as e:
        print(f"Quelle nicht lesbar: {QUELLE} ({e})", file=sys.stderr)
        return 3

    phasen = daten.get("plan") or []
    zeilen_gesamt = [z for p in phasen for z in (p.get("zeilen") or [])]
    n_ges = len(zeilen_gesamt)
    n_fertig = sum(1 for z in zeilen_gesamt if klasse(z.get("status", "")) == "fertig")

    aus: list[str] = []
    aus.append("# Planstand Nakama")
    aus.append("")
    aus.append("> **Erzeugt, nicht gepflegt.** Quelle ist `docs/hub/hub.json`;")
    aus.append("> dieses Blatt entsteht daraus mit `py -3.13 tools/hub/plan_blatt.py`.")
    aus.append("> Aenderungen hier gehen beim naechsten Lauf verloren.")
    aus.append("")
    aus.append(f"**Stand:** {daten.get('stand', '?')} · "
               f"**{n_fertig} von {n_ges} Schritten fertig**")
    aus.append("")
    aus.append(f"`{balken(n_fertig, n_ges, 40)}` {round(100 * n_fertig / n_ges) if n_ges else 0} %")
    aus.append("")

    schritt = alsText(daten.get("naechster_schritt"))
    if schritt:
        aus.append(f"**Als Naechstes:** {schritt}")
        aus.append("")

    bei_dir = daten.get("bei_dir") or []
    if bei_dir:
        aus.append(f"**Bei dir liegen:** {len(bei_dir)} Punkt(e) — "
                   f"Details auf der Briefing-Seite ({daten.get('hub_url', '')})")
        aus.append("")

    # ── Uebersicht je Phase ────────────────────────────────────────────────
    aus.append("## Phasen auf einen Blick")
    aus.append("")
    aus.append("| Phase | Fortschritt | fertig | offen |")
    aus.append("|---|---|---:|---:|")
    for p in phasen:
        zl = p.get("zeilen") or []
        f = sum(1 for z in zl if klasse(z.get("status", "")) == "fertig")
        aus.append(f"| **{p.get('phase', '?')}** — {p.get('titel', '')} "
                   f"| `{balken(f, len(zl))}` | {f} | {len(zl) - f} |")
    aus.append("")

    # ── Mermaid: der Weg als Kette ─────────────────────────────────────────
    aus.append("## Der Weg")
    aus.append("")
    aus.append("```mermaid")
    aus.append("flowchart LR")
    for i, p in enumerate(phasen):
        zl = p.get("zeilen") or []
        f = sum(1 for z in zl if klasse(z.get("status", "")) == "fertig")
        name = str(p.get("phase", f"P{i}")).replace('"', "'")
        stil = "fertig" if zl and f == len(zl) else ("laeuft" if f else "offen")
        aus.append(f'  P{i}["{name}<br/>{f}/{len(zl)}"]:::{stil}')
        if i:
            aus.append(f"  P{i-1} --> P{i}")
    aus.append("  classDef fertig fill:#1f6f43,stroke:#2ea36a,color:#eaf6ef")
    aus.append("  classDef laeuft fill:#7a5a12,stroke:#c99a2e,color:#fdf6e6")
    aus.append("  classDef offen  fill:#2a2f36,stroke:#4a525c,color:#c7ced8")
    aus.append("```")
    aus.append("")

    # ── Alle Schritte ──────────────────────────────────────────────────────
    aus.append("## Alle Schritte")
    aus.append("")
    for p in phasen:
        zl = p.get("zeilen") or []
        f = sum(1 for z in zl if klasse(z.get("status", "")) == "fertig")
        aus.append(f"### {p.get('phase', '?')} — {p.get('titel', '')}  ({f}/{len(zl)})")
        ziel = (p.get("ziel") or "").strip()
        if ziel:
            aus.append("")
            aus.append(f"*{ziel}*")
        aus.append("")
        for z in zl:
            k = klasse(z.get("status", ""))
            tick = z.get("ticket") or ""
            tick = f" `{tick}`" if tick else ""
            datum = z.get("datum") or ""
            datum = f" · {datum}" if datum and k == "fertig" else ""
            aus.append(f"- {ZEICHEN[k]} **{z.get('id', '?')}**{tick} — "
                       f"{z.get('text', '')} ({WORT[k]}{datum})")
        aus.append("")

    aus.append("---")
    aus.append("")
    aus.append("■ fertig · ▨ läuft · □ offen — „fertig\" heisst in diesem Projekt: "
               "es gibt ein Beweismanifest in `docs/beweise/`.")
    aus.append("")

    ZIEL.write_text("\n".join(aus), encoding="utf-8")
    print(f"geschrieben: {ZIEL.relative_to(WURZEL)} "
          f"({n_fertig}/{n_ges} fertig, {len(phasen)} Phasen)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
