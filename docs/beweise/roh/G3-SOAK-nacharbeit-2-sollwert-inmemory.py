#!/usr/bin/env python3
"""Rotbeweis Runde 2 — der K-S5-Sollwert darf nicht aus dem Bericht kommen.

Faehrt NUR `urteile()` in-memory, je einmal gegen den Pruefer VOR dem Fix
(Stand 882b2aa) und gegen den Stand danach. Drei Faelle, alle auf demselben
gespeicherten gruenen Bericht:

  a) erwartet = 0, backoff_deckel_erreicht = 0, gefahren = true,
     urteil = getroffen   -> muss Exit 2 sein
  b) `erwartet` fehlt, backoff_deckel_erreicht = 0, gefahren = true,
     urteil = nicht_getroffen (in sich konsistent)  -> muss Exit 2 sein
  c) der unveraenderte gruene Bericht  -> muss Exit 0 bleiben

Aufruf vom Workspace-Root, mit dem alten Pruefstand als Argument:

    git show 882b2aa:tools/eq-copilot/pruefe_session_soak.py > <tmp>/pruefe_alt.py
    py -3.13 docs/beweise/roh/G3-SOAK-nacharbeit-2-sollwert-inmemory.py <tmp>/pruefe_alt.py

Der alte Stand wird bewusst NICHT ins Repo gelegt: er steht in der Historie.
"""

import copy
import importlib.util
import json
import pathlib
import sys
import types

WURZEL = pathlib.Path(__file__).resolve().parents[3]
BERICHT = WURZEL / "docs/beweise/roh/G3-SOAK-nacharbeit-1-killpunkt-bericht.json"
STAENDE = {
    "VOR dem Fix (882b2aa)": pathlib.Path(sys.argv[1]),
    "NACH dem Fix (Runde 2)": WURZEL / "tools/eq-copilot/pruefe_session_soak.py",
}


def lade(pfad, name):
    spec = importlib.util.spec_from_file_location(name, pfad)
    modul = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(modul)
    return modul


def fall_a(b):
    b["kill"]["k_s5"].update({"backoff_deckel_erreicht": 0, "erwartet": 0,
                              "gefahren": True, "urteil": "getroffen"})


def fall_b(b):
    del b["kill"]["k_s5"]["erwartet"]
    b["kill"]["k_s5"].update({"backoff_deckel_erreicht": 0, "gefahren": True,
                              "urteil": "nicht_getroffen"})


def fall_c(b):
    pass


FAELLE = [
    ("a", "erwartet=0, backoff=0, gefahren=true, urteil=getroffen", fall_a, 2),
    ("b", "erwartet fehlt, backoff=0, urteil=nicht_getroffen", fall_b, 2),
    ("c", "unveraenderter gruener Bericht", fall_c, 0),
]

roh = json.loads(BERICHT.read_text(encoding="utf-8"))
args = types.SimpleNamespace(sonden=16, minuten=5, neustarts=2, langsam=0.25,
                             langsam_ms=120, mutant=None, mutant_liste=False,
                             bericht=str(BERICHT))
bilanz = []
for standname, pfad in STAENDE.items():
    modul = lade(pfad, "soak_" + standname.split()[0].lower())
    for kennung, text, mutation, soll in FAELLE:
        print("=" * 78)
        print(f"== {standname} · Fall {kennung}: {text}")
        print(f"== erwarteter Exit: {soll}")
        print("=" * 78)
        b = copy.deepcopy(roh)
        mutation(b)
        exit_ = modul.urteile(b, args)
        print(f"== Exit {exit_} (erwartet {soll}) -> "
              f"{'PASSEND' if exit_ == soll else 'ABWEICHUNG'}\n")
        bilanz.append((standname, kennung, soll, exit_))

print("=" * 78)
print("== BILANZ")
print("=" * 78)
for standname, kennung, soll, exit_ in bilanz:
    print(f"  {standname:24s} Fall {kennung}: Exit {exit_} (Sollwert-Riegel "
          f"verlangt {soll} nach dem Fix)")
