# Phasenabtastung ausgewaehlter Faelle: Umschaltphase ueber eine Tonperiode (bzw. 64 Phasen),
# Spitze Produkt P und SVF-Param S am selben Ton, Verfahrensanteil E = P - max(S, 0), jeweils Maximum ueber Phasen.
import math
import numpy as np
import v5_rampe as v

faelle = [
    (('highshelf', 80, 24, 10, 40, 6, -10), 68.0),
    (('bell', 80, 0.707, -10, 40, 2.828, 10), 20.0),
    (('bell', 40, 3, 12, 20, 3, 12), 38.2),
    (('bell', 30, 3, -8, 30, 3, 12), 51.0),
    (('highshelf', 30, 3, 12, 30, 3, -8), 26.7),
    (('lowshelf', 8000, 24, 12, 8000, 24, -8), 7303.5),
    (('bell', 1000, 24, -10, 1000, 24, 10), None),     # Fall des Vorpruefers (b): Ton mit groesstem E suchen
    (('bell', 1000, 1, 12, 500, 1, 12), None),
    (('highcut', 10000, 24, 0, 5000, 24, 0), None),
]
for c, ton in faelle:
    if ton is None:
        P, S, *_ = v.fall(*c)
        E = P - np.maximum(S, 0); ton = float(v.TOENE[int(np.argmax(E))])
    v.TOENE = np.array([ton]); v.W = 2 * np.pi * v.TOENE / v.fs
    per = max(8, min(64, int(round(v.fs / ton))))
    schritt = max(1, int(round(v.fs / ton / per)))
    Ps, Ss, Xs, As = [], [], [], []
    for k in range(per):
        P, S, X, QSm, QSp, absP, absS = v.fall(*c, n0=200000 + k * schritt)
        Ps.append(P[0]); Ss.append(S[0]); Xs.append(X[0]); As.append(absP[0])
    Ps, Ss = np.array(Ps), np.array(Ss); E = Ps - np.maximum(Ss, 0)
    print(f"{str(c):48s} Ton {ton:7.1f} Hz, {per} Phasen: P max {Ps.max():+6.2f} | S max {Ss.max():+6.2f} | X max {max(Xs):+6.2f} | "
          f"E max {E.max():+6.2f} (Median {np.median(E):+5.2f}) | abs P max {max(As):+6.1f} dB re Eingang")
    v.TOENE = np.unique(np.geomspace(20.0, 20000.0, 97)); v.W = 2 * np.pi * v.TOENE / v.fs
