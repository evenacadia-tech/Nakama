#!/usr/bin/env py -3.13
"""NAK-94 Nacharbeit 5 (30.08.2026): Probe-Journale vom WRITER erzeugen.

WARUM ES DIESEN ERZEUGER GIBT

  Drei Pruefrunden hintereinander fanden dieselbe Klasse: ein von Hand
  geschriebenes Journal, das "in der Form des Writers" sein sollte, wich an
  je einem weiteren Feld ab.

    Nacharbeit 2/3  `quelle` - ein Feld, das kein Writer schreibt.
    Nacharbeit 4    Transaktions-ID mit Bindestrichen; `Ist-TransaktionsId`
                    prueft gegen ^[0-9a-f]{32}$ und haette sie abgewiesen.
    Nacharbeit 5    `vorher_sha256` + `gesichert` gesetzt, aber
                    `vorher_sha256_innen` null - Install-Nakama.ps1 bricht
                    bei einem vst3-Ziel mit vorhandenem Vorzustand ab, bevor
                    es ein solches Journal ueberhaupt schreiben koennte.

  Die Ursache ist nicht Unaufmerksamkeit, sondern die Aufgabe selbst: der
  Writer traegt mehr aneinander gebundene Invarianten, als ein Leser
  aufzaehlen kann. Deshalb wird die Form ab jetzt nicht mehr nachgeschrieben,
  sondern ERZEUGT und byteweise eingefroren (Wegwechsel W1 des Dirigenten,
  Manifest SONDE-007c, "Dirigentenstand NAK-94 ... Pruefer 5").

WIE

  In derselben %TEMP%-Sandbox, die Kanon-Bein A18
  (tools/eq-copilot/pruefe_installer_gegenpfad.py) fuer den Gegenpfad
  benutzt: eine Kopie des ECHTEN Install-Nakama.ps1 mit genau drei ersetzten
  Zeilen (Host-Riegel und die zwei fest verdrahteten Produktziele), ein
  Manifest, dessen Ziele in die Sandbox zeigen, und "gebaute" Artefakte an
  den Pfaden, die das Manifest nachrechnet. Die drei Ersetzungszeilen werden
  aus A18 IMPORTIERT, nicht abgeschrieben - aendert A18 sie, folgt dieser
  Erzeuger.

  INSTALLIERT WIRD NICHTS. Kein Program Files, keine Rechteerhoehung, kein
  Testschalter im Installer.

WAS EINGEFROREN WIRD

  eq-copilot/fixtures/installer/journale/<fall>.json  - die Bytes, die
  Schreibe-JsonAtomar geschrieben hat (UTF-8 ohne BOM, ConvertTo-Json,
  CRLF - deshalb `-text` in .gitattributes), dazu MANIFEST.json mit Fall,
  Stand, Installer-Befehl und SHA-256 je Datei.

  Volatile Felder (zeit, transaktions_id, Pfade, Hashes) bleiben, wie der
  Writer sie schrieb. Die Proben in pruefe_installer_manifest.py [3b]
  vergleichen Struktur und Status, nicht diese Werte - eine Fixtur, die
  einen Zeitstempel festschreibt, waere beim naechsten Lauf falsch, ohne
  dass sich etwas geaendert haette.

WAS HIER NICHT ENTSTEHT

  VORBEREITET, KOMPENSATION, ERROR_TEILSTAND und RUECKWEG_AKTIV sind
  Durchgangsstaende: der Writer schreibt sie, ueberschreibt sie im selben
  Lauf aber wieder. Ein abgeschlossener Lauf hinterlaesst sie nicht, und
  A18 simuliert ihren Abbruch nicht deterministisch - es SETZT sie von Hand
  in den Anker. Sie sind darum keine Writer-Fixturen, sondern deklarierte
  Mutanten (`mutant_von`/`abweichung`) und werden in [3b] aus genau einer
  Writer-Fixtur abgeleitet.

Aufrufe:
  py -3.13 tools/eq-copilot/erzeuge_installer_journale.py            # erzeugen
  py -3.13 tools/eq-copilot/erzeuge_installer_journale.py --pruefen  # Hashes nachrechnen

Exitcodes: 0 gruen · 2 rot (Erzeugung fehlgeschlagen, Hash weicht ab, eine im
MANIFEST gefuehrte Datei fehlt, eine verwaiste Datei liegt daneben oder eine
Statusklasse aus JOURNAL_PFLICHTSTATUS fehlt im Korpus).
"""

from __future__ import annotations

import hashlib
import json
import pathlib
import shutil
import subprocess
import sys
import tempfile

WURZEL = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

# Aus A18 importiert, nicht abgeschrieben: dieselben drei Ersetzungen, dieselbe
# Bundle-Attrappe. Aendert A18 seine Sandbox, folgt dieser Erzeuger.
from pruefe_installer_gegenpfad import (  # noqa: E402
    BROKER_POLICY_ALT, RUHE_ALT, RUHE_NEU, VST_POLICY_ALT, bundle_bauen,
)
from pruefe_installer_manifest import (  # noqa: E402
    JOURNAL_PFLICHTSTATUS, datei_hash, ordner_hash,
)

INSTALL = WURZEL / "eq-copilot" / "install"
IDENTITAET = WURZEL / "eq-copilot" / "identity" / "plugin-identities-v1.json"
JOURNALE = WURZEL / "eq-copilot" / "fixtures" / "installer" / "journale"
MANIFEST_WEG = JOURNALE / "MANIFEST.json"

FIXTUR_SCHEMA = "nakama.installer-journale/v1"


def _sha256(bytes_: bytes) -> str:
    return hashlib.sha256(bytes_).hexdigest().upper()


def _stand() -> str:
    lauf = subprocess.run(["git", "-C", str(WURZEL), "rev-parse", "--short=7", "HEAD"],
                          capture_output=True, text=True)
    return lauf.stdout.strip() if lauf.returncode == 0 else "unbekannt"


def pruefen() -> int:
    """Nur nachrechnen: liegen die eingefrorenen Bytes so, wie MANIFEST sagt?

    Bewusst OHNE pwsh und ohne Sandbox - diese Haelfte muss ueberall laufen,
    auch dort, wo kein Installerlauf moeglich ist. Sie beantwortet die Frage
    "hat jemand eine Fixtur von Hand angefasst?", nicht "ist die Mechanik
    noch dieselbe".

    BEFUND NAK-94, Pruefer 6 (30.08.2026): eine im MANIFEST gefuehrte, aber
    fehlende Datei war hier schon rot - eine Fixtur, die ZUSAMMEN mit ihrer
    MANIFEST-Zeile verschwindet, dagegen nicht. Am Stand `165d9ae` gemessen:
    ohne `error-rueckgerollt.json` und ohne ihren Eintrag lief `--pruefen` mit
    Exit 0 durch und meldete drei Fixturen als vollstaendig. Deshalb haengt
    diese Haelfte jetzt an derselben Statusachse wie [3b]
    (`JOURNAL_PFLICHTSTATUS`, importiert statt abgeschrieben): sie steht
    ausserhalb des Korpus und laesst sich nicht mit ihm loeschen.
    """
    if not MANIFEST_WEG.is_file():
        print(f"FEHLER  kein MANIFEST: {MANIFEST_WEG}")
        return 2
    manifest = json.loads(MANIFEST_WEG.read_text(encoding="utf-8"))
    klagen: list[str] = []
    genannt = set()
    for fall in manifest["faelle"]:
        weg = JOURNALE / fall["datei"]
        genannt.add(fall["datei"])
        if not weg.is_file():
            klagen.append(f"{fall['datei']}: fehlt")
            continue
        ist = _sha256(weg.read_bytes())
        if ist != fall["sha256"]:
            klagen.append(f"{fall['datei']}: SHA-256 {ist} != {fall['sha256']}")
            continue
        kopf = json.loads(weg.read_text(encoding="utf-8-sig"))
        if kopf.get("status") != fall["status"]:
            klagen.append(f"{fall['datei']}: Status {kopf.get('status')!r} "
                          f"!= {fall['status']!r}")
            continue
        print(f"  ok      {fall['datei']}  status={fall['status']}  {ist[:16]}")
    # Eine verwaiste Datei ist genauso ein Befund wie eine fehlende: sie
    # koennte von Hand danebengelegt und von einer Probe gelesen werden.
    for weg in sorted(JOURNALE.glob("*.json")):
        if weg.name != "MANIFEST.json" and weg.name not in genannt:
            klagen.append(f"{weg.name}: liegt da, steht aber in keinem MANIFEST-Fall")
    # Derselbe Anker wie in [3b]: eine ganze Statusklasse darf nicht samt
    # ihrer MANIFEST-Zeile verschwinden, ohne dass es jemand sagt.
    vorhanden = {fall.get("status") for fall in manifest["faelle"]}
    for pflicht in JOURNAL_PFLICHTSTATUS:
        if pflicht not in vorhanden:
            klagen.append(f"kein Fall mit Journalstatus {pflicht} - der Korpus "
                          "traegt die Zusagen von [3b] nicht mehr")
    if klagen:
        print("\nFEHLGESCHLAGEN:")
        for k in klagen:
            print("  - " + k)
        return 2
    print(f"\n{len(manifest['faelle'])} Writer-Fixturen bytegleich zum MANIFEST "
          f"(Stand der Erzeugung: {manifest['stand']}).")
    return 0


def erzeugen() -> int:
    pwsh = shutil.which("pwsh") or shutil.which("powershell")
    if pwsh is None:
        print("FEHLER  pwsh nicht gefunden - ohne PowerShell laeuft der Writer nicht")
        return 2

    eingefroren: list[dict] = []

    def lauf(skript, *argumente):
        e = subprocess.run([pwsh, "-NoProfile", "-File", str(skript), *argumente],
                           capture_output=True, text=True)
        return e.returncode, (e.stdout or "") + (e.stderr or "")

    with tempfile.TemporaryDirectory(prefix="nakama-journale-") as tmp:
        sand = pathlib.Path(tmp)
        zielVst3 = sand / "ziel" / "VST3"
        zielBroker = sand / "ziel" / "programme" / "evenacadia" / "Nakama"
        inst = sand / "eq-copilot" / "install"
        inst.mkdir(parents=True)
        (sand / "eq-copilot" / "identity").mkdir(parents=True)
        shutil.copy2(INSTALL / "Install-Nakama.ps1", inst)
        shutil.copy2(INSTALL / "NakamaOrdnerHash.ps1", inst)
        shutil.copy2(IDENTITAET, sand / "eq-copilot" / "identity")

        print("[0] Sandbox-Kopie des ECHTEN Skripts, genau drei Zeilen ersetzt")
        quelle = (inst / "Install-Nakama.ps1").read_text(encoding="utf-8-sig")
        vst_policy_neu = ("$erlaubteVst3Basis = Kanonischer-Pfad '"
                          + str(zielVst3).replace("'", "''") + "'")
        broker_policy_neu = ("$erlaubteBrokerBasis = Kanonischer-Pfad '"
                             + str(zielBroker).replace("'", "''") + "'")
        ersetzungen = ((RUHE_ALT, RUHE_NEU),
                       (VST_POLICY_ALT, vst_policy_neu),
                       (BROKER_POLICY_ALT, broker_policy_neu))
        # Dieselbe fail-closed-Vorbedingung wie in A18: findet sich eine
        # Originalzeile nicht GENAU einmal, wird nichts erzeugt. Sonst
        # entstuende eine Fixtur aus einem Installer, der anders aussieht als
        # der ausgelieferte - und niemand saehe es der Datei an.
        for alt, _neu in ersetzungen:
            if quelle.count(alt) != 1:
                print(f"FEHLER  Originalzeile steht nicht genau einmal: {alt.strip()[:70]}")
                return 2
        for alt, neu in ersetzungen:
            quelle = quelle.replace(alt, neu, 1)
            print(f"      - {alt.strip()}")
            print(f"      + {neu.strip()}")
        (inst / "Install-Nakama.ps1").write_text(quelle, encoding="utf-8")
        skript = inst / "Install-Nakama.ps1"

        manifest = json.loads((INSTALL / "nakama-installer-v1.json")
                              .read_text(encoding="utf-8"))
        zielVst3.mkdir(parents=True)      # existiert (wie Common Files\VST3)
        # zielBroker existiert BEWUSST NICHT - der Installer legt die Kette an.
        manifest["ziele"]["vst3_verzeichnis"] = zielVst3.as_posix()
        manifest["ziele"]["broker_verzeichnis"] = zielBroker.as_posix()

        ident = json.loads(IDENTITAET.read_text(encoding="utf-8"))
        bundles = {z["id"]: z["bundle"] for z in ident["ziele"]}
        aktivBundles = {z["id"]: z["bundle"] for z in ident["ziele"]
                        if "stillgelegt" not in z}
        oeffentlichesJournal = inst / "install-ergebnis.json"

        for a in manifest["artefakte"]:
            q = sand / a["quelle"]
            if a["art"] == "vst3":
                bundle_bauen(q, "0.3.0", b"NEU-" + a["ziel_id"].encode())
                a["sha256"] = ordner_hash(q)
            else:
                q.parent.mkdir(parents=True, exist_ok=True)
                q.write_bytes(b"broker-neu")
                a["sha256"] = datei_hash(q)

        altesMain = zielVst3 / bundles["main"]

        def vorzustand():
            if (sand / "ziel").exists():
                shutil.rmtree(sand / "ziel")
            zielVst3.mkdir(parents=True)
            bundle_bauen(altesMain, "0.1.0", b"ALT-main",
                         extra=("Contents/altmuell.txt", b"Rest aus einem frueheren Bau"))
            oeffentlichesJournal.unlink(missing_ok=True)

        vorzustand()
        altInnen = datei_hash(altesMain / "Contents" / "x86_64-win" / bundles["main"])

        # Damit der Rueckweg ohne -Erzwingen durchlaeuft: der Sandbox-Vorstand
        # wird als bekannter Stand eingefroren, genau wie A18 es in seinem
        # Block [5] tut. Ohne ihn verweigert NAK-41 ("unbekannt zaehlt wie
        # aelter") - und ein erzwungener Rueckweg waere eine andere Form
        # (`erzwungen: true`) als die, die ein User normalerweise erzeugt.
        manifest["rueckweg"]["bekannte_staende"].append({
            "sha256": altInnen, "hash_art": "datei-innen", "ziel_id": "main",
            "state_schema": 2,
            "beschreibung": "Sandbox-Vorstand, gegen die Binaerdatei eingefroren"})

        def schreibe_manifest():
            (inst / "nakama-installer-v1.json").write_text(
                json.dumps(manifest, ensure_ascii=False, indent=2) + "\n",
                encoding="utf-8")

        schreibe_manifest()

        def einfrieren(datei: str, fall: str, befehl: str, erwartet: str,
                       beschreibung: str) -> bool:
            rohe = oeffentlichesJournal.read_bytes()
            kopf = json.loads(rohe.decode("utf-8-sig"))
            if kopf.get("status") != erwartet:
                print(f"  FEHLER  {fall}: Status {kopf.get('status')!r}, "
                      f"erwartet {erwartet!r} - nichts eingefroren")
                return False
            (JOURNALE / datei).write_bytes(rohe)
            eingefroren.append({
                "datei": datei, "fall": fall, "befehl": befehl,
                "status": erwartet, "beschreibung": beschreibung,
                "sha256": _sha256(rohe), "bytes": len(rohe),
            })
            print(f"  ok      {fall}  ->  {datei}  ({len(rohe)} B, "
                  f"{_sha256(rohe)[:16]})")
            return True

        JOURNALE.mkdir(parents=True, exist_ok=True)
        gut = True

        print("\n[1] OK nach Erstinstallation ueber einen vorgefundenen Stand")
        code, aus = lauf(skript)
        if code != 0 or "INSTALLATION OK" not in aus:
            print(f"  FEHLER  Installationslauf endete mit Exit {code}\n{aus[-800:]}")
            return 2
        gut &= einfrieren(
            "ok-erstinstallation.json", "OK nach Erstinstallation",
            "Install-Nakama.ps1", "OK",
            "Vorgefunden war NUR das Main-Bundle (aelterer Stand). main traegt "
            "deshalb vorher_sha256, vorher_sha256_innen und gesichert; die "
            "uebrigen Ziele lagen nicht vor und tragen dort null.")

        print("\n[2] RUECKWEG nach dem Gegenpfad")
        code, aus = lauf(skript, "-Rueckweg")
        if code != 0 or "RUECKWEG OK" not in aus:
            print(f"  FEHLER  Rueckweglauf endete mit Exit {code}\n{aus[-800:]}")
            return 2
        gut &= einfrieren(
            "rueckweg-nach-gegenpfad.json", "RUECKWEG nach Gegenpfad",
            "Install-Nakama.ps1 -Rueckweg", "RUECKWEG",
            "Abschliessendes Schreibe-Ergebnis des RUECKWEG-Zweigs: sieben "
            "Felder, KEINE Liste eintraege - der regulaer abgeschlossene "
            "Rueckweg fuehrt sie nicht.")

        print("\n[3] OK nach Tausch (zweiter Lauf ueber den installierten Stand)")
        code, aus = lauf(skript)
        if code != 0:
            print(f"  FEHLER  erster Lauf des Tauschpaares: Exit {code}\n{aus[-800:]}")
            return 2
        code, aus = lauf(skript)
        if code != 0 or "INSTALLATION OK" not in aus:
            print(f"  FEHLER  zweiter Lauf des Tauschpaares: Exit {code}\n{aus[-800:]}")
            return 2
        gut &= einfrieren(
            "ok-nach-tausch.json", "OK nach Tausch",
            "Install-Nakama.ps1 (zweiter Lauf ueber den installierten Stand)",
            "OK",
            "Jetzt lag JEDES Ziel vor: alle Eintraege tragen einen "
            "Vorzustand samt Sicherung, die vst3-Eintraege zusaetzlich "
            "vorher_sha256_innen.")

        print("\n[4] ERROR_RUECKGEROLLT nach spaetem Abbruch mit voller Kompensation")
        vorzustand()
        blocker = zielBroker
        blocker.parent.mkdir(parents=True)
        blocker.write_bytes(b"kein Verzeichnis")
        code, aus = lauf(skript)
        if code != 1 or "kompensiert" not in aus:
            print(f"  FEHLER  erwartet war ein kompensierter Abbruch, Exit {code}\n{aus[-800:]}")
            return 2
        gut &= einfrieren(
            "error-rueckgerollt.json", "ERROR_RUECKGEROLLT nach voller Kompensation",
            "Install-Nakama.ps1 (Broker-Ziel durch eine Datei blockiert)",
            "ERROR_RUECKGEROLLT",
            "Eine Datei liegt dort, wo das Broker-Verzeichnis entstehen "
            "muesste. Der Fehler faellt nach dem dritten Tauschakt; der "
            "Writer kompensiert rueckwaerts und schreibt jeden Eintrag mit "
            "rollback_abgeschlossen fort.")

        if not gut:
            return 2

    stand = _stand()
    MANIFEST_WEG.write_text(json.dumps({
        "schema": FIXTUR_SCHEMA,
        "erzeugt_von": "tools/eq-copilot/erzeuge_installer_journale.py",
        "stand": stand,
        "writer": "eq-copilot/install/Install-Nakama.ps1",
        "sandbox": (
            "A18-Mechanik (tools/eq-copilot/pruefe_installer_gegenpfad.py): "
            "Kopie des echten Installers unter %TEMP% mit genau drei ersetzten "
            "Zeilen (Host-Riegel, VST3-Produktpolicy, Broker-Produktpolicy), "
            "Manifest-Ziele in die Sandbox gebogen. Es wurde nichts installiert."
        ),
        "volatile_felder": ["zeit", "transaktions_id", "ziel", "gesichert",
                            "sha256", "vorher_sha256", "vorher_sha256_innen",
                            "erzeugte_ordner", "getan", "bekannte_staende"],
        "hinweis": (
            "Bytes wie Schreibe-JsonAtomar sie geschrieben hat (UTF-8 ohne BOM, "
            "ConvertTo-Json, CRLF). .gitattributes haelt sie mit -text "
            "bytegleich. Die Proben in [3b] vergleichen Struktur und Status, "
            "nicht die volatilen Werte."
        ),
        "faelle": eingefroren,
    }, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(f"\n{len(eingefroren)} Writer-Journale eingefroren unter "
          f"{JOURNALE.relative_to(WURZEL).as_posix()} (Stand {stand}).")
    return 0


def main() -> int:
    if "--pruefen" in sys.argv[1:]:
        print("[pruefen] eingefrorene Writer-Journale gegen ihr MANIFEST")
        return pruefen()
    return erzeugen()


if __name__ == "__main__":
    sys.exit(main())
