"""Szenario-Runner des Laufzeit-Arms (Plan S25e, Register NAK-286).

Fährt ein Szenario (JSON unter docs/gesundheit/szenarien/) direkt über die
Bibliothek des FL-Studio-MCP (loopMIDI -> Controller-Skript -> JSON), ohne
den MCP-Umweg, und schreibt jeden Schritt roh in eine Markdown-Datei.

Aufruf (aus dem MCP-Repo heraus, damit `fl_studio_mcp` importierbar ist):

    uv run --directory C:\\Users\\phili\\Projekte\\fl-studio-mcp --python 3.12 \
        python <repo>/tools/fl/szenario.py <szenario.json> [--roh <datei.md>] \
        [--diagnose-pid <pid>] [--render-ordner <ordner>] [--head <sha8>]
    ... szenario.py --ping            # nur Ping, JSON auf stdout
    py -3.13 tools/fl/szenario.py --selbsttest [--nur <fall>]   # ohne FL, ohne MCP-Repo
    py -3.13 tools/fl/szenario.py --rechne <auftrag.json> --ausgabe <ergebnis.json>
    py -3.13 tools/fl/szenario.py --energieprofil <Testtrack.wav>

Exitcodes: 0 bestanden · 3 Voraussetzung des Laufs fehlt (kein Port, kein Ping,
Ping ohne passende Anforderungskennung, Piano-Roll-Weg im Importgraphen) ·
4 mindestens eine Erwartung verfehlt · 2 Szenariodatei unbrauchbar ·
5 Voraussetzung dieses Szenarios fehlt (KETTE, Formatfehler, VERSATZ, fehlendes
Nulltesturteil, fehlende Rechnung aus F-28, Umlauf ohne Wrap, fremde Quelle,
UNGEMESSEN: eine MCP-Antwort ohne passende Anforderungskennung).
Innerhalb eines Szenarios gilt der Vorrang 3 vor 5 vor 4 (VORAUSSETZUNG vor
VERFEHLT); der Runner fährt nach 5 weiter und bricht nur nach 3 ab.

Szenarioformat:

    {"id": "...", "titel": "...", "frischer_start": true, "schritte": [
        {"aktion": "system.ping", "erwarte": {"program_title": {"enthaelt": "FL Studio"}}},
        {"aktion": "mixer.getPeaks", "params": {"track": 1}, "erwarte": {"peak_max": {"min": 0.01}}},
        {"aktion": "mixer.getRouting", "params": {"track": 1}, "erwarte": {"routes.*.level": {"roh": true}}},
        {"aktion": "lokal.briefkasten", "params": {"rollen": ["gen", "probeeq", "broker"]}},
        {"warte_s": 3}
    ]}

Erwartungen je Feld: gleich · enthaelt · min · max · nicht_leer (true) ·
endlich (true) · bereich ([min, max]) · in_menge (Liste) · teilmenge (Liste) ·
roh (true: Wert schreiben, nicht bewerten; null bleibt null). Feldnamen mit
Punkt sind Pfade (`a.b.c`, Zahl = Listenindex, `*` = alle Elemente); ein
Feldname ohne Punkt liest das oberste Feld wie bisher. `frist_s` je Schritt;
ohne `frist_s` entscheidet der MCP (`FL_MCP_TIMEOUT`, sonst 2 s). Ein Timeout
wird genau einmal wiederholt, mit derselben Frist (FL verschluckt gelegentlich
einen MIDI-Trigger); ein zweiter Timeout ist ein verfehlter Schritt.

Antwortzuordnung (NAK-309, R-309-2): jeder Versuch trägt eine neue
Anforderungskennung (`request_id`, 32 Hex-Zeichen), die Wiederholung eine
eigene. Nur eine Antwort mit genau dieser Kennung ist eine Messung; eine mit
fremder oder ohne Kennung heißt UNGEMESSEN, wird verworfen und gezählt, und der
Schritt misst nichts. Die Szenariodateien fragen dazu das Echo ab (`track`,
`index`, `mode`). Den Importweg des MCP-Pakets hält `lade_mcp_modul`:
`fl_studio_mcp/utils/__init__.py` läuft nie, und `fl_trigger` oder `pynput` im
Prozess ist Exit 3 (NAK-286 M-22).

Lokale Aktionen gehen nie an FL: lokal.briefkasten, lokal.nulltest,
lokal.fenster, lokal.umlauf, lokal.stellen (Beschreibung in tools/fl/LIES-MICH.md).
Der Inhalt einer Antwortdatei wird nie als Pfad oder Parameter übernommen;
Auswahl und Größenriegel entscheiden an Name und Größe, bevor eine Datei
geöffnet wird.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import math
import os
import re
import secrets
import subprocess
import sys
import tempfile
import time
import traceback
from collections import Counter
from datetime import datetime, timezone
from pathlib import Path

EXIT_OK, EXIT_DATEI, EXIT_VORAUSSETZUNG, EXIT_VERFEHLT, EXIT_SZENARIO = 0, 2, 3, 4, 5
_RANG = {EXIT_OK: 0, EXIT_VERFEHLT: 1, EXIT_DATEI: 1, EXIT_SZENARIO: 2, EXIT_VORAUSSETZUNG: 3}

REPO = Path(__file__).resolve().parents[2]
LOCALAPPDATA = Path(os.environ.get("LOCALAPPDATA") or (Path.home() / "AppData" / "Local"))

ANFRAGE_FORMAT = "nakama.diagnose.anfrage.v1"
ANTWORT_FORMAT = "nakama.diagnose.antwort.v1"
NULLTEST_FORMAT = "nakama.laufzeit.nulltest.v1"
RENDER_FORMAT = "nakama.laufzeit.render.v1"                 # Renderstatus des Runners (laufzeit.ps1, F-18, P-21)
REFERENZ_FORMAT = "nakama.laufzeit.nulltest.referenz.v1"    # Ergebnis eines Referenzschritts (nulltest.py, M-64)
UMSCHLAG_SCHLUESSEL = frozenset({"format", "anfrage_id", "rolle", "instanz_id", "laufzeit_id", "pid",
                                 "erzeugt_utc", "version", "snapshot", "frame", "aggregat", "gruende"})
KENNUNG = re.compile(r"^[0-9a-f]{32}$")
ANTWORT_NAME = re.compile(r"^([0-9a-f]{32})\.(gen|probeeq|broker)\.([0-9]+)\.([0-9a-f]{16})\.([0-9a-f]{32})\.json$")
TEMP_NAME = re.compile(r"^[0-9a-f]{32}\.(gen|probeeq|broker)\.[0-9]+\.[0-9a-f]{16}\.[0-9a-f]{32}\.json\.tmp-[0-9]+$")
ANTWORT_MIN_BYTES = 2
ANTWORT_MAX_BYTES = 16 * 1024 * 1024  # 16 MiB (M-53 (c))
ANTWORT_FRIST_S = 10.0                # je Anfrage (M-56)
NACHLAUF_S = 2.2                      # zwei Briefkastentakte (1 000 ms, F-12) plus Schreibzeit: spaete Doppelte sehen
POSITION_ABSTAND_S = 0.25             # > 2 Audiopuffer (4 096 Samples / 44 100 Hz = 92,9 ms): Gleichstand heisst Stillstand
FENSTER_FRIST_S = 5.0                 # M-21, erzwungen ueber den Unterprozess der Erfassung (P-19)
PLUGIN_FENSTER_MIN = (200, 100)       # P-17: ein Plugin-Fensterbild unter Breite 200 oder Hoehe 100 ist eingeklappt
# Die Erfassung laeuft im Unterprozess (P-19): der Interpreter des Szenarioprozesses - unter dem Runner das Python
# des MCP-Venvs, laufzeit.ps1 startet szenario.py ueber uv run - ruft nur capture_process_window und schreibt dessen
# Antwort als eine JSON-Zeile. Argumente: PID, Ziel, Bilderordner, Name, Plugin (leer = keins). Das Modul kommt ueber
# lade_mcp_modul aus dieser Datei (NAK-309 M-51): derselbe Stellvertreter wie im Szenarioprozess, dieselbe Wache - steht
# der Piano-Roll-Weg danach im Prozess, endet das Skript vor der Erfassung.
FENSTER_SKRIPT = ("import json, sys; from pathlib import Path; sys.path.insert(0, {hier!r}); "
                  "from szenario import lade_mcp_modul; "
                  "erfasse = lade_mcp_modul('fl_studio_mcp.utils.fenster').capture_process_window; a = sys.argv[1:6]; "
                  "print(json.dumps(erfasse(int(a[0]), a[1], Path(a[2]), a[3], plugin=a[4] or None), "
                  "ensure_ascii=True, default=str))").format(hier=str(Path(__file__).resolve().parent))

RATE = 44100
ZELLE = 4410                          # 0,1 s bei 44 100 Hz (AnalyseEngine.cpp:270)
QUELLE_FRAMES = 5_470_096             # Testtrack.wav (Manifest §0.4)
METRICS_VERSION = "m4.1-2026-08-15"   # AnalyseEngine.h:60
AKTIV_GATE_DB = -60.0                 # AnalyseEngine.cpp:13
TAKT_S = 4 * 60 / 130                 # Testtrack mit 130 BPM (KALIBRIER-PROTOKOLL.md:66)


def schlechter(a: int, b: int) -> int:
    """Vorrang innerhalb eines Szenarios: 3 vor 5 vor 4 und 2 vor 0."""
    return a if _RANG.get(a, 1) >= _RANG.get(b, 1) else b


def _json(wert) -> str:
    return json.dumps(wert, ensure_ascii=False, sort_keys=True)


def _kompakt(obj, limit: int = 400) -> str:
    text = json.dumps(obj, ensure_ascii=False, sort_keys=True)
    return text if len(text) <= limit else text[: limit - 1] + "…"


def _zelle(text) -> str:
    """Markdown-Tabellenzelle: Pipe maskiert, Zeilenumbruch entfernt."""
    return str(text).replace("|", "\\|").replace("\n", " ")


def _ist_zahl(wert) -> bool:
    return isinstance(wert, (int, float)) and not isinstance(wert, bool)


def _utc() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def sha256_datei(pfad: Path) -> str | None:
    try:
        h = hashlib.sha256()
        with open(pfad, "rb") as f:
            for block in iter(lambda: f.read(1 << 20), b""):
                h.update(block)
        return h.hexdigest().upper()
    except OSError:
        return None


# ---------------------------------------------------------------- Erwartungen

ERWARTUNGSARTEN = ("gleich", "enthaelt", "min", "max", "nicht_leer", "endlich", "bereich", "in_menge",
                   "teilmenge", "roh")
_FEHLT = object()


def werte_an_pfad(obj, pfad: str) -> list[tuple[str, object]]:
    """(konkreter Pfad, Wert) je Treffer. Ohne Punkt: das oberste Feld wie in der
    ersten Fassung (fehlend = None). Mit Punkt: Zahl = Listenindex, `*` = alle."""
    if "." not in pfad:
        return [(pfad, obj.get(pfad) if isinstance(obj, dict) else None)]
    treffer: list[tuple[str, object]] = [("", obj)]
    for teil in pfad.split("."):
        neu: list[tuple[str, object]] = []
        for weg, wert in treffer:
            vorn = f"{weg}." if weg else ""
            if teil == "*":
                if isinstance(wert, list):
                    neu += [(f"{vorn}{i}", w) for i, w in enumerate(wert)]
                elif isinstance(wert, dict):
                    neu += [(f"{vorn}{k}", w) for k, w in wert.items()]
            elif isinstance(wert, list) and re.fullmatch(r"-?\d+", teil):
                i = int(teil)
                neu.append((f"{vorn}{teil}", wert[i] if -len(wert) <= i < len(wert) else _FEHLT))
            elif isinstance(wert, dict):
                neu.append((f"{vorn}{teil}", wert.get(teil, _FEHLT)))
            else:
                neu.append((f"{vorn}{teil}", _FEHLT))
        treffer = neu
    return treffer


def _pruefe(erwartung: dict, antwort: dict) -> tuple[list[str], list[str]]:
    """Verfehlte Erwartungen (leer = bestanden) und Rohwerte der Art `roh`."""
    fehler: list[str] = []
    roh: list[str] = []
    if not antwort.get("success"):
        return [f"Antwort ohne Erfolg: {antwort.get('error', antwort)}"], roh
    for feld, regel in erwartung.items():
        treffer = werte_an_pfad(antwort, feld)
        for art, soll in regel.items():
            if art not in ERWARTUNGSARTEN:
                fehler.append(f"{feld}: unbekannte Erwartungsart {art!r}")
                continue
            if art == "roh":
                if soll:
                    werte = [None if w is _FEHLT else w for _, w in treffer]
                    roh.append(f"{feld}={_json(werte if '*' in feld else (werte[0] if werte else None))}")
                continue
            if art == "teilmenge":
                werte = [w for _, w in treffer if w is not _FEHLT]
                if "*" not in feld and len(werte) == 1 and isinstance(werte[0], list):
                    werte = werte[0]
                if not werte or any(w not in soll for w in werte):
                    fehler.append(f"{feld}: erwartet nicht leere Teilmenge von {soll!r}, ist {werte!r}")
                continue
            if not treffer:
                fehler.append(f"{feld}: kein Wert an diesem Pfad")
                continue
            for weg, wert in treffer:
                w = None if wert is _FEHLT else wert
                if art == "gleich" and w != soll:
                    fehler.append(f"{weg}: erwartet gleich {soll!r}, ist {w!r}")
                elif art == "enthaelt" and (w is None or str(soll) not in str(w)):
                    fehler.append(f"{weg}: erwartet enthält {soll!r}, ist {w!r}")
                elif art == "min" and (not _ist_zahl(w) or w < soll):
                    fehler.append(f"{weg}: erwartet ≥ {soll}, ist {w!r}")
                elif art == "max" and (not _ist_zahl(w) or w > soll):
                    fehler.append(f"{weg}: erwartet ≤ {soll}, ist {w!r}")
                elif art == "nicht_leer" and soll and not w:
                    fehler.append(f"{weg}: erwartet nicht leer, ist {w!r}")
                elif art == "endlich" and soll and not (_ist_zahl(w) and math.isfinite(w)):
                    fehler.append(f"{weg}: erwartet endliche Zahl, ist {w!r}")
                elif art == "bereich" and not (_ist_zahl(w) and soll[0] <= w <= soll[1]):
                    fehler.append(f"{weg}: erwartet in [{soll[0]}, {soll[1]}], ist {w!r}")
                elif art == "in_menge" and w not in soll:
                    fehler.append(f"{weg}: erwartet eines von {soll!r}, ist {w!r}")
    return fehler, roh


# ---------------------------------------------------------------- Importweg des MCP-Pakets (NAK-286 M-22, NAK-309)

MCP_UTILS = "fl_studio_mcp.utils"
PIANO_ROLL_WEG = ("fl_studio_mcp.utils.fl_trigger", "pynput")


class ImportgraphFehler(RuntimeError):
    """Nach dem Import steht der Piano-Roll-Weg (fl_trigger, pynput) im Prozess."""


def piano_roll_module() -> list[str]:
    return sorted(m for m in sys.modules if any(m == w or m.startswith(w + ".") for w in PIANO_ROLL_WEG))


def lade_mcp_modul(name: str):
    """Ein Modul aus fl_studio_mcp.utils laden, ohne dass dessen __init__.py laeuft (NAK-309 M-50, M-51).

    Python laedt vor fl_studio_mcp.utils.connection das Elternpaket fl_studio_mcp.utils, und dessen __init__.py
    importiert fl_trigger (Nachbarrepo). Ein Modul, das schon in sys.modules steht, erfuellt den Import, ohne
    ausgefuehrt zu werden, und ein Unterpaket wird ueber das __path__ seines Elternpakets gefunden
    (Python-Sprachreferenz "The import system", 5.3, 5.3.1, 5.3.4). Deshalb steht vor dem Import ein Stellvertreter
    mit dem echten __path__ in sys.modules. Danach prueft eine Wache sys.modules; steht der Piano-Roll-Weg dort -
    auch weil ein anderer ihn vorher geladen hat -, wirft sie ImportgraphFehler mit dem Modulnamen."""
    import importlib
    import importlib.machinery
    import importlib.util

    if MCP_UTILS not in sys.modules:
        paket = importlib.import_module("fl_studio_mcp")
        if MCP_UTILS not in sys.modules:
            spec = importlib.machinery.ModuleSpec(MCP_UTILS, None, is_package=True)
            spec.submodule_search_locations = [str(Path(p) / "utils") for p in paket.__path__]
            stellvertreter = importlib.util.module_from_spec(spec)
            sys.modules[MCP_UTILS] = stellvertreter
            paket.utils = stellvertreter
    modul = importlib.import_module(name)
    geladen = piano_roll_module()
    if geladen:
        raise ImportgraphFehler(f"Piano-Roll-Weg geladen: {', '.join(geladen)} (NAK-286 M-22)")
    return modul


def _nimmt_kennung(funktion) -> bool:
    """Nimmt send_command eine Anforderungskennung an (MCP-Stand ab NAK-309, M-49)?"""
    import inspect

    try:
        parameter = inspect.signature(funktion).parameters
    except (TypeError, ValueError):
        return False
    return "request_id" in parameter or any(p.kind is inspect.Parameter.VAR_KEYWORD for p in parameter.values())


# ---------------------------------------------------------------- Umgebung

class Umgebung:
    """Alle Wirkungen nach aussen: Uhr, Dateien, MCP-Verbindung, Prozesse,
    Unterprozesse. Der Selbsttest ersetzt sie durch Attrappen."""

    def jetzt(self) -> float:
        return time.monotonic()

    def schlafe(self, sekunden: float) -> None:
        if sekunden > 0:
            time.sleep(sekunden)

    def verbindung(self):
        try:
            verbindung_modul = lade_mcp_modul("fl_studio_mcp.utils.connection")
        except ImportgraphFehler as e:
            return None, f"Importgraph: {e}"
        except ImportError as e:
            raise SystemExit(
                f"fl_studio_mcp nicht importierbar ({e}); aus dem MCP-Repo starten: "
                "uv run --directory <fl-studio-mcp> python szenario.py ..."
            ) from e
        conn = verbindung_modul.get_connection()
        if not _nimmt_kennung(conn.send_command):
            return None, ("MCP-Bibliothek ohne Anforderungskennung (send_command ohne request_id): Stand gegen "
                          "tools/fl/mcp-stand.json pruefen (NAK-309 M-49)")
        try:
            conn.ensure_connected()
        except RuntimeError as e:
            return None, str(e)
        return conn, None

    def liste(self, ordner: Path) -> list[tuple[str, int]]:
        """Namen und Groessen aus dem Verzeichniseintrag - ohne eine Datei zu oeffnen."""
        try:
            with os.scandir(ordner) as eintraege:
                return [(e.name, e.stat(follow_symlinks=False).st_size)
                        for e in eintraege if e.is_file(follow_symlinks=False)]
        except FileNotFoundError:
            return []

    def lies(self, pfad: Path, grenze: int) -> bytes:
        with open(pfad, "rb") as f:
            return f.read(grenze + 1)

    def lies_text(self, pfad: Path) -> str | None:
        try:
            return pfad.read_text(encoding="utf-8")
        except OSError:
            return None

    def existiert(self, pfad: Path) -> bool:
        return pfad.exists()

    def lege_ordner_an(self, pfad: Path) -> None:
        pfad.mkdir(parents=True, exist_ok=True)

    def loesche(self, pfad: Path) -> None:
        try:
            pfad.unlink()
        except FileNotFoundError:
            pass

    def schreibe_atomar(self, pfad: Path, daten: bytes) -> None:
        """Temp-Datei, dann Umbenennen mit Ersetzen (M-55); ein kurz geoeffnetes
        Ziel (eine Instanz liest gerade) wird einige Male erneut versucht."""
        tmp = pfad.with_name(f"{pfad.name}.tmp-{os.getpid()}")
        tmp.write_bytes(daten)
        for versuch in range(100):
            try:
                os.replace(tmp, pfad)
                return
            except PermissionError:
                if versuch == 99:
                    raise
                time.sleep(0.01)

    def kennung(self) -> str:
        """128 Bit Zufall als 32 Hex-Zeichen (F-2)."""
        return secrets.token_hex(16)

    def mcp_kennung(self) -> str:
        """Anforderungskennung je MCP-Versuch (NAK-309, R-309-2): 128 Bit Zufall als 32 Hex-Zeichen."""
        return secrets.token_hex(16)

    def sha256(self, pfad: Path) -> str | None:
        return sha256_datei(pfad)

    def schreibzeit(self, pfad: Path) -> float | None:
        """Aenderungszeit einer Datei auf der Uhr von jetzt(): die Taktphase einer Instanz
        steht in der Schreibzeit ihrer Antwort, nicht in der Zeit, zu der der Runner sie sieht
        (Laufzeitlauf 15.09.2026: Eingang bis 0,1 s nach dem Schreiben)."""
        try:
            return os.stat(pfad).st_mtime - time.time() + time.monotonic()
        except OSError:
            return None

    @staticmethod
    def tasklist(abbild: str) -> list[list[str]]:
        """Zeilen von `tasklist /FO CSV /NH` als Felder. Die Konsole schreibt in der
        OEM-Codepage; auf einem deutschen Windows traegt die Meldung ohne Treffer einen
        Umlaut (0x81). Mit text=True scheiterte daran der Lesefaden von subprocess und
        stdout kam als None (Laufzeitlauf 15.09.2026, Szenario snapshot-runde01) - deshalb
        Bytes, dekodiert mit Ersetzung."""
        try:
            r = subprocess.run(["tasklist", "/FI", f"IMAGENAME eq {abbild}", "/FO", "CSV", "/NH"],
                               capture_output=True, timeout=15)
        except (OSError, subprocess.SubprocessError):
            return []
        text = (r.stdout or b"").decode("oem" if os.name == "nt" else "utf-8", errors="replace")
        return [[t.strip('"') for t in zeile.split('","')] for zeile in text.splitlines()]

    def broker_pids(self) -> list[int]:
        return [int(t[1]) for t in self.tasklist("eqcop-broker.exe")
                if len(t) > 1 and t[0].lower().startswith("eqcop-broker") and t[1].isdigit()]

    def fl_pids(self) -> list[int]:
        return [int(t[1]) for t in self.tasklist("FL64.exe")
                if len(t) > 1 and t[0].lower() == "fl64.exe" and t[1].isdigit()]

    @staticmethod
    def _unterprozess(befehl: list[str], frist: float, bezeichnung: str) -> tuple[int, str, str]:
        """Unterprozess mit erzwungener Frist (P-19): subprocess.run(timeout=frist) beendet ihn bei Ablauf, und der
        Aufruf kehrt spaetestens dann mit Exit 2 und "Frist <n> s ueberschritten" zurueck. Das traegt nur ein Startweg,
        bei dem das Beenden des gestarteten Prozesses den arbeitenden beendet: py -3.13 und das Python eines Venvs
        ja, uv run nicht (gemessen 15.09.2026, Blockade 25 s, Frist 5 s: Rueckkehr nach 5,0 s gegen 25,1 s)."""
        umgebung = dict(os.environ, PYTHONIOENCODING="utf-8")
        try:
            r = subprocess.run(befehl, capture_output=True, text=True, encoding="utf-8", errors="replace",
                               timeout=frist, env=umgebung)
        except subprocess.TimeoutExpired:
            return 2, "", f"Frist {frist:g} s ueberschritten: {bezeichnung}"
        except OSError as e:
            return 2, "", f"{bezeichnung}: {e}"
        return r.returncode, r.stdout or "", r.stderr or ""

    def _python313(self, argumente: list[str], frist: float) -> tuple[int, str]:
        befehl = ["py", "-3.13", *argumente]
        code, aus, fehler = self._unterprozess(befehl, frist, " ".join(befehl))
        return code, aus + fehler

    def nulltest(self, argumente: list[str]) -> tuple[int, str]:
        return self._python313([str(Path(__file__).with_name("nulltest.py")), *argumente], 900)

    def rechne(self, auftrag: dict) -> dict:
        with tempfile.TemporaryDirectory(prefix="nak286-rechne-") as tmp:
            ein, aus = Path(tmp) / "auftrag.json", Path(tmp) / "ergebnis.json"
            ein.write_text(json.dumps(auftrag), encoding="utf-8")
            code, text = self._python313([str(Path(__file__).resolve()), "--rechne", str(ein), "--ausgabe", str(aus)],
                                         3600)
            if code != 0 or not aus.exists():
                return {"fehler": f"Rechnung Exit {code}: {text[-1500:]}"}
            return json.loads(aus.read_text(encoding="utf-8"))

    def fenster_befehl(self, pid: int, ziel: str, ordner: Path, name: str, plugin: str | None) -> list[str]:
        return [sys.executable, "-c", FENSTER_SKRIPT, str(pid), ziel, str(ordner), name, plugin or ""]

    def fenster(self, pid: int, ziel: str, ordner: Path, name: str, plugin: str | None) -> dict:
        """Fenstererfassung als Unterprozess mit der Frist aus M-21 (P-19); die Antwort ist seine letzte JSON-Zeile."""
        code, aus, fehler = self._unterprozess(self.fenster_befehl(pid, ziel, ordner, name, plugin),
                                               frist=FENSTER_FRIST_S, bezeichnung=f"Fenstererfassung {ziel} (PID {pid})")
        zeile = next((z for z in reversed(aus.splitlines()) if z.startswith("{")), None)
        if zeile is None:
            return {"success": False, "error": (fehler.strip() or f"Erfassung Exit {code} ohne Antwort")[-600:]}
        try:
            antwort = json.loads(zeile)
        except ValueError as e:
            return {"success": False, "error": f"Erfassung Exit {code}: Antwort ist kein JSON ({e})"}
        if not isinstance(antwort, dict):
            return {"success": False, "error": f"Erfassung Exit {code}: Antwort ist kein JSON-Objekt"}
        return antwort


# ---------------------------------------------------------------- Briefkasten: der Runner als Anfragender und Leser

class Antwort:
    """Eine gewertete Antwortdatei: Name, Rolle, PID aus dem Namen; der Umschlag
    liefert nur Messwerte, nie einen Pfad oder Parameter."""

    def __init__(self, name: str, rolle: str, pid: int, groesse: int, umschlag: dict, kennung: str):
        self.name = name
        self.rolle = rolle
        self.pid = pid
        self.groesse = groesse
        self.umschlag = umschlag
        self.kennung = kennung
        self.anfrage: Anfrage | None = None

    @property
    def snapshot(self) -> dict:
        return self.umschlag.get("snapshot") or {}

    @property
    def frame(self) -> dict:
        return self.umschlag.get("frame") or {}

    def wert(self, pfad: str):
        if "." not in pfad:
            return self.umschlag.get(pfad)
        treffer = werte_an_pfad(self.umschlag, pfad)
        if not treffer:
            return None
        wert = treffer[0][1]
        return None if wert is _FEHLT else wert


class Auswahl:
    def __init__(self, kennung: str):
        self.kennung = kennung
        self.kandidaten: dict[str, list[Antwort]] = {}
        self.gewertet: dict[str, Antwort] = {}
        self.mehrdeutig: dict[str, list[Antwort]] = {}
        self.fehlt: list[str] = []
        self.fremd: list[tuple[str, str]] = []
        self.verworfen: list[tuple[str, str]] = []


class Anfrage:
    def __init__(self, kennung: str, geschrieben: float, erwartet: dict[str, set[int]], notizen: list[str]):
        self.kennung = kennung
        self.geschrieben = geschrieben
        self.erwartet = erwartet
        self.notizen = notizen
        self.eingang: dict[str, float] = {}
        self.auswahl: Auswahl | None = None
        self.p_vor = None
        self.p_nach = None


class Briefkasten:
    def __init__(self, umg: Umgebung, diagnose_ordner: Path):
        self.umg = umg
        self.ordner = diagnose_ordner
        self.antworten = diagnose_ordner / "antwort"
        self.anfrage_pfad = diagnose_ordner / "anfrage.json"
        self.bereit = False
        self.vorgelegt: set[str] = set()
        self.cache: dict[str, tuple[int, dict | None, str | None]] = {}

    def bereite(self) -> None:
        """Ordner anlegen; zuerst eine liegende Anfrage, dann Antworten frueherer
        Laeufe entfernen (§10.2 P-1) - nur Antwort- und Temp-Namen, sonst nichts."""
        if self.bereit:
            return
        self.umg.lege_ordner_an(self.ordner)
        self.umg.lege_ordner_an(self.antworten)
        if self.umg.existiert(self.anfrage_pfad):
            self.umg.loesche(self.anfrage_pfad)
        for name, _groesse in self.umg.liste(self.antworten):
            if ANTWORT_NAME.match(name) or TEMP_NAME.match(name):
                self.umg.loesche(self.antworten / name)
        self.bereit = True

    def frage(self, erwartet: dict[str, set[int]], notizen: list[str] | None = None) -> Anfrage:
        self.bereite()
        kennung = self.umg.kennung()
        if not KENNUNG.match(kennung) or kennung in self.vorgelegt:
            raise RuntimeError(f"Anfragekennung ungueltig oder doppelt: {kennung!r}")
        self.vorgelegt.add(kennung)
        daten = json.dumps({"format": ANFRAGE_FORMAT, "anfrage_id": kennung}, separators=(",", ":")).encode("utf-8")
        self.umg.schreibe_atomar(self.anfrage_pfad, daten)
        return Anfrage(kennung, self.umg.jetzt(), erwartet, list(notizen or []))

    def raeume_anfrage_ab(self) -> None:
        if self.umg.existiert(self.anfrage_pfad):
            self.umg.loesche(self.anfrage_pfad)

    @staticmethod
    def _pruefe_umschlag(roh: bytes, kennung: str, rolle: str, pid: int, instanz16: str,
                         laufzeit32: str) -> tuple[dict | None, str | None]:
        if len(roh) > ANTWORT_MAX_BYTES:
            return None, "beim Lesen groesser als 16 MiB"
        if roh[:3] == b"\xef\xbb\xbf":
            return None, "BOM"
        try:
            umschlag = json.loads(roh.decode("utf-8"))
        except (UnicodeDecodeError, ValueError) as e:
            return None, f"kein JSON: {e}"
        if not isinstance(umschlag, dict):
            return None, "kein JSON-Objekt"
        if set(umschlag) != UMSCHLAG_SCHLUESSEL:
            return None, f"Schluesselmenge weicht ab: {sorted(set(umschlag) ^ UMSCHLAG_SCHLUESSEL)}"
        instanz = umschlag.get("instanz_id")
        pruefungen = (
            (umschlag["format"] == ANTWORT_FORMAT, "format"),
            (umschlag["anfrage_id"] == kennung, "anfrage_id"),
            (umschlag["rolle"] == rolle, "rolle"),
            (_ist_zahl(umschlag["pid"]) and umschlag["pid"] == pid, "pid"),
            (umschlag["laufzeit_id"] == laufzeit32, "laufzeit_id"),
            (isinstance(instanz, str)
             and hashlib.sha256(instanz.encode("utf-8")).hexdigest()[:16] == instanz16, "instanz_id"),
        )
        for gut, feld in pruefungen:
            if not gut:
                return None, f"Kopf passt nicht zum Namen: {feld}"
        return umschlag, None

    def auswahl(self, kennung: str, erwartet: dict[str, set[int]]) -> Auswahl:
        """Name und Groesse vor jedem Oeffnen (M-53 (c)); nur aktuelle Kennung und
        erwartete PID werden gelesen (M-56, M-57)."""
        aus = Auswahl(kennung)
        for name, groesse in sorted(self.umg.liste(self.antworten)):
            if ".tmp-" in name:
                aus.verworfen.append((name, "Temp-Datei, nie gelesen"))
                continue
            treffer = ANTWORT_NAME.match(name)
            if not treffer:
                continue
            if groesse < ANTWORT_MIN_BYTES or groesse > ANTWORT_MAX_BYTES:
                aus.verworfen.append((name, f"Groesse {groesse} Bytes ausserhalb [2 B, 16 MiB], nie gelesen"))
                continue
            anfrage_id, rolle, pid_text, instanz16, laufzeit32 = treffer.groups()
            pid = int(pid_text)
            if anfrage_id != kennung:
                aus.fremd.append((name, "fremd: andere Kennung"))
                continue
            if rolle not in erwartet:
                aus.fremd.append((name, f"fremd: Rolle {rolle} ohne Erwartung"))
                continue
            if pid not in erwartet[rolle]:
                aus.fremd.append((name, f"fremd: PID {pid} nicht erwartet {sorted(erwartet[rolle])}"))
                continue
            eintrag = self.cache.get(name)
            if eintrag is None or eintrag[0] != groesse:
                roh = self.umg.lies(self.antworten / name, ANTWORT_MAX_BYTES)
                eintrag = (groesse, *self._pruefe_umschlag(roh, kennung, rolle, pid, instanz16, laufzeit32))
                self.cache[name] = eintrag
            _, umschlag, grund = eintrag
            if umschlag is None:
                aus.verworfen.append((name, f"unbrauchbar: {grund}"))
                continue
            aus.kandidaten.setdefault(rolle, []).append(Antwort(name, rolle, pid, groesse, umschlag, kennung))
        for rolle in erwartet:
            liste = aus.kandidaten.get(rolle, [])
            if len(liste) == 1:
                aus.gewertet[rolle] = liste[0]
            elif len(liste) > 1:
                aus.mehrdeutig[rolle] = liste
            else:
                aus.fehlt.append(rolle)
        return aus

    def sammle(self, anfrage: Anfrage, frist: float = ANTWORT_FRIST_S, takt: float = 0.02) -> Auswahl:
        while True:
            aus = self.auswahl(anfrage.kennung, anfrage.erwartet)
            jetzt = self.umg.jetzt()
            for rolle in list(aus.gewertet) + list(aus.mehrdeutig):
                anfrage.eingang.setdefault(rolle, jetzt)
            if all(r in aus.gewertet or r in aus.mehrdeutig for r in anfrage.erwartet):
                break
            if jetzt - anfrage.geschrieben >= frist:
                break
            self.umg.schlafe(takt)
        self._verknuepfe(anfrage, aus)
        return aus

    def nachlauf(self, anfragen: list[Anfrage]) -> None:
        """Nach der letzten Anfrage zwei Takte warten und jede Auswahl neu bilden:
        eine spaet eintreffende zweite Antwort derselben Rolle macht sie mehrdeutig."""
        if not anfragen:
            return
        rest = NACHLAUF_S - (self.umg.jetzt() - max(a.geschrieben for a in anfragen))
        if rest > 0:
            self.umg.schlafe(rest)
        for anfrage in anfragen:
            self._verknuepfe(anfrage, self.auswahl(anfrage.kennung, anfrage.erwartet))

    @staticmethod
    def _verknuepfe(anfrage: Anfrage, aus: Auswahl) -> None:
        anfrage.auswahl = aus
        for antwort in aus.gewertet.values():
            antwort.anfrage = anfrage


def erwartete_rollen(lauf: "Lauf", rollen: list[str]) -> tuple[dict[str, set[int]], list[str]]:
    """Gen und Probeeq von der Diagnose-PID; der Broker nur, wenn ein Prozess
    eqcop-broker laeuft (M-57) - der Runner startet ihn nie."""
    erwartet: dict[str, set[int]] = {}
    notizen: list[str] = []
    for rolle in rollen:
        if rolle in ("gen", "probeeq"):
            if lauf.diagnose_pid is not None:
                erwartet[rolle] = {int(lauf.diagnose_pid)}
            else:
                pids = set(lauf.umg.fl_pids())
                erwartet[rolle] = pids
                notizen.append(f"{rolle}: Diagnose-PID nicht uebergeben, erwartet werden alle FL64-PIDs {sorted(pids)}")
        elif rolle == "broker":
            pids = lauf.umg.broker_pids()
            if pids:
                erwartet[rolle] = set(pids)
                notizen.append(f"broker: Prozess eqcop-broker laeuft, PID {sorted(pids)}")
            else:
                notizen.append("broker: Broker laeuft nicht - keine Erwartung an die Rolle broker")
    return erwartet, notizen


def _sende(conn, aktion: str, params: dict | None, frist: float | None, umg: Umgebung,
           verworfen: list | None = None) -> dict:
    """Ein Timeout wird genau einmal wiederholt, mit derselben Frist (M-78).

    NAK-309 (R-309-2): jeder Versuch traegt eine neue Anforderungskennung, auch die Wiederholung. Nur eine Antwort
    mit genau dieser Kennung ist eine Messung; jede andere - fremde Kennung, keine Kennung (Controller alten
    Stands), die verspaetete Antwort des ersten Versuchs bei der Wiederholung (M-46) - kommt als UNGEMESSEN zurueck
    und landet in `verworfen`."""
    def einmal() -> dict:
        kennung = umg.mcp_kennung()
        try:
            antwort = conn.send_command(aktion, params or {}, frist, request_id=kennung)
        except RuntimeError as e:
            antwort = {"success": False, "error": str(e), "request_id": kennung}
        return _zuordnen(aktion, kennung, antwort, verworfen)

    antwort = einmal()
    if not antwort.get("_ungemessen") and not antwort.get("success") and "Timeout" in str(antwort.get("error", "")):
        umg.schlafe(0.5)
        antwort = einmal()
        antwort["_wiederholt"] = True
    return antwort


def _zuordnen(aktion: str, kennung: str, antwort, verworfen: list | None) -> dict:
    """R-309-2: gehoert die Antwort zu diesem Versuch? Sonst ist sie keine Messung - verworfen und gezaehlt."""
    erhalten = antwort.get("request_id") if isinstance(antwort, dict) else None
    if erhalten == kennung:
        return antwort
    if verworfen is not None:
        verworfen.append({"aktion": aktion, "gesendet": kennung, "erhalten": erhalten})
    return {"success": False, "_ungemessen": True, "_verworfen": antwort,
            "error": (f"UNGEMESSEN (Antwort ohne passende Kennung: gesendet {kennung}, "
                      f"erhalten {erhalten if erhalten is not None else 'keine'})")}


def _antwort_zelle(antwort) -> str:
    """Rohzelle einer MCP-Antwort. Die Anforderungskennung steht vorn, damit die Kuerzung sie nie abschneidet
    (NAK-309 M-41)."""
    if not isinstance(antwort, dict):
        return _kompakt(antwort)
    kennung = antwort.get("request_id")
    rest = {k: v for k, v in antwort.items() if k != "request_id"}
    return (f"request_id={kennung} " if kennung is not None else "") + _kompakt(rest)


class Lauf:
    def __init__(self, umg: Umgebung, args, roh: list[str]):
        self.umg = umg
        self.roh = roh
        self.diagnose_pid = getattr(args, "diagnose_pid", None)
        laufzeit = LOCALAPPDATA / "evenacadia" / "nakama-laufzeit"
        self.render_ordner = Path(getattr(args, "render_ordner", None) or laufzeit / "render")
        self.diagnose_ordner = Path(getattr(args, "diagnose_ordner", None) or LOCALAPPDATA / "evenacadia" / "nakama" / "diagnose")
        self.projekt_ordner = Path(getattr(args, "projekt_ordner", None) or laufzeit / "projekt")
        self.bilder_ordner = laufzeit / "bilder"
        self.head = getattr(args, "head", None) or "ohne-head"
        self.conn = None
        self.letzte: dict[str, dict] = {}
        self.szenario: dict = {}
        self.details: list[str] = []
        self.briefkasten = Briefkasten(umg, self.diagnose_ordner)
        # NAK-309 (R-309-2): verworfene MCP-Antworten ohne passende Kennung, ueber alle Szenarien dieses Laufs
        self.verworfen: list[dict] = []

    def quelle(self) -> Path:
        kandidaten = [REPO / "eq-copilot" / "kalibration" / "Testtrack.wav",
                      self.projekt_ordner / "Samples" / "Testtrack.wav"]
        return next((k for k in kandidaten if self.umg.existiert(k)), kandidaten[0])

    def sende(self, aktion: str, params: dict | None = None, frist: float | None = None) -> dict:
        return _sende(self.conn, aktion, params, frist, self.umg, self.verworfen)

    def loop_ms(self) -> int | None:
        wert = (self.letzte.get("transport.getLength") or {}).get("milliseconds")
        return int(wert) if _ist_zahl(wert) and wert > 0 else None


class Lesung:
    def __init__(self, ms, t: float, ende: str | None):
        self.ms = ms
        self.t = t
        self.ende = ende


class Positionen:
    """Positionsklammern ueber transport.getPosition (M-18) und die Enden einer
    Anfragefolge: Wrap, stehender Transport, Position ueber L (F-22, F-28)."""

    def __init__(self, lauf: Lauf, loop_ms: int):
        self.lauf = lauf
        self.loop_ms = loop_ms
        self.lesungen: list[tuple[float, float]] = []

    def lies(self) -> Lesung:
        antwort = self.lauf.sende("transport.getPosition")
        t = self.lauf.umg.jetzt()
        ms = antwort.get("ms") if antwort.get("success") else None
        if not _ist_zahl(ms):
            return Lesung(None, t, f"Position nicht lesbar: {antwort.get('error', antwort)}")
        ende = None
        if self.lesungen:
            if ms < self.lesungen[-1][1]:
                ende = "wrap"
            else:
                for t_alt, ms_alt in reversed(self.lesungen):
                    if t - t_alt >= POSITION_ABSTAND_S:
                        if ms == ms_alt:
                            ende = "Transport steht"
                        break
        if ende is None and ms > self.loop_ms:
            ende = "Position ueber L"
        self.lesungen.append((t, ms))
        return Lesung(ms, t, ende)


# ---------------------------------------------------------------- Rohzeilen einer Anfrage

def auswahl_zeilen(anfrage: Anfrage) -> list[str]:
    aus = anfrage.auswahl
    zeilen = [f"- Anfrage `{anfrage.kennung}`, erwartet {_json({r: sorted(p) for r, p in anfrage.erwartet.items()})}, "
              f"Positionsklammer {anfrage.p_vor} → {anfrage.p_nach} ms"]
    zeilen += [f"  - {notiz}" for notiz in anfrage.notizen]
    if aus is None:
        return zeilen
    for rolle, antwort in sorted(aus.gewertet.items()):
        gruende = [g.get("code") for g in (antwort.umschlag.get("gruende") or []) if isinstance(g, dict)]
        eingang = anfrage.eingang.get(rolle)
        nach = f", {eingang - anfrage.geschrieben:.2f} s nach dem Schreiben" if eingang is not None else ""
        zeilen.append(f"  - {rolle}: `{antwort.name}` ({antwort.groesse} B, Version {antwort.wert('version')!r}, "
                      f"gruende {gruende}{nach})")
    for rolle, liste in sorted(aus.mehrdeutig.items()):
        zeilen.append(f"  - {rolle}: MEHRDEUTIG " + _json([
            {"name": a.name, "instanz_id": a.wert("instanz_id"), "laufzeit_id": a.wert("laufzeit_id")} for a in liste]))
    for rolle in aus.fehlt:
        zeilen.append(f"  - {rolle}: fehlt")
    for name, grund in aus.fremd:
        zeilen.append(f"  - roh {grund}: `{name}`")
    for name, grund in aus.verworfen:
        zeilen.append(f"  - verworfen ({grund}): `{name}`")
    return zeilen


def auswahl_maengel(anfrage: Anfrage, frist: float = ANTWORT_FRIST_S) -> list[str]:
    aus = anfrage.auswahl
    maengel = []
    if aus is None:
        return ["keine Auswahl"]
    if aus.fehlt:
        maengel.append(f"Rolle ohne Antwort nach {frist:g} s: {', '.join(sorted(aus.fehlt))}")
    if aus.mehrdeutig:
        maengel.append(f"mehrdeutig: {', '.join(sorted(aus.mehrdeutig))}")
    return maengel


# ---------------------------------------------------------------- Nulltesturteil und Kettenverschiebung

def lies_nulltest(lauf: Lauf) -> dict | None:
    text = lauf.umg.lies_text(lauf.render_ordner / "ergebnis.json")
    if text is None:
        return None
    try:
        ergebnis = json.loads(text)
    except ValueError:
        return None
    return ergebnis if isinstance(ergebnis, dict) else None


def kettenverschiebung(ergebnis: dict | None) -> dict:
    """Delta_K aus dem Nulltesturteil (F-27, M-66): BITIDENTISCH und VERSATZ 0,0 dB,
    KETTE 20*log10|g|; ohne Urteil fehlt die Voraussetzung."""
    if not ergebnis or ergebnis.get("format") != NULLTEST_FORMAT:
        return {"fehlt": "ohne Nulltesturteil: ergebnis.json fehlt oder ist fremd"}
    urteil = ergebnis.get("urteil")
    v = ergebnis.get("v")
    if urteil in ("BITIDENTISCH", "VERSATZ") and isinstance(v, int) and not isinstance(v, bool):
        return {"urteil": urteil, "v": v, "g": 1.0, "delta_k_db": 0.0}
    if urteil == "KETTE" and isinstance(v, int) and not isinstance(v, bool):
        g = ergebnis.get("g")
        if _ist_zahl(g) and math.isfinite(g) and g != 0:
            return {"urteil": urteil, "v": v, "g": g, "delta_k_db": 20.0 * math.log10(abs(g))}
        return {"fehlt": f"KETTE ohne endlichen Faktor (g = {g!r})"}
    return {"fehlt": f"ohne Nulltesturteil: {urteil} ({ergebnis.get('grund')})"}


# ---------------------------------------------------------------- lokal.briefkasten, lokal.nulltest, lokal.fenster

def lokal_briefkasten(lauf: Lauf, schritt: dict) -> tuple[int, str, list[str]]:
    p = schritt.get("params") or {}
    rollen = list(p.get("rollen") or ["gen", "probeeq"])
    frist = float(p.get("frist_s", ANTWORT_FRIST_S))
    erwartet, notizen = erwartete_rollen(lauf, rollen)
    bk = lauf.briefkasten
    anfrage = bk.frage(erwartet, notizen)
    bk.sammle(anfrage, frist)
    bk.nachlauf([anfrage])
    bk.raeume_anfrage_ab()
    maengel = auswahl_maengel(anfrage, frist)
    zeilen = auswahl_zeilen(anfrage)
    erwartung = schritt.get("erwarte") or {}
    for rolle, antwort in sorted(anfrage.auswahl.gewertet.items()):
        m, roh = _pruefe(erwartung, dict(antwort.umschlag, success=True))
        maengel += [f"{rolle}: {x}" for x in m]
        zeilen += [f"  - {rolle} roh {r}" for r in roh]
        zeilen.append(f"  - {rolle}: snapshot.zustand {_json(antwort.wert('snapshot.zustand'))}, "
                      f"frame.frames_gebaut {_json(antwort.wert('frame.frames_gebaut'))}")
    kurz = "; ".join(maengel) if maengel else f"beantwortet: {', '.join(sorted(anfrage.auswahl.gewertet))}"
    return (EXIT_VERFEHLT if maengel else EXIT_OK), kurz, zeilen


REFERENZ_ORDNER = "referenz"          # P-21 (a): laufzeit.ps1 rendert je Referenzprojekt nach render\referenz\<Projekt>\
REFERENZ_VERGLEICHE = ("verarbeitung_ein", "ohne_slots")


def ist_projektname(name: str) -> bool:
    """P-18: ein Dateiname ohne Pfadanteil mit Endung .flp (dieselbe Regel wie Ist-Projektname in laufzeit.ps1)."""
    return bool(name) and Path(name).name == name and name.lower().endswith(".flp")


def lokal_referenz(lauf: Lauf, vergleich: str, projekt: str) -> tuple[int, str, list[str]]:
    """M-64, P-21 (b) bis (f): ein Referenzschritt liest den Renderstatus des Projekts aus params.projekt, den der Runner
    vor dem FL-Start unter render\\referenz\\<Projekt>\\ schreibt - nicht die Existenz einer Datei im Repo-Ordner. Ohne
    Render endet der Schritt mit Szenario-Exit 5 und dem Grund aus dem Renderstatus, nie still (P-3); mit Render vergleicht
    nulltest.py (verarbeitung_ein gegen die Quelle, ohne_slots gegen den Auslieferungsrender). Das Ergebnis steht im Ordner
    des Projekts, nie in der ergebnis.json des Auslieferungsrenders: ein Referenzschritt loest keinen Rueckweg aus (M-65)."""
    if not ist_projektname(projekt):
        return (EXIT_SZENARIO, f"VORAUSSETZUNG {vergleich}: kein Projektdateiname {projekt!r} (P-18)",
                [f"- `params.projekt` {_json(projekt)} ist kein Dateiname mit Endung .flp: kein Renderstatus, kein Render (M-64)"])
    ordner = lauf.render_ordner / REFERENZ_ORDNER / projekt
    status_pfad = ordner / "render.json"
    text = lauf.umg.lies_text(status_pfad)
    try:
        status = json.loads(text) if text is not None else None
    except ValueError:
        status = None
    if not isinstance(status, dict) or status.get("format") != RENDER_FORMAT:
        grund = "kein Renderstatus" if text is None else "Renderstatus unlesbar oder fremd"
        return (EXIT_SZENARIO, f"VORAUSSETZUNG {vergleich}: {grund} fuer {projekt}",
                [f"- Renderstatus `{status_pfad}`: {grund} - ohne Render dieses Zustands kein Vergleich (M-64), nie still"])
    if Path(str(status.get("projekt") or "")).name != projekt:
        return (EXIT_SZENARIO, f"VORAUSSETZUNG {vergleich}: Renderstatus nennt {status.get('projekt')!r} statt {projekt}",
                [f"- Renderstatus `{status_pfad}`: projekt `{status.get('projekt')}` statt `{projekt}` - kein Vergleich (M-64)"])
    if not status.get("datei") or status.get("grund"):
        grund = status.get("grund") or "Renderstatus nennt keine Renderdatei"
        return (EXIT_SZENARIO, f"VORAUSSETZUNG {vergleich}: {grund}",
                [f"- Renderstatus `{status_pfad}`: projekt `{status.get('projekt')}`, grund {_json(grund)} - kein Render "
                 f"dieses Zustands (M-64), nie still"])
    ms = lauf.loop_ms()
    if ms is None:
        return (EXIT_SZENARIO, f"VORAUSSETZUNG {vergleich}: Songlaenge fehlt (transport.getLength vor lokal.nulltest)",
                [f"- Renderstatus `{status_pfad}` nennt den Render `{status.get('datei')}`; ohne Songlaenge kein N (F-19)"])
    ergebnis_pfad = ordner / "ergebnis.json"
    argumente = ["--vergleich", vergleich, "--renderstatus", str(status_pfad), "--songlaenge-ms", str(ms),
                 "--ergebnis", str(ergebnis_pfad)]
    if vergleich == "verarbeitung_ein":
        argumente += ["--quelle", str(lauf.quelle())]
    else:
        argumente += ["--auslieferung", str(lauf.render_ordner / "ergebnis.json")]
    code, text = lauf.umg.nulltest(argumente)
    zeilen = [f"- Referenzprojekt `{projekt}`: Renderstatus `{status_pfad}`, SHA-256 Projekt {status.get('sha256_projekt')}, "
              f"Renderdauer {status.get('dauer_s')} s, Render `{status.get('datei')}`",
              f"- `nulltest.py --vergleich {vergleich}` Exit {code}, Songlaenge {ms} ms"]
    zeilen += [f"  - {z}" for z in text.splitlines() if z.strip()][-25:]
    ergebnis = None
    ergebnis_text = lauf.umg.lies_text(ergebnis_pfad)
    if ergebnis_text is not None:
        try:
            ergebnis = json.loads(ergebnis_text)
        except ValueError:
            ergebnis = None
    if isinstance(ergebnis, dict):
        zeilen.append(f"- ergebnis.json ({vergleich}): `{_zelle(_kompakt(ergebnis, 4000))}`")
    if (code not in (EXIT_OK, EXIT_VERFEHLT, EXIT_SZENARIO) or not isinstance(ergebnis, dict)
            or ergebnis.get("format") != REFERENZ_FORMAT or ergebnis.get("vergleich") != vergleich
            or ergebnis.get("exit") != code):
        return EXIT_VERFEHLT, f"VERFEHLT {vergleich}: nulltest.py ohne Urteil (Exit {code})", zeilen
    wort = {EXIT_OK: "GEMESSEN", EXIT_VERFEHLT: "VERFEHLT", EXIT_SZENARIO: "VORAUSSETZUNG"}[code]
    kurz = (f"{wort} {vergleich}: {ergebnis.get('befund')} v={ergebnis.get('v')} g_db={ergebnis.get('g_db')} "
            f"Abweichungen={ergebnis.get('abweichungen')} {ergebnis.get('grund') or ''}").strip()
    return code, kurz, zeilen


def lokal_nulltest(lauf: Lauf, schritt: dict) -> tuple[int, str, list[str]]:
    p = schritt.get("params") or {}
    vergleich = p.get("vergleich", "auslieferung")
    if vergleich in REFERENZ_VERGLEICHE:
        # M-64, P-21: Render "Verarbeitung ein" und "ohne Slots" aus den Projekten der Karte U43 (K-286-1).
        return lokal_referenz(lauf, vergleich, str(p.get("projekt") or ""))
    if vergleich != "auslieferung":
        return EXIT_VERFEHLT, f"unbekannter Vergleich {vergleich!r} (auslieferung, verarbeitung_ein, ohne_slots)", []
    ms = lauf.loop_ms()
    if ms is None:
        return EXIT_SZENARIO, "VORAUSSETZUNG: Songlaenge fehlt (transport.getLength vor lokal.nulltest)", []
    quelle = lauf.quelle()
    code, text = lauf.umg.nulltest(["--renderstatus", str(lauf.render_ordner / "render.json"),
                                    "--quelle", str(quelle), "--songlaenge-ms", str(ms),
                                    "--ergebnis", str(lauf.render_ordner / "ergebnis.json")])
    ergebnis = lies_nulltest(lauf)
    zeilen = [f"- `nulltest.py` Exit {code}, Quelle `{quelle}`, Songlaenge {ms} ms"]
    zeilen += [f"  - {z}" for z in text.splitlines() if z.strip()][-25:]
    if ergebnis:
        zeilen.append(f"- ergebnis.json: `{_zelle(_kompakt(ergebnis, 4000))}`")
    if code not in (EXIT_OK, EXIT_VERFEHLT, EXIT_SZENARIO) or ergebnis is None:
        return EXIT_VERFEHLT, f"nulltest.py ohne Urteil (Exit {code})", zeilen
    kurz = (f"{ergebnis.get('urteil')} v={ergebnis.get('v')} g={ergebnis.get('g')} "
            f"Abweichungen={ergebnis.get('abweichungen')} {ergebnis.get('grund') or ''}").strip()
    return code, kurz, zeilen


def lokal_fenster(lauf: Lauf, schritt: dict) -> tuple[int, str, list[str]]:
    p = schritt.get("params") or {}
    ziel = p.get("ziel", "fl")
    plugin = p.get("plugin")
    if lauf.diagnose_pid is None:
        return EXIT_VERFEHLT, "Diagnose-PID unbekannt - kein Fenster zuzuordnen", []
    name = f"{lauf.head}-{ziel}-{datetime.now().strftime('%Y%m%d-%H%M%S')}"
    t0 = lauf.umg.jetzt()
    try:
        # Die Frist erzwingt der Unterprozess der Erfassung (P-19): kehrt er nicht rechtzeitig zurueck, wird er
        # beendet, und die Antwort traegt "Frist 5 s ueberschritten". dauer_s steht nur roh.
        r = lauf.umg.fenster(int(lauf.diagnose_pid), ziel, lauf.bilder_ordner, name, plugin)
    except Exception as e:  # noqa: BLE001 - ein Erfassungsfehler ist ein verfehlter Schritt
        r = {"success": False, "error": f"{type(e).__name__}: {e}"}
    dauer = lauf.umg.jetzt() - t0
    antwort = {"pfad": r.get("path"), "breite": r.get("width"), "hoehe": r.get("height"),
               "sha256": r.get("sha256"), "einfarbig": r.get("uniform"), "fenster": r.get("window_title"),
               "klasse": r.get("window_class"), "dauer_s": round(dauer, 3)}
    maengel = []
    if r.get("error"):
        maengel.append(str(r["error"]))
    if r.get("uniform") is True and not any("uniform" in m for m in maengel):
        maengel.append("einfarbig: das Bild traegt keinen Fensterinhalt")
    if not r.get("success") and not maengel:
        maengel.append("Erfassung ohne Erfolg")
    # P-17 (§38.2): ein Plugin-Fensterbild unter 200 x 100 Pixel (Breite unter 200 oder Hoehe unter 100) ist ein
    # eingeklappter FL-Wrapper - Szenario-Voraussetzung mit Breite x Hoehe in der Rohzeile, nie gruen (Handgriff K-286-2,
    # Karte U43). Die Groesse ist das Fensterrechteck der Erfassung; das FL-Hauptfenster bleibt unberuehrt.
    breite, hoehe = r.get("width"), r.get("height")
    eingeklappt = (ziel == "plugin" and _ist_zahl(breite) and _ist_zahl(hoehe)
                   and (breite < PLUGIN_FENSTER_MIN[0] or hoehe < PLUGIN_FENSTER_MIN[1]))
    kurz = _json(antwort)
    if eingeklappt:
        kurz += (f" · VORAUSSETZUNG: eingeklappt: {breite} x {hoehe} Pixel (Plugin-Fenster unter "
                 f"{PLUGIN_FENSTER_MIN[0]} x {PLUGIN_FENSTER_MIN[1]}; Handgriff K-286-2, Karte U43)")
    if maengel:
        kurz += " · VERFEHLT: " + "; ".join(maengel)
    code = schlechter(EXIT_VERFEHLT if maengel else EXIT_OK, EXIT_SZENARIO if eingeklappt else EXIT_OK)
    return code, kurz, [f"- Antwort {_json(antwort)}"]


# ---------------------------------------------------------------- lokal.umlauf: Messpunkt innerhalb eines Umlaufs (F-28)

def umlauf_folge(lauf: Lauf, rollen: list[str], loop_ms: int, deckel: int | None = None):
    """Anfragen im Sekundentakt mit Positionsklammer, bis eine rueckwaerts laufende
    Position den Wrap zeigt oder die Folge ohne Wrap endet (M-67)."""
    bk = lauf.briefkasten
    pos = Positionen(lauf, loop_ms)
    anfragen: list[Anfrage] = []
    grenze = deckel or int(loop_ms / 1000) * 3 + 30
    ende = None
    for _ in range(grenze):
        vor = pos.lies()
        if vor.ende:
            ende = vor.ende
            break
        erwartet, notizen = erwartete_rollen(lauf, rollen)
        anfrage = bk.frage(erwartet, notizen)
        anfrage.p_vor = vor.ms
        bk.sammle(anfrage)
        nach = pos.lies()
        anfrage.p_nach = nach.ms
        anfragen.append(anfrage)
        if nach.ende:
            ende = nach.ende
            break
        rest = 1.0 - (lauf.umg.jetzt() - anfrage.geschrieben)
        if rest > 0:
            lauf.umg.schlafe(rest)
    else:
        ende = f"Deckel von {grenze} Anfragen ohne Wrap"
    bk.nachlauf(anfragen)
    bk.raeume_anfrage_ab()
    return anfragen, ende, pos


def waehle_gewertete(anfragen: list[Anfrage], rolle: str) -> tuple[Antwort | None, list[str]]:
    """Die letzte Antwort vor dem Wrap, deren Materialende nicht hinter das der
    vorigen Antwort derselben Rolle zurueckfaellt (F-28)."""
    gewertet = None
    vorige_e = None
    verlauf = []
    for anfrage in anfragen:
        antwort = anfrage.auswahl.gewertet.get(rolle) if anfrage.auswahl else None
        if antwort is None:
            continue
        e = antwort.wert("frame.material_ende_projektsample")
        if not isinstance(e, int) or isinstance(e, bool):
            verlauf.append(f"`{anfrage.kennung[:8]}` E null: Anker nicht lesbar, nicht waehlbar")
            continue
        if vorige_e is not None and e < vorige_e:
            verlauf.append(f"`{anfrage.kennung[:8]}` E {e} < {vorige_e}: Anlaufdaten des naechsten Umlaufs, nie gewertet")
            break
        gewertet = antwort
        vorige_e = e
    return gewertet, verlauf


def anker_rechnen(antwort: Antwort, v: int, n_loop: int) -> dict:
    """Materialzeit, Fortlaufbedingung (P-10), Kopfverlust K und Referenzausschnitte
    [max(0, K - v), K + n - v) je Anker (P-7, F-28)."""
    s, f = antwort.snapshot, antwort.frame
    rate = (s.get("sensor") or {}).get("samplerate")
    a: dict = {"rate": rate}
    if rate != RATE:
        a["voraussetzung"] = f"Hostrate {rate!r} statt 44 100 Hz"
        return a
    gesamt, schwer = s.get("gesamt_sekunden"), f.get("schwer_sekunden")
    n_l = round(gesamt * RATE) if _ist_zahl(gesamt) else None
    n_s = round(schwer * RATE) if _ist_zahl(schwer) else None
    e = f.get("material_ende_projektsample")
    fortlaufend, stillstand = f.get("hostzeit_fortlaufend_samples"), f.get("hostzeit_stillstand_bloecke")
    a.update(n_l=n_l, n_s=n_s, e=e, fortlaufend=fortlaufend, stillstand=stillstand,
             bloecke_max_samples=f.get("bloecke_max_samples"))
    if n_l is None:
        a["roh"] = "gesamt_sekunden null: Materialzeit nicht lesbar"
        return a
    if not isinstance(e, int) or isinstance(e, bool):
        a["roh"] = "E null: Anker nicht lesbar"
        return a
    if not (isinstance(fortlaufend, int) and isinstance(stillstand, int) and fortlaufend == n_l and stillstand == 0):
        a["roh"] = (f"Fortlaufbedingung nicht erfuellt oder nicht lesbar (P-10): hostzeit_fortlaufend_samples "
                    f"{fortlaufend!r}, n_L {n_l}, hostzeit_stillstand_bloecke {stillstand!r}")
        return a
    k = e - n_l
    a["k"] = k
    if k < 0:
        a["voraussetzung"] = f"K = {k} < 0 bei erfuellter Fortlaufbedingung"
        return a
    for art, n in (("leicht", n_l), ("schwer", n_s)):
        if n is None or n <= 0:
            continue
        ende = k + n - v
        grenze_unten, grenze_oben = max(0, -v), min(n_loop - v, QUELLE_FRAMES)
        if not (grenze_unten < ende <= grenze_oben):
            a["voraussetzung"] = (f"Anker {art} ausserhalb des Loop-Ausschnitts der Quelle: Ende {ende} "
                                  f"nicht in ({grenze_unten}, {grenze_oben}]")
            return a
        a[art] = [max(0, k - v), ende]
    if k < v:
        a["roh"] = f"K = {k} < v = {v}: das Material der Engine beginnt vor Quellframe 0 (P-7)"
        return a
    a["s0"] = k - v
    return a


def rechnungsauftrag(lauf: Lauf, kette: dict, n_loop: int, anker: dict[str, dict]) -> dict:
    auftrag = {"quelle": str(lauf.quelle()), "quelle_sha256": str(lauf.szenario.get("quelle_sha256") or "").upper(),
               "v": kette["v"], "urteil": kette["urteil"], "delta_k_db": kette["delta_k_db"], "n_loop": n_loop,
               "ordner": str(lauf.render_ordner), "head": lauf.head, "anker": [], "u_unten": []}
    for rolle, a in sorted(anker.items()):
        if "roh" in a or "voraussetzung" in a:
            continue
        for art in ("leicht", "schwer"):
            if a.get(art):
                auftrag["anker"].append({"rolle": rolle, "art": art, "ausschnitt": a[art], "s_lufs": art == "schwer"})
        auftrag["u_unten"].append({"rolle": rolle, "s0": a["s0"], "n_l": a["n_l"]})
    return auftrag


def vorbedingung(antwort: Antwort, u_unten: dict | None, n_loop: int) -> tuple[list[str], list[str]]:
    s = antwort.snapshot
    roh: list[str] = []
    version = s.get("metrics_version")
    if version != METRICS_VERSION:
        return [f"Vergleichsbasis verschieden: metrics_version {version!r} statt {METRICS_VERSION}"], roh
    maengel = []
    if s.get("zustand") != "messbereit":
        maengel.append(f"Vorbedingung: zustand {s.get('zustand')!r} statt messbereit")
    aktiv = s.get("aktiv_sekunden")
    aktiv_zellen = round(aktiv * 10) if _ist_zahl(aktiv) else None
    u_oben_zellen = n_loop // ZELLE
    if aktiv_zellen is None:
        maengel.append("Vorbedingung: aktiv_sekunden null")
    elif aktiv_zellen > u_oben_zellen:
        maengel.append(f"Vorbedingung: aktiv_sekunden {aktiv} > U_oben {u_oben_zellen / 10:.1f} s (ueber den Wrap integriert)")
    if u_unten is None:
        roh.append("U_unten roh: kein hergeleiteter Anker (Fortlaufbedingung, E oder K < v)")
    else:
        zellen = u_unten.get("min_zellen")
        roh.append(f"U_unten {zellen / 10:.1f} s = 0,1 s x min Z_phi (phi {u_unten.get('phi_min')}; "
                   f"phi 0: {u_unten.get('zellen_phi0')} Zellen; Schwelle {u_unten.get('schwelle_db')} dB, "
                   f"Abstand {u_unten.get('delta_db')} dB), aktiv_sekunden {aktiv}")
        if aktiv_zellen is not None and zellen > aktiv_zellen:
            maengel.append(f"Vorbedingung: U_unten {zellen / 10:.1f} s > aktiv_sekunden {aktiv}")
    return maengel, roh


def band_pruefen(band: dict, rolle: str, antwort: Antwort, anker: dict, je_ausschnitt: dict, kette: dict) -> dict:
    ergebnis = {"kurz": band.get("kurz"), "rolle": rolle, "feld": band.get("feld")}
    if band.get("art") == "teilmenge":
        wert = [w for _, w in werte_an_pfad(antwort.umschlag, band["feld"]) if w is not _FEHLT]
    else:
        wert = antwort.wert(band["feld"])
    ergebnis["wert"] = wert
    if not str(band.get("rechnung") or "").startswith("F-28"):
        return {**ergebnis, "status": "roh", "grund": "ohne Rechnung in F-28 (P-5)"}
    if rolle not in (band.get("rollen") or []):
        return {**ergebnis, "status": "roh", "grund": "kein Band fuer diese Rolle"}
    ausschnitt = anker.get(band["anker"])
    if "roh" in anker or not ausschnitt:
        return {**ergebnis, "status": "roh", "grund": anker.get("roh") or f"kein Anker {band['anker']} (n = 0)"}
    referenz = je_ausschnitt.get(tuple(ausschnitt))
    if not referenz or referenz.get("fehler"):
        return {**ergebnis, "status": "roh", "grund": f"keine Referenz fuer {ausschnitt}"}
    ergebnis["ausschnitt"] = ausschnitt
    if band.get("art") == "teilmenge":
        offline = [float(x) for x in referenz["resonanzen"]]
        gut = bool(wert) and all(_ist_zahl(w) and any(abs(w - o) <= band["toleranz"] + 1e-9 for o in offline)
                                 for w in wert)
        return {**ergebnis, "referenz": offline, "status": "im Band" if gut else "verfehlt",
                "seite": "im Band" if gut else "teilmenge"}
    r = referenz.get(band["referenz"])
    if not _ist_zahl(r):
        return {**ergebnis, "status": "roh", "grund": f"Referenz {band['referenz']} nicht gerechnet ({r!r})"}
    mitte = r + (kette["delta_k_db"] if band.get("delta_k") else 0.0)
    breite = band["toleranz"] + (referenz["s_lufs"]["spanne"] if band.get("plus_s_lufs") else 0.0)
    ergebnis.update(referenz=r, mitte=mitte, breite=breite)
    if not _ist_zahl(wert):
        return {**ergebnis, "status": "roh", "grund": "Wert null"}
    seite = "im Band" if mitte - breite <= wert <= mitte + breite else ("unter" if wert < mitte - breite else "ueber")
    return {**ergebnis, "status": "im Band" if seite == "im Band" else "verfehlt", "seite": seite}


def zuordnen(ergebnisse: list[dict]) -> list[str]:
    """Zuordnung eines Bandfehlschlags nach F-28, Faelle (1) bis (5)."""
    je_feld: dict[str, dict[str, dict]] = {}
    for e in ergebnisse:
        je_feld.setdefault(e["kurz"], {})[e["rolle"]] = e
    gleich_verfehlt = []
    for kurz, rollen in je_feld.items():
        verfehlt = [e for e in rollen.values() if e["status"] == "verfehlt"]
        if len(verfehlt) == 2 and verfehlt[0]["seite"] == verfehlt[1]["seite"]:
            gleich_verfehlt.append(kurz)
    if len(gleich_verfehlt) > 1:
        return [f"Vergleichsbasis: beide Rollen verfehlen {', '.join(gleich_verfehlt)} auf derselben Seite"]
    gruende = []
    for kurz, rollen in je_feld.items():
        bewertet = {r: e for r, e in rollen.items() if e["status"] in ("im Band", "verfehlt")}
        verfehlt = {r: e for r, e in bewertet.items() if e["status"] == "verfehlt"}
        if not verfehlt:
            continue
        if len(bewertet) == 2:
            if kurz in gleich_verfehlt:
                gruende.append(f"Referenz oder Toleranz: {kurz}")
            else:
                gruende.append(f"Messung: {', '.join(sorted(verfehlt))} ({kurz})")
            continue
        rolle, e = next(iter(verfehlt.items()))
        andere = [x for r, x in rollen.items() if r != rolle]
        rohwert = andere[0].get("wert") if andere else None
        gleiche_seite = _ist_zahl(rohwert) and (
            (e["seite"] == "unter" and rohwert < e["mitte"] - e["breite"])
            or (e["seite"] == "ueber" and rohwert > e["mitte"] + e["breite"]))
        gruende.append(f"Referenz oder Toleranz: {kurz}" if gleiche_seite else f"Messung: {rolle} ({kurz})")
    return gruende


def folge_tabelle(anfragen: list[Anfrage], rollen: list[str]) -> list[str]:
    zeilen = ["", "| # | Kennung | p_vor ms | p_nach ms | " + " | ".join(
        f"{r}: zustand · aktiv s · gesamt s · E · schwer s" for r in rollen) + " |",
              "|---|---|---|---|" + "---|" * len(rollen)]
    for nr, anfrage in enumerate(anfragen, 1):
        zellen = []
        for rolle in rollen:
            a = anfrage.auswahl.gewertet.get(rolle) if anfrage.auswahl else None
            if a is None:
                zellen.append("—")
                continue
            zellen.append(" · ".join(_json(a.wert(p)) for p in (
                "snapshot.zustand", "snapshot.aktiv_sekunden", "snapshot.gesamt_sekunden",
                "frame.material_ende_projektsample", "frame.schwer_sekunden")))
        zeilen.append(f"| {nr} | `{anfrage.kennung[:8]}` | {anfrage.p_vor} | {anfrage.p_nach} | "
                      + " | ".join(_zelle(z) for z in zellen) + " |")
    zeilen.append("")
    return zeilen


def lokal_umlauf(lauf: Lauf, schritt: dict) -> tuple[int, str, list[str]]:
    p = schritt.get("params") or {}
    rollen = [r for r in (p.get("rollen") or ["gen", "probeeq"]) if r in ("gen", "probeeq")]
    vergleich = p.get("vergleich") or {}
    kette = kettenverschiebung(lies_nulltest(lauf))
    zeilen = [f"- Nulltesturteil und Kettenverschiebung: {_json(kette)}"]
    if "fehlt" in kette:
        return EXIT_SZENARIO, f"VORAUSSETZUNG: {kette['fehlt']}", zeilen
    loop_ms = lauf.loop_ms()
    if loop_ms is None:
        return EXIT_SZENARIO, "VORAUSSETZUNG: Songlaenge fehlt (transport.getLength vor lokal.umlauf)", zeilen
    n_loop = loop_ms * RATE // 1000
    v = kette["v"]
    anfragen, ende, pos = umlauf_folge(lauf, rollen, loop_ms)
    lauf.sende("transport.stop")
    zeilen += folge_tabelle(anfragen, rollen)
    zeilen.append(f"- Ende der Folge: {ende}; {len(anfragen)} Anfragen, {len(pos.lesungen)} Positionen")
    for anfrage in anfragen:
        zeilen += auswahl_zeilen(anfrage)
    if ende != "wrap":
        return EXIT_SZENARIO, f"VORAUSSETZUNG: Umlauf ohne Wrap ({ende})", zeilen
    maengel = []
    for anfrage in anfragen:
        maengel += [f"Anfrage `{anfrage.kennung[:8]}`: {m}" for m in auswahl_maengel(anfrage)]
    gewertet: dict[str, Antwort] = {}
    anker: dict[str, dict] = {}
    for rolle in rollen:
        antwort, verlauf = waehle_gewertete(anfragen, rolle)
        zeilen += [f"- {rolle}: {x}" for x in verlauf]
        if antwort is None:
            maengel.append(f"{rolle}: keine gewertete Antwort vor dem Wrap")
            continue
        gewertet[rolle] = antwort
        anker[rolle] = anker_rechnen(antwort, v, n_loop)
        zeilen.append(f"- {rolle}: gewertet `{antwort.name}` (p_vor {antwort.anfrage.p_vor} ms), "
                      f"Anker {_json(anker[rolle])}")
    voraussetzungen = [f"{r}: {a['voraussetzung']}" for r, a in sorted(anker.items()) if "voraussetzung" in a]
    if voraussetzungen:
        return EXIT_SZENARIO, "VORAUSSETZUNG: " + "; ".join(voraussetzungen), zeilen
    auftrag = rechnungsauftrag(lauf, kette, n_loop, anker)
    rechnung: dict = {}
    if auftrag["anker"] or auftrag["u_unten"]:
        t0 = lauf.umg.jetzt()
        rechnung = lauf.umg.rechne(auftrag)
        # F-28: die Rechnung steht vollstaendig roh, ohne Kappung (die Zeile traegt jeden Referenzausschnitt und U_unten).
        zeilen.append(f"- Rechnung F-28 ({lauf.umg.jetzt() - t0:.1f} s): `{_zelle(_json(rechnung))}`")
        if not isinstance(rechnung, dict) or rechnung.get("fehler"):
            fehler = rechnung.get("fehler") if isinstance(rechnung, dict) else rechnung
            return EXIT_SZENARIO, f"VORAUSSETZUNG: Rechnung aus F-28 entstand nicht: {fehler}", zeilen
        # F-28, P-7: je Referenzausschnitt (Maschinenartefakt) eine Rohzeile mit Anfangs- und Endframe, K, v und SHA-256.
        gerechnet = {(x.get("rolle"), x.get("art")): x for x in rechnung.get("anker", []) if isinstance(x, dict)}
        for a in auftrag["anker"]:
            x = gerechnet.get((a["rolle"], a["art"])) or {}
            von, bis = a["ausschnitt"]
            zeilen.append(f"- Referenzausschnitt {a['rolle']} {a['art']}: Frames [{von}, {bis}), "
                          f"K {anker[a['rolle']].get('k')}, v {v}, SHA-256 {x.get('sha256')}, Datei {x.get('pfad')}")
    je_ausschnitt = {tuple(x["ausschnitt"]): x for x in rechnung.get("anker", [])}
    je_u_unten = {x["rolle"]: x for x in rechnung.get("u_unten", [])}
    ergebnisse = []
    for rolle, antwort in gewertet.items():
        m, roh = vorbedingung(antwort, je_u_unten.get(rolle), n_loop)
        zeilen += [f"- {rolle}: {x}" for x in roh]
        for feld in vergleich.get("roh") or []:
            zeilen.append(f"- {rolle} roh `{feld}` = {_json([w for _, w in werte_an_pfad(antwort.umschlag, feld) if w is not _FEHLT])}")
        if m:
            maengel += [f"{rolle}: {x}" for x in m]
            continue
        for band in vergleich.get("baender") or []:
            ergebnisse.append(band_pruefen(band, rolle, antwort, anker[rolle], je_ausschnitt, kette))
    zeilen += [f"- Band {e['kurz']} {e['rolle']}: {_json(e)}" for e in ergebnisse]
    if vergleich.get("rohvergleich_runde01"):
        zeilen.append(f"- Rohvergleich Runde 01 (nur daneben, F-28): {_json(vergleich['rohvergleich_runde01'])}")
    maengel += zuordnen(ergebnisse)
    kurz = "; ".join(maengel) if maengel else "gemessen: " + ", ".join(
        f"{r} `{a.kennung[:8]}`" for r, a in sorted(gewertet.items()))
    return (EXIT_VERFEHLT if maengel else EXIT_OK), kurz, zeilen


# ---------------------------------------------------------------- lokal.stellen: Karte U40 als Messung (F-21 bis F-23)

F23_FELDER = ("evidenz_frisch", "aktivitaet", "abdeckung", "konvergenz", "lufs_s", "true_peak_db", "peak_db",
              "evidenz_fenster", "summe_fenster_aktiv", "summe_fenster_gesamt", "offen_fenster_aktiv",
              "offen_fenster_gesamt", "frames_gebaut", "integration_samples", "nicht_endlich_rahmen",
              "projekt_sample_start", "sample_count", "spielt")
EINHEITSFELDER = ("aktivitaet", "abdeckung", "konvergenz")


def naechster_tick(phase: float, t: float) -> float:
    """Erster Takt nach t fuer Takte bei phase + k (Sekundentakt des Briefkastens, F-12)."""
    return phase + math.floor(t - phase) + 1.0


# Zeitmodell einer Stelle, gemessen in den Laufzeitlaeufen vom 15.09.2026
# (docs/beweise/roh/NAK-286-laufzeit-34bdf159-lauf1.md bis -lauf2.md und NAK-286-laufzeit-34bdf159-lauf3.md,
# Szenario u40-aktivitaetsgate): ein Rahmen endet 0,015 bis 0,05 s vor der Schreibzeit seiner Antwort und ist bis
# 25 061 Samples (Gen, 0,57 s) und 28 244 Samples (Probeeq, 0,64 s) lang; Gen schreibt je Takt zur selben
# Millisekunde (+-5 ms). Jeder MCP-Befehl blockiert den Runner bis zur Antwort: transport.start vor einer
# ersten Anfrage nach dem Start, eine Positionsabfrage vor jeder Anfrage nach dem Start, zwei zwischen der
# letzten Antwort und der naechsten Anfrage (Lauf 3, S3: die Anfrage "0,05 s nach dem Start" kam nach dem
# Flag-Takt von Probeeq). Ein Plan zaehlt die Kombinationen dieser Grenzen, unter denen je Rolle zwei
# Rahmen ganz in der Stelle liegen.
RAHMEN_LAENGE_S = {"gen": 0.60, "probeeq": 0.65}
PHASENFEHLER_S = (-0.02, 0.02)


def simuliere_stelle(start: float, t1: float, phasen: dict[str, float], dauer: float, ende_vor: float = 0.03,
                     befehl: float = 0.05, fehler: dict[str, float] | None = None,
                     deckel: int = 12) -> dict[str, list[tuple[float, float]]]:
    """Rahmen [Anfang, Ende) je Rolle in Stellenzeit (Sekunden ab dem Start): Gen schreibt im
    naechsten Takt, Probeeq setzt im naechsten Takt das Flag und schreibt einen Takt spaeter
    (P-9); die naechste Anfrage folgt erst nach beiden Antworten (F-13: eine neue Kennung
    verwirft eine wartende). Eine Anfrage nach dem Start entsteht nach einer Positionsabfrage
    (befehl), die naechste nach Erkennen (0,02 s) und zwei Positionsabfragen, fruehestens 1 s
    nach der vorigen plus eine Positionsabfrage. Takte liegen bei phase + k, phase ist die
    Schreibzeit einer Antwort."""
    fehler = fehler or {}
    rahmen: dict[str, list[tuple[float, float]]] = {r: [] for r in phasen}
    w = t1 + befehl if t1 >= start else t1
    for _ in range(deckel):
        schreiben = {}
        for rolle, phase in phasen.items():
            tick = naechster_tick(phase + fehler.get(rolle, 0.0), w)
            schreiben[rolle] = tick + 1.0 if rolle == "probeeq" else tick
            ende = schreiben[rolle] - ende_vor - start
            rahmen[rolle].append((ende - RAHMEN_LAENGE_S.get(rolle, 0.65), ende))
        if all(rahmen[r][-1][1] >= dauer for r in phasen):
            break
        w = max(max(schreiben.values()) + 0.02 + 2.0 * befehl, w + 1.0 + befehl)
    return rahmen


def stellen_rahmen_in(rahmen: list[tuple[float, float]], dauer: float) -> list[tuple[float, float]]:
    return [(a, e) for a, e in rahmen if a >= 0.0 and e <= dauer]


def plan_bewerten(start: float, t1: float, phasen: dict[str, float], dauer: float,
                  befehl_s: float = 0.0) -> tuple[int, int, float]:
    """(tragende Kombinationen, alle Kombinationen, kleinste Marge): eine Kombination traegt, wenn
    je Rolle zwei Rahmen ganz in der Stelle liegen; die Marge ist der Abstand des besten
    Rahmenpaars zu den Grenzen der Stelle, das Minimum ueber Rollen und tragende Kombinationen.
    Grenzen: Rahmenende 0 bis 0,10 s plus Befehlsdauer vor der Schreibzeit (der Ton beginnt nach
    transport.start), Befehlsdauer gemessen bis doppelt plus 0,03 s, Phasenfehler +-0,02 s je Rolle."""
    tragend, alle, marge = 0, 0, math.inf
    for ende_vor in (0.0, 0.10 + befehl_s):
        for befehl in (befehl_s, 2.0 * befehl_s + 0.03):
            for fehler_gen in PHASENFEHLER_S:
                for fehler_probeeq in PHASENFEHLER_S:
                    alle += 1
                    rahmen = simuliere_stelle(start, t1, phasen, dauer, ende_vor, befehl,
                                              {"gen": fehler_gen, "probeeq": fehler_probeeq})
                    kleinste = math.inf
                    for liste in rahmen.values():
                        drin = stellen_rahmen_in(liste, dauer)
                        paare = [min(drin[i][0], dauer - drin[j][1]) for i in range(len(drin))
                                 for j in range(i + 1, len(drin))]
                        if not paare:
                            kleinste = None
                            break
                        kleinste = min(kleinste, max(paare))
                    if kleinste is not None:
                        tragend += 1
                        marge = min(marge, kleinste)
    return tragend, alle, (marge if tragend else -math.inf)


def plane_stelle(phasen: dict[str, float], dauer: float, jetzt: float,
                 befehl_s: float = 0.0) -> tuple[float, float, float, int, int] | None:
    """Start und erste Anfrage so legen, dass die meisten Kombinationen des Zeitmodells je Rolle
    zwei Rahmen in der Stelle tragen, bei gleicher Zahl mit der groessten Marge; die erste Anfrage
    darf vor dem Start liegen, gezaehlt wird nach dem Materialausschnitt, nicht nach der Anfragezeit.
    Waehrend transport.start auf seine Antwort wartet, schreibt der Runner nichts: eine erste Anfrage
    von 0,02 s vor dem Start bis 2 x Befehlsdauer + 0,05 s danach entfaellt."""
    bester = None
    for i in range(60):
        start = jetzt + 0.6 + i * 0.03
        kandidaten = {round(start + 2.0 * befehl_s + 0.05, 4), round(start - 0.3, 4)}
        for phase in phasen.values():
            tick = naechster_tick(phase, jetzt + 0.15)
            while tick < start + 1.2:
                for abstand in (-0.5, -0.25, 0.15, 0.35):
                    kandidaten.add(round(tick + abstand, 4))
                tick += 1.0
        for t1 in sorted(kandidaten):
            if t1 < jetzt + 0.1 or start - 0.02 <= t1 < start + 2.0 * befehl_s + 0.05:
                continue
            tragend, alle, marge = plan_bewerten(start, t1, phasen, dauer, befehl_s)
            if bester is None or (tragend, marge) > (bester[3], bester[2]):
                bester = (start, t1, marge, tragend, alle)
    return bester


def zaehle_antworten(anfragen: list[Anfrage], rolle: str, a_frames: int, b_frames: int):
    """Gezaehlt wird nach dem Materialausschnitt des Rahmens, nie nach der
    Anfragezeit: ganz in der Stelle, bei laufender Wiedergabe, nicht vor dem
    Ausschnitt der vorigen gezaehlten Antwort; null ist nie 0 (F-22)."""
    gezaehlt: list[Antwort] = []
    urteile: dict[str, str] = {}
    vorige_start = None
    for anfrage in anfragen:
        antwort = anfrage.auswahl.gewertet.get(rolle) if anfrage.auswahl else None
        if antwort is None:
            continue
        start = antwort.wert("frame.projekt_sample_start")
        anzahl = antwort.wert("frame.sample_count")
        spielt = antwort.wert("frame.spielt")
        if not isinstance(start, int) or isinstance(start, bool) or not isinstance(anzahl, int) or isinstance(anzahl, bool):
            urteile[antwort.name] = "Materialausschnitt null: zaehlt nie"
        elif spielt is not True:
            urteile[antwort.name] = f"spielt {spielt!r}: zaehlt nicht"
        elif not (a_frames <= start and start + anzahl <= b_frames):
            urteile[antwort.name] = f"Ausschnitt [{start}, {start + anzahl}) nicht ganz in [{a_frames}, {b_frames})"
        elif vorige_start is not None and start < vorige_start:
            urteile[antwort.name] = "beginnt vor dem Ausschnitt der vorigen gezaehlten Antwort"
        else:
            urteile[antwort.name] = "gezaehlt"
            gezaehlt.append(antwort)
            vorige_start = start
    return gezaehlt, urteile


def am_stellenende(anfragen: list[Anfrage], rolle: str, b_frames: int) -> bool:
    for anfrage in anfragen:
        antwort = anfrage.auswahl.gewertet.get(rolle) if anfrage.auswahl else None
        if antwort is None:
            continue
        start, anzahl = antwort.wert("frame.projekt_sample_start"), antwort.wert("frame.sample_count")
        if isinstance(start, int) and isinstance(anzahl, int) and start + anzahl >= b_frames:
            return True
    return False


def deltas(gezaehlt: list[Antwort]) -> dict:
    """Differenzen nur aus kumulativen Zaehlern, nie aus offen_fenster_* oder abdeckung (F-23)."""
    erste, letzte = gezaehlt[0], gezaehlt[-1]

    def differenz(feld: str):
        a, b = erste.wert(f"frame.{feld}"), letzte.wert(f"frame.{feld}")
        return b - a if isinstance(a, int) and isinstance(b, int) else None

    def ausschnitt(antwort: Antwort):
        start, anzahl = antwort.wert("frame.projekt_sample_start"), antwort.wert("frame.sample_count")
        return [start, start + anzahl]

    return {"antworten": len(gezaehlt), "d_frames_gebaut": differenz("frames_gebaut"),
            "d_summe_fenster_aktiv": differenz("summe_fenster_aktiv"),
            "d_summe_fenster_gesamt": differenz("summe_fenster_gesamt"),
            "erster_ausschnitt": ausschnitt(erste), "letzter_ausschnitt": ausschnitt(letzte)}


def plausibel(gezaehlt: list[Antwort], delta: dict) -> list[str]:
    """Nur Plausibilitaet, kein Sollwert an aktivitaet oder abdeckung (M-72)."""
    maengel = []
    for antwort in gezaehlt:
        for feld in F23_FELDER:
            wert = antwort.wert(f"frame.{feld}")
            if _ist_zahl(wert) and not math.isfinite(wert):
                maengel.append(f"{antwort.name[:8]} {feld} nicht endlich")
        for feld in EINHEITSFELDER:
            wert = antwort.wert(f"frame.{feld}")
            if wert is not None and not (_ist_zahl(wert) and 0.0 <= wert <= 1.0):
                maengel.append(f"{antwort.name[:8]} {feld} {wert!r} nicht in [0, 1]")
        rahmen = antwort.wert("frame.nicht_endlich_rahmen")
        if rahmen is not None and rahmen != 0:
            maengel.append(f"{antwort.name[:8]} nicht_endlich_rahmen {rahmen!r} statt 0")
    aktiv, gesamt = delta["d_summe_fenster_aktiv"], delta["d_summe_fenster_gesamt"]
    if aktiv is None or gesamt is None or not (0 <= aktiv <= gesamt):
        maengel.append(f"0 <= Delta summe_fenster_aktiv ({aktiv}) <= Delta summe_fenster_gesamt ({gesamt}) verletzt")
    return maengel


STELLEN_KOPF = ("Stelle", "Rolle", "durchlauf", "kombinationen", "Kennung", "Zaehlung", *F23_FELDER, "lage im Host",
                "p_vor ms", "p_nach ms")


def lage_im_host(a_frames: int, b_frames: int, v: int, v_bekannt: bool) -> str:
    """Lage der Stelle im Host fuer jede ihrer Rohzeilen (F-23): Hostframes und Versatz v; ohne Nulltesturteil
    steht die Stelle unverschoben, und die Zeile traegt den Vermerk v unbekannt (F-22)."""
    return f"[{a_frames}, {b_frames}) " + (f"v = {v}" if v_bekannt else "v unbekannt")


def stellen_zeile(stelle_id: str, rolle: str, durchlauf: str, kombinationen: str, antwort: Antwort, urteil: str,
                  lage: str) -> str:
    """Rohzeile je Antwort in der Spaltenfolge von STELLEN_KOPF: durchlauf und die getragenen Kombinationen seines
    Zeitplans (P-16), Kennung, Zaehlung, die Felder aus F-23 samt Materialausschnitt, Lage im Host, Positionsklammer."""
    werte = " | ".join(_zelle(_json(antwort.wert(f"frame.{f}"))) for f in F23_FELDER)
    anfrage = antwort.anfrage
    klammer = f"{anfrage.p_vor} | {anfrage.p_nach}" if anfrage else "— | —"
    return (f"| {stelle_id} | {rolle} | {durchlauf} | {kombinationen} | `{antwort.kennung[:8]}` | {_zelle(urteil)} | "
            f"{werte} | {_zelle(lage)} | {klammer} |")


def stellen_durchlauf(lauf: Lauf, rollen: list[str], phasen: dict[str, float], a_frames: int, b_frames: int,
                      loop_ms: int, zeilen: list[str], name: str, gelesen: dict | None = None,
                      befehl_s: float = 0.0) -> tuple[list[Anfrage], int, int]:
    """Ein Durchlauf ueber die Stelle nach F-22: Stopp, Position auf den Anfang in Hostzeit,
    getPosition in einem eigenen Befehl (ausser der Durchlauf beginnt an der schon gesetzten
    Position), Zeitplan, Start, Anfragen bis zu einem Ende, Stopp. Erwartet werden nur die Rollen
    dieses Durchlaufs. Rueckgabe: die Anfragen und die tragenden und alle Kombinationen seines
    Zeitplans (P-16: sie stehen in jeder Rohzeile)."""
    bk, umg = lauf.briefkasten, lauf.umg
    if gelesen is None:
        start_ms = round(a_frames * 1000 / RATE)
        stopp = lauf.sende("transport.stop")
        gesetzt = lauf.sende("transport.setPosition", {"position": start_ms, "mode": 0})
        t_befehl = umg.jetzt()
        gelesen = lauf.sende("transport.getPosition")
        befehl_s = max(befehl_s, umg.jetzt() - t_befehl)
        zeilen.append(f"- {name}: Stopp `{_kompakt(stopp)}`; setPosition {start_ms} ms `{_kompakt(gesetzt)}`; "
                      f"getPosition roh `{_kompakt(gelesen)}` nach {befehl_s:.3f} s")
    dauer = (b_frames - a_frames) / RATE
    jetzt = umg.jetzt()
    plan = plane_stelle({r: phasen[r] for r in rollen}, dauer, jetzt, befehl_s) if all(r in phasen for r in rollen) else None
    start, t1, marge, tragend, alle = plan if plan else (jetzt + 0.2, jetzt + 0.25, None, 0, 0)
    marge_text = "—" if marge is None or not math.isfinite(marge) else f"{marge:.3f} s"
    zeilen.append(f"- {name}: Zeitplan erste Anfrage {t1 - start:+.2f} s zum Start, {tragend} von {alle} Kombinationen "
                  f"des Zeitmodells tragen zwei Rahmen je Rolle, Marge {marge_text}")
    erwartet, notizen = erwartete_rollen(lauf, rollen)

    def warte_bis(zeitpunkt: float) -> None:
        rest = zeitpunkt - umg.jetzt()
        if rest > 0:
            umg.schlafe(rest)

    offen = None
    if t1 < start:
        warte_bis(t1)
        offen = bk.frage(erwartet, notizen)
        offen.p_vor = gelesen.get("ms")
    warte_bis(start)
    t_start = umg.jetzt()
    gestartet = lauf.sende("transport.start")
    start_dauer = umg.jetzt() - t_start
    if offen is None:
        warte_bis(t1)
    zeilen.append(f"- {name}: Start `{_kompakt(gestartet)}` nach {start_dauer:.3f} s (geplant ab "
                  f"{t_start - start:+.3f} s), erste Anfrage geplant {t1 - start:+.2f} s zum Start")
    pos = Positionen(lauf, loop_ms)
    anfragen: list[Anfrage] = []
    deckel = int(dauer) + 8
    ende = None
    while len(anfragen) < deckel:
        if offen is None:
            vor = pos.lies()
            if vor.ende:
                ende = vor.ende
                break
            erwartet, notizen = erwartete_rollen(lauf, rollen)
            offen = bk.frage(erwartet, notizen)
            offen.p_vor = vor.ms
        bk.sammle(offen)
        nach = pos.lies()
        offen.p_nach = nach.ms
        anfragen.append(offen)
        geschrieben = offen.geschrieben
        offen = None
        if nach.ende:
            ende = nach.ende
            break
        if all(am_stellenende(anfragen, r, b_frames) for r in rollen):
            ende = "Stellenende erreicht"
            break
        rest = 1.0 - (umg.jetzt() - geschrieben)
        if rest > 0:
            umg.schlafe(rest)
    else:
        ende = f"Deckel von {deckel} Anfragen"
    zeilen.append(f"- {name}: Ende der Anfragen: {ende}; Stopp `{_kompakt(lauf.sende('transport.stop'))}`")
    return anfragen, tragend, alle


def messe_stelle(lauf: Lauf, rollen: list[str], stelle: dict, a_frames: int, b_frames: int, loop_ms: int,
                 vermerk: str, lage: str) -> tuple[int, list[str], list[str]]:
    """Eine Stelle nach F-22 bis F-23. Traegt der gemeinsame Zeitplan nicht jede Kombination des
    Zeitmodells (Probeeq liefert hoechstens alle zwei Takte einen Rahmen, ein Rahmen ist bis 0,64 s
    lang), faehrt jede Rolle einen eigenen Durchlauf mit eigener Positionierung (Laufzeitlauf
    15.09.2026: S3 trug gemeinsam 8 von 16 Kombinationen, Probeeq bekam einen Rahmen)."""
    bk, umg = lauf.briefkasten, lauf.umg
    sid = str(stelle.get("id"))
    zeilen = ["", f"#### Stelle {sid} {stelle.get('name', '')}: Quelle [{stelle.get('von_s')}, {stelle.get('bis_s')}) s, "
                  f"Host [{a_frames}, {b_frames}) Frames{vermerk}"]
    start_ms = round(a_frames * 1000 / RATE)
    stopp = lauf.sende("transport.stop")
    gesetzt = lauf.sende("transport.setPosition", {"position": start_ms, "mode": 0})
    t_befehl = umg.jetzt()
    gelesen = lauf.sende("transport.getPosition")
    befehl_s = umg.jetzt() - t_befehl
    zeilen.append(f"- Stopp `{_kompakt(stopp)}`; setPosition {start_ms} ms `{_kompakt(gesetzt)}`; "
                  f"getPosition roh `{_kompakt(gelesen)}` nach {befehl_s:.3f} s")
    erwartet, notizen = erwartete_rollen(lauf, rollen)
    probe = bk.frage(erwartet, notizen)
    bk.sammle(probe)
    phasen: dict[str, float] = {}
    phasen_quelle: dict[str, str] = {}
    for r in rollen:
        antwort = probe.auswahl.gewertet.get(r) if probe.auswahl else None
        zeit = umg.schreibzeit(bk.antworten / antwort.name) if antwort is not None else None
        if zeit is not None:
            phasen[r], phasen_quelle[r] = zeit, "Schreibzeit"
        elif r in probe.eingang:
            phasen[r], phasen_quelle[r] = probe.eingang[r], "Eingang"
    dauer = (b_frames - a_frames) / RATE
    gemeinsam = plane_stelle(phasen, dauer, umg.jetzt(), befehl_s) if len(phasen) == len(rollen) else None
    getrennt = len(rollen) > 1 and gemeinsam is not None and gemeinsam[3] < gemeinsam[4]
    plan_text = (f"gemeinsamer Zeitplan: {gemeinsam[3]} von {gemeinsam[4]} Kombinationen des Zeitmodells tragen zwei "
                 f"Rahmen je Rolle" if gemeinsam else "kein gemeinsamer Zeitplan (Phase fehlt)")
    zeilen.append(f"- Phasenmessung (nicht gezaehlt) `{probe.kennung[:8]}`: je Rolle "
                  f"{_json({r: round(t - probe.geschrieben, 3) for r, t in phasen.items()})} s nach dem Schreiben "
                  f"der Anfrage ({_json(phasen_quelle)}), Eingang beim Runner "
                  f"{_json({r: round(t - probe.geschrieben, 3) for r, t in probe.eingang.items()})} s; {plan_text}"
                  + ("; getrennte Durchlaeufe je Rolle" if getrennt else ""))
    gruppen = [[r] for r in rollen] if getrennt else [list(rollen)]
    durchlauf = "getrennt" if getrennt else "gemeinsam"
    anfragen_je_rolle: dict[str, list[Anfrage]] = {}
    kombinationen_je_rolle: dict[str, str] = {}
    alle_anfragen: list[Anfrage] = []
    for nummer, gruppe in enumerate(gruppen):
        anfragen, tragend, alle = stellen_durchlauf(lauf, gruppe, phasen, a_frames, b_frames, loop_ms, zeilen,
                                                    "Durchlauf " + " und ".join(gruppe), gelesen if nummer == 0 else None,
                                                    befehl_s)
        alle_anfragen += anfragen
        for r in gruppe:
            anfragen_je_rolle[r] = anfragen
            kombinationen_je_rolle[r] = f"{tragend}/{alle}"
    bk.nachlauf([probe] + alle_anfragen)
    bk.raeume_anfrage_ab()
    for anfrage in [probe] + alle_anfragen:
        zeilen += auswahl_zeilen(anfrage)
    code, maengel = EXIT_OK, []
    for anfrage in alle_anfragen:
        for m in auswahl_maengel(anfrage):
            maengel.append(f"Stelle {sid} Anfrage `{anfrage.kennung[:8]}`: {m}")
            code = schlechter(code, EXIT_VERFEHLT)
    zeilen += ["", "| " + " | ".join(STELLEN_KOPF) + " |", "|" + "---|" * len(STELLEN_KOPF)]
    ergebnisse = {}
    for rolle in rollen:
        anfragen = anfragen_je_rolle.get(rolle, [])
        gezaehlt, urteile = zaehle_antworten(anfragen, rolle, a_frames, b_frames)
        for anfrage in anfragen:
            antwort = anfrage.auswahl.gewertet.get(rolle) if anfrage.auswahl else None
            if antwort is not None:
                zeilen.append(stellen_zeile(sid, rolle, durchlauf, kombinationen_je_rolle.get(rolle, "0/0"), antwort,
                                            urteile.get(antwort.name, "—"), lage))
        if len(gezaehlt) < 2:
            maengel.append(f"Stelle {sid} {rolle}: nicht gemessen: zu kurz ({len(gezaehlt)} gezaehlte Antwort(en))")
            code = schlechter(code, EXIT_VERFEHLT)
            continue
        delta = deltas(gezaehlt)
        ergebnisse[rolle] = delta
        for m in plausibel(gezaehlt, delta):
            maengel.append(f"Stelle {sid} {rolle}: Plausibilitaet: {m}")
            code = schlechter(code, EXIT_VERFEHLT)
    zeilen.append("")
    for rolle, delta in ergebnisse.items():
        zeilen.append(f"- Stelle {sid} {rolle} (durchlauf {durchlauf}, kombinationen {kombinationen_je_rolle.get(rolle, '0/0')}): "
                      f"{_json(delta)}{vermerk}")
    return code, zeilen, maengel


def lokal_stellen(lauf: Lauf, schritt: dict) -> tuple[int, str, list[str]]:
    p = schritt.get("params") or {}
    rollen = [r for r in (p.get("rollen") or ["gen", "probeeq"]) if r in ("gen", "probeeq")]
    soll = str(lauf.szenario.get("quelle_sha256") or "").upper()
    quelle = lauf.quelle()
    ist = lauf.umg.sha256(quelle)
    zeilen = [f"- Quelle `{quelle}` SHA-256 {ist}, erwartet {soll or '(keiner)'}"]
    if not soll or ist != soll:
        return EXIT_SZENARIO, f"VORAUSSETZUNG: fremde oder fehlende Quelle (SHA-256 {ist})", zeilen
    loop_ms = lauf.loop_ms()
    if loop_ms is None:
        return EXIT_SZENARIO, "VORAUSSETZUNG: Songlaenge fehlt (transport.getLength vor lokal.stellen)", zeilen
    n_loop = loop_ms * RATE // 1000
    kette = kettenverschiebung(lies_nulltest(lauf))
    v_bekannt = "fehlt" not in kette
    v = kette["v"] if v_bekannt else 0
    vermerk = "" if v_bekannt else " · v unbekannt"
    zeilen.append(f"- Versatz v = {v} Frames{vermerk}; Nulltest {_json(kette)}")
    code, maengel = EXIT_OK, []
    for stelle in p.get("stellen") or []:
        a = round(float(stelle["von_s"]) * RATE) + v
        b = round(float(stelle["bis_s"]) * RATE) + v
        a_frames, b_frames = max(a, 0), min(b, n_loop)
        if b_frames <= a_frames:
            zeilen.append(f"- Stelle {stelle.get('id')}: Host [{a}, {b}) ohne Schnitt mit dem Loop [0, {n_loop}) - "
                          f"Szenario-Voraussetzung{vermerk}")
            code = schlechter(code, EXIT_SZENARIO)
            continue
        c, z, m = messe_stelle(lauf, rollen, stelle, a_frames, b_frames, loop_ms, vermerk,
                               lage_im_host(a_frames, b_frames, v, v_bekannt))
        code = schlechter(code, c)
        zeilen += z
        maengel += m
    kurz = "; ".join(maengel) if maengel else ("gemessen, Rohdaten unten" if code == EXIT_OK
                                               else "Voraussetzung einer Stelle fehlt")
    return code, kurz, zeilen


# ---------------------------------------------------------------- Rechnung aus F-28 (py -3.13: numpy, scipy, soundfile, pyloudnorm)

def _nulltest_modul():
    ordner = str(Path(__file__).resolve().parent)
    if ordner not in sys.path:
        sys.path.insert(0, ordner)
    import nulltest

    return nulltest


def lade_analyze():
    """analyze() aus analyze-track.py, Ladeweg wie tools/eq-copilot/erzeuge_fixtures.py:146-162."""
    import importlib.util

    for kandidat in (REPO / "tools" / "analyze-track.py", Path.home() / "FL-Studio" / "tools" / "analyze-track.py"):
        if kandidat.exists():
            spec = importlib.util.spec_from_file_location("analyze_track", kandidat)
            modul = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(modul)  # type: ignore[union-attr]
            return modul, kandidat
    return None, None


def lade_quelle_int(pfad: Path):
    """Quelle als 24-Bit-Ganzzahlen k (Frames x 2); Sample = k / 2^23."""
    import numpy as np

    nt = _nulltest_modul()
    w = nt.lies_kopf(pfad)
    if not (w.tag == nt.PCM and w.bits == 24 and w.kanaele == 2 and w.rate == RATE):
        raise ValueError(f"Quelle nicht PCM 24 Bit stereo 44,1 kHz: {w.beschreibung()}")
    with open(pfad, "rb") as f:
        f.seek(w.daten_offset)
        roh = f.read(w.frames * w.block)
    b = np.frombuffer(roh, dtype=np.uint8).reshape(-1, 3).astype(np.int32)
    k = b[:, 0] | (b[:, 1] << 8) | (b[:, 2] << 16)
    k = np.where(k >= 1 << 23, k - (1 << 24), k)
    return w, k.reshape(-1, 2)


def schreibe_ausschnitt(w, von: int, bis: int, ziel: Path) -> str:
    """Referenzausschnitt [von, bis) als WAV aus den Originalbytes der Quelle; SHA-256 ueber die Datenbytes."""
    import struct

    with open(w.pfad, "rb") as f:
        f.seek(w.daten_offset + von * w.block)
        daten = f.read((bis - von) * w.block)
    fmt = struct.pack("<HHIIHH", 1, 2, RATE, RATE * w.block, w.block, 24)
    inhalt = b"WAVE" + b"fmt " + struct.pack("<I", len(fmt)) + fmt + b"data" + struct.pack("<I", len(daten)) + daten
    ziel.write_bytes(b"RIFF" + struct.pack("<I", len(inhalt)) + inhalt)
    return hashlib.sha256(daten).hexdigest().upper()


_LUFS_ARBEIT: tuple | None = None


def _lufs_start(pfad: str) -> None:
    global _LUFS_ARBEIT
    import pyloudnorm as pyln
    import soundfile as sf

    daten, rate = sf.read(pfad, always_2d=True)
    _LUFS_ARBEIT = (daten.astype("float64"), pyln.Meter(rate))


def _lufs_teil(phis: list[int]) -> list[tuple[int, float]]:
    daten, meter = _LUFS_ARBEIT  # type: ignore[misc]
    return [(phi, float(meter.integrated_loudness(daten[phi:]))) for phi in phis]


def s_lufs(pfad: Path) -> dict:
    """Rasterspanne S_LUFS: LUFS integriert (pyloudnorm wie analyze(), analyze-track.py:243)
    ueber den Ausschnitt ohne seine ersten phi Frames, phi = 0 bis 4 409 (F-28)."""
    from concurrent.futures import ProcessPoolExecutor

    t0 = time.perf_counter()
    arbeiter = max(1, min(os.cpu_count() or 1, 12))
    phis = list(range(ZELLE))
    teile = [phis[i::arbeiter * 4] for i in range(arbeiter * 4)]
    with ProcessPoolExecutor(max_workers=arbeiter, initializer=_lufs_start, initargs=(str(pfad),)) as pool:
        werte = sorted(x for teil in pool.map(_lufs_teil, teile) for x in teil)
    endlich = [(phi, l) for phi, l in werte if math.isfinite(l)]
    if not endlich:
        return {"fehler": "kein endlicher LUFS-Wert ueber die Rasterversaetze"}
    tief = min(endlich, key=lambda x: x[1])
    hoch = max(endlich, key=lambda x: x[1])
    return {"min": tief[1], "phi_min": tief[0], "max": hoch[1], "phi_max": hoch[0], "spanne": hoch[1] - tief[1],
            "phi0": werte[0][1], "versaetze": len(werte), "nicht_endlich": len(werte) - len(endlich),
            "arbeiter": arbeiter, "dauer_s": round(time.perf_counter() - t0, 2)}


def low_frac_kanal(teil) -> float | None:
    """low_frac in der Definition des Snapshots (F-28, Ergaenzung 15.09.2026): die Welch-Stufe
    `referenz` der Engine mit n = 8 192 und Sprung 4 096, periodischem Hann, PSD je Kanal und
    Leistungsmittel (PSD_L + PSD_R) / 2, gemittelt nur ueber aktive Segmente (mittlere
    Kanalenergie ueber -60 dB), Anteil der Bins unter 250 Hz (eq-copilot/plugin/src/AnalyseEngine.cpp:69-89,
    :110-154, :211, :838-894; die Randbins werden dort halbiert und wieder verdoppelt, im Verhaeltnis
    ohne Wirkung, die Dichteskala kuerzt sich). analyze() rechnet low_frac ueber den Mid-Mix
    0,5 (L + R) (analyze-track.py:236, :278-285) - eine andere Groesse; ohne ein ganzes aktives
    Segment None."""
    import numpy as np

    n, hop = 8192, 4096
    x = np.asarray(teil, dtype=np.float64) / (1 << 23)
    if x.ndim != 2 or x.shape[0] < n:
        return None
    fenster = 0.5 - 0.5 * np.cos(2.0 * np.pi * np.arange(n) / n)
    summe = np.zeros(n // 2 + 1)
    aktiv = 0
    for anfang in range(0, x.shape[0] - n + 1, hop):
        seg = x[anfang:anfang + n]
        energie = 0.5 * float((seg[:, 0] ** 2).sum() + (seg[:, 1] ** 2).sum())
        if not 10.0 * math.log10(energie / n + 1e-30) > AKTIV_GATE_DB:
            continue
        aktiv += 1
        summe += 0.5 * (np.abs(np.fft.rfft(seg[:, 0] * fenster)) ** 2 + np.abs(np.fft.rfft(seg[:, 1] * fenster)) ** 2)
    gesamt = float(summe.sum())
    if aktiv == 0 or gesamt <= 0.0:
        return None
    frequenz = np.arange(n // 2 + 1) * RATE / n
    return float(summe[frequenz < 250.0].sum() / gesamt)


def u_unten(k, s0: int, n_l: int, delta_k_db: float, urteil: str) -> dict:
    """U_unten = 0,1 s x min_phi Z_phi (F-28): Z_phi zaehlt die vollstaendigen Zellen
    [s0 + phi + j*4410, ... + 4410) in [s0, s0 + n_L), deren mittlere Kanalenergie der
    Quelle, samplegenau und ganzzahlig gerechnet, ueber -60 dB - Delta_K + delta liegt;
    Frames ausserhalb der Quelle tragen keine Energie.
    delta = -10*log10(1 - eps): eps = (2*4410 + 2) * 2^-53 deckt die Rundung der
    Double-Summe der Engine (AnalyseEngine.cpp:384-389, 4 410 nicht negative Summanden,
    Produkt und Summe je Sample); bei KETTE kommt die Float-32-Rundung des Hostpfads
    hinzu, 2^-21 relativ auf die Energie (zwei Rundungsstufen je Sample, nulltest.py)."""
    import numpy as np

    m = k.shape[0]
    index = np.arange(s0, s0 + n_l, dtype=np.int64)
    gueltig = (index >= 0) & (index < m)
    energie = np.zeros(n_l, dtype=np.int64)
    kk = k[index[gueltig]].astype(np.int64)
    energie[gueltig] = kk[:, 0] * kk[:, 0] + kk[:, 1] * kk[:, 1]
    summe = np.zeros(n_l + 1, dtype=np.int64)
    np.cumsum(energie, out=summe[1:])  # modulo 2^64; jede Zellsumme < 4 410 * 2^47 < 2^63 bleibt exakt
    eps = (2 * ZELLE + 2) * 2.0 ** -53 + (2.0 ** -21 if urteil == "KETTE" else 0.0)
    delta_db = -10.0 * math.log10(1.0 - eps)
    schwelle_db = AKTIV_GATE_DB - delta_k_db + delta_db
    schwelle_summe = 10.0 ** (schwelle_db / 10.0) * (2 * ZELLE * 2.0 ** 46)
    zellen = n_l // ZELLE
    phi = np.arange(ZELLE, dtype=np.int64)[:, None]
    j = np.arange(zellen, dtype=np.int64)[None, :]
    anfang = phi + j * ZELLE
    ganz = anfang + ZELLE <= n_l
    anfang = np.where(ganz, anfang, 0)
    if zellen:
        zellsumme = summe[anfang + ZELLE] - summe[anfang]
        z = ((zellsumme.astype(np.float64) > schwelle_summe) & ganz).sum(axis=1)
    else:
        z = np.zeros(ZELLE, dtype=np.int64)
    return {"s0": s0, "n_l": n_l, "min_zellen": int(z.min()), "phi_min": int(z.argmin()),
            "max_zellen": int(z.max()), "zellen_phi0": int(z[0]), "u_unten_s": round(0.1 * int(z.min()), 1),
            "delta_db": delta_db, "schwelle_db": schwelle_db, "eps": eps,
            "frames_ausserhalb_quelle": int((~gueltig).sum())}


def zellenenergie_db(k):
    """Zellenenergie je 100 ms der Quelle: 10*log10 der mittleren Kanalenergie (F-21)."""
    import numpy as np

    zellen = k.shape[0] // ZELLE
    kk = k[: zellen * ZELLE].astype(np.int64)
    summe = (kk[:, 0] * kk[:, 0] + kk[:, 1] * kk[:, 1]).reshape(zellen, ZELLE).sum(axis=1)
    return 10.0 * np.log10(summe.astype(np.float64) / (2 * ZELLE * 2.0 ** 46) + 1e-30)


def stelle_statistik(db, von_s: float, bis_s: float) -> dict:
    von = math.ceil(round(von_s * RATE) / ZELLE)
    bis = round(bis_s * RATE) // ZELLE
    werte = db[von:bis]
    if werte.size == 0:
        return {"zellen": 0}
    return {"zellen": int(werte.size), "mittel_db": round(float(werte.mean()), 3),
            "min_db": round(float(werte.min()), 3), "max_db": round(float(werte.max()), 3),
            "anteil_66_40": round(float(((werte >= -66.0) & (werte <= -40.0)).mean()), 4)}


def waehle_stellen(k) -> list[dict]:
    """S1 Intro und S3 Ausklang fest, S2 das Vier-Takt-Fenster mit der kleinsten
    mittleren Zellenenergie (arithmetisches Mittel in dB) unter den Fenstern ab
    Takt 3 bis Takt 20 (F-21)."""
    db = zellenenergie_db(k)
    fenster = []
    for takt in range(3, 21):
        von = (takt - 1) * TAKT_S
        bis = von + 4 * TAKT_S
        fenster.append((stelle_statistik(db, von, bis)["mittel_db"], takt, von, bis))
    _mittel, takt, von, bis = min(fenster)
    stellen = [
        {"id": "S1", "name": "Intro", "von_s": 0.0, "bis_s": 3.692, "takte": "1-2"},
        {"id": "S2", "name": "ruhige Passage", "von_s": round(von, 3), "bis_s": round(bis, 3),
         "takte": f"{takt}-{takt + 3}"},
        {"id": "S3", "name": "Ausklang", "von_s": 42.462, "bis_s": 45.596, "takte": "24 bis 0,1 s vor dem Loopende"},
    ]
    for stelle in stellen:
        stelle["energie"] = stelle_statistik(db, stelle["von_s"], stelle["bis_s"])
    return stellen


def rechne_auftrag(auftrag: dict, analyze=None, quelle_k=None) -> dict:
    """Referenzausschnitte, analyze() je Ausschnitt, S_LUFS fuer schwere Anker und
    U_unten je Rolle; jede Zahl mit ihrer Materialzeit (F-28)."""
    t0 = time.perf_counter()
    w = None
    if quelle_k is None:
        ist = sha256_datei(Path(auftrag["quelle"]))
        if not ist or ist != str(auftrag.get("quelle_sha256") or "").upper():
            return {"fehler": f"Quelle mit fremdem oder fehlendem SHA-256: {ist}"}
        w, quelle_k = lade_quelle_int(Path(auftrag["quelle"]))
    analyze_pfad = None
    if analyze is None:
        modul, analyze_pfad = lade_analyze()
        if modul is None:
            return {"fehler": "analyze-track.py fehlt (FL-Studio-Repo)"}
        analyze = modul.analyze
    ordner = Path(auftrag["ordner"])
    ordner.mkdir(parents=True, exist_ok=True)
    aus: dict = {"quelle_sha256": auftrag.get("quelle_sha256"), "analyze": str(analyze_pfad) if analyze_pfad else None,
                 "anker": [], "u_unten": []}
    cache: dict[tuple[int, int], dict] = {}
    for anker in auftrag.get("anker", []):
        von, bis = int(anker["ausschnitt"][0]), int(anker["ausschnitt"][1])
        eintrag = cache.get((von, bis))
        if eintrag is None:
            ziel = ordner / f"ausschnitt-{auftrag.get('head', 'lauf')}-{von}-{bis}.wav"
            sha = schreibe_ausschnitt(w, von, bis, ziel) if w is not None else None
            t_analyze = time.perf_counter()
            a = analyze(str(ziel))
            teil = quelle_k[von:bis]
            eintrag = {"ausschnitt": [von, bis], "pfad": str(ziel), "sha256": sha,
                       "lufs": float(a["lufs"]), "tp_dbtp": float(a["tp_dbtp"]), "width": float(a["width"]),
                       "corr": float(a["corr"]), "low_frac": float(a["low_frac"]), "low_frac_kanal": low_frac_kanal(teil),
                       "centroid_mag": float(a["centroid_mag"]),
                       "resonanzen": [round(float(r["freq"]), 1) for r in a["resonances"]],
                       "stichprobenspitze": [max(abs(int(teil[:, c].max())), abs(int(teil[:, c].min()))) / (1 << 23)
                                             for c in range(2)] if teil.size else [0.0, 0.0],
                       "analyze_s": round(time.perf_counter() - t_analyze, 2)}
            cache[(von, bis)] = eintrag
        if anker.get("s_lufs") and "s_lufs" not in eintrag:
            eintrag["s_lufs"] = s_lufs(Path(eintrag["pfad"]))
            if eintrag["s_lufs"].get("fehler"):
                return {"fehler": f"S_LUFS: {eintrag['s_lufs']['fehler']}"}
        aus["anker"].append({"rolle": anker["rolle"], "art": anker["art"], **eintrag})
    for u in auftrag.get("u_unten", []):
        aus["u_unten"].append({"rolle": u["rolle"], **u_unten(quelle_k, int(u["s0"]), int(u["n_l"]),
                                                               float(auftrag["delta_k_db"]), str(auftrag["urteil"]))})
    aus["dauer_s"] = round(time.perf_counter() - t0, 2)
    return aus


# ---------------------------------------------------------------- Szenario fahren

LOKALE_AKTIONEN = {
    "lokal.briefkasten": lokal_briefkasten,
    "lokal.nulltest": lokal_nulltest,
    "lokal.fenster": lokal_fenster,
    "lokal.umlauf": lokal_umlauf,
    "lokal.stellen": lokal_stellen,
}


def fahre(szenario_pfad: Path, roh: list[str], lauf: Lauf) -> int:
    try:
        szenario = json.loads(szenario_pfad.read_text(encoding="utf-8"))
        schritte = szenario["schritte"]
        if not isinstance(schritte, list):
            raise ValueError("schritte ist keine Liste")
    except (OSError, ValueError, KeyError, TypeError) as e:
        roh.append(f"## Szenario `{szenario_pfad.name}`: UNBRAUCHBAR ({e})\n")
        return EXIT_DATEI

    lauf.szenario = szenario
    lauf.details = []
    conn, fehler = lauf.umg.verbindung()
    kopf = f"## Szenario `{szenario.get('id', szenario_pfad.stem)}` — {szenario.get('titel', '')}\n"
    roh.append(kopf)
    if conn is None:
        roh.append(f"VORAUSSETZUNG FEHLT: {fehler}\n")
        return EXIT_VORAUSSETZUNG
    lauf.conn = conn

    roh.append("| # | Aktion | Parameter | Antwort | Urteil |\n|---|---|---|---|---|")
    verfehlt = 0
    ungemessen = 0
    verworfen_vorher = len(lauf.verworfen)
    ergebnis = EXIT_OK
    for nr, schritt in enumerate(schritte, 1):
        if "warte_s" in schritt:
            lauf.umg.schlafe(float(schritt["warte_s"]))
            roh.append(f"| {nr} | warte | {schritt['warte_s']} s | — | — |")
            continue
        aktion = schritt.get("aktion")
        if not aktion:
            roh.append(f"| {nr} | ? | — | — | VERFEHLT (kein `aktion`) |")
            verfehlt += 1
            ergebnis = schlechter(ergebnis, EXIT_VERFEHLT)
            continue
        params = schritt.get("params") or {}
        p = _kompakt(params).replace("|", "\\|")
        if aktion.startswith("lokal."):
            funktion = LOKALE_AKTIONEN.get(aktion)
            vorher = len(lauf.verworfen)
            try:
                if funktion is None:
                    code, kurz, zeilen = EXIT_VERFEHLT, f"unbekannte lokale Aktion {aktion}", []
                else:
                    code, kurz, zeilen = funktion(lauf, schritt)
            except Exception as e:  # noqa: BLE001 - eine Ausnahme ist ein verfehlter Schritt mit Rohausgabe
                code, kurz, zeilen = EXIT_VERFEHLT, f"Ausnahme {type(e).__name__}: {e}", traceback.format_exc().splitlines()
            finally:
                try:
                    lauf.briefkasten.raeume_anfrage_ab()
                except OSError:
                    pass
            urteil = {EXIT_OK: "ok", EXIT_SZENARIO: "VORAUSSETZUNG"}.get(code, "VERFEHLT")
            # NAK-309 (R-309-2): hat die lokale Aktion eine MCP-Antwort ohne passende Kennung verworfen (etwa eine
            # Positionsabfrage), misst dieser Schritt nichts - UNGEMESSEN, Szenario-Exit 5.
            neu_verworfen = len(lauf.verworfen) - vorher
            if neu_verworfen:
                code = schlechter(code, EXIT_SZENARIO)
                urteil = "UNGEMESSEN"
                kurz = f"{kurz} · UNGEMESSEN: {neu_verworfen} MCP-Antwort(en) ohne passende Kennung verworfen"
                ungemessen += 1
            if code != EXIT_OK:
                verfehlt += 1
            ergebnis = schlechter(ergebnis, code)
            roh.append(f"| {nr} | `{aktion}` | `{p}` | {_zelle(kurz)} | {urteil}{' (Details unten)' if zeilen else ''} |")
            if zeilen:
                lauf.details += ["", f"### Schritt {nr} `{aktion}`", ""] + zeilen
            continue
        frist = float(schritt["frist_s"]) if schritt.get("frist_s") is not None else None
        antwort = _sende(conn, aktion, params, frist, lauf.umg, lauf.verworfen)
        if antwort.get("_ungemessen"):
            # NAK-309 (R-309-2, M-42, M-44): keine Messung - der Schritt wird nie an einer fremden Antwort bewertet.
            ungemessen += 1
            verfehlt += 1
            ergebnis = EXIT_VORAUSSETZUNG if aktion == "system.ping" else schlechter(ergebnis, EXIT_SZENARIO)
            a = _antwort_zelle(antwort.get("_verworfen")).replace("|", "\\|")
            roh.append(f"| {nr} | `{aktion}` | `{p}` | `{a}` | {_zelle(antwort['error'])} |")
            if ergebnis == EXIT_VORAUSSETZUNG:
                roh.append("\nAbbruch: Ping ohne passende Anforderungskennung — der Controller in FL trägt keine "
                           "Kennung (Stand vor 2026-09-18) oder die Antwort gehört zu einem anderen Befehl.\n")
                return ergebnis
            continue
        if antwort.get("success"):
            lauf.letzte[aktion] = antwort
        maengel, rohwerte = _pruefe(schritt.get("erwarte") or {}, antwort)
        if aktion == "system.ping" and not antwort.get("success"):
            ergebnis = EXIT_VORAUSSETZUNG
        urteil = "ok" if not maengel else "VERFEHLT: " + "; ".join(maengel)
        if rohwerte:
            urteil += " · roh: " + "; ".join(rohwerte)
        if maengel:
            verfehlt += 1
            ergebnis = schlechter(ergebnis, EXIT_VERFEHLT)
        a = _antwort_zelle(antwort).replace("|", "\\|")
        roh.append(f"| {nr} | `{aktion}` | `{p}` | `{a}` | {_zelle(urteil)} |")
        if ergebnis == EXIT_VORAUSSETZUNG:
            roh.append("\nAbbruch: kein Ping — FL läuft nicht oder der Controller antwortet nicht.\n")
            return ergebnis

    ergebnis_zeile = f"\n**Ergebnis:** {len(schritte) - verfehlt} von {len(schritte)} Schritten bestanden."
    verworfen = lauf.verworfen[verworfen_vorher:]
    if verworfen:
        ergebnis_zeile += (f" {ungemessen} Schritt(e) UNGEMESSEN; {len(verworfen)} MCP-Antwort(en) ohne passende "
                           "Kennung verworfen (NAK-309 R-309-2).")
        lauf.details += ["", "### Verworfene MCP-Antworten (ohne passende Kennung)", ""] + [
            f"- `{v['aktion']}`: gesendet {v['gesendet']}, erhalten {v['erhalten'] if v['erhalten'] is not None else 'keine'}"
            for v in verworfen]
    roh.append(ergebnis_zeile + "\n")
    roh += lauf.details
    return ergebnis


def ping(umg: Umgebung) -> int:
    conn, fehler = umg.verbindung()
    # NAK-309 (M-50): der echte Importgraph des Laufs steht in der Ping-Zeile; laufzeit.ps1 protokolliert ihn.
    module = sorted(m for m in sys.modules if m == "fl_studio_mcp" or m.startswith("fl_studio_mcp."))
    if conn is None:
        print(json.dumps({"success": False, "error": fehler, "fl_studio_mcp_module": module}, ensure_ascii=False))
        return EXIT_VORAUSSETZUNG
    antwort = _sende(conn, "system.ping", {}, 3.0, umg)
    antwort["port_name"] = conn.get_status().get("port_name")
    antwort["fl_studio_mcp_module"] = module
    print(json.dumps(antwort, ensure_ascii=False))
    return EXIT_OK if antwort.get("success") else EXIT_VORAUSSETZUNG


def main(argv: list[str], umg: Umgebung | None = None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("szenarien", nargs="*", type=Path, help="Szenariodateien (JSON)")
    ap.add_argument("--roh", type=Path, help="Markdown-Datei, an die die Rohausgabe angehängt wird")
    ap.add_argument("--ping", action="store_true", help="nur Ping, JSON auf stdout")
    ap.add_argument("--diagnose-pid", type=int, help="PID des Diagnose-FL (laufzeit.ps1, Fenstertitel)")
    ap.add_argument("--render-ordner", type=Path)
    ap.add_argument("--diagnose-ordner", type=Path)
    ap.add_argument("--projekt-ordner", type=Path)
    ap.add_argument("--head")
    ap.add_argument("--selbsttest", action="store_true", help="Selbsttest ohne FL und ohne MCP-Repo")
    ap.add_argument("--nur", action="append", default=[], help="nur diese Selbsttestfaelle")
    ap.add_argument("--rechne", type=Path, help="Rechnung aus F-28 (py -3.13)")
    ap.add_argument("--ausgabe", type=Path)
    ap.add_argument("--energieprofil", type=Path, help="Stellen fuer U40 aus der Quelle (F-21)")
    args = ap.parse_args(argv)

    if args.selbsttest:
        return selbsttest(args.nur)
    if args.rechne:
        if not args.ausgabe:
            ap.error("--rechne braucht --ausgabe")
        ergebnis = rechne_auftrag(json.loads(args.rechne.read_text(encoding="utf-8")))
        args.ausgabe.write_text(json.dumps(ergebnis, ensure_ascii=False), encoding="utf-8")
        return EXIT_OK
    if args.energieprofil:
        _w, k = lade_quelle_int(args.energieprofil)
        print(json.dumps({"quelle_sha256": sha256_datei(args.energieprofil), "stellen": waehle_stellen(k)},
                         ensure_ascii=False, indent=1))
        return EXIT_OK
    umg = umg or Umgebung()
    if args.ping:
        return ping(umg)
    if not args.szenarien:
        ap.error("mindestens eine Szenariodatei oder --ping")

    roh: list[str] = [f"\n<!-- szenario.py {_utc()} -->"]
    lauf = Lauf(umg, args, roh)
    schlechtester = EXIT_OK
    for pfad in args.szenarien:
        code = fahre(pfad, roh, lauf)
        schlechtester = schlechter(schlechtester, code)
        if code == EXIT_VORAUSSETZUNG:
            break

    text = "\n".join(roh) + "\n"
    if args.roh:
        args.roh.parent.mkdir(parents=True, exist_ok=True)
        with args.roh.open("a", encoding="utf-8") as f:
            f.write(text)
    else:
        sys.stdout.write(text)
    print(f"SZENARIO EXIT={schlechtester}")
    return schlechtester


# ---------------------------------------------------------------- Selbsttest (A35): Attrappen

class TestUmgebung(Umgebung):
    """Uhr, Dateien, Verbindung, Prozesse und Unterprozesse als Attrappen. Die Zeit
    laeuft nur ueber schlafe(); eine Datei ist ab ihrem Zeitpunkt sichtbar."""

    def __init__(self):
        self.t = 1000.0
        self.dateien: dict[Path, tuple[bytes, float]] = {}
        self.ordner: set[Path] = set()
        self.oeffnungen: Counter = Counter()
        self.geloescht: list[str] = []
        self.anfragen: list[dict] = []
        self.anfrage_bytes: list[bytes] = []
        self.kennungen = 0
        self.mcp_kennungen = 0
        self.conn = None
        self.instanzen = None
        self.broker: list[int] = []
        self.fl: list[int] = []
        self.rechnung = None
        self.rechne_auftraege: list[dict] = []
        self.nulltest_aufrufe: list[list[str]] = []
        self.nulltest_antwort = (EXIT_OK, "NULLTEST Attrappe")
        self.fenster_ergebnis = None

    def jetzt(self) -> float:
        return self.t

    def schlafe(self, sekunden: float) -> None:
        if sekunden > 0:
            self.t += sekunden

    def verbindung(self):
        return (self.conn, None) if self.conn else (None, "kein virtueller MIDI-Port (Attrappe)")

    def daten(self, pfad: Path) -> bytes:
        """Inhalt einer Attrappendatei; ein Callable (Antwort, deren Rahmen zur Antwortzeit entsteht)
        wird beim ersten Zugriff nach seinem Zeitpunkt festgeschrieben."""
        inhalt, ab = self.dateien[pfad]
        if callable(inhalt):
            inhalt = inhalt()
            self.dateien[pfad] = (inhalt, ab)
        return inhalt

    def liste(self, ordner: Path) -> list[tuple[str, int]]:
        return [(p.name, len(self.daten(p))) for p, (_inhalt, ab) in sorted(self.dateien.items(), key=lambda kv: kv[0])
                if p.parent == ordner and ab <= self.t]

    def lies(self, pfad: Path, grenze: int) -> bytes:
        self.oeffnungen[pfad.name] += 1
        return self.daten(pfad)[: grenze + 1]

    def lies_text(self, pfad: Path) -> str | None:
        eintrag = self.dateien.get(pfad)
        return self.daten(pfad).decode("utf-8") if eintrag and eintrag[1] <= self.t else None

    def existiert(self, pfad: Path) -> bool:
        return (pfad in self.dateien and self.dateien[pfad][1] <= self.t) or pfad in self.ordner

    def lege_ordner_an(self, pfad: Path) -> None:
        self.ordner.add(pfad)

    def loesche(self, pfad: Path) -> None:
        self.geloescht.append(pfad.name)
        self.dateien.pop(pfad, None)

    def lege(self, pfad: Path, daten: bytes, ab: float | None = None) -> None:
        self.dateien[pfad] = (daten, self.t if ab is None else ab)

    def schreibe_atomar(self, pfad: Path, daten: bytes) -> None:
        self.dateien[pfad] = (daten, self.t)
        if pfad.name == "anfrage.json":
            self.anfrage_bytes.append(daten)
            anfrage = json.loads(daten.decode("utf-8"))
            self.anfragen.append(anfrage)
            if self.instanzen is not None:
                self.instanzen.anfrage(self, pfad.parent / "antwort", anfrage)

    def kennung(self) -> str:
        self.kennungen += 1
        return f"{self.kennungen:032x}"

    def mcp_kennung(self) -> str:
        """Anforderungskennungen der Attrappe: c000...1, c000...2, ... - vorhersagbar je Versuch (NAK-309)."""
        self.mcp_kennungen += 1
        return f"c{self.mcp_kennungen:031x}"

    def sha256(self, pfad: Path) -> str | None:
        eintrag = self.dateien.get(pfad)
        return hashlib.sha256(self.daten(pfad)).hexdigest().upper() if eintrag else None

    def schreibzeit(self, pfad: Path) -> float | None:
        eintrag = self.dateien.get(pfad)
        return eintrag[1] if eintrag else None

    def broker_pids(self) -> list[int]:
        return list(self.broker)

    def fl_pids(self) -> list[int]:
        return list(self.fl)

    def nulltest(self, argumente: list[str]) -> tuple[int, str]:
        self.nulltest_aufrufe.append(list(argumente))
        if callable(self.nulltest_antwort):
            return self.nulltest_antwort(argumente)
        return self.nulltest_antwort

    def rechne(self, auftrag: dict) -> dict:
        self.rechne_auftraege.append(json.loads(json.dumps(auftrag)))
        if callable(self.rechnung):
            return self.rechnung(auftrag)
        return self.rechnung if self.rechnung is not None else {"fehler": "keine Attrappen-Rechnung"}

    def fenster(self, pid: int, ziel: str, ordner: Path, name: str, plugin: str | None) -> dict:
        return self.fenster_ergebnis(pid, ziel, plugin) if callable(self.fenster_ergebnis) else dict(self.fenster_ergebnis or {})


class TestVerbindung:
    """Verbindungsattrappe mit Transport: die Position laeuft mit der Uhr-Attrappe
    und springt am Loopende auf 0; Vorgaben je Aktion ueberschreiben das.

    Anforderungskennung (NAK-309): `kennung` sagt, was die Attrappe zurueckgibt - "echo" wie der Controller ab
    2026-09-18, "ohne" wie ein Controller alten Stands, "fremd", oder eine Funktion (aktion, request_id, frueher
    gesendete Kennungen) -> Kennung oder None. Eine Vorgabe, die selbst eine Kennung traegt (verspaetete Antwort),
    behaelt sie."""

    def __init__(self, umg: TestUmgebung, loop_ms: int = 45696, antworten: dict | None = None, positionen=None,
                 kennung="echo"):
        self.umg = umg
        self.loop_ms = loop_ms
        self.antworten = antworten or {}
        self.positionen = positionen
        self.kennung = kennung
        self.aufrufe: list[tuple[str, dict, float | None]] = []
        self.kennungen: list[str | None] = []
        self.spielt = False
        self.basis_ms = 0.0
        self.start_t = umg.t

    def position(self) -> float:
        if self.positionen is not None:
            return self.positionen(self)
        if not self.spielt:
            return self.basis_ms
        return (self.basis_ms + (self.umg.t - self.start_t) * 1000.0) % self.loop_ms

    def send_command(self, aktion: str, params: dict | None = None, timeout: float | None = None,
                     request_id: str | None = None) -> dict:
        frueher = list(self.kennungen)
        self.kennungen.append(request_id)
        antwort = self._antwort(aktion, params, timeout)
        if not isinstance(antwort, dict) or "request_id" in antwort:
            return antwort
        antwort = dict(antwort)
        art = self.kennung(aktion, request_id, frueher) if callable(self.kennung) else self.kennung
        if art == "echo":
            if request_id is not None:
                antwort["request_id"] = request_id
        elif art == "fremd":
            antwort["request_id"] = "f" * 32
        elif isinstance(art, str) and art != "ohne":
            antwort["request_id"] = art
        return antwort

    def _antwort(self, aktion: str, params: dict | None, timeout: float | None) -> dict:
        params = dict(params or {})
        self.aufrufe.append((aktion, params, timeout))
        vorgabe = self.antworten.get(aktion)
        if isinstance(vorgabe, list) and vorgabe:
            return dict(vorgabe.pop(0))
        if callable(vorgabe):
            return vorgabe(params, timeout)
        if isinstance(vorgabe, dict):
            return dict(vorgabe)
        if aktion == "transport.start":
            self.basis_ms, self.start_t, self.spielt = self.position(), self.umg.t, True
            return {"success": True, "is_playing": True}
        if aktion == "transport.stop":
            self.basis_ms, self.spielt = self.position(), False
            return {"success": True, "stopped": True}
        if aktion == "transport.setPosition":
            wert = float(params.get("position", 0))
            self.basis_ms = wert if params.get("mode", 1) == 0 else wert * 1000.0
            self.start_t = self.umg.t
            return {"success": True, "requested_position": params.get("position"), "mode": params.get("mode", 1)}
        if aktion == "transport.getPosition":
            ms = self.position()
            return {"success": True, "ms": int(ms), "seconds": int(ms // 1000), "hint": "1:01:00"}
        if aktion == "transport.getLength":
            return {"success": True, "milliseconds": self.loop_ms, "seconds": round(self.loop_ms / 1000), "ticks": 10236, "bars": 27}
        return {"success": True}

    def zaehle(self, aktion: str) -> int:
        return sum(1 for a, _p, _t in self.aufrufe if a == aktion)

    def get_status(self) -> dict:
        return {"port_name": "loopMIDI Port 1 (Attrappe)"}


def attrappe_name(kennung: str, rolle: str, pid: int, instanz: str, laufzeit: str) -> str:
    return f"{kennung}.{rolle}.{pid}.{hashlib.sha256(instanz.encode('utf-8')).hexdigest()[:16]}.{laufzeit}.json"


def attrappe_umschlag(kennung: str, rolle: str, pid: int, instanz: str, laufzeit: str, snapshot, frame) -> dict:
    gruende = []
    if snapshot is None:
        gruende.append({"feld": "snapshot", "code": "noch_keine_messdaten", "text": "Attrappe"})
    if frame is None:
        gruende.append({"feld": "frame", "code": "noch_keine_messdaten", "text": "Attrappe"})
    gruende.append({"feld": "aggregat", "code": "rolle_ohne_aggregat", "text": "Attrappe"})
    return {"format": ANTWORT_FORMAT, "anfrage_id": kennung, "rolle": rolle, "instanz_id": instanz,
            "laufzeit_id": laufzeit, "pid": pid, "erzeugt_utc": "2026-09-15T01:00:00Z", "version": "0.0.0-attrappe",
            "snapshot": snapshot, "frame": frame, "aggregat": None, "gruende": gruende}


def attrappe_snapshot(zustand="messbereit", aktiv=40.0, gesamt=42.0, lufs=-22.45, tp=-7.8, width=0.33, corr=0.5,
                      low_frac=0.78, centroid=876.0, resonanzen=(459.6, 688.7), rate=RATE,
                      metrics=METRICS_VERSION) -> dict:
    return {"snapshot_version": 3, "metrics_version": metrics, "created_utc": "2026-09-15T01:00:00Z",
            "sensor": {"sensor_id": "attrappe", "role": "sensor", "label": "", "pair_id": None,
                       "samplerate": rate, "channels": 2},
            "zustand": zustand, "aktiv_sekunden": aktiv, "gesamt_sekunden": gesamt,
            "loudness": {"lufs_integriert": lufs, "true_peak_dbtp": tp},
            "spektral": {"centroid_mag_hz": centroid, "low_frac": low_frac},
            "stereo": {"width": width, "corr": corr}, "ltas": {}, "raw_audio": None,
            "resonanzen": [{"freq_hz": f, "db_over": 7.0, "persistenz": 0.6, "klasse": "dauerhaft"} for f in resonanzen]}


def attrappe_frame(**werte) -> dict:
    frame = {"frames_gebaut": 100, "summe_fenster_gesamt": 400, "summe_fenster_aktiv": 300, "evidenz_frisch": True,
             "samplerate": RATE, "aktivitaet": 0.5, "abdeckung": 0.6, "konvergenz": 0.4, "evidenz_fenster": 4,
             "lufs_m": -25.0, "lufs_s": -24.0, "true_peak_db": -8.0, "peak_db": -8.5, "integration_samples": 44100,
             "nicht_endlich_rahmen": 0, "offen_fenster_gesamt": 2, "offen_fenster_aktiv": 1, "schwer_sekunden": 41.75,
             "material_ende_projektsample": None, "bloecke_max_samples": 512, "hostzeit_fortlaufend_samples": None,
             "hostzeit_stillstand_bloecke": None, "projekt_sample_start": None, "sample_count": None, "spielt": None}
    frame.update(werte)
    return frame


class Skriptinstanzen:
    """Gen, Probeeq und Broker als Attrappe: vorlage(rolle, nummer, umg) liefert je
    Anfrage None, ein dict oder eine Liste von dicts (snapshot, frame, verzoegerung,
    pid, instanz, laufzeit, name, roh)."""

    def __init__(self, vorlage, pid: int = 4242, verzoegerung: dict | None = None):
        self.vorlage = vorlage
        self.pid = pid
        self.nummer = -1
        self.verzoegerung = verzoegerung or {"gen": 0.4, "probeeq": 1.4, "broker": 0.3}

    def anfrage(self, umg: TestUmgebung, ordner: Path, anfrage: dict) -> None:
        self.nummer += 1
        for rolle in ("gen", "probeeq", "broker"):
            eintraege = self.vorlage(rolle, self.nummer, umg)
            if eintraege is None:
                continue
            for e in ([eintraege] if isinstance(eintraege, dict) else eintraege):
                pid = e.get("pid", self.pid)
                instanz = e.get("instanz", f"instanz-{rolle}")
                laufzeit = e.get("laufzeit", hashlib.md5(f"{rolle}:{instanz}".encode("utf-8")).hexdigest())
                name = e.get("name") or attrappe_name(anfrage["anfrage_id"], rolle, pid, instanz, laufzeit)
                if "roh" in e:
                    daten = e["roh"]
                elif "frame_zur_antwortzeit" in e:
                    def daten(kennung=anfrage["anfrage_id"], rolle=rolle, pid=pid, instanz=instanz, laufzeit=laufzeit, e=e):
                        return json.dumps(attrappe_umschlag(kennung, rolle, pid, instanz, laufzeit, e.get("snapshot"),
                                                            e["frame_zur_antwortzeit"]())).encode("utf-8")
                else:
                    daten = json.dumps(attrappe_umschlag(anfrage["anfrage_id"], rolle, pid, instanz, laufzeit,
                                                         e.get("snapshot"), e.get("frame"))).encode("utf-8")
                umg.lege(ordner / name, daten, umg.t + e.get("verzoegerung", self.verzoegerung[rolle]))


def attrappe_lauf(umg: TestUmgebung, szenario: dict | None = None, **argumente) -> Lauf:
    class Argumente:
        diagnose_pid = 4242
        render_ordner = Path("C:/attrappe/render")
        diagnose_ordner = Path("C:/attrappe/diagnose")
        projekt_ordner = Path("C:/attrappe/projekt")
        head = "abcdef12"

    for schluessel, wert in argumente.items():
        setattr(Argumente, schluessel, wert)
    lauf = Lauf(umg, Argumente, [])
    lauf.conn = umg.conn
    lauf.szenario = szenario or {}
    return lauf


# ---------------------------------------------------------------- Selbsttest (A35): Faelle

FAELLE: list[tuple[str, str, object]] = []


def fall(zeile: str, name: str):
    def eintragen(funktion):
        FAELLE.append((zeile, name, funktion))
        return funktion
    return eintragen


class Pruefer:
    def __init__(self):
        self.fehler: list[str] = []

    def __call__(self, bedingung: bool, text: str) -> None:
        if not bedingung:
            self.fehler.append(text)


def fahre_attrappe(umg: TestUmgebung, szenario: dict, **argumente) -> tuple[int, str, Lauf]:
    """Faehrt ein Szenario-Dict durch fahre() mit Attrappenverbindung und -dateien."""
    lauf = attrappe_lauf(umg, szenario, **argumente)
    with tempfile.TemporaryDirectory(prefix="nak286-szenario-") as tmp:
        pfad = Path(tmp) / f"{szenario.get('id', 'attrappe')}.json"
        pfad.write_text(json.dumps(szenario), encoding="utf-8")
        roh: list[str] = []
        lauf.roh = roh
        code = fahre(pfad, roh, lauf)
    return code, "\n".join(roh), lauf


@fall("M-20", "frist_ohne_schrittwert")
def fall_frist_ohne_schrittwert(p: Pruefer) -> None:
    vorher = os.environ.pop("FL_MCP_TIMEOUT", None)
    try:
        for umgebungswert in (None, "4.5"):
            if umgebungswert is None:
                os.environ.pop("FL_MCP_TIMEOUT", None)
            else:
                os.environ["FL_MCP_TIMEOUT"] = umgebungswert
            umg = TestUmgebung()
            umg.conn = TestVerbindung(umg)
            code, _roh, _lauf = fahre_attrappe(umg, {"id": "frist", "schritte": [
                {"aktion": "system.ping"}, {"aktion": "transport.getStatus", "frist_s": 7}]})
            fristen = [t for _a, _p, t in umg.conn.aufrufe]
            p(fristen == [None, 7.0], f"FL_MCP_TIMEOUT={umgebungswert}: uebergebene Fristen {fristen} statt [None, 7.0] "
                                      "(ohne frist_s entscheidet der MCP)")
            p(code == EXIT_OK, f"FL_MCP_TIMEOUT={umgebungswert}: Exit {code}")
    finally:
        os.environ.pop("FL_MCP_TIMEOUT", None)
        if vorher is not None:
            os.environ["FL_MCP_TIMEOUT"] = vorher


@fall("M-78", "timeout_genau_einmal")
def fall_timeout_genau_einmal(p: Pruefer) -> None:
    timeout = {"success": False, "error": "Timeout waiting for FL Studio response after 2.0s."}
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg, antworten={"transport.getStatus": [timeout, {"success": True, "is_playing": False}]})
    code, roh, _ = fahre_attrappe(umg, {"id": "a", "schritte": [{"aktion": "transport.getStatus", "frist_s": 3}]})
    aufrufe = umg.conn.zaehle("transport.getStatus")
    p(code == EXIT_OK and aufrufe == 2 and '"_wiederholt": true' in roh, f"(a) Exit {code}, {aufrufe} Aufrufe, _wiederholt fehlt")
    p([t for _a, _p, t in umg.conn.aufrufe] == [3.0, 3.0], f"(a) Fristen {[t for _a, _p, t in umg.conn.aufrufe]} statt [3.0, 3.0]")
    p(abs(umg.t - 1000.5) < 1e-9, f"(a) Pause {umg.t - 1000.0:.3f} s statt 0,5 s auf der Uhr-Attrappe")
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg, antworten={"transport.getStatus": [timeout, timeout, {"success": True}]})
    code, _roh, _ = fahre_attrappe(umg, {"id": "b", "schritte": [{"aktion": "transport.getStatus"}]})
    aufrufe = umg.conn.zaehle("transport.getStatus")
    p(code == EXIT_VERFEHLT and aufrufe == 2, f"(b) Exit {code} nach {aufrufe} Aufrufen statt verfehlt nach genau 2")


@fall("M-22", "kein_piano_roll_weg")
def fall_kein_piano_roll_weg(p: Pruefer) -> None:
    import ast

    baum = ast.parse(Path(__file__).read_text(encoding="utf-8"))
    treffer = []
    for knoten in ast.walk(baum):
        if isinstance(knoten, ast.Import):
            namen = [a.name for a in knoten.names]
        elif isinstance(knoten, ast.ImportFrom):
            namen = [knoten.module or ""] + [a.name for a in knoten.names]
        else:
            continue
        treffer += [f"{n} (szenario.py:{knoten.lineno})" for n in namen if "fl_trigger" in n or "pynput" in n]
    p(not treffer, f"Piano-Roll-Weg importiert: {treffer}")
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg)
    fahre_attrappe(umg, {"id": "x", "schritte": [{"aktion": "system.ping"}]})
    geladen = sorted(m for m in sys.modules if "fl_trigger" in m or m.startswith("pynput"))
    p(not geladen, f"nach dem Attrappenlauf geladen: {geladen}")


ERFASSUNG_ATTRAPPE = (
    "import os\n"
    "import time\n"
    "from pathlib import Path\n"
    "\n"
    "\n"
    "def capture_process_window(pid, target, directory, name, plugin=None):\n"
    "    Path({pid_datei!r}).write_text(str(os.getpid()), encoding='utf-8')\n"
    "    time.sleep({schlaf!r})\n"
    "    return {{'success': True, 'path': str(Path(directory) / (name + '.png')), 'width': 640, 'height': 480,\n"
    "            'sha256': '0' * 64, 'uniform': False, 'window_class': 'TFruityLoopsMainForm', 'window_title': 'Attrappe'}}\n"
)
# Nach der Rueckkehr von subprocess.run(timeout) kann der arbeitende Prozess hinter einem Startprogramm (py -3.13) noch
# enden: gemessen 15.09.2026 unmittelbar danach in 1 von 9 Laeufen am Leben, 0,5 s danach in 3 von 3 Laeufen beendet.
ENDE_WARTEN_MS = 2000


class ErfassungsUmgebung(TestUmgebung):
    """Attrappen wie TestUmgebung, die Fenstererfassung aber ueber den Weg der echten Umgebung."""

    fenster = Umgebung.fenster


def prozess_lebt(pid: int) -> bool:
    """Laeuft der Prozess nach hoechstens ENDE_WARTEN_MS noch (Windows: OpenProcess, WaitForSingleObject)?"""
    if os.name != "nt":
        try:
            os.kill(pid, 0)
        except OSError:
            return False
        return True
    import ctypes
    from ctypes import wintypes

    kernel32 = ctypes.WinDLL("kernel32", use_last_error=True)
    kernel32.OpenProcess.argtypes = [wintypes.DWORD, wintypes.BOOL, wintypes.DWORD]
    kernel32.OpenProcess.restype = wintypes.HANDLE
    kernel32.WaitForSingleObject.argtypes = [wintypes.HANDLE, wintypes.DWORD]
    kernel32.WaitForSingleObject.restype = wintypes.DWORD
    kernel32.CloseHandle.argtypes = [wintypes.HANDLE]
    handle = kernel32.OpenProcess(0x00100000 | 0x1000, False, pid)  # SYNCHRONIZE | PROCESS_QUERY_LIMITED_INFORMATION
    if not handle:
        return False
    try:
        return kernel32.WaitForSingleObject(handle, ENDE_WARTEN_MS) == 0x102  # WAIT_TIMEOUT: laeuft noch
    finally:
        kernel32.CloseHandle(handle)


@fall("M-21", "fenster_frist_erzwungen")
def fall_fenster_frist_erzwungen(p: Pruefer) -> None:
    """P-19: die Frist der Fenstererfassung ist eine erzwungene Obergrenze. Die Attrappe ersetzt nur das
    Erfassungsmodul fl_studio_mcp.utils.fenster (vorn im Suchpfad) und blockiert 4 x die Frist, wie PrintWindow bei
    haengendem FL (T-17); Startweg, Argumente und Antwort sind die des Runners. Die Rueckkehr muss vor der Mitte
    zwischen Frist und Blockade liegen - ohne erzwungene Frist kaeme sie erst mit dem Ende der Blockade."""
    schlaf = 4 * FENSTER_FRIST_S
    grenze = (FENSTER_FRIST_S + schlaf) / 2
    with tempfile.TemporaryDirectory(prefix="nak286-erfassung-", ignore_cleanup_errors=True) as tmp:
        wurzel = Path(tmp)
        pid_datei = wurzel / "erfassung.pid"
        (wurzel / "fl_studio_mcp" / "utils").mkdir(parents=True)
        (wurzel / "fl_studio_mcp" / "__init__.py").write_text("", encoding="utf-8")
        (wurzel / "fl_studio_mcp" / "utils" / "__init__.py").write_text("", encoding="utf-8")
        (wurzel / "fl_studio_mcp" / "utils" / "fenster.py").write_text(
            ERFASSUNG_ATTRAPPE.format(pid_datei=str(pid_datei), schlaf=schlaf), encoding="utf-8")
        suchpfad, pythonpath = list(sys.path), os.environ.get("PYTHONPATH")
        sys.path.insert(0, tmp)
        os.environ["PYTHONPATH"] = tmp if not pythonpath else tmp + os.pathsep + pythonpath
        try:
            umg = ErfassungsUmgebung()
            umg.conn = TestVerbindung(umg)
            t0 = time.monotonic()
            code, roh, _lauf = fahre_attrappe(umg, {"id": "fenster", "schritte": [
                {"aktion": "lokal.fenster", "params": {"ziel": "fl"}}, {"aktion": "system.ping"}]})
            dauer = time.monotonic() - t0
        finally:
            sys.path[:] = suchpfad
            if pythonpath is None:
                os.environ.pop("PYTHONPATH", None)
            else:
                os.environ["PYTHONPATH"] = pythonpath
            for modul in [m for m in sys.modules if m == "fl_studio_mcp" or m.startswith("fl_studio_mcp.")]:
                del sys.modules[modul]
        erfasser = int(pid_datei.read_text(encoding="utf-8")) if pid_datei.exists() else None
    lebt = prozess_lebt(erfasser) if erfasser else None
    p(dauer < grenze, f"Frist nicht erzwungen: lokal.fenster kehrte nach {dauer:.2f} s zurueck (Frist {FENSTER_FRIST_S:g} s, "
                      f"die Attrappe blockiert {schlaf:g} s, Grenze {grenze:g} s)")
    p(erfasser is not None and erfasser != os.getpid() and lebt is False,
      f"Erfassung nicht als beendeter Unterprozess: PID {erfasser} (Selbsttestprozess {os.getpid()}), lebt {lebt}")
    zeile = next((z for z in roh.splitlines() if z.startswith("| 1 | `lokal.fenster` |")), "")
    p(f"Frist {FENSTER_FRIST_S:g} s ueberschritten" in zeile and zeile.rstrip().endswith("| VERFEHLT (Details unten) |"),
      f"Schritt ohne VERFEHLT mit 'Frist {FENSTER_FRIST_S:g} s ueberschritten': '{zeile[:220]}'")
    p(umg.conn.zaehle("system.ping") == 1 and code == EXIT_VERFEHLT,
      f"Szenarioprozess lief nach der Erfassung nicht weiter: system.ping {umg.conn.zaehle('system.ping')}-mal, Exit {code}")


@fall("M-21", "fenster_eingeklappt")
def fall_fenster_eingeklappt(p: Pruefer) -> None:
    """P-17 (§38.2): ein Plugin-Fensterbild unter 200 x 100 Pixel ist ein eingeklappter FL-Wrapper. Der Schritt endet mit
    Szenario-Voraussetzung (Exit 5), Grund eingeklappt und Breite x Hoehe in der Rohzeile, nie gruen; das FL-Hauptfenster
    bleibt unberuehrt. Die Attrappe liefert das Bild aus L-286-1 (67 x 31, TPluginForm) und die Raender der Mindestgroesse."""

    def bild(breite: int, hoehe: int, klasse: str, titel: str) -> dict:
        return {"success": True, "path": f"C:/attrappe/bilder/{klasse}-{breite}x{hoehe}.png", "width": breite,
                "height": hoehe, "sha256": "0" * 64, "uniform": False, "window_class": klasse, "window_title": titel}

    def fahre_fenster(fl: tuple[int, int], plugin: tuple[int, int]) -> tuple[int, str, str]:
        umg = TestUmgebung()
        umg.conn = TestVerbindung(umg)
        umg.fenster_ergebnis = lambda _pid, ziel, _plugin: (
            bild(*plugin, "TPluginForm", "Nakama Probeeq (Insert 1)") if ziel == "plugin"
            else bild(*fl, "TFruityLoopsMainForm", "Nakama-Diagnose.flp - FL Studio 2026"))
        code, roh, _lauf = fahre_attrappe(umg, {"id": "fenster", "schritte": [
            {"aktion": "lokal.fenster", "params": {"ziel": "fl"}},
            {"aktion": "lokal.fenster", "params": {"ziel": "plugin", "plugin": "Nakama Probeeq"}}]})
        zeilen = roh.splitlines()
        return (code, next((z for z in zeilen if z.startswith("| 1 | `lokal.fenster` |")), ""),
                next((z for z in zeilen if z.startswith("| 2 | `lokal.fenster` |")), ""))

    code, zeile_fl, zeile_plugin = fahre_fenster((1920, 1032), (67, 31))
    p(code == EXIT_SZENARIO and "eingeklappt: 67 x 31 Pixel" in zeile_plugin
      and zeile_plugin.rstrip().endswith("| VORAUSSETZUNG (Details unten) |"),
      f"Plugin-Fensterbild 67 x 31 nicht als eingeklappt: Exit {code} statt {EXIT_SZENARIO}, Zeile '{zeile_plugin[-160:]}'")
    p(zeile_fl.rstrip().endswith("| ok (Details unten) |") and "eingeklappt" not in zeile_fl,
      f"FL-Hauptfenster 1920 x 1032 neben dem eingeklappten Plugin-Fenster nicht ok: '{zeile_fl[-160:]}'")
    for breite, hoehe, erwartet in ((199, 480, EXIT_SZENARIO), (640, 99, EXIT_SZENARIO), (200, 100, EXIT_OK)):
        code, _zeile_fl, zeile_plugin = fahre_fenster((1920, 1032), (breite, hoehe))
        eingeklappt = f"eingeklappt: {breite} x {hoehe} Pixel" in zeile_plugin
        p(code == erwartet and eingeklappt == (erwartet == EXIT_SZENARIO),
          f"Mindestgroesse 200 x 100: Plugin-Fensterbild {breite} x {hoehe} endet mit Exit {code} statt {erwartet} "
          f"(eingeklappt in der Zeile: {eingeklappt})")
    code, zeile_fl, _zeile_plugin = fahre_fenster((67, 31), (640, 480))
    p(code == EXIT_OK and "eingeklappt" not in zeile_fl and zeile_fl.rstrip().endswith("| ok (Details unten) |"),
      f"FL-Hauptfenster 67 x 31 als eingeklappter Wrapper gewertet: Exit {code}, Zeile '{zeile_fl[-160:]}'")


@fall("M-55", "anfrage_schreiben_und_abraeumen")
def fall_anfrage_schreiben_und_abraeumen(p: Pruefer) -> None:
    umg = TestUmgebung()
    bk = attrappe_lauf(umg).briefkasten
    alt = attrappe_name("f" * 32, "gen", 4242, "gen", "a" * 32)
    umg.lege(bk.anfrage_pfad, b'{"format":"nakama.diagnose.anfrage.v1","anfrage_id":"' + b"f" * 32 + b'"}')
    umg.lege(bk.antworten / alt, b"{}")
    umg.lege(bk.antworten / (alt + ".tmp-1"), b"")
    umg.lege(bk.antworten / "notiz.txt", b"fremd")
    erste = bk.frage({"gen": {4242}})
    zweite = bk.frage({"gen": {4242}})
    p(umg.geloescht[:3] == ["anfrage.json", alt, alt + ".tmp-1"], f"Aufraeumen: Reihenfolge {umg.geloescht} (zuerst die Anfrage, dann die Antworten)")
    p(bk.antworten / "notiz.txt" in umg.dateien, "fremde Datei im Antwortordner entfernt")
    p(erste.kennung != zweite.kennung and all(KENNUNG.match(a["anfrage_id"]) for a in umg.anfragen), f"Kennungen {erste.kennung}, {zweite.kennung}")
    p(all(set(json.loads(b)) == {"format", "anfrage_id"} and not b.startswith(b"\xef\xbb\xbf") for b in umg.anfrage_bytes),
      "Anfrage nicht genau {format, anfrage_id} als UTF-8 ohne BOM")
    bk.raeume_anfrage_ab()
    p(bk.anfrage_pfad not in umg.dateien, "anfrage.json nach dem Abraeumen vorhanden")


@fall("M-53", "antwortgroessen")
def fall_antwortgroessen(p: Pruefer) -> None:
    umg = TestUmgebung()
    bk = attrappe_lauf(umg).briefkasten
    bk.bereite()
    kennung = "a" * 32
    instanz, laufzeit = "instanz-gen", "b" * 32
    name = attrappe_name(kennung, "gen", 4242, instanz, laufzeit)
    gueltig = json.dumps(attrappe_umschlag(kennung, "gen", 4242, instanz, laufzeit, attrappe_snapshot(), attrappe_frame()))
    riesig_gueltig = gueltig.replace(",", "," + " " * 2_000_000, 9).encode("utf-8")
    vier = {
        name + ".tmp-4242": b"{}",
        attrappe_name(kennung, "gen", 4242, "instanz-leer", "c" * 32): b"",
        attrappe_name(kennung, "gen", 4242, "instanz-17mib", "d" * 32): b"x" * (17 * 1024 * 1024),
        name: riesig_gueltig,
    }
    for n, daten in vier.items():
        umg.lege(bk.antworten / n, daten)
    p(len(riesig_gueltig) > ANTWORT_MAX_BYTES and json.loads(riesig_gueltig)["rolle"] == "gen",
      "Attrappe: die gueltige Antwort ist nicht ueber 16 MiB oder kein gueltiger Umschlag")
    aus = bk.auswahl(kennung, {"gen": {4242}})
    p("gen" in aus.fehlt and "gen" not in aus.gewertet and "gen" not in aus.mehrdeutig,
      f"Groessenriegel: gen gewertet {bool(aus.gewertet.get('gen'))}, mehrdeutig {bool(aus.mehrdeutig.get('gen'))} statt fehlt")
    oeffnungen = {n[:12] + "…" + n[-12:]: umg.oeffnungen[n] for n in vier}
    p(all(v == 0 for v in oeffnungen.values()), f"Leseoeffnungen vor der Groessenpruefung: {oeffnungen}")
    klein = "e" * 32
    umg.lege(bk.antworten / attrappe_name(klein, "gen", 4242, instanz, laufzeit),
             json.dumps(attrappe_umschlag(klein, "gen", 4242, instanz, laufzeit, attrappe_snapshot(), attrappe_frame())).encode("utf-8"))
    p(bk.auswahl(klein, {"gen": {4242}}).gewertet.get("gen") is not None, "Gegenprobe: gueltige Antwort normaler Groesse nicht gewertet")


@fall("M-56", "antworten_auswahl")
def fall_antworten_auswahl(p: Pruefer) -> None:
    umg = TestUmgebung()
    bk = attrappe_lauf(umg).briefkasten
    bk.bereite()
    kennung, alt = "1" * 32, "2" * 32

    def lege(k: str, rolle: str, pid: int, instanz: str, laufzeit: str) -> None:
        umschlag = attrappe_umschlag(k, rolle, pid, instanz, laufzeit, attrappe_snapshot(), attrappe_frame())
        umg.lege(bk.antworten / attrappe_name(k, rolle, pid, instanz, laufzeit), json.dumps(umschlag).encode("utf-8"))

    lege(kennung, "gen", 4242, "gen-a", "a" * 32)
    lege(kennung, "probeeq", 4242, "sonde-a", "b" * 32)
    lege(kennung, "gen", 9999, "gen-fremd", "c" * 32)
    lege(alt, "probeeq", 4242, "sonde-a", "b" * 32)
    umg.lege(bk.antworten / (attrappe_name(kennung, "gen", 4242, "gen-a", "a" * 32) + ".tmp-4242"), b"{}")
    erwartet = {"gen": {4242}, "probeeq": {4242}}
    aus = bk.auswahl(kennung, erwartet)
    gen = aus.gewertet.get("gen")
    p(gen is not None and gen.pid == 4242 and gen.wert("instanz_id") == "gen-a",
      f"gen: gewertet {gen.name if gen else None}, mehrdeutig {[a.name for a in aus.mehrdeutig.get('gen', [])]}")
    p(aus.gewertet.get("probeeq") is not None and aus.gewertet["probeeq"].kennung == kennung, "probeeq: aktuelle Antwort nicht gewertet")
    p(any("PID 9999" in g for _n, g in aus.fremd) and any("andere Kennung" in g for _n, g in aus.fremd),
      f"fremde PID und alte Kennung nicht roh als fremd: {aus.fremd}")
    lege(kennung, "gen", 4242, "gen-b", "d" * 32)
    lege(kennung, "probeeq", 4242, "sonde-a", "e" * 32)
    aus = bk.auswahl(kennung, erwartet)
    p(set(aus.mehrdeutig) == {"gen", "probeeq"} and not aus.gewertet,
      f"mehrdeutig (andere instanz_id; gleiche instanz_id, andere laufzeit_id): {sorted(aus.mehrdeutig)}")
    # Traeger von "fremde PIDs werden nie gewertet" ist allein der PID-Filter: liegt fuer die Rolle nur die Antwort
    # einer fremden PID, haelt keine andere Schranke (Mehrdeutigkeit) die Zusage aufrecht (P-20).
    umg3 = TestUmgebung()
    bk3 = attrappe_lauf(umg3).briefkasten
    bk3.bereite()
    fremd_name = attrappe_name(kennung, "gen", 9999, "gen-fremd", "c" * 32)
    umg3.lege(bk3.antworten / fremd_name, json.dumps(attrappe_umschlag(
        kennung, "gen", 9999, "gen-fremd", "c" * 32, attrappe_snapshot(), attrappe_frame())).encode("utf-8"))
    aus = bk3.auswahl(kennung, {"gen": {4242}})
    fremde = aus.gewertet.get("gen")
    p(fremde is None and aus.fehlt == ["gen"],
      f"fremde Antwort gewertet: gen <- {fremde.name if fremde else None} (PID {fremde.pid if fremde else None}), "
      f"fehlt {aus.fehlt}")
    p(any(n == fremd_name and "PID 9999" in g for n, g in aus.fremd),
      f"Antwort der fremden PID ohne Antwort der Diagnose-PID nicht roh als fremd: {aus.fremd}")
    umg2 = TestUmgebung()
    bk2 = attrappe_lauf(umg2).briefkasten
    anfrage = bk2.frage({"gen": {4242}})
    aus = bk2.sammle(anfrage)
    p(aus.fehlt == ["gen"] and abs(umg2.t - anfrage.geschrieben - ANTWORT_FRIST_S) < 0.05,
      f"Frist: fehlt {aus.fehlt} nach {umg2.t - anfrage.geschrieben:.2f} s")
    p(auswahl_maengel(anfrage) == ["Rolle ohne Antwort nach 10 s: gen"], f"Maengel {auswahl_maengel(anfrage)}")


@fall("M-57", "ohne_brokerprozess_keine_erwartung")
def fall_ohne_brokerprozess_keine_erwartung(p: Pruefer) -> None:
    def vorlage(rolle, _nummer, _umg):
        return None if rolle == "broker" else {"snapshot": attrappe_snapshot(), "frame": attrappe_frame()}

    szenario = {"id": "broker", "schritte": [{"aktion": "lokal.briefkasten", "params": {"rollen": ["gen", "probeeq", "broker"]}}]}
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg)
    umg.instanzen = Skriptinstanzen(vorlage)
    code, roh, _ = fahre_attrappe(umg, szenario)
    p(code == EXIT_OK, f"ohne Brokerprozess: Exit {code} (die Rolle broker darf nicht erwartet werden)")
    p("Broker laeuft nicht" in roh, "ohne Brokerprozess: Rohzeile 'Broker laeuft nicht' fehlt")
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg)
    umg.broker = [7777]
    umg.instanzen = Skriptinstanzen(vorlage)
    code, roh, _ = fahre_attrappe(umg, szenario)
    p(code == EXIT_VERFEHLT and "Rolle ohne Antwort nach 10 s: broker" in roh, f"mit Brokerprozess ohne Antwort: Exit {code}")
    p("PID [7777]" in roh, "mit Brokerprozess: PID nicht roh")


BEREITSCHAFT_13_09 = [  # docs/beweise/roh/NAK-283-laufzeit-7ad6b6f2.md:53-66
    ("system.ping", {"api_version": 45, "controller": "FLStudioMCP", "fl_version": "Producer Edition v26.1.4 [build 5589]", "is_playing": False, "program_title": "FL Studio 2026", "success": True}),
    ("plugins.getName", {"name": "EQ-Copilot", "success": True}),
    ("plugins.getName", {"name": "Nakama Probeeq", "success": True}),
    ("channels.getInfo", {"color": "-0x7f808a", "index": 0, "is_muted": False, "is_selected": True, "is_solo": False, "name": "Testtrack", "pan": 0.0, "pitch": 0.0, "success": True, "target_fx_track": 1, "volume": 0.78125}),
    ("transport.getLength", {"bars": 27, "milliseconds": 45696, "seconds": 46, "success": True, "ticks": 10236}),
    ("transport.setLoopMode", {"mode": "song", "success": True}),
    ("transport.stop", {"stopped": True, "success": True}),
    ("transport.setPosition", {"position": "1:01:00", "success": True}),
    ("transport.start", {"is_playing": True, "success": True}),
    ("mixer.getPeaks", {"name": "Insert 1", "peak_left": 0.4949530065059662, "peak_max": 0.4949530065059662, "peak_right": 0.4754391014575958, "success": True, "track": 1}),
    ("mixer.getPeaks", {"name": "Master", "peak_left": 0.4949530065059662, "peak_max": 0.4949530065059662, "peak_right": 0.4754391014575958, "success": True, "track": 0}),
    ("transport.getStatus", {"is_playing": True, "is_recording": False, "loop_mode": "song", "position": "3:15:20", "success": True}),
    ("transport.stop", {"stopped": True, "success": True}),
]


@fall("M-58", "erste_fassung_unveraendert")
def fall_erste_fassung_unveraendert(p: Pruefer) -> None:
    szenario = json.loads((REPO / "docs" / "gesundheit" / "szenarien" / "bereitschaft.json").read_text(encoding="utf-8"))
    antworten: dict[str, list[dict]] = {}
    for aktion, antwort in BEREITSCHAFT_13_09:
        antworten.setdefault(aktion, []).append(antwort)
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg, antworten=antworten)
    code, roh, _ = fahre_attrappe(umg, szenario)
    p(code == EXIT_OK and "**Ergebnis:** 14 von 14 Schritten bestanden." in roh,
      f"bereitschaft.json gegen die Antworten vom 13.09.2026: Exit {code}, " + ("; ".join(z for z in roh.splitlines() if "VERFEHLT" in z) or "ohne 14 von 14"))
    p(sum(1 for z in roh.splitlines() if z.endswith("| ok |")) == 13, "nicht jede der 13 Aktionen ok")


@fall("M-58", "erwartungsarten_und_punktpfade")
def fall_erwartungsarten_und_punktpfade(p: Pruefer) -> None:
    antwort = {"success": True, "routes": [{"dest": 0, "level": 0.8}, {"dest": 2, "level": 0.5}], "wert": 1.5,
               "nan": float("nan"), "leer": None, "liste": [1, 2]}
    fehler, roh = _pruefe({"routes.*.level": {"bereich": [0.0, 1.0], "roh": True}, "routes.0.dest": {"gleich": 0},
                           "wert": {"endlich": True, "in_menge": [1.5, 2.0], "min": 1.0, "max": 2.0},
                           "routes.*.dest": {"teilmenge": [0, 1, 2]}, "leer": {"roh": True}, "liste": {"nicht_leer": True}}, antwort)
    p(not fehler, f"erfuellte Erwartungen verfehlt: {fehler}")
    p("routes.*.level=[0.8, 0.5]" in roh and "leer=null" in roh, f"roh schreibt nicht wie gelesen: {roh}")
    fehler, _ = _pruefe({"nan": {"endlich": True}, "routes.1.level": {"bereich": [0.6, 1.0]}, "wert": {"in_menge": [2.0]},
                         "routes.*.dest": {"teilmenge": [0, 1]}, "fehlt.tief": {"gleich": 1}}, antwort)
    p(len(fehler) == 5, f"verletzte Erwartungen nicht einzeln gemeldet: {fehler}")


@fall("M-11", "szenario_abschnitte")
def fall_szenario_abschnitte(p: Pruefer) -> None:
    """M-11 an der Zeile: ein Szenario haengt als eigener Abschnitt an (## Szenario), die Einzelheiten einer lokalen
    Aktion als eigener Unterabschnitt (### Schritt)."""
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg)
    umg.instanzen = Skriptinstanzen(lambda rolle, _n, _umg: ({"snapshot": attrappe_snapshot(), "frame": attrappe_frame()}
                                                             if rolle == "gen" else None))
    _code, roh, _lauf = fahre_attrappe(umg, {"id": "abschnitt", "titel": "Attrappe", "schritte": [
        {"aktion": "lokal.briefkasten", "params": {"rollen": ["gen"]}}]})
    zeilen = roh.splitlines()
    p(bool(zeilen) and zeilen[0] == "## Szenario `abschnitt` — Attrappe",
      f"Szenario ohne eigenen Abschnitt: erste Zeile '{zeilen[0] if zeilen else ''}'")
    p("### Schritt 1 `lokal.briefkasten`" in zeilen, "Einzelheiten einer lokalen Aktion ohne eigenen Unterabschnitt")


RES_OFFLINE = (229.8, 354.4, 459.6, 546.6, 688.7, 919.3)
VERGLEICH_TEST = {"baender": [
    {"kurz": "LUFS", "feld": "snapshot.loudness.lufs_integriert", "anker": "schwer", "referenz": "lufs", "toleranz": 0.07,
     "plus_s_lufs": True, "delta_k": True, "rollen": ["gen", "probeeq"], "rechnung": "F-28 Attrappe"},
    {"kurz": "TP", "feld": "snapshot.loudness.true_peak_dbtp", "anker": "leicht", "referenz": "tp_dbtp", "toleranz": 0.12,
     "delta_k": True, "rollen": ["probeeq"], "rechnung": "F-28 Attrappe"},
    {"kurz": "width", "feld": "snapshot.stereo.width", "anker": "schwer", "referenz": "width", "toleranz": 0.01,
     "rollen": ["gen", "probeeq"], "rechnung": "F-28 Attrappe"},
    {"kurz": "resonanzen", "feld": "snapshot.resonanzen.*.freq_hz", "anker": "schwer", "referenz": "resonanzen",
     "art": "teilmenge", "toleranz": 0.1, "rollen": ["gen", "probeeq"], "rechnung": "F-28 Attrappe"}]}


class Umlaufmodell:
    """Antworten passend zur Transportposition der Verbindungsattrappe: Kopfverlust K,
    Gen publiziert schwer 0,55 s aelter als leicht (ein schweres Intervall aelter als
    die Positionsklammer), Probeeq schwer = leicht; nach dem Wrap Anlaufdaten."""

    def __init__(self, v: int = 0, delta_k: float = 0.0, k: int = 1024, aenderung=None, u_unten_zellen: int = 0):
        self.v, self.delta_k, self.k = v, delta_k, k
        self.aenderung = aenderung
        self.u_unten_zellen = u_unten_zellen

    @staticmethod
    def referenz(von: int, bis: int) -> dict:
        return {"lufs": -22.45 + 1.0 * bis / RATE + 0.3 * von / 75600, "tp_dbtp": -7.8,
                "width": 0.33 + 0.1 * bis / RATE, "corr": 0.5, "low_frac": 0.78, "centroid_mag": 829.0,
                "resonanzen": list(RES_OFFLINE), "s_lufs": {"spanne": 0.0}}

    def vorlage(self, rolle: str, nummer: int, umg: TestUmgebung):
        if rolle == "broker":
            return None
        verzoegerung = {"gen": 0.3, "probeeq": 1.3}[rolle]
        ende_ms = umg.conn.position() + verzoegerung * 1000
        if ende_ms >= umg.conn.loop_ms:
            e = round((ende_ms - umg.conn.loop_ms) * RATE / 1000) + self.k
            eintrag = {"snapshot": attrappe_snapshot(zustand="sammelt", aktiv=0.8, gesamt=(e - self.k) / RATE),
                       "frame": attrappe_frame(material_ende_projektsample=e, hostzeit_fortlaufend_samples=e - self.k,
                                               hostzeit_stillstand_bloecke=0, schwer_sekunden=0.0),
                       "verzoegerung": verzoegerung, "nach_wrap": True}
        else:
            e = round(ende_ms * RATE / 1000)
            n_l = e - self.k
            n_s = n_l - round(0.55 * RATE) if rolle == "gen" else n_l
            schwer = self.referenz(max(0, self.k - self.v), self.k + n_s - self.v)
            leicht = self.referenz(max(0, self.k - self.v), self.k + n_l - self.v)
            aktiv = round(min(max(n_l, 0) / RATE * 0.9, 45.0), 1)
            eintrag = {"snapshot": attrappe_snapshot(zustand="messbereit" if aktiv >= 15 else "sammelt", aktiv=aktiv,
                                                     gesamt=n_l / RATE, lufs=schwer["lufs"] + self.delta_k,
                                                     tp=leicht["tp_dbtp"] + self.delta_k, width=schwer["width"],
                                                     resonanzen=(459.6, 919.3)),
                       "frame": attrappe_frame(material_ende_projektsample=e, hostzeit_fortlaufend_samples=n_l,
                                               hostzeit_stillstand_bloecke=0, schwer_sekunden=n_s / RATE),
                       "verzoegerung": verzoegerung}
        if self.aenderung:
            self.aenderung(rolle, nummer, eintrag, umg)
        return eintrag

    def rechnung(self, auftrag: dict) -> dict:
        anker = [{"rolle": a["rolle"], "art": a["art"], "ausschnitt": a["ausschnitt"], "sha256": "0" * 64,
                  **self.referenz(*a["ausschnitt"])} for a in auftrag["anker"]]
        u = [{"rolle": x["rolle"], "s0": x["s0"], "n_l": x["n_l"], "min_zellen": self.u_unten_zellen, "phi_min": 0,
              "zellen_phi0": self.u_unten_zellen, "schwelle_db": -60.0, "delta_db": 4.3e-12} for x in auftrag["u_unten"]]
        return {"anker": anker, "u_unten": u, "dauer_s": 0.0}


def umlauf_attrappe(modell: Umlaufmodell, urteil: str | None = "BITIDENTISCH", v: int = 0, g: float = 1.0,
                    positionen=None, ergebnis: dict | None = None, vergleich: dict | None = None):
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg, positionen=positionen)
    if urteil is not None or ergebnis is not None:
        inhalt = ergebnis or {"format": NULLTEST_FORMAT, "urteil": urteil, "v": v, "g": g}
        umg.lege(Path("C:/attrappe/render/ergebnis.json"), json.dumps(inhalt).encode("utf-8"))
    umg.instanzen = Skriptinstanzen(modell.vorlage)
    umg.rechnung = modell.rechnung
    szenario = {"id": "umlauf", "quelle_sha256": "AB" * 32, "schritte": [
        {"aktion": "transport.getLength"}, {"aktion": "transport.start"},
        {"aktion": "lokal.umlauf", "params": {"rollen": ["gen", "probeeq"], "vergleich": vergleich or VERGLEICH_TEST}}]}
    code, roh, lauf = fahre_attrappe(umg, szenario)
    return code, roh, umg


def gewertete_namen(roh: str) -> dict[str, str]:
    return dict(re.findall(r"- (gen|probeeq): gewertet `([^`]+)`", roh))


@fall("M-67", "snapshot_messpunkt_und_baender")
def fall_snapshot_messpunkt_und_baender(p: Pruefer) -> None:
    code, roh, umg = umlauf_attrappe(Umlaufmodell())
    p(code == EXIT_OK, "Grundlauf (Referenz, Anker n_S, Resonanzen, width): Exit "
                       f"{code}: " + "; ".join(z for z in roh.splitlines() if "VERFEHLT" in z or "VORAUSSETZUNG" in z)[:900])
    namen = gewertete_namen(roh)
    nach_wrap = [n for n, (b, _t) in ((p_.name, v) for p_, v in umg.dateien.items()) if b"\"sammelt\"" in b and b"0.8" in b]
    p(set(namen) == {"gen", "probeeq"} and not set(namen.values()) & set(nach_wrap),
      f"(1) gewertet {namen}, Antworten nach dem Wrap {nach_wrap[:2]}")
    auftrag = umg.rechne_auftraege[-1] if umg.rechne_auftraege else {}
    p(sorted({(a["rolle"], a["art"]) for a in auftrag.get("anker", [])}) == [("gen", "leicht"), ("gen", "schwer"), ("probeeq", "leicht"), ("probeeq", "schwer")],
      f"Rechnungsauftrag ohne die Anker beider Rollen: {auftrag.get('anker')}")

    def u_oben(rolle, _n, eintrag, _umg):
        if rolle == "gen" and eintrag["snapshot"]["zustand"] == "messbereit":
            eintrag["snapshot"]["aktiv_sekunden"] = 55.0

    code, roh, _ = umlauf_attrappe(Umlaufmodell(aenderung=u_oben))
    p(code == EXIT_VERFEHLT and "U_oben" in roh, f"(2) aktiv_sekunden 55,0 (ueber den Wrap integriert): Exit {code}")

    def zu_laut(rolle, _n, eintrag, _umg):
        if not eintrag.get("nach_wrap"):
            eintrag["snapshot"]["loudness"]["lufs_integriert"] += 0.5

    code, roh, _ = umlauf_attrappe(Umlaufmodell(aenderung=zu_laut))
    gruende = re.findall(r"(Referenz oder Toleranz: \w+|Messung: [\w, ]+ \(\w+\)|Vergleichsbasis: [^|;]+)", roh)
    p(code == EXIT_VERFEHLT and "Referenz oder Toleranz: LUFS" in gruende and not any(g.startswith("Messung") for g in gruende),
      f"(7) beide Rollen verfehlen nur LUFS auf derselben Seite: Exit {code}, Gruende {sorted(set(gruende))}")

    code, roh, _ = umlauf_attrappe(Umlaufmodell(v=-75_600), urteil="VERSATZ", v=-75_600)
    p(code == EXIT_OK, f"(6) v = -75 600 mit spaetem Anker: Exit {code} statt GEMESSEN: "
                       + "; ".join(z for z in roh.splitlines() if "VORAUSSETZUNG" in z or "VERFEHLT" in z)[:600])

    stehend = lambda conn: 12_000.0 if conn.spielt else 0.0  # noqa: E731
    code, roh, umg = umlauf_attrappe(Umlaufmodell(), positionen=stehend)
    p(code == EXIT_SZENARIO and "Umlauf ohne Wrap" in roh and len(umg.anfragen) <= 3,
      f"(8) Position steht nach dem Start: Exit {code}, {len(umg.anfragen)} Anfragen")

    k = _phasen_attrappe()
    ergebnis = u_unten(k, 2205, 399 * ZELLE, 0.0, "BITIDENTISCH")
    antwort = Antwort("x", "gen", 4242, 10, {"snapshot": attrappe_snapshot(aktiv=19.9), "frame": attrappe_frame()}, "0" * 32)
    maengel, _roh = vorbedingung(antwort, {"rolle": "gen", **ergebnis}, 2_015_193)
    p(ergebnis["zellen_phi0"] == 199 and ergebnis["max_zellen"] == 398 and ergebnis["min_zellen"] <= 199 and not maengel,
      f"(5) phasenverschobene Attrappe: Z_0 {ergebnis['zellen_phi0']}, max {ergebnis['max_zellen']}, min {ergebnis['min_zellen']}, Vorbedingung {maengel}")


def _phasen_attrappe():
    """400 Quellzellen, abwechselnd Halbzellenenergien (4G, 0) und (0, 4G) mit 2G = 1,5 x Gate."""
    import numpy as np

    halbe = ZELLE // 2
    amplitude = round(math.sqrt(3e-6) * (1 << 23))  # Sampleenergie 4G = 3e-6, Zellmittel 2G = 1,5e-6 > 1e-6
    k = np.zeros((400 * ZELLE, 2), dtype=np.int32)
    for j in range(400):
        anfang = j * ZELLE + (0 if j % 2 == 0 else halbe)
        k[anfang:anfang + halbe, :] = amplitude
    return k


@fall("M-67", "referenzausschnitt_rohzeile")
def fall_referenzausschnitt_rohzeile(p: Pruefer) -> None:
    """F-28 an der Zeile: je Referenzausschnitt eine Rohzeile mit Anfangs- und Endframe, K, v und SHA-256
    (Maschinenartefakt, P-7); K kommt aus dem Anker, v aus dem Nulltesturteil, der SHA-256 aus der Rechnung."""
    code, roh, umg = umlauf_attrappe(Umlaufmodell(v=1), urteil="VERSATZ", v=1)
    auftrag = umg.rechne_auftraege[-1] if umg.rechne_auftraege else {"anker": []}
    p(code == EXIT_OK and len(auftrag["anker"]) == 4, f"Umlauf ohne die vier Anker: Exit {code}, {auftrag['anker']}")
    for anker in auftrag["anker"]:
        von, bis = anker["ausschnitt"]
        soll = f"- Referenzausschnitt {anker['rolle']} {anker['art']}: Frames [{von}, {bis}), K 1024, v 1, SHA-256 {'0' * 64}"
        p(any(z.startswith(soll) for z in roh.splitlines()),
          f"Referenzausschnitt ohne Rohzeile mit Frames, K, v und SHA-256: erwartet '{soll}'")


@fall("M-67", "umlauf_rohzeilen")
def fall_umlauf_rohzeilen(p: Pruefer) -> None:
    """F-28 an der Zeile: lokal.umlauf schreibt je Anfrage eine Zeile mit der Positionsklammer, je gewertete Antwort
    eine Zeile mit dem Anker samt n_L, E, beiden Fortlaufzaehlern und der Hostblocklaenge - auch fuer eine Rolle,
    deren Fortlaufbedingung nicht erfuellt ist (Gen mit einem Block stehender Hostzeit) -, die Rechnung mit ihrer
    Dauer und vollstaendig (hier laenger als 8 000 Zeichen), je Band und Rolle, je Rohfeld und je Rolle U_unten eine
    Zeile und den Rohvergleich Runde 01."""
    def stillstand(rolle, _n, eintrag, _umg):
        if rolle == "gen" and not eintrag.get("nach_wrap"):
            eintrag["frame"]["hostzeit_stillstand_bloecke"] = 1

    modell = Umlaufmodell(aenderung=stillstand)
    basis = modell.rechnung
    modell.rechnung = lambda auftrag: dict(basis(auftrag), notiz="x" * 9000)
    vergleich = dict(VERGLEICH_TEST, roh=["snapshot.spektral.centroid_mag_hz"],
                     rohvergleich_runde01={"lufs_integriert_offline": -22.41})
    _code, roh, _umg = umlauf_attrappe(modell, vergleich=vergleich)
    zeilen = roh.splitlines()
    p(any(re.match(r"^- Rechnung F-28 \(\d+\.\d s\): `", z) for z in zeilen), "Rechnung ohne ihre Dauer in der Rohzeile")
    rechnung: dict = {}
    for z in zeilen:
        treffer = re.match(r"^- Rechnung F-28[^`]*`(.*)`$", z)
        if treffer:
            try:
                rechnung = json.loads(treffer.group(1).replace("\\|", "|"))
            except ValueError:
                rechnung = {}
    p(len(rechnung.get("notiz", "")) == 9000 and len(rechnung.get("anker", [])) == 2,
      f"Rechnung nicht vollstaendig in der Rohzeile: Schluessel {sorted(rechnung)}")
    for band in vergleich["baender"]:
        for rolle in ("gen", "probeeq"):
            kopf = f"- Band {band['kurz']} {rolle}: "
            zeile = next((z for z in zeilen if z.startswith(kopf)), "")
            try:
                werte = json.loads(zeile[len(kopf):]) if zeile else {}
            except ValueError:
                werte = {}
            p("wert" in werte and "status" in werte, f"Band ohne Rohzeile mit Wert und Status: {band['kurz']} {rolle}")
    for rolle in ("gen", "probeeq"):
        p(any(z.startswith(f"- {rolle} roh `snapshot.spektral.centroid_mag_hz` = [") for z in zeilen),
          f"Rohfeld ohne Rohzeile: {rolle} snapshot.spektral.centroid_mag_hz")
        p(any(z.startswith(f"- {rolle}: U_unten ") for z in zeilen), f"U_unten ohne Rohzeile: {rolle}")
    p(any(z.startswith("- Rohvergleich Runde 01 (nur daneben, F-28): {") for z in zeilen), "Rohvergleich Runde 01 ohne Rohzeile")
    folge = re.search(r"^- Ende der Folge: [^;]*; (\d+) Anfragen", roh, re.MULTILINE)
    klammern = re.findall(r"^\| \d+ \| `[0-9a-f]{8}` \| -?\d+ \| -?\d+ \|", roh, re.MULTILINE)
    p(folge is not None and len(klammern) == int(folge.group(1)),
      f"Anfrage ohne Rohzeile mit Positionsklammer: {folge.group(1) if folge else None} Anfragen, {len(klammern)} Zeilen")
    for rolle, stillstand_soll in (("gen", 1), ("probeeq", 0)):
        treffer = re.search(rf"^- {rolle}: gewertet `[^`]+` \(p_vor -?\d+ ms\), Anker (\{{.*\}})$", roh, re.MULTILINE)
        anker = json.loads(treffer.group(1)) if treffer else {}
        p({"n_l", "e", "fortlaufend", "stillstand", "bloecke_max_samples"} <= set(anker) and anker.get("stillstand") == stillstand_soll,
          f"Anker ohne Rohzeile mit n_L, E, Fortlaufzaehlern und Hostblocklaenge: {rolle} {sorted(anker)}")


@fall("M-68", "f28_nur_hergeleitete_baender")
def fall_f28_nur_hergeleitete_baender(p: Pruefer) -> None:
    szenario = json.loads((REPO / "docs" / "gesundheit" / "szenarien" / "snapshot-runde01.json").read_text(encoding="utf-8"))
    umlauf = [s for s in szenario.get("schritte", []) if s.get("aktion") == "lokal.umlauf"]
    baender = umlauf[0]["params"]["vergleich"]["baender"] if umlauf else []
    soll = {"LUFS": (0.07, True, True, ["gen", "probeeq"]), "TP": (0.12, False, True, ["probeeq"]),
            "width": (0.01, False, False, ["gen", "probeeq"]), "corr": (0.01, False, False, ["gen", "probeeq"]),
            "low_frac": (0.02, False, False, ["gen", "probeeq"]), "resonanzen": (0.1, False, False, ["gen", "probeeq"])}
    ist = {b.get("kurz"): (b.get("toleranz"), bool(b.get("plus_s_lufs")), bool(b.get("delta_k")), b.get("rollen")) for b in baender}
    p(ist == soll, f"Baender weichen von F-28 ab: {ist}")
    p(all(str(b.get("rechnung", "")).startswith("F-28") for b in baender), "ein Band ohne Rechnung aus F-28")
    p(not any("centroid" in str(b.get("feld")) for b in baender), "Band fuer centroid_mag_hz (ohne Referenz derselben Definition)")
    p(szenario.get("frischer_start") is True and re.fullmatch(r"[0-9A-F]{64}", str(szenario.get("quelle_sha256"))),
      "frischer_start oder SHA-256 der Quelle fehlt")


@fall("M-66", "kettenverschiebung_aus_nulltest")
def fall_kettenverschiebung_aus_nulltest(p: Pruefer) -> None:
    delta = 20 * math.log10(0.5)
    code, roh, _ = umlauf_attrappe(Umlaufmodell(delta_k=delta), urteil="KETTE", g=0.5)
    p(code == EXIT_OK, f"(1) KETTE g = 0,5 (Delta_K {delta:.2f} dB): Exit {code}: "
                       + "; ".join(z for z in roh.splitlines() if "VERFEHLT" in z)[:600])
    for urteil, v in (("BITIDENTISCH", 0), ("VERSATZ", 1)):
        code, _roh, _ = umlauf_attrappe(Umlaufmodell(v=v), urteil=urteil, v=v)
        p(code == EXIT_OK, f"{urteil}: Exit {code} (Delta_K 0,0 dB)")
    code, roh, _ = umlauf_attrappe(Umlaufmodell(), ergebnis={"format": NULLTEST_FORMAT, "urteil": "FORMATFEHLER", "v": None, "g": None, "grund": "PCM 16"})
    p(code == EXIT_SZENARIO and "ohne Nulltesturteil" in roh, f"(2) Formatfehler: Exit {code} statt 5")
    code, roh, _ = umlauf_attrappe(Umlaufmodell(), urteil=None)
    p(code == EXIT_SZENARIO, f"(2) ohne ergebnis.json: Exit {code} statt 5")


@fall("M-64", "referenzschritte_aus_renderstatus")
def fall_referenzschritte_aus_renderstatus(p: Pruefer) -> None:
    """P-21 (b), (e), (f): lokal.nulltest liest fuer verarbeitung_ein und ohne_slots den Renderstatus des Projekts aus
    params.projekt im Render-Ordner des Runners (referenz\\<Projekt>\\render.json), nicht die Existenz einer Datei im
    Repo-Ordner. Mit Render ruft er nulltest.py mit dem Vergleich und uebernimmt dessen Urteil (gemessen, verfehlt); ohne
    Render endet der Schritt mit Szenario-Exit 5, dem Grund aus dem Renderstatus und einer Rohzeile, nie still (P-3).
    Kein Rueckweg: das Ergebnis steht im Ordner des Projekts, die ergebnis.json des Auslieferungsrenders bleibt (M-65)."""
    render = Path("C:/attrappe/render")
    fixtures = REPO / "eq-copilot" / "fixtures" / "fl"
    namen = {"verarbeitung_ein": "Nakama-Diagnose-Verarbeitung.flp", "ohne_slots": "Nakama-Diagnose-Referenz.flp"}
    befund_gemessen = {"verarbeitung_ein": "ABWEICHUNG", "ohne_slots": "GLEICH"}
    auslieferung = json.dumps({"format": NULLTEST_FORMAT, "urteil": "BITIDENTISCH", "v": 0, "g": 1.0,
                               "sha256_render_bereich": "6F" * 32})
    mit_render = {"format": "nakama.laufzeit.render.v1", "head": "abcdef12", "sha256_projekt": "AB" * 32,
                  "datei": True, "grund": None, "dauer_s": 4.1, "exit": 0}
    fehlt = {"format": "nakama.laufzeit.render.v1", "head": "abcdef12", "sha256_projekt": None, "datei": None,
             "grund": "Referenzprojekt fehlt (Karte U43, K-286-1)"}

    def fahre_referenzen(status: dict | None, urteile: dict | None = None, koeder=("verarbeitung_ein",)):
        umg = TestUmgebung()
        umg.conn = TestVerbindung(umg)
        umg.lege(render / "ergebnis.json", auslieferung.encode("utf-8"))
        for vergleich, name in namen.items():
            if vergleich in koeder:  # der fruehere Existenzweg: ein Projekt im Repo-Ordner (vor P-21 "Weg R2 nicht gebaut")
                umg.lege(fixtures / name, b"FLP-Attrappe im Repo-Ordner")
            if status is not None:
                s = dict(status, projekt=f"C:/attrappe/projekt/{name}")
                if s.get("datei"):
                    s["datei"] = str(render / "referenz" / name / name.replace(".flp", ".wav"))
                umg.lege(render / "referenz" / name / "render.json", json.dumps(s).encode("utf-8"))

        def nulltest(argumente: list[str]) -> tuple[int, str]:
            a = dict(zip(argumente[::2], argumente[1::2]))
            vergleich = a.get("--vergleich")
            code, befund = (urteile or {}).get(vergleich, (EXIT_OK, befund_gemessen.get(vergleich)))
            wort = {EXIT_OK: "GEMESSEN", EXIT_VERFEHLT: "VERFEHLT"}.get(code, "VORAUSSETZUNG")
            ergebnis = {"format": "nakama.laufzeit.nulltest.referenz.v1", "vergleich": vergleich, "urteil": wort,
                        "befund": befund, "exit": code, "v": 0, "abweichungen": 7 if code == EXIT_OK else 0, "g_db": -0.5}
            if a.get("--ergebnis"):
                umg.lege(Path(a["--ergebnis"]), json.dumps(ergebnis).encode("utf-8"))
            return code, f"NULLTEST Vergleich {vergleich} · Urteil {wort} · Exit {code} · Befund {befund}"

        umg.nulltest_antwort = nulltest
        code, roh, _lauf = fahre_attrappe(umg, {"id": "nulltest-host", "schritte": [
            {"aktion": "transport.getLength", "erwarte": {"milliseconds": {"min": 1}}},
            {"aktion": "lokal.nulltest", "params": {"vergleich": "verarbeitung_ein", "projekt": namen["verarbeitung_ein"]}},
            {"aktion": "lokal.nulltest", "params": {"vergleich": "ohne_slots", "projekt": namen["ohne_slots"]}}]})
        zeilen = roh.splitlines()
        return (code, roh, umg, next((z for z in zeilen if z.startswith("| 2 | `lokal.nulltest` |")), ""),
                next((z for z in zeilen if z.startswith("| 3 | `lokal.nulltest` |")), ""))

    # (i) mit Render gemessen - auch ohne Projekt im Repo-Ordner (ohne_slots): der Renderstatus entscheidet
    code, roh, umg, z_ein, z_ohne = fahre_referenzen(mit_render)
    p(code == EXIT_OK and "| GEMESSEN verarbeitung_ein: ABWEICHUNG" in z_ein and z_ein.rstrip().endswith("| ok (Details unten) |"),
      f"verarbeitung_ein mit Renderstatus und Render: Exit {code} statt {EXIT_OK}, Zeile '{z_ein[-220:]}'")
    p("| GEMESSEN ohne_slots: GLEICH" in z_ohne and z_ohne.rstrip().endswith("| ok (Details unten) |"),
      f"ohne_slots mit Renderstatus und Render (kein Projekt im Repo-Ordner): Zeile '{z_ohne[-220:]}'")
    aufrufe = {}
    for argumente in umg.nulltest_aufrufe:
        a = dict(zip(argumente[::2], argumente[1::2]))
        aufrufe[a.get("--vergleich")] = a
    for vergleich, name in namen.items():
        a = aufrufe.get(vergleich) or {}
        p(a.get("--renderstatus") == str(render / "referenz" / name / "render.json"),
          f"{vergleich}: nulltest.py liest den Renderstatus {a.get('--renderstatus')!r} statt den von {name}")
        p(a.get("--ergebnis") == str(render / "referenz" / name / "ergebnis.json"),
          f"{vergleich}: Ergebnis nach {a.get('--ergebnis')!r} statt in den Ordner des Referenzprojekts")
        p(any(z.startswith(f"- Referenzprojekt `{name}`") and "SHA-256 Projekt " + "AB" * 32 in z and "Renderdauer 4.1 s" in z
              for z in roh.splitlines()),
          f"{vergleich}: Rohzeile ohne Projektname, SHA-256 des Projekts und Renderdauer aus dem Renderstatus")
        p(f"NULLTEST Vergleich {vergleich} · Urteil GEMESSEN" in roh, f"{vergleich}: Rohzeile von nulltest.py fehlt in den Details")
    p(bool((aufrufe.get("verarbeitung_ein") or {}).get("--quelle")), "verarbeitung_ein: nulltest.py ohne --quelle")
    p((aufrufe.get("ohne_slots") or {}).get("--auslieferung") == str(render / "ergebnis.json"),
      f"ohne_slots: --auslieferung {(aufrufe.get('ohne_slots') or {}).get('--auslieferung')!r} statt der ergebnis.json des "
      "Auslieferungsrenders")
    p(umg.lies_text(render / "ergebnis.json") == auslieferung,
      "ergebnis.json des Auslieferungsrenders durch einen Referenzschritt veraendert (Rueckweg, M-65)")
    # (ii) verfehlt: nulltest.py meldet fuer verarbeitung_ein 0 Abweichungen
    code, _roh, _umg, z_ein, _z_ohne = fahre_referenzen(mit_render, urteile={"verarbeitung_ein": (EXIT_VERFEHLT, "BITIDENTISCH")})
    p(code == EXIT_VERFEHLT and "| VERFEHLT verarbeitung_ein: BITIDENTISCH" in z_ein
      and z_ein.rstrip().endswith("| VERFEHLT (Details unten) |"),
      f"verarbeitung_ein mit 0 Abweichungen: Exit {code} statt {EXIT_VERFEHLT}, Zeile '{z_ein[-220:]}'")
    # (iii) fehlendes Referenzprojekt: Grund aus dem Renderstatus, Szenario-Exit 5 und Rohzeile - auch mit Koeder im Repo-Ordner
    code, roh, umg, z_ein, z_ohne = fahre_referenzen(fehlt)
    p(code == EXIT_SZENARIO, f"fehlendes Referenzprojekt: Exit {code} statt {EXIT_SZENARIO}")
    for vergleich, zeile in (("verarbeitung_ein", z_ein), ("ohne_slots", z_ohne)):
        erwartet = f"| VORAUSSETZUNG {vergleich}: Referenzprojekt fehlt (Karte U43, K-286-1) | VORAUSSETZUNG (Details unten) |"
        p(zeile.rstrip().endswith(erwartet), f"fehlendes Referenzprojekt ({vergleich}): Zeile '{zeile[-220:]}' statt '{erwartet}'")
        status_pfad = render / "referenz" / namen[vergleich] / "render.json"
        p(any(z.startswith(f"- Renderstatus `{status_pfad}`") and "Referenzprojekt fehlt (Karte U43, K-286-1)" in z
              for z in roh.splitlines()),
          f"fehlendes Referenzprojekt ({vergleich}) still: keine Rohzeile mit Renderstatus und Grund")
    p(not umg.nulltest_aufrufe, f"fehlendes Referenzprojekt: nulltest.py trotzdem gerufen ({len(umg.nulltest_aufrufe)}-mal)")
    # (iv) ohne Renderstatus (der Runner hat nicht gerendert): Szenario-Exit 5 mit Grund, nie still
    code, _roh, _umg, z_ein, _z_ohne = fahre_referenzen(None, koeder=())
    p(code == EXIT_SZENARIO and "| VORAUSSETZUNG verarbeitung_ein: kein Renderstatus" in z_ein
      and z_ein.rstrip().endswith("| VORAUSSETZUNG (Details unten) |"),
      f"ohne Renderstatus: Exit {code} statt {EXIT_SZENARIO}, Zeile '{z_ein[-220:]}'")


@fall("M-81", "fortlaufbedingung_teilbloecke")
def fall_fortlaufbedingung_teilbloecke(p: Pruefer) -> None:
    teilblock = Antwort("t", "gen", 4242, 10, {
        "snapshot": attrappe_snapshot(aktiv=15.0, gesamt=512 / RATE),
        "frame": attrappe_frame(material_ende_projektsample=1280, hostzeit_fortlaufend_samples=256,
                                hostzeit_stillstand_bloecke=1, schwer_sekunden=512 / RATE)}, "0" * 32)
    fortlaufend = Antwort("f", "probeeq", 4242, 10, {
        "snapshot": attrappe_snapshot(aktiv=15.0, gesamt=512 / RATE),
        "frame": attrappe_frame(material_ende_projektsample=1536, hostzeit_fortlaufend_samples=512,
                                hostzeit_stillstand_bloecke=0, schwer_sekunden=512 / RATE)}, "0" * 32)
    umg = TestUmgebung()
    lauf = attrappe_lauf(umg, {"quelle_sha256": "AB" * 32})
    kette = {"urteil": "BITIDENTISCH", "v": 0, "g": 1.0, "delta_k_db": 0.0}
    anker = {"gen": anker_rechnen(teilblock, 0, 2_015_193), "probeeq": anker_rechnen(fortlaufend, 0, 2_015_193)}
    auftrag = rechnungsauftrag(lauf, kette, 2_015_193, anker)
    p("roh" in anker["gen"] and "Fortlaufbedingung" in anker["gen"]["roh"] and "256" in anker["gen"]["roh"] and "stillstand_bloecke 1" in anker["gen"]["roh"],
      f"Teilblock (n_L 512, E 1 280, fortlaufend 256, Stillstand 1): Anker {anker['gen']}")
    p(not any(a["rolle"] == "gen" for a in auftrag["anker"] + auftrag["u_unten"]),
      f"Teilblock: Ausschnittdatei oder U_unten angefordert ab K = 768: {[a for a in auftrag['anker'] if a['rolle'] == 'gen']}")
    p(anker["probeeq"].get("k") == 1024 and any(a["rolle"] == "probeeq" for a in auftrag["anker"]),
      f"erfuellte Bedingung: Anker {anker['probeeq']}")
    band = band_pruefen(VERGLEICH_TEST["baender"][0], "gen", teilblock, anker["gen"], {}, kette)
    p(band["status"] == "roh", f"Teilblock: Band {band}")


@fall("M-69", "stellen_aus_energieprofil")
def fall_stellen_aus_energieprofil(p: Pruefer) -> None:
    import numpy as np

    rng = np.random.default_rng(69)
    k = rng.integers(-(1 << 20), 1 << 20, size=(round(45.7 * RATE), 2)).astype(np.int32)
    leise_von, leise_bis = round(10 * TAKT_S * RATE), round(14 * TAKT_S * RATE)
    k[leise_von:leise_bis] //= 1000
    stellen = waehle_stellen(k)
    s1, s2, s3 = stellen
    p(s2["takte"] == "11-14" and s2["von_s"] == round(10 * TAKT_S, 3),
      f"S2 Takte {s2['takte']} ab {s2['von_s']} s statt der leisen Stelle ab Takt 11 ({round(10 * TAKT_S, 3)} s)")
    p((s1["von_s"], s1["bis_s"], s3["von_s"], s3["bis_s"]) == (0.0, 3.692, 42.462, 45.596), "S1 oder S3 nicht nach F-21")
    p(s2["energie"]["mittel_db"] < s1["energie"]["mittel_db"] - 30, f"Energieprofil S2 {s2['energie']} gegen S1 {s1['energie']}")
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg)
    umg.lege(REPO / "eq-copilot" / "kalibration" / "Testtrack.wav", b"fremde Bytes")
    code, roh, _ = fahre_attrappe(umg, {"id": "u40", "quelle_sha256": "AB" * 32, "schritte": [
        {"aktion": "transport.getLength"}, {"aktion": "lokal.stellen", "params": {"stellen": []}}]})
    p(code == EXIT_SZENARIO and "fremde oder fehlende Quelle" in roh, f"fremde Quelle: Exit {code} statt 5")


def rahmen(start, nummer: int, **werte) -> dict:
    frame = attrappe_frame(projekt_sample_start=start, sample_count=4410, spielt=True, frames_gebaut=40 + nummer,
                           summe_fenster_gesamt=12 + 8 * nummer, summe_fenster_aktiv=9 + 5 * nummer,
                           offen_fenster_gesamt=3 - nummer % 3, offen_fenster_aktiv=1)
    frame.update(werte)
    return frame


def stellen_attrappe(frames, v: int = 0, urteil: str | None = "BITIDENTISCH", positionen=None,
                     verzoegerung: dict | None = None, stelle: dict | None = None):
    """frames(rolle, nummer, anfang): anfang ist der Materialanfang eines 4 410-Sample-Rahmens, der zur
    Antwortzeit an der Transportposition der Verbindungsattrappe endet."""
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg, positionen=positionen)
    umg.lege(REPO / "eq-copilot" / "kalibration" / "Testtrack.wav", b"QUELLE-ATTRAPPE")
    if urteil:
        umg.lege(Path("C:/attrappe/render/ergebnis.json"),
                 json.dumps({"format": NULLTEST_FORMAT, "urteil": urteil, "v": v, "g": 1.0}).encode("utf-8"))
    verzoegerung = verzoegerung or {"gen": 0.3, "probeeq": 1.3, "broker": 0.2}

    def vorlage(rolle, nummer, _umg):
        if rolle == "broker":
            return None
        if nummer == 0:
            return {"snapshot": None, "frame": attrappe_frame(projekt_sample_start=900_000, sample_count=4410, spielt=False)}
        def zur_antwortzeit():
            return frames(rolle, nummer, max(0, round(umg.conn.position() * RATE / 1000) - 4410))

        return {"snapshot": None, "frame_zur_antwortzeit": zur_antwortzeit}

    umg.instanzen = Skriptinstanzen(vorlage, verzoegerung=verzoegerung)
    szenario = {"id": "u40", "quelle_sha256": hashlib.sha256(b"QUELLE-ATTRAPPE").hexdigest().upper(), "schritte": [
        {"aktion": "transport.getLength"},
        {"aktion": "lokal.stellen", "params": {"rollen": ["gen", "probeeq"], "stellen": [
            stelle or {"id": "S1", "name": "Intro", "von_s": 0.0, "bis_s": 3.692}]}}]}
    code, roh, _ = fahre_attrappe(umg, szenario)
    return code, roh, umg


def zaehlungen(roh: str, rolle: str, sid: str = "S1") -> list[str]:
    """Zaehlungsspalte der Rohzeilen einer Rolle (die Spalte hinter der Kennung), in der Reihenfolge der Anfragen."""
    return [z.strip() for z in re.findall(rf"^\| {sid} \| {rolle} \|(?: [^|`]*? \|)*? `[0-9a-f]{{8}}` \| ([^|]+) \|", roh,
                                          re.MULTILINE)]


def gezaehlt_je_rolle(roh: str, sid: str = "S1") -> dict[str, int]:
    return {r: sum(1 for z in zaehlungen(roh, r, sid) if z == "gezaehlt") for r in ("gen", "probeeq")}


@fall("M-70", "stelle_materialausschnitt")
def fall_stelle_materialausschnitt(p: Pruefer) -> None:
    code, roh, umg = stellen_attrappe(lambda rolle, n, anfang: rahmen(anfang, n))
    p(code == EXIT_OK and min(gezaehlt_je_rolle(roh).values()) >= 2,
      f"Grundlauf: Exit {code}, gezaehlt {gezaehlt_je_rolle(roh)}")

    def vor_dem_start(rolle, n, anfang):
        return rahmen(900_000 if (rolle, n) == ("gen", 2) else anfang, n)

    _code, roh, _ = stellen_attrappe(vor_dem_start)
    zweite = (zaehlungen(roh, "gen") + ["(keine Zeile)"] * 2)[1]
    p("nicht ganz in" in zweite, f"(1) Rahmen aus der Zeit vor dem Start bei Klammer in der Stelle: Zaehlung '{zweite}'")

    _code, _roh, umg = stellen_attrappe(lambda rolle, n, anfang: rahmen(anfang, n), v=75_600, urteil="VERSATZ")
    gesetzt = [pa for a, pa, _t in umg.conn.aufrufe if a == "transport.setPosition"]
    p(gesetzt[:1] == [{"position": 1714, "mode": 0}], f"(2) v = +75 600: setPosition {gesetzt[:1]} statt 1 714 ms (Stellenanfang plus 1,714286 s)")

    def ohne_start(rolle, n, anfang):
        return rahmen(None if (rolle, n) == ("gen", 1) else anfang, n)

    _code, roh, _ = stellen_attrappe(ohne_start)
    erste = (zaehlungen(roh, "gen") + ["(keine Zeile)"])[0]
    p("null" in erste, f"(3) projekt_sample_start null: Zaehlung '{erste}'")

    stehend = lambda conn: 1_000.0 if conn.spielt else 0.0  # noqa: E731 - steht innerhalb der Stelle
    _code, roh, umg = stellen_attrappe(lambda rolle, n, anfang: rahmen(anfang, n), positionen=stehend)
    p("Transport steht" in roh and len(umg.anfragen) <= 5, f"(4) Position steht nach dem Start: {len(umg.anfragen)} Anfragen")


@fall("M-70", "stelle_zeitplan")
def fall_stelle_zeitplan(p: Pruefer) -> None:
    # Zeitmodell aus den Laufzeitlaeufen vom 15.09.2026: die Planung traegt nie weniger Kombinationen als
    # eine erste Anfrage 0,05 s nach dem Start; S1 (3,692 s) traegt im mittleren Fall bei jeder Phase
    # zwei Rahmen je Rolle; die Phasen aus Lauf 1 (Gen schreibt 0,945 s nach Probeeq) tragen S1 und S3.
    for dauer in (3.134, 3.692):
        for zehntel in (0, 2, 4, 6, 8):
            phasen = {"gen": 100.37 + zehntel / 10, "probeeq": 100.37}
            plan = plane_stelle(phasen, dauer, 100.0)
            if plan is None:
                p(False, f"Stelle {dauer} s, Phasenabstand {zehntel / 10:.1f} s: kein Zeitplan")
                continue
            start, t1, _marge, tragend, alle = plan
            ohne = plan_bewerten(100.6, 100.65, phasen, dauer)
            p(tragend >= ohne[0], f"Stelle {dauer} s, Phasenabstand {zehntel / 10:.1f} s: geplant {tragend} von "
                                  f"{alle} Kombinationen, ohne Planung {ohne[0]}")
            if dauer > 3.5:
                mitte = simuliere_stelle(start, t1, phasen, dauer, 0.05, 0.02)
                gezaehlt = {r: len(stellen_rahmen_in(liste, dauer)) for r, liste in mitte.items()}
                p(min(gezaehlt.values()) >= 2, f"S1, Phasenabstand {zehntel / 10:.1f} s: im mittleren Fall {gezaehlt}")
    gemessen = {"gen": 100.945, "probeeq": 100.0}
    for dauer in (3.134, 3.692):
        start, t1, _marge, tragend, alle = plane_stelle(gemessen, dauer, 99.2)
        mitte = simuliere_stelle(start, t1, gemessen, dauer, 0.05, 0.02)
        gezaehlt = {r: len(stellen_rahmen_in(liste, dauer)) for r, liste in mitte.items()}
        p(min(gezaehlt.values()) >= 2, f"Phasen aus Lauf 1, Stelle {dauer} s: im mittleren Fall {gezaehlt}, "
                                       f"{tragend} von {alle} Kombinationen")
    # Laufzeitlauf 2: Gen schrieb 0,962 s nach Probeeq; gemeinsam traegt S3 nicht jede Kombination,
    # je Rolle allein schon (Grund fuer getrennte Durchlaeufe in messe_stelle).
    lauf2 = {"gen": 100.962, "probeeq": 100.0}
    gemeinsam = plane_stelle(lauf2, 3.134, 99.2)
    einzeln = {r: plane_stelle({r: t}, 3.134, 99.2) for r, t in lauf2.items()}
    p(gemeinsam[3] < gemeinsam[4] and all(e[3] == e[4] for e in einzeln.values()),
      f"S3 mit den Phasen aus Lauf 2: gemeinsam {gemeinsam[3]} von {gemeinsam[4]}, "
      f"einzeln {({r: (e[3], e[4]) for r, e in einzeln.items()})}")
    # Laufzeitlauf 3: waehrend transport.start blockiert, entsteht keine Anfrage. Ohne diesen Ausschluss
    # waehlt der Plan bei den folgenden Phasen die erste Anfrage 0,017 s vor dem Start, also im Fenster
    # (Suche ueber Phasenlagen im Abstand von 0,1 s, 15.09.2026); mit Ausschluss liegt sie ausserhalb.
    fenster_fall = {"gen": 100.0 + 0.4 + 0.013, "probeeq": 100.0 + 0.6 + 0.057}
    start, t1, _marge, tragend, alle = plane_stelle(fenster_fall, 3.134, 99.2, 0.05)
    p(not (start - 0.02 <= t1 < start + 2.0 * 0.05 + 0.05),
      f"Befehlsdauer 0,05 s: erste Anfrage {t1 - start:+.3f} s zum Start (waehrend transport.start blockiert), "
      f"{tragend} von {alle} Kombinationen")


@fall("M-70", "stelle_zwei_durchlaeufe")
def fall_stelle_zwei_durchlaeufe(p: Pruefer) -> None:
    """Traegt der gemeinsame Zeitplan nicht jede Kombination, misst jede Rolle in einem eigenen
    Durchlauf mit eigener Positionierung und erwartet nur sich selbst; beide Rollen zaehlen zwei Rahmen.
    P-16 an der Zeile: jede Rohzeile traegt durchlauf (getrennt hier, gemeinsam in S2) und die Zahl der
    getragenen Kombinationen ihres Durchlaufs."""
    stelle = {"id": "S3", "name": "Ausklang", "von_s": 42.462, "bis_s": 45.596}
    code, roh, umg = stellen_attrappe(lambda rolle, n, anfang: rahmen(anfang, n), stelle=stelle,
                                      verzoegerung={"gen": 0.3, "probeeq": 1.338, "broker": 0.2})
    gesetzt = [pa for a, pa, _t in umg.conn.aufrufe if a == "transport.setPosition"]
    zaehlung = gezaehlt_je_rolle(roh, "S3")
    rohzeilen_tragen_durchlauf(p, roh, "S3", "getrennt")
    p("getrennte Durchlaeufe je Rolle" in roh, "gemeinsamer Zeitplan traegt nicht jede Kombination, trotzdem ein Durchlauf")
    p(gesetzt == [{"position": 42462, "mode": 0}] * 2, f"Positionierungen {gesetzt} statt zweimal 42 462 ms")
    p('erwartet {"gen": [4242]}' in roh and 'erwartet {"probeeq": [4242]}' in roh,
      "ein Durchlauf erwartet mehr als seine Rolle")
    p(code == EXIT_OK and min(zaehlung.values()) >= 2, f"Exit {code}, gezaehlt {zaehlung}")
    stelle = {"id": "S2", "name": "ruhige Passage", "von_s": 3.692, "bis_s": 11.077}
    code, roh, _umg = stellen_attrappe(lambda rolle, n, anfang: rahmen(anfang, n), stelle=stelle)
    rohzeilen_tragen_durchlauf(p, roh, "S2", "gemeinsam")
    p(code == EXIT_OK, f"S2 im gemeinsamen Durchlauf: Exit {code}")


def rohzeilen_tragen_durchlauf(p: Pruefer, roh: str, sid: str, durchlauf: str) -> None:
    """P-16 an der Zeile: jede Rohzeile und die Differenzzeile einer Rolle tragen durchlauf und <getragen>/<alle>
    ihres Durchlaufs; der Zeitplan des Durchlaufs im Begleittext liefert nur den Sollwert."""
    plaene = {name: f"{tragend}/{alle}" for name, tragend, alle in re.findall(
        r"^- Durchlauf ([a-z ]+): Zeitplan erste Anfrage [^,]*, (\d+) von (\d+) Kombinationen", roh, re.MULTILINE)}
    for rolle in ("gen", "probeeq"):
        plan = plaene.get(rolle if durchlauf == "getrennt" else "gen und probeeq")
        soll = f"| {sid} | {rolle} | {durchlauf} | {plan} |"
        zeilen = [z for z in roh.splitlines() if z.startswith(f"| {sid} | {rolle} |")]
        ohne = [z for z in zeilen if not z.startswith(soll)]
        p(plan is not None and bool(zeilen) and not ohne,
          f"Rohzeile ohne durchlauf und kombinationen ihres Durchlaufs: {sid} {rolle} erwartet '{soll}', "
          f"{len(ohne)} von {len(zeilen)} Zeilen ohne, z. B. '{(ohne or zeilen or [''])[0][:60]}'")
        soll_differenz = f'- Stelle {sid} {rolle} (durchlauf {durchlauf}, kombinationen {plan}): {{"antworten": '
        p(any(z.startswith(soll_differenz) for z in roh.splitlines()),
          f"Differenzzeile ohne durchlauf und kombinationen: {sid} {rolle} erwartet '{soll_differenz}'")


@fall("M-71", "roh_schreibt_null_als_null")
def fall_roh_schreibt_null_als_null(p: Pruefer) -> None:
    antwort = Antwort("n", "gen", 4242, 10, {"snapshot": None, "frame": attrappe_frame(
        aktivitaet=None, abdeckung=None, lufs_s=None, projekt_sample_start=0, sample_count=4410, spielt=True)}, "0" * 32)
    zellen = [z.strip() for z in stellen_zeile("S1", "gen", "gemeinsam", "16/16", antwort, "gezaehlt",
                                                "[0, 162817) v = 0").split("|")[1:-1]]
    wert = dict(zip(STELLEN_KOPF, zellen))
    p(len(zellen) == len(STELLEN_KOPF), f"Rohzeile mit {len(zellen)} Zellen statt {len(STELLEN_KOPF)} nach STELLEN_KOPF")
    p(wert["aktivitaet"] == "null" and wert["abdeckung"] == "null" and wert["lufs_s"] == "null",
      f"null nicht als null geschrieben: {wert}")
    p(wert["projekt_sample_start"] == "0" and wert["spielt"] == "true", f"0 oder true nicht roh: {wert}")


@fall("M-71", "deltas_ueber_evidenzabschluesse")
def fall_deltas_ueber_evidenzabschluesse(p: Pruefer) -> None:
    erste = Antwort("a", "gen", 4242, 10, {"snapshot": None, "frame": attrappe_frame(
        summe_fenster_gesamt=12, summe_fenster_aktiv=9, offen_fenster_gesamt=3, offen_fenster_aktiv=2, frames_gebaut=40,
        projekt_sample_start=0, sample_count=4410, spielt=True)}, "0" * 32)
    letzte = Antwort("b", "gen", 4242, 10, {"snapshot": None, "frame": attrappe_frame(
        summe_fenster_gesamt=20, summe_fenster_aktiv=14, offen_fenster_gesamt=1, offen_fenster_aktiv=1, frames_gebaut=52,
        projekt_sample_start=44100, sample_count=4410, spielt=True)}, "0" * 32)
    delta = deltas([erste, letzte])
    p((delta["d_summe_fenster_gesamt"], delta["d_summe_fenster_aktiv"], delta["d_frames_gebaut"]) == (8, 5, 12),
      f"Differenzen nicht aus den kumulativen Zaehlern: {delta}")
    maengel = plausibel([erste, letzte], delta)
    p(not maengel, f"korrekter Bau als unplausibel gemeldet (M-72): {maengel}")


@fall("M-71", "rohzeile_lage_im_host")
def fall_rohzeile_lage_im_host(p: Pruefer) -> None:
    """F-23 und F-22 an der Zeile: jede Rohzeile einer Stelle traegt die Lage der Stelle im Host mit v; ohne
    Nulltesturteil steht die Stelle unverschoben, und jede Rohzeile traegt den Vermerk v unbekannt."""
    ende = round(3.692 * RATE)
    for urteil, v, soll, text in (("VERSATZ", ZELLE, f"[{ZELLE}, {ende + ZELLE}) v = {ZELLE}", "Rohzeile ohne Lage im Host mit v"),
                                  (None, 0, f"[0, {ende}) v unbekannt", "Rohzeile ohne Vermerk v unbekannt")):
        _code, roh, _umg = stellen_attrappe(lambda rolle, n, anfang: rahmen(anfang, n), v=v, urteil=urteil)
        zeilen = [z for z in roh.splitlines() if re.match(r"^\| S1 \| (gen|probeeq) \|", z)]
        ohne = [z for z in zeilen if f"| {soll} |" not in z]
        p(bool(zeilen) and not ohne, f"{text}: erwartet '{soll}', {len(ohne)} von {len(zeilen)} Zeilen ohne")


@fall("M-71", "stelle_klammer_und_differenzen")
def fall_stelle_klammer_und_differenzen(p: Pruefer) -> None:
    """F-23 an der Zeile: jede Rohzeile einer Stelle traegt ihre Positionsklammer, und die Differenzzeile je Stelle
    und Rolle traegt die Zahl der Antworten, die drei Differenzen und beide Ausschnitte."""
    _code, roh, _umg = stellen_attrappe(lambda rolle, n, anfang: rahmen(anfang, n))
    zeilen = [z for z in roh.splitlines() if re.match(r"^\| S1 \| (gen|probeeq) \|", z)]
    ohne = [z for z in zeilen if not re.search(r"\| -?\d+ \| -?\d+ \|$", z)]
    p(bool(zeilen) and not ohne, f"Rohzeile ohne Positionsklammer: {len(ohne)} von {len(zeilen)} Zeilen ohne p_vor und p_nach, "
                                 f"z. B. '{(ohne or [''])[0][-50:]}'")
    for rolle in ("gen", "probeeq"):
        treffer = re.search(rf"^- Stelle S1 {rolle} \([^)]*\): (\{{.*\}})", roh, re.MULTILINE)
        werte = json.loads(treffer.group(1)) if treffer else {}
        soll = {"antworten", "d_frames_gebaut", "d_summe_fenster_aktiv", "d_summe_fenster_gesamt", "erster_ausschnitt",
                "letzter_ausschnitt"}
        p(soll <= set(werte), f"Differenzzeile ohne Zahl der Antworten, Differenzen und Ausschnitte: S1 {rolle} {sorted(werte)}")


@fall("M-72", "u40_ohne_sollwert")
def fall_u40_ohne_sollwert(p: Pruefer) -> None:
    szenario = json.loads((REPO / "docs" / "gesundheit" / "szenarien" / "u40-aktivitaetsgate.json").read_text(encoding="utf-8"))
    felder = [f for s in szenario["schritte"] for f in (s.get("erwarte") or {})]
    p(not any("aktivitaet" in f or "abdeckung" in f for f in felder), f"Erwartung an aktivitaet oder abdeckung: {felder}")
    stellen = [s for s in szenario["schritte"] if s.get("aktion") == "lokal.stellen"]
    p(len(stellen) == 1 and set(stellen[0].get("params", {})) <= {"rollen", "stellen"} and not stellen[0].get("erwarte"),
      "lokal.stellen traegt Erwartungen oder weitere Parameter (Sollwert oder Sichtbarkeitsgrenze)")
    if stellen:
        eintraege = stellen[0]["params"]["stellen"]
        p([s.get("id") for s in eintraege] == ["S1", "S2", "S3"]
          and all(set(s) <= {"id", "name", "von_s", "bis_s", "takte", "energie"} for s in eintraege),
          f"Stellen mit Sollwert oder nicht S1 bis S3: {eintraege}")
    p(re.fullmatch(r"[0-9A-F]{64}", str(szenario.get("quelle_sha256"))) is not None, "SHA-256 der Quelle fehlt")


@fall("M-57", "tasklist_oem_ohne_treffer")
def fall_tasklist_oem_ohne_treffer(p: Pruefer) -> None:
    """Die echte Umgebung liest tasklist. Die Attrappe von subprocess.run bildet nach, was CPython bei
    text=True tut: scheitert das Dekodieren im Lesefaden, kommt stdout als None
    (subprocess.Popen._communicate: stdout = stdout[0] if stdout else None)."""
    ohne = "INFORMATION: Es werden keine Aufgaben mit den angegebenen Kriterien ausgef\u00fchrt.\r\n".encode("cp850")
    ausgaben = {"eqcop-broker.exe": ohne, "FL64.exe": b'"FL64.exe","4242","Console","1","812.345 K"\r\n'}
    echt = subprocess.run

    def attrappe(befehl, **argumente):
        daten = ausgaben[befehl[2].split(" eq ")[1]]
        if argumente.get("text") or argumente.get("encoding"):
            try:
                daten = daten.decode(argumente.get("encoding") or "cp1252")
            except UnicodeDecodeError:
                daten = None
        return subprocess.CompletedProcess(befehl, 0, stdout=daten, stderr=b"")

    subprocess.run = attrappe
    try:
        umg = Umgebung()
        try:
            broker = umg.broker_pids()
        except Exception as e:  # noqa: BLE001 - die Ausnahme ist der Befund
            broker = f"{type(e).__name__}: {e}"
        fl = umg.fl_pids()
    finally:
        subprocess.run = echt
    p(broker == [], f"ohne Brokerprozess (Meldung mit Umlaut in der OEM-Codepage): {broker!r} statt []")
    p(fl == [4242], f"FL64 mit PID 4242: {fl!r}")


@fall("M-68", "low_frac_in_snapshotdefinition")
def fall_low_frac_in_snapshotdefinition(p: Pruefer) -> None:
    """Gegenphasiger Bass loescht sich im Mid-Mix aus, in der Kanalenergie des Snapshots nicht
    (AnalyseEngine.cpp:124-126); die Referenz fuer low_frac folgt der Kanalenergie."""
    import numpy as np

    t = np.arange(4 * 8192) / RATE
    bass = 0.3 * np.sin(2 * np.pi * 110.0 * t)
    hoch = 0.1 * np.sin(2 * np.pi * 3000.0 * t)
    k = np.stack([np.round((bass + hoch) * (1 << 23)), np.round((hoch - bass) * (1 << 23))], axis=1).astype(np.int32)
    wert = low_frac_kanal(k)
    erwartet = 0.3 ** 2 / (0.3 ** 2 + 0.1 ** 2)
    p(wert is not None and abs(wert - erwartet) < 0.01,
      f"gegenphasiger Bass: low_frac_kanal {wert!r} statt rund {erwartet:.3f} (im Mid-Mix rund 0)")
    p(low_frac_kanal((k // 20000).astype(np.int32)) is None, "nur Segmente unter -60 dB: low_frac_kanal nicht None")


@fall("M-68", "rechnung_rohwerte")
def fall_rechnung_rohwerte(p: Pruefer) -> None:
    """F-28 an der Zeile: der Eintrag eines Referenzausschnitts, den die Rechnungszeile schreibt, traegt den Wert
    low_frac von analyze() roh neben der Referenz low_frac_kanal (F-28 (8); Rauschen, drei Segmente ueber dem Gate)
    und die Stichprobenspitze je Kanal der Quelle neben R_TP (True Peak, M-62; Spitzen 0,5 und 0,25 gesetzt)."""
    import numpy as np

    k = np.random.default_rng(28).integers(-(1 << 20), 1 << 20, size=(3 * 8192, 2)).astype(np.int32)
    k[100, 0], k[200, 1] = -(1 << 22), 1 << 21  # Spitzen 0,5 links und 0,25 rechts, Rauschen unter 0,125

    def analyze(_pfad):
        return {"lufs": -20.0, "tp_dbtp": -3.0, "width": 0.4, "corr": 0.5, "low_frac": 0.123, "centroid_mag": 900.0,
                "resonances": []}

    with tempfile.TemporaryDirectory(prefix="nak286-rechnung-", ignore_cleanup_errors=True) as tmp:
        aus = rechne_auftrag({"ordner": tmp, "head": "t", "v": 0, "urteil": "BITIDENTISCH", "delta_k_db": 0.0, "u_unten": [],
                              "anker": [{"rolle": "gen", "art": "leicht", "ausschnitt": [0, 3 * 8192], "s_lufs": False}]},
                             analyze=analyze, quelle_k=k)
    eintrag = (aus.get("anker") or [{}])[0]
    p(eintrag.get("low_frac") == 0.123 and isinstance(eintrag.get("low_frac_kanal"), float),
      f"Referenzausschnitt ohne low_frac von analyze() neben low_frac_kanal: {sorted(eintrag)}")
    p(eintrag.get("stichprobenspitze") == [0.5, 0.25] and eintrag.get("tp_dbtp") == -3.0,
      f"Referenzausschnitt ohne Stichprobenspitze je Kanal neben R_TP: stichprobenspitze {eintrag.get('stichprobenspitze')}, "
      f"tp_dbtp {eintrag.get('tp_dbtp')}")


# ---------------------------------------------------------------- NAK-309 Etappe 3: Antwortzuordnung und Importweg

ECHO_FELDER = {"mixer.getPeaks": "track", "channels.getInfo": "index", "transport.setLoopMode": "mode"}


def _mcp_kennung_nr(n: int) -> str:
    """Die n-te Anforderungskennung der TestUmgebung (ab 1), wie mcp_kennung() sie vergibt."""
    return f"c{n:031x}"


@fall("309/M-41", "antwort_mit_kennung_gemessen")
def fall_antwort_mit_kennung_gemessen(p: Pruefer) -> None:
    """Eine Antwort mit der gesendeten Kennung ist eine Messung; jeder Versuch traegt eine eigene Kennung aus 32
    Hex-Zeichen, und die Rohzeile nennt sie."""
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg)
    code, roh, lauf = fahre_attrappe(umg, {"id": "kennung", "schritte": [
        {"aktion": "system.ping"}, {"aktion": "transport.getStatus"}]})
    gesendet = umg.conn.kennungen
    p(code == EXIT_OK and not lauf.verworfen, f"zugeordnete Antworten: Exit {code}, verworfen {lauf.verworfen}")
    p(len(gesendet) == 2 and len(set(gesendet)) == 2 and all(k and KENNUNG.match(k) for k in gesendet),
      f"je Versuch eine eigene Kennung aus 32 Hex-Zeichen: {gesendet}")
    p(all(f"request_id={k}" in roh for k in gesendet if k), f"die Rohzeile nennt die Kennung nicht: {roh[-300:]}")
    p("**Ergebnis:** 2 von 2 Schritten bestanden." in roh, "Ergebnis nicht 2 von 2")


@fall("309/M-42", "fremde_oder_fehlende_kennung_ungemessen")
def fall_fremde_oder_fehlende_kennung_ungemessen(p: Pruefer) -> None:
    """Eine Antwort mit fremder und eine ohne Kennung (Controller alten Stands) sind keine Messung: der Schritt heisst
    UNGEMESSEN, wird gezaehlt und nie bewertet; das Szenario endet mit Exit 5, nie mit 0. Der Vorrang 3 vor 5 vor 4
    bleibt: ein verfehlter Schritt daneben aendert Exit 5 nicht, ein Ping ohne Kennung ist Exit 3."""
    schritte = [{"aktion": "transport.getStatus", "erwarte": {"is_playing": {"gleich": True}}},
                {"aktion": "mixer.getPeaks", "params": {"track": 1}, "erwarte": {"peak_max": {"min": 0.0}}}]
    vorgaben = {"transport.getStatus": {"success": True, "is_playing": True},
                "mixer.getPeaks": {"success": True, "track": 1, "peak_max": 0.5}}
    for art, erhalten in (("fremd", "f" * 32), ("ohne", "keine")):
        umg = TestUmgebung()
        umg.conn = TestVerbindung(umg, kennung=art, antworten=dict(vorgaben))
        code, roh, lauf = fahre_attrappe(umg, {"id": art, "schritte": schritte})
        p(code == EXIT_SZENARIO, f"{art}: Exit {code} statt {EXIT_SZENARIO}")
        p(len(lauf.verworfen) == 2 and all(v["erhalten"] == (None if art == "ohne" else erhalten) for v in lauf.verworfen),
          f"{art}: verworfen {lauf.verworfen}")
        p(roh.count("UNGEMESSEN (Antwort ohne passende Kennung: gesendet") == 2 and f"erhalten {erhalten})" in roh,
          f"{art}: Rohzeilen ohne UNGEMESSEN mit gesendeter und erhaltener Kennung")
        p("| ok |" not in roh and ("0 von 2 Schritten bestanden. 2 Schritt(e) UNGEMESSEN; 2 MCP-Antwort(en) ohne "
                                   "passende Kennung verworfen") in roh,
          f"{art}: Ergebniszeile zaehlt die verworfenen Antworten nicht")
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg, antworten={"transport.getStatus": {"success": True, "is_playing": False},
                                              "mixer.getPeaks": {"success": True, "track": 1, "peak_max": 0.5}},
                              kennung=lambda aktion, rid, _frueher: rid if aktion == "transport.getStatus" else None)
    code, roh, _lauf = fahre_attrappe(umg, {"id": "vorrang", "schritte": schritte})
    p(code == EXIT_SZENARIO and "VERFEHLT: is_playing" in roh,
      f"verfehlt neben ungemessen: Exit {code} statt {EXIT_SZENARIO} (Vorrang 5 vor 4)")
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg, kennung="ohne")
    code, roh, _lauf = fahre_attrappe(umg, {"id": "ping", "schritte": [
        {"aktion": "system.ping"}, {"aktion": "transport.getStatus"}]})
    p(code == EXIT_VORAUSSETZUNG and "Abbruch: Ping ohne passende Anforderungskennung" in roh
      and umg.conn.zaehle("transport.getStatus") == 0,
      f"Ping ohne Kennung: Exit {code} statt {EXIT_VORAUSSETZUNG}, Aufrufe nach dem Ping "
      f"{umg.conn.zaehle('transport.getStatus')}")
    # Eine lokale Aktion, die intern sendet (wie die Positionsabfrage in lokal.umlauf und lokal.stellen): wurde dabei
    # eine Antwort ohne passende Kennung verworfen, misst der Schritt nichts - auch wenn die Aktion selbst ok meldet.
    def lokal_attrappe(lauf: Lauf, _schritt: dict) -> tuple[int, str, list[str]]:
        antwort = lauf.sende("transport.getPosition")
        return EXIT_OK, f"Position {antwort.get('ms')}", []

    LOKALE_AKTIONEN["lokal.attrappe"] = lokal_attrappe
    try:
        umg = TestUmgebung()
        umg.conn = TestVerbindung(umg, kennung=lambda aktion, rid, _frueher: None if aktion == "transport.getPosition" else rid)
        code, roh, lauf = fahre_attrappe(umg, {"id": "lokal", "schritte": [{"aktion": "lokal.attrappe"}]})
    finally:
        del LOKALE_AKTIONEN["lokal.attrappe"]
    zeile = next((z for z in roh.splitlines() if z.startswith("| 1 | `lokal.attrappe`")), "")
    p(code == EXIT_SZENARIO and zeile.rstrip().endswith("| UNGEMESSEN |") and len(lauf.verworfen) == 1,
      f"lokale Aktion mit verworfener Antwort: Exit {code}, Zeile '{zeile[-200:]}', verworfen {lauf.verworfen}")


@fall("309/M-43", "echo_abweichung_verfehlt")
def fall_echo_abweichung_verfehlt(p: Pruefer) -> None:
    """Kennung passt, das Echo widerspricht: der zweite mixer.getPeaks-Schritt der echten bereitschaft.json (track 0)
    erhaelt track 1 - verfehlt, nicht bestanden."""
    szenario = json.loads((REPO / "docs" / "gesundheit" / "szenarien" / "bereitschaft.json").read_text(encoding="utf-8"))
    antworten: dict[str, list[dict]] = {}
    for aktion, antwort in BEREITSCHAFT_13_09:
        antworten.setdefault(aktion, []).append(dict(antwort))
    antworten["mixer.getPeaks"][1]["track"] = 1
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg, antworten=antworten)
    code, roh, lauf = fahre_attrappe(umg, szenario)
    zeile = next((z for z in roh.splitlines() if z.startswith("| 12 | `mixer.getPeaks`")), "")
    p(code == EXIT_VERFEHLT and "track: erwartet gleich 0, ist 1" in zeile and not lauf.verworfen,
      f"Echo abweichend: Exit {code}, Zeile '{zeile[-220:]}'")


@fall("309/M-44", "spaete_antwort_ohne_kette")
def fall_spaete_antwort_ohne_kette(p: Pruefer) -> None:
    """Schritt A laeuft zweimal ins Timeout (verfehlt, NAK-286 M-78); danach bringt die Verbindung fuer B die
    verspaetete Antwort von A (Kennung des zweiten A-Versuchs), fuer C deren eigene. B wird nie an den Werten von A
    gemessen (UNGEMESSEN, gezaehlt), C ist wieder gemessen, das Szenario endet mit Exit 5. Die Attrappe vergibt die
    Kennungen in der Folge der Versuche: A1, A2, B, C."""
    timeout = {"success": False, "error": "Timeout waiting for FL Studio response after 2.0s."}
    spaet_a = {"success": True, "track": 1, "name": "Insert 1", "peak_max": 0.49, "request_id": _mcp_kennung_nr(2)}
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg, antworten={"mixer.getPeaks": [timeout, timeout, spaet_a],
                                              "transport.getStatus": {"success": True, "is_playing": True}})
    code, roh, lauf = fahre_attrappe(umg, {"id": "kette", "schritte": [
        {"aktion": "mixer.getPeaks", "params": {"track": 1}, "erwarte": {"peak_max": {"min": 0.01}}},
        {"aktion": "mixer.getPeaks", "params": {"track": 0}, "erwarte": {"peak_max": {"min": 0.01}}},
        {"aktion": "transport.getStatus", "erwarte": {"is_playing": {"gleich": True}}}]})
    zeilen = roh.splitlines()
    a, b, c = (next((z for z in zeilen if z.startswith(f"| {n} |")), "") for n in (1, 2, 3))
    p("VERFEHLT" in a and "Timeout" in a, f"A nicht verfehlt nach zwei Timeouts: '{a[-160:]}'")
    p(f"UNGEMESSEN (Antwort ohne passende Kennung: gesendet {_mcp_kennung_nr(3)}, erhalten {_mcp_kennung_nr(2)})" in b
      and "| ok |" not in b, f"B an der verspaeteten Antwort von A gemessen: '{b[-220:]}'")
    p(c.rstrip().endswith("| ok |"), f"C nach der verworfenen Antwort nicht wieder gemessen: '{c[-160:]}'")
    p(code == EXIT_SZENARIO and len(lauf.verworfen) == 1, f"Exit {code} statt {EXIT_SZENARIO}, verworfen {lauf.verworfen}")


@fall("309/M-46", "wiederholung_mit_neuer_kennung")
def fall_wiederholung_mit_neuer_kennung(p: Pruefer) -> None:
    """Die Wiederholung nach einem Timeout traegt eine neue Kennung; kommt bei ihr die verspaetete Antwort des ersten
    Versuchs an, zaehlt sie nicht als ihre Antwort (UNGEMESSEN). Genau zwei Aufrufe, Pause 0,5 s (M-78)."""
    timeout = {"success": False, "error": "Timeout waiting for FL Studio response after 3.0s."}
    spaet = {"success": True, "is_playing": True, "request_id": _mcp_kennung_nr(1)}
    umg = TestUmgebung()
    umg.conn = TestVerbindung(umg, antworten={"transport.getStatus": [timeout, spaet]})
    code, _roh, lauf = fahre_attrappe(umg, {"id": "wdh", "schritte": [
        {"aktion": "transport.getStatus", "frist_s": 3, "erwarte": {"is_playing": {"gleich": True}}}]})
    gesendet = umg.conn.kennungen
    p(len(gesendet) == 2 and gesendet[0] != gesendet[1], f"Wiederholung ohne neue Kennung: {gesendet}")
    p(code == EXIT_SZENARIO and lauf.verworfen == [{"aktion": "transport.getStatus", "gesendet": _mcp_kennung_nr(2),
                                                    "erhalten": _mcp_kennung_nr(1)}],
      f"verspaetete Antwort des ersten Versuchs als Antwort der Wiederholung gewertet: Exit {code}, "
      f"verworfen {lauf.verworfen}")
    p(abs(umg.t - 1000.5) < 1e-9, f"Pause {umg.t - 1000.0:.3f} s statt 0,5 s auf der Uhr-Attrappe")


@fall("309/M-48", "echo_in_jeder_szenariodatei")
def fall_echo_in_jeder_szenariodatei(p: Pruefer) -> None:
    """Jeder Schritt mixer.getPeaks, channels.getInfo und transport.setLoopMode in jeder Szenariodatei fragt das Echo
    ab, das die Aufnahme vom 13.09.2026 traegt (track, index, mode), mit `gleich` auf den gesendeten Wert."""
    ordner = REPO / "docs" / "gesundheit" / "szenarien"
    gezaehlt, fehlt = 0, []
    for datei in sorted(ordner.glob("*.json")):
        szenario = json.loads(datei.read_text(encoding="utf-8"))
        for nr, schritt in enumerate(szenario.get("schritte", []), 1):
            feld = ECHO_FELDER.get(schritt.get("aktion"))
            if feld is None:
                continue
            gezaehlt += 1
            soll = (schritt.get("params") or {}).get(feld)
            regel = (schritt.get("erwarte") or {}).get(feld) or {}
            if "gleich" not in regel or regel["gleich"] != soll:
                fehlt.append(f"{datei.name} Schritt {nr} {schritt['aktion']} ({feld})")
    p(gezaehlt > 0 and not fehlt, f"{gezaehlt} Echo-Schritte, ohne Echo-Erwartung: {fehlt}")


ATTRAPPE_UTILS_INIT = ("from fl_studio_mcp.utils.connection import get_connection\n"
                       "from fl_studio_mcp.utils import fl_trigger\n")
ATTRAPPE_FL_TRIGGER = "from pathlib import Path\nPath({marke!r}).write_text('geladen', encoding='utf-8')\n"
ATTRAPPE_CONNECTION = (
    "{vorspann}"
    "class _Verbindung:\n"
    "    def ensure_connected(self):\n"
    "        pass\n"
    "    def send_command(self, action, params=None, timeout=None, request_id=None):\n"
    "        return {{'success': True, 'request_id': request_id, 'program_title': 'FL Studio 2026',\n"
    "                'fl_version': 'Attrappe'}}\n"
    "    def get_status(self):\n"
    "        return {{'port_name': 'Attrappe'}}\n"
    "_verbindung = _Verbindung()\n"
    "def get_connection():\n"
    "    return _verbindung\n")
ATTRAPPE_FENSTER = (
    "{vorspann}"
    "from pathlib import Path\n"
    "def capture_process_window(pid, target, directory, name, plugin=None):\n"
    "    Path({marke!r}).write_text('erfasst', encoding='utf-8')\n"
    "    return {{'success': True, 'path': str(Path(directory) / (name + '.png')), 'width': 640, 'height': 480,\n"
    "            'sha256': '0' * 64, 'uniform': False, 'window_class': 'TFruityLoopsMainForm',\n"
    "            'window_title': 'Attrappe'}}\n")


def attrappe_mcp_paket(wurzel: Path, *, trigger_in_connection: bool = False, trigger_in_fenster: bool = False) -> dict:
    """Ein Paket fl_studio_mcp wie im Nachbarrepo: utils/__init__.py laedt connection und fl_trigger, fl_trigger
    schreibt beim Import eine Marke. Wahlweise laedt schon connection.py oder fenster.py fl_trigger (Gegenteil)."""
    utils = wurzel / "fl_studio_mcp" / "utils"
    utils.mkdir(parents=True)
    marken = {"trigger": wurzel / "fl_trigger.geladen", "erfasst": wurzel / "fenster.erfasst"}
    vorspann = "from fl_studio_mcp.utils import fl_trigger\n"
    (wurzel / "fl_studio_mcp" / "__init__.py").write_text("", encoding="utf-8")
    (utils / "__init__.py").write_text(ATTRAPPE_UTILS_INIT, encoding="utf-8")
    (utils / "fl_trigger.py").write_text(ATTRAPPE_FL_TRIGGER.format(marke=str(marken["trigger"])), encoding="utf-8")
    (utils / "connection.py").write_text(ATTRAPPE_CONNECTION.format(vorspann=vorspann if trigger_in_connection else ""),
                                         encoding="utf-8")
    (utils / "fenster.py").write_text(ATTRAPPE_FENSTER.format(vorspann=vorspann if trigger_in_fenster else "",
                                                              marke=str(marken["erfasst"])), encoding="utf-8")
    return marken


def unterprozess_mit_attrappe(befehl: list[str], wurzel: Path) -> tuple[int, str, str]:
    """Unterprozess unter diesem Python mit dem Attrappenpaket vorn im Suchpfad (A35 bleibt ohne MCP-Repo)."""
    umgebung = dict(os.environ, PYTHONPATH=str(wurzel), PYTHONIOENCODING="utf-8")
    r = subprocess.run(befehl, capture_output=True, text=True, encoding="utf-8", errors="replace", timeout=120,
                       env=umgebung)
    return r.returncode, r.stdout or "", r.stderr or ""


def letzte_json_zeile(text: str) -> dict:
    zeile = next((z for z in reversed(text.splitlines()) if z.startswith("{")), None)
    try:
        wert = json.loads(zeile) if zeile else {}
    except ValueError:
        return {}
    return wert if isinstance(wert, dict) else {}


@fall("309/M-50", "importgraph_ohne_fl_trigger")
def fall_importgraph_ohne_fl_trigger(p: Pruefer) -> None:
    """Der echte Importweg der Szenarien (Umgebung.verbindung ueber `szenario.py --ping`) in einem Unterprozess unter
    diesem Python mit einem Attrappenpaket fl_studio_mcp, dessen utils/__init__.py fl_trigger laedt: der
    Stellvertreter haelt __init__.py draussen, fl_trigger wird nie geladen, und die Ping-Zeile nennt die geladenen
    fl_studio_mcp-Module. Gegenteil: laedt connection.py selbst fl_trigger, endet der Ping mit Exit 3 und nennt das
    Modul."""
    befehl = [sys.executable, str(Path(__file__).resolve()), "--ping"]
    with tempfile.TemporaryDirectory(prefix="nak309-import-", ignore_cleanup_errors=True) as tmp:
        marken = attrappe_mcp_paket(Path(tmp))
        code, aus, fehler = unterprozess_mit_attrappe(befehl, Path(tmp))
        antwort = letzte_json_zeile(aus)
        module = antwort.get("fl_studio_mcp_module") or []
        p(code == EXIT_OK and antwort.get("success") is True,
          f"Ping ueber das Attrappenpaket: Exit {code}, Antwort {antwort}, stderr {fehler[-300:]}")
        p("fl_studio_mcp.utils.connection" in module and not any("fl_trigger" in m for m in module),
          f"Importgraph der Ping-Zeile: {module}")
        p(not marken["trigger"].exists(), "fl_trigger wurde geladen (utils/__init__.py lief)")
    with tempfile.TemporaryDirectory(prefix="nak309-import-", ignore_cleanup_errors=True) as tmp:
        attrappe_mcp_paket(Path(tmp), trigger_in_connection=True)
        code, aus, fehler = unterprozess_mit_attrappe(befehl, Path(tmp))
        antwort = letzte_json_zeile(aus)
        p(code == EXIT_VORAUSSETZUNG and "Importgraph" in str(antwort.get("error"))
          and "fl_studio_mcp.utils.fl_trigger" in str(antwort.get("error")),
          f"Gegenteil (connection.py laedt fl_trigger): Exit {code}, Antwort {antwort}, stderr {fehler[-300:]}")


@fall("309/M-51", "fensterskript_ohne_fl_trigger")
def fall_fensterskript_ohne_fl_trigger(p: Pruefer) -> None:
    """Der zweite Importweg: das Skript der Fenstererfassung (Befehl aus fenster_befehl) laedt fenster ueber denselben
    Stellvertreter - fl_trigger wird nie geladen, die Erfassung laeuft. Gegenteil: laedt fenster.py selbst
    fl_trigger, bricht das Skript vor der Erfassung ab."""
    with tempfile.TemporaryDirectory(prefix="nak309-fenster-", ignore_cleanup_errors=True) as tmp:
        marken = attrappe_mcp_paket(Path(tmp))
        befehl = Umgebung().fenster_befehl(4242, "fl", Path(tmp) / "bilder", "m51", None)
        code, aus, fehler = unterprozess_mit_attrappe(befehl, Path(tmp))
        antwort = letzte_json_zeile(aus)
        p(code == 0 and antwort.get("success") is True and marken["erfasst"].exists(),
          f"Erfassung ueber das Attrappenpaket: Exit {code}, Antwort {antwort}, stderr {fehler[-300:]}")
        p(not marken["trigger"].exists(), "fl_trigger wurde im Erfassungsskript geladen (utils/__init__.py lief)")
    with tempfile.TemporaryDirectory(prefix="nak309-fenster-", ignore_cleanup_errors=True) as tmp:
        marken = attrappe_mcp_paket(Path(tmp), trigger_in_fenster=True)
        befehl = Umgebung().fenster_befehl(4242, "fl", Path(tmp) / "bilder", "m51", None)
        code, aus, fehler = unterprozess_mit_attrappe(befehl, Path(tmp))
        p(code != 0 and not marken["erfasst"].exists() and "Piano-Roll-Weg geladen" in fehler,
          f"Gegenteil (fenster.py laedt fl_trigger): Exit {code}, erfasst {marken['erfasst'].exists()}, "
          f"stderr {fehler[-300:]}")


def selbsttest(nur: list[str]) -> int:
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")  # type: ignore[attr-defined]
    except (AttributeError, ValueError):
        pass
    auswahl = {x.strip() for n in nur for x in n.split(",") if x.strip()}
    rot = gelaufen = 0
    for zeile, name, funktion in FAELLE:
        if auswahl and name not in auswahl and zeile not in auswahl:
            continue
        gelaufen += 1
        pruefer = Pruefer()
        try:
            funktion(pruefer)
        except Exception as e:  # noqa: BLE001 - eine Ausnahme ist ein roter Fall mit Ort
            ort = traceback.extract_tb(e.__traceback__)[-1]
            pruefer.fehler.append(f"Ausnahme {type(e).__name__}: {e} (szenario.py:{ort.lineno})")
        if pruefer.fehler:
            rot += 1
            print(f"[ROT] {zeile} {name}: " + " | ".join(pruefer.fehler))
        else:
            print(f"[ok] {zeile} {name}")
    if gelaufen == 0:
        print(f"SELBSTTEST szenario.py: kein Fall passt zu {sorted(auswahl)}")
        return EXIT_DATEI
    print(f"SELBSTTEST szenario.py: {gelaufen - rot} ok, {rot} rot")
    return EXIT_VERFEHLT if rot else EXIT_OK


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
