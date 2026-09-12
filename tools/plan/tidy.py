#!/usr/bin/env python3
"""Tidy - C++-Statikanalyse des Plugins mit clang-tidy, gezaehlt als Ratsche.

Aufruf vom Workspace-Root:

    py -3.13 tools/plan/tidy.py [--json] [--jobs N] [--nur TEILPFAD]
                                [--selbsttest] [--mit-selbsttest]

Exitcode 0 = Grenze gehalten · 4 = Grenze gerissen · 2 = Werkzeugfehler
(Configure bricht ab, eine Uebersetzungseinheit parst nicht, clang-tidy
stuerzt ab, Selbsttest rot) · 3 = Voraussetzung fehlt (Visual Studio 2022 mit
C++-Werkzeugen und Clang-Komponente, cmake, ninja). Nur Standardbibliothek.

WARUM ES DIESES SKRIPT GIBT (NAK-288, 12.09.2026)
--------------------------------------------------
Bis zum 12.09.2026 uebersetzte der Kanon das Plugin unter /W4 und sonst
nichts: kein clang-tidy, kein cppcheck, kein Sanitizer. Der Broker hat seit
NAK-223 eine Clippy-Ratsche in gesundheit.py; das Plugin - Audio-Thread,
Echtzeit, JUCE - hatte keinen statischen Blick. User 12.09.2026: "die luecke
die du entdeckt hast muss geschlossen werden." Dieses Skript ist dieser
Blick, in der Form, die der Kanon kennt: ein Python-Bein mit Selbsttest,
Exitcodes und einer Ratsche auf dem gemessenen Iststand (Bein A33).

WAS GEMESSEN WIRD
-----------------
Eindeutige clang-tidy-Fundstellen (Datei, Zeile, Spalte, Check) unter
`eq-copilot/plugin/` ohne `tests/` und ohne den flatc-Codegen
`vertrag/generiert/`, ueber alle Uebersetzungseinheiten, die der Produktbau
dort uebersetzt. Der Regelsatz steht in
`eq-copilot/plugin/.clang-tidy` (clang-tidy liest ihn selbst); dieses Skript
waehlt keine Checks und kennt keine Ausnahmen.

WIE
---
  (1) Voraussetzungen finden: vswhere -> Visual Studio 2022 (Build Tools oder
      Edition) mit `VC\\Auxiliary\\Build\\vcvars64.bat`; clang-tidy (PATH,
      sonst `VC\\Tools\\Llvm\\x64\\bin`, sonst `C:\\Program Files\\LLVM`);
      ninja und cmake (PATH, sonst die Kopien unter
      `Common7\\IDE\\CommonExtensions\\Microsoft\\CMake`). Fehlt eines: Exit 3.
  (2) Umgebung aus vcvars64.bat lesen (INCLUDE, LIB, PATH), damit cmake
      cl.exe findet und clang-tidy die MSVC-STL und das Windows-SDK.
  (3) Eigener Baum `eq-copilot/build-tidy` mit dem Ninja-Generator und cl.exe:
      nur dort schreibt CMake eine compile_commands.json (der Visual-Studio-
      Generator des Kanons kann das nicht). Der Baum wird konfiguriert, nie
      gebaut; die einzige generierte Kopfdatei (BrokerInstallBinding.h)
      entsteht beim Configure. JUCE und FlatBuffers holt FetchContent in
      diesen Baum, der Brueckenpatch laeuft dort wie im Kanon.
  (4) Datenbank filtern: je Quelldatei im Quellort genau EIN Eintrag, der
      des ersten Ziels in der Datenbank (CMake schreibt sie deterministisch).
      Dieselbe Datei uebersetzt in bis zu vierzehn Ziele mit denselben
      Warnschaltern und bis auf Bundle-Defines gleichen Praeprozessorwerten;
      mehrfach gefahren dauerte es das Mehrfache und zaehlte nichts Neues.
      `/MP` (Bauparallelitaet) und `/showIncludes` (Ninja-Abhaengigkeits-
      ausgabe) werden entfernt. Die gefilterte Datenbank liegt unter
      `build-tidy/tidy-db/`.
  (5) clang-tidy je Uebersetzungseinheit parallel (Threads = Prozessoren);
      den cl-Treibermodus waehlt clang-tidy am Kommandonamen; `--header-filter`
      auf `eq-copilot/plugin`, JUCE-Header bleiben draussen; Fundstellen
      unter `tests/` werden hier nachgefiltert.
  (6) Zaehlen: eindeutige (Datei, Zeile, Spalte, Check). Dieselbe Kopfzeile in
      zwanzig Uebersetzungseinheiten ist EIN Befund.

WAS EIN FEHLER IST UND WAS NICHT
--------------------------------
  * Eine Uebersetzungseinheit mit `error:`-Diagnose ist NICHT gemessen; der
    Lauf endet mit 2 (Werkzeugfehler) und nennt sie. Eine halb geparste
    Einheit meldet weniger Funde, und "weniger" saehe wie Fortschritt aus.
  * Ein clang-tidy-Prozess mit Exitcode != 0 ohne `error:`-Zeile ist ebenso
    ein Werkzeugfehler (Absturz, fehlende Datenbank, Zeitlimit).
  * `--nur TEILPFAD` misst nur Einheiten, deren Pfad den Teil enthaelt: eine
    TEILMESSUNG ohne Ratschenurteil (Exit 0/2/3, nie 4). Fuer Rotbeweise und
    Handpruefungen einzelner Dateien.

HERKUNFT DER SCHWELLE
---------------------
  clang-tidy-Fundstellen   Grenze = Iststand (Ratsche), Ziel 0. Gemessen am
                           12.09.2026 (siehe TIDY_GRENZE und das Manifest
                           docs/beweise/NAK-288.md) mit clang-tidy 19.1.5 aus
                           den Build Tools 2022 (MSVC 14.44). Jede neue
                           Fundstelle reisst; gesenkte Zahlen werden
                           nachgezogen, nie erhoeht. Eine Aenderung des
                           Regelsatzes aendert den Iststand und die Ratsche im
                           selben Commit.
"""
from __future__ import annotations

import argparse
import concurrent.futures
import json
import os
import pathlib
import re
import shutil
import subprocess
import sys
import time

WURZEL = pathlib.Path(__file__).resolve().parents[2]
QUELLORT = "eq-copilot/plugin"
# Pfadanfaenge unter QUELLORT, die nicht zaehlen: Tests (keine Produktzusage)
# und der flatc-Codegen (Bein A9 haelt ihn bytegleich zur Neuerzeugung; wie
# `broker/src/generiert/` in gesundheit.py kein Wartungsaufwand).
AUSGENOMMEN = ("tests/", "vertrag/generiert/")
BAUORDNER = "eq-copilot/build-tidy"
DB_ORDNER = "tidy-db"
HEADER_FILTER = r"eq-copilot[/\\]plugin[/\\]"
TIDY_ZIEL = 0
# Ratsche: Iststand 12.09.2026 mit dem Regelsatz aus eq-copilot/plugin/.clang-tidy
# und den Ausnahmen oben (Erstmessung NAK-288, 41 Einheiten, clang-tidy 19.1.5;
# die rohe Erstmessung zaehlte 101: 8 optin.performance.Padding, 10 im Codegen;
# Manifest docs/beweise/NAK-288.md). Nur senken.
TIDY_GRENZE = 83
KONFIGURIER_LIMIT_S = 40 * 60      # erste Konfiguration holt JUCE und FlatBuffers
EINHEIT_LIMIT_S = 20 * 60
LANGSAMSTE_ZEIGEN = 5


class Voraussetzung(Exception):
    """Etwas, das dieser Rechner nicht hat. Exit 3, kein Befund."""


class Werkzeugfehler(Exception):
    """Das Messwerkzeug misst nicht. Exit 2, nie ein Wert."""


# --------------------------------------------------------------- reine Helfer


def norm(pfad: str) -> str:
    """Vergleichsform eines Windows-Pfads: Vorwaertsstriche, Kleinschreibung."""
    return pfad.replace("\\", "/").lower()


def einordnen_norm(pfad_norm: str, wurzel_norm: str):
    """Relativer Pfad (Vergleichsform), wenn die Datei im Quellort liegt und
    nicht in einem ausgenommenen Ordner; sonst None. Rein."""
    basis = wurzel_norm.rstrip("/") + "/"
    if not pfad_norm.startswith(basis + QUELLORT + "/"):
        return None
    rel = pfad_norm[len(basis):]
    rest = rel[len(QUELLORT) + 1:]
    if any(rest.startswith(a) for a in AUSGENOMMEN):
        return None
    return rel


def bereinige_kommando(kommando: str) -> str:
    """Entfernt /MP und /showIncludes (auch mit Bindestrich); alles andere bleibt."""
    kommando = re.sub(r"(?<=\s)[/-]showIncludes(?=\s|$)", "", kommando)
    kommando = re.sub(r"(?<=\s)[/-]MP\d*(?=\s|$)", "", kommando)
    return re.sub(r"[ \t]{2,}", " ", kommando).strip()


def bereinige_argumente(argumente):
    return [a for a in argumente
            if not re.fullmatch(r"[/-]showIncludes", a)
            and not re.fullmatch(r"[/-]MP\d*", a)]


def filtere_datenbank(eintraege, wurzel_norm):
    """Je Quelldatei im Quellort genau ein Eintrag, Reihenfolge der ersten
    Nennung; Eintraege ausserhalb und unter ausgenommenen Ordnern fallen weg.
    Rein: arbeitet nur auf den Dictionaries."""
    gewaehlt, gesehen = [], set()
    for e in eintraege:
        datei = e.get("file", "")
        if not datei:
            continue
        if not (len(datei) > 1 and datei[1] == ":") and not datei.startswith(("/", "\\")):
            # Relativ zu `directory` (CMake schreibt absolut; die Form ist im
            # Format erlaubt). normpath loest `..` auf, sonst faellt eine Datei
            # `build-tidy/../plugin/x.cpp` still aus dem Quellort.
            datei = os.path.normpath(e.get("directory", "").rstrip("/\\") + "/" + datei)
        n = norm(datei)
        if einordnen_norm(n, wurzel_norm) is None or n in gesehen:
            continue
        gesehen.add(n)
        neu = {"directory": e.get("directory", ""), "file": datei}
        if "arguments" in e:
            neu["arguments"] = bereinige_argumente(e["arguments"])
        else:
            neu["command"] = bereinige_kommando(e.get("command", ""))
        gewaehlt.append(neu)
    return gewaehlt


DIAGNOSE = re.compile(
    r"^(?P<pfad>(?:[A-Za-z]:)?[^:\r\n]+):(?P<zeile>\d+):(?P<spalte>\d+): "
    r"(?P<art>warning|error): (?P<text>.*?)(?: \[(?P<check>[A-Za-z0-9_.,-]+)\])?$")


def parse_diagnosen(text: str):
    """Alle warning-/error-Zeilen als (pfad, zeile, spalte, art, text, check).
    note-Zeilen, Treiberwarnungen ohne Ort und Zusammenfassungen fallen weg."""
    funde = []
    for zeile in text.splitlines():
        m = DIAGNOSE.match(zeile.rstrip())
        if m:
            funde.append((m["pfad"], int(m["zeile"]), int(m["spalte"]),
                          m["art"], m["text"], m["check"] or ""))
    return funde


def parse_umgebung(text: str):
    """`set`-Ausgabe als Dictionary; Zeilen ohne `=` fallen weg."""
    env = {}
    for zeile in text.splitlines():
        if "=" in zeile and not zeile.startswith("="):
            k, v = zeile.split("=", 1)
            env[k] = v
    return env


def dedupliziere(funde, wurzel_real: str):
    """Eindeutige Fundstellen im Quellort. `funde` = Liste von
    (pfad_real, zeile, spalte, check, text); Rueckgabe sortiert nach
    (Datei, Zeile, Spalte, Check) mit relativem Anzeigepfad."""
    wn = norm(wurzel_real)
    eindeutig = {}
    for pfad_real, zeile, spalte, check, text in funde:
        rel_norm = einordnen_norm(norm(pfad_real), wn)
        if rel_norm is None:
            continue
        anzeige = pfad_real[len(wurzel_real.rstrip("/\\")) + 1:].replace("\\", "/")
        eindeutig.setdefault((rel_norm, zeile, spalte, check), (anzeige, zeile, spalte, check, text))
    return sorted(eindeutig.values(), key=lambda f: (f[0].lower(), f[1], f[2], f[3]))


def bewerte(ist, ziel, grenze):
    """Ein Wort fuer eine Zahl - dieselbe Bewertung wie in gesundheit.py."""
    if ist is None:
        return "NICHT MESSBAR"
    if grenze is not None and ist > grenze:
        return "GRENZE"
    if ziel is not None and ist > ziel:
        return "ZIEL"
    return "OK"


# ------------------------------------------------------------- Voraussetzungen


def finde_werkzeuge():
    """Pfade zu vswhere-Installation, vcvars64, clang-tidy, ninja, cmake."""
    programme_x86 = os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)")
    vswhere = pathlib.Path(programme_x86) / "Microsoft Visual Studio" / "Installer" / "vswhere.exe"
    if not vswhere.exists():
        raise Voraussetzung(f"vswhere.exe fehlt ({vswhere}) - kein Visual Studio 2022 installiert")
    lauf = subprocess.run(
        [str(vswhere), "-latest", "-products", "*",
         "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
         "-property", "installationPath"],
        capture_output=True, text=True, encoding="utf-8", errors="replace")
    install = lauf.stdout.strip().splitlines()
    if lauf.returncode != 0 or not install:
        raise Voraussetzung("vswhere findet keine Installation mit C++-Werkzeugen (VC.Tools.x86.x64)")
    ip = pathlib.Path(install[0])
    vcvars = ip / "VC" / "Auxiliary" / "Build" / "vcvars64.bat"
    if not vcvars.exists():
        raise Voraussetzung(f"vcvars64.bat fehlt unter {ip}")

    def suche(name, *kandidaten):
        im_pfad = shutil.which(name)
        if im_pfad:
            return im_pfad
        for k in kandidaten:
            if pathlib.Path(k).exists():
                return str(k)
        raise Voraussetzung(f"{name} weder im PATH noch unter {ip} - "
                            + ("Clang-Komponente der Build Tools nachinstallieren"
                               if name == "clang-tidy" else "CMake-Komponente der Build Tools nachinstallieren"))

    cmake_basis = ip / "Common7" / "IDE" / "CommonExtensions" / "Microsoft" / "CMake"
    return dict(
        installation=str(ip),
        vcvars=str(vcvars),
        clang_tidy=suche("clang-tidy", ip / "VC" / "Tools" / "Llvm" / "x64" / "bin" / "clang-tidy.exe",
                         r"C:\Program Files\LLVM\bin\clang-tidy.exe"),
        ninja=suche("ninja", cmake_basis / "Ninja" / "ninja.exe"),
        cmake=suche("cmake", cmake_basis / "CMake" / "bin" / "cmake.exe",
                    r"C:\Program Files\CMake\bin\cmake.exe"),
    )


def vs_umgebung(vcvars: str):
    """Umgebungsvariablen nach vcvars64.bat (INCLUDE, LIB, PATH, VCToolsVersion)."""
    comspec = os.environ.get("COMSPEC", "cmd.exe")
    befehl = f'"{comspec}" /s /c ""{vcvars}" >nul 2>&1 && set"'
    lauf = subprocess.run(befehl, capture_output=True, text=True,
                          encoding="oem", errors="replace", timeout=120)
    env = parse_umgebung(lauf.stdout)
    if lauf.returncode != 0 or "INCLUDE" not in env:
        raise Werkzeugfehler(f"vcvars64.bat lieferte keine Umgebung (Exit {lauf.returncode}): "
                             + lauf.stderr.strip()[-300:])
    return env


def version_von(exe: str, env=None) -> str:
    try:
        lauf = subprocess.run([exe, "--version"], capture_output=True, text=True,
                              encoding="utf-8", errors="replace", env=env, timeout=60)
    except (OSError, subprocess.TimeoutExpired) as e:
        return f"(Version nicht lesbar: {e})"
    zeilen = [z.strip() for z in lauf.stdout.splitlines() if z.strip()]
    for z in zeilen:
        if "version" in z.lower():
            return z
    return zeilen[0] if zeilen else "(keine Ausgabe)"


# ---------------------------------------------------------------- Configure


def konfiguriere(wurzel: pathlib.Path, werkzeuge, env):
    bau = wurzel / BAUORDNER
    argumente = [werkzeuge["cmake"], "-S", "eq-copilot", "-B", BAUORDNER.replace("\\", "/"),
                 "-G", "Ninja", "-DCMAKE_BUILD_TYPE=Release",
                 "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                 "-DCMAKE_C_COMPILER=cl.exe", "-DCMAKE_CXX_COMPILER=cl.exe",
                 f"-DCMAKE_MAKE_PROGRAM={werkzeuge['ninja']}"]
    t0 = time.monotonic()
    try:
        lauf = subprocess.run(argumente, cwd=str(wurzel), capture_output=True, text=True,
                              encoding="utf-8", errors="replace", env=env,
                              timeout=KONFIGURIER_LIMIT_S)
    except subprocess.TimeoutExpired:
        raise Werkzeugfehler(f"cmake-Configure ueber dem Zeitlimit ({KONFIGURIER_LIMIT_S} s)")
    dauer = time.monotonic() - t0
    if lauf.returncode != 0:
        schwanz = "\n".join((lauf.stdout + "\n" + lauf.stderr).splitlines()[-30:])
        raise Werkzeugfehler(f"cmake-Configure Exit {lauf.returncode} nach {dauer:.0f} s:\n{schwanz}")
    db = bau / "compile_commands.json"
    if not db.exists():
        raise Werkzeugfehler(f"Configure lief, aber {db} fehlt (kein Ninja-Generator?)")
    return db, dauer


def lade_datenbank(db: pathlib.Path):
    try:
        return json.loads(db.read_text(encoding="utf-8"))
    except (OSError, ValueError) as e:
        raise Werkzeugfehler(f"compile_commands.json nicht lesbar: {e}")


# ------------------------------------------------------------------ Messung


def fahre_einheit(clang_tidy: str, db_ordner: pathlib.Path, datei: str, env):
    t0 = time.monotonic()
    try:
        lauf = subprocess.run(
            [clang_tidy, "--quiet", "-p", str(db_ordner),
             "--header-filter=" + HEADER_FILTER, datei],
            capture_output=True, text=True, encoding="utf-8", errors="replace",
            env=env, timeout=EINHEIT_LIMIT_S)
        return dict(datei=datei, rc=lauf.returncode, out=lauf.stdout, err=lauf.stderr,
                    sekunden=time.monotonic() - t0)
    except subprocess.TimeoutExpired:
        return dict(datei=datei, rc=124, out="", err=f"Zeitlimit {EINHEIT_LIMIT_S} s",
                    sekunden=time.monotonic() - t0)
    except OSError as e:
        return dict(datei=datei, rc=127, out="", err=str(e), sekunden=time.monotonic() - t0)


def miss(wurzel: pathlib.Path, werkzeuge, env, eintraege, jobs: int):
    """Faehrt clang-tidy ueber die Eintraege; liefert (funde, fehler, ergebnisse)."""
    db_ordner = wurzel / BAUORDNER / DB_ORDNER
    db_ordner.mkdir(parents=True, exist_ok=True)
    (db_ordner / "compile_commands.json").write_text(
        json.dumps(eintraege, indent=1), encoding="utf-8")
    dateien = [e["file"] for e in eintraege]
    with concurrent.futures.ThreadPoolExecutor(max_workers=max(1, jobs)) as pool:
        ergebnisse = list(pool.map(
            lambda d: fahre_einheit(werkzeuge["clang_tidy"], db_ordner, d, env), dateien))

    wurzel_real = os.path.realpath(str(wurzel))
    roh, fehler = [], []
    for e in ergebnisse:
        diagnosen = parse_diagnosen(e["out"] + "\n" + e["err"])
        einheit = anzeige_pfad(e["datei"], wurzel_real)
        e["einheit"] = einheit
        errors = [d for d in diagnosen if d[3] == "error"]
        if errors:
            fehler.append((einheit, [f"{d[0]}:{d[1]}:{d[2]}: {d[4]}" for d in errors[:5]]))
        elif e["rc"] != 0:
            fehler.append((einheit, [f"clang-tidy Exit {e['rc']}: {e['err'].strip()[-300:]}"]))
        e["warnungen_roh"] = 0
        for pfad, zeile, spalte, art, text, check in diagnosen:
            if art != "warning":
                continue
            if not (len(pfad) > 1 and pfad[1] == ":") and not pfad.startswith(("/", "\\")):
                pfad = str(wurzel / BAUORDNER / pfad)
            roh.append((os.path.realpath(pfad), zeile, spalte, check, text))
            e["warnungen_roh"] += 1
    return dedupliziere(roh, wurzel_real), fehler, ergebnisse


def anzeige_pfad(pfad: str, wurzel_real: str) -> str:
    real = os.path.realpath(pfad)
    if norm(real).startswith(norm(wurzel_real).rstrip("/") + "/"):
        return real[len(wurzel_real.rstrip("/\\")) + 1:].replace("\\", "/")
    return real.replace("\\", "/")


# ------------------------------------------------------------------ Bericht


def je_check(funde):
    zaehler = {}
    for _, _, _, check, _ in funde:
        zaehler[check] = zaehler.get(check, 0) + 1
    return sorted(zaehler.items(), key=lambda kv: (-kv[1], kv[0]))


def drucke_bericht(kopf, funde, fehler, ergebnisse, status, teilmessung):
    print("NAKAMA-TIDY")
    for k, v in kopf.items():
        print(f"{k:<10}: {v}")
    if teilmessung:
        print("")
        print(f"ACHTUNG: TEILMESSUNG (--nur {teilmessung}) - kein Ratschenurteil, "
              "die Zahl ist nicht mit der Grenze vergleichbar")
    print("")
    ist = len(funde)
    zeilen = [("Maass", "Ist", "Ziel", "Grenze", "Status"),
              ("clang-tidy-Fundstellen (plugin)", str(ist), str(TIDY_ZIEL), str(TIDY_GRENZE), status)]
    breiten = [max(len(z[i]) for z in zeilen) for i in range(5)]
    print("  ".join(zeilen[0][i].ljust(breiten[i]) for i in range(5)))
    print("  ".join("-" * b for b in breiten))
    print("  ".join(zeilen[1][i].ljust(breiten[i]) for i in range(5)))
    print("")
    print(f"Uebersetzungseinheiten: {len(ergebnisse)} ({QUELLORT} ohne {', '.join(AUSGENOMMEN)}), "
          f"davon fehlerfrei geparst: {len(ergebnisse) - len(fehler)}")
    langsam = sorted(ergebnisse, key=lambda e: -e["sekunden"])[:LANGSAMSTE_ZEIGEN]
    print("Langsamste Einheiten: " + ", ".join(f"{e['einheit']} {e['sekunden']:.0f} s" for e in langsam))
    if fehler:
        print("")
        print(f"WERKZEUGFEHLER: {len(fehler)} Uebersetzungseinheit(en) nicht messbar:")
        for einheit, gruende in fehler:
            print(f"  {einheit}")
            for g in gruende:
                print(f"    {g}")
    if funde:
        print("")
        print("Fundstellen je Check:")
        for check, n in je_check(funde):
            print(f"  {n:>5}  {check}")
        print("")
        print("Fundstellen (eindeutig ueber Datei, Zeile, Spalte, Check):")
        for datei, zeile, spalte, check, text in funde:
            print(f"  {datei}:{zeile}:{spalte} [{check}] {text}")
    print("")
    if fehler:
        print("URTEIL: WERKZEUGFEHLER - keine vollstaendige Messung")
    elif teilmessung:
        print(f"URTEIL: TEILMESSUNG - {ist} Fundstelle(n) in den gewaehlten Einheiten, kein Ratschenurteil")
    elif status == "GRENZE":
        print(f"URTEIL: GRENZE GERISSEN - {ist} Fundstellen, hoechstens {TIDY_GRENZE} erlaubt")
    else:
        print(f"URTEIL: Grenze gehalten - {ist} Fundstellen von hoechstens {TIDY_GRENZE} (Ziel {TIDY_ZIEL})")


# --------------------------------------------------------------- Selbsttest


def selbsttest() -> int:
    """Faelle im Speicher durch dieselben Funktionen. Jede Erwartung mit Gegenteil."""
    faelle = []

    def pruefe(name, ist, soll):
        faelle.append((name, ist == soll, f"ist={ist!r} soll={soll!r}"))

    w = "c:/users/phili/nakama"

    # --- Einordnung: Quellort, Ausnahme, Fremdes ----------------------------
    pruefe("Quelldatei im Plugin liegt im Quellort",
           einordnen_norm("c:/users/phili/nakama/eq-copilot/plugin/src/a.cpp", w),
           "eq-copilot/plugin/src/a.cpp")
    pruefe("tests/ ist ausgenommen",
           einordnen_norm("c:/users/phili/nakama/eq-copilot/plugin/tests/a.cpp", w), None)
    pruefe("ein Ordner, der nur mit tests beginnt, ist nicht ausgenommen",
           einordnen_norm("c:/users/phili/nakama/eq-copilot/plugin/testsonde/a.cpp", w),
           "eq-copilot/plugin/testsonde/a.cpp")
    pruefe("flatc-Codegen unter vertrag/generiert/ ist ausgenommen",
           einordnen_norm("c:/users/phili/nakama/eq-copilot/plugin/vertrag/generiert/x_generated.h", w),
           None)
    pruefe("vertrag/ selbst zaehlt (Eingabe in Vergleichsform)",
           einordnen_norm(norm(r"C:\Users\phili\nakama\eq-copilot\plugin\vertrag\NakamaVertrag.cpp"), w),
           "eq-copilot/plugin/vertrag/nakamavertrag.cpp")
    pruefe("JUCE unter _deps liegt nicht im Quellort",
           einordnen_norm("c:/users/phili/nakama/eq-copilot/build-tidy/_deps/juce-src/x.h", w), None)
    pruefe("Broker liegt nicht im Quellort",
           einordnen_norm("c:/users/phili/nakama/broker/src/lib.rs", w), None)
    pruefe("fremde Wurzel liegt nicht im Quellort",
           einordnen_norm("c:/users/phili/anderes/eq-copilot/plugin/src/a.cpp", w), None)
    pruefe("Wurzel mit Schlussstrich ist dieselbe Wurzel",
           einordnen_norm("c:/users/phili/nakama/eq-copilot/plugin/core/a.h", w + "/"),
           "eq-copilot/plugin/core/a.h")
    pruefe("norm: Rueckstriche und Grossschreibung verschwinden",
           norm(r"C:\Users\Phili\NAKAMA\eq-copilot\Plugin\src\A.cpp"),
           "c:/users/phili/nakama/eq-copilot/plugin/src/a.cpp")

    # --- Datenbankfilter ----------------------------------------------------
    db = [
        {"directory": "c:/b", "file": r"C:\Users\phili\nakama\eq-copilot\plugin\src\a.cpp",
         "command": "cl.exe /nologo /MP /showIncludes /W4 -c a.cpp"},
        {"directory": "c:/b", "file": "C:/Users/phili/nakama/eq-copilot/plugin/src/a.cpp",
         "command": "cl.exe /nologo -c a.cpp"},
        {"directory": "c:/b", "file": "C:/Users/phili/nakama/eq-copilot/plugin/tests/t.cpp",
         "command": "cl.exe -c t.cpp"},
        {"directory": "c:/b", "file": "C:/Users/phili/nakama/eq-copilot/build-tidy/_deps/juce-src/j.cpp",
         "command": "cl.exe -c j.cpp"},
        {"directory": "c:/users/phili/nakama/eq-copilot/build-tidy", "file": "../plugin/core/r.cpp",
         "arguments": ["cl.exe", "/MP4", "-showIncludes", "-c", "r.cpp"]},
    ]
    gefiltert = filtere_datenbank(db, w)
    pruefe("Filter: eine Datei in zwei Zielen ergibt einen Eintrag, tests und _deps fallen weg",
           len(gefiltert), 2)
    pruefe("Filter: der erste Eintrag gewinnt",
           gefiltert[0]["command"] if gefiltert else "", "cl.exe /nologo /W4 -c a.cpp")
    pruefe("Filter: relativer Dateipfad wird an directory gehaengt und .. aufgeloest",
           norm(gefiltert[1]["file"]) if len(gefiltert) > 1 else "",
           "c:/users/phili/nakama/eq-copilot/plugin/core/r.cpp")
    pruefe("Filter: arguments-Form verliert /MP4 und -showIncludes",
           gefiltert[1].get("arguments") if len(gefiltert) > 1 else None,
           ["cl.exe", "-c", "r.cpp"])
    pruefe("bereinige: /MD bleibt, /MP faellt",
           bereinige_kommando("cl -MD /MP /Foa.obj -c a.cpp"), "cl -MD /Foa.obj -c a.cpp")
    pruefe("bereinige: Kommando ohne die Schalter bleibt gleich",
           bereinige_kommando("cl -MD -c a.cpp"), "cl -MD -c a.cpp")

    # --- Diagnosen ----------------------------------------------------------
    text = "\n".join([
        r"C:\Users\phili\nakama\eq-copilot\plugin\src\a.cpp:12:5: warning: uninitialized [clang-analyzer-core.uninitialized.Assign]",
        r"C:\Users\phili\nakama\eq-copilot\plugin\src\a.cpp:13:1: note: here",
        "C:/Users/phili/nakama/eq-copilot/plugin/core/b.h:7:9: warning: text with [brackets] inside [bugprone-a,bugprone-b]",
        "C:/x/c.cpp:1:1: error: unknown type name 'Foo'",
        "warning: argument unused during compilation: '/GL' [clang-diagnostic-unused-command-line-argument]",
        "41358 warnings generated.",
        "Suppressed 41300 warnings (41300 in non-user code).",
    ])
    d = parse_diagnosen(text)
    pruefe("parse: drei Diagnosen mit Ort, note und Treiberwarnung nicht", len(d), 3)
    pruefe("parse: Windows-Pfad mit Laufwerk bleibt ganz",
           d[0][0] if d else "", r"C:\Users\phili\nakama\eq-copilot\plugin\src\a.cpp")
    pruefe("parse: Zeile und Spalte sind Zahlen", (d[0][1], d[0][2]) if d else None, (12, 5))
    pruefe("parse: Check steht ohne Klammern", d[0][5] if d else "", "clang-analyzer-core.uninitialized.Assign")
    pruefe("parse: eckige Klammern im Text gehoeren zum Text",
           d[1][4] if len(d) > 1 else "", "text with [brackets] inside")
    pruefe("parse: Mehrfach-Check bleibt eine Zeichenkette",
           d[1][5] if len(d) > 1 else "", "bugprone-a,bugprone-b")
    pruefe("parse: error wird als error erkannt", d[2][3] if len(d) > 2 else "", "error")
    pruefe("parse: leerer Text liefert nichts", parse_diagnosen(""), [])

    # --- Deduplizierung -----------------------------------------------------
    wr = r"C:\Users\phili\nakama"
    a = (wr + r"\eq-copilot\plugin\src\a.cpp", 5, 3, "bugprone-x", "t")
    a2 = (wr + "/eq-copilot/plugin/src/A.CPP", 5, 3, "bugprone-x", "t")
    b = (wr + r"\eq-copilot\plugin\src\a.cpp", 5, 4, "bugprone-x", "t")
    c = (wr + r"\eq-copilot\plugin\src\a.cpp", 5, 3, "bugprone-y", "t")
    t = (wr + r"\eq-copilot\plugin\tests\t.cpp", 1, 1, "bugprone-x", "t")
    j = (wr + r"\eq-copilot\build-tidy\_deps\juce-src\j.h", 1, 1, "bugprone-x", "t")
    pruefe("dedup: dieselbe Fundstelle aus zwei Einheiten zaehlt einmal",
           len(dedupliziere([a, a2], wr)), 1)
    pruefe("dedup: andere Spalte zaehlt zweimal", len(dedupliziere([a, b], wr)), 2)
    pruefe("dedup: anderer Check am selben Ort zaehlt zweimal", len(dedupliziere([a, c], wr)), 2)
    pruefe("dedup: tests und JUCE zaehlen nicht", len(dedupliziere([t, j], wr)), 0)
    pruefe("dedup: Anzeigepfad ist relativ mit Vorwaertsstrichen",
           dedupliziere([a], wr)[0][0], "eq-copilot/plugin/src/a.cpp")
    pruefe("dedup: sortiert nach Datei, Zeile, Spalte",
           [f[2] for f in dedupliziere([b, a], wr)], [3, 4])

    # --- Bewertung: Ratsche haelt genau an der Grenze ------------------------
    pruefe("Grenze exakt getroffen reisst nicht",
           bewerte(TIDY_GRENZE, TIDY_ZIEL, TIDY_GRENZE), "ZIEL" if TIDY_GRENZE > TIDY_ZIEL else "OK")
    pruefe("Grenze plus eins reisst", bewerte(TIDY_GRENZE + 1, TIDY_ZIEL, TIDY_GRENZE), "GRENZE")
    pruefe("Null Fundstellen ist OK", bewerte(0, TIDY_ZIEL, TIDY_GRENZE), "OK")
    pruefe("nicht messbar ist nie rot", bewerte(None, TIDY_ZIEL, TIDY_GRENZE), "NICHT MESSBAR")
    pruefe("Ratsche ist nie unter dem Ziel", TIDY_GRENZE >= TIDY_ZIEL, True)

    # --- Umgebung -----------------------------------------------------------
    env = parse_umgebung("INCLUDE=C:\\a;C:\\b\nPfad=x=y\nkeine gleichheit\n=C:=C:\\\n")
    pruefe("Umgebung: erste Gleichheit trennt", env.get("Pfad"), "x=y")
    pruefe("Umgebung: INCLUDE kommt an", env.get("INCLUDE"), "C:\\a;C:\\b")
    pruefe("Umgebung: Zeilen ohne Gleichheit und Laufwerksmarken fallen weg", len(env), 2)

    # --- Check-Zaehler ------------------------------------------------------
    pruefe("je Check: absteigend, dann alphabetisch",
           je_check([("a", 1, 1, "z", ""), ("a", 2, 1, "b", ""), ("a", 3, 1, "z", "")]),
           [("z", 2), ("b", 1)])

    rot = [(n, t) for n, ok, t in faelle if not ok]
    for n, ok, t in faelle:
        print(("  ok   " if ok else "  ROT  ") + n + ("" if ok else "   " + t))
    print("")
    print(f"Selbsttest: {len(faelle) - len(rot)}/{len(faelle)} bestanden")
    return 0 if not rot else 2


# ------------------------------------------------------------------- Hauptlauf


def main() -> int:
    p = argparse.ArgumentParser(add_help=True, description=(
        "clang-tidy ueber die Uebersetzungseinheiten des Plugins, gezaehlt als Ratsche."))
    p.add_argument("--json", action="store_true", help="Ergebnis als JSON")
    p.add_argument("--jobs", type=int, default=os.cpu_count() or 4,
                   help="parallele clang-tidy-Prozesse (Standard: Prozessoren)")
    p.add_argument("--nur", metavar="TEILPFAD",
                   help="Pruefschalter: nur Einheiten, deren Pfad den Teil enthaelt (Teilmessung)")
    p.add_argument("--selbsttest", action="store_true", help="nur die eingebauten Faelle fahren")
    p.add_argument("--mit-selbsttest", action="store_true", dest="mit_selbsttest",
                   help="Selbsttest voranstellen, dann messen (Form des Kanon-Beins)")
    p.add_argument("--wurzel", help="Pruefschalter: anderen Baum messen")
    a = p.parse_args()

    # clang-tidy schreibt UTF-8, die Konsole des Runners nicht unbedingt; ein
    # nicht darstellbares Zeichen in einem Fundtext darf den Bericht nicht mit
    # einem Traceback beenden (das saehe wie ein Absturz des Werkzeugs aus).
    for strom in (sys.stdout, sys.stderr):
        if hasattr(strom, "reconfigure"):
            strom.reconfigure(errors="replace")

    if a.selbsttest:
        return selbsttest()
    if a.mit_selbsttest:
        print("SELBSTTEST")
        if selbsttest() != 0:
            print("")
            print("ABBRUCH: Selbsttest rot - es wird nicht gemessen.", file=sys.stderr)
            return 2
        print("")

    wurzel = pathlib.Path(a.wurzel).resolve() if a.wurzel else WURZEL
    try:
        werkzeuge = finde_werkzeuge()
    except Voraussetzung as e:
        print(f"Voraussetzung fehlt: {e}", file=sys.stderr)
        return 3

    beginn = time.monotonic()
    try:
        env = vs_umgebung(werkzeuge["vcvars"])
        db, dauer_konfig = konfiguriere(wurzel, werkzeuge, env)
        eintraege = filtere_datenbank(lade_datenbank(db), norm(os.path.realpath(str(wurzel))))
        if a.nur:
            eintraege = [e for e in eintraege if a.nur.lower() in norm(e["file"])]
        if not eintraege:
            raise Werkzeugfehler(f"keine Uebersetzungseinheit unter {QUELLORT} in {db}"
                                 + (f" mit Teilpfad {a.nur!r}" if a.nur else ""))
        funde, fehler, ergebnisse = miss(wurzel, werkzeuge, env, eintraege, a.jobs)
    except Werkzeugfehler as e:
        print(f"Werkzeugfehler: {e}", file=sys.stderr)
        return 2

    ist = len(funde)
    status = bewerte(ist, TIDY_ZIEL, TIDY_GRENZE)
    kopf = {
        "Wurzel": str(wurzel) + ("  (UEBERSCHRIEBEN)" if a.wurzel else ""),
        "Baum": f"{BAUORDNER} (Ninja, cl.exe, Configure {dauer_konfig:.0f} s)",
        "clang-tidy": f"{werkzeuge['clang_tidy']} - {version_von(werkzeuge['clang_tidy'], env)}",
        "MSVC": env.get("VCToolsVersion", "(VCToolsVersion nicht gesetzt)"),
        "Regelsatz": f"{QUELLORT}/.clang-tidy",
        "Dauer": f"{time.monotonic() - beginn:.0f} s mit {a.jobs} Prozess(en)",
    }
    if a.json:
        print(json.dumps({
            "wurzel": str(wurzel), "clang_tidy": kopf["clang-tidy"], "msvc": kopf["MSVC"],
            "teilmessung": a.nur, "einheiten": len(ergebnisse),
            "nicht_messbar": [dict(einheit=e, gruende=g) for e, g in fehler],
            "ist": ist, "ziel": TIDY_ZIEL, "grenze": TIDY_GRENZE, "status": status,
            "je_check": dict(je_check(funde)),
            "treffer": [dict(datei=d, zeile=z, spalte=s, check=c, text=t) for d, z, s, c, t in funde],
            "dauer_s": round(time.monotonic() - beginn),
        }, indent=2, ensure_ascii=True))
    else:
        drucke_bericht(kopf, funde, fehler, ergebnisse, status, a.nur)

    if fehler:
        return 2
    if a.nur:
        return 0
    return 4 if status == "GRENZE" else 0


if __name__ == "__main__":
    sys.exit(main())
