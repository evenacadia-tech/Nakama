# Nur die Intervallachse (experiment.rs:386-392 mit Saat 42), hohe Laufzahl, iid-Fensterreihe (skalenfrei).
import numpy as np
import b1_kern as k
rng = np.random.default_rng(99)
R = 20000
print("Anteil der Laeufe, deren 95-%%-Intervall die 0 NICHT enthaelt (R=%d je Zeile)" % R)
for n in (2, 4, 5, 6, 8, 12, 16, 20, 24, 32, 40, 64, 100):
    W = k.gewichte(n, 42)
    zeile = []
    for nb_faktor, name in ((None, "K"), (1, "E n_b=n_r"), (2, "E n_b=2n_r")):
        x = rng.normal(0, 1, (R, n))
        if nb_faktor is not None:
            x = x - rng.normal(0, 1, (R, nb_faktor * n)).mean(axis=1, keepdims=True)
        bm = np.sort(x @ W.T / n, axis=1)
        lo, hi = bm[:, 10], bm[:, 390]
        zeile.append("%s %5.1f %%" % (name, 100 * np.mean((lo > 0) | (hi < 0))))
    print("  n_r=%3d: " % n + " | ".join(zeile))
