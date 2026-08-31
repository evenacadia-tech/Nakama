#!/usr/bin/env python3
"""A14 - Der gemeinsame Kern traegt keine Bundle-Identitaet (S8 / SONDE-007a, §53.4).

DAS GATE (Entwurf §53.4, woertlich): "Der gemeinsame Kern sieht keine
JucePlugin_*-Konstanten; Identitaet kommt ausschliesslich aus
plugin-identities-v1.json ueber die duennen Target-Schichten. Andernfalls
erhalten zwei der drei Bundles die Identitaetskonstanten des dritten."

WAS DIESES BEIN MISST - und warum es die anderen Riegel braucht:

  K1 (plugin/state/NakamaKernRiegel.h, #error im Uebersetzer) und K2/K2b/K2c
  (cmake/NakamaKern.cmake) reden ueber die BAUBESCHREIBUNG: welche Makros
  definiert sind, welche Linkkanten existieren. Beide koennen recht haben,
  waehrend das gebaute Artefakt die Identitaet trotzdem traegt - etwa weil
  ein Wert aus einer generierten Kopfdatei oder einem Stringliteral kommt.

  Dieses Bein misst deshalb das ARTEFAKT, in drei Abschnitten:

  [0] Neubau. Vor der Messung werden Objekte, Tlogs und Lib des
      Kernverzeichnisses geloescht und von MSBuild aus der heutigen
      Projektdatei neu erzeugt (`cmake --build <bau> --config Release
      --target NakamaKern`; ueber ZERO_CHECK laeuft dabei das Configure samt
      K2/K2b/K2c mit). Gemessen wird nur, was gerade entstanden ist - eine
      "veraltete Lib" gibt es damit nicht mehr zu erkennen. Drei Belege:
      jede Kernquelle steht in der Bauausgabe, jedes Objekt und die Lib sind
      juenger als der Zeitanker, Objektzahl = Quellzahl.
  [1] Gegenprobe. Derselbe Scanner muss im gebauten Gen-Bundle die Werte
      FINDEN, die dort stehen muessen (hersteller.name, main.produktname,
      main.plugin_code, main.component_cid, main.controller_cid). Ein
      Scanner, der nichts findet, koennte schlicht kaputt sein - sein
      Schweigen ueber den Kern waere dann wertlos.
  [2] Kern. NakamaKern.lib wird byteweise gegen JEDE Nadel aus
      plugin-identities-v1.json durchsucht: Text in ASCII und UTF-16LE,
      Viercodes zusaetzlich als 4-Byte-Integer in beiden Byteordnungen, CIDs
      als 16 rohe Bytes in logischer und COM-vertauschter Ordnung.
  [3] Bauform. Das Archiv enthaelt genau die erwarteten Kernobjekte und kein
      JUCE-Modulobjekt (die Kopf-Fassade in NakamaKern.cmake haelt).

WAS DIESES BEIN NICHT MEHR TUT (NAK-100, 30.08.2026): Bis Runde 19 von S8
trug es zusaetzlich einen Textscan aller Compiler-Eingaben mit
Praeprozessor-Nachbau (K1b), einen Ortsriegel ueber das Leseprotokoll, einen
JUCE-Baum-Riegel und Frischewachen ueber Projektdatei und Tlog - zusammen
rund 3 400 Zeilen, die nicht das Gate massen, sondern Umgehungen des
Pruefskripts durch absichtliche Sabotage in repo-eigenen Quellen. Dieses
Bedrohungsmodell ist nicht das der S8-Riegel; es steht als Haertung im
Register (NAK-103). Die Frischewachen waren seit dem Neubau in Abschnitt [0]
gegenstandslos; der JUCE-Baum wird vom Quellhash-Gate aus S3 (B3) bewacht.

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py [bauverzeichnis]
    py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py --nur-messen
    py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py --selbsttest

Exitcodes: 0 gruen · 2 rot · 3 Voraussetzung fehlt (Neubau nicht moeglich
oder fehlgeschlagen, Artefakte nicht auffindbar, oder --nur-messen: dann ist
ueber die Frische nichts gemessen und nichts behauptet). Ein bereits
registrierter Befund GEWINNT gegen jede fehlende Voraussetzung: war vor dem
Abbruch schon etwas rot, endet der Lauf mit 2 statt 3; die Klartextzeile
"VORAUSSETZUNG: ..." bleibt zusaetzlich stehen. Jeder Voraussetzungs-Rueckweg
geht durch `voraussetzung_exit()`.
"""

from __future__ import annotations

import json
import os
import pathlib
import shutil
import subprocess
import sys
import tempfile
import time

WURZEL = pathlib.Path(__file__).resolve().parents[2]
IDENTITAET = WURZEL / "eq-copilot" / "identity" / "plugin-identities-v1.json"
KERNQUELLEN = WURZEL / "eq-copilot" / "plugin"
KERN_CMAKE = KERNQUELLEN / "CMakeLists.txt"

# Die Objektliste wird von Hand gefuehrt, unabhaengig von NAKAMA_KERN_QUELLEN
# in plugin/CMakeLists.txt: waechst der Kern, spricht dieses Bein beim ersten
# Lauf, statt still durchzurutschen (so gemessen bei S9 und S14-15). Nicht im
# Kern: core/ipc/PipeToken.obj - es traegt den Pipe-Namensraum und damit den
# eingefrorenen Herstellernamen; plugin/CMakeLists.txt sagt, warum.
ERWARTETE_OBJEKTE = {
    "NakamaKanon.obj",
    "NakamaLebenslauf.obj",
    "NakamaParameter.obj",
    "NakamaState.obj",
    "NakamaVertrag.obj",
    "WireEnvelope.obj",
    "IpcVerbindung.obj",
    "BrokerLifecycle.obj",
    "ControlClient.obj",
    "TelemetryClient.obj",
}

# Die Werte, die im gebauten Gen-Bundle stehen MUESSEN (Gegenprobe). Die
# reservierten Sondenwerte stehen dort zu Recht nicht; hersteller.code und
# main.bundle liegen nicht zwingend als Text im Binaerbild.
PFLICHT_GEGENPROBE = ("hersteller.name", "main.produktname", "main.plugin_code",
                      "main.component_cid", "main.controller_cid")

LF = chr(10)
fehler: list[str] = []
ok = 0


# ── Berichtsform ────────────────────────────────────────────────────────────

def pruefe(bedingung: bool, text: str, zusatz: str = "") -> None:
    global ok
    zeile = text + (f"  [{zusatz}]" if zusatz else "")
    if bedingung:
        ok += 1
        print("  ok      " + zeile)
    else:
        fehler.append(zeile)
        print("  FEHLER  " + zeile)


def fehlerbericht() -> None:
    """Die gesammelten Befunde als Klartext - wortgleich an JEDEM Ausgang."""
    print()
    print(f"{ok} ok, {len(fehler)} Fehler")
    if fehler:
        print()
        print("FEHLGESCHLAGEN:")
        for f in fehler:
            print("  - " + f)


def voraussetzung_exit() -> int:
    """Exitcode fuer jeden Ausgang "Voraussetzung fehlt".

    OHNE registrierten Befund 3, MIT registriertem Befund 2, NIE 0: eine
    fehlende Voraussetzung verschweigt nie, was schon gemessen WURDE.
    """
    fehlerbericht()
    if fehler:
        print()
        print("Ein registrierter Befund gewinnt gegen die fehlende Voraussetzung: Exit 2.")
        return 2
    return 3


def _kurz(p: pathlib.Path) -> str:
    try:
        return p.resolve().relative_to(WURZEL).as_posix()
    except (ValueError, OSError):
        return str(p)


# ── [0] Neubau ──────────────────────────────────────────────────────────────

def finde_cmake() -> pathlib.Path | None:
    """Dieselbe Suche wie tools/beweise.ps1 - PATH zuerst, dann VS 2022."""
    aus_pfad = shutil.which("cmake")
    if aus_pfad:
        return pathlib.Path(aus_pfad)
    kandidaten: list[pathlib.Path] = []
    for stamm in (os.environ.get("ProgramFiles(x86)"), os.environ.get("ProgramFiles")):
        if not stamm:
            continue
        for ausgabe in ("BuildTools", "Community", "Professional", "Enterprise"):
            kandidaten.append(pathlib.Path(stamm) / "Microsoft Visual Studio" / "2022"
                              / ausgabe / "Common7" / "IDE" / "CommonExtensions"
                              / "Microsoft" / "CMake" / "CMake" / "bin" / "cmake.exe")
        kandidaten.append(pathlib.Path(stamm) / "CMake" / "bin" / "cmake.exe")
    for kandidat in kandidaten:
        if kandidat.is_file():
            return kandidat
    return None


# GEMESSEN (S8 Runde 5, 29.08.2026): NakamaKern.dir/<konfig>/ enthaelt nicht nur
# Ausgaben. CMake legt dort auch generierte EINGABEN ab (cmake_pch.hxx bei
# target_precompile_headers). Ein pauschales "alles loeschen" machte den Baum
# unbaubar. Geloescht wird nach Endung; Unbekanntes wird benannt, nicht still
# behalten. Und: `--clean-first` waere falsch - es cleant die ganze Solution.
_NEUBAU_AUSGABEN = frozenset({
    ".obj", ".lib", ".pdb", ".idb", ".ilk", ".pch", ".res", ".exp", ".tlog",
    ".lastbuildstate", ".log", ".recipe", ".iobj", ".ipdb", ".metagen",
})
_NEUBAU_EINGABEN = frozenset({
    ".hxx", ".hpp", ".h", ".cxx", ".cpp", ".c", ".rsp", ".txt", ".props", ".rc",
})


def _kernartefakte(bau: pathlib.Path, konfig: str) -> tuple[list[pathlib.Path], list[pathlib.Path]]:
    kerndirs = sorted(p for p in bau.glob("**/NakamaKern.dir/" + konfig) if p.is_dir())
    libs = sorted(p for p in bau.glob("plugin/**/NakamaKern.lib")
                  if p.is_file() and p.parent.name.lower() == konfig.lower())
    return kerndirs, libs


def kern_neubau(bau: pathlib.Path, konfig: str = "Release",
                cmake: pathlib.Path | None = None) -> dict:
    """Loescht die Kernartefakte und laesst MSBuild sie vollstaendig neu erzeugen.

    `ok` ist nur True, wenn wirklich gebaut wurde; jeder andere Ausgang ist
    eine fehlende Voraussetzung. Erst wird geprueft, ob gebaut werden KANN,
    dann erst geloescht - sonst waere der Baum kaputt, um nichts zu messen.
    """
    protokoll = {"ok": False, "grund": "", "ausgabe": "", "sekunden": 0.0,
                 "marke_ns": 0, "geloescht": 0, "befehl": "", "unbekannt": []}
    if cmake is None:
        cmake = finde_cmake()
    if cmake is None:
        protokoll["grund"] = "cmake nicht gefunden - weder im PATH noch unter Visual Studio 2022"
        return protokoll
    if not cmake.is_file():
        protokoll["grund"] = f"cmake nicht ausfuehrbar: {cmake}"
        return protokoll
    if not bau.is_dir():
        protokoll["grund"] = f"{_kurz(bau)} ist kein Verzeichnis - nicht konfiguriert"
        return protokoll

    # Zeitanker auf DEMSELBEN Dateisystem wie die Bauartefakte: eine Datei ist
    # eine Messung, time.time() waere eine Annahme ueber Uhr und Dateisystem.
    marke = bau / ".nakama-neubau-marke"
    try:
        marke.write_bytes(b"")
        protokoll["marke_ns"] = marke.stat().st_mtime_ns
    except OSError as exc:
        protokoll["grund"] = f"Zeitanker im Bauverzeichnis nicht schreibbar: {exc}"
        return protokoll
    finally:
        try:
            marke.unlink()
        except OSError:
            pass

    kerndirs, libs = _kernartefakte(bau, konfig)
    geloescht = 0
    unbekannt: list[str] = []
    try:
        for kerndir in kerndirs:
            for pfad in sorted(kerndir.rglob("*"), reverse=True):
                if not pfad.is_file():
                    continue
                endung = pfad.suffix.lower()
                if pfad.parent.suffix.lower() == ".tlog" or endung in _NEUBAU_AUSGABEN:
                    pfad.unlink()
                    geloescht += 1
                    continue
                if endung not in _NEUBAU_EINGABEN:
                    unbekannt.append(str(pfad.relative_to(kerndir)))
        for lib in libs:
            lib.unlink()
            geloescht += 1
    except OSError as exc:
        protokoll["grund"] = f"Kernartefakt nicht loeschbar: {exc}"
        return protokoll
    protokoll["geloescht"] = geloescht
    protokoll["unbekannt"] = sorted(unbekannt)

    argumente = [str(cmake), "--build", str(bau), "--config", konfig, "--target", "NakamaKern"]
    protokoll["befehl"] = " ".join(argumente)
    beginn = time.perf_counter()
    try:
        lauf = subprocess.run(argumente, capture_output=True, text=True,
                              errors="replace", check=False)
    except OSError as exc:
        protokoll["sekunden"] = time.perf_counter() - beginn
        protokoll["grund"] = f"Neubau nicht startbar: {exc}"
        return protokoll
    protokoll["sekunden"] = time.perf_counter() - beginn
    protokoll["ausgabe"] = (lauf.stdout + lauf.stderr).strip()
    if lauf.returncode != 0:
        protokoll["grund"] = f"Neubau des Kerns fehlgeschlagen (Exit {lauf.returncode})"
        return protokoll
    protokoll["ok"] = True
    return protokoll


def neubau_belegt(protokoll: dict, quellen: list[pathlib.Path],
                  objekte: list[pathlib.Path], lib: pathlib.Path) -> list[str]:
    """Hat der Neubau wirklich jede Kern-TU uebersetzt und neu gelinkt? Fail-closed."""
    klagen: list[str] = []
    marke = protokoll.get("marke_ns", 0)
    ausgabe = protokoll.get("ausgabe", "")
    if protokoll.get("unbekannt"):
        klagen.append("Datei im Kernverzeichnis, die weder Bauausgabe noch bekannte "
                      "Eingabe ist: " + ", ".join(protokoll["unbekannt"]))
    fehlend = sorted(q.name for q in quellen if q.name not in ausgabe)
    if fehlend:
        klagen.append("Bauausgabe nennt diese Uebersetzungseinheiten nicht: " + ", ".join(fehlend))
    if not objekte:
        klagen.append("nach dem Neubau liegt kein einziges Objekt im Kernverzeichnis")
    alt = sorted(o.name for o in objekte if o.stat().st_mtime_ns < marke)
    if alt:
        klagen.append("Objekt aelter als der Neubau (nicht neu uebersetzt): " + ", ".join(alt))
    if len(objekte) != len(quellen):
        klagen.append(f"{len(objekte)} Objekte, aber {len(quellen)} Kernquellen")
    if not lib.is_file():
        klagen.append("NakamaKern.lib fehlt nach dem Neubau")
    elif lib.stat().st_mtime_ns < marke:
        klagen.append("NakamaKern.lib ist aelter als der Neubau - nicht neu gelinkt")
    return klagen


def kernquellen_aus_cmake(cmake_text: str | None = None) -> list[pathlib.Path]:
    """Liest NAKAMA_KERN_QUELLEN aus plugin/CMakeLists.txt (set(...) bis zur Klammer)."""
    text = cmake_text if cmake_text is not None else KERN_CMAKE.read_text(encoding="utf-8", errors="replace")
    zeilen = text.splitlines()
    eintraege: list[str] = []
    im_block = False
    gefunden = False
    for zeile in zeilen:
        inhalt = zeile.split("#", 1)[0]
        if not im_block:
            kompakt = inhalt.replace(" ", "").replace(chr(9), "")
            if kompakt.startswith("set(NAKAMA_KERN_QUELLEN"):
                im_block = True
                gefunden = True
                inhalt = inhalt.split("NAKAMA_KERN_QUELLEN", 1)[1]
            else:
                continue
        if inhalt.strip() == ")":
            break
        if ")" in inhalt:
            inhalt = inhalt.split(")", 1)[0]
            im_block = False
        for token in inhalt.replace(";", " ").split():
            eintraege.append(token.strip('"'))
        if not im_block:
            break
    if not gefunden:
        raise RuntimeError("NAKAMA_KERN_QUELLEN fehlt in plugin/CMakeLists.txt")
    if not eintraege:
        raise RuntimeError("NAKAMA_KERN_QUELLEN ist leer")
    if cmake_text is not None:
        return [KERNQUELLEN / e for e in eintraege]
    quellen = [(KERNQUELLEN / eintrag).resolve() for eintrag in eintraege]
    for quelle in quellen:
        if not quelle.is_relative_to(KERNQUELLEN.resolve()) or not quelle.is_file():
            raise RuntimeError(f"Kernquelle fehlt oder liegt ausserhalb plugin/**: {quelle}")
    return quellen


# ── [1]/[2] Nadeln und Suche ────────────────────────────────────────────────

def nadeln_aus_identitaet(js: dict | None = None) -> dict[str, tuple[str, str]]:
    """Jeder eingefrorene Identitaetswert als benannte Nadel (art, wert).

    Nicht abgeschrieben: was in plugin-identities-v1.json steht, wird gesucht.
    Kommt ein Ziel hinzu, sucht dieses Bein es von selbst mit.
      "text"   - Stringliteral (Produktname, Herstellername, Bundle)
      "fourcc" - Text plus 4-Byte-Integer in beiden Byteordnungen
      "cid"    - 16 rohe Bytes, logisch und COM-vertauscht (die Identitaets-
                 datei nennt die Vertauschung unter "hinweis_zur_byteordnung")
    """
    if js is None:
        js = json.loads(IDENTITAET.read_text(encoding="utf-8"))
    nadeln: dict[str, tuple[str, str]] = {
        "hersteller.name": ("text", js["hersteller"]["name"]),
        "hersteller.code": ("fourcc", js["hersteller"]["code"]),
    }
    for ziel in js["ziele"]:
        kennung = ziel["id"]
        nadeln[f"{kennung}.plugin_code"] = ("fourcc", ziel["plugin_code"])
        for feld in ("produktname", "bundle"):
            if ziel.get(feld):
                nadeln[f"{kennung}.{feld}"] = ("text", ziel[feld])
        for feld in ("component_cid", "controller_cid"):
            if ziel.get(feld):
                nadeln[f"{kennung}.{feld}"] = ("cid", ziel[feld])
    return nadeln


def _com_vertauscht(roh: bytes) -> bytes:
    """TUID-Byteordnung unter COM_COMPATIBLE (VST3-SDK funknownimpl.h, UID::toTUID):
    die ersten acht Bytes als GUID-Gruppen 4/2/2 jeweils umgekehrt, die letzten acht bleiben."""
    return roh[3::-1] + roh[5:3:-1] + roh[7:5:-1] + roh[8:]


def suche(inhalt: bytes, art: str, nadel: str) -> list[str]:
    """Findet die Nadel in allen Formen, in denen sie im Artefakt liegen kann."""
    formen: list[str] = []
    if nadel.encode("ascii", errors="ignore") == nadel.encode("utf-8"):
        if nadel.encode("ascii") in inhalt:
            formen.append("ascii")
    if nadel.encode("utf-16-le") in inhalt:
        formen.append("utf-16le")
    if art == "fourcc":
        roh = nadel.encode("ascii")
        if len(roh) != 4:
            raise ValueError(f"Viercode ist nicht vier ASCII-Bytes lang: {nadel!r}")
        if roh in inhalt:
            formen.append("fourcc-int-be")
        if roh[::-1] in inhalt:
            formen.append("fourcc-int-le")
    if art == "cid":
        roh = bytes.fromhex(nadel)
        if roh in inhalt:
            formen.append("roh16")
        if _com_vertauscht(roh) in inhalt:
            formen.append("roh16-com")
    return formen


def gegenprobe(kontroll_bytes: bytes, nadeln: dict[str, tuple[str, str]]) -> list[tuple[bool, str, str]]:
    """[1] je Pflichtnadel: (gefunden, Text, Formen)."""
    ergebnis = []
    for name in PFLICHT_GEGENPROBE:
        art, wert = nadeln[name]
        formen = suche(kontroll_bytes, art, wert)
        ergebnis.append((bool(formen), f"Gegenprobe findet {name} = {wert!r} im gebauten Bundle",
                         ",".join(formen) if formen else "NICHT GEFUNDEN"))
    return ergebnis


def kernscan(kern_bytes: bytes, nadeln: dict[str, tuple[str, str]]) -> list[tuple[bool, str, str]]:
    """[2] je Nadel: (nicht gefunden, Text, Formen)."""
    ergebnis = []
    for name, (art, wert) in sorted(nadeln.items()):
        formen = suche(kern_bytes, art, wert)
        ergebnis.append((not formen, f"NakamaKern.lib traegt {name} = {wert!r} NICHT",
                         ",".join(formen)))
    return ergebnis


# ── [3] Bauform ─────────────────────────────────────────────────────────────

_ARCH_MAGIE = b"!<arch>" + bytes([10])
_ARCH_KOPFENDE = bytes([0x60, 10])


def archivmitglieder(inhalt: bytes) -> list[str] | None:
    """Mitgliedsnamen eines COFF-Archivs (.lib): 8-Byte-Magie, je Mitglied ein
    60-Byte-Kopf (Name[16] Datum[12] Uid[6] Gid[6] Modus[8] Groesse[10] Ende[2]).
    MSVC legt Linkermitglieder ("/") und optional eine Langnamentabelle ("//")
    voran; lange Namen stehen dann als "/<offset>" im Kopf. None = kein Archiv."""
    if not inhalt.startswith(_ARCH_MAGIE):
        return None
    pos = 8
    langnamen = b""
    namen: list[str] = []
    while pos + 60 <= len(inhalt):
        kopf = inhalt[pos:pos + 60]
        if kopf[58:60] != _ARCH_KOPFENDE:
            break
        roh = kopf[0:16].decode("ascii", errors="replace").strip()
        try:
            groesse = int(kopf[48:58].decode("ascii").strip())
        except ValueError:
            break
        daten_ab = pos + 60
        if roh == "//":
            langnamen = inhalt[daten_ab:daten_ab + groesse]
        elif roh in ("/", ""):
            pass
        elif roh.startswith("/") and roh[1:].isdigit():
            versatz = int(roh[1:])
            ende = langnamen.find(bytes([0]), versatz)
            namen.append(langnamen[versatz:ende].decode("ascii", errors="replace").rstrip("/"))
        else:
            namen.append(roh.rstrip("/"))
        pos = daten_ab + groesse + (groesse % 2)
    return namen


def bauform(mitglieder: list[str] | None) -> list[tuple[bool, str, str]]:
    """[3] Archivmitglieder = erwartete Kernobjekte, kein JUCE-Modulobjekt."""
    if mitglieder is None:
        return [(False, "NakamaKern.lib ist ein lesbares COFF-Archiv", "")]
    namen = {pathlib.PurePath(m).name for m in mitglieder}
    juce = sorted(n for n in namen if n.lower().startswith("juce"))
    return [
        (namen == ERWARTETE_OBJEKTE,
         f"Archivmitglieder sind genau die {len(ERWARTETE_OBJEKTE)} Kernobjekte",
         ", ".join(sorted(namen)) if namen != ERWARTETE_OBJEKTE else ""),
        (not juce, "kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)", ", ".join(juce)),
    ]


def waehle_release(treffer: list[pathlib.Path], was: str) -> pathlib.Path | None:
    """Aus mehreren Baukonfigurationen die Release-Fassung (T2-Befund 23.08.:
    `sorted()[0]` haette die Debug-Lib gemessen, weil Debug vor Release sortiert)."""
    if not treffer:
        return None
    release = [t for t in treffer if "Release" in t.parts]
    if release:
        return release[0]
    if len(treffer) > 1:
        print(f"VORAUSSETZUNG: {was} mehrdeutig, keine Release-Fassung dabei:", file=sys.stderr)
        for t in treffer:
            print(f"  {t}", file=sys.stderr)
        return None
    return treffer[0]


def finde_bundle_binary(bau: pathlib.Path) -> pathlib.Path | None:
    treffer = [t for t in sorted(bau.glob("plugin/EqCopilot_artefacts/**/EQ-Copilot.vst3/**/*.vst3"))
               if t.is_file()]
    return waehle_release(treffer, "EQ-Copilot-Bundle")


# ── Selbsttest: jeder Riegel einmal beim Fallen gesehen ─────────────────────

def _archiv_bauen(mitglieder: list[str]) -> bytes:
    """Ein synthetisches COFF-Archiv mit Langnamentabelle - wie MSVC es schreibt."""
    def kopf(name: str, groesse: int) -> bytes:
        return (name.ljust(16) + "0".ljust(12) + "0".ljust(6) + "0".ljust(6)
                + "0".ljust(8) + str(groesse).ljust(10)).encode("ascii") + _ARCH_KOPFENDE

    def mitglied(name: str, daten: bytes) -> bytes:
        return kopf(name, len(daten)) + daten + (bytes([10]) if len(daten) % 2 else b"")

    langnamen = b""
    versaetze: dict[str, int] = {}
    for m in mitglieder:
        if len(m) + 1 > 16:
            versaetze[m] = len(langnamen)
            langnamen += m.encode("ascii") + b"/" + bytes([0])
    aus = _ARCH_MAGIE + mitglied("/", b"x")
    if langnamen:
        aus += mitglied("//", langnamen)
    for m in mitglieder:
        name = f"/{versaetze[m]}" if m in versaetze else m + "/"
        aus += mitglied(name, b"OBJ")
    return aus


def selbsttest() -> int:
    global ok
    print("A14-Selbsttest: jeder Riegel einmal beim Fallen gesehen (baulos)")
    js = json.loads(IDENTITAET.read_text(encoding="utf-8"))
    nadeln = nadeln_aus_identitaet(js)

    print()
    print("[S1] Nadelformen")
    little = bytes.fromhex("70637145")  # 0x45716370 little-endian
    big = bytes.fromhex("45716370")
    cid_text = "ABCDEF019182FAEB45766E6145716370"
    cid = bytes.fromhex(cid_text)
    pruefe("fourcc-int-le" in suche(little, "fourcc", "Eqcp"), "little-endian Immediate 0x45716370 wird gefunden")
    pruefe("fourcc-int-be" in suche(big, "fourcc", "Eqcp"), "big-endian Integerfolge 0x45716370 wird gefunden")
    pruefe("roh16" in suche(cid, "cid", cid_text), "16-Byte-CID roh wird gefunden")
    pruefe("roh16-com" in suche(_com_vertauscht(cid), "cid", cid_text), "16-Byte-CID COM-vertauscht wird gefunden")
    pruefe("utf-16le" in suche("Nakama".encode("utf-16-le"), "text", "Nakama"), "UTF-16LE-Text wird gefunden")
    pruefe(len(nadeln) >= 7 and "main.plugin_code" in nadeln,
           f"{len(nadeln)} Nadeln aus der Identitaetsdatei, main.plugin_code dabei")

    print()
    print("[S2] Gegenprobe - faellt, sobald eine Pflichtnadel fehlt")
    volle = b"".join(nadeln[n][1].encode("ascii") if nadeln[n][0] != "cid" else bytes.fromhex(nadeln[n][1])
                     for n in PFLICHT_GEGENPROBE)
    pruefe(all(e[0] for e in gegenprobe(volle, nadeln)), "Bundle mit allen Pflichtnadeln: Gegenprobe gruen")
    ohne_cid = b"".join(nadeln[n][1].encode("ascii") for n in PFLICHT_GEGENPROBE if nadeln[n][0] != "cid")
    rot = [e for e in gegenprobe(ohne_cid, nadeln) if not e[0]]
    pruefe(len(rot) == 2 and all("cid" in e[1] for e in rot),
           "Bundle ohne CIDs: genau die zwei CID-Zeilen fallen", "; ".join(e[1] for e in rot))

    print()
    print("[S3] Kernscan - faellt bei jeder Nadelform")
    sauber = b"nichts von belang" * 4
    pruefe(all(e[0] for e in kernscan(sauber, nadeln)), "sauberer Kern: alle Nadeln NICHT gefunden")
    formen_erwartet = {
        "ascii": js["hersteller"]["name"].encode("ascii"),
        "fourcc-int-le": js["ziele"][0]["plugin_code"].encode("ascii")[::-1],
        "roh16-com": _com_vertauscht(bytes.fromhex(js["ziele"][0]["component_cid"])),
    }
    for form, bytesatz in formen_erwartet.items():
        gefallen = [e for e in kernscan(sauber + bytesatz + sauber, nadeln) if not e[0]]
        pruefe(bool(gefallen) and any(form in e[2] for e in gefallen),
               f"Kern mit eingepflanzter Nadel ({form}): Scan faellt und nennt die Form",
               "; ".join(e[2] for e in gefallen))

    print()
    print("[S4] Bauform - Archivleser und Objektliste")
    erwartet = sorted(ERWARTETE_OBJEKTE)
    pruefe(archivmitglieder(_archiv_bauen(erwartet)) == erwartet,
           "synthetisches Archiv mit Kurz- und Langnamen wird vollstaendig gelesen")
    pruefe(all(e[0] for e in bauform(erwartet)), "genau die Kernobjekte: Bauform gruen")
    fremd = bauform(erwartet + ["juce_core.obj"])
    pruefe(not fremd[0][0] and not fremd[1][0] and "juce_core.obj" in fremd[1][2],
           "juce_core.obj im Archiv: beide Bauform-Zeilen fallen und nennen es")
    pruefe(bauform(None)[0][0] is False, "kein Archiv: Bauform faellt")
    pruefe(archivmitglieder(b"kein archiv") is None, "Nicht-Archiv wird als None erkannt")

    print()
    print("[S5] Neubau-Beleg - faellt bei altem Objekt, fehlender Lib, fehlender TU")
    with tempfile.TemporaryDirectory() as tmp:
        d = pathlib.Path(tmp)
        quellen = [d / "A.cpp", d / "B.cpp"]
        for q in quellen:
            q.write_text("", encoding="utf-8")
        objekte = [d / "A.obj", d / "B.obj"]
        for o in objekte:
            o.write_bytes(b"o")
        lib = d / "NakamaKern.lib"
        lib.write_bytes(b"l")
        marke_alt = min(p.stat().st_mtime_ns for p in objekte + [lib]) - 1
        gut = {"marke_ns": marke_alt, "ausgabe": "A.cpp B.cpp", "unbekannt": []}
        pruefe(neubau_belegt(gut, quellen, objekte, lib) == [], "konsistenter Neubau: keine Klage")
        marke_zukunft = max(p.stat().st_mtime_ns for p in objekte + [lib]) + 10**9
        k = neubau_belegt({"marke_ns": marke_zukunft, "ausgabe": "A.cpp B.cpp", "unbekannt": []}, quellen, objekte, lib)
        pruefe(any("aelter" in z for z in k) and any("nicht neu gelinkt" in z for z in k),
               "Objekte und Lib aelter als der Zeitanker: beide Klagen", " | ".join(k))
        k = neubau_belegt({"marke_ns": marke_alt, "ausgabe": "A.cpp", "unbekannt": []}, quellen, objekte, lib)
        pruefe(any("B.cpp" in z for z in k), "Bauausgabe ohne B.cpp: Klage nennt B.cpp", " | ".join(k))
        k = neubau_belegt({"marke_ns": marke_alt, "ausgabe": "A.cpp B.cpp", "unbekannt": ["x.bin"]}, quellen, objekte, lib)
        pruefe(any("x.bin" in z for z in k), "unbekannte Datei im Kernverzeichnis wird benannt", " | ".join(k))
        lib.unlink()
        k = neubau_belegt(gut, quellen, objekte, lib)
        pruefe(any("fehlt" in z for z in k), "fehlende Lib nach Neubau: Klage", " | ".join(k))
        k = neubau_belegt(gut, quellen, objekte[:1], lib)
        pruefe(any("Objekte, aber" in z for z in k), "Objektzahl ungleich Quellzahl: Klage", " | ".join(k))

    print()
    print("[S6] Kernquellen aus CMakeLists, Release-Wahl, Voraussetzungs-Ausgang")
    echt = kernquellen_aus_cmake()
    pruefe(len(echt) == len(ERWARTETE_OBJEKTE) and {q.stem + ".obj" for q in echt} == ERWARTETE_OBJEKTE,
           f"NAKAMA_KERN_QUELLEN ({len(echt)}) und ERWARTETE_OBJEKTE decken sich",
           ", ".join(sorted(q.stem for q in echt)))
    synth = 'set(NAKAMA_KERN_QUELLEN' + LF + '  "a/b.cpp" # Kommentar' + LF + '  c.cpp' + LF + ')' + LF
    pruefe([p.name for p in kernquellen_aus_cmake(synth)] == ["b.cpp", "c.cpp"],
           "set(...)-Block wird ohne Kommentare und Anfuehrungszeichen gelesen")
    leer_faellt = False
    try:
        kernquellen_aus_cmake("set(ANDERES 1)")
    except RuntimeError:
        leer_faellt = True
    pruefe(leer_faellt, "fehlendes NAKAMA_KERN_QUELLEN ist ein Fehler, kein leeres Ergebnis")
    d_ = pathlib.Path("x") / "Debug" / "NakamaKern.lib"
    r_ = pathlib.Path("x") / "Release" / "NakamaKern.lib"
    pruefe(waehle_release([d_, r_], "lib") == r_, "Release gewinnt gegen Debug, unabhaengig von der Sortierung")

    ok_vorher, fehler_vorher = ok, list(fehler)
    with open(os.devnull, "w", encoding="utf-8") as devnull:
        alt_stdout, sys.stdout = sys.stdout, devnull
        try:
            fehler.clear()
            exit_ohne = voraussetzung_exit()
            fehler.append("probe")
            exit_mit = voraussetzung_exit()
        finally:
            sys.stdout = alt_stdout
            fehler.clear()
            fehler.extend(fehler_vorher)
            ok = ok_vorher
    pruefe(exit_ohne == 3 and exit_mit == 2,
           "Voraussetzung fehlt: Exit 3 ohne, Exit 2 mit registriertem Befund")

    fehlerbericht()
    return 2 if fehler else 0


# ── Der Messlauf ────────────────────────────────────────────────────────────

def main() -> int:
    argumente = sys.argv[1:]
    if argumente and argumente[0] == "--selbsttest":
        return selbsttest()
    nur_messen = False
    rest: list[str] = []
    for argument in argumente:
        if argument == "--nur-messen":
            nur_messen = True
        elif argument.startswith("-"):
            print(f"Unbekannte Option: {argument}", file=sys.stderr)
            return voraussetzung_exit()
        else:
            rest.append(argument)
    bau = pathlib.Path(rest[0]) if rest else WURZEL / "eq-copilot" / "build"
    if not bau.is_absolute():
        bau = (WURZEL / bau).resolve()

    neubau: dict = {}
    if nur_messen:
        print("HINWEIS: --nur-messen - es wird NICHT gebaut. Ohne Neubau kein Frische-Urteil;")
        print("         dieser Lauf endet nie mit 0. Identitaetspruefung und Bauform laufen trotzdem.")
    else:
        neubau = kern_neubau(bau)
        if not neubau["ok"]:
            print("VORAUSSETZUNG: " + neubau["grund"], file=sys.stderr)
            for zeile in neubau["ausgabe"].splitlines()[-15:]:
                print("  | " + zeile, file=sys.stderr)
            print("  Ohne Neubau des Kerns wird nichts gemessen und nichts behauptet.", file=sys.stderr)
            return voraussetzung_exit()

    kern_kandidaten = sorted(bau.glob("plugin/**/NakamaKern.lib"))
    if not kern_kandidaten:
        print(f"VORAUSSETZUNG: NakamaKern.lib nicht gefunden unter {bau}.", file=sys.stderr)
        print("  Erst bauen: cmake --build <bau> --config Release --target NakamaKern", file=sys.stderr)
        return voraussetzung_exit()
    kern = waehle_release(kern_kandidaten, "NakamaKern.lib")
    if kern is None:
        return voraussetzung_exit()
    kontrolle = finde_bundle_binary(bau)
    if kontrolle is None:
        print(f"VORAUSSETZUNG: gebautes EQ-Copilot-Bundle nicht gefunden unter {bau}.", file=sys.stderr)
        print("  Ohne Gegenprobe ist ein leeres Suchergebnis im Kern nicht aussagekraeftig.", file=sys.stderr)
        return voraussetzung_exit()

    kern_bytes = kern.read_bytes()
    kontroll_bytes = kontrolle.read_bytes()
    nadeln = nadeln_aus_identitaet()
    print(f"Kern      : {kern.relative_to(WURZEL)}  ({len(kern_bytes)} Byte)")
    print(f"Gegenprobe: {kontrolle.relative_to(WURZEL)}  ({len(kontroll_bytes)} Byte)")
    print(f"Nadeln    : {len(nadeln)} aus {IDENTITAET.relative_to(WURZEL)}")

    print()
    print("[0] Frische - der Kern wurde fuer diese Messung neu gebaut")
    if nur_messen:
        print("  --      nicht gebaut (--nur-messen); ueber die Frische des gemessenen")
        print("          Artefakts behauptet dieser Lauf nichts")
    else:
        try:
            quellen = kernquellen_aus_cmake()
        except RuntimeError as exc:
            quellen = []
            pruefe(False, "NAKAMA_KERN_QUELLEN ist lesbar", str(exc))
        objekte = sorted(p for p in bau.glob("**/NakamaKern.dir/Release/*.obj") if p.is_file())
        klagen = neubau_belegt(neubau, quellen, objekte, kern)
        pruefe(not klagen,
               f"Kernartefakte geloescht und in {neubau['sekunden']:.1f}s neu erzeugt "
               f"({neubau['geloescht']} Dateien entfernt, {len(quellen)} Uebersetzungseinheiten, "
               f"{len(objekte)} Objekte, Lib neu gelinkt)",
               " | ".join(klagen))

    print()
    print("[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?")
    for bedingung, text, zusatz in gegenprobe(kontroll_bytes, nadeln):
        pruefe(bedingung, text, zusatz)

    print()
    print("[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen")
    for bedingung, text, zusatz in kernscan(kern_bytes, nadeln):
        pruefe(bedingung, text, zusatz)

    print()
    print("[3] Bauform - der Kern enthaelt genau seine eigenen Objekte")
    for bedingung, text, zusatz in bauform(archivmitglieder(kern_bytes)):
        pruefe(bedingung, text, zusatz)

    if nur_messen:
        print()
        print("VORAUSSETZUNG: ohne Neubau kein Frische-Urteil (--nur-messen).")
        return voraussetzung_exit()
    fehlerbericht()
    return 2 if fehler else 0


if __name__ == "__main__":
    sys.exit(main())
