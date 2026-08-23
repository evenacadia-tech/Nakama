#!/usr/bin/env py -3.13
"""S9-Nacharbeit 23.08.2026 - Kanon-Bein A18: der Gegenpfad, GEFAHREN.

WARUM ES DIESES BEIN GIBT

  Hausinvariante: installieren <-> Rollback, beide Haelften im selben
  Aenderungssatz. S9 hat beide Haelften geschrieben - aber niemand hat die
  zweite je AUSGEFUEHRT. Der T2-Pruefer sagte das selbst (Manifest
  SONDE-007b §5.9): "Nur gelesen und gegen den Vertrag gemessen. Weder
  installiert noch zurueckgerollt."

  Beim ersten wirklichen Lauf fielen zwei echte Fehler heraus, die dem Lesen
  entgangen waren:

    1. `$zieleNachId[$null]` - PowerShell wirft bei einem leeren
       Hashtable-Schluessel. Der Broker-Eintrag traegt berechtigterweise
       keine ziel_id, also starb der Rueckweg ab der ZWEITEN Installation.
    2. Ein verweigerter Rueckweg (NAK-41) schrieb `status: ERROR` in
       install-ergebnis.json und zerstoerte damit die Quelle, aus der die
       angebotene Wiederholung mit -Erzwingen haette lesen muessen. Der
       Ausweg, den der Riegel nennt, war nach EINER Verweigerung zu.

  Eine Zusage, die nie ausgefuehrt wird, ist keine Zusage. Darum laeuft der
  Gegenpfad ab jetzt im Kanon mit.

WAS ES NICHT TUT

  Es installiert NICHTS. Die Installation bleibt ein Klick des Users mit
  Rechteerhoehung (NAK-32/NAK-41). Gefahren wird das ECHTE
  Install-Nakama.ps1 in einer Sandbox unter %TEMP%, deren Manifest
  `ziele.*` dorthin zeigt - kein Testschalter im Skript, keine
  Rechteerhoehung, kein Program Files.

DER EINE UNTERSCHIED, offen protokolliert

  Riegel 3 ("Ruhe") verweigert, solange FL Studio laeuft. In der Sandbox
  haelt kein Host ein Bundle, und ob der Kanon gruen ist, darf nicht davon
  abhaengen, ob der User gerade FL offen hat. Die Sandbox-Kopie ersetzt
  darum GENAU EINE Zeile, und dieses Bein
    * behauptet vorher, dass die Zeile im Original steht (fehlt sie, faellt
      das Bein - der Riegel darf nicht unbemerkt verschwinden),
    * druckt die Ersetzung als Zweizeilen-Diff mit.
"""

from __future__ import annotations

import json
import pathlib
import shutil
import subprocess
import sys
import tempfile

WURZEL = pathlib.Path(__file__).resolve().parents[2]
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

from pruefe_installer_manifest import datei_hash, ordner_hash  # noqa: E402

INSTALL = WURZEL / "eq-copilot" / "install"
IDENTITAET = WURZEL / "eq-copilot" / "identity" / "plugin-identities-v1.json"

RUHE_ALT = "    $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)"
RUHE_NEU = "    $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle"

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


def bundle_bauen(ordner: pathlib.Path, version: str, binaer: bytes, extra=None) -> None:
    (ordner / "Contents" / "Resources").mkdir(parents=True, exist_ok=True)
    (ordner / "Contents" / "x86_64-win").mkdir(parents=True, exist_ok=True)
    (ordner / "Contents" / "Resources" / "moduleinfo.json").write_text(
        json.dumps({"Name": ordner.stem, "Version": version}) + "\n", encoding="utf-8")
    (ordner / "Contents" / "x86_64-win" / ordner.name).write_bytes(binaer)
    if extra:
        (ordner / extra[0]).write_bytes(extra[1])


def main() -> int:
    pwsh = shutil.which("pwsh") or shutil.which("powershell")
    if pwsh is None:
        pruefe(False, "pwsh gefunden - ohne PowerShell laesst sich der Ausfuehrende nicht fahren")
        print(f"\n{ok} ok, {len(fehler)} Fehler")
        return 2

    def lauf(skript, *argumente):
        e = subprocess.run([pwsh, "-NoProfile", "-File", str(skript), *argumente],
                           capture_output=True, text=True)
        return e.returncode, (e.stdout or "") + (e.stderr or "")

    with tempfile.TemporaryDirectory(prefix="nakama-gegenpfad-") as tmp:
        sand = pathlib.Path(tmp)
        inst = sand / "eq-copilot" / "install"
        inst.mkdir(parents=True)
        (sand / "eq-copilot" / "identity").mkdir(parents=True)
        shutil.copy2(INSTALL / "Install-Nakama.ps1", inst)
        shutil.copy2(INSTALL / "NakamaOrdnerHash.ps1", inst)
        shutil.copy2(IDENTITAET, sand / "eq-copilot" / "identity")

        print("[0] Sandbox-Kopie des ECHTEN Skripts, genau eine Zeile ersetzt")
        quelle = (inst / "Install-Nakama.ps1").read_text(encoding="utf-8-sig")
        if not pruefe(RUHE_ALT in quelle,
                      "Riegel 3 ('Ruhe') steht unveraendert im Original"):
            print(f"\n{ok} ok, {len(fehler)} Fehler")
            return 2
        (inst / "Install-Nakama.ps1").write_text(quelle.replace(RUHE_ALT, RUHE_NEU, 1),
                                                 encoding="utf-8")
        print(f"      - {RUHE_ALT.strip()}")
        print(f"      + {RUHE_NEU.strip()}")

        manifest = json.loads((INSTALL / "nakama-installer-v1.json").read_text(encoding="utf-8"))
        zielVst3 = sand / "ziel" / "VST3"
        zielBroker = sand / "ziel" / "programme" / "evenacadia" / "Nakama"
        zielVst3.mkdir(parents=True)          # existiert (wie Common Files\VST3)
        # zielBroker existiert BEWUSST NICHT - der Installer muss die Kette anlegen.
        manifest["ziele"]["vst3_verzeichnis"] = zielVst3.as_posix()
        manifest["ziele"]["broker_verzeichnis"] = zielBroker.as_posix()

        ident = json.loads(IDENTITAET.read_text(encoding="utf-8"))
        bundles = {z["id"]: z["bundle"] for z in ident["ziele"]}

        # "Gebaute" Artefakte an genau den Pfaden, die das Manifest nachrechnet.
        # Bewusst KEIN echtes Bundle: dieses Bein misst die Mechanik des
        # Gegenpfades, nicht die Binaerdateien - und laeuft damit auch ohne Bau.
        for a in manifest["artefakte"]:
            q = sand / a["quelle"]
            if a["art"] == "vst3":
                bundle_bauen(q, "0.3.0", b"NEU-" + a["ziel_id"].encode())
                a["sha256"] = ordner_hash(q)
            else:
                q.parent.mkdir(parents=True, exist_ok=True)
                q.write_bytes(b"broker-neu")
                a["sha256"] = datei_hash(q)

        # Vorgefundener Stand: NUR das Main-Bundle ist installiert - mit altem
        # moduleinfo.json UND einer Datei, die der neue Bau nicht mehr hat.
        altesMain = zielVst3 / bundles["main"]

        def vorzustand():
            if (sand / "ziel").exists():
                shutil.rmtree(sand / "ziel")
            zielVst3.mkdir(parents=True)
            bundle_bauen(altesMain, "0.1.0", b"ALT-main",
                         extra=("Contents/altmuell.txt", b"Rest aus einem frueheren Bau"))
            (inst / "install-ergebnis.json").unlink(missing_ok=True)

        vorzustand()
        altInnen = datei_hash(altesMain / "Contents" / "x86_64-win" / bundles["main"])
        altOrdner = ordner_hash(altesMain)

        def schreibe_manifest():
            (inst / "nakama-installer-v1.json").write_text(
                json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

        schreibe_manifest()
        skript = inst / "Install-Nakama.ps1"

        print("\n[1] Installieren")
        code, aus = lauf(skript)
        pruefe(code == 0 and "INSTALLATION OK" in aus,
               "Installationslauf endet mit Exit 0", f"Exit {code}")

        print("\n[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)")
        for zid, bname in bundles.items():
            pruefe((zielVst3 / bname / "Contents" / "Resources" / "moduleinfo.json").is_file(),
                   f"{zid}: Bundle traegt moduleinfo.json", bname)
        pruefe(json.loads((zielVst3 / bundles["main"] / "Contents/Resources/moduleinfo.json")
                          .read_text())["Version"] == "0.3.0",
               "main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands")
        pruefe(not (zielVst3 / bundles["main"] / "Contents" / "altmuell.txt").exists(),
               "main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt")
        pruefe((zielBroker / "eqcop-broker.exe").is_file(),
               "Broker liegt im selbst angelegten Verzeichnis")

        print("\n[3] -Pruefen sieht den ganzen Ordner (T2-5 c)")
        code, aus = lauf(skript, "-Pruefen")
        pruefe(aus.count("aktuell") == 4, "alle vier Artefakte melden `aktuell`",
               f"{aus.count('aktuell')}x")
        (zielVst3 / bundles["main"] / "Contents/Resources/moduleinfo.json").write_text(
            '{"Name":"EQ-Copilot","Version":"0.0.0"}\n', encoding="utf-8")
        code, aus = lauf(skript, "-Pruefen")
        # Die Binaerdatei ist unveraendert - die alte Pfadformel haette hier
        # weiter `aktuell` gesagt. Genau das ist Befund T2-5 (c).
        pruefe("ABWEICHEND" in aus, "eine Aenderung NUR am moduleinfo.json faellt auf")

        print("\n[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg")
        vorzustand()
        code, _ = lauf(skript)
        pruefe(code == 0, "frischer Installationslauf ueber den Vorstand", f"Exit {code}")
        code, aus = lauf(skript, "-Rueckweg")
        pruefe(code == 1 and "NAK-41" in aus, "Rueckweg verweigert, NAK-41 benannt", f"Exit {code}")
        pruefe("unbekannt" in aus, "der unbekannte Vorstand zaehlt wie aelter")
        # Und der Ausweg, den die Verweigerung anbietet, muss offen BLEIBEN.
        pruefe(json.loads((inst / "install-ergebnis.json").read_text())["status"] == "OK",
               "die verweigerte Wiederherstellung hat ihre eigene Quelle NICHT zerstoert")

        print("\n[5] Das datei-innen-Nachschlagen findet einen historischen Stand")
        pruefe(altInnen != altOrdner, "Ordner-Hash und Dateihash des Vorstands sind verschieden",
               f"innen {altInnen[:12]} | ordner {altOrdner[:12]}")
        manifest["rueckweg"]["bekannte_staende"].append({
            "sha256": altInnen, "hash_art": "datei-innen", "ziel_id": "main",
            "state_schema": 2, "beschreibung": "Sandbox-Vorstand, gegen die Binaerdatei eingefroren"})
        schreibe_manifest()
        # Eine fremde Datei in einem selbst angelegten Verzeichnis: sie muss es halten.
        (zielBroker / "fremd.txt").write_bytes(b"von jemand anderem")
        code, aus = lauf(skript, "-Rueckweg")
        pruefe(code == 0 and "RUECKWEG OK" in aus,
               "Rueckweg laeuft jetzt ohne -Erzwingen durch", f"Exit {code}")

        print("\n[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)")
        pruefe((zielVst3 / bundles["main"] / "Contents" / "altmuell.txt").is_file(),
               "main: auch die Datei des Vorstands ist zurueck - der ganze Ordner")
        pruefe(ordner_hash(zielVst3 / bundles["main"]) == altOrdner,
               "main: bytegleich zum Vorzustand", altOrdner[:16])
        for zid in ("passive-probe", "active-probe"):
            pruefe(not (zielVst3 / bundles[zid]).exists(),
                   f"{zid}: KEIN leeres .vst3-Gehaeuse zurueckgeblieben", bundles[zid])
        pruefe(not (zielBroker / "eqcop-broker.exe").exists(), "Broker entfernt")
        pruefe(zielBroker.is_dir() and (zielBroker / "fremd.txt").is_file(),
               "das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen")

        print("\n[7] Leer geraeumt verschwindet die selbst angelegte Kette")
        shutil.rmtree(sand / "ziel" / "programme")
        pruefe(not zielBroker.exists(), "Vorzustand: die Broker-Kette gibt es nicht")
        code, _ = lauf(skript)
        pruefe(code == 0 and zielBroker.is_dir(), "der naechste Lauf legt die Kette selbst an")
        lauf(skript, "-Rueckweg", "-Erzwingen")
        pruefe(not zielBroker.exists() and not (sand / "ziel" / "programme").exists(),
               "die ganze selbst angelegte Kette ist weg")
        pruefe(zielVst3.is_dir(), "das VORGEFUNDENE Zielverzeichnis bleibt unangetastet")

        print("\n[8] Der art-Riegel im Ausfuehrenden (T2-4)")
        manifest["artefakte"].append({"art": "standalone", "name": "Nakama.exe",
                                      "quelle": "irgendwo/Nakama.exe",
                                      "sha256": "0" * 64})
        schreibe_manifest()
        code, aus = lauf(skript, "-Pruefen")
        pruefe(code == 1 and "Unbekannte Artefaktsorte" in aus,
               "eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen",
               f"Exit {code}")

    print(f"\n{ok} ok, {len(fehler)} Fehler")
    if fehler:
        print("\nFEHLGESCHLAGEN:")
        for f in fehler:
            print("  - " + f)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
