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
#
# 🔑 "gebaut" ist NICHT "abgenommen". Bis 23.08. zaehlte dieses Blatt beides in
# denselben Topf — damit stand S12-13 als gruenes ■ im Blatt, waehrend sein
# T2-Urteil NEEDS_WORK lautete. Ein Blatt, das ein offenes Urteil als fertig
# malt, behauptet Fortschritt; genau das verbietet die Projektregel. Die beiden
# Zustaende haben deshalb eigene Zeichen und eigene Zahlen, und die Kopfzahl
# nennt die abgenommene zuerst.
ABGENOMMEN = {"erledigt", "fertig", "abgeschlossen"}
GEBAUT = {"gebaut"}
LAEUFT = {"laeuft", "läuft", "in arbeit", "begonnen", "offen_teilweise", "naechster", "nächster"}


def klasse(status: str) -> str:
    s = (status or "").strip().lower()
    if s in ABGENOMMEN:
        return "abgenommen"
    if s in GEBAUT:
        return "gebaut"
    if s in LAEUFT:
        return "laeuft"
    return "offen"


ZEICHEN = {"abgenommen": "■", "gebaut": "▣", "laeuft": "▨", "offen": "□"}
WORT = {"abgenommen": "abgenommen", "gebaut": "gebaut, Urteil offen",
        "laeuft": "läuft", "offen": "offen"}


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


def balken(abgenommen: int, gebaut: int, gesamt: int, breite: int = 24) -> str:
    """Zwei Toene, nicht einer: voll = abgenommen, halb = gebaut ohne Urteil.
    Der Balken darf nicht mehr Gruen zeigen, als ein Pruefer bestaetigt hat."""
    if gesamt <= 0:
        return "—"
    voll = round(breite * abgenommen / gesamt)
    mittel = max(0, round(breite * (abgenommen + gebaut) / gesamt) - voll)
    return "█" * voll + "▓" * mittel + "░" * max(0, breite - voll - mittel)


def zaehle(zeilen: list) -> tuple[int, int]:
    """(abgenommen, gebaut) — beide getrennt, nie summiert."""
    a = sum(1 for z in zeilen if klasse(z.get("status", "")) == "abgenommen")
    g = sum(1 for z in zeilen if klasse(z.get("status", "")) == "gebaut")
    return a, g


def main() -> int:
    try:
        daten = json.loads(QUELLE.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as e:
        print(f"Quelle nicht lesbar: {QUELLE} ({e})", file=sys.stderr)
        return 3

    phasen = daten.get("plan") or []
    zeilen_gesamt = [z for p in phasen for z in (p.get("zeilen") or [])]
    n_ges = len(zeilen_gesamt)
    n_abg, n_geb = zaehle(zeilen_gesamt)
    n_rest = n_ges - n_abg - n_geb

    aus: list[str] = []
    aus.append("# Planstand Nakama")
    aus.append("")
    aus.append("> **Erzeugt, nicht gepflegt.** Quelle ist `docs/hub/hub.json`;")
    aus.append("> dieses Blatt entsteht daraus mit `py -3.13 tools/hub/plan_blatt.py`.")
    aus.append("> Aenderungen hier gehen beim naechsten Lauf verloren.")
    aus.append("> ⚠️ Die Quelle wird von Hand gepflegt — das Blatt ist nur so frisch")
    aus.append("> wie der letzte Eintrag in `hub.json`, nicht so frisch wie das Repo.")
    aus.append("")
    aus.append(f"**Stand:** {daten.get('stand', '?')} · "
               f"**{n_abg} von {n_ges} Schritten abgenommen** · "
               f"{n_geb} gebaut, Urteil offen · {n_rest} offen")
    aus.append("")
    aus.append(f"`{balken(n_abg, n_geb, n_ges, 40)}` "
               f"{round(100 * n_abg / n_ges) if n_ges else 0} % abgenommen, "
               f"{round(100 * (n_abg + n_geb) / n_ges) if n_ges else 0} % gebaut")
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
    aus.append("| Phase | Fortschritt | abgenommen | gebaut, Urteil offen | offen |")
    aus.append("|---|---|---:|---:|---:|")
    for p in phasen:
        zl = p.get("zeilen") or []
        a, g = zaehle(zl)
        aus.append(f"| **{p.get('phase', '?')}** — {p.get('titel', '')} "
                   f"| `{balken(a, g, len(zl))}` | {a} | {g} | {len(zl) - a - g} |")
    aus.append("")

    # ── Mermaid: der Weg als Kette ─────────────────────────────────────────
    aus.append("## Der Weg")
    aus.append("")
    aus.append("```mermaid")
    aus.append("flowchart LR")
    for i, p in enumerate(phasen):
        zl = p.get("zeilen") or []
        a, g = zaehle(zl)
        name = str(p.get("phase", f"P{i}")).replace('"', "'")
        # Gruen erst, wenn ALLE Zeilen der Phase abgenommen sind — ein offenes
        # Urteil faerbt die Phase gelb, nicht gruen.
        stil = "fertig" if zl and a == len(zl) else ("laeuft" if a or g else "offen")
        zusatz = f"<br/>+{g} gebaut" if g else ""
        aus.append(f'  P{i}["{name}<br/>{a}/{len(zl)} abgenommen{zusatz}"]:::{stil}')
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
        a, g = zaehle(zl)
        kopf = f"{a}/{len(zl)} abgenommen" + (f", {g} gebaut" if g else "")
        aus.append(f"### {p.get('phase', '?')} — {p.get('titel', '')}  ({kopf})")
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
            datum = f" · {datum}" if datum and k in ("abgenommen", "gebaut") else ""
            aus.append(f"- {ZEICHEN[k]} **{z.get('id', '?')}**{tick} — "
                       f"{z.get('text', '')} ({WORT[k]}{datum})")
        aus.append("")

    aus.append("---")
    aus.append("")
    aus.append("**■ abgenommen** — gebaut, gemessen UND von einem frischen Pruefer "
               "bestaetigt (T2/T3-Urteil PASS).")
    aus.append("**▣ gebaut, Urteil offen** — es gibt ein Beweismanifest in "
               "`docs/beweise/`, aber kein PASS: der Pruefer steht aus oder hat "
               "NEEDS_WORK gesagt. Zaehlt nicht als fertig.")
    aus.append("**▨ läuft** · **□ offen** — noch kein Manifest.")
    aus.append("")

    ZIEL.write_text("\n".join(aus), encoding="utf-8")
    print(f"geschrieben: {ZIEL.relative_to(WURZEL)} "
          f"({n_abg} abgenommen, {n_geb} gebaut ohne Urteil, {n_ges} gesamt, "
          f"{len(phasen)} Phasen)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
