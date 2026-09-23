# B1 Zusatz 2: Wiederholung derselben Passage, Musikbewegung stark / schwach, volle vs. Teilabdeckung.
import numpy as np
import b1_kern as k
rng = np.random.default_rng(777)
B = k.MESSBAR; m = len(B); P = 25
def lauf(res_durchlaeufe, g_sd, band_sd, R=300):
    alarm = 0
    for _ in range(R):
        g = np.cumsum(rng.normal(0, g_sd, P)); g -= g.mean()
        mu = rng.normal(0, band_sd, (P, m)) + g[:, None]
        base = np.concatenate([mu + rng.normal(0, 1.0, (P, m)) for _ in range(2)])
        nres = max(1, int(round(res_durchlaeufe * P)))
        res = mu[np.arange(nres) % P] + rng.normal(0, 1.0, (nres, m))
        _, _, _, bef, _ = k.urteil(res - base.mean(axis=0, keepdims=True))
        alarm += bef != "Unveraendert"
    return alarm / R
print("Zielmetrik != Unveraendert unter H0 (keine Aenderung), Baseline 2 volle Durchlaeufe a 25 Fenster, Schaetzrauschen 1 dB")
for name, g_sd, band_sd in (("gleichfoermiges Material", 0.1, 0.2), ("bewegtes Material", 1.0, 1.5)):
    teile = []
    for r in (0.16, 0.4, 0.6, 1.0, 2.0):
        teile.append("%2d Fenster: %5.1f %%" % (max(1, int(round(r*P))), 100 * lauf(r, g_sd, band_sd)))
    print("  %-26s " % name + " | ".join(teile))
