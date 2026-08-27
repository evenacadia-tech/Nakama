#!/usr/bin/env py -3.13
"""S9/SONDE-007b Abschnitt 3 - Kanon-Bein A17: das Installer-Manifest.

WAS DIESES BEIN BEHAUPTET

  Das Installer-Manifest (eq-copilot/install/nakama-installer-v1.json) traegt
  KEINE zweite Identitaet. Es nennt nur Ziel-IDs; jeder Produktname, jeder
  Viercode und jede Class-ID bleibt in identity/plugin-identities-v1.json.
  Genau das war NAK-52 auf der Bauseite, und ein Installer ist die andere
  Haelfte derselben Gefahr: ein Paket, das Bundlenamen selbst fuehrt, kann
  eine Datei ausliefern, deren Name nirgends eingefroren ist.

  Die Quellpfade werden deshalb nicht gelesen, sondern NACHGERECHNET: aus
  `cmake_ziel` und dem Bundlenamen der Identitaetsdatei entsteht genau ein
  Pfad, und der muss im Manifest stehen. Ein Pfad, der auf ein fremdes Bundle
  zeigt, faellt hier.

ORDNER-HASH v1 (S9-Nacharbeit 23.08.2026, T2-Befund T2-5)

  Ein VST3-Bundle ist unter Windows ein ORDNER; die Auslieferungseinheit ist
  seit der Nacharbeit derselbe Ordner. Dieses Bein traegt die PYTHON-Haelfte
  des Ordner-Hashes (Vertrag §2.1) - die PowerShell-Haelfte liegt in
  eq-copilot/install/NakamaOrdnerHash.ps1 und wird vom Installer benutzt.

  Weil ein Hash, den ZWEI Sprachen bilden, nur so viel wert ist wie ihre
  Uebereinstimmung, misst dieses Bein sie gegeneinander - an einem
  SYNTHETISCHEN Ordner, nicht an einem gebauten Bundle, damit die Kreuzprobe
  auch ohne Bau laeuft. Fehlt `pwsh`, ist das ein FEHLER und keine stille
  Auslassung: eine Kreuzprobe, die nicht lief, hat nichts bewiesen.

DIE GEGENPROBE (S8-Lehre, Manifest SONDE-007a):

  "Ein Riegel, der etwas NICHT findet, sagt nichts, bis gezeigt ist, dass er
   ueberhaupt etwas finden kann."

  Darum faehrt `--gegenprobe` (im normalen Lauf immer mit) jede Strukturregel
  ein zweites Mal gegen ein ABSICHTLICH verdorbenes Manifest im Speicher. Wer
  dort gruen bliebe, prueft nichts.

Aufrufe:
  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py            # Kanon
  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --hashen   # Release
"""

from __future__ import annotations

import argparse
import copy
import hashlib
import json
import ntpath
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
from datetime import datetime, timezone

WURZEL = pathlib.Path(__file__).resolve().parents[2]
MANIFEST = WURZEL / "eq-copilot" / "install" / "nakama-installer-v1.json"
IDENTITAET = WURZEL / "eq-copilot" / "identity" / "plugin-identities-v1.json"
BROKER_CARGO = WURZEL / "broker" / "Cargo.toml"
PS_ORDNERHASH = WURZEL / "eq-copilot" / "install" / "NakamaOrdnerHash.ps1"
STATE_CPP = WURZEL / "eq-copilot" / "plugin" / "state" / "NakamaState.cpp"

SCHEMA = "nakama.installer/v1"
HEX64 = re.compile(r"^[0-9A-F]{64}$")
THUMBPRINT = re.compile(r"^(?:[0-9A-F]{40}|[0-9A-F]{64})$")
ARTEN = ("vst3", "broker")
CMAKE_ZIEL = re.compile(r"^[A-Za-z][A-Za-z0-9_]*$")


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
    """Wie im Identitaetstest: 3 vs 3. Ein viertes Ziel im Identitaetsmanifest
    ohne Installer-Eintrag bringt dieses Bein zum Sprechen, statt still
    ungemessen zu bleiben."""
    ident_ziele = i.get("ziele", [])
    ids = [z.get("id") for z in ident_ziele]
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
        and len(ident_ziele) == 3
        and all(isinstance(zid, str) and zid for zid in ids)
        and len([zid.casefold() for zid in ids]) == len(set(zid.casefold() for zid in ids))
        and sichere_bundles
        and len([b.casefold() for b in bundles]) == len(set(b.casefold() for b in bundles))
        and all(type(z.get("state_schema")) is int and z["state_schema"] == STATE_SCHEMA
                for z in ident_ziele)
    )
    passt = (
        identity_ok
        and sorted(x for x in aus_manifest if x is not None) == sorted(ids)
        and len(aus_manifest) == len(set(aus_manifest))
    )
    return passt, f"{len(aus_manifest)} vs {len(ids)}; identity={'ok' if identity_ok else 'ungueltig'}"


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
    for ziel in i["ziele"]:
        for feld in ("plugin_code", "component_cid", "controller_cid"):
            wert = ziel.get(feld)
            if wert and wert in ohne_quellen:
                treffer.append(f"{ziel['id']}.{feld}")
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
    (r_jedes_ziel_genau_einmal, "Identitaet ist kollisionsfrei, schema=2 und jedes Ziel hat genau einen VST3-Eintrag"),
    (r_quellpfade_nachgerechnet, "jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet"),
    (r_keine_identitaetsliterale, "kein Viercode, keine Class-ID im Installer-Manifest"),
    (r_broker, "genau ein Broker-Artefakt, aus dem Release-Pfad der Crate"),
    (r_broker_heisst_wie_die_crate, "der Broker-Binaername kommt aus broker/Cargo.toml"),
    (r_zielverzeichnisse, "VST3 nach Common Files, Broker geschuetzt unter Program Files"),
    (r_signatur_ehrlich, "die Signaturzeile behauptet keine Pruefung ohne Mittel"),
    (r_hashfelder, "jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben"),
    (r_bekannte_staende, "jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema"),
    (r_rueckweg_vollstaendig, "der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)"),
]


# ── Gegenprobe: jede Regel muss an einem verdorbenen Manifest FALLEN ────────

def verdirb(m: dict) -> dict:
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
    # S9-Nacharbeit (T2-4): eine dritte Artefaktsorte. Genau dieser Eintrag
    # rutschte vorher durch ALLE zwoelf Regeln - `_vst3()` filtert ihn weg,
    # `r_broker` zaehlt ihn nicht - und landete im Skript im Broker-Zweig.
    k["artefakte"].append({"art": "standalone", "name": "Nakama.exe",
                           "quelle": "irgendwo/Nakama.exe", "sha256": "kein hash"})
    k["ziele"] = {"vst3_verzeichnis": "C:/Temp", "broker_verzeichnis": "%LOCALAPPDATA%/nakama"}
    k["signatur"] = {"verfahren": "sha256-manifest", "authenticode_thumbprint": None, "warum_null": ""}
    k["rueckweg"] = {"strategie": "", "bekannte_staende": [
        {"sha256": "xx", "ziel_id": "?", "state_schema": "eins", "hash_art": "erfunden"}]}
    return k


def adversariale_strukturproben(m: dict, i: dict) -> None:
    """Die vier ehemals offenen Kanten einzeln brechen.

    Die grobe `verdirb`-Probe beweist, dass eine Regel ueberhaupt sehen kann.
    Diese Proben beweisen dagegen die konkreten Umgehungen aus Paket 03:
    Zielkollision, Identity/Writer-Schemadrift und lexikalisches Traversal.
    """
    print("\n[3] Adversariale Pfad- und Identitaetsgegenproben")

    ident = copy.deepcopy(i)
    manifest = copy.deepcopy(m)
    ident["ziele"][1]["bundle"] = ident["ziele"][0]["bundle"]
    zid = ident["ziele"][1]["id"]
    artefakt = next(a for a in manifest["artefakte"] if a.get("ziel_id") == zid)
    artefakt["quelle"] = (
        f"eq-copilot/build/plugin/{artefakt['cmake_ziel']}_artefacts/Release/VST3/"
        f"{ident['ziele'][1]['bundle']}"
    )
    pruefe(not r_jedes_ziel_genau_einmal(manifest, ident)[0],
           "faellt an einer Bundle-Zielkollision")

    for wert, name in ((1, "1"), ("kaputt", "Text"), (None, "fehlend")):
        ident = copy.deepcopy(i)
        if wert is None:
            ident["ziele"][0].pop("state_schema", None)
        else:
            ident["ziele"][0]["state_schema"] = wert
        pruefe(not r_jedes_ziel_genau_einmal(m, ident)[0],
               f"faellt an Identity-state_schema {name}")

    manifest = copy.deepcopy(m)
    manifest["ziele"]["broker_verzeichnis"] = "C:/Program Files/../Temp/Nakama"
    pruefe(not r_zielverzeichnisse(manifest, i)[0],
           "faellt an kanonischem Broker-Zieltraversal")

    manifest = copy.deepcopy(m)
    artefakt = next(a for a in manifest["artefakte"] if a.get("art") == "vst3")
    artefakt["cmake_ziel"] = "../../../../outside"
    bundle = _ziele(i)[artefakt["ziel_id"]]["bundle"]
    artefakt["quelle"] = (
        f"eq-copilot/build/plugin/{artefakt['cmake_ziel']}_artefacts/Release/VST3/{bundle}"
    )
    pruefe(not r_quellpfade_nachgerechnet(manifest, i)[0],
           "faellt an cmake_ziel-Quelltraversal")

    manifest = copy.deepcopy(m)
    manifest["rueckweg"]["verzeichnis"] = "eq-copilot/install/rueckweg"
    pruefe(not r_rueckweg_vollstaendig(manifest, i)[0],
           "faellt an benutzerbeschreibbaren Rueckweg-Backups")


# ── Hashen (Release-Schritt, nicht Kanon) ──────────────────────────────────

def datei_hash(pfad: pathlib.Path) -> str:
    h = hashlib.sha256()
    with pfad.open("rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest().upper()


def hashen(manifest: dict) -> int:
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
    MANIFEST.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"\ngeschrieben: {MANIFEST.relative_to(WURZEL)}")
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


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--hashen", action="store_true",
                   help="Release-Schritt: sha256 aus den gebauten Artefakten festschreiben")
    args = p.parse_args()

    manifest = json.loads(MANIFEST.read_text(encoding="utf-8"))
    identitaet = json.loads(IDENTITAET.read_text(encoding="utf-8"))

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

    print("\n[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest")
    kaputt = verdirb(manifest)
    for regel, text in REGELN:
        try:
            bedingung, _ = regel(kaputt, identitaet)
        except Exception as e:            # eine Regel, die stolpert, hat gesehen
            bedingung = False             # dass etwas nicht stimmt - das zaehlt
            _ = e
        pruefe(not bedingung, "faellt am verdorbenen Manifest: " + text)

    adversariale_strukturproben(manifest, identitaet)

    print("\n[4] Auslieferungsstand")
    offen = [a.get("ziel_id") or a.get("name") for a in manifest["artefakte"] if a.get("sha256") is None]
    if offen:
        # KEIN Fehler: ein Manifest ohne Hashes ist der ehrliche Normalfall
        # zwischen zwei Releases. Es ist nur nicht ausliefer-BAR, und genau
        # das steht hier - statt still gruen zu sein.
        print(f"  hinweis nicht ausgeliefert - {len(offen)} Artefakt(e) ohne Hash: {', '.join(offen)}")
        print("          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).")
    else:
        for a in manifest["artefakte"]:
            pfad = WURZEL / a["quelle"]
            art = a.get("art")
            name = a.get("ziel_id") or a.get("name")
            if art not in ARTEN or not artefakt_liegt_vor(pfad, art):
                pruefe(False, f"{name}: das festgeschriebene Artefakt liegt nicht vor", a["quelle"])
                continue
            try:
                ist = artefakt_hash(pfad, art)
            except OrdnerHashFehler as e:
                pruefe(False, f"{name}: Ordner-Hash nicht bildbar", str(e))
                continue
            pruefe(ist == a["sha256"],
                   f"{name}: gebautes Artefakt stimmt mit dem festgeschriebenen Hash", a["sha256"][:16])

    kreuzprobe()

    print(f"\n{ok} ok, {len(fehler)} Fehler")
    if fehler:
        print("\nFEHLGESCHLAGEN:")
        for f in fehler:
            print("  - " + f)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
