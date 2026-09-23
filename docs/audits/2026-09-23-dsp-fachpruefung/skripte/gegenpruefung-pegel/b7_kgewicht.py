# B7 unabhaengig: Nakama-K-Kette (KGewichtung.h:63-116, RBJ) gegen BS.1770-5 Tab. 1/2 (48 kHz)
# und gegen eine frequenzgang-treue Neuberechnung bei anderen Raten (BS.1770-5 S. 4/5:
# "same frequency response that the specified filter provides at 48 kHz").
import numpy as np
from scipy import signal, optimize

def rbj_shelf(fs, G=4.0, Q=1/np.sqrt(2), fc=1500.0):
    A = 10**(G/40); w0 = 2*np.pi*fc/fs; al = np.sin(w0)/(2*Q); c = np.cos(w0)
    a0 = (A+1) - (A-1)*c + 2*np.sqrt(A)*al
    b = np.array([A*((A+1)+(A-1)*c+2*np.sqrt(A)*al), -2*A*((A-1)+(A+1)*c), A*((A+1)+(A-1)*c-2*np.sqrt(A)*al)])/a0
    a = np.array([1.0, 2*((A-1)-(A+1)*c)/a0, ((A+1)-(A-1)*c-2*np.sqrt(A)*al)/a0])
    return b, a

def rbj_hp(fs, Q=0.5, fc=38.0):
    w0 = 2*np.pi*fc/fs; al = np.sin(w0)/(2*Q); c = np.cos(w0); a0 = 1+al
    return np.array([(1+c)/2, -(1+c), (1+c)/2])/a0, np.array([1.0, -2*c/a0, (1-al)/a0])

ITU_S = (np.array([1.53512485958697, -2.69169618940638, 1.19839281085285]), np.array([1.0, -1.69065929318241, 0.73248077421585]))
ITU_H = (np.array([1.0, -2.0, 1.0]), np.array([1.0, -1.99004745483398, 0.99007225036621]))

def H(ba, f, fs):
    _, h = signal.freqz(ba[0], ba[1], worN=np.atleast_1d(f), fs=fs)
    return h

def kdb(stages, f, fs):
    h = np.ones(len(np.atleast_1d(f)), complex)
    for s in stages: h = h*H(s, f, fs)
    return 20*np.log10(np.abs(h))

fs = 48000.0
bs, as_ = rbj_shelf(fs); bh, ah = rbj_hp(fs)
print("RBJ-HP b0 = %.8f (ITU 1.0) -> %.5f dB;  a1 %.14f vs %.14f;  a2 %.14f vs %.14f" % (bh[0], 20*np.log10(bh[0]), ah[1], ITU_H[1][1], ah[2], ITU_H[1][2]))
print("RBJ-Shelf b = %s\n      ITU b = %s\n  RBJ a = %s\n  ITU a = %s" % (bs, ITU_S[0], as_, ITU_S[1]))

f = np.geomspace(20, 20000, 6000)
d_hp = kdb([(bh, ah)], f, fs) - kdb([ITU_H], f, fs)
d_sh = kdb([(bs, as_)], f, fs) - kdb([ITU_S], f, fs)
d = d_hp + d_sh
print("\n48 kHz, Anteil Hochpass: min %.4f max %.4f dB (1 kHz: %.4f)" % (d_hp.min(), d_hp.max(), np.interp(1000, f, d_hp)))
print("48 kHz, Anteil Shelf   : min %.4f max %.4f dB (1 kHz: %.4f)" % (d_sh.min(), d_sh.max(), np.interp(1000, f, d_sh)))
i = np.argmax(np.abs(d)); print("48 kHz, Kette gesamt   : min %.4f max %.4f dB, |max| bei %.0f Hz" % (d.min(), d.max(), f[i]))
for fx in [20, 30, 50, 100, 200, 500, 997, 1500, 2000, 3000, 5000, 10000, 16000, 20000]:
    print("   %6d Hz: RBJ-ITU %+.4f dB (HP %+.4f, Shelf %+.4f)" % (fx, np.interp(fx, f, d), np.interp(fx, f, d_hp), np.interp(fx, f, d_sh)))
g997 = kdb([(bs, as_), (bh, ah)], 997.0, fs)[0]; g997i = kdb([ITU_S, ITU_H], 997.0, fs)[0]
print("K-Gang bei 997 Hz: ITU %+.5f dB (-0,691-Konstante kompensiert %.3f), RBJ %+.5f dB -> 0-dBFS-Sinus: ITU %.3f LKFS, RBJ %.3f LKFS"
      % (g997i, 0.691, g997, -0.691 + 10*np.log10(0.5) + g997i + 0.0, -0.691 + 10*np.log10(0.5) + g997))

# Lautheitsdifferenz fuer Rauschspektren (Leistungsintegral ueber 20 Hz..20 kHz)
def lufs_delta(stA, fsA, stB, fsB, slope, flo=20.0, fhi=20000.0):
    ff = np.geomspace(flo, fhi, 40000); w = np.gradient(ff)
    psd = (ff/1000.0)**(slope/(10*np.log10(2)))
    pa = np.sum(psd*10**(kdb(stA, ff, fsA)/10)*w); pb = np.sum(psd*10**(kdb(stB, ff, fsB)/10)*w)
    return 10*np.log10(pa/pb)
print("\nLUFS(RBJ) - LUFS(ITU) bei 48 kHz:")
for sl, nm in [(0.0, "weiss"), (-3.0103, "rosa"), (-4.5, "-4,5 dB/Okt"), (-6.0206, "braun")]:
    print("   %-12s %+.4f LU" % (nm, lufs_delta([(bs, as_), (bh, ah)], fs, [ITU_S, ITU_H], fs, sl)))

# Zeitbereichs-Gegenprobe: echtes Rauschen durch beide Ketten
rng = np.random.default_rng(7)
n = 48000*30
for nm, slope in [("weiss", 0.0), ("rosa", -3.0103)]:
    X = np.fft.rfft(rng.standard_normal(n)); fr = np.fft.rfftfreq(n, 1/fs); fr[0] = fr[1]
    x = np.fft.irfft(X*(fr/1000.0)**(slope/(20*np.log10(2))), n)
    ya = signal.lfilter(bh, ah, signal.lfilter(bs, as_, x)); yb = signal.lfilter(*ITU_H, signal.lfilter(*ITU_S, x))
    print("   Zeitbereich %-6s: %+.4f LU" % (nm, 10*np.log10(np.mean(ya[48000:]**2)/np.mean(yb[48000:]**2))))

# Andere Raten: Ziel = ITU-48-kHz-Gang in Hz. Referenz: an ITU-Tabelle angepasste Analogparameter
# (Bilinear mit Vorverzerrung, Form libebur128/DeMan), bei 48 kHz exakt = Tabelle.
def deman(fs):
    f0 = 1681.974450955533; G = 3.999843853973347; Q = 0.7071752369554196
    K = np.tan(np.pi*f0/fs); Vh = 10**(G/20); Vb = Vh**0.4996667741545416
    a0 = 1 + K/Q + K*K
    pb = np.array([(Vh + Vb*K/Q + K*K)/a0, 2*(K*K - Vh)/a0, (Vh - Vb*K/Q + K*K)/a0]); pa = np.array([1.0, 2*(K*K-1)/a0, (1 - K/Q + K*K)/a0])
    f0 = 38.13547087602444; Q = 0.5003270373238773; K = np.tan(np.pi*f0/fs)
    return (pb, pa), (np.array([1.0, -2.0, 1.0]), np.array([1.0, 2*(K*K-1)/(1+K/Q+K*K), (1-K/Q+K*K)/(1+K/Q+K*K)]))
dm = deman(48000.0)
print("\nKontrolle DeMan@48k gegen Tabelle: max |diff| = %.2e dB" % np.max(np.abs(kdb(list(dm), f, fs) - kdb([ITU_S, ITU_H], f, fs))))
print("Andere Raten: RBJ(fs) gegen ITU-48k-Gang in Hz (bis min(20k, 0,45 fs)), 997-Hz-Versatz, LUFS rosa/-4,5:")
for fsx in [44100.0, 48000.0, 88200.0, 96000.0, 192000.0]:
    ff = np.geomspace(20, min(20000.0, 0.45*fsx), 4000)
    r = [rbj_shelf(fsx), rbj_hp(fsx)]
    dd = kdb(r, ff, fsx) - kdb([ITU_S, ITU_H], ff, 48000.0)
    m15 = ff <= 15000
    print("   fs=%6.0f: max|d| %.4f dB bei %5.0f Hz (<=15 kHz: %.4f) | 997 Hz: %+.4f dB | rosa %+.4f LU | -4,5 %+.4f LU"
          % (fsx, np.max(np.abs(dd)), ff[np.argmax(np.abs(dd))], np.max(np.abs(dd[m15])), np.interp(997, ff, dd),
             lufs_delta(r, fsx, [ITU_S, ITU_H], 48000.0, -3.0103, 20, min(20000, 0.45*fsx)),
             lufs_delta(r, fsx, [ITU_S, ITU_H], 48000.0, -4.5, 20, min(20000, 0.45*fsx))))
