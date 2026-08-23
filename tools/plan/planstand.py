#!/usr/bin/env python3
"""Rechnet den Planstand aus dem Repo und schreibt docs/PLAN-STAND.md.

WARUM ES DIESES SKRIPT GIBT (User 23.08.2026): der Planstand stand bis dahin
als `status`-Text in docs/hub/hub.json und wurde von Hand umgestellt. Wer es
vergass, hinterliess einen still falschen Stand — und nichts schlug an. Der
User: „es gibt keine automatische aktualisation , das heisst er wird driften
und somit drifted plan stand auch."

Also wird der Status jetzt GEMESSEN, nach derselben Regel wie im Beweis-Runner:

  Was gibt es?     docs/plan/plan.json   — autorierter Text, KEIN Statusfeld
  Gebaut?          liegt der Beleg (docs/beweise/<Ticket>.md)?
  Abgenommen?      Urteilsmarke im Manifest, auf der GEFORDERTEN Pruefstufe
  Wie frisch?      Stempel im Blatt gegen HEAD

Die Marke ist das einzige Stueck Status, das ein Mensch schreibt — weil ein
Urteil kein Messwert ist:

  <!-- NAKAMA-URTEIL: T2 PASS 2026-08-22 -->
  <!-- NAKAMA-URTEIL: T2 NEEDS_WORK 2026-08-23 nachgearbeitet -->

🔑 Fail-closed an zwei Stellen. Erstens zaehlt nur PASS auf der geforderten
Stufe (oder hoeher) als abgenommen — fehlende Marke, T1 (Selbstaudit des
Erbauers) oder NEEDS_WORK heissen „gebaut, Urteil offen". Vergessen
untertreibt damit, statt zu uebertreiben. Zweitens wird eine Marke, die
NAKAMA-URTEIL sagt aber nicht parst, NICHT still uebergangen, sondern als
Warnung ins Blatt geschrieben: ein Tippfehler darf keinen Schritt unsichtbar
herabstufen.

Aufruf vom Repo-Root:  py -3.13 tools/plan/planstand.py
Exit 0 = geschrieben · 3 = Quelle fehlt/unlesbar · 4 = Marke unlesbar.
"""
from __future__ import annotations

import datetime
import json
import pathlib
import re
import subprocess
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
PLAN = WURZEL / "docs" / "plan" / "plan.json"
FRAGEN = WURZEL / "docs" / "plan" / "fragen.json"
ZIEL = WURZEL / "docs" / "PLAN-STAND.md"

# Alles, woraus dieses Blatt gerechnet wird. Aendert sich hier nichts, kann
# sich am Planstand nichts geaendert haben — darauf beruht der Auslöser des
# Hooks (tools/hooks/planstand.sh) UND die Schleifenfreiheit des Stempels.
QUELLEN = ("docs/plan", "docs/beweise", "tools/plan")

# Die Marke. Das vierte Wort ist optional und sagt bei NEEDS_WORK, ob die
# Befunde geschlossen sind — daraus faellt die Antwort „was ist als Naechstes
# dran": ein OFFENER Befund ist Arbeit, ein nachgearbeiteter wartet nur auf
# einen frischen Pruefer.
MARKE = re.compile(
    r"<!--\s*NAKAMA-URTEIL:\s*(T[123])\s+(PASS|NEEDS_WORK)\s+"
    r"(\d{4}-\d{2}-\d{2})(?:\s+(offen|nachgearbeitet))?\s*-->"
)
MARKE_ROH = re.compile(r"<!--[^>]*NAKAMA-URTEIL[^>]*-->")
# Die Bilanzzeile, die tools/beweise.ps1 selbst schreibt — eine gemessene Zahl,
# keine abgeschriebene. Die Trennzeichen wechseln ueber die Manifeste hinweg
# (·, |, -, —), deshalb bewusst lose gefasst.
KANON = re.compile(r"Urteil:\*\*\s*(GRUEN|ROT)\s*[-—]+\s*(\d+)/(\d+)\s*Kanon")

RANG = {"T1": 1, "T2": 2, "T3": 3}

ZEICHEN = {"abgenommen": "■", "gebaut": "▣", "offen": "□"}
WORT = {"abgenommen": "abgenommen", "gebaut": "gebaut", "offen": "offen"}


def git(*args: str) -> str:
    try:
        return subprocess.run(["git", "-C", str(WURZEL), *args],
                              capture_output=True, text=True, timeout=20).stdout.strip()
    except (OSError, subprocess.SubprocessError):
        return ""


def marken_lesen(pfad: pathlib.Path, warnungen: list[str]) -> list[dict]:
    """Alle Marken einer Datei, in Dateireihenfolge. Spaeter schlaegt frueher:
    eine neue Pruefrunde wird unten angehaengt und soll gelten."""
    try:
        text = pfad.read_text(encoding="utf-8", errors="replace")
    except OSError as e:
        warnungen.append(f"{pfad.name}: nicht lesbar ({e})")
        return []
    gut = [
        {"stufe": m.group(1), "urteil": m.group(2), "datum": m.group(3),
         "befunde": m.group(4) or ""}
        for m in MARKE.finditer(text)
    ]
    # Gegenprobe: steht irgendwo NAKAMA-URTEIL, das die strenge Form NICHT
    # trifft? Dann ist die Marke kaputt und der Schritt faellt still zurueck —
    # genau das darf nicht unbemerkt passieren.
    if len(MARKE_ROH.findall(text)) != len(gut):
        warnungen.append(
            f"{pfad.name}: eine NAKAMA-URTEIL-Zeile ist unlesbar "
            f"({len(MARKE_ROH.findall(text))} gefunden, {len(gut)} lesbar) — "
            f"Form: <!-- NAKAMA-URTEIL: T2 PASS 2026-08-23 [offen|nachgearbeitet] -->"
        )
    return gut


def kanon_lesen(pfad: pathlib.Path) -> str:
    try:
        text = pfad.read_text(encoding="utf-8", errors="replace")
    except OSError:
        return ""
    treffer = KANON.findall(text)
    if not treffer:
        return ""
    farbe, n, ges = treffer[-1]           # der juengste Lauf im Manifest
    return f"Kanon {n}/{ges} {'grün' if farbe == 'GRUEN' else 'ROT'}"


def messen(schritt: dict, warnungen: list[str]) -> dict:
    """Der ganze Status eines Schrittes — gemessen, nichts uebernommen."""
    erg = dict(schritt)
    erg.update(status="offen", urteil=None, kanon="", hinweis="")

    beleg = (schritt.get("beleg") or "").strip()
    if not beleg:
        return erg
    pfad = WURZEL / beleg
    if not pfad.exists():
        erg["hinweis"] = f"Beleg `{beleg}` fehlt"
        return erg

    # User-Termin: es gibt keine Pruefstufe, die Rohdaten SIND das Ergebnis.
    if schritt.get("stufe") is None:
        if pfad.is_dir() and not any(pfad.iterdir()):
            erg["hinweis"] = f"`{beleg}` ist leer"
            return erg
        erg["status"] = "abgenommen"
        erg["hinweis"] = "gemessen (Rohdaten)"
        return erg

    erg["status"] = "gebaut"
    if pfad.is_dir():
        return erg
    erg["kanon"] = kanon_lesen(pfad)

    marken = marken_lesen(pfad, warnungen)
    verlangt = RANG.get(schritt.get("stufe") or "", 2)
    # Nur Marken, die mindestens so hoch sind wie verlangt, koennen abnehmen.
    # Die juengste davon entscheidet.
    passend = [m for m in marken if RANG[m["stufe"]] >= verlangt]
    if passend:
        m = passend[-1]
        erg["urteil"] = m
        if m["urteil"] == "PASS":
            erg["status"] = "abgenommen"
        else:
            erg["hinweis"] = ("Befund offen" if m["befunde"] == "offen"
                              else "nachgearbeitet, frisches Urteil fehlt"
                              if m["befunde"] == "nachgearbeitet" else "NEEDS_WORK")
    elif marken:
        tiefste = marken[-1]
        erg["urteil"] = tiefste
        erg["hinweis"] = (f"nur {tiefste['stufe']} belegt, "
                          f"{schritt['stufe']} verlangt")
    else:
        erg["hinweis"] = f"{schritt['stufe']} steht aus"
    return erg


def balken(abg: int, geb: int, ges: int, breite: int = 24) -> str:
    if ges <= 0:
        return "—"
    voll = round(breite * abg / ges)
    mittel = max(0, round(breite * (abg + geb) / ges) - voll)
    return "█" * voll + "▓" * mittel + "░" * max(0, breite - voll - mittel)


def main() -> int:
    try:
        plan = json.loads(PLAN.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as e:
        print(f"Quelle nicht lesbar: {PLAN} ({e})", file=sys.stderr)
        return 3
    try:
        offene_fragen = json.loads(FRAGEN.read_text(encoding="utf-8")).get("offen", [])
    except (OSError, json.JSONDecodeError):
        offene_fragen = []          # Fragen sind Beiwerk; ihr Fehlen darf das Blatt nicht kippen

    warnungen: list[str] = []
    phasen = []
    for ph in plan.get("phasen", []):
        phasen.append({**ph, "schritte": [messen(s, warnungen)
                                          for s in ph.get("schritte", [])]})
    alle = [s for p in phasen for s in p["schritte"]]
    n_ges = len(alle)
    n_abg = sum(1 for s in alle if s["status"] == "abgenommen")
    n_geb = sum(1 for s in alle if s["status"] == "gebaut")

    # Was ist als Naechstes dran? Abgeleitet, nicht getippt: ein OFFENER Befund
    # ist Arbeit und geht vor; sonst der erste Schritt, an dem noch nichts liegt.
    nacharbeit = [s for s in alle if s["hinweis"] == "Befund offen"]
    unbelegt = [s for s in alle if s["status"] == "offen" and not s["hinweis"]]
    if nacharbeit:
        s = nacharbeit[0]
        naechster = (f"**Nacharbeit an {s['id']}** — der Prüfer hat einen Befund "
                     f"offen gelassen ({s['beleg']}).")
    elif unbelegt:
        s = unbelegt[0]
        naechster = f"**{s['id']}**" + (f" `{s['ticket']}`" if s["ticket"] else "") + f" — {s['text']}"
    else:
        naechster = "nichts Offenes im Plan."
    wartend = [s for s in alle
               if s["hinweis"] == "nachgearbeitet, frisches Urteil fehlt"
               or s["hinweis"].endswith("steht aus")]

    # 🔑 Gestempelt wird der QUELLSTAND — der letzte Commit, der eine Quelle
    # angefasst hat —, NICHT HEAD. Mit HEAD baute sich eine Endlosschleife:
    # der Commit dieses Blattes aendert HEAD, beim naechsten Befehl wiche der
    # Stempel wieder ab, es wuerde neu gerechnet und neu committet, ewig. Der
    # Blatt-Commit beruehrt keine Quelle, also steht der Quellstand still und
    # die Schleife schliesst sich von selbst.
    sha = git("log", "-1", "--format=%h", "--",
              *QUELLEN) or git("rev-parse", "--short", "HEAD") or "?"
    heute = datetime.date.today().isoformat()
    # Gerechnet wird aus dem ARBEITSBAUM, gestempelt wird ein Commit. Solange
    # beides deckungsgleich ist, stimmt der Stempel; sobald eine Quelle
    # uncommittet ist, stimmt er nicht mehr — und das muss dranstehen, sonst
    # behauptet das Blatt eine Herkunft, die es nicht hat.
    schmutzig = bool(git("status", "--porcelain", "--", *QUELLEN))

    a: list[str] = []
    a.append("# Planstand Nakama")
    a.append("")
    a.append(f"<!-- quellstand: {sha} -->")
    a.append("")
    a.append("> **Gerechnet, nicht gepflegt.** Dieses Blatt entsteht aus dem Repo:")
    a.append("> `py -3.13 tools/plan/planstand.py`. Es wird **nie** von Hand editiert —")
    a.append("> jeder Lauf überschreibt es. Was hier steht, ist gemessen:")
    a.append("> ein Schritt gilt als *gebaut*, wenn sein Beweismanifest liegt, und als")
    a.append("> *abgenommen* erst, wenn dort eine Urteilsmarke der geforderten Prüfstufe")
    a.append("> mit **PASS** steht. Fehlt sie, gilt der Schritt als nicht abgenommen.")
    a.append("")
    a.append(f"**Stand:** {heute} · Quellstand `{sha}` · "
             f"**{n_abg} von {n_ges} abgenommen** · {n_geb} gebaut · "
             f"{n_ges - n_abg - n_geb} offen")
    a.append("")
    if schmutzig:
        a.append("> ⚠️ Gerechnet aus dem Arbeitsbaum: unter `docs/plan/`, `docs/beweise/`")
        a.append(f"> oder `tools/plan/` liegen Änderungen, die noch nicht in `{sha}` sind.")
        a.append("")
    a.append(f"`{balken(n_abg, n_geb, n_ges, 40)}` "
             f"{round(100 * n_abg / n_ges) if n_ges else 0} % abgenommen · "
             f"{round(100 * (n_abg + n_geb) / n_ges) if n_ges else 0} % gebaut")
    a.append("")
    a.append(f"**Als Nächstes:** {naechster}")
    a.append("")
    if wartend:
        a.append("**Wartet auf ein Urteil** (gebaut, nachgemessen, aber ohne PASS eines "
                 "frischen Prüfers): " + " · ".join(f"`{s['id']}`" for s in wartend))
        a.append("")
    if offene_fragen:
        a.append(f"**Bei dir liegen {len(offene_fragen)} Fragen** — "
                 f"`{', '.join(f.get('id', '?') for f in offene_fragen)}`. "
                 f"Sie werden im Chat gestellt: Skill `/fragen`.")
        a.append("")
    if warnungen:
        a.append("> ⚠️ **Unlesbare Urteilsmarken** — diese Schritte werden vorsichtshalber")
        a.append("> als *nicht abgenommen* geführt:")
        for w in warnungen:
            a.append(f"> - {w}")
        a.append("")

    a.append("## Phasen auf einen Blick")
    a.append("")
    a.append("| Phase | Fortschritt | abgenommen | gebaut | offen |")
    a.append("|---|---|---:|---:|---:|")
    for p in phasen:
        sl = p["schritte"]
        ab = sum(1 for s in sl if s["status"] == "abgenommen")
        gb = sum(1 for s in sl if s["status"] == "gebaut")
        a.append(f"| **{p.get('phase','?')}** — {p.get('titel','')} "
                 f"| `{balken(ab, gb, len(sl))}` | {ab} | {gb} | {len(sl)-ab-gb} |")
    a.append("")

    a.append("## Der Weg")
    a.append("")
    a.append("```mermaid")
    a.append("flowchart LR")
    for i, p in enumerate(phasen):
        sl = p["schritte"]
        ab = sum(1 for s in sl if s["status"] == "abgenommen")
        gb = sum(1 for s in sl if s["status"] == "gebaut")
        name = str(p.get("phase", f"P{i}")).replace('"', "'")
        stil = "fertig" if sl and ab == len(sl) else ("laeuft" if ab or gb else "offen")
        zusatz = f"<br/>+{gb} gebaut" if gb else ""
        a.append(f'  P{i}["{name}<br/>{ab}/{len(sl)} abgenommen{zusatz}"]:::{stil}')
        if i:
            a.append(f"  P{i-1} --> P{i}")
    a.append("  classDef fertig fill:#1f6f43,stroke:#2ea36a,color:#eaf6ef")
    a.append("  classDef laeuft fill:#7a5a12,stroke:#c99a2e,color:#fdf6e6")
    a.append("  classDef offen  fill:#2a2f36,stroke:#4a525c,color:#c7ced8")
    a.append("```")
    a.append("")

    a.append("## Alle Schritte")
    a.append("")
    for p in phasen:
        sl = p["schritte"]
        ab = sum(1 for s in sl if s["status"] == "abgenommen")
        gb = sum(1 for s in sl if s["status"] == "gebaut")
        kopf = f"{ab}/{len(sl)} abgenommen" + (f", {gb} gebaut" if gb else "")
        a.append(f"### {p.get('phase','?')} — {p.get('titel','')}  ({kopf})")
        if (p.get("ziel") or "").strip():
            a.append("")
            a.append(f"*{p['ziel'].strip()}*")
        a.append("")
        for s in sl:
            teile = [WORT[s["status"]]]
            if s["urteil"]:
                u = s["urteil"]
                teile.append(f"{u['stufe']} {u['urteil']} {u['datum']}")
            if s["hinweis"]:
                teile.append(s["hinweis"])
            if s["kanon"]:
                teile.append(s["kanon"])
            tick = f" `{s['ticket']}`" if s.get("ticket") else ""
            a.append(f"- {ZEICHEN[s['status']]} **{s['id']}**{tick} — {s.get('text','')} "
                     f"({' · '.join(teile)})")
        a.append("")

    a.append("---")
    a.append("")
    a.append("**■ abgenommen** — Beweismanifest liegt UND ein Prüfer der geforderten "
             "Stufe hat **PASS** gegeben.")
    a.append("**▣ gebaut** — Manifest liegt, Prüfungen sind gefahren, aber es gibt kein "
             "PASS: der Prüfer steht aus oder hat NEEDS_WORK gesagt. Zählt nicht als fertig.")
    a.append("**□ offen** — noch kein Beleg.")
    a.append("")
    a.append("Quelle des Textes: `docs/plan/plan.json` · Quelle des Status: die "
             "Urteilsmarken in `docs/beweise/` · Fragen an dich: `docs/plan/fragen.json`.")
    a.append("")

    ZIEL.write_text("\n".join(a), encoding="utf-8")
    print(f"geschrieben: {ZIEL.relative_to(WURZEL)} "
          f"({n_abg} abgenommen, {n_geb} gebaut, {n_ges} gesamt, aus {sha})")
    for w in warnungen:
        print(f"  WARNUNG: {w}", file=sys.stderr)
    return 4 if warnungen else 0


if __name__ == "__main__":
    sys.exit(main())
