#!/usr/bin/env py -3.13
"""NAK-309 Etappe 4 (T3-02-07, M-61) - Feldzaehlung der Abdeckungskarte K4.

WAS GEZAEHLT WIRD

  Die Zaehlregel der Karte (docs/gesundheit/abdeckungskarte.md, K4; Tiefenaudit
  Phase 2, Abschnitt "Zaehlregel"): jeder benannte Property-Ort im Schema-2-Baum,
  Wurzel 1 + Common 7 + MainProject 8 + Parameters 114 + Dsp 6 = 136. Je Feld
  zaehlt, ob mindestens ein eingefrorenes Fixture unter eq-copilot/fixtures/state/
  die Eigenschaft an genau diesem Ort traegt.

ZAEHLWEG

  Jedes Byte-Bild eq-copilot/fixtures/state/schema2/*.bin wird als JUCE-ValueTree
  geladen - kein Textvergleich an den Bytes. Das Format ist an der JUCE-Quelle
  gelesen (eq-copilot/build/_deps/juce-src, JUCE 8.0.9):
    juce_ValueTree.cpp  SharedObject::writeToStream: Typname (writeString),
                        Eigenschaftszahl (writeCompressedInt), je Eigenschaft Name
                        (writeString) und Wert (var::writeToStream), Kinderzahl
                        (writeCompressedInt), jedes Kind rekursiv
    juce_OutputStream.cpp  writeString: UTF-8 mit abschliessendem NUL;
                        writeCompressedInt: ein Laengenbyte (Bit 7 = negativ),
                        danach so viele Bytes little-endian
    juce_Variant.cpp    jeder Wert beginnt mit writeCompressedInt(Laenge), danach
                        genau so viele Bytes (Marker und Daten); void hat Laenge 0
  Eine Datei, die nicht genau bis zu ihrem letzten Byte aufgeht, ist ein
  Werkzeugfehler (Exit 2), nie eine Zahl.

  Die JSON-Dateien unter jcs/, dto/ und preset/ sind keine State-v2-Byte-Bilder
  (JCS-Dokumente, Parameter-DTO und Preset sind eigene Vertraege); sie werden je
  Ordner mit ihrer Anzahl genannt und zaehlen nicht.

POPULATION

  Wurzel, Common, MainProject und Dsp: die Namen unten, jeder beim Lauf gegen
  seine juce::Identifier-Konstante in eq-copilot/plugin/state/NakamaState.cpp
  geprueft (fehlt eine, Exit 2). Parameters: schema, dsp_schema_version und die
  ersten anzahl_host_parameter Kennungen aus
  eq-copilot/schemas/state/nakama-parameter-v2.json ("ids").

AUFRUF (vom Workspace-Root)

  py -3.13 docs/beweise/roh/NAK-309-etappe4-feldzaehlung.py
  py -3.13 docs/beweise/roh/NAK-309-etappe4-feldzaehlung.py --ohne schema2/main-binding-v1.bin
  ... --roh docs/beweise/roh/NAK-309-etappe4-feldzaehlung.txt --lauf "vorher"

  --ohne laesst ein Byte-Bild weg (Rotbeweis M-61). --roh haengt die Ausgabe als
  Abschnitt an die Rohdatei an (legt sie an, wenn sie fehlt).

EXIT

  0 gezaehlt und die Karte nennt dieselbe Zahl; 4 gezaehlt, aber Karte oder
  KONZEPT nennen eine andere Zahl; 2 Werkzeugfehler.
"""

from __future__ import annotations

import argparse
import datetime
import hashlib
import json
import pathlib
import re
import subprocess
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[3]
FIXTURES = WURZEL / "eq-copilot" / "fixtures" / "state"
STATE_CPP = WURZEL / "eq-copilot" / "plugin" / "state" / "NakamaState.cpp"
VERTRAG = WURZEL / "eq-copilot" / "schemas" / "state" / "nakama-parameter-v2.json"
KARTE = WURZEL / "docs" / "gesundheit" / "abdeckungskarte.md"
KONZEPT = WURZEL / "docs" / "gesundheit" / "KONZEPT.md"

# Knoten -> (Identifier-Konstante, Name) in der Reihenfolge des Writers.
FESTE_ORTE: dict[str, list[tuple[str, str]]] = {
    "NakamaState": [("kSchema", "schema")],
    "Common": [("kSchema", "schema"), ("kInstanceId", "instance_id"), ("kKind", "plugin_kind"),
               ("kPosition", "measurement_position"), ("kLabel", "label"), ("kPairId", "pair_id"),
               ("kBinding", "project_binding_id")],
    "MainProject": [("kSchema", "schema"), ("kMainMitglieder", "confirmed_members_v1"),
                    ("kMainPassagen", "manual_passages_v1"), ("kMainIntents", "source_intents_v1"),
                    ("kMainSchutz", "intent_protections_v1"), ("kMainBeziehungen", "intent_relations_v1"),
                    ("kMainIntentRev", "intent_revision_v1"), ("kMainAssistent", "assistant_step_v1")],
    "Dsp": [("kSchema", "schema"), ("kDspRevision", "state_revision"), ("kDspOccupied", "occupied_v1"),
            ("kDspZonen", "schutz_zonen_v1"), ("kDspUndoRing", "undo_ring_v1"), ("kDspUndoCursor", "undo_cursor")],
}
KNOTEN = ("NakamaState", "Common", "MainProject", "Parameters", "Dsp")
WURZELTYP = "NakamaState"
KINDTYPEN = ("Common", "MainProject", "Parameters", "Dsp")


class Werkzeugfehler(Exception):
    pass


# -- JUCE-ValueTree-Leser ----------------------------------------------------------


class Leser:
    def __init__(self, daten: bytes, name: str):
        self.d = daten
        self.i = 0
        self.name = name

    def fehler(self, text: str) -> Werkzeugfehler:
        return Werkzeugfehler(f"{self.name}: {text} (Offset {self.i})")

    def byte(self) -> int:
        if self.i >= len(self.d):
            raise self.fehler("Datei endet vorzeitig")
        b = self.d[self.i]
        self.i += 1
        return b

    def compressed_int(self) -> int:
        kopf = self.byte()
        anzahl = kopf & 0x7F
        if anzahl > 4:
            raise self.fehler(f"compressed int mit {anzahl} Bytes")
        wert = 0
        for k in range(anzahl):
            wert |= self.byte() << (8 * k)
        return -wert if kopf & 0x80 else wert

    def string(self) -> str:
        ende = self.d.find(b"\x00", self.i)
        if ende < 0:
            raise self.fehler("String ohne NUL")
        roh = self.d[self.i:ende]
        self.i = ende + 1
        try:
            return roh.decode("utf-8")
        except UnicodeDecodeError:
            raise self.fehler("String nicht UTF-8") from None

    def var_ueberspringen(self) -> int:
        laenge = self.compressed_int()
        if laenge < 0 or self.i + laenge > len(self.d):
            raise self.fehler(f"var-Laenge {laenge} ausserhalb der Datei")
        marker = self.d[self.i] if laenge > 0 else None
        self.i += laenge
        return -1 if marker is None else marker

    def knoten(self, tiefe: int = 0) -> dict:
        if tiefe > 64:
            raise self.fehler("Baum tiefer als 64")
        typ = self.string()
        anzahl = self.compressed_int()
        if anzahl < 0:
            raise self.fehler("negative Eigenschaftszahl")
        props: list[str] = []
        marker: dict[str, int] = {}
        for _ in range(anzahl):
            name = self.string()
            marker[name] = self.var_ueberspringen()
            props.append(name)
        kinder_zahl = self.compressed_int()
        if kinder_zahl < 0:
            raise self.fehler("negative Kinderzahl")
        kinder = [self.knoten(tiefe + 1) for _ in range(kinder_zahl)]
        return {"typ": typ, "props": props, "marker": marker, "kinder": kinder}


def lade_baum(pfad: pathlib.Path) -> dict:
    daten = pfad.read_bytes()
    lesen = Leser(daten, pfad.relative_to(FIXTURES).as_posix())
    baum = lesen.knoten()
    if lesen.i != len(daten):
        raise lesen.fehler(f"{len(daten) - lesen.i} Bytes hinter dem Wurzelknoten")
    return baum


# -- Population --------------------------------------------------------------------


def population() -> dict[str, list[str]]:
    quelle = STATE_CPP.read_text(encoding="utf-8")
    for knoten, orte in FESTE_ORTE.items():
        for konstante, name in orte:
            muster = re.compile(r"juce::Identifier\s+" + re.escape(konstante) + r'\s*\("' + re.escape(name) + r'"\)')
            if not muster.search(quelle):
                raise Werkzeugfehler(f"NakamaState.cpp nennt {konstante} nicht als \"{name}\" ({knoten})")
    vertrag = json.loads(VERTRAG.read_text(encoding="utf-8"))
    host = int(vertrag["anzahl_host_parameter"])
    ids = list(vertrag["ids"])
    if len(ids) < host:
        raise Werkzeugfehler("Parametervertrag nennt weniger ids als anzahl_host_parameter")
    pop = {k: [n for _, n in FESTE_ORTE[k]] for k in ("NakamaState", "Common", "MainProject")}
    pop["Parameters"] = ["schema", "dsp_schema_version"] + ids[:host]
    pop["Dsp"] = [n for _, n in FESTE_ORTE["Dsp"]]
    for k, namen in pop.items():
        if len(set(namen)) != len(namen):
            raise Werkzeugfehler(f"Population {k} nennt einen Namen doppelt")
    return pop


# -- Karte und KONZEPT ---------------------------------------------------------------


def zahl_karte() -> int | None:
    text = KARTE.read_text(encoding="utf-8")
    t = re.search(r"\*\*Population:\*\*\s*State v2 mit\s+(\d+)\s+persistenten Feldern.*?(\d+)\s+mit\s+eingefrorenem\s+"
                  r"Fixture", text, re.S)
    return int(t.group(2)) if t else None


def zahl_konzept() -> int | None:
    text = KONZEPT.read_text(encoding="utf-8")
    t = re.search(r"StateMigrationTest\s*\((\d+)\s+von\s+(\d+)\s+Feldern\s+mit\s+eingefrorenem\s+Fixture", text)
    return int(t.group(1)) if t else None


# -- Lauf --------------------------------------------------------------------------


def zaehle(ohne: list[str]) -> tuple[int, list[str]]:
    z: list[str] = []
    pop = population()
    gesamt = sum(len(v) for v in pop.values())
    z.append(f"Population: {gesamt} Felder (" + ", ".join(f"{k} {len(pop[k])}" for k in KNOTEN) + ")")
    getragen: dict[str, set[str]] = {k: set() for k in KNOTEN}
    fremd: list[str] = []
    bins = sorted(p for p in (FIXTURES / "schema2").glob("*.bin") if p.is_file())
    weggelassen = [b for b in bins if b.relative_to(FIXTURES).as_posix() in ohne]
    unbekannt_ohne = [o for o in ohne if not (FIXTURES / o).is_file()]
    if unbekannt_ohne:
        raise Werkzeugfehler(f"--ohne nennt keine Datei: {', '.join(unbekannt_ohne)}")
    z.append("")
    z.append(f"Byte-Bilder unter eq-copilot/fixtures/state/schema2/: {len(bins)}"
             + (f", davon weggelassen (--ohne): {', '.join(b.name for b in weggelassen)}" if weggelassen else ""))
    for b in bins:
        rel = b.relative_to(FIXTURES).as_posix()
        daten = b.read_bytes()
        kennung = f"  {rel}  {len(daten)} Bytes  SHA-256 {hashlib.sha256(daten).hexdigest().upper()}"
        if b in weggelassen:
            z.append(kennung + "  WEGGELASSEN")
            continue
        baum = lade_baum(b)
        if baum["typ"] != WURZELTYP:
            z.append(kennung + f"  Wurzel {baum['typ']!r} - kein State-v2-Baum, zaehlt nicht")
            continue
        teile = [f"NakamaState {len(baum['props'])}"]
        for name in baum["props"]:
            (getragen["NakamaState"].add(name) if name in pop["NakamaState"]
             else fremd.append(f"{rel}: NakamaState.{name}"))
        for kind in baum["kinder"]:
            if kind["typ"] not in KINDTYPEN:
                fremd.append(f"{rel}: Kind {kind['typ']} (kein Knoten der Population)")
                continue
            teile.append(f"{kind['typ']} {len(kind['props'])}")
            for name in kind["props"]:
                (getragen[kind["typ"]].add(name) if name in pop[kind["typ"]]
                 else fremd.append(f"{rel}: {kind['typ']}.{name}"))
            if kind["kinder"]:
                fremd.append(f"{rel}: {kind['typ']} traegt {len(kind['kinder'])} Kind(er) (nicht Teil der Population)")
        z.append(kennung + "  Eigenschaften je Knoten: " + ", ".join(teile))
    json_dateien = {o: sum(1 for p in (FIXTURES / o).rglob("*") if p.is_file()) for o in ("jcs", "dto", "preset")}
    z.append("Nicht gezaehlt (keine State-v2-Byte-Bilder, eigene Vertraege): "
             + ", ".join(f"{o}/ {n} Datei(en)" for o, n in json_dateien.items()) + ", MANIFEST.json")
    z.append("")
    summe = 0
    fehlend: list[str] = []
    for k in KNOTEN:
        n = sum(1 for name in pop[k] if name in getragen[k])
        summe += n
        fehlend += [f"{k}.{name}" for name in pop[k] if name not in getragen[k]]
        z.append(f"  {k:<12} {n:>3} von {len(pop[k]):>3} Feldern mit eingefrorenem Fixture")
    z.append(f"  {'Summe':<12} {summe:>3} von {gesamt:>3}")
    z.append("Ohne eingefrorenes Fixture: " + (", ".join(fehlend) if fehlend else "keines"))
    z.append("Eigenschaften oder Kinder ausserhalb der Population (nicht gezaehlt): "
             + ("; ".join(fremd) if fremd else "keine"))
    return summe, z


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description="NAK-309 Etappe 4 (M-61): Feldzaehlung der Abdeckungskarte K4")
    p.add_argument("--ohne", action="append", default=[], metavar="REL",
                   help="dieses Byte-Bild (relativ zu eq-copilot/fixtures/state/) weglassen")
    p.add_argument("--roh", type=pathlib.Path, help="Ausgabe als Abschnitt an diese Rohdatei anhaengen")
    p.add_argument("--lauf", default="", help="Name des Abschnitts in der Rohdatei")
    a = p.parse_args(argv)
    z: list[str] = []
    kopf = subprocess.run(["git", "-C", str(WURZEL), "rev-parse", "--short=8", "HEAD"], capture_output=True,
                          text=True).stdout.strip()
    werkzeug = hashlib.sha256(pathlib.Path(__file__).read_bytes()).hexdigest().upper()
    z.append(f"== Feldzaehlung K4{(' - ' + a.lauf) if a.lauf else ''} ==")
    z.append(f"Stand: {datetime.datetime.now():%Y-%m-%d %H:%M:%S}, HEAD {kopf}; Werkzeug "
             f"docs/beweise/roh/NAK-309-etappe4-feldzaehlung.py (SHA-256 {werkzeug})")
    z.append("Aufruf: py -3.13 docs/beweise/roh/NAK-309-etappe4-feldzaehlung.py"
             + "".join(f" --ohne {o}" for o in a.ohne))
    try:
        summe, zeilen = zaehle(a.ohne)
    except (Werkzeugfehler, OSError, ValueError, KeyError) as f:
        z.append(f"WERKZEUGFEHLER - {type(f).__name__}: {f}")
        print("\n".join(z))
        return 2
    z += zeilen
    karte, konzept = zahl_karte(), zahl_konzept()
    z.append("")
    z.append(f"Karte (docs/gesundheit/abdeckungskarte.md, K4 Population) nennt: {karte}; "
             f"KONZEPT (docs/gesundheit/KONZEPT.md, StateMigrationTest) nennt: {konzept}; gezaehlt: {summe}")
    widerspruch = karte != summe or konzept != summe
    z.append(f"Karte oder KONZEPT widerspricht der Zaehlung: {'JA' if widerspruch else 'NEIN'}")
    code = 4 if widerspruch else 0
    z.append(f"Exit {code}")
    text = "\n".join(z) + "\n"
    print(text, end="")
    if a.roh:
        ziel = a.roh if a.roh.is_absolute() else WURZEL / a.roh
        with open(ziel, "a", encoding="utf-8", newline="\n") as f:
            if ziel.stat().st_size if ziel.exists() else 0:
                f.write("\n")
            f.write(text)
    return code


if __name__ == "__main__":
    sys.exit(main())
