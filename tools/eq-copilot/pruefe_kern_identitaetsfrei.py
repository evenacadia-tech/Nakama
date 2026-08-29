#!/usr/bin/env python3
"""Der gemeinsame Kern traegt keine Bundle-Identitaet (S8 / SONDE-007a, §53.4).

WAS HIER GEMESSEN WIRD - und warum es die anderen Riegel braucht:

  K1 (plugin/state/NakamaKernRiegel.h) und K2 (cmake/NakamaKern.cmake) reden
  ueber die BAUBESCHREIBUNG: welche Makros definiert sind, welche Linkkanten
  existieren. Beide koennen recht haben, waehrend das gebaute Artefakt die
  Identitaet trotzdem traegt - etwa weil ein nicht namentlich bekannter Wert
  aus einer generierten Kopfdatei oder einem Stringliteral hereinkommt.

  Dieses Bein misst deshalb die DATEI: es liest NakamaKern.lib byteweise und
  sucht jeden eingefrorenen Identitaetswert aus
  eq-copilot/identity/plugin-identities-v1.json - Text in ASCII/UTF-16LE,
  Viercodes zusaetzlich als 4-Byte-Integer in beiden Byteordnungen und CIDs als
  16 rohe Bytes in logischer und COM-vertauschter Ordnung.

DIE GEGENPROBE IST DER EIGENTLICHE PUNKT (Regel des Hauses: ein Riegel, den
niemand hat fallen sehen, ist keiner). Ein Scanner, der nichts findet, sagt
nichts aus - er koennte schlicht kaputt sein. Darum laeuft derselbe Scanner
mit denselben Nadeln zusaetzlich ueber das GEBAUTE EQ-Copilot-Bundle, wo die
Werte stehen MUESSEN. Findet er sie dort nicht, faellt dieses Bein - sein
Schweigen ueber den Kern waere sonst wertlos.

DRITTE PRUEFUNG - Bauform statt Inhalt: der Kern darf keine JUCE-Modulobjekte
enthalten. Die Kopf-Fassade (cmake/NakamaKern.cmake) haelt die JUCE-.cpp
draussen; das laesst sich am Archiv direkt ablesen, weil eine .lib ihre
Mitgliedsnamen im Klartext fuehrt. Waere die Fassade kaputt, laege hier
juce_core.obj neben NakamaState.obj - und der Kern haette zwei Wahrheiten
ueber denselben Code.

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py [bauverzeichnis]
    py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py --selbsttest

Exitcodes: 0 gruen · 2 rot · 3 Voraussetzung fehlt (nicht gebaut).
"""

from __future__ import annotations

import json
import pathlib
import re
import sys

WURZEL = pathlib.Path(__file__).resolve().parents[2]
IDENTITAET = WURZEL / "eq-copilot" / "identity" / "plugin-identities-v1.json"
KERNQUELLEN = WURZEL / "eq-copilot" / "plugin"
KERN_CMAKE = KERNQUELLEN / "CMakeLists.txt"

# T2-Befund 23.08.: Die Uebersetzung des Kerns haengt nicht nur an seinen
# Quellen, sondern auch an der Baubeschreibung - die Definemenge der
# Kopf-Fassade steht dort. Genau dieser Fall ist im Manifest belegt (B8: "die
# Uebersetzung des Kerns hat sich dadurch geaendert, also waere der fruehere
# Lauf kein Beleg mehr gewesen"), und die erste Fassung der Frischepruefung sah
# ihn nicht: Fassade geaendert, keine .cpp beruehrt, Bein gruen auf alter Lib.
#
# ⚠️ ZWEI Anlaeufe, beide am 23.08., beide gemessen - der erste war falsch:
#
#   Anlauf 1 bewachte per mtime die handgeschriebenen Dateien
#   plugin/CMakeLists.txt und cmake/NakamaKern.cmake. Eine Aenderung am
#   Identitaetsblock von EqCopilot, die den Kern nicht beruehrt, faerbte
#   dieses Bein rot - und es BLIEB rot.
#   Anlauf 2 wollte stattdessen die erzeugte NakamaKern.vcxproj bewachen.
#   Auch falsch, und diesmal aus einem Grund, den nur die Messung zeigt:
#   MSBuild entscheidet ueber .tlog-Dateien, nicht ueber den Zeitstempel der
#   Projektdatei. Ein Bau nach der Aenderung linkte die Lib NICHT neu - der
#   Riegel blieb genauso haengen.
#
# 🔑 Eine mtime-Wache taugt nur an einer Datei, die der Bau auch VERBRAUCHT.
#    Keine der beiden ist das. Statt einen dritten Stellvertreter zu suchen,
#    fragt dieses Bein jetzt die einzige Stelle, die die Wahrheit kennt:
#    MSBuild schreibt in CL.command.1.tlog die vollstaendige Kommandozeile,
#    mit der jede Kernquelle zuletzt uebersetzt WURDE. Dagegen laesst sich die
#    Definemenge halten, die die Projektdatei heute vorschreibt. Weicht sie
#    ab, wurde die Lib mit anderen Schaltern gebaut - genau die Frage aus B8.
#    Und es heilt sich selbst, weil ein Bau die .tlog neu schreibt.


def _defines_aus_vcxproj(datei: pathlib.Path, konfig: str = "Release|x64") -> set[str]:
    text = datei.read_text(encoding="utf-8", errors="replace")
    bedingung = (r"'\$\(Configuration\)\|\$\(Platform\)'=='"
                 + re.escape(konfig) + r"'")
    muster = r'<ItemDefinitionGroup Condition="' + bedingung + r'">(.*?)</ItemDefinitionGroup>'
    gefunden: set[str] = set()
    for gruppe in re.finditer(muster, text, re.S):
        treffer = re.search(r"<PreprocessorDefinitions>(.*?)</PreprocessorDefinitions>",
                            gruppe.group(1), re.S)
        if treffer:
            gefunden.update(_normiere(d) for d in treffer.group(1).split(";")
                             if d and not d.startswith("%"))

    # Visual Studio erzeugt aus CharacterSet weitere compilerwirksame Defines,
    # die nicht in <PreprocessorDefinitions> stehen. Sie pauschal als erlaubte
    # Extras auszunehmen waere wieder ein einseitiger Vergleich: aendert sich
    # CharacterSet, muss genau diese Sollmenge mitwandern.
    eigenschaften = (r'<PropertyGroup Condition="' + bedingung
                     + r'"[^>]*>(.*?)</PropertyGroup>')
    for gruppe in re.finditer(eigenschaften, text, re.S):
        zeichensatz = re.search(r"<CharacterSet>(.*?)</CharacterSet>", gruppe.group(1), re.S)
        if not zeichensatz:
            continue
        wert = zeichensatz.group(1).strip()
        if wert == "MultiByte":
            gefunden.add("_MBCS")
        elif wert == "Unicode":
            gefunden.update({"UNICODE", "_UNICODE"})

    return gefunden


def _defines_aus_tlog(datei: pathlib.Path) -> list[tuple[str, set[str]]]:
    # MSBuild schreibt die .tlog als UTF-16LE mit BOM.
    text = datei.read_bytes().decode("utf-16-le", errors="replace").lstrip("﻿")
    einheiten: list[tuple[str, set[str]]] = []
    quelle = "<unbekannte Kernquelle>"
    for zeile in text.splitlines():
        if not zeile.strip():
            continue
        if zeile.startswith("^"):
            quelle = pathlib.PureWindowsPath(zeile[1:].strip()).name
            continue
        gefunden = {
            _normiere(d)
            for d in re.findall(r'(?<!\S)(?:/D|-D)\s*("(?:[^"\\]|\\.)*"|\S+)', zeile)
        }
        einheiten.append((quelle, gefunden))
    return einheiten


def _normiere(define: str) -> str:
    """`"CMAKE_INTDIR=\\"Release\\""` und `CMAKE_INTDIR="Release"` sind dasselbe."""
    return define.replace("\\", "").replace('"', "").strip()


def _define_abweichungen(soll: set[str], ist: set[str]) -> tuple[list[str], list[str]]:
    """Fehlende und veraltete zusaetzliche Defines, also beide Richtungen."""
    return sorted(soll - ist), sorted(ist - soll)

# Die erwarteten Archivmitglieder bleiben absichtlich unabhaengig von CMake:
# ein neues Kernobjekt soll dieses Bein zum Sprechen bringen, nicht still
# durchrutschen. Die Frische-Abhaengigkeiten darunter werden dagegen aus der
# echten NAKAMA_KERN_QUELLEN-Liste und deren lokalen Includes abgeleitet.
ERWARTETE_OBJEKTE = {
    "NakamaKanon.obj",
    "NakamaLebenslauf.obj",
    "NakamaParameter.obj",
    "NakamaState.obj",
    "NakamaVertrag.obj",
    # S14-15/SONDE-010 (29.08.2026): der v3-IPC-Weg ist Teil des geteilten
    # Bodens. Diese vier Zeilen sind der Grund, warum die Liste von Hand
    # gefuehrt wird - beim ersten Lauf nach dem Zuwachs hat A14 gesprochen,
    # statt still durchzurutschen. Beim selben Lauf fiel ausserdem [2] auf:
    # core/ipc/PipeToken.cpp traegt den Pipe-Namensraum `evenacadia.nakama|v3|`
    # und damit den eingefrorenen HERSTELLERNAMEN. Nicht der Riegel wurde
    # aufgeweicht, sondern die Datei aus dem Kern genommen - sie gehoert in die
    # duenne Zielschicht (plugin/CMakeLists.txt sagt, warum). Deshalb steht
    # hier KEIN PipeToken.obj.
    "WireEnvelope.obj",
    "IpcVerbindung.obj",
    "ControlClient.obj",
    "TelemetryClient.obj",
}

_INCLUDE_ZEILE = re.compile(r'^\s*#\s*include\s+(.+?)\s*$', re.MULTILINE)
_INCLUDE_LITERAL = re.compile(r'^[<"]([^">]+)[">]')


def _kernquellen_aus_cmake() -> list[pathlib.Path]:
    """Liest die tatsaechlichen NAKAMA_KERN_QUELLEN aus plugin/CMakeLists.txt."""
    text = KERN_CMAKE.read_text(encoding="utf-8", errors="replace")
    treffer = re.search(
        r"^\s*set\(\s*NAKAMA_KERN_QUELLEN\b(.*?)^\s*\)", text, re.MULTILINE | re.DOTALL
    )
    if not treffer:
        raise RuntimeError("NAKAMA_KERN_QUELLEN fehlt in plugin/CMakeLists.txt")

    block = re.sub(r"#.*$", "", treffer.group(1), flags=re.MULTILINE)
    eintraege = [t.strip('"') for t in re.findall(r'"[^"]*"|[^\s;]+', block)]
    quellen = [(KERNQUELLEN / eintrag).resolve() for eintrag in eintraege]
    if not quellen:
        raise RuntimeError("NAKAMA_KERN_QUELLEN ist leer")
    for quelle in quellen:
        if not quelle.is_relative_to(KERNQUELLEN.resolve()) or not quelle.is_file():
            raise RuntimeError(f"Kernquelle fehlt oder liegt ausserhalb plugin/**: {quelle}")
    return quellen


def _lokales_include_aufloesen(datei: pathlib.Path, include: str) -> pathlib.Path | None:
    """Loest ein Include so auf, wie die Kern-Includepfade es erlauben."""
    kandidaten = [
        datei.parent / include,
        KERNQUELLEN / include,
        KERNQUELLEN / "state" / include,
        KERNQUELLEN / "vertrag" / include,
    ]
    for kandidat in kandidaten:
        if kandidat.is_file():
            aufgeloest = kandidat.resolve()
            if aufgeloest.is_relative_to(KERNQUELLEN.resolve()):
                return aufgeloest

    # Ein Include kann ueber einen spaeter ergaenzten plugin-Unterordner
    # erreichbar werden. Mehrdeutigkeit ist ROT: dann kennen wir CMakes echte
    # Include-Reihenfolge nicht und duerfen keine Abhaengigkeit erraten.
    name = pathlib.PurePosixPath(include.replace("\\", "/")).name
    treffer = sorted(p.resolve() for p in KERNQUELLEN.rglob(name) if p.is_file())
    if len(treffer) > 1:
        rel = ", ".join(str(p.relative_to(KERNQUELLEN.resolve())) for p in treffer)
        raise RuntimeError(f"lokales Include {include!r} ist mehrdeutig: {rel}")
    return treffer[0] if treffer else None


def kern_quellabhaengigkeiten() -> list[pathlib.Path]:
    """Transitive lokale Include-Huelle aller echten Kern-Uebersetzungseinheiten."""
    offen = list(_kernquellen_aus_cmake())
    gesehen: set[pathlib.Path] = set()

    while offen:
        datei = offen.pop()
        if datei in gesehen:
            continue
        gesehen.add(datei)
        text = datei.read_text(encoding="utf-8", errors="replace")
        for include_zeile in _INCLUDE_ZEILE.findall(text):
            treffer = _INCLUDE_LITERAL.match(include_zeile)
            if treffer is None:
                rel = datei.relative_to(KERNQUELLEN)
                raise RuntimeError(
                    f"nicht literal aufloesbares Include in {rel}: {include_zeile}"
                )
            include = treffer.group(1)
            abhaengigkeit = _lokales_include_aufloesen(datei, include)
            if abhaengigkeit is not None and abhaengigkeit not in gesehen:
                offen.append(abhaengigkeit)

    return sorted(gesehen)

fehler: list[str] = []
ok = 0


def pruefe(bedingung: bool, text: str, zusatz: str = "") -> None:
    global ok
    zeile = text + (f"  [{zusatz}]" if zusatz else "")
    if bedingung:
        ok += 1
        print("  ok      " + zeile)
    else:
        fehler.append(zeile)
        print("  FEHLER  " + zeile)


def nadeln_aus_identitaet() -> dict[str, tuple[str, str]]:
    """Jeder eingefrorene Identitaetswert als benannte Nadel (art, wert).

    Nicht abgeschrieben: was in plugin-identities-v1.json steht, wird gesucht.
    Kommt dort ein viertes Ziel hinzu, sucht dieses Bein es von selbst mit.

    Drei Arten, weil numerische Identitaet nicht nur als Text im Binaerbild liegt:
      "text"   - Stringliteral (Produktname, Herstellername, Bundle)
      "fourcc" - Text plus 4-Byte-Integer in beiden Byteordnungen
      "cid"    - 16 rohe Bytes; die Identitaetsdatei sagt selbst, dass der TUID
                 unter COM_COMPATIBLE in den ersten acht Bytes vertauscht ist
                 ("hinweis_zur_byteordnung"), also werden beide Ordnungen gesucht.
    """
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
    """TUID-Byteordnung unter COM_COMPATIBLE (VST3-SDK funknownimpl.h, UID::toTUID).

    Die ersten acht Bytes stehen als GUID: 4er-Gruppe und zwei 2er-Gruppen
    jeweils in umgekehrter Reihenfolge; die letzten acht bleiben, wie sie sind.
    """
    return roh[3::-1] + roh[5:3:-1] + roh[7:5:-1] + roh[8:]


def suche(inhalt: bytes, art: str, nadel: str) -> list[str]:
    """Findet die Nadel in allen Formen, in denen sie im Artefakt liegen kann."""
    formen = []
    if nadel.encode("ascii", errors="ignore") == nadel.encode("utf-8"):
        if nadel.encode("ascii") in inhalt:
            formen.append("ascii")
    if nadel.encode("utf-16-le") in inhalt:
        formen.append("utf-16le")

    if art == "fourcc":
        roh = nadel.encode("ascii")
        if len(roh) != 4:
            raise ValueError(f"Viercode ist nicht vier ASCII-Bytes lang: {nadel!r}")
        # Die Big-Endian-Folge ist bytegleich mit ASCII, bleibt aber als eigene
        # Form benannt: sie repraesentiert den numerischen Wert 0x45716370.
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


def selbsttest() -> int:
    """Kleine baulose Vorfuehrung der binaeren Nadelkodierungen."""
    print("A14-Selbsttest: Viercode-Integer und CID-Bytefolgen")
    little = bytes.fromhex("70637145")  # 0x45716370 als little-endian Immediate
    big = bytes.fromhex("45716370")
    cid_text = "ABCDEF019182FAEB45766E6145716370"
    cid = bytes.fromhex(cid_text)

    little_formen = suche(little, "fourcc", "Eqcp")
    big_formen = suche(big, "fourcc", "Eqcp")
    cid_formen = suche(cid, "cid", cid_text)
    pruefe("fourcc-int-le" in little_formen,
           "little-endian Immediate 0x45716370 wird gefunden",
           ",".join(little_formen))
    pruefe("fourcc-int-be" in big_formen,
           "big-endian Integerfolge 0x45716370 wird gefunden",
           ",".join(big_formen))
    pruefe("roh16" in cid_formen,
           "16-Byte-CID-Suche bleibt erhalten",
           ",".join(cid_formen))

    fehlend, zusaetzlich = _define_abweichungen(
        {"HEUTE=1", "NUR_HEUTE=1"}, {"HEUTE=1", "NUR_GEBAUT=1"}
    )
    pruefe(fehlend == ["NUR_HEUTE=1"],
           "Frische-Defines erkennen fehlende gebaute Werte",
           ",".join(fehlend))
    pruefe(zusaetzlich == ["NUR_GEBAUT=1"],
           "Frische-Defines erkennen veraltete zusaetzliche Werte",
           ",".join(zusaetzlich))

    abhaengigkeiten = {
        p.relative_to(KERNQUELLEN).as_posix() for p in kern_quellabhaengigkeiten()
    }
    pruefe("vertrag/NakamaUtf8.h" in abhaengigkeiten,
           "rekursive Kern-Includehuelle enthaelt NakamaUtf8.h")

    print(f"\n{ok} ok, {len(fehler)} Fehler")
    return 2 if fehler else 0


def archivmitglieder(inhalt: bytes) -> list[str] | None:
    """Liest die Mitgliedsnamen eines COFF-Archivs (.lib / ar).

    Format: 8-Byte-Magie, dann je Mitglied ein 60-Byte-Kopf
    (Name[16] Datum[12] Uid[6] Gid[6] Modus[8] Groesse[10] Ende[2]).
    MSVC legt zwei Linkermitglieder ("/") und optional eine Langnamentabelle
    ("//") voran; lange Namen stehen dann als "/<offset>" im Kopf.
    Gibt None zurueck, wenn die Datei kein Archiv ist.
    """
    if not inhalt.startswith(b"!<arch>\n"):
        return None

    pos = 8
    langnamen = b""
    namen: list[str] = []

    while pos + 60 <= len(inhalt):
        kopf = inhalt[pos:pos + 60]
        if kopf[58:60] != b"`\n":
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
            pass  # Linkermitglied, kein Objekt
        elif roh.startswith("/") and roh[1:].isdigit():
            versatz = int(roh[1:])
            ende = langnamen.find(b"\0", versatz)
            namen.append(langnamen[versatz:ende].decode("ascii", errors="replace").rstrip("/"))
        else:
            namen.append(roh.rstrip("/"))

        pos = daten_ab + groesse + (groesse % 2)  # Mitglieder sind gerade ausgerichtet

    return namen


def waehle_release(treffer: list[pathlib.Path], was: str) -> pathlib.Path | None:
    """Waehlt aus mehreren Baukonfigurationen die Release-Fassung.

    T2-Befund 23.08.: Bis dahin stand hier `sorted(...)[0]`. Ein
    Mehrkonfigurations-Generator (Visual Studio) legt Debug und Release
    nebeneinander, und "Debug" sortiert VOR "Release" - dieses Bein haette
    dann die Debug-Lib gemessen, waehrend der Runner Release baut und
    beglaubigt, und die Frischepruefung haette die falsche Datei bewacht.
    Ein Bein, das ein Artefakt misst, darf sich das Artefakt nicht per Zufall
    der Sortierreihenfolge aussuchen.
    """
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


def main() -> int:
    if len(sys.argv) == 2 and sys.argv[1] == "--selbsttest":
        return selbsttest()
    if len(sys.argv) > 1 and sys.argv[1].startswith("-"):
        print(f"Unbekannte Option: {sys.argv[1]}", file=sys.stderr)
        return 3

    bau = pathlib.Path(sys.argv[1]) if len(sys.argv) > 1 else WURZEL / "eq-copilot" / "build"
    if not bau.is_absolute():
        bau = (WURZEL / bau).resolve()

    kern_kandidaten = sorted(bau.glob("plugin/**/NakamaKern.lib"))
    if not kern_kandidaten:
        print(f"VORAUSSETZUNG: NakamaKern.lib nicht gefunden unter {bau}\\plugin.", file=sys.stderr)
        print("  Erst bauen: cmake --build <bau> --config Release --target NakamaKern", file=sys.stderr)
        return 3
    kern = waehle_release(kern_kandidaten, "NakamaKern.lib")
    if kern is None:
        return 3

    kontrolle = finde_bundle_binary(bau)
    if kontrolle is None:
        print(f"VORAUSSETZUNG: gebautes EQ-Copilot-Bundle nicht gefunden unter {bau}.", file=sys.stderr)
        print("  Ohne Gegenprobe ist ein leeres Suchergebnis im Kern nicht aussagekraeftig.", file=sys.stderr)
        return 3

    kern_bytes = kern.read_bytes()
    kontroll_bytes = kontrolle.read_bytes()
    nadeln = nadeln_aus_identitaet()

    print(f"Kern      : {kern.relative_to(WURZEL)}  ({len(kern_bytes)} Byte)")
    print(f"Gegenprobe: {kontrolle.relative_to(WURZEL)}  ({len(kontroll_bytes)} Byte)")
    print(f"Nadeln    : {len(nadeln)} aus {IDENTITAET.relative_to(WURZEL)}")

    # ── 0. Misst dieses Bein ueberhaupt den aktuellen Stand? ────────────────
    # Der Baustand-Riegel des Runners (tools/beweise.ps1) laeuft nur ueber die
    # .exe-Beine ('Art -eq plugin'). NakamaKern.lib ist eine neue ART von
    # gemessenem Artefakt und faellt durch dieses Raster: ohne -Bauen koennte
    # dieses Bein eine veraltete Lib messen und gruen melden, waehrend die
    # Quelle laengst etwas anderes sagt. Also bewacht das Bein seine eigene
    # Frische. Die Huelle wird aus NAKAMA_KERN_QUELLEN und allen rekursiven,
    # lokalen Includes unter plugin/** berechnet; eine handgepflegte
    # Headerliste waere selbst wieder eine stille Luecke.
    print("\n[0] Frische - misst dieses Bein den aktuellen Quellstand?")
    lib_zeit = kern.stat().st_mtime
    try:
        bewacht = kern_quellabhaengigkeiten()
    except RuntimeError as exc:
        bewacht = []
        pruefe(False, "lokale Include-Huelle der Kernquellen ist eindeutig ableitbar", str(exc))
    juenger = [q for q in bewacht if q.is_file() and q.stat().st_mtime > lib_zeit]
    pruefe(not juenger,
           "NakamaKern.lib ist nicht aelter als die Kernquellen",
           ", ".join(q.relative_to(KERNQUELLEN).as_posix() for q in juenger) if juenger else "")

    # Zweite Haelfte: mit WELCHEN Schaltern wurde sie gebaut? Siehe Kopf.
    projekt_kandidaten = sorted(bau.glob("**/NakamaKern.vcxproj"))
    tlog_kandidaten = sorted(bau.glob("**/NakamaKern.tlog/CL.command.1.tlog"))
    projekt = (projekt_kandidaten[0] if len(projekt_kandidaten) == 1
               else waehle_release(projekt_kandidaten, "NakamaKern.vcxproj"))
    tlog = waehle_release(tlog_kandidaten, "CL.command.1.tlog")
    if projekt is not None and tlog is not None:
        soll = _defines_aus_vcxproj(projekt)
        ist_je_einheit = _defines_aus_tlog(tlog)
        abweichung = []
        for quelle, ist in ist_je_einheit:
            fehlend, zusaetzlich = _define_abweichungen(soll, ist)
            if fehlend:
                abweichung.append(f"{quelle}: nicht gebaut: " + ", ".join(fehlend))
            if zusaetzlich:
                abweichung.append(f"{quelle}: nur gebaut: " + ", ".join(zusaetzlich))
        if not soll:
            abweichung.append("Projektdatei nennt keine Defines")
        if not ist_je_einheit:
            abweichung.append("Tlog nennt keine Uebersetzungseinheit")
        pruefe(bool(soll) and bool(ist_je_einheit) and not abweichung,
               f"jede gebaute TU hat exakt die heutige Definemenge "
               f"({len(ist_je_einheit)} TUs, {len(soll)} Defines)",
               " | ".join(abweichung))
    else:
        pruefe(False,
               "Bau-Protokoll des Kerns gefunden (CL.command.1.tlog + NakamaKern.vcxproj)",
               "ohne sie ist nicht feststellbar, mit welchen Schaltern die Lib entstand")

    # ── 1. Die Gegenprobe zuerst: taugt der Scanner ueberhaupt? ──────────────
    # Nur Werte, die im gebauten Main-Bundle stehen MUESSEN. Die reservierten
    # Codes (NkPr/NkAc) stehen dort zu Recht nicht - sie sind nicht gebaut.
    print("\n[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?")
    pflicht = ["hersteller.name", "main.produktname", "main.plugin_code",
               "main.component_cid", "main.controller_cid"]
    for name in pflicht:
        art, wert = nadeln[name]
        formen = suche(kontroll_bytes, art, wert)
        pruefe(bool(formen),
               f"Gegenprobe findet {name} = {wert!r} im gebauten Bundle",
               ",".join(formen) if formen else "NICHT GEFUNDEN")

    # ── 2. Der Kern selbst ──────────────────────────────────────────────────
    print("\n[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen")
    for name, (art, wert) in sorted(nadeln.items()):
        formen = suche(kern_bytes, art, wert)
        pruefe(not formen,
               f"NakamaKern.lib traegt {name} = {wert!r} NICHT",
               ",".join(formen) if formen else "")

    # ── 3. Bauform: nur die Kernobjekte, keine JUCE-Modulquelle ─────────────
    print("\n[3] Bauform - der Kern enthaelt genau seine eigenen Objekte")
    mitglieder = archivmitglieder(kern_bytes)
    if mitglieder is None:
        pruefe(False, "NakamaKern.lib ist ein lesbares COFF-Archiv")
    else:
        namen = {pathlib.PurePath(m).name for m in mitglieder}
        pruefe(namen == ERWARTETE_OBJEKTE,
               f"Archivmitglieder sind genau die {len(ERWARTETE_OBJEKTE)} Kernobjekte",
               ", ".join(sorted(namen)) if namen != ERWARTETE_OBJEKTE else "")
        juce = sorted(n for n in namen if n.lower().startswith("juce"))
        pruefe(not juce,
               "kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)",
               ", ".join(juce) if juce else "")

    print(f"\n{ok} ok, {len(fehler)} Fehler")
    if fehler:
        print("\nFEHLGESCHLAGEN:")
        for f in fehler:
            print("  - " + f)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
