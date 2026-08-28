#!/usr/bin/env python3
"""Codegen-Drift-Riegel fuer den FlatBuffers-Vertrag (SONDE-005b).

Entwurf §65 gibt `SONDE-005` das Gate:

    "C++/Rust validieren identisch; **Codegen-Drift ist 0**"

und §53 sagt, wie das gemeint ist:

    "Eine gepinnte flatc-Version erzeugt C++ und Rust; die generierten
     Dateien werden committed und ein Drift-Test verlangt bitgleichen
     Neugenerierungsdiff."

Dieses Skript ist dieser Test. Es prueft fuenf Dinge, und vier davon sind
Voraussetzungen der fuenften:

  1. `flatc` ist DA und traegt die gepinnte Version;
  2. sein CMake-POST_BUILD misst im sauberen Checkout den tatsaechlichen
     Git-Commit UND den SHA-256 des gebauten Binaries; beides stimmt mit Pin
     und ausgefuehrter Datei ueberein (die Version allein unterscheidet die
     mehreren 25.12.19-Schnitte nicht);
  3. die Rust-Crate `flatbuffers` traegt DIESELBE Version (der erzeugte Code
     ruft in die Laufzeit; eine andere Version ist entweder ein
     Uebersetzungsfehler oder - schlimmer - stille Inkompatibilitaet);
  4. der Feld-ID-Riegel ist gruen (Aufruf von `pruefe_fbs_feldids.py`);
  5. die Neugenerierung ist BYTEGLEICH zum committeten Stand.

Mit `--conform` kommt eine sechste Pruefung dazu: dass `flatc --conform`
wirklich scharf ist. Ein Riegel, den niemand fallen gesehen hat, ist eine
Behauptung - deshalb werden vier schemabrechende Mutationen erzeugt und
gemessen, dass jede abgelehnt wird.

Exitcodes: 0 gruen · 2 Drift oder Riegel rot · 3 Voraussetzung fehlt.

Der Sidecar `nakama-flatc-commit-<CONFIG>.txt` hat exakt dieses Format:

    commit <40 hexadezimale Zeichen>
    sha256 <64 hexadezimale Zeichen>

Nur der POST_BUILD-Schritt des `flatc`-Ziels schreibt ihn. Fuer isolierte
Riegelproben koennen Binary und Sidecar mit `--flatc` und `--beleg` umgebogen
werden; ohne diese Argumente gelten weiter die Pfadzeiger aus dem Release-Bau.
"""

from __future__ import annotations

import argparse
import hashlib
import json
import pathlib
import re
import subprocess
import sys
import tempfile

WURZEL = pathlib.Path(__file__).resolve().parents[2]
WERKZEUG = WURZEL / "eq-copilot/schemas/v3/flatbuffers/WERKZEUG.json"
BAU = WURZEL / "eq-copilot/build"
CARGO = WURZEL / "broker/Cargo.toml"
KONFIGURATION = "Release"


def sha256(pfad: pathlib.Path) -> str:
    return hashlib.sha256(pfad.read_bytes()).hexdigest()


def finde_flatc(vorgegeben: pathlib.Path | None = None) -> pathlib.Path | None:
    """Der Bau schreibt den Pfad hin; wir raten ihn nicht.

    Ein geratener Pfad, der ins Leere zeigt, saehe aus wie "kein flatc" - und
    der Drift-Test wuerde uebersprungen. Genau die Pruefung, die nicht
    fehlschlagen kann.
    """
    if vorgegeben is not None:
        if vorgegeben.is_file():
            return vorgegeben
        print(f"  ROT: vorgegebenes flatc gibt es nicht: {vorgegeben}")
        return None

    zeiger = BAU / f"nakama-flatc-pfad-{KONFIGURATION}.txt"
    if zeiger.exists():
        kandidat = pathlib.Path(zeiger.read_text(encoding="utf-8").strip())
        if kandidat.exists():
            return kandidat
        print(f"  ROT: Zeiger {zeiger} verweist auf {kandidat}, das es nicht gibt")
        return None
    return None


def belegter_bau(vorgegeben: pathlib.Path | None = None) \
        -> tuple[str | None, str | None, pathlib.Path, str | None]:
    """Liest Bauzeit-Commit und Binary-Hash aus dem POST_BUILD-Sidecar.

    WERKZEUG.json darf nicht selbst Quelle dieses Werts sein: dann wuerde der
    Pruefer wieder nur die Behauptung ausgeben, die er pruefen soll. Fehlender
    oder ungueltiger Beleg ist ROT und wird nie als uebersprungene Voraussetzung
    behandelt, weil ein vorhandenes Binary ohne Provenienz kein gepinntes
    Werkzeug ist.
    """
    beleg = (vorgegeben if vorgegeben is not None
             else BAU / f"nakama-flatc-commit-{KONFIGURATION}.txt")
    if not beleg.exists():
        return None, None, beleg, "fehlt"
    try:
        zeilen = beleg.read_text(encoding="ascii").splitlines()
    except (OSError, UnicodeError) as fehler:
        return None, None, beleg, f"nicht lesbar: {fehler}"
    if len(zeilen) != 2:
        return None, None, beleg, "Format ist nicht exakt zweizeilig"

    commit_treffer = re.fullmatch(r"commit ([0-9a-fA-F]{40})", zeilen[0])
    hash_treffer = re.fullmatch(r"sha256 ([0-9a-fA-F]{64})", zeilen[1])
    if commit_treffer is None or hash_treffer is None:
        return None, None, beleg, "Format oder Hex-Laenge ist ungueltig"
    return (commit_treffer.group(1).lower(), hash_treffer.group(1).lower(),
            beleg, None)


def argumente_lesen(argv: list[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--conform", action="store_true",
        help="zusaetzlich vier brechende Schema-Mutationen pruefen")
    parser.add_argument(
        "--flatc", type=pathlib.Path,
        help="flatc-Binary fuer eine isolierte Riegelprobe (sonst Release-Pfadzeiger)")
    parser.add_argument(
        "--beleg", type=pathlib.Path,
        help="Commit/SHA-256-Sidecar fuer eine isolierte Riegelprobe")
    return parser.parse_args(argv)


def version_von(flatc: pathlib.Path) -> str | None:
    lauf = subprocess.run([str(flatc), "--version"], capture_output=True, text=True)
    if lauf.returncode != 0:
        return None
    treffer = re.search(r"(\d+\.\d+\.\d+)", lauf.stdout + lauf.stderr)
    return treffer.group(1) if treffer else None


def cargo_version() -> tuple[str | None, str | None]:
    """(Anforderung aus Cargo.toml, AUFGELOESTE Version aus Cargo.lock).

    T2-Runde 3, Befund 4: vorher wurde nur die Anforderung gelesen.
    `flatbuffers = "25.12.19"` bedeutet in Cargo aber `^25.12.19`, also alles
    unter 26.0.0 - nach einem `cargo update` haette der Riegel weiter gruen
    gemeldet, waehrend der erzeugte Code in eine ANDERE Laufzeit ruft. Genau
    das, was WERKZEUG.json als "schlimmer - stille Inkompatibilitaet"
    beschreibt. Die Wahrheit steht in Cargo.lock.
    """
    anforderung = None
    if CARGO.exists():
        for zeile in CARGO.read_text(encoding="utf-8").splitlines():
            treffer = re.match(r'\s*flatbuffers\s*=\s*"([^"]+)"', zeile)
            if treffer:
                anforderung = treffer.group(1)
                break

    aufgeloest = None
    lock = CARGO.with_name("Cargo.lock")
    if lock.exists():
        zeilen = lock.read_text(encoding="utf-8").splitlines()
        for i, zeile in enumerate(zeilen):
            if zeile.strip() == 'name = "flatbuffers"':
                for folge in zeilen[i + 1:i + 4]:
                    m = re.match(r'\s*version\s*=\s*"([^"]+)"', folge)
                    if m:
                        aufgeloest = m.group(1)
                        break
                break
    return anforderung, aufgeloest


def erzeuge(flatc: pathlib.Path, aufruf: dict, schema: pathlib.Path,
            ziel: pathlib.Path) -> subprocess.CompletedProcess:
    ziel.mkdir(parents=True, exist_ok=True)
    befehl = [str(flatc), *aufruf["argumente"], "-o", str(ziel), str(schema)]
    return subprocess.run(befehl, capture_output=True, text=True)


def pruefe_conform(flatc: pathlib.Path, schema: pathlib.Path) -> int:
    """Fuehrt vor, dass `flatc --conform` wirklich ablehnt.

    Die vier Mutationen sind genau die Regeln aus Entwurf §53: ein Feld
    entfernen, seinen Typ aendern, eine id umhaengen, einen Enumwert
    verschieben. Wer eine davon durchlaesst, hat kein Gate.
    """
    original = schema.read_text(encoding="utf-8")
    mutationen = [
        ("Feld entfernt",
         lambda t: t.replace("  saturated:bool (id: 5);\n", "")),
        ("Feldtyp geaendert",
         lambda t: t.replace("sample_count:uint (id: 5);", "sample_count:ulong (id: 5);")),
        ("Feld-ID umgehaengt",
         lambda t: t.replace("metrics_version:uint (id: 2);", "metrics_version:uint (id: 9);")
                    .replace("korrelation:float = null (id: 9);", "korrelation:float = null (id: 2);")),
        ("Enumwert verschoben",
         lambda t: t.replace("nakama_log64_v1 = 2,", "nakama_log64_v1 = 3,")),
    ]

    rot = 0
    with tempfile.TemporaryDirectory() as tmp:
        tmpp = pathlib.Path(tmp)
        for name, mutiere in mutationen:
            text = mutiere(original)
            if text == original:
                print(f"  ROT: Mutation '{name}' hat nichts geaendert - der Anker stimmt nicht mehr")
                rot += 1
                continue
            # T2-Runde 3, Befund 7: `str.replace` ersetzt ALLE Vorkommen. Die
            # Gegenprobe oben faengt nur "gar nichts geaendert", nicht "an zwei
            # Stellen geaendert" - dann pruefte die Mutation etwas anderes, als
            # ihr Name sagt. Der Laengenunterschied verraet es.
            if abs(len(text) - len(original)) > 400:
                print(f"  ROT: Mutation '{name}' hat zu viel veraendert "
                      f"({abs(len(text) - len(original))} Zeichen) - traf der Anker mehrfach?")
                rot += 1
                continue
            kandidat = tmpp / "kandidat.fbs"
            kandidat.write_text(text, encoding="utf-8", newline="")
            lauf = subprocess.run(
                [str(flatc), "--conform", str(schema), "--cpp", "-o", str(tmpp / "aus"),
                 str(kandidat)],
                capture_output=True, text=True)
            meldung = (lauf.stdout + lauf.stderr).strip().splitlines()
            grund = next((z.strip() for z in meldung if "conform" in z), "")
            if lauf.returncode == 0:
                print(f"  ROT: '{name}' wurde AKZEPTIERT - --conform ist nicht scharf")
                rot += 1
            else:
                print(f"  ok:  '{name}' abgelehnt (Exit {lauf.returncode}) - {grund}")

        # Und die Gegenprobe: der unveraenderte Stand MUSS durchgehen, sonst
        # meldet der Riegel nur, dass flatc immer schimpft.
        unveraendert = tmpp / "gleich.fbs"
        unveraendert.write_text(original, encoding="utf-8", newline="")
        lauf = subprocess.run(
            [str(flatc), "--conform", str(schema), "--cpp", "-o", str(tmpp / "aus2"),
             str(unveraendert)],
            capture_output=True, text=True)
        if lauf.returncode != 0:
            print(f"  ROT: der UNVERAENDERTE Stand wird abgelehnt (Exit {lauf.returncode}) - "
                  "der Riegel meldet nur, dass flatc immer schimpft")
            rot += 1
        else:
            print("  ok:  unveraenderter Stand wird akzeptiert (Gegenprobe)")

    return rot


def main(argv: list[str]) -> int:
    argumente = argumente_lesen(argv)
    if not WERKZEUG.exists():
        print(f"VORAUSSETZUNG FEHLT: {WERKZEUG} nicht gefunden")
        return 3
    steckbrief = json.loads(WERKZEUG.read_text(encoding="utf-8"))
    schema = WURZEL / steckbrief["schema"]
    if not schema.exists():
        print(f"VORAUSSETZUNG FEHLT: {schema} nicht gefunden")
        return 3

    print(f"Gepinnt: flatbuffers {steckbrief['version']} @ {steckbrief['git_commit'][:12]}")

    flatc = finde_flatc(argumente.flatc)
    if flatc is None:
        print("VORAUSSETZUNG FEHLT: flatc nicht gefunden.")
        print("  Der Bau schreibt seinen Pfad nach eq-copilot/build/nakama-flatc-pfad-Release.txt.")
        print("  Nachziehen mit: cmake --build eq-copilot/build --config Release --target flatc")
        return 3

    gemessen = version_von(flatc)
    if gemessen is None:
        print(f"VORAUSSETZUNG FEHLT: {flatc} laesst sich nicht nach seiner Version fragen")
        return 3
    if gemessen != steckbrief["version"]:
        # T2-Runde 3, Befund 10: das war frueher Exit 3. Aber das Werkzeug ist
        # DA - was es meldet, widerspricht dem Pin. Das ist eine widerlegte
        # Behauptung (2), keine fehlende Voraussetzung (3). Der Unterschied
        # traegt: 3 heisst "nicht gemessen", 2 heisst "gemessen und falsch".
        print(f"  ROT: flatc meldet {gemessen}, gepinnt ist {steckbrief['version']}. "
              "'Drift ist 0' waere damit eine Aussage ueber ein anderes Werkzeug - "
              "entweder ist der Steckbrief manipuliert oder der Bau veraltet.")
        return 2
    print(f"  flatc: {gemessen}  ({flatc})")

    commit, beleg_hash, commit_beleg, beleg_fehler = belegter_bau(argumente.beleg)
    if beleg_fehler is not None:
        print(f"  ROT: Binary-Beleg {beleg_fehler}: {commit_beleg}. Ein vorhandenes "
              "flatc mit passender --version belegt bei mehreren Upstream-Schnitten "
              "nicht den gepinnten Quellstand.")
        return 2
    erwartet = steckbrief["git_commit"].lower()
    if commit != erwartet:
        print(f"  ROT: flatc-Commit {commit} weicht vom Pin {erwartet} ab "
              f"(Beleg: {commit_beleg}).")
        return 2
    try:
        binary_hash = sha256(flatc)
    except OSError as fehler:
        print(f"  ROT: SHA-256 des ausgefuehrten flatc nicht lesbar: {flatc}: {fehler}")
        return 2
    if beleg_hash != binary_hash:
        print(f"  ROT: flatc-SHA-256 {binary_hash} weicht vom gebundenen Hash "
              f"{beleg_hash} ab (Beleg: {commit_beleg}).")
        return 2
    print(f"  flatc-Beleg: Commit {commit}, sha256={binary_hash}  ({commit_beleg})")

    anforderung, aufgeloest = cargo_version()
    if anforderung != steckbrief["rust_crate"]:
        print(f"  ROT: broker/Cargo.toml fordert flatbuffers = {anforderung!r}, gepinnt "
              f"ist {steckbrief['rust_crate']!r}.")
        return 2
    if aufgeloest is None:
        print("  ROT: broker/Cargo.lock fuehrt kein flatbuffers - die tatsaechlich "
              "verwendete Version ist damit unbekannt.")
        return 2
    if aufgeloest != steckbrief["rust_crate"]:
        print(f"  ROT: broker/Cargo.lock loest flatbuffers auf {aufgeloest!r} auf, "
              f"gepinnt ist {steckbrief['rust_crate']!r}. Der erzeugte Rust-Code ruft "
              "in eine Laufzeit, die er nicht kennt.")
        return 2
    print(f"  Rust-Crate: {aufgeloest} (Cargo.lock; Anforderung {anforderung})")

    # Der Feld-ID-Riegel gehoert dazu: ein Schema ohne ids waere bitgleich
    # reproduzierbar und trotzdem falsch.
    ids = subprocess.run([sys.executable, str(WURZEL / "tools/eq-copilot/pruefe_fbs_feldids.py")],
                         capture_output=True, text=True)
    if ids.returncode != 0:
        print("  ROT: der Feld-ID-Riegel ist rot:")
        print("    " + ids.stdout.strip().replace("\n", "\n    "))
        return 2
    print("  Feld-IDs: 0 rot")

    if argumente.conform:
        print("\n--conform (Evolutionsriegel):")
        rot = pruefe_conform(flatc, schema)
        print(f"\nPruefungen: {'0 rot' if rot == 0 else f'{rot} rot'}")
        return 0 if rot == 0 else 2

    print("\nCodegen-Drift:")
    drift: list[str] = []
    with tempfile.TemporaryDirectory() as tmp:
        for aufruf in steckbrief["codegen_aufrufe"]:
            ziel = pathlib.Path(tmp) / aufruf["sprache"]
            lauf = erzeuge(flatc, aufruf, schema, ziel)
            if lauf.returncode != 0:
                print(f"  ROT: flatc {aufruf['sprache']} scheiterte (Exit {lauf.returncode}): "
                      f"{(lauf.stdout + lauf.stderr).strip()[:300]}")
                return 2

            frisch = ziel / aufruf["datei"]
            committed = WURZEL / aufruf["ziel"] / aufruf["datei"]
            if not committed.exists():
                print(f"  ROT: {aufruf['ziel']}/{aufruf['datei']} ist nicht committed")
                drift.append(aufruf["datei"])
                continue

            a, b = sha256(committed), sha256(frisch)
            if a != b:
                print(f"  ROT: DRIFT in {aufruf['ziel']}/{aufruf['datei']}")
                print(f"       committed  sha256={a}")
                print(f"       neu erzeugt sha256={b}")
                drift.append(aufruf["datei"])
            else:
                zeilen = len(committed.read_text(encoding="utf-8").splitlines())
                print(f"  ok:  {aufruf['ziel']}/{aufruf['datei']}  "
                      f"{zeilen} Zeilen  sha256={a[:16]}...")

    if drift:
        print(f"\nDrift: {len(drift)} Datei(en) - die committete Fassung ist nicht die, "
              "die das Schema heute erzeugt.")
        return 2
    print(f"\nDrift: 0 Dateien")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
