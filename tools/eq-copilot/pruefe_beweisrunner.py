#!/usr/bin/env py -3.13
"""NAK-309 Etappe 2 (T3-09-01, R-309-6) - Kanon-Bein A36 und Baustand des
Beweis-Runners: die Frischepruefung der Pruefbinaries.

WAS DIE FRISCHEPRUEFUNG BEHAUPTET

  tools/beweise.ps1 verweigert die Beglaubigung (Exit 4), wenn ein
  Pruefbinary aelter ist als eine Datei, aus der es entsteht. Welche Dateien
  das sind, leitet dieses Werkzeug aus dem ab, was der Bau tatsaechlich
  gelesen hat, nicht aus einer handgepflegten Ordnerliste (bis NAK-309 fehlte
  dort eq-copilot/plugin/dsp/, Befund T3-09-01):

    Frischebaum eines Binaries
      * der Tracking-Ordner (*.tlog), dessen Link-Schreiblog das Binary als
        Ausgabe nennt; sein Zwischenordner traegt genau einen solchen Ordner;
      * alle Eintraege seiner Leselogs (*.read.*.tlog: CL, link, Lib-link,
        rc, CustomBuild) ohne die Regenerationsregel CMakeLists.txt ->
        CMakeFiles/generate.stamp: Konfigurationseingaben zaehlen ueber den
        Konfigurationsstand (unten), nie ueber den eigenen Zeitstempel, sonst
        waere nach jedem --hashen jedes Binary veraltet (Feinheit F-4);
      * die Projektdatei <Ziel>.vcxproj (Uebersetzeroptionen);
      * ein Eintrag, den ein ANDERER Tracking-Ordner des Baubaums als Ausgabe
        nennt (Bibliothek, Objekt, Ressource), zaehlt ueber DESSEN
        Frischebaum: so erreicht jedes Binary, das NakamaKern.lib linkt, die
        Kernquellen samt dsp/, und der Zeitstempel der Bibliothek zaehlt nie,
        denn A14 baut sie in jedem Lauf neu (F-2). Eine Bibliothek, ein
        Objekt oder eine Ressource unter dem Baubaum ohne Erzeuger ist nicht
        ableitbar;
      * Eintraege ausserhalb der Arbeitskopie (Werkzeugkette, Windows SDK,
        System) zaehlen nicht. MSBuild nimmt Include- und Bibliotheksordner
        der Werkzeugkette, die SDK-Includes und $(SystemRoot) selbst aus
        seinem Vergleich (ExcludePath, Microsoft.Cpp.MSVC.Toolset.*.props),
        und ein Windows-Update (TZRES.DLL steht im Leselog von NakamaKern)
        machte sonst jedes Binary dauerhaft veraltet, weil ein No-op-Bau
        nicht neu linkt (NAK-25).

    Konfigurationsstand
      Keine Datei der Arbeitskopie, die eine generate.stamp.depend aus
      CMakeFiles/generate.stamp.list nennt, ist juenger als ihr
      generate.stamp - dieselbe Sicht wie ZERO_CHECK. Sonst ist jedes Binary
      des Baubaums VERALTET ("Konfiguration ausstehend: <Datei>").

    Urteil je Binary
      frisch (keine Eingabe streng juenger als das Binary, verglichen auf
      100-ns-Ticks; dieselbe Regel wie MSBuild: nur eine juengere Eingabe
      verlangt einen Neubau), VERALTET (nennt die juengste Eingabe; eine
      fehlende Eingabe zaehlt als veraltet), NICHT ABLEITBAR (nennt den
      Grund; nie gruen), "stillgelegt, nicht beurteilt" und
      "nicht gebaut, nicht beurteilt".

    Gemessene Ziele (NAK-309 Etappe 4, R-309-9, M-75)
      Jedes Ziel aus bau_ziele, das kein Bein der Population ist (im Runner
      $gemesseneZiele: Ziele, die ein Bein ausfuehrt oder misst), bekommt
      dasselbe Urteil wie ein Pruefbinary. Seine Zieldatei nennt der
      CMake-Export, nie eine Namensregel; fehlt das Ziel im Export, ist es
      NICHT ABLEITBAR.

    Kreuzprobe (M-06)
      Gegen den CMake-Export nakama-frischebaum-<Konfiguration>.json
      (eq-copilot/plugin/CMakeLists.txt, file(GENERATE)): jeder
      Kernverbraucher, den der Kanon baut, erreicht in seinem Frischebaum
      alle Kernquellen; ein statischer Verbraucher (JUCE-SharedCode) ueber
      seine Endziele (JUCE_ACTIVE_PLUGIN_TARGETS). Sonst "Frischebaum deckt
      <Ziel> nicht". Ein Verbraucher, den der Kanon nicht baut oder dessen
      Binary fehlt, wird genannt, nicht beurteilt.

    Inventar (M-08)
      Jede getrackte Datei unter eq-copilot/plugin mit Endung .cpp .h .hpp
      .c .inl .in steht im Frischebaum eines Ziels, das -Bauen baut, im
      Konfigurationsstand oder in der Ausnahmeliste (je Eintrag ein Grund).
      Sonst "Frischebaum unvollstaendig: <Datei>".

GRENZE (M-04)

  Verglichen werden Zeitstempel, keine Inhalte. Eine Quelle, die nach einer
  Mutation per Copy-Item mit ihrem alten Zeitstempel zurueckgespielt wird,
  gilt als frisch, obwohl das Binary aus der Mutante entstand (NAK-230); den
  Schutz traegt der Rotbeweis-Ablauf (nach jeder Ruecknahme LastWriteTime auf
  jetzt). Eine Aenderung an Werkzeugkette oder Windows SDK ausserhalb der
  Arbeitskopie sieht die Pruefung nicht.

MODI

  --baustand <population.json> [--bau-bestaetigt]
      Der Aufruf des Runners. population.json: {"konfiguration": "Release",
      "population": [{"kuerzel", "name", "binary", "stillgelegt"}, ...],
      "bau_ziele": [...]}. Mit --bau-bestaetigt hat -Bauen unmittelbar vorher
      erfolgreich gebaut: der Zeitvergleich folgt dem Urteil des
      Buildsystems, Ableitung, Kreuzprobe und Inventar laufen trotzdem.
      Ausgabe: Tabelle und eine Zeile "BAUSTAND-JSON {...}" fuer den Runner;
      je Eintrag nennt sie die beurteilte Datei ("binary") und ob es ein
      gemessenes Ziel ist ("gemessen").
  --nicht-gelaufen <meldeordner>
      NAK-309 Etappe 4 (T3-09-04, R-309-4): der Meldeordner eines cargo-Beins
      nach dem Lauf. Jede Marke heisst <Test>.nicht-gelaufen und traegt je
      Zeile "<Test>: <Grund>". Exit 0 leer (jeder Test mit Meldeweg lief),
      3 NOT RUN (Test und Grund je Marke; eine Marke eines anderen Tests und
      zwei Marken desselben Tests zaehlen ebenso, nie gruen), 2 unlesbar
      (Ordner fehlt, ein Eintrag ist keine Marke, eine Zeile hat nicht die
      Form, kein UTF-8). Ausgabe: je Marke eine Zeile und eine Zeile
      "NICHT-GELAUFEN-JSON {...}" fuer den Runner.
  --selbsttest [--nur <fall>]
      Kanon-Bein A36: ein Attrappen-Baubaum im Temp-Ordner (Tracking-Logs in
      UTF-16 mit BOM wie MSBuild, CustomBuild in UTF-8), jede Erwartung mit
      ihrem Gegenteil; Faelle nach den Matrixzeilen M-01 bis M-08, M-10,
      M-67, M-70 und M-75 aus docs/beweise/NAK-309.md. --nur nimmt einen
      Fallnamen oder eine Matrixzeile.

EXITCODES

  0 frisch bzw. Selbsttest gruen bzw. Meldeordner leer; 4 veraltet, Deckungs-
  oder Inventarluecke bzw. Selbsttest rot; 3 nicht ableitbar bzw. NOT RUN;
  2 Werkzeugfehler bzw. Meldeordner unlesbar.
"""

from __future__ import annotations

import argparse
import contextlib
import io
import json
import ntpath
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
from dataclasses import dataclass, field
from datetime import datetime

WURZEL = pathlib.Path(__file__).resolve().parents[2]
BAUBAUM = "eq-copilot/build"
KONFIGURATION = "Release"
INVENTAR_ORT = "eq-copilot/plugin"
INVENTAR_ENDUNGEN = (".cpp", ".h", ".hpp", ".c", ".inl", ".in")

# Gemessen am Baubaum vom 18.09.2026 (NAK-309 Etappe 2): diese getrackten
# Dateien stehen in keinem Frischebaum der Ziele, die -Bauen baut, weil nur
# Ziele sie uebersetzen, die der Kanon nicht baut (eq-copilot/plugin/
# CMakeLists.txt). Ein Eintrag mit "/" am Ende gilt fuer den ganzen Unterbaum.
# Was der Kanon nicht baut, bezeugt er nicht als frisch (Pruefliste D).
INVENTAR_AUSNAHMEN: tuple[tuple[str, str], ...] = (
    ("eq-copilot/plugin/probe/", "nur EqCopPipeProbe, ein Handwerkzeug ausserhalb des Kanons"),
    ("eq-copilot/plugin/spike/", "nur EqCopAuxSpike und EqCopAuxSpikeTest, nicht im Kanon (NAK-37)"),
    ("eq-copilot/plugin/tests/AuxSpikeTestMain.cpp", "nur EqCopAuxSpikeTest, nicht im Kanon (NAK-37)"),
    ("eq-copilot/plugin/tests/PaintBenchMain.cpp", "nur EqCopPaintBench, ein Messwerkzeug ausserhalb des Kanons"),
    ("eq-copilot/plugin/hostprobe/HostProbeFactory.cpp",
     "nur das VST3-Ziel EqCopHostProbe, das der Kanon nicht baut (B3b misst EqCopHostProbeTest)"),
)

GRENZZEILE = ("Grenze: Zeitstempel, kein Inhalt - eine per Copy-Item mit altem "
              "Zeitstempel zurueckgespielte Quelle gilt als frisch (NAK-230).")

JSON_MARKE = "BAUSTAND-JSON "

LESELOG = re.compile(r"^(?P<werkzeug>.+?)(?:\.\d+)?\.read\.\d+\.tlog$", re.IGNORECASE)
SCHREIBLOG = re.compile(r"^(?P<werkzeug>.+?)(?:\.\d+)?\.write\.\d+\.tlog$", re.IGNORECASE)
LINKWERKZEUGE = frozenset({"link", "lib-link"})
# Erzeugnisse des Uebersetzers: unter dem Baubaum zaehlen sie nur ueber ihren
# Erzeuger. Ohne Erzeuger ist der Baum nicht ableitbar (M-07 b).
ERZEUGNISSE = (".lib", ".obj", ".res")

STAND_FRISCH = "frisch"
STAND_BAU = "frisch (Bau bestaetigt)"
STAND_VERALTET = "VERALTET"
STAND_NICHT_ABLEITBAR = "NICHT ABLEITBAR"
STAND_STILLGELEGT = "stillgelegt, nicht beurteilt"
STAND_NICHT_GEBAUT = "nicht gebaut, nicht beurteilt"


class NichtAbleitbar(Exception):
    """Der Frischebaum ist aus dem Baubaum nicht ableitbar - nie gruen (M-07)."""


def _schluessel(pfad: str) -> str:
    """Vergleichsform eines Windows-Pfads: normalisiert, ohne Gross/klein."""
    return ntpath.normcase(ntpath.normpath(pfad))


def _mtime_ns(pfad: str) -> int:
    # M-03: 100-ns-Ticks von NTFS, nie auf Sekunden gerundet.
    return os.stat(pfad).st_mtime_ns


def _zeit(ns: int) -> str:
    return datetime.fromtimestamp(ns / 1e9).strftime("%Y-%m-%d %H:%M:%S")


def lies_log(pfad: pathlib.Path, kurz: str) -> str:
    """Tracking-Logs sind UTF-16 mit BOM (CL, link, Lib-link, rc; gemessen),
    die CustomBuild-Logs UTF-8 ohne BOM. Alles andere ist nicht ableitbar."""
    try:
        roh = pfad.read_bytes()
    except OSError as f:
        raise NichtAbleitbar(f"{kurz}: nicht lesbar ({type(f).__name__})") from None
    try:
        if roh.startswith(b"\xff\xfe"):
            text = roh[2:].decode("utf-16-le")
        elif roh.startswith(b"\xfe\xff"):
            text = roh[2:].decode("utf-16-be")
        elif roh.startswith(b"\xef\xbb\xbf"):
            text = roh[3:].decode("utf-8")
        else:
            text = roh.decode("utf-8")
    except UnicodeDecodeError as f:
        raise NichtAbleitbar(f"{kurz}: Kodierung nicht lesbar ({f.reason})") from None
    if "\x00" in text:
        raise NichtAbleitbar(f"{kurz}: NUL im Text (UTF-16 ohne BOM?)")
    return text


def abschnitte(text: str, kurz: str) -> list[tuple[tuple[str, ...], list[str]]]:
    """Leselog und Schreiblog nach MS Learn, "Visual C++ project
    extensibility": "^" leitet eine oder mehrere Quellen ein (getrennt durch
    einen senkrechten Strich), danach je Zeile eine Datei; alle als volle
    Pfade."""
    liste: list[tuple[tuple[str, ...], list[str]]] = []
    aktuell: tuple[tuple[str, ...], list[str]] | None = None
    for zeile in text.splitlines():
        zeile = zeile.strip()
        if not zeile:
            continue
        if zeile.startswith("^"):
            quellen = tuple(q.strip() for q in zeile[1:].split("|") if q.strip())
            if not quellen:
                raise NichtAbleitbar(f"{kurz}: ^-Zeile ohne Quelle")
            for q in quellen:
                if not ntpath.isabs(q):
                    raise NichtAbleitbar(f"{kurz}: kein voller Pfad: {q}")
            aktuell = (quellen, [])
            liste.append(aktuell)
        elif aktuell is None:
            raise NichtAbleitbar(f"{kurz}: Eintrag vor der ersten ^-Zeile")
        else:
            if not ntpath.isabs(zeile):
                raise NichtAbleitbar(f"{kurz}: kein voller Pfad: {zeile}")
            aktuell[1].append(zeile)
    return liste


@dataclass
class Baum:
    """Frischebaum eines Tracking-Ordners: Eingaben der Arbeitskopie
    (Schluessel -> Pfad wie im Log), die Werkzeuge mit mindestens einem
    Abschnitt und die Zahl der nicht gezaehlten Eintraege ausserhalb."""
    eingaben: dict[str, str] = field(default_factory=dict)
    werkzeuge: set[str] = field(default_factory=set)
    extern: int = 0


class Ableitung:
    """Frischebaeume aus den Tracking-Logs eines Baubaums."""

    def __init__(self, wurzel: pathlib.Path, baubaum: pathlib.Path):
        self.wurzel = wurzel
        self.baubaum = baubaum
        self.w_schluessel = _schluessel(str(wurzel)).rstrip("\\") + "\\"
        self.b_schluessel = _schluessel(str(baubaum)).rstrip("\\") + "\\"
        self.index: dict[str, set[tuple[str, str]]] = {}
        self._ordner: dict[str, pathlib.Path] = {}
        self._schreib: dict[str, list[tuple[str, tuple[str, ...], list[str]]]] = {}
        self._memo: dict[str, Baum] = {}
        self._in_arbeit: set[str] = set()
        self._zeiten: dict[str, int | None] = {}
        if not baubaum.is_dir():
            raise NichtAbleitbar(f"Baubaum {self.kurz(str(baubaum))} fehlt")
        self._indexieren()

    # -- Hilfen --------------------------------------------------------------

    def kurz(self, pfad: str) -> str:
        """Anzeigeform: echte Schreibweise, relativ zur Arbeitskopie."""
        try:
            echt = os.path.realpath(pfad)
        except OSError:
            echt = pfad
        if _schluessel(echt).startswith(self.w_schluessel):
            return os.path.relpath(echt, os.path.realpath(self.wurzel)).replace("\\", "/")
        return echt

    def intern(self, schluessel: str) -> bool:
        return schluessel.startswith(self.w_schluessel)

    def im_baubaum(self, schluessel: str) -> bool:
        return schluessel.startswith(self.b_schluessel)

    def zeit(self, schluessel: str, pfad: str) -> int | None:
        if schluessel not in self._zeiten:
            try:
                self._zeiten[schluessel] = _mtime_ns(pfad)
            except OSError:
                self._zeiten[schluessel] = None
        return self._zeiten[schluessel]

    # -- Schreiblogs: wer erzeugt was ------------------------------------------

    def _indexieren(self) -> None:
        for ort, _ordner, dateien in os.walk(self.baubaum):
            if not ort.lower().endswith(".tlog"):
                continue
            tdir = pathlib.Path(ort)
            t_schluessel = _schluessel(ort)
            self._ordner[t_schluessel] = tdir
            eintraege: list[tuple[str, tuple[str, ...], list[str]]] = []
            for name in sorted(dateien):
                treffer = SCHREIBLOG.match(name)
                if not treffer:
                    continue
                werkzeug = treffer.group("werkzeug").lower()
                kurz = self.kurz(str(tdir / name))
                for quellen, ausgaben in abschnitte(lies_log(tdir / name, kurz), kurz):
                    eintraege.append((werkzeug, quellen, ausgaben))
                    for a in ausgaben:
                        self.index.setdefault(_schluessel(a), set()).add((t_schluessel, werkzeug))
            self._schreib[t_schluessel] = eintraege

    def erzeuger(self, schluessel: str, pfad: str) -> str | None:
        """Tracking-Ordner, dessen Schreiblog die Datei als Ausgabe nennt."""
        treffer = self.index.get(schluessel)
        if not treffer:
            return None
        ordner = sorted({t for t, _ in treffer})
        if len(ordner) > 1:
            raise NichtAbleitbar(f"{self.kurz(pfad)}: mehrere Tracking-Ordner nennen die Datei als "
                                 f"Ausgabe ({', '.join(self.kurz(str(self._ordner[o])) for o in ordner)})")
        return ordner[0]

    def _ist_regeneration(self, t_schluessel: str, quellen: tuple[str, ...]) -> bool:
        """Die CMake-Regenerationsregel: derselbe CustomBuild-Abschnitt schreibt
        CMakeFiles/generate.stamp (F-4)."""
        gesucht = {_schluessel(q) for q in quellen}
        for werkzeug, w_quellen, ausgaben in self._schreib.get(t_schluessel, []):
            if werkzeug != "custombuild" or {_schluessel(q) for q in w_quellen} != gesucht:
                continue
            if any(ntpath.basename(_schluessel(a)) == "generate.stamp" for a in ausgaben):
                return True
        return False

    # -- Frischebaum -----------------------------------------------------------

    def fuer_binary(self, binary: pathlib.Path) -> Baum:
        """M-07 a: ohne Link-Schreiblog, das das Binary nennt, ohne CL- und
        Link-Leselog kein Frischebaum."""
        schluessel = _schluessel(str(binary))
        treffer = self.index.get(schluessel, set())
        link = sorted({t for t, w in treffer if w in LINKWERKZEUGE})
        if not link:
            raise NichtAbleitbar(f"kein Link-Tracking-Log des Baubaums nennt {self.kurz(str(binary))} "
                                 "als Ausgabe")
        if len(link) > 1:
            raise NichtAbleitbar(f"{self.kurz(str(binary))}: mehrere Tracking-Ordner nennen es als Ausgabe")
        baum = self.fuer_ordner(link[0])
        fehlt = []
        if "cl" not in baum.werkzeuge:
            fehlt.append("CL-Leselog")
        if not baum.werkzeuge & LINKWERKZEUGE:
            fehlt.append("Link-Leselog")
        if fehlt:
            raise NichtAbleitbar(f"{self.kurz(str(self._ordner[link[0]]))}: {' und '.join(fehlt)} "
                                 "fehlt oder ist leer")
        return baum

    def fuer_ordner(self, t_schluessel: str) -> Baum:
        if t_schluessel in self._memo:
            return self._memo[t_schluessel]
        if t_schluessel in self._in_arbeit:
            raise NichtAbleitbar(f"zyklische Erzeugerkette an {self.kurz(str(self._ordner[t_schluessel]))}")
        self._in_arbeit.add(t_schluessel)
        try:
            baum = self._ableiten(t_schluessel)
        finally:
            self._in_arbeit.discard(t_schluessel)
        self._memo[t_schluessel] = baum
        return baum

    def _projektdatei(self, zwischen: pathlib.Path) -> pathlib.Path:
        # CMake-VS-Generator: <Binaerordner>/<Ziel>.dir/<Konfiguration>/ neben
        # <Binaerordner>/<Ziel>.vcxproj.
        ziel_ordner = zwischen.parent
        if not ziel_ordner.name.lower().endswith(".dir"):
            raise NichtAbleitbar(f"{self.kurz(str(zwischen))}: kein Zwischenordner <Ziel>.dir/<Konfiguration>")
        projekt = ziel_ordner.parent / (ziel_ordner.name[:-4] + ".vcxproj")
        if not projekt.is_file():
            raise NichtAbleitbar(f"Projektdatei {self.kurz(str(projekt))} fehlt")
        return projekt

    def _ableiten(self, t_schluessel: str) -> Baum:
        tdir = self._ordner[t_schluessel]
        zwischen = tdir.parent
        tlogs = sorted(p.name for p in zwischen.iterdir() if p.is_dir() and p.name.lower().endswith(".tlog"))
        if len(tlogs) != 1:
            # M-07 d: zwei Tracking-Ordner lassen offen, welcher den Bau traegt.
            raise NichtAbleitbar(f"{self.kurz(str(zwischen))}: {len(tlogs)} Tracking-Ordner statt genau einem "
                                 f"({', '.join(tlogs)})")
        projekt = self._projektdatei(zwischen)
        baum = Baum()
        baum.eingaben[_schluessel(str(projekt))] = str(projekt)
        for datei in sorted(tdir.iterdir()):
            treffer = LESELOG.match(datei.name) if datei.is_file() else None
            if not treffer:
                continue
            werkzeug = treffer.group("werkzeug").lower()
            kurz = self.kurz(str(datei))
            for quellen, abhaengige in abschnitte(lies_log(datei, kurz), kurz):
                if werkzeug == "custombuild" and self._ist_regeneration(t_schluessel, quellen):
                    continue  # F-4: Konfigurationseingaben zaehlen ueber den Stempel
                baum.werkzeuge.add(werkzeug)
                for pfad in (*quellen, *abhaengige):
                    self._einordnen(t_schluessel, pfad, baum)
        if not baum.werkzeuge:
            raise NichtAbleitbar(f"{self.kurz(str(tdir))}: kein Leselog mit Eintrag")
        return baum

    def _einordnen(self, t_schluessel: str, pfad: str, baum: Baum) -> None:
        schluessel = _schluessel(pfad)
        if not self.intern(schluessel):
            baum.extern += 1
            return
        if self.im_baubaum(schluessel):
            erzeuger = self.erzeuger(schluessel, pfad)
            if erzeuger is not None:
                if erzeuger == t_schluessel:
                    return  # eigenes Zwischenprodukt: zaehlt ueber die Quellen dieses Ordners
                # M-01, M-06: ein fremdes Erzeugnis zaehlt ueber den Frischebaum seines Erzeugers.
                unter = self.fuer_ordner(erzeuger)
                baum.eingaben.update(unter.eingaben)
                baum.extern += unter.extern
                return
            if schluessel.endswith(ERZEUGNISSE):
                raise NichtAbleitbar(f"{self.kurz(pfad)} ist keinem Zwischenordner zuzuordnen "
                                     "(kein *.write.*.tlog nennt die Datei als Ausgabe)")
        baum.eingaben[schluessel] = pfad


# -- Konfigurationsstand ---------------------------------------------------------


@dataclass
class Konfiguration:
    stempel: int = 0
    eingaben: dict[str, str] = field(default_factory=dict)
    ausstehend: list[str] = field(default_factory=list)


def _lies_text(pfad: pathlib.Path, kurz: str) -> str:
    try:
        return pfad.read_text(encoding="utf-8")
    except (OSError, UnicodeDecodeError) as f:
        raise NichtAbleitbar(f"{kurz}: nicht lesbar ({type(f).__name__})") from None


def konfigurationsstand(abl: Ableitung) -> Konfiguration:
    """M-10: jede Datei der Arbeitskopie aus einer generate.stamp.depend, die
    generate.stamp.list nennt, ist nicht juenger als ihr Stempel."""
    liste = abl.baubaum / "CMakeFiles" / "generate.stamp.list"
    if not liste.is_file():
        raise NichtAbleitbar(f"{abl.kurz(str(liste))} fehlt")
    k = Konfiguration()
    stempel = [z.strip() for z in _lies_text(liste, abl.kurz(str(liste))).splitlines() if z.strip()]
    if not stempel:
        raise NichtAbleitbar(f"{abl.kurz(str(liste))} nennt keinen Stempel")
    for s in stempel:
        pfad = pathlib.Path(s)
        abhaengig = pfad.with_name(pfad.name + ".depend")
        if not pfad.is_file():
            raise NichtAbleitbar(f"{abl.kurz(str(pfad))} fehlt")
        if not abhaengig.is_file():
            raise NichtAbleitbar(f"{abl.kurz(str(abhaengig))} fehlt")
        stempel_ns = _mtime_ns(str(pfad))
        k.stempel += 1
        for zeile in _lies_text(abhaengig, abl.kurz(str(abhaengig))).splitlines():
            zeile = zeile.strip()
            if not zeile or zeile.startswith("#"):
                continue
            schluessel = _schluessel(zeile)
            if not abl.intern(schluessel):
                continue
            k.eingaben[schluessel] = zeile
            ns = abl.zeit(schluessel, zeile)
            if ns is None or ns > stempel_ns:  # M-10 a: Stempelvergleich
                k.ausstehend.append(zeile)
    return k


# -- Baustand ------------------------------------------------------------------


@dataclass
class Ergebnis:
    exit: int
    zeilen: list[str]
    json: dict


def _lies_export(abl: Ableitung, konfig: str) -> dict:
    pfad = abl.baubaum / "plugin" / f"nakama-frischebaum-{konfig}.json"
    if not pfad.is_file():
        raise NichtAbleitbar(f"CMake-Export {abl.kurz(str(pfad))} fehlt")
    try:
        daten = json.loads(pfad.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as f:
        raise NichtAbleitbar(f"CMake-Export {abl.kurz(str(pfad))} nicht lesbar ({type(f).__name__})") from None
    fehlt = [s for s in ("kern_quellen", "verbraucher", "ziele") if s not in daten]
    if (fehlt or not isinstance(daten.get("kern_quellen"), list) or not daten["kern_quellen"]
            or not isinstance(daten.get("verbraucher"), list) or not isinstance(daten.get("ziele"), dict)):
        raise NichtAbleitbar(f"CMake-Export {abl.kurz(str(pfad))} ohne die gelesene Form "
                             f"(kern_quellen, verbraucher, ziele)")
    if daten.get("konfiguration") != konfig:
        raise NichtAbleitbar(f"CMake-Export traegt Konfiguration {daten.get('konfiguration')!r} statt {konfig!r}")
    return daten


def _git_dateien(wurzel: pathlib.Path) -> list[str]:
    try:
        lauf = subprocess.run(["git", "-C", str(wurzel), "--no-optional-locks", "ls-files", "-z", "--",
                               INVENTAR_ORT], capture_output=True, check=False)
    except OSError as f:
        raise NichtAbleitbar(f"git ls-files nicht ausfuehrbar ({type(f).__name__})") from None
    if lauf.returncode != 0:
        raise NichtAbleitbar(f"git ls-files {INVENTAR_ORT}: Exit {lauf.returncode}")
    return [d for d in lauf.stdout.decode("utf-8").split("\0") if d]


def _ausgenommen(rel: str, ausnahmen: tuple[tuple[str, str], ...]) -> str | None:
    klein = rel.lower()
    for eintrag, grund in ausnahmen:
        e = eintrag.lower()
        if (e.endswith("/") and klein.startswith(e)) or klein == e:
            return grund
    return None


def baustand(wurzel: pathlib.Path, eingabe: dict, bau_bestaetigt: bool,
             git_dateien: list[str] | None = None,
             ausnahmen: tuple[tuple[str, str], ...] = INVENTAR_AUSNAHMEN) -> Ergebnis:
    konfig = eingabe.get("konfiguration") or KONFIGURATION
    baubaum = wurzel / eingabe.get("baubaum", BAUBAUM)
    population = eingabe.get("population") or []
    bau_ziele = list(eingabe.get("bau_ziele") or [])
    z: list[str] = []
    z.append(f"[Baustand] Frischebaum je Pruefbinary und je gemessenem Ziel (Bein gem.) aus den "
             f"MSBuild-Tracking-Logs ({BAUBAUM}, {konfig})"
             + ("  - Bau bestaetigt: der Zeitvergleich folgt dem Buildsystem" if bau_bestaetigt else ""))
    z.append("  " + GRENZZEILE)  # M-04: die Grenze steht in jeder Ausgabe
    zeilen_binaries: list[dict] = []
    luecken: list[str] = []
    grund_global: str | None = None
    abl: Ableitung | None = None
    export: dict = {}
    konf = Konfiguration()
    try:
        abl = Ableitung(wurzel, baubaum)
        export = _lies_export(abl, konfig)
        konf = konfigurationsstand(abl)
    except NichtAbleitbar as f:
        grund_global = str(f)

    # -- Population: Frischebaum und Urteil je Binary ------------------------
    # M-05: der Runner uebergibt jedes Plugin-Bein samt Marke; stillgelegte
    # bleiben draussen - kein Frischeurteil, kein Einfluss auf den Exit.
    #
    # R-309-9 (M-75): jedes Ziel, das der Runner baut und ein Bein ausfuehrt
    # oder misst ($gemesseneZiele), bekommt dasselbe Urteil. Es steht in
    # bau_ziele, aber nicht unter den Beinen; seine Zieldatei nennt der
    # CMake-Export, nie eine Namensregel des Runners. Fehlt es dort, ist es
    # nicht ableitbar.
    beine = {p.get("name", "") for p in population}
    alle = [dict(p, gemessen=False) for p in population]
    for ziel in bau_ziele:
        if ziel in beine:
            continue
        datei = (export.get("ziele", {}).get(ziel) or {}).get("datei") or ""
        alle.append({"kuerzel": "gem.", "name": ziel, "binary": datei, "stillgelegt": None, "gemessen": True})
    draussen = {p.get("name", "") for p in alle if p.get("stillgelegt")}
    baeume: dict[str, Baum] = {}
    gruende: dict[str, str] = {}
    for p in alle:
        name = p.get("name", "")
        if p["gemessen"] and not p.get("binary") and grund_global is None:
            gruende[name] = f"Ziel {name} fehlt im CMake-Export"
            continue
        binary = pathlib.Path(p.get("binary") or "")
        if name in draussen or grund_global is not None or abl is None or not binary.is_file():
            continue
        try:
            erwartet = (export.get("ziele", {}).get(name) or {}).get("datei")
            if erwartet and _schluessel(erwartet) != _schluessel(str(binary)):
                raise NichtAbleitbar(f"Binary laut Runner {abl.kurz(str(binary))} ist nicht die Zieldatei "
                                     f"laut CMake-Export ({abl.kurz(erwartet)})")
            baeume[name] = abl.fuer_binary(binary)
        except NichtAbleitbar as f:
            gruende[name] = str(f)
    alle_ns = [abl.zeit(k, v) for b in baeume.values() for k, v in b.eingaben.items()] if abl else []
    juengste_aller_baeume = max([n for n in alle_ns if n is not None], default=-1)

    veraltet = False
    nicht_ableitbar = grund_global is not None
    for p in alle:
        name = p.get("name", "")
        eintrag = {"kuerzel": p.get("kuerzel", ""), "name": name, "stand": "", "juengste_eingabe": "",
                   "juengste_zeit": "", "grund": "", "binary": p.get("binary") or "", "gemessen": p["gemessen"]}
        binary = pathlib.Path(p.get("binary") or "")
        if name in draussen:
            eintrag["stand"] = STAND_STILLGELEGT
        elif grund_global is not None:
            eintrag["stand"] = STAND_NICHT_ABLEITBAR
            eintrag["grund"] = grund_global
        elif name in gruende:
            eintrag["stand"] = STAND_NICHT_ABLEITBAR
            eintrag["grund"] = gruende[name]
            nicht_ableitbar = True
        elif not binary.is_file():
            eintrag["stand"] = STAND_NICHT_GEBAUT
        else:
            baum = baeume[name]
            binary_ns = _mtime_ns(str(binary))
            juengste_ns, juengste, fehlend = -1, "", []
            for schluessel, pfad in baum.eingaben.items():
                ns = abl.zeit(schluessel, pfad)
                if ns is None:
                    fehlend.append(pfad)  # F-3: eine fehlende Eingabe zaehlt als veraltet
                elif ns > juengste_ns:
                    juengste_ns, juengste = ns, pfad
            if juengste:
                eintrag["juengste_eingabe"] = abl.kurz(juengste)
                eintrag["juengste_zeit"] = _zeit(juengste_ns)
            vergleich_ns = juengste_ns  # M-02: je Binary gegen den EIGENEN Frischebaum
            ist_veraltet = vergleich_ns > binary_ns  # M-03: nur streng juenger ist veraltet
            if bau_bestaetigt:
                eintrag["stand"] = STAND_BAU
            elif konf.ausstehend:
                eintrag["stand"] = STAND_VERALTET
                eintrag["grund"] = "Konfiguration ausstehend: " + abl.kurz(konf.ausstehend[0])
            elif fehlend:
                eintrag["stand"] = STAND_VERALTET
                eintrag["grund"] = "Eingabe fehlt: " + fehlend[0]
            elif ist_veraltet:
                eintrag["stand"] = STAND_VERALTET
            else:
                eintrag["stand"] = STAND_FRISCH
            veraltet = veraltet or eintrag["stand"] == STAND_VERALTET
        zeilen_binaries.append(eintrag)

    z.append("")
    z.append(f"  {'Bein':<6} {'Pruefbinary oder gemessenes Ziel':<38} {'Stand':<30} juengste Eingabe")
    for e in zeilen_binaries:
        rechts = e["juengste_eingabe"] + (f" ({e['juengste_zeit']})" if e["juengste_zeit"] else "")
        if e["grund"]:
            rechts = (rechts + "  " if rechts else "") + "[" + e["grund"] + "]"
        z.append(f"  {e['kuerzel']:<6} {e['name']:<38} {e['stand']:<30} {rechts}".rstrip())
    if grund_global is not None:
        z.append("")
        z.append(f"  NICHT ABLEITBAR: {grund_global}")
    else:
        z.append("")
        z.append(f"  Konfiguration: {konf.stempel} Stempel, {len(konf.eingaben)} Eingaben der Arbeitskopie, "
                 + ("ausstehend: " + ", ".join(abl.kurz(a) for a in konf.ausstehend)
                    if konf.ausstehend else "keine ausstehend"))

    # -- Kreuzprobe (M-06) und Inventar (M-08) ----------------------------------
    kreuz: list[dict] = []
    if grund_global is None and abl is not None:
        ziele = export["ziele"]
        kern = [_schluessel(str(wurzel / q)) for q in export["kern_quellen"]]
        im_bau = set(bau_ziele)

        def baum_von(ziel: str) -> Baum | None:
            datei = (ziele.get(ziel) or {}).get("datei")
            if not datei or not pathlib.Path(datei).is_file():
                return None
            return abl.fuer_binary(pathlib.Path(datei))

        for v in export["verbraucher"]:
            name = v.get("name", "")
            typ = v.get("typ", "")
            pruefziele = list(v.get("endziele") or []) if typ == "STATIC_LIBRARY" else [name]
            status, detail = "", ""
            if typ == "STATIC_LIBRARY" and not pruefziele:
                status, detail = STAND_NICHT_ABLEITBAR, "statischer Kernverbraucher ohne Endziel"
                nicht_ableitbar = True
            elif name not in im_bau and not (im_bau & set(pruefziele)):
                status = "nicht im Kanon, nicht beurteilt"
            else:
                status = "gedeckt"
                for ziel in pruefziele:
                    try:
                        baum = baum_von(ziel)
                    except NichtAbleitbar as f:
                        status, detail = STAND_NICHT_ABLEITBAR, f"{ziel}: {f}"
                        nicht_ableitbar = True
                        break
                    if baum is None:
                        status, detail = STAND_NICHT_GEBAUT, f"{ziel}: Zieldatei fehlt"
                        break
                    ohne = [k for k in kern if k not in baum.eingaben]
                    if ohne:  # M-06: die Kreuzprobe traegt die Zusage
                        status = "NICHT GEDECKT"
                        detail = f"{ziel}: ohne {abl.kurz(ohne[0])}" + (f" und {len(ohne) - 1} weitere"
                                                                     if len(ohne) > 1 else "")
                        luecken.append(f"Frischebaum deckt {name} nicht")
                        break
            kreuz.append({"name": name, "typ": typ, "status": status, "detail": detail})
        zahl = {s: sum(1 for k in kreuz if k["status"] == s)
                for s in ("gedeckt", "NICHT GEDECKT", STAND_NICHT_GEBAUT, "nicht im Kanon, nicht beurteilt",
                          STAND_NICHT_ABLEITBAR)}
        z.append("")
        z.append(f"[Kreuzprobe] gebundene Ziele laut CMake-Export: {len(kreuz)} "
                 f"({sum(1 for k in kreuz if k['typ'] == 'STATIC_LIBRARY')} statisch ueber ihre Endziele); "
                 f"gedeckt {zahl['gedeckt']}, NICHT GEDECKT {zahl['NICHT GEDECKT']}, "
                 f"nicht gebaut {zahl[STAND_NICHT_GEBAUT]}, nicht im Kanon {zahl['nicht im Kanon, nicht beurteilt']}, "
                 f"nicht ableitbar {zahl[STAND_NICHT_ABLEITBAR]}; {len(kern)} Kernquellen")
        for k in kreuz:
            if k["status"] != "gedeckt":
                z.append(f"  {k['name']:<38} {k['status']}" + (f"  [{k['detail']}]" if k["detail"] else ""))

        # Inventar: Vereinigung der Frischebaeume aller Ziele, die -Bauen baut.
        abgedeckt: set[str] = set(konf.eingaben)
        inventar_grund: str | None = None
        for ziel in bau_ziele:
            if ziel not in ziele:
                inventar_grund = f"Ziel {ziel} fehlt im CMake-Export"
                break
            try:
                baum = baum_von(ziel)
            except NichtAbleitbar as f:
                inventar_grund = f"{ziel}: {f}"
                break
            if baum is None:
                inventar_grund = f"Zieldatei von {ziel} fehlt"
                break
            abgedeckt.update(baum.eingaben)
        z.append("")
        if inventar_grund is None:
            try:
                dateien = _git_dateien(wurzel) if git_dateien is None else git_dateien
            except NichtAbleitbar as f:
                inventar_grund = str(f)
        if inventar_grund is not None:
            nicht_ableitbar = True
            z.append(f"[Inventar] NICHT ABLEITBAR: {inventar_grund}")
        else:
            geprueft = [d for d in dateien if d.lower().endswith(INVENTAR_ENDUNGEN)]  # M-08: das Inventar
            ausgenommen: list[tuple[str, str]] = []
            fehlend_inventar: list[str] = []
            for rel in geprueft:
                if _schluessel(str(wurzel / rel)) in abgedeckt:
                    continue
                grund = _ausgenommen(rel, ausnahmen)
                if grund is not None:
                    ausgenommen.append((rel, grund))
                    continue
                fehlend_inventar.append(rel)
            for rel in fehlend_inventar:  # eine Datei ohne Frischebaum verweigert die Beglaubigung
                luecken.append(f"Frischebaum unvollstaendig: {rel}")
            z.append(f"[Inventar] {len(geprueft)} getrackte Dateien unter {INVENTAR_ORT} mit Endung "
                     f"{' '.join(INVENTAR_ENDUNGEN)}: im Frischebaum "
                     f"{len(geprueft) - len(ausgenommen) - len(fehlend_inventar)}, ausgenommen mit Grund "
                     f"{len(ausgenommen)}, Luecken {len(fehlend_inventar)}")
            for rel, grund in ausgenommen:
                z.append(f"  ausgenommen: {rel} - {grund}")
            for eintrag, _grund in ausnahmen:
                if not any(_ausgenommen(rel, ((eintrag, ""),)) is not None for rel in geprueft):
                    z.append(f"  hinweis: Ausnahme ohne getrackte Datei: {eintrag}")
            for rel in fehlend_inventar:
                z.append(f"  Frischebaum unvollstaendig: {rel}")

    if nicht_ableitbar:
        code = 3
    elif (veraltet and not bau_bestaetigt) or luecken:
        code = 4
    else:
        code = 0
    urteil = {0: "FRISCH", 3: "NICHT ABLEITBAR", 4: "NICHT FRISCH"}[code]
    z.append("")
    z.append(f"Baustand: {urteil} (Exit {code})"
             + (f" - {len(luecken)} Luecke(n)" if luecken else ""))
    daten = {"exit": code, "veraltet": bool(veraltet and not bau_bestaetigt), "nicht_ableitbar": nicht_ableitbar,
             "luecken": luecken, "grund": grund_global or "",
             "konfiguration_ausstehend": [abl.kurz(a) for a in konf.ausstehend] if abl else [],
             "binaries": zeilen_binaries, "kreuzprobe": kreuz,
             "juengste_aller_baeume": _zeit(juengste_aller_baeume) if juengste_aller_baeume >= 0 else ""}
    return Ergebnis(code, z, daten)


def baustand_cli(pfad: pathlib.Path, bau_bestaetigt: bool, wurzel: pathlib.Path) -> int:
    try:
        eingabe = json.loads(pfad.read_text(encoding="utf-8-sig"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as f:
        print(f"WERKZEUGFEHLER - {pfad}: {type(f).__name__}: {f}")
        return 2
    if not isinstance(eingabe, dict) or not isinstance(eingabe.get("population"), list):
        print(f"WERKZEUGFEHLER - {pfad}: keine Population")
        return 2
    ergebnis = baustand(wurzel, eingabe, bau_bestaetigt)
    for zeile in ergebnis.zeilen:
        print(zeile)
    print(JSON_MARKE + json.dumps(ergebnis.json, ensure_ascii=True))
    return ergebnis.exit


# == NOT RUN (NAK-309 Etappe 4, T3-09-04, R-309-4, F-21) ==========================

MARKE = re.compile(r"^(?P<test>[A-Za-z0-9_]+)\.nicht-gelaufen$")
NICHT_GELAUFEN_JSON = "NICHT-GELAUFEN-JSON "


@dataclass
class Meldung:
    exit: int
    zeilen: list[str]
    marken: list[tuple[str, str]]
    grund: str = ""


def nicht_gelaufen(ordner: pathlib.Path) -> Meldung:
    """Der Meldeordner eines cargo-Beins: 0 leer, 3 NOT RUN, 2 unlesbar.

    Der Runner legt den Ordner je Bein frisch an; eine Marke eines anderen
    Tests kann nur ein fremder Schreiber hineinlegen und zaehlt deshalb wie
    jede Marke als NOT RUN - nie gruen. Unlesbar ist nie leer: wer den
    Meldeweg nicht lesen kann, weiss nicht, ob jeder Test lief."""
    z = [f"Meldeordner {ordner}"]

    def unlesbar(grund: str) -> Meldung:
        z.append(f"  UNLESBAR: {grund}")
        z.append("Meldeweg: UNLESBAR (Exit 2)")
        return Meldung(2, z, [], grund)

    if not ordner.is_dir():
        return unlesbar("Meldeordner fehlt")
    try:
        eintraege = sorted(ordner.iterdir(), key=lambda p: p.name)
    except OSError as f:
        return unlesbar(f"Meldeordner nicht lesbar ({type(f).__name__})")
    marken: list[tuple[str, str]] = []
    for eintrag in eintraege:
        treffer = MARKE.match(eintrag.name)
        if treffer is None or not eintrag.is_file():
            return unlesbar(f"keine Marke: {eintrag.name}")
        try:
            text = eintrag.read_bytes().decode("utf-8")
        except (OSError, UnicodeDecodeError) as f:
            return unlesbar(f"{eintrag.name} nicht lesbar ({type(f).__name__})")
        if not text.endswith("\n") or "\r" in text:
            return unlesbar(f"{eintrag.name}: keine Folge von Zeilen mit LF-Ende")
        praefix = treffer.group("test") + ": "
        for zeile in text[:-1].split("\n"):
            if not zeile.startswith(praefix) or not zeile[len(praefix):].strip():
                return unlesbar(f"{eintrag.name}: Zeile ist nicht '<Test>: <Grund>'")
            marken.append((treffer.group("test"), zeile[len(praefix):]))
    if not marken:
        z.append("Meldeweg: leer - jeder Test mit Meldeweg ist gelaufen (Exit 0)")
        return Meldung(0, z, [])
    for test, grund in marken:
        z.append(f"  NOT RUN {test}: {grund}")
    tests = len({test for test, _ in marken})
    z.append(f"Meldeweg: NOT RUN - {tests} Test(s) nicht gelaufen (Exit 3)")
    return Meldung(3, z, marken)


def nicht_gelaufen_cli(ordner: pathlib.Path) -> int:
    meldung = nicht_gelaufen(ordner)
    for zeile in meldung.zeilen:
        print(zeile)
    daten = {"exit": meldung.exit, "grund": meldung.grund,
             "marken": [{"test": t, "grund": g} for t, g in meldung.marken]}
    print(NICHT_GELAUFEN_JSON + json.dumps(daten, ensure_ascii=True))
    return meldung.exit


# == Selbsttest (Bein A36) =====================================================

T_QUELLE = 1_789_000_000_500_000_000     # Quellen, Kopfdateien, Projektdateien (halbe Sekunde)
T_ERZEUGT = T_QUELLE + 20 * 10**9          # erzeugter Header (configure_file)
T_STEMPEL = T_QUELLE + 30 * 10**9          # generate.stamp
T_BAU = T_QUELLE + 60 * 10**9              # Objekte, Bibliotheken
T_BINARY = T_QUELLE + 120 * 10**9          # Binaries
SEKUNDE = 10**9

KERN = ("eq-copilot/plugin/dsp/DspKern.cpp", "eq-copilot/plugin/state/NakamaState.cpp",
        "eq-copilot/plugin/core/ipc/BrokerLifecycle.cpp")
KERN_KOPF = "eq-copilot/plugin/dsp/DspKern.h"
HEADER_IN = "eq-copilot/plugin/core/ipc/BrokerInstallBinding.h.in"
MANIFEST_REL = "eq-copilot/install/nakama-installer-v1.json"


class Attrappe:
    """Ein Attrappen-Baubaum im Temp-Ordner, gebaut wie ihn MSBuild und CMake
    hinterlassen (gemessen am Baubaum vom 18.09.2026): Leselogs mit
    Grossbuchstaben-Pfaden in UTF-16 mit BOM, CustomBuild in UTF-8,
    abgekuerzte Tracking-Ordnernamen, generate.stamp.list mit Stempeln und
    CMake-Export. Die Wurzel enthaelt ein Leerzeichen."""

    def __init__(self, wurzel: pathlib.Path, ohne_kern: bool = False, stillgelegt_alt: bool = False):
        self.w = wurzel
        self.bau = wurzel / "eq-copilot" / "build"
        self.pb = self.bau / "plugin"
        self.ziele: dict[str, dict] = {}
        self.verbraucher: list[dict] = []
        self.bau_ziele: list[str] = []
        self.population: list[dict] = []
        self.inventar: list[str] = []
        # Quellen
        for rel in (*KERN, KERN_KOPF, HEADER_IN, "eq-copilot/plugin/CMakeLists.txt", "eq-copilot/CMakeLists.txt",
                    MANIFEST_REL, "eq-copilot/identity/plugin-identities-v1.json",
                    "eq-copilot/plugin/tests/DspGoldenTestMain.cpp", "eq-copilot/plugin/tests/TransactionTestMain.cpp",
                    "eq-copilot/plugin/tests/ShotTestMain.cpp", "eq-copilot/plugin/tests/OhneKernMain.cpp",
                    "eq-copilot/plugin/tests/SunaNullTestMain.cpp", "eq-copilot/plugin/src/PluginProcessor.cpp",
                    "eq-copilot/plugin/src/Vst3Huelle.cpp", "eq-copilot/plugin/probe/PipeProbeMain.cpp"):
            self.datei(rel, b"// " + rel.encode() + b"\n", T_QUELLE)
        self.inventar = [r for r in (*KERN, KERN_KOPF, HEADER_IN, "eq-copilot/plugin/CMakeLists.txt",
                                     "eq-copilot/plugin/tests/DspGoldenTestMain.cpp",
                                     "eq-copilot/plugin/tests/TransactionTestMain.cpp",
                                     "eq-copilot/plugin/tests/ShotTestMain.cpp",
                                     "eq-copilot/plugin/src/PluginProcessor.cpp",
                                     "eq-copilot/plugin/src/Vst3Huelle.cpp",
                                     "eq-copilot/plugin/probe/PipeProbeMain.cpp")]
        self.erzeugt = self.pb / "generated" / "nakama" / "BrokerInstallBinding.h"
        self.datei_abs(self.erzeugt, b"// erzeugt\n", T_ERZEUGT)
        # Kern
        kern_lib = self.pb / "Release" / "NakamaKern.lib"
        self.ziel("NakamaKern", "STATIC_LIBRARY", kern_lib, "NakamaKern.tlog",
                  cl=[(q, [KERN_KOPF] + ([str(self.erzeugt)] if q.endswith("BrokerLifecycle.cpp") else []))
                      for q in KERN],
                  link_werkzeug="Lib-link", link_extern=[r"C:\WINDOWS\SYSTEM32\TZRES.DLL"])
        self.bau_ziele.append("NakamaKern")
        # Testbinaries (B6, B7, B15) linken den Kern
        for kuerzel, name, quelle, tlog in (
                ("B6", "EqCopDspGoldenTest", "eq-copilot/plugin/tests/DspGoldenTestMain.cpp", "EqCopDsp.4CC300A8.tlog"),
                ("B7", "EqCopTransactionTest", "eq-copilot/plugin/tests/TransactionTestMain.cpp",
                 "EqCopTra.6D1B22F0.tlog"),
                ("B15", "EqCopShot", "eq-copilot/plugin/tests/ShotTestMain.cpp", "EqCopShot.tlog")):
            exe = self.pb / f"{name}_artefacts" / "Release" / f"{name}.exe"
            self.ziel(name, "EXECUTABLE", exe, tlog, cl=[(quelle, [KERN_KOPF])], libs=[kern_lib],
                      link_extern=[r"C:\PROGRAM FILES (X86)\WINDOWS KITS\10\LIB\10.0.26100.0\UM\X64\KERNEL32.LIB"])
            self.verbraucher.append({"name": name, "typ": "EXECUTABLE", "endziele": []})
            self.bau_ziele.append(name)
            self.population.append({"kuerzel": kuerzel, "name": name, "binary": str(exe), "stillgelegt": None})
        # Statischer Verbraucher (JUCE-SharedCode) mit Endziel
        shared = self.pb / "EqCopilot_artefacts" / "Release" / "EQ-Copilot_SharedCode.lib"
        self.ziel("EqCopilot", "STATIC_LIBRARY", shared, "EqCopilot.tlog",
                  cl=[("eq-copilot/plugin/src/PluginProcessor.cpp", [KERN_KOPF])], link_werkzeug="Lib-link")
        vst3 = (self.pb / "EqCopilot_artefacts" / "Release" / "VST3" / "EQ-Copilot.vst3" / "Contents"
                / "x86_64-win" / "EQ-Copilot.vst3")
        self.ziel("EqCopilot_VST3", "MODULE_LIBRARY", vst3, "EqCopilot_VST3.tlog",
                  cl=[("eq-copilot/plugin/src/Vst3Huelle.cpp", [])], libs=[shared, kern_lib])
        self.verbraucher.append({"name": "EqCopilot", "typ": "STATIC_LIBRARY", "endziele": ["EqCopilot_VST3"]})
        self.bau_ziele.append("EqCopilot_VST3")
        # Ein Verbraucher, den der Kanon nicht baut (EqCopPipeProbe-Muster)
        probe = self.pb / "EqCopPipeProbe_artefacts" / "Release" / "EqCopPipeProbe.exe"
        self.ziel("EqCopPipeProbe", "EXECUTABLE", probe, "EqCopPip.1FD290DF.tlog",
                  cl=[("eq-copilot/plugin/probe/PipeProbeMain.cpp", [])], libs=[kern_lib])
        self.verbraucher.append({"name": "EqCopPipeProbe", "typ": "EXECUTABLE", "endziele": []})
        if ohne_kern:
            # M-06: ein Kernverbraucher, dessen Link-Leselog die Kernbibliothek nicht nennt
            ohne = self.pb / "EqCopOhneKern_artefacts" / "Release" / "EqCopOhneKern.exe"
            self.ziel("EqCopOhneKern", "EXECUTABLE", ohne, "EqCopOhn.2A2A2A2A.tlog",
                      cl=[("eq-copilot/plugin/tests/OhneKernMain.cpp", [])], libs=[])
            self.verbraucher.append({"name": "EqCopOhneKern", "typ": "EXECUTABLE", "endziele": []})
            self.bau_ziele.append("EqCopOhneKern")
            self.inventar.append("eq-copilot/plugin/tests/OhneKernMain.cpp")
        # A15: stillgelegtes Bein mit altem Binary
        suna = self.pb / "EqCopSunaNullTest_artefacts" / "Release" / "EqCopSunaNullTest.exe"
        self.ziel("EqCopSunaNullTest", "EXECUTABLE", suna, "EqCopSun.5A5A5A5A.tlog",
                  cl=[("eq-copilot/plugin/tests/SunaNullTestMain.cpp", [])], libs=[])
        if stillgelegt_alt:
            self.zeit(suna, T_QUELLE - 3600 * SEKUNDE)
        self.population.append({"kuerzel": "A15", "name": "EqCopSunaNullTest", "binary": str(suna),
                                "stillgelegt": "seit 2026-08-28: Ziel stillgelegt"})
        # Konfiguration und Export
        self.stempel()
        self.export()

    # -- Bausteine -----------------------------------------------------------

    def abs(self, rel: str) -> pathlib.Path:
        return self.w / rel

    def datei(self, rel: str, inhalt: bytes, ns: int) -> pathlib.Path:
        return self.datei_abs(self.abs(rel), inhalt, ns)

    def datei_abs(self, pfad: pathlib.Path, inhalt: bytes, ns: int) -> pathlib.Path:
        pfad.parent.mkdir(parents=True, exist_ok=True)
        pfad.write_bytes(inhalt)
        self.zeit(pfad, ns)
        return pfad

    @staticmethod
    def zeit(pfad: pathlib.Path, ns: int) -> None:
        os.utime(pfad, ns=(ns, ns))

    @staticmethod
    def log(pfad: pathlib.Path, abschnitte_: list[tuple[list[str], list[str]]], utf16: bool = True) -> None:
        text = "".join("^" + "|".join(q).upper() + "\r\n" + "".join(a.upper() + "\r\n" for a in abh)
                       for q, abh in abschnitte_)
        pfad.parent.mkdir(parents=True, exist_ok=True)
        pfad.write_bytes(b"\xff\xfe" + text.encode("utf-16-le") if utf16 else text.encode("utf-8"))

    def ziel(self, name: str, typ: str, binary: pathlib.Path, tlog_name: str,
             cl: list[tuple[str, list[str]]], libs: list[pathlib.Path] | None = None,
             link_werkzeug: str = "link", link_extern: list[str] | None = None) -> None:
        zwischen = self.pb / f"{name}.dir" / "Release"
        tdir = zwischen / tlog_name
        projekt = self.pb / f"{name}.vcxproj"
        self.datei_abs(projekt, b"<Project/>\n", T_QUELLE)
        objekte = []
        cl_lesen, cl_schreiben = [], []
        for quelle, abh in cl:
            q = str(self.abs(quelle))
            obj = zwischen / (pathlib.PurePath(quelle).stem + ".obj")
            self.datei_abs(obj, b"obj", T_BAU)
            objekte.append(str(obj))
            cl_lesen.append(([q], [a if os.path.isabs(a) else str(self.abs(a)) for a in abh]
                             + [r"C:\PROGRAM FILES (X86)\MICROSOFT VISUAL STUDIO\2022\BUILDTOOLS\VC\TOOLS"
                                r"\MSVC\14.44.35207\INCLUDE\VCRUNTIME.H"]))
            cl_schreiben.append(([q], [str(obj)]))
        self.log(tdir / "CL.read.1.tlog", cl_lesen)
        self.log(tdir / "CL.write.1.tlog", cl_schreiben)
        self.log(tdir / f"{link_werkzeug}.read.1.tlog",
                 [(objekte, [str(x) for x in (libs or [])] + list(link_extern or []))])
        self.log(tdir / f"{link_werkzeug}.write.1.tlog", [(objekte, [str(binary)])])
        regel = [str(self.abs("eq-copilot/plugin/CMakeLists.txt"))]
        self.log(tdir / "CustomBuild.read.1.tlog",
                 [(regel, [str(self.abs(MANIFEST_REL)), str(self.abs(HEADER_IN)),
                           str(self.bau / "CMakeFiles" / "cmake.verify_globs")])], utf16=False)
        self.log(tdir / "CustomBuild.write.1.tlog",
                 [(regel, [str(self.pb / "CMakeFiles" / "generate.stamp")])], utf16=False)
        self.datei_abs(binary, b"MZ", T_BAU if binary.suffix.lower() == ".lib" else T_BINARY)
        self.ziele[name] = {"typ": typ, "datei": str(binary).replace("\\", "/")}

    def stempel(self) -> None:
        cmf = self.bau / "CMakeFiles"
        oben = cmf / "generate.stamp"
        plugin = self.pb / "CMakeFiles" / "generate.stamp"
        self.datei_abs(cmf / "cmake.verify_globs", b"# glob\n", T_QUELLE)
        self.datei_abs(oben.with_name("generate.stamp.depend"),
                       ("# CMake generation dependency list for this directory.\n"
                        + str(self.abs("eq-copilot/CMakeLists.txt")).replace("\\", "/") + "\n"
                        + "C:/Program Files/CMake/share/cmake-3.31/Modules/CMakeCXXInformation.cmake\n").encode(),
                       T_STEMPEL)
        self.datei_abs(plugin.with_name("generate.stamp.depend"),
                       ("# CMake generation dependency list for this directory.\n"
                        + str(cmf / "cmake.verify_globs").replace("\\", "/") + "\n"
                        + str(self.w / "eq-copilot" / "cmake").replace("\\", "/")
                        + "/../identity/plugin-identities-v1.json\n"
                        + str(self.abs("eq-copilot/plugin")).replace("\\", "/") + "/../install/"
                        + "nakama-installer-v1.json\n"
                        + str(self.abs("eq-copilot/plugin/CMakeLists.txt")).replace("\\", "/") + "\n"
                        + str(self.abs(HEADER_IN)).replace("\\", "/") + "\n").encode(), T_STEMPEL)
        for s in (oben, plugin):
            self.datei_abs(s, b"# CMake generation timestamp file for this directory.\n", T_STEMPEL)
        self.datei_abs(cmf / "generate.stamp.list",
                       (str(oben).replace("\\", "/") + "\n" + str(plugin).replace("\\", "/") + "\n").encode(),
                       T_STEMPEL)

    def export(self) -> None:
        daten = {"schema": "nakama.frischebaum/v1", "konfiguration": "Release", "kern": "NakamaKern",
                 "kern_quellen": list(KERN), "verbraucher": self.verbraucher, "ziele": self.ziele}
        self.datei_abs(self.pb / "nakama-frischebaum-Release.json",
                       json.dumps(daten, indent=2).encode(), T_STEMPEL)

    def eingabe(self) -> dict:
        return {"konfiguration": "Release", "population": [dict(p) for p in self.population],
                "bau_ziele": list(self.bau_ziele)}

    def lauf(self, bau_bestaetigt: bool = False, eingabe: dict | None = None,
             inventar: list[str] | None = None,
             ausnahmen: tuple[tuple[str, str], ...] | None = None) -> Ergebnis:
        return baustand(self.w, eingabe or self.eingabe(), bau_bestaetigt,
                        git_dateien=list(self.inventar if inventar is None else inventar),
                        ausnahmen=((("eq-copilot/plugin/probe/", "EqCopPipeProbe baut der Kanon nicht"),)
                                   if ausnahmen is None else ausnahmen))

    def alles_neu_gebaut(self, ns: int) -> None:
        """Ein frischer Bau: jedes Objekt, jede Bibliothek und jedes Binary
        bekommt den Zeitstempel ns."""
        for pfad in self.pb.rglob("*"):
            if pfad.is_file() and pfad.suffix.lower() in (".obj", ".lib", ".exe", ".vst3"):
                self.zeit(pfad, ns)


def _stand(e: Ergebnis, name: str) -> dict:
    for b in e.json["binaries"]:
        if b["name"] == name:
            return b
    return {"stand": "(fehlt)", "juengste_eingabe": "", "grund": ""}


def _kreuz(e: Ergebnis, name: str) -> dict:
    for k in e.json.get("kreuzprobe", []):
        if k["name"] == name:
            return k
    return {"status": "(fehlt)", "detail": ""}


def _neue_attrappe(ordner: pathlib.Path, **optionen) -> Attrappe:
    wurzel = pathlib.Path(tempfile.mkdtemp(prefix="nakama a36 ", dir=ordner))
    return Attrappe(wurzel, **optionen)


def _fall_m01(ordner):
    a = _neue_attrappe(ordner)
    p = []
    e = a.lauf()
    p.append((e.exit == 0 and _stand(e, "EqCopDspGoldenTest")["stand"] == STAND_FRISCH,
              "Gegenteil: ohne Aenderung ist B6 frisch und der Baustand Exit 0",
              f"Exit {e.exit}, B6 {_stand(e, 'EqCopDspGoldenTest')['stand']}"))
    a.zeit(a.abs("eq-copilot/plugin/dsp/DspKern.cpp"), T_BINARY + 10 * SEKUNDE)
    e = a.lauf()
    for name in ("EqCopDspGoldenTest", "EqCopTransactionTest"):
        s = _stand(e, name)
        p.append((s["stand"] == STAND_VERALTET and s["juengste_eingabe"] == "eq-copilot/plugin/dsp/DspKern.cpp",
                  f"nur dsp/DspKern.cpp geaendert: {name} VERALTET, juengste Eingabe dsp/DspKern.cpp",
                  f"{s['stand']}, {s['juengste_eingabe']}"))
    p.append((e.exit == 4, "die Beglaubigung wird verweigert (Exit 4)", f"Exit {e.exit}"))
    return "Nur eine DSP-Quelle geaendert -> VERALTET ueber NakamaKern.lib", p


def _fall_m02(ordner):
    a = _neue_attrappe(ordner)
    p = []
    a.zeit(a.abs("eq-copilot/plugin/dsp/DspKern.cpp"), T_BINARY + 10 * SEKUNDE)
    a.alles_neu_gebaut(T_BINARY + 20 * SEKUNDE)
    e = a.lauf()
    p.append((e.exit == 0 and all(_stand(e, n)["stand"] == STAND_FRISCH
                                  for n in ("EqCopDspGoldenTest", "EqCopTransactionTest", "EqCopShot")),
              "frischer Bau nach der DSP-Aenderung: B6, B7, B15 frisch, Exit 0", f"Exit {e.exit}"))
    a.zeit(a.abs("eq-copilot/plugin/tests/ShotTestMain.cpp"), T_BINARY + 30 * SEKUNDE)
    e = a.lauf()
    b6, b7, b15 = (_stand(e, n) for n in ("EqCopDspGoldenTest", "EqCopTransactionTest", "EqCopShot"))
    p.append((b6["stand"] == STAND_FRISCH and b7["stand"] == STAND_FRISCH,
              "eine juengere Quelle ausserhalb ihres Frischebaums laesst B6 und B7 frisch",
              f"B6 {b6['stand']}, B7 {b7['stand']}"))
    p.append((b15["stand"] == STAND_VERALTET and b15["juengste_eingabe"].endswith("tests/ShotTestMain.cpp"),
              "Gegenteil: B15, dessen Frischebaum sie enthaelt, ist VERALTET",
              f"B15 {b15['stand']}, {b15['juengste_eingabe']}"))
    return "Frischer Bau wird angenommen; Vergleich je Binary gegen den eigenen Frischebaum", p


def _fall_m03(ordner):
    a = _neue_attrappe(ordner)
    p = []
    # Die eigene Quelle von B6, nicht eine Kernquelle: seit M-75 steht auch
    # NakamaKern.lib (T_BAU) im Zeitvergleich, und eine Kernquelle auf T_BINARY
    # machte sie zu Recht veraltet - der Zahlenrand gilt dem Binary von B6.
    dsp = a.abs("eq-copilot/plugin/tests/DspGoldenTestMain.cpp")
    a.zeit(dsp, T_BINARY)
    e = a.lauf()
    s = _stand(e, "EqCopDspGoldenTest")
    p.append((s["stand"] == STAND_FRISCH and e.exit == 0,
              "(a) Eingabe mit exakt der LastWriteTime des Binaries: frisch", f"{s['stand']}, Exit {e.exit}"))
    a.zeit(dsp, T_BINARY + 100)
    e = a.lauf()
    s = _stand(e, "EqCopDspGoldenTest")
    p.append((s["stand"] == STAND_VERALTET and e.exit == 4,
              "(b) ein Tick (100 ns) juenger: VERALTET", f"{s['stand']}, Exit {e.exit}"))
    return "Zahlenrand: Gleichstand ist frisch, ein Tick darueber veraltet", p


def _fall_m04(ordner):
    a = _neue_attrappe(ordner)
    p = []
    dsp = a.abs("eq-copilot/plugin/dsp/DspKern.cpp")
    original = dsp.read_bytes()
    dsp.write_bytes(original + b"// Mutante\n")
    a.alles_neu_gebaut(T_BINARY + 10 * SEKUNDE)  # das Binary entsteht aus der Mutante
    dsp.write_bytes(original)
    a.zeit(dsp, T_QUELLE)  # Copy-Item: Inhalt und alter Zeitstempel zurueck
    e = a.lauf()
    s = _stand(e, "EqCopDspGoldenTest")
    p.append((s["stand"] == STAND_FRISCH, "Ruecknahme mit altem Zeitstempel gilt als frisch (Grenze, keine Zusage)",
              s["stand"]))
    p.append((any(GRENZZEILE in zeile for zeile in e.zeilen), "die Ausgabe nennt die Grenze",
              "Grenze: Zeitstempel, kein Inhalt"))
    return "Copy-Item-Ruecknahme ist die dokumentierte Grenze", p


def _fall_m05(ordner):
    a = _neue_attrappe(ordner, stillgelegt_alt=True)
    p = []
    e = a.lauf()
    s = _stand(e, "EqCopSunaNullTest")
    p.append((s["stand"] == STAND_STILLGELEGT and e.exit == 0,
              "stillgelegtes Bein mit altem Binary: 'stillgelegt, nicht beurteilt', Exit 0",
              f"{s['stand']}, Exit {e.exit}"))
    eingabe = a.eingabe()
    for q in eingabe["population"]:
        if q["name"] == "EqCopSunaNullTest":
            q["stillgelegt"] = None
    e = a.lauf(eingabe=eingabe)
    s = _stand(e, "EqCopSunaNullTest")
    p.append((s["stand"] == STAND_VERALTET and e.exit == 4,
              "Gegenteil: dasselbe Binary ohne Marke ist VERALTET (Exit 4)", f"{s['stand']}, Exit {e.exit}"))
    return "Stillgelegtes Bein bleibt draussen", p


def _fall_m06(ordner):
    p = []
    a = _neue_attrappe(ordner)
    e = a.lauf()
    p.append((_kreuz(e, "EqCopDspGoldenTest")["status"] == "gedeckt" and _kreuz(e, "EqCopilot")["status"] == "gedeckt"
              and e.exit == 0,
              "Gegenteil: Verbraucher mit Kern und statischer Verbraucher ueber sein Endziel sind gedeckt, Exit 0",
              f"B6 {_kreuz(e, 'EqCopDspGoldenTest')['status']}, EqCopilot {_kreuz(e, 'EqCopilot')['status']}, "
              f"Exit {e.exit}"))
    p.append((_kreuz(e, "EqCopPipeProbe")["status"] == "nicht im Kanon, nicht beurteilt",
              "ein Verbraucher, den der Kanon nicht baut, wird genannt, nicht beurteilt",
              _kreuz(e, "EqCopPipeProbe")["status"]))
    a = _neue_attrappe(ordner, ohne_kern=True)
    e = a.lauf()
    p.append((_kreuz(e, "EqCopOhneKern")["status"] == "NICHT GEDECKT"
              and "Frischebaum deckt EqCopOhneKern nicht" in e.json["luecken"] and e.exit == 4,
              "Kernverbraucher, dessen Link-Leselog den Kern nicht nennt: 'Frischebaum deckt ... nicht', Exit 4",
              f"{_kreuz(e, 'EqCopOhneKern')['status']}, Exit {e.exit}"))
    os.remove(a.ziele["EqCopTransactionTest"]["datei"])
    e = a.lauf()
    p.append((_kreuz(e, "EqCopTransactionTest")["status"] == STAND_NICHT_GEBAUT and e.exit != 0,
              "ein Kanon-Verbraucher ohne Binary wird genannt, nicht beurteilt - und nie gruen",
              f"{_kreuz(e, 'EqCopTransactionTest')['status']}, Exit {e.exit}"))
    return "Jedes gebundene Kernziel ist gedeckt (Kreuzprobe gegen den CMake-Export)", p


def _fall_m07(ordner):
    p = []
    a = _neue_attrappe(ordner)
    e = a.lauf()
    p.append((e.exit == 0, "Gegenteil: intakter Baum ist ableitbar (Exit 0)", f"Exit {e.exit}"))
    zw = a.pb / "EqCopDspGoldenTest.dir" / "Release"
    tdir = zw / "EqCopDsp.4CC300A8.tlog"

    def pruefe_na(titel: str, e: Ergebnis) -> None:
        s = _stand(e, "EqCopDspGoldenTest")
        p.append((e.exit == 3 and e.json["nicht_ableitbar"] and s["stand"] == STAND_NICHT_ABLEITBAR,
                  titel + ": NICHT ABLEITBAR, Exit 3, nie 0", f"{s['stand']}, Exit {e.exit}, {s['grund'][:90]}"))

    os.remove(tdir / "CL.read.1.tlog")
    pruefe_na("(a) CL-Leselog von B6 fehlt", a.lauf())
    a = _neue_attrappe(ordner)
    tdir = a.pb / "EqCopDspGoldenTest.dir" / "Release" / "EqCopDsp.4CC300A8.tlog"
    os.remove(tdir / "link.write.1.tlog")
    pruefe_na("(a) kein Link-Schreiblog nennt B6", a.lauf())
    a = _neue_attrappe(ordner)
    tdir = a.pb / "EqCopDspGoldenTest.dir" / "Release" / "EqCopDsp.4CC300A8.tlog"
    objekt = str(a.pb / "EqCopDspGoldenTest.dir" / "Release" / "DspGoldenTestMain.obj")
    a.log(tdir / "link.read.1.tlog", [([objekt], [str(a.pb / "Release" / "NakamaKern.lib"),
                                                  str(a.pb / "Release" / "Fremd.lib")])])
    pruefe_na("(b) Bibliothek unter dem Baubaum ohne Erzeuger", a.lauf())
    a = _neue_attrappe(ordner)
    os.remove(a.pb / "nakama-frischebaum-Release.json")
    pruefe_na("(c) CMake-Export fehlt", a.lauf())
    a = _neue_attrappe(ordner)
    os.remove(a.pb / "CMakeFiles" / "generate.stamp.depend")
    pruefe_na("(c) generate.stamp.depend fehlt", a.lauf())
    a = _neue_attrappe(ordner)
    (a.pb / "EqCopDspGoldenTest.dir" / "Release" / "EqCopDsp.ALT00000.tlog").mkdir()
    pruefe_na("(d) zwei Tracking-Ordner im Zwischenordner", a.lauf())
    return "Ohne Ableitung kein Gruen", p


def _fall_m08(ordner):
    p = []
    a = _neue_attrappe(ordner)
    e = a.lauf()
    p.append((e.exit == 0, "Gegenteil: jede getrackte Datei im Frischebaum oder mit Grund ausgenommen, Exit 0",
              f"Exit {e.exit}"))
    rel = "eq-copilot/plugin/tests/VergessenTest.cpp"
    a.datei(rel, b"// vergessen\n", T_QUELLE)
    e = a.lauf(inventar=a.inventar + [rel])
    p.append((f"Frischebaum unvollstaendig: {rel}" in e.json["luecken"] and e.exit == 4,
              "eine getrackte Datei ohne Frischebaum und ohne Ausnahme verweigert die Beglaubigung (Exit 4)",
              f"Exit {e.exit}, Luecken {e.json['luecken']}"))
    e = a.lauf(inventar=a.inventar + [rel],
               ausnahmen=(("eq-copilot/plugin/probe/", "EqCopPipeProbe baut der Kanon nicht"),
                          (rel, "Attrappe: mit Grund ausgenommen")))
    p.append((e.exit == 0, "Gegenteil: dieselbe Datei mit Grund in der Ausnahmeliste, Exit 0", f"Exit {e.exit}"))
    return "Inventar: eine Datei ohne Frischebaum faellt", p


def _fall_m10a(ordner):
    p = []
    a = _neue_attrappe(ordner)
    a.zeit(a.abs(MANIFEST_REL), T_STEMPEL + SEKUNDE)
    e = a.lauf()
    alle = [_stand(e, n) for n in ("EqCopDspGoldenTest", "EqCopTransactionTest", "EqCopShot")]
    p.append((all(s["stand"] == STAND_VERALTET
                  and s["grund"] == "Konfiguration ausstehend: eq-copilot/install/nakama-installer-v1.json"
                  for s in alle) and e.exit == 4,
              "Manifest juenger als generate.stamp: jedes Binary VERALTET 'Konfiguration ausstehend', Exit 4",
              f"Exit {e.exit}, {alle[0]['stand']} {alle[0]['grund']}"))
    a.zeit(a.abs(MANIFEST_REL), T_STEMPEL)
    e = a.lauf()
    p.append((e.exit == 0 and _stand(e, "EqCopDspGoldenTest")["stand"] == STAND_FRISCH,
              "Gegenteil: Manifest nicht juenger als der Stempel, frisch", f"Exit {e.exit}"))
    return "Konfiguration ausstehend ist veraltet (Sicht wie ZERO_CHECK)", p


def _fall_m10b(ordner):
    p = []
    a = _neue_attrappe(ordner)
    a.zeit(a.abs(MANIFEST_REL), T_BINARY + 10 * SEKUNDE)     # --hashen nach dem Bau
    for s in (a.bau / "CMakeFiles" / "generate.stamp", a.pb / "CMakeFiles" / "generate.stamp"):
        a.zeit(s, T_BINARY + 20 * SEKUNDE)                   # neu konfiguriert, Header inhaltsgleich
    e = a.lauf()
    p.append((e.exit == 0 and _stand(e, "EqCopDspGoldenTest")["stand"] == STAND_FRISCH,
              "nach dem Konfigurieren ohne Inhaltsaenderung: frisch, obwohl das Manifest juenger als das Binary ist",
              f"Exit {e.exit}, {_stand(e, 'EqCopDspGoldenTest')['stand']}"))
    a.zeit(a.pb / "EqCopDspGoldenTest.vcxproj", T_BINARY + 20 * SEKUNDE)
    e = a.lauf()
    s = _stand(e, "EqCopDspGoldenTest")
    p.append((s["stand"] == STAND_VERALTET and s["juengste_eingabe"].endswith("EqCopDspGoldenTest.vcxproj")
              and _stand(e, "EqCopTransactionTest")["stand"] == STAND_FRISCH,
              "Gegenteil: geaenderte Projektdatei (Uebersetzeroptionen) macht nur ihr Binary VERALTET",
              f"B6 {s['stand']} {s['juengste_eingabe']}"))
    a.zeit(a.pb / "EqCopDspGoldenTest.vcxproj", T_QUELLE)
    a.zeit(a.erzeugt, T_BINARY + 20 * SEKUNDE)
    e = a.lauf()
    s = _stand(e, "EqCopTransactionTest")
    p.append((s["stand"] == STAND_VERALTET and s["juengste_eingabe"].endswith("BrokerInstallBinding.h"),
              "Gegenteil: ein inhaltlich geaenderter erzeugter Header macht jeden Kernverbraucher VERALTET",
              f"B7 {s['stand']} {s['juengste_eingabe']}"))
    return "Konfiguration ohne Inhaltsaenderung bleibt frisch", p


def _fall_bau(ordner):
    p = []
    a = _neue_attrappe(ordner, ohne_kern=True)
    a.zeit(a.abs("eq-copilot/plugin/dsp/DspKern.cpp"), T_BINARY + 10 * SEKUNDE)
    e = a.lauf(bau_bestaetigt=True)
    p.append((_stand(e, "EqCopDspGoldenTest")["stand"] == STAND_BAU,
              "nach bestaetigtem Bau folgt der Zeitvergleich dem Buildsystem",
              _stand(e, "EqCopDspGoldenTest")["stand"]))
    p.append((e.exit == 4 and "Frischebaum deckt EqCopOhneKern nicht" in e.json["luecken"],
              "Gegenteil: die Kreuzprobe laeuft trotzdem und verweigert (Exit 4)", f"Exit {e.exit}"))
    return "Bau bestaetigt: Kreuzprobe und Inventar laufen weiter", p


def gemessene_im_runner(text: str) -> list[str]:
    """Quelltextwache ueber tools/beweise.ps1 (M-75 (e)): die Ziele aus
    $gemesseneZiele gehen ueber $zuBauen in bau_ziele der Frischepruefung;
    leer = gehalten."""
    befunde: list[str] = []
    block = re.search(r"^\$gemesseneZiele = @\((?P<rumpf>.*?)^\)", text, re.S | re.M)
    ziele = re.findall(r"^\s*Ziel\s*=\s*'([^']+)'", block.group("rumpf"), re.M) if block else []
    if not ziele:
        befunde.append("$gemesseneZiele nennt kein Ziel")
    zeilen = [z.strip() for z in text.splitlines()]
    zufuehrung = (r"\$zuBauen \+= @\(\$gemesseneZiele \| Where-Object \{ \$cmakeText -match "
                  r"\[regex\]::Escape\(\$_\.Marker\) \} \| ForEach-Object \{ \$_\.Ziel \}\)")
    if sum(1 for z in zeilen if re.fullmatch(zufuehrung, z)) != 1:
        befunde.append("die gemessenen Ziele gehen nicht ueber $zuBauen in den Bau")
    if not any("bau_ziele = @($zuBauen)" in z and z.startswith("[ordered]@{") for z in zeilen):
        befunde.append("bau_ziele der Frischepruefung ist nicht $zuBauen")
    return befunde


def _fall_m75(ordner):
    p = []
    a = _neue_attrappe(ordner)
    e = a.lauf()
    vst3, kern = _stand(e, "EqCopilot_VST3"), _stand(e, "NakamaKern")
    p.append((vst3["stand"] == STAND_FRISCH and kern["stand"] == STAND_FRISCH and e.exit == 0
              and all(b["gemessen"] for b in e.json["binaries"] if b["name"] in ("EqCopilot_VST3", "NakamaKern")),
              "Gegenteil: die gemessenen Ziele (EqCopilot_VST3, NakamaKern: in bau_ziele, ohne Bein) stehen im "
              "Zeitvergleich und sind frisch, Exit 0", f"VST3 {vst3['stand']}, Kern {kern['stand']}, Exit {e.exit}"))
    a.zeit(a.abs("eq-copilot/plugin/src/Vst3Huelle.cpp"), T_BINARY + 10 * SEKUNDE)
    e = a.lauf()
    vst3 = _stand(e, "EqCopilot_VST3")
    beine_frisch = all(_stand(e, n)["stand"] == STAND_FRISCH
                       for n in ("EqCopDspGoldenTest", "EqCopTransactionTest", "EqCopShot"))
    p.append((vst3["stand"] == STAND_VERALTET and vst3["juengste_eingabe"].endswith("src/Vst3Huelle.cpp")
              and beine_frisch and e.exit == 4,
              "(a) eine Eingabe nur des gemessenen Ziels juenger: VERALTET, die Beglaubigung wird verweigert "
              "(Exit 4), obwohl jedes Pruefbinary frisch ist",
              f"VST3 {vst3['stand']} {vst3['juengste_eingabe']}, Beine frisch {beine_frisch}, Exit {e.exit}"))
    a = _neue_attrappe(ordner)
    os.remove(a.pb / "EqCopilot_VST3.dir" / "Release" / "EqCopilot_VST3.tlog" / "CL.read.1.tlog")
    e = a.lauf()
    vst3 = _stand(e, "EqCopilot_VST3")
    p.append((vst3["stand"] == STAND_NICHT_ABLEITBAR and e.exit == 3,
              "(b) CL-Leselog des gemessenen Ziels fehlt: NICHT ABLEITBAR, Exit 3, nie gruen",
              f"{vst3['stand']}, Exit {e.exit}"))
    a = _neue_attrappe(ordner)
    del a.ziele["EqCopilot_VST3"]
    a.export()
    e = a.lauf()
    vst3 = _stand(e, "EqCopilot_VST3")
    p.append((vst3["stand"] == STAND_NICHT_ABLEITBAR and "fehlt im CMake-Export" in vst3["grund"] and e.exit == 3,
              "(c) ein gemessenes Ziel, das im CMake-Export fehlt: NICHT ABLEITBAR, nie eine Namensregel",
              f"{vst3['stand']} [{vst3['grund']}], Exit {e.exit}"))
    a = _neue_attrappe(ordner)
    os.remove(a.ziele["EqCopilot_VST3"]["datei"])
    e = a.lauf()
    vst3 = _stand(e, "EqCopilot_VST3")
    p.append((vst3["stand"] == STAND_NICHT_GEBAUT and e.exit != 0,
              "(d) Zieldatei fehlt: genannt, nicht beurteilt - und der Lauf ist nie gruen",
              f"{vst3['stand']}, Exit {e.exit}"))
    text = RUNNER.read_text(encoding="utf-8")
    b = gemessene_im_runner(text)
    p.append((not b, "(e) tools/beweise.ps1: $gemesseneZiele geht ueber $zuBauen in bau_ziele der Frischepruefung",
              "; ".join(b) if b else "gehalten"))
    ohne = "\n".join(z for z in text.splitlines() if not z.strip().startswith("$zuBauen += @($gemesseneZiele"))
    p.append((bool(gemessene_im_runner(ohne)), "Gegenteil: ohne die Zufuehrung faellt die Wache",
              "; ".join(gemessene_im_runner(ohne))))
    return "Gemessene Ziele stehen im Zeitvergleich wie jedes Pruefbinary (R-309-9)", p


def _fall_m67(ordner):
    p = []
    m = pathlib.Path(tempfile.mkdtemp(prefix="nakama a36 meldeweg ", dir=ordner))
    test = "store_weist_reparse_punkt_im_pfad_ab"
    e = nicht_gelaufen(m)
    p.append((e.exit == 0 and not e.marken, "Gegenteil: leerer Meldeordner - jeder Test lief, Exit 0",
              f"Exit {e.exit}"))
    marke = m / f"{test}.nicht-gelaufen"
    marke.write_bytes(f"{test}: Reparse-Fall nicht gemessen: Testschalter\n".encode())
    e = nicht_gelaufen(m)
    p.append((e.exit == 3 and e.marken == [(test, "Reparse-Fall nicht gemessen: Testschalter")],
              "eine Marke <Test>: <Grund> ist NOT RUN mit Test und Grund (Exit 3: fehlende Voraussetzung, "
              "der Runner macht daraus UNVOLLSTAENDIG, nie gruen)", f"Exit {e.exit}, {e.marken}"))
    with open(marke, "ab") as f:
        f.write(f"{test}: zweiter Grund\n".encode())
    e = nicht_gelaufen(m)
    p.append((e.exit == 3 and len(e.marken) == 2 and {t for t, _ in e.marken} == {test},
              "zwei Marken desselben Tests: beide Gruende, weiter NOT RUN", f"Exit {e.exit}, {len(e.marken)} Marken"))
    (m / "anderer_test.nicht-gelaufen").write_bytes(b"anderer_test: Marke eines fremden Beins\n")
    e = nicht_gelaufen(m)
    p.append((e.exit == 3 and ("anderer_test", "Marke eines fremden Beins") in e.marken,
              "eine Marke eines anderen Tests zaehlt ebenso als NOT RUN - nie gruen", f"Exit {e.exit}"))

    def unlesbar(titel: str, name: str, inhalt: bytes | None) -> None:
        u = pathlib.Path(tempfile.mkdtemp(prefix="nakama a36 unlesbar ", dir=ordner))
        if inhalt is None:
            os.rmdir(u)
        else:
            (u / name).write_bytes(inhalt)
        e = nicht_gelaufen(u)
        p.append((e.exit == 2 and not e.marken, f"unlesbar ({titel}): Exit 2, nie leer", f"Exit {e.exit}, {e.grund}"))

    unlesbar("Meldeordner fehlt", "", None)
    unlesbar("fremde Datei", "notiz.txt", b"irgendwas\n")
    unlesbar("Zeile nennt einen anderen Test", f"{test}.nicht-gelaufen", b"anderer_test: Grund\n")
    unlesbar("Marke ohne Grund", f"{test}.nicht-gelaufen", f"{test}: \n".encode())
    unlesbar("leere Marke", f"{test}.nicht-gelaufen", b"")
    unlesbar("kein UTF-8", f"{test}.nicht-gelaufen", test.encode() + b": \xff\xfe\n")
    unlesbar("CRLF statt LF", f"{test}.nicht-gelaufen", f"{test}: Grund\r\n".encode())
    return "NOT RUN aus dem Meldeordner: leer 0, Marke 3, unlesbar 2", p


RUNNER = WURZEL / "tools" / "beweise.ps1"

# Die Zeilen des Urteilsblocks von tools/beweise.ps1 in ihrer Reihenfolge (M-70):
# ROT vor der fehlenden Voraussetzung vor NICHT BEGLAUBIGT, jede mit ihrem Exitcode.
URTEILSFOLGE = (
    r"\$gruen = @\(\$gelaufen \| Where-Object \{ \$_\.Symbol -in @\('\[OK\]', '\[HINWEIS\]'\) \}\)",
    r"\$exitcode = 0",
    r"(?:else)?if \(\$rot -gt 0\) \{",
    r"\$exitcode = 2",
    r'\$urteil = "ROT - .*"',
    r"(?:else)?if \(\$fehlendeVoraussetzung -gt 0\) \{",
    r"\$exitcode = 3",
    r'\$urteil = "UNVOLLSTAENDIG - \$\(\$gruen\.Count\) gruen, .*"',
    r"elseif \(\$veraltet\) \{",
    r"\$exitcode = 4",
    r'\$urteil = "NICHT BEGLAUBIGT - .*"',
    r'\$urteil = "GRUEN - .*"',
)


def urteilsvorrang(text: str) -> list[str]:
    """Quelltextwache ueber den Urteilsblock des Runners (M-70); leer = gehalten.
    NOT RUN zaehlt nie als gruen (die Gruenzaehlung nimmt nur [OK] und [HINWEIS]),
    ist eine fehlende Voraussetzung (Exit 3), und ROT geht vor."""
    zeilen = [z.strip() for z in text.splitlines()]
    befunde: list[str] = []

    def einmal(muster: str) -> int:
        treffer = [i for i, z in enumerate(zeilen) if re.fullmatch(muster, z)]
        if len(treffer) != 1:
            befunde.append(f"{len(treffer)} statt genau einer Zeile: {muster}")
            return -1
        return treffer[0]

    folge = [einmal(m) for m in URTEILSFOLGE]
    if all(i >= 0 for i in folge) and (folge != sorted(folge) or not zeilen[folge[2]].startswith("if ")):
        befunde.append("Reihenfolge des Urteilsblocks verletzt (ROT vor Voraussetzung vor NICHT BEGLAUBIGT)")
    if sum(1 for z in zeilen if z.startswith("$gruen =")) != 1:
        befunde.append("die Gruenzaehlung ist nicht genau eine Zuweisung")
    symbol = einmal(r"\$zeile\.Symbol = '\[NOT RUN\]'")
    if symbol >= 0 and not any(re.fullmatch(r"\$fehlendeVoraussetzung\+\+", z) for z in zeilen[symbol + 1:symbol + 4]):
        befunde.append("NOT RUN zaehlt nicht als fehlende Voraussetzung")
    if not any("$nichtGelaufen Bein(e) NOT RUN, siehe Uebersicht" in z and z.startswith("if ($nichtGelaufen -gt 0)")
               for z in zeilen):
        befunde.append("der Nachsatz 'k Bein(e) NOT RUN' fehlt im Urteil")
    return befunde


def _fall_m70(ordner):
    p = []
    text = RUNNER.read_text(encoding="utf-8")
    b = urteilsvorrang(text)
    p.append((not b, "tools/beweise.ps1: ROT vor fehlender Voraussetzung vor NICHT BEGLAUBIGT, [NOT RUN] "
                     "fehlt in der Gruenzaehlung und ist eine fehlende Voraussetzung",
              "; ".join(b) if b else "gehalten"))
    vertauscht = text.replace("if ($rot -gt 0) {", "if ($__rot -gt 0) {", 1) \
                     .replace("elseif ($fehlendeVoraussetzung -gt 0) {", "if ($rot -gt 0) {", 1) \
                     .replace("if ($__rot -gt 0) {", "elseif ($fehlendeVoraussetzung -gt 0) {", 1)
    p.append((bool(urteilsvorrang(vertauscht)), "Gegenteil: dieselbe Quelle mit vertauschten Zweigen faellt",
              "; ".join(urteilsvorrang(vertauscht))[:120]))
    mitgezaehlt = text.replace("@('[OK]', '[HINWEIS]')", "@('[OK]', '[HINWEIS]', '[NOT RUN]')", 1)
    p.append((bool(urteilsvorrang(mitgezaehlt)), "Gegenteil: [NOT RUN] in der Gruenzaehlung faellt",
              "; ".join(urteilsvorrang(mitgezaehlt))[:120]))
    return "Urteilsvorrang: ROT vor NOT RUN, NOT RUN nie gruen (Quelltextwache)", p


SELBSTTEST_FAELLE = (
    ("M-01", "dsp_aenderung_ohne_bau_verweigert", _fall_m01),
    ("M-02", "frischer_bau_akzeptiert", _fall_m02),
    ("M-03", "gleichstand_und_ein_tick", _fall_m03),
    ("M-04", "copy_item_ruecknahme_ist_grenze", _fall_m04),
    ("M-05", "stillgelegt_bleibt_draussen", _fall_m05),
    ("M-06", "kernverbraucher_ohne_kern_faellt", _fall_m06),
    ("M-07", "ohne_ableitung_kein_gruen", _fall_m07),
    ("M-08", "inventarluecke_faellt", _fall_m08),
    ("M-10", "configure_ausstehend_ist_veraltet", _fall_m10a),
    ("M-10", "configure_ohne_inhaltsaenderung_bleibt_frisch", _fall_m10b),
    ("--bau-bestaetigt", "bau_bestaetigt_prueft_deckung", _fall_bau),
    ("M-67", "nicht_gelaufen_macht_unvollstaendig", _fall_m67),
    ("M-70", "urteilsvorrang_rot_vor_nicht_gelaufen", _fall_m70),
    ("M-75", "gemessene_ziele_im_zeitvergleich", _fall_m75),
)


def selbsttest(nur: str | None) -> int:
    faelle = [f for f in SELBSTTEST_FAELLE if nur is None or nur in (f[0], f[1])]
    if not faelle:
        print(f"WERKZEUGFEHLER - kein Selbsttestfall heisst {nur!r}")
        return 2
    print("Selbsttest pruefe_beweisrunner.py (NAK-309 Bein A36): Attrappen-Baubaum im Temp-Ordner, "
          "jede Erwartung mit ihrem Gegenteil")
    bestanden = 0
    with tempfile.TemporaryDirectory(prefix="nakama-a36-") as tmp:
        for zeile, name, fall in faelle:
            try:
                titel, pruefungen = fall(pathlib.Path(tmp))
            except Exception as f:  # noqa: BLE001 - ein abgestuerzter Fall ist rot
                titel, pruefungen = "Fall abgebrochen", [(False, f"{type(f).__name__}: {f}", "")]
            ok = bool(pruefungen) and all(x[0] for x in pruefungen)
            print(("  ok      " if ok else "  ROT     ") + f"[{zeile} {name}] {titel}")
            for p_ok, text, detail in pruefungen:
                print(("      ok   " if p_ok else "      ROT  ") + text + (f"  [{detail}]" if detail else ""))
            bestanden += ok
    print()
    print(f"Selbsttest: {bestanden} von {len(faelle)} Faellen bestanden")
    return 0 if bestanden == len(faelle) else 4


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(description="NAK-309: Frischepruefung des Beweis-Runners (Bein A36)")
    g = p.add_mutually_exclusive_group(required=True)
    g.add_argument("--baustand", type=pathlib.Path, metavar="POPULATION_JSON",
                   help="Baustand der Population gegen den echten Baubaum (Aufruf des Runners)")
    g.add_argument("--selbsttest", action="store_true", help="Kanon-Bein A36: Attrappen-Baubaum")
    g.add_argument("--nicht-gelaufen", type=pathlib.Path, metavar="MELDEORDNER",
                   help="NOT-RUN-Marken eines cargo-Beins lesen (Aufruf des Runners)")
    p.add_argument("--bau-bestaetigt", action="store_true",
                   help="-Bauen hat unmittelbar vorher erfolgreich gebaut")
    p.add_argument("--nur", metavar="FALL", help="nur diesen Selbsttestfall (Name oder Matrixzeile)")
    p.add_argument("--wurzel", type=pathlib.Path, default=WURZEL,
                   help="Arbeitskopie (Vorgabe: zwei Ebenen ueber diesem Skript)")
    args = p.parse_args(argv)
    try:
        if args.selbsttest:
            return selbsttest(args.nur)
        if args.nicht_gelaufen is not None:
            return nicht_gelaufen_cli(args.nicht_gelaufen)
        return baustand_cli(args.baustand, args.bau_bestaetigt, args.wurzel.resolve())
    except Exception as f:  # noqa: BLE001 - Werkzeugfehler, nie ein Traceback als Urteil
        print(f"WERKZEUGFEHLER - {type(f).__name__}: {f}")
        return 2


if __name__ == "__main__":
    sys.exit(main())
