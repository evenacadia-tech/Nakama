#!/usr/bin/env python3
"""SONDE-012 E-L06: echter Rust-Coordinator -> C++ Main-SourcesModel, p95."""

from __future__ import annotations

import json
import os
import pathlib
import subprocess
import sys
import time

WURZEL = pathlib.Path(__file__).resolve().parents[2]
BROKER = WURZEL / "broker/target/release/eqcop-broker-sonde012-probe.exe"
CLIENT = (WURZEL / "eq-copilot/build/plugin/"
          "EqCopSonde012SourcesLatencyTest_artefacts/Release/"
          "EqCopSonde012SourcesLatencyTest.exe")
PROBE = r"\\.\pipe\evenacadia.nakama.v3.probe."
PRODUKTION = r"\\.\pipe\evenacadia.eq-copilot.v1"
GOLDEN = r"\\.\pipe\evenacadia.nakama.v3.BNSM62JZZCCXIDV3PJZAEHMZPA"


def main() -> int:
    for pfad, bau in (
        (BROKER, "cargo build --release --manifest-path broker/Cargo.toml "
                 "--bin eqcop-broker-sonde012-probe"),
        (CLIENT, "cmake --build eq-copilot/build --config Release "
                 "--target EqCopSonde012SourcesLatencyTest"),
    ):
        if not pfad.exists():
            print(f"VORAUSSETZUNG FEHLT: {pfad.relative_to(WURZEL)}\n  {bau}")
            return 3

    for name in (PRODUKTION, GOLDEN):
        for programm in (BROKER, CLIENT):
            lauf = subprocess.run([str(programm), name], cwd=WURZEL,
                                  capture_output=True, text=True,
                                  encoding="utf-8", errors="replace", timeout=30)
            if lauf.returncode != 3:
                print(f"ROT: {programm.name} akzeptiert Nicht-Probe-Pipe {name}")
                return 2
    print("  ok      beide Programme verweigern Produktions- und Golden-Pipe")

    pipe = f"{PROBE}sonde012-l06.{os.getpid()}.{int(time.time())}"
    broker = subprocess.Popen([str(BROKER), pipe, "150"], cwd=WURZEL,
                              stdin=subprocess.PIPE, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE, text=True, encoding="utf-8",
                              errors="replace")
    try:
        bereit = broker.stdout.readline().strip() if broker.stdout else ""
        if not bereit.startswith("BEREIT "):
            detail = broker.stderr.read() if broker.stderr else ""
            print(f"ROT: Broker nicht bereit: {bereit!r} {detail!r}")
            return 2
        lauf = subprocess.run([str(CLIENT), pipe], cwd=WURZEL,
                              capture_output=True, text=True, encoding="utf-8",
                              errors="replace", timeout=140)
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

    bericht: dict[str, float] = {}
    for zeile in lauf.stdout.splitlines():
        if zeile.strip().startswith("{"):
            try:
                bericht = json.loads(zeile)
            except json.JSONDecodeError:
                pass
    grenzen = {
        "p95_16_2048_ms": 300.0, "p95_16_4096_ms": 300.0,
        "p95_16_16384_ms": 750.0, "p95_32_2048_ms": 300.0,
        "p95_32_4096_ms": 300.0, "p95_32_16384_ms": 750.0,
    }
    ok = lauf.returncode == 0 and set(bericht) == set(grenzen)
    ok = ok and all(0.0 < float(bericht[k]) <= grenze
                    for k, grenze in grenzen.items())
    print(("GRUEN" if ok else "ROT") + " - sechs p95-Grenzen und Anzeige-Revisionen")
    return 0 if ok else 2


if __name__ == "__main__":
    sys.exit(main())
