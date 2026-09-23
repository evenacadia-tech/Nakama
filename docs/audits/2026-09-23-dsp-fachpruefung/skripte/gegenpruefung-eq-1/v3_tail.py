# Befund 3 (K13): Abklingzeit eingeschalteter Baender aus dem Polradius (RBJ, DspFilter.h:116-218) und
# dem SVF (dynamisches Band, wirksamer Gain bis +24 dB, R7 F2); zusaetzlich Zeitbereichs-Gegenprobe.
import math, numpy as np
import v5_rampe as v
fs = v.fs
def t60_rbj(typ, f, q, g):
    k = v.rbj(typ, f, q, g); r = np.abs(np.roots([1, k[3], k[4]])).max()
    tau = -1 / (fs * math.log(r)); return r, tau, 6.9078 * tau
def t60_svf(typ, f, q, g):
    A, B, C, D = v.svf_ss(v.svf(typ, f, q, g)); r = np.abs(np.linalg.eigvals(A)).max()
    tau = -1 / (fs * math.log(r)); return r, tau, 6.9078 * tau
for typ, f, q, g in (('bell', 20, 24, 12), ('bell', 20, 24, -12), ('bell', 50, 8, 12), ('bell', 1000, 2, 6), ('lowcut', 20, 24, 0),
                     ('notch', 50, 24, 0), ('lowshelf', 30, 0.707, 12)):
    r, tau, t60 = t60_rbj(typ, f, q, g)
    print(f"  RBJ {typ:8s} {f:>5} Hz Q {q:>5} {g:+3d} dB: Polradius {r:.8f}, tau {tau*1e3:8.1f} ms, T60 {t60:6.2f} s")
for typ, f, q, g in (('bell', 20, 24, 24), ('bell', 20, 24, 12)):
    r, tau, t60 = t60_svf(typ, f, q, g)
    print(f"  SVF {typ:8s} {f:>5} Hz Q {q:>5} eff {g:+3d} dB: Polradius {r:.8f}, tau {tau*1e3:8.1f} ms, T60 {t60:6.2f} s")
# Zeitbereich: Ton 20 Hz 2 s, dann Stille; Nachklang -60 dB unter Spitze der letzten Periode?
k = v.rbj('bell', 20, 24, 12); from scipy.signal import lfilter
n = np.arange(int(12 * fs)); x = np.where(n < 2 * fs, np.sin(2 * math.pi * 20 / fs * n), 0.0)
y = lfilter(k[:3], [1, k[3], k[4]], x)
ref = np.max(np.abs(y[int(1.95 * fs):int(2 * fs)])); after = np.abs(y[int(2 * fs):])
seg = int(0.05 * fs); env = np.array([after[i:i + seg].max() for i in range(0, len(after) - seg, seg)])
for db in (20, 40, 60):
    i = np.argmax(env < ref * 10 ** (-db / 20)); print(f"  Zeitbereich Bell 20 Hz Q24 +12: -{db} dB nach {i*0.05:.2f} s ab Tonende")
i4 = int(4 * fs); print(f"  Pegel des Nachklangs 4,0 s nach Tonende: {20*math.log10(after[i4:i4+seg].max()/ref):+.1f} dB re Tonpegel davor")
