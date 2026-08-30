#!/usr/bin/env python3
"""Rundenbilanz - woran hat eine Nacharbeitsrunde wirklich gearbeitet?

Lehre aus S8 SONDE-007a (30.08.2026): zwoelf Runden in Folge aenderten null
Zeilen Produktcode, nur Pruefskripte und Manifeste - und niemand hat es
gemessen. Dieses Werkzeug misst es. Der Dirigent ruft es nach JEDER Runde auf
(Skill §3.3) und schreibt die Zeile in den Dirigentenstand.

Aufruf:
    py -3.13 tools/dirigent/rundenbilanz.py <basis>..<head> [<basis2>..<head2> ...]
    py -3.13 tools/dirigent/rundenbilanz.py --runden <sha0> <sha1> <sha2> ...
        (jedes Paar aufeinanderfolgender SHAs ist eine Runde)

Klassen (nach Pfad, nicht nach Inhalt):
    Produkt        eq-copilot/plugin, eq-copilot/cmake, eq-copilot/schemas,
                   eq-copilot/identity, eq-copilot/install, eq-copilot/fixtures,
                   broker/src, broker/Cargo.*, third_party
    Tests          Pfadteil "tests"/"test" oder Dateiname mit "Test" unter
                   Produktwurzeln
    Pruefwerkzeug  tools/**
    Doku           docs/**, design/**, wissen/**, *.md an der Wurzel,
                   .claude/**, .agents/**
    Sonstiges      alles andere

Signal: hat eine Runde Produkt+Tests = 0 Zeilen, gilt sie als "ohne
Produktfortschritt". Zwei solche Runden in Folge loesen den Konvergenzentscheid
aus (Skill §3.4) - vor dem Rundenbudget, nicht erst danach.

Exit 0 immer; das Werkzeug urteilt nicht, es zeigt.
"""
from __future__ import annotations

import pathlib
import subprocess
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]

PRODUKT_WURZELN = (
    "eq-copilot/plugin/", "eq-copilot/cmake/", "eq-copilot/schemas/",
    "eq-copilot/identity/", "eq-copilot/install/", "eq-copilot/fixtures/",
    "broker/src/", "third_party/",
)
DOKU_WURZELN = ("docs/", "design/", "wissen/", ".claude/", ".agents/")


def klasse(pfad: str) -> str:
    p = pfad.replace(chr(92), "/")
    teile = p.split("/")
    name = teile[-1]
    if p.startswith("tools/"):
        return "Pruefwerkzeug"
    if p.startswith(DOKU_WURZELN) or (len(teile) == 1 and name.lower().endswith(".md")):
        return "Doku"
    if p.startswith(PRODUKT_WURZELN) or p.startswith("broker/Cargo"):
        if any(t.lower() in ("tests", "test") for t in teile[:-1]) or "Test" in name:
            return "Tests"
        return "Produkt"
    if p.startswith("broker/tests/"):
        return "Tests"
    return "Sonstiges"


def numstat(bereich: str) -> list[tuple[int, int, str]]:
    lauf = subprocess.run(
        ["git", "--no-optional-locks", "-C", str(WURZEL), "diff", "--numstat", bereich],
        capture_output=True, text=True, errors="replace", check=False)
    if lauf.returncode != 0:
        print(f"git diff {bereich} fehlgeschlagen: {lauf.stderr.strip()}", file=sys.stderr)
        return []
    zeilen = []
    for z in lauf.stdout.splitlines():
        teile = z.split(chr(9))
        if len(teile) != 3:
            continue
        plus = int(teile[0]) if teile[0].isdigit() else 0
        minus = int(teile[1]) if teile[1].isdigit() else 0
        zeilen.append((plus, minus, teile[2]))
    return zeilen


KLASSEN = ("Produkt", "Tests", "Pruefwerkzeug", "Doku", "Sonstiges")


def bilanz(bereich: str) -> dict[str, tuple[int, int, int]]:
    summe = {k: [0, 0, 0] for k in KLASSEN}
    for plus, minus, pfad in numstat(bereich):
        k = klasse(pfad)
        summe[k][0] += 1
        summe[k][1] += plus
        summe[k][2] += minus
    return {k: tuple(v) for k, v in summe.items()}


def zeile(bereich: str, b: dict[str, tuple[int, int, int]]) -> str:
    teile = []
    for k in KLASSEN:
        dateien, plus, minus = b[k]
        if dateien:
            teile.append(f"{k} {dateien} Datei(en) +{plus}/-{minus}")
    if not teile:
        teile.append("keine Aenderung")
    return f"{bereich}: " + " | ".join(teile)


def main() -> int:
    args = sys.argv[1:]
    if not args:
        print(__doc__)
        return 0
    bereiche: list[str] = []
    if args[0] == "--runden":
        shas = args[1:]
        if len(shas) < 2:
            print("--runden braucht mindestens zwei SHAs", file=sys.stderr)
            return 0
        bereiche = [f"{a}..{b}" for a, b in zip(shas, shas[1:])]
    else:
        bereiche = args

    ohne_fortschritt_in_folge = 0
    schlimmste_folge = 0
    for bereich in bereiche:
        b = bilanz(bereich)
        produkt = b["Produkt"][1] + b["Produkt"][2] + b["Tests"][1] + b["Tests"][2]
        print(zeile(bereich, b))
        if produkt == 0:
            ohne_fortschritt_in_folge += 1
            print("    -> OHNE PRODUKTFORTSCHRITT (Produkt+Tests = 0 Zeilen)")
        else:
            ohne_fortschritt_in_folge = 0
        schlimmste_folge = max(schlimmste_folge, ohne_fortschritt_in_folge)

    if len(bereiche) > 1:
        print()
        if schlimmste_folge >= 2:
            print(f"KONVERGENZ-SIGNAL: {schlimmste_folge} Runden in Folge ohne "
                  f"Produktfortschritt - Konvergenzentscheid nach Skill §3.4, "
                  f"keine weitere Punktkorrektur-Runde.")
        else:
            print("kein Konvergenz-Signal (maximal eine Runde in Folge ohne Produktfortschritt)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
