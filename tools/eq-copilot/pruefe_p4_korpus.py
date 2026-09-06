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

WAS NAK-182 DARAN GEAENDERT HAT
-------------------------------
Drei Befunde des Gates G4 trafen dieses Skript, und alle drei hatten dieselbe
Form: es meldete gruen, wo es haette rot melden muessen.

1. **Zwei Wahrheiten ueber dieselbe Menge (C6, R1a).** `_passt` kannte nur
   `unbekannt`, `moegliche_starke` schloss vier Werte aus. Ein Korpusfall mit
   `wahrheit: unvergleichbar` und `aussage: stark` blieb gruen - genau der
   Gate-6-Verstoss, den das Bein finden soll -, und `recall` konnte dabei
   ueber 1,0 steigen, ohne dass ein Riegel reagierte. Es gibt ab jetzt ZWEI
   benannte Mengen, sie stehen im **Erzeuger** und reisen im **MANIFEST**
   mit, und dieses Skript liest sie VON DORT:

       KEINE_BEHAUPTUNG  jede Aussage darauf ist falsch, auch die schwache
       KEINE_STARKE      = KEINE_BEHAUPTUNG + nicht_kausal, nicht_exakt;
                           nur die STARKE Aussage ist falsch

   Der Unterschied ist keine Feinheit: ein verteiltes Paar liefert
   Banddifferenzen (schwach richtig), nur keinen Frequenzgang (stark falsch);
   eine Insertprobe liefert einen Beitrag, nur keinen exakten
   Mastersummenbeitrag. Wuerde man beides in eine Menge werfen, waere
   entweder Gate 7 im Pruefer blind oder zwei richtige Faelle waeren falsch.

2. **Der Fallname war ein Etikett (C7/L-03, R2).** Geprueft war nur, ob das
   BEIN im Kanon-Runner steht - nie, ob der genannte `fall` irgendwo
   existiert. Von 22 Namen standen 8 woertlich in einem Test. Jeder Fall
   traegt jetzt `quelle` (Datei ab Repo-Wurzel plus woertlicher Bezeichner),
   und dieses Skript sucht den Bezeichner in der Datei.

3. **Die Quotenwachen hatten keinen Weg zu fallen (MP1-6).** `recall > 1` und
   `precision > 1` sind nach R1a strukturell unerreichbar; eine Wache ohne
   ausfuehrbaren Negativtest ist keine gemessene Zusage. Die Riegelauswertung
   ist deshalb eine eigene Funktion `_riegel`, die A26 auf die ECHTEN
   Kennzahlen anwendet und der Selbsttest synthetische Dicts einspeist.

Dazu misst das Bein den zweiten Teil des Entscheids G4 §8: keine der
geschlossenen Mengen des v3-Vertrags benennt einen "sicheren Ausloeser".

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_p4_korpus.py
    py -3.13 tools/eq-copilot/pruefe_p4_korpus.py --ausfuehrlich
    py -3.13 tools/eq-copilot/pruefe_p4_korpus.py --selbsttest     (Bein A27)

Exitcodes: 0 gruen · 2 Gate rot · 3 Korpus fehlt.
"""

from __future__ import annotations

import json
import pathlib
import re
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
KORPUS = WURZEL / "eq-copilot" / "fixtures" / "p4-korpus"
RUNNER = WURZEL / "tools" / "beweise.ps1"
SCHEMA = WURZEL / "eq-copilot" / "schemas" / "v3" / "eq-ipc-v3.schema.json"

# Ein Fall gilt als STARKE Behauptung, wenn Nakama ihn `stark` nennt.
# `schwach` ist eine Aussage mit ausgewiesener Unsicherheit, `unsicher` eine
# Enthaltung.
STARK = "stark"
SCHWACH = "schwach"
ENTHALTUNG = "unsicher"

# Die Gates des Exit-Kriteriums.
GATE_FALSCHE_STARKE = 0          # M-79: die Zahl, die null sein MUSS
GATE_FALSCHE_SCHWACHE = 0        # NAK-182 R1a: dasselbe fuer die schwache Seite
GATE_BRIER_MAX = 0.25            # schlechter als Muenzwurf waere >= 0,25
GATE_COVERAGE_MIN = 0.5          # unter der Haelfte waere das Werkzeug stumm
GATE_KALIBRIERUNG_MAX = 0.15     # mittlere Abweichung Konfidenz vs. Trefferquote
GATE_QUOTE_MAX = 1.0             # Precision und Recall sind Quoten (NAK-182)

# Die fuenf geschlossenen Mengen des v3-Vertrags, an denen der Entscheid
# G4 §8 gemessen wird: kein P4-Datenweg benennt einen "sicheren Ausloeser".
VERTRAGSMENGEN = {
    "urteil (M-46)": ("$defs", "session_experiment", "properties", "urteil"),
    "alignment_klasse": ("$defs", "alignment_klasse"),
    "kettenbefund": ("$defs", "session_paar", "properties", "kettenbefund"),
    "ausschluss": ("$defs", "session_paar", "properties", "ausschluss"),
    "konfidenz.klasse": ("$defs", "konfidenz", "properties", "klasse"),
}

# Geschlossene Liste kausaler Bezeichner. Sie steht hier und nirgends sonst;
# ein Wort mehr ist eine bewusste Erweiterung, kein Zufall.
KAUSALE_WOERTER = ("ursache", "ursaechlich", "ausloeser", "ausgeloest",
                   "verursacht", "kausal", "cause", "caused", "trigger",
                   "grund_fuer")


def _lade() -> dict:
    if not KORPUS.exists():
        print(f"ROT: Korpus fehlt: {KORPUS}")
        raise SystemExit(3)
    manifest = json.loads((KORPUS / "MANIFEST.json").read_text(encoding="utf-8"))
    klassen = {}
    for name in manifest["klassen"]:
        klassen[name] = json.loads((KORPUS / f"{name}.json").read_text(encoding="utf-8"))
    return {"manifest": manifest, "klassen": klassen}


def _mengen(manifest: dict) -> dict:
    """Die zwei geschlossenen Mengen — AUS DEM MANIFEST, nicht aus einer
    Kopie hier.

    Genau daran hing C6: `_passt` und `moegliche_starke` fuehrten dieselbe
    Menge zweimal und liefen auseinander. Fehlt der Block im MANIFEST, ist
    das rot und nicht ein stiller Rueckfall auf eine eingebaute Liste.
    """
    m = manifest.get("mengen")
    if not isinstance(m, dict):
        return {}
    noetig = ("keine_behauptung", "keine_starke", "erlaubte_wahrheiten")
    if any(not isinstance(m.get(k), list) or not m[k] for k in noetig):
        return {}
    return {k: tuple(m[k]) for k in noetig}


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
    return re.search(rf"Kuerzel\s*=\s*'{re.escape(kuerzel)}'", runnertext) is not None


def _quelle_pruefen(fall: dict) -> str | None:
    """NAK-182 R2: steht der Bezeichner WOERTLICH in der genannten Datei?

    Dieselbe Bauform wie `_bein_bekannt`, eine Ebene tiefer: vom Bein auf den
    Fall. Ein Pfad ins Leere ist rot, kein Hinweis.
    """
    quelle = fall.get("quelle")
    if not isinstance(quelle, dict) or set(quelle) != {"datei", "bezeichner"}:
        return f"{fall.get('fall')}: `quelle` fehlt oder hat die falsche Form"
    datei = WURZEL / quelle["datei"]
    if not datei.is_file():
        return f"{fall['fall']}: Quelldatei fehlt: {quelle['datei']}"
    text = datei.read_text(encoding="utf-8", errors="replace")
    if quelle["bezeichner"] not in text:
        return (f"{fall['fall']}: Bezeichner {quelle['bezeichner']!r} steht nicht "
                f"in {quelle['datei']}")
    return None


def _passt(fall: dict, mengen: dict) -> bool:
    """Ob die Aussage zur Wahrheit passt (NAK-182 R1a).

    Eine Enthaltung passt IMMER: sie behauptet nichts. Das ist keine
    Nachsicht, sondern §49.4 - ein konservatives `unsicher` ist besser als
    eine ueberzeugende falsche Ursache, und eine Kennzahl, die es bestraft,
    zoege das Werkzeug genau in die falsche Richtung.

    Darueber liegen die zwei Mengen: eine STARKE Aussage passt, wenn die
    Wahrheit nicht in `KEINE_STARKE` steht; eine SCHWACHE, wenn sie nicht in
    `KEINE_BEHAUPTUNG` steht.
    """
    if fall["aussage"] == ENTHALTUNG:
        return True
    if fall["aussage"] == STARK:
        return fall["wahrheit"] not in mengen["keine_starke"]
    return fall["wahrheit"] not in mengen["keine_behauptung"]


def _kennzahlen(faelle: list[dict], mengen: dict) -> dict:
    gesamt = len(faelle)
    stark = [f for f in faelle if f["aussage"] == STARK]
    enthaltungen = [f for f in faelle if f["aussage"] == ENTHALTUNG]

    # Eine Behauptung ist FALSCH, wenn die Wahrheit sie nicht traegt. Im
    # Korpus ist jeder Fall so gebaut, dass seine Aussage zur Wahrheit passt;
    # ein Fall, bei dem das nicht mehr stimmt, faellt hier auf.
    falsche_starke = [f for f in stark if not _passt(f, mengen)]
    falsche_schwache = [f for f in faelle
                        if f["aussage"] == SCHWACH and not _passt(f, mengen)]

    # Precision und Recall ueber die starken Behauptungen. Zaehler und Nenner
    # lesen DIESELBE Menge - das ist die Lehre aus C6.
    richtige_starke = len(stark) - len(falsche_starke)
    moegliche_starke = len([f for f in faelle
                            if f["wahrheit"] not in mengen["keine_starke"]])
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
    brier = (sum((float(f["konfidenz"]) - (1.0 if _passt(f, mengen) else 0.0)) ** 2
                 for f in behauptungen) / n_b) if n_b else 0.0

    mittlere_konfidenz = (sum(float(f["konfidenz"]) for f in behauptungen) / n_b) if n_b else 0.0
    trefferquote = (sum(1 for f in behauptungen if _passt(f, mengen)) / n_b) if n_b else 0.0
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
        "namen_falscher_schwacher": [f["fall"] for f in falsche_schwache],
    }


def _riegel(k: dict, klasse: str) -> list[str]:
    """Die Riegel ueber einer Kennzahlmenge — als EIGENE Funktion.

    NAK-182 MP1-6: `recall > 1` und `precision > 1` sind ueber den Korpusweg
    nach R1a strukturell unerreichbar (`richtige_starke` ist per Konstruktion
    hoechstens `moegliche_starke`). Eine Wache ohne ausfuehrbaren
    Negativtest ist keine gemessene Zusage - deshalb steht die Auswertung
    hier, wird von `main` auf die ECHTEN Kennzahlen jeder Klasse angewandt
    und vom Selbsttest mit synthetischen Dicts gefuettert. Ein Riegel, der
    nur im Selbsttest existierte, waere die zweite Wahrheit, an der C6 hing.
    """
    #

    # ⚠️ Geprueft wird NUR, was im Dict steht. Je Klasse uebergibt `main`
    # die zwei Behauptungszaehler, fuer das Gesamt alle Kennzahlen - genau
    # die Aufteilung, die vor NAK-182 galt. §49.4 nennt Brier, Coverage und
    # Kalibrierung als GESAMTzahlen; sie zusaetzlich je Klasse zu riegeln
    # waere eine neue Zusage und braucht einen eigenen Entscheid (Manifest
    # §4.7 Nr. 4). Der Selbsttest speist dagegen ein volles Dict ein und
    # erreicht damit jede Wache.
    rot: list[str] = []
    if k.get("falsche_starke", 0) > GATE_FALSCHE_STARKE:
        rot.append(f"{klasse}: {k['falsche_starke']} falsche starke Behauptung(en): "
                   f"{', '.join(k.get('namen_falscher_starker', []))}")
    if k.get("falsche_schwache", 0) > GATE_FALSCHE_SCHWACHE:
        rot.append(f"{klasse}: {k['falsche_schwache']} falsche schwache Behauptung(en): "
                   f"{', '.join(k.get('namen_falscher_schwacher', []))}")
    if "precision" in k and k.get("precision", 0.0) > GATE_QUOTE_MAX:
        rot.append(f"{klasse}: Precision {k['precision']:.4f} > {GATE_QUOTE_MAX} - "
                   f"eine Quote kann das nicht sein")
    if "recall" in k and k.get("recall", 0.0) > GATE_QUOTE_MAX:
        rot.append(f"{klasse}: Recall {k['recall']:.4f} > {GATE_QUOTE_MAX} - "
                   f"eine Quote kann das nicht sein")
    if "brier" in k and k.get("brier", 0.0) >= GATE_BRIER_MAX:
        rot.append(f"{klasse}: Brier Score {k['brier']:.4f} >= {GATE_BRIER_MAX}")
    if "coverage" in k and k.get("coverage", 1.0) < GATE_COVERAGE_MIN:
        rot.append(f"{klasse}: Coverage {k['coverage']:.2f} < {GATE_COVERAGE_MIN}")
    if "kalibrierung" in k and k.get("kalibrierung", 0.0) > GATE_KALIBRIERUNG_MAX:
        rot.append(f"{klasse}: Kalibrierung {k['kalibrierung']:.4f} > "
                   f"{GATE_KALIBRIERUNG_MAX}")
    return rot


def _enum_lesen(schema: dict, pfad: tuple[str, ...]) -> list[str] | None:
    knoten: object = schema
    for teil in pfad:
        if not isinstance(knoten, dict) or teil not in knoten:
            return None
        knoten = knoten[teil]
    if not isinstance(knoten, dict):
        return None
    werte = knoten.get("enum")
    return werte if isinstance(werte, list) and werte else None


def _vertragsmengen_pruefen(schema: dict | None) -> tuple[list[str], int]:
    """Entscheid G4 §8, zweite Haelfte: kein P4-Datenweg benennt einen
    "sicheren Ausloeser".

    Gemessen an den fuenf geschlossenen Mengen des Vertrags. Fehlt eine, ist
    das rot - sonst koennte eine umbenannte Definition den Riegel still
    leeren.
    """
    rot: list[str] = []
    if schema is None:
        return [f"Vertragsschema fehlt: {SCHEMA}"], 0
    geprueft = 0
    for name, pfad in VERTRAGSMENGEN.items():
        werte = _enum_lesen(schema, pfad)
        if werte is None:
            rot.append(f"geschlossene Menge fehlt im Vertrag: {name} ({'/'.join(pfad)})")
            continue
        geprueft += 1
        for wert in werte:
            klein = str(wert).lower()
            for wort in KAUSALE_WOERTER:
                if wort in klein:
                    rot.append(f"kausaler Bezeichner in {name}: {wert!r} enthaelt "
                               f"{wort!r} - P4 gibt keine Ursachenaussage ab")
    return rot, geprueft


def main(argv: list[str]) -> int:
    ausfuehrlich = "--ausfuehrlich" in argv
    daten = _lade()
    manifest = daten["manifest"]
    mengen = _mengen(manifest)
    runnertext = RUNNER.read_text(encoding="utf-8") if RUNNER.exists() else ""
    schema = (json.loads(SCHEMA.read_text(encoding="utf-8"))
              if SCHEMA.exists() else None)

    print(f"P4-Referenzkorpus: {manifest['faelle_gesamt']} Faelle in "
          f"{len(daten['klassen'])} Klassen")
    # NAK-182 R6: die Luecke steht im Kopf, nicht im Kleingedruckten. Eine
    # Zusage, die still fehlt, ist keine ausgewiesene Luecke.
    for luecke in manifest.get("nicht_gemessen", []):
        print(f"  in P4 nicht gemessen: {luecke['zusage']} -> {luecke['ticket']}")
    print()

    rot: list[str] = []
    if not mengen:
        # Ohne die Mengen gibt es keine Bewertung - und ganz sicher keinen
        # Rueckfall auf eine eingebaute Kopie.
        print("  ROT: das MANIFEST fuehrt `mengen` nicht (KEINE_BEHAUPTUNG / "
              "KEINE_STARKE / ERLAUBTE_WAHRHEITEN)")
        return 2

    gesamtfaelle: list[dict] = []

    for name in sorted(daten["klassen"]):
        klasse = daten["klassen"][name]
        faelle = klasse["faelle"]
        gesamtfaelle.extend(faelle)
        k = _kennzahlen(faelle, mengen)

        # Jedes genannte Bein muss der Runner wirklich fuehren.
        for bein in klasse["beine"]:
            if not _bein_bekannt(bein, runnertext):
                rot.append(f"{name}: Bein {bein} steht nicht im Kanon-Runner")

        # NAK-182 R2: und jeder Fall muss seine Quelle woertlich tragen.
        for f in faelle:
            befund = _quelle_pruefen(f)
            if befund:
                rot.append(f"{name}: {befund}")
            if f["wahrheit"] not in mengen["erlaubte_wahrheiten"]:
                rot.append(f"{name}: {f['fall']}: Wahrheit {f['wahrheit']!r} steht "
                           f"nicht in ERLAUBTE_WAHRHEITEN")

        marke = "[ok] " if not (k["falsche_starke"] or k["falsche_schwache"]) else "[ROT]"
        print(f"{marke} {name:<10} {k['faelle']:>2} Faelle · "
              f"{k['starke_behauptungen']} stark · {k['enthaltungen']} Enthaltungen · "
              f"Precision {k['precision']:.2f} · Recall {k['recall']:.2f} · "
              f"Brier {k['brier']:.4f} · Coverage {k['coverage']:.2f}")
        if ausfuehrlich:
            for f in faelle:
                print(f"        {f['bein']:<4} {f['fall']}")
                print(f"             {f['quelle']['datei']} :: {f['quelle']['bezeichner']}")
                print(f"             Wahrheit {f['wahrheit']!r} -> Aussage "
                      f"{f['aussage']!r} ({f['konfidenz']:.2f})")
                if f.get("hinweis"):
                    print(f"             {f['hinweis']}")
        # Je Klasse riegeln die zwei Behauptungszaehler - so wie vor
        # NAK-182 `falsche_starke`. Die Verteilungskennzahlen gehoeren
        # dem Gesamt (§49.4).
        rot += _riegel({s: k[s] for s in ("falsche_starke", "falsche_schwache",
                                          "namen_falscher_starker",
                                          "namen_falscher_schwacher")}, name)

    print()
    gesamt = _kennzahlen(gesamtfaelle, mengen)
    print(f"Gesamt: {gesamt['faelle']} Faelle · "
          f"{gesamt['falsche_starke']} falsche starke Behauptungen · "
          f"{gesamt['falsche_schwache']} falsche schwache · "
          f"Brier {gesamt['brier']:.4f} · Kalibrierung {gesamt['kalibrierung']:.4f} · "
          f"Coverage {gesamt['coverage']:.2f} · "
          f"Enthaltungsrate {gesamt['enthaltungsrate']:.2f}")

    rot += _riegel(gesamt, "Gesamt")

    # Alle vier Klassen muessen da sein - eine fehlende macht das Gate rot,
    # statt es leiser gruen zu machen.
    for pflicht in ("referenz", "zeitachse", "vergleich", "stereo"):
        if pflicht not in daten["klassen"]:
            rot.append(f"Klasse fehlt: {pflicht}")

    vertragsrot, geprueft = _vertragsmengen_pruefen(schema)
    rot += vertragsrot
    print(f"Vertrag: {geprueft} von {len(VERTRAGSMENGEN)} geschlossenen Mengen "
          f"gelesen, kein kausaler Bezeichner darin (G4 §8)")

    if rot:
        print()
        for r in rot:
            print(f"  ROT: {r}")
        return 2
    print("Exit-Gate P4: alle vier Klassen bestehen, keine falsche Behauptung.")
    return 0


# ── A27: der Selbsttest des Pruefers ─────────────────────────────────────
#
# NAK-182 R1: das Skript hatte bis hierher KEIN Testartefakt. Der einzige
# Beleg war ein Rotbeweis, der ausgerechnet den einen Wert manipulierte, den
# `_passt` fing. Der Selbsttest laeuft OHNE Repo-Fixture: er baut seine
# Faelle im Speicher und schickt sie durch dieselben Funktionen, die A26
# fuehrt. Jede Erwartung wird MIT ihrem Gegenteil geprueft - sonst waere er
# auch dann gruen, wenn ein Riegel immer rot meldete.

_SELBST_MENGEN = {
    "keine_behauptung": ("unbekannt", "unvergleichbar", "zeitvariabel"),
    "keine_starke": ("unbekannt", "unvergleichbar", "zeitvariabel",
                     "nicht_kausal", "nicht_exakt"),
    "erlaubte_wahrheiten": ("unbekannt", "unvergleichbar", "zeitvariabel",
                            "nicht_kausal", "nicht_exakt", "innerhalb"),
}


def _selbsttest() -> int:
    bestanden = 0
    gescheitert: list[str] = []

    def pruefe(ok: bool, was: str, zusatz: str = "") -> None:
        nonlocal bestanden
        print(f"{'[ok]  ' if ok else '[ROT] '} {was}" + (f"  [{zusatz}]" if zusatz else ""))
        if ok:
            bestanden += 1
        else:
            gescheitert.append(was)

    def fall(wahrheit: str, aussage: str, konf: float = 0.9) -> dict:
        return {"bein": "B0", "fall": "synthetisch", "wahrheit": wahrheit,
                "aussage": aussage, "konfidenz": konf}

    print("A27 · Selbsttest von pruefe_p4_korpus.py (NAK-182 R1, MP1-6)")
    print()

    # ── Korpusweg: S-01a…e — stark auf jedem Wert aus KEINE_STARKE ───────
    print("Korpusweg (synthetischer Korpus -> _kennzahlen -> _riegel)")
    for w in _SELBST_MENGEN["keine_starke"]:
        k = _kennzahlen([fall(w, STARK)], _SELBST_MENGEN)
        pruefe(k["falsche_starke"] == 1,
               f"S-01 {w}: eine STARKE Aussage darauf ist eine falsche Behauptung",
               f"falsche_starke={k['falsche_starke']}")
        g = _kennzahlen([fall(w, ENTHALTUNG, 0.5)], _SELBST_MENGEN)
        pruefe(g["falsche_starke"] == 0,
               f"S-01 {w} Gegenteil: dieselbe Wahrheit mit `unsicher` ist keine",
               f"falsche_starke={g['falsche_starke']}")

    # ── S-02a…c — schwach auf KEINE_BEHAUPTUNG ───────────────────────────
    for w in _SELBST_MENGEN["keine_behauptung"]:
        k = _kennzahlen([fall(w, SCHWACH)], _SELBST_MENGEN)
        pruefe(k["falsche_schwache"] == 1,
               f"S-02 {w}: auch eine SCHWACHE Aussage darauf ist falsch",
               f"falsche_schwache={k['falsche_schwache']}")
        g = _kennzahlen([fall(w, ENTHALTUNG, 0.5)], _SELBST_MENGEN)
        pruefe(g["falsche_schwache"] == 0,
               f"S-02 {w} Gegenteil: mit `unsicher` ist sie keine",
               f"falsche_schwache={g['falsche_schwache']}")

    # ── S-02d…e — die TRENNUNG der zwei Mengen ───────────────────────────
    #
    # Ohne diese zwei Zeilen waere `KEINE_STARKE` von `KEINE_BEHAUPTUNG`
    # nicht unterscheidbar, und der Selbsttest waere auch mit einer einzigen
    # Menge gruen.
    for w in ("nicht_kausal", "nicht_exakt"):
        k = _kennzahlen([fall(w, SCHWACH)], _SELBST_MENGEN)
        pruefe(k["falsche_schwache"] == 0 and k["falsche_starke"] == 0,
               f"S-02 {w}: die SCHWACHE Aussage traegt diese Wahrheit - genau das "
               f"trennt KEINE_STARKE von KEINE_BEHAUPTUNG",
               f"falsche_schwache={k['falsche_schwache']}")
        g = _kennzahlen([fall(w, STARK)], _SELBST_MENGEN)
        pruefe(g["falsche_starke"] == 1,
               f"S-02 {w} Gegenteil: die STARKE ist es nicht",
               f"falsche_starke={g['falsche_starke']}")

    # ── S-07 — eine Wahrheit ausserhalb der Menge ────────────────────────
    aussen = "erfundene_wahrheit"
    pruefe(aussen not in _SELBST_MENGEN["erlaubte_wahrheiten"],
           "S-07: eine Wahrheit ausserhalb ERLAUBTE_WAHRHEITEN wird als solche erkannt")
    pruefe("innerhalb" in _SELBST_MENGEN["erlaubte_wahrheiten"],
           "S-07 Gegenteil: ein Wert aus der Menge nicht")

    # ── Kennzahlweg: die Quotenwachen ────────────────────────────────────
    print()
    print("Kennzahlweg (_riegel direkt) - der Korpus kann diese Faelle nach R1a")
    print("nicht mehr erzeugen, die Wachen brauchen trotzdem einen Weg zu fallen")
    grund = {"falsche_starke": 0, "falsche_schwache": 0, "precision": 1.0,
             "recall": 1.0, "brier": 0.01, "coverage": 0.8, "kalibrierung": 0.05}

    def kennzahl(**anders) -> dict:
        d = dict(grund)
        d.update(anders)
        return d

    faelle_kennzahl = [
        ("S-03 recall", kennzahl(recall=1.07), kennzahl(recall=1.00), "Recall"),
        ("S-04 precision", kennzahl(precision=1.02), kennzahl(precision=1.00), "Precision"),
        ("S-04b falsche_starke", kennzahl(falsche_starke=1), kennzahl(), "starke"),
        ("S-04c falsche_schwache", kennzahl(falsche_schwache=1), kennzahl(), "schwache"),
        ("S-04d brier", kennzahl(brier=0.26), kennzahl(brier=0.24), "Brier"),
        ("S-04e coverage", kennzahl(coverage=0.49), kennzahl(coverage=0.51), "Coverage"),
        ("S-04e kalibrierung", kennzahl(kalibrierung=0.16), kennzahl(kalibrierung=0.14),
         "Kalibrierung"),
    ]
    for name, schlecht, gut, wort in faelle_kennzahl:
        r = _riegel(schlecht, "selbsttest")
        pruefe(any(wort.lower() in z.lower() for z in r),
               f"{name}: der Riegel meldet rot und nennt {wort}",
               "; ".join(r) if r else "keine rote Zeile")
        pruefe(not _riegel(gut, "selbsttest"),
               f"{name} Gegenteil: innerhalb des Gates keine rote Zeile")

    # ── S-05/S-06 — die Quellenpruefung ──────────────────────────────────
    print()
    print("Quellenweg (_quelle_pruefen)")
    echt = {"fall": "x", "quelle": {"datei": "tools/eq-copilot/pruefe_p4_korpus.py",
                                    "bezeichner": "_quelle_pruefen"}}
    pruefe(_quelle_pruefen(echt) is None,
           "S-05 Gegenteil: ein Bezeichner, der in der Datei steht, ist gruen")
    # ⚠️ Der erfundene Bezeichner wird ZUSAMMENGESETZT. Stuende er als
    # Literal hier, faende ihn die Suche in genau dieser Datei - und der
    # Selbsttest pruefte sein eigenes Gegenteil.
    erfunden = {"fall": "x", "quelle": {"datei": "tools/eq-copilot/pruefe_p4_korpus.py",
                                        "bezeichner": "gibt_es" + "_nicht_" + "9f3a2b1c"}}
    befund = _quelle_pruefen(erfunden)
    pruefe(befund is not None and "steht nicht in" in befund,
           "S-05: ein erfundener Bezeichner ist rot und die Meldung nennt Datei "
           "und Bezeichner", befund or "")
    leer = {"fall": "x", "quelle": {"datei": "gibt/es/nicht.cpp", "bezeichner": "x"}}
    befund = _quelle_pruefen(leer)
    pruefe(befund is not None and "Quelldatei fehlt" in befund,
           "S-06: ein Pfad ins Leere ist rot", befund or "")

    # ── S-08/S-09 — die geschlossenen Mengen des Vertrags ────────────────
    print()
    print("Vertragsweg (_vertragsmengen_pruefen)")
    echtes = json.loads(SCHEMA.read_text(encoding="utf-8")) if SCHEMA.exists() else None
    r, geprueft = _vertragsmengen_pruefen(echtes)
    pruefe(not r and geprueft == len(VERTRAGSMENGEN),
           "S-08/S-09 Gegenteil: das Vertragsschema traegt alle fuenf Mengen und "
           "keinen kausalen Bezeichner", "; ".join(r) if r else f"{geprueft} Mengen")
    if echtes is not None:
        ohne = json.loads(json.dumps(echtes))
        del ohne["$defs"]["alignment_klasse"]
        r2, _ = _vertragsmengen_pruefen(ohne)
        pruefe(any("alignment_klasse" in z for z in r2),
               "S-08: fehlt eine der fuenf Mengen, ist das rot", "; ".join(r2))
        mit = json.loads(json.dumps(echtes))
        mit["$defs"]["session_paar"]["properties"]["kettenbefund"]["enum"].append(
            "sicherer_ausloeser")
        r3, _ = _vertragsmengen_pruefen(mit)
        pruefe(any("kausaler Bezeichner" in z for z in r3),
               "S-09: ein Enumwert mit kausalem Bezeichner ist rot", "; ".join(r3))

    print()
    print(f"{bestanden} bestanden, {len(gescheitert)} gescheitert")
    if gescheitert:
        for g in gescheitert:
            print(f"  ROT: {g}")
        return 2
    print("A27: der Pruefer faellt an jeder Zusage, die er behauptet.")
    return 0


if __name__ == "__main__":
    if "--selbsttest" in sys.argv[1:]:
        raise SystemExit(_selbsttest())
    raise SystemExit(main(sys.argv[1:]))
