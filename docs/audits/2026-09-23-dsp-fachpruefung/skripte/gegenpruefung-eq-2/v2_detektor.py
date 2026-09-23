# K3 Detektor mit der Band-Guete. Quelle: DspProgramm.cpp:347-355 (entwurfBandpass(fs, freqHzWirksam, b.q)),
# DspFilter.h:224-238 (RBJ-Bandpass, 0 dB Spitze), DspKern.cpp:831-868 (Leistung -> Pegelstufe -> Huellkurve),
# DspFilter.h:289 (kPegelFensterMs = 10), DspKern.cpp:871-891 (Steuerschritt, Kennlinie).
import numpy as np
from v2lib import bandpass, lowshelf, highshelf, bell, resp, db20, detektor_kette

fs = 48000.0


def einschwingzeit(f, q, attack, hold=0.0, release=100.0, dauer=1.5, quadratur=True):
    n = int(fs * dauer)
    t = np.arange(n) / fs
    a = 0.1  # ca. -20 dBFS je Komponente, weit ueber jeder sinnvollen Schwelle unwichtig: gemessen wird e_db
    x0 = a * np.sin(2 * np.pi * f * t)
    x1 = a * np.cos(2 * np.pi * f * t) if quadratur else None
    env = detektor_kette(x0, x1, fs, f, q, attack, hold, release)
    edb = 10 * np.log10(np.maximum(env, 1e-30))
    ende = np.mean(edb[int(n * 0.8):])
    i1 = np.argmax(edb >= ende - 1.0)
    i3 = np.argmax(edb >= ende - 3.0)
    return 1000 * i1 / fs, 1000 * i3 / fs, ende


print("=== (a) Zeit bis e_db innerhalb 1 dB (bzw. 3 dB) des Endwerts, Ton auf Bandmitte ab t = 0 ===")
print("    (Stereo-Quadraturton wie E-25, damit keine 2f-Welligkeit die Zeitmessung verschmiert)")
for f, q in ((100.0, 24.0), (100.0, 12.0), (100.0, 4.0), (100.0, 1.0), (1000.0, 24.0), (1000.0, 1.0)):
    tau_bp = q / (np.pi * f) * 1000  # Huellkurvenzeitkonstante (Amplitude) eines Resonators: Q/(pi f0)
    for att in (0.1, 10.0, 100.0):
        t1, t3, e = einschwingzeit(f, q, att)
        print(f"  {f:6.0f} Hz Q {q:5.1f} (tau_BP {tau_bp:6.1f} ms)  Attack {att:6.1f} ms:  -3 dB nach {t3:6.1f} ms,  -1 dB nach {t1:6.1f} ms")

print()
print("  Einkanaliger Sinus 100 Hz Q 24 (reale Welligkeit), Attack 0,1 / 10 ms:")
for att in (0.1, 10.0):
    t1, t3, e = einschwingzeit(100.0, 24.0, att, quadratur=False)
    print(f"    Attack {att:5.1f} ms: -3 dB nach {t3:6.1f} ms, -1 dB nach {t1:6.1f} ms")

print()
print("=== (a2) Breitbandiges Material: Detektorpegel von weissem Rauschen haengt an Q (gleicher Eingang) ===")
rng = np.random.default_rng(1)
n = int(fs * 4)
x0 = rng.standard_normal(n) * 0.05
x1 = rng.standard_normal(n) * 0.05
for q in (0.707, 1.0, 4.0, 24.0):
    env = detektor_kette(x0, x1, fs, 1000.0, q, 10.0, 0.0, 100.0)
    print(f"  1 kHz Q {q:6.3f}: e_db (Mittel ab 1 s) = {10*np.log10(np.mean(env[int(fs):])):7.2f} dB")

print()
print("=== (b) Shelf-Detektor: Empfindlichkeit des Bandpasses auf fc (Q des Shelfs) gegen die Shelf-Wirkung ===")
for typ, fc, q, g in (("low_shelf", 200.0, 0.707, -6.0), ("high_shelf", 5000.0, 0.707, -6.0)):
    bp = bandpass(fs, fc, q)
    sh = lowshelf(fs, fc, q, g) if typ == "low_shelf" else highshelf(fs, fc, q, g)
    print(f"  {typ} {fc:.0f} Hz Q {q} {g:+.0f} dB:")
    for k in (0.1, 0.25, 0.5, 1.0, 2.0, 4.0, 10.0):
        f = k * fc
        if f > 0.45 * fs:
            continue
        print(f"    {k:5.2f}*fc = {f:8.1f} Hz: Detektor {db20(resp(bp, fs, f)[0]):7.2f} dB | Shelf-Wirkung {db20(resp(sh, fs, f)[0]):7.2f} dB")
