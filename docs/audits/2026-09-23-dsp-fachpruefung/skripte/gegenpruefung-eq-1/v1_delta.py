# Befund 1 (E9): Pegel des Delta-Hoerwegs, unabhaengig nachgerechnet.
# Produktformel: DspKern.cpp:1436-1448  l = (pl - dryL) * dbInLinear(kDeltaMakeupDb), kDeltaMakeupDb = 12 (DspProgramm.h:217)
# pl = cAusL = kompletter Committed-Pfad inkl. Input-Trim, Baender, Auto-Gain, Mix, Output-Trim (DspKern.cpp:968-1052)
# RBJ-Entwuerfe selbst ausgeschrieben nach DspFilter.h:116-218 (nicht aus nk.py importiert).
import math
import numpy as np
from scipy.signal import lfilter

fs = 48000.0
MK = 10 ** (12 / 20)          # dbInLinear(12)

def db(x): return 20 * math.log10(max(abs(x), 1e-300))

def bell(f, q, g):
    A = 10 ** (g / 40); w = 2 * math.pi * f / fs; al = math.sin(w) / (2 * q); c = math.cos(w); a0 = 1 + al / A
    return np.array([(1 + al * A) / a0, -2 * c / a0, (1 - al * A) / a0]), np.array([1, -2 * c / a0, (1 - al / A) / a0])

def lowshelf(f, q, g):
    A = 10 ** (g / 40); w = 2 * math.pi * f / fs; al = math.sin(w) / (2 * q); c = math.cos(w); z = 2 * math.sqrt(A) * al
    a0 = (A + 1) + (A - 1) * c + z
    return (np.array([A * ((A + 1) - (A - 1) * c + z), 2 * A * ((A - 1) - (A + 1) * c), A * ((A + 1) - (A - 1) * c - z)]) / a0,
            np.array([1, -2 * ((A - 1) + (A + 1) * c) / a0, ((A + 1) + (A - 1) * c - z) / a0]))

def highshelf(f, q, g):
    A = 10 ** (g / 40); w = 2 * math.pi * f / fs; al = math.sin(w) / (2 * q); c = math.cos(w); z = 2 * math.sqrt(A) * al
    a0 = (A + 1) - (A - 1) * c + z
    return (np.array([A * ((A + 1) + (A - 1) * c + z), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - z)]) / a0,
            np.array([1, 2 * ((A - 1) - (A + 1) * c) / a0, ((A + 1) - (A - 1) * c - z) / a0]))

def lowcut(f, q, g=0):
    w = 2 * math.pi * f / fs; al = math.sin(w) / (2 * q); c = math.cos(w); a0 = 1 + al
    return np.array([(1 + c) / 2, -(1 + c), (1 + c) / 2]) / a0, np.array([1, -2 * c / a0, (1 - al) / a0])

def notch(f, q, g=0):
    w = 2 * math.pi * f / fs; al = math.sin(w) / (2 * q); c = math.cos(w); a0 = 1 + al
    return np.array([1, -2 * c, 1]) / a0, np.array([1, -2 * c / a0, (1 - al) / a0])

def H(ba, f):
    b, a = ba; z = np.exp(-1j * 2 * np.pi * np.asarray(f) / fs)
    return (b[0] + b[1] * z + b[2] * z * z) / (a[0] + a[1] * z + a[2] * z * z)

print("A) Punktwerte |Delta| = |H-1| * 10^(12/20) relativ zum Dry-Pegel derselben Frequenz")
fl = np.geomspace(20, 20000, 20000)
faelle = [
    ("Bell +12 dB Q1 @1 kHz, bei f0", bell(1000, 1, 12), [1000.0]),
    ("Bell -12 dB Q1 @1 kHz, bei f0", bell(1000, 1, -12), [1000.0]),
    ("Notch Q1 @1 kHz, bei f0", notch(1000, 1), [1000.0]),
    ("Low-Shelf +12 dB Q0.707 @200 Hz, bei 30 Hz", lowshelf(200, 0.707, 12), [30.0]),
    ("Low-Cut Q0.707 @100 Hz, bei 30 Hz", lowcut(100, 0.707), [30.0]),
]
for name, ba, f in faelle:
    h = H(ba, f)[0]
    print(f"  {name:45s}: |H| {db(h):+7.2f} dB, |H-1| {db(h-1):+7.2f} dB, Delta {db((h-1)*MK):+7.2f} dB ueber Dry, "
          f"{db((h-1)*MK)-db(h):+7.2f} dB ueber Processed")

print("\nB) Maximum ueber 20 Hz..20 kHz (Delta ueber Dry derselben Frequenz)")
for name, ba in [("Low-Shelf +12 dB Q24 @1 kHz", lowshelf(1000, 24, 12)), ("High-Shelf +12 dB Q24 @1 kHz", highshelf(1000, 24, 12)),
                 ("Bell +12 dB Q24 @20 Hz", bell(20, 24, 12))]:
    h = H(ba, fl); d = np.abs(h - 1) * MK; k = int(np.argmax(d))
    print(f"  {name:32s}: max |H| {20*np.log10(np.abs(h).max()):+7.2f} dB, max Delta {20*np.log10(d[k]):+7.2f} dB bei {fl[k]:8.1f} Hz")

print("\nC) Globale Stufen (breitbandig, frequenzunabhaengig): Delta ueber Dry")
for name, g in [("Output-Trim +24 dB (Vertragsmax.)", 24), ("Output-Trim +12 dB", 12), ("Output-Trim +6 dB", 6),
                ("Output-Trim -24 dB", -24), ("eff. Bandgain +24 dB (gain+Range, R7 F2) bei f0", 24)]:
    h = 10 ** (g / 20)
    print(f"  {name:48s}: Delta {db((h-1)*MK):+7.2f} dB ueber Dry")

print("\nD) Rosa Rauschen, 10 s, Pegel des Delta-Signals gegen Dry und Processed (RMS und Spitze)")
rng = np.random.default_rng(1)
n = int(10 * fs)
wn = rng.standard_normal(n)
# Rosa per Voss-McCartney-Naeherung: 1/f-Filter (Paul Kellett 'refined')
b_p = [0.049922035, -0.095993537, 0.050612699, -0.004408786]; a_p = [1, -2.494956002, 2.017265875, -0.522189400]
x = lfilter(b_p, a_p, wn); x = x[int(fs):]; x /= np.max(np.abs(x)); x *= 10 ** (-6 / 20)   # Spitze -6 dBFS
for name, ba in [("Bell +12 dB Q1 @1 kHz", bell(1000, 1, 12)), ("Low-Shelf +12 dB Q0.707 @200 Hz", lowshelf(200, 0.707, 12)),
                 ("Low-Cut Q0.707 @100 Hz", lowcut(100, 0.707)), ("Bell +6 dB Q2 @3 kHz", bell(3000, 2, 6))]:
    y = lfilter(ba[0], ba[1], x); d = (y - x) * MK
    rms = lambda s: 20 * np.log10(np.sqrt(np.mean(s * s)))
    pk = lambda s: 20 * np.log10(np.max(np.abs(s)))
    print(f"  {name:34s}: RMS dry {rms(x):+6.1f}  proc {rms(y):+6.1f}  delta {rms(d):+6.1f} dBFS | Spitze dry {pk(x):+6.1f}  proc {pk(y):+6.1f}  delta {pk(d):+6.1f} dBFS")
