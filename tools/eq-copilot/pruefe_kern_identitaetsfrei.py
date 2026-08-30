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
zusaetzlich ueber das GEBAUTE EQ-Copilot-Bundle - dort aber nur mit der
TEILMENGE der Nadeln, die in genau diesem Bundle stehen MUSS: der
Pflichtmenge aus Abschnitt [1], also hersteller.name, main.produktname,
main.plugin_code, main.component_cid und main.controller_cid. Die Werte der
Sonden-Ziele (Suna/Probeeq) werden dort ausdruecklich NICHT erwartet - sie
gehoeren in deren eigene Bundles, und dieses Bein prueft sie dort nicht;
hersteller.code und main.bundle stehen ebenfalls nicht in der Pflichtmenge.
Findet der Scanner die Pflichtmenge dort nicht, faellt dieses Bein - sein
Schweigen ueber den Kern waere sonst wertlos. Gegen den KERN dagegen laeuft
in Abschnitt [2] JEDE Nadel aus der Identitaetsdatei; ihre Anzahl gibt der
Lauf in der Zeile "Nadeln    :" aus, sie steht nicht in diesem Kopf.

DRITTE PRUEFUNG - Bauform statt Inhalt: der Kern darf keine JUCE-Modulobjekte
enthalten. Die Kopf-Fassade (cmake/NakamaKern.cmake) haelt die JUCE-.cpp
draussen; das laesst sich am Archiv direkt ablesen, weil eine .lib ihre
Mitgliedsnamen im Klartext fuehrt. Waere die Fassade kaputt, laege hier
juce_core.obj neben NakamaState.obj - und der Kern haette zwei Wahrheiten
ueber denselben Code.

FRISCHE - seit Runde 5 (29.08.2026) wird sie nicht mehr NACHGEBAUT, sondern
HERGESTELLT. Vier Runden lang hat dieses Bein MSBuilds Frischeentscheidung
nachgebildet, und jede Runde fand eine weitere Eingabeklasse, die der Nachbau
nicht sah - zuletzt ein ENTFERNTES AdditionalOptions-Token und die 448 externen
Header, die keine lokale mtime-Wache je erreicht. Solange dieses Bein die
Entscheidung nachbaut, ist die naechste Klasse nur noch nicht gefunden.

Stattdessen loescht Abschnitt [0] vor jeder Messung die Objekte, die Tlogs und
die Lib des Kernverzeichnisses und laesst MSBuild sie aus der heutigen
Projektdatei neu erzeugen (`cmake --build <bau> --config Release --target
NakamaKern`, dabei laeuft ueber ZERO_CHECK auch das Configure samt K2/K2b/K2c
mit). Gemessen wird danach das Artefakt, das gerade entstanden ist - eine
"veraltete Lib" gibt es nicht mehr zu erkennen. Ohne Neubau (`--nur-messen`)
gibt es kein gruenes Frische-Urteil: der Lauf endet ueber
`voraussetzung_exit()` - OHNE registrierten Befund mit 3, MIT registriertem
Befund mit 2, NIE mit 0.

Die frueheren Frischewachen bleiben als DIAGNOSE erhalten und beantworten ab
jetzt die Frage "WOMIT wurde gebaut": configure_frische, die vier
Schalterklassen beidseitig (AdditionalOptions ausdruecklich nur auf
Enthaltensein), tu_mengen_abgleich und linkfrische.

K1b, DER TLOG-ORTSRIEGEL UND DER JUCE-BAUM-RIEGEL kamen in derselben Runde
dazu, weil K1 nur Anfang und Ende einer TU sieht:

  K1b - kein JucePlugin_-Token im Quelltext der TATSAECHLICHEN
        Compiler-Eingaben (aus dem frisch geschriebenen CL.read.1.tlog, also
        inklusive /FI und vorkompilierter Koepfe) plus der literalen
        Include-Huelle als Gegenprobe. Gescannt wird seit Runde 7 JEDE gelesene
        Datei ausser denen aus den JUCE-Modulen und den Toolchain-/SDK-Wurzeln
        - also plugin/** und alles Uebrige. Einzige Ausnahme:
        NakamaKernRiegel.h - und die ist seit Runde 15 keine Freistellung,
        sondern ein ABGLEICH: jedes JucePlugin_-Token dieser Datei muss
        namentlich in der Makroliste stehen, die der Praeprozessor in
        DERSELBEN Datei abfragt, und in einem der Riegelkontexte
        (`defined (...)` im `#if`-Kranz, `#ifdef`/`#ifndef`, das blosse
        Praefix im `#error`-Fliesstext). Jeder andere Name und jeder
        bekannte Name im falschen Kontext - auch ein `#undef` - ist ROT und
        wird beim Namen genannt.
  Tlog-Ortsriegel - JEDE vom Compiler gelesene Datei stammt aus einer
        erlaubten, aus dem Bau ABGELEITETEN Wurzel oder ist eine namentlich
        erlaubte Systemdatei; juce_audio_plugin_client (dort liegen alle
        `#define JucePlugin_` der JUCE-Module), generierte
        JuceLibraryCode-Header und alles Unbekannte sind ROT. Ohne
        Endungsausnahme (Runde 6). Seit Runde 7 ist %SystemRoot% KEINE
        erlaubte Wurzel mehr: erlaubt sind dort nur die gemessenen Dateinamen
        (SYSTEMDATEIEN), jede andere Datei darunter ist ROT.
  JUCE-Baum-Riegel - juce-src ist der gepinnte Tag plus genau der eine
        Nakama-VST3-Patch. Seit Runde 6 drei Zeilen: HEAD IST der Commit des
        Tags (verglichen, nicht nur beschrieben); ausser den Patchdateien ist
        nichts geaendert, unverfolgt oder IGNORIERT; und die Patchdateien
        tragen genau den Patch - Inhalt gegen den in einem temporaeren Index
        gerechneten Sollzustand "Tag + Patch". Reine Loeschungen ausserhalb
        modules/** bleiben geduldet und werden benannt. Das schliesst den
        zuvor bewusst offen gelassenen Weg W8.

AUSDRUECKLICH NICHT BEHAUPTET: der Inhalt der Toolchain- und SDK-Header
ausserhalb des Repos (nur ihre Herkunft aus den abgeleiteten Wurzeln wird
geprueft, kein Fingerprint) - die namentlich erlaubten Systemdateien dagegen
durchsucht K1b seit Runde 7 roh nach dem Token -, ein Compilerwechsel
innerhalb derselben lastbuildstate-Kennung, und der Inhalt von Dateien, die
im JUCE-Baum ausserhalb modules/** GELOESCHT sind (sie werden benannt; eine
geloeschte Datei kann keine Compiler-Eingabe werden).

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py [bauverzeichnis]
    py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py --nur-messen
    py -3.13 tools/eq-copilot/pruefe_kern_identitaetsfrei.py --selbsttest

Exitcodes: 0 gruen · 2 rot · 3 Voraussetzung fehlt (Neubau nicht moeglich,
Configure veraltet, kein schreibbares temporaeres Verzeichnis fuer den
Sollindex des JUCE-Baums, oder --nur-messen - dann ist ueber die Frische
nichts gemessen und nichts behauptet) - und zwar OHNE registrierten Befund;
MIT registriertem Befund wird aus derselben 3 eine 2, siehe den Absatz
darunter.

Ein bereits registrierter Befund GEWINNT gegen jede fehlende Voraussetzung:
war vor dem Abbruch schon etwas rot, endet der Lauf mit 2 statt 3, und die
Klartextzeile "VORAUSSETZUNG: ..." bleibt zusaetzlich stehen. Fuer JEDEN
dieser Ausgaenge - den unmoeglichen oder fehlgeschlagenen Bau (F13)
eingeschlossen - gilt derselbe Satz: OHNE registrierten Befund 3, MIT
registriertem Befund 2, NIE 0 (Matrix F13/F14/F15, Runde 8/9). Die Zusage
gilt fuer jeden Aufruf von main(), nicht nur fuer einen frischen Prozess.
Jeder Voraussetzungs-Rueckweg des Beins geht dafuer durch
`voraussetzung_exit()`; ein Ausgang, der daran vorbeikaeme, waere ein Befund.
"""

from __future__ import annotations

import contextlib
import hashlib
import io
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import time

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

# ── SONDE-007a Runde 5 (sechster Pruefer zu S8, 29.08.2026) ─────────────────
#
# Runde 4 hat den Nachbau der MSBuild-Frischeentscheidung um vier
# Schalterklassen erweitert. Runde 5 fand die naechste Klasse (ein ENTFERNTES
# AdditionalOptions-Token, das im alten Tlog stehen bleibt) und die
# uebernaechste (externe Header: 460 gelesene Dateien, 12 lokal bewacht). Das
# ist die dritte Schicht derselben Frage - und solange dieses Bein MSBuilds
# Entscheidung NACHBAUT, ist die naechste Eingabeklasse nur noch nicht
# gefunden.
#
# 🔑 Der Weg wechselt deshalb: Frische wird nicht mehr nachgebaut, sondern
#    HERGESTELLT. Vor jeder Messung loescht dieses Bein die Objekte, die Tlogs
#    und die Lib des Kernverzeichnisses und laesst MSBuild sie aus der heutigen
#    Projektdatei neu erzeugen. Danach gibt es keine "veraltete Lib" mehr, die
#    man erkennen muesste - es gibt nur die, die gerade entstanden ist.
#    Gegenstandslos werden damit auf einen Schlag: Defines, Includepfade, /FI,
#    /std:, AdditionalOptions in BEIDE Richtungen, lokale und externe Header,
#    geloeschte oder juengere .obj/.lib, jede weitere ClCompile-Eigenschaft
#    (Optimization, RuntimeLibrary, PrecompiledHeader, ... und TU-Overrides),
#    manipulierte oder unvollstaendige Tlogs und der Toolchainwechsel.
#
# ⚠️ GEMESSEN, nicht angenommen (29.08.2026): `cmake --build <bau> --config
#    Release --target NakamaKern --clean-first` ist der FALSCHE Weg.
#    --clean-first cleant die GANZE Solution, nicht das genannte Ziel: nach
#    einem einzigen Lauf waren 29 Bundle-Artefakte geloescht - alle .vst3, alle
#    Testbinaries, flatc.exe, die vier vst3_helper.exe. Ein Beweislauf haette
#    danach an jedem anderen Bein gehangen. Deshalb loescht dieses Bein GEZIELT
#    NakamaKern.dir/<konfig>/* und die NakamaKern.lib derselben Konfiguration
#    und baut danach `--target NakamaKern`.
#
# Was von den alten Wachen bleibt, bleibt als DIAGNOSE: configure_frische, die
# vier Schalterklassen, tu_mengen_abgleich und linkfrische beantworten ab jetzt
# die Frage "WOMIT wurde gebaut", nicht mehr "ist es frisch". Ihre Ausgabetexte
# sagen das; die AdditionalOptions-Klasse nennt sich ausdruecklich
# "Enthaltensein", weil sie nur das prueft (Prueflistenregel E).

JUCE_PATCH = WURZEL / "third_party" / "patches" / "juce-8.0.9-nakama-vst3-bridge.patch"
# Der gepinnte JUCE-Tag. Er steht hier NEBEN dem Patchnamen und wird gegen ihn
# geprueft (Runde 6): ein Patch fuer einen anderen Tag darf nicht still gegen
# diesen Commit gemessen werden.
JUCE_TAG = "8.0.9"
K1B_AUSNAHME = KERNQUELLEN / "state" / "NakamaKernRiegel.h"
_K1B_TOKEN = "JucePlugin_"
# Die Probe des fuenfzehnten Pruefers, woertlich: definieren, benutzen,
# wieder entfernen - unter einem Namen, den K1 nicht kennt. Sie steht hier
# neben der Ausnahme, damit Selbsttest und Bruchtreiber DIESELBEN Bytes
# anhaengen (Runde 15).
_K1B_FREMDPROBE = (b"\n#define JucePlugin_Fremd 1\n"
                   b"#if JucePlugin_Fremd\n#endif\n"
                   b"#undef JucePlugin_Fremd\n")
# Dieselbe Frage in der Kurzform: eine BENUTZUNG darf keinen Namen
# legitimieren, sonst traegt sich jedes Makro selbst in die Liste ein.
_K1B_KURZPROBE = b"\n#ifdef JucePlugin_Fremd\n#endif\n"
_WINKITS_SCHLUESSEL = os.sep.join(
    ["SOFTWARE", "Microsoft", "Windows Kits", "Installed Roots"])

# BEFUND P1, Runde 7 (29.08.2026): bis Runde 6 war das GANZE Windows-System-
# verzeichnis eine erlaubte WURZEL. Gemessen am echten Baum ergab ein
# `/FI C:\Windows\Tracing\nakama-forced.h` - ein Kopf, der JucePlugin_IsSynth
# definiert und wieder entfernt - einen vollstaendig GRUENEN Lauf: der
# Ortsriegel zaehlte ihn unter "Windows-System", und K1b sah ihn nicht, weil
# K1b nur plugin/** uebernahm. Ein Verzeichnis unter %SystemRoot%, in das ein
# normaler Benutzer schreiben darf, war damit ein offener Weg an K1, K1b, K2
# und K3 vorbei.
#
# Deshalb sind Systemdateien seit Runde 7 NAMENTLICH erlaubt, nicht ueber
# ihren Ort.
#
# BEFUND P2, Runde 8 (29.08.2026): bis dahin berief sich diese Liste auf die
# Probe P5-W5b, deren eingefuegte Rohausgabe die Namen dieser Liste gar nicht
# zeigt - eine Behauptung ohne Rohausgabe (Manifest §2). Die Liste steht
# seither auf einer eingefuegten Messung. Gemessen mit dem MSBuild-FileTracker
# ueber eine Wegwerf-Uebersetzungseinheit unter %TEMP% (Befehl, Stand und
# vollstaendige Rohausgabe: Manifest SONDE-007a, Abschnitt "Nacharbeit
# Runde 8", Probe P8-SYS):
#
#     Tracker.exe /if diag /r probe.cpp /c cl.exe /c /W4 /nologo /Foprobe.obj probe.cpp
#     -> cl.read.1.tlog, 125 Zeilen, davon unter %SystemRoot%:
#        C:\WINDOWS\GLOBALIZATION\SORTING\SORTDEFAULT.NLS
#        C:\WINDOWS\SYSTEM32\TZRES.DLL
#
# Die Namen dieser Liste erscheinen dort, und KEIN weiterer unter %SystemRoot%.
# Gemessen wurde ausserdem, was die Zeile "Diagnosefall" frueher zu eng sagte:
# dieselben Dateien stehen auch im Protokoll einer TU OHNE jede Diagnose und
# mit /utf-8 /bigobj. WOVON der Zugriff ausgeloest wird, ist damit NICHT
# gemessen und wird hier nicht behauptet - P8-SYS zeigt dieselben Dateien mit
# und ohne Diagnose, nicht ihre Ursache. Das Leseprotokoll des KERNS nennt
# dagegen keine Datei unter %SystemRoot%
# (`Windows-System 0`); die Liste deckt damit einen Fall ab, den der Kanon
# heute nicht erreicht, und ist genau deshalb eng zu halten.
#
# Jede andere Datei unter %SystemRoot% ist ROT und wird namentlich genannt.
# Faellt hier je eine weitere Systemdatei auf, ist der Weg: die Tlog-Zeile
# MESSEN, ihre Rohausgabe ins Manifest einfuegen, dann den Namen hier
# nachtragen - in dieser Reihenfolge. Eine Wurzel oder ein Praefix kaeme nie
# wieder zurueck.
SYSTEMDATEIEN = (
    os.path.join("System32", "tzres.dll"),
    os.path.join("Globalization", "Sorting", "sortdefault.nls"),
)

# K1b nimmt seit Runde 7 JEDE Datei aus dem Leseprotokoll ausser denen aus
# diesen drei Wurzeln - also plugin/** UND alles Uebrige. Die Namen sind die
# Schluessel aus erlaubte_leseorte(); fehlt einer, bildet K1b seine Menge
# nicht, sondern klagt (fail-closed).
K1B_AUSSCHLUSS_WURZELN = ("juce-src/modules", "MSVC-Toolset", "Windows-SDK")


class VoraussetzungFehlt(RuntimeError):
    """Eine Voraussetzung des Beins fehlt - nie ein gruenes Urteil ueber den Kern.

    BEFUND P2, Runde 7 (29.08.2026): der Sollindex-Vergleich des
    JUCE-Baum-Riegels braucht ein schreibbares temporaeres Verzeichnis. War
    keines da, flog `FileNotFoundError` aus `TemporaryDirectory()` bis nach
    oben durch: Traceback und Exit 1 - ein Ausgang, den weder der Runner noch
    ein Leser als "Voraussetzung fehlt" erkennen konnte. Ein Bein, das seine
    Eingaben nicht bekommt, sagt das und behauptet nichts.

    Der Exitcode dafuer kommt ausschliesslich aus `voraussetzung_exit()`:
    OHNE registrierten Befund 3, MIT registriertem Befund 2, NIE 0 - fuer
    JEDEN Voraussetzungs-Ausgang, den unmoeglichen oder fehlgeschlagenen Bau
    eingeschlossen (Matrix F13/F14/F15, Runde 8/9). Diese Zeile sagte bis
    Runde 10 nur "Exit 3" und war damit an derselben Stelle zu eng wie der
    Skriptkopf.
    """


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


# ⚠️ GEMESSEN (Probe P5-W5b, 29.08.2026): NakamaKern.dir/<konfig>/ enthaelt
#    nicht nur AUSGABEN. CMake legt dort auch GENERIERTE EINGABEN ab - bei
#    aktivem target_precompile_headers etwa cmake_pch.hxx, geschrieben vom
#    Generate-Schritt, nicht vom Bau. Ein pauschales "alles loeschen" machte
#    den Baum unbaubar (error C1083: cmake_pch.hxx not found). Geloescht wird
#    deshalb nach Endung, und was weder als Ausgabe noch als bekannte Eingabe
#    erkannt wird, ist eine KLAGE - nicht stillschweigend behalten.
_NEUBAU_AUSGABEN = frozenset({
    ".obj", ".lib", ".pdb", ".idb", ".ilk", ".pch", ".res", ".exp", ".tlog",
    ".lastbuildstate", ".log", ".recipe", ".iobj", ".ipdb", ".metagen",
})
_NEUBAU_EINGABEN = frozenset({
    ".hxx", ".hpp", ".h", ".cxx", ".cpp", ".c", ".rsp", ".txt", ".props", ".rc",
})


def _kernartefakte(bau: pathlib.Path, konfig: str) -> tuple[list[pathlib.Path], list[pathlib.Path]]:
    """Die Verzeichnisse und Libs, die der Neubau vorher entfernt."""
    kerndirs = sorted(p for p in bau.glob("**/NakamaKern.dir/" + konfig) if p.is_dir())
    libs = sorted(p for p in bau.glob("plugin/**/NakamaKern.lib")
                  if p.is_file() and p.parent.name.lower() == konfig.lower())
    return kerndirs, libs


def kern_neubau(bau: pathlib.Path, konfig: str = "Release",
                cmake: pathlib.Path | None = None) -> dict:
    """Loescht die Kernartefakte und laesst MSBuild sie vollstaendig neu erzeugen.

    Gibt ein Protokoll zurueck. `ok` ist nur dann True, wenn wirklich gebaut
    wurde; jeder andere Ausgang ist eine fehlende Voraussetzung, nie ein
    Urteil ueber den Kern. Den Exitcode dafuer gibt ausschliesslich
    `voraussetzung_exit()`: OHNE registrierten Befund 3, MIT registriertem
    Befund 2, NIE 0 (Matrix F13/F14/F15, Runde 8/9).

    Die Reihenfolge ist Absicht: erst wird geprueft, ob ueberhaupt gebaut
    werden KANN, dann erst geloescht. Ein Bein, das die Artefakte entfernt und
    danach feststellt, dass es kein cmake gibt, haette den Baum kaputt gemacht,
    um nichts zu messen.
    """
    protokoll = {"ok": False, "grund": "", "ausgabe": "", "sekunden": 0.0,
                 "marke_ns": 0, "geloescht": 0, "befehl": "",
                 "behalten": [], "unbekannt": []}

    if cmake is None:
        cmake = finde_cmake()
    if cmake is None:
        protokoll["grund"] = ("cmake nicht gefunden - weder im PATH noch unter Visual "
                              "Studio 2022; ohne Neubau gibt es kein Frische-Urteil")
        return protokoll
    # Auch ein AUSDRUECKLICH uebergebener Pfad wird geprueft, nicht nur der
    # selbst gesuchte: sonst loescht dieses Bein die Artefakte und stellt erst
    # danach fest, dass es sie nicht wieder erzeugen kann (Selbsttest R5-3a).
    if not cmake.is_file():
        protokoll["grund"] = (f"cmake nicht ausfuehrbar: {cmake}; ohne Neubau gibt es "
                              f"kein Frische-Urteil")
        return protokoll
    if not bau.is_dir():
        protokoll["grund"] = f"{_kurz(bau)} ist kein Verzeichnis - nicht konfiguriert"
        return protokoll

    # Zeitanker auf DEMSELBEN Dateisystem wie die Bauartefakte. Ein
    # time.time() waere eine Annahme ueber Uhr und Dateisystem; eine Datei ist
    # eine Messung.
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
    behalten: list[str] = []
    unbekannt: list[str] = []
    try:
        for kerndir in kerndirs:
            for pfad in sorted(kerndir.rglob("*"), reverse=True):
                if not pfad.is_file():
                    continue
                endung = pfad.suffix.lower()
                # Alles IM .tlog-Verzeichnis ist MSBuilds eigene Buchhaltung -
                # auch die endungslose Marke `unsuccessfulbuild`, die ein
                # abgebrochener Bau hinterlaesst (gemessen bei P5-W5b).
                if pfad.parent.suffix.lower() == ".tlog" or endung in _NEUBAU_AUSGABEN:
                    pfad.unlink()
                    geloescht += 1
                    continue
                behalten.append(pfad.name)
                if endung not in _NEUBAU_EINGABEN:
                    unbekannt.append(str(pfad.relative_to(kerndir)))
        for lib in libs:
            lib.unlink()
            geloescht += 1
    except OSError as exc:
        protokoll["grund"] = f"Kernartefakt nicht loeschbar: {exc}"
        return protokoll
    protokoll["geloescht"] = geloescht
    protokoll["behalten"] = sorted(behalten)
    protokoll["unbekannt"] = sorted(unbekannt)

    argumente = [str(cmake), "--build", str(bau), "--config", konfig,
                 "--target", "NakamaKern"]
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
    """Hat der Neubau wirklich jede Kern-TU uebersetzt und neu gelinkt?

    Drei Belege, alle fail-closed: die Bauausgabe nennt jede Quelle, jedes
    erwartete Objekt und die Lib sind juenger als der Zeitanker, und die Anzahl
    der Objekte stimmt mit der Quellmenge ueberein. Ein Bau, der nichts zu tun
    fand, faellt hier auf - genau das darf nach dem Loeschen nicht vorkommen.
    """
    klagen: list[str] = []
    marke = protokoll.get("marke_ns", 0)
    ausgabe = protokoll.get("ausgabe", "")

    unbekannt = protokoll.get("unbekannt") or []
    if unbekannt:
        klagen.append("Datei im Kernverzeichnis, die weder als Bauausgabe noch als "
                      "bekannte Eingabe erkannt wird: " + ", ".join(unbekannt))

    fehlend = [q.name for q in quellen if q.name not in ausgabe]
    if fehlend:
        klagen.append("Bauausgabe nennt diese Uebersetzungseinheiten nicht: "
                      + ", ".join(sorted(fehlend)))

    if not objekte:
        klagen.append("nach dem Neubau liegt kein einziges Objekt im Kernverzeichnis")
    alt = sorted(o.name for o in objekte if o.stat().st_mtime_ns < marke)
    if alt:
        klagen.append("Objekt aelter als der Neubau (nicht neu uebersetzt): "
                      + ", ".join(alt))
    if len(objekte) != len(quellen):
        klagen.append(f"{len(objekte)} Objekte, aber {len(quellen)} Kernquellen")

    if not lib.is_file():
        klagen.append("NakamaKern.lib fehlt nach dem Neubau")
    elif lib.stat().st_mtime_ns < marke:
        klagen.append("NakamaKern.lib ist aelter als der Neubau - nicht neu gelinkt")
    return klagen


# ── Der Tlog-Ortsriegel (W6/W7) ─────────────────────────────────────────────
#
# Das frisch geschriebene CL.read.1.tlog ist die einzige Stelle, die sagt,
# welche Dateien der Compiler in diesem Lauf WIRKLICH gelesen hat - inklusive
# der Header aus /FI und aus vorkompilierten Koepfen, die in keiner literalen
# Include-Huelle stehen. Geprueft wird eine ERLAUBNISLISTE, keine
# Verbotsliste: "kein juce_audio_plugin_client" waere wieder der Nachbau der
# Frage nach der naechsten unbekannten Klasse.
#
# Die Toolchainwurzeln werden ABGELEITET, nicht eingetragen: die MSVC-Wurzel
# aus dem Compilerpfad DIESES Bauverzeichnisses, die SDK-Wurzel aus KitsRoot10
# plus der TargetPlatformVersion aus NakamaKern.lastbuildstate. Laesst sich
# eine Wurzel nicht ableiten, ist das ROT - ein hart eingetragener Pfad waere
# an einem anderen Rechner still falsch.


def _normpfad(pfad) -> str:
    return os.path.normcase(os.path.normpath(str(pfad)))


def _unter(pfad: str, wurzel: str) -> bool:
    return pfad == wurzel or pfad.startswith(wurzel + os.sep)


def lastbuildstate_lesen(tlogdir: pathlib.Path) -> tuple[dict, str, list[str]]:
    """Toolset, VCToolsVersion und TargetPlatformVersion - reine Diagnose.

    Sie sagt, WOMIT gebaut wurde. Ein Urteil ueber Frische leitet dieses Bein
    daraus nicht mehr ab (das erledigt der Neubau); eine fehlende oder
    unlesbare Datei ist trotzdem eine Klage und kein stilles Ja.
    """
    datei = tlogdir / "NakamaKern.lastbuildstate"
    if not datei.is_file():
        return {}, "", [f"{_kurz(datei)} fehlt - womit gebaut wurde ist nicht ablesbar"]
    roh = datei.read_text(encoding="utf-8", errors="replace").strip()
    erste = roh.splitlines()[0] if roh.splitlines() else ""
    werte = {}
    for stueck in erste.split(":"):
        if "=" in stueck:
            schluessel, _, wert = stueck.partition("=")
            werte[schluessel.strip()] = wert.strip()
    fehlend = [s for s in ("PlatformToolSet", "VCToolsVersion", "TargetPlatformVersion")
               if s not in werte]
    klagen = ([f"{_kurz(datei)} nennt {', '.join(fehlend)} nicht"] if fehlend else [])
    return werte, roh, klagen


def _msvc_wurzel(bau: pathlib.Path) -> tuple[pathlib.Path | None, str]:
    """Toolsetwurzel aus dem Compilerpfad DIESES Bauverzeichnisses."""
    treffer = sorted(bau.glob("CMakeFiles/*/CMakeCXXCompiler.cmake"))
    if not treffer:
        return None, ("CMakeFiles/*/CMakeCXXCompiler.cmake fehlt - der Compilerpfad "
                      "dieses Bauverzeichnisses ist nicht ableitbar")
    text = treffer[-1].read_text(encoding="utf-8", errors="replace")
    passung = re.search(r'set\(CMAKE_CXX_COMPILER\s+"([^"]+)"', text)
    if not passung:
        return None, f"{_kurz(treffer[-1])} nennt CMAKE_CXX_COMPILER nicht"
    cl = pathlib.Path(passung.group(1))
    # <...>/VC/Tools/MSVC/<version>/bin/Host<arch>/<arch>/cl.exe
    if len(cl.parents) < 4:
        return None, f"Compilerpfad zu kurz fuer eine Toolsetwurzel: {cl}"
    wurzel = cl.parents[3]
    if not wurzel.is_dir():
        return None, f"abgeleitete MSVC-Wurzel existiert nicht: {wurzel}"
    return wurzel, ""


def _sdk_wurzel(sdk_version: str) -> tuple[pathlib.Path | None, str]:
    """Windows-SDK-Includewurzel aus KitsRoot10 plus TargetPlatformVersion."""
    if not sdk_version:
        return None, "TargetPlatformVersion unbekannt - SDK-Wurzel nicht ableitbar"
    try:
        import winreg
    except ImportError:
        return None, "winreg nicht verfuegbar - SDK-Wurzel nicht ableitbar"
    try:
        with winreg.OpenKey(winreg.HKEY_LOCAL_MACHINE, _WINKITS_SCHLUESSEL) as schluessel:
            stamm = pathlib.Path(winreg.QueryValueEx(schluessel, "KitsRoot10")[0])
    except OSError as exc:
        return None, f"KitsRoot10 nicht lesbar: {exc}"
    wurzel = stamm / "Include" / sdk_version
    if not wurzel.is_dir():
        return None, f"abgeleitete SDK-Wurzel existiert nicht: {wurzel}"
    return wurzel, ""


def _systemwurzel() -> tuple[pathlib.Path | None, str]:
    """Windows-Systemverzeichnis aus der UMGEBUNG (%SystemRoot%).

    Der Compiler liest Systemdateien, die kein Uebersetzungsstoff sind -
    GEMESSEN (Probe P8-SYS, Runde 8, 29.08.2026): System32/tzres.dll und
    Globalization/Sorting/sortdefault.nls stehen roh in einem CL.read.1.tlog,
    das ueber den MSBuild-FileTracker an einer Wegwerf-TU unter %TEMP%
    entstand - in der TU MIT Diagnose ebenso wie in der Gegenprobe OHNE jede
    Diagnose. WOVON der Zugriff ausgeloest wird, ist nicht gemessen und wird
    hier nicht behauptet. Befehl, Stand und vollstaendige Rohausgabe: Manifest
    SONDE-007a, Abschnitt "Nacharbeit Runde 8", Befund 3.
    Seit Befund B1 (Runde 6) laufen sie durch die
    Erlaubnisliste statt an ihr vorbei; seit Befund P1 (Runde 7) ist diese
    Wurzel aber KEINE Erlaubnis mehr, sondern nur noch der Anker, an dem die
    NAMENTLICH erlaubten Dateien aus SYSTEMDATEIEN haengen. Alles andere
    darunter ist ROT.

    Nicht abgeschrieben: der Pfad kommt aus der Umgebung. Fehlt er, ist das
    eine Klage und kein stiller Verzicht - sonst waere jede Systemdatei
    ploetzlich "unbekannter Ort", oder schlimmer: die Wurzel raten.
    """
    roh = os.environ.get("SystemRoot") or os.environ.get("windir") or ""
    if not roh:
        return None, ("weder %SystemRoot% noch %windir% gesetzt - das "
                      "Windows-Systemverzeichnis ist nicht ableitbar")
    wurzel = pathlib.Path(roh)
    if not wurzel.is_dir():
        return None, f"abgeleitetes Windows-Systemverzeichnis existiert nicht: {wurzel}"
    return wurzel, ""


def erlaubte_leseorte(bau: pathlib.Path, zustand: dict,
                      ) -> tuple[list[tuple[str, pathlib.Path]],
                                 list[tuple[str, pathlib.Path]],
                                 dict[str, str], list[str]]:
    """Die Erlaubnisliste des Ortsriegels - vollstaendig aus dem Bau abgeleitet.

    Rueckgabe: (erlaubte Wurzeln, verbotene Unterwurzeln, namentlich erlaubte
    EINZELDATEIEN, Klagen). Laesst sich eine Wurzel nicht ableiten, ist das
    eine Klage und kein stiller Verzicht - sonst waere eine unbekannte
    Herkunft plotzlich "erlaubt", weil niemand hinsah.

    Die Einzeldateien sind seit Befund P1 (Runde 7) der einzige Weg, auf dem
    etwas unter %SystemRoot% erlaubt ist: normalisierter Pfad -> Anzeigename.
    """
    klagen: list[str] = []
    erlaubt: list[tuple[str, pathlib.Path]] = [("plugin", KERNQUELLEN)]

    juce_module = bau / "_deps" / "juce-src" / "modules"
    if juce_module.is_dir():
        erlaubt.append(("juce-src/modules", juce_module))
    else:
        klagen.append(f"{_kurz(juce_module)} fehlt - die JUCE-Modulwurzel ist nicht "
                      f"ableitbar")
    # Dort liegen (gemessen per grep ueber alle Module) ALLE `#define
    # JucePlugin_` von JUCE: detail/juce_CheckSettingMacros.h,
    # VST3/juce_VST3ModuleInfo.h und juce_audio_plugin_client_AAX.cpp.
    verboten = [("juce_audio_plugin_client", juce_module / "juce_audio_plugin_client")]

    msvc, klage = _msvc_wurzel(bau)
    if msvc is None:
        klagen.append(klage)
    else:
        erwartet = zustand.get("VCToolsVersion", "")
        if erwartet and msvc.name != erwartet:
            klagen.append(f"abgeleitete MSVC-Wurzel {msvc.name} passt nicht zur "
                          f"VCToolsVersion {erwartet} des lastbuildstate")
        erlaubt.append(("MSVC-Toolset", msvc))

    sdk, klage = _sdk_wurzel(zustand.get("TargetPlatformVersion", ""))
    if sdk is None:
        klagen.append(klage)
    else:
        erlaubt.append(("Windows-SDK", sdk))

    # Befund B1 (Runde 6): die Systemdateien laufen durch die Erlaubnisliste
    # statt an ihr vorbei. Befund P1 (Runde 7): sie sind NAMENTLICH erlaubt,
    # nicht ueber ihr Verzeichnis - %SystemRoot% ist nur noch der Anker, an
    # dem die gemessenen Namen aus SYSTEMDATEIEN haengen.
    systemdateien: dict[str, str] = {}
    system, klage = _systemwurzel()
    if system is None:
        klagen.append(klage)
    else:
        for rel in SYSTEMDATEIEN:
            systemdateien[_normpfad(system / rel)] = "Windows-System"
    return erlaubt, verboten, systemdateien, klagen


def tlog_gelesene_dateien(tlog: pathlib.Path) -> tuple[list[str], list[str]]:
    """(Marker, gelesene Dateien) aus einem CL.read.1.tlog - beide roh."""
    text = tlog.read_bytes().decode("utf-16-le", errors="replace").lstrip("﻿")
    marker: list[str] = []
    gelesen: list[str] = []
    for zeile in text.splitlines():
        zeile = zeile.strip()
        if not zeile:
            continue
        if zeile.startswith("^"):
            marker.extend(t for t in zeile[1:].split("|") if t.strip())
        else:
            gelesen.append(zeile)
    return marker, sorted(set(gelesen))


# ⚠️ GEMESSEN (Probe P8-SYS, Runde 8, 29.08.2026): cl.exe nennt im
#    Leseprotokoll auch Dateien, die KEIN Uebersetzungsstoff sind -
#    C:\Windows\System32\tzres.dll und Globalization\Sorting\sortdefault.nls
#    stehen roh im CL.read.1.tlog einer Wegwerf-TU unter %TEMP% - in der TU
#    MIT Diagnose und in der Gegenprobe OHNE jede Diagnose gleichermassen.
#    Wodurch der Zugriff ausgeloest wird, ist NICHT gemessen; die Rohzeilen
#    stehen im Manifest SONDE-007a, Abschnitt "Nacharbeit Runde 8", Befund 3.
#    Eine .dll oder .nls kann keinen Praeprozessorzustand in eine TU tragen;
#    sie am Ort zu messen faerbte den Kanon sporadisch rot, ohne etwas zu
#    bewachen.
#
# BEFUND B1, Runde 6 (29.08.2026): dagegen half frueher eine Liste ORTSFREIER
#    ENDUNGEN (.dll/.nls/.exe/.mui/.dat/.bin), die VOR der Erlaubnisliste
#    uebersprungen wurde. Das war ein Loch, kein Filter: /FI ist an keine
#    Endung gebunden, und gemessen wurde, dass `C:\OUTSIDE\forced.dat` deshalb
#    KEINE Klage ergab, waehrend derselbe Pfad als `.h` ROT war. Eine Endung
#    sagt nichts darueber, ob der Compiler die Datei als Quelltext gelesen hat.
#
#    Deshalb gilt jetzt ohne Ausnahme: JEDE Datei aus dem CL.read.1.tlog laeuft
#    durch die Erlaubnisliste. Damit ist der Riegel fail-closed: was weder in
#    eine Wurzel faellt noch namentlich erlaubt ist, wird genannt.
#
# BEFUND P1, Runde 7 (29.08.2026): das Windows-Systemverzeichnis war dabei
#    eine WURZEL - und damit erlaubt war alles darunter, auch die Verzeichnisse,
#    in die ein normaler Benutzer schreiben darf. Gemessen am echten Baum:
#    `/FI C:\Windows\Tracing\nakama-forced.h` mit `#define JucePlugin_IsSynth`
#    und `#undef` lief GRUEN durch (Windows-System 1), weil der Ortsriegel den
#    Ort erlaubte und K1b nur plugin/** uebernahm.
#
#    Seither sind die Systemdateien NAMENTLICH erlaubt (SYSTEMDATEIEN, aus der
#    Messung), und K1b nimmt jede Datei des Leseprotokolls ausser JUCE-Modulen
#    und Toolchain/SDK. Beide Haelften greifen unabhaengig: der Ortsriegel
#    nennt die fremde Datei, K1b liest ihren Inhalt.


def tlog_ortsriegel(gelesen: list[str], marker: list[str],
                    quellen: list[pathlib.Path],
                    erlaubt: list[tuple[str, pathlib.Path]],
                    verboten: list[tuple[str, pathlib.Path]],
                    systemdateien: dict[str, str] | None = None,
                    ) -> tuple[list[str], dict[str, int]]:
    """Stammt jede gelesene Datei aus einem erlaubten Ort oder ist sie benannt?

    Erlaubnisliste, fail-closed: was in keine Wurzel faellt und in keiner
    Namensliste steht, ist ROT und wird namentlich genannt. `verboten` sind
    Ausnahmen INNERHALB einer erlaubten Wurzel (heute: juce_audio_plugin_client
    unter juce-src/modules) - dort liegen alle `#define JucePlugin_` der
    JUCE-Module.

    Es gibt KEINE Endungsausnahme (Befund B1, Runde 6): auch .dll, .nls, .dat
    oder eine Datei ohne Endung wird gemessen. Und es gibt seit Befund P1
    (Runde 7) keine Ortserlaubnis mehr fuer %SystemRoot%: `systemdateien`
    bildet genau die gemessenen EINZELPFADE ab (normalisiert -> Anzeigename);
    jede andere Datei darunter faellt in keine Wurzel und ist damit ROT.

    Fehlt eine heutige Kernquelle als Marker, ist auch das ROT: Schweigen
    duerfte nie ein Ja sein.
    """
    systemdateien = systemdateien or {}
    klagen: list[str] = []
    zaehlung: dict[str, int] = {name: 0 for name, _ in erlaubt}
    for name in systemdateien.values():
        zaehlung.setdefault(name, 0)

    markernamen = {pathlib.PurePath(m).name.upper() for m in marker}
    fehlende = sorted(q.name for q in quellen if q.name.upper() not in markernamen)
    if fehlende:
        klagen.append("Kernquelle fehlt als Marker im Leseprotokoll: "
                      + ", ".join(fehlende))

    verboten_norm = [(name, _normpfad(pfad)) for name, pfad in verboten]
    erlaubt_norm = [(name, _normpfad(pfad)) for name, pfad in erlaubt]

    for roh in gelesen:
        pfad = _normpfad(roh)
        treffer = None
        for name, wurzel in verboten_norm:
            if _unter(pfad, wurzel):
                klagen.append(f"gelesen aus verbotenem Ort ({name}): {roh}")
                treffer = name
                break
        if treffer is not None:
            continue
        # Namentliche Erlaubnis VOR den Wurzeln: sie ist die engere Aussage
        # und die einzige, die unter %SystemRoot% noch gilt (Befund P1).
        name = systemdateien.get(pfad)
        if name is not None:
            zaehlung[name] += 1
            continue
        for name, wurzel in erlaubt_norm:
            if _unter(pfad, wurzel):
                zaehlung[name] += 1
                treffer = name
                break
        if treffer is None:
            klagen.append(f"gelesen aus unbekanntem Ort: {roh}")
    return klagen, zaehlung


# ── K1b: Quelltext-Token ueber die tatsaechlichen Compiler-Eingaben ─────────
#
# K1 sieht nur, was am ANFANG und am ENDE einer TU definiert ist. Ein Header,
# der JucePlugin_IsSynth definiert, in #if benutzt und vor dem TU-Ende wieder
# entfernt, entgeht ihm - und K3 findet nichts, weil ein #if keine
# Identitaetsbytes hinterlaesst. K1b schliesst genau diese Luecke im Quelltext.
#
# Gescannt wird die VEREINIGUNG aus zwei Mengen, beide werden benannt:
#   1. die Dateien aus dem frisch geschriebenen CL.read.1.tlog, die NICHT aus
#      den JUCE-Modulen und nicht aus den Toolchain-/SDK-Wurzeln stammen - das
#      sind die tatsaechlichen Compiler-Eingaben, also auch /FI und
#      vorkompilierte Koepfe, die in keiner literalen Huelle auftauchen;
#   2. die literale Include-Huelle aus kern_quellabhaengigkeiten() als
#      Gegenprobe (sie faellt fail-closed auf nicht literal aufloesbare oder
#      mehrdeutige Includes).
#
# BEFUND P1, Runde 7 (29.08.2026): Menge 1 war auf plugin/** beschraenkt. Ein
# per /FI erzwungener Kopf ausserhalb - gemessen unter %SystemRoot% - lief
# damit an K1b vorbei, waehrend der Ortsriegel ihn ueber seine Wurzel erlaubte.
# Seither ist die Menge das KOMPLEMENT der drei Ausschlusswurzeln: plugin/**
# und alles Uebrige. Der Ausschluss ist begruendet, nicht bequem - der Inhalt
# der JUCE-Module ist Sache des JUCE-Baum-Riegels, und ueber Toolchain- und
# SDK-Koepfe ausserhalb des Repos behauptet dieses Bein ausdruecklich nichts
# (dort wird nur die Herkunft geprueft). Laesst sich eine dieser drei Wurzeln
# nicht ableiten, bildet K1b seine Menge NICHT, sondern klagt.
#
# Kommentare werden vor dem Scan entfernt, sonst fiele der Riegel an den
# Hinweiszeilen der Kernquellen selbst ("// K1 - keine JucePlugin_*-Konstante
# im Kern"). Stringliterale bleiben im Scan - ein Token darin kostet nichts und
# ein Ueberspringen waere eine Luecke.
#
# Die namentlich erlaubten Systemdateien (SYSTEMDATEIEN) sind Binaerdateien:
# ein C++-Parser ueber sie ist sinnlos und faellt an einem unpaarigen
# Anfuehrungszeichen fail-closed ROT. Sie werden deshalb ROH nach dem Token
# durchsucht - ASCII und UTF-16LE -, nicht uebersprungen.


def ohne_kommentare(text: str, wo: str) -> str:
    """Entfernt // und /* */ und laesst Stringliterale stehen.

    Fail-closed: ein nicht abgeschlossener Blockkommentar ist ROT, nicht
    "Rest ignorieren". Zeilenumbrueche bleiben erhalten, damit die gemeldeten
    Zeilennummern die des Originals sind.
    """
    aus: list[str] = []
    i = 0
    laenge = len(text)
    while i < laenge:
        zeichen = text[i]
        if zeichen in ('"', "'"):
            grenze = zeichen
            aus.append(zeichen)
            i += 1
            geschlossen = False
            while i < laenge:
                if text[i] == "\\" and i + 1 < laenge:
                    aus.append(text[i:i + 2])
                    i += 2
                    continue
                aus.append(text[i])
                if text[i] == grenze:
                    i += 1
                    geschlossen = True
                    break
                i += 1
            if not geschlossen:
                raise RuntimeError(f"nicht abgeschlossenes Literal in {wo}")
            continue
        if text.startswith("//", i):
            while i < laenge and text[i] != "\n":
                i += 1
            continue
        if text.startswith("/*", i):
            ende = text.find("*/", i + 2)
            if ende < 0:
                raise RuntimeError(f"nicht abgeschlossener Blockkommentar in {wo}")
            aus.append("\n" * text.count("\n", i, ende + 2))
            i = ende + 2
            continue
        aus.append(zeichen)
        i += 1
    return "".join(aus)


def k1b_eingaben_aus_tlog(gelesen: list[str],
                          erlaubt: list[tuple[str, pathlib.Path]],
                          ) -> tuple[list[pathlib.Path], list[str]]:
    """Die K1b-Menge aus dem Leseprotokoll: alles ausser JUCE, Toolchain, SDK.

    Fail-closed (Befund P1, Runde 7): fehlt eine der drei Ausschlusswurzeln in
    `erlaubt`, weil sie sich nicht ableiten liess, wird KEINE Menge gebildet.
    Sonst faenden sich hier ploetzlich zehntausend SDK-Koepfe wieder - und ein
    ROT daran waere kein Befund ueber den Kern, sondern Rauschen.
    """
    vorhanden = {name for name, _ in erlaubt}
    fehlend = [n for n in K1B_AUSSCHLUSS_WURZELN if n not in vorhanden]
    if fehlend:
        return [], [f"K1b-Menge nicht bildbar: die Ausschlusswurzel(n) "
                    f"{', '.join(fehlend)} sind nicht abgeleitet"]
    wurzeln = [_normpfad(pfad) for name, pfad in erlaubt
               if name in K1B_AUSSCHLUSS_WURZELN]
    return ([pathlib.Path(p) for p in gelesen
             if not any(_unter(_normpfad(p), w) for w in wurzeln)], [])


# Die Ausnahme fuer NakamaKernRiegel.h ist seit Runde 15 (Befund P1 des
# fuenfzehnten Pruefers) KEINE Freistellung mehr, sondern ein ABGLEICH.
#
# Vorher zaehlte der Zweig nur `roh.count("JucePlugin_")` und uebersprang die
# Datei. Eine Zaehlung sagt nichts ueber Namen: ein zusaetzlicher, unbekannter
# `#define JucePlugin_Fremd` / benutzen / `#undef` in genau dieser Datei blieb
# gruen. K1 kennt den Namen nicht (sein `#if defined`-Kranz ist namentlich),
# K2 sieht keine interne Definition, und K3 kann bei reiner
# Praeprozessor-Nutzung leer bleiben - gemessen als Reproduktion @ 0d5b7d5.
#
# EINE QUELLE, ZWEI VERBRAUCHER: die Makroliste, gegen die abgeglichen wird,
# ist genau die, die der Praeprozessor in dieser Datei abfragt. Sie wird aus
# den `defined (JucePlugin_*)`-Abfragen der `#if`/`#elif`-Zeilen derselben
# Datei gelesen - nicht im Skript nachgepflegt. Damit koennen K1 und K1b nicht
# auseinanderlaufen.
#
# ERLAUBTE KONTEXTE (Riegelmuster dieser Datei):
#   1. `defined (Name)` in einer `#if`/`#elif`-Zeile - die Abfrage selbst.
#      NUR diese Zeilen bilden die Liste; die Kurzform traegt sich NICHT
#      selbst ein, sonst waere jeder Name durch seine eigene Benutzung
#      legitimiert.
#   2. `#ifdef Name` / `#ifndef Name` - dieselbe Abfrage in kurzer Form, aber
#      nur fuer einen Namen, den der `#if`-Kranz schon fuehrt.
#   3. das blosse Praefix `JucePlugin_` ohne folgendes Namenszeichen in einer
#      `#error`-Zeile - der Fliesstext der Riegelmeldung ("JucePlugin_*").
# ROT ist alles andere, NAMENTLICH: ein unbekannter Name (steht nicht in der
# K1-Liste) und jeder bekannte Name im falschen Kontext. `#define` und `#undef`
# sind auch fuer bekannte Namen ROT - ein `#undef JucePlugin_Name` vor dem
# Kranz wuerde K1 fuer genau dieses Makro still entwaffnen.
#
# Kommentare werden vorher entfernt, wie bei jeder anderen Datei auch: was in
# einem Kommentar steht, ist fuer den Uebersetzer kein Token. Die rohe Zahl
# bleibt trotzdem in der Ausgabe stehen - sie ist Diagnose, kein Urteil.
#
# AUSDRUECKLICHE NICHTZUSAGE, in die sichere Richtung: erkannt wird die
# Klammerform `defined (Name)`. Ein `defined Name` ohne Klammern - in C
# ebenfalls gueltig - traegt sich NICHT in die Liste ein; sein Vorkommen ist
# dann ROT ("steht nicht in der K1-Makroliste"), nicht still gruen. Wer die
# Riegeldatei je auf die klammerlose Form umstellt, faellt hier auf und nicht
# irgendwo spaeter.

_K1B_NAME = re.compile(r"JucePlugin_[A-Za-z0-9_]*")
_K1B_ABFRAGE = re.compile(r"\bdefined\s*\(\s*(JucePlugin_[A-Za-z0-9_]+)\s*\)")
_K1B_KURZABFRAGE = re.compile(r"^#\s*ifn?def\s+(JucePlugin_[A-Za-z0-9_]+)\b")
_K1B_DIREKTIVE = re.compile(r"^#\s*([A-Za-z_]+)")


def _logische_zeilenarten(zeilen: list[str]) -> list[str]:
    """Direktivenart je PHYSISCHER Zeile, Backslash-Fortsetzungen mitgezaehlt.

    Der Riegelkranz in NakamaKernRiegel.h ist EINE logische `#if`-Zeile ueber
    46 physische; die `#error`-Meldung ebenso ueber sechs. Ohne diese Abbildung
    saehe eine Zeilenweise-Pruefung `|| defined (...)` als gewoehnlichen Text.
    Rueckgabe ist 0-basiert und so lang wie `zeilen`; "" heisst: keine
    Direktive.
    """
    arten = [""] * len(zeilen)
    i = 0
    while i < len(zeilen):
        start = i
        while i < len(zeilen) and zeilen[i].rstrip().endswith("\\"):
            i += 1
        treffer = _K1B_DIREKTIVE.match(zeilen[start].lstrip())
        art = treffer.group(1) if treffer else ""
        for n in range(start, min(i, len(zeilen) - 1) + 1):
            arten[n] = art
        i += 1
    return arten


def k1b_ausnahme_abgleich(datei: pathlib.Path,
                          roh: str) -> tuple[list[str], dict]:
    """Gleicht JEDES JucePlugin_-Token der Riegeldatei namentlich ab.

    Rueckgabe: (Klagen, Angaben). `Angaben` traegt `roh` (Token im Rohtext,
    Diagnose), `code` (Token im kommentarfreien Quelltext), `makros` (die aus
    DERSELBEN Datei gelesene K1-Liste) und `abgeglichen` (Vorkommen, die
    namentlich und im erlaubten Kontext standen). Fail-closed: unlesbarer
    Quelltext und eine nicht ableitbare Makroliste sind ROT.
    """
    kurz = _kurz(datei)
    angaben = {"roh": roh.count(_K1B_TOKEN), "code": 0,
               "makros": [], "abgeglichen": 0}
    try:
        text = ohne_kommentare(roh, kurz)
    except RuntimeError as exc:
        return [str(exc)], angaben

    zeilen = text.splitlines()
    arten = _logische_zeilenarten(zeilen)
    angaben["code"] = text.count(_K1B_TOKEN)

    # Schritt 1: die Liste, die K1 in DIESER Datei abfragt.
    makros: set[str] = set()
    for nr, zeile in enumerate(zeilen):
        if arten[nr] in ("if", "elif"):
            makros.update(_K1B_ABFRAGE.findall(zeile))
    angaben["makros"] = sorted(makros)
    if not makros:
        return ([f"{kurz}: die K1-Makroliste ist aus dieser Datei nicht "
                 f"ableitbar - ohne sie ist die K1b-Ausnahme kein Abgleich"],
                angaben)

    # Schritt 2: jedes Vorkommen gegen Liste UND Kontext.
    klagen: list[str] = []
    for nr, zeile in enumerate(zeilen):
        if _K1B_TOKEN not in zeile:
            continue
        art = arten[nr]
        erlaubt = {t.span(1) for t in _K1B_ABFRAGE.finditer(zeile)
                   if art in ("if", "elif") and t.group(1) in makros}
        kurzform = _K1B_KURZABFRAGE.match(zeile.lstrip())
        if kurzform and kurzform.group(1) in makros:
            versatz = len(zeile) - len(zeile.lstrip())
            erlaubt.add((kurzform.start(1) + versatz,
                         kurzform.end(1) + versatz))
        for treffer in _K1B_NAME.finditer(zeile):
            name = treffer.group(0)
            if treffer.span() in erlaubt:
                angaben["abgeglichen"] += 1
                continue
            if name == _K1B_TOKEN and art == "error":
                angaben["abgeglichen"] += 1     # Fliesstext der Riegelmeldung
                continue
            wo = f"#{art}" if art else "Quelltext ausserhalb einer Direktive"
            grund = ("steht nicht in der K1-Makroliste dieser Datei"
                     if name not in makros
                     else f"steht in der K1-Makroliste, aber in {wo}")
            klagen.append(f"{kurz}: Zeile {nr + 1}: {name!r} {grund} - die "
                          f"K1b-Ausnahme ist ein Abgleich, keine Freistellung")

    # Die eigene Rechnung muss aufgehen: JEDES Vorkommen des Tokens ist
    # entweder abgeglichen oder beklagt. Geht sie nicht auf, hat der Abgleich
    # etwas uebersehen (etwa ineinander geschachtelte Praefixe) - dann ist die
    # Ausnahme ROT, nicht still gruen.
    if not klagen and angaben["abgeglichen"] != angaben["code"]:
        klagen.append(f"{kurz}: {angaben['code']} JucePlugin_-Token im "
                      f"kommentarfreien Quelltext, aber nur "
                      f"{angaben['abgeglichen']} einzeln abgeglichen - der "
                      f"Abgleich deckt die Datei nicht vollstaendig ab")
    return klagen, angaben


def k1b_riegel(dateien: list[pathlib.Path],
               ausnahme: pathlib.Path,
               roh_scannen: dict[str, str] | None = None,
               ) -> tuple[list[str], int, dict, int]:
    """Kein JucePlugin_-Token im Quelltext der Compiler-Eingaben.

    `roh_scannen` sind die namentlich erlaubten Systemdateien (normalisierter
    Pfad -> Anzeigename): Binaerstoff, der roh nach dem Token durchsucht wird,
    statt durch den C++-Kommentarparser zu laufen.

    Rueckgabe: (Klagen, geprueft, Angaben zur benannten Ausnahme, davon roh
    durchsucht). Der letzte Wert zaehlt, was WIRKLICH roh gemessen wurde - die
    Laenge der Erlaubnisliste waere eine groessere Zahl als die Messung
    (Prueflistenregel E). Die Ausnahme wird seit Runde 15 nicht mehr gezaehlt
    und uebersprungen, sondern durch `k1b_ausnahme_abgleich` namentlich gegen
    die K1-Makroliste derselben Datei geprueft; `gesehen` sagt, ob sie in
    dieser Eingabemenge ueberhaupt vorkam.
    """
    roh_scannen = roh_scannen or {}
    klagen: list[str] = []
    ausnahme_norm = _normpfad(ausnahme)
    geprueft = 0
    in_ausnahme = {"gesehen": False, "roh": 0, "code": 0,
                   "makros": [], "abgeglichen": 0}
    roh_geprueft = 0
    for datei in sorted(set(dateien)):
        if _normpfad(datei) in roh_scannen:
            try:
                bytes_ = datei.read_bytes()
            except OSError as exc:
                klagen.append(f"{_kurz(datei)}: nicht lesbar ({exc})")
                continue
            geprueft += 1
            roh_geprueft += 1
            for kodierung in ("ascii", "utf-16-le"):
                if _K1B_TOKEN.encode(kodierung) in bytes_:
                    klagen.append(f"{_kurz(datei)}: {_K1B_TOKEN}-Token in den "
                                  f"Rohbytes ({kodierung})")
            continue
        try:
            roh = datei.read_text(encoding="utf-8", errors="replace")
        except OSError as exc:
            klagen.append(f"{_kurz(datei)}: nicht lesbar ({exc})")
            continue
        if _normpfad(datei) == ausnahme_norm:
            ausnahmeklagen, angaben = k1b_ausnahme_abgleich(datei, roh)
            klagen.extend(ausnahmeklagen)
            in_ausnahme = {"gesehen": True, **angaben}
            continue
        geprueft += 1
        try:
            text = ohne_kommentare(roh, _kurz(datei))
        except RuntimeError as exc:
            klagen.append(str(exc))
            continue
        zeilen = [str(nr) for nr, zeile in enumerate(text.splitlines(), 1)
                  if _K1B_TOKEN in zeile]
        if zeilen:
            klagen.append(f"{_kurz(datei)}: {_K1B_TOKEN}-Token im Quelltext, "
                          f"Zeile(n) {', '.join(zeilen[:8])}")
    return klagen, geprueft, in_ausnahme, roh_geprueft


# ── JUCE-Baum-Riegel (W8) ───────────────────────────────────────────────────
#
# Der einzige Weg, der nach K1b und dem Ortsriegel offen blieb: eine
# MANIPULIERTE Kopie eines JUCE-Modulheaders an einem erlaubten Ort, die eine
# Konstante definiert, benutzt und wieder entfernt. Dagegen half der frueher
# angefuehrte Wrapper-Riegel nicht - der hasht eine einzelne VST3-Datei
# (cmake/NakamaBruecke.cmake), nicht juce_core.
#
# Der Riegel ist deshalb ein Herkunftsnachweis ueber den ganzen Baum: juce-src
# ist der gepinnte Tag plus genau der eine benannte Nakama-Patch. Inhalt und
# Abwesenheit werden unterschieden - eine geloeschte Datei kann keine
# Compiler-Eingabe werden, eine geaenderte oder eine unverfolgte sehr wohl.


def juce_baum_status_pruefen(kennungen: list[tuple[str, str]],
                             patchdateien: set[str]) -> tuple[list[str], list[str]]:
    """Klassifiziert `git status --porcelain`-Zeilen. (Klagen, geduldete Loeschungen)

    ROT ist alles, was Inhalt in den Baum bringen kann: unverfolgte Dateien und
    jede Aenderung ausserhalb der Patchdateien. Reine Loeschungen koennen keine
    Compiler-Eingabe werden; unter modules/** sind sie trotzdem ROT (dort
    stuende sonst ein anderer Header zur Aufloesung bereit), ausserhalb werden
    sie gezaehlt und benannt - nie verschwiegen.

    BEFUND B2 (ii), Runde 6: IGNORIERTE Dateien - Statuscode "!!", nur mit
    `status --ignored` ueberhaupt sichtbar - zaehlen genau wie unverfolgte.
    JUCEs eigene .gitignore deckt u. a. `*.pch` ab; gemessen wurde, dass eine
    untergeschobene `modules/juce_core/NakamaProbe.pch` in `--porcelain -uall`
    gar nicht erschien und erst in `--porcelain --ignored -uall` als "!!". Ein
    vorkompilierter Kopf traegt Makros - ignoriert heisst nicht harmlos.
    """
    klagen: list[str] = []
    loeschungen: list[str] = []
    for kennung, pfad in kennungen:
        buchstaben = set(kennung.replace(" ", ""))
        if kennung in ("??", "!!"):
            art = "unverfolgte" if kennung == "??" else "ignorierte"
            klagen.append(f"{art} Datei im JUCE-Baum: {pfad}")
        elif buchstaben & set("MARCTU"):
            if pfad not in patchdateien:
                klagen.append(f"geaendert, steht aber nicht im Nakama-Patch: {pfad}")
        elif "D" in buchstaben:
            if pfad.startswith("modules/"):
                klagen.append(f"geloeschte Moduldatei im JUCE-Baum: {pfad}")
            else:
                loeschungen.append(pfad)
        else:
            klagen.append(f"unbekannter git-Status {kennung!r} fuer {pfad}")
    return klagen, sorted(loeschungen)


def _patchdateien(patch: pathlib.Path) -> set[str]:
    namen: set[str] = set()
    for zeile in patch.read_text(encoding="utf-8", errors="replace").splitlines():
        passung = re.match(r"^diff --git a/(\S+) b/(\S+)\s*$", zeile)
        if passung:
            namen.add(passung.group(2))
    return namen


def _git(*argumente: str,
         umgebung: dict[str, str] | None = None) -> tuple[int, str, str]:
    """(Exit, stdout, stderr) - GETRENNT.

    BEFUND B3, Runde 6 (29.08.2026): frueher lieferte diese Huelle
    `stdout + stderr` als EINEN Strom. Git schreibt Warnungen aber auf stderr
    und endet trotzdem mit 0 - gemessen mit einem unlesbaren globalen Ignore:

        $ git -c core.excludesFile=C:/pagefile.sys status --porcelain -uall
        warning: unable to access 'C:/pagefile.sys': Permission denied
        warning: unable to access 'C:/pagefile.sys': Permission denied
        ?? .claude/settings.local.json
        exit=0

    Die Warnzeile geriet damit in den Porcelain-Parser, ergab den
    "Statuscode" `wa` und haette einen gueltigen JUCE-Baum ROT gefaerbt (im
    Probelauf: `--nur-messen` endete mit Exit 2 statt 3, also F14 verdeckt).

    Regel seither: bei Exit 0 ist AUSSCHLIESSLICH stdout Datenstrom. stderr
    wird gesondert erfasst und als `hinweis` ausgegeben - nicht verschwiegen,
    aber auch nie als Daten gelesen. Bei Exit != 0 traegt stderr die
    Fehlermeldung und wird dort zitiert.

    `umgebung` ergaenzt die Prozessumgebung (fuer GIT_INDEX_FILE beim
    Sollvergleich); ohne sie wird die eigene unveraendert weitergereicht.
    """
    try:
        lauf = subprocess.run(["git", "--no-optional-locks", *argumente],
                              capture_output=True, text=True, errors="replace",
                              check=False,
                              env=({**os.environ, **umgebung} if umgebung else None))
    except OSError as exc:
        return 127, "", str(exc)
    return lauf.returncode, lauf.stdout, lauf.stderr


def _patch_soll_vergleich(juce: pathlib.Path, patch: pathlib.Path,
                          patchdateien: set[str]) -> tuple[list[str], list[str]]:
    """Tragen die Patchdateien GENAU den Patch? (Klagen, stderr-Hinweise)

    BEFUND B2 (iii), Runde 6: `git apply --check --reverse` prueft nur, ob die
    Hunks rueckwaerts PASSEN - nicht, ob die Datei nur sie traegt. Gemessen am
    echten Baum: eine zusaetzliche Zeile am Dateiende von
    juce_audio_plugin_client_VST3.cpp liess `apply --check --reverse` mit
    Exit 0 schweigen.

    Der dichte Weg vergleicht nicht Hunk-Muster, sondern INHALT gegen den
    gerechneten Sollzustand "Tag + Patch":

        read-tree HEAD          temporaerer Index := gepinnter Commit
        apply --cached <patch>  temporaerer Index := Commit + genau der Patch
        update-index --refresh  Statdaten angleichen (hasht dabei jede Datei,
                                deren Stat unbekannt ist)
        diff-files -- <patchdateien>   nennt jede Abweichung des Arbeitsbaums

    Der Arbeitsindex des JUCE-Baums wird dabei nicht angefasst: GIT_INDEX_FILE
    zeigt auf eine Wegwerfdatei in einem temporaeren Verzeichnis.

    Der Vergleich ist auf die Patchdateien beschraenkt - fuer alles andere ist
    Zeile (ii) zustaendig, die jede Aenderung ausserhalb dieser Menge ohnehin
    ROT meldet.
    """
    klagen: list[str] = []
    hinweise: list[str] = []
    # Befund P2, Runde 7: ohne schreibbares Temp gibt es keinen Sollindex und
    # damit keinen Vergleich - das ist eine fehlende Voraussetzung, kein
    # Traceback und erst recht kein Urteil ueber den JUCE-Baum. Den Exitcode
    # gibt `voraussetzung_exit()`: ohne registrierten Befund 3, mit
    # registriertem Befund 2, nie 0 (Matrix F13/F14/F15).
    try:
        verzeichnis = tempfile.TemporaryDirectory()
    except OSError as exc:
        raise VoraussetzungFehlt(
            "kein schreibbares temporaeres Verzeichnis fuer den Sollindex des "
            f"JUCE-Baums ({exc.__class__.__name__}: {exc}) - ohne ihn ist "
            f"'{JUCE_TAG} + Patch' nicht rechenbar. TMPDIR, TEMP und TMP "
            "pruefen.") from exc
    with verzeichnis as roh:
        umgebung = {"GIT_INDEX_FILE": str(pathlib.Path(roh) / "nakama-soll.index")}
        code, _, err = _git("-C", str(juce), "read-tree", "HEAD", umgebung=umgebung)
        if code != 0:
            return ([f"Sollzustand nicht rechenbar (read-tree Exit {code}): "
                     f"{err.strip()[:200]}"], hinweise)
        code, _, err = _git("-C", str(juce), "apply", "--cached", str(patch),
                            umgebung=umgebung)
        if code != 0:
            return ([f"der Nakama-Patch laesst sich nicht auf den gepinnten Commit "
                     f"legen (apply --cached Exit {code}): {err.strip()[:200]}"],
                    hinweise)
        # Exit und Ausgabe von --refresh sind hier kein Urteil: "needs update"
        # steht auch fuer die geduldeten Loeschungen ausserhalb modules/**.
        # Geurteilt wird ueber diff-files, und zwar nur ueber die Patchdateien.
        _git("-C", str(juce), "update-index", "--refresh", umgebung=umgebung)
        code, ausgabe, err = _git("-c", "core.quotepath=false", "-C", str(juce),
                                  "diff-files", "--name-only",
                                  "--", *sorted(patchdateien), umgebung=umgebung)
        if code != 0:
            return ([f"Sollvergleich fehlgeschlagen (diff-files Exit {code}): "
                     f"{err.strip()[:200]}"], hinweise)
        hinweise.extend(z.strip() for z in err.splitlines() if z.strip())
        for pfad in ausgabe.splitlines():
            if pfad.strip():
                klagen.append("Patchdatei traegt nicht genau den Patch (zusaetzliche "
                              f"Aenderung gegenueber '{JUCE_TAG} + Patch'): {pfad.strip()}")
    return klagen, hinweise


def juce_baum_riegel(bau: pathlib.Path) -> tuple[list[str], list[str], dict]:
    """Ist juce-src der gepinnte Tag plus genau der eine Nakama-Patch?

    Drei Zeilen, seit Befund B2 (Runde 6) alle drei notwendig:

      (i)   HEAD IST der Commit des gepinnten Tags. Frueher wurde nur die
            Ausgabe von `git describe` als Text abgelegt und mit nichts
            verglichen - ein Baum auf einem anderen Tag waere durchgelaufen.
      (ii)  `status --porcelain --ignored -uall`: die geaenderte Menge ist
            genau die Patchdateimenge. Ohne `--ignored` blieb eine von JUCEs
            eigener .gitignore gedeckte Fremddatei unsichtbar.
      (iii) Die Patchdateien tragen GENAU den Patch - Inhalt gegen den
            gerechneten Sollzustand, siehe _patch_soll_vergleich().
            `apply --check --reverse` genuegte dafuer nicht: es prueft nur,
            ob die Hunks rueckwaerts passen, und schwieg zu einer
            zusaetzlichen Zeile ausserhalb ihrer Kontexte. Es ist deshalb
            ERSETZT, nicht ergaenzt - eine schwaechere Doppelwache laedt nur
            dazu ein, ihr zu glauben.

    Bewusst NICHT verschaerft: eine Loeschung ausserhalb modules/** bleibt
    geduldet und benannt (siehe juce_baum_status_pruefen). Eine geloeschte
    Datei kann keine Compiler-Eingabe werden, und die drei Zeilen oben
    schliessen jeden Weg, auf dem INHALT in den Baum kommt.
    """
    info: dict = {"beschreibung": "", "patchdateien": 0, "loeschungen": [],
                  "hinweise": []}
    juce = bau / "_deps" / "juce-src"
    if not juce.is_dir():
        return ([f"{_kurz(juce)} fehlt - der JUCE-Baum ist nicht pruefbar"], [], info)
    if not JUCE_PATCH.is_file():
        return ([f"{_kurz(JUCE_PATCH)} fehlt - ohne den Patch ist der erlaubte "
                 f"Baumzustand nicht definiert"], [], info)

    patchdateien = _patchdateien(JUCE_PATCH)
    info["patchdateien"] = len(patchdateien)
    if not patchdateien:
        return ([f"{_kurz(JUCE_PATCH)} nennt keine Datei - kein pruefbarer Patch"],
                [], info)

    klagen: list[str] = []

    def hinweis(text: str) -> None:
        """stderr wird gesagt, nie als Daten gelesen (Befund B3)."""
        info["hinweise"].extend(z.strip() for z in text.splitlines() if z.strip())

    # Der gepinnte Tag muss zum Patchnamen passen - sonst messen wir einen
    # fremden Patch gegen diesen Commit.
    if JUCE_TAG not in JUCE_PATCH.name:
        klagen.append(f"gepinnter Tag {JUCE_TAG} kommt im Patchnamen "
                      f"{JUCE_PATCH.name} nicht vor")

    # -- (i) HEAD ist der Commit des gepinnten Tags -------------------------
    code_kopf, kopf, err_kopf = _git("-C", str(juce), "rev-parse", "HEAD")
    hinweis(err_kopf)
    code_tag, tag, err_tag = _git("-C", str(juce), "rev-parse", JUCE_TAG + "^{commit}")
    hinweis(err_tag)
    kopf, tag = kopf.strip(), tag.strip()
    if code_kopf != 0:
        klagen.append("HEAD des JUCE-Baums nicht lesbar "
                      f"(rev-parse Exit {code_kopf}): {err_kopf.strip()[:200]}")
        info["beschreibung"] = "HEAD nicht ermittelbar"
    elif code_tag != 0:
        klagen.append(f"der gepinnte Tag {JUCE_TAG} fehlt im JUCE-Baum "
                      f"(rev-parse Exit {code_tag}): {err_tag.strip()[:200]}")
        info["beschreibung"] = f"HEAD {kopf[:12]}, Tag {JUCE_TAG} fehlt"
    elif kopf != tag:
        klagen.append(f"HEAD {kopf[:12]} ist NICHT der Commit des Tags "
                      f"{JUCE_TAG} ({tag[:12]})")
        info["beschreibung"] = f"HEAD {kopf[:12]}, nicht Tag {JUCE_TAG}"
    else:
        info["beschreibung"] = f"Tag {JUCE_TAG} ({kopf[:12]})"

    # -- (ii) genau die Patchdateimenge ist geaendert -----------------------
    code, ausgabe, err = _git("-c", "core.quotepath=false", "-C", str(juce),
                              "status", "--porcelain", "--ignored", "-uall")
    if code != 0:
        klagen.append("git status im JUCE-Baum fehlgeschlagen: "
                      + (err.strip() or ausgabe.strip())[:200])
        return (klagen, [], info)
    hinweis(err)

    kennungen: list[tuple[str, str]] = []
    for zeile in ausgabe.splitlines():
        if len(zeile) < 4:
            continue
        pfad = zeile[3:].strip().strip('"')
        if " -> " in pfad:
            pfad = pfad.split(" -> ")[-1]
        kennungen.append((zeile[:2], pfad.replace(os.sep, "/")))

    statusklagen, loeschungen = juce_baum_status_pruefen(kennungen, patchdateien)
    klagen.extend(statusklagen)
    info["loeschungen"] = loeschungen

    # -- (iii) die Patchdateien tragen genau den Patch ----------------------
    sollklagen, sollhinweise = _patch_soll_vergleich(juce, JUCE_PATCH, patchdateien)
    klagen.extend(sollklagen)
    info["hinweise"].extend(sollhinweise)
    return klagen, loeschungen, info


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


def fehlerbericht() -> None:
    """Die gesammelten Befunde als Klartext - wortgleich an JEDEM Ausgang."""
    print(f"\n{ok} ok, {len(fehler)} Fehler")
    if fehler:
        print("\nFEHLGESCHLAGEN:")
        for f in fehler:
            print("  - " + f)


def voraussetzung_exit() -> int:
    """Exitcode fuer jeden Ausgang "Voraussetzung fehlt" (Matrix F13/F14/F15).

    BEFUND P2, Runde 8 (29.08.2026): der Sollindex-Temp-Ausgang gab 3 zurueck,
    obwohl `fehler` bereits einen Identitaetsbefund trug - gemessen mit einer
    zusaetzlichen roten K1b-Eingabe: `FEHLER` gedruckt, `AUDIT_RETURN=3`.
    `fehler` wird global gesammelt und stand nur am ENDE von main() zur
    Auswertung; jeder `return 3` davor sprang daran vorbei.

    Die Matrix sagt fuer F14 (`--nur-messen`) seit Runde 5: ein echter
    Identitaetsbefund gewinnt und macht aus der 3 eine 2 - eine fehlende
    Voraussetzung verschweigt nie, was schon gemessen WURDE. Fuer den
    Sollindex-Temp (F15) gilt dieselbe Zeile - und seit Runde 9 (30.08.2026)
    sagt F13 (Bau nicht moeglich) sie ausdruecklich mit: die Zeile gilt fuer
    JEDEN Aufruf von main(), nicht nur fuer einen frischen Prozess.

    Deshalb gibt es genau EINEN Ausgang: jeder Voraussetzungs-Rueckweg des
    Beins geht durch diese Funktion. Der Klartext "VORAUSSETZUNG: ..." steht
    davor und bleibt in beiden Faellen stehen; ein neuer `return 3`, der hier
    vorbeikaeme, waere derselbe Befund noch einmal.
    """
    fehlerbericht()
    if fehler:
        print("\nEin registrierter Befund gewinnt gegen die fehlende "
              "Voraussetzung (Matrix F13/F14/F15): Exit 2.")
        return 2
    return 3


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
    _selbsttest_runde5()
    _selbsttest_runde6()
    _selbsttest_runde7()

    print(f"\n{ok} ok, {len(fehler)} Fehler")
    return 2 if fehler else 0


def _selbsttest_runde5() -> None:
    """Runde 5 baulos: Neubau-Beleg, Ortsriegel, K1b und JUCE-Baum-Riegel.

    Jede neue Pruefung wird hier einmal absichtlich gebrochen - eine Wache, die
    niemand hat fallen sehen, ist keine. Kuenstliche Eingaben genuegen, weil
    alle vier reine Funktionen ueber Text, Pfadlisten und Statuszeilen sind;
    der echte git-Aufruf und der echte Bau werden am echten Baum vorgefuehrt
    (Proben P5-* im Manifest).
    """
    print("\nRunde-5-Selbsttest: Neubau, Leseorte, K1b, JUCE-Baum")

    # ── R5-2: lastbuildstate ist Diagnose, aber keine stille ────────────────
    with tempfile.TemporaryDirectory() as roh:
        tlogdir = pathlib.Path(roh)
        werte, _, klagen = lastbuildstate_lesen(tlogdir)
        pruefe(werte == {} and any("fehlt" in k for k in klagen),
               "R5-2a: fehlender lastbuildstate ist eine Klage, kein stilles Ja",
               " | ".join(klagen) if klagen else "keine Klage")

        (tlogdir / "NakamaKern.lastbuildstate").write_text(
            "PlatformToolSet=v143:VCToolArchitecture=Native64Bit:"
            "VCToolsVersion=14.44.35207:TargetPlatformVersion=10.0.26100.0:\n"
            "Release|x64|C:|\n", encoding="utf-8")
        werte, rohtext, klagen = lastbuildstate_lesen(tlogdir)
        pruefe(not klagen and werte.get("VCToolsVersion") == "14.44.35207"
               and werte.get("TargetPlatformVersion") == "10.0.26100.0",
               "R5-2b: Toolset, VCToolsVersion und SDK-Version werden gelesen",
               rohtext.splitlines()[0] if rohtext else "leer")

        (tlogdir / "NakamaKern.lastbuildstate").write_text(
            "PlatformToolSet=v143:\n", encoding="utf-8")
        _, _, klagen = lastbuildstate_lesen(tlogdir)
        pruefe(any("VCToolsVersion" in k for k in klagen),
               "R5-2c: eine unvollstaendige State-Zeile wird benannt",
               " | ".join(klagen) if klagen else "keine Klage")

    # ── R5-3: ohne moeglichen Bau wird NICHTS geloescht und nichts gemessen ─
    with tempfile.TemporaryDirectory() as roh:
        leer = pathlib.Path(roh)
        opfer = leer / "plugin" / "Release"
        opfer.mkdir(parents=True)
        (opfer / "NakamaKern.lib").write_bytes(b"nicht anfassen")

        protokoll = kern_neubau(leer, cmake=leer / "gibtesnicht.exe")
        pruefe(not protokoll["ok"] and protokoll["geloescht"] == 0
               and (opfer / "NakamaKern.lib").is_file(),
               "R5-3a: unauffindbares cmake bricht ab, BEVOR etwas geloescht wird",
               protokoll["grund"] or "kein Grund genannt")

        protokoll = kern_neubau(leer)
        pruefe(not protokoll["ok"] and "fehlgeschlagen" in protokoll["grund"],
               "R5-3b: ein fehlschlagender Bau ist eine fehlende Voraussetzung",
               protokoll["grund"] or "kein Grund genannt")

    # ── R5-5: der Neubau-Beleg selbst ───────────────────────────────────────
    with tempfile.TemporaryDirectory() as roh:
        basis = pathlib.Path(roh)
        quelle = basis / "Eins.cpp"
        quelle.write_text("// leer\n", encoding="utf-8")
        objekt = basis / "Eins.obj"
        lib = basis / "NakamaKern.lib"
        objekt.write_bytes(b"o")
        lib.write_bytes(b"l")
        marke = min(objekt.stat().st_mtime_ns, lib.stat().st_mtime_ns) - 1

        gut = {"marke_ns": marke, "ausgabe": "  Eins.cpp\n  NakamaKern.vcxproj -> ...lib"}
        pruefe(neubau_belegt(gut, [quelle], [objekt], lib) == [],
               "R5-5a: vollstaendiger Neubau bleibt klaglos")

        ohne_zeile = {"marke_ns": marke, "ausgabe": "  nichts zu tun"}
        klagen = neubau_belegt(ohne_zeile, [quelle], [objekt], lib)
        pruefe(any("nennt diese Uebersetzungseinheiten nicht" in k for k in klagen),
               "R5-5b: eine nicht uebersetzte TU faellt auf",
               " | ".join(klagen) if klagen else "keine Klage")

        os.utime(objekt, ns=(marke - 10_000_000_000, marke - 10_000_000_000))
        klagen = neubau_belegt(gut, [quelle], [objekt], lib)
        pruefe(any("aelter als der Neubau" in k for k in klagen),
               "R5-5c: ein Objekt aus einem frueheren Lauf faellt auf",
               " | ".join(klagen) if klagen else "keine Klage")

        objekt.unlink()
        lib.unlink()
        klagen = neubau_belegt(gut, [quelle], [], lib)
        pruefe(any("kein einziges Objekt" in k for k in klagen)
               and any("fehlt nach dem Neubau" in k for k in klagen),
               "R5-5d: fehlende Objekte und fehlende Lib sind ROT",
               " | ".join(klagen) if klagen else "keine Klage")

    # ── R5-6/R5-7: K1b ueber Quelltext ──────────────────────────────────────
    with tempfile.TemporaryDirectory() as roh:
        basis = pathlib.Path(roh)
        ausnahme = basis / "NakamaKernRiegel.h"
        ausnahme.write_text("#if defined (JucePlugin_Name)\n#error nein\n#endif\n",
                            encoding="utf-8")

        sauber = basis / "Sauber.h"
        sauber.write_text(
            "// S8: dieser Kern sieht keine JucePlugin_*-Konstante\n"
            "/* auch im Blockkommentar steht JucePlugin_IsSynth nur als Wort */\n"
            "inline int wert() { return 1; }\n", encoding="utf-8")
        klagen, geprueft, treffer, _ = k1b_riegel([sauber, ausnahme], ausnahme)
        pruefe(klagen == [] and geprueft == 1 and treffer["gesehen"]
               and treffer["makros"] == ["JucePlugin_Name"]
               and treffer["abgeglichen"] == treffer["code"] == 1,
               "R5-6a: Kommentare mit JucePlugin_ bleiben gruen, die Ausnahme "
               "wird namentlich abgeglichen",
               f"Ausnahme: {treffer['code']} Token, {treffer['abgeglichen']} "
               f"abgeglichen, Makroliste {treffer['makros']}"
               + (" | " + " | ".join(klagen) if klagen else ""))

        offen = basis / "Offen.h"
        offen.write_text("#define JucePlugin_Name \"X\"\n", encoding="utf-8")
        klagen, _, _, _ = k1b_riegel([offen, ausnahme], ausnahme)
        pruefe(any("Offen.h" in k and "JucePlugin_" in k for k in klagen),
               "R5-6b: ein Header, der JucePlugin_Name definiert, ist ROT",
               " | ".join(klagen) if klagen else "keine Klage")

        verdeckt = basis / "Verdeckt.h"
        verdeckt.write_text(
            "#define JucePlugin_IsSynth 0\n"
            "#if JucePlugin_IsSynth\n#endif\n"
            "#undef JucePlugin_IsSynth\n", encoding="utf-8")
        klagen, _, _, _ = k1b_riegel([verdeckt, ausnahme], ausnahme)
        pruefe(any("Verdeckt.h" in k for k in klagen),
               "R5-7: definiert-genutzt-entfernt (die Luecke aus Befund 3) ist ROT",
               " | ".join(klagen) if klagen else "keine Klage")

        kaputt = basis / "Kaputt.h"
        kaputt.write_text("/* nie geschlossen\nint x;\n", encoding="utf-8")
        klagen, _, _, _ = k1b_riegel([kaputt, ausnahme], ausnahme)
        pruefe(any("nicht abgeschlossener Blockkommentar" in k for k in klagen),
               "R5-7b: unlesbarer Quelltext ist ROT, nicht uebersprungen",
               " | ".join(klagen) if klagen else "keine Klage")

    # ── R15-1: die K1b-AUSNAHME ist ein Abgleich, keine Freistellung ─────────
    #
    # Befund P1 des fuenfzehnten Pruefers (Runde 15). Gearbeitet wird auf
    # KOPIEN der echten Riegeldatei in einem Temp-Verzeichnis; die Datei im
    # Baum wird nie angefasst - R15-1g haelt ihren sha256 vorher und nachher
    # dagegen.
    with tempfile.TemporaryDirectory() as roh:
        basis = pathlib.Path(roh)
        try:
            riegelbytes = K1B_AUSNAHME.read_bytes()
        except OSError as exc:
            riegelbytes = None
            pruefe(False, "R15-1: die Riegeldatei ist lesbar", str(exc))
        if riegelbytes is not None:
            vorher = hashlib.sha256(riegelbytes).hexdigest()

            echt = basis / "Echt.h"
            echt.write_bytes(riegelbytes)
            klagen, _, angaben, _ = k1b_riegel([echt], echt)
            pruefe(klagen == [] and angaben["gesehen"]
                   and len(angaben["makros"]) > 10
                   and angaben["abgeglichen"] == angaben["code"] > 0,
                   f"R15-1a: die echte Riegeldatei besteht den Abgleich - "
                   f"{angaben['code']} Token im kommentarfreien Quelltext, "
                   f"alle gegen die {len(angaben['makros'])} Makros der "
                   f"K1-Liste DERSELBEN Datei",
                   " | ".join(klagen) if klagen else
                   f"roh {angaben['roh']}, code {angaben['code']}, "
                   f"abgeglichen {angaben['abgeglichen']}")

            # Genau der Weg aus dem Befund: definieren, benutzen, entfernen.
            fremd = basis / "Fremd.h"
            fremd.write_bytes(riegelbytes + _K1B_FREMDPROBE)
            klagen, _, angaben, _ = k1b_riegel([fremd], fremd)
            pruefe(len(klagen) >= 3
                   and all("JucePlugin_Fremd" in k for k in klagen)
                   and all("steht nicht in der K1-Makroliste" in k
                           for k in klagen),
                   "R15-1b: ein unbekanntes JucePlugin_Fremd in der Kopie der "
                   "Riegeldatei ist ROT - namentlich, an jeder Fundstelle",
                   " | ".join(klagen) if klagen else "keine Klage")

            # Ein BEKANNTER Name im falschen Kontext: `#undef` vor dem Kranz
            # wuerde K1 fuer genau dieses Makro still entwaffnen.
            entwaffnet = basis / "Entwaffnet.h"
            entwaffnet.write_bytes(b"#undef JucePlugin_Name\n" + riegelbytes)
            klagen, _, _, _ = k1b_riegel([entwaffnet], entwaffnet)
            pruefe(any("JucePlugin_Name" in k and "#undef" in k
                       for k in klagen),
                   "R15-1c: ein BEKANNTER Name im falschen Kontext (#undef vor "
                   "dem Kranz) ist ROT - die Liste allein genuegt nicht",
                   " | ".join(klagen) if klagen else "keine Klage")

            # Fail-closed: ohne ableitbare Makroliste gibt es keinen Abgleich.
            ohne = basis / "OhneKranz.h"
            ohne.write_text("/* nur ein Kommentar mit JucePlugin_Name */\n",
                            encoding="utf-8")
            klagen, _, angaben, _ = k1b_riegel([ohne], ohne)
            pruefe(any("K1-Makroliste" in k and "nicht ableitbar" in k
                       for k in klagen) and angaben["makros"] == [],
                   "R15-1d: ohne ableitbare K1-Makroliste ist die Ausnahme ROT "
                   "(fail-closed), nicht still gruen",
                   " | ".join(klagen) if klagen else "keine Klage")

            # Die stehende Gegenprobe zum Befund: die fruehere ZAEHLUNG an
            # derselben Kopie. Sie waechst nur, klagt nie - erst der Abgleich
            # nennt den Namen.
            gezaehlt = fremd.read_text(encoding="utf-8",
                                       errors="replace").count(_K1B_TOKEN)
            abgleichklagen = k1b_riegel([fremd], fremd)[0]
            pruefe(gezaehlt > 0 and abgleichklagen != [],
                   "R15-1e: Gegenprobe zum Befund - die fruehere Zaehlung "
                   f"liefert an derselben Kopie {gezaehlt} Treffer und keine "
                   f"einzige Klage, der Abgleich {len(abgleichklagen)}",
                   f"Zaehlung {gezaehlt} Treffer / 0 Klagen, Abgleich "
                   f"{len(abgleichklagen)} Klagen")

            # Die Kurzform traegt sich nicht selbst in die Liste ein.
            kurz_fremd = basis / "KurzFremd.h"
            kurz_fremd.write_bytes(riegelbytes + _K1B_KURZPROBE)
            klagen, _, angaben, _ = k1b_riegel([kurz_fremd], kurz_fremd)
            pruefe(any("JucePlugin_Fremd" in k
                       and "steht nicht in der K1-Makroliste" in k
                       for k in klagen)
                   and "JucePlugin_Fremd" not in angaben["makros"],
                   "R15-1f: auch die Kurzform `#ifdef JucePlugin_Fremd` ist "
                   "ROT - eine Benutzung legitimiert keinen Namen",
                   " | ".join(klagen) if klagen else "keine Klage")

            nachher = hashlib.sha256(K1B_AUSNAHME.read_bytes()).hexdigest()
            pruefe(vorher == nachher,
                   "R15-1g: die Riegeldatei im Baum ist unveraendert - "
                   f"sha256 {vorher[:16]}",
                   f"{vorher[:16]} -> {nachher[:16]}")

    # ── R5-8/R5-9/R5-13: der Tlog-Ortsriegel ────────────────────────────────
    with tempfile.TemporaryDirectory() as roh:
        basis = pathlib.Path(roh)
        plugin = basis / "plugin"
        module = basis / "juce-src" / "modules"
        apc = module / "juce_audio_plugin_client"
        for pfad in (plugin, apc, module / "juce_core"):
            pfad.mkdir(parents=True, exist_ok=True)
        erlaubt = [("plugin", plugin), ("juce-src/modules", module)]
        verboten = [("juce_audio_plugin_client", apc)]
        quelle = plugin / "Eins.cpp"
        quelle.write_text("", encoding="utf-8")
        marker = [str(quelle).upper()]

        klagen, zaehlung = tlog_ortsriegel(
            [str(module / "juce_core" / "juce_core.h").upper()],
            marker, [quelle], erlaubt, verboten)
        pruefe(klagen == [] and zaehlung["juce-src/modules"] == 1,
               "R5-8a: ein juce_core-Header ist erlaubt und wird gezaehlt",
               " | ".join(klagen))

        klagen, _ = tlog_ortsriegel(
            [str(apc / "detail" / "juce_CheckSettingMacros.h").upper()],
            marker, [quelle], erlaubt, verboten)
        pruefe(any("verbotenem Ort" in k for k in klagen),
               "R5-8b: ein juce_audio_plugin_client-Header ist ROT",
               " | ".join(klagen) if klagen else "keine Klage")

        klagen, _ = tlog_ortsriegel(
            [str(basis / "EqCopilot_artefacts" / "JuceLibraryCode" / "JuceHeader.h").upper()],
            marker, [quelle], erlaubt, verboten)
        pruefe(any("unbekanntem Ort" in k for k in klagen),
               "R5-9: ein generierter JuceLibraryCode-Header ist ROT",
               " | ".join(klagen) if klagen else "keine Klage")

        klagen, _ = tlog_ortsriegel([], [], [quelle], erlaubt, verboten)
        pruefe(any("fehlt als Marker" in k for k in klagen),
               "R5-13: eine Kernquelle ohne Marker im Leseprotokoll ist ROT",
               " | ".join(klagen) if klagen else "keine Klage")

    # ── R5-11: die Klassifizierung des JUCE-Baum-Riegels ────────────────────
    patch = {"modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp"}
    klagen, loeschungen = juce_baum_status_pruefen(
        [(" M", "modules/juce_audio_plugin_client/juce_audio_plugin_client_VST3.cpp")],
        patch)
    pruefe(klagen == [] and loeschungen == [],
           "R5-11a: die Patchdatei selbst darf geaendert sein")

    klagen, _ = juce_baum_status_pruefen(
        [(" M", "modules/juce_core/juce_core.h")], patch)
    pruefe(any("nicht im Nakama-Patch" in k for k in klagen),
           "R5-11b: eine geaenderte juce_core-Datei ist ROT (Weg W8)",
           " | ".join(klagen) if klagen else "keine Klage")

    klagen, _ = juce_baum_status_pruefen([("??", "modules/juce_core/Fremd.h")], patch)
    pruefe(any("unverfolgte Datei" in k for k in klagen),
           "R5-11c: eine unverfolgte Datei im JUCE-Baum ist ROT",
           " | ".join(klagen) if klagen else "keine Klage")

    klagen, _ = juce_baum_status_pruefen([(" D", "modules/juce_core/juce_core.h")], patch)
    pruefe(any("geloeschte Moduldatei" in k for k in klagen),
           "R5-11d: eine geloeschte Moduldatei ist ROT",
           " | ".join(klagen) if klagen else "keine Klage")

    klagen, loeschungen = juce_baum_status_pruefen(
        [(" D", "examples/DemoRunner/Builds/Android/app/src/debug/res/values/string.xml")],
        patch)
    pruefe(klagen == [] and loeschungen and "string.xml" in loeschungen[0],
           "R5-11e: eine Loeschung ausserhalb modules/** wird benannt, nicht verschwiegen",
           ", ".join(loeschungen))

    klagen, _ = juce_baum_status_pruefen([("XY", "irgendwas")], patch)
    pruefe(any("unbekannter git-Status" in k for k in klagen),
           "R5-11f: ein unbekannter Statuscode ist ROT, kein stilles Ja",
           " | ".join(klagen) if klagen else "keine Klage")


def _probe_repo(wurzel: pathlib.Path, tag: str,
                dateien: dict[str, str]) -> None:
    """Kleines Wegwerf-Repo: ein Commit, ein Tag, sonst nichts.

    Eigene user-/gpg-Einstellungen, damit die Probe nicht von der Umgebung
    des Rechners abhaengt, auf dem sie laeuft.
    """
    wurzel.mkdir(parents=True, exist_ok=True)
    for name, inhalt in dateien.items():
        ziel = wurzel / name
        ziel.parent.mkdir(parents=True, exist_ok=True)
        ziel.write_text(inhalt, encoding="utf-8", newline="\n")
    _git("init", "-q", "-b", "master", str(wurzel))
    for schluessel, wert in (("user.email", "probe@nakama.invalid"),
                             ("user.name", "Nakama-Probe"),
                             ("commit.gpgsign", "false"),
                             ("core.autocrlf", "false")):
        _git("-C", str(wurzel), "config", schluessel, wert)
    _git("-C", str(wurzel), "add", "--", *sorted(dateien))
    _git("-C", str(wurzel), "commit", "-q", "-m", "Basis")
    _git("-C", str(wurzel), "tag", tag)


def _r6_3_warnlauf() -> tuple[int, str, str]:
    """Ein git-Lauf, der DETERMINISTISCH mit 0 endet und auf stderr warnt.

    BEFUND P2, Runde 8 (29.08.2026): die Vorfassung hoffte auf die Warnung und
    wiederholte den Versuch bis zu zwoelfmal. Gemessen (20 Laeufe je Variante,
    git 2.54.0.windows.1): der blosse `status` traf die Voraussetzung nur
    16/20, mit unlesbarer excludesFile 17/20, mit vorherigem
    `update-index --really-refresh` 15/20. Eine Wiederholung senkt die
    Flackerwahrscheinlichkeit, sie beseitigt sie nicht - und eine Wache, deren
    Voraussetzung nur probabilistisch eintritt, ist kein Beleg.

    🔑 Der Grund ist NICHT die Warnung, sondern der STAT-CACHE. `git status`
       vergleicht zuerst Groesse und mtime des Indexeintrags. Weicht die
       GROESSE ab, weiss git ohne Lesen, dass die Datei geaendert ist - der
       Konvertierungspfad wird nie betreten, `.gitattributes` nie gelesen, und
       genau deshalb bleibt die Warnung aus. Erst wenn die Groesse GLEICH
       bleibt und nur die mtime abweicht, MUSS git den Inhalt neu hashen und
       konsultiert dabei die Attribute.

    Deshalb steht hier "eins\\n" im Commit und "zwei\\n" im Arbeitsbaum: fuenf
    Bytes gegen fuenf Bytes. Gemessen 20/20 (Rohausgabe im Manifest,
    Abschnitt "Nacharbeit Runde 8").

    Der Rueckgabewert ist der ganze Lauf (Exit, stdout, stderr) - die Wache
    misst die STROMTRENNUNG des `_git`-Wrappers, nicht eine bestimmte
    git-Warnung; deren Wortlaut darf sich mit der git-Version aendern, ohne
    dass sich an der gemessenen Zusage etwas aendert.
    """
    with tempfile.TemporaryDirectory() as roh:
        repo = pathlib.Path(roh) / "warnrepo"
        _probe_repo(repo, "0.0.1", {
            "datei.txt": "eins\n",
            # Ein negatives Muster in .gitattributes ist der portable
            # Ausloeser: git warnt auf stderr und endet trotzdem mit 0.
            ".gitattributes": "* -text\n!*.foo bar\n",
        })
        # GLEICHE Groesse, anderer Inhalt - siehe oben. Die Zeile ist der
        # ganze Determinismus dieser Wache; wer sie entfernt, bekommt die
        # Flackerprobe von Runde 7 zurueck.
        (repo / "datei.txt").write_text("zwei\n", encoding="utf-8", newline="\n")
        return _git("-c", "core.quotepath=false", "-C", str(repo),
                    "status", "--porcelain", "--ignored", "-uall")


def _selbsttest_runde6() -> None:
    """Runde 6 baulos: die vier Befunde B1-B4 als Wache, jede einmal gebrochen.

    B1 Ortsriegel ohne Endungsausnahme, B2 der JUCE-Baum-Riegel ueber HEAD,
    ignorierte Dateien und Patchinhalt, B3 getrennte git-Stroeme. B4 ist ein
    Manifestbefund und hat hier nichts zu pruefen.
    """
    print("\nA14-Selbsttest, Runde 6: Ortsriegel ohne Endungsausnahme, "
          "JUCE-Baum ueber HEAD/ignoriert/Inhalt, getrennte git-Stroeme")

    # -- B1: keine Endung entkommt der Erlaubnisliste -----------------------
    with tempfile.TemporaryDirectory() as roh:
        basis = pathlib.Path(roh)
        plugin = basis / "plugin"
        module = basis / "juce-src" / "modules"
        apc = module / "juce_audio_plugin_client"
        system = basis / "Windows"
        for pfad in (plugin, apc, module / "juce_core", system / "System32"):
            pfad.mkdir(parents=True, exist_ok=True)
        erlaubt = [("plugin", plugin), ("juce-src/modules", module)]
        # Seit Befund P1 (Runde 7) ist %SystemRoot% keine Wurzel mehr: die
        # gemessenen Systemdateien haengen NAMENTLICH an ihr.
        systemdateien = {_normpfad(system / rel): "Windows-System"
                         for rel in SYSTEMDATEIEN}
        verboten = [("juce_audio_plugin_client", apc)]
        quelle = plugin / "Eins.cpp"
        quelle.write_text("", encoding="utf-8")
        marker = [str(quelle).upper()]

        gebrochen = []
        for endung in (".dat", ".dll", ".nls", ".exe", ".mui", ".bin"):
            klagen, _ = tlog_ortsriegel([str(basis / "OUTSIDE" / ("forced" + endung))],
                                        marker, [quelle], erlaubt, verboten,
                                        systemdateien)
            if not any("unbekanntem Ort" in k for k in klagen):
                gebrochen.append(endung)
        pruefe(not gebrochen,
               "R6-1a: eine per /FI gelesene Datei ausserhalb der erlaubten Orte ist "
               "ROT - unabhaengig von der Endung (Befund B1)",
               "stumm geblieben bei: " + ", ".join(gebrochen) if gebrochen
               else "geprueft: .dat .dll .nls .exe .mui .bin")

        klagen, zaehlung = tlog_ortsriegel(
            [str(system / SYSTEMDATEIEN[0])], marker, [quelle],
            erlaubt, verboten, systemdateien)
        pruefe(klagen == [] and zaehlung.get("Windows-System") == 1,
               "R6-1b: eine NAMENTLICH gefuehrte Systemdatei ist erlaubt und wird "
               "gezaehlt, nicht ueber ihre Endung (Runde 7: auch nicht ueber ihren Ort)",
               " | ".join(klagen) if klagen else f"Windows-System "
               f"{zaehlung.get('Windows-System')}")

        pruefe("ohne Uebersetzungsstoff" not in zaehlung,
               "R6-1c: die ortsfreie Sammelspalte ist weg - jede Datei zaehlt "
               "unter ihrem Ort",
               ", ".join(zaehlung))

    # -- B3: git schreibt Warnungen auf stderr und endet mit 0 --------------
    #
    # Die Voraussetzung wird seit Runde 8 HERGESTELLT, nicht abgewartet:
    # `_r6_3_warnlauf()` erzwingt den Attributlesevorgang ueber den
    # Stat-Cache. Warnt git trotzdem nicht, ist die Probe ROT - eine Wache
    # ohne ihre Voraussetzung belegt nichts, und Schweigen ist hier kein Ja.
    code, aus, err = _r6_3_warnlauf()
    pruefe(code == 0 and "warning:" in err and "warning:" not in aus,
           "R6-3a: bei Exit 0 traegt nur stdout Daten; die git-Warnung steht "
           "getrennt auf stderr (Befund B3)",
           f"Exit {code} | stderr: {err.splitlines()[0] if err.splitlines() else '-'}")

    # Was der frueher gemischte Strom angerichtet haette - wortgleich
    # nachgestellt, damit der Befund nicht nur behauptet ist.
    gemischt = [(z[:2], z[3:].strip()) for z in (aus + err).splitlines()
                if len(z) >= 4]
    alt_klagen, _ = juce_baum_status_pruefen(gemischt, {"datei.txt"})
    sauber = [(z[:2], z[3:].strip()) for z in aus.splitlines() if len(z) >= 4]
    neu_klagen, _ = juce_baum_status_pruefen(sauber, {"datei.txt"})
    pruefe(any("unbekannter git-Status" in k for k in alt_klagen)
           and neu_klagen == [],
           "R6-3b: der frueher gemischte Strom haette die Warnzeile als "
           "Statuscode gelesen; der getrennte tut es nicht",
           "gemischt: " + (alt_klagen[0] if alt_klagen else "keine Klage")
           + " || getrennt: " + (" | ".join(neu_klagen) if neu_klagen
                                 else "keine Klage"))

    # -- B2: HEAD, ignorierte Dateien und Patchinhalt -----------------------
    global JUCE_PATCH, JUCE_TAG
    merk_patch, merk_tag = JUCE_PATCH, JUCE_TAG
    try:
        with tempfile.TemporaryDirectory() as roh:
            basis = pathlib.Path(roh)
            bau = basis / "build"
            juce = bau / "_deps" / "juce-src"
            grund = [f"z{nr:02d}" for nr in range(1, 31)]
            _probe_repo(juce, "1.0.0", {
                "datei.txt": "\n".join(grund) + "\n",
                ".gitignore": "*.pch\n",
            })
            # Arbeitsbaum := Tag + Patch; der Patch ist genau diese Aenderung.
            gepatcht = list(grund)
            gepatcht[1] = "z02 vom Nakama-Patch"
            (juce / "datei.txt").write_text("\n".join(gepatcht) + "\n",
                                            encoding="utf-8", newline="\n")
            _, diff, _ = _git("-C", str(juce), "diff")
            JUCE_PATCH = basis / "probe-1.0.0-nakama.patch"
            JUCE_PATCH.write_text(diff, encoding="utf-8", newline="\n")
            JUCE_TAG = "1.0.0"

            klagen, loeschungen, info = juce_baum_riegel(bau)
            pruefe(klagen == [] and info["beschreibung"].startswith("Tag 1.0.0"),
                   "R6-2a: Tag plus genau der Patch ist gruen (Grundstellung der "
                   "drei neuen Zeilen)",
                   " | ".join(klagen) if klagen else info["beschreibung"])

            # (i) HEAD wandert weiter, der Baum bleibt bytegleich.
            _git("-C", str(juce), "commit", "-q", "--allow-empty", "-m", "danach")
            klagen, _, info = juce_baum_riegel(bau)
            pruefe(any("NICHT der Commit des Tags" in k for k in klagen),
                   "R6-2b: HEAD neben dem gepinnten Tag ist ROT, obwohl der "
                   "Arbeitsbaum unveraendert ist (Befund B2 i)",
                   " | ".join(klagen) if klagen else "keine Klage")
            _git("-C", str(juce), "tag", "-f", "1.0.0", "HEAD")

            # (ii) eine von .gitignore gedeckte Fremddatei im Modulbaum.
            fremd = juce / "modules" / "juce_core" / "NakamaProbe.pch"
            fremd.parent.mkdir(parents=True, exist_ok=True)
            fremd.write_text("#define JucePlugin_Name \"X\"\n", encoding="utf-8")
            _, ohne, _ = _git("-C", str(juce), "status", "--porcelain", "-uall")
            klagen, _, _ = juce_baum_riegel(bau)
            pruefe(any("ignorierte Datei" in k and "NakamaProbe.pch" in k
                       for k in klagen) and "NakamaProbe" not in ohne,
                   "R6-2c: eine ignorierte Fremddatei ist ROT - ohne --ignored "
                   "war sie unsichtbar (Befund B2 ii)",
                   " | ".join(klagen) if klagen else "keine Klage")
            fremd.unlink()

            # (iii) eine zusaetzliche Zeile IN der Patchdatei - und zwar am
            # Dateiende, also weit ausserhalb der Hunk-Kontexte. Genau so hat
            # sie am echten Baum apply --check --reverse ueberlebt.
            (juce / "datei.txt").write_text(
                "\n".join(gepatcht + ["z31 heimlich dazugekommen"]) + "\n",
                encoding="utf-8", newline="\n")
            rueck, _, _ = _git("-C", str(juce), "apply", "--check", "--reverse",
                               str(JUCE_PATCH))
            klagen, _, _ = juce_baum_riegel(bau)
            pruefe(any("nicht genau den Patch" in k for k in klagen) and rueck == 0,
                   "R6-2d: eine zusaetzliche Zeile in der Patchdatei ist ROT, "
                   "waehrend apply --check --reverse dazu schweigt (Befund B2 iii)",
                   f"apply --check --reverse Exit {rueck} | "
                   + (" | ".join(klagen) if klagen else "keine Klage"))
            (juce / "datei.txt").write_text("\n".join(gepatcht) + "\n",
                                            encoding="utf-8", newline="\n")
            klagen, _, _ = juce_baum_riegel(bau)
            pruefe(klagen == [],
                   "R6-2e: zurueckgenommen ist der Baum wieder gruen",
                   " | ".join(klagen) if klagen else "")
    finally:
        JUCE_PATCH, JUCE_TAG = merk_patch, merk_tag


def _selbsttest_runde7() -> None:
    """Runde 7 baulos: die beiden Codebefunde P1 und P2, jeder einmal gebrochen.

    P1 hat zwei Haelften, und beide muessen einzeln greifen: der Ortsriegel
    nennt eine fremde Datei unter %SystemRoot%, und K1b LIEST sie. Faellt eine
    Haelfte weg, ist der Weg wieder offen - deshalb wird jede fuer sich
    gemessen. P2 ist die fehlende Voraussetzung statt Traceback.

    Runde 8 haengt an denselben Ausgang die Zeile F15 an: der Exitcode dieser
    fehlenden Voraussetzung (R8-1) und die Struktur, die ihn erzwingt (R8-2).
    """
    global ok
    print("\nA14-Selbsttest, Runde 7: Systemdateien namentlich, K1b ueber alle "
          "Eingaben, benannte Voraussetzung statt Traceback ohne Temp")

    # -- P1a: unter %SystemRoot% ist nur der gemessene NAME erlaubt ---------
    with tempfile.TemporaryDirectory() as roh:
        basis = pathlib.Path(roh)
        plugin = basis / "plugin"
        module = basis / "juce-src" / "modules"
        msvc = basis / "MSVC" / "14.44"
        sdk = basis / "SDK" / "10.0"
        system = basis / "Windows"
        for pfad in (plugin, module / "juce_core", msvc, sdk,
                     system / "System32", system / "Tracing"):
            pfad.mkdir(parents=True, exist_ok=True)
        erlaubt = [("plugin", plugin), ("juce-src/modules", module),
                   ("MSVC-Toolset", msvc), ("Windows-SDK", sdk)]
        systemdateien = {_normpfad(system / rel): "Windows-System"
                         for rel in SYSTEMDATEIEN}
        quelle = plugin / "Eins.cpp"
        quelle.write_text("", encoding="utf-8")
        marker = [str(quelle).upper()]

        # Genau der gemessene Fall: ein per /FI erzwungener Kopf in einem
        # Unterverzeichnis von %SystemRoot%, in das ein normaler Benutzer
        # schreiben darf.
        fremd = system / "Tracing" / "nakama-forced.h"
        klagen, zaehlung = tlog_ortsriegel([str(fremd)], marker, [quelle],
                                           erlaubt, [], systemdateien)
        pruefe(any("unbekanntem Ort" in k for k in klagen)
               and zaehlung.get("Windows-System") == 0,
               "R7-1a: eine nicht namentlich gefuehrte Datei unter %SystemRoot% "
               "ist ROT (Befund P1) - der Ort allein erlaubt nichts mehr",
               " | ".join(klagen) if klagen else "keine Klage")

        klagen, zaehlung = tlog_ortsriegel(
            [str(system / rel) for rel in SYSTEMDATEIEN], marker, [quelle],
            erlaubt, [], systemdateien)
        pruefe(klagen == [] and zaehlung.get("Windows-System") == len(SYSTEMDATEIEN),
               f"R7-1b: die {len(SYSTEMDATEIEN)} gemessenen Systemdateien sind "
               f"namentlich erlaubt und werden gezaehlt",
               " | ".join(klagen) if klagen
               else f"Windows-System {zaehlung.get('Windows-System')}")

        # R7-1c misst die ENTSCHEIDUNG selbst, nicht nur ihre Wirkung: die
        # beiden Proben oben bekommen ihre Erlaubnisliste als Argument und
        # blieben deshalb gruen, als die Bruchprobe %SystemRoot% in
        # erlaubte_leseorte() wieder als WURZEL eintrug (gemessen, Runde 7).
        # Eine Wache, die den Ruecksprung nicht sieht, ist keine.
        echt_erlaubt, _, echt_systemdateien, _ = erlaubte_leseorte(
            basis / "kein-bauverzeichnis", {})
        echt_system, systemklage = _systemwurzel()
        unter_system = ([name for name, pfad in echt_erlaubt
                         if _unter(_normpfad(pfad), _normpfad(echt_system))]
                        if echt_system is not None else [])
        erwartet = ({_normpfad(echt_system / rel) for rel in SYSTEMDATEIEN}
                    if echt_system is not None else set())
        pruefe(echt_system is not None and not unter_system
               and set(echt_systemdateien) == erwartet,
               "R7-1c: erlaubte_leseorte() fuehrt unter %SystemRoot% KEINE Wurzel "
               "mehr, sondern genau die gemessenen Dateinamen (Befund P1)",
               systemklage or (f"Wurzel(n) unter %SystemRoot%: "
                               f"{', '.join(unter_system)}" if unter_system
                               else ", ".join(sorted(echt_systemdateien))))

        # -- P1b: K1b nimmt jede Eingabe ausser JUCE, Toolchain und SDK -----
        gelesen = [str(quelle), str(fremd),
                   str(module / "juce_core" / "juce_core.h"),
                   str(msvc / "include" / "vector"),
                   str(sdk / "um" / "windows.h")]
        menge, mengenklagen = k1b_eingaben_aus_tlog(gelesen, erlaubt)
        namen = sorted(p.name for p in menge)
        pruefe(mengenklagen == [] and namen == ["Eins.cpp", "nakama-forced.h"],
               "R7-2a: die K1b-Menge ist das Komplement der drei "
               "Ausschlusswurzeln - der erzwungene Kopf ist DRIN, JUCE, "
               "Toolchain und SDK sind draussen",
               ", ".join(namen) if not mengenklagen else " | ".join(mengenklagen))

        for weg in K1B_AUSSCHLUSS_WURZELN:
            rest = [(n, p) for n, p in erlaubt if n != weg]
            _, mengenklagen = k1b_eingaben_aus_tlog(gelesen, rest)
            pruefe(any(weg in k for k in mengenklagen),
                   f"R7-2b: ohne die Ausschlusswurzel {weg} bildet K1b KEINE "
                   f"Menge, sondern klagt (fail-closed)",
                   " | ".join(mengenklagen) if mengenklagen else "keine Klage")

        # Der Inhalt, um den es geht: definieren, benutzen, wieder entfernen.
        fremd.write_text("#define JucePlugin_IsSynth 0\n"
                         "#if JucePlugin_IsSynth\n#endif\n"
                         "#undef JucePlugin_IsSynth\n", encoding="utf-8")
        klagen, geprueft, _, _ = k1b_riegel(menge, K1B_AUSNAHME)
        pruefe(any("nakama-forced.h" in k for k in klagen) and geprueft == 2,
               "R7-2c: K1b liest den erzwungenen Kopf und nennt sein "
               "JucePlugin_-Token - K1 und K3 sehen davon nichts",
               " | ".join(klagen) if klagen else "keine Klage")

        # -- P1c: die benannten Systemdateien werden ROH durchsucht ---------
        binaer = system / SYSTEMDATEIEN[0]
        binaer.parent.mkdir(parents=True, exist_ok=True)
        binaer.write_bytes(b"\x00\x01' unpaariges Zeichen \"\x02\x03")
        klagen, geprueft, _, roh_geprueft = k1b_riegel([binaer], K1B_AUSNAHME,
                                                     systemdateien)
        pruefe(klagen == [] and geprueft == 1 and roh_geprueft == 1,
               "R7-3a: eine benannte Systemdatei wird roh gemessen, nicht als "
               "C++ geparst - unpaarige Zeichen sind kein ROT",
               " | ".join(klagen) if klagen
               else f"geprueft {geprueft}, davon roh {roh_geprueft}")

        for kodierung in ("ascii", "utf-16-le"):
            binaer.write_bytes(b"\x00" + _K1B_TOKEN.encode(kodierung) + b"\x00")
            klagen, _, _, _ = k1b_riegel([binaer], K1B_AUSNAHME, systemdateien)
            pruefe(any(kodierung in k for k in klagen),
                   f"R7-3b: ein JucePlugin_-Token in den Rohbytes ({kodierung}) "
                   f"einer benannten Systemdatei ist ROT",
                   " | ".join(klagen) if klagen else "keine Klage")

    # -- P2: ohne schreibbares Temp gibt es eine benannte Voraussetzung statt
    #    eines Tracebacks; den Exitcode gibt danach `voraussetzung_exit()` -
    #    ohne registrierten Befund 3, mit registriertem Befund 2, nie 0 ------
    merk = tempfile.tempdir
    try:
        tempfile.tempdir = str(pathlib.Path(tempfile.gettempdir())
                               / "nakama-gibt-es-nicht-r7")
        gefangen = ""
        try:
            _patch_soll_vergleich(WURZEL, JUCE_PATCH, {"egal"})
        except VoraussetzungFehlt as exc:
            gefangen = str(exc)
        except OSError as exc:            # genau das, was vorher hochflog
            gefangen = f"UNGEFANGEN {exc!r}"
        pruefe(gefangen.startswith("kein schreibbares temporaeres Verzeichnis"),
               "R7-4: ohne schreibbares Temp meldet der Sollindex-Vergleich eine "
               "fehlende Voraussetzung im Klartext statt eines Tracebacks "
               "(Befund P2)",
               gefangen or "keine Ausnahme")
    finally:
        tempfile.tempdir = merk

    # -- P2 Runde 8/9: ein registrierter Befund gewinnt gegen jede fehlende
    #    Voraussetzung - ohne Befund 3, mit Befund 2, nie 0, und das an
    #    JEDEM Ausgang, den unmoeglichen Bau eingeschlossen
    #    (Matrix F13/F14/F15) ------------------------------------------
    #
    # Gemessen am Stand a94c33e: derselbe Sollindex-Temp-Ausgang gab 3, obwohl
    # eine rote K1b-Eingabe schon `FEHLER` gedruckt hatte. Die Zusage der
    # Matrix ist aber "Fehler gesammelt -> 2, sonst 3, nie 0" - und genau die
    # misst diese Wache, an der Stelle, an der sie faellt.
    merk_fehler = list(fehler)
    merk_ok = ok
    try:
        fehler.clear()
        with contextlib.redirect_stdout(io.StringIO()):
            ohne = voraussetzung_exit()
        fehler.append("kuenstlicher Befund fuer R8-1")
        with contextlib.redirect_stdout(io.StringIO()) as gesagt:
            mit = voraussetzung_exit()
        text = gesagt.getvalue()
    finally:
        fehler.clear()
        fehler.extend(merk_fehler)
        ok = merk_ok
    pruefe(ohne == 3 and mit == 2 and "kuenstlicher Befund fuer R8-1" in text,
           "R8-1: ein Voraussetzungs-Ausgang gibt 2, sobald ein Befund "
           "registriert ist, sonst 3 - und nennt den Befund im Klartext "
           "(Matrix F13/F14/F15)",
           f"ohne Befund {ohne}, mit Befund {mit}")

    # Dieselbe Zusage strukturell: KEIN Ausgang von main() darf an
    # `voraussetzung_exit()` vorbei eine 3 zurueckgeben. Ein neuer `return 3`
    # waere genau der Befund aus Runde 8 noch einmal - hier faellt er auf.
    #
    # ⚠️ Beim Brechen gemessen (29.08.2026): ein Vergleich auf den nackten Text
    #    "return 3" ging an `return 3  # Kommentar` vorbei - die erste Fassung
    #    dieser Wache fiel nicht. Gesucht wird deshalb mit Muster, samt
    #    optionalem Zeilenkommentar.
    quelle = pathlib.Path(__file__).read_text(encoding="utf-8")
    rumpf = quelle.split("\ndef main() -> int:\n", 1)
    nackte = ([z.strip() for z in rumpf[1].splitlines()
               if re.match(r"^\s*return\s+3\s*(#.*)?$", z)]
              if len(rumpf) == 2 else ["main() nicht gefunden"])
    pruefe(not nackte,
           "R8-2: in main() gibt es keinen nackten `return 3` mehr - jeder "
           "Voraussetzungs-Ausgang geht durch voraussetzung_exit()",
           (f"{len(nackte)} nackte return 3 in main(): "
            + " | ".join(nackte[:3])) if nackte else "")


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

    # ── Runde 5: Frische wird HERGESTELLT, nicht nachgebaut ─────────────────
    # Erst bauen, dann messen. Ein fehlgeschlagener oder unmoeglicher Bau ist
    # eine fehlende Voraussetzung, nie ein Urteil ueber den Kern: der Lauf
    # endet ueber `voraussetzung_exit()` - ohne registrierten Befund 3, mit
    # registriertem Befund 2, nie 0 (Matrix F13/F14/F15, Runde 8/9).
    neubau: dict = {}
    if nur_messen:
        print("HINWEIS: --nur-messen - es wird NICHT gebaut.")
        print("         Ohne Neubau kein Frische-Urteil; dieser Lauf endet nie mit 0,")
        print("         ohne weiteren Befund mit Exit 3. Die Identitaetspruefung und")
        print("         die Riegel laufen trotzdem und koennen fuer sich rot sein.")
    else:
        neubau = kern_neubau(bau)
        if not neubau["ok"]:
            print("VORAUSSETZUNG: " + neubau["grund"], file=sys.stderr)
            if neubau.get("ausgabe"):
                for zeile in neubau["ausgabe"].splitlines()[-15:]:
                    print("  | " + zeile, file=sys.stderr)
            print("  Ohne Neubau des Kerns wird nichts gemessen und nichts behauptet.",
                  file=sys.stderr)
            return voraussetzung_exit()

    # NAK-85: der Bau loest ueber ZERO_CHECK das Configure selbst aus. Diese
    # Wache prueft danach nur noch nach, dass das auch geschehen ist - sie ist
    # Diagnose, keine eigene Frischequelle mehr.
    klagen = configure_frische(bau)
    if klagen and not nur_messen:
        print(f"VORAUSSETZUNG: Configure veraltet trotz Neubau - {klagen[0]}",
              file=sys.stderr)
        for weitere in klagen[1:]:
            print(f"  auch: {weitere}", file=sys.stderr)
        return voraussetzung_exit()

    kern_kandidaten = sorted(bau.glob("plugin/**/NakamaKern.lib"))
    if not kern_kandidaten:
        print(f"VORAUSSETZUNG: NakamaKern.lib nicht gefunden unter {bau}.", file=sys.stderr)
        print("  Erst bauen: cmake --build <bau> --config Release --target NakamaKern",
              file=sys.stderr)
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

    # Die Archivmitglieder werden schon hier gelesen: die TU-Mengenpruefung in
    # [0c] gleicht gegen sie ab, [3] nutzt dieselbe Liste weiter unten.
    mitglieder = archivmitglieder(kern_bytes)

    try:
        quellen = _kernquellen_aus_cmake()
    except RuntimeError as exc:
        quellen = []
        pruefe(False, "NAKAMA_KERN_QUELLEN ist lesbar", str(exc))

    tlogdir_kandidaten = sorted(p for p in bau.glob("**/NakamaKern.tlog") if p.is_dir())
    tlogdir = (tlogdir_kandidaten[0] if len(tlogdir_kandidaten) == 1
               else waehle_release(tlogdir_kandidaten, "NakamaKern.tlog"))
    kerndir = tlogdir.parent if tlogdir is not None else None
    objekte = (sorted(p for p in kerndir.rglob("*.obj") if p.is_file())
               if kerndir is not None else [])

    # ── 0. Der Neubau selbst ────────────────────────────────────────────────
    print("\n[0] Frische - der Kern wurde fuer diese Messung neu gebaut")
    if nur_messen:
        # KEIN pruefe(): ein Fehlschlag hier waere Exit 2 und damit ein Urteil
        # ueber den Kern. Gemessen wurde aber gar nichts - das ist eine
        # fehlende Voraussetzung, und die endet unten ueber
        # `voraussetzung_exit()`: ohne registrierten Befund 3, mit
        # registriertem Befund 2, nie 0 (Matrix F13/F14/F15).
        print("  --      nicht gebaut (--nur-messen); ueber die Frische des gemessenen")
        print("          Artefakts behauptet dieser Lauf nichts")
    else:
        neubauklagen = neubau_belegt(neubau, quellen, objekte, kern)
        pruefe(not neubauklagen,
               f"Kernartefakte geloescht und in {neubau['sekunden']:.1f}s neu erzeugt "
               f"({neubau['geloescht']} Dateien entfernt, {len(quellen)} "
               f"Uebersetzungseinheiten uebersetzt, {len(objekte)} Objekte, Lib neu gelinkt)",
               " | ".join(neubauklagen))

    # ── 0b. Die Riegel ueber die tatsaechlichen Compiler-Eingaben ───────────
    print("\n[0b] Riegel - Quelltext, Leseorte, JUCE-Baum")
    zustand, roh_lbs, lbs_klagen = ({}, "", ["NakamaKern.tlog nicht gefunden"])
    if tlogdir is not None:
        zustand, roh_lbs, lbs_klagen = lastbuildstate_lesen(tlogdir)

    tlog_read = tlogdir / "CL.read.1.tlog" if tlogdir is not None else None
    gelesen: list[str] = []
    marker: list[str] = []
    # Fail-closed vorbelegt: ohne ableitbare Wurzeln bildet K1b unten KEINE
    # Menge, sondern klagt - eine leere Erlaubnisliste darf nie "alles erlaubt"
    # oder "nichts zu pruefen" bedeuten.
    erlaubt: list[tuple[str, pathlib.Path]] = []
    systemdateien: dict[str, str] = {}
    if tlog_read is None or not tlog_read.is_file():
        pruefe(False, "das Leseprotokoll des Compilers (CL.read.1.tlog) liegt vor",
               "ohne es ist nicht feststellbar, was der Compiler wirklich gelesen hat")
    else:
        if nur_messen:
            # Ohne Neubau ist das Leseprotokoll irgendein aelteres - der
            # Ortsriegel darunter bleibt aussagekraeftig, die HERKUNFT des
            # Protokolls aber nicht. Das wird gesagt, nicht behauptet.
            print("  --      CL.read.1.tlog stammt NICHT aus diesem Lauf "
                  "(--nur-messen); die Orte darunter sind Diagnose")
        else:
            frisch = tlog_read.stat().st_mtime_ns >= neubau.get("marke_ns", 0)
            pruefe(frisch,
                   "CL.read.1.tlog stammt aus diesem Neubau",
                   "" if frisch else
                   "das Leseprotokoll ist aelter als der Bau dieses Laufs")
        marker, gelesen = tlog_gelesene_dateien(tlog_read)

        erlaubt, verboten, systemdateien, wurzelklagen = erlaubte_leseorte(bau, zustand)
        if wurzelklagen:
            pruefe(False, "die erlaubten Leseorte sind aus dem Bau ableitbar",
                   " | ".join(wurzelklagen))
        else:
            ortsklagen, zaehlung = tlog_ortsriegel(gelesen, marker, quellen,
                                                   erlaubt, verboten, systemdateien)
            umfang = ", ".join(f"{name} {anzahl}" for name, anzahl in zaehlung.items())
            pruefe(not ortsklagen,
                   f"alle {len(gelesen)} vom Compiler gelesenen Dateien stammen aus "
                   f"erlaubten Wurzeln oder sind eine der {len(systemdateien)} "
                   f"namentlich erlaubten Systemdateien ({umfang})",
                   " | ".join(ortsklagen[:6]))

    # K1b: jede Compiler-Eingabe ausser JUCE-Modulen und Toolchain/SDK - also
    # plugin/** UND alles Uebrige (Befund P1, Runde 7) - plus die literale
    # Huelle als Gegenprobe. Beide Mengen werden benannt.
    aus_tlog, mengenklagen = k1b_eingaben_aus_tlog(gelesen, erlaubt)
    try:
        huelle = kern_quellabhaengigkeiten()
    except RuntimeError as exc:
        huelle = []
        pruefe(False, "lokale Include-Huelle der Kernquellen ist eindeutig ableitbar",
               str(exc))
    eingaben = list(aus_tlog) + list(huelle)
    k1b_klagen, k1b_geprueft, in_ausnahme, k1b_roh = k1b_riegel(
        eingaben, K1B_AUSNAHME, systemdateien)
    k1b_klagen = mengenklagen + k1b_klagen
    ausnahmetext = (
        f"Ausnahme NakamaKernRiegel.h: {in_ausnahme['code']} Token im "
        f"kommentarfreien Quelltext, alle namentlich gegen die "
        f"{len(in_ausnahme['makros'])} Makros der K1-Liste DERSELBEN Datei "
        f"abgeglichen und nur in Riegelkontexten ({in_ausnahme['roh']} Token "
        f"im Rohtext inkl. Kommentaren)"
        if in_ausnahme["gesehen"] else
        "NakamaKernRiegel.h war in dieser Eingabemenge nicht dabei - ueber sie "
        "sagt dieser Lauf nichts")
    pruefe(bool(eingaben) and not k1b_klagen,
           f"keine der {k1b_geprueft} Compiler-Eingaben ausserhalb der JUCE-Module "
           f"und der Toolchain-/SDK-Wurzeln traegt ein JucePlugin_-Token (Tlog "
           f"{len(set(aus_tlog))}, Huelle {len(set(huelle))}, davon {k1b_roh} "
           f"benannte Systemdatei(en) roh durchsucht; {ausnahmetext})",
           " | ".join(k1b_klagen[:6]) if k1b_klagen else
           ("keine Compiler-Eingabe gefunden" if not eingaben else ""))

    try:
        baumklagen, loeschungen, bauminfo = juce_baum_riegel(bau)
    except VoraussetzungFehlt as exc:
        # Befund P2, Runde 7: kein Traceback, kein Exit 1 - eine benannte
        # fehlende Voraussetzung. Der Lauf hat bis hier gemessen und behauptet
        # ueber den JUCE-Baum ausdruecklich nichts; der Ausgang geht durch
        # `voraussetzung_exit()` - ohne registrierten Befund 3, mit
        # registriertem Befund 2, nie 0 (Matrix F13/F14/F15).
        print("\nVORAUSSETZUNG: " + str(exc))
        print("VORAUSSETZUNG: " + str(exc), file=sys.stderr)
        return voraussetzung_exit()
    pruefe(not baumklagen,
           f"juce-src ist {bauminfo['beschreibung']} plus genau der Nakama-VST3-Patch "
           f"(HEAD = Tag; nichts Geaendertes, Unverfolgtes oder Ignoriertes "
           f"ausserhalb der {bauminfo['patchdateien']} Patchdatei(en); diese tragen "
           f"genau den Patch; {len(loeschungen)} benannte Loeschung(en) ausserhalb "
           f"modules/**)",
           " | ".join(baumklagen[:6]) if baumklagen else
           (", ".join(loeschungen[:5]) if loeschungen else ""))
    # Befund B3: git schreibt Warnungen auf stderr und endet trotzdem mit 0.
    # Sie sind Hinweis, nie Datenstrom - gesagt wird beides.
    for zeile in bauminfo.get("hinweise", [])[:6]:
        print("  hinweis git im JUCE-Baum: " + zeile)

    # ── 0c. Diagnose: womit wurde gebaut ───────────────────────────────────
    print("\n[0c] Diagnose - womit wurde gebaut (kein Frische-Urteil)")
    pruefe(not lbs_klagen,
           "lastbuildstate nennt Toolset, VCToolsVersion und TargetPlatformVersion",
           " | ".join(lbs_klagen) if lbs_klagen else roh_lbs.splitlines()[0])
    if nur_messen and klagen:
        # Dieselbe Einordnung wie im Normalmodus: ein veraltetes Configure ist
        # eine fehlende Voraussetzung, kein Urteil ueber den Kern. Der Lauf
        # endet unten ueber `voraussetzung_exit()` - ohne registrierten Befund
        # 3, mit registriertem Befund 2, nie 0 (Matrix F13/F14/F15). Runde 11
        # hat genau diesen Zweig gemessen: mit einem vorher registrierten
        # Befund endet er mit 2, nicht mit der frueher hier behaupteten 3.
        print("  --      Configure veraltet (Voraussetzung, kein Urteil): "
              + " | ".join(klagen))
    else:
        pruefe(not klagen,
               "Configure ist juenger als jede CMake-Eingabe, die der Generator "
               "verbraucht hat",
               " | ".join(klagen))

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

        # (1) Die TU-Menge - keine Frischefrage, sondern "ist der Kern still
        #     gewachsen?". Genau diese Frage hat beim IPC-Zuwachs gesprochen.
        tuklagen, aktuell, veraltet = tu_mengen_abgleich(einheiten, quellen, mitglieder)
        hinweise = list(tuklagen)
        if veraltet:
            hinweise.append(
                (f"{len(veraltet)} veralteter Tlog-Eintrag: " if len(veraltet) == 1
                 else f"{len(veraltet)} veraltete Tlog-Eintraege: ") + ", ".join(veraltet))
        pruefe(bool(quellen) and not tuklagen,
               f"Tlog, NAKAMA_KERN_QUELLEN und Archiv nennen dieselben "
               f"{len(aktuell)} Uebersetzungseinheiten",
               " | ".join(hinweise))

        # (2) Die Schalter, mit denen genau diese Einheiten uebersetzt wurden.
        if soll is not None:
            schalterklagen = schalter_abgleich(soll, soll_zusatz, aktuell)
            umfang = ", ".join(f"{KLASSENNAME[k]} {len(soll[k])}" for k in SCHALTERKLASSEN)
            pruefe(bool(aktuell) and not schalterklagen,
                   f"womit gebaut: jede der {len(aktuell)} TUs traegt die heutigen "
                   f"Schalter der Projektdatei ({umfang}; "
                   f"{len(soll_zusatz)} AdditionalOptions-Token auf Enthaltensein)",
                   " | ".join(schalterklagen) if schalterklagen
                   else ("keine gebaute Uebersetzungseinheit" if not aktuell else ""))

        # (3) Und wurde nach der letzten Uebersetzung auch gelinkt?
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

    if nur_messen:
        # F14: die Identitaetspruefung ist gelaufen und steht oben - ueber die
        # Frische ist damit trotzdem nichts gemessen. Ein gruenes Urteil waere
        # eine Behauptung ohne Messung, also Exit 3 statt 0. Ein registrierter
        # Befund gewinnt auch hier und macht daraus 2 - dieselbe Entscheidung
        # wie an jedem anderen Voraussetzungs-Ausgang (Runde 8).
        print("\nVORAUSSETZUNG: ohne Neubau kein Frische-Urteil (--nur-messen).")
        print("  Die Identitaetspruefung oben ist gelaufen; ueber die Frische des")
        print("  gemessenen Artefakts behauptet dieser Lauf nichts.")
        return voraussetzung_exit()
    fehlerbericht()
    return 2 if fehler else 0


if __name__ == "__main__":
    sys.exit(main())
