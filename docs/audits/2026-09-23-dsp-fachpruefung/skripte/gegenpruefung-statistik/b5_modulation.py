# B5 Zusatz: echte musikalische Pegelbewegung (gleichverteilt ueber X dB) plus Schaetzstreuung;
# M1-Spanne P95-P50 (1-dB-Histogramm, Rang ceil(pN)) gegen die wahre Spanne der Pegelbewegung.
import numpy as np
rng = np.random.default_rng(77)
def m1(v_db):
    d = np.sort(np.round(v_db + rng.uniform(0, 1))); N = len(d)
    return d[int(np.ceil(.95 * N)) - 1] - d[int(np.ceil(.5 * N)) - 1]
N = 1400
print("X = Spannweite der wahren Pegelbewegung (gleichverteilt), wahre P95-P50 = 0.45*X")
for X in (0, 4, 8, 12):
    for name, T in (("Rauschen 1 Bin", None), ("Ton+Rauschen 1 Bin, Excess 10 dB", 9.0)):
        sp = []; wl = 0; wg = 0
        for _ in range(300):
            L = rng.uniform(0, X, N)                              # wahre Pegelbewegung in dB
            n = (rng.normal(0, 1, N) + 1j * rng.normal(0, 1, N)) / np.sqrt(2)
            p = np.abs(n) ** 2 if T is None else np.abs(np.sqrt(T) + n) ** 2
            s = m1(10 * np.log10(p) + L); sp.append(s); wl += s > 6; wg += s > 10
        print("  X=%2d dB  %-34s wahr %.1f dB  M1-Spanne Median %4.1f dB  'Wellen' %5.1f %%  'wellig'(>10) %5.1f %%"
              % (X, name, 0.45 * X, np.median(sp), 100 * wl / 300, 100 * wg / 300))
