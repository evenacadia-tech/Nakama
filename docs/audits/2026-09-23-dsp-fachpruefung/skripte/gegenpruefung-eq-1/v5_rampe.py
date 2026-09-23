# Befund 5 (B2/K7): Rampe statischer Baender INNERHALB der NAK-311-Sprungkriterien.
# Produkt (unabhaengig ausgeschrieben):
#   DF2T-Tick                                   DspFilter.h:90-96
#   lineare Koeffizientenmischung               DspKern.cpp:43-52, Aufruf :902
#   t = 1 - (rampeRest - i - 1)/256 -> (i+1)/256 DspKern.cpp:793-795, rest = kRampeSamples = 256 (:633, DspProgramm.h:91)
#   Zustand wandert mit                         DspKern.cpp:595-603
#   Rampenweg nur wenn Kriterien halten         DspKern.cpp:360-366; DspProgramm.h:150,158,168 (2,0 / 4,0 / 20 dB, strikt groesser reisst)
# Vergleich:
#   S) TPT-SVF (Zavalishin/Simper), Parameter je Sample interpoliert (log f, log Q, dB linear), Zustand stetig
#   X) Crossfade 256 Samples, warmes A gegen KALT gestartetes B (Weg ueber dem Kriterium, DspKern.cpp:1116-1125, R-311-13)
#   QS) quasistatische Huelle: max_t |H_t(w)| der Zwischenentwuerfe (Produkt: lineare Koeffizientenmischung; Param: Neuentwurf)
# Mass wie R-311-13 / B6 Abschnitt R: Spitze im Fenster 0,5 s ab Umschaltsample gegen den groesseren der beiden
# stationaeren Pegel dieses Tons (dB). Eingang: Sinus Amplitude 1, eingeschwungen (analytischer Zustand).
import math, sys, itertools, json
import numpy as np
from scipy.signal import lfilter

fs = 48000.0
R = 256
NWIN = int(0.5 * fs)
TOENE = np.unique(np.concatenate([np.geomspace(20.0, 20000.0, 97)]))
W = 2 * np.pi * TOENE / fs

# ---------------- RBJ (DspFilter.h:116-238) ----------------
def rbj(typ, f, q, g):
    w0 = 2 * math.pi * f / fs; al = math.sin(w0) / (2 * q); c = math.cos(w0)
    if typ == 'bell':
        A = 10 ** (g / 40); a0 = 1 + al / A
        return np.array([(1 + al * A) / a0, -2 * c / a0, (1 - al * A) / a0, -2 * c / a0, (1 - al / A) / a0])
    if typ == 'lowshelf':
        A = 10 ** (g / 40); z = 2 * math.sqrt(A) * al; a0 = (A + 1) + (A - 1) * c + z
        return np.array([A * ((A + 1) - (A - 1) * c + z) / a0, 2 * A * ((A - 1) - (A + 1) * c) / a0,
                         A * ((A + 1) - (A - 1) * c - z) / a0, -2 * ((A - 1) + (A + 1) * c) / a0, ((A + 1) + (A - 1) * c - z) / a0])
    if typ == 'highshelf':
        A = 10 ** (g / 40); z = 2 * math.sqrt(A) * al; a0 = (A + 1) - (A - 1) * c + z
        return np.array([A * ((A + 1) + (A - 1) * c + z) / a0, -2 * A * ((A - 1) + (A + 1) * c) / a0,
                         A * ((A + 1) + (A - 1) * c - z) / a0, 2 * ((A - 1) - (A + 1) * c) / a0, ((A + 1) - (A - 1) * c - z) / a0])
    a0 = 1 + al
    if typ == 'notch':   return np.array([1 / a0, -2 * c / a0, 1 / a0, -2 * c / a0, (1 - al) / a0])
    if typ == 'lowcut':  return np.array([(1 + c) / 2 / a0, -(1 + c) / a0, (1 + c) / 2 / a0, -2 * c / a0, (1 - al) / a0])
    if typ == 'highcut': return np.array([(1 - c) / 2 / a0, (1 - c) / a0, (1 - c) / 2 / a0, -2 * c / a0, (1 - al) / a0])
    raise ValueError(typ)

def Hrbj(k, w):
    z = np.exp(-1j * w)
    return (k[0] + k[1] * z + k[2] * z * z) / (1 + k[3] * z + k[4] * z * z)

# ---------------- TPT-SVF (Simper), dieselben analogen Prototypen, BLT mit Vorverzerrung auf f0 ----------------
def svf(typ, f, q, g):
    A = 10 ** (g / 40); G = math.tan(math.pi * f / fs); k = 1 / q
    if typ == 'bell':      k = 1 / (q * A); m = (1.0, k * (A * A - 1), 0.0)
    elif typ == 'lowshelf':  G = G / math.sqrt(A); m = (1.0, k * (A - 1), A * A - 1)
    elif typ == 'highshelf': G = G * math.sqrt(A); m = (A * A, k * (1 - A) * A, 1 - A * A)
    elif typ == 'notch':   m = (1.0, -k, 0.0)
    elif typ == 'lowcut':  m = (1.0, -k, -1.0)
    elif typ == 'highcut': m = (0.0, 0.0, 1.0)
    a1 = 1 / (1 + G * (G + k)); a2 = G * a1; a3 = G * a2
    return (a1, a2, a3) + m

def svf_ss(c):
    a1, a2, a3, m0, m1, m2 = c
    A = np.array([[2 * a1 - 1, -2 * a2], [2 * a2, 1 - 2 * a3]]); B = np.array([2 * a2, 2 * a3])
    C = np.array([m1 * a1 + m2 * a2, -m1 * a2 + m2 * (1 - a3)]); D = m0 + m1 * a2 + m2 * a3
    return A, B, C, D

def Hsvf(c, w):
    A, B, C, D = svf_ss(c)
    out = np.empty(len(w), complex)
    for i, wi in enumerate(w):
        out[i] = C @ np.linalg.solve(np.exp(1j * wi) * np.eye(2) - A, B) + D
    return out

def fall(typ, fA, qA, gA, fB, qB, gB, n0=200000):
    kA = rbj(typ, fA, qA, gA); kB = rbj(typ, fB, qB, gB)
    HA = Hrbj(kA, W); HB = Hrbj(kB, W)
    ref = np.maximum(np.abs(HA), np.abs(HB))
    n = np.arange(n0, n0 + NWIN)
    x_of = lambda nn: np.sin(np.outer(np.atleast_1d(nn), W))     # (len(nn), K)
    # ---------- Produkt ----------
    yA = lambda nn: np.imag(HA * np.exp(1j * np.outer(np.atleast_1d(nn), W)))
    z1 = (yA(n0) - kA[0] * x_of(n0))[0]
    z2 = ((yA(n0 + 1) - kA[0] * x_of(n0 + 1)) - kA[1] * x_of(n0) + kA[3] * yA(n0))[0]
    pk = np.zeros(len(W))
    X = x_of(n[:R])
    for i in range(R):
        t = (i + 1) / R
        b0, b1, b2, a1, a2 = kA + (kB - kA) * t
        x = X[i]; y = b0 * x + z1
        z1 = b1 * x - a1 * y + z2; z2 = b2 * x - a2 * y
        pk = np.maximum(pk, np.abs(y))
    for j in range(len(W)):
        xs = np.sin(W[j] * n[R:])
        y, _ = lfilter(kB[:3], [1, kB[3], kB[4]], xs, zi=[z1[j], z2[j]])
        pk[j] = max(pk[j], np.max(np.abs(y)))
    P = 20 * np.log10(pk / ref)
    # ---------- SVF, Parameter interpoliert ----------
    cA = svf(typ, fA, qA, gA); AA, BA, CA, DA = svf_ss(cA)
    S0 = np.array([np.linalg.solve(np.exp(1j * wi) * np.eye(2) - AA, BA) for wi in W])   # (K,2)
    s = np.imag(S0 * np.exp(1j * W * n0)[:, None])                                      # Zustand vor n0
    ic1 = s[:, 0].copy(); ic2 = s[:, 1].copy()
    ps = np.zeros(len(W))
    lfA, lfB, lqA, lqB = math.log(fA), math.log(fB), math.log(qA), math.log(qB)
    for i in range(R):
        t = (i + 1) / R
        a1, a2, a3, m0, m1, m2 = svf(typ, math.exp(lfA + (lfB - lfA) * t), math.exp(lqA + (lqB - lqA) * t), gA + (gB - gA) * t)
        x = X[i]; v3 = x - ic2; v1 = a1 * ic1 + a2 * v3; v2 = ic2 + a2 * ic1 + a3 * v3
        ic1 = 2 * v1 - ic1; ic2 = 2 * v2 - ic2
        ps = np.maximum(ps, np.abs(m0 * x + m1 * v1 + m2 * v2))
    cB = svf(typ, fB, qB, gB); AB, BB, CB, DB = svf_ss(cB)
    # SVF-Zustand -> gleichwertiger DF2T-Zustand des RBJ-Entwurfs B (gleiche Uebertragungsfunktion; Nullinput-Antwort)
    st = np.stack([ic1, ic2], 1)                 # (K,2)
    y0 = st @ CB; y1 = (st @ AB.T) @ CB
    zz1 = y0; zz2 = y1 + kB[3] * y0
    for j in range(len(W)):
        xs = np.sin(W[j] * n[R:])
        y, _ = lfilter(kB[:3], [1, kB[3], kB[4]], xs, zi=[zz1[j], zz2[j]])
        ps[j] = max(ps[j], np.max(np.abs(y)))
    S = 20 * np.log10(ps / ref)
    # ---------- Crossfade: warmes A, kaltes B ----------
    px = np.zeros(len(W))
    for j in range(len(W)):
        xs = np.sin(W[j] * n)
        yb = lfilter(kB[:3], [1, kB[3], kB[4]], xs)
        ya = np.abs(HA[j]) * np.sin(W[j] * n + np.angle(HA[j]))
        wgt = np.ones(NWIN); wgt[:R] = np.arange(R) / R
        px[j] = np.max(np.abs(ya * (1 - wgt) + yb * wgt))
    Xf = 20 * np.log10(px / ref)
    # ---------- quasistatisch ----------
    ts = np.arange(1, R + 1) / R
    qsm = np.max(np.abs(np.array([Hrbj(kA + (kB - kA) * t, W) for t in ts])), 0)
    qsp = np.max(np.abs(np.array([Hrbj(rbj(typ, math.exp(lfA + (lfB - lfA) * t), math.exp(lqA + (lqB - lqA) * t), gA + (gB - gA) * t), W) for t in ts])), 0)
    QSm = 20 * np.log10(qsm / ref); QSp = 20 * np.log10(qsp / ref)
    absP = 20 * np.log10(pk); absS = 20 * np.log10(ps)
    return P, S, Xf, QSm, QSp, absP, absS

def selbsttest():
    # SVF-B muss als LTI dasselbe tun wie RBJ-B (sonst taugt die Zustandsumrechnung nicht)
    for typ, f, q, g in (('bell', 1000, 24, 12), ('lowshelf', 200, 0.707, -12), ('highshelf', 5000, 3, 9), ('notch', 60, 24, 0),
                         ('lowcut', 25, 24, 0), ('highcut', 15000, 0.15, 0)):
        d = np.max(np.abs(Hsvf(svf(typ, f, q, g), W) - Hrbj(rbj(typ, f, q, g), W)))
        print(f"  Selbsttest {typ:9s}: max |H_svf - H_rbj| = {d:.2e}")
    # Zeitbereich: SVF je Sample gegen umgerechneten DF2T-Zustand
    typ, f, q, g = 'bell', 300, 24, 12
    c = svf(typ, f, q, g); A, B, C, D = svf_ss(c); k = rbj(typ, f, q, g)
    rng = np.random.default_rng(3); s = rng.standard_normal(2); x = rng.standard_normal(500)
    a1, a2, a3, m0, m1, m2 = c; ic1, ic2 = s; ys = []
    for xi in x:
        v3 = xi - ic2; v1 = a1 * ic1 + a2 * v3; v2 = ic2 + a2 * ic1 + a3 * v3
        ic1 = 2 * v1 - ic1; ic2 = 2 * v2 - ic2; ys.append(m0 * xi + m1 * v1 + m2 * v2)
    y0 = C @ s; y1 = C @ (A @ s)
    yd, _ = lfilter(k[:3], [1, k[3], k[4]], x, zi=[y0, y1 + k[3] * y0])
    print(f"  Selbsttest Zustandsumrechnung: max Abweichung {np.max(np.abs(np.array(ys) - yd)):.2e}")

if __name__ == '__main__':
    selbsttest()
    faelle = []
    GAINTYP = ('bell', 'lowshelf', 'highshelf')
    # a) Frequenzsprung genau auf dem Kriterium (Faktor 2), beide Richtungen
    for typ in ('bell', 'lowshelf', 'highshelf', 'notch', 'lowcut', 'highcut'):
        for f in (20, 50, 200, 1000, 5000, 10000):
            for q in (0.15, 0.707, 3, 24):
                for g in ((12, -12, 6) if typ in GAINTYP else (0,)):
                    faelle.append((typ, f, q, g, 2 * f, q, g)); faelle.append((typ, 2 * f, q, g, f, q, g))
    # b) Guetesprung Faktor 4
    for typ in ('bell', 'lowshelf', 'highshelf', 'notch', 'lowcut', 'highcut'):
        for f in (30, 1000, 8000):
            for qa, qb in ((0.15, 0.6), (0.707, 2.828), (6, 24)):
                for g in ((12, -12) if typ in GAINTYP else (0,)):
                    faelle.append((typ, f, qa, g, f, qb, g)); faelle.append((typ, f, qb, g, f, qa, g))
    # c) Gainsprung 20 dB
    for typ in GAINTYP:
        for f in (30, 1000, 8000):
            for q in (0.707, 3, 24):
                for ga, gb in ((-10, 10), (-12, 8), (-8, 12)):
                    faelle.append((typ, f, q, ga, f, q, gb)); faelle.append((typ, f, q, gb, f, q, ga))
    # d) alle drei Kriterien zugleich auf der Grenze
    for typ in ('bell', 'lowshelf', 'highshelf', 'notch', 'lowcut', 'highcut'):
        for f in (40, 1000, 5000):
            for (qa, qb) in ((6, 24), (24, 6), (0.707, 2.828), (2.828, 0.707)):
                for (fa, fb) in ((f, 2 * f), (2 * f, f)):
                    for (ga, gb) in (((-10, 10), (10, -10)) if typ in GAINTYP else ((0, 0),)):
                        faelle.append((typ, fa, qa, ga, fb, qb, gb))
    # Kriterien pruefen (Produktregel: strikt groesser reisst)
    ok = lambda a, b, gr: max(a, b) <= gr * min(a, b)
    faelle = [c for c in faelle if ok(c[1], c[4], 2.0) and ok(c[2], c[5], 4.0) and abs(c[3] - c[6]) <= 20 and max(c[1], c[4]) <= 20000]
    print(f"{len(faelle)} Faelle innerhalb der Kriterien", flush=True)
    rows = []
    for idx, c in enumerate(faelle):
        P, S, Xf, QSm, QSp, absP, absS = fall(*c)
        kP = int(np.argmax(P)); d = P - S; kD = int(np.argmax(d))
        rows.append(dict(fall=c, P=float(P[kP]), tonP=float(TOENE[kP]), S=float(np.max(S)), X=float(np.max(Xf)),
                         QSm=float(np.max(QSm)), QSp=float(np.max(QSp)), PmS=float(d[kD]), tonD=float(TOENE[kD]),
                         absP_D=float(absP[kD]), absS_D=float(absS[kD]), absPmax=float(np.max(absP)), absSmax=float(np.max(absS)),
                         S_at_kP=float(S[kP]), QSp_at_kP=float(QSp[kP]), QSm_at_kP=float(QSm[kP])))
        if idx % 50 == 0: print(f"  {idx}/{len(faelle)}", flush=True)
    json.dump(rows, open('v5_rampe.json', 'w'))
    print("fertig")
