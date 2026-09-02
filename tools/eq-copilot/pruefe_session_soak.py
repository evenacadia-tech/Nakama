#!/usr/bin/env python3
"""G3-SOAK — der Dauerlauf des v3-Sessionwegs (Kanon A24).

Startet den RUST-Probe-Broker auf einer ausschliesslichen Probe-Pipe und faehrt
danach die ECHTEN C++-Clients dagegen: 1 Main mit `subscribe_session` und dem
echten `SourcesModel`, N Sonden mit Control- und Telemetriepaar, dazu je Sonde
ein echter `SondeProcessor` am Ganzblockpfad.

WAS HIER GEMESSEN WIRD
----------------------
Das Phasengate G3 (`docs/bauaufteilung-sonden.md:385`) verlangt einen
60-Minuten-Dauerlauf. Ein Soak ist nicht dasselbe wie das Lastbein A22: A22
misst acht Sekunden Flut, hier geht es darum, dass ueber lange Zeit NICHTS
wegdriftet — keine Mitgliedschaft, keine Adresse, kein Speicher, keine
Antwortzeit — und dass ein Brokerneustart mittendrin den Zustand vollstaendig
und richtig wiederherstellt, statt eine alte Wahrheit weiterzuspielen.

Die Zusagen Z1-Z8, ihre Fristen und die Killmatrix stehen in
`docs/beweise/G3-SOAK.md`. Dieses Skript besitzt:

  * den Pipenamen-Riegel (Zeile S15): beide Programme MUESSEN Produktions- und
    Golden-Pipename mit Exit 3 verweigern, und das wird gefahren, nicht
    behauptet;
  * den Brokerlebenszyklus samt Kill und Neustart, mit den Barrieren aus §6;
  * die minuetliche Speicherkurve beider Prozesse (Z3) ueber
    `GetProcessMemoryInfo` — reine Standardbibliothek, kein psutil;
  * das Urteil ueber alle Zusagen.

WIE HIER GEURTEILT WIRD
-----------------------
Jedes Urteil folgt aus einem Beleg IM BERICHT, keines wird uebernommen. Zwei
Regeln tragen das:

  * ein fehlendes Pflichtfeld ist ROT, nie uebersprungen — eine Schleife ueber
    die tatsaechlich gelieferten Eintraege laeuft bei leerer Liste null Mal und
    bliebe gruen;
  * die Killurteile werden aus ihren Belegfeldern NACHGERECHNET und gegen das
    gelieferte `urteil` gehalten; ein Widerspruch ist ein Befund am Bein.

Beides kam aus der Codex-Abschlusspruefung vom 02.09.2026 (Thread 01a0626a,
Befunde 3, 7, 13, 14, 15, 17).

DIE PIPE
--------
Immer ein Probe-Name mit PID und Zeitstempel. Weder dieses Bein noch eines der
beiden Programme darf die Produktions-Pipe anfassen. Zeile S10 misst das
zusaetzlich am Namensraum: nach dem Start darf kein Produktions- oder
Goldenname NEU im Pipe-Namensraum stehen, und die eigene Probe-Pipe muss dort
sichtbar sein — sonst misst die Auflistung nichts und beweist nichts.

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_session_soak.py
        [--sonden 16] [--minuten 2] [--neustarts 1] [--langsam 0.25]
        [--mutant <name>]   # nur fuer den Rotbeweis, siehe --mutant-liste

Exitcodes: 0 gruen · 2 Zusage verfehlt · 3 Voraussetzung fehlt.
"""

from __future__ import annotations

import argparse
import ctypes
import ctypes.wintypes as wt
import json
import os
import pathlib
import queue
import subprocess
import sys
import threading
import time

WURZEL = pathlib.Path(__file__).resolve().parents[2]
BROKER = WURZEL / "broker/target/release/eqcop-broker-sonde012-probe.exe"
CLIENT = (WURZEL / "eq-copilot/build/plugin/EqCopSessionSoak_artefacts/Release/"
                   "EqCopSessionSoak.exe")
PRODUKTIONS_PIPE = r"\\.\pipe\evenacadia.eq-copilot.v1"
GOLDEN_PIPE = r"\\.\pipe\evenacadia.nakama.v3.BNSM62JZZCCXIDV3PJZAEHMZPA"
# Der EINE Probe-Namensraum, identisch zu broker/src/transport/pipetoken.rs:28.
PROBE_PRAEFIX = r"\\.\pipe\evenacadia.nakama.v3.probe."
# Der Named-Pipe-Namensraum als auflistbares Verzeichnis. `os.listdir` liefert
# die Namen OHNE diesen Praefix.
PIPE_NAMENSRAUM = "//./pipe"
PIPE_PRAEFIX_LAENGE = len(r"\\.\pipe" + "\\")

# Fristen und Budgets — die Zahlen stehen im Manifest §3 mit ihrer Quelle.
FRIST_MS = 60000            # Z2, Manifest §3.1 (Schranke 45.051 ms + Zuschlag)
SCHRANKE_MS = 45051         # gerechnete obere Schranke je Clientpaar
P0_SCHRANKE_MS = 1000       # wie A22, IpcLastMain.cpp:42
BUDGET_PROZENT = 10         # Z3, Manifest §3.2
BUDGET_BYTES = 16 * 1024 * 1024
SPEICHER_TAKT_S = 60
# Eine Endprobe nur, wenn seit der letzten Probe wenigstens so viel Zeit
# verging. Ohne diese Schwelle entstand am Laufende ein zweiter Punkt mit
# identischem RSS unmittelbar nach dem letzten regulaeren — eine
# Wachstumskurve, die S07 nicht falsifizieren konnte (Codex-Befund 2).
SPEICHER_ENDPROBE_MIN_S = 30
TOTZEIT_KS5_S = 21          # > 15,5 s Backoff-Folge, Manifest K-S5
# S08: `FeatureEngine::nimmBlock` baut einen Frame, sobald `kLiveIntervallS`
# Samples im Livefenster stehen (`core/analysis/FeatureEngine.h:445,610-613`).
# Aus Samplerate und Blockgroesse des Laufs folgt daraus, wie viele Ganzbloecke
# hoechstens auf eine Publikation kommen duerfen.
LIVE_INTERVALL_S = 0.1
# Anlauf, Fensterwechsel und der Abbau kosten Publikationen; 0,8 laesst dafuer
# Luft und faellt trotzdem, sobald ein Analyseworker stehenbleibt.
PUBLIKATION_TOLERANZ = 0.8

# Die Pruefpunktnamen aus der Verhaltensmatrix in docs/beweise/G3-SOAK.md §5.
# Jede Matrixzeile nennt dort genau diese Marke; das Laufprotokoll druckt sie
# mit, damit ein Verweis aus dem Manifest im Protokoll wiederzufinden ist.
PRUEFPUNKTE = {
    "S01": "A24:topologie_steht_in_frist",
    "S02": "A24:mitgliedschaft_nach_warmup",
    "S03": "A24:mitgliedschaft_driftet_nicht",
    "S04": "A24:p0_kein_verlust_und_p95",
    "S05": "A24:langsame_bleiben_mitglied",
    "S06": "A24:cap_ersetzt_aeltesten_und_blockiert_nie",
    "S07": "A24:speicherkurve_im_budget",
    "S08": "A24:audio_ganzblock_ohne_drop",
    "S09": "A24:kill_beendet_subscription_sichtbar",
    "S10": "A24:backoff_ohne_namenswechsel",
    "S11": "A24:reconnect_vollstaendig_in_frist",
    "S12": "A24:alte_epoche_kommt_nie_wieder",
    "S13": "A24:kein_stale_ausserhalb_neustart",
    "S14": "A24:sauberer_abbau",
    "S15": "A24:verweigert_produktion_und_golden",
    "S16": "A24:skalierungsreihe_1_4_8_16_32",
}

# Killpunkt -> Pruefpunktname (Killmatrix §6).
KILLPUNKTE = {
    "k_s1": "A24:kill_im_frameverkehr",
    "k_s2": "A24:kill_waehrend_subscribe",
    "k_s3": "A24:kill_waehrend_heartbeat",
    "k_s4": "A24:kill_mit_langsamem_leser",
    "k_s5": "A24:kill_aus_backoff_deckel",
}

MUTANTEN = {
    "s02": "erwartet eine Sonde mehr, als gefahren wird — die Vollstaendigkeit "
           "der Mitgliedschaft faellt (Zeile S02)",
    "s04": "setzt die P0-Latenzschranke auf 0 ms — jede gemessene Latenz faellt "
           "(Zeile S04)",
    "s07": "setzt das Wachstumsbudget auf 0 — jedes Wachstum faellt (Zeile S07)",
    "s11": "setzt die Neustartfrist auf 0 ms — jeder Reconnect faellt (Zeile S11)",
    "s12": "verfaelscht den gemessenen Zaehler `alte_epoche_nach_neustart_gesehen` "
           "auf 1 — die Wache faellt (Zeile S12)",
    "s15": "uebergibt den Produktions-Pipenamen — beide Programme muessen mit "
           "Exit 3 verweigern (Zeile S15)",
    "s03": "loescht eine Brokergeneration bis auf einen Messpunkt — eine "
           "unterabgetastete Generation ist unzureichend, nicht bestanden "
           "(Zeile S07, Abschnitt 3.2)",
    "s08": "setzt `audio.publikationen` auf 0 — ein stehengebliebener "
           "Analyseworker faellt (Zeile S08)",
    "s16": "setzt `langsam_anzahl` auf 0 — die Skalierungsformel "
           "max(1, round(N x Anteil)) faellt (Zeile S16)",
    "kill": "loescht die Killbelege — jedes Killurteil muss aus seinem "
            "Belegfeld folgen und fehlende Pflichteintraege sind rot (§6, §7)",
    "neustart": "loescht die Neustartberichte — S11/S12 duerfen nie vakuos "
                "gruen sein (Zeile S11, S12)",
    "s14": "laesst den Abbau haengen — ein Orchestrierungs-Timeout muss mit "
           "Urteilsbericht und Exit 2 enden, nie mit Exit 1 ohne Bericht "
           "(Zeile S14)",
    # Ab hier: je ein Mutant fuer die 17 Defekte der Codex-Abschlusspruefung
    # vom 02.09.2026 (Thread 01a0626a). Jeder verfaelscht GENAU EINE Groesse.
    "k_s3_beleg": "haengt K-S3 einen Beleg an, den der Schlussbericht nicht "
                  "traegt — die Kreuzprobe gegen `p0.verloren_im_neustartfenster` "
                  "faellt (Defekt 1)",
    "speicherluecke": "zieht alle Speicherproben auf denselben Zeitpunkt — eine "
                      "Kurve ohne echten Minutenabstand faellt (Defekt 2)",
    "s06_grund": "meldet eine Ablehnung ohne gezaehlten Grund — die "
                 "Grundzuordnung faellt (Defekt 4)",
    "s06_fluter": "setzt den Rueckstau der langsamen Sonden auf 0 und laesst "
                  "den Fluter stehen — S06 darf sich nicht beim Fluter "
                  "bedienen (Defekt 5)",
    "s03_sofort": "macht eine Snapshot-Uebernahme unvollstaendig — die "
                  "Sofortpruefung faellt (Defekt 6)",
    "s09_getrennt": "setzt `s09_quellen_getrennt` auf 0 — je Kill muss die "
                    "Quellensicht `getrennt` zeigen (Defekt 8)",
    "s10_define": "meldet den Lauf ohne NAKAMA_PHASE_B_TEST_NO_PRODUCT_V3 und "
                  "einen neuen Produktionsnamen im Namensraum (Defekt 9)",
    "s11_paare": "meldet nur ein Clientpaar in der Reconnect-Verteilung — die "
                 "Verteilung muss Main plus alle Sonden tragen (Defekt 10)",
    "k_s4_widerspruch": "laesst K-S4 `getroffen` melden, obwohl das Flag zum "
                        "Killzeitpunkt 0 war (Defekt 11)",
    "k_s5_totzeit": "meldet den Neustart ohne Totzeiterfassung — K-S1/K-S5 "
                    "muessen waehrend der Totzeit belegt sein (Defekt 12)",
    "s14_nachlauf": "setzt das Nachlauffenster auf 0 ms — ohne Nachlauf ist ein "
                    "offener ACK kein Verlust (Defekt 16)",
}


def kurzname(voller_pipename: str) -> str:
    """Der Name, unter dem eine Pipe im Namensraum auftaucht."""
    return voller_pipename[PIPE_PRAEFIX_LAENGE:]


def pipe_namensraum() -> set[str] | None:
    """Alle offenen Named Pipes dieser Maschine — oder None, wenn nicht lesbar.

    Zeile S10. Register NAK-118 hat den Probe-Riegel genau mit dieser Messung
    falsifiziert: erst die Namensraumauflistung zeigte, dass durch einen
    Traversal-Namen eine Pipe AUSSERHALB des Probe-Raums entstanden war. Der
    C++-Zaehler allein ist kein Beweis — er sieht nur, was das Programm selbst
    an seine Clients uebergibt.

    `None` statt einer leeren Menge ist Absicht: eine fehlgeschlagene
    Auflistung wuerde sonst still beweisen, dass nichts da ist.
    """
    try:
        return set(os.listdir(PIPE_NAMENSRAUM))
    except OSError:
        return None


# ───────────────────────────────────────────────── Speicher (Z3, Zeile S07)

class _PMC(ctypes.Structure):
    _fields_ = [("cb", wt.DWORD), ("PageFaultCount", wt.DWORD),
                ("PeakWorkingSetSize", ctypes.c_size_t),
                ("WorkingSetSize", ctypes.c_size_t),
                ("QuotaPeakPagedPoolUsage", ctypes.c_size_t),
                ("QuotaPagedPoolUsage", ctypes.c_size_t),
                ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t),
                ("QuotaNonPagedPoolUsage", ctypes.c_size_t),
                ("PagefileUsage", ctypes.c_size_t),
                ("PeakPagefileUsage", ctypes.c_size_t)]


_PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
_PROCESS_VM_READ = 0x0010


def rss_bytes(pid: int) -> int:
    """Working Set eines Prozesses. 0, wenn er nicht (mehr) greifbar ist."""
    k32, psapi = ctypes.windll.kernel32, ctypes.windll.psapi
    h = k32.OpenProcess(_PROCESS_QUERY_LIMITED_INFORMATION | _PROCESS_VM_READ,
                        False, pid)
    if not h:
        return 0
    try:
        pmc = _PMC()
        pmc.cb = ctypes.sizeof(_PMC)
        if not psapi.GetProcessMemoryInfo(h, ctypes.byref(pmc), pmc.cb):
            return 0
        return int(pmc.WorkingSetSize)
    finally:
        k32.CloseHandle(h)


# ───────────────────────────────────────────────── Prozesshilfen

class Leser:
    """Liest stdout eines Kindes in einen Thread und legt Zeilen in eine Queue.

    Ohne eigenen Thread blockierte ein `readline()` die Killzeitplanung, und
    ein volles Pipe-Puffer haengte das Kind auf (Landmine: Pipe-Deadlock nach
    Vordergrund-Timeout).
    """

    def __init__(self, strom):
        self.q: "queue.Queue[str]" = queue.Queue()
        self.alle: list[str] = []
        self._t = threading.Thread(target=self._lauf, args=(strom,), daemon=True)
        self._t.start()

    def _lauf(self, strom):
        for zeile in strom:
            z = zeile.rstrip("\r\n")
            self.alle.append(z)
            self.q.put(z)
        self.q.put("")

    def warte_auf(self, praefix: str, frist_s: float) -> str | None:
        ende = time.monotonic() + frist_s
        while time.monotonic() < ende:
            try:
                z = self.q.get(timeout=0.2)
            except queue.Empty:
                continue
            if z.startswith(praefix):
                return z
        return None


def broker_starten(pipe: str, sekunden: int):
    p = subprocess.Popen([str(BROKER), pipe, str(sekunden)], cwd=WURZEL,
                         stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                         stderr=subprocess.PIPE, text=True, encoding="utf-8",
                         errors="replace", bufsize=1)
    bereit = p.stdout.readline().strip() if p.stdout else ""
    if not bereit.startswith("BEREIT"):
        rest = p.stderr.read() if p.stderr else ""
        p.kill()
        raise RuntimeError(f"Probe-Broker meldet nicht BEREIT: {bereit!r} {rest!r}")
    return p


def broker_toeten(p) -> None:
    p.kill()
    try:
        p.wait(timeout=15)
    except subprocess.TimeoutExpired:
        pass


def riegel_faehrt() -> tuple[bool, dict]:
    """Zeile S15: der Riegel wird GEFAHREN, nicht behauptet.

    Liefert die gemessenen Exitcodes mit zurueck, damit `urteile()` auch diesen
    Pruefpunkt aus einem Beleg entscheidet statt aus einem `True`.
    """
    belege: dict = {}
    ok = True
    for name, kurz in ((GOLDEN_PIPE, "golden"), (PRODUKTIONS_PIPE, "produktion")):
        for programm in (BROKER, CLIENT):
            lauf = subprocess.run([str(programm), name], cwd=WURZEL,
                                  capture_output=True, text=True,
                                  encoding="utf-8", errors="replace", timeout=60)
            belege[f"{kurz}_{programm.stem}"] = lauf.returncode
            if lauf.returncode != 3:
                print(f"ROT: {programm.name} nimmt den {kurz}-Pipenamen an "
                      f"(Exit {lauf.returncode})")
                ok = False
    if ok:
        print("  ok      beide Programme verweigern Produktions- und Golden-Pipename")
    return ok, belege


# ───────────────────────────────────────────────── Der Lauf

def fahre(args) -> tuple[int, dict]:
    """Faehrt den Lauf und liefert (exitcode, bericht).

    Ein Exitcode != 0 kommt IMMER mit einer Urteilszeile. Ein Orchestrierungs-
    Timeout ist kein Werkzeugabsturz, sondern ein gebrochenes S14 und wird als
    solches gemeldet (Codex-Befund 17).
    """
    warmup_s = 120 if args.minuten >= 30 else 20
    mess_s = args.minuten * 60
    pipe = f"{PROBE_PRAEFIX}soak.{os.getpid()}.{int(time.time())}"
    assert pipe.startswith(PROBE_PRAEFIX) and len(pipe) > len(PROBE_PRAEFIX)
    assert PRODUKTIONS_PIPE not in pipe and pipe != GOLDEN_PIPE
    print(f"Probe-Pipe: {pipe}")

    # Zeile S10: der Namensraum VOR dem Start. Was hier schon steht, gehoert
    # nicht uns; nur ein danach neu erschienener Produktions- oder Goldenname
    # waere ein Befund.
    namensraum_vor = pipe_namensraum()

    # Grosszuegige Brokerlaufzeit: Warmup + Messzeit + alle Fristen + Reserve.
    broker_s = warmup_s + mess_s + args.neustarts * 120 + 300
    broker = broker_starten(pipe, broker_s)
    klient = subprocess.Popen(
        [str(CLIENT), pipe, str(args.sonden), str(mess_s), str(warmup_s),
         str(int(round(args.langsam * 100))), str(args.langsam_ms)],
        cwd=WURZEL, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.PIPE, text=True, encoding="utf-8", errors="replace",
        bufsize=1)
    aus = Leser(klient.stdout)
    speicher = {"client": [], "broker": []}
    generation = 0
    pipe_messung: dict = {"namensraum_lesbar": namensraum_vor is not None}
    abbau_haenger = ""
    vorbedingung_fehlt = False

    def sag(zeile: str) -> None:
        klient.stdin.write(zeile + "\n")
        klient.stdin.flush()

    def probe(minute: int, sekunden: float, im_fenster: bool) -> None:
        """Eine Speicherprobe beider Prozesse.

        `sekunden` traegt die BEOBACHTETE Messzeit dieser Probe, damit die
        Kurve echte Abstaende traegt und nicht nur eine Minutennummer.
        """
        eintrag = {"minute": minute, "sekunden": round(sekunden, 1),
                   "im_neustartfenster": im_fenster}
        speicher["client"].append({**eintrag, "rss_bytes": rss_bytes(klient.pid)})
        speicher["broker"].append({**eintrag, "generation": generation,
                                   "rss_bytes": rss_bytes(broker.pid)})

    try:
        if aus.warte_auf("TOPOLOGIE_STEHT", FRIST_MS / 1000.0 + 30) is None:
            print("ROT [S01] die Topologie stand nicht in der Frist")
            return 2, {}

        # Zeile S10: jetzt stehen alle Clientverbindungen. Der Namensraum muss
        # unsere Probe-Pipe zeigen (sonst misst die Auflistung nichts) und darf
        # keinen Produktions- oder Goldennamen NEU tragen.
        namensraum_nach = pipe_namensraum()
        if namensraum_vor is not None and namensraum_nach is not None:
            neu = namensraum_nach - namensraum_vor
            pipe_messung.update({
                "namensraum_lesbar": True,
                "eigene_probe_sichtbar": kurzname(pipe) in namensraum_nach,
                "produktionsname_neu": kurzname(PRODUKTIONS_PIPE) in neu,
                "goldenname_neu": kurzname(GOLDEN_PIPE) in neu,
                "namen_vor": len(namensraum_vor),
                "namen_nach": len(namensraum_nach),
            })

        if aus.warte_auf("WARMUP_FERTIG", warmup_s + 60) is None:
            print("ROT [S02] das Warmup endete nicht")
            return 2, {}
        print(f"  ok      Topologie steht, Warmup {warmup_s} s vorbei")

        # Neustart k bei Bruchteil k/(neustarts+1) der Messzeit. Barrieren:
        # Neustart 1 = K-S4, Neustart 2 = K-S2 + K-S5-Totzeit (Manifest §6/§9).
        plan = []
        for k in range(1, args.neustarts + 1):
            art = "k_s4" if k == 1 else ("k_s2" if k == 2 else "plain")
            totzeit = TOTZEIT_KS5_S if k == 2 else 1
            plan.append((k, mess_s * k / (args.neustarts + 1), art, totzeit))

        gemessen = 0.0            # beobachtete Messzeit; Neustartfenster zaehlen nicht
        naechste_probe = 0.0
        letzte_probe_s = 0.0
        minute = 0
        probe(minute, gemessen, False)
        t_letzt = time.monotonic()

        for idx, zielzeit, art, totzeit in plan:
            while gemessen < zielzeit:
                time.sleep(0.5)
                jetzt = time.monotonic()
                gemessen += jetzt - t_letzt
                t_letzt = jetzt
                if gemessen >= naechste_probe + SPEICHER_TAKT_S:
                    naechste_probe += SPEICHER_TAKT_S
                    minute += 1
                    probe(minute, gemessen, False)
                    letzte_probe_s = gemessen

            # Barriere abwarten (Manifest §6). Kommt sie nicht, wird der
            # Killpunkt spaeter als nicht_getroffen berichtet, nie als bestanden.
            # Barrierefenster: erst jetzt flutet die Sonde 0 ihre Schleuse, so
            # dass der verbundene Lauf im Schreibzweig steht (K-S1 Punkt 2).
            # Dauerhaftes Fluten waere kein realistischer Soak und drueckte die
            # P0-Latenz (gemessen im Rauchtest 02.09.2026).
            sag(f"BARRIERE {art}")
            time.sleep(1.5)
            if art == "k_s4":
                # Die Meldung kommt bei jedem Eintritt in die Verzoegerung; im
                # Barrierefenster dauert sie 3 s, also killt der Pruefer sicher
                # hinein.
                aus.warte_auf("LANGSAM_IN_VERZOEGERUNG", 30)
            elif art == "k_s2":
                # Nach einem abgeschlossenen Neustart subscribt der Main von
                # sich aus nicht erneut; ohne erzwungenen Reconnect haette
                # K-S2 gar kein Fenster. `MAIN_RECONNECT` nutzt die bestehende
                # Client-API `ControlClient::reconnect()`.
                sag("MAIN_RECONNECT")
                aus.warte_auf("MAIN_SUBSCRIBE_GESENDET", 60)
            sag(f"KILL_VORBEREITEN {idx} {art}")
            aus.warte_auf("KILL_BEREIT", 30)

            # DER KILL. Erst danach — und mit der gemessenen Killdauer — meldet
            # der Pruefer das Ereignis; das Programm belegt K-S2 und K-S4 gegen
            # diesen Zeitpunkt (Befund 11).
            t_kill = time.monotonic()
            broker_toeten(broker)
            kill_dauer_ms = int((time.monotonic() - t_kill) * 1000)
            sag("BARRIERE_AUS")
            sag(f"KILL_ERFOLGT {idx} {kill_dauer_ms}")
            t_tot = time.monotonic()
            time.sleep(totzeit)

            # K-S1 und K-S5 werden erfasst, WAEHREND der Broker noch tot ist.
            # Nach dem Neustart koennte schon ein erfolgreicher Versuch gegen
            # den neuen Broker den Deckelzaehler fuellen (Befund 12).
            sag(f"TOTZEIT_ENDE {idx}")
            if aus.warte_auf("TOTZEIT_ERFASST", 30) is None:
                print(f"ROT [k_s5] Neustart {idx}: das Programm hat die "
                      f"Totzeitbelege nicht bestaetigt")
                return 2, {}

            broker = broker_starten(pipe, broker_s)
            generation += 1
            totzeit_ms = int((time.monotonic() - t_tot) * 1000)
            sag(f"BEREIT {idx} {totzeit_ms}")
            # Eine Probe WIRKLICH im Neustartfenster: der neue Broker lebt, die
            # Clients reconnecten noch.
            probe(minute, gemessen, True)
            fertig = aus.warte_auf("NEUSTART_VOLLSTAENDIG", FRIST_MS / 1000.0 + 30)
            print(f"  ..      Neustart {idx} ({art}, Totzeit {totzeit_ms} ms): "
                  f"{fertig or 'KEINE MELDUNG'}")
            t_letzt = time.monotonic()          # Neustartfenster zaehlt nicht mit
            # Basiswert der NEUEN Brokergeneration (Manifest §3.2). Ohne diesen
            # Punkt haette die letzte Generation im Kanonlauf nur einen einzigen
            # Messpunkt und waere damit unzureichend.
            probe(minute, gemessen, False)
            letzte_probe_s = gemessen

        while gemessen < mess_s:
            time.sleep(0.5)
            jetzt = time.monotonic()
            gemessen += jetzt - t_letzt
            t_letzt = jetzt
            if gemessen >= naechste_probe + SPEICHER_TAKT_S:
                naechste_probe += SPEICHER_TAKT_S
                minute += 1
                probe(minute, gemessen, False)
                letzte_probe_s = gemessen
        # Eine Endprobe nur, wenn seit der letzten wirklich Zeit verging. Sonst
        # entstuende ein zweiter Punkt mit identischem RSS (Befund 2).
        if gemessen - letzte_probe_s >= SPEICHER_ENDPROBE_MIN_S:
            minute += 1
            probe(minute, gemessen, False)

        sag("ENDE")
        # Das Programm faehrt jetzt sein Nachlauffenster (S14) und baut danach
        # ab; die Frist deckt beides mit Reserve.
        klient.wait(timeout=180)
    except subprocess.TimeoutExpired as f:
        # S14 gebrochen: der Abbau haengt. Das ist eine verfehlte Zusage, kein
        # Werkzeugabsturz — Urteilsbericht und Exit 2, nie Exit 1 ohne Bericht.
        abbau_haenger = f"Abbau haengt: {f}"
        print(f"ROT [S14 · {PRUEFPUNKTE['S14']}] {abbau_haenger}")
    except RuntimeError as f:
        # Der (neue) Broker meldet kein BEREIT. Das ist eine fehlende
        # Voraussetzung (Exit 3, §7) — auch dafuer gibt es eine Urteilszeile
        # und nie einen nackten Exit 1.
        print(f"VORAUSSETZUNG FEHLT [S11] Probe-Broker nicht bereit: {f}")
        vorbedingung_fehlt = True
    finally:
        try:
            if klient.poll() is None:
                klient.kill()
                klient.wait(timeout=30)
        except (OSError, subprocess.TimeoutExpired):
            abbau_haenger = abbau_haenger or "der Klientprozess liess sich nicht beenden"
        try:
            if broker.stdin:
                broker.stdin.write("STOP\n")
                broker.stdin.flush()
            broker.wait(timeout=20)
        except (OSError, subprocess.TimeoutExpired):
            broker_toeten(broker)

    bericht: dict = {}
    for zeile in aus.alle:
        if zeile.strip().startswith("{"):
            try:
                bericht = json.loads(zeile)
            except json.JSONDecodeError:
                pass
    if vorbedingung_fehlt:
        return 3, {}
    if not bericht:
        fehlertext = klient.stderr.read() if klient.stderr else ""
        print(f"ROT [S14 · {PRUEFPUNKTE['S14']}] das C++-Programm lieferte "
              f"keinen Bericht. Exit {klient.returncode}. "
              f"{abbau_haenger} {fehlertext[:900]}")
        return 2, {}

    bericht["speicher"] = {
        "takt_s": SPEICHER_TAKT_S,
        "budget_prozent": BUDGET_PROZENT,
        "budget_bytes": BUDGET_BYTES,
        **speicher,
    }
    bericht["client_exit"] = klient.returncode
    bericht["abbau_haenger"] = abbau_haenger
    # Die Namensraummessung gehoert zu S10 und steht neben dem C++-Zaehler.
    bericht.setdefault("pipe", {}).update(pipe_messung)
    mutiere(bericht, args)
    return 0, bericht


def mutiere(bericht: dict, args) -> None:
    """Rotbeweis: verfaelscht GENAU EINE Groesse im fertigen Bericht.

    Die Mutation greift erst nach dem Lauf an, damit der Lauf selbst
    unveraendert bleibt und wirklich nur die geprueft Groesse faellt.
    """
    if args.mutant == "s07":
        # Das Budget steht IM Bericht, damit auch dieser Mutant ueber
        # `--bericht` an einem gespeicherten Lauf wiederholbar ist.
        bericht["speicher"]["budget_prozent"] = 0
        bericht["speicher"]["budget_bytes"] = 0
    elif args.mutant == "s12":
        for n in bericht.get("neustart", []):
            n["alte_epoche_nach_neustart_gesehen"] = 1
    elif args.mutant == "s03":
        # Eine Brokergeneration auf einen einzigen gueltigen Messpunkt kuerzen.
        gen = {p["generation"] for p in bericht["speicher"]["broker"]}
        ziel = max(gen)
        behalten, erster = [], True
        for p in bericht["speicher"]["broker"]:
            if p["generation"] != ziel or p.get("im_neustartfenster"):
                behalten.append(p)
            elif erster:
                behalten.append(p)
                erster = False
        bericht["speicher"]["broker"] = behalten
    elif args.mutant == "s08":
        bericht["audio"]["publikationen"] = 0
        bericht["audio"]["publikationen_je_sonde_min"] = 0
    elif args.mutant == "s16":
        bericht["langsam_anzahl"] = 0
    elif args.mutant == "kill":
        bericht["kill"] = {}
    elif args.mutant == "neustart":
        bericht["neustart"] = []
    elif args.mutant == "s14":
        bericht["client_exit"] = None
        bericht["abbau_haenger"] = "kuenstlicher Abbau-Haenger (--mutant s14)"
    elif args.mutant == "k_s3_beleg":
        bericht["kill"]["k_s3"]["p0_ohne_ack_im_fenster"] = 7
        bericht["kill"]["k_s3"]["urteil"] = "getroffen"
    elif args.mutant == "speicherluecke":
        for p in bericht["speicher"]["client"]:
            p["sekunden"] = 0.0
        for p in bericht["speicher"]["broker"]:
            p["sekunden"] = 0.0
    elif args.mutant == "s06_grund":
        bericht["langsam"]["abgelehnt"] = 1
    elif args.mutant == "s06_fluter":
        bericht["langsam"]["ersetzte_liveframes"] = 0
    elif args.mutant == "s03_sofort":
        bericht["mitgliedschaft"]["vollstaendig"] -= 1
    elif args.mutant == "s09_getrennt":
        for n in bericht.get("neustart", []):
            n["s09_quellen_getrennt"] = 0
    elif args.mutant == "s10_define":
        bericht["pipe"]["ohne_produkt_v3"] = False
        bericht["pipe"]["produktionsname_neu"] = True
    elif args.mutant == "s11_paare":
        for n in bericht.get("neustart", []):
            n["reconnect_paare"] = 1
    elif args.mutant == "k_s4_widerspruch":
        bericht["kill"]["k_s4"]["flag_zum_killzeitpunkt"] = 0
    elif args.mutant == "k_s5_totzeit":
        for n in bericht.get("neustart", []):
            n["totzeit_erfasst"] = False
    elif args.mutant == "s14_nachlauf":
        bericht["p0"]["nachlauf_ms"] = 0


# ───────────────────────────────────────────────── Urteil

def urteile(bericht: dict, args) -> int:
    """Jedes Urteil folgt aus einem Beleg IM BERICHT — keines wird uebernommen.

    Zwei Regeln tragen diese Funktion:

    1. **Ein fehlendes Pflichtfeld ist rot, nie uebersprungen.** Eine Schleife
       ueber die tatsaechlich gelieferten Eintraege ist kein Test: liefert das
       C++-Programm nichts, laeuft sie null Mal und bleibt gruen
       (Codex-Befunde 14 und 15).
    2. **Killurteile werden nachgerechnet.** Der Bericht traegt `urteil` UND
       den Beleg; hier wird das Urteil aus dem Beleg neu abgeleitet und gegen
       das gelieferte gehalten. Ein Widerspruch ist ein Befund am Bein.
    """
    fehler = 0

    def pruefe(ok: bool, zeile: str, text: str, detail: str) -> None:
        """Ein Pruefpunkt der Verhaltensmatrix.

        `zeile` ist die Matrix-ID (S01 ...); die Ausgabe nennt zusaetzlich den
        Pruefpunktnamen `A24:<name>` aus derselben Zeile, damit ein Verweis aus
        dem Manifest im Laufprotokoll wiederzufinden ist.
        """
        nonlocal fehler
        marke = PRUEFPUNKTE.get(zeile, "")
        kopf = f"[{zeile} · {marke}]" if marke else f"[{zeile}]"
        print(("  ok      " if ok else "  ROT     ") + f"{kopf} {text}  [{detail}]")
        if not ok:
            fehler += 1

    FEHLT = object()

    def feld(quelle: dict, *namen):
        """Ein PFLICHTFELD. Fehlt es, ist das ein Befund, kein `None`."""
        wert = quelle
        for n in namen:
            if not isinstance(wert, dict) or n not in wert:
                return FEHLT
            wert = wert[n]
        return wert

    def pflicht(quelle: dict, pfad: str, zeile: str, *namen) -> bool:
        """Meldet ein fehlendes Pflichtfeld als eigenen roten Pruefpunkt."""
        if feld(quelle, *namen) is FEHLT:
            pruefe(False, zeile, f"Pflichtfeld `{pfad}` steht im Bericht",
                   "fehlt")
            return False
        return True

    p0_schranke = 0 if args.mutant == "s04" else P0_SCHRANKE_MS
    frist = 0 if args.mutant == "s11" else FRIST_MS
    soll_sonden = args.sonden + 1 if args.mutant == "s02" else args.sonden

    m = bericht.get("mitgliedschaft", {})
    p0 = bericht.get("p0", {})
    lv = bericht.get("liveness", {})
    lg = bericht.get("langsam", {})
    au = bericht.get("audio", {})
    sp = bericht.get("speicher", {})
    pi = bericht.get("pipe", {})

    # ── S14: sauberer Abbau, inklusive haengender Orchestrierung ────────────
    pruefe(bericht.get("client_exit") == 0 and not bericht.get("abbau_haenger"),
           "S14", "das C++-Programm endet sauber",
           f"Exit {bericht.get('client_exit')}"
           + (f", {bericht['abbau_haenger']}" if bericht.get("abbau_haenger") else ""))

    pruefe(0 <= bericht.get("topologie_ms", -1) <= frist, "S01",
           f"die Topologie stand in der Frist ({frist} ms)",
           f"{bericht.get('topologie_ms')} ms")
    pruefe(bericht.get("sonden") == soll_sonden, "S02",
           f"der Lauf traegt genau {soll_sonden} Sonden", str(bericht.get("sonden")))

    # ── S03: jede Snapshot-UEBERNAHME wurde sofort geprueft ────────────────
    if pflicht(bericht, "mitgliedschaft.uebernahmen", "S03", "mitgliedschaft", "uebernahmen"):
        pruefe(m.get("snapshot_pruefungen", 0) > 0
               and m.get("vollstaendig") == m.get("snapshot_pruefungen"), "S03",
               "jede Snapshot-Uebernahme war sofort vollstaendig und richtig",
               f"{m.get('vollstaendig')}/{m.get('snapshot_pruefungen')} bei "
               f"{m.get('uebernahmen')} Uebernahmen")
        pruefe(m.get("ungueltig") == 0, "S03",
               "kein Snapshot wurde als ungueltig abgewiesen",
               str(m.get("ungueltig")))
    # Die 250-ms-Abtastung bleibt ein eigener Pruefpunkt: sie sieht Drift, die
    # ohne neuen Snapshot entsteht.
    if pflicht(bericht, "mitgliedschaft.abtast_pruefungen", "S03",
               "mitgliedschaft", "abtast_pruefungen"):
        pruefe(m.get("abtast_pruefungen", 0) > 0
               and m.get("abtast_vollstaendig") == m.get("abtast_pruefungen"), "S03",
               "auch die laufende Abtastung sah nie eine abweichende Sicht",
               f"{m.get('abtast_vollstaendig')}/{m.get('abtast_pruefungen')}")

    pruefe(m.get("fremde_adresse") == 0, "S02",
           "nie eine fremde Adresse im Snapshot", str(m.get("fremde_adresse")))
    pruefe(m.get("fuehrendes_main_falsch") == 0, "S02",
           "fuehrendes_main war immer die Main-Adresse",
           str(m.get("fuehrendes_main_falsch")))
    pruefe(p0.get("gesendet", 0) > 0 and p0.get("verloren_ausserhalb_neustart") == 0,
           "S04", "kein P0 geht ausserhalb der Neustartfenster verloren",
           f"{p0.get('beantwortet')}/{p0.get('gesendet')}, "
           f"{p0.get('verloren_ausserhalb_neustart')} verloren "
           f"(Nachlauffenster {p0.get('nachlauf_ms')} ms)")
    pruefe(0 < p0.get("latenz_p95_ms", -1) <= p0_schranke, "S04",
           f"P0-ACK p95 unter {p0_schranke} ms",
           f"p95 {p0.get('latenz_p95_ms')} ms, max {p0.get('latenz_max_ms')} ms")
    # S14-Gegenpfad: ohne Nachlauffenster koennte ein gesunder Lauf an einem
    # zuletzt eingereihten Heartbeat scheitern.
    pruefe(p0.get("nachlauf_ms", 0) >= 2000, "S14",
           "vor dem Abbau lief ein Nachlauffenster fuer ausstehende ACKs",
           f"{p0.get('nachlauf_ms')} ms")

    pruefe(lg.get("immer_mitglied") is True, "S05",
           "die langsamen Leser blieben durchgehend Mitglied",
           str(lg.get("immer_mitglied")))

    # ── S06: der Rueckstau der LANGSAMEN Sonden, nicht der des Fluters ─────
    if pflicht(bericht, "langsam.veroeffentlichungen", "S06",
               "langsam", "veroeffentlichungen"):
        pruefe(lg.get("ersetzte_liveframes", 0) > 0, "S06",
               "die Schleuse der langsamen Leser lag wirklich im Rueckstau",
               f"{lg.get('ersetzte_liveframes')} ersetzt bei "
               f"{lg.get('veroeffentlichungen')} Veroeffentlichungen "
               f"(Fluter getrennt: "
               f"{bericht.get('fluter', {}).get('ersetzte_liveframes')})")
        pruefe(lg.get("neueste_verworfen") == 0, "S06",
               "und dabei fiel nie der neueste Frame (replace-oldest)",
               f"{lg.get('neueste_verworfen')} neueste verworfen")
        pruefe(lg.get("abgelehnt") == lg.get("zu_gross", 0) + lg.get("neueste_verworfen", 0),
               "S06", "jede abgelehnte Veroeffentlichung hat einen gezaehlten Grund",
               f"{lg.get('abgelehnt')} abgelehnt (aus dem Rueckgabewert) "
               f"= {lg.get('zu_gross')} + {lg.get('neueste_verworfen')}")
        pruefe(lg.get("blockiert_andere_nicht") in (True, "nicht_anwendbar"), "S06",
               "kein langsamer Leser bremst die schnellen",
               f"{lg.get('blockiert_andere_nicht')} (schnelle p95 "
               f"{lg.get('schnelle_p95_ms')} ms)")

    # ── S08: Ganzbloecke UND Producer-Frames ───────────────────────────────
    pruefe(au.get("bloecke", 0) > 0
           and au.get("ganzblockdrops_ueberlauf") == 0
           and au.get("ganzblockdrops_oversize") == 0, "S08",
           "0 Ganzblockdrops auf dem echten Sondenpfad",
           f"{au.get('bloecke')} Bloecke, "
           f"{au.get('ganzblockdrops_ueberlauf')} Ueberlauf, "
           f"{au.get('ganzblockdrops_oversize')} Oversize")
    pruefe(au.get("kontinuitaetsbrueche") == 0, "S08",
           "keine unerklaerten Kontinuitaetsbrueche",
           str(au.get("kontinuitaetsbrueche")))
    if pflicht(bericht, "audio.publikationen_je_sonde_min", "S08",
               "audio", "publikationen_je_sonde_min"):
        # Aus der Laufzeit gerechnet, nicht aus einem frueheren Lauf
        # abgeschrieben: `nimmBlock` baut einen Frame, sobald LIVE_INTERVALL_S
        # Samples stehen; das sind ceil(LIVE_INTERVALL_S * fs / block) Bloecke.
        fs = au.get("samplerate") or 0
        block = au.get("blockgroesse") or 0
        bloecke_je_frame = (
            -(-int(LIVE_INTERVALL_S * fs) // int(block)) if fs and block else 0)
        soll = (int(au.get("bloecke_je_sonde_min", 0) / bloecke_je_frame
                    * PUBLIKATION_TOLERANZ) if bloecke_je_frame else 0)
        pruefe(au.get("publikationen", 0) > 0
               and bloecke_je_frame > 0
               and au.get("publikationen_je_sonde_min", 0) >= soll
               and soll > 0, "S08",
               "jede Sonde hat je Analysefenster einen Producer-Frame "
               "veroeffentlicht",
               f"{au.get('publikationen')} gesamt, kleinste Sonde "
               f"{au.get('publikationen_je_sonde_min')} >= {soll} "
               f"({au.get('bloecke_je_sonde_min')} Bloecke / {bloecke_je_frame} "
               f"je Fenster, Toleranz {PUBLIKATION_TOLERANZ})")

    pruefe(lv.get("stale_ausserhalb_neustart") == 0, "S13",
           "kein Mitglied wurde ausserhalb der Neustartfenster stale",
           str(lv.get("stale_ausserhalb_neustart")))
    pruefe(lv.get("evicted_ausserhalb_neustart") == 0, "S13",
           "kein Mitglied verschwand ausserhalb der Neustartfenster",
           str(lv.get("evicted_ausserhalb_neustart")))

    # ── S10: der C++-Zaehler UND die Namensraummessung ─────────────────────
    pruefe(pi.get("fremder_name_versucht") == 0
           and pi.get("clientnamen_geprueft", 0) >= 2 * (args.sonden + 1)
           and pi.get("ohne_produkt_v3") is True, "S10",
           "kein Client bekam je einen anderen Pipenamen",
           f"{pi.get('fremder_name_versucht')} fremd bei "
           f"{pi.get('clientnamen_geprueft')} geprueften Uebergaben, "
           f"ohne_produkt_v3 {pi.get('ohne_produkt_v3')}")
    if pflicht(bericht, "pipe.produktionsname_neu", "S10", "pipe", "produktionsname_neu"):
        pruefe(pi.get("namensraum_lesbar") is True
               and pi.get("eigene_probe_sichtbar") is True
               and pi.get("produktionsname_neu") is False
               and pi.get("goldenname_neu") is False, "S10",
               "im Pipe-Namensraum entstand kein Produktions- oder Goldenname",
               f"eigene Probe sichtbar {pi.get('eigene_probe_sichtbar')}, "
               f"produktionsname_neu {pi.get('produktionsname_neu')}, "
               f"goldenname_neu {pi.get('goldenname_neu')}, "
               f"{pi.get('namen_vor')} -> {pi.get('namen_nach')} Namen")

    pruefe(bericht.get("subscription", {}).get("weg_im_neustartfenster", 0) > 0,
           "S09",
           "der Kill beendet die Subscription des Main sichtbar",
           f"{bericht.get('subscription', {}).get('weg_im_neustartfenster')} mal "
           f"ohne aktive Subscription im Neustartfenster gesehen")
    # S15 aus den GEMESSENEN Exitcodes des Riegellaufs, nicht aus einem `True`.
    rg = bericht.get("riegel", {})
    pruefe(len(rg) == 4 and all(c == 3 for c in rg.values()), "S15",
           "beide Programme verweigern Produktions- und Golden-Pipename",
           f"{rg}" if rg else "kein Riegelbeleg im Bericht")

    # ── S16: die Skalierungsformel wird gerechnet, nicht behauptet ─────────
    n_sonden = bericht.get("sonden", 0)
    # `std::lround` rundet die Haelfte VON NULL WEG; Pythons `round` rundet
    # zur geraden Zahl. floor(x + 0.5) trifft die C++-Seite exakt
    # (SessionSoakMain.cpp, `main()`).
    soll_langsam = max(1, int(n_sonden * args.langsam + 0.5)) if n_sonden else 0
    pruefe(n_sonden >= 1 and bericht.get("langsam_anzahl") == soll_langsam, "S16",
           "langsam_anzahl folgt max(1, round(N x Anteil))",
           f"N = {n_sonden}, langsam_anzahl {bericht.get('langsam_anzahl')}, "
           f"erwartet {soll_langsam}")
    if n_sonden == 1:
        # Bei N = 1 hat "blockiert die anderen nicht" keine Gegenpartei; das
        # Feld MUSS `nicht_anwendbar` melden, nie `true` (Matrix S16).
        pruefe(lg.get("blockiert_andere_nicht") == "nicht_anwendbar", "S16",
               "bei N = 1 meldet `blockiert_andere_nicht` nicht_anwendbar",
               str(lg.get("blockiert_andere_nicht")))

    # ── S11/S12: genau so viele Neustartberichte wie geplant ───────────────
    neustarts = bericht.get("neustart", [])
    pruefe(len(neustarts) == args.neustarts, "S11",
           f"der Bericht traegt genau {args.neustarts} Neustart(e)",
           f"{len(neustarts)} geliefert")
    for n in neustarts:
        dauer = n.get("bereit_bis_vollstaendig_ms", -1)
        rec = n.get("reconnect_ms", {})
        pruefe(0 <= dauer <= frist, "S11",
               f"Neustart {n.get('index')}: Snapshot wieder vollstaendig in der Frist",
               f"{dauer} ms von {frist} ms (Schranke {SCHRANKE_MS} ms, "
               f"reconnect max {rec.get('max')} ms)")
        # Die Verteilung muss ueber ALLE Clientpaare gehen — Main plus N Sonden.
        pruefe(n.get("reconnect_paare") == args.sonden + 1
               and rec.get("min") is not None
               and rec.get("max", 0) <= frist, "S11",
               f"Neustart {n.get('index')}: Reconnect je Clientpaar gemessen",
               f"{n.get('reconnect_paare')} Paare (erwartet {args.sonden + 1}), "
               f"min {rec.get('min')} / p95 {rec.get('p95')} / max {rec.get('max')} ms")
        # S09 JE KILL: ein Gesamtzaehler liesse einen Kill ohne sichtbares
        # `disconnected` durchgehen (Befund 8).
        pruefe(n.get("s09_subscription_weg", 0) > 0
               and n.get("s09_quellen_getrennt", 0) > 0, "S09",
               f"Neustart {n.get('index')}: Subscription weg UND Quellen getrennt",
               f"{n.get('s09_subscription_weg')} ohne Subscription, "
               f"{n.get('s09_quellen_getrennt')} mit getrennter Quelle, "
               f"{n.get('s09_sichten')} Sichten im Fenster")
        pruefe(n.get("kill_erfolgt_gesehen") is True
               and n.get("totzeit_erfasst") is True, "S09",
               f"Neustart {n.get('index')}: Kill und Totzeit wurden belegt",
               f"kill_erfolgt {n.get('kill_erfolgt_gesehen')}, "
               f"totzeit_erfasst {n.get('totzeit_erfasst')} "
               f"(Killdauer {n.get('kill_dauer_ms')} ms)")
        pruefe(n.get("alte_epoche_nach_neustart_gesehen") == 0, "S12",
               f"Neustart {n.get('index')}: die alte broker_epoch kam nie wieder",
               str(n.get("alte_epoche_nach_neustart_gesehen")))
        pruefe(bool(n.get("epoch_neu")) and n.get("epoch_neu") != n.get("epoch_alt"),
               "S12", f"Neustart {n.get('index')}: neue broker_epoch",
               f"{(n.get('epoch_alt') or '')[:8]}… -> {(n.get('epoch_neu') or '')[:8]}…")
        if n.get("bereit_bis_vollstaendig_ms", 0) > SCHRANKE_MS:
            print(f"  HINWEIS [S11] Neustart {n.get('index')} lag ueber der "
                  f"gerechneten Schranke {SCHRANKE_MS} ms, aber in der Frist — "
                  f"der benannte Zuschlag hat getragen (Manifest §3.1).")

    # Z3 je Kurve: Basis ist die erste Probe nach dem Warmup ausserhalb eines
    # Neustartfensters; Brokerkurven gelten je Generation (Manifest §3.2).
    def wachstum(punkte, budget_p, budget_b):
        gueltig = [p for p in punkte if not p.get("im_neustartfenster")]
        if len(gueltig) < 2:
            return None
        basis, ende = gueltig[0]["rss_bytes"], gueltig[-1]["rss_bytes"]
        grenze = max(basis * budget_p / 100.0, budget_b)
        spanne = gueltig[-1].get("sekunden", 0) - gueltig[0].get("sekunden", 0)
        return basis, ende, ende - basis, grenze, spanne

    for name, punkte in (("Client", sp.get("client", [])),):
        w = wachstum(punkte, sp.get("budget_prozent", 0), sp.get("budget_bytes", 0))
        pruefe(w is not None, "S07",
               f"{name}: mindestens zwei Messpunkte ausserhalb der Neustartfenster",
               f"{len([p for p in punkte if not p.get('im_neustartfenster')])} Punkte")
        if w is None:
            continue
        basis, ende, delta, grenze, spanne = w
        # Zwei Punkte reichen nur, wenn zwischen ihnen wirklich Zeit liegt. Die
        # Vorfassung erzeugte am Laufende eine zweite Probe unmittelbar nach der
        # letzten regulaeren; die "Kurve" hatte dann Spanne 0 und konnte S07
        # nicht falsifizieren (Befund 2).
        pruefe(spanne >= SPEICHER_ENDPROBE_MIN_S, "S07",
               f"{name}: die Kurve traegt echte Minutenabstaende",
               f"{spanne:.0f} s zwischen erstem und letztem gueltigen Punkt "
               f"(mindestens {SPEICHER_ENDPROBE_MIN_S} s)")
        pruefe(delta <= grenze, "S07",
               f"{name}-Working-Set bleibt im Budget",
               f"{basis/1e6:.1f} -> {ende/1e6:.1f} MB, +{delta/1e6:.1f} MB, "
               f"Grenze {grenze/1e6:.1f} MB ueber {spanne:.0f} s")

    generationen = sorted({p["generation"] for p in sp.get("broker", [])})
    pruefe(len(generationen) == args.neustarts + 1, "S07",
           f"die Speicherkurve traegt {args.neustarts + 1} Brokergeneration(en)",
           f"{len(generationen)} geliefert: {generationen}")
    for g in generationen:
        punkte = [p for p in sp.get("broker", []) if p["generation"] == g]
        w = wachstum(punkte, sp.get("budget_prozent", 0), sp.get("budget_bytes", 0))
        gueltige = len([p for p in punkte if not p.get("im_neustartfenster")])
        # Abschnitt 3.2: eine Generation mit weniger als zwei Messpunkten ist
        # UNZUREICHEND und damit NICHT bestanden — kein `continue` (Befund 3).
        pruefe(w is not None, "S07",
               f"Broker Generation {g}: mindestens zwei Messpunkte "
               f"ausserhalb der Neustartfenster",
               f"{gueltige} von {len(punkte)} Punkten"
               + ("" if w is not None else " — unzureichend ist nicht bestanden"))
        if w is None:
            continue
        basis, ende, delta, grenze, spanne = w
        pruefe(spanne >= SPEICHER_ENDPROBE_MIN_S, "S07",
               f"Broker Generation {g}: die Kurve traegt echte Minutenabstaende",
               f"{spanne:.0f} s (mindestens {SPEICHER_ENDPROBE_MIN_S} s)")
        pruefe(delta <= grenze, "S07",
               f"Broker-Working-Set Generation {g} bleibt im Budget",
               f"{basis/1e6:.1f} -> {ende/1e6:.1f} MB, +{delta/1e6:.1f} MB, "
               f"Grenze {grenze/1e6:.1f} MB ueber {spanne:.0f} s")

    # ── Killpunkte: das Urteil wird NACHGERECHNET, nicht uebernommen ───────
    #
    # ZWEI Anweisungen, kein `fehler += killurteile(...)`: `killurteile` erhoeht
    # ueber das durchgereichte `pruefe` denselben `fehler`, und ein augmented
    # assignment laedt die linke Seite VOR dem Aufruf — der Zuwachs aus der
    # Closure ginge beim Zurueckschreiben verloren. Selbst gemessen am
    # Rotbeweis `--mutant k_s3_beleg` (02.09.2026): rote Zeile, Exit 0.
    zusatz = killurteile(bericht, args, pruefe)
    fehler += zusatz

    print("GRUEN" if fehler == 0 else "ROT")
    return 0 if fehler == 0 else 2


def killurteile(bericht: dict, args, pruefe) -> int:
    """Leitet jedes `kill.*.urteil` aus seinen Belegfeldern neu ab.

    Die Ableitungen stehen woertlich in Manifest §6:

      k_s1  getroffen  <=>  telemetrie_handle_fehler > 0
      k_s2  getroffen  <=>  snapshot_vor_kill == False
      k_s3  getroffen  <=>  p0_ohne_ack_im_fenster > 0
                            (und das ist p0.verloren_im_neustartfenster)
      k_s4  getroffen  <=>  flag_zum_killzeitpunkt > 0
      k_s5  getroffen  <=>  backoff_deckel_erreicht == erwartet (N + 1)

    Ein Punkt ohne `gefahren` ist `nicht_gefahren`. Fehlt ein Pflichteintrag
    oder widerspricht das gelieferte Urteil der Ableitung, ist das rot.
    """
    fehler = 0
    kill = bericht.get("kill", {})
    belege = {
        "k_s1": ("telemetrie_handle_fehler", lambda v, e: v > 0),
        "k_s2": ("snapshot_vor_kill", lambda v, e: v is False),
        "k_s3": ("p0_ohne_ack_im_fenster", lambda v, e: v > 0),
        "k_s4": ("flag_zum_killzeitpunkt", lambda v, e: v > 0),
        "k_s5": ("backoff_deckel_erreicht", lambda v, e: v == e),
    }

    print("  --      Killpunkte (Urteil aus dem Beleg abgeleitet, nicht uebernommen):")
    for name, (belegfeld, treffer) in belege.items():
        marke = KILLPUNKTE.get(name, "")
        eintrag = kill.get(name)
        if not isinstance(eintrag, dict):
            print(f"  ROT     [{name} · {marke}] Pflichteintrag fehlt im Bericht")
            fehler += 1
            continue
        fehlend = [f for f in (belegfeld, "gefahren", "urteil") if f not in eintrag]
        if fehlend:
            print(f"  ROT     [{name} · {marke}] Pflichtfeld(er) fehlen: {fehlend}")
            fehler += 1
            continue
        wert = eintrag[belegfeld]
        erwartet_wert = eintrag.get("erwartet")
        abgeleitet = ("nicht_gefahren" if not eintrag["gefahren"]
                      else ("getroffen" if treffer(wert, erwartet_wert)
                            else "nicht_getroffen"))
        stimmt = abgeleitet == eintrag["urteil"]
        beleg = {k: v for k, v in eintrag.items() if k != "urteil"}
        print(("  ok      " if stimmt else "  ROT     ")
              + f"[{name} · {marke}] {eintrag['urteil']} "
                f"(abgeleitet: {abgeleitet})  {beleg}")
        if not stimmt:
            fehler += 1

    # Kreuzprobe: K-S3 IST der endgueltige Verlust im Neustartfenster. Weichen
    # beide Zahlen ab, misst der Killbeleg etwas anderes als der Schlussbericht
    # (genau der Fehler, den Befund 1 beschrieb).
    ks3 = kill.get("k_s3", {})
    if isinstance(ks3, dict) and "p0_ohne_ack_im_fenster" in ks3:
        pruefe(ks3["p0_ohne_ack_im_fenster"]
               == bericht.get("p0", {}).get("verloren_im_neustartfenster"), "S04",
               "K-S3 zaehlt genau die endgueltig im Fenster verlorenen ACKs",
               f"{ks3['p0_ohne_ack_im_fenster']} vs. "
               f"{bericht.get('p0', {}).get('verloren_im_neustartfenster')}")
    else:
        pruefe(False, "S04", "K-S3 traegt seinen Beleg", "fehlt")
    return fehler


def main(argv: list[str]) -> int:
    p = argparse.ArgumentParser(add_help=True)
    p.add_argument("--sonden", type=int, default=16)
    p.add_argument("--minuten", type=int, default=2)
    p.add_argument("--neustarts", type=int, default=1)
    p.add_argument("--langsam", type=float, default=0.25)
    p.add_argument("--langsam-ms", type=int, default=120,
                   help="kuenstliche Verzoegerung eines langsamen Lesers je "
                        "Callback; Diagnose, Standard 120 ms")
    p.add_argument("--mutant", choices=sorted(MUTANTEN), default=None,
                   help="Rotbeweis: verfaelscht genau eine Groesse, siehe --mutant-liste")
    p.add_argument("--mutant-liste", action="store_true")
    p.add_argument("--bericht", default=None,
                   help="NUR das Urteil ueber einen gespeicherten Bericht "
                        "fahren, ohne Lauf. Fuer den Rotbeweis: jeder Mutant "
                        "laesst sich damit an EINEM echten Lauf zeigen, statt "
                        "je Mutant einen weiteren Soak zu fahren.")
    args = p.parse_args(argv)

    if args.mutant_liste:
        for k, v in sorted(MUTANTEN.items()):
            print(f"  {k}: {v}")
        return 0

    if args.bericht:
        # Kein Lauf, kein Riegel, keine Messung — nur `urteile()` ueber einen
        # vorhandenen Bericht. Der Kanon ruft dieses Werkzeug OHNE `--bericht`;
        # die Bannerzeile macht jede Verwechslung im Protokoll sichtbar.
        with open(args.bericht, encoding="utf-8") as f:
            bericht = json.load(f)
        print(f"NUR URTEIL aus {args.bericht} — kein Lauf, keine Messung"
              + (f", Mutant {args.mutant}" if args.mutant else ""))
        mutiere(bericht, args)
        return urteile(bericht, args)

    for pfad, bau in (
        (BROKER, "cargo build --release --manifest-path broker/Cargo.toml "
                 "--bin eqcop-broker-sonde012-probe"),
        (CLIENT, "cmake --build eq-copilot/build --config Release "
                 "--target EqCopSessionSoak"),
    ):
        if not pfad.exists():
            print(f"VORAUSSETZUNG FEHLT: {pfad.relative_to(WURZEL)}\n  {bau}")
            return 3

    if args.mutant == "s15":
        # Rotbeweis S15: der Riegel wird gegen die Produktionspipe gefahren und
        # muss beide Programme mit Exit 3 abweisen.
        for programm in (BROKER, CLIENT):
            lauf = subprocess.run([str(programm), PRODUKTIONS_PIPE], cwd=WURZEL,
                                  capture_output=True, text=True,
                                  encoding="utf-8", errors="replace", timeout=60)
            print(f"  {programm.name} mit Produktions-Pipename: Exit {lauf.returncode}")
            if lauf.returncode != 3:
                return 2
        print("VORAUSSETZUNG: beide Programme verweigern die Produktionspipe (Exit 3)")
        return 3

    riegel_ok, riegel_belege = riegel_faehrt()
    if not riegel_ok:
        return 2

    try:
        code, bericht = fahre(args)
    except RuntimeError as f:
        # Der ERSTE Brokerstart liegt vor dem `try` in `fahre` — auch er darf
        # nie in einem nackten Exit 1 enden (Zeile S14, Befund 17).
        print(f"VORAUSSETZUNG FEHLT [S01] Probe-Broker nicht bereit: {f}")
        return 3
    if code != 0:
        return code
    # Der Riegelbeleg gehoert in den Bericht, damit `urteile()` auch S15 aus
    # gemessenen Exitcodes entscheidet und nicht aus einem `True`.
    bericht["riegel"] = riegel_belege
    print("Bericht: " + json.dumps(bericht, ensure_ascii=False, sort_keys=True))
    return urteile(bericht, args)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
