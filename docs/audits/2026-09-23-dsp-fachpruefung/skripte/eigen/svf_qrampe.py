# Eigene Gegenprobe: Ueberschwingen des Simper-SVF-Bells bei Q-Absenkung (Formeln aus DspSvf.h)
import numpy as np
def entwurf(g, q, db):
    A = 10**(db/40); k = 1/(q*A)
    a1 = 1/(1+g*(g+k)); a2 = g*a1; a3 = g*a2
    return a1, a2, a3, 1.0, k*(A*A-1), 0.0
def lauf(fs, f0, db, q0, q1, rampe, fton, dauer=2.0, wechsel=1.0, takt=8):
    n = int(dauer*fs); t = np.arange(n)/fs
    x = np.sin(2*np.pi*fton*t)
    g = np.tan(np.pi*f0/fs); ic1 = ic2 = 0.0; y = np.empty(n)
    iw = int(wechsel*fs)
    k_alt = entwurf(g, q0, db); k_neu = entwurf(g, q0, db)
    for i in range(n):
        # Nakama-Takt: alle 'takt' Samples neuer Entwurf aus interpoliertem Q (in der Rampe), dazwischen linear
        if (i % takt) == 0:
            k_alt = k_neu
            if i < iw: q = q0
            elif i >= iw + rampe: q = q1
            else: q = q0 + (q1 - q0) * (i - iw + takt) / rampe
            k_neu = entwurf(g, q, db)
        w = ((i % takt) + 1) / takt
        a1, a2, a3, m0, m1, m2 = [ka + (kn - ka) * w for ka, kn in zip(k_alt, k_neu)]
        v3 = x[i] - ic2; v1 = a1*ic1 + a2*v3; v2 = ic2 + a2*ic1 + a3*v3
        ic1 = 2*v1 - ic1; ic2 = 2*v2 - ic2
        y[i] = m0*x[i] + m1*v1 + m2*v2
    stat_vor = np.max(np.abs(y[iw-int(0.2*fs):iw])); stat_nach = np.max(np.abs(y[-int(0.2*fs):]))
    spitze = np.max(np.abs(y[iw:])); return 20*np.log10(spitze/max(stat_vor, stat_nach)), stat_vor, stat_nach
for f0, q0, q1 in [(50, 24, 6), (50, 24, 12), (1000, 24, 6), (50, 6, 24)]:
    ue, sv, sn = lauf(48000, f0, 12.0, q0, q1, 256, f0)
    print(f"f0={f0} Hz Q {q0}->{q1}, +12 dB, Ton bei f0: Ueberschwingen {ue:+.1f} dB (stationaer vor {20*np.log10(sv):+.1f} dB, nach {20*np.log10(sn):+.1f} dB)")
