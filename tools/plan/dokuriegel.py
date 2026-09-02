#!/usr/bin/env python3
"""Dokuriegel — zwei Riegel gegen die zwei Fehlerklassen, die Plandokumente still verderben.

Aufruf vom Workspace-Root:

    py -3.13 tools/plan/dokuriegel.py <datei> [<datei> ...]

Exitcode 1, sobald ein Befund steht; 0, wenn alles sauber ist. Nur Standardbibliothek.

RIEGEL 1 — TABELLEN
-------------------
Jede Markdown-Tabelle traegt durchgehend gleich viele Spalten. Die Klasse Fehler,
die das findet, entsteht durch ein ungeschuetztes `|` in einer Zelle: die Zeile
bekommt eine Spalte zu viel, der Renderer schiebt den Rest weg, und der Inhalt
verschwindet aus der Ansicht, ohne dass irgendetwas rot wird. Muster:
`docs/beweise/PR1.md` Paragraph 9.4 F (30.08.2026) meldete genau so fuenf Zeilen
in `docs/offene-punkte.md`. Ein `|` in einer Zelle wird mit Backslash geschuetzt.

Gezaehlt wird an nicht-escapten Pipes; `\\|` gilt als Inhalt, nicht als Trenner.
Die Spaltenzahl der ERSTEN Zeile eines Tabellenblocks ist das Mass. Ein
Tabellenblock ist eine ununterbrochene Folge von Zeilen, die mit `|` beginnen.
Codebloecke (```) werden uebersprungen: dort ist `|` gewoehnlicher Text.

RIEGEL 2 — VERWEISE
-------------------
Jeder repo-relative Pfadverweis zeigt auf eine existierende Datei oder ein
existierendes Verzeichnis. Erkannt werden zwei Formen:

  * Markdown-Links `[text](pfad)`, die NICHT in Backticks stehen — aufgeloest
    relativ zum Ordner der Datei. Ein Link in Backticks ist zitierter Code, kein
    lebender Verweis;
  * vollstaendige Backtick-Spannen, die mit einem der bekannten Wurzelordner
    beginnen (`docs/`, `design/`, `eq-copilot/`, `broker/`, `tools/`, `wissen/`,
    `.claude/`, `.agents/`) — aufgeloest relativ zum Workspace-Root. Die ganze
    Spanne zaehlt, damit Pfade mit Leerzeichen nicht mitten im Namen abreissen.

Eingezaeunte Codebloecke (```) sind fuer BEIDE Riegel aussen vor: dort steht roh
eingefuegte Werkzeugausgabe. Ein Pfad darin ist Zitat, kein lebender Verweis —
ein Manifest, das den Rotlauf eines Riegels dokumentiert, wuerde sonst an seiner
eigenen Beweisausgabe scheitern.

Nicht geprueft werden URLs (`http:`, `https:`, `mailto:`), reine Anker (`#...`),
Windows-Pfade mit Laufwerksbuchstaben und alles, was erkennbar ein **Muster**
statt eines Verweises ist: ein Pfad mit `*`, `?`, `<`, `>`, `{`, `}` oder `…`
(Glob, Brace-Ausdruck, Auslassung) und ein Pfad, der auf `-` oder `_` endet
(im Text abgeschnitten). Vorlagenplatzhalter wie `SONDE-0NN` fallen unter
dieselbe Regel.

AUSNAHME, ausdruecklich und eng gefasst
---------------------------------------
Ein Verweis wird **benannt statt gezaehlt**, wenn in seinem Umfeld — 200 Zeichen
davor bis 200 Zeichen danach, innerhalb derselben Datei — mindestens eines
dieser Woerter steht:

    Verlauf · historisch · Historie · Archiv · archiviert · geparkt ·
    stillgelegt · ueberholt/überholt · gestrichen · Grabstein · Studie ·
    entfallen · abgeschafft · gelöscht/geloescht · entfernt ·
    „nicht in diesem Repo" · „im FL-Studio-Repo" · „im Design-Repo" · Vorlage

Begruendung: ein Plandokument darf sagen „diese Quelle ist Verlauf" und dabei
ihren Pfad nennen, auch wenn die Datei laengst weg ist. Das ist keine kaputte
Referenz, sondern eine Spurangabe. Der Riegel listet solche Faelle trotzdem auf,
damit sie sichtbar bleiben — sie erhoehen nur die Befundzahl nicht.

Dieselbe Ausnahme gilt spiegelbildlich fuer den ZIELPFAD: ein Pfad, der im
Umfeld ausdruecklich als noch nicht vorhanden bezeichnet wird —

    „noch nicht angelegt" · „anzulegen" · „Zielpfad" · „nicht ins Repo"

— beschreibt Arbeit, die erst entstehen soll. Auch er wird benannt, nicht
gezaehlt, und in der Ausgabe von der historischen Klasse getrennt gehalten.

Eine Markierung gilt fuer die ganze Datei, nicht nur fuer die eine Fundstelle:
ein Dokument erklaert den Status eines Pfades einmal und nennt ihn danach
weiter. Trifft die Markierung an KEINER Stelle zu, ist der Verweis ein Befund.

Das Fenster ist bewusst ein Zeichenfenster und keine Zeile: in `plan.json` steht
ein ganzer Schritttext als EIN String, eine Zeilenregel waere dort wirkungslos
und eine Ganzstring-Regel viel zu grob.
"""

from __future__ import annotations

import json
import re
import sys
from pathlib import Path

WURZEL = Path(__file__).resolve().parents[2]
BS = chr(92)

WURZELORDNER = ("docs/", "design/", "eq-copilot/", "broker/", "tools/",
                "wissen/", ".claude/", ".agents/")

ZIELPFAD = ("noch nicht angelegt", "anzulegen", "Zielpfad", "nicht ins Repo")

HISTORISCH = ("Verlauf", "historisch", "Historie", "Archiv", "archiviert",
              "geparkt", "stillgelegt", "ueberholt", "überholt", "gestrichen",
              "Grabstein", "Studie", "entfallen", "abgeschafft", "gelöscht",
              "geloescht", "entfernt", "nicht in diesem Repo",
              "im FL-Studio-Repo", "im Design-Repo", "Vorlage")

FENSTER = 200


# --------------------------------------------------------------------- Riegel 1

def zellen(zeile: str) -> list[str]:
    """Zerlegt eine Markdown-Tabellenzeile an nicht-escapten Pipes."""
    teile: list[str] = []
    akt: list[str] = []
    i = 0
    while i < len(zeile):
        c = zeile[i]
        if c == BS and i + 1 < len(zeile):
            akt.append(c)
            akt.append(zeile[i + 1])
            i += 2
            continue
        if c == "|":
            teile.append("".join(akt))
            akt = []
            i += 1
            continue
        akt.append(c)
        i += 1
    teile.append("".join(akt))
    return teile


def tabellen_pruefen(pfad: Path, text: str) -> list[str]:
    if pfad.suffix.lower() != ".md":
        return []
    befunde: list[str] = []
    zeilen = text.split("\n")
    im_codeblock = False
    block_mass: int | None = None
    block_start = 0
    for nr, zeile in enumerate(zeilen, 1):
        if zeile.lstrip().startswith("```"):
            im_codeblock = not im_codeblock
            block_mass = None
            continue
        if im_codeblock:
            block_mass = None
            continue
        if not zeile.startswith("|"):
            block_mass = None
            continue
        anzahl = len(zellen(zeile))
        if block_mass is None:
            block_mass = anzahl
            block_start = nr
            continue
        if anzahl != block_mass:
            befunde.append(
                f"{pfad.as_posix()}:{nr} TABELLE UNEINHEITLICH: "
                f"{anzahl} Spaltentrenner statt {block_mass} "
                f"(Tabelle ab Zeile {block_start})")
    return befunde


# --------------------------------------------------------------------- Riegel 2

MD_LINK = re.compile(r"]\(([^)\s]+)\)")
BACKTICK = re.compile("`([^`" + chr(92) + "n]+)`")
MUSTERZEICHEN = "<>*?{}…"


def ist_ueberspringbar(ziel: str) -> bool:
    if not ziel or ziel.startswith("#"):
        return True
    if ziel.startswith(("http://", "https://", "mailto:", "//")):
        return True
    if re.match(r"^[A-Za-z]:", ziel):
        return True
    if any(z in ziel for z in MUSTERZEICHEN):
        return True
    if ziel.endswith(("-", "_")):
        return True
    if VORLAGE.search(ziel):
        return True            # Vorlagenplatzhalter wie `SONDE-0NN`
    return False


FUNDSTELLE = re.compile(r":\d+(?:[-,]\d+)*$")
VORLAGE = re.compile(r"\bN{2,}\b|0NN")


def saeubern(ziel: str) -> str:
    ziel = ziel.split("#", 1)[0].split("?", 1)[0]
    ziel = ziel.rstrip(".,;:)»\"'`")
    # Fundstellen wie `datei.cpp:820-830` oder `schema.json:205,252` benennen
    # eine Stelle IN der Datei, nicht eine andere Datei.
    ziel = FUNDSTELLE.sub("", ziel)
    return ziel


def umfeld_markiert(text: str, pos: int) -> tuple[str, str] | None:
    """(Klasse, Wort) der Ausnahme im Umfeld — oder None."""
    umfeld = text[max(0, pos - FENSTER): pos + FENSTER]
    for wort in ZIELPFAD:
        if wort in umfeld:
            return ("ZIELPFAD", wort)
    for wort in HISTORISCH:
        if wort in umfeld:
            return ("HISTORISCH", wort)
    return None


def codebloecke(text: str) -> list[tuple[int, int]]:
    """Spannen eingezaeunter Codebloecke (```), als (start, ende)."""
    grenzen = [m.start() for m in re.finditer(r"^```", text, re.M)]
    return list(zip(grenzen[0::2], grenzen[1::2]))


def verweise_sammeln(pfad: Path, text: str) -> list[tuple[str, int]]:
    """Alle Kandidaten als (Pfadangabe, Position im Text)."""
    treffer: list[tuple[str, int]] = []
    in_backticks: list[tuple[int, int]] = codebloecke(text)
    fenced = list(in_backticks)
    for m in BACKTICK.finditer(text):
        if any(a <= m.start() < b for a, b in fenced):
            continue            # roh eingefuegte Ausgabe, kein lebender Verweis
        in_backticks.append((m.start(), m.end()))
        spanne = m.group(1).strip()
        if spanne.startswith(WURZELORDNER):
            treffer.append((spanne, m.start(1)))
    for m in MD_LINK.finditer(text):
        pos = m.start(1)
        if any(a <= pos < b for a, b in in_backticks):
            continue            # zitierter Code, kein lebender Verweis
        treffer.append((m.group(1), pos))
    return treffer


def aufloesen(pfad: Path, ziel: str) -> list[Path]:
    """Beide Lesarten eines Verweises.

    Zuerst relativ zum Ordner der Datei — das ist die Lesart eines
    Markdown-Links und die erklaerte Konvention von `design/LIES-MICH.md`
    (alle Pfade relativ zu `design/`, also meint `docs/DESIGN-GESETZE.md`
    dort `design/docs/DESIGN-GESETZE.md`). Beginnt das Ziel mit einem
    Wurzelordner, zaehlt zusaetzlich die Lesart vom Workspace-Root. Ein Pfad,
    der in genau einer Lesart existiert, ist kein Befund. Bis 02.09.2026
    entschied allein das Praefix, und jeder ordnerrelative `docs/…`-Link
    unter `design/` wurde als Verweis ins Leere gezaehlt.
    """
    kandidaten = [pfad.parent / ziel]
    if ziel.startswith(WURZELORDNER):
        kandidaten.append(WURZEL / ziel)
    return kandidaten


def existiert(pfad: Path, ziel: str) -> bool:
    """Pfad mit Leerzeichen sind erlaubt; ein Aufruf mit Schaltern ist keiner.

    Erst der ganze String, dann — nur wenn der nicht existiert — der Teil vor
    dem ersten Leerzeichen. So bleibt `design/Nakama Designausarbeitungen …`
    ein Pfad und `tools/beweise.ps1 -Bauen` ein Aufruf.
    """
    if any(k.exists() for k in aufloesen(pfad, ziel)):
        return True
    kopf = ziel.split(" ", 1)[0]
    return kopf != ziel and any(k.exists() for k in aufloesen(pfad, kopf))


def verweise_pruefen(pfad: Path, text: str) -> tuple[list[str], list[str]]:
    befunde: list[str] = []
    benannt: list[str] = []
    gesehen: set[tuple[str, bool]] = set()
    # Erster Durchgang: welche Pfade traegt diese Datei ueberhaupt mit einer
    # Markierung? Ein Dokument erklaert den Status eines Pfades EINMAL und
    # nennt ihn danach weiter — deshalb gilt die Markierung fuer die ganze
    # Datei, nicht nur fuer die eine Fundstelle.
    markiert: dict[str, tuple[str, str]] = {}
    for roh, pos in verweise_sammeln(pfad, text):
        if ist_ueberspringbar(roh):
            continue
        ziel = saeubern(roh)
        if not ziel or ist_ueberspringbar(ziel) or existiert(pfad, ziel):
            continue
        m = umfeld_markiert(text, pos)
        if m and ziel not in markiert:
            markiert[ziel] = m
    for roh, pos in verweise_sammeln(pfad, text):
        if ist_ueberspringbar(roh):
            continue
        ziel = saeubern(roh)
        if not ziel or ist_ueberspringbar(ziel):
            continue
        if existiert(pfad, ziel):
            continue
        marke = markiert.get(ziel) or umfeld_markiert(text, pos)
        schluessel = (ziel, marke is not None)
        if schluessel in gesehen:
            continue
        gesehen.add(schluessel)
        zeilennr = text.count("\n", 0, pos) + 1
        if marke:
            klasse, wort = marke
            text_klasse = ("als historisch markiert" if klasse == "HISTORISCH"
                           else "als noch nicht angelegter Zielpfad markiert")
            benannt.append(
                f"{pfad.as_posix()}:{zeilennr} VERWEIS OHNE ZIEL, aber "
                f"{text_klasse} („{wort}“ im Umfeld): {ziel}")
        else:
            befunde.append(
                f"{pfad.as_posix()}:{zeilennr} VERWEIS INS LEERE: {ziel}")
    return befunde, benannt


# --------------------------------------------------------------------- Lauf

def text_lesen(pfad: Path) -> str:
    roh = pfad.read_text(encoding="utf-8")
    if pfad.suffix.lower() == ".json":
        # Verweise stehen in den Werten, nicht in der Struktur. Der Rohtext
        # taugt dafuer, weil JSON-Strings ihre Pfade unveraendert tragen; ein
        # Gueltigkeitscheck kommt gratis dazu.
        json.loads(roh)
    return roh


def main(argv: list[str]) -> int:
    if len(argv) < 2:
        print("Aufruf: py -3.13 tools/plan/dokuriegel.py <datei> [<datei> ...]",
              file=sys.stderr)
        return 2
    alle_befunde: list[str] = []
    alle_benannt: list[str] = []
    for arg in argv[1:]:
        pfad = Path(arg)
        if not pfad.is_absolute():
            pfad = WURZEL / arg
        if not pfad.exists():
            alle_befunde.append(f"{arg} DATEI FEHLT")
            continue
        text = text_lesen(pfad)
        rel = pfad.relative_to(WURZEL) if pfad.is_relative_to(WURZEL) else pfad
        t = tabellen_pruefen(rel, text)
        v, b = verweise_pruefen(rel, text)
        alle_befunde += t + v
        alle_benannt += b
        if not (t or v):
            print(f"{rel.as_posix()}: sauber"
                  + (f" ({len(b)} benannt)" if b else ""))
    for zeile in alle_benannt:
        print(zeile)
    for zeile in alle_befunde:
        print(zeile)
    print(f"Dokuriegel: {len(alle_befunde)} Befund(e), "
          f"{len(alle_benannt)} benannt (historisch oder Zielpfad)")
    return 1 if alle_befunde else 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
