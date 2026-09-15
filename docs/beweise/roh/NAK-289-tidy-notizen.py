#!/usr/bin/env python3
"""NAK-289 Etappe 1: clang-tidy MIT Notizen fuer ausgewaehlte Uebersetzungseinheiten.

tools/plan/tidy.py zaehlt nur warning-Zeilen und verwirft die note-Zeilen. Fuer
die Einordnung je Fundstelle (welcher Aufruf wirft, welcher Pfad) braucht es die
Notizen. Dieses Skript nutzt die Werkzeugsuche und die VS-Umgebung aus tidy.py
und die gefilterte Datenbank, die der letzte volle Lauf unter
eq-copilot/build-tidy/tidy-db geschrieben hat. Es misst nichts fuer die Ratsche.

Aufruf: py -3.13 nak289-tidy-notizen.py <Datei relativ zur Wurzel> ...
"""
import pathlib
import subprocess
import sys

WURZEL = pathlib.Path(r"C:\Users\phili\Projekte\Nakama")
sys.path.insert(0, str(WURZEL / "tools" / "plan"))
import tidy  # noqa: E402

werkzeuge = tidy.finde_werkzeuge()
umgebung = tidy.vs_umgebung(werkzeuge["vcvars"])
datenbank = WURZEL / tidy.BAUORDNER / tidy.DB_ORDNER
print("clang-tidy:", werkzeuge["clang_tidy"])
print("Datenbank :", datenbank)
rc = 0
for datei in sys.argv[1:]:
    pfad = str((WURZEL / datei).resolve())
    lauf = subprocess.run(
        [werkzeuge["clang_tidy"], "--quiet", "-p", str(datenbank),
         "--header-filter=" + tidy.HEADER_FILTER, pfad],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
        env=umgebung, timeout=1800)
    print("")
    print("==== %s (clang-tidy Exit %d)" % (datei, lauf.returncode))
    for zeile in (lauf.stdout + "\n" + lauf.stderr).splitlines():
        if ": warning: " in zeile or ": note: " in zeile or ": error: " in zeile:
            print(zeile.rstrip())
    if lauf.returncode != 0:
        rc = 2
sys.exit(rc)
