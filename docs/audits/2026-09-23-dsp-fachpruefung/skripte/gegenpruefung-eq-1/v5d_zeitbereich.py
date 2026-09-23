# Gegenprobe ohne analytischen Anfangszustand: reiner Zeitbereich, Vorlauf 2 s mit Entwurf A aus Nullzustand,
# dann Rampe ueber 256 Samples (Produkt: DF2T + lineare Koeffizientenmischung; Referenz: TPT-SVF mit Parameterinterpolation),
# dann 0,5 s mit B. Spitze im Fenster ab Umschaltsample gegen max(|H_A|,|H_B|).
import math
import numpy as np
import v5_rampe as v

def lauf(c, ton, vorlauf_s=2.0):
    typ, fA, qA, gA, fB, qB, gB = c
    fs = v.fs; R = v.R; N0 = int(vorlauf_s * fs); N = N0 + R + v.NWIN
    n = np.arange(N); x = np.sin(2 * math.pi * ton / fs * n)
    kA = v.rbj(typ, fA, qA, gA); kB = v.rbj(typ, fB, qB, gB)
    w = np.array([2 * math.pi * ton / fs])
    ref = max(abs(v.Hrbj(kA, w)[0]), abs(v.Hrbj(kB, w)[0]))
    # Produkt
    z1 = z2 = 0.0; yP = np.empty(N)
    for i in range(N):
        if i < N0: k = kA
        elif i < N0 + R: k = kA + (kB - kA) * ((i - N0 + 1) / R)
        else: k = kB
        y = k[0] * x[i] + z1; z1 = k[1] * x[i] - k[3] * y + z2; z2 = k[2] * x[i] - k[4] * y; yP[i] = y
    # SVF mit Parameterinterpolation
    ic1 = ic2 = 0.0; yS = np.empty(N)
    cA = v.svf(typ, fA, qA, gA); cB = v.svf(typ, fB, qB, gB)
    for i in range(N):
        if i < N0: c_ = cA
        elif i < N0 + R:
            t = (i - N0 + 1) / R
            c_ = v.svf(typ, math.exp(math.log(fA) + (math.log(fB) - math.log(fA)) * t),
                       math.exp(math.log(qA) + (math.log(qB) - math.log(qA)) * t), gA + (gB - gA) * t)
        else: c_ = cB
        a1, a2, a3, m0, m1, m2 = c_
        v3 = x[i] - ic2; v1 = a1 * ic1 + a2 * v3; v2 = ic2 + a2 * ic1 + a3 * v3
        ic1 = 2 * v1 - ic1; ic2 = 2 * v2 - ic2; yS[i] = m0 * x[i] + m1 * v1 + m2 * v2
    fen = slice(N0, N)
    # Kontrolle: Vorlauf eingeschwungen? letzte Periode vor N0 gegen |H_A|
    per = int(round(fs / ton)); vor = np.max(np.abs(yP[N0 - per:N0])) / abs(v.Hrbj(kA, w)[0])
    return 20 * math.log10(np.max(np.abs(yP[fen])) / ref), 20 * math.log10(np.max(np.abs(yS[fen])) / ref), 20 * math.log10(vor)

for c, ton in ((('bell', 40, 3, 12, 20, 3, 12), 38.2), (('highshelf', 30, 3, 12, 30, 3, -8), 26.7),
               (('bell', 80, 0.707, -10, 40, 2.828, 10), 20.0), (('lowshelf', 5000, 0.707, 6, 50, 0.707, 6), 1000.0)):
    p, s, vor = lauf(c, ton)
    print(f"{str(c):46s} Ton {ton:7.1f} Hz: Produkt {p:+6.2f} dB | SVF-Param {s:+6.2f} dB | Vorlauf eingeschwungen: {vor:+.3f} dB")
