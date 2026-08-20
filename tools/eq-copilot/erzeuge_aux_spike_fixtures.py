#!/usr/bin/env python3
"""Impuls-Fixtures fuer SONDE-004a (User-Termin A, Aux-/PDC-Spike).

Erzeugt je Projektrate eine WAV mit: Stille — EIN Impuls — Stille.

Warum je Rate eine eigene Datei: laeuft das FL-Projekt auf 44100 und die Datei
auf 48000, resampelt FL und der Impuls verschmiert ueber mehrere Samples. Fuer
den gemessenen VERSATZ (Main gegen Aux) waere das noch verkraftbar, weil beide
Wege dieselbe Datei tragen — fuer die Frage "wie scharf ist die Flanke" aber
nicht. Die Rate des Projekts und die der Datei muessen zusammenpassen.

Warum 0,9 statt 1,0: Vollaussteuerung provoziert Intersample-Overs und
Begrenzer irgendwo in der Kette; 0,9 liegt weit ueber der Impulsschwelle des
Spikes (0,25) und weit unter der Uebersteuerung.

Aufruf:  py -3.13 tools/eq-copilot/erzeuge_aux_spike_fixtures.py
Ausgabe: eq-copilot/fixtures/aux-spike/impuls-<rate>.wav  (+ SHA-256 auf stdout)

Die WAVs sind bewusst NICHT eingecheckt (regenerierbar); im Repository liegen
der Erzeuger und das Manifest mit den Hashes (Entwurf §66.3).
"""

from __future__ import annotations

import hashlib
import pathlib
import struct
import sys
import wave

RATEN = (44100, 48000)
DAUER_S = 2.0
IMPULS_BEI_S = 0.5
IMPULS_SAMPLES = 3       # kurzer Burst statt Einzelsample: ueberlebt Dither/Resampling
IMPULS_WERT = 0.9
KANAELE = 2
BREITE_BYTES = 2         # 16 Bit PCM


def erzeuge(rate: int, ziel: pathlib.Path) -> tuple[int, str]:
    """Schreibt die Datei und liefert (Impulsposition in Samples, SHA-256)."""
    gesamt = int(round(DAUER_S * rate))
    impuls_ab = int(round(IMPULS_BEI_S * rate))
    wert = int(round(IMPULS_WERT * 32767))

    rahmen = bytearray()
    for i in range(gesamt):
        s = wert if impuls_ab <= i < impuls_ab + IMPULS_SAMPLES else 0
        rahmen += struct.pack("<hh", s, s)

    ziel.parent.mkdir(parents=True, exist_ok=True)
    with wave.open(str(ziel), "wb") as w:
        w.setnchannels(KANAELE)
        w.setsampwidth(BREITE_BYTES)
        w.setframerate(rate)
        w.writeframes(bytes(rahmen))

    digest = hashlib.sha256(ziel.read_bytes()).hexdigest().upper()
    return impuls_ab, digest


def main() -> int:
    wurzel = pathlib.Path(__file__).resolve().parents[2]
    ordner = wurzel / "eq-copilot" / "fixtures" / "aux-spike"

    for rate in RATEN:
        ziel = ordner / f"impuls-{rate}.wav"
        pos, digest = erzeuge(rate, ziel)
        print(f"{ziel.name}  rate={rate}  impuls_ab_sample={pos}  "
              f"dauer_samples={int(round(DAUER_S * rate))}  sha256={digest}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
