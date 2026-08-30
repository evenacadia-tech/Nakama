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

  Die Reihenfolge in [4b] ist STATUS VOR LISTE (Nacharbeit 3, 29.08.2026):
  Schema, dann Journalstatus, dann erst `eintraege`. Ein regulaer
  abgeschlossener Rueckweg schreibt gar keine Eintragsliste; stuende die
  Statussperre dahinter, meldete [4b] fuer ihn "fuehrt keine Liste" statt des
  artefaktweisen "installierter Stand unbekannt (Journalstatus RUECKWEG)".
  Die Liste wird deshalb nur noch im Status-OK-Pfad verlangt.

  [3b] ist nach ZUSAGEN gegliedert (Z1..Z7) und laesst JE ZUSAGE genau EINEN
  eigenen Bruch fallen - eine Wache, die niemand hat fallen sehen, ist keine.
  Eine Zusage, die ueber mehrere Werte parametrisiert ist (das OK-Urteil ueber
  jede OK-Fixtur, die Statussperre ueber jeden Nicht-OK-Status), ist EINE
  Zusage mit EINEM Bruch; ihre Werte zaehlt die Ausgabe, nicht dieser Text.
  Die Brueche B6-Z1..B6-Z7 stehen mit ROT und Ruecknahme in
  docs/beweise/SONDE-007c.md (NAK-94 Nacharbeit 6, 30.08.2026).

  Z1 BRICHT AN EINEM GEAENDERTEN BYTE; EINE FEHLENDE PFLICHTDATEI BRICHT DEN
  BLOCK AB (NAK-94 Nacharbeit 7, 30.08.2026). Beides ist ROT, aber es ist
  nicht dasselbe: liegt die Datei vor, ist lesbar und traegt die von Z2..Z7
  gelesene Writer-Struktur, weicht nur ihr SHA-256 ab, so faerbt das genau
  EINE Zeile - Z1 - und Z2..Z7 laufen auf dem vollstaendigen Korpus gruen
  weiter. Fehlt die Datei, ist sie unlesbar oder kein Journalobjekt, liegt
  eine verwaiste daneben oder fehlt eine Statusklasse, dann haelt [3b] an
  (fail-closed), weil die uebrigen Zusagen nichts Vollstaendiges mehr messen
  koennten. Belegt als B7-Z1 (Byte, zwei Stufen) und als eigene Probe
  "Pflichtmenge" (Umbenennen -> Abbruch -> Ruecknahme) in
  docs/beweise/SONDE-007c.md.

  NACH ROTEM Z1 LAUFEN Z2..Z7 NUR AUF EINEM STRUKTURELL GUELTIGEN OBJEKT
  WEITER; SONST BRICHT [3b] KONTROLLIERT AB (NAK-94 Nacharbeit 8,
  30.08.2026, Befund des achten Pruefers). Eine einzelne Byteaenderung kann
  die STRUKTUR treffen: wird `eintraege` zu `xntraege`, bleibt der Kopf ein
  JSON-Objekt, und Z3 stuerbe unten an `k["eintraege"][0]` mit einem
  KeyError. _writer_struktur() prueft deshalb VOR jeder Benutzung, ob der
  Kopf die von Z2..Z7 GELESENE Form traegt - `schema`, `status` und
  `transaktions_id` fuer jeden Fall, dazu bei MANIFEST-Status OK eine nicht
  leere Liste `eintraege` aus Objekten mit Kennung (ziel_id oder name) und
  den Feldern aus JOURNAL_EINTRAGSFELDER. Fehlt etwas, endet [3b] mit
  Klartext ("Fixtur geaendert und strukturell unbrauchbar") und Exit
  ungleich 0, NIE mit einem Traceback: Unbekanntes ist ROT, nicht laut
  (Pruefliste D). Geprueft werden SCHLUESSEL und Grobform, nie Werte - ueber
  die urteilen weiter Z4 und Z5. Belegt als B8-Z1 in drei Stufen (a
  semantikneutrale Byteaenderung in `zeit` -> Z1 rot, Z2..Z7 gruen; b
  `eintraege` -> `xntraege` -> Abbruch; c `schema` in der RUECKWEG-Fixtur ->
  Abbruch), jede zurueckgenommen, in docs/beweise/SONDE-007c.md.

  JEDE GELESENE JSON-DATEI WIRD VOR DEM ZUGRIFF STRUKTURELL GEPRUEFT; VERSTOSS
  = KONTROLLIERTER ABBRUCH (NAK-94 Nacharbeit 9, 30.08.2026, Befund des
  neunten Pruefers). Die Pruefung aus Nacharbeit 8 galt nur den JOURNALEN -
  das Korpusmanifest selbst ging ungeprueft in `korpus["faelle"]`, und eine
  einzelne Byteaenderung ("faelle" -> "xaelle") toetete den Lauf mit
  `KeyError: 'faelle'`, bevor Z1 urteilen konnte. Dieselbe Tuer stand an den
  beiden Dateien offen, die main() liest ("artefakte" -> "xrtefakte" starb in
  r_art_bekannt, "ziele" -> "xiele" in _ziele()). Jetzt hat JEDE gelesene
  JSON-Datei ihren Strukturvertrag, geprueft VOR dem ersten Zugriff - die
  ersten drei ueber _lies_geprueft(), die beiden anderen an ihrer Lesestelle:

    nakama-installer-v1.json   _installermanifest_struktur - Wurzel Objekt,
                               `artefakte` nicht leere Liste von Objekten mit
                               Zeichenkette `quelle`, `ziele` Objekt,
                               `rueckweg.bekannte_staende` Liste
    plugin-identities-v1.json  _identitaet_struktur - Wurzel Objekt, `ziele`
                               nicht leere Liste von Objekten mit `id`, je
                               AKTIVEM Ziel `produktname` und `bundle`,
                               `hersteller.code` Zeichenkette
    journale/MANIFEST.json     _journalkorpus_struktur - Wurzel Objekt,
                               `stand` Zeichenkette, `faelle` nicht leere
                               Liste; je Fall `datei`, `status`, `fall` und
                               `befehl` als nicht leere Zeichenketten und
                               `sha256` als HEX64 in Grossbuchstaben
    journale/<fall>.json       _writer_struktur (Nacharbeit 8, oben)
    install-ergebnis.json      Schema- und Statussperre in [4b]

  Verstoss = kontrollierter Abbruch mit Klartext und Exit ungleich 0, NIE ein
  Traceback. Die einzige Ausnahme ist [4b]: dort ist derselbe Verstoss ein
  kontrollierter HINWEIS mit Rueckkehr, weil dieser Block per Zusage nie
  urteilt und nie abbricht (Befund C2). Geprueft werden auch hier SCHLUESSEL
  und Grobform, nie Werte - ein GEAENDERTER sha256-Wert im Korpusmanifest
  laesst die Struktur gueltig und faerbt weiterhin genau Z1, waehrend Z2..Z7
  gruen weiterlaufen. Belegt als P9-a..P9-f in docs/beweise/SONDE-007c.md,
  jede Probe zurueckgenommen.

  WEGWECHSEL W3 (NAK-94 Nacharbeit 10, 30.08.2026, Befund des zehnten
  Pruefers). Drei Runden lang hat je ein Pruefer EIN weiteres Feld gefunden,
  dessen Byteaenderung einen Traceback statt eines Strukturhalts ergab -
  zuletzt `ziel_id` (KeyError in [3]) und ein ungueltiges UTF-8-Byte
  (UnicodeDecodeError in `_lies_geprueft`, ein ValueError, den der OSError-
  Zweig nicht fing). Feld fuer Feld nachzuziehen schliesst die Klasse nicht;
  sie wird deshalb STRUKTURELL geschlossen und GEMESSEN:

    Erste Verteidigung  die Strukturvertraege oben. Sie sagen im Klartext,
                        WELCHE Datei welche Form nicht traegt. Neu am
                        Installer-Manifest: `ziel_id` als Zeichenkette an
                        jedem VST3-Artefakt (adversariale_strukturproben()
                        liest es hart).
    Zentraler Faenger   `_geschuetzt()` ueberfuehrt JEDE Ausnahme, die kein
                        Strukturhalt ist, in eine Klartextzeile mit Typ,
                        Meldung, Datei und Zeile des Ausloesers, Exit 2 und
                        ohne Traceback (nur mit --debug). Der eigene
                        SystemExit "Gegenprobe unmoeglich" laeuft absichtlich
                        durch - er ist schon kontrolliert.
                        EINMAL geschrieben, ZWEIMAL benutzt (Nacharbeit 11):
                        dieselbe Funktion legt main() um den ganzen Lauf und
                        [3c] um jeden einzelnen Fuzz-Fall. Der Fuzz hat keine
                        zweite Ausnahmebehandlung mehr - wer die Funktion
                        entwaffnet, macht damit den Fuzz rot.
    Messung             [3c] Byte-Kipp-Fuzz: jede gelesene JSON-Datei, jedes
                        Byte einzeln auf 0xFF und 0x20, in-process durch
                        Lesen, Strukturpruefung und jeden verbrauchenden
                        Block - seit Nacharbeit 11 auch den mutierenden
                        Zweig --hashen, mit Schreibziel unter %TEMP% statt im
                        Repo; [3c/0] kommt damit ohne Handausnahme aus und
                        [3c/1] haelt den sha256 des Manifests im Repo vor und
                        nach dem Fuzz dagegen. Zugesagt und gezaehlt: keine
                        Ausnahme ausser Strukturhalt und "Gegenprobe
                        unmoeglich". Im Kanon laeuft ein deterministisches
                        Sample (jedes n-te Byte, n in der Ausgabe),
                        vollstaendig mit --fuzz-voll; der vollstaendige Lauf
                        liegt als eigene Roh-Datei unter docs/beweise/roh/.

  ZWEI SORTEN PROBE-JOURNALE, seit NAK-94 Nacharbeit 5 (30.08.2026):

    Writer-Fixtur   von Install-Nakama.ps1 SELBST in der A18-Sandbox erzeugt
                    und byteweise eingefroren
                    (eq-copilot/fixtures/installer/journale/, MANIFEST.json
                    mit Fall, Stand, Befehl und SHA-256; Erzeuger
                    tools/eq-copilot/erzeuge_installer_journale.py). [3b]
                    rechnet die Hashes vor der Benutzung nach - eine von Hand
                    angefasste Fixtur ist ROT, nicht stillschweigend anders.
                    PFLICHT ist JEDER im MANIFEST gefuehrte Fall; daneben muss
                    die Statusachse JOURNAL_PFLICHTSTATUS vertreten sein, die
                    ausserhalb des Korpus steht und sich deshalb nicht mit ihm
                    loeschen laesst. Welche Fixtur welche Achse traegt, liest
                    [3b] aus dem MANIFEST - eine fuenfte faehrt automatisch
                    mit.

    Mutant          im Skript aus GENAU EINER Writer-Fixtur abgeleitet, mit
                    benannter Abweichung (`mutant_von`, `abweichung`). Fuer
                    Staende, die ein abgeschlossener Lauf nicht hinterlaesst
                    (VORBEREITET, KOMPENSATION, ERROR_TEILSTAND,
                    RUECKWEG_AKTIV), fuer erfundene und fehlende Statuswerte
                    und fuer verstuemmelte Journale.

  Jede Probe nennt ihre Sorte in der Ausgabezeile. Von Hand geschriebene
  "Writer-Formen" gibt es nicht mehr: drei Pruefrunden hintereinander fanden
  daran je ein weiteres abweichendes Feld (`quelle`, gestrichelte
  Transaktions-ID, `vorher_sha256_innen` null bei gesetztem Vorzustand).

Aufrufe:
  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py            # Kanon
  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --release  # Auslieferung
  py -3.13 tools/eq-copilot/pruefe_installer_manifest.py --hashen   # festschreiben
"""

from __future__ import annotations

import argparse
import ast
import contextlib
import copy
import hashlib
import io
import json
import ntpath
import os
import pathlib
import re
import shutil
import subprocess
import sys
import tempfile
import time
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

SCHEMA = "nakama.installer/v1"
ERGEBNIS_SCHEMA = "nakama.install-ergebnis/v1"
# Der EINZIGE Journalstatus, unter dem der gespeicherte sha256 den Stand
# beschreibt, der wirklich liegt. Install-Nakama.ps1 kennt daneben
# VORBEREITET, KOMPENSATION, ERROR_TEILSTAND, ERROR_RUECKGEROLLT,
# RUECKWEG_AKTIV und RUECKWEG - jeder davon steht fuer ein Ziel, das bereits
# ganz oder halb wiederhergestellt sein kann (NAK-94 Nacharbeit 2).
ERGEBNIS_STATUS_OK = "OK"
# Die Journal-Fixturen der Gegenproben [3b] werden nicht mehr nachgeschrieben,
# sondern vom WRITER erzeugt und byteweise eingefroren (NAK-94 Nacharbeit 5,
# 30.08.2026 - Wegwechsel W1 des Dirigenten). Erzeuger und Sandbox:
# tools/eq-copilot/erzeuge_installer_journale.py.
#
# Die drei Runden davor sind der Grund: eine handgeschriebene "Writer-Form"
# fiel jedes Mal an genau einem weiteren Feld auf, weil Install-Nakama.ps1
# mehr aneinander gebundene Invarianten traegt, als ein Leser aufzaehlen kann
# (`quelle` gibt es dort nicht; die Transaktions-ID hat keine Bindestriche;
# `vorher_sha256_innen` darf bei gesetztem `vorher_sha256` und art=vst3 nicht
# null sein - Install-Nakama.ps1 bricht sonst ab, bevor es schreibt).
JOURNAL_FIXTUREN = (WURZEL / "eq-copilot" / "fixtures" / "installer" / "journale")
JOURNAL_FIXTUR_MANIFEST = JOURNAL_FIXTUREN / "MANIFEST.json"
# Pflichtmenge sind ALLE Faelle aus MANIFEST.json - keine handgepflegte
# Teilmenge mehr. Fehlt einer, bricht [3b] laut ab, statt still weniger zu
# pruefen (Befund NAK-94, Pruefer 6, 30.08.2026: die alte Namensliste nannte
# drei von vier Dateien, und die vierte Probe uebersprang sich selbst).
#
# Ein zweiter Anker haelt den Fall, dass eine Fixtur ZUSAMMEN mit ihrer
# MANIFEST-Zeile verschwindet: die Statusachse, ueber die [3b] etwas
# behauptet. Sie steht hier - ausserhalb des Korpus -, damit ein Loeschen im
# Korpus sie nicht mitnimmt. Wer eine Statusklasse aus dem Korpus nimmt, muss
# die Zusage hier mit derselben Hand streichen.
JOURNAL_PFLICHTSTATUS = ("OK", "RUECKWEG", "ERROR_RUECKGEROLLT")

# Die Felder, die Z2..Z7 aus einem OK-Journaleintrag LESEN - nicht mehr und
# nicht weniger (NAK-94 Nacharbeit 8, 30.08.2026). Die Strukturpruefung
# _writer_struktur() verlangt sie als SCHLUESSEL; ueber ihre WERTE urteilen
# weiter Z4/Z5, nicht die Pruefung. Die Kennung steht nicht in der Liste,
# weil _artefakt_name() zwei Namen gelten laesst (ziel_id ODER name).
JOURNAL_EINTRAGSFELDER = ("sha256", "mutation_abgeschlossen",
                          "rollback_abgeschlossen")
HEX64 = re.compile(r"^[0-9A-F]{64}$")
THUMBPRINT = re.compile(r"^(?:[0-9A-F]{40}|[0-9A-F]{64})$")
ARTEN = ("vst3", "broker")
CMAKE_ZIEL = re.compile(r"^[A-Za-z][A-Za-z0-9_]*$")
# Die Felder, die [3b] aus einem MANIFEST-Fall des Writer-Korpus LIEST oder als
# Herkunft benennt: `datei` (Dateiname), `status` (Achse OK/RUECKWEG/...),
# `fall` und `befehl` (Herkunft - welcher Lauf, welcher Installer-Befehl). Der
# vierte gelesene Wert, `sha256`, steht getrennt, weil er zusaetzlich HEX64 in
# Grossbuchstaben sein muss (NAK-94 Nacharbeit 9, 30.08.2026).
JOURNAL_FALLFELDER = ("datei", "status", "fall", "befehl")


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


def _journalkorpus_struktur(korpus: object) -> list[str]:
    """Traegt MANIFEST.json des Writer-Korpus die von [3b] GELESENE Form?

    Die Pruefung, die der neunte Pruefer verlangt hat (NAK-94 Nacharbeit 9):
    Wurzel ist ein Objekt, `faelle` eine nicht leere Liste, und JEDER Fall
    traegt die gelesenen Felder mit dem richtigen Typ - `datei`, `status`,
    `fall` und `befehl` als nicht leere Zeichenketten, `sha256` als HEX64 in
    Grossbuchstaben (genau die Form, gegen die `_writer_fixturen` die
    Fixturbytes nachrechnet). Der Herkunftsstand `stand` steht ebenfalls hier,
    weil die A17-Behauptung ihn als Bestandteil dieses Korpusmanifests nennt.

    Ueber die WERTE urteilt weiter Z1 (Bytegleichheit, Vollstaendigkeit,
    Statusachse) - diese Pruefung sagt nur, ob Z1 ueberhaupt urteilen kann."""
    if not isinstance(korpus, dict):
        return [f"Wurzel ist kein Objekt ({type(korpus).__name__})"]
    fehlt: list[str] = []
    if not isinstance(korpus.get("stand"), str) or not korpus["stand"].strip():
        fehlt.append("'stand' ist keine nicht leere Zeichenkette "
                     f"({type(korpus.get('stand')).__name__})")
    faelle = korpus.get("faelle")
    if not isinstance(faelle, list) or not faelle:
        fehlt.append(f"keine nicht leere Liste 'faelle' ({type(faelle).__name__})")
        return fehlt
    for index, fall in enumerate(faelle):
        if not isinstance(fall, dict):
            fehlt.append(f"faelle[{index}] ist kein Objekt "
                         f"({type(fall).__name__})")
            continue
        for feld in JOURNAL_FALLFELDER:
            wert = fall.get(feld)
            if not isinstance(wert, str) or not wert.strip():
                fehlt.append(f"faelle[{index}] ohne nicht leere Zeichenkette "
                             f"{feld!r} ({type(wert).__name__})")
        sha = fall.get("sha256")
        if not isinstance(sha, str) or not HEX64.match(sha):
            fehlt.append(f"faelle[{index}] ohne SHA-256 in Grossbuchstaben "
                         f"'sha256' ({sha!r})")
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


def _writer_struktur(kopf: dict, fall: dict) -> list[str]:
    """Traegt dieser Journalkopf die von Z2..Z7 GELESENE Writer-Struktur?

    BEFUND NAK-94, Pruefer 8 (30.08.2026): seit Nacharbeit 7 laeuft [3b] nach
    einem geaenderten BYTE weiter, weil die Datei ja vorliegt und ein
    JSON-Objekt bleibt. Eine einzelne Byteaenderung kann aber die STRUKTUR
    treffen - wird `eintraege` zu `xntraege`, ist der Kopf weiterhin ein
    Objekt, und Z3 stirbt unten an `k["eintraege"][0]` mit einem KeyError.
    Ein Traceback ist kein Urteil: Unbekanntes ist ROT, nie laut
    (Pruefliste D). Diese Pruefung steht deshalb VOR jeder Benutzung.

    Geprueft werden SCHLUESSEL und Grobform, nicht Werte - ueber die Werte
    urteilen Z4 (Hashvergleich) und Z5 (Statussperre), und eine Pruefung, die
    ihnen vorgreift, verschluckte genau den Bruch, den sie belegen sollen.

    Der Schnitt laeuft am MANIFEST-Status des Falls entlang, nicht am Status
    IM Kopf: der MANIFEST-Status entscheidet, ob dieser Fall unten in Z4 (OK)
    oder in Z5 (alles andere) landet.

      JEDER Fall   `schema` ist ERGEBNIS_SCHEMA, `status` und
                   `transaktions_id` sind Zeichenketten - _installierter_stand()
                   haengt an den ersten beiden, Z7 an der dritten.
      NUR OK       zusaetzlich `eintraege` als NICHT LEERE Liste von Objekten,
                   jedes mit einer Kennung (ziel_id oder name) und den Feldern
                   aus JOURNAL_EINTRAGSFELDER. Fuer einen Nicht-OK-Fall wird
                   sie NICHT verlangt: die Statussperre steht vor der Liste,
                   und der regulaer abgeschlossene Rueckweg schreibt gar keine
                   (Nacharbeit 3).
    """
    fehlt: list[str] = []
    if kopf.get("schema") != ERGEBNIS_SCHEMA:
        fehlt.append(f"schema ist {kopf.get('schema')!r} statt "
                     f"{ERGEBNIS_SCHEMA!r}")
    for feld in ("status", "transaktions_id"):
        if not isinstance(kopf.get(feld), str):
            fehlt.append(f"{feld} ist keine Zeichenkette "
                         f"({type(kopf.get(feld)).__name__})")
    if fall.get("status") != ERGEBNIS_STATUS_OK:
        return fehlt
    eintraege = kopf.get("eintraege")
    if not isinstance(eintraege, list) or not eintraege:
        fehlt.append("keine nicht leere Liste 'eintraege' "
                     f"({type(eintraege).__name__})")
        return fehlt
    for index, e in enumerate(eintraege):
        if not isinstance(e, dict):
            fehlt.append(f"eintraege[{index}] ist kein Objekt "
                         f"({type(e).__name__})")
            continue
        ohne = [f for f in JOURNAL_EINTRAGSFELDER if f not in e]
        if "ziel_id" not in e and "name" not in e:
            ohne.insert(0, "ziel_id/name")
        if ohne:
            fehlt.append(f"eintraege[{index}] ohne {', '.join(ohne)}")
    return fehlt


def _writer_fixturen() -> tuple[dict[str, tuple[bytes, dict, dict]],
                               list[str], list[str]]:
    """[3b] Z1: die eingefrorenen Writer-Journale laden - fail-closed.

    Gibt `(korpus, klagen, abbruch)` zurueck; `korpus` bildet je Fall auf
    `(rohe Bytes, gelesener Kopf, MANIFEST-Eintrag)` ab. JEDE Klage macht Z1
    ROT. `abbruch` ist die Teilmenge der Klagen, nach denen [3b] die
    Writer-Form ueberhaupt nicht mehr messen kann; der Aufrufer haelt dann den
    ganzen Block an - eine stillschweigend ausgelassene Gegenprobe ist
    schlimmer als keine (dieselbe Regel wie in adversariale_strukturproben).

    Der Schnitt zwischen beiden (NAK-94 Nacharbeit 7, 30.08.2026, Befund des
    siebten Pruefers):

      ABBRUCH  die Pflichtdatei FEHLT; sie liegt vor, ist aber nicht lesbar
               oder kein Objekt; eine verwaiste Datei liegt daneben; eine
               Statusklasse aus JOURNAL_PFLICHTSTATUS ist nicht mehr
               vertreten; oder das MANIFEST selbst fehlt. Dann ist der Korpus
               unvollstaendig oder unbestimmt, und Z2..Z7 haetten nichts
               Vollstaendiges zu messen. Seit NAK-94 Nacharbeit 8
               (30.08.2026) gehoert dazu auch: die Datei liegt vor und ist
               ein Objekt, traegt aber NICHT die von Z2..Z7 gelesene
               Writer-Struktur (_writer_struktur). Eine einzelne
               Byteaenderung kann `eintraege` zu `xntraege` machen - dann
               waere Z3 unten ein KeyError statt eines Urteils.

      NUR ROT  die Datei LIEGT VOR, ist lesbar UND traegt die Writer-Struktur,
               ihr SHA-256 weicht aber vom MANIFEST ab: sie ist keine
               eingefrorene Writer-Form mehr. Das
               sagt Z1 - und nur Z1. Der Korpus ist vollstaendig, also laufen
               Z2..Z7 auf ihm weiter und bleiben gruen. Genau das macht den
               Bruch unterscheidbar: EIN semantikneutral geaendertes Byte
               faerbt eine einzige Zusagenzeile.

    Pflicht ist JEDER in MANIFEST.json gefuehrte Fall; die Bytes werden vor
    der Benutzung gegen den dort festgeschriebenen SHA-256 nachgerechnet, und
    eine verwaiste Datei im Korpusverzeichnis ist derselbe Befund wie eine
    fehlende. Dazu muss die Statusachse JOURNAL_PFLICHTSTATUS vertreten sein.

    BEFUND NAK-94, Pruefer 6 (30.08.2026): bis hierher war die Pflichtmenge
    ein Namenstupel mit DREI der vier Dateien, und die vierte Probe uebersprang
    sich weiter unten ausdruecklich selbst. Fiel `error-rueckgerollt.json`
    samt MANIFEST-Zeile weg, blieben A17 und
    `erzeuge_installer_journale.py --pruefen` gruen - am Stand `165d9ae`
    gemessen als 113 statt 114 ok, ohne eine einzige Zeile darueber. Zwei
    Anker schliessen das: die Pflichtmenge IST jetzt das MANIFEST, und die
    Statusachse liegt ausserhalb des Korpus.
    """
    klagen: list[str] = []
    abbruch: list[str] = []

    def haltend(klage: str) -> None:
        """Eine Klage, nach der [3b] nichts Vollstaendiges mehr messen kann."""
        klagen.append(klage)
        abbruch.append(klage)

    if not JOURNAL_FIXTUR_MANIFEST.is_file():
        fehlt_manifest = (
            "der Writer-Journalkorpus fehlt "
            f"({JOURNAL_FIXTUR_MANIFEST.relative_to(WURZEL).as_posix()}) - "
            "erzeugen mit: py -3.13 tools/eq-copilot/erzeuge_installer_journale.py")
        return {}, [fehlt_manifest], [fehlt_manifest]
    # Vor dem ersten Zugriff auf `faelle`: traegt das Korpusmanifest ueberhaupt
    # die Form, die [3b] liest? Am Basis-Stand b8dcbe1 machte eine einzelne
    # Byteaenderung ("faelle" -> "xaelle") daraus ein `KeyError: 'faelle'` in
    # der naechsten Zeile - ein Traceback statt eines Urteils (NAK-94
    # Nacharbeit 9, Befund des neunten Pruefers). Der Verstoss geht jetzt
    # denselben Weg wie ein fehlendes MANIFEST: Z1 rot UND Abbruch, weil ohne
    # bestimmten Korpus keine der uebrigen Zusagen etwas Vollstaendiges misst.
    try:
        korpus = _lies_geprueft(JOURNAL_FIXTUR_MANIFEST, _journalkorpus_struktur)
    except Strukturhalt as halt:
        return {}, [str(halt)], [str(halt)]
    geladen: dict[str, tuple[bytes, dict, dict]] = {}
    for fall in korpus["faelle"]:
        weg = JOURNAL_FIXTUREN / fall["datei"]
        if not weg.is_file():
            haltend(f"{fall['datei']}: im MANIFEST gefuehrt, liegt aber nicht vor")
            continue
        rohe = weg.read_bytes()
        ist = hashlib.sha256(rohe).hexdigest().upper()
        if ist != fall["sha256"]:
            # KEIN Abbruch: die Datei liegt vor. Sie ist keine eingefrorene
            # Writer-Form mehr - das sagt Z1 -, aber der Korpus ist
            # vollstaendig, also messen Z2..Z7 unten auf ihm weiter. Nur so
            # faerbt ein geaendertes Byte genau EINE Zusagenzeile.
            klagen.append(
                f"{fall['datei']}: SHA-256 {ist} statt {fall['sha256']} - von Hand "
                "geaendert? Dann ist sie keine Writer-Form mehr")
        try:
            kopf = json.loads(rohe.decode("utf-8-sig"))
        except (UnicodeDecodeError, json.JSONDecodeError) as fehler:
            # Liegt vor, ist aber nicht lesbar: hier hoert die Unterscheidung
            # auf - ohne Kopf kann keine andere Zusage etwas ueber diesen Fall
            # sagen, also bricht der Block ab.
            haltend(f"{fall['datei']}: liegt vor, ist aber nicht lesbar "
                    f"({type(fehler).__name__}) - ohne Kopf misst [3b] den Fall "
                    "nicht mehr")
            continue
        if not isinstance(kopf, dict):
            # Lesbar, aber kein Journalkopf. Ohne Abbruch liefe [3b] mit einem
            # Nicht-Objekt im Korpus weiter und stuerbe unten mit einem
            # Traceback - Unbekanntes ist ROT, nicht laut (Pruefliste D).
            haltend(f"{fall['datei']}: liegt vor und ist lesbar, ist aber kein "
                    f"Journalobjekt ({type(kopf).__name__}) - [3b] kann daran "
                    "keine Zusage messen")
            continue
        strukturfehler = _writer_struktur(kopf, fall)
        if strukturfehler:
            # Lesbares Objekt, aber nicht die Form, die Z2..Z7 lesen. Ohne
            # Abbruch liefe [3b] weiter und stuerbe unten im ersten Zugriff
            # mit einem Traceback - Unbekanntes ist ROT, nicht laut
            # (Pruefliste D, NAK-94 Nacharbeit 8).
            haltend(f"{fall['datei']}: Fixtur geaendert und strukturell "
                    "unbrauchbar - liegt vor und ist ein Objekt, traegt aber "
                    "nicht die von Z2..Z7 gelesene Writer-Struktur: "
                    + "; ".join(strukturfehler))
            continue
        geladen[fall["datei"]] = (rohe, kopf, fall)
    # Eine verwaiste Datei ist derselbe Befund wie eine fehlende: sie koennte
    # von Hand danebengelegt werden und saehe wie eine Writer-Fixtur aus.
    genannt = {fall["datei"] for fall in korpus["faelle"]}
    for weg in sorted(JOURNAL_FIXTUREN.glob("*.json")):
        if weg.name != "MANIFEST.json" and weg.name not in genannt:
            haltend(f"{weg.name}: liegt im Korpus, steht in keinem MANIFEST-Fall")
    # Der Anker, den ein Loeschen im Korpus NICHT mitnimmt.
    vorhanden = {fall.get("status") for fall in korpus["faelle"]}
    fehlt = [s for s in JOURNAL_PFLICHTSTATUS if s not in vorhanden]
    if fehlt:
        haltend(
            f"kein Fall mit Journalstatus {', '.join(fehlt)} - [3b] behauptet "
            "sonst etwas ueber eine Statusklasse, die es nicht mehr misst")
    return geladen, klagen, abbruch


def _manifest_zum_journal(manifest: dict, journal: dict) -> dict:
    """Manifestkopie, deren Artefakt-Hashes zu diesem Journal passen.

    Die Fixtur ist die Wahrheit - sie stammt vom Writer. Angepasst wird die
    andere Seite: jedes Manifestartefakt bekommt den `sha256` des Eintrags mit
    derselben Kennung. Damit misst die Probe den heutigen LESER an einem echten
    Writer-Journal, statt eine Zahl in die Fixtur zu schreiben, die dort nie
    stuende. Ein Artefakt ohne passenden Eintrag bleibt unveraendert - die
    Probe faellt dann sichtbar, statt still weniger zu messen.
    """
    # Nur STRING-Kennungen: `{"ziel_id": ["main"]}` waere als dict-Schluessel
    # nicht hashbar. Genau dieser TypeError war Befund C2 - eine Hilfsfunktion
    # der Proben darf ihn nicht durch die Hintertuer wieder einfuehren.
    nach_kennung = {}
    for e in journal.get("eintraege") or []:
        if isinstance(e, dict) and isinstance(_artefakt_name(e), str):
            nach_kennung[_artefakt_name(e)] = e
    kopie = copy.deepcopy(manifest)
    for a in kopie["artefakte"]:
        e = nach_kennung.get(_artefakt_name(a))
        if e is not None and isinstance(e.get("sha256"), str):
            a["sha256"] = e["sha256"]
    return kopie


def gegenproben_nacharbeit(manifest: dict) -> None:
    """[3b] Die Gegenproben zu [4] und [4b] - gegliedert nach ZUSAGEN.

    Beide Ursprungsbefunde waren stille Ausfaelle: C1 liess ein fehlendes
    Bundle im Kanon gruen, C2 toetete den Lauf mit einem TypeError. Eine
    Wache, die niemand hat fallen sehen, ist keine - also faellt sie hier.

    Seit NAK-94 Nacharbeit 5 (30.08.2026) faehrt der Block ZWEI Sorten
    Probe-Journale und nennt sie in jeder Zeile: eingefrorene Writer-Fixturen
    und daraus abgeleitete, benannte Mutanten. Handschrift gibt es nicht mehr.

    NAK-94 Nacharbeit 6 (30.08.2026), Regel des Dirigenten nach dem Befund des
    sechsten Pruefers: der Block ist nach ZUSAGEN gegliedert (Z1..Z7), und JE
    ZUSAGE gibt es genau EINEN diskriminierenden Bruch (B6-Z1..B6-Z7, belegt
    mit ROT und Ruecknahme in docs/beweise/SONDE-007c.md). Eine Zusage, die
    ueber mehrere Werte parametrisiert ist - das OK-Urteil ueber jede
    OK-Fixtur, die Statussperre ueber jeden Nicht-OK-Status -, ist EINE
    Zusage; ihre Werte zaehlt die Ausgabe, nicht dieser Text.

    NAK-94 Nacharbeit 7 (30.08.2026), Befund des siebten Pruefers: bis dahin
    hielt JEDE Z1-Klage den ganzen Block an, auch die an einem geaenderten
    Byte - Z2..Z7 liefen dann gar nicht, statt gruen zu bleiben, und der Bruch
    unterschied nichts. Jetzt trennt _writer_fixturen() zwischen "liegt vor,
    aber geaendert" (nur Z1 rot) und "fehlt / unlesbar / verwaist / Statusachse
    weg" (Abbruch). Der fail-closed Abbruch bleibt; er hat mit der Probe
    "Pflichtmenge" seinen eigenen Bruch.
    """
    global INSTALL_ERGEBNIS
    print("\n[3b] Gegenproben zu [4] Auslieferungsstand und [4b] installiertem Stand")
    print("     Gegliedert nach Zusagen Z1..Z7 - je Zusage ein eigener Bruch (B6-Zx).")

    # -- Z1: der Writer-Korpus ist vollstaendig und bytegleich --------------
    #
    # Diese Zusage traegt alle anderen. Was danach passiert, haengt daran, WAS
    # ihr fehlt (NAK-94 Nacharbeit 7, 30.08.2026):
    #
    #   ein geaendertes BYTE  Die Datei liegt vor und ist lesbar. Z1 wird rot,
    #                         der Block laeuft weiter - Z2..Z7 messen auf dem
    #                         VOLLSTAENDIGEN Korpus und bleiben gruen. So ist
    #                         der Bruch unterscheidbar: eine Zusage, eine
    #                         rote Zeile.
    #   eine fehlende DATEI   Der Korpus ist unvollstaendig. Der Block bricht
    #                         ab (fail-closed) - eine stillschweigend
    #                         ausgelassene Gegenprobe ist schlimmer als keine.
    fixturen, korpusklagen, korpusabbruch = _writer_fixturen()
    stati = sorted({e["status"] for _r, _k, e in fixturen.values()})
    pruefe(not korpusklagen,
           "Z1 [Writer-Korpus]: jeder in MANIFEST.json gefuehrte Fall liegt vor "
           "und ist bytegleich, keine verwaiste Datei daneben, und die "
           f"Statusachse {', '.join(JOURNAL_PFLICHTSTATUS)} ist vertreten",
           " | ".join(korpusklagen) if korpusklagen
           else f"{len(fixturen)} Faelle, Status {', '.join(stati)}")
    if korpusabbruch:
        print("     [3b] bricht hier ab: ohne vollstaendigen Korpus misst der "
              "Block die Writer-Form nicht mehr, und eine stillschweigend "
              "ausgelassene Gegenprobe ist schlimmer als keine.  Grund: "
              + " | ".join(korpusabbruch))
        return
    if korpusklagen:
        print("     [3b] laeuft weiter: die beanstandete(n) Datei(en) liegen vor "
              "und sind lesbar, der Korpus ist also vollstaendig. Z2..Z7 messen "
              "auf ihm weiter - rot ist damit genau eine Zusage: Z1.")

    # Die Fixturen der beiden Achsen kommen aus dem KORPUS, nicht aus einer
    # Namensliste im Skript: kommt eine fuenfte Fixtur dazu, faehrt [3b] sie
    # mit, ohne dass jemand hier etwas nachtraegt.
    ok_dateien = [d for d, (_r, _k, e) in sorted(fixturen.items())
                  if e["status"] == ERGEBNIS_STATUS_OK]
    nicht_ok_dateien = [d for d, (_r, _k, e) in sorted(fixturen.items())
                        if e["status"] != ERGEBNIS_STATUS_OK]
    mutant_quelle = ok_dateien[0]

    # -- Z2: ein `sha256: null` beendet den Artefaktcheck nicht -------------
    #
    # Die Probe braucht ZWEI Artefakte: eines ohne Hash und ein anderes, das
    # fehlt. Faellt das Manifest je auf eines zusammen, misst sie nichts mehr -
    # und eine stillschweigend ausgelassene Gegenprobe ist schlimmer als keine.
    if len(manifest["artefakte"]) < 2:
        raise SystemExit(
            "Gegenprobe unmoeglich: die Z2-Probe braucht ZWEI Artefakte - eines "
            "ohne festgeschriebenen Hash und ein anderes, das fehlt. Mit nur "
            "einem laesst sich nicht zeigen, dass der Artefaktcheck WEITERLAEUFT."
        )
    probe = copy.deepcopy(manifest)
    probe["artefakte"][0]["sha256"] = None
    fehlt = probe["artefakte"][-1]
    fehlt["quelle"] = fehlt["quelle"] + "-GIBT-ES-NICHT"
    ausgabe, klagen = _probelauf(lambda: auslieferungsstand(probe, hart=False))
    pruefe(any("liegt nicht vor" in k for k in klagen),
           "Z2 [Kanon]: ein fehlendes Artefakt ist auch im Kanon ROT, wenn ein "
           "anderes keinen festgeschriebenen Hash traegt (Befund C1)",
           " | ".join(klagen[:2]) if klagen else "keine Klage")
    pruefe("Ordner-Hash bildbar" in ausgabe,
           "Z2 [Kanon]: das Artefakt ohne Hash wird trotzdem gemessen (liegt "
           "vor, Ordner-Hash bildbar) statt uebersprungen",
           next((z.strip() for z in ausgabe.splitlines()
                 if "Ordner-Hash bildbar" in z), "keine solche Zeile"))
    _, hart_klagen = _probelauf(lambda: auslieferungsstand(
        copy.deepcopy(probe), hart=True))
    pruefe(any("liegt nicht vor" in k for k in hart_klagen)
           and any("ohne Hash" in k for k in hart_klagen),
           "Z2 [--release]: dieselbe Lage macht unter --release BEIDE Befunde "
           "zu Fehlern - den fehlenden Hash und das fehlende Artefakt",
           " | ".join(hart_klagen[:2]) if hart_klagen else "keine Klage")

    with tempfile.TemporaryDirectory(prefix="nakama-journal-") as tmp:
        journal = pathlib.Path(tmp) / "install-ergebnis.json"
        merk = INSTALL_ERGEBNIS
        INSTALL_ERGEBNIS = journal
        try:
            def mutant(quelle: str, abweichung: str, wandel) -> tuple[dict, str]:
                """Genau EINE Writer-Fixtur, genau EINE benannte Abweichung.

                Der Mutant entsteht aus den gelesenen Fixturbytes, nie aus
                einem selbst gebauten Kopf: alles ausser der genannten
                Abweichung bleibt damit Writer-Form.
                """
                kopf = copy.deepcopy(fixturen[quelle][1])
                wandel(kopf)
                return kopf, f"Mutant von {quelle}: {abweichung}"

            def fixtur_lauf(datei: str, gegen: dict | None = None) -> str:
                """Die eingefrorenen BYTES fahren, nicht ein neu serialisiertes
                Abbild - sonst misst die Probe json.dumps und nicht den Writer."""
                journal.write_bytes(fixturen[datei][0])
                ziel = gegen if gegen is not None else _manifest_zum_journal(
                    manifest, fixturen[datei][1])
                text, offen = _probelauf(lambda: installierter_stand(ziel))
                return text if not offen else text + " KLAGEN " + " | ".join(offen)

            def mutant_lauf(kopf: dict, gegen: dict | None = None) -> str:
                journal.write_text(json.dumps(kopf), encoding="utf-8")
                ziel = gegen if gegen is not None else _manifest_zum_journal(
                    manifest, kopf)
                text, offen = _probelauf(lambda: installierter_stand(ziel))
                return text if not offen else text + " KLAGEN " + " | ".join(offen)

            def ohne_hinweis(ausgabe: str, muster: str) -> list[str]:
                """Welche Artefakte fehlen in der erwarteten Zeilenmenge?

                Der Bericht ist artefaktweise; eine Probe, die nur `in ausgabe`
                fragt, waere schon zufrieden, wenn EIN Artefakt die Zeile
                traegt. Gefordert wird sie fuer jedes.
                """
                return [_artefakt_name(a) for a in manifest["artefakte"]
                        if muster.format(name=_artefakt_name(a)) not in ausgabe]

            # -- Z3: eine unbrauchbare Kennung ist ein Hinweis, kein Abbruch -
            #
            # Beide Seiten tragen dieselbe Zusage: der Fehler darf im JOURNAL
            # stehen oder im MANIFEST - [4b] faellt in keinem Fall ein Urteil
            # und toetet in keinem Fall den Kanonlauf (Befund C2).
            kopf, sorte = mutant(
                mutant_quelle,
                "eintraege -> [Eintrag mit ziel_id als Liste, Nicht-Objekt]",
                lambda k: k.update(eintraege=[
                    {**k["eintraege"][0], "ziel_id": ["main"]}, "keine Abbildung"]))
            ausgabe = mutant_lauf(kopf, manifest)
            pruefe("ohne lesbare Kennung" in ausgabe and "kein Objekt" in ausgabe,
                   f"Z3 [{sorte}]: eine unbrauchbare Kennung IM JOURNAL ist ein "
                   "Hinweis, kein TypeError - und [4b] faellt kein Urteil",
                   " / ".join(z.strip() for z in ausgabe.splitlines()
                              if "hinweis 0" in z or "hinweis 1" in z))

            kaputt = _manifest_zum_journal(manifest, fixturen[mutant_quelle][1])
            kaputt["artefakte"][0]["ziel_id"] = ["main"]
            ausgabe = fixtur_lauf(mutant_quelle, kaputt)
            pruefe("nicht auswertbar" in ausgabe,
                   f"Z3 [Writer-Fixtur {mutant_quelle}]: dieselbe Zusage von der "
                   "anderen Seite - ein Fehler IM MANIFEST bleibt ebenfalls ein "
                   "Hinweis, [4b] toetet keinen Kanonlauf",
                   next((z.strip() for z in ausgabe.splitlines()
                         if "nicht auswertbar" in z), "keine solche Zeile"))

            # -- Z4: OK-Journal -> Hashvergleich und sein `ok` --------------
            #
            # EINE Zusage ueber JEDE OK-Fixtur des Korpus. Die Fixturen kommen
            # aus MANIFEST.json; ihre Zahl steht in der Ausgabe, nicht hier.
            # Jede stammt aus einem echten Install-Nakama.ps1-Lauf in der
            # A18-Sandbox; ihre volatilen Werte - zeit, transaktions_id, Pfade,
            # Hashes - bleiben, wie der Writer sie schrieb.
            for datei in ok_dateien:
                fall = fixturen[datei][2]
                ausgabe = fixtur_lauf(datei)
                fehlend = ohne_hinweis(
                    ausgabe, "ok      {name}: installierter Stand = Manifest")
                pruefe(not fehlend,
                       f"Z4 [Writer-Fixtur {datei}]: bei Journalstatus OK und "
                       "abgeschlossenen, nicht zurueckgerollten Eintraegen "
                       f"bleibt der Hashvergleich und sein ok ({fall['fall']})",
                       ("ohne ok: " + ", ".join(fehlend)) if fehlend
                       else "alle Artefakte ok")

            # -- Z5: alles ausser OK -> artefaktweise "unbekannt" -----------
            #
            # EINE Zusage, ueber drei Wertemengen parametrisiert: die
            # Nicht-OK-Fixturen des Korpus (echte Writer-Form), die Durchgangs-
            # und erfundenen Statuswerte als Mutanten aus DERSELBEN OK-Fixtur
            # (damit nur der Status abweicht) und ein Journal ganz ohne
            # `status`. Alle drei sagen dasselbe: ohne Journalstatus OK gibt es
            # keinen Hashvergleich, weil der gespeicherte Hash dann nur den
            # Stand vor dem Gegenakt beweist.
            for datei in nicht_ok_dateien:
                status = fixturen[datei][2]["status"]
                ausgabe = fixtur_lauf(datei)
                fehlend = ohne_hinweis(
                    ausgabe, "hinweis {name}: installierter Stand unbekannt "
                             f"(Journalstatus {status})")
                pruefe(not fehlend
                       and "keine Liste 'eintraege'" not in ausgabe
                       and "installierter Stand = Manifest" not in ausgabe,
                       f"Z5 [Writer-Fixtur {datei}]: Journalstatus {status} "
                       "meldet artefaktweise 'unbekannt' - ohne Hashvergleich "
                       "und ohne 'keine Liste'",
                       ("ohne Hinweis: " + ", ".join(fehlend)) if fehlend
                       else next((z.strip() for z in ausgabe.splitlines()
                                  if "Journal:" in z), "keine Journalzeile"))

            for status in ("VORBEREITET", "KOMPENSATION", "ERROR_TEILSTAND",
                           "ERROR_RUECKGEROLLT", "RUECKWEG_AKTIV", "RUECKWEG",
                           "NEUER_STATUS_2099"):
                kopf, sorte = mutant(mutant_quelle, f"status -> {status}",
                                     lambda k, s=status: k.update(status=s))
                ausgabe = mutant_lauf(kopf)
                fehlend = ohne_hinweis(
                    ausgabe, "hinweis {name}: installierter Stand unbekannt "
                             f"(Journalstatus {status})")
                pruefe(not fehlend and "installierter Stand = Manifest" not in ausgabe,
                       f"Z5 [{sorte}]: meldet den installierten Stand als "
                       "unbekannt - ohne Hashvergleich",
                       ("ohne Hinweis: " + ", ".join(fehlend)) if fehlend
                       else "alle Artefakte als unbekannt gemeldet")

            kopf, sorte = mutant(mutant_quelle, "status entfernt",
                                 lambda k: k.pop("status", None))
            ausgabe = mutant_lauf(kopf)
            fehlend = ohne_hinweis(
                ausgabe, "hinweis {name}: installierter Stand unbekannt "
                         "(Journalstatus fehlt)")
            pruefe(not fehlend and "installierter Stand = Manifest" not in ausgabe,
                   f"Z5 [{sorte}]: ein Journal OHNE status meldet den "
                   "installierten Stand als unbekannt - Schweigen ist kein OK",
                   ("ohne Hinweis: " + ", ".join(fehlend)) if fehlend
                   else next((z.strip() for z in ausgabe.splitlines()
                              if "Journalstatus fehlt" in z), "keine solche Zeile"))

            # -- Z6: Status OK ohne Eintragsliste ---------------------------
            #
            # Der fruehere Fall (c). Bis Nacharbeit 4 zaehlte er als
            # Writer-Form-Probe und behauptete damit, der Writer koenne ein
            # OK-Journal ohne `eintraege` schreiben - er kann es nicht
            # (Install-Nakama.ps1 legt die Liste immer an, der OK-Pfad setzt
            # danach nur `status` und `zeit`). Als benannter Mutant sagt die
            # Probe dasselbe ueber den LESER, ohne etwas Falsches ueber den
            # Writer zu behaupten.
            kopf, sorte = mutant(mutant_quelle, "eintraege entfernt",
                                 lambda k: k.pop("eintraege", None))
            ausgabe = mutant_lauf(kopf, manifest)
            pruefe("keine Liste 'eintraege'" in ausgabe
                   and "installierter Stand = Manifest" not in ausgabe
                   and "installierter Stand unbekannt" not in ausgabe,
                   f"Z6 [{sorte}]: bei Status OK ohne Eintragsliste bleibt es "
                   "bei 'fuehrt keine Liste eintraege' - die Statussperre "
                   "verschluckt sie nicht",
                   next((z.strip() for z in ausgabe.splitlines()
                         if "keine Liste" in z), "keine solche Zeile"))

            # -- Z7: die Fixtur-IDs gegen die Regex des WRITERS -------------
            #
            # Nicht abgeschrieben: das Muster wird aus Install-Nakama.ps1
            # gelesen, die IDs kommen aus den eingefrorenen Writer-Fixturen.
            # Aendert der Writer seine ID-Form, faellt diese Zeile - und nicht
            # erst der naechste Pruefer.
            #
            # PowerShells `-match` ist von Haus aus ohne Ruecksicht auf
            # Gross-/Kleinschreibung, `re.match` nicht. Diese Wache ist damit
            # STRENGER als der Writer: was hier besteht, besteht dort auch.
            writer_text = INSTALLER.read_text(encoding="utf-8")
            muster = re.search(r"function Ist-TransaktionsId.*?-match\s*'([^']+)'",
                               writer_text, re.S)
            ids = {d: k.get("transaktions_id") for d, (_r, k, _f) in fixturen.items()}
            passt = muster is not None and bool(ids) and all(
                isinstance(i, str) and re.match(muster.group(1), i) is not None
                for i in ids.values())
            pruefe(passt,
                   "Z7 [Writer-Fixturen]: jede eingefrorene Transaktions-ID "
                   "besteht die Ist-TransaktionsId-Regex aus Install-Nakama.ps1 "
                   "- eine gestrichelte UUID taete es nicht",
                   f"Muster {muster.group(1)!r} gegen {sorted(ids.values())}"
                   if muster else "Ist-TransaktionsId im Writer nicht gefunden")
        finally:
            INSTALL_ERGEBNIS = merk


# ── Hashen (Release-Schritt, nicht Kanon) ──────────────────────────────────

def datei_hash(pfad: pathlib.Path) -> str:
    h = hashlib.sha256()
    with pfad.open("rb") as f:
        for block in iter(lambda: f.read(1 << 20), b""):
            h.update(block)
    return h.hexdigest().upper()


def hashen(manifest: dict, ziel: pathlib.Path | None = None) -> int:
    """Der mutierende Release-Zweig `--hashen`.

    `ziel` ist das Schreibziel; ohne Angabe das Manifest im Repo. Der Byte-
    Kipp-Fuzz [3c] setzt hier eine KOPIE unter %TEMP% ein: `--hashen` ist der
    einzige Zweig, der das gelesene Manifest weiterverarbeitet und schreibt,
    er gehoert deshalb in die Deckung - aber niemals mit Schreibwirkung ins
    Repo (NAK-94 Nacharbeit 11). Vorher stand er als Handausnahme in
    FUZZ_OHNE_JSON und war ungemessen.
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


# ── [3c] Byte-Kipp-Fuzz ueber JEDE gelesene JSON-Datei ──────────────────────
#
# NAK-94 Nacharbeit 10, Wegwechsel W3 (30.08.2026). Drei Runden lang hat je ein
# Pruefer EIN weiteres Feld gefunden, dessen Fehlen einen Traceback statt eines
# Strukturhalts ergab: "faelle" (Nacharbeit 9), "artefakte"/"ziele" (ebenfalls
# 9) und zuletzt "ziel_id" plus ein ungueltiges UTF-8-Byte (Nacharbeit 10).
# Feld fuer Feld nachzuziehen schliesst die Klasse nicht - es verschiebt sie
# auf das naechste Feld.
#
# Deshalb wird die Zusage jetzt GEMESSEN statt behauptet. Jede von diesem Bein
# gelesene JSON-Datei wird Byte fuer Byte gekippt - auf 0xFF (ungueltiges UTF-8)
# und auf 0x20 (gueltiges Zeichen, das Schluessel, Zahlen und Trennzeichen
# zerlegt) - und in-process durch Lesen, Strukturpruefung und JEDEN Block
# gefahren, der eine dieser Dateien verbraucht. Zugesagt ist: keine Ausnahme
# ausser `Strukturhalt`. Alles andere waere ein Traceback und ist hier ROT,
# benannt mit Datei, Byte, Kippwert, Ausnahmetyp und Ausloeserzeile.
#
# ZWEI AUSGAENGE ZAEHLEN NICHT ALS BRUCH, weil beide kontrolliert sind und
# Klartext tragen: `Strukturhalt` (die erste Verteidigung) und der eigene
# `SystemExit("Gegenprobe unmoeglich: ...")` aus [3]/[3b] - eine Byteaenderung
# kann die Datenlage so verkleinern, dass eine Gegenprobe nichts mehr misst,
# und dann ist der Abbruch die richtige Antwort. Beide werden getrennt
# gezaehlt und ausgewiesen, nie verschwiegen.
#
# KEIN SUBPROZESS JE BYTE: das waeren zehntausende Prozessstarts. Der Inhalt
# wird stattdessen fuer die Dauer EINES Laufs ueberlagert
# (`_dateien_ersetzt`), und `artefakt_hash` wird waehrend des Fuzz gemerkt -
# die gebauten Artefakte aendern sich dabei nicht, wohl aber die Frage, WELCHES
# Artefakt gehasht wird.

FUZZ_KIPPWERTE = (0xFF, 0x20)
# Der Schritt des Kanon-Samples. Deterministisch (jedes n-te Byte, beginnend
# bei 0), damit derselbe Stand dieselben Stellen prueft; der vollstaendige Lauf
# ueber JEDES Byte laeuft mit --fuzz-voll und steht als Rohausgabe im Manifest.
FUZZ_SCHRITT = 24
_FUZZ_SCHLUESSEL = re.compile(rb'"([A-Za-z_][A-Za-z0-9_]*)"\s*:')


def fuzz_stellen(roh: bytes, schritt: int) -> list[int]:
    """Die Byte-Stellen eines Laufs - deterministisch, aus den Bytes gerechnet.

    ZWEI HAELFTEN, weil ein reines Stride-Sample strukturell blind fuer kurze
    Tokens ist: bei Schritt n kann jeder Schluessel, der kuerzer als n ist,
    vollstaendig zwischen zwei Stichproben liegen - und genau an
    SCHLUESSELNAMEN hingen die letzten drei Befunde ("faelle", "artefakte",
    "ziel_id").

      1. jedes n-te Byte, beginnend bei 0 (bei n = 1 also jedes Byte),
      2. das ERSTE Namensbyte jedes JSON-Schluessels. Ein Kipp dort macht aus
         `"ziel_id"` genau das `"xiel_id"` des zehnten Pruefers.

    Beide Haelften haengen nur vom Dateiinhalt ab; derselbe Stand prueft
    dieselben Stellen.
    """
    stellen = set(range(0, len(roh), max(1, schritt)))
    stellen.update(treffer.start(1)
                   for treffer in _FUZZ_SCHLUESSEL.finditer(roh))
    return sorted(stellen)


def fuzz_dateien() -> list[pathlib.Path]:
    """Jede JSON-Datei, die dieses Bein liest - deterministisch sortiert.

    Die Writer-Fixturen kommen aus dem Verzeichnis, nicht aus einer Liste im
    Skript: eine neue Fixtur ist damit automatisch im Fuzz, statt hier
    nachgepflegt werden zu muessen.
    """
    dateien = [MANIFEST, IDENTITAET, JOURNAL_FIXTUR_MANIFEST]
    dateien += [p for p in sorted(JOURNAL_FIXTUREN.glob("*.json"))
                if p.name != JOURNAL_FIXTUR_MANIFEST.name]
    dateien.append(INSTALL_ERGEBNIS)
    return [p for p in dateien if p.is_file()]


@contextlib.contextmanager
def _dateien_ersetzt(ersatz: dict):
    """Ueberlagert den INHALT einzelner Dateien fuer die Dauer des Blocks.

    `read_text` dekodiert dabei ECHT - ein ungueltiges Byte gibt denselben
    `UnicodeDecodeError` wie von der Platte. `open()` bleibt unberuehrt: die
    gebauten Artefakte werden weiter von der Platte gehasht.
    """
    norm = {os.path.normcase(os.path.normpath(str(p))): b
            for p, b in ersatz.items()}
    alt_bytes, alt_text = pathlib.Path.read_bytes, pathlib.Path.read_text

    def neue_bytes(self, *a, **k):
        roh = norm.get(os.path.normcase(os.path.normpath(str(self))))
        return alt_bytes(self, *a, **k) if roh is None else roh

    def neuer_text(self, encoding=None, errors=None, **k):
        roh = norm.get(os.path.normcase(os.path.normpath(str(self))))
        if roh is None:
            return alt_text(self, encoding=encoding, errors=errors, **k)
        return roh.decode(encoding or "utf-8", errors or "strict")

    pathlib.Path.read_bytes, pathlib.Path.read_text = neue_bytes, neuer_text
    try:
        yield
    finally:
        pathlib.Path.read_bytes, pathlib.Path.read_text = alt_bytes, alt_text


def _fuzz_hashziel() -> pathlib.Path:
    """Das Schreibziel des gefuzzten `--hashen`-Zweigs: eine Manifest-KOPIE
    unter %TEMP%, NIE das Manifest im Repo.

    Fester Name statt `mkdtemp`: der Fuzz faehrt den Zweig zehntausendfach und
    darf dabei nicht zehntausend Verzeichnisse hinterlassen. Die Datei wird
    jedes Mal ueberschrieben; ihr Inhalt ist Wegwerfstoff, gemessen wird nur,
    dass der Zweig kontrolliert bleibt.
    """
    return pathlib.Path(tempfile.gettempdir()) / "nakama-nak94-fuzz-manifest.json"


def _fuzz_verbraucher() -> None:
    """Lesen, Strukturpruefung und JEDER Block, der eine gelesene Datei nutzt.

    Dieselbe Reihenfolge und dieselben Aufrufe wie in `_lauf()` - nur ohne die
    Kreuzprobe, die keine JSON-Datei liest, und ohne die Ausgabe. Die
    Ausnahmebehandlung in [2] ist woertlich uebernommen: dort zaehlt eine
    stolpernde Regel als Fund, und der Fuzz darf das nicht strenger sehen als
    der echte Lauf.

    Der `--hashen`-Zweig ist seit NAK-94 Nacharbeit 11 dabei (Befund des
    elften Pruefers). Er ist im echten Lauf ein ALTERNATIVER Ausgang, laeuft
    hier also zuletzt und auf einer tiefen Kopie - `hashen()` traegt sha256
    und Zeitstempel in das Objekt ein, und die Blocke davor sollen den
    unveraenderten Stand sehen. Geschrieben wird nach %TEMP%.
    """
    manifest = _lies_geprueft(MANIFEST, _installermanifest_struktur)
    identitaet = _lies_geprueft(IDENTITAET, _identitaet_struktur)
    for regel, _text in REGELN:
        regel(manifest, identitaet)
    kaputt = verdirb(manifest, identitaet)
    kaputte_identitaet = verdirb_identitaet(identitaet)
    for regel, _text in REGELN:
        try:
            regel(kaputt, kaputte_identitaet)
        except Exception:
            pass
    adversariale_strukturproben(manifest, identitaet)
    gegenproben_nacharbeit(manifest)
    auslieferungsstand(manifest, hart=False)
    installierter_stand(manifest)
    hashen(copy.deepcopy(manifest), _fuzz_hashziel())


def _fuzz_erzeuger() -> None:
    """Der ZWEITE Leser desselben Korpusmanifests: `pruefen()` des Erzeugers.

    BEFUND DES ZWOELFTEN PRUEFERS (NAK-94 Nacharbeit 12): der Erzeuger griff
    ohne Strukturpruefung auf `journale/MANIFEST.json` zu und starb bei einer
    einzelnen Byteaenderung mit einem Traceback, waehrend dieses Bein an
    derselben Datei laengst einen Klartext-Abbruch lieferte. Er liest sie jetzt
    ueber `_lies_geprueft`/`_journalkorpus_struktur` - und wird hier
    MITGEFUZZT, damit das gemessen ist und nicht behauptet.

    EIGENER VERBRAUCHER, NICHT ANGEHAENGT: `_fuzz_verbraucher()` bricht bei
    einem Strukturhalt ab, und zwar an genau der Datei, um die es hier geht -
    ein Aufruf am Ende jener Funktion waere fuer jede Mutation des
    Korpusmanifests unerreichbar und damit eine Wache, die nichts misst.
    `_fuzz_einmal()` faehrt deshalb BEIDE Verbraucher, jeden in derselben
    Huelle `_geschuetzt()`, und nimmt die schwerste Klasse.

    EIN MODUL, EINE Strukturhalt-KLASSE: laeuft dieses Bein als `__main__`,
    legte der Import des Erzeugers eine ZWEITE Kopie dieses Moduls an - deren
    `Strukturhalt` waere eine andere Klasse, `_geschuetzt()` fienge sie nicht,
    und jeder Fuzz-Fall waere falsch "unkontrolliert". Der Eintrag unten haengt
    das laufende Modul deshalb unter seinen Dateinamen, BEVOR importiert wird.
    """
    sys.modules.setdefault("pruefe_installer_manifest", sys.modules[__name__])
    from erzeuge_installer_journale import pruefen as korpus_nachrechnen
    korpus_nachrechnen()


# Beide Verbraucher der gefuzzten JSON-Dateien, in dieser Reihenfolge gefahren.
# Die Deckungspruefung [3c/0] rechnet ueber ihre Vereinigung.
FUZZ_VERBRAUCHER = (_fuzz_verbraucher, _fuzz_erzeuger)


# Bloecke, die `_lauf()` faehrt und die `_fuzz_verbraucher()` NICHT faehrt,
# weil sie keine der gelesenen JSON-Dateien anfassen. Jede ANDERE Abweichung
# ist ROT: sonst waechst `_lauf` still um einen Leser, den [3c] nie sieht -
# und die Zusage "jede gelesene JSON-Datei" waere wieder eine Behauptung.
#
# `hashen` stand hier bis NAK-94 Nacharbeit 11 als Handausnahme ("nur unter
# --hashen, und der Pfad endet vor [3c]"). Genau daran hing die falsche
# Vollstaendigkeitszusage von [3c/0]: der Zweig verarbeitet das gelesene
# Manifest sehr wohl. Er ist jetzt gefuzzt - mit Schreibziel unter %TEMP% -
# und steht deshalb NICHT mehr in dieser Liste.
FUZZ_OHNE_JSON = {
    "print",             # Ausgabe
    "pruefe",            # Urteil, kein Leser
    "len", "max", "str",  # eingebaut
    "byte_kipp_fuzz",    # der Fuzz selbst - er darf sich nicht selbst fahren
    "kreuzprobe",        # hasht Ordner und faehrt PowerShell, liest kein JSON
    "_stillgelegte",     # arbeitet auf dem schon gelesenen Objekt
}


def _aufgerufene(funktionsname: str) -> set[str]:
    """Die beim Namen aufgerufenen Funktionen EINER Funktion, aus dem AST.

    Gelesen wird die eigene Quelle - kein Import, keine Ausfuehrung.
    """
    try:
        baum = ast.parse(pathlib.Path(__file__).read_text(encoding="utf-8"))
    except (OSError, SyntaxError, ValueError):
        return set()
    for knoten in ast.walk(baum):
        if (isinstance(knoten, ast.FunctionDef)
                and knoten.name == funktionsname):
            return {k.func.id for k in ast.walk(knoten)
                    if isinstance(k, ast.Call) and isinstance(k.func, ast.Name)}
    return set()


def fuzz_deckung() -> set[str]:
    """Bloecke aus `_lauf`, die der Fuzz nicht faehrt und nicht fahren darf.

    Gerechnet wird ueber die VEREINIGUNG aller Verbraucher (Nacharbeit 12) -
    seit der Erzeuger-Leser dazugehoert, ist es nicht mehr nur einer.
    """
    gefahren: set[str] = set()
    for fn in FUZZ_VERBRAUCHER:
        gefahren |= _aufgerufene(fn.__name__)
    return _aufgerufene("_lauf") - gefahren - FUZZ_OHNE_JSON


# Von der schwersten zur leichtesten: faehrt ein Fall mehrere Verbraucher,
# entscheidet die schwerste Klasse ueber das Urteil. `unkontrolliert` ist der
# einzige Bruch der Zusage und steht deshalb vorn.
_FUZZ_RANG = ("unkontrolliert", "gegenprobe_unmoeglich", "strukturhalt")


def _fuzz_einmal(ersatz: dict) -> tuple[str, str]:
    """EIN Lauf mit ueberlagertem Inhalt. (Klasse, Klartext)

    Klassen: `strukturhalt`, `gegenprobe_unmoeglich`, `befund`, `gruen` und
    `unkontrolliert` - die letzte ist der Bruch der Zusage.

    Die Klassifikation kommt seit NAK-94 Nacharbeit 11 aus `_geschuetzt()` -
    DERSELBEN Funktion, die `main()` um den ganzen Lauf legt. Hier steht keine
    zweite Ausnahmebehandlung mehr; wird die gemeinsame auf Durchreichen
    gestellt, faellt [3c] beim ersten Strukturhalt aus.

    Seit Nacharbeit 12 laufen ALLE Verbraucher aus `FUZZ_VERBRAUCHER`, jeder
    in seiner eigenen Huelle und unabhaengig davon, ob ein frueherer abgebrochen
    ist: sonst haette der Erzeuger-Leser fuer genau die Mutationen, um die es
    bei ihm geht, nie eine Zeile gesehen.
    """
    global ok
    merk_ok, merk_fehler = ok, list(fehler)
    fehler.clear()
    try:
        with contextlib.redirect_stdout(io.StringIO()), \
                contextlib.redirect_stderr(io.StringIO()), \
                _dateien_ersetzt(ersatz):
            ergebnisse = [_geschuetzt(fn) for fn in FUZZ_VERBRAUCHER]
        for rang in _FUZZ_RANG:
            for klasse, text, _ in ergebnisse:
                if klasse == rang:
                    return klasse, text
        return ("befund", "; ".join(fehler[:2])) if fehler else ("gruen", "")
    finally:
        fehler.clear()
        fehler.extend(merk_fehler)
        ok = merk_ok


def byte_kipp_fuzz(schritt: int) -> None:
    """[3c]: jede gelesene JSON-Datei ueberlebt jede Ein-Byte-Aenderung."""
    voll = schritt <= 1
    print("\n[3c] Byte-Kipp-Fuzz - jede gelesene JSON-Datei, jedes Byte auf "
          "0xFF und 0x20" + ("" if voll else
                             f"; Kanon-Sample: jedes {schritt}. Byte PLUS das "
                             f"erste Namensbyte jedes JSON-Schluessels "
                             f"(--fuzz-voll faehrt alle)"))
    dateien = fuzz_dateien()
    if not dateien:
        pruefe(False, "[3c] es gibt gelesene JSON-Dateien zum Kippen")
        return

    # Den Erzeuger-Verbraucher EINMAL vorher anfassen, damit sein Import nicht
    # im ersten Fall unter der Dateiueberlagerung stattfindet. Gemessen wird
    # das Lesen, nicht das Importieren.
    _geschuetzt(_fuzz_erzeuger)

    # Bevor gezaehlt wird: faehrt der Fuzz ueberhaupt dieselben Bloecke wie der
    # echte Lauf? Sonst sagte eine gruene Zahl nur, dass ein VERALTETER
    # Ausschnitt keine Ausnahme wirft.
    offen = fuzz_deckung()
    pruefe(not offen,
           "[3c/0] der Fuzz faehrt jeden Block aus _lauf(), der eine gelesene "
           "JSON-Datei anfassen kann - die uebrigen stehen namentlich in "
           f"FUZZ_OHNE_JSON ({len(FUZZ_OHNE_JSON)}); Verbraucher: "
           + ", ".join(fn.__name__ for fn in FUZZ_VERBRAUCHER),
           "nicht gefahren: " + ", ".join(sorted(offen)) if offen else "")

    # Der gefuzzte `--hashen`-Zweig SCHREIBT. Gemessen wird deshalb nicht nur,
    # dass er kontrolliert bleibt, sondern auch, dass er das Manifest im Repo
    # nicht anfasst - vorher/nachher als sha256 ueber die echten Bytes, ausser-
    # halb jeder Ueberlagerung gelesen (NAK-94 Nacharbeit 11).
    manifest_vorher = hashlib.sha256(MANIFEST.read_bytes()).hexdigest()

    gemerkt: dict = {}
    echter_hash = artefakt_hash

    def gemerkter_hash(pfad, art):
        schluessel = (str(pfad), art)
        if schluessel not in gemerkt:
            gemerkt[schluessel] = echter_hash(pfad, art)
        return gemerkt[schluessel]

    zaehler = {"laeufe": 0, "strukturhalt": 0, "gegenprobe_unmoeglich": 0,
               "befund": 0, "gruen": 0, "unkontrolliert": 0}
    brueche: list[str] = []
    bytes_gesamt = 0
    beginn = time.perf_counter()
    globals()["artefakt_hash"] = gemerkter_hash
    try:
        for weg in dateien:
            roh = weg.read_bytes()
            kurz = weg.relative_to(WURZEL).as_posix()
            stellen = fuzz_stellen(roh, schritt)
            bytes_gesamt += len(stellen)
            fuer_datei = dict.fromkeys(zaehler, 0)
            for stelle in stellen:
                for wert in FUZZ_KIPPWERTE:
                    if roh[stelle] == wert:
                        continue          # kein Kipp - dasselbe Byte
                    klasse, text = _fuzz_einmal(
                        {weg: roh[:stelle] + bytes([wert]) + roh[stelle + 1:]})
                    zaehler["laeufe"] += 1
                    zaehler[klasse] += 1
                    fuer_datei["laeufe"] += 1
                    fuer_datei[klasse] += 1
                    if klasse == "unkontrolliert":
                        brueche.append(f"{kurz}: Byte {stelle} -> "
                                       f"0x{wert:02X}: {text}")
            print(f"      {kurz:52} {fuer_datei['laeufe']:6} Laeufe | "
                  f"Strukturhalt {fuer_datei['strukturhalt']:6} | "
                  f"Gegenprobe unmoeglich {fuer_datei['gegenprobe_unmoeglich']:5} | "
                  f"Befund {fuer_datei['befund']:5} | "
                  f"gruen {fuer_datei['gruen']:5} | "
                  f"UNKONTROLLIERT {fuer_datei['unkontrolliert']}")
    finally:
        globals()["artefakt_hash"] = echter_hash
    dauer = time.perf_counter() - beginn

    manifest_nachher = hashlib.sha256(MANIFEST.read_bytes()).hexdigest()
    pruefe(manifest_vorher == manifest_nachher,
           "[3c/1] der gefuzzte --hashen-Zweig hat das Manifest im Repo nicht "
           f"angefasst - sha256 {manifest_vorher[:16]}; sein Schreibziel ist "
           f"die Kopie {_fuzz_hashziel()}",
           f"{manifest_vorher[:16]} -> {manifest_nachher[:16]}")

    for zeile in brueche[:8]:
        print("      BRUCH " + zeile)
    if len(brueche) > 8:
        print(f"      ... und {len(brueche) - 8} weitere")
    pruefe(zaehler["unkontrolliert"] == 0,
           f"[3c] {len(dateien)} gelesene JSON-Datei(en), {bytes_gesamt} "
           f"gekippte Byte-Stellen, {zaehler['laeufe']} Laeufe: KEINE Ausnahme "
           f"ausser Strukturhalt "
           f"({zaehler['strukturhalt']}) und dem eigenen 'Gegenprobe "
           f"unmoeglich' ({zaehler['gegenprobe_unmoeglich']}); "
           f"Befund {zaehler['befund']}, gruen {zaehler['gruen']}"
           + ("" if voll else f"; Sample jedes {schritt}. Byte plus jedes "
                              f"erste Schluesselnamensbyte"),
           (f"{zaehler['unkontrolliert']} unkontrollierte Ausnahme(n)"
            if brueche else f"{dauer:.1f}s"))


def _argumente(argv=None):
    p = argparse.ArgumentParser(description=__doc__)
    g = p.add_mutually_exclusive_group()
    g.add_argument("--hashen", action="store_true",
                   help="Release-Schritt: sha256 aus den gebauten Artefakten festschreiben")
    g.add_argument("--release", action="store_true",
                   help="Auslieferungsschritt: [4] vergleicht HART gegen die festgeschriebenen "
                        "Hashes (Exit 2 bei Abweichung). Ohne dieses Flag ist eine Abweichung "
                        "ein Hinweis - nach einem Relink ist sie der Normalfall (NAK-94).")
    p.add_argument("--fuzz-voll", action="store_true",
                   help="[3c] ueber JEDES Byte jeder gelesenen JSON-Datei statt "
                        "ueber das Kanon-Sample (Minuten statt Sekunden)")
    p.add_argument("--fuzz-schritt", type=int, default=FUZZ_SCHRITT,
                   metavar="N",
                   help=f"Schrittweite des [3c]-Samples (Vorgabe {FUZZ_SCHRITT}); "
                        "die Schluesselnamensbytes sind immer dabei")
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
    """DIE eine kontrollierte Huelle - EINMAL geschrieben, ZWEIMAL benutzt.

    BEFUND DES ELFTEN PRUEFERS (NAK-94 Nacharbeit 11): bis dahin hatte der
    Fuzz seine EIGENE Ausnahmebehandlung. `_fuzz_einmal()` rief
    `_fuzz_verbraucher()` direkt und fing selbst ab; `main()` lief im Fuzz nie.
    Der zentrale Faenger liess sich also entfernen, ohne dass [3c] rot wurde -
    gemessen @ 4fcb4a8: mit auf Durchreichen gestelltem `main()` blieb
    derselbe Fuzz-Fall unveraendert 'gruen'. Eine Wache, die nur eine Kopie
    ihrer selbst misst, beweist nichts ueber das Original.

    Seitdem legt DIESE Funktion sich in `main()` um den ganzen Lauf und in
    `[3c]` um jeden einzelnen Fuzz-Fall. Wer sie auf Durchreichen stellt,
    macht damit zwangslaeufig den Fuzz rot.

    Rueckgabe: (Klasse, Klartext, Wert). `Wert` ist bei "gruen" der Rueckgabe-
    wert von `fn`, sonst die gefangene Ausnahme.

      "gruen"                 - durchgelaufen, kein Abbruch
      "strukturhalt"          - Vertragsbruch einer gelesenen JSON-Datei
      "gegenprobe_unmoeglich" - SystemExit: der eigene, bereits kontrollierte
                                Ausgang aus [3]/[3b]. Er traegt seinen
                                Klartext selbst und ist KEINE unerwartete
                                Ausnahme; `main()` reicht ihn deshalb weiter.
      "unkontrolliert"        - jede andere Ausnahme. NUR sie ist der Bruch
                                der Zusage.
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
    """Zentraler Faenger (NAK-94 Nacharbeit 10, Wegwechsel W3).

    Die Strukturpruefungen bleiben die erste Verteidigung; sie sagen im
    Klartext, WELCHE Datei welche Form nicht traegt. Was ihnen entgeht, endet
    hier: eine Klartextzeile mit Ausnahmetyp, Meldung, Datei und Zeile des
    Ausloesers, Exit 2, KEIN Traceback auf stdout oder stderr. Den Traceback
    gibt es nur mit `--debug`.

    Gefangen wird ueber `_geschuetzt()` - dieselbe Funktion, die [3c] um jeden
    Fuzz-Fall legt (NAK-94 Nacharbeit 11). `SystemExit` laeuft absichtlich
    durch: das ist der eigene, bereits kontrollierte Ausgang 'Gegenprobe
    unmoeglich' aus [3]/[3b].
    """
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
    gegenproben_nacharbeit(manifest)

    byte_kipp_fuzz(1 if args.fuzz_voll else max(1, args.fuzz_schritt))

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
