#!/usr/bin/env python3
"""Verifikator fuer das Kalibrier-Teststueck 01 (7 Pruefungen).

Prueft die GESCHRIEBENEN Dateien mit einem fremden Parser (mido) — eine
Pruefung mit derselben Rechenregel wie der Schreiber wuerde einen Regelfehler
nur bestaetigen (Lehre aus tools/midi-ideen).

Aufruf:  py -3.13 tools/eq-copilot/verify_testsong.py
"""
from __future__ import annotations

import importlib.util
import sys
from collections import Counter
from pathlib import Path

import mido

WURZEL = Path(__file__).resolve().parents[2]
ZIEL = WURZEL / "eq-copilot" / "kalibration" / "testsong-01"

spec = importlib.util.spec_from_file_location(
    "erzeuge_testsong", Path(__file__).with_name("erzeuge_testsong.py"))
gen = importlib.util.module_from_spec(spec)
spec.loader.exec_module(gen)  # type: ignore[union-attr]

BAR = 384          # PPQ 96 * 4
T9 = 8 * BAR       # Tick-Start Takt 9
ENDE_T16 = 16 * BAR
ENDE_T24 = 24 * BAR
T24 = 23 * BAR
BBM_PC = {10, 1, 5}          # A# C# F — erlaubte Pitch-Klassen im Schlusstakt
NUR_VOLL = {"02_Klavier-2_Arpeggio.mid", "05_Pad-Elektronisch.mid",
            "07_Schlagzeug_GM.mid", "08_Gitarre_Motiv.mid"}
AB_T9 = NUR_VOLL | {"04_Pad-Chor.mid"}
SCHLUSS_DATEIEN = {"01_Klavier-1_Haupt.mid", "03_Pad-Streicher.mid",
                   "04_Pad-Chor.mid", "06_Bass.mid"}

fehler = 0


def melde(ok: bool, was: str, detail: str = "") -> None:
    global fehler
    if ok:
        return
    fehler += 1
    print(f"  FEHLER {was} — {detail}")


def datei_events(pfad: Path):
    """(note_on-Multiset, note_off-Multiset, [(tick,key,vel,on?)]) via mido."""
    mid = mido.MidiFile(str(pfad))
    ons: Counter = Counter()
    offs: Counter = Counter()
    fluss = []
    for spur in mid.tracks:
        tick = 0
        for ev in spur:
            tick += ev.time
            if ev.type == "note_on" and ev.velocity > 0:
                ons[(tick, ev.note, ev.velocity)] += 1
                fluss.append((tick, ev.note, ev.velocity, True))
            elif ev.type in ("note_off", "note_on"):
                offs[(tick, ev.note)] += 1
                fluss.append((tick, ev.note, 0, False))
    return ons, offs, sorted(fluss, key=lambda e: (e[0], e[3]))


def main() -> int:
    tracks = gen.baue()
    gen.audit(tracks)

    # V1 Determinismus: zweiter Bau ist ereignisgleich zum ersten.
    tracks2 = gen.baue()
    for name in tracks:
        melde(tracks[name].notes == tracks2[name].notes,
              "V1 Determinismus", name)

    dateien = sorted(p.name for p in ZIEL.glob("*.mid"))
    melde(dateien == sorted(tracks.keys()), "V1 Dateibestand",
          f"{dateien} vs {sorted(tracks.keys())}")

    for name, tr in tracks.items():
        pfad = ZIEL / name
        if not pfad.exists():
            melde(False, "V2 Datei fehlt", name)
            continue
        ons, offs, fluss = datei_events(pfad)

        # V2 Round-Trip: Ereignismengen == Speicherzustand des Generators
        # (note_on/off-MULTISETS — Paar-Vergleich waere bei Ueberlappung falsch).
        soll_on = Counter((t, k, v) for t, k, l, v in tr.notes)
        soll_off = Counter((t + l, k) for t, k, l, v in tr.notes)
        melde(ons == soll_on, "V2 note_on-Menge", name)
        melde(offs == soll_off, "V2 note_off-Menge", name)

        # V3 Tonleiter/Kit unabhaengig auf der DATEI.
        drums = "Schlagzeug" in name
        for (_t, key, _v), _n in ons.items():
            if drums:
                melde(key in gen.KIT, "V3 Kit-Vokabular", f"{name}: {key}")
            else:
                melde(key % 12 in gen.SCALE, "V3 Tonleiter", f"{name}: {key}")

        # V4 Same-Pitch-Overlap: nie zwei aktive Noten gleicher Tonhoehe.
        aktiv: Counter = Counter()
        for tick, key, _vel, ist_on in fluss:
            if ist_on:
                aktiv[key] += 1
                melde(aktiv[key] <= 1, "V4 Ueberlappung", f"{name}: {key} @ {tick}")
            else:
                aktiv[key] -= 1

        # V5 Struktur: Teil-Fenster + Schlusstakt-Tonika + Gesamtende.
        erster_on = min(t for (t, _k, _v) in soll_on)
        letzter_off = max(t for (t, _k) in soll_off)
        if name in AB_T9:
            melde(erster_on >= T9, "V5 beginnt erst im vollen Teil",
                  f"{name}: Tick {erster_on}")
        if name in NUR_VOLL:
            melde(letzter_off <= ENDE_T16, "V5 endet mit dem vollen Teil",
                  f"{name}: Tick {letzter_off}")
        melde(letzter_off <= ENDE_T24, "V5 endet im Stueck", f"{name}: {letzter_off}")
        if name in SCHLUSS_DATEIEN:
            for (t, key, _v) in soll_on:
                if t >= T24 and not drums:
                    melde(key % 12 in BBM_PC, "V5 Schlusstakt = Tonika",
                          f"{name}: MIDI {key}")

        # V6 Sektions-Dynamik: voller Teil lauter als ruhiger (01 + 06).
        if name in ("01_Klavier-1_Haupt.mid", "06_Bass.mid"):
            ruhig = [v for (t, _k, v) in soll_on if t < T9]
            voll = [v for (t, _k, v) in soll_on if T9 <= t < ENDE_T16]
            melde(bool(ruhig) and bool(voll)
                  and sum(voll) / len(voll) > sum(ruhig) / len(ruhig),
                  "V6 Sektions-Dynamik", name)

    # V7 Doku nennt jede Datei.
    lies = (ZIEL / "00_LIES-MICH.md")
    melde(lies.exists(), "V7 LIES-MICH fehlt")
    if lies.exists():
        text = lies.read_text(encoding="utf-8")
        for name in tracks:
            melde(name in text, "V7 Doku nennt Datei", name)

    gesamt = sum(len(t.notes) for t in tracks.values())
    print(f"{'TESTSONG OK' if fehler == 0 else 'TESTSONG FEHLGESCHLAGEN'} — "
          f"8 Dateien, {gesamt} Noten, {fehler} Fehler")
    return 0 if fehler == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
