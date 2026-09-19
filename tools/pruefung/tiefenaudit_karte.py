#!/usr/bin/env python3
"""Agenten-Tiefenaudit: Bereichskarte pruefen, Befunde zuordnen, Auftraege rechnen.

    py -3.13 tools/pruefung/tiefenaudit_karte.py --pruefen
    py -3.13 tools/pruefung/tiefenaudit_karte.py --befunde [--json]
    py -3.13 tools/pruefung/tiefenaudit_karte.py --auftraege <zielordner> --sha <sha>
    py -3.13 tools/pruefung/tiefenaudit_karte.py --berichte <ordner>
    py -3.13 tools/pruefung/tiefenaudit_karte.py --betroffen <basis>..<ziel>
    py -3.13 tools/pruefung/tiefenaudit_karte.py --selbsttest

Quelle ist docs/gesundheit/tiefenaudit/karte.json (Bereiche, Naehte, Kopplungen).
Auftraege werden gerechnet, nie von Hand geschrieben; bekannte Problemstellen
kommen aus den Befundquellen der Karte, nicht aus der Karte selbst.
Spielregeln: docs/gesundheit/tiefenaudit/LIES-MICH.md. Nur Standardbibliothek,
kein Modell, kein Netz. Exit 0 = sauber, Exit 4 = Riss, Exit 2 = Werkzeugfehler.
"""
import argparse
import io
import itertools
import json
import os
import re
import subprocess
import sys

ORDNER = "docs/gesundheit/tiefenaudit"
KARTE = f"{ORDNER}/karte.json"
VORLAGEN = {"bereich": f"{ORDNER}/auftrag-bereich.md", "naht": f"{ORDNER}/auftrag-naht.md"}
GROSSE_DATEI = 2000
FUSSKOPF = "SCOPE-BEWEIS"


class Werkzeugfehler(Exception):
    pass


def muster_zu_regex(muster):
    """`**` trifft beliebig tief, `*` nur innerhalb eines Pfadteils."""
    teile = re.split(r"(\*\*|\*)", muster)
    aus = "".join(".*" if t == "**" else "[^/]*" if t == "*" else re.escape(t) for t in teile)
    return re.compile("^" + aus + "$")


def treffer(muster, dateien):
    rx = muster_zu_regex(muster)
    return [d for d in dateien if rx.match(d)]


def lies_text(pfad):
    return io.open(pfad, encoding="utf-8", errors="replace", newline="").read()


def getrackte_dateien():
    r = subprocess.run(["git", "ls-files", "-z"], capture_output=True)
    if r.returncode != 0:
        raise Werkzeugfehler("git ls-files schlug fehl: " + r.stderr.decode("utf-8", "replace"))
    return [p for p in r.stdout.decode("utf-8", "replace").split("\0") if p]


def zeilen_von_platte(pfad):
    try:
        with open(pfad, "rb") as f:
            return f.read().count(b"\n")
    except OSError:
        return 0


def bereichsdateien(karte, dateien):
    """id -> sortierte Dateiliste; dazu die Muster ohne Treffer."""
    je_bereich, leer = {}, []
    for b in karte["bereiche"]:
        menge = set()
        for m in b["pfade"]:
            t = treffer(m, dateien)
            if not t:
                leer.append((b["id"], m))
            menge.update(t)
        je_bereich[b["id"]] = sorted(menge)
    return je_bereich, leer


def pruefe(karte, dateien, zeilen):
    """Liefert (risse, hinweise, je_bereich). Rein: keine Platte, kein git."""
    risse, hinweise = [], []
    ids = [b["id"] for b in karte["bereiche"]] + [n["id"] for n in karte["naehte"]] \
        + [k["id"] for k in karte["kopplungen"]]
    for doppelt in sorted({i for i in ids if ids.count(i) > 1}):
        risse.append(f"Kennung doppelt: {doppelt}")
    je_bereich, leer = bereichsdateien(karte, dateien)
    for bid, m in leer:
        risse.append(f"{bid}: Pfad oder Muster ohne getrackte Datei: {m}")
    bekannt = set(karte["invarianten"])
    for b in karte["bereiche"]:
        for inv in b.get("invarianten", []):
            if inv not in bekannt:
                risse.append(f"{b['id']}: unbekannte Invariante {inv}")
        if b.get("spur") not in ("sammler", "sicherheit"):
            risse.append(f"{b['id']}: Spur muss sammler oder sicherheit sein")
        if not b.get("fragen"):
            risse.append(f"{b['id']}: keine Fragen")
    bereichs_ids = {b["id"] for b in karte["bereiche"]}
    for n in karte["naehte"]:
        for bid in n["bereiche"]:
            if bid not in bereichs_ids:
                risse.append(f"{n['id']}: Naht nennt unbekannten Bereich {bid}")
        if len(n["bereiche"]) < 2:
            risse.append(f"{n['id']}: eine Naht braucht mindestens zwei Bereiche")
    for k in karte["kopplungen"]:
        for m in k["wenn"] + k["dann_auch"]:
            if not treffer(m, dateien):
                risse.append(f"{k['id']}: Kopplungsende ohne getrackte Datei: {m}")
        if not k.get("beleg") or not k.get("warum"):
            risse.append(f"{k['id']}: Kopplung ohne Beleg oder Begruendung")
    # Pflichtabdeckung: jede Quelldatei der Wurzeln gehoert zu einem Bereich.
    pa = karte["pflichtabdeckung"]
    pflicht = set()
    for w in pa["wurzeln"]:
        pflicht.update(treffer(w, dateien))
    pflicht = {d for d in pflicht if os.path.splitext(d)[1].lower() in pa["endungen"]}
    for a in pa["ausgenommen"]:
        t = set(treffer(a["pfad"], dateien))
        if not t:
            risse.append(f"Ausnahme ohne getrackte Datei: {a['pfad']}")
        if not a.get("grund"):
            risse.append(f"Ausnahme ohne Grund: {a['pfad']}")
        pflicht -= t
    gedeckt = set(itertools.chain.from_iterable(je_bereich.values()))
    for d in sorted(pflicht - gedeckt):
        risse.append(f"ohne Bereich: {d}")
    budget = karte["zeilenbudget_je_bereich"]
    for bid, liste in je_bereich.items():
        summe = sum(zeilen(d) for d in liste)
        if summe > budget:
            risse.append(f"{bid}: {summe} Zeilen ueber dem Budget {budget}; Bereich teilen")
        for d in liste:
            if zeilen(d) > GROSSE_DATEI:
                hinweise.append(f"{bid}: grosse Datei {d} ({zeilen(d)} Zeilen); der Auftrag verlangt stueckweises Lesen")
    mehrfach = [d for d in gedeckt if sum(d in l for l in je_bereich.values()) > 1]
    for d in sorted(mehrfach):
        hinweise.append("in mehreren Bereichen (zulaessig, aber bewusst halten): " + d)
    return risse, hinweise, je_bereich


# --- Befunde -----------------------------------------------------------------

BEFUNDZEILE = re.compile(r"^\|\s*(T\d+-[A-Z0-9]+-\d+)\s*\|")  # T3-15-05 (Phase) oder T4-P02-03 (Bereich)
PFAD = re.compile(r"([A-Za-z0-9_.\-]+(?:/[A-Za-z0-9_.\-]+)+)")


def lies_befunde(quellen):
    befunde = {}
    for q in quellen:
        for zeile in lies_text(q).splitlines():
            m = BEFUNDZEILE.match(zeile)
            if not m:
                continue
            zellen = [z.strip() for z in zeile.strip().strip("|").split("|")]
            if len(zellen) < 5 or m.group(1) in befunde:
                continue  # Skeptiker- und Praezisierungszeilen wiederholen die Kennung
            orte = sorted({p for p in PFAD.findall(zellen[4])})
            befunde[m.group(1)] = {"schwere": zellen[1], "klasse": zellen[2],
                                   "kategorie": zellen[3], "orte": orte, "quelle": q}
    return befunde


def ordne_befunde(karte, dateien):
    je_bereich, _ = bereichsdateien(karte, dateien)
    datei_zu_bereich = {}
    for bid, liste in je_bereich.items():
        for d in liste:
            datei_zu_bereich.setdefault(d, []).append(bid)
    befunde = lies_befunde(karte["befundquellen"])
    stellen, naehte, ohne = {}, {}, []
    for kennung, b in befunde.items():
        bereiche = sorted({bid for o in b["orte"] for bid in datei_zu_bereich.get(o, [])})
        b["bereiche"] = bereiche
        if not bereiche:
            ohne.append(kennung)
        for bid in bereiche:
            stellen.setdefault(bid, []).append(kennung)
        for paar in itertools.combinations(bereiche, 2):
            naehte.setdefault(paar, []).append(kennung)
    return befunde, stellen, naehte, ohne


# --- Auftraege ---------------------------------------------------------------

def fuelle(vorlage, werte):
    aus = vorlage
    for k, v in werte.items():
        aus = aus.replace("{{" + k + "}}", v)
    rest = re.findall(r"\{\{[A-Z_]+\}\}", aus)
    if rest:
        raise Werkzeugfehler("Platzhalter ohne Wert: " + ", ".join(sorted(set(rest))))
    return aus


def liste(zeilen_):
    return "\n".join("- " + z for z in zeilen_) if zeilen_ else "- (keine)"


def rechne_auftraege(karte, dateien, ziel, sha):
    risse, _, je_bereich = pruefe(karte, dateien, zeilen_von_platte)
    if risse:
        raise Werkzeugfehler("Karte hat Risse; erst --pruefen sauber fahren")
    befunde, stellen, beob, _ = ordne_befunde(karte, dateien)
    vorl = {k: lies_text(v) for k, v in VORLAGEN.items()}
    os.makedirs(ziel, exist_ok=True)
    verzeichnis = []

    def kopplungen_fuer(pfade):
        aus = []
        for k in karte["kopplungen"]:
            enden = k["wenn"] + k["dann_auch"]
            if any(treffer(m, pfade) for m in enden):
                aus.append(f"{k['id']}: aendert sich {', '.join(k['wenn'])}, dann auch "
                           f"{', '.join(k['dann_auch'])}. {k['warum']} (Beleg: {k['beleg']})")
        return aus

    def stellen_fuer(bids):
        aus = []
        for kennung in sorted({x for bid in bids for x in stellen.get(bid, [])}):
            b = befunde[kennung]
            aus.append(f"{kennung} ({b['schwere']}, {b['kategorie']}, {b['klasse']}): "
                       + "; ".join(b["orte"][:4]) + f" [Wortlaut in {b['quelle']}]")
        return aus

    gemeinsam = {"SHA": sha, "NICHT_MELDEN": liste(karte["nicht_melden"])}
    for b in karte["bereiche"]:
        if b["spur"] != "sammler":
            continue
        pf = je_bereich[b["id"]]
        werte = dict(gemeinsam, ID=b["id"], NAME=b["name"], KLASSEN=", ".join(b["klassen"]),
                     DATEIEN=liste(f"{d} ({zeilen_von_platte(d)} Zeilen)" for d in pf),
                     DATEIZAHL=str(len(pf)),
                     INVARIANTEN=liste(f"{i}: {karte['invarianten'][i]}" for i in b["invarianten"]),
                     FRAGEN=liste(b["fragen"]), BEKANNTE_STELLEN=liste(stellen_fuer([b["id"]])),
                     KOPPLUNGEN=liste(kopplungen_fuer(pf)), BERICHT=f"{b['id']}.md")
        io.open(os.path.join(ziel, f"{b['id']}-auftrag.txt"), "w", encoding="utf-8",
                newline="\n").write(fuelle(vorl["bereich"], werte))
        verzeichnis.append({"id": b["id"], "art": "bereich", "name": b["name"], "dateien": len(pf)})
    for n in karte["naehte"]:
        pf = sorted({d for bid in n["bereiche"] for d in je_bereich[bid]})
        paare = [f"{a}+{c}: {', '.join(ids)}" for (a, c), ids in sorted(beob.items())
                 if a in n["bereiche"] and c in n["bereiche"]]
        werte = dict(gemeinsam, ID=n["id"], NAME=n["name"], KETTE=n["kette"], FRAGE=n["frage"],
                     BEREICHE=liste(f"{bid}: {next(b['name'] for b in karte['bereiche'] if b['id'] == bid)}"
                                    f" -> Bericht {bid}.md" for bid in n["bereiche"]),
                     BEKANNTE_STELLEN=liste(stellen_fuer(n["bereiche"])),
                     BEOBACHTETE_NAEHTE=liste(paare), KOPPLUNGEN=liste(kopplungen_fuer(pf)),
                     BERICHT=f"{n['id']}.md")
        io.open(os.path.join(ziel, f"{n['id']}-auftrag.txt"), "w", encoding="utf-8",
                newline="\n").write(fuelle(vorl["naht"], werte))
        verzeichnis.append({"id": n["id"], "art": "naht", "name": n["name"], "bereiche": n["bereiche"]})
    io.open(os.path.join(ziel, "verzeichnis.json"), "w", encoding="utf-8", newline="\n").write(
        json.dumps({"sha": sha, "auftraege": verzeichnis}, ensure_ascii=False, indent=1) + "\n")
    return verzeichnis


# --- Berichte ----------------------------------------------------------------

def pruefe_berichte(karte, dateien, ordner):
    _, _, je_bereich = pruefe(karte, dateien, lambda d: 0)
    risse = []
    erwartet = [(b["id"], len(je_bereich[b["id"]])) for b in karte["bereiche"] if b["spur"] == "sammler"]
    erwartet += [(n["id"], None) for n in karte["naehte"]]
    for kennung, zahl in erwartet:
        pfad = os.path.join(ordner, f"{kennung}.md")
        if not os.path.exists(pfad):
            risse.append(f"{kennung}: Bericht fehlt"); continue
        text = lies_text(pfad)
        if FUSSKOPF not in text:
            risse.append(f"{kennung}: kein {FUSSKOPF}; der Bericht gilt als nicht gelaufen"); continue
        fuss = text[text.rindex(FUSSKOPF):]
        feld = dict(re.findall(r"^([a-z_]+):\s*(.+?)\s*$", fuss, re.M))
        if feld.get("kennung") != kennung:
            risse.append(f"{kennung}: Fuss nennt Kennung {feld.get('kennung')!r}")
        if feld.get("fehlende_dateien") != "0":
            risse.append(f"{kennung}: fehlende_dateien = {feld.get('fehlende_dateien')!r}, verlangt 0")
        if zahl is not None and feld.get("dateien_gelesen") != str(zahl):
            risse.append(f"{kennung}: dateien_gelesen = {feld.get('dateien_gelesen')!r}, Karte verlangt {zahl}")
        if not re.search(rf"^FERTIG \| {re.escape(kennung)} \|", fuss, re.M):
            risse.append(f"{kennung}: Schlusszeile FERTIG fehlt (abgeschnittener Bericht)")
    return risse


# --- Betroffen ---------------------------------------------------------------

def betroffen(karte, dateien, geaendert):
    """Aus geaenderten Pfaden: beruehrte Bereiche, Naehte und offene Kopplungsenden.

    Ein offenes Ende ist kein Fehler, sondern eine Frage: x wurde geaendert, das
    gekoppelte y nicht. Rein: keine Platte, kein git.
    """
    je_bereich, _ = bereichsdateien(karte, dateien)
    direkt = sorted(bid for bid, l in je_bereich.items() if set(l) & set(geaendert))
    offen, gekoppelt = [], set()
    for k in karte["kopplungen"]:
        if not any(treffer(m, geaendert) for m in k["wenn"]):
            continue
        for ende in k["dann_auch"]:
            ziel = treffer(ende, dateien)
            gekoppelt.update(bid for bid, l in je_bereich.items() if set(l) & set(ziel))
            if not treffer(ende, geaendert):
                offen.append((k["id"], ende, k["warum"]))
    naehte = [n["id"] for n in karte["naehte"]
              if len(set(n["bereiche"]) & (set(direkt) | gekoppelt)) >= 2 and set(n["bereiche"]) & set(direkt)]
    return direkt, sorted(gekoppelt - set(direkt)), naehte, offen


# --- Selbsttest --------------------------------------------------------------

def selbsttest():
    dateien = ["a/x.cpp", "a/y.cpp", "b/z.rs", "b/gross.rs"]
    zeilen = {"a/x.cpp": 10, "a/y.cpp": 10, "b/z.rs": 10, "b/gross.rs": 9000}.get

    def karte(**aender):
        k = {"invarianten": {"I": "t"}, "zeilenbudget_je_bereich": 10000,
             "pflichtabdeckung": {"wurzeln": ["a/**", "b/**"], "endungen": [".cpp", ".rs"], "ausgenommen": []},
             "bereiche": [{"id": "A", "spur": "sammler", "invarianten": ["I"], "fragen": ["f"], "pfade": ["a/**"]},
                          {"id": "B", "spur": "sammler", "invarianten": ["I"], "fragen": ["f"], "pfade": ["b/z.rs", "b/gross.rs"]}],
             "naehte": [{"id": "N", "bereiche": ["A", "B"]}],
             "kopplungen": [{"id": "K", "wenn": ["a/x.cpp"], "dann_auch": ["b/z.rs"], "warum": "w", "beleg": "b"}]}
        k.update(aender)
        return k

    faelle = [
        ("saubere Karte", karte(), None),
        ("toter Pfad", karte(bereiche=[dict(karte()["bereiche"][0], pfade=["a/**", "a/weg.cpp"]), karte()["bereiche"][1]]), "ohne getrackte Datei"),
        ("Datei ohne Bereich", karte(bereiche=[dict(karte()["bereiche"][0], pfade=["a/x.cpp"]), karte()["bereiche"][1]]), "ohne Bereich: a/y.cpp"),
        ("doppelte Kennung", karte(naehte=[{"id": "A", "bereiche": ["A", "B"]}]), "Kennung doppelt: A"),
        ("Naht mit unbekanntem Bereich", karte(naehte=[{"id": "N", "bereiche": ["A", "Q"]}]), "unbekannten Bereich Q"),
        ("Kopplung ohne Beleg", karte(kopplungen=[{"id": "K", "wenn": ["a/x.cpp"], "dann_auch": ["b/z.rs"], "warum": "w", "beleg": ""}]), "ohne Beleg"),
        ("Budget gerissen", karte(zeilenbudget_je_bereich=5000), "ueber dem Budget"),
    ]
    fehl = 0
    for name, k, erwartet in faelle:
        risse, _, _ = pruefe(k, dateien, zeilen)
        gut = (not risse) if erwartet is None else any(erwartet in r for r in risse)
        print(("ok  " if gut else "ROT ") + name + ("" if gut else f" -> {risse}"))
        fehl += 0 if gut else 1
    gut = muster_zu_regex("a/*/c.h").match("a/b/c.h") and not muster_zu_regex("a/*/c.h").match("a/b/d/c.h") \
        and muster_zu_regex("a/**").match("a/b/d/c.h")
    print(("ok  " if gut else "ROT ") + "Mustergrenzen * und **")
    fehl += 0 if gut else 1
    zeile = "| T9-01-02 | high | K4 | DEFEKT | a/x.cpp:12; b/z.rs:3 | Text | Beleg | Bricht | Vorschlag | offen |"
    m = BEFUNDZEILE.match(zeile)
    gut = bool(m) and PFAD.findall(zeile.split("|")[5]) == ["a/x.cpp", "b/z.rs"]
    print(("ok  " if gut else "ROT ") + "Befundzeile und Orte")
    fehl += 0 if gut else 1
    direkt, gek, naehte, offen = betroffen(karte(), dateien, ["a/x.cpp"])
    gut = direkt == ["A"] and gek == ["B"] and naehte == ["N"] and [o[:2] for o in offen] == [("K", "b/z.rs")]
    print(("ok  " if gut else "ROT ") + "betroffen: x geaendert, gekoppeltes Ende offen")
    fehl += 0 if gut else 1
    direkt, gek, naehte, offen = betroffen(karte(), dateien, ["a/x.cpp", "b/z.rs"])
    gut = direkt == ["A", "B"] and not offen
    print(("ok  " if gut else "ROT ") + "betroffen: beide Enden geaendert, nichts offen")
    fehl += 0 if gut else 1
    return fehl


def main():
    p = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    p.add_argument("--pruefen", action="store_true")
    p.add_argument("--befunde", action="store_true")
    p.add_argument("--json", action="store_true")
    p.add_argument("--auftraege")
    p.add_argument("--sha")
    p.add_argument("--berichte")
    p.add_argument("--betroffen", metavar="BASIS..ZIEL")
    p.add_argument("--selbsttest", action="store_true")
    a = p.parse_args()
    try:
        if a.selbsttest:
            fehl = selbsttest()
            print("TIEFENAUDIT_SELBSTTEST: " + ("PASS" if not fehl else f"ROT ({fehl})"))
            return 0 if not fehl else 4
        karte = json.loads(lies_text(KARTE))
        dateien = getrackte_dateien()
        if a.pruefen:
            risse, hinweise, je_bereich = pruefe(karte, dateien, zeilen_von_platte)
            for b in karte["bereiche"]:
                l = je_bereich[b["id"]]
                print(f"{b['id']:<4} {len(l):>3} Dateien {sum(zeilen_von_platte(d) for d in l):>6} Zeilen  {b['spur']:<10} {b['name']}")
            print(f"Bereiche {len(karte['bereiche'])} · Naehte {len(karte['naehte'])} · Kopplungen {len(karte['kopplungen'])}")
            for h in hinweise:
                print("HINWEIS " + h)
            for r in risse:
                print("RISS    " + r)
            print("TIEFENAUDIT_VORPRUEFUNG: " + ("PASS" if not risse else f"RISS ({len(risse)})"))
            return 0 if not risse else 4
        if a.befunde:
            befunde, stellen, naehte, ohne = ordne_befunde(karte, dateien)
            if a.json:
                print(json.dumps({"befunde": befunde, "je_bereich": stellen, "ohne_bereich": ohne,
                                  "beobachtete_naehte": {"+".join(k): v for k, v in naehte.items()}},
                                 ensure_ascii=False, indent=1))
                return 0
            print(f"Befunde {len(befunde)} aus {len(karte['befundquellen'])} Quelle(n)")
            for b in karte["bereiche"]:
                ids = stellen.get(b["id"], [])
                print(f"{b['id']:<4} {len(ids):>3}  {' '.join(ids)}")
            print("Beobachtete Naehte (ein Befund nennt Orte in zwei Bereichen):")
            for (x, y), ids in sorted(naehte.items(), key=lambda kv: -len(kv[1])):
                print(f"  {x}+{y:<4} {len(ids):>2}  {' '.join(ids)}")
            print(f"Ohne Bereich ({len(ohne)}): {' '.join(ohne)}")
            return 0
        if a.auftraege:
            if not a.sha:
                raise Werkzeugfehler("--auftraege verlangt --sha (der festgehaltene Stand des Laufs)")
            v = rechne_auftraege(karte, dateien, a.auftraege, a.sha)
            print(f"{len(v)} Auftraege nach {a.auftraege} geschrieben (Stand {a.sha})")
            return 0
        if a.betroffen:
            r = subprocess.run(["git", "diff", "--name-only", "-z", a.betroffen], capture_output=True)
            if r.returncode != 0:
                raise Werkzeugfehler("git diff schlug fehl: " + r.stderr.decode("utf-8", "replace").strip())
            geaendert = [x for x in r.stdout.decode("utf-8", "replace").split("\0") if x]
            direkt, gek, naehte, offen = betroffen(karte, dateien, geaendert)
            name = {b["id"]: b["name"] for b in karte["bereiche"]}
            print(f"{a.betroffen}: {len(geaendert)} geaenderte Pfade")
            for bid in direkt:
                print(f"BERUEHRT  {bid}  {name[bid]}")
            for bid in gek:
                print(f"GEKOPPELT {bid}  {name[bid]}")
            for nid in naehte:
                print(f"NAHT      {nid}")
            for kid, ende, warum in offen:
                print(f"OFFEN     {kid}: {ende} nicht mitgeaendert. {warum}")
            print("TIEFENAUDIT_BETROFFEN: " + (f"{len(offen)} offene Kopplungsenden" if offen else "kein offenes Kopplungsende"))
            return 0
        if a.berichte:
            risse = pruefe_berichte(karte, dateien, a.berichte)
            for r in risse:
                print("RISS    " + r)
            print("TIEFENAUDIT_BERICHTE: " + ("PASS" if not risse else f"RISS ({len(risse)})"))
            return 0 if not risse else 4
        p.print_help()
        return 2
    except (Werkzeugfehler, OSError, ValueError, KeyError) as e:
        print(f"WERKZEUGFEHLER: {e}")
        return 2


if __name__ == "__main__":
    sys.exit(main())
