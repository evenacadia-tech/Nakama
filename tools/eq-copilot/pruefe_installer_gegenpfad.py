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
  Install-Nakama.ps1 in einer Sandbox unter %TEMP%, deren Manifest und die
  beiden kopierten Produktpolicy-Zeilen dorthin zeigen - kein Testschalter im
  Skript, keine Rechteerhoehung, kein Program Files.

DIE DREI UNTERSCHIEDE, offen protokolliert

  Riegel 3 ("Ruhe") verweigert, solange FL Studio laeuft. In der Sandbox
  haelt kein Host ein Bundle, und ob der Kanon gruen ist, darf nicht davon
  abhaengen, ob der User gerade FL offen hat. Die Sandbox-Kopie ersetzt
  darum GENAU DREI Zeilen: den Host-Riegel sowie die zwei fest verdrahteten
  Produktziele. Dieses Bein behauptet vorher, dass jede Originalzeile exakt
  einmal steht, und druckt alle Ersetzungen als Zweizeilen-Diff mit.
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
VST_POLICY_ALT = "$erlaubteVst3Basis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::CommonProgramFiles)) 'VST3')"
BROKER_POLICY_ALT = "$erlaubteBrokerBasis = Kanonischer-Pfad (Join-Path ([Environment]::GetFolderPath([Environment+SpecialFolder]::ProgramFiles)) 'evenacadia\\Nakama')"

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
        vst_policy_neu = "$erlaubteVst3Basis = Kanonischer-Pfad '" + str(zielVst3).replace("'", "''") + "'"
        broker_policy_neu = "$erlaubteBrokerBasis = Kanonischer-Pfad '" + str(zielBroker).replace("'", "''") + "'"
        ersetzungen = (
            (RUHE_ALT, RUHE_NEU, "Riegel 3 ('Ruhe') steht unveraendert im Original"),
            (VST_POLICY_ALT, vst_policy_neu, "kanonische VST3-Produktpolicy steht unveraendert im Original"),
            (BROKER_POLICY_ALT, broker_policy_neu, "kanonische Broker-Produktpolicy steht unveraendert im Original"),
        )
        policy_pruefungen = [pruefe(quelle.count(alt) == 1, text)
                             for alt, _neu, text in ersetzungen]
        if not all(policy_pruefungen):
            print(f"\n{ok} ok, {len(fehler)} Fehler")
            return 2
        for alt, neu, _text in ersetzungen:
            quelle = quelle.replace(alt, neu, 1)
            print(f"      - {alt.strip()}")
            print(f"      + {neu.strip()}")
        (inst / "Install-Nakama.ps1").write_text(quelle, encoding="utf-8")

        manifest = json.loads((INSTALL / "nakama-installer-v1.json").read_text(encoding="utf-8"))
        zielVst3.mkdir(parents=True)          # existiert (wie Common Files\VST3)
        # zielBroker existiert BEWUSST NICHT - der Installer muss die Kette anlegen.
        manifest["ziele"]["vst3_verzeichnis"] = zielVst3.as_posix()
        manifest["ziele"]["broker_verzeichnis"] = zielBroker.as_posix()

        ident = json.loads(IDENTITAET.read_text(encoding="utf-8"))
        # S9b/SONDE-007c: `bundles` fuehrt weiter JEDE Kennung - auch die
        # stillgelegte, denn Block [3b] braucht ihren Bundlenamen. Alles, was
        # eine INSTALLATION erwartet, laeuft dagegen ueber `aktivBundles`.
        # Beides zu vermischen war der Fehler, den dieses Bein beim ersten
        # Lauf nach der Stilllegung sofort gemeldet hat: es verlangte ein
        # installiertes "Nakama Suna.vst3", das niemand mehr ausliefert.
        bundles = {z["id"]: z["bundle"] for z in ident["ziele"]}
        aktivBundles = {z["id"]: z["bundle"] for z in ident["ziele"]
                        if "stillgelegt" not in z}
        # Die Ziele neben dem Main-Bundle standen bis zum 28.08.2026 als
        # ("passive-probe", "active-probe") in fuenf Schleifen. Das war eine
        # abgeschriebene Liste - sie verlangte nach der Stilllegung ein
        # Bundle, das niemand mehr ausliefert. Jetzt kommt sie aus derselben
        # Datei wie alles andere (S9b/SONDE-007c).
        nebenZiele = [zid for zid in aktivBundles if zid != "main"]
        assert nebenZiele, "ohne ein Ziel neben main misst dieses Bein die halbe Mechanik nicht"
        oeffentlichesJournal = inst / "install-ergebnis.json"
        transaktionsOrdner = zielBroker.parent / ".nakama-installer"
        transaktionsWeg = transaktionsOrdner / "aktive-transaktion.json"
        sicherungsWurzel = transaktionsOrdner / "backups"
        repoRueckweg = inst / "rueckweg"
        repoRueckweg.mkdir()
        repoSentinel = repoRueckweg / "NICHT-AUTORITATIV.txt"
        repoSentinel.write_bytes(b"nur diagnose, niemals backupquelle")

        def lese_transaktion():
            return json.loads(transaktionsWeg.read_text(encoding="utf-8-sig"))

        def schreibe_transaktion(stand, auch_oeffentlich=False):
            text = json.dumps(stand, ensure_ascii=False, indent=2) + "\n"
            transaktionsWeg.write_text(text, encoding="utf-8")
            if auch_oeffentlich:
                oeffentlichesJournal.write_text(text, encoding="utf-8")

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
            oeffentlichesJournal.unlink(missing_ok=True)

        vorzustand()
        altInnen = datei_hash(altesMain / "Contents" / "x86_64-win" / bundles["main"])
        altOrdner = ordner_hash(altesMain)

        def schreibe_manifest():
            (inst / "nakama-installer-v1.json").write_text(
                json.dumps(manifest, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")

        schreibe_manifest()
        skript = inst / "Install-Nakama.ps1"

        print("\n[0b] Nicht rueckwegfaehiges Fremdbundle faellt vor der Installation")
        kaputtePayload = altesMain / "Contents" / "x86_64-win" / bundles["main"]
        kaputtePayload.unlink()
        kaputterVorstand = ordner_hash(altesMain)
        code, aus = lauf(skript)
        pruefe(code == 1 and "keine beweisbare erwartete innere Payload" in aus,
               "Bundle ohne erwartete innere Payload wird fail-closed verweigert",
               f"Exit {code}")
        pruefe(ordner_hash(altesMain) == kaputterVorstand
               and not transaktionsWeg.exists(),
               "Verweigerung laesst Fremdbundle unveraendert und erzeugt keinen aktiven Anker")
        vorzustand()

        print("\n[1] Installieren")
        code, aus = lauf(skript)
        pruefe(code == 0 and "INSTALLATION OK" in aus,
               "Installationslauf endet mit Exit 0", f"Exit {code}: {aus[-500:]}")

        ersterAnker = lese_transaktion()
        geschuetzteBackups = [transaktionsOrdner / e["gesichert"]
                              for e in ersterAnker["eintraege"] if e["gesichert"]]
        pruefe(transaktionsOrdner.parent != inst.parent and not transaktionsOrdner.is_relative_to(inst),
               "geschuetzter Anker liegt getrennt von Repo-Diagnose und Repo-rueckweg")
        pruefe(all(p.is_relative_to(
            sicherungsWurzel / ersterAnker["transaktions_id"]) for p in geschuetzteBackups),
            "jede Sicherung ist relativ an backups/<transaktions-id> gebunden")
        pruefe(repoSentinel.read_bytes() == b"nur diagnose, niemals backupquelle",
               "Repo-rueckweg bleibt beim erhoehten Installationspfad unangetastet")
        pruefe(not oeffentlichesJournal.read_bytes().startswith(b"\xef\xbb\xbf")
               and not transaktionsWeg.read_bytes().startswith(b"\xef\xbb\xbf"),
               "beide atomaren JSON-Veroeffentlichungen sind UTF-8 ohne BOM")
        pruefe(not list(inst.glob("install-ergebnis.json.tmp-*"))
               and not list(transaktionsOrdner.glob("aktive-transaktion.json.tmp-*"))
               and not (inst / "nakama-installer.lock").exists(),
               "wiederholte Veroeffentlichung hinterlaesst weder Temp noch autoritativen Repo-Lock")

        print("\n[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)")
        for zid, bname in aktivBundles.items():
            pruefe((zielVst3 / bname / "Contents" / "Resources" / "moduleinfo.json").is_file(),
                   f"{zid}: Bundle traegt moduleinfo.json", bname)
        pruefe(json.loads((zielVst3 / bundles["main"] / "Contents/Resources/moduleinfo.json")
                          .read_text())["Version"] == "0.3.0",
               "main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands")
        pruefe(not (zielVst3 / bundles["main"] / "Contents" / "altmuell.txt").exists(),
               "main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt")
        pruefe((zielBroker / "eqcop-broker.exe").is_file(),
               "Broker liegt im selbst angelegten Verzeichnis")

        print("\n[2b] Aktiver Recovery-Anker wird nicht still superseded")
        zielHashes = [ordner_hash(zielVst3 / aktivBundles[zid]) for zid in aktivBundles]
        lockWeg = transaktionsOrdner / "installer.lock"
        with lockWeg.open("a+b"):
            code, aus = lauf(skript)
        pruefe(code == 1 and "mutationsfaehiger Nakama-Installerlauf" in aus,
               "geschuetzter Lock sperrt auch einen Lauf aus einer anderen Repo-Kopie",
               f"Exit {code}")
        lockWeg.unlink()
        for status in ("VORBEREITET", "KOMPENSATION", "ERROR_TEILSTAND", "RUECKWEG_AKTIV"):
            offenerAnker = json.loads(json.dumps(ersterAnker))
            offenerAnker["status"] = status
            schreibe_transaktion(offenerAnker)
            ankerBytes = transaktionsWeg.read_bytes()
            code, aus = lauf(skript)
            pruefe(code == 1 and "Erst -Rueckweg" in aus,
                   f"{status} verlangt Rueckweg statt neuem Journalstart", f"Exit {code}")
            pruefe(transaktionsWeg.read_bytes() == ankerBytes
                   and zielHashes == [ordner_hash(zielVst3 / aktivBundles[zid]) for zid in aktivBundles],
                   f"{status}: verweigerter Neustart laesst Anker und Ziele bytegleich")

        schreibe_transaktion(ersterAnker)
        alteId = ersterAnker["transaktions_id"]
        code, aus = lauf(skript)
        zweiterAnker = lese_transaktion()
        pruefe(code == 0 and zweiterAnker["transaktions_id"] != alteId,
               "OK-Anker wird erst durch eine neue vollstaendig gesicherte Transaktion superseded",
               f"Exit {code}")
        backupIds = sorted(p.name for p in sicherungsWurzel.iterdir() if p.is_dir())
        pruefe(backupIds == [zweiterAnker["transaktions_id"]],
               "Supersede entfernt alte per-Transaktion-Backups ohne den aktiven Stand zu verlieren")
        pruefe(repoSentinel.read_bytes() == b"nur diagnose, niemals backupquelle"
               and not list(transaktionsOrdner.glob("aktive-transaktion.json.tmp-*")),
               "zweite Veroeffentlichung nutzt weder Repo-rueckweg noch Temp-Reste")

        print("\n[3] -Pruefen sieht den ganzen Ordner (T2-5 c)")
        code, aus = lauf(skript, "-Pruefen")
        # Die Zahl kommt aus dem Manifest, nicht aus dem Gedaechtnis
        # (S9b/SONDE-007c): mit der Stilllegung von Suna sind es zwei Bundles
        # plus Broker statt drei plus Broker. Ein Test, der die alte Zahl
        # festschreibt, misst eine Wunschvorstellung statt den gebauten Stand.
        sollArtefakte = len(manifest["artefakte"])
        pruefe(aus.count("aktuell") == sollArtefakte,
               f"alle {sollArtefakte} Artefakte melden `aktuell`",
               f"{aus.count('aktuell')}x")
        (zielVst3 / bundles["main"] / "Contents/Resources/moduleinfo.json").write_text(
            '{"Name":"EQ-Copilot","Version":"0.0.0"}\n', encoding="utf-8")
        code, aus = lauf(skript, "-Pruefen")
        # Die Binaerdatei ist unveraendert - die alte Pfadformel haette hier
        # weiter `aktuell` gesagt. Genau das ist Befund T2-5 (c).
        pruefe("ABWEICHEND" in aus, "eine Aenderung NUR am moduleinfo.json faellt auf")

        # ── S9b/SONDE-007c: die Altlast-Zusage wird AUSGEFUEHRT ────────────
        # Das Manifest sagt "melden, nicht loeschen". Eine Zusage, die nie
        # gefahren wird, ist keine Zusage - genau die Lehre, die dieses Bein
        # ueberhaupt entstehen liess (S9-Nacharbeit 23.08.). Also: das
        # stillgelegte Bundle wirklich hinlegen, messen was das Skript sagt,
        # und messen dass es danach noch da ist.
        stillgelegte = [z for z in ident["ziele"] if "stillgelegt" in z]
        if stillgelegte:
            print("\n[3b] Ein stillgelegtes Bundle wird gemeldet, nicht geloescht")
            altlast = zielVst3 / bundles[stillgelegte[0]["id"]]
            bundle_bauen(altlast, "0.0.1", b"ALT-stillgelegt")
            vorher = ordner_hash(altlast)
            code, aus = lauf(skript, "-Pruefen")
            pruefe("ALTLAST" in aus and str(altlast) in aus,
                   "das vorgefundene stillgelegte Bundle wird mit vollem Pfad gemeldet")
            pruefe("Remove-Item -Recurse -Force" in aus,
                   "die Meldung nennt den Handgriff, statt nur zu klagen")
            pruefe(altlast.is_dir() and ordner_hash(altlast) == vorher,
                   "und der Installer hat es NICHT angefasst - bytegleich vorgefunden")
            shutil.rmtree(altlast)
            code, aus = lauf(skript, "-Pruefen")
            pruefe("ALTLAST" not in aus and "stillgelegt seit" in aus,
                   "ohne Altbestand bleibt die Zeile ehrlich (`nicht installiert`)")
        else:
            pruefe(True, "kein stillgelegtes Ziel in der Identitaetsdatei - nichts zu melden")

        # ── S9b/SONDE-007c: der Fall OHNE stillgelegtes Ziel ───────────────
        # Der T1-Selbstaudit fand ihn durch Lesen, nicht durch Laufen:
        # `@() | Sort-Object` liefert $null, und `Compare-Object $null $null`
        # bricht ab - unter $ErrorActionPreference='Stop' ein harter Fehler.
        # Der Riegel feuerte heute nicht, weil es genau ein stillgelegtes Ziel
        # gibt; er feuerte an dem Tag, an dem das letzte verschwindet. Ein
        # gelesener Fehler ist erst geschlossen, wenn sein Fall gefahren wird.
        if stillgelegte:
            print("\n[3c] Eine Auslieferung ganz OHNE stillgelegtes Ziel laeuft weiter")
            identWeg = sand / "eq-copilot" / "identity" / IDENTITAET.name
            identAlt = identWeg.read_bytes()
            manifestAlt = (inst / "nakama-installer-v1.json").read_bytes()
            try:
                ohne = json.loads(identAlt.decode("utf-8"))
                ohne["ziele"] = [z for z in ohne["ziele"] if "stillgelegt" not in z]
                identWeg.write_text(json.dumps(ohne, ensure_ascii=False, indent=2) + "\n",
                                    encoding="utf-8")
                ohneManifest = json.loads(manifestAlt.decode("utf-8"))
                ohneManifest.pop("stillgelegte_ziele", None)
                (inst / "nakama-installer-v1.json").write_text(
                    json.dumps(ohneManifest, ensure_ascii=False, indent=2) + "\n",
                    encoding="utf-8")
                code, aus = lauf(skript, "-Pruefen")
                pruefe(code == 0 and "ALTLAST" not in aus and "stillgelegt" not in aus,
                       "ohne stillgelegtes Ziel laeuft -Pruefen durch und schweigt darueber",
                       f"Exit {code}")
            finally:
                identWeg.write_bytes(identAlt)
                (inst / "nakama-installer-v1.json").write_bytes(manifestAlt)

        # ── Nacharbeit Runde 1 (29.08.2026, T2-Befund P1 Nr. 1) ───────────
        # Eine Stilllegungsmarke, die vorhanden, aber kein Objekt ist. Bis
        # dahin fragte der Installer `$null -ne $_.stillgelegt` - und eine
        # Property mit JSON-`null` liefert denselben `$null` wie eine
        # FEHLENDE Property. Das Ziel zaehlte damit als AKTIV: fail-OPEN an
        # der Stelle, die fail-closed heisst. Gemessen wird hier die neue
        # Zusage: Marke vorhanden = stillgelegt, kaputter Inhalt = harter
        # Abbruch mit eigener Meldung - nie ein Ruecksprung nach "aktiv".
        # Der zweite Teil ist der gefaehrliche Fall: Marke unlesbar UND ein
        # Artefakteintrag da. Genau diese Kombination haette ein
        # stillgelegtes Bundle wieder ausgeliefert.
        if stillgelegte:
            print("\n[3d] Eine unlesbare Stilllegungsmarke sperrt, statt 'aktiv' zu heissen")
            identWeg = sand / "eq-copilot" / "identity" / IDENTITAET.name
            identAlt = identWeg.read_bytes()
            manifestAlt = (inst / "nakama-installer-v1.json").read_bytes()
            try:
                for marke, name in ((None, "null"), ("weg", "String"),
                                    ([], "Array"), (7, "Zahl")):
                    kaputt = json.loads(identAlt.decode("utf-8"))
                    for z in kaputt["ziele"]:
                        if z["id"] == stillgelegte[0]["id"]:
                            z["stillgelegt"] = marke
                    identWeg.write_text(json.dumps(kaputt, ensure_ascii=False, indent=2) + "\n",
                                        encoding="utf-8")
                    code, aus = lauf(skript, "-Pruefen")
                    pruefe(code == 1 and "unlesbare Stilllegungsmarke" in aus,
                           f"Marke vom Typ {name} bricht ab und benennt sich",
                           f"Exit {code}")

                # Und derselbe Fall MIT geschmuggeltem Artefakteintrag: die
                # Marke muss immer noch sperren, bevor irgendetwas kopiert
                # wird. Die Kennung ist eingefroren - der Bundlename kommt
                # deshalb aus der Identitaetsdatei, nicht aus diesem Skript.
                kaputt = json.loads(identAlt.decode("utf-8"))
                for z in kaputt["ziele"]:
                    if z["id"] == stillgelegte[0]["id"]:
                        z["stillgelegt"] = None
                identWeg.write_text(json.dumps(kaputt, ensure_ascii=False, indent=2) + "\n",
                                    encoding="utf-8")
                geschmuggelt = json.loads(manifestAlt.decode("utf-8"))
                vorlage = next(a for a in geschmuggelt["artefakte"] if a.get("art") == "vst3")
                schmuggel = dict(vorlage)
                schmuggel["ziel_id"] = stillgelegte[0]["id"]
                schmuggel["sha256"] = None
                geschmuggelt["artefakte"].append(schmuggel)
                geschmuggelt.pop("stillgelegte_ziele", None)
                (inst / "nakama-installer-v1.json").write_text(
                    json.dumps(geschmuggelt, ensure_ascii=False, indent=2) + "\n",
                    encoding="utf-8")
                code, aus = lauf(skript, "-Pruefen")
                pruefe(code == 1 and "unlesbare Stilllegungsmarke" in aus,
                       "auch MIT geschmuggeltem Artefakteintrag sperrt die Marke zuerst",
                       f"Exit {code}")
            finally:
                identWeg.write_bytes(identAlt)
                (inst / "nakama-installer-v1.json").write_bytes(manifestAlt)

        print("\n[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg")
        vorzustand()
        code, _ = lauf(skript)
        pruefe(code == 0, "frischer Installationslauf ueber den Vorstand", f"Exit {code}")
        code, aus = lauf(skript, "-Rueckweg")
        pruefe(code == 1 and "NAK-41" in aus, "Rueckweg verweigert, NAK-41 benannt", f"Exit {code}")
        pruefe("unbekannt" in aus, "der unbekannte Vorstand zaehlt wie aelter")
        # Und der Ausweg, den die Verweigerung anbietet, muss offen BLEIBEN.
        pruefe(json.loads(oeffentlichesJournal.read_text())["status"] == "OK"
               and lese_transaktion()["status"] == "OK",
               "die verweigerte Wiederherstellung hat Diagnose UND geschuetzten Anker nicht zerstoert")

        print("\n[5] Das datei-innen-Nachschlagen findet einen historischen Stand")
        pruefe(altInnen != altOrdner, "Ordner-Hash und Dateihash des Vorstands sind verschieden",
               f"innen {altInnen[:12]} | ordner {altOrdner[:12]}")
        manifest["rueckweg"]["bekannte_staende"].append({
            "sha256": altInnen, "hash_art": "datei-innen", "ziel_id": "main",
            "state_schema": 2, "beschreibung": "Sandbox-Vorstand, gegen die Binaerdatei eingefroren"})
        schreibe_manifest()
        code, aus = lauf(skript, "-Rueckweg")
        pruefe(code == 1 and "NAK-41" in aus,
               "spaete Repo-Manifestaenderung kann die geschuetzte NAK-41-Einstufung nicht umschreiben")
        vorzustand()
        code, _ = lauf(skript)
        pruefe(code == 0, "frische Transaktion friert den nun bekannten historischen Stand ein")
        autoritat = lese_transaktion()
        repoFakeBackup = repoRueckweg / "angreifer-main.bundle"
        bundle_bauen(repoFakeBackup, "ANGREIFER", b"NICHT-WIEDERHERSTELLEN")
        fakeHash = ordner_hash(repoFakeBackup)
        repoManipuliert = json.loads(json.dumps(autoritat))
        repoManipuliert["eintraege"][0]["gesichert"] = (
            "eq-copilot/install/rueckweg/angreifer-main.bundle")
        repoManipuliert["eintraege"][0]["vorher_sha256"] = fakeHash
        oeffentlichesJournal.write_text(json.dumps(repoManipuliert), encoding="utf-8")
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
        pruefe(ordner_hash(repoFakeBackup) == fakeHash and repoSentinel.is_file(),
               "manipuliertes Repo-Journal/Repo-rueckweg beeinflusst Restore und Schreibpfad nicht")
        for zid in nebenZiele:
            pruefe(not (zielVst3 / aktivBundles[zid]).exists(),
                   f"{zid}: KEIN leeres .vst3-Gehaeuse zurueckgeblieben", aktivBundles[zid])
        pruefe(not (zielBroker / "eqcop-broker.exe").exists(), "Broker entfernt")
        pruefe(zielBroker.is_dir() and (zielBroker / "fremd.txt").is_file(),
               "das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen")

        print("\n[7] Leer geraeumt verschwindet nur das beweisbar eigene Ziel")
        shutil.rmtree(sand / "ziel" / "programme")
        pruefe(not zielBroker.exists(), "Vorzustand: die Broker-Kette gibt es nicht")
        code, _ = lauf(skript)
        pruefe(code == 0 and zielBroker.is_dir(), "der naechste Lauf legt die Kette selbst an")
        lauf(skript, "-Rueckweg", "-Erzwingen")
        pruefe(not zielBroker.exists(), "das im Journal belegte Broker-Ziel ist weg")
        pruefe((sand / "ziel" / "programme").is_dir(),
               "unbelegte Elternverzeichnisse bleiben unangetastet")
        pruefe(zielVst3.is_dir(), "das VORGEFUNDENE Zielverzeichnis bleibt unangetastet")

        print("\n[8] Fehler nach drei Tauschakten wird vollstaendig kompensiert")
        vorzustand()
        blocker = zielBroker
        blocker.parent.mkdir(parents=True)
        blocker.write_bytes(b"kein Verzeichnis")
        code, aus = lauf(skript)
        pruefe(code == 1 and "kompensiert" in aus,
               "spaeter Installationsfehler endet laut und nicht als Teilerfolg", f"Exit {code}")
        journal = json.loads((inst / "install-ergebnis.json").read_text())
        pruefe(journal["status"] == "ERROR_RUECKGEROLLT",
               "Journal bestaetigt die vollstaendige Kompensation", journal["status"])
        pruefe(ordner_hash(altesMain) == altOrdner,
               "Main ist nach der Kompensation bytegleich zum Vorzustand")
        for zid in nebenZiele:
            pruefe(not (zielVst3 / aktivBundles[zid]).exists(),
                   f"{zid}: kein Teilstand nach spaetem Fehler")
        pruefe(blocker.is_file() and blocker.read_bytes() == b"kein Verzeichnis",
               "der fremde Blocker wurde nicht angetastet")

        fehlerAnker = lese_transaktion()
        alteFehlerId = fehlerAnker["transaktions_id"]
        pruefe(fehlerAnker["status"] == "ERROR_RUECKGEROLLT"
               and not (sicherungsWurzel / alteFehlerId).exists(),
               "vollstaendig kompensierter Endstand gibt seine nutzlosen Backups frei")
        blocker.unlink()
        code, aus = lauf(skript)
        neuerAnker = lese_transaktion()
        pruefe(code == 0 and neuerAnker["transaktions_id"] != alteFehlerId,
               "ERROR_RUECKGEROLLT darf nach sicherem Cleanup explizit superseded werden",
               f"Exit {code}")
        pruefe(sorted(p.name for p in sicherungsWurzel.iterdir() if p.is_dir())
               == [neuerAnker["transaktions_id"]],
               "nach Fehler-Supersede bleibt nur die aktive Backup-Generation")

        print("\n[9] Harter Abbruch in Kopiere-Stand bleibt rueckwegfaehig")
        vorzustand()
        code, _ = lauf(skript)
        pruefe(code == 0, "Ausgangsinstallation fuer den Hard-Crash-Zwischenstand")
        crashJournal = lese_transaktion()
        for index, eintrag in enumerate(crashJournal["eintraege"]):
            eintrag["mutation_begonnen"] = index == 0
            eintrag["mutation_abgeschlossen"] = False
            eintrag["rollback_abgeschlossen"] = False
        crashJournal["status"] = "VORBEREITET"
        # Exakter Zustand waehrend des ersten rekursiven Copy: Main ist nur
        # halb da, alle spaeteren Ziele wurden noch nicht angefasst.
        shutil.rmtree(altesMain)
        bundle_bauen(altesMain, "HALB", b"PARTIAL-COPY")
        for zid in nebenZiele:
            shutil.rmtree(zielVst3 / aktivBundles[zid])
        shutil.rmtree(zielBroker)
        teilHash = ordner_hash(altesMain)
        schreibe_transaktion(crashJournal, auch_oeffentlich=True)
        code, aus = lauf(skript, "-Rueckweg", "-Erzwingen")
        pruefe(code == 0 and "RUECKWEG OK" in aus,
               "VORBEREITET + mutation_begonnen restauriert statt Drift zu melden", f"Exit {code}")
        pruefe(ordner_hash(altesMain) == altOrdner and ordner_hash(altesMain) != teilHash,
               "der partielle Bundle-Stand ist bytegleich durch den Vorzustand ersetzt")

        vorzustand()
        code, _ = lauf(skript)
        pruefe(code == 0, "Ausgangsinstallation fuer einen unterbrochenen inversen Pfad")
        crashJournal = lese_transaktion()
        crashJournal["status"] = "RUECKWEG_AKTIV"
        # Rueckwaerts wurden Broker und beide Sonden bereits entfernt; Main
        # starb mitten im Restore aus seiner vorhandenen Sicherung.
        for eintrag in crashJournal["eintraege"]:
            eintrag["rollback_abgeschlossen"] = eintrag["ziel_id"] != "main"
        for zid in nebenZiele:
            shutil.rmtree(zielVst3 / aktivBundles[zid])
        shutil.rmtree(zielBroker)
        shutil.rmtree(altesMain)
        bundle_bauen(altesMain, "HALB-RUECKWEG", b"PARTIAL-RESTORE")
        schreibe_transaktion(crashJournal, auch_oeffentlich=True)
        code, aus = lauf(skript, "-Rueckweg", "-Erzwingen")
        pruefe(code == 0 and "RUECKWEG OK" in aus,
               "RUECKWEG_AKTIV setzt einen partiellen inversen Pfad sicher fort", f"Exit {code}")
        pruefe(ordner_hash(altesMain) == altOrdner,
               "auch der unterbrochene inverse Pfad endet bytegleich im Vorzustand")

        print("\n[10] Fremde Journalpfade werden vor dem ersten Schreibzugriff verworfen")
        vorzustand()
        code, _ = lauf(skript)
        pruefe(code == 0, "Ausgangsinstallation fuer Journal-Missbrauchstests")
        originalJournal = lese_transaktion()
        sentinel = sand / "NICHT-ANFASSEN.txt"
        sentinel.write_bytes(b"fremd")

        # Die Repo-Kopie ist nur Diagnose. Selbst ein formal gueltiger fremder
        # Zielpfad dort darf den geschuetzten Plan nicht beeinflussen.
        manipuliert = json.loads(json.dumps(originalJournal))
        manipuliert["eintraege"][0]["ziel"] = sentinel.as_posix()
        oeffentlichesJournal.write_text(json.dumps(manipuliert), encoding="utf-8")
        code, aus = lauf(skript, "-Rueckweg", "-Pruefen", "-Erzwingen")
        pruefe(code == 0 and "wuerde zurueckstellen" in aus and sentinel.read_bytes() == b"fremd",
               "manipulierte Repo-Diagnose ist keine Rueckwegautoritaet", f"Exit {code}")

        schreibe_transaktion(manipuliert)
        code, _ = lauf(skript, "-Rueckweg", "-Erzwingen")
        pruefe(code == 1 and sentinel.read_bytes() == b"fremd",
               "fremdes Ziel selbst im geschuetzten Journal wird nicht beschrieben", f"Exit {code}")

        fremderOrdner = sand / "NICHT-LOESCHEN"
        fremderOrdner.mkdir()
        manipuliert = json.loads(json.dumps(originalJournal))
        manipuliert["eintraege"][0]["erzeugte_ordner"] = [fremderOrdner.as_posix()]
        schreibe_transaktion(manipuliert)
        code, _ = lauf(skript, "-Rueckweg", "-Erzwingen")
        pruefe(code == 1 and fremderOrdner.is_dir(),
               "fremder Loeschpfad im Journal wird nicht entfernt", f"Exit {code}")

        manipuliert = json.loads(json.dumps(originalJournal))
        manipuliert["eintraege"][0]["gesichert"] = "NICHT-ANFASSEN.txt"
        schreibe_transaktion(manipuliert)
        code, _ = lauf(skript, "-Rueckweg", "-Erzwingen")
        pruefe(code == 1 and sentinel.read_bytes() == b"fremd",
               "Sicherung ausserhalb der festen Rueckwegwurzel wird verworfen", f"Exit {code}")

        print("\n[11] Geschuetzte Autoritaet, Backup- und Zieldrift")
        schreibe_transaktion(originalJournal)
        transaktionsWeg.unlink()
        mainNeuHash = ordner_hash(zielVst3 / bundles["main"])
        code, aus = lauf(skript, "-Rueckweg", "-Erzwingen")
        pruefe(code == 1 and "Repo-Diagnose allein" in aus
               and ordner_hash(zielVst3 / bundles["main"]) == mainNeuHash,
               "oeffentliches Journal allein autorisiert keinen Rueckweg", f"Exit {code}")
        schreibe_transaktion(originalJournal)

        mainBackupRel = originalJournal["eintraege"][0]["gesichert"]
        mainBackup = transaktionsOrdner / mainBackupRel
        backupInfo = mainBackup / "Contents" / "Resources" / "moduleinfo.json"
        backupInfo.write_text('{"kaputt":true}\n', encoding="utf-8")
        # Angreifer stimmt die UNGESCHUETZTE Diagnose konsistent auf seine
        # manipulierte Sicherung ab. Der geschuetzte Vorhash bleibt unangetastet.
        oeffentlichManipuliert = json.loads(json.dumps(originalJournal))
        oeffentlichManipuliert["eintraege"][0]["vorher_sha256"] = ordner_hash(mainBackup)
        oeffentlichManipuliert["eintraege"][0]["vorher_sha256_innen"] = datei_hash(
            mainBackup / "Contents" / "x86_64-win" / bundles["main"])
        oeffentlichesJournal.write_text(json.dumps(oeffentlichManipuliert), encoding="utf-8")
        mainNeuHash = ordner_hash(zielVst3 / bundles["main"])
        code, _ = lauf(skript, "-Rueckweg", "-Erzwingen")
        pruefe(code == 1 and ordner_hash(zielVst3 / bundles["main"]) == mainNeuHash,
               "Sicherung + selbstautorisierter Repo-Hash stoppen am geschuetzten Vorhash", f"Exit {code}")

        vorzustand()
        code, _ = lauf(skript)
        pruefe(code == 0, "frische Ausgangsinstallation fuer Zieldrift")
        driftDatei = zielVst3 / bundles["active-probe"] / "Contents" / "Resources" / "moduleinfo.json"
        driftDatei.write_text('{"drift":true}\n', encoding="utf-8")
        driftBytes = driftDatei.read_bytes()
        mainNeuHash = ordner_hash(zielVst3 / bundles["main"])
        code, _ = lauf(skript, "-Rueckweg", "-Erzwingen")
        pruefe(code == 1 and driftDatei.read_bytes() == driftBytes,
               "veraendertes Installationsziel wird nicht ueberschrieben", f"Exit {code}")
        pruefe(ordner_hash(zielVst3 / bundles["main"]) == mainNeuHash,
               "Zieldrift faellt in der Vollvorpruefung auf; Main blieb unangetastet")

        print("\n[12] Der art-Riegel im Ausfuehrenden (T2-4)")
        manifest["artefakte"].append({"art": "standalone", "name": "Nakama.exe",
                                      "quelle": "irgendwo/Nakama.exe",
                                      "sha256": "0" * 64})
        schreibe_manifest()
        code, aus = lauf(skript, "-Pruefen")
        pruefe(code == 1 and "Unbekannte Artefaktsorte" in aus,
               "eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen",
               f"Exit {code}")

        print("\n[13] Runtime bindet Produktziele, Bundlekollision und State-Schema 2")
        manifest["artefakte"].pop()
        schreibe_manifest()
        identityWeg = sand / "eq-copilot" / "identity" / "plugin-identities-v1.json"

        identKaputt = json.loads(json.dumps(ident))
        identKaputt["ziele"][1]["bundle"] = identKaputt["ziele"][0]["bundle"]
        identityWeg.write_text(json.dumps(identKaputt), encoding="utf-8")
        code, aus = lauf(skript, "-Pruefen")
        pruefe(code == 1 and "Bundle-Ziel ist doppelt" in aus,
               "Runtime verwirft eine Bundle-Zielkollision", f"Exit {code}")

        identKaputt = json.loads(json.dumps(ident))
        identKaputt["ziele"][0]["state_schema"] = 1
        identityWeg.write_text(json.dumps(identKaputt), encoding="utf-8")
        code, aus = lauf(skript, "-Pruefen")
        pruefe(code == 1 and "NakamaState-Schema 2" in aus,
               "Runtime bindet Identity typstreng an State-Schema 2", f"Exit {code}")
        identityWeg.write_text(json.dumps(ident), encoding="utf-8")

        brokerProduktziel = manifest["ziele"]["broker_verzeichnis"]
        manifest["ziele"]["broker_verzeichnis"] = brokerProduktziel + "/../Temp/Nakama"
        schreibe_manifest()
        code, aus = lauf(skript, "-Pruefen")
        pruefe(code == 1 and "kanonischen Produktpolicy" in aus,
               "Runtime verwirft kanonisches Broker-Zieltraversal", f"Exit {code}")
        manifest["ziele"]["broker_verzeichnis"] = brokerProduktziel
        schreibe_manifest()

    print(f"\n{ok} ok, {len(fehler)} Fehler")
    if fehler:
        print("\nFEHLGESCHLAGEN:")
        for f in fehler:
            print("  - " + f)
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
