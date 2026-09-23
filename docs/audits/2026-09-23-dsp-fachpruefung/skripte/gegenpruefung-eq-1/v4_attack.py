# Befund 4 (K2): Wirkt ein 10-ms-Pegelfenster als Boden fuer die Attack?
# Kette nach Quelle (unabhaengig ausgeschrieben):
#   Detektor-Bandpass RBJ const-0dB-peak auf freq/q des Bandes      DspFilter.h:224-238, DspProgramm.cpp:349
#   Leistung stereo (d0^2+d1^2)/2                                     DspKern.cpp:839-842
#   Pegelbegriff: symm. Ein-Pol, pol = exp(-1/(fs*0,010))            DspFilter.h:289, :306-311, :363-368; DspKern.cpp:866
#   Huellkurve Attack/Hold/Release (asymmetrisch)                    DspFilter.h:393-409; DspKern.cpp:867
#   dB = 10 log10(leistung)                                           DspFilter.h:417-422
#   Kennlinie g = r * min(1, max(0, e_db - t)/12)                    DspFilter.h:436-442; DspKern.cpp:886
#   Steuerrate 8 Samples, Entwurf wirkt mit Gewicht 0..1 ueber 8 Sa. DspProgramm.h:219-236; DspKern.cpp:871-895
import math
import numpy as np
from scipy.signal import lfilter

fs = 48000.0

def pol(ms):
    tau = ms * 1e-3
    return 0.0 if not (tau > 0) else math.exp(-1.0 / (fs * tau))

def bandpass(f, q):
    w = 2 * math.pi * f / fs; al = math.sin(w) / (2 * q); c = math.cos(w); a0 = 1 + al
    return [al / a0, 0.0, -al / a0], [1.0, -2 * c / a0, (1 - al) / a0]

def lauf(f0, q, att, hold, rel, fenster_ms, amp, thr, rng=-12.0, dur=0.25, stereo=True):
    n = int(dur * fs); t = np.arange(n) / fs
    xl = amp * np.sin(2 * math.pi * f0 * t)
    xr = amp * np.cos(2 * math.pi * f0 * t) if stereo else xl
    b, a = bandpass(f0, q)
    d0 = lfilter(b, a, xl); d1 = lfilter(b, a, xr)
    p = (d0 * d0 + d1 * d1) * 0.5
    pm = pol(fenster_ms)
    if pm > 0:  # symmetrischer Pol
        p = lfilter([1 - pm], [1, -pm], p)
    aP, rP = pol(att), pol(rel); hS = int(round(hold * 1e-3 * fs))
    L = 0.0; hr = 0; env = np.empty(n)
    for i in range(n):
        x = p[i]
        if x > L: L = aP * L + (1 - aP) * x; hr = hS
        elif hr > 0: hr -= 1
        else: L = rP * L + (1 - rP) * x
        env[i] = L
    edb = 10 * np.log10(np.maximum(env, 1e-24))
    g = rng * np.clip((edb - thr) / 12.0, 0.0, 1.0)
    return edb, g

def t_bis(sig, ziel, tol, von_unten=True):
    # erstes Sample, ab dem |sig - ziel| <= tol DAUERHAFT gilt
    ok = np.abs(sig - ziel) <= tol
    if not ok[-1]: return float('nan')
    idx = np.where(~ok)[0]
    k = 0 if len(idx) == 0 else idx[-1] + 1
    return k / fs * 1000

print("Analytisch, zwei Pole in Reihe fuer einen Leistungssprung (Attack-Pol ~0):")
for tol in (1.0, 3.0):
    frac = 10 ** (-tol / 10)
    print(f"  tau_m=10 ms: e_db auf -{tol:.0f} dB unter Endwert nach {10*math.log(1/(1-frac)):.2f} ms")

print("\nQuadraturton auf der Bandmitte (E-25), Release 100 ms, Hold 0, Range -12 dB")
print("Zeit ab Toneinsatz bis dauerhaft innerhalb 1 dB des Endwerts, fuer e_db und fuer die Auslenkung g_dyn")
print(" (Steuerrate addiert 0,17..0,31 ms bei 48 kHz, hier nicht simuliert)")
amp = 0.5   # Detektorleistung (a^2/2) = -9,03 dBFS
for f0, q in ((1000.0, 1.0), (1000.0, 0.707), (100.0, 1.0)):
    for att in (0.1, 1.0, 10.0):
        for ueber in (3.0, 6.0, 11.0, 13.0, 18.0, 30.0):
            res = []
            for fen in (10.0, 0.0):
                edb, g = lauf(f0, q, att, 0.0, 100.0, fen, amp, thr=-1e9)   # thr egal fuer e_db
                end = edb[-1]
                thr = end - ueber
                g = -12.0 * np.clip((edb - thr) / 12.0, 0.0, 1.0)
                res.append((t_bis(edb, end, 1.0), t_bis(g, g[-1], 1.0), g[-1]))
            (e10, g10, gend), (e0, g0, _) = res
            print(f"  f0 {f0:6.0f} Q {q:5.3f} Attack {att:5.1f} ms, Pegel {ueber:4.0f} dB ueber Threshold (Endauslenkung {gend:+6.2f} dB): "
                  f"e_db {e10:6.2f} ms (ohne Fenster {e0:5.2f}) | g_dyn {g10:6.2f} ms (ohne Fenster {g0:5.2f})")
        print()
