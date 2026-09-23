# Preis des Gegenentwurfs: Gruppenlaufzeit des LR4-Allpasses auf M (LP^2 + HP^2 der Biquads aus DspFilter.h:187-218),
# numerisch aus der entfalteten Phase (freqz), dazu die analoge Formel 2*sqrt2/wc bei DC. Vergleich: HP 1. Ordnung auf S.
import numpy as np
from scipy import signal
from v2lib import lowcut, highcut, resp, db20
fs = 48000.0
for fc in (60.0, 120.0, 250.0):
    hp = lowcut(fs, fc, 1/np.sqrt(2)); lp = highcut(fs, fc, 1/np.sqrt(2))
    f = np.geomspace(fc/20, fc*20, 20001)
    Hap = resp(hp, fs, f)**2 + resp(lp, fs, f)**2
    ph = np.unwrap(np.angle(Hap))
    gd = -np.gradient(ph, 2*np.pi*f)
    pick = lambda x: gd[np.argmin(np.abs(f-x))]*1000
    print(f"fc {fc:5.0f} Hz: |AP| max Abweichung {np.max(np.abs(db20(Hap))):.1e} dB; Gruppenlaufzeit bei fc/10 {pick(fc/10):.2f} ms,"
          f" fc {pick(fc):.2f} ms, 2fc {pick(2*fc):.2f} ms, 10fc {pick(10*fc):.3f} ms (analog DC: {2*np.sqrt(2)/(2*np.pi*fc)*1000:.2f} ms)")
fc = 120.0
b1, a1 = signal.bilinear([1.0, 0.0], [1.0, 2*np.pi*fc], fs=fs)
for k in (0.25, 0.5, 2.0, 4.0, 10.0):
    _, h = signal.freqz(b1, a1, worN=[k*fc], fs=fs); H = h[0]
    print(f"  HP 1. Ordnung auf S, {k:5.2f}*fc: Rest-Seite {db20(H):7.2f} dB, R/L hart links {db20((1-H)/(1+H)):7.2f} dB")
