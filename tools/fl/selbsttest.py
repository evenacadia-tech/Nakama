"""Selbsttest des Laufzeit-Arms (Bein A35; Plan S25e, Register NAK-286).

Faehrt ohne FL, ohne Installation und ohne MCP-Repo die drei Selbsttests gegen
Attrappen:

    py -3.13 tools/fl/szenario.py --selbsttest
    py -3.13 tools/fl/nulltest.py --selbsttest
    pwsh -NoProfile -File tools/fl/laufzeit.ps1 -Selbsttest

Exit 0 = alle Faelle gruen, 4 = mindestens ein Fall rot, 2 = Werkzeugfehler (ein
Selbsttest nicht startbar oder mit einem anderen Exit). `--nur <fall>` reicht die
Auswahl an alle drei weiter; ein Teil ohne passenden Fall zaehlt dann nicht.
"""

from __future__ import annotations

import argparse
import os
import shutil
import subprocess
import sys
import time
from pathlib import Path

HIER = Path(__file__).resolve().parent


def main(argv: list[str]) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--nur", action="append", default=[], help="nur diese Faelle (Name oder Matrixzeile)")
    args = ap.parse_args(argv)
    try:
        sys.stdout.reconfigure(encoding="utf-8", errors="replace")  # type: ignore[attr-defined]
    except (AttributeError, ValueError):
        pass
    nur_python = [x for n in args.nur for x in ("--nur", n)]
    pwsh = shutil.which("pwsh")
    teile = [
        ("szenario.py", [sys.executable, str(HIER / "szenario.py"), "--selbsttest", *nur_python]),
        ("nulltest.py", [sys.executable, str(HIER / "nulltest.py"), "--selbsttest", *nur_python]),
        ("laufzeit.ps1", None if pwsh is None else
         [pwsh, "-NoProfile", "-File", str(HIER / "laufzeit.ps1"), "-Selbsttest",
          *(["-Nur", ",".join(args.nur)] if args.nur else [])]),
    ]
    umgebung = dict(os.environ, PYTHONIOENCODING="utf-8")
    rot = werkzeug = False
    gelaufen = 0
    for name, befehl in teile:
        if befehl is None:
            print(f"[FEHLT] {name}: pwsh nicht im Pfad")
            werkzeug = True
            continue
        t0 = time.monotonic()
        try:
            r = subprocess.run(befehl, capture_output=True, text=True, encoding="utf-8", errors="replace",
                               env=umgebung, timeout=1800)
        except (OSError, subprocess.SubprocessError) as e:
            print(f"[FEHLT] {name}: {e}")
            werkzeug = True
            continue
        ausgabe = ((r.stdout or "") + (r.stderr or "")).rstrip()
        print(f"=== {name}: Exit {r.returncode} ({time.monotonic() - t0:.1f} s) ===")
        print(ausgabe)
        if r.returncode == 2 and args.nur and "kein Fall passt" in ausgabe:
            continue
        gelaufen += 1
        if r.returncode == 4:
            rot = True
        elif r.returncode != 0:
            werkzeug = True
    if rot:
        print("SELBSTTEST A35: ROT")
        return 4
    if werkzeug or gelaufen == 0:
        print("SELBSTTEST A35: WERKZEUGFEHLER")
        return 2
    print("SELBSTTEST A35: GRUEN")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
