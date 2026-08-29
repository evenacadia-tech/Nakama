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

DREI HAERTEGRADE FUER DENSELBEN HASH (NAK-94, 29.08.2026)

  Der `sha256` eines Artefakts beantwortet je nach Aufrufer eine ANDERE Frage,
  und darum darf eine Abweichung nicht ueberall dasselbe bedeuten:

    Kanon (ohne Flag)   "welcher Bau liegt gerade da?"
                        Eine Abweichung ist ein HINWEIS. Ein Relink aendert
                        die Bundlebytes auch ohne Quelltextaenderung - seit
                        A14 den Kern vor jeder Messung neu baut, ist das der
                        Normalfall. Rot waere hier eine Dauerwarnung, die
                        nichts mehr unterscheidet.

    --release           "frieren wir genau diesen Stand ein?"
                        HART. Abweichung, fehlende Hashes oder ein fehlendes
                        Artefakt = Exit 2. Das ist der Auslieferungsschritt;
                        wer hier gruen ist, darf `--hashen` und danach
                        installieren.

    Install-Nakama.ps1  "darf das kopiert werden?"
                        HART und unveraendert (Riegel 2 'Echtheit'): jede
                        Quelle wird gegen den Manifest-Hash geprueft, bevor
                        irgendetwas das Zielverzeichnis anfasst.

  Was in KEINEM Modus weich wird: ein fehlendes Artefakt und ein nicht
  bildbarer Ordner-Hash. Beides kann ein Relink nicht verursachen - dort bleibt
  der Riegel fail-closed (Pruefliste D). Seit der Nacharbeit 1 (29.08.2026,
  Befund C1) gilt das UNABHAENGIG von `sha256`: die Existenzfrage wird fuer
  jedes Artefakt gestellt, verglichen wird nur, wo ein Hash festgeschrieben
  ist. Vorher beendete ein einziges `sha256: null` den ganzen Artefaktcheck
  mit `return` - ein umbenanntes Bundle blieb im Kanon gruen.

  [4b] berichtet zusaetzlich, ob der INSTALLIERTE Stand (install-ergebnis.json)
  dem heutigen Manifest entspricht. Dieser Block urteilt nie und BRICHT NIE AB
  (Befund C2): eine unbrauchbare Kennung im Journal oder im Manifest wird zum
  Hinweis, nicht zum TypeError. Installieren ist ein bewusster Admin-Handgriff
  des Users und keine Zusage des Kanons.

  [3b] laesst beide Kanten einmal fallen - eine Wache, die niemand hat fallen
  sehen, ist keine.

Aufrufe:
  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py            # Kanon
  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --release  # Auslieferung
  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --hashen   # festschreiben
"""

from __future__ import annotations

import argparse
import contextlib
import copy
import hashlib
import io
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
INSTALL_ERGEBNIS = WURZEL / "eq-copilot" / "install" / "install-ergebnis.json"
IDENTITAET = WURZEL / "eq-copilot" / "identity" / "plugin-identities-v1.json"
BROKER_CARGO = WURZEL / "broker" / "Cargo.toml"
PS_ORDNERHASH = WURZEL / "eq-copilot" / "install" / "NakamaOrdnerHash.ps1"
STATE_CPP = WURZEL / "eq-copilot" / "plugin" / "state" / "NakamaState.cpp"

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


def _probelauf(arbeit) -> tuple[str, list[str]]:
    """Laesst einen Riegel PROBEWEISE laufen. (Ausgabe, Klagen des Probelaufs)

    Ausgabe und Klagen werden abgefangen; Zaehler und Urteil des echten Laufs
    bleiben unberuehrt. Eine Ausnahme aus `arbeit` wird NICHT geschluckt - eine
    Gegenprobe, die selbst stirbt, darf nicht als bestanden durchgehen.
    """
    global ok
    merk_ok, merk_fehler = ok, list(fehler)
    fehler.clear()
    puffer = io.StringIO()
    try:
        with contextlib.redirect_stdout(puffer):
            arbeit()
        gefunden = list(fehler)
    finally:
        fehler.clear()
        fehler.extend(merk_fehler)
        ok = merk_ok
    return puffer.getvalue(), gefunden


def gegenproben_nacharbeit(manifest: dict) -> None:
    """[3b] Die beiden Kanten aus NAK-94 Nacharbeit 1, einzeln gebrochen.

    Beide Befunde waren stille Ausfaelle: C1 liess ein fehlendes Bundle im
    Kanon gruen, C2 toetete den Lauf mit einem TypeError. Eine Wache, die
    niemand hat fallen sehen, ist keine - also faellt sie hier.
    """
    global INSTALL_ERGEBNIS
    print("\n[3b] Gegenproben zu [4] Auslieferungsstand und [4b] installiertem Stand")

    # -- C1: ein `sha256: null` darf den Artefaktcheck nicht beenden --------
    #
    # Die Probe braucht ZWEI Artefakte: eines ohne Hash und ein anderes, das
    # fehlt. Faellt das Manifest je auf eines zusammen, misst sie nichts mehr -
    # und eine stillschweigend ausgelassene Gegenprobe ist schlimmer als keine
    # (dieselbe Regel wie in adversariale_strukturproben).
    if len(manifest["artefakte"]) < 2:
        raise SystemExit(
            "Gegenprobe unmoeglich: die C1-Probe braucht ZWEI Artefakte - eines "
            "ohne festgeschriebenen Hash und ein anderes, das fehlt. Mit nur "
            "einem laesst sich nicht zeigen, dass der Artefaktcheck WEITERLAEUFT."
        )
    probe = copy.deepcopy(manifest)
    probe["artefakte"][0]["sha256"] = None
    fehlt = probe["artefakte"][-1]
    fehlt["quelle"] = fehlt["quelle"] + "-GIBT-ES-NICHT"
    ausgabe, klagen = _probelauf(lambda: auslieferungsstand(probe, hart=False))
    pruefe(any("liegt nicht vor" in k for k in klagen),
           "C1: ein fehlendes Artefakt ist auch im Kanon ROT, wenn ein anderes "
           "keinen festgeschriebenen Hash traegt",
           " | ".join(klagen[:2]) if klagen else "keine Klage")
    pruefe("Ordner-Hash bildbar" in ausgabe,
           "C1: das Artefakt ohne Hash wird trotzdem gemessen (liegt vor, "
           "Ordner-Hash bildbar) statt uebersprungen",
           next((z.strip() for z in ausgabe.splitlines()
                 if "Ordner-Hash bildbar" in z), "keine solche Zeile"))
    _, hart_klagen = _probelauf(lambda: auslieferungsstand(
        copy.deepcopy(probe), hart=True))
    pruefe(any("liegt nicht vor" in k for k in hart_klagen)
           and any("ohne Hash" in k for k in hart_klagen),
           "C1: unter --release sind BEIDE Befunde Fehler - der fehlende Hash "
           "und das fehlende Artefakt",
           " | ".join(hart_klagen[:2]) if hart_klagen else "keine Klage")

    # -- C2: [4b] bricht nie ab ---------------------------------------------
    with tempfile.TemporaryDirectory(prefix="nakama-journal-") as tmp:
        journal = pathlib.Path(tmp) / "install-ergebnis.json"
        merk = INSTALL_ERGEBNIS
        INSTALL_ERGEBNIS = journal
        try:
            # (a) unbrauchbare Kennung IM JOURNAL - Hinweis, kein Absturz.
            journal.write_text(json.dumps({
                "schema": ERGEBNIS_SCHEMA, "status": "OK",
                "zeit": "2026-08-29T00:00:00Z",
                "eintraege": [{"ziel_id": ["main"]}, "keine Abbildung"],
            }), encoding="utf-8")
            ausgabe, klagen = _probelauf(lambda: installierter_stand(manifest))
            pruefe("ohne lesbare Kennung" in ausgabe
                   and "kein Objekt" in ausgabe and not klagen,
                   "C2: ein Journaleintrag mit ziel_id als Liste ist ein Hinweis, "
                   "kein TypeError - und [4b] faellt kein Urteil",
                   " / ".join(z.strip() for z in ausgabe.splitlines()
                              if "hinweis 0" in z or "hinweis 1" in z))

            # (b) unbrauchbare Kennung IM MANIFEST - die Huelle faengt sie.
            kaputt = copy.deepcopy(manifest)
            kaputt["artefakte"][0]["ziel_id"] = ["main"]
            journal.write_text(json.dumps({
                "schema": ERGEBNIS_SCHEMA, "status": "OK",
                "zeit": "2026-08-29T00:00:00Z",
                "eintraege": [{"ziel_id": "main", "mutation_abgeschlossen": True,
                               "sha256": "0" * 64, "ziel": "irgendwo"}],
            }), encoding="utf-8")
            ausgabe, klagen = _probelauf(lambda: installierter_stand(kaputt))
            pruefe("nicht auswertbar" in ausgabe and not klagen,
                   "C2: auch ein Fehler auf der MANIFEST-Seite bleibt ein "
                   "Hinweis - [4b] toetet keinen Kanonlauf",
                   next((z.strip() for z in ausgabe.splitlines()
                         if "nicht auswertbar" in z), "keine solche Zeile"))

            # -- P2 (Nacharbeit 2): ok NUR bei Journalstatus OK -------------
            #
            # Derselbe Eintrag, nur ein anderer Journalkopf. Genau das war der
            # Befund: der Hash stimmte, das Ziel aber konnte halb
            # wiederhergestellt sein.
            name = _artefakt_name(manifest["artefakte"][0])
            eintrag = {
                "ziel_id": manifest["artefakte"][0].get("ziel_id"),
                "quelle": manifest["artefakte"][0].get("quelle"),
                "mutation_begonnen": True, "mutation_abgeschlossen": True,
                "rollback_abgeschlossen": False,
                "sha256": "0" * 64, "ziel": "irgendwo",
            }
            mit_hash = copy.deepcopy(manifest)
            mit_hash["artefakte"] = [copy.deepcopy(manifest["artefakte"][0])]
            mit_hash["artefakte"][0]["sha256"] = "0" * 64

            def journal_lauf(kopf: dict) -> str:
                journal.write_text(json.dumps(kopf), encoding="utf-8")
                text, offen = _probelauf(lambda: installierter_stand(mit_hash))
                return text if not offen else text + " KLAGEN " + " | ".join(offen)

            grund = {"schema": ERGEBNIS_SCHEMA, "zeit": "2026-08-29T00:00:00Z",
                     "eintraege": [eintrag]}
            ausgabe = journal_lauf({**grund, "status": ERGEBNIS_STATUS_OK})
            pruefe(f"ok      {name}: installierter Stand = Manifest" in ausgabe,
                   "P2: bei Journalstatus OK und abgeschlossenem, nicht "
                   "zurueckgerolltem Eintrag bleibt der Hashvergleich und sein ok",
                   next((z.strip() for z in ausgabe.splitlines()
                         if name in z), "keine Zeile zum Artefakt"))

            # Jeder andere Statuswert aus Install-Nakama.ps1, dazu ein
            # unbekannter und ein fehlender - abgelesen, nicht geraten.
            for status in ("VORBEREITET", "KOMPENSATION", "ERROR_TEILSTAND",
                           "ERROR_RUECKGEROLLT", "RUECKWEG_AKTIV", "RUECKWEG",
                           "NEUER_STATUS_2099"):
                ausgabe = journal_lauf({**grund, "status": status})
                pruefe(f"hinweis {name}: installierter Stand unbekannt "
                       f"(Journalstatus {status})" in ausgabe
                       and "installierter Stand = Manifest" not in ausgabe,
                       f"P2: Journalstatus {status} meldet den installierten "
                       f"Stand als unbekannt - ohne Hashvergleich",
                       next((z.strip() for z in ausgabe.splitlines()
                             if name in z), "keine Zeile zum Artefakt"))

            ausgabe = journal_lauf(grund)
            pruefe(f"hinweis {name}: installierter Stand unbekannt "
                   "(Journalstatus fehlt)" in ausgabe
                   and "installierter Stand = Manifest" not in ausgabe,
                   "P2: ein Journal OHNE status meldet den installierten Stand "
                   "als unbekannt - Schweigen ist kein OK",
                   next((z.strip() for z in ausgabe.splitlines()
                         if name in z), "keine Zeile zum Artefakt"))
        finally:
            INSTALL_ERGEBNIS = merk


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
    except (OSError, json.JSONDecodeError) as e:
        print(f"  hinweis install-ergebnis.json nicht lesbar: {e}")
        return
    if not isinstance(journal, dict) or journal.get("schema") != ERGEBNIS_SCHEMA:
        print(f"  hinweis install-ergebnis.json traegt nicht {ERGEBNIS_SCHEMA} "
              f"(gelesen: {journal.get('schema') if isinstance(journal, dict) else type(journal).__name__!r})")
        return

    eintraege = journal.get("eintraege")
    if not isinstance(eintraege, list):
        print("  hinweis install-ergebnis.json fuehrt keine Liste 'eintraege'")
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
    status = journal.get("status")
    if status != ERGEBNIS_STATUS_OK:
        wie = status if isinstance(status, str) else (
            "fehlt" if status is None else repr(status))
        for a in manifest["artefakte"]:
            print(f"  hinweis {_artefakt_name(a)}: installierter Stand unbekannt "
                  f"(Journalstatus {wie})")
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


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    g = p.add_mutually_exclusive_group()
    g.add_argument("--hashen", action="store_true",
                   help="Release-Schritt: sha256 aus den gebauten Artefakten festschreiben")
    g.add_argument("--release", action="store_true",
                   help="Auslieferungsschritt: [4] vergleicht HART gegen die festgeschriebenen "
                        "Hashes (Exit 2 bei Abweichung). Ohne dieses Flag ist eine Abweichung "
                        "ein Hinweis - nach einem Relink ist sie der Normalfall (NAK-94).")
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
    gegenproben_nacharbeit(manifest)

    auslieferungsstand(manifest, hart=args.release)
    installierter_stand(manifest)

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
