#!/usr/bin/env python3
"""Ende-zu-Ende-Lastbein des v3-Nachrichtenwegs (SONDE-010, Kanon A22).

Startet den RUST-Probe-Broker auf einer Probe-Pipe und faehrt danach die
ECHTEN C++-Clients dagegen. Zwei Sprachen, ein Draht — das ist der einzige
Beweis, den weder ein Rust-Unittest noch ein C++-Bein allein fuehren kann.

WAS HIER GEMESSEN WIRD
----------------------
Der Gate-Text von Entwurf §65 lautet fuer SONDE-010: "CRC/Fuzz/Backpressure/
Reconnect ohne P0-Starvation". Die ersten vier Worte messen `transport_fuzz`,
der Envelope-Korpus und das C++-Bein B10. Der letzte Halbsatz ist erst dann
eine Aussage, wenn er eine ZAHL hat:

  * 32 Sonden fluten gleichzeitig Telemetrie (P2), so schnell, dass die
    Schleuse mit Cap 2 nachweislich Frames ERSETZT — sonst gaebe es keinen
    Rueckstau und der Test bewiese nichts;
  * waehrenddessen sendet jede Sonde P0-Heartbeats;
  * gemessen wird: KEIN P0-Frame geht verloren, und die P0-Antwortlatenz
    bleibt unter der Schranke.

DIE PIPE
--------
Immer ein Probe-Name mit PID und Zeitstempel. Weder dieses Bein noch eines
der beiden Programme darf die Produktions-Pipe anfassen; beide verweigern den
Dienst, wenn sie ihren Namen im Argument sehen.

Aufruf:
    py -3.13 tools/eq-copilot/pruefe_ipc_last.py [--sonden 32] [--sekunden 8]

Exitcodes: 0 gruen · 2 Gate verfehlt · 3 Voraussetzung fehlt.
"""

from __future__ import annotations

import argparse
import json
import os
import pathlib
import subprocess
import sys
import time

WURZEL = pathlib.Path(__file__).resolve().parents[2]
BROKER = WURZEL / "broker/target/release/eqcop-broker-v3probe.exe"
LAST = WURZEL / "eq-copilot/build/plugin/EqCopIpcLast_artefacts/Release/EqCopIpcLast.exe"
PRODUKTIONS_PIPE = r"\\.\pipe\evenacadia.eq-copilot.v1"


def probe_pipename() -> str:
    return (r"\\.\pipe\evenacadia.nakama.v3.last." f"{os.getpid()}.{int(time.time())}")


def main(argv: list[str]) -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--sonden", type=int, default=32)
    p.add_argument("--sekunden", type=int, default=8)
    args = p.parse_args(argv)

    for pfad, hinweis in ((BROKER, "cargo build --release --manifest-path broker/Cargo.toml"),
                          (LAST, "cmake --build eq-copilot/build --config Release "
                                 "--target EqCopIpcLast")):
        if not pfad.exists():
            print(f"VORAUSSETZUNG FEHLT: {pfad.relative_to(WURZEL)}")
            print(f"  {hinweis}")
            return 3

    pipe = probe_pipename()
    assert PRODUKTIONS_PIPE not in pipe

    print(f"Probe-Pipe: {pipe}")
    broker = subprocess.Popen(
        [str(BROKER), pipe, str(args.sekunden + 40)],
        stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE,
        text=True, encoding="utf-8", errors="replace", cwd=WURZEL)

    try:
        bereit = broker.stdout.readline().strip() if broker.stdout else ""
        if not bereit.startswith("BEREIT"):
            fehler = broker.stderr.read() if broker.stderr else ""
            print(f"ROT: Probe-Broker meldet nicht BEREIT: {bereit!r} {fehler!r}")
            return 2
        print(f"  {bereit}")

        lauf = subprocess.run(
            [str(LAST), pipe, str(args.sonden), str(args.sekunden)],
            capture_output=True, text=True, encoding="utf-8", errors="replace",
            cwd=WURZEL, timeout=args.sekunden + 120)
        print(lauf.stdout.rstrip())
        if lauf.stderr.strip():
            print(lauf.stderr.rstrip())
    finally:
        try:
            if broker.stdin:
                broker.stdin.write("STOP\n")
                broker.stdin.flush()
        except OSError:
            pass
        try:
            broker.wait(timeout=20)
        except subprocess.TimeoutExpired:
            broker.kill()
            broker.wait(timeout=10)

    rest = broker.stdout.read() if broker.stdout else ""
    bericht = {}
    for zeile in rest.splitlines():
        zeile = zeile.strip()
        if zeile.startswith("{"):
            try:
                bericht = json.loads(zeile)
            except json.JSONDecodeError:
                pass
    print("Brokerbericht: " + json.dumps(bericht, ensure_ascii=False, sort_keys=True))

    client = {}
    for zeile in lauf.stdout.splitlines():
        zeile = zeile.strip()
        if zeile.startswith("{"):
            try:
                client = json.loads(zeile)
            except json.JSONDecodeError:
                pass

    fehler = 0

    def pruefe(ok: bool, text: str, detail: str) -> None:
        nonlocal fehler
        print(("  ok      " if ok else "  ROT     ") + text + f"  [{detail}]")
        if not ok:
            fehler += 1

    pruefe(lauf.returncode == 0, "das C++-Lastbein selbst ist gruen",
           f"Exit {lauf.returncode}")
    pruefe(bool(bericht), "der Rust-Broker hat einen Bericht geliefert",
           "leer" if not bericht else f"{len(bericht)} Felder")

    if bericht:
        soll = args.sonden
        pruefe(bericht.get("control_verbindungen", 0) >= soll,
               f"{soll} Control-Verbindungen im Broker angekommen",
               str(bericht.get("control_verbindungen")))
        pruefe(bericht.get("telemetrie_verbindungen", 0) >= soll,
               f"{soll} Telemetrieverbindungen GEKOPPELT (nicht nur verbunden)",
               str(bericht.get("telemetrie_verbindungen")))
        pruefe(bericht.get("p0", 0) > 0 and bericht.get("p0_beantwortet", 0) > 0,
               "der Broker hat P0 gesehen und beantwortet",
               f"{bericht.get('p0')} empfangen / {bericht.get('p0_beantwortet')} beantwortet")
        pruefe(bericht.get("p2", 0) > 0, "und P2 ueber die zweiten Verbindungen",
               str(bericht.get("p2")))
        pruefe(bericht.get("geschlossen_envelope", 0) == 0
               and bericht.get("geschlossen_rate", 0) == 0
               and bericht.get("geschlossen_p0_ueberlauf", 0) == 0,
               "keine Verbindung wegen Envelope, Rate oder P0-Ueberlauf geschlossen",
               f"envelope={bericht.get('geschlossen_envelope')} "
               f"rate={bericht.get('geschlossen_rate')} "
               f"p0={bericht.get('geschlossen_p0_ueberlauf')}")

    if client:
        pruefe(client.get("p0_gesendet", 0) == client.get("p0_beantwortet", -1),
               "kein P0-Frame geht verloren (Clientseite)",
               f"{client.get('p0_beantwortet')}/{client.get('p0_gesendet')}")
        pruefe(client.get("p2_ersetzt", 0) > 0,
               "es lag wirklich Rueckstau an",
               f"{client.get('p2_ersetzt')} ersetzte P2-Frames")

    print("GRUEN" if fehler == 0 else "ROT")
    return 0 if fehler == 0 else 2


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
