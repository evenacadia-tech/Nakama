# B1 Zusatz: dieselbe Passage wird ohne Aenderung erneut gespielt (echte Nullhypothese eines Vorher/Nachher-Versuchs).
# Musik = feste Folge je Fensterposition (gemeinsamer Pegelverlauf + Bandprofil), Schaetzrauschen je Fenster neu.
# Baseline = 2 volle Durchlaeufe; Resultat = 1, 1.5 oder 2 Durchlaeufe (Teilabdeckung -> Gewichtung verschoben).
import numpy as np
import b1_kern as k
rng = np.random.default_rng(4242)
B = k.MESSBAR; m = len(B); P = 25          # 25 Fenster je Durchlauf (~8 s bei 0,32 s)
def musik():
    g = np.cumsum(rng.normal(0, 1.0, P)); g -= g.mean()             # langsamer gemeinsamer Pegelverlauf (dB)
    prof = rng.normal(0, 3.0, (P, m)) * 0.5 + g[:, None] * 1.0      # Bandverlauf
    return prof
def lauf(res_durchlaeufe, R=400):
    alarm = miss = 0
    for _ in range(R):
        mu = musik()
        base = np.concatenate([mu + rng.normal(0, 1.0, (P, m)) for _ in range(2)])
        nres = int(round(res_durchlaeufe * P))
        idx = np.arange(nres) % P
        res = mu[idx] + rng.normal(0, 1.0, (nres, m))
        D = res - base.mean(axis=0, keepdims=True)
        u, o, sig, bef, _ = k.urteil(D)
        miss += (u > 0) or (o < 0); alarm += bef != "Unveraendert"
    return miss / R, alarm / R
for r in (1.0, 1.5, 2.0, 0.6):
    mi, al = lauf(r)
    print("Resultat %.1f Durchlaeufe (%d Fenster), Baseline 2 Durchlaeufe: Intervall ohne 0 %.1f %%, Zielmetrik != Unveraendert %.1f %%" % (r, int(round(r*P)), 100*mi, 100*al))
