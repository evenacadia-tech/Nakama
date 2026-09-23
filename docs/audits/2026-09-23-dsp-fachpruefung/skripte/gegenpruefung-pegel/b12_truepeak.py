# B12 unabhaengig: TruePeak.h (8x, 193 Taps, Kaiser beta 8, sinc(m/8), phasenweise Summe 1)
# gegen EBU Tech 3341 V4 Tabelle 1, Faelle 15-23 (+0,2/-0,4 dB) und gegen die +-0,1-dB-Zusage (§49.3).
import numpy as np
from scipy import signal

def fe_fir():
    L, T, mitte, beta = 8, 193, 96, 8.0
    m = np.arange(T) - mitte
    ideal = np.where(m == 0, 1.0, np.sin(np.pi*m/L)/(np.pi*m/L + (m == 0)))
    h = ideal*np.i0(beta*np.sqrt(np.clip(1 - (m/mitte)**2, 0, 1)))/np.i0(beta)
    for p in range(L):
        h[p::L] /= h[p::L].sum()
    return [h[p::L] for p in range(L)]
PH = fe_fir()

def tp(x):                                  # wie tick(): Maximum ueber alle 8 Phasen (Phase 0 = Originalsample)
    return max(np.max(np.abs(signal.lfilter(g, [1.0], x))) for g in PH)

db = lambda v: 20*np.log10(v)
print("Phase 0 ist das Originalsample:", np.allclose(PH[0], np.eye(1, len(PH[0]), 12)[0]))

# A) EBU 15-19 bei 48 kHz (1 s, 10-ms-Rampen), Soll -6,0 bzw. +3,0 dBTP
fs = 48000.0; n = np.arange(int(fs)); ramp = np.minimum(1, np.minimum(n, n[::-1])/(0.01*fs))
print("\nA) EBU Tech 3341 Faelle 15-19 (Soll, gemessen, Abweichung zur Soll-Zahl; Toleranz +0,2/-0,4):")
for fall, f, a, ph, soll in [(15, fs/4, 0.5, 0.0, -6.0), (16, fs/4, 0.5, 45.0, -6.0), (17, fs/6, 0.5, 60.0, -6.0),
                             (18, fs/8, 0.5, 67.5, -6.0), (19, fs/4, 1.41, 45.0, 3.0)]:
    x = a*np.sin(2*np.pi*f*n/fs + np.deg2rad(ph))*ramp
    m = db(tp(x)); wahr = db(a)
    print("   #%d: Soll %+.1f, wahr %+.3f, gemessen %+.3f dBTP -> gegen Soll %+.3f, gegen wahren Scheitel %+.3f dB"
          % (fall, soll, wahr, m, m - soll, m - wahr))

# B) EBU 20-23: bei 4*fs synthetisiert, tiefpassgefiltert, mit Versatz 0..3 dezimiert
fs4 = 4*fs; t = np.arange(int(0.5*fs4))
f1, f2 = fs/6, fs/4
x4 = 0.5*np.sin(2*np.pi*f1*t/fs4)
start = int(round(0.25*fs4/(fs4/f1)))*int(round(fs4/f1))        # Nulldurchgang von f1
per = int(round(fs4/f2))                                        # 16 Samples = eine Periode fs/4
x4[start:start+per] = np.sin(2*np.pi*f2*np.arange(per)/fs4)
rest = np.arange(len(x4) - (start+per))
x4[start+per:] = 0.5*np.sin(2*np.pi*f1*rest/fs4)                # phasenstetig: faengt wieder bei 0 an
lp = signal.firwin(4095, 21000.0, window=("kaiser", 12.0), fs=fs4)
y4 = signal.filtfilt(lp, [1.0], x4)
ref = np.max(np.abs(signal.resample_poly(y4[start-4000:start+4000], 16, 1)))
print("\nB) EBU Faelle 20-23 (Soll 0,0 dBTP +0,2/-0,4), wahrer Scheitel des bandbegrenzten Signals %+.3f dBTP:" % db(ref))
for fall, off in [(20, 0), (21, 1), (22, 2), (23, 3)]:
    x = y4[off::4]
    m = db(tp(x)); sp = db(np.max(np.abs(x)))
    print("   #%d: Sample-Peak %+.3f, True Peak %+.3f dBTP -> gegen Soll %+.3f, gegen wahren Scheitel %+.3f dB" % (fall, sp, m, m, m - db(ref)))

# C) Wo haelt die +-0,1-dB-Zusage? Sinus bei f/fs, 64 Phasen, schlimmster Fall
print("\nC) Sinus, 64 Phasenlagen: groesster Unter-/Ueberlesefehler gegen den wahren Scheitel")
for fr in [0.10, 0.20, 0.25, 0.30, 0.35, 0.38, 0.39, 0.40, 0.41, 0.42, 0.4167, 0.44, 0.45, 0.46, 0.47]:
    nn = np.arange(1200); errs = []
    for phi in np.linspace(0, 2*np.pi, 64, endpoint=False):
        x = 0.5*np.sin(2*np.pi*fr*nn + phi)
        y = max(np.max(np.abs(signal.lfilter(g, [1.0], x)[300:1100])) for g in PH)
        errs.append(db(y/0.5))
    print("   f/fs=%.4f (%5.0f Hz bei 48 kHz, %5.0f Hz bei 44,1 kHz): min %+.3f max %+.3f dB%s"
          % (fr, fr*48000, fr*44100, min(errs), max(errs), "" if (min(errs) >= -0.1 and max(errs) <= 0.1) else "   <- ausserhalb +-0,1"))

# D) Zweiggang je Phase (Betrag), Gitterschranke 20log10(cos(pi f/8)) (TruePeak.h:26, BS.1770-5 Anh. 2 Att. 1)
print("\nD) Filtergang der 8 Zweige (min/max ueber Phasen) und Gitterschranke")
for fr in [0.30, 0.35, 0.3857, 0.39, 0.40, 0.42, 0.45, 0.47, 0.49]:
    w = 2*np.pi*fr
    mags = [abs(np.sum(g*np.exp(-1j*w*np.arange(len(g))))) for g in PH]
    print("   f/fs=%.4f: Zweige min %+.3f max %+.3f dB | Gitterschranke %+.3f dB" % (fr, db(min(mags)), db(max(mags)), db(np.cos(np.pi*fr/8))))

# E) Transient: bandbegrenzter Einzelimpuls (Kaiser-gefensterter sinc, Grenze fc), zufaellige Unterabtastlage
rng = np.random.default_rng(12)
print("\nE) Bandbegrenzter Einzelimpuls, 300 Lagen: Fehler TP_gemessen - TP_wahr")
for fc in [0.25, 0.35, 0.39, 0.42, 0.45]:
    errs = []
    for tau in rng.uniform(0, 1, 300):
        tt = np.arange(-400, 401) - tau
        win = np.i0(6.0*np.sqrt(np.clip(1 - (tt/300.0)**2, 0, 1)))/np.i0(6.0)
        x = 2*fc*np.sinc(2*fc*tt)*win
        td = np.linspace(-3, 3, 24001)
        wahr = np.max(np.abs(2*fc*np.sinc(2*fc*td)*np.i0(6.0*np.sqrt(np.clip(1-(td/300.0)**2, 0, 1)))/np.i0(6.0)))
        errs.append(db(tp(x)/wahr))
    errs = np.array(errs)
    print("   fc=%.2f fs (%5.0f Hz @48k): min %+.3f max %+.3f dB, Anteil ausserhalb +-0,1: %.0f%%" % (fc, fc*48000, errs.min(), errs.max(), 100*np.mean(np.abs(errs) > 0.1)))

# F) Bandbegrenzter Tonburst: s(t) = 2B sinc(2B t) cos(2 pi f0 t + phi), Spektrum nur in [f0-B, f0+B] < fs/2
print("\nF) Bandbegrenzter Tonburst (B = 0,03 fs, Spektrum [f0-B, f0+B]), 400 Lagen/Phasen: Fehler TP_gemessen - TP_wahr")
B = 0.03
for f0 in [0.25, 0.35, 0.38, 0.40, 0.42, 0.44, 0.46]:
    errs = []
    for _ in range(400):
        tau = rng.uniform(0, 1); phi = rng.uniform(0, 2*np.pi)
        tt = np.arange(-600, 601) - tau
        win = lambda t: np.i0(8.0*np.sqrt(np.clip(1 - (t/550.0)**2, 0, 1)))/np.i0(8.0)
        s = lambda t: 2*B*np.sinc(2*B*t)*np.cos(2*np.pi*f0*t + phi)*win(t)
        x = s(tt)
        td = np.linspace(-20, 20, 160001)
        wahr = np.max(np.abs(s(td)))
        errs.append(db(tp(x)/wahr))
    errs = np.array(errs)
    print("   f0=%.2f fs (%5.0f Hz @48k, %5.0f Hz @44,1k), Band bis %.2f fs: min %+.3f max %+.3f dB, ausserhalb +-0,1: %3.0f%%, unter -0,4: %3.0f%%"
          % (f0, f0*48000, f0*44100, f0+B, errs.min(), errs.max(), 100*np.mean(np.abs(errs) > 0.1), 100*np.mean(errs < -0.4)))
