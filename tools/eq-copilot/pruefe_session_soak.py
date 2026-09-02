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

DIE PIPE
--------
Immer ein Probe-Name mit PID und Zeitstempel. Weder dieses Bein noch eines der
beiden Programme darf die Produktions-Pipe anfassen.

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

# Fristen und Budgets — die Zahlen stehen im Manifest §3 mit ihrer Quelle.
FRIST_MS = 60000            # Z2, Manifest §3.1 (Schranke 45.051 ms + Zuschlag)
SCHRANKE_MS = 45051         # gerechnete obere Schranke je Clientpaar
P0_SCHRANKE_MS = 1000       # wie A22, IpcLastMain.cpp:42
BUDGET_PROZENT = 10         # Z3, Manifest §3.2
BUDGET_BYTES = 16 * 1024 * 1024
SPEICHER_TAKT_S = 60
TOTZEIT_KS5_S = 21          # > 15,5 s Backoff-Folge, Manifest K-S5

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
}


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


def riegel_faehrt() -> bool:
    """Zeile S15: der Riegel wird GEFAHREN, nicht behauptet."""
    for name, hinweis in ((GOLDEN_PIPE, "Golden-Pipename aus §48.3"),
                          (PRODUKTIONS_PIPE, "v1-Produktions-Pipe")):
        for programm in (BROKER, CLIENT):
            lauf = subprocess.run([str(programm), name], cwd=WURZEL,
                                  capture_output=True, text=True,
                                  encoding="utf-8", errors="replace", timeout=60)
            if lauf.returncode != 3:
                print(f"ROT: {programm.name} nimmt den {hinweis} an "
                      f"(Exit {lauf.returncode})")
                return False
    print("  ok      beide Programme verweigern Produktions- und Golden-Pipename")
    return True


# ───────────────────────────────────────────────── Der Lauf

def fahre(args) -> tuple[int, dict]:
    warmup_s = 120 if args.minuten >= 30 else 20
    mess_s = args.minuten * 60
    pipe = f"{PROBE_PRAEFIX}soak.{os.getpid()}.{int(time.time())}"
    assert pipe.startswith(PROBE_PRAEFIX) and len(pipe) > len(PROBE_PRAEFIX)
    assert PRODUKTIONS_PIPE not in pipe and pipe != GOLDEN_PIPE
    print(f"Probe-Pipe: {pipe}")

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

    def sag(zeile: str) -> None:
        klient.stdin.write(zeile + "\n")
        klient.stdin.flush()

    def probe(minute: int, im_fenster: bool) -> None:
        speicher["client"].append({"minute": minute, "rss_bytes": rss_bytes(klient.pid),
                                   "im_neustartfenster": im_fenster})
        speicher["broker"].append({"generation": generation, "minute": minute,
                                   "rss_bytes": rss_bytes(broker.pid),
                                   "im_neustartfenster": im_fenster})

    try:
        if aus.warte_auf("TOPOLOGIE_STEHT", FRIST_MS / 1000.0 + 30) is None:
            print("ROT: die Topologie stand nicht in der Frist (Zeile S01)")
            return 2, {}
        if aus.warte_auf("WARMUP_FERTIG", warmup_s + 60) is None:
            print("ROT: das Warmup endete nicht (Zeile S02)")
            return 2, {}
        print(f"  ok      Topologie steht, Warmup {warmup_s} s vorbei")

        # Neustart k bei Bruchteil k/(neustarts+1) der Messzeit. Barrieren:
        # Neustart 1 = K-S4, Neustart 2 = K-S2 + K-S5-Totzeit (Manifest §6/§9).
        plan = []
        for k in range(1, args.neustarts + 1):
            art = "k_s4" if k == 1 else ("k_s2" if k == 2 else "plain")
            totzeit = TOTZEIT_KS5_S if k == 2 else 1
            plan.append((mess_s * k / (args.neustarts + 1), art, totzeit))

        gemessen = 0.0            # beobachtete Messzeit; Neustartfenster zaehlen nicht
        naechste_probe = 0.0
        minute = 0
        probe(minute, False)
        t_letzt = time.monotonic()

        for zielzeit, art, totzeit in plan:
            while gemessen < zielzeit:
                time.sleep(0.5)
                jetzt = time.monotonic()
                gemessen += jetzt - t_letzt
                t_letzt = jetzt
                if gemessen >= naechste_probe + SPEICHER_TAKT_S:
                    naechste_probe += SPEICHER_TAKT_S
                    minute += 1
                    probe(minute, False)

            generation += 1
            idx = generation
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

            broker_toeten(broker)
            sag("BARRIERE_AUS")
            sag(f"KILL_GESCHEHEN {idx}")
            t_tot = time.monotonic()
            time.sleep(totzeit)
            broker = broker_starten(pipe, broker_s)
            totzeit_ms = int((time.monotonic() - t_tot) * 1000)
            sag(f"BEREIT {idx} {totzeit_ms}")
            fertig = aus.warte_auf("NEUSTART_VOLLSTAENDIG", FRIST_MS / 1000.0 + 30)
            print(f"  ..      Neustart {idx} ({art}, Totzeit {totzeit_ms} ms): "
                  f"{fertig or 'KEINE MELDUNG'}")
            t_letzt = time.monotonic()          # Neustartfenster zaehlt nicht mit
            probe(minute, True)

        while gemessen < mess_s:
            time.sleep(0.5)
            jetzt = time.monotonic()
            gemessen += jetzt - t_letzt
            t_letzt = jetzt
            if gemessen >= naechste_probe + SPEICHER_TAKT_S:
                naechste_probe += SPEICHER_TAKT_S
                minute += 1
                probe(minute, False)
        minute += 1
        probe(minute, False)

        sag("ENDE")
        klient.wait(timeout=180)
    finally:
        try:
            if klient.poll() is None:
                klient.kill()
                klient.wait(timeout=30)
        except OSError:
            pass
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
    if not bericht:
        fehlertext = klient.stderr.read() if klient.stderr else ""
        print(f"ROT: das C++-Programm lieferte keinen Bericht. Exit "
              f"{klient.returncode}. {fehlertext[:900]}")
        return 2, {}

    bericht["speicher"] = {
        "takt_s": SPEICHER_TAKT_S,
        "budget_prozent": 0 if args.mutant == "s07" else BUDGET_PROZENT,
        "budget_bytes": 0 if args.mutant == "s07" else BUDGET_BYTES,
        **speicher,
    }
    bericht["client_exit"] = klient.returncode
    if args.mutant == "s12":
        for n in bericht.get("neustart", []):
            n["alte_epoche_nach_neustart_gesehen"] = 1
    return 0, bericht


# ───────────────────────────────────────────────── Urteil

def urteile(bericht: dict, args) -> int:
    fehler = 0

    def pruefe(ok: bool, zeile: str, text: str, detail: str) -> None:
        nonlocal fehler
        print(("  ok      " if ok else "  ROT     ") + f"[{zeile}] {text}  [{detail}]")
        if not ok:
            fehler += 1

    p0_schranke = 0 if args.mutant == "s04" else P0_SCHRANKE_MS
    frist = 0 if args.mutant == "s11" else FRIST_MS
    soll_sonden = args.sonden + 1 if args.mutant == "s02" else args.sonden

    m = bericht.get("mitgliedschaft", {})
    p0 = bericht.get("p0", {})
    lv = bericht.get("liveness", {})
    lg = bericht.get("langsam", {})
    au = bericht.get("audio", {})
    sp = bericht.get("speicher", {})

    pruefe(bericht.get("client_exit") == 0, "S14",
           "das C++-Programm endet sauber", f"Exit {bericht.get('client_exit')}")
    pruefe(0 <= bericht.get("topologie_ms", -1) <= frist, "S01",
           f"die Topologie stand in der Frist ({frist} ms)",
           f"{bericht.get('topologie_ms')} ms")
    pruefe(bericht.get("sonden") == soll_sonden, "S02",
           f"der Lauf traegt genau {soll_sonden} Sonden", str(bericht.get("sonden")))
    pruefe(m.get("snapshot_pruefungen", 0) > 0
           and m.get("vollstaendig") == m.get("snapshot_pruefungen"), "S03",
           "jede Snapshot-Pruefung war vollstaendig und richtig",
           f"{m.get('vollstaendig')}/{m.get('snapshot_pruefungen')}")
    pruefe(m.get("fremde_adresse") == 0, "S02",
           "nie eine fremde Adresse im Snapshot", str(m.get("fremde_adresse")))
    pruefe(m.get("fuehrendes_main_falsch") == 0, "S02",
           "fuehrendes_main war immer die Main-Adresse",
           str(m.get("fuehrendes_main_falsch")))
    pruefe(p0.get("gesendet", 0) > 0 and p0.get("verloren_ausserhalb_neustart") == 0,
           "S04", "kein P0 geht ausserhalb der Neustartfenster verloren",
           f"{p0.get('beantwortet')}/{p0.get('gesendet')}, "
           f"{p0.get('verloren_ausserhalb_neustart')} verloren")
    pruefe(0 < p0.get("latenz_p95_ms", -1) <= p0_schranke, "S04",
           f"P0-ACK p95 unter {p0_schranke} ms",
           f"p95 {p0.get('latenz_p95_ms')} ms, max {p0.get('latenz_max_ms')} ms")
    pruefe(lg.get("immer_mitglied") is True, "S05",
           "die langsamen Leser blieben durchgehend Mitglied",
           str(lg.get("immer_mitglied")))
    pruefe(lg.get("ersetzte_liveframes", 0) > 0, "S06",
           "es lag wirklich Rueckstau an (Cap hat ersetzt)",
           f"{lg.get('ersetzte_liveframes')} ersetzt")
    pruefe(lg.get("neueste_verworfen") == 0, "S06",
           "und dabei fiel nie der neueste Frame (replace-oldest)",
           f"{lg.get('neueste_verworfen')} neueste verworfen")
    pruefe(lg.get("abgelehnt") == lg.get("zu_gross", 0) + lg.get("neueste_verworfen", 0),
           "S06", "jede Ablehnung hat einen gezaehlten Grund",
           f"{lg.get('abgelehnt')} = {lg.get('zu_gross')} + {lg.get('neueste_verworfen')}")
    pruefe(lg.get("blockiert_andere_nicht") in (True, "nicht_anwendbar"), "S06",
           "kein langsamer Leser bremst die schnellen",
           f"{lg.get('blockiert_andere_nicht')} (schnelle p95 "
           f"{lg.get('schnelle_p95_ms')} ms)")
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
    pruefe(lv.get("stale_ausserhalb_neustart") == 0, "S13",
           "kein Mitglied wurde ausserhalb der Neustartfenster stale",
           str(lv.get("stale_ausserhalb_neustart")))
    pruefe(lv.get("evicted_ausserhalb_neustart") == 0, "S13",
           "kein Mitglied verschwand ausserhalb der Neustartfenster",
           str(lv.get("evicted_ausserhalb_neustart")))

    for n in bericht.get("neustart", []):
        dauer = n.get("bereit_bis_vollstaendig_ms", -1)
        pruefe(0 <= dauer <= frist, "S11",
               f"Neustart {n.get('index')}: Snapshot wieder vollstaendig in der Frist",
               f"{dauer} ms von {frist} ms (Schranke {SCHRANKE_MS} ms, "
               f"reconnect max {n.get('reconnect_ms', {}).get('max')} ms)")
        pruefe(n.get("alte_epoche_nach_neustart_gesehen") == 0, "S12",
               f"Neustart {n.get('index')}: die alte broker_epoch kam nie wieder",
               str(n.get("alte_epoche_nach_neustart_gesehen")))
        pruefe(bool(n.get("epoch_neu")) and n.get("epoch_neu") != n.get("epoch_alt"),
               "S12", f"Neustart {n.get('index')}: neue broker_epoch",
               f"{n.get('epoch_alt')[:8]}… -> {(n.get('epoch_neu') or '')[:8]}…")
        if n.get("bereit_bis_vollstaendig_ms", 0) > SCHRANKE_MS:
            print(f"  HINWEIS [S11] Neustart {n.get('index')} lag ueber der "
                  f"gerechneten Schranke {SCHRANKE_MS} ms, aber in der Frist — "
                  f"der benannte Zuschlag hat getragen (Manifest §3.1).")

    # Z3 je Kurve: Basis ist die erste Probe nach dem Warmup ausserhalb eines
    # Neustartfensters; Brokerkurven gelten je Generation (Manifest §3.2).
    def wachstum(punkte, budget_p, budget_b, name):
        gueltig = [p for p in punkte if not p.get("im_neustartfenster")]
        if len(gueltig) < 2:
            return None
        basis, ende = gueltig[0]["rss_bytes"], gueltig[-1]["rss_bytes"]
        grenze = max(basis * budget_p / 100.0, budget_b)
        return basis, ende, ende - basis, grenze

    for name, punkte in (("Client", sp.get("client", [])),):
        w = wachstum(punkte, sp.get("budget_prozent", 0), sp.get("budget_bytes", 0), name)
        if w is None:
            print(f"  ROT     [S07] {name}: weniger als zwei Messminuten "
                  f"ausserhalb der Neustartfenster — kein Wachstumswert, "
                  f"nicht bestanden")
            fehler += 1
            continue
        basis, ende, delta, grenze = w
        pruefe(delta <= grenze, "S07",
               f"{name}-Working-Set bleibt im Budget",
               f"{basis/1e6:.1f} -> {ende/1e6:.1f} MB, +{delta/1e6:.1f} MB, "
               f"Grenze {grenze/1e6:.1f} MB")

    generationen = sorted({p["generation"] for p in sp.get("broker", [])})
    for g in generationen:
        punkte = [p for p in sp.get("broker", []) if p["generation"] == g]
        w = wachstum(punkte, sp.get("budget_prozent", 0), sp.get("budget_bytes", 0),
                     f"Broker Generation {g}")
        if w is None:
            print(f"  HINWEIS [S07] Broker Generation {g}: unzureichend "
                  f"({len(punkte)} Proben) — kein Wachstumswert, nicht bestanden")
            continue
        basis, ende, delta, grenze = w
        pruefe(delta <= grenze, "S07",
               f"Broker-Working-Set Generation {g} bleibt im Budget",
               f"{basis/1e6:.1f} -> {ende/1e6:.1f} MB, +{delta/1e6:.1f} MB, "
               f"Grenze {grenze/1e6:.1f} MB")

    # Killpunkte sind Berichtslage, kein Rotmacher: ein nicht getroffener Punkt
    # zaehlt nie als bestanden, haelt aber das Bein nicht auf (Manifest §7).
    print("  --      Killpunkte (kein Rotmacher, aber nie stiller Erfolg):")
    for name, eintrag in sorted(bericht.get("kill", {}).items()):
        print(f"            {name}: {eintrag.get('urteil')}  "
              f"{ {k: v for k, v in eintrag.items() if k != 'urteil'} }")

    print("GRUEN" if fehler == 0 else "ROT")
    return 0 if fehler == 0 else 2


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
    args = p.parse_args(argv)

    if args.mutant_liste:
        for k, v in sorted(MUTANTEN.items()):
            print(f"  {k}: {v}")
        return 0

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

    if not riegel_faehrt():
        return 2

    code, bericht = fahre(args)
    if code != 0:
        return code
    print("Bericht: " + json.dumps(bericht, ensure_ascii=False, sort_keys=True))
    return urteile(bericht, args)


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
