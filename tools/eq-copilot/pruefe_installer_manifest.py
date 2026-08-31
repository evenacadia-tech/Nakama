#!/usr/bin/env py -3.13
"""S9/SONDE-007b Abschnitt 3 - Kanon-Bein A17: das Installer-Manifest.

WAS DIESES BEIN BEHAUPTET

  Das Installer-Manifest (eq-copilot/install/nakama-installer-v1.json) traegt
  KEINE zweite Identitaet. Es nennt nur Ziel-IDs; jeder Produktname, jeder
  Viercode und jede Class-ID bleibt in identity/plugin-identities-v1.json.
  Das ist NAK-52 auf der Installerseite: ein Paket, das Bundlenamen selbst
  fuehrt, kann eine Datei ausliefern, deren Name nirgends eingefroren ist.
  Die Quellpfade werden deshalb nicht gelesen, sondern NACHGERECHNET.

DIE BLOECKE

  [1] Struktur      15 Regeln (REGELN) ueber Manifest und Identitaetsdatei:
                    Schema, Identitaetsquelle, geschlossene `art`-Menge,
                    jedes aktive Ziel genau einmal, stillgelegte Ziele
                    benannt und ihre Marke lesbar, Quellpfad nachgerechnet,
                    keine Identitaetsliterale, Broker aus Cargo.toml,
                    geschuetzte Zielverzeichnisse, ehrliche Signaturzeile,
                    Hashfelder, bekannte Staende, vollstaendiger Rueckweg.
  [2] Gegenprobe    dieselben Regeln an einem absichtlich verdorbenen
                    Manifest (und einer verdorbenen Identitaet): jede Regel
                    muss dort FALLEN, sonst prueft sie nichts.
  [3] Kanten        die konkreten Umgehungen einzeln gebrochen - Bundle-
                    Zielkollision, state_schema-Drift, stillgelegtes Ziel
                    wieder ausgeliefert oder still verschwunden, unlesbare
                    Stilllegungsmarke, ungueltige ziel_id-Typen, Literale
                    ausserhalb der Pfade, Pfadtraversal.
  [4] Auslieferung  gebautes Artefakt gegen den festgeschriebenen Hash. Im
                    Kanon ist eine Abweichung ein HINWEIS (ein Relink aendert
                    Bytes - seit A14 den Kern neu baut, der Normalfall);
                    mit --release HART. Fehlendes Artefakt oder nicht
                    bildbarer Ordner-Hash bleiben in beiden Modi Fehler
                    (NAK-94).
  [4c] Startbindung  der von CMake erzeugte Produktheader stimmt in Pfad,
                    SHA-256 und optionalem Thumbprint mit dem Manifest
                    ueberein; Manifest-Aenderungen sind Configure-Depends.
  [4b] Installiert  Bericht ohne Urteil, der nie abbricht: entspricht der
                    installierte Stand (install-ergebnis.json) dem Manifest?
                    `ok` nur bei Journalstatus OK; Status vor Liste.
  [5] Kreuzprobe    Ordner-Hash v1 (Vertrag §2.1) in Python gegen die
                    PowerShell-Haelfte (NakamaOrdnerHash.ps1) an einem
                    synthetischen Ordner; Nicht-ASCII bricht beidseitig ab.
  [6] Signatur      Das im Test aus einer signierten Windows-Systemfixture
                    gelesene, OS-vertrauenswuerdige Signerzertifikat faehrt
                    die produktive C++-WinVerifyTrust-Kette fuer gueltig und
                    falschen Thumbprint; eine unsigned Temp-Kopie liefert
                    den Fehlend-Fall. Kein Zertifikatsspeicher wird geaendert.

  Jede gelesene JSON-Datei geht VOR dem ersten Zugriff durch einen
  Strukturvertrag (Schluessel und Grobform); ein Verstoss ist ein
  kontrollierter Abbruch mit Klartext, nie ein Traceback. Ein zentraler
  Faenger (_geschuetzt) verwandelt jede andere Ausnahme in eine Klartextzeile
  mit Ort, Exit 2 (Traceback nur mit --debug).

WAS DIESES BEIN NICHT MEHR TUT (NAK-100, 30.08.2026)

  Bis dahin trug es [3b] (Journal-Gegenproben Z1..Z7 ueber einen vom
  Installer erzeugten Fixture-Korpus mit eigenem Erzeuger) und [3c] (Byte-
  Kipp-Fuzz ueber jede gelesene JSON-Datei mit Deckungsrechnung) - zusammen
  rund 1 400 Zeilen, die nicht das Gate des Tickets massen, sondern die
  Robustheit des Pruefskripts gegen gekippte Bytes in seinen eigenen
  Eingaben. Das Gate (eine Identitaet, ein Ort; Auslieferung nachgerechnet;
  Rueckweg vollstaendig) messen [1]..[5]. Die Zusagen aus NAK-94 ([4] weich
  im Kanon, [4b] Bericht, Status vor Liste) bleiben.

AUFRUF

  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py            # Kanon
  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --release  # hart
  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --hashen   # festschreiben

  Exit 0 gruen, 2 rot oder kontrollierter Abbruch, 3 --hashen ohne
  vollstaendige Artefakte.
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import ntpath
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import traceback
from datetime import datetime, timezone

WURZEL = pathlib.Path(__file__).resolve().parents[2]
MANIFEST = WURZEL / "eq-copilot" / "install" / "nakama-installer-v1.json"
INSTALL_ERGEBNIS = WURZEL / "eq-copilot" / "install" / "install-ergebnis.json"
IDENTITAET = WURZEL / "eq-copilot" / "identity" / "plugin-identities-v1.json"
INSTALLER = WURZEL / "eq-copilot" / "install" / "Install-Nakama.ps1"
BROKER_CARGO = WURZEL / "broker" / "Cargo.toml"
PS_ORDNERHASH = WURZEL / "eq-copilot" / "install" / "NakamaOrdnerHash.ps1"
STATE_CPP = WURZEL / "eq-copilot" / "plugin" / "state" / "NakamaState.cpp"
IPC_TEST_EXE = (WURZEL / "eq-copilot" / "build" / "plugin"
                / "EqCopIpcTest_artefacts" / "Release" / "EqCopIpcTest.exe")
BROKER_BINDING = (WURZEL / "eq-copilot" / "build" / "plugin" / "generated"
                  / "nakama" / "BrokerInstallBinding.h")
PLUGIN_CMAKE = WURZEL / "eq-copilot" / "plugin" / "CMakeLists.txt"

SCHEMA = "nakama.installer/v1"
ERGEBNIS_SCHEMA = "nakama.install-ergebnis/v1"
# Der EINZIGE Journalstatus, unter dem der gespeicherte sha256 den Stand
# beschreibt, der wirklich liegt. Install-Nakama.ps1 kennt daneben
# VORBEREITET, KOMPENSATION, ERROR_TEILSTAND, ERROR_RUECKGEROLLT,
# RUECKWEG_AKTIV und RUECKWEG - jeder davon steht fuer ein Ziel, das bereits
# ganz oder halb wiederhergestellt sein kann (NAK-94 Nacharbeit 2).
ERGEBNIS_STATUS_OK = "OK"

HEX64 = re.compile(r"^[0-9A-F]{64}$")
THUMBPRINT = re.compile(r"^(?:[0-9A-F]{40}|[0-9A-F]{64})$")
ARTEN = ("vst3", "broker")
CMAKE_ZIEL = re.compile(r"^[A-Za-z][A-Za-z0-9_]*$")


# ─@@STRICH@ Strukturvertrag der gelesenen JSON-Dateien ──────────────────────────
#
# BEFUND NAK-94, Pruefer 9 (30.08.2026): eine einzelne Byteaenderung von
# "faelle" zu "xaelle" in fixtures/installer/journale/MANIFEST.json liess den
# Lauf mit `KeyError: 'faelle'` sterben, BEVOR Z1 ein rotes Urteil oder den
# vorgesehenen Klartext-Abbruch ausgeben konnte. Dieselbe Tuer stand an den
# beiden Dateien offen, die main() liest - gemessen am Basis-Stand b8dcbe1:
# "artefakte" -> "xrtefakte" im Installer-Manifest starb in r_art_bekannt,
# "ziele" -> "xiele" in der Identitaetsdatei in _ziele(). Ein Traceback ist
# kein Urteil: Unbekanntes ist ROT, nicht laut (Pruefliste D).
#
# Regel seither: JEDE von diesem Bein gelesene JSON-Datei geht durch eine
# Strukturpruefung, BEVOR irgendein Feld benutzt wird; ein Verstoss ist ein
# kontrollierter Abbruch mit Klartext und Exit ungleich 0, nie ein Traceback.
# Geprueft werden SCHLUESSEL und Grobform der GELESENEN Felder - ueber ihre
# WERTE urteilen die Regeln in [1]/[2] und die Zusagen Z1..Z7, und eine
# Pruefung, die ihnen vorgriffe, verschluckte genau die Brueche, die sie
# belegen sollen. Die einzige Ausnahme ist der urteilsfreie Berichtsblock
# [4b] (install-ergebnis.json): dort ist ein Verstoss ein kontrollierter
# HINWEIS mit Rueckkehr, weil dieser Block per Zusage nie urteilt und nie
# abbricht (Befund C2, NAK-94 Nacharbeit 2). Auch dort steht die Pruefung vor
# dem Zugriff - nur ihr Ausgang ist ein anderer.


class Strukturhalt(Exception):
    """Kontrollierter Abbruch: eine gelesene JSON-Datei traegt nicht die Form,
    die dieses Bein liest. Klartext statt Traceback."""


def _lies_geprueft(weg: pathlib.Path, pruefung) -> dict:
    """Liest eine JSON-Datei und gibt sie NUR strukturgeprueft heraus.

    Drei Stufen, jede mit eigenem Klartext: lesbar, gueltiges JSON, und die
    von diesem Bein gelesene Grobform. Jede Stufe wirft `Strukturhalt`; der
    Aufrufer macht daraus ein rotes Urteil oder einen Abbruch - nie einen
    Traceback."""
    kurz = weg.relative_to(WURZEL).as_posix()
    try:
        rohe = weg.read_text(encoding="utf-8")
    except OSError as fehler:
        raise Strukturhalt(f"{kurz}: nicht lesbar "
                           f"({type(fehler).__name__}: {fehler})") from None
    except UnicodeDecodeError as fehler:
        # NAK-94 Nacharbeit 10, Befund des zehnten Pruefers: ein einzelnes
        # gekipptes Byte (0x7B -> 0xFF) machte hier einen Traceback statt
        # eines Strukturhalts. UnicodeDecodeError ist ein ValueError, kein
        # OSError - der Zweig darueber fing ihn nicht.
        raise Strukturhalt(f"{kurz}: keine gueltige UTF-8-Datei "
                           f"({type(fehler).__name__}: {fehler})") from None
    try:
        daten = json.loads(rohe)
    except json.JSONDecodeError as fehler:
        raise Strukturhalt(f"{kurz}: kein gueltiges JSON ({fehler})") from None
    klagen = pruefung(daten)
    if klagen:
        raise Strukturhalt(f"{kurz}: traegt nicht die von diesem Bein gelesene "
                           "Struktur: " + "; ".join(klagen))
    return daten


def _installermanifest_struktur(m: object) -> list[str]:
    """Traegt das Installer-Manifest die von [1]..[4] GELESENE Grobform?

    Verlangt wird nur, was dieses Bein HART liest (`m["..."]`, nicht `.get`):
    die Wurzel als Objekt, `artefakte` als nicht leere Liste von Objekten mit
    einer Zeichenkette `quelle` (der Pfad, den [4] und `hashen` zusammensetzen),
    `ziele` als Objekt und `rueckweg` als Objekt mit der Liste
    `bekannte_staende`. `stillgelegte_ziele` steht NICHT hier: jeder Zugriff
    darauf laeuft schon ueber `.get` samt Typpruefung.

    NAK-94 Nacharbeit 10 (Befund des zehnten Pruefers): dazu `ziel_id` UND
    `cmake_ziel` als Zeichenketten an jedem VST3-Artefakt.
    `adversariale_strukturproben()` liest beide HART, und je eine einzelne
    Byteaenderung ("ziel_id" -> "xiel_id" bzw. ein gekipptes `cmake_ziel`)
    endete dort mit einem KeyError; den zweiten Fall hat erst der
    Byte-Kipp-Fuzz [3c] gefunden, nicht der Pruefer. Die Auswahl
    `art == "vst3"` steht hier genauso wie dort: eine Grobform-Regel darf nur
    verlangen, was wirklich gelesen wird."""
    if not isinstance(m, dict):
        return [f"Wurzel ist kein Objekt ({type(m).__name__})"]
    fehlt: list[str] = []
    artefakte = m.get("artefakte")
    if not isinstance(artefakte, list) or not artefakte:
        fehlt.append("keine nicht leere Liste 'artefakte' "
                     f"({type(artefakte).__name__})")
    else:
        for index, a in enumerate(artefakte):
            if not isinstance(a, dict):
                fehlt.append(f"artefakte[{index}] ist kein Objekt "
                             f"({type(a).__name__})")
                continue
            if not isinstance(a.get("quelle"), str):
                fehlt.append(f"artefakte[{index}] ohne Zeichenkette 'quelle' "
                             f"({type(a.get('quelle')).__name__})")
            if a.get("art") == "vst3":
                for feld in ("ziel_id", "cmake_ziel"):
                    if not isinstance(a.get(feld), str):
                        fehlt.append(f"artefakte[{index}] (vst3) ohne "
                                     f"Zeichenkette {feld!r} "
                                     f"({type(a.get(feld)).__name__})")
    ziele = m.get("ziele")
    if not isinstance(ziele, dict):
        fehlt.append(f"'ziele' ist kein Objekt ({type(ziele).__name__})")
    rueckweg = m.get("rueckweg")
    if not isinstance(rueckweg, dict):
        fehlt.append(f"'rueckweg' ist kein Objekt ({type(rueckweg).__name__})")
    elif not isinstance(rueckweg.get("bekannte_staende"), list):
        fehlt.append("'rueckweg.bekannte_staende' ist keine Liste "
                     f"({type(rueckweg.get('bekannte_staende')).__name__})")
    return fehlt


def _identitaet_struktur(i: object) -> list[str]:
    """Traegt die Identitaetsdatei die von diesem Bein GELESENE Grobform?

    Hart gelesen werden `i["ziele"]` (in `_ziele`, `_aktive` und den
    Gegenproben), `z["id"]` je Ziel und `i["hersteller"]["code"]`; von einem
    AKTIVEN Ziel zusaetzlich `produktname` und `bundle`, die
    `adversariale_strukturproben` woertlich in ein Probemanifest schreibt.
    Ueber die WERTE - Kollisionsfreiheit, schema=2, Bundlenamen - urteilt
    weiter `r_jedes_ziel_genau_einmal`, nicht diese Pruefung."""
    if not isinstance(i, dict):
        return [f"Wurzel ist kein Objekt ({type(i).__name__})"]
    fehlt: list[str] = []
    ziele = i.get("ziele")
    if not isinstance(ziele, list) or not ziele:
        fehlt.append(f"keine nicht leere Liste 'ziele' ({type(ziele).__name__})")
    else:
        for index, z in enumerate(ziele):
            if not isinstance(z, dict):
                fehlt.append(f"ziele[{index}] ist kein Objekt "
                             f"({type(z).__name__})")
                continue
            if not isinstance(z.get("id"), str):
                fehlt.append(f"ziele[{index}] ohne Zeichenkette 'id' "
                             f"({type(z.get('id')).__name__})")
            if "stillgelegt" in z:
                continue
            for feld in ("produktname", "bundle"):
                if not isinstance(z.get(feld), str):
                    fehlt.append(f"ziele[{index}] (aktiv) ohne Zeichenkette "
                                 f"{feld!r} ({type(z.get(feld)).__name__})")
    hersteller = i.get("hersteller")
    if not isinstance(hersteller, dict):
        fehlt.append("'hersteller' ist kein Objekt "
                     f"({type(hersteller).__name__})")
    elif not isinstance(hersteller.get("code"), str):
        fehlt.append("'hersteller.code' ist keine Zeichenkette "
                     f"({type(hersteller.get('code')).__name__})")
    return fehlt


def _writer_state_schema() -> int:
    text = STATE_CPP.read_text(encoding="utf-8")
    treffer = re.search(r"constexpr\s+int\s+kRootSchema\s*=\s*(\d+)\s*;", text)
    if treffer is None:
        raise RuntimeError(f"kRootSchema fehlt in {STATE_CPP}")
    return int(treffer.group(1))


STATE_SCHEMA = _writer_state_schema()


# ── Ordner-Hash v1 (Vertrag §2.1) ───────────────────────────────────────────


class OrdnerHashFehler(Exception):
    """Der Ordner verlaesst den Bereich, in dem beide Implementierungen
    nachweislich dasselbe rechnen. Lieber laut abbrechen als zwei Zahlen."""


def _ascii_pfad(rel: str) -> bool:
    return all(0x20 <= ord(z) <= 0x7E for z in rel)


def ordner_hash(ordner: pathlib.Path) -> str:
    """SHA-256 ueber einen Ordner nach Vertrag §2.1.

    Zeile je DATEI: `<64 Hex GROSS> <Leerzeichen> <relpfad mit '/'> <LF>`,
    aufsteigend nach relpfad sortiert, UTF-8, darueber SHA-256.

    Verzeichnisse zaehlen nicht mit - ein leeres Verzeichnis traegt nichts,
    was ein Host laedt, und beide Seiten muessten sich sonst einigen, ob
    `Contents/` eine Zeile bekommt.
    """
    if not ordner.is_dir():
        raise OrdnerHashFehler(f"'{ordner}' ist kein Verzeichnis.")

    zeilen: list[tuple[bytes, str]] = []
    for eintrag in sorted(ordner.rglob("*"), key=lambda p: p.as_posix()):
        # Reparse-Punkte: `-Recurse` steigt in ein Verzeichnis-Reparse NICHT
        # hinab, rglob schon - dieselbe Definition ergaebe zwei Hashes.
        if eintrag.is_symlink() or eintrag.is_junction():
            raise OrdnerHashFehler(
                f"Reparse-Punkt in der Auslieferung ('{eintrag}'). Nicht vorgesehen - "
                "Python und PowerShell wuerden ihn verschieden behandeln."
            )
        if not eintrag.is_file():
            continue
        rel = eintrag.relative_to(ordner).as_posix()
        if not _ascii_pfad(rel):
            raise OrdnerHashFehler(
                f"Pfad ist nicht ASCII ('{rel}'). Nur ASCII sortiert in beiden "
                "Implementierungen gleich."
            )
        zeilen.append((rel.encode("utf-8"), datei_hash(eintrag)))

    zeilen.sort(key=lambda z: z[0])
    strom = b"".join(h.encode("ascii") + b" " + p + b"\n" for p, h in zeilen)
    return hashlib.sha256(strom).hexdigest().upper()


def artefakt_hash(pfad: pathlib.Path, art: str) -> str:
    """`vst3` ist ein Ordner, `broker` eine Datei."""
    return ordner_hash(pfad) if art == "vst3" else datei_hash(pfad)


def artefakt_liegt_vor(pfad: pathlib.Path, art: str) -> bool:
    return pfad.is_dir() if art == "vst3" else pfad.is_file()

ok = 0
fehler: list[str] = []


def pruefe(bedingung: bool, text: str, zusatz: str = "") -> bool:
    global ok
    zeile = text + (f"  [{zusatz}]" if zusatz else "")
    if bedingung:
        ok += 1
        print("  ok      " + zeile)
    else:
        fehler.append(zeile)
        print("  FEHLER  " + zeile)
    return bedingung


# ── Strukturregeln ──────────────────────────────────────────────────────────
#
# Jede Regel ist eine reine Funktion (manifest, identitaet) -> (bool, zusatz).
# Genau deshalb kann die Gegenprobe sie gegen ein verdorbenes Manifest erneut
# aufrufen, ohne den Prueftext zu duplizieren.


def _ziele(identitaet: dict) -> dict[str, dict]:
    return {z["id"]: z for z in identitaet["ziele"]}


def _aktive(identitaet: dict) -> list[dict]:
    """Ziele, die heute gebaut und ausgeliefert werden.

    S9b/SONDE-007c (28.08.2026): ein Ziel mit dem Feld `stillgelegt` bleibt in
    der Identitaetsdatei stehen - seine Kennung ist gesperrt, nicht frei -,
    gehoert aber nicht mehr in die Auslieferung. Die Trennung faellt auf die
    ANWESENHEIT des Feldes, nicht auf seinen Inhalt: fail-closed.

    Damit sagt diese Funktion, WELCHER Menge ein Ziel angehoert - und sonst
    nichts. Ueber die Lesbarkeit der Marke sagte bis zur Nacharbeit Runde 2
    (29.08.2026) auch keine andere Regel etwas; seither misst sie
    `r_stilllegungsmarke_lesbar`.
    """
    return [z for z in identitaet.get("ziele", []) if "stillgelegt" not in z]


def _stillgelegte(identitaet: dict) -> list[dict]:
    return [z for z in identitaet.get("ziele", []) if "stillgelegt" in z]


def _vst3(manifest: dict) -> list[dict]:
    return [a for a in manifest["artefakte"] if a.get("art") == "vst3"]


def _windows_kanon(pfad: object) -> str | None:
    """Kanonisiert einen absoluten Windows-Pfad rein lexikalisch.

    Der Validator laeuft auch ohne vorhandenes Installationsziel; `resolve()`
    waere deshalb die falsche Operation. `ntpath` verhindert zugleich, dass
    das Host-OS die Windows-Semantik der Manifestpfade umdeutet.
    """
    if not isinstance(pfad, str) or not re.match(r"^[A-Za-z]:[\\/]", pfad):
        return None
    return ntpath.normcase(ntpath.normpath(pfad.replace("/", "\\")))


def r_schema(m: dict, _i: dict):
    return m.get("schema") == SCHEMA, str(m.get("schema"))


def r_identitaetsquelle(m: dict, _i: dict):
    pfad = m.get("identitaetsquelle", "")
    return (WURZEL / pfad).resolve() == IDENTITAET, pfad


def r_jedes_ziel_genau_einmal(m: dict, i: dict):
    """Jedes AKTIVE Ziel genau einmal - und kein stillgelegtes.

    Bis zum 28.08.2026 stand hier die harte Zahl `len(ident_ziele) == 3`. Sie
    war nie die eigentliche Aussage: die traegt der Mengenvergleich darunter
    (ein viertes Ziel ohne Installer-Eintrag bringt dieses Bein zum Sprechen).
    Mit S9b/SONDE-007c wurde die Zahl ausserdem falsch - drei Kennungen, zwei
    Bundles. Ein Test misst den gebauten Stand, nicht eine gewuenschte Zahl.

    Kollisionsfreiheit gilt weiter ueber ALLE Kennungen, auch die
    stillgelegten: NkPr und "Nakama Suna.vst3" bleiben gesperrt, ein neues
    Ziel darf sie nicht wiederverwenden."""
    ident_ziele = i.get("ziele", [])
    aktive = _aktive(i)
    ids = [z.get("id") for z in ident_ziele]
    aktiv_ids = [z.get("id") for z in aktive]
    bundles = [z.get("bundle") for z in ident_ziele]
    aus_manifest = [a.get("ziel_id") for a in _vst3(m)]
    sichere_bundles = all(
        isinstance(b, str)
        and b not in ("", ".", "..")
        and ntpath.basename(b) == b
        and b.lower().endswith(".vst3")
        for b in bundles
    )
    identity_ok = (
        STATE_SCHEMA == 2
        and len(ident_ziele) >= 1
        and len(aktive) >= 1          # eine Auslieferung ohne Bundle waere keine
        and all(isinstance(zid, str) and zid for zid in ids)
        and len([zid.casefold() for zid in ids]) == len(set(zid.casefold() for zid in ids))
        and sichere_bundles
        and len([b.casefold() for b in bundles]) == len(set(b.casefold() for b in bundles))
        and all(type(z.get("state_schema")) is int and z["state_schema"] == STATE_SCHEMA
                for z in ident_ziele)
    )
    passt = (
        identity_ok
        and sorted(x for x in aus_manifest if x is not None) == sorted(aktiv_ids)
        and len(aus_manifest) == len(set(aus_manifest))
    )
    return passt, (f"{len(aus_manifest)} vs {len(aktiv_ids)} aktiv "
                   f"({len(ident_ziele)} Kennungen gesamt); "
                   f"identity={'ok' if identity_ok else 'ungueltig'}")


def _brauchbare_ids(werte: list, wo: str) -> tuple[list[str], list[str]]:
    """Trennt nichtleere String-IDs von allem anderen - VOR jeder Mengen- oder
    Sortieroperation.

    Nacharbeit Runde 1 (29.08.2026, T2-Befund P2): `sorted()` und `set()`
    setzen voraus, dass ihre Elemente vergleichbar bzw. hashbar sind. Eine ID
    vom Typ Liste oder Objekt - `"ziel_id": []` genuegt - liess die Regel
    darunter mit `TypeError: unhashable type: 'list'` sterben, statt das
    Manifest kontrolliert abzulehnen. Ein Absturz ist kein Regelbefund: er
    bricht das ganze Bein ab, statt zu sagen, WAS am Manifest falsch ist.
    Deshalb wird hier zuerst validiert und erst danach verglichen."""
    gut, fehler = [], []
    for index, wert in enumerate(werte):
        if isinstance(wert, str) and wert.strip():
            gut.append(wert)
        else:
            fehler.append(f"{wo}[{index}]: ziel_id ist keine nichtleere "
                          f"Zeichenkette ({type(wert).__name__}: {wert!r})")
    return gut, fehler


# Vertrag §2.3 (eq-copilot/schemas/installer/nakama-installer-v1.md, Zeile 87
# und 208) nennt diese vier Felder als Pflicht - aber KEINEN Typ. Gemessen
# wird deshalb der Typ, den das echte Manifest fuehrt: in
# eq-copilot/install/nakama-installer-v1.json ist jedes der vier eine
# nichtleere Zeichenkette, `kennung_bleibt` eingeschlossen (dort ein Satz
# ueber die gesperrte Kennung, kein Boolean). Ein hier erfundener Typ waere
# eine zweite Vertragswahrheit.
EINTRAG_PFLICHTFELDER = ("seit", "warum", "umgang_mit_altbestand", "kennung_bleibt")


def r_stillgelegte_benannt(m: dict, i: dict):
    """Die zweite Haelfte: ein Ziel darf nicht STILL aus der Auslieferung fallen.

    Ohne diese Regel genuegte es, einen Artefakteintrag zu loeschen - der
    Zaehlvergleich oben bliebe gruen, weil er dann gegen eine ebenso
    geschrumpfte Sollmenge misst. Erst die Forderung "jedes stillgelegte Ziel
    steht im Manifest namentlich, mit Datum, Grund und Umgang, und nur die
    stillgelegten" macht das Verschwinden sichtbar. Gefordert wird ausserdem,
    dass ein stillgelegtes Ziel NICHT als Artefakt auftaucht."""
    fehler = []
    # Beide Seiten werden validiert, nicht nur die Manifestseite: die
    # Identitaetsdatei ist eingefroren, aber diese Regel misst sie - und eine
    # Regel, die an ihrer eigenen Eingabe abstuerzt, misst nichts.
    stillgelegt_roh = [z.get("id") for z in _stillgelegte(i)]
    stillgelegt_gut, stillgelegt_fehler = _brauchbare_ids(stillgelegt_roh, "identitaet.stillgelegt")
    fehler += stillgelegt_fehler
    stillgelegt_ids = sorted(stillgelegt_gut)

    eintraege = m.get("stillgelegte_ziele")
    if not isinstance(eintraege, list):
        return (not stillgelegt_roh) and not fehler, \
            "; ".join(fehler + ["kein `stillgelegte_ziele`-Block"])
    benannt_roh = [e.get("ziel_id") for e in eintraege if isinstance(e, dict)]
    benannt, benannt_fehler = _brauchbare_ids(benannt_roh, "stillgelegte_ziele")
    fehler += benannt_fehler
    if len(benannt_roh) != len(eintraege):
        fehler.append("ein Eintrag ist kein Objekt")
    if sorted(benannt) != stillgelegt_ids:
        fehler.append(f"benannt {sorted(benannt)} != stillgelegt {stillgelegt_ids}")
    if len(benannt) != len(set(benannt)):
        fehler.append("ein Ziel ist doppelt benannt")
    for e in eintraege:
        if not isinstance(e, dict):
            continue
        # Nacharbeit Runde 2 (29.08.2026, T2-Befund P2): typstreng VOR der
        # Konvertierung. Hier stand `str(e.get(feld, "")).strip()` - und
        # `str(None)` ist "None", also vier nichtleere Zeichen. Gemessen am
        # Stand 05dbbb1 (Manifest SONDE-007c, Rohbeleg C2g): `null`, `[]`
        # und `{}` bestanden diese Zeile in JEDEM der vier Felder; nur "" und
        # "   " fielen. A17 nahm damit eine Stilllegung ohne brauchbares
        # Datum, ohne Grund und ohne Umgang an - gegen Vertrag §2.3.
        for feld in EINTRAG_PFLICHTFELDER:
            wert = e.get(feld)
            if not (isinstance(wert, str) and wert.strip()):
                fehler.append(f"{e.get('ziel_id')!r}: {feld} ist keine nichtleere "
                              f"Zeichenkette ({type(wert).__name__}: {wert!r})")
    # Auch diese Menge wird aus ungeprueften Manifestwerten gebaut: eine
    # `ziel_id` vom Typ Liste in `artefakte` wuerde sie sonst genauso
    # sprengen. Nicht-Strings koennen ohnehin kein stillgelegtes Ziel
    # bezeichnen; die Artefaktseite selbst prueft `r_jedes_ziel_genau_einmal`.
    aus_artefakten = {a.get("ziel_id") for a in _vst3(m)
                      if isinstance(a.get("ziel_id"), str)}
    for zid in stillgelegt_ids:
        if zid in aus_artefakten:
            fehler.append(f"{zid}: stillgelegt, steht aber in `artefakte`")
    return not fehler, "; ".join(fehler)


# Die Felder, die eine Stilllegungsmarke lesbar machen. Sie sind nicht hier
# erfunden, sondern an den anderen drei Lesern gemessen:
# eq-copilot/cmake/NakamaIdentitaet.cmake holt sich `stillgelegt am` und
# `stillgelegt entscheid` fuer seine Abbruchmeldung, und IdentityTestMain.cpp
# fordert fuer genau dieselben beiden `toString().isNotEmpty()` ("die
# Stilllegung nennt Datum und Entscheid"). A17 misst ab hier dasselbe Paar.
MARKEN_PFLICHTFELDER = ("am", "entscheid")


def r_stilllegungsmarke_lesbar(_m: dict, i: dict):
    """Nacharbeit Runde 2 (29.08.2026, T2-Befund P1): eine Marke, die DA ist,
    aber nichts sagt.

    `_aktive`/`_stillgelegte` fallen auf die ANWESENHEIT des Feldes. Das ist
    richtig und bleibt so - nur die Anwesenheit ist fail-closed. Danach sah
    aber keine der vierzehn Regeln mehr auf den WERT. Gemessen am Stand
    05dbbb1 (Rohbeleg C2f im Manifest SONDE-007c): `stillgelegt` als null,
    String, Array, Zahl, Boolean, leeres Objekt oder `{"am": ""}` liess ALLE
    vierzehn Regeln gruen, A17 endete mit Exit 0 - waehrend der CMake-Leser,
    EqCopIdentityTest und Install-Nakama.ps1 denselben Eingang hart ablehnen.
    Ein Bein, das als einziges der vier Leser schweigt, macht die
    Vier-Leser-Symmetrie zu einer Behauptung statt zu einer Messung.

    Die Marke ist deshalb ein eigener Riegel: Objekt mit `am` und `entscheid`
    als nichtleere Zeichenketten, sonst Regelbefund. Was diese Regel NICHT
    tut: ein Ziel wieder aktiv machen. Eine kaputte Marke bedeutet weiter
    "stillgelegt", nur unbrauchbar beschrieben."""
    fehler = []
    for z in _stillgelegte(i):
        zid = z.get("id")
        marke = z.get("stillgelegt")
        if not isinstance(marke, dict):
            fehler.append(f"{zid!r}: Stilllegungsmarke ist kein Objekt "
                          f"({type(marke).__name__}: {marke!r})")
            continue
        for feld in MARKEN_PFLICHTFELDER:
            wert = marke.get(feld)
            if not (isinstance(wert, str) and wert.strip()):
                fehler.append(f"{zid!r}: `stillgelegt.{feld}` ist keine nichtleere "
                              f"Zeichenkette ({type(wert).__name__}: {wert!r})")
    return not fehler, "; ".join(fehler)


def r_art_bekannt(m: dict, _i: dict):
    """S9-Nacharbeit (T2-Befund T2-4): `art` ist eine GESCHLOSSENE Menge.

    Vorher sah das keine der zwoelf Regeln: `_vst3()` filtert auf
    `art == "vst3"`, die Broker-Regel zaehlt nur `broker` - ein drittes Wort
    faellt durch beide Siebe. Im Skript landet es dann im Broker-ZWEIG von
    `Ziel-Pfad()`, also unter `ziele.broker_verzeichnis`: genau dem Pfad, den
    Vertrag §4 schuetzt, weil er ab SONDE-010 ein Spawn-Ziel wird.

    Der realistische Fall ist ein Tippfehler beim Erweitern, kein Angriff -
    und genau dagegen ist eine geschlossene Menge da."""
    fremd = [str(a.get("art")) for a in m["artefakte"] if a.get("art") not in ARTEN]
    return not fremd, ", ".join(fremd)


def r_quellpfade_nachgerechnet(m: dict, i: dict):
    """Der Kern dieses Beins: der Pfad wird abgeleitet, nicht geglaubt.

    Seit der S9-Nacharbeit zeigt er auf den Bundle-ORDNER, nicht mehr auf die
    innere Binaerdatei (Vertrag §2.1, T2-Befund T2-5)."""
    ziele = _ziele(i)
    abweichungen = []
    for a in _vst3(m):
        ziel = ziele.get(a.get("ziel_id"))
        if ziel is None:
            abweichungen.append(f"{a.get('ziel_id')}: kein Identitaetseintrag")
            continue
        cmake_ziel = a.get("cmake_ziel")
        if not isinstance(cmake_ziel, str) or CMAKE_ZIEL.fullmatch(cmake_ziel) is None:
            abweichungen.append(f"{a.get('ziel_id')}: unsicheres cmake_ziel {cmake_ziel!r}")
            continue
        erwartet = (
            f"eq-copilot/build/plugin/{cmake_ziel}_artefacts/Release/VST3/"
            f"{ziel['bundle']}"
        )
        if a.get("quelle") != erwartet:
            abweichungen.append(f"{a.get('ziel_id')}: {a.get('quelle')!r} != {erwartet!r}")
            continue
        quellpfad = (WURZEL / erwartet).resolve()
        bauwurzel = (WURZEL / "eq-copilot" / "build" / "plugin").resolve()
        if not quellpfad.is_relative_to(bauwurzel):
            abweichungen.append(f"{a.get('ziel_id')}: Quelle verlaesst {bauwurzel}")
    return not abweichungen, "; ".join(abweichungen)


def r_keine_identitaetsliterale(m: dict, i: dict):
    """Viercodes und Class-IDs duerfen im Installer-Manifest NIRGENDS stehen.

    Produkt- und Bundlenamen sind ausgenommen - sie stecken zwangslaeufig im
    Dateipfad. Genau deshalb rechnet r_quellpfade_nachgerechnet sie nach:
    zusammen ist die Aussage dicht, einzeln waere jede Haelfte loechrig."""
    text = json.dumps(m, ensure_ascii=False)
    ohne_quellen = re.sub(r'"quelle"\s*:\s*"[^"]*"', '"quelle":""', text)
    treffer = []
    # S9b/SONDE-007c (28.08.2026): `produktname` und `bundle` stehen jetzt mit
    # in der Liste. Der Kopf dieser Datei sagt seit dem 23.08. "WEDER
    # Produktnamen NOCH Viercodes NOCH Class-IDs" - gemessen wurde davon bis
    # heute nur die zweite und dritte Haelfte. Aufgefallen ist es beim
    # Schreiben des Stilllegungs-Blocks: dessen Fliesstext haette den
    # Bundlenamen beilaeufig ein zweites Mal festgeschrieben, und keine Regel
    # haette das gesehen. Die Ausnahme fuer `quelle` bleibt - dort steckt der
    # Bundlename zwangslaeufig im Pfad, und genau deshalb rechnet
    # r_quellpfade_nachgerechnet ihn nach.
    for ziel in i["ziele"]:
        for feld in ("plugin_code", "component_cid", "controller_cid",
                     "produktname", "bundle"):
            wert = ziel.get(feld)
            if wert and wert in ohne_quellen:
                treffer.append(f"{ziel['id']}.{feld}")
    # `hersteller.name` steht bewusst NICHT in dieser Liste: er ist Bestandteil
    # der von Vertrag §4 festgelegten geschuetzten Pfade
    # (C:/Program Files/evenacadia/...), also dieselbe Zwangslage wie der
    # Bundlename in `quelle` - und im Gegensatz zum Herstellercode geht er in
    # keine Class-ID ein. Der CODE bleibt verboten.
    code = i["hersteller"]["code"]
    if code in ohne_quellen:
        treffer.append("hersteller.code")
    return not treffer, ", ".join(treffer)


def r_broker(m: dict, _i: dict):
    eintraege = [a for a in m["artefakte"] if a.get("art") == "broker"]
    if len(eintraege) != 1:
        return False, f"{len(eintraege)} Broker-Eintraege"
    quelle = eintraege[0].get("quelle", "")
    erwartet = "broker/target/release/eqcop-broker.exe"
    return quelle == erwartet, quelle


def r_broker_heisst_wie_die_crate(m: dict, _i: dict):
    """Der Binaername kommt aus broker/Cargo.toml, nicht aus dem Gedaechtnis."""
    text = BROKER_CARGO.read_text(encoding="utf-8")
    treffer = re.search(r'^\s*name\s*=\s*"([^"]+)"', text, re.MULTILINE)
    crate = treffer.group(1) if treffer else ""
    eintraege = [a for a in m["artefakte"] if a.get("art") == "broker"]
    # `bool(eintraege)` ist kein Zierrat: ohne diese Zeile waere `all()` ueber
    # einer leeren Liste WAHR - die Regel bliebe gruen, gerade weil es nichts
    # zu pruefen gibt. Genau daran ist die erste Fassung in der Gegenprobe
    # aufgefallen (23.08.), nicht im Nachdenken.
    passt = bool(crate) and bool(eintraege) and all(
        a.get("quelle", "").endswith(f"/{crate}.exe") and a.get("name") == f"{crate}.exe"
        for a in eintraege
    )
    return passt, crate


def r_zielverzeichnisse(m: dict, _i: dict):
    """Der Broker liegt geschuetzt. §53.9 nennt Installationspfad und
    Betriebssystemschutz massgeblich; ein per-User-Pfad waere ein Spawn-Ziel,
    das jeder Nutzerprozess ohne Rechteerhoehung austauschen kann."""
    ziele = m.get("ziele", {})
    vst3 = ziele.get("vst3_verzeichnis", "")
    broker = ziele.get("broker_verzeichnis", "")
    passt = (
        _windows_kanon(vst3) == _windows_kanon("C:/Program Files/Common Files/VST3")
        and _windows_kanon(broker) == _windows_kanon("C:/Program Files/evenacadia/Nakama")
    )
    return passt, f"{vst3} | {broker}"


def r_signatur_ehrlich(m: dict, _i: dict):
    """Keine behauptete Pruefung ohne Mittel: ist kein Thumbprint da, MUSS
    dastehen, warum."""
    s = m.get("signatur", {})
    if s.get("verfahren") != "sha256-manifest":
        return False, str(s.get("verfahren"))
    if s.get("authenticode_thumbprint") is None:
        return bool(s.get("warum_null", "").strip()), "kein Zertifikat, Grund steht da"
    return THUMBPRINT.match(str(s["authenticode_thumbprint"]).upper()) is not None, "Thumbprint"


def r_hashfelder(m: dict, _i: dict):
    schlecht = []
    for a in m["artefakte"]:
        h = a.get("sha256")
        if h is not None and not HEX64.match(str(h)):
            schlecht.append(a.get("ziel_id") or a.get("name"))
    return not schlecht, ", ".join(schlecht)


def r_bekannte_staende(m: dict, i: dict):
    """Seit dem Umzug auf den Ordner (Vertrag §5.1) traegt jeder Eintrag
    ausserdem `hash_art`: `ordner` (Ordner-Hash v1) oder `datei-innen`
    (Hash NUR der inneren Binaerdatei). Ohne dieses Feld waere nicht
    entscheidbar, wogegen der Rueckweg vergleicht - und die zwei
    historischen Eintraege wurden gegen die Binaerdatei eingefroren, lange
    bevor der Ordner die Einheit war."""
    ziele = _ziele(i)
    schlecht = []
    for e in m["rueckweg"]["bekannte_staende"]:
        if not HEX64.match(str(e.get("sha256", ""))):
            schlecht.append(f"{e.get('sha256')!r}: kein SHA-256")
        if e.get("ziel_id") not in ziele:
            schlecht.append(f"{e.get('ziel_id')!r}: unbekanntes Ziel")
        if not isinstance(e.get("state_schema"), int):
            schlecht.append(f"{e.get('sha256')!r}: state_schema fehlt")
        if e.get("hash_art") not in ("ordner", "datei-innen"):
            schlecht.append(f"{e.get('sha256')!r}: hash_art {e.get('hash_art')!r}")
    return not schlecht, "; ".join(schlecht)


def r_rueckweg_vollstaendig(m: dict, _i: dict):
    r = m.get("rueckweg", {})
    noetig = ("strategie", "verzeichnis", "ergebnisdatei", "transaktionsanker",
              "nak_41", "bekannte_staende")
    fehlend = [k for k in noetig if not r.get(k)]
    if _windows_kanon(r.get("transaktionsanker")) != _windows_kanon(
            "C:/Program Files/evenacadia/.nakama-installer/aktive-transaktion.json"):
        fehlend.append("transaktionsanker: kanonische Program-Files-Autoritaet")
    if _windows_kanon(r.get("verzeichnis")) != _windows_kanon(
            "C:/Program Files/evenacadia/.nakama-installer/backups"):
        fehlend.append("verzeichnis: geschuetzte per-Transaktion-Backups")
    nak = r.get("nak_41", {})
    benannt = nak.get("punkt") == "NAK-41" and bool(nak.get("riegel", "").strip())
    if not benannt:
        fehlend.append("nak_41.punkt/riegel")
    return not fehlend, ", ".join(fehlend)


REGELN = [
    (r_schema, "Manifest traegt das Vertragsschema nakama.installer/v1"),
    (r_identitaetsquelle, "es zeigt auf die eingefrorene Identitaetsdatei"),
    (r_art_bekannt, "jede `art` ist vst3 oder broker - eine geschlossene Menge"),
    (r_jedes_ziel_genau_einmal, "Identitaet ist kollisionsfrei, schema=2 und jedes AKTIVE Ziel hat genau einen VST3-Eintrag"),
    (r_stillgelegte_benannt, "jedes stillgelegte Ziel ist benannt (Datum, Grund, Umgang) und steht in keinem Artefakt"),
    (r_stilllegungsmarke_lesbar, "jede Stilllegungsmarke ist lesbar - Objekt mit `am` und `entscheid`"),
    (r_quellpfade_nachgerechnet, "jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet"),
    (r_keine_identitaetsliterale, "kein Viercode, keine Class-ID, kein Produkt- oder Bundlename im Installer-Manifest (ausser im Pfad)"),
    (r_broker, "genau ein Broker-Artefakt, aus dem Release-Pfad der Crate"),
    (r_broker_heisst_wie_die_crate, "der Broker-Binaername kommt aus broker/Cargo.toml"),
    (r_zielverzeichnisse, "VST3 nach Common Files, Broker geschuetzt unter Program Files"),
    (r_signatur_ehrlich, "die Signaturzeile behauptet keine Pruefung ohne Mittel"),
    (r_hashfelder, "jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben"),
    (r_bekannte_staende, "jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema"),
    (r_rueckweg_vollstaendig, "der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)"),
]


# ── Gegenprobe: jede Regel muss an einem verdorbenen Manifest FALLEN ────────

def verdirb(m: dict, i: dict) -> dict:
    """Ein Manifest, das gegen JEDE Regel gleichzeitig verstoesst.

    Bewusst grob: die Gegenprobe beweist nicht, WIE fein eine Regel ist,
    sondern DASS sie ueberhaupt etwas sieht. Eine Regel, die auch hier gruen
    bleibt, prueft nichts."""
    k = copy.deepcopy(m)
    k["schema"] = "nakama.installer/v0"
    k["identitaetsquelle"] = "eq-copilot/identity/erfunden.json"
    for a in k["artefakte"]:
        if a.get("art") == "broker":
            # NICHT entfernen, sondern verderben: eine leere Broker-Liste
            # liesse `all()` vacuously wahr werden, und die Gegenprobe pruefte
            # eine Regel, die gar nichts zu sehen bekommt.
            a["quelle"] = "broker/target/release/fremd.exe"
            a["name"] = "fremd.exe"
        if a.get("art") == "vst3":
            # Der teuerste Fehler, den dieses Bein verhindern soll: ein Pfad,
            # der auf das Bundle eines ANDEREN Ziels zeigt.
            a["quelle"] = (
                "eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/"
                "EQ-Copilot.vst3/Contents/x86_64-win/EQ-Copilot.vst3"
            )
            a["sha256"] = "kein hash"
    vst3 = [a for a in k["artefakte"] if a.get("art") == "vst3"]
    if vst3:
        k["artefakte"].remove(vst3[-1])                  # ein Ziel fehlt jetzt
        vst3[0]["plugin_code_kopie"] = "NkPr"            # zweite Identitaet
        # ... und ein Produktname, seit die Regel auch den verbietet (S9b).
        # Er kommt aus der Identitaetsdatei, nicht aus einem Literal: ein hier
        # abgeschriebener Name waere selbst die zweite Wahrheit, gegen die
        # die Regel gerichtet ist.
        vst3[0]["produktname_kopie"] = i["ziele"][0]["produktname"]
    # S9-Nacharbeit (T2-4): eine dritte Artefaktsorte. Genau dieser Eintrag
    # rutschte vorher durch ALLE zwoelf Regeln - `_vst3()` filtert ihn weg,
    # `r_broker` zaehlt ihn nicht - und landete im Skript im Broker-Zweig.
    k["artefakte"].append({"art": "standalone", "name": "Nakama.exe",
                           "quelle": "irgendwo/Nakama.exe", "sha256": "kein hash"})
    # S9b/SONDE-007c: ein erfundener Name statt einer leeren Liste. Eine leere
    # Liste faellt nur, SOLANGE es ein stillgelegtes Ziel gibt - dieser Eintrag
    # faellt immer, weil er weder zur Sollmenge passt noch seine Pflichtfelder
    # traegt. Eine Gegenprobe, die von der Datenlage abhaengt, ist keine.
    k["stillgelegte_ziele"] = [{"ziel_id": "erfunden-fuer-die-gegenprobe"}]
    k["ziele"] = {"vst3_verzeichnis": "C:/Temp", "broker_verzeichnis": "%LOCALAPPDATA%/nakama"}
    k["signatur"] = {"verfahren": "sha256-manifest", "authenticode_thumbprint": None, "warum_null": ""}
    k["rueckweg"] = {"strategie": "", "bekannte_staende": [
        {"sha256": "xx", "ziel_id": "?", "state_schema": "eins", "hash_art": "erfunden"}]}
    return k


def verdirb_identitaet(i: dict) -> dict:
    """Die Identitaetsseite derselben Gegenprobe (Nacharbeit Runde 2).

    `verdirb` verdirbt nur das Manifest. `r_stilllegungsmarke_lesbar` liest
    als einzige Regel allein die Identitaet - an einem nur manifestseitig
    verdorbenen Stand bliebe sie gruen, und der Block [2] behauptete dann
    eine Gegenprobe, die es fuer sie nie gab.

    Verdorben wird genau der WERT der Marke, nicht ihre ANWESENHEIT: die
    Mengen aus `_aktive`/`_stillgelegte` bleiben danach Zeichen fuer Zeichen
    dieselben. Eine Gegenprobe, die nebenbei die Mengen verschoebe, bewiese
    fuer die uebrigen dreizehn Regeln etwas anderes, als ihre Zeile sagt."""
    k = copy.deepcopy(i)
    for z in k.get("ziele", []):
        if "stillgelegt" in z:
            z["stillgelegt"] = None
    return k


def _gegenprobe_braucht(wert, warum: str):
    """Gibt `wert` zurueck - oder beendet die Gegenprobe KONTROLLIERT.

    `next(...)` ohne Vorgabe stirbt mit `StopIteration`, ein Zugriff auf eine
    fehlende Kennung mit `KeyError`; beides waere ein Traceback. Eine einzelne
    Byteaenderung reicht dafuer aus - gemessen vom Byte-Kipp-Fuzz [3c]
    (NAK-94 Nacharbeit 10). Traegt die Datenlage eine Gegenprobe nicht mehr,
    endet sie deshalb wie die beiden aelteren Faelle: mit Klartext und Exit
    ungleich 0. Eine stillschweigend ausgelassene Gegenprobe waere schlimmer
    als keine.
    """
    if wert is None:
        raise SystemExit("Gegenprobe unmoeglich: " + warum)
    return wert


def adversariale_strukturproben(m: dict, i: dict) -> None:
    """Die vier ehemals offenen Kanten einzeln brechen.

    Die grobe `verdirb`-Probe beweist, dass eine Regel ueberhaupt sehen kann.
    Diese Proben beweisen dagegen die konkreten Umgehungen aus Paket 03:
    Zielkollision, Identity/Writer-Schemadrift und lexikalisches Traversal.
    """
    print("\n[3] Adversariale Pfad- und Identitaetsgegenproben")

    # S9b/SONDE-007c: die Indizes werden GESUCHT, nicht angenommen. Vorher
    # stand hier `ziele[1]` - der Eintrag der passiven Sonde. Seit ihrer
    # Stilllegung hat er kein Artefakt mehr, und `next(...)` waere mit
    # StopIteration gestorben: eine Gegenprobe, die an der Datenlage haengt,
    # verschwindet genau dann, wenn sich die Datenlage aendert.
    aktiv_idx = [n for n, z in enumerate(i["ziele"]) if "stillgelegt" not in z]
    if len(aktiv_idx) < 2:
        raise SystemExit(
            "Gegenprobe unmoeglich: die Bundle-Zielkollision braucht ZWEI aktive "
            "Ziele. Weniger als zwei bedeutet, dass diese Probe nichts mehr misst - "
            "und eine stillschweigend ausgelassene Gegenprobe ist schlimmer als keine."
        )
    a0, a1 = aktiv_idx[0], aktiv_idx[1]

    ident = copy.deepcopy(i)
    manifest = copy.deepcopy(m)
    ident["ziele"][a1]["bundle"] = ident["ziele"][a0]["bundle"]
    zid = ident["ziele"][a1]["id"]
    artefakt = _gegenprobe_braucht(
        next((a for a in manifest["artefakte"]
              if a.get("ziel_id") == zid), None),
        f"zum aktiven Ziel {zid!r} fuehrt das Manifest kein Artefakt - genau "
        "dieses braucht die Bundle-Zielkollision.")
    artefakt["quelle"] = (
        f"eq-copilot/build/plugin/{artefakt['cmake_ziel']}_artefacts/Release/VST3/"
        f"{ident['ziele'][a1]['bundle']}"
    )
    pruefe(not r_jedes_ziel_genau_einmal(manifest, ident)[0],
           "faellt an einer Bundle-Zielkollision")

    for wert, name in ((1, "1"), ("kaputt", "Text"), (None, "fehlend")):
        ident = copy.deepcopy(i)
        if wert is None:
            ident["ziele"][a0].pop("state_schema", None)
        else:
            ident["ziele"][a0]["state_schema"] = wert
        pruefe(not r_jedes_ziel_genau_einmal(m, ident)[0],
               f"faellt an Identity-state_schema {name}")

    # S9b/SONDE-007c: die beiden konkreten Umgehungen der Stilllegung.
    # (a) Ein stillgelegtes Ziel wird wieder ausgeliefert.
    # (b) Ein aktives Ziel faellt still aus der Auslieferung, indem nur sein
    #     Artefakteintrag verschwindet.
    stillgelegte = _stillgelegte(i)
    if stillgelegte:
        manifest = copy.deepcopy(m)
        vorlage = _gegenprobe_braucht(
            next((a for a in manifest["artefakte"]
                  if a.get("art") == "vst3"), None),
            "das Manifest fuehrt kein VST3-Artefakt, das als Vorlage fuer ein "
            "heimlich wieder ausgeliefertes stillgelegtes Ziel dienen koennte.")
        heimlich = copy.deepcopy(vorlage)
        heimlich["ziel_id"] = stillgelegte[0]["id"]
        manifest["artefakte"].append(heimlich)
        pruefe(not r_stillgelegte_benannt(manifest, i)[0],
               "faellt, wenn ein stillgelegtes Ziel doch ausgeliefert wird")

        manifest = copy.deepcopy(m)
        manifest["stillgelegte_ziele"] = []
        pruefe(not r_stillgelegte_benannt(manifest, i)[0],
               "faellt, wenn ein stillgelegtes Ziel nirgends benannt ist")

        # -- Nacharbeit Runde 2 (29.08.2026, T2-Befund P1) ------------------
        # Die Marke selbst. Gemessen am Stand 05dbbb1 (Rohbeleg C2f) lief
        # JEDE dieser Varianten durch alle vierzehn Regeln - A17 sagte Exit 0,
        # waehrend CMake-Leser, EqCopIdentityTest und Installer denselben
        # Eingang hart ablehnen. Die Identitaetsdatei ist eingefroren (NAK-30);
        # mutiert wird ausschliesslich eine TIEFE KOPIE im Speicher.
        for marke, name in (
            (None,                  "null"),
            ("stillgelegt",         "eine Zeichenkette"),
            ([],                    "ein leeres Array"),
            (7,                     "eine Zahl"),
            (True,                  "ein Boolean"),
            ({},                    "ein leeres Objekt"),
            ({"am": ""},            "ein Objekt mit leerem `am`"),
            ({"am": "2026-08-28"},  "ein Objekt ohne `entscheid`"),
        ):
            ident = copy.deepcopy(i)
            for z in ident["ziele"]:
                if z["id"] == stillgelegte[0]["id"]:
                    z["stillgelegt"] = marke
            pruefe(not r_stilllegungsmarke_lesbar(m, ident)[0],
                   f"faellt, wenn die Stilllegungsmarke {name} ist")
            # Und die zweite Haelfte derselben Zusage: eine kaputte Marke ist
            # nie ein Ruecksprung nach "aktiv". Ohne diese Zeile bewiese die
            # obige nur, dass jemand meckert - nicht, dass der Riegel haelt.
            pruefe(len(_aktive(ident)) == len(_aktive(i))
                   and len(_stillgelegte(ident)) == len(_stillgelegte(i)),
                   f"und das Ziel bleibt trotzdem stillgelegt, wenn die Marke {name} ist")

        # Nacharbeit Runde 1 (29.08.2026, T2-Befund P2): eine ID, die keine
        # nichtleere Zeichenkette ist. Bis heute starb die Regel hier an
        # `TypeError: unhashable type: 'list'` bzw. an `sorted()` ueber
        # gemischte Typen - ein Absturz, der das ganze Bein abbricht, statt
        # das Manifest kontrolliert abzulehnen. Jede dieser Proben muss ROT
        # werden und dabei am Leben bleiben; genau das misst `pruefe`, denn
        # eine geworfene Ausnahme kaeme hier gar nicht mehr an.
        #
        # Und wie bei der Bundle-Zielkollision weiter unten gilt: eine
        # Gegenprobe, die an der Datenlage haengt, ist keine. Fehlt die
        # Grundlage, sagt das Bein das laut, statt die Proben zu ueberspringen.
        if not (m.get("stillgelegte_ziele") or []):
            pruefe(False,
                   "Gegenprobe unmoeglich: die Identitaetsdatei kennt ein stillgelegtes "
                   "Ziel, `stillgelegte_ziele` im Manifest ist aber leer")
        else:
            for name, mutation in (
                ("ziel_id ist eine leere Liste",   lambda e: e.update(ziel_id=[])),
                ("ziel_id ist ein Objekt",         lambda e: e.update(ziel_id={"a": 1})),
                ("ziel_id ist eine leere Zeichenkette", lambda e: e.update(ziel_id="")),
                ("ziel_id ist nur Leerraum",       lambda e: e.update(ziel_id="   ")),
                ("ziel_id ist eine Zahl",          lambda e: e.update(ziel_id=7)),
                ("ziel_id fehlt ganz",             lambda e: e.pop("ziel_id", None)),
            ):
                manifest = copy.deepcopy(m)
                mutation(manifest["stillgelegte_ziele"][0])
                pruefe(not r_stillgelegte_benannt(manifest, i)[0],
                       f"faellt kontrolliert (ohne Absturz), wenn {name}")

            # Gemischte Typen in DERSELBEN Liste - der Fall, an dem schon
            # `sorted()` stirbt, nicht erst `set()`.
            manifest = copy.deepcopy(m)
            zweiter = copy.deepcopy(manifest["stillgelegte_ziele"][0])
            zweiter["ziel_id"] = []
            manifest["stillgelegte_ziele"].append(zweiter)
            pruefe(not r_stillgelegte_benannt(manifest, i)[0],
                   "faellt kontrolliert bei gemischten ziel_id-Typen in einer Liste")

            # -- Nacharbeit Runde 2 (29.08.2026, T2-Befund P2) --------------
            # Die Pflichtfelder je Manifesteintrag. Bis 05dbbb1 stand hier
            # `str(e.get(feld, "")).strip()`: aus JSON-`null` wurde "None", aus
            # `[]` wurde "[]". Gemessen (Rohbeleg C2g): drei der fuenf Varianten
            # je Feld blieben gruen - nur "" und "   " fielen.
            for feld in EINTRAG_PFLICHTFELDER:
                for wert, name in ((None, "null"), ([], "ein leeres Array"),
                                   ({}, "ein leeres Objekt"), ("", "leer"),
                                   ("   ", "nur Leerraum"), (7, "eine Zahl")):
                    manifest = copy.deepcopy(m)
                    manifest["stillgelegte_ziele"][0][feld] = wert
                    pruefe(not r_stillgelegte_benannt(manifest, i)[0],
                           f"faellt, wenn `{feld}` {name} ist")

    manifest = copy.deepcopy(m)
    entfernt = _gegenprobe_braucht(
        next((a for a in manifest["artefakte"]
              if a.get("art") == "vst3"), None),
        "das Manifest fuehrt kein VST3-Artefakt mehr, das still aus der "
        "Auslieferung fallen koennte.")
    manifest["artefakte"].remove(entfernt)
    pruefe(not r_jedes_ziel_genau_einmal(manifest, i)[0],
           "faellt, wenn ein aktives Ziel still aus der Auslieferung faellt")

    # S9b/SONDE-007c: die neue Haelfte der Literalregel EINZELN gebrochen.
    # Die grobe `verdirb`-Probe traegt schon einen Viercode - sie wuerde also
    # auch dann fallen, wenn der Produktname gar nicht geprueft wuerde.
    manifest = copy.deepcopy(m)
    manifest["nur_ein_produktname"] = _aktive(i)[0]["produktname"]
    pruefe(not r_keine_identitaetsliterale(manifest, i)[0],
           "faellt an einem Produktnamen ausserhalb der Pfade")
    manifest = copy.deepcopy(m)
    manifest["nur_ein_bundlename"] = _aktive(i)[0]["bundle"]
    pruefe(not r_keine_identitaetsliterale(manifest, i)[0],
           "faellt an einem Bundlenamen ausserhalb der Pfade")

    manifest = copy.deepcopy(m)
    manifest["ziele"]["broker_verzeichnis"] = "C:/Program Files/../Temp/Nakama"
    pruefe(not r_zielverzeichnisse(manifest, i)[0],
           "faellt an kanonischem Broker-Zieltraversal")

    manifest = copy.deepcopy(m)
    artefakt = _gegenprobe_braucht(
        next((a for a in manifest["artefakte"]
              if a.get("art") == "vst3"), None),
        "das Manifest fuehrt kein VST3-Artefakt, an dem sich ein "
        "cmake_ziel-Quelltraversal zeigen liesse.")
    artefakt["cmake_ziel"] = "../../../../outside"
    ziel = _gegenprobe_braucht(
        _ziele(i).get(artefakt["ziel_id"]),
        f"die Identitaet kennt kein Ziel {artefakt['ziel_id']!r}, auf das das "
        "VST3-Artefakt zeigt.")
    bundle = _gegenprobe_braucht(
        ziel.get("bundle"),
        f"das Ziel {artefakt['ziel_id']!r} traegt keinen Bundlenamen, aus dem "
        "sich ein Quellpfad rechnen liesse.")
    artefakt["quelle"] = (
        f"eq-copilot/build/plugin/{artefakt['cmake_ziel']}_artefacts/Release/VST3/{bundle}"
    )
    pruefe(not r_quellpfade_nachgerechnet(manifest, i)[0],
           "faellt an cmake_ziel-Quelltraversal")

    manifest = copy.deepcopy(m)
    manifest["rueckweg"]["verzeichnis"] = "eq-copilot/install/rueckweg"
    pruefe(not r_rueckweg_vollstaendig(manifest, i)[0],
           "faellt an benutzerbeschreibbaren Rueckweg-Backups")


def datei_hash(pfad: pathlib.Path) -> str:
    h = hashlib.sha256()
    with pfad.open("rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest().upper()


def hashen(manifest: dict, ziel: pathlib.Path | None = None) -> int:
    """Der mutierende Release-Zweig `--hashen`.

    `ziel` ist das Schreibziel; ohne Angabe das Manifest im Repo.
    """
    ziel = ziel or MANIFEST
    print("[hashen] Artefakte gegen den gebauten Stand festschreiben")
    alle_da = True
    for a in manifest["artefakte"]:
        pfad = (WURZEL / a["quelle"]).resolve()
        art = a.get("art")
        name = a.get("ziel_id") or a.get("name")
        if not pfad.is_relative_to(WURZEL):
            print(f"  FEHLER  {name}: Quelle verlaesst die Repo-Wurzel ({pfad}).")
            alle_da = False
            continue
        if art not in ARTEN:
            print(f"  FEHLER  {name}: unbekannte art {art!r} - vst3 oder broker, nichts sonst.")
            alle_da = False
            continue
        if not artefakt_liegt_vor(pfad, art):
            was = "Ordner" if art == "vst3" else "Datei"
            print(f"  FEHLT   {name}: {a['quelle']}  ({was})")
            alle_da = False
            continue
        # Ein Bundle-Ordner OHNE Datei haette einen wohldefinierten Hash (den
        # des leeren Bytestroms) - und waere trotzdem keine Auslieferung.
        # Definiert heisst nicht ausliefer-bar.
        if art == "vst3" and not any(p.is_file() for p in pfad.rglob("*")):
            print(f"  FEHLER  {name}: Bundle-Ordner ist leer ({a['quelle']}).")
            alle_da = False
            continue
        try:
            a["sha256"] = artefakt_hash(pfad, art)
        except OrdnerHashFehler as e:
            print(f"  FEHLER  {name}: {e}")
            alle_da = False
            continue
        print(f"  ok      {name} = {a['sha256']}")
    if not alle_da:
        print(
            "\nABGEBROCHEN - nicht alle Artefakte liegen ausliefer-bar vor. Ein Manifest\n"
            "mit halben Hashes waere eine Auslieferung, die nur zur Haelfte eingefroren ist."
        )
        return 3
    manifest["hashes_erzeugt_am"] = datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ")
    ziel.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    try:
        wo = ziel.relative_to(WURZEL).as_posix()
    except ValueError:
        wo = str(ziel)
    print(f"\ngeschrieben: {wo}")
    return 0


# ── Kreuzprobe: Python gegen PowerShell (Vertrag §2.1) ─────────────────────


def _synthetischer_ordner(wurzel: pathlib.Path) -> None:
    """Ein Ordner, der genau die Stellen trifft, an denen zwei
    Implementierungen auseinanderlaufen KOENNTEN:

      * `B.txt` vs `a.txt` - ordinal steht 'B' (0x42) VOR 'a' (0x61),
        kulturabhaengig sortiert Windows sie andersherum. Sortierte eine
        Seite nach Locale, faellt genau hier auf.
      * ein Name MIT Leerzeichen - das Trennzeichen der Hashzeile,
      * zwei Ebenen Verschachtelung wie im echten Bundle,
      * ein LEERES Verzeichnis - es darf keine Zeile erzeugen.
    """
    (wurzel / "Contents" / "Resources").mkdir(parents=True)
    (wurzel / "Contents" / "x86_64-win").mkdir(parents=True)
    (wurzel / "leer").mkdir(parents=True)
    (wurzel / "a.txt").write_bytes(b"a")
    (wurzel / "B.txt").write_bytes(b"B")
    (wurzel / "mit leerzeichen.txt").write_bytes(b"")
    (wurzel / "Contents" / "Resources" / "moduleinfo.json").write_bytes(b'{"x":1}\n')
    (wurzel / "Contents" / "x86_64-win" / "inner.bin").write_bytes(bytes(range(256)))


def kreuzprobe() -> None:
    """Ein Hash, den zwei Sprachen bilden, ist nur so viel wert wie ihre
    Uebereinstimmung. Gemessen wird an einem SYNTHETISCHEN Ordner, damit die
    Probe auch ohne gebautes Bundle laeuft."""
    print("\n[5] Ordner-Hash v1 - Python gegen PowerShell")

    if not PS_ORDNERHASH.is_file():
        pruefe(False, "die PowerShell-Haelfte liegt vor", str(PS_ORDNERHASH))
        return
    pwsh = shutil.which("pwsh") or shutil.which("powershell")
    if pwsh is None:
        # Schweigen waere hier das Schlimmste: die Kreuzprobe IST der Beweis,
        # dass beide Seiten dasselbe rechnen. Lief sie nicht, ist nichts bewiesen.
        pruefe(False, "pwsh gefunden - ohne die zweite Sprache beweist diese Kreuzprobe nichts")
        return

    with tempfile.TemporaryDirectory(prefix="nakama-ordnerhash-") as tmp:
        ordner = pathlib.Path(tmp) / "Probe.vst3"
        ordner.mkdir()
        _synthetischer_ordner(ordner)

        aus_python = ordner_hash(ordner)
        lauf = subprocess.run(
            [pwsh, "-NoProfile", "-File", str(PS_ORDNERHASH), str(ordner)],
            capture_output=True, text=True,
        )
        aus_ps = lauf.stdout.strip().splitlines()[-1].strip() if lauf.stdout.strip() else ""

        if not pruefe(lauf.returncode == 0, "die PowerShell-Haelfte laeuft durch",
                      (lauf.stderr.strip() or "")[:200]):
            return
        pruefe(HEX64.match(aus_python) is not None, "Python liefert einen SHA-256", aus_python[:16])
        pruefe(aus_python == aus_ps,
               "beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash",
               f"py {aus_python[:16]} | ps {aus_ps[:16]}")

        # Und der Riegel selbst: ein Nicht-ASCII-Pfad muss ABBRECHEN, nicht
        # irgendeinen Hash liefern. Ohne diese Zeile waere die Einengung nur
        # eine Behauptung im Vertrag.
        (ordner / "gruße.txt").write_bytes(b"x")
        try:
            ordner_hash(ordner)
            pruefe(False, "Nicht-ASCII im Pfad bricht ab (Python)")
        except OrdnerHashFehler:
            pruefe(True, "Nicht-ASCII im Pfad bricht ab (Python)")
        nicht_ascii = subprocess.run(
            [pwsh, "-NoProfile", "-File", str(PS_ORDNERHASH), str(ordner)],
            capture_output=True, text=True,
        )
        pruefe(nicht_ascii.returncode != 0, "Nicht-ASCII im Pfad bricht ab (PowerShell)",
               f"Exit {nicht_ascii.returncode}")


# ── [4] Auslieferungsstand und [4b] installierter Stand (NAK-94) ───────────


def _artefakt_name(eintrag: dict) -> str:
    """Dieselbe Identitaet auf beiden Seiten: die Ziel-ID, wo es eine gibt,
    sonst der Name. Der Broker hat keine Ziel-ID (er ist kein Plugin-Ziel der
    Identitaetsdatei), und das Installationsjournal schreibt fuer ihn
    `ziel_id: null` - beide Seiten landen darum auf `eqcop-broker.exe`."""
    return eintrag.get("ziel_id") or eintrag.get("name") or "<ohne Kennung>"


def auslieferungsstand(manifest: dict, hart: bool) -> None:
    """Gebautes Artefakt gegen den festgeschriebenen Hash.

    `hart` (nur bei --release) macht aus einer Abweichung einen Fehler. Im
    Kanon ist sie ein Hinweis: nach einem Relink MUSS sie auftreten, und ein
    Riegel, der immer rot ist, unterscheidet nichts mehr (NAK-94).

    Zwei Fragen, die NICHT dasselbe sind (Befund C1, Nacharbeit 1):

      LIEGT das festgeschriebene Artefakt ueberhaupt vor und ist sein Hash
      bildbar? Das wird fuer JEDES Artefakt geprueft, unabhaengig davon, ob
      `sha256` gesetzt ist, und ist in BEIDEN Modi ein Fehler. Ein Relink
      aendert Bytes; er laesst kein Bundle verschwinden.

      STIMMT der Hash mit dem festgeschriebenen ueberein? Verglichen wird nur,
      wo `sha256` gesetzt ist. Ein `null` bleibt Hinweis (Kanon) bzw. Fehler
      (--release).

    Frueher beendete ein einziges `sha256: null` mit `return` den ganzen
    Artefaktcheck - gemessen: ein umbenanntes Bundle blieb im Kanon gruen."""
    print("\n[4] Auslieferungsstand"
          + ("  - HART (--release: das hier ist die Auslieferung)" if hart
             else "  - Kanon: eine Abweichung ist ein Hinweis, kein Fehler"))
    offen = [_artefakt_name(a) for a in manifest["artefakte"] if a.get("sha256") is None]
    if offen:
        text = f"nicht ausgeliefert - {len(offen)} Artefakt(e) ohne Hash: {', '.join(offen)}"
        if hart:
            # Unter --release ist ein Manifest ohne Hashes kein Zwischenstand,
            # sondern ein Paket, das der Installer sofort abweisen wuerde. Gruen
            # zu melden hiesse, eine Auslieferung zu bescheinigen, die es nicht
            # gibt.
            pruefe(False, text)
        else:
            # KEIN Fehler: ein Manifest ohne Hashes ist der ehrliche Normalfall
            # zwischen zwei Releases. Es ist nur nicht ausliefer-BAR, und genau
            # das steht hier - statt still gruen zu sein.
            print("  hinweis " + text)
        print("          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).")
        # KEIN return (Befund C1): die Existenzfrage steht unabhaengig davon.

    for a in manifest["artefakte"]:
        pfad = WURZEL / a["quelle"]
        art = a.get("art")
        name = _artefakt_name(a)
        # Fehlendes Artefakt und nicht bildbarer Ordner-Hash bleiben in BEIDEN
        # Modi Fehler: ein Relink aendert Bytes, er laesst kein Bundle
        # verschwinden. Was der Relink nicht verursachen kann, bleibt
        # fail-closed (Pruefliste D).
        if art not in ARTEN or not artefakt_liegt_vor(pfad, art):
            pruefe(False, f"{name}: das festgeschriebene Artefakt liegt nicht vor", a["quelle"])
            continue
        try:
            ist = artefakt_hash(pfad, art)
        except OrdnerHashFehler as e:
            pruefe(False, f"{name}: Ordner-Hash nicht bildbar", str(e))
            continue
        soll = a.get("sha256")
        if soll is None:
            # Ohne festgeschriebenen Hash gibt es nichts zu VERGLEICHEN - dass
            # das Artefakt vorliegt und sein Hash bildbar ist, ist trotzdem
            # gemessen und wird als solches gezaehlt.
            pruefe(True, f"{name}: Artefakt liegt vor, Ordner-Hash bildbar",
                   f"gebaut {ist[:16]}; kein festgeschriebener Hash zum Vergleich")
            continue
        if ist == soll:
            pruefe(True, f"{name}: gebautes Artefakt stimmt mit dem festgeschriebenen Hash",
                   soll[:16])
        elif hart:
            pruefe(False, f"{name}: gebautes Artefakt stimmt mit dem festgeschriebenen Hash",
                   f"Manifest {soll[:16]} | gebaut {ist[:16]}")
        else:
            print(f"  hinweis {name}: Bau weicht vom festgeschriebenen Paket ab "
                  f"(nach Relink erwartet; vor einer Auslieferung --hashen)"
                  f"  [Manifest {soll[:16]} | gebaut {ist[:16]}]")


def installierter_stand(manifest: dict) -> None:
    """[4b] als Bericht, der NIE abbricht (Befund C2, Nacharbeit 1).

    Gemessen wurde, dass ein Journaleintrag `{"ziel_id": ["main"]}` den ganzen
    Lauf mit `TypeError: unhashable type: 'list'` beendete - ein Bericht ohne
    Urteil darf einen Kanonlauf nicht toeten. Die eigentliche Arbeit steht in
    _installierter_stand(); hier steht nur die Huelle, die jeden Fehler in
    einen Hinweis verwandelt.
    """
    print("\n[4b] Installierter Stand  - Bericht, kein Urteil")
    try:
        _installierter_stand(manifest)
    except Exception as e:
        print(f"  hinweis install-ergebnis.json nicht auswertbar: {e!r}")


def cmake_broker_startbindung(manifest: dict) -> None:
    """Der tatsaechlich kompilierte Header folgt dem Installer-Manifest.

    Der Vergleich ist absichtlich NACH dem CMake-Lauf: eine bloss richtige
    Template-Datei beweist nicht, dass ein geaenderter Manifesthash den Build
    erreicht hat. `CMAKE_CONFIGURE_DEPENDS` schliesst genau dieses Stalefenster.
    """
    print("\n[4c] Manifestgebundene Broker-Startwerte")
    broker = [a for a in manifest["artefakte"] if a.get("art") == "broker"]
    if len(broker) != 1 or not BROKER_BINDING.is_file():
        pruefe(False, "generierter BrokerInstallBinding-Header liegt vor",
               str(BROKER_BINDING.relative_to(WURZEL)))
        return

    text = BROKER_BINDING.read_text(encoding="utf-8")

    def literal(name: str) -> str | None:
        treffer = re.search(rf"\b{re.escape(name)}\s*=\s*L?\"([^\"]*)\"\s*;", text)
        return treffer.group(1) if treffer else None

    artefakt = broker[0]
    erwartet = (
        manifest["ziele"].get("broker_verzeichnis", "").rstrip("/\\")
        + "/" + str(artefakt.get("name", ""))
    )
    thumb = manifest.get("signatur", {}).get("authenticode_thumbprint") or ""
    ist = (literal("brokerPfad"), literal("brokerSha256"),
           literal("authenticodeThumbprint"))
    soll = (erwartet, artefakt.get("sha256"), thumb)
    pruefe(ist == soll,
           "generierter Produktheader entspricht Pfad, Hash und Thumbprint des Manifests",
           f"ist={ist!r} soll={soll!r}")

    cmake = PLUGIN_CMAKE.read_text(encoding="utf-8")
    pruefe("CMAKE_CONFIGURE_DEPENDS" in cmake
               and '"${NAKAMA_INSTALLER_MANIFEST}"' in cmake,
           "Installer-Manifest ist CMake-Configure-Dependency")


def _installierter_stand(manifest: dict) -> None:
    """Entspricht der INSTALLIERTE Stand dem heutigen Manifest?

    Reiner Bericht. Dieser Block erzeugt nie einen Fehler: Installieren ist ein
    bewusster Admin-Handgriff des Users, kein Bestandteil eines Kanonlaufs. Ein
    Rechner ohne Installation ist kein defekter Rechner - er hat nur nicht
    installiert, und genau das steht dann hier.

    Gelesen wird `install-ergebnis.json`, das Install-Nakama.ps1 nach jedem
    Zug schreibt. Sein `eintraege[].sha256` ist nicht bloss eine Absicht: das
    Skript prueft nach dem Kopieren den Zielstand gegen genau diesen Wert und
    bricht sonst ab. Der Eintrag sagt also, was WIRKLICH liegt."""
    if not INSTALL_ERGEBNIS.is_file():
        print("  hinweis nichts installiert - install-ergebnis.json liegt nicht vor "
              f"({INSTALL_ERGEBNIS.relative_to(WURZEL).as_posix()})")
        return
    try:
        journal = json.loads(INSTALL_ERGEBNIS.read_text(encoding="utf-8"))
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as e:
        print(f"  hinweis install-ergebnis.json nicht lesbar: {e}")
        return
    if not isinstance(journal, dict) or journal.get("schema") != ERGEBNIS_SCHEMA:
        print(f"  hinweis install-ergebnis.json traegt nicht {ERGEBNIS_SCHEMA} "
              f"(gelesen: {journal.get('schema') if isinstance(journal, dict) else type(journal).__name__!r})")
        return

    print(f"  Journal: status={journal.get('status')!r}  zeit={journal.get('zeit')!r}")

    # BEFUND P2, NAK-94 Nacharbeit 2 (29.08.2026): der Journalstatus wurde nur
    # GEDRUCKT, nie ausgewertet. Ein Journal mit status="KOMPENSATION" oder
    # "ERROR_TEILSTAND" und einem Eintrag mit mutation_abgeschlossen=true,
    # rollback_abgeschlossen=false erreichte den Hashvergleich und meldete
    # "ok ... installierter Stand = Manifest".
    #
    # Install-Nakama.ps1 setzt genau diese Werte, WAEHREND es das Ziel wieder
    # in den Vorzustand zurueckkopiert: der gespeicherte sha256 beweist dann
    # nur den Stand VOR dem fehlgeschlagenen Gegenakt, nicht das, was liegt.
    # Dasselbe gilt fuer VORBEREITET (Fenster vor der Mutation), RUECKWEG_AKTIV
    # und RUECKWEG (Deinstallation) und ERROR_RUECKGEROLLT (vollstaendig
    # zurueckgerollt) - die Statuswerte stehen in Install-Nakama.ps1, sie sind
    # hier abgelesen und nicht geraten.
    #
    # Regel seither: `ok` gibt es NUR bei status == "OK". Jeder andere Status,
    # ein unbekannter und ein fehlender melden "installierter Stand unbekannt"
    # OHNE Hashvergleich - ein Hash, der nichts beweist, wird gar nicht erst
    # gezeigt. Die Eintragsmarken (abgeschlossen, nicht zurueckgerollt) prueft
    # die Schleife unten weiterhin einzeln; beide Haelften gelten zusammen.
    #
    # BEFUND P2, NAK-94 Nacharbeit 3 (29.08.2026): diese Sperre stand HINTER
    # der Eintragsliste - und ein regulaer abgeschlossener Rueckweg schreibt
    # gar keine. Install-Nakama.ps1 legt am Ende des Gegenpfads ein Journal aus
    # genau sieben Feldern ab (schema, status='RUECKWEG', transaktions_id,
    # erzwungen, warnungen, getan, zeit); `eintraege` ist keines davon.
    # Gemessen am Stand a94c33e meldete [4b] dafuer
    # "install-ergebnis.json fuehrt keine Liste 'eintraege'" statt des
    # zugesagten artefaktweisen "Journalstatus RUECKWEG" - die Zusage des
    # Runners galt also fuer das reale Rueckweg-Journal nicht.
    #
    # Regel seither: die Statussperre steht unmittelbar nach der Schema-
    # Pruefung und VOR jeder Verwendung von `eintraege`. Bei jedem Status != OK
    # endet die Funktion ohne Hashvergleich; ob eine Eintragsliste vorliegt,
    # ist dann unerheblich. `eintraege` wird nur noch im OK-Pfad verlangt -
    # dort ist ihr Fehlen die richtige Aussage.
    status = journal.get("status")
    if status != ERGEBNIS_STATUS_OK:
        wie = status if isinstance(status, str) else (
            "fehlt" if status is None else repr(status))
        for a in manifest["artefakte"]:
            print(f"  hinweis {_artefakt_name(a)}: installierter Stand unbekannt "
                  f"(Journalstatus {wie})")
        return

    eintraege = journal.get("eintraege")
    if not isinstance(eintraege, list):
        print("  hinweis install-ergebnis.json fuehrt keine Liste 'eintraege'")
        return

    # Die Kennung MUSS eine Zeichenkette sein: `_artefakt_name` reicht
    # durch, was im Journal steht, und `{"ziel_id": ["main"]}` waere als
    # dict-Schluessel nicht hashbar (Befund C2).
    nach_kennung: dict[str, list[dict]] = {}
    for index, e in enumerate(eintraege):
        if not isinstance(e, dict):
            print(f"  hinweis {index}: Journaleintrag ist kein Objekt "
                  f"({type(e).__name__})")
            continue
        kennung = _artefakt_name(e)
        if not isinstance(kennung, str):
            print(f"  hinweis {index}: Journaleintrag ohne lesbare Kennung "
                  f"({kennung!r})")
            continue
        nach_kennung.setdefault(kennung, []).append(e)

    for a in manifest["artefakte"]:
        name = _artefakt_name(a)
        soll = a.get("sha256")
        treffer = nach_kennung.get(name, [])
        if not treffer:
            print(f"  hinweis {name}: nicht installiert - kein Eintrag im Journal")
            continue
        if len(treffer) > 1:
            print(f"  hinweis {name}: {len(treffer)} Journaleintraege mit derselben Kennung "
                  "- der erste wird berichtet")
        e = treffer[0]
        if e.get("rollback_abgeschlossen") is True:
            print(f"  hinweis {name}: zurueckgerollt - der Vorzustand liegt, nicht dieses Paket")
            continue
        if e.get("mutation_abgeschlossen") is not True:
            print(f"  hinweis {name}: Installation nicht abgeschlossen "
                  f"(mutation_begonnen={e.get('mutation_begonnen')!r})")
            continue
        ist = str(e.get("sha256") or "").upper()
        if not HEX64.match(ist):
            print(f"  hinweis {name}: Journaleintrag traegt keinen SHA-256 ({e.get('sha256')!r})")
            continue
        if not isinstance(soll, str) or not HEX64.match(soll):
            print(f"  hinweis {name}: das Manifest hat keinen Hash - der installierte Stand "
                  f"({ist[:16]}) laesst sich mit nichts vergleichen")
            continue
        ziel = e.get("ziel") or "?"
        if ist == soll:
            print(f"  ok      {name}: installierter Stand = Manifest  [{soll[:16]}]  {ziel}")
        else:
            print(f"  hinweis {name}: installierter Stand ist ein anderer als der im Manifest "
                  f"festgeschriebene  [installiert {ist[:16]} | Manifest {soll[:16]}]  {ziel}")


def authenticode_systemfixture():
    """A17-Haelfte von L-12: die echte C++-Pruefkette gegen das fuer diesen
    Test gelesene Signerzertifikat einer OS-vertrauenswuerdigen, signierten
    Windows-Fixture. Eine unsigned Temp-Kopie liefert den Fehlend-Fall; der
    falsche und der aus WinVerifyTrust gelesene echte Thumbprint liefern die
    beiden anderen Urteile. Kein Zertifikatsspeicher wird veraendert."""
    system_datei = pathlib.Path(os.environ.get("SystemRoot", r"C:\Windows")) \
        / "System32" / "notepad.exe"
    if not IPC_TEST_EXE.is_file() or not system_datei.is_file():
        grund = (f"C++-Testprogramm fehlt: {IPC_TEST_EXE}"
                 if not IPC_TEST_EXE.is_file()
                 else f"signierte Windows-Fixture fehlt: {system_datei}")
        pruefe(False, "Windows-Signaturfixture: WinVerifyTrust gueltig/fehlend/falsch", grund)
        return

    with tempfile.TemporaryDirectory(prefix="nakama-a17-authenticode-") as tmp:
        unsigned = pathlib.Path(tmp) / "unsigned.exe"
        shutil.copy2(IPC_TEST_EXE, unsigned)
        def lauf(pfad: pathlib.Path, sha256: str, thumb: str, erwartet: str):
            return subprocess.run(
                [str(IPC_TEST_EXE), "--phase-b-verify-binary", str(pfad),
                 sha256, thumb, erwartet], text=True, capture_output=True,
                encoding="utf-8", errors="replace", check=False)

        unsigned_hash = hashlib.sha256(unsigned.read_bytes()).hexdigest().upper()
        system_hash = hashlib.sha256(system_datei.read_bytes()).hexdigest().upper()
        fehlend = lauf(unsigned, unsigned_hash, "0" * 40,
                       "signaturFehltOderUngueltig")
        falsch = lauf(system_datei, system_hash, "0" * 40, "signerFalsch")
        signer_treffer = re.search(r"\bsigner=([0-9A-F]{40}|[0-9A-F]{64})\b",
                                   falsch.stdout)
        gueltig = (lauf(system_datei, system_hash, signer_treffer.group(1), "ok")
                   if signer_treffer else None)
        laeufe = [fehlend, falsch] + ([gueltig] if gueltig is not None else [])
        ausgabe = "\n".join(
            x for run in laeufe for x in (run.stdout.strip(), run.stderr.strip()) if x)
        pruefe(all(run.returncode == 0 for run in laeufe)
                   and signer_treffer is not None and gueltig is not None,
               "Windows-Signaturfixture: WinVerifyTrust gueltig/fehlend/falsch",
               ausgabe[-1800:] if ausgabe else "keine Ausgabe")


def _argumente(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    g = p.add_mutually_exclusive_group()
    g.add_argument("--hashen", action="store_true",
                   help="Release-Schritt: sha256 aus den gebauten Artefakten festschreiben")
    g.add_argument("--release", action="store_true",
                   help="Auslieferungsschritt: [4] vergleicht HART gegen die festgeschriebenen "
                        "Hashes (Exit 2 bei Abweichung). Ohne dieses Flag ist eine Abweichung "
                        "ein Hinweis - nach einem Relink ist sie der Normalfall (NAK-94).")
    p.add_argument("--debug", action="store_true",
                   help="bei einer unerwarteten Ausnahme zusaetzlich den Traceback "
                        "zeigen; ohne dieses Flag gibt es nur die Klartextzeile")
    return p.parse_args(argv)


def _abbruch_klartext(fehler: BaseException) -> str:
    """Typ, Meldung und AUSLOESERZEILE in einer Zeile - alles, was von einem
    Traceback gebraucht wird, ohne einen zu drucken.

    Ohne den Ort waere die Meldung eines `KeyError` nur der Schluesselname und
    damit fast nutzlos: 'ziel_id' sagt nicht, WO gelesen wurde."""
    spur = traceback.extract_tb(fehler.__traceback__)
    ort = ""
    if spur:
        rahmen = spur[-1]
        try:
            datei = (pathlib.Path(rahmen.filename).resolve()
                     .relative_to(WURZEL).as_posix())
        except (ValueError, OSError):
            datei = rahmen.filename
        ort = f" @ {datei}:{rahmen.lineno}"
    return f"{type(fehler).__name__}: {fehler}{ort}"


def _geschuetzt(fn, debug: bool = False) -> tuple[str, str, object]:
    """Die eine kontrollierte Huelle um den ganzen Lauf.

    Rueckgabe: (Klasse, Klartext, Wert). 'gruen' = durchgelaufen; 'strukturhalt' =
    Vertragsbruch einer gelesenen JSON-Datei; 'gegenprobe_unmoeglich' = der eigene,
    bereits kontrollierte SystemExit aus [3]; 'unkontrolliert' = jede andere
    Ausnahme - nur sie ist der Bruch der Zusage 'nie ein Traceback'.
    """
    try:
        return "gruen", "", fn()
    except Strukturhalt as halt:
        return "strukturhalt", str(halt), halt
    except SystemExit as aus:
        return "gegenprobe_unmoeglich", str(aus), aus
    except Exception as unerwartet:
        if debug:
            traceback.print_exc()
        return "unkontrolliert", _abbruch_klartext(unerwartet), unerwartet


def main(argv=None) -> int:
    """Zentraler Faenger: was den Strukturvertraegen entgeht, endet als Klartextzeile
    mit Typ, Meldung, Datei und Zeile, Exit 2, ohne Traceback (nur mit --debug).
    SystemExit laeuft durch - der eigene, kontrollierte Ausgang aus [3]."""
    args = _argumente(argv)
    klasse, text, wert = _geschuetzt(lambda: _lauf(args), args.debug)
    if klasse == "gruen":
        return wert
    if klasse == "gegenprobe_unmoeglich":
        raise wert
    zeile = (f"ABGEBROCHEN - {text}" if klasse == "strukturhalt" else
             "ABGEBROCHEN - unerwartete Ausnahme, kontrolliert beendet: "
             + text + ("" if args.debug else "  (Traceback mit --debug)"))
    print("")
    print(zeile)
    print(zeile, file=sys.stderr)
    return 2


def _lauf(args) -> int:
    print("Strukturvertrag: jede von diesem Bein gelesene JSON-Datei wird VOR "
          "dem ersten Zugriff strukturell geprueft;")
    print("                 Verstoss = kontrollierter Abbruch mit Klartext, nie "
          "ein Traceback - im urteilsfreien [4b] ein Hinweis.")
    try:
        manifest = _lies_geprueft(MANIFEST, _installermanifest_struktur)
        identitaet = _lies_geprueft(IDENTITAET, _identitaet_struktur)
    except Strukturhalt as halt:
        # Ohne diese beiden Dateien in gelesener Form misst KEIN Block etwas.
        # Am Basis-Stand b8dcbe1 starben hier "artefakte" -> "xrtefakte"
        # (in r_art_bekannt) und "ziele" -> "xiele" (in _ziele) mit einem
        # Traceback; jetzt ist es ein Urteil (NAK-94 Nacharbeit 9).
        print("")
        print(f"ABGEBROCHEN - {halt}")
        return 2

    if args.hashen:
        # Auch der mutierende Release-Aufruf muss erst denselben Strukturvertrag
        # bestehen. Sonst koennte er ausgerechnet die Pfad- und Zielregeln
        # umgehen, die der normale Kanonlauf prueft, und beliebige `quelle`-
        # Eintraege mit einem gueltigen Hash adeln.
        print("[0] Struktur vor dem mutierenden Release-Schritt")
        for regel, text in REGELN:
            try:
                bedingung, zusatz = regel(manifest, identitaet)
            except Exception as e:
                bedingung, zusatz = False, str(e)
            pruefe(bedingung, text, zusatz)
        if fehler:
            print("\nABGEBROCHEN - ein strukturell ungueltiges Manifest wird nicht gehasht.")
            return 2
        return hashen(manifest)

    print("[1] Struktur - eine Identitaet, ein Ort")
    for regel, text in REGELN:
        bedingung, zusatz = regel(manifest, identitaet)
        pruefe(bedingung, text, zusatz)

    print("\n[2] Gegenprobe - dieselben Regeln an verdorbener Eingabe")
    kaputt = verdirb(manifest, identitaet)
    kaputte_identitaet = verdirb_identitaet(identitaet)
    for regel, text in REGELN:
        if regel is r_stilllegungsmarke_lesbar and not _stillgelegte(identitaet):
            # Ehrlicher als eine gruene Zeile: ohne stillgelegtes Ziel gibt es
            # keine Marke zu verderben - dann hat diese Regel aber auch nichts
            # mehr zu schuetzen. Gesagt wird das, nicht verschwiegen.
            print("  hinweis kein stillgelegtes Ziel - gegenstandslos: " + text)
            continue
        try:
            bedingung, _ = regel(kaputt, kaputte_identitaet)
        except Exception as e:            # eine Regel, die stolpert, hat gesehen
            bedingung = False             # dass etwas nicht stimmt - das zaehlt
            _ = e
        pruefe(not bedingung, "faellt an verdorbener Eingabe: " + text)

    adversariale_strukturproben(manifest, identitaet)

    auslieferungsstand(manifest, hart=args.release)
    cmake_broker_startbindung(manifest)
    installierter_stand(manifest)

    kreuzprobe()
    print("\n[6] Authenticode-Pruefkette mit signierter Windows-Systemfixture")
    authenticode_systemfixture()

    print(f"\n{ok} ok, {len(fehler)} Fehler")
    if fehler:
        print("\nFEHLGESCHLAGEN:")
        for f in fehler:
            print("  - " + f)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
