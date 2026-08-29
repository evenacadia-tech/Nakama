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

FRISCHE - die Zusage lautet "misst nie ein veraltetes Artefakt", und sie wird
an fuenf Stellen gehalten, weil ein Artefakt auf fuenf Arten veralten kann.
Vor allem anderen prueft `configure_frische`, ob die erzeugte Projektdatei
ueberhaupt noch dem heutigen CMake-Stand entspricht - sonst waeren beide Seiten
jedes folgenden Vergleichs gemeinsam veraltet (NAK-85). Abschnitt [0] haelt
danach

  1. die Lib gegen die Zeitstempel ihrer Quellen,
  2. die Uebersetzungseinheiten des Tlogs gegen NAKAMA_KERN_QUELLEN und gegen
     die Objekte im Archiv - drei Mengen, paarweise gleich; ein veralteter
     Tlog-Eintrag wird benannt, aber nicht mitgezaehlt,
  3. die gebauten Schalter gegen die heutige Projektdatei, und zwar in vier
     Klassen beidseitig (Defines, Includepfade, erzwungene Includes,
     Sprachstandard) plus den AdditionalOptions-Tokens auf Enthaltensein -
     welche ClCompile-Elemente NICHT abgebildet sind, steht im Kommentarkopf
     "NAK-85 Runde 4" weiter unten,
  4. die Lib gegen ihre eigenen Objekte und ihr Tlog (wurde nach der letzten
     Uebersetzung ueberhaupt gelinkt?).

Punkte 2 bis 4 kamen mit der vierten T3-Runde zu NAK-85 dazu (29.08.2026).

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py [bauverzeichnis]
    py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py --selbsttest

Exitcodes: 0 gruen · 2 rot · 3 Voraussetzung fehlt (nicht gebaut oder
Configure veraltet - dann ist nichts gemessen und nichts behauptet).
"""

from __future__ import annotations

import json
import os
import pathlib
import re
import sys
import tempfile

WURZEL = pathlib.Path(__file__).resolve().parents[2]
IDENTITAET = WURZEL / "eq-copilot" / "identity" / "plugin-identities-v1.json"
KERNQUELLEN = WURZEL / "eq-copilot" / "plugin"
KERN_CMAKE = KERNQUELLEN / "CMakeLists.txt"
KERN_FASSADE = WURZEL / "eq-copilot" / "cmake" / "NakamaKern.cmake"

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
#
# ⚠️ Anlauf 3 war noetig (NAK-85, 28.08.2026): dieser Vergleich traegt die
#    Zusage nur, solange die .vcxproj SELBST aktuell ist. Wer CMake aendert und
#    dieses Bein ohne Configure startet, haelt eine alte Projektdatei gegen ein
#    altes Tlog - sie stimmen ueberein. Die Ergaenzung steht bei
#    `configure_frische` weiter unten und laeuft VOR jeder Messung.


# ── NAK-85 Runde 4 (vierter T3-Pruefer zu S8, 29.08.2026) ───────────────────
#
# Anlauf 3 (unten bei `configure_frische`) schloss die Luecke "alte .vcxproj
# gegen altes Tlog". Der vierte Pruefer fand die naechste: der Vergleich sah nur
# DEFINES. Wer eine kernrelevante CMake-Aenderung konfiguriert, die einen
# NICHT-Define-Schalter dreht - ein `/FI`, ein `/std:`, ein Token aus
# AdditionalOptions - und dann NICHT baut, hat eine frische Projektdatei
# (`configure_frische` ist zufrieden), ein altes Tlog und trotzdem gleiche
# Definemengen: gruen auf einer Lib, die mit anderen Schaltern entstand.
# Am 29.08. gemessen (Rohausgabe im Manifest): `/Zc:__cplusplus` stand in der
# Projektdatei, nicht im Tlog - Exit 0.
#
# Gemessen wird deshalb weiter an der Stelle, die MSBuild VERBRAUCHT (dem
# Tlog), aber in fuenf Schalterklassen statt einer:
#
#   defines             /D, -D                 beide Richtungen
#   includepfade        /I, -I, /external:I    beide Richtungen
#   erzwungene_includes /FI, -FI               beide Richtungen
#   sprachstandard      /std:...               beide Richtungen
#   AdditionalOptions   uebrige Tokens der Projektdatei -> muessen im Tlog STEHEN
#
# Ein Token, das CMake ueber AdditionalOptions durchreicht und das zu einer der
# vier Klassen gehoert (`/external:I`, ein durchgereichtes `/std:` oder `/FI`),
# wird dort einsortiert statt in die letzte Klasse - beide Seiten benutzen
# dieselbe Zuordnung `_klassen_aus_tokens`.
#
# ⚠️ Die letzte Klasse kann nur Enthaltensein pruefen, nicht Mengengleichheit:
#    die Kommandozeile traegt ausserdem jeden Schalter, den MSBuild selbst aus
#    den uebrigen ClCompile-Elementen erzeugt (/nologo, /W4, /Ox, /MD, ...).
#    "Kein zusaetzliches Token" waere hier also eine Behauptung ohne Messung.
#    Der Tokenvergleich laeuft ausserdem case-insensitiv, weil MSBuild Pfade im
#    Tlog in Grossschreibung ablegt.
#
# ⚠️ NICHT abgebildet und damit ausdruecklich nicht behauptet sind die uebrigen
#    ClCompile-Elemente der Projektdatei - heute: AssemblerListingLocation,
#    BasicRuntimeChecks, DebugInformationFormat, ExceptionHandling,
#    InlineFunctionExpansion, MinimalRebuild, MultiProcessorCompilation,
#    ObjectFileName, Optimization, PrecompiledHeader, RuntimeLibrary,
#    ScanSourceForModuleDependencies, SupportJustMyCode, UseFullPaths,
#    WarningLevel - sowie Uebersteuerungen an einzelnen <ClCompile Include=...>
#    Eintraegen (heute gibt es keine). Sie brauchten je eine eigene
#    Enum->Schalter-Tabelle; wer sie ergaenzt, ergaenzt auch die Behauptung in
#    tools/beweise.ps1 und den Selbsttest.
#
# ⚠️ Die Reihenfolge der Includepfade wird NICHT verglichen, nur die Menge.

# Enum-Werte der Projektdatei -> cl-Schalter. Quelle ist die Werkzeug-
# beschreibung des installierten Toolsets (MSBuild cl.xml, EnumProperty
# LanguageStandard / LanguageStandard_C, VS 2022 v170), nicht das Gedaechtnis.
# Ein unbekannter Wert ist ROT (Prueflistenregel D), nie ein stilles "kein
# /std:" - genau so entstuende sonst wieder ein einseitig blinder Vergleich.
_STD_CXX_AUS_VCXPROJ = {
    "": None,
    "default": None,
    "stdcpp14": "/std:c++14",
    "stdcpp17": "/std:c++17",
    "stdcpp20": "/std:c++20",
    "stdcpp23": "/std:c++23preview",
    "stdcpplatest": "/std:c++latest",
}
_STD_C_AUS_VCXPROJ = {
    "": None,
    "default": None,
    "stdc11": "/std:c11",
    "stdc17": "/std:c17",
    "stdclatest": "/std:clatest",
}

# Anzeigenamen der beidseitig verglichenen Klassen. Die Reihenfolge ist die der
# Ausgabe; `SCHALTERKLASSEN` ist zugleich die Menge, ueber die verglichen wird.
SCHALTERKLASSEN = ("defines", "includepfade", "erzwungene_includes", "sprachstandard")
KLASSENNAME = {
    "defines": "Defines",
    "includepfade": "Includepfade",
    "erzwungene_includes": "erzwungene Includes",
    "sprachstandard": "Sprachstandard",
}


def _zerlege_kommandozeile(zeile: str) -> list[str]:
    """Zerlegt eine cl-Kommandozeile in Tokens und achtet dabei auf Quoting.

    Ein Anfuehrungszeichen hinter einer UNGERADEN Zahl Backslashes ist ein
    Zeichen, keine Klammer - so schreibt MSBuild die Defines mit Stringwert und
    das Ausgabeverzeichnis mit verdoppeltem Schlussbackslash. Ein naiver Split
    an Leerzeichen risse beide auseinander, und der Vergleich klagte dann ueber
    selbst erzeugten Schrott.
    """
    tokens: list[str] = []
    akt: list[str] = []
    in_klammer = False
    backslashes = 0
    for zeichen in zeile:
        if zeichen == "\\":
            backslashes += 1
            akt.append(zeichen)
            continue
        if zeichen == '"':
            if backslashes % 2 == 0:
                in_klammer = not in_klammer
            akt.append(zeichen)
            backslashes = 0
            continue
        backslashes = 0
        if zeichen.isspace() and not in_klammer:
            if akt:
                tokens.append("".join(akt))
                akt = []
        else:
            akt.append(zeichen)
    if akt:
        tokens.append("".join(akt))
    return tokens


# Praefixe je Klasse. Dieselbe Tabelle bedient BEIDE Seiten - das Tlog und die
# AdditionalOptions der Projektdatei -, damit ein Schalter, den CMake ueber
# AdditionalOptions durchreicht, in DERSELBEN Klasse landet wie einer aus dem
# eigenen ClCompile-Element. Ohne das faende der Vergleich ihn im Tlog als
# "nur gebaut" und faerbte grundlos rot; genau so waere `/external:I`
# ausgegangen, fuer das das installierte Toolset gar keine eigene
# ClCompile-Eigenschaft kennt (cl.xml: nur /I und /external:env:).
KLASSEN_PRAEFIXE = {
    "defines": ("/D", "-D"),
    "includepfade": ("/external:I", "/I", "-I"),
    "erzwungene_includes": ("/FI", "-FI"),
}
# Laengster Praefix zuerst, damit `/external:I` nicht an einem kuerzeren
# haengenbleibt.
_PRAEFIX_ZU_KLASSE = tuple(sorted(
    ((praefix, klasse) for klasse, praefixe in KLASSEN_PRAEFIXE.items()
     for praefix in praefixe),
    key=lambda eintrag: len(eintrag[0]), reverse=True))


def _klassen_aus_tokens(tokens: list[str]) -> tuple[dict[str, set[str]], list[str]]:
    """Ordnet Tokens einer cl-Kommandozeile den Schalterklassen zu.

    Gibt (Klassen, uebrige Tokens). cl erlaubt beide Schreibweisen: `/Ipfad`
    und `/I pfad`; der Sprachstandard steht immer als ganzes Token.
    """
    klassen: dict[str, set[str]] = {klasse: set() for klasse in SCHALTERKLASSEN}
    rest: list[str] = []
    i = 0
    while i < len(tokens):
        token = tokens[i]
        if token.startswith("/std:") or token.startswith("-std:"):
            klassen["sprachstandard"].add(token)
            i += 1
            continue
        treffer = next(((p, k) for p, k in _PRAEFIX_ZU_KLASSE if token.startswith(p)), None)
        if treffer is None:
            rest.append(token)
            i += 1
            continue
        praefix, klasse = treffer
        wert = token[len(praefix):]
        if not wert and i + 1 < len(tokens):
            i += 1
            wert = tokens[i]
        if wert:
            klassen[klasse].add(_normiere(wert) if klasse == "defines"
                                else _pfad_schluessel(wert))
        i += 1
    return klassen, rest


def _pfad_schluessel(wert: str) -> str:
    """Vergleichbare Form eines Pfadarguments.

    Die Projektdatei schreibt den Pfad in Originalschreibung, das Tlog dieselbe
    Stelle in Grossschreibung und in Anfuehrungszeichen. Ohne Klammern, ohne
    Escapes und ueber normcase/normpath sind beide dasselbe.
    """
    w = wert.strip()
    if len(w) >= 2 and w.startswith('"') and w.endswith('"'):
        w = w[1:-1]
    w = w.replace('\\"', '"')
    if w.endswith("\\\\"):
        w = w[:-1]
    if not w:
        return ""
    return os.path.normcase(os.path.normpath(w))


def _xml_text(roh: str) -> str:
    """XML-Entitaeten aufloesen.

    Die Projektdatei schreibt Anfuehrungszeichen und kaufmaennisches Und als
    Entitaet (im heutigen NakamaKern.vcxproj vier `&amp;` in den
    CustomBuild-Zeilen). Unaufgeloest verglichen wir spaeter
    `&quot;Pfad&quot;` gegen den Pfad aus dem Tlog und faerbten grundlos rot.
    `&amp;` zuletzt, damit `&amp;quot;` nicht zweimal aufgeloest wird.
    """
    for entitaet, zeichen in (("&lt;", "<"), ("&gt;", ">"), ("&quot;", '"'),
                              ("&apos;", "'"), ("&#39;", "'"), ("&amp;", "&")):
        roh = roh.replace(entitaet, zeichen)
    return roh


def _element_text(block: str, name: str) -> str:
    treffer = re.search(rf"<{name}>(.*?)</{name}>", block, re.S)
    return _xml_text(treffer.group(1)).strip() if treffer else ""


def _element_liste(block: str, name: str) -> list[str]:
    """Semikolonliste eines ClCompile-Elements ohne die MSBuild-Rueckverweise."""
    werte: list[str] = []
    for treffer in re.finditer(rf"<{name}>(.*?)</{name}>", block, re.S):
        for teil in _xml_text(treffer.group(1)).split(";"):
            teil = teil.strip()
            if teil and not teil.startswith("%"):
                werte.append(teil)
    return werte


def _clcompile_block(datei: pathlib.Path, konfig: str) -> str:
    """Nur die <ClCompile>-Teile der passenden ItemDefinitionGroup.

    Die Einengung auf ClCompile ist noetig: dieselbe Gruppe traegt
    <PreprocessorDefinitions> und <AdditionalIncludeDirectories> auch unter
    <ResourceCompile> und <Midl>, und die uebersetzen keine Kernquelle.
    """
    text = datei.read_text(encoding="utf-8", errors="replace")
    bedingung = (r"'\$\(Configuration\)\|\$\(Platform\)'=='"
                 + re.escape(konfig) + r"'")
    muster = (r'<ItemDefinitionGroup Condition="' + bedingung
              + r'">(.*?)</ItemDefinitionGroup>')
    stuecke: list[str] = []
    for gruppe in re.finditer(muster, text, re.S):
        for cl in re.finditer(r"<ClCompile>(.*?)</ClCompile>", gruppe.group(1), re.S):
            stuecke.append(cl.group(1))
    return "\n".join(stuecke)


def _zeichensatz_defines(datei: pathlib.Path, konfig: str) -> set[str]:
    """Visual Studio erzeugt aus CharacterSet weitere compilerwirksame Defines.

    Sie stehen nicht in <PreprocessorDefinitions>. Sie pauschal als erlaubte
    Extras auszunehmen waere wieder ein einseitiger Vergleich: aendert sich
    CharacterSet, muss genau diese Sollmenge mitwandern.
    """
    text = datei.read_text(encoding="utf-8", errors="replace")
    bedingung = (r"'\$\(Configuration\)\|\$\(Platform\)'=='"
                 + re.escape(konfig) + r"'")
    eigenschaften = (r'<PropertyGroup Condition="' + bedingung
                     + r'"[^>]*>(.*?)</PropertyGroup>')
    gefunden: set[str] = set()
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


def _sprachstandard_soll(block: str) -> set[str]:
    werte: set[str] = set()
    for element, tabelle in (("LanguageStandard", _STD_CXX_AUS_VCXPROJ),
                             ("LanguageStandard_C", _STD_C_AUS_VCXPROJ)):
        roh = _element_text(block, element)
        schluessel = roh.lower()
        if schluessel not in tabelle:
            raise RuntimeError(
                f"unbekannter <{element}>-Wert {roh!r} in der Projektdatei - "
                f"dieses Bein kennt den zugehoerigen cl-Schalter nicht und raet nicht")
        schalter = tabelle[schluessel]
        if schalter:
            werte.add(schalter)
    return werte


def schalter_aus_vcxproj(datei: pathlib.Path,
                         konfig: str = "Release|x64",
                         ) -> tuple[dict[str, set[str]], set[str]]:
    """Sollschalter der Projektdatei: (die vier Vergleichsklassen, AdditionalOptions)."""
    block = _clcompile_block(datei, konfig)
    optionen = [token for token
                in _zerlege_kommandozeile(_element_text(block, "AdditionalOptions"))
                if not token.startswith("%")]
    aus_optionen, zusatz = _klassen_aus_tokens(optionen)
    klassen = {
        "defines": ({_normiere(d) for d in _element_liste(block, "PreprocessorDefinitions")}
                    | _zeichensatz_defines(datei, konfig)
                    | aus_optionen["defines"]),
        "includepfade": ({_pfad_schluessel(p)
                          for p in _element_liste(block, "AdditionalIncludeDirectories")}
                         | aus_optionen["includepfade"]),
        "erzwungene_includes": ({_pfad_schluessel(p)
                                 for p in _element_liste(block, "ForcedIncludeFiles")}
                                | aus_optionen["erzwungene_includes"]),
        "sprachstandard": _sprachstandard_soll(block) | aus_optionen["sprachstandard"],
    }
    return klassen, set(zusatz)


def schalter_aus_tlog(datei: pathlib.Path) -> list[tuple[str, dict[str, set[str]], set[str]]]:
    """Je Uebersetzungseinheit: (Quellpfad, Schalterklassen, alle Tokens normiert).

    MSBuild schreibt die .tlog als UTF-16LE mit BOM.
    """
    text = datei.read_bytes().decode("utf-16-le", errors="replace").lstrip("﻿")
    einheiten: list[tuple[str, dict[str, set[str]], set[str]]] = []
    quelle = "<unbekannte Kernquelle>"
    for zeile in text.splitlines():
        if not zeile.strip():
            continue
        if zeile.startswith("^"):
            quelle = zeile[1:].strip()
            continue
        tokens = _zerlege_kommandozeile(zeile)
        klassen, _ = _klassen_aus_tokens(tokens)
        einheiten.append((quelle, klassen,
                          {os.path.normcase(token) for token in tokens}))
    return einheiten


def _normiere(define: str) -> str:
    """Ein Define mit Stringwert liest sich in Projektdatei und Tlog verschieden.

    Die Projektdatei schreibt ihn mit einfachen Anfuehrungszeichen, das Tlog
    zusaetzlich mit Backslash-Escapes. Beide meinen denselben Wert.
    """
    return define.replace("\\", "").replace('"', "").strip()


def _mengen_abweichungen(soll: set[str], ist: set[str]) -> tuple[list[str], list[str]]:
    """Fehlende und veraltete zusaetzliche Werte, also beide Richtungen."""
    return sorted(soll - ist), sorted(ist - soll)


def tu_mengen_abgleich(tlog_einheiten: list[tuple[str, dict[str, set[str]], set[str]]],
                       kernquellen: list[pathlib.Path],
                       archivnamen: list[str] | None,
                       ) -> tuple[list[str],
                                  list[tuple[str, dict[str, set[str]], set[str]]],
                                  list[str]]:
    """Drei Mengen, paarweise gleich - Tlog, NAKAMA_KERN_QUELLEN und Archiv.

    NAK-85 Runde 4, zweiter Befund: Visual Studios inkrementelles Tlog BEHAELT
    den Eintrag einer entfernten Quelle. Wer nur ueber die vorhandenen
    Datensaetze laeuft, verlangt nie die heutige TU-Menge - am 29.08. nannte das
    Tlog zehn Quellen einschliesslich des laengst aus dem Kern genommenen
    PipeToken.cpp, waehrend CMake-Liste und Archiv neun trugen, und A14 meldete
    trotzdem "10 TUs" und Exit 0.

    Ein veralteter Eintrag ist kein Fehler des Baus - er ist Buchhaltung von
    MSBuild. Er wird deshalb nicht gezaehlt, aber auch nicht verschwiegen.
    Fehlt dagegen eine heutige Kernquelle im Tlog oder im Archiv, ist das ROT:
    dann sagt das Bauprotokoll nichts ueber eine Uebersetzungseinheit, die in
    der gemessenen Lib steckt.

    Gibt (Klagen, die aktuellen Tlog-Einheiten, die veralteten Eintraege).
    """
    # Verglichen wird ueber den NORMIERTEN PFAD, nicht ueber den Dateinamen:
    # MSBuild legt die Quellzeile des Tlogs in Grossschreibung ab, CMake nennt
    # dieselbe Datei in ihrer echten Schreibweise. Ein Vergleich der rohen
    # Namen faende deshalb nie einen Treffer - im Selbsttest H/I gemessen.
    nach_schluessel = {_norm_schluessel(p): p for p in kernquellen}

    aktuell: list[tuple[str, dict[str, set[str]], set[str]]] = []
    veraltet: list[str] = []
    gesehen: set[str] = set()
    doppelt: list[str] = []
    for quelle, klassen, tokens in tlog_einheiten:
        schluessel = _norm_schluessel(pathlib.Path(quelle))
        echte = nach_schluessel.get(schluessel)
        if echte is None:
            # Existiert die Datei noch (sie ist nur nicht mehr Kern), nennt
            # resolve() sie in der Schreibweise des Dateisystems; sonst bleibt
            # die Schreibweise stehen, die das Tlog selbst traegt.
            alt = pathlib.Path(quelle)
            veraltet.append(alt.resolve().name if alt.is_file()
                            else pathlib.PureWindowsPath(quelle).name)
            continue
        if schluessel in gesehen:
            doppelt.append(echte.name)
        gesehen.add(schluessel)
        # Der Name aus CMake, nicht der aus dem Tlog: so liest sich die Ausgabe
        # in der Schreibweise des Quellbaums.
        aktuell.append((echte.name, klassen, tokens))

    klagen: list[str] = []
    if not tlog_einheiten:
        klagen.append("Tlog nennt keine Uebersetzungseinheit")
    for name in sorted(doppelt):
        klagen.append(f"{name}: mehrfach im Tlog")

    for schluessel in sorted(set(nach_schluessel) - gesehen):
        klagen.append(f"{nach_schluessel[schluessel].name}: in NAKAMA_KERN_QUELLEN, "
                      f"aber ohne Tlog-Eintrag")

    if archivnamen is None:
        klagen.append("Archivmitglieder nicht lesbar - TU-Menge nicht gegen das "
                      "Archiv abgleichbar")
    else:
        # Objekt- und Quellname teilen sich den Stamm; verglichen wird
        # unabhaengig von der Schreibweise, angezeigt wird der echte Name.
        objekte = {os.path.normcase(pathlib.PurePath(m).stem): pathlib.PurePath(m).name
                   for m in archivnamen}
        quellen_nach_stamm = {os.path.normcase(p.stem): p for p in kernquellen}
        for stamm in sorted(set(quellen_nach_stamm) - set(objekte)):
            klagen.append(f"{quellen_nach_stamm[stamm].name}: in NAKAMA_KERN_QUELLEN, "
                          f"aber ohne Objekt im Archiv")
        for stamm in sorted(set(objekte) - set(quellen_nach_stamm)):
            klagen.append(f"{objekte[stamm]}: im Archiv, aber nicht in NAKAMA_KERN_QUELLEN")

    return klagen, aktuell, sorted(veraltet)


def schalter_abgleich(soll: dict[str, set[str]], soll_zusatz: set[str],
                      einheiten: list[tuple[str, dict[str, set[str]], set[str]]],
                      ) -> list[str]:
    """Jede gebaute TU gegen die heutigen Schalter der Projektdatei.

    Vier Klassen in BEIDE Richtungen, AdditionalOptions nur auf Enthaltensein -
    warum das so ist, steht im Kopf dieses Abschnitts.
    """
    klagen: list[str] = []
    if not soll["defines"]:
        klagen.append("Projektdatei nennt keine Defines")
    for quelle, klassen, tokens in einheiten:
        for klasse in SCHALTERKLASSEN:
            fehlend, zusaetzlich = _mengen_abweichungen(soll[klasse], klassen[klasse])
            if fehlend:
                klagen.append(f"{quelle}: {KLASSENNAME[klasse]} nicht gebaut: "
                              + ", ".join(fehlend))
            if zusaetzlich:
                klagen.append(f"{quelle}: {KLASSENNAME[klasse]} nur gebaut: "
                              + ", ".join(zusaetzlich))
        offen = sorted(t for t in soll_zusatz if os.path.normcase(t) not in tokens)
        if offen:
            klagen.append(f"{quelle}: AdditionalOptions nicht gebaut: " + ", ".join(offen))
    return klagen


def linkfrische(lib: pathlib.Path, objekte: list[pathlib.Path],
                tlog: pathlib.Path) -> list[str]:
    """Ist die Lib nach der letzten Uebersetzung auch neu gelinkt worden?

    NAK-85 Runde 4, erster Befund, zweite Haelfte: der Schaltervergleich haelt
    Projektdatei und Tlog zusammen, sagt aber nichts darueber, ob die .lib
    dieses Tlog ueberhaupt gesehen hat. Wird uebersetzt und nicht gelinkt -
    Einzeldatei-Bau, abgebrochener Lauf -, misst dieses Bein ein Archiv, das
    aelter ist als seine eigenen Objekte.
    """
    if not objekte:
        return [f"keine .obj neben {_kurz(tlog)} - ohne sie ist der Linkstand "
                f"nicht feststellbar"]
    libzeit = lib.stat().st_mtime
    juenger = sorted(o.name for o in objekte if o.stat().st_mtime > libzeit)
    klagen: list[str] = []
    if juenger:
        klagen.append("Lib nicht neu gelinkt nach letzter Uebersetzung: "
                      + ", ".join(juenger))
    if tlog.stat().st_mtime > libzeit:
        klagen.append("Lib nicht neu gelinkt nach letzter Uebersetzung: "
                      f"{_kurz(tlog)} ist juenger")
    return klagen


# ── NAK-85 (dritter T3-Pruefer zu S8, 28.08.2026) ───────────────────────────
#
# Der Definevergleich oben haelt die ERZEUGTE Projektdatei gegen das Tlog. Beide
# koennen eintraechtig veraltet sein: wer cmake/NakamaKern.cmake oder
# plugin/CMakeLists.txt aendert und dieses Bein direkt startet, vergleicht die
# ALTE .vcxproj mit dem ALTEN Tlog. Sie stimmen ueberein, das Bein meldet gruen -
# und Behauptung 14 ("A14 misst nie ein veraltetes Artefakt") ist gebrochen.
#
# Ein dritter mtime-Stellvertreter waere derselbe Fehler wie Anlauf 1 und 2 im
# Kopf dieser Datei. Am 29.08. gemessen: NakamaKern.vcxproj trug 06:08, der
# generate.stamp desselben Bauverzeichnisses 15:12 - CMake schreibt eine
# Projektdatei nur bei INHALTLICHER Aenderung neu (copy-if-different), beruehrt
# den Stamp aber bei JEDEM Configure. Der Stamp ist damit der einzige ehrliche
# Zeitanker; die .vcxproj ist genau der Stellvertreter, an dem Anlauf 2 scheiterte.
#
# Gefragt wird deshalb wieder die Stelle, die es weiss: der VS-Generator schreibt
# je Bauverzeichnis CMakeFiles/generate.stamp.depend - die Liste der CMake-
# Eingaben, die er dort verbraucht hat. Ist eine gelistete Eingabe juenger als
# der zugehoerige generate.stamp, steht das Configure aus.
#
# WELCHE Verzeichnisse zaehlen, sagt der Generator ebenfalls selbst:
# <bau>/CMakeFiles/generate.stamp.list nennt genau die Verzeichnisse DIESES
# Configures. Am 29.08. waren das 7, waehrend ein blindes Glob ueber das
# Bauverzeichnis 13 gefunden haette - die sechs zusaetzlichen sind
# FetchContent-Subbuilds (eigene CMake-Projekte) und Reste eines frueheren
# JUCE-Layouts. Sie koennten diesen Riegel mit fremden Ursachen faerben, ohne
# ueber den Kern etwas auszusagen. Keine handgepflegte Eingabeliste.
#
# Die beiden Dateien, die der Befund nennt, liegen in VERSCHIEDENEN Listen:
# plugin/CMakeLists.txt im Verzeichnis <bau>/plugin, cmake/NakamaKern.cmake im
# Wurzelverzeichnis (es kommt per include() aus eq-copilot/CMakeLists.txt).
# Wer nur das Plugin-Verzeichnis prueft, uebersieht genau die Datei aus NAK-85.
KONFIG_PFLICHTEINGABEN = (KERN_CMAKE, KERN_FASSADE)


def _norm_schluessel(p: pathlib.Path) -> str:
    """Vergleichbare Form eines Pfades.

    CMake schreibt Eingaben teils unnormiert (`cmake/../identity/x.json`), und
    Windows unterscheidet Gross-/Kleinschreibung nicht. Ohne beides zu
    vereinheitlichen wuerde die Abdeckungskontrolle unten falsch klagen.
    """
    try:
        aufgeloest = p.resolve()
    except OSError:
        aufgeloest = pathlib.Path(os.path.normpath(str(p)))
    return os.path.normcase(str(aufgeloest))


def _kurz(p: pathlib.Path) -> str:
    """Repo-relativ, wo moeglich - CMake-Module liegen ausserhalb."""
    try:
        return p.resolve().relative_to(WURZEL).as_posix()
    except (ValueError, OSError):
        return str(p)


def _zeilen_ohne_kommentar(datei: pathlib.Path) -> list[str]:
    return [z.strip() for z in datei.read_text(encoding="utf-8", errors="replace").splitlines()
            if z.strip() and not z.lstrip().startswith("#")]


def configure_frische(bau: pathlib.Path) -> list[str]:
    """Ist das Configure juenger als jede CMake-Eingabe, die es verbraucht hat?

    Gibt die Klagen zurueck; eine leere Liste heisst "Configure ist aktuell".
    Prueflistenregel D: jede Unklarheit ist eine Klage, nie ein stilles Ja -
    fehlende Stampliste, fehlender Stamp, fehlende Eingabeliste, verschwundene
    Eingabe und blinde Wache faerben genauso wie ein zu junger Zeitstempel.
    """
    liste = bau / "CMakeFiles" / "generate.stamp.list"
    if not liste.is_file():
        return [f"{_kurz(liste)} fehlt - Bauverzeichnis nicht konfiguriert"]

    stamps = []
    for zeile in _zeilen_ohne_kommentar(liste):
        p = pathlib.Path(zeile)
        stamps.append(p if p.is_absolute() else bau / zeile)
    if not stamps:
        return [f"{_kurz(liste)} nennt kein Bauverzeichnis - Configure unvollstaendig"]

    klagen: list[str] = []
    gesehen: set[str] = set()
    for stamp in stamps:
        depend = stamp.with_name("generate.stamp.depend")
        if not stamp.is_file():
            klagen.append(f"{_kurz(stamp)} fehlt - Verzeichnis nicht konfiguriert")
            continue
        if not depend.is_file():
            klagen.append(f"{_kurz(depend)} fehlt - Eingabeliste des Generators nicht lesbar")
            continue
        stampzeit = stamp.stat().st_mtime
        for zeile in _zeilen_ohne_kommentar(depend):
            eingabe = pathlib.Path(zeile)
            if not eingabe.is_absolute():
                eingabe = depend.parent / zeile
            gesehen.add(_norm_schluessel(eingabe))
            if not eingabe.is_file():
                klagen.append(f"{_kurz(eingabe)} ist als CMake-Eingabe gelistet, "
                              f"existiert aber nicht mehr")
                continue
            if eingabe.stat().st_mtime > stampzeit:
                klagen.append(f"{_kurz(eingabe)} juenger als {_kurz(stamp)}")

    # Fail-closed gegen das Blindwerden der Wache selbst. Das ist keine
    # Ersatz-Eingabeliste - die bleibt generatorgepflegt -, sondern dieselbe
    # Vorsicht wie bei ERWARTETE_OBJEKTE: aendert CMake sein Layout oder faellt
    # eine dieser Dateien aus der Generatorliste, soll dieses Bein SPRECHEN
    # statt still durchzurutschen.
    for pflicht in KONFIG_PFLICHTEINGABEN:
        if _norm_schluessel(pflicht) not in gesehen:
            klagen.append(f"{_kurz(pflicht)} steht in keiner generate.stamp.depend - "
                          f"die Frischewache ist an dieser Datei blind")
    return klagen


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

    fehlend, zusaetzlich = _mengen_abweichungen(
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

    _selbsttest_configure_frische()
    _selbsttest_schalter_und_tu()

    print(f"\n{ok} ok, {len(fehler)} Fehler")
    return 2 if fehler else 0


def _selbsttest_schalter_und_tu() -> None:
    """NAK-85 Runde 4 baulos: Schalterklassen, TU-Menge und Linkfrische.

    Kuenstliche Projektdatei und kuenstliches Tlog genuegen, weil beide Seiten
    reine Textquellen sind. Jede neue Pruefung wird hier einmal absichtlich
    gebrochen - eine Wache, die niemand hat fallen sehen, ist keine.
    """
    print("\nNAK-85-Runde-4-Selbsttest: Schalterklassen, TU-Menge, Linkfrische")

    vcxproj_text = (
        '<?xml version="1.0" encoding="utf-8"?>\n'
        '<Project>\n'
        '  <PropertyGroup Condition="\'$(Configuration)|$(Platform)\'==\'Release|x64\'"'
        ' Label="Configuration">\n'
        '    <CharacterSet>MultiByte</CharacterSet>\n'
        '  </PropertyGroup>\n'
        '  <ItemDefinitionGroup Condition="\'$(Configuration)|$(Platform)\'==\'Release|x64\'">\n'
        '    <ClCompile>\n'
        '      <AdditionalIncludeDirectories>C:\\Kuenstlich\\Ein;C:\\Kuenstlich\\Zwei;'
        '%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>\n'
        '      <AdditionalOptions>%(AdditionalOptions) /utf-8 /bigobj</AdditionalOptions>\n'
        '      <ForcedIncludeFiles>C:\\Kuenstlich\\Vorspann.h</ForcedIncludeFiles>\n'
        '      <LanguageStandard>stdcpp20</LanguageStandard>\n'
        '      <PreprocessorDefinitions>%(PreprocessorDefinitions);HEUTE=1;'
        'CMAKE_INTDIR="Release"</PreprocessorDefinitions>\n'
        '    </ClCompile>\n'
        '    <ResourceCompile>\n'
        '      <PreprocessorDefinitions>%(PreprocessorDefinitions);NUR_RESSOURCE=1'
        '</PreprocessorDefinitions>\n'
        '      <AdditionalIncludeDirectories>C:\\Kuenstlich\\NurRessource'
        '</AdditionalIncludeDirectories>\n'
        '    </ResourceCompile>\n'
        '  </ItemDefinitionGroup>\n'
        '</Project>\n'
    )

    # So schreibt MSBuild die Zeile: Pfade in Grossschreibung und in
    # Anfuehrungszeichen, der Stringwert des Defines zusaetzlich escaped.
    VOLL = ('/c /I"C:\\KUENSTLICH\\EIN" /I"C:\\KUENSTLICH\\ZWEI" /nologo /W4 /Ox'
            ' /D _MBCS /D HEUTE=1 /D "CMAKE_INTDIR=\\"Release\\""'
            ' /FI"C:\\KUENSTLICH\\VORSPANN.H" /std:c++20 /Fo"KUENSTLICH.DIR\\RELEASE\\\\"'
            ' /utf-8 /bigobj C:\\KUENSTLICH\\EINS.CPP')

    with tempfile.TemporaryDirectory() as roh:
        heim = pathlib.Path(roh)
        projekt = heim / "Kuenstlich.vcxproj"
        projekt.write_text(vcxproj_text, encoding="utf-8")

        def tlog_schreiben(*saetze: tuple[str, str]) -> pathlib.Path:
            datei = heim / "CL.command.1.tlog"
            text = "".join(f"^{quelle}\n{zeile}\n" for quelle, zeile in saetze)
            datei.write_bytes("\ufeff".encode("utf-16-le") + text.encode("utf-16-le"))
            return datei

        soll, soll_zusatz = schalter_aus_vcxproj(projekt)
        pruefe(soll["defines"] == {"_MBCS", "HEUTE=1", "CMAKE_INTDIR=Release"},
               "A: Projektdatei liefert genau die ClCompile-Defines plus CharacterSet",
               ", ".join(sorted(soll["defines"])))
        pruefe(all("nurressource" not in p for p in soll["includepfade"]),
               "B: ResourceCompile faellt nicht in die ClCompile-Klassen",
               ", ".join(sorted(soll["includepfade"])))
        pruefe(soll["sprachstandard"] == {"/std:c++20"} and len(soll["erzwungene_includes"]) == 1
               and soll_zusatz == {"/utf-8", "/bigobj"},
               "C: Sprachstandard, erzwungenes Include und AdditionalOptions gelesen",
               f"{sorted(soll['sprachstandard'])} | {sorted(soll_zusatz)}")

        # AdditionalOptions ist der Weg, auf dem CMake Schalter durchreicht, fuer
        # die MSBuild keine eigene ClCompile-Eigenschaft hat. Landen sie nicht
        # in ihrer Klasse, meldet der Tlog-Vergleich sie als "nur gebaut".
        durchgereicht = vcxproj_text.replace(
            "<AdditionalOptions>%(AdditionalOptions) /utf-8 /bigobj</AdditionalOptions>",
            "<AdditionalOptions>%(AdditionalOptions) /utf-8 /bigobj"
            " /external:I &quot;C:\\Kuenstlich\\Fremd&quot;</AdditionalOptions>")
        projekt.write_text(durchgereicht.replace(
            "<LanguageStandard>stdcpp20</LanguageStandard>", ""), encoding="utf-8")
        soll_d, zusatz_d = schalter_aus_vcxproj(projekt)
        pruefe(any("fremd" in p for p in soll_d["includepfade"])
               and soll_d["sprachstandard"] == set()
               and not any("external" in z for z in zusatz_d),
               "R: /external:I aus AdditionalOptions landet in der Includeklasse",
               f"{sorted(soll_d['includepfade'])} | Rest {sorted(zusatz_d)}")
        mit_fremd = VOLL.replace(' /utf-8', ' /external:I"C:\\KUENSTLICH\\FREMD" /utf-8')
        pruefe(any("Sprachstandard nur gebaut" in k for k in schalter_abgleich(
                   soll_d, zusatz_d,
                   schalter_aus_tlog(tlog_schreiben(("C:\\KUENSTLICH\\EINS.CPP", mit_fremd)))))
               and not any("Includepfade" in k for k in schalter_abgleich(
                   soll_d, zusatz_d,
                   schalter_aus_tlog(tlog_schreiben(("C:\\KUENSTLICH\\EINS.CPP", mit_fremd))))),
               "S: derselbe Pfad im Tlog ist damit kein Includebefund mehr")
        projekt.write_text(vcxproj_text, encoding="utf-8")

        einheit = schalter_aus_tlog(tlog_schreiben(("C:\\KUENSTLICH\\EINS.CPP", VOLL)))
        pruefe(schalter_abgleich(soll, soll_zusatz, [(n, k, t) for n, k, t in einheit]) == [],
               "D: passende Kommandozeile bleibt klaglos")

        # Jede Klasse einmal absichtlich gebrochen - fehlend und zusaetzlich.
        brueche = (
            ("Includepfade", VOLL.replace(' /I"C:\\KUENSTLICH\\ZWEI"', ""),
             "Includepfade nicht gebaut"),
            ("erzwungene Includes", VOLL.replace(' /FI"C:\\KUENSTLICH\\VORSPANN.H"', ""),
             "erzwungene Includes nicht gebaut"),
            ("Sprachstandard", VOLL.replace(" /std:c++20", " /std:c++17"),
             "Sprachstandard nicht gebaut"),
            ("AdditionalOptions", VOLL.replace(" /bigobj", ""),
             "AdditionalOptions nicht gebaut"),
            ("Defines fehlend", VOLL.replace(" /D HEUTE=1", ""),
             "Defines nicht gebaut"),
            ("Defines zusaetzlich", VOLL.replace(" /nologo", " /nologo /D NUR_GEBAUT=1"),
             "Defines nur gebaut"),
        )
        for name, zeile, erwartet in brueche:
            klagen = schalter_abgleich(
                soll, soll_zusatz,
                schalter_aus_tlog(tlog_schreiben(("C:\\KUENSTLICH\\EINS.CPP", zeile))))
            pruefe(any(erwartet in k for k in klagen),
                   f"E: gebrochene Klasse {name} wird benannt",
                   " | ".join(klagen) if klagen else "keine Klage")

        pruefe(any("Sprachstandard nur gebaut" in k for k in schalter_abgleich(
                   {**soll, "sprachstandard": set()}, soll_zusatz,
                   schalter_aus_tlog(tlog_schreiben(("C:\\KUENSTLICH\\EINS.CPP", VOLL))))),
               "F: ein /std: ohne Entsprechung in der Projektdatei ist ebenfalls eine Klage")

        projekt.write_text(
            vcxproj_text.replace("<LanguageStandard>stdcpp20", "<LanguageStandard>stdcpp99"),
            encoding="utf-8")
        try:
            schalter_aus_vcxproj(projekt)
            pruefe(False, "G: unbekannter LanguageStandard-Wert ist ROT, kein stilles Ja")
        except RuntimeError as exc:
            pruefe("stdcpp99" in str(exc),
                   "G: unbekannter LanguageStandard-Wert ist ROT, kein stilles Ja", str(exc))
        projekt.write_text(vcxproj_text, encoding="utf-8")

        # ── TU-Menge: Tlog, NAKAMA_KERN_QUELLEN und Archiv ──────────────────
        quelle_a = heim / "Eins.cpp"
        quelle_b = heim / "Zwei.cpp"
        for q in (quelle_a, quelle_b):
            q.write_text("// kuenstlich\n", encoding="utf-8")
        kernquellen = [quelle_a, quelle_b]
        archiv = ["Eins.obj", "Zwei.obj"]

        beide = schalter_aus_tlog(tlog_schreiben((str(quelle_a).upper(), VOLL),
                                                 (str(quelle_b).upper(), VOLL)))
        klagen, aktuell, veraltet = tu_mengen_abgleich(beide, kernquellen, archiv)
        pruefe(klagen == [] and len(aktuell) == 2 and veraltet == [],
               "H: deckungsgleiche TU-Mengen bleiben klaglos",
               " | ".join(klagen))

        mit_altem = schalter_aus_tlog(tlog_schreiben(
            (str(quelle_a).upper(), VOLL), (str(quelle_b).upper(), VOLL),
            ("C:\\KUENSTLICH\\ENTFERNT.CPP", VOLL)))
        klagen, aktuell, veraltet = tu_mengen_abgleich(mit_altem, kernquellen, archiv)
        pruefe(klagen == [] and len(aktuell) == 2 and veraltet == ["ENTFERNT.CPP"],
               "I: veralteter Tlog-Eintrag wird nicht gezaehlt, aber benannt",
               f"{len(aktuell)} aktuell, veraltet: {', '.join(veraltet)}")

        nur_eine = schalter_aus_tlog(tlog_schreiben((str(quelle_a).upper(), VOLL)))
        klagen, aktuell, veraltet = tu_mengen_abgleich(nur_eine, kernquellen, archiv)
        pruefe(any("Zwei.cpp" in k and "ohne Tlog-Eintrag" in k for k in klagen),
               "J: eine Kernquelle ohne Tlog-Eintrag ist ROT",
               " | ".join(klagen) if klagen else "keine Klage")

        klagen, _, _ = tu_mengen_abgleich(beide, kernquellen, ["Eins.obj"])
        pruefe(any("Zwei.cpp" in k and "ohne Objekt im Archiv" in k for k in klagen),
               "K: eine Kernquelle ohne Objekt im Archiv ist ROT",
               " | ".join(klagen) if klagen else "keine Klage")

        klagen, _, _ = tu_mengen_abgleich(beide, kernquellen, archiv + ["Fremd.obj"])
        pruefe(any("Fremd.obj" in k and "nicht in NAKAMA_KERN_QUELLEN" in k for k in klagen),
               "L: ein Objekt ohne Kernquelle ist ROT",
               " | ".join(klagen) if klagen else "keine Klage")

        klagen, _, _ = tu_mengen_abgleich(beide, kernquellen, None)
        pruefe(any("nicht lesbar" in k for k in klagen),
               "M: ein unlesbares Archiv ist eine Klage, kein stilles Ja",
               " | ".join(klagen) if klagen else "keine Klage")

        # ── Linkfrische: Lib gegen ihre Objekte und ihr Tlog ─────────────────
        lib = heim / "Kuenstlich.lib"
        objekt = heim / "Eins.obj"
        tlogdatei = tlog_schreiben((str(quelle_a).upper(), VOLL))
        lib.write_text("# lib\n", encoding="utf-8")
        objekt.write_text("# obj\n", encoding="utf-8")

        spaet = objekt.stat().st_mtime + 1000.0
        os.utime(lib, (spaet, spaet))
        os.utime(tlogdatei, (spaet - 100.0, spaet - 100.0))
        pruefe(linkfrische(lib, [objekt], tlogdatei) == [],
               "N: frisch gelinkte Lib bleibt klaglos")

        os.utime(objekt, (spaet + 100.0, spaet + 100.0))
        klagen = linkfrische(lib, [objekt], tlogdatei)
        pruefe(any("nicht neu gelinkt" in k and "Eins.obj" in k for k in klagen),
               "O: Objekt juenger als die Lib ist ROT",
               " | ".join(klagen) if klagen else "keine Klage")

        os.utime(objekt, (spaet - 100.0, spaet - 100.0))
        os.utime(tlogdatei, (spaet + 100.0, spaet + 100.0))
        klagen = linkfrische(lib, [objekt], tlogdatei)
        pruefe(any("nicht neu gelinkt" in k for k in klagen),
               "P: Tlog juenger als die Lib ist ROT",
               " | ".join(klagen) if klagen else "keine Klage")

        pruefe(linkfrische(lib, [], tlogdatei) != [],
               "Q: ohne Objekte gibt es kein gruenes Linkurteil")


def _selbsttest_configure_frische() -> None:
    """NAK-85 baulos: die vier Faelle der Configure-Frischewache.

    Ein kuenstliches Bauverzeichnis genuegt, weil die Wache nur Stamps,
    Eingabelisten und Zeitstempel liest. Die echten Pflichteingaben stehen mit
    in der kuenstlichen Liste - so laeuft die Abdeckungskontrolle mit.
    """
    print("\nNAK-85-Selbsttest: Configure-Frischewache (kuenstliches Bauverzeichnis)")
    with tempfile.TemporaryDirectory() as roh:
        bau = pathlib.Path(roh)
        cmf = bau / "CMakeFiles"
        cmf.mkdir()
        stamp = cmf / "generate.stamp"
        depend = cmf / "generate.stamp.depend"
        liste = cmf / "generate.stamp.list"
        eingabe = bau / "Kuenstlich.cmake"

        eingabe.write_text("# Platzhalter\n", encoding="utf-8")
        stamp.write_text("# stamp\n", encoding="utf-8")
        liste.write_text(str(stamp) + "\n", encoding="utf-8")

        def schreibe_depend(mit_pflicht: bool) -> None:
            zeilen = ["# CMake generation dependency list for this directory.",
                      str(eingabe)]
            if mit_pflicht:
                # bewusst unnormiert, wie CMake es selbst schreibt
                zeilen += [str(p.parent / ".." / p.parent.name / p.name).replace("\\", "/")
                           for p in KONFIG_PFLICHTEINGABEN]
            depend.write_text("\n".join(zeilen) + "\n", encoding="utf-8")

        # Der Stamp muss juenger sein als die echten Pflichtdateien, sonst
        # klagte Fall A zu Recht ueber sie statt ueber die kuenstliche Eingabe.
        spaet = max(p.stat().st_mtime for p in KONFIG_PFLICHTEINGABEN) + 1000.0
        os.utime(stamp, (spaet, spaet))

        schreibe_depend(mit_pflicht=True)
        os.utime(eingabe, (spaet - 100.0, spaet - 100.0))
        klagen = configure_frische(bau)
        pruefe(klagen == [],
               "A: aktuelles Configure bleibt klaglos",
               " | ".join(klagen))

        os.utime(eingabe, (spaet + 100.0, spaet + 100.0))
        klagen = configure_frische(bau)
        pruefe(len(klagen) == 1 and "Kuenstlich.cmake" in klagen[0]
               and "juenger als" in klagen[0],
               "B: geaenderte CMake-Eingabe ohne Configure wird benannt",
               " | ".join(klagen) if klagen else "keine Klage")

        os.utime(eingabe, (spaet - 100.0, spaet - 100.0))
        schreibe_depend(mit_pflicht=False)
        klagen = configure_frische(bau)
        pruefe(len(klagen) == len(KONFIG_PFLICHTEINGABEN)
               and all("blind" in k for k in klagen),
               "C: eine ungelistete Pflichteingabe macht die Wache nicht still",
               " | ".join(klagen) if klagen else "keine Klage")

        schreibe_depend(mit_pflicht=True)
        eingabe.unlink()
        klagen = configure_frische(bau)
        pruefe(any("existiert aber nicht mehr" in k for k in klagen),
               "C2: verschwundene CMake-Eingabe ist eine Klage, kein stilles Ja",
               " | ".join(klagen) if klagen else "keine Klage")

        liste.unlink()
        klagen = configure_frische(bau)
        pruefe(len(klagen) == 1 and "nicht konfiguriert" in klagen[0],
               "D: ohne generate.stamp.list gibt es kein gruenes Urteil",
               " | ".join(klagen) if klagen else "keine Klage")


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

    # NAK-85: bevor irgendetwas gemessen wird - beschreibt das Bauverzeichnis
    # ueberhaupt noch den heutigen CMake-Stand? Ist es das nicht, sind .vcxproj
    # UND Tlog gemeinsam veraltet und ihre Uebereinstimmung sagt nichts. Kein
    # Urteil ueber den Kern, sondern eine fehlende Voraussetzung: Exit 3.
    klagen = configure_frische(bau)
    if klagen:
        print(f"VORAUSSETZUNG: Configure veraltet - {klagen[0]}; "
              f"neu konfigurieren/bauen", file=sys.stderr)
        for weitere in klagen[1:]:
            print(f"  auch: {weitere}", file=sys.stderr)
        print("  Erst konfigurieren und bauen: "
              "pwsh -File tools/beweise.ps1 -Bauen -Ziel <manifest>", file=sys.stderr)
        return 3

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
    # Die Archivmitglieder werden schon hier gelesen: die TU-Mengenpruefung in
    # [0] gleicht gegen sie ab, [3] nutzt dieselbe Liste weiter unten.
    mitglieder = archivmitglieder(kern_bytes)

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

    # Zweite Haelfte: WELCHE Einheiten, mit WELCHEN Schaltern - und ist die Lib
    # danach ueberhaupt neu gelinkt worden? Siehe Kopf (NAK-85 Runde 4).
    projekt_kandidaten = sorted(bau.glob("**/NakamaKern.vcxproj"))
    tlog_kandidaten = sorted(bau.glob("**/NakamaKern.tlog/CL.command.1.tlog"))
    projekt = (projekt_kandidaten[0] if len(projekt_kandidaten) == 1
               else waehle_release(projekt_kandidaten, "NakamaKern.vcxproj"))
    tlog = waehle_release(tlog_kandidaten, "CL.command.1.tlog")
    if projekt is None or tlog is None:
        pruefe(False,
               "Bau-Protokoll des Kerns gefunden (CL.command.1.tlog + NakamaKern.vcxproj)",
               "ohne sie ist nicht feststellbar, mit welchen Schaltern die Lib entstand")
    else:
        soll: dict[str, set[str]] | None
        try:
            soll, soll_zusatz = schalter_aus_vcxproj(projekt)
        except RuntimeError as exc:
            soll, soll_zusatz = None, set()
            pruefe(False, "Schalter der Projektdatei sind eindeutig lesbar", str(exc))
        einheiten = schalter_aus_tlog(tlog)
        try:
            quellen = _kernquellen_aus_cmake()
        except RuntimeError as exc:
            quellen = []
            pruefe(False, "NAKAMA_KERN_QUELLEN ist lesbar", str(exc))

        # (1) Die TU-Menge selbst - drei Mengen, paarweise gleich.
        klagen, aktuell, veraltet = tu_mengen_abgleich(einheiten, quellen, mitglieder)
        hinweise = list(klagen)
        if veraltet:
            hinweise.append(
                (f"{len(veraltet)} veralteter Tlog-Eintrag: " if len(veraltet) == 1
                 else f"{len(veraltet)} veraltete Tlog-Eintraege: ") + ", ".join(veraltet))
        pruefe(bool(quellen) and not klagen,
               f"Tlog, NAKAMA_KERN_QUELLEN und Archiv nennen dieselben "
               f"{len(aktuell)} Uebersetzungseinheiten",
               " | ".join(hinweise))

        # (2) Die Schalter, mit denen genau diese Einheiten uebersetzt wurden.
        if soll is not None:
            schalterklagen = schalter_abgleich(soll, soll_zusatz, aktuell)
            umfang = ", ".join(f"{KLASSENNAME[k]} {len(soll[k])}" for k in SCHALTERKLASSEN)
            pruefe(bool(aktuell) and not schalterklagen,
                   f"jede der {len(aktuell)} gebauten TUs traegt exakt die heutigen "
                   f"Schalter der Projektdatei ({umfang}; "
                   f"{len(soll_zusatz)} AdditionalOptions-Token enthalten)",
                   " | ".join(schalterklagen) if schalterklagen
                   else ("keine gebaute Uebersetzungseinheit" if not aktuell else ""))

        # (3) Und wurde nach der letzten Uebersetzung auch gelinkt?
        objekte = sorted(p for p in tlog.parent.parent.rglob("*.obj") if p.is_file())
        linkklagen = linkfrische(kern, objekte, tlog)
        pruefe(not linkklagen,
               f"NakamaKern.lib ist nicht aelter als die {len(objekte)} Objekte "
               f"ihres Bauverzeichnisses und nicht aelter als ihr Tlog",
               " | ".join(linkklagen))

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
