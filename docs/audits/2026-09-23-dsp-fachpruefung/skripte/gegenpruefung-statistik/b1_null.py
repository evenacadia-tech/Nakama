# B1: Nullrate der Experiment-Zielmetrik, exakt nach experiment.rs + experiment_verdrahtung.rs:1009-1034.
# Zeitreihe je Band = Resultatfenster minus MITTEL der Baselinefenster (experiment_verdrahtung.rs:1028).
# Modelle: "K" = Baselinemittel exakt bekannt (Modell des Vorpruefers), "E" = aus n_b Fenstern geschaetzt (Code).
import sys, numpy as np
import b1_kern as k

rng = np.random.default_rng(20260923)
BAENDER = k.MESSBAR
m = len(BAENDER)

def serie(R, n, rho, common):
    """R Laeufe, n Fenster, m Baender; AR(1) in der Zeit, optional gemeinsamer Faktor ueber Baender."""
    e = rng.normal(0, 1, (R, n, m))
    if common > 0:
        c = rng.normal(0, 1, (R, n, 1))
        e = np.sqrt(1 - common) * e + np.sqrt(common) * c
    if rho > 0:
        x = np.empty_like(e)
        x[:, 0] = e[:, 0]
        for i in range(1, n):
            x[:, i] = rho * x[:, i - 1] + np.sqrt(1 - rho * rho) * e[:, i]
        return x
    return e

def lauf(n_r, n_b, rho=0.0, common=0.0, R=2000, modell="E"):
    Ws = np.stack([k.gewichte(n_r, 42 + int(b)) for b in BAENDER])   # (m,400,n)
    W42 = k.gewichte(n_r, 42)
    miss = alarm = verb = versch = anysig = 0
    p05 = 0.0
    chunk = 250
    for s in range(0, R, chunk):
        r = min(chunk, R - s)
        if modell == "K":
            D = serie(r, n_r, rho, common)                      # wahres Mittel 0 abgezogen
        else:
            # Baseline und Resultat aus DEMSELBEN Prozess: eine Reihe, geteilt
            z = serie(r, n_b + n_r, rho, common)
            base, res = z[:, :n_b], z[:, n_b:]
            D = res - base.mean(axis=1, keepdims=True)
        fm = D.mean(axis=2)                                     # (r,n) Fenstermittel ueber Baender
        bm = np.sort(fm @ W42.T / n_r, axis=1)                  # (r,400)
        lo, hi = bm[:, 10], bm[:, 390]
        mittel = np.einsum("bzn,rnb->rbz", Ws, D) / n_r         # (r,m,400)
        p = k.p_aus_mitteln(mittel)                              # (r,m)
        for i in range(r):
            sig = k.fdr(p[i]).sum()
            ex = (lo[i] > 0) or (hi[i] < 0)
            miss += ex
            anysig += sig > 0
            p05 += (p[i] <= 0.05).mean()
            if sig > 0 and lo[i] > 0:
                verb += 1
            elif sig > 0 and hi[i] < 0:
                versch += 1
    return dict(miss=miss / R, anysig=anysig / R, p05=p05 / R, verb=verb / R, versch=versch / R)

if __name__ == "__main__":
    R = int(sys.argv[1]) if len(sys.argv) > 1 else 1000
    print("m = %d messbare Baender, %d Laeufe je Zeile" % (m, R))
    print("Modell K = Baselinemittel exakt bekannt (Nachbildung Vorpruefer), E = Baseline aus n_b = n_r Fenstern (Code)")
    print("%-28s %6s %8s %8s %8s %8s" % ("Fall", "n_r", "IV!=0", "p<=.05", ">=1 FDR", "Alarm"))
    for modell, rho, common in [("K", 0.0, 0.0), ("E", 0.0, 0.0), ("E", 0.5, 0.0), ("E", 0.0, 0.5)]:
        for n in [2, 3, 4, 5, 6, 8, 12, 16, 20, 24, 32, 40]:
            r = lauf(n, n, rho, common, R=R, modell=modell)
            print("%-28s %6d %7.1f%% %7.1f%% %7.1f%% %7.1f%%" % (
                "%s iid" % modell if rho == 0 and common == 0 else "%s AR1=%.1f gem=%.1f" % (modell, rho, common),
                n, 100 * r["miss"], 100 * r["p05"], 100 * r["anysig"], 100 * (r["verb"] + r["versch"])))
        print()
