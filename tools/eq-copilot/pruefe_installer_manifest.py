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


def _aktive(identitaet: dict) -> list[dict]:
    """Ziele, die heute gebaut und ausgeliefert werden.

    S9b/SONDE-007c (28.08.2026): ein Ziel mit dem Feld `stillgelegt` bleibt in
    der Identitaetsdatei stehen - seine Kennung ist gesperrt, nicht frei -,
    gehoert aber nicht mehr in die Auslieferung. Die Trennung faellt auf die
    ANWESENHEIT des Feldes, nicht auf seinen Inhalt: fail-closed.
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
        for feld in ("seit", "warum", "umgang_mit_altbestand", "kennung_bleibt"):
            if not str(e.get(feld, "")).strip():
                fehler.append(f"{e.get('ziel_id')!r}: {feld} fehlt")
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
    artefakt = next(a for a in manifest["artefakte"] if a.get("ziel_id") == zid)
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
        vorlage = next(a for a in manifest["artefakte"] if a.get("art") == "vst3")
        heimlich = copy.deepcopy(vorlage)
        heimlich["ziel_id"] = stillgelegte[0]["id"]
        manifest["artefakte"].append(heimlich)
        pruefe(not r_stillgelegte_benannt(manifest, i)[0],
               "faellt, wenn ein stillgelegtes Ziel doch ausgeliefert wird")

        manifest = copy.deepcopy(m)
        manifest["stillgelegte_ziele"] = []
        pruefe(not r_stillgelegte_benannt(manifest, i)[0],
               "faellt, wenn ein stillgelegtes Ziel nirgends benannt ist")

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

    manifest = copy.deepcopy(m)
    entfernt = next(a for a in manifest["artefakte"] if a.get("art") == "vst3")
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
    kaputt = verdirb(manifest, identitaet)
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
