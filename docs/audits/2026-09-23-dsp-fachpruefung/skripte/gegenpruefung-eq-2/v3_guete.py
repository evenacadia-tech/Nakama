# K4/A5 Q-Semantik. Quelle: nakama-parameter-v2.json:166-177 (q 0,15-24, Default 1,0, ein Wert fuer alle Typen),
# DspFilter.h:133-218 (RBJ-Shelves mit Q statt Slope S; Cuts = EIN Biquad, 12 dB/Okt), DspProgramm.cpp:297,304.
import numpy as np
from v2lib import lowshelf, highshelf, lowcut, highcut, resp, db20

fs = 48000.0
f = np.geomspace(10.0, 0.45 * fs, 20000)

print("=== Shelves: Ueberschwinger ueber das Plateau / unter 0 dB (1 kHz, 48 kHz) ===")
for g in (12.0, -12.0):
    for q in (0.5, 0.707, 1.0, 2.0, 4.0, 24.0):
        for name, fn in (("low_shelf", lowshelf), ("high_shelf", highshelf)):
            h = db20(resp(fn(fs, 1000.0, q, g), fs, f))
            if g > 0:
                ueber = h.max() - g       # ueber dem Plateau
                unter = h.min() - 0.0     # unter 0 dB (Gegenseite)
            else:
                ueber = h.max() - 0.0     # ueber 0 dB
                unter = h.min() - g       # unter dem Plateau
            print(f"  {name:10s} {g:+5.0f} dB Q {q:6.3f}: max {h.max():+7.2f} dB, min {h.min():+7.2f} dB"
                  f"  -> ueber Soll {ueber:+6.2f} dB, unter Soll {unter:+6.2f} dB")
print()
print("  RBJ-Slope S (Cookbook) zum Vergleich: 1/Q = sqrt((A + 1/A)(1/S - 1) + 2); S = 1 (steilste monotone Flanke):")
for g in (6.0, 12.0):
    A = 10 ** (g / 40)
    for S in (1.0,):
        q = 1 / np.sqrt((A + 1 / A) * (1 / S - 1) + 2)
        print(f"    Gain {g:+.0f} dB, S = {S}: Q = {q:.4f}")

print()
print("=== Cuts: ein Biquad (12 dB/Okt), Guete = Resonanz (100 Hz bzw. 5 kHz, 48 kHz) ===")
for name, fn, fc in (("low_cut", lowcut, 100.0), ("high_cut", highcut, 5000.0)):
    for q in (0.15, 0.5, 0.707, 1.0, 2.0, 24.0):
        ba = fn(fs, fc, q)
        h = db20(resp(ba, fs, f))
        hc = db20(resp(ba, fs, fc)[0])
        i = np.argmax(h)
        print(f"  {name:8s} {fc:6.0f} Hz Q {q:6.3f}: an der Eckfrequenz {hc:+7.2f} dB, Spitze {h[i]:+7.2f} dB bei {f[i]:8.1f} Hz")
    # Steilheit weit im Sperrbereich
    ba = fn(fs, fc, 0.707)
    fa, fb = (fc / 8, fc / 16) if name == "low_cut" else (min(fc * 2.5, 0.45*fs/2), min(fc * 5, 0.45*fs))
    print(f"    Steilheit zwischen {fa:.1f} und {fb:.1f} Hz: {db20(resp(ba, fs, fa)[0]) - db20(resp(ba, fs, fb)[0]):.2f} dB je Oktave")
