# E8/K10 Auto-Gain-Gitter. Quelle: DspProgramm.h:238-243 (121 Stellen 20 Hz - 20 kHz),
# DspProgramm.cpp:76-83 (autoGainGitterHz), :175-214 (Stereoformel: E = 0,5(|H_L|^2+|H_R|^2), g = -10 log10(mean E)).
# Stereo-Band: H_L = H_R = H.
import numpy as np
from v2lib import bell, lowshelf, resp

fs = 48000.0
k = np.arange(121)
gitter = 20.0 * (1000.0 ** (k / 120.0))
assert abs(gitter[0] - 20.0) < 1e-12 and abs(gitter[-1] - 20000.0) < 1e-9
schritt = 1000.0 ** (1 / 120.0)   # 1/12 Oktave = Faktor 2^(log2(1000)/120)
print(f"Gitterschritt: Faktor {schritt:.6f} = {np.log2(schritt)*12:.4f}/12 Oktave (nicht exakt 1/12: log2(1000)/120 = {np.log2(1000)/120:.5f} Okt)")


def autogain(ba):
    H = resp(ba, fs, gitter)
    return -10 * np.log10(np.mean(np.abs(H) ** 2))


def dicht(ba, n=200000):
    # gleiche Gewichtung je Oktave, aber dicht: Referenz fuer "was das Gitter annaehert"
    ff = 20.0 * (1000.0 ** (np.linspace(0, 1, n)))
    H = resp(ba, fs, ff)
    return -10 * np.log10(np.mean(np.abs(H) ** 2))


print()
print("=== Bell +12 dB, Mitte innerhalb EINER Gitterzelle verschoben (um 1 kHz) ===")
i0 = np.argmin(np.abs(gitter - 1000.0))
for q in (24.0, 12.0, 8.0, 4.0, 1.0):
    werte = []
    for tt in np.linspace(0, 1, 401):
        fc = gitter[i0] * schritt ** tt
        werte.append(autogain(bell(fs, fc, q, 12.0)))
    werte = np.array(werte)
    ref = dicht(bell(fs, gitter[i0] * schritt ** 0.5, q, 12.0))
    print(f"  Q {q:5.1f}: Auto-Gain zwischen {werte.max():+.3f} und {werte.min():+.3f} dB (Spanne {werte.max()-werte.min():.3f} dB);"
          f" dichtes Mittel {ref:+.3f} dB")

print()
print("=== Dasselbe mit Bell -12 dB Q 24 (Absenkung) ===")
werte = np.array([autogain(bell(fs, gitter[i0] * schritt ** tt, 24.0, -12.0)) for tt in np.linspace(0, 1, 401)])
print(f"  Auto-Gain zwischen {werte.max():+.3f} und {werte.min():+.3f} dB (Spanne {werte.max()-werte.min():.3f} dB)")

print()
print("=== Bell +12 dB Q 24 an einer Gitterstelle gegen die Zellmitte, alle Oktaven gleich? (Stichprobe 100 Hz, 10 kHz) ===")
for fz in (100.0, 10000.0):
    j = np.argmin(np.abs(gitter - fz))
    a = autogain(bell(fs, gitter[j], 24.0, 12.0))
    b = autogain(bell(fs, gitter[j] * schritt ** 0.5, 24.0, 12.0))
    print(f"  um {fz:.0f} Hz: auf der Stelle {a:+.3f} dB, in der Zellmitte {b:+.3f} dB")
