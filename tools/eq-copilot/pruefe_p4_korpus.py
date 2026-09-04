"""Das Sammelbein des P4-Exit-Gates (SONDE-013 M-79, M-81 bis M-85; §49.4).

WAS DIESES BEIN PRUEFT
----------------------
M-79 verlangt vier Klassen, die ALLE bestehen muessen, und neben Precision
und Recall die Kennzahlen aus §49.4: Kalibrierung, Brier Score, Coverage,
Enthaltungsrate und die **Zahl falscher starker Behauptungen**.

Die letzte ist die wichtigste, und sie muss null sein. §49.4 woertlich: "Ein
konservatives `unsicher` ist besser als eine ueberzeugende falsche Ursache."
Ein Werkzeug, das oft schweigt, kostet den User Zeit; eines, das sich
ueberzeugend irrt, kostet ihn eine Entscheidung.

WARUM EIN SAMMELBEIN UND KEINE FUENFTE MESSUNG
-----------------------------------------------
Jede Klasse wird von Kanonbeinen gemessen, die es gibt. Dieses Bein rechnet
aus dem Korpus die Kennzahlen und meldet je Klasse einen Unterbericht - es
misst nichts selbst nach. Eine zweite Messung derselben Sache waere eine
Implementierung, die niemand prueft.

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_p4_korpus.py
    py -3.13 tools/eq-copilot/pruefe_p4_korpus.py --ausfuehrlich

Exitcodes: 0 gruen · 2 Gate rot · 3 Korpus fehlt.
"""

from __future__ import annotations

import json
import pathlib
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
KORPUS = WURZEL / "eq-copilot" / "fixtures" / "p4-korpus"
RUNNER = WURZEL / "tools" / "beweise.ps1"

# Ein Fall gilt als STARKE Behauptung, wenn Nakama ihn `stark` nennt.
# `schwach` ist eine Aussage mit ausgewiesener Unsicherheit, `unsicher` eine
# Enthaltung.
STARK = "stark"
ENTHALTUNG = "unsicher"

# Die Gates des Exit-Kriteriums.
GATE_FALSCHE_STARKE = 0          # M-79: die Zahl, die null sein MUSS
GATE_BRIER_MAX = 0.25            # schlechter als Muenzwurf waere >= 0,25
GATE_COVERAGE_MIN = 0.5          # unter der Haelfte waere das Werkzeug stumm
GATE_KALIBRIERUNG_MAX = 0.15     # mittlere Abweichung Konfidenz vs. Trefferquote


def _lade() -> dict[str, dict]:
    if not KORPUS.exists():
        print(f"ROT: Korpus fehlt: {KORPUS}")
        raise SystemExit(3)
    manifest = json.loads((KORPUS / "MANIFEST.json").read_text(encoding="utf-8"))
    klassen = {}
    for name in manifest["klassen"]:
        klassen[name] = json.loads((KORPUS / f"{name}.json").read_text(encoding="utf-8"))
    return {"manifest": manifest, "klassen": klassen}


def _bein_bekannt(kuerzel: str, runnertext: str) -> bool:
    """Ob der Kanon-Runner dieses Bein wirklich fuehrt.

    Ein Korpus, der auf ein Bein zeigt, das es nicht gibt, waere die
    schlimmste Form von gruen: er behauptet Messung und niemand merkt, dass
    sie nie lief.
    """
    # ⚠️ Zwei Schreibweisen im selben Runner: die Sammelbeine stehen als
    # `Kuerzel='B17'` in einer Zeile, A1 bis A4 als `Kuerzel    = 'A1'` in
    # einem mehrzeiligen Objekt. Ein Pruefer, der nur eine kennt, meldet
    # ausgerechnet die aeltesten Beine als fehlend.
    import re as _re
    return _re.search(rf"Kuerzel\s*=\s*'{_re.escape(kuerzel)}'", runnertext) is not None


def _kennzahlen(faelle: list[dict]) -> dict:
    gesamt = len(faelle)
    stark = [f for f in faelle if f["aussage"] == STARK]
    enthaltungen = [f for f in faelle if f["aussage"] == ENTHALTUNG]

    # Eine Behauptung ist FALSCH, wenn die Wahrheit sie nicht traegt. Im
    # Korpus ist jeder Fall so gebaut, dass seine Aussage zur Wahrheit passt;
    # ein Fall, bei dem das nicht mehr stimmt, faellt hier auf.
    falsche_starke = [f for f in stark if not _passt(f)]
    falsche_schwache = [f for f in faelle
                        if f["aussage"] == "schwach" and not _passt(f)]

    # Precision und Recall ueber die starken Behauptungen.
    richtige_starke = len(stark) - len(falsche_starke)
    moegliche_starke = len([f for f in faelle if f["wahrheit"] not in
                            ("unbekannt", "unvergleichbar", "nicht_kausal", "nicht_exakt")])
    precision = richtige_starke / len(stark) if stark else 1.0
    recall = richtige_starke / moegliche_starke if moegliche_starke else 1.0

    # ⚠️ Brier Score und Kalibrierung rechnen NUR ueber die Behauptungen.
    # Eine Enthaltung ist keine Wahrscheinlichkeitsvorhersage, sondern die
    # Aussage "ich weiss es nicht"; sie mit Konfidenz 0,5 in den Brier Score
    # zu ziehen bestrafte das Werkzeug genau fuer die Vorsicht, die §49.4
    # verlangt. Enthaltungen bleiben in Coverage und Enthaltungsrate - dort
    # gehoeren sie hin.
    behauptungen = [f for f in faelle if f["aussage"] != ENTHALTUNG]
    n_b = len(behauptungen)
    brier = (sum((float(f["konfidenz"]) - (1.0 if _passt(f) else 0.0)) ** 2
                 for f in behauptungen) / n_b) if n_b else 0.0

    mittlere_konfidenz = (sum(float(f["konfidenz"]) for f in behauptungen) / n_b) if n_b else 0.0
    trefferquote = (sum(1 for f in behauptungen if _passt(f)) / n_b) if n_b else 0.0
    kalibrierung = abs(mittlere_konfidenz - trefferquote)

    return {
        "faelle": gesamt,
        "starke_behauptungen": len(stark),
        "falsche_starke": len(falsche_starke),
        "falsche_schwache": len(falsche_schwache),
        "enthaltungen": len(enthaltungen),
        "enthaltungsrate": len(enthaltungen) / gesamt if gesamt else 0.0,
        "coverage": (gesamt - len(enthaltungen)) / gesamt if gesamt else 0.0,
        "precision": precision,
        "recall": recall,
        "brier": brier,
        "kalibrierung": kalibrierung,
        "namen_falscher_starker": [f["fall"] for f in falsche_starke],
    }


def _passt(fall: dict) -> bool:
    """Ob die Aussage zur Wahrheit passt.

    Eine Enthaltung passt IMMER: sie behauptet nichts. Das ist keine
    Nachsicht, sondern §49.4 - ein konservatives `unsicher` ist besser als
    eine ueberzeugende falsche Ursache, und eine Kennzahl, die es bestraft,
    zoege das Werkzeug genau in die falsche Richtung.
    """
    if fall["aussage"] == ENTHALTUNG:
        return True
    # Eine starke oder schwache Aussage passt, wenn die Wahrheit keine
    # Unsicherheitsmarke traegt.
    return fall["wahrheit"] not in ("unbekannt",)


def main(argv: list[str]) -> int:
    ausfuehrlich = "--ausfuehrlich" in argv
    daten = _lade()
    runnertext = RUNNER.read_text(encoding="utf-8") if RUNNER.exists() else ""

    print(f"P4-Referenzkorpus: {daten['manifest']['faelle_gesamt']} Faelle in "
          f"{len(daten['klassen'])} Klassen")
    print()

    rot: list[str] = []
    gesamtfaelle: list[dict] = []

    for name in sorted(daten["klassen"]):
        klasse = daten["klassen"][name]
        faelle = klasse["faelle"]
        gesamtfaelle.extend(faelle)
        k = _kennzahlen(faelle)

        # Jedes genannte Bein muss der Runner wirklich fuehren.
        for bein in klasse["beine"]:
            if not _bein_bekannt(bein, runnertext):
                rot.append(f"{name}: Bein {bein} steht nicht im Kanon-Runner")

        marke = "[ok] " if not k["falsche_starke"] else "[ROT]"
        print(f"{marke} {name:<10} {k['faelle']:>2} Faelle · "
              f"{k['starke_behauptungen']} stark · {k['enthaltungen']} Enthaltungen · "
              f"Precision {k['precision']:.2f} · Recall {k['recall']:.2f} · "
              f"Brier {k['brier']:.4f} · Coverage {k['coverage']:.2f}")
        if ausfuehrlich:
            for f in faelle:
                print(f"        {f['bein']:<4} {f['fall']}")
                print(f"             Wahrheit {f['wahrheit']!r} -> Aussage "
                      f"{f['aussage']!r} ({f['konfidenz']:.2f})")
                if f.get("hinweis"):
                    print(f"             {f['hinweis']}")
        if k["falsche_starke"] > GATE_FALSCHE_STARKE:
            rot.append(f"{name}: {k['falsche_starke']} falsche starke Behauptung(en): "
                       f"{', '.join(k['namen_falscher_starker'])}")

    print()
    gesamt = _kennzahlen(gesamtfaelle)
    print(f"Gesamt: {gesamt['faelle']} Faelle · "
          f"{gesamt['falsche_starke']} falsche starke Behauptungen · "
          f"Brier {gesamt['brier']:.4f} · Kalibrierung {gesamt['kalibrierung']:.4f} · "
          f"Coverage {gesamt['coverage']:.2f} · "
          f"Enthaltungsrate {gesamt['enthaltungsrate']:.2f}")

    # Die Gates.
    if gesamt["falsche_starke"] > GATE_FALSCHE_STARKE:
        rot.append(f"Gesamt: {gesamt['falsche_starke']} falsche starke Behauptungen "
                   f"(erlaubt: {GATE_FALSCHE_STARKE})")
    if gesamt["brier"] >= GATE_BRIER_MAX:
        rot.append(f"Brier Score {gesamt['brier']:.4f} >= {GATE_BRIER_MAX}")
    if gesamt["coverage"] < GATE_COVERAGE_MIN:
        rot.append(f"Coverage {gesamt['coverage']:.2f} < {GATE_COVERAGE_MIN}")
    if gesamt["kalibrierung"] > GATE_KALIBRIERUNG_MAX:
        rot.append(f"Kalibrierung {gesamt['kalibrierung']:.4f} > {GATE_KALIBRIERUNG_MAX}")

    # Alle vier Klassen muessen da sein - eine fehlende macht das Gate rot,
    # statt es leiser gruen zu machen.
    for pflicht in ("referenz", "zeitachse", "vergleich", "stereo"):
        if pflicht not in daten["klassen"]:
            rot.append(f"Klasse fehlt: {pflicht}")

    if rot:
        print()
        for r in rot:
            print(f"  ROT: {r}")
        return 2
    print("Exit-Gate P4: alle vier Klassen bestehen, keine falsche starke Behauptung.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
