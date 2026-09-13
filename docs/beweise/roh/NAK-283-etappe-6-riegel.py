#!/usr/bin/env python3
"""NAK-283 Etappe 6 - Riegel und Basislaeufe (Bauer, 13.09.2026).

Riegel ueber die Zusagen, die kein Kanonbein allein traegt (M-66, M-67, M-68,
M-70), und die Basislaeufe fuer die Zeilen, die am Basis-SHA ROT sind (M-61,
M-62, M-69). Jede Pruefung druckt eine Zeile `  ok      <Kennung> ...` oder
`  FEHLER  <Kennung> ...`; Exit 0 nur, wenn keine FEHLER-Zeile steht.

Aufruf vom Workspace-Root:
  py -3.13 docs/beweise/roh/NAK-283-etappe-6-riegel.py M-66
  py -3.13 docs/beweise/roh/NAK-283-etappe-6-riegel.py M-67 [--revision SHA]
  py -3.13 docs/beweise/roh/NAK-283-etappe-6-riegel.py M-68 [--registerstand]
  py -3.13 docs/beweise/roh/NAK-283-etappe-6-riegel.py M-70 [--revision SHA]
  py -3.13 docs/beweise/roh/NAK-283-etappe-6-riegel.py basis-M-61-M-62 [--revision SHA]
  py -3.13 docs/beweise/roh/NAK-283-etappe-6-riegel.py basis-M-69 [--revision SHA]

Ohne `--revision` misst jeder Riegel den Arbeitsbaum; mit `--revision` den
Stand dieser Revision (git show), fuer die Basislaeufe am Basis-SHA.
"""
from __future__ import annotations

import argparse
import contextlib
import ctypes
import importlib.util
import io
import json
import os
import pathlib
import re
import subprocess
import sys
import tempfile

WURZEL = pathlib.Path(__file__).resolve().parents[3]
BASIS_ETAPPE_6 = "7b1f38b7bddcf67965d1bbc2609fd74464cfaaea"
GOLDEN = "eq-copilot/plugin/tests/Sonde013TruePeakGoldenTest.cpp"
SOAK = "tools/eq-copilot/pruefe_session_soak.py"
NACHTRAG = "Nachtrag 13.09.2026 (NAK-283, N01)"
HINWEIS_STATIONAER = "EBU Tech 3341 Faelle 15-19: fs/4, fs/6, fs/8 bei 0,50 und 1,41 FFS"
HINWEIS_TRANSIENT = ("EBU Tech 3341 Faelle 20-23: transient, vier Abtastversaetze, "
                     "±0,1 dB gegen validierte Referenz plus Normtoleranz +0,2/−0,4 dBTP")
WORTLAUT_MARKE = "M-68 · Wortlaut für die Registerzeile NAK-199"

_fehler = 0
_gesamt = 0


def zeile(ok: bool, kennung: str, text: str) -> None:
    global _fehler, _gesamt
    _gesamt += 1
    print(("  ok      " if ok else "  FEHLER  ") + f"{kennung} {text}")
    if not ok:
        _fehler += 1


def git_show(revision: str, pfad: str) -> str | None:
    lauf = subprocess.run(["git", "-C", str(WURZEL), "show", f"{revision}:{pfad}"],
                          capture_output=True)
    return lauf.stdout.decode("utf-8") if lauf.returncode == 0 else None


def lade_modul(name: str, pfad: pathlib.Path):
    spec = importlib.util.spec_from_file_location(name, pfad)
    modul = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(modul)
    return modul


def lade_modul_aus_revision(name: str, revision: str, pfad: str, ordner: pathlib.Path):
    text = git_show(revision, pfad)
    if text is None:
        raise SystemExit(f"{pfad} fehlt an {revision}")
    datei = ordner / f"{name}.py"
    datei.write_text(text, encoding="utf-8")
    return lade_modul(name, datei)


def zellen(tabellenzeile: str) -> list[str]:
    """Zellen an nicht geschuetzten Pipes, wie `dokuriegel.py` sie zaehlt."""
    return re.split(r"(?<!\\)\|", tabellenzeile)


def normiert(text: str) -> list[str]:
    return text.replace("\r\n", "\n").split("\n")


# ─────────────────────────────────────────────────────────────── M-66

def riegel_m66(_args) -> None:
    """Der P4-Korpus nennt beide True-Peak-Fallgruppen getrennt (M-66)."""
    erzeuger = lade_modul("erzeuge_p4_korpus_riegel", WURZEL / "tools/eq-copilot/erzeuge_p4_korpus.py")
    neu = json.loads(erzeuger.baue()["referenz.json"])
    bestand = json.loads((WURZEL / "eq-copilot/fixtures/p4-korpus/referenz.json")
                         .read_text(encoding="utf-8"))
    for herkunft, daten in (("Neuerzeugung", neu), ("committeter Korpus", bestand)):
        faelle = {f["fall"]: f for f in daten["faelle"]}
        stat = faelle.get("true_peak_matches_ebu_test_set")
        trans = faelle.get("true_peak_matches_ebu_transient_set")
        zeile(stat is not None and stat.get("hinweis") == HINWEIS_STATIONAER,
              f"K1 {herkunft}:", "der bestehende Eintrag true_peak_matches_ebu_test_set "
              f"behaelt seinen ehrlichen hinweis (Faelle 15-19) - "
              f"{stat.get('hinweis') if stat else 'Eintrag fehlt'!r}")
        zeile(trans is not None and trans.get("bein") == "B17"
              and trans.get("quelle") == {"datei": GOLDEN,
                                          "bezeichner": "true_peak_matches_ebu_transient_set"}
              and trans.get("hinweis") == HINWEIS_TRANSIENT,
              f"K2 {herkunft}:", "ein ZWEITER Eintrag der Klasse referenz fuer "
              "true_peak_matches_ebu_transient_set mit bein B17, quelle auf den neuen "
              f"Fallnamen und dem hinweis der Matrix - "
              f"{json.dumps(trans, ensure_ascii=False) if trans else 'Eintrag fehlt'}")
    text = (WURZEL / GOLDEN).read_text(encoding="utf-8")
    zeile("true_peak_matches_ebu_transient_set" in text, "K3",
          "der Bezeichner steht woertlich im Golden (B17)")


# ─────────────────────────────────────────────────────────────── M-67

def riegel_m67(args) -> None:
    """SONDE-013 M-02 und M-81: nur Zuwachs in den Nachweisspalten (M-67)."""
    pfad = "docs/beweise/SONDE-013.md"
    basis = git_show(BASIS_ETAPPE_6, pfad)
    if args.revision:
        ziel, herkunft = git_show(args.revision, pfad), f"Revision {args.revision[:8]}"
    else:
        ziel, herkunft = (WURZEL / pfad).read_bytes().decode("utf-8"), "Arbeitsbaum"
    print(f"M-67 an {herkunft}, verglichen mit dem Basis-SHA {BASIS_ETAPPE_6[:8]}")
    b, z = normiert(basis), normiert(ziel or "")
    zeile(len(b) == len(z), "R1", f"dieselbe Zeilenzahl ({len(b)} / {len(z)})")
    # Die Matrixzeilen selbst: `| M-81 |` beginnt auch eine Zeile der Tabelle in
    # §15, deshalb zaehlt zusaetzlich der Wortlaut der Zusage.
    i02 = [i for i, t in enumerate(b) if t.startswith("| M-02 |") and "8-fach-Pfad bleibt" in t]
    i81 = [i for i, t in enumerate(b)
           if t.startswith("| M-81 |") and "8-fach-True-Peak-Pfad besteht" in t]
    zeile(len(i02) == 1 and len(i81) == 1, "R1",
          f"die Matrixzeilen M-02 und M-81 stehen je genau einmal (Zeilen "
          f"{[i + 1 for i in i02]} und {[i + 1 for i in i81]})")
    erlaubt = set(i02 + i81)
    fremd = [i + 1 for i in range(min(len(b), len(z))) if b[i] != z[i] and i not in erlaubt]
    zeile(not fremd, "R1", "keine andere Zeile veraendert"
          + (f" - veraendert: Zeilen {fremd[:12]}" if fremd else ""))
    for name, idx, pflicht in (("M-02", i02, ("20 bis 23", "15 bis 23",
                                              "true_peak_matches_ebu_transient_set")),
                               ("M-81", i81, ("15 bis 23",
                                              "true_peak_matches_ebu_transient_set"))):
        if len(idx) != 1 or idx[0] >= len(z):
            continue
        zb, zz = zellen(b[idx[0]]), zellen(z[idx[0]])
        zeile(len(zb) == len(zz), f"R2 {name}", f"gleiche Zellenzahl ({len(zb)} / {len(zz)})")
        if len(zb) != len(zz):
            continue
        # Zellen: '' · ID · Zustand · Zusage · Nachweis · Quelle und Status · ''
        andere = [k for k in range(len(zb)) if k != 4 and zb[k] != zz[k]]
        zeile(not andere, f"R2 {name}", "ID-, Zustands-, Zusage- und Statusspalte "
              "unveraendert - nur die Nachweisspalte darf wachsen"
              + (f" (veraendert: Zelle {andere})" if andere else ""))
        alt = zb[4].rstrip()
        zuwachs = zz[4][len(alt):] if zz[4].startswith(alt) else None
        zeile(zuwachs is not None, f"R3 {name}",
              "die Nachweisspalte beginnt unveraendert mit ihrem Bestand (nur Anfuegen)")
        zeile(zuwachs is not None and NACHTRAG in zuwachs and all(p in zuwachs for p in pflicht),
              f"R4 {name}", f"der Zuwachs traegt den datierten Nachtrag und nennt {list(pflicht)}")
        zeile("BELEGT, gemessen" in zz[5], f"R5 {name}",
              "die Statusspalte sagt weiter BELEGT, gemessen - ergaenzt, nie umgeschrieben")
    if not args.revision:
        lauf = subprocess.run([sys.executable, str(WURZEL / "tools/plan/dokuriegel.py"), pfad],
                              cwd=WURZEL, capture_output=True, text=True, encoding="utf-8")
        letzte = lauf.stdout.strip().splitlines()[-1] if lauf.stdout.strip() else ""
        zeile(lauf.returncode == 0, "R6", f"dokuriegel.py ohne Befund (Exit {lauf.returncode}: {letzte})")


# ─────────────────────────────────────────────────────────────── M-68

def wortlaut_aus_manifest(pfad: pathlib.Path) -> str | None:
    text = pfad.read_text(encoding="utf-8").replace("\r\n", "\n")
    i = text.rfind(WORTLAUT_MARKE)
    if i < 0:
        return None
    j = text.find("```text\n", i)
    k = text.find("\n```", j + 8) if j >= 0 else -1
    return text[j + 8:k].strip() if j >= 0 and k >= 0 else None


def riegel_m68(args) -> None:
    """NAK-199 nennt den gemessenen Umfang (M-68). Das Register ist
    Dirigentensache: der Riegel misst die Fixfassung (Registerzeile plus der
    Wortlaut aus dem Etappenabschnitt), `--registerstand` die Zeile, wie sie
    im Register steht."""
    golden = (WURZEL / GOLDEN).read_text(encoding="utf-8")
    korpus = (WURZEL / "eq-copilot/fixtures/p4-korpus/referenz.json").read_text(encoding="utf-8")
    gemessen = ("true_peak_matches_ebu_transient_set" in golden
                and "true_peak_matches_ebu_transient_set" in korpus)
    zeile(gemessen, "R0", "erst der Nachweis: B17 und P4-Korpus tragen "
          "true_peak_matches_ebu_transient_set (M-65, M-66)")
    wortlaut = wortlaut_aus_manifest(WURZEL / "docs/beweise/NAK-283.md")
    zeile(wortlaut is not None, "R1", "der Wortlaut steht im Etappenabschnitt von "
          "docs/beweise/NAK-283.md (Marke und text-Block)")
    zeile(wortlaut is not None and "13.09.2026" in wortlaut and "(NAK-283, N01)" in wortlaut
          and "True Peak jetzt über 15 bis 23 gedeckt" in wortlaut and "LUFS-I" in wortlaut,
          "R1", f"der Wortlaut nennt Datum, Ticket, den Umfang 15 bis 23 und den offenen "
          f"LUFS-I-Teil: {wortlaut!r}")
    register = WURZEL / "docs/offene-punkte.md"
    zeilen = normiert(register.read_bytes().decode("utf-8"))
    idx = [i for i, t in enumerate(zeilen) if t.startswith("| NAK-199 |")]
    zeile(len(idx) == 1, "R2", "die Registerzeile NAK-199 steht genau einmal")
    if len(idx) != 1:
        return
    alt = zeilen[idx[0]]
    kopf = alt[:alt.rstrip().rfind("|")].rstrip()
    if args.registerstand:
        neu, herkunft, ort = alt, "Registerstand", "am Registerstand"
    else:
        neu, herkunft, ort = f"{kopf} {wortlaut or ''} |", "Fixfassung", "an der Fixfassung"
    print(f"M-68 {ort} der Zeile NAK-199")
    zeile(not gemessen or ("15 bis 23" in neu and "(NAK-283, N01)" in neu), "R3",
          f"{herkunft}: die Zeile behaelt nicht den alten Umfang, obwohl der neue gemessen "
          f"ist - sie nennt 15 bis 23 mit Datum und Ticket")
    zeile(neu.startswith(kopf) and "ist für LUFS-I nicht gedeckt" in neu, "R4",
          f"{herkunft}: nur Anfuegen - der bestehende Wortlaut samt offenem LUFS-I-Teil "
          f"bleibt unberuehrt")
    dok = lade_modul("dokuriegel_riegel", WURZEL / "tools/plan/dokuriegel.py")
    text = "\n".join(zeilen[:idx[0]] + [neu] + zeilen[idx[0] + 1:])
    rel = pathlib.Path("docs/offene-punkte.md")
    befunde = dok.tabellen_pruefen(rel, text) + dok.verweise_pruefen(rel, text)[0]
    zeile(not befunde, "R5", f"{herkunft}: dokuriegel.py ohne Befund"
          + (": " + "; ".join(befunde[:3]) if befunde else ""))


# ─────────────────────────────────────────────────────────────── M-70

def riegel_m70(args) -> None:
    """Scheitern ist an einem eigenen Status erkennbar - beide Haelften (M-70)."""
    c_pfad = "eq-copilot/plugin/tests/Sonde014IntentTest.cpp"
    namen_c = ("m283_07_bis_09",
               "entferne_intent_an_der_revisionsobergrenze_mutiert_nichts",
               "entferne_schutzangabe_an_der_revisionsobergrenze_mutiert_nichts",
               "entferne_beziehung_an_der_revisionsobergrenze_mutiert_nichts")
    namen_p = ("fehlende_rss_messung_ist_kein_pass",
               "messfehler_und_nullmessung_sind_unterscheidbar")
    if args.revision:
        c_text = git_show(args.revision, c_pfad) or ""
        p_text = git_show(args.revision, SOAK) or ""
        herkunft = f"Revision {args.revision[:8]}"
    else:
        c_text = (WURZEL / c_pfad).read_text(encoding="utf-8")
        p_text = (WURZEL / SOAK).read_text(encoding="utf-8")
        herkunft = "Arbeitsbaum"
    print(f"M-70 an {herkunft}")
    zeile(all(n in c_text for n in namen_c), "C1", "C++-Haelfte (F11, Etappe 2): "
          "Sonde014IntentTest traegt M-07 bis M-09 - Ablehnung ohne Mutation plus gesetzter grund")
    zeile(all(n in p_text for n in namen_p), "P1", "Python-Haelfte (F13, Etappe 6): "
          "pruefe_session_soak.py --selbsttest traegt M-61 und M-62 - Messpunkt mit eigenem "
          "Gueltigkeitsmerkmal")
    if args.revision:
        print("  --      die Laeufe C2, C3 und P2 gibt es nur am Arbeitsbaum")
        return
    exe = (WURZEL / "eq-copilot/build/plugin/EqCopSonde014IntentTest_artefacts/Release/"
                    "EqCopSonde014IntentTest.exe")
    quellen = [WURZEL / c_pfad, WURZEL / "eq-copilot/plugin/state/NakamaState.cpp"]
    frisch = exe.exists() and all(exe.stat().st_mtime >= q.stat().st_mtime for q in quellen)
    zeile(frisch, "C2", "das B27-Binary existiert und ist juenger als Test und NakamaState.cpp")
    if exe.exists():
        lauf = subprocess.run([str(exe)], cwd=WURZEL, capture_output=True, text=True,
                              encoding="utf-8", errors="replace", timeout=900)
        zeilen = lauf.stdout.splitlines()
        ok = all(any(z.startswith("  ok ") and n in z for z in zeilen) for n in namen_c[1:])
        rot = [z for z in zeilen if z.startswith("  FEHLER") and re.search(r"M-0[789]", z)]
        zeile(lauf.returncode == 0 and ok and not rot, "C3",
              f"B27 gruen am Stand: Exit {lauf.returncode}, M-07 bis M-09 je mit ok-Zeile "
              f"{ok}, FEHLER-Zeilen zu M-07 bis M-09: {len(rot)}")
    lauf = subprocess.run([sys.executable, str(WURZEL / SOAK), "--selbsttest"], cwd=WURZEL,
                          capture_output=True, text=True, encoding="utf-8", errors="replace",
                          timeout=900)
    ok61 = "  ok      [M-61 · fehlende_rss_messung_ist_kein_pass]" in lauf.stdout
    ok62 = "  ok      [M-62 · messfehler_und_nullmessung_sind_unterscheidbar]" in lauf.stdout
    zeile(lauf.returncode == 0 and ok61 and ok62, "P2",
          f"Python-Selbsttest gruen: Exit {lauf.returncode}, M-61 ok {ok61}, M-62 ok {ok62}")


# ─────────────────────────────────────────────────── Basislauf M-61, M-62

class _WindowsErsatz:
    """Ersetzt `ctypes.windll` fuer die alte Messstelle: OpenProcess und
    GetProcessMemoryInfo gelingen, das Working Set bleibt 0 - ein Prozess mit
    tatsaechlich 0 Bytes, den es real nicht gibt."""

    class _Aufruf:
        def __init__(self, rueckgabe):
            self.rueckgabe = rueckgabe

        def __call__(self, *argumente):
            return self.rueckgabe

    def __init__(self):
        self.kernel32 = type("K32", (), {"OpenProcess": self._Aufruf(1),
                                         "CloseHandle": self._Aufruf(1)})()
        self.psapi = type("Psapi", (), {"GetProcessMemoryInfo": self._Aufruf(1)})()


def basis_m61_m62(args) -> None:
    with tempfile.TemporaryDirectory(prefix="nak283-e6-") as tmp:
        neu = lade_modul("soak_neu", WURZEL / SOAK)
        if args.revision:
            modul = lade_modul_aus_revision("soak_revision", args.revision, SOAK, pathlib.Path(tmp))
            herkunft = f"Revision {args.revision[:8]}"
        else:
            modul, herkunft = neu, "Arbeitsbaum"
        hat_merkmal = hasattr(modul, "rss_messung")
        print(f"Basislauf M-61/M-62 an {herkunft} (Messstelle mit Gueltigkeitsmerkmal: {hat_merkmal})")
        zeit = {"minute": 1, "sekunden": 60.0, "im_neustartfenster": False}

        if hat_merkmal:
            fehlmessung = modul.rss_messung(0)
            fehlpunkt = modul.speicherpunkt(zeit, fehlmessung)
            nullpunkt = modul.speicherpunkt(zeit, modul.rss_messung(1234, api=modul._ErsatzSpeicherApi(0)))
        else:
            fehlwert = modul.rss_bytes(0)                      # PID 0 laesst sich nie oeffnen
            fehlpunkt = {**zeit, "rss_bytes": fehlwert}        # so baut die alte probe() den Punkt
            echt = ctypes.windll
            ctypes.windll = _WindowsErsatz()
            try:
                nullwert = modul.rss_bytes(1234)
            finally:
                ctypes.windll = echt
            nullpunkt = {**zeit, "rss_bytes": nullwert}
        a, b = json.dumps(fehlpunkt, sort_keys=True), json.dumps(nullpunkt, sort_keys=True)
        zeile(a != b, "M-62", f"{herkunft}: Messfehler und Nullmessung sind im Bericht "
              f"verschieden - Messfehler {a}, Nullmessung {b}")

        # M-61: derselbe gruene Bericht (im Speicher, Zahlen des Kontrolllaufs 4ff6f248),
        # jede tragende Messung durch die Messung eines nicht abfragbaren Prozesses ersetzt.
        bericht = neu._selbsttest_bericht()
        for kurve in ("client", "broker"):
            for i, p in enumerate(bericht["speicher"][kurve]):
                zeitteil = {k: v for k, v in p.items() if not k.startswith("rss_")}
                if hat_merkmal:
                    messung = fehlmessung if not p["im_neustartfenster"] else \
                        {k: v for k, v in p.items() if k.startswith("rss_")}
                    bericht["speicher"][kurve][i] = modul.speicherpunkt(zeitteil, messung)
                else:
                    wert = fehlpunkt["rss_bytes"] if not p["im_neustartfenster"] else p["rss_bytes"]
                    bericht["speicher"][kurve][i] = {**zeitteil, "rss_bytes": wert}
        puffer = io.StringIO()
        with contextlib.redirect_stdout(puffer):
            code = modul.urteile(bericht, neu._selbsttest_args())
        text = puffer.getvalue()
        im_budget = any(z.startswith("  ok      [S07") and "im Budget" in z for z in text.splitlines())
        fehlt = any("Messung fehlt" in z for z in text.splitlines())
        zeile(code != 0 and not im_budget and fehlt, "M-61",
              f"{herkunft}: eine fehlende Messung ist kein PASS - Exit {code}, "
              f"S07 im Budget {im_budget}, MESSUNG FEHLT {fehlt}")
        for z in text.splitlines():
            if "[S07" in z or z.startswith(("GRUEN", "ROT", "MESSUNG FEHLT")):
                print("             " + z.strip())


# ────────────────────────────────────────────────────────── Basislauf M-69

def basis_m69(args) -> None:
    with tempfile.TemporaryDirectory(prefix="nak283-e6-") as tmp:
        neu = lade_modul("gesundheit_neu", WURZEL / "tools/plan/gesundheit.py")
        if args.revision:
            modul = lade_modul_aus_revision("gesundheit_revision", args.revision,
                                            "tools/plan/gesundheit.py", pathlib.Path(tmp))
            herkunft = f"Revision {args.revision[:8]}"
            gleich = subprocess.run(["git", "-C", str(WURZEL), "diff", "--quiet", args.revision,
                                     "--", "broker/src", "eq-copilot/plugin",
                                     ":(exclude)eq-copilot/plugin/tests"]).returncode == 0
            zeile(gleich, "V0", f"die Produktquellen des Arbeitsbaums gleichen {herkunft} "
                  "(git diff --quiet, Tests ausgenommen) - der Arbeitsbaum steht fuer ihren Baum")
        else:
            modul, herkunft = neu, "Arbeitsbaum"
        print(f"Basislauf M-69 an {herkunft}")
        try:
            gemessen = {rel for rel, _ in modul.sammle_quellen(WURZEL)}
        except RuntimeError as f:
            zeile(False, "M-69", f"{herkunft}: Werkzeugfehler beim Sammeln: {f}")
            return
        luecken = neu.inventarluecken(WURZEL, gemessen)
        details = []
        for unterbaum, anzahl in luecken:
            ort = unterbaum.replace(" (direkt)", "")
            dateien = [p for p in (WURZEL / ort).rglob("*") if neu.ist_quelldatei(p)
                       and p.relative_to(WURZEL).as_posix() not in gemessen
                       and not neu.ausnahme(p.relative_to(WURZEL).as_posix())]
            zeilen = sum(neu.zeilen_zaehlen(neu.lies_text(p)) for p in dateien)
            groesste = max(dateien, key=lambda p: neu.zeilen_zaehlen(neu.lies_text(p)))
            details.append(f"{unterbaum}: {anzahl} Datei(en), {zeilen} Zeilen, groesste "
                           f"{groesste.relative_to(WURZEL).as_posix()} mit "
                           f"{neu.zeilen_zaehlen(neu.lies_text(groesste))}")
        zeile(not luecken, "M-69", f"{herkunft}: kein Unterbaum mit Quelldateien bleibt "
              f"ungemessen - gemessen {len(gemessen)} Dateien, ungemessen {len(luecken)} "
              f"Unterbaeume mit {sum(n for _, n in luecken)} Dateien")
        for d in details:
            print("             " + d)
        if args.revision:
            puffer = io.StringIO()
            with contextlib.redirect_stdout(puffer):
                code = modul.selbsttest()
            letzte = [z for z in puffer.getvalue().splitlines() if z.startswith("Selbsttest:")]
            print(f"  --      Selbsttest des Werkzeugs an {herkunft}: Exit {code}, "
                  f"{letzte[-1] if letzte else '?'} - kein Fall misst den Quellumfang, "
                  f"kein Riegel meldet die Luecke")


def main(argv: list[str]) -> int:
    os.chdir(WURZEL)
    # Die Zusagetexte tragen ±, − und ·: ohne UTF-8 bricht die Ausgabe in einer
    # umgeleiteten Windows-Konsole mit UnicodeEncodeError ab.
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    p = argparse.ArgumentParser()
    p.add_argument("riegel", choices=("M-66", "M-67", "M-68", "M-70",
                                      "basis-M-61-M-62", "basis-M-69"))
    p.add_argument("--revision", default=None)
    p.add_argument("--registerstand", action="store_true")
    args = p.parse_args(argv)
    {"M-66": riegel_m66, "M-67": riegel_m67, "M-68": riegel_m68, "M-70": riegel_m70,
     "basis-M-61-M-62": basis_m61_m62, "basis-M-69": basis_m69}[args.riegel](args)
    print(f"RIEGEL {args.riegel}: {_gesamt - _fehler} von {_gesamt} ok")
    return 0 if _fehler == 0 else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
