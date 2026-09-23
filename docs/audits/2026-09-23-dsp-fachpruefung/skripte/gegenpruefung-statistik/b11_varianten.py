# B11 Gegenprobe: welche Aenderung beseitigt die Fehlausloesungen auf stationaerem Material?
# (a) Code; (b) +Maxfilter; (c) +Spitzenwahl; (d) log10(1+lambda*E)-Kompression wie SuperFlux Gl. 4 (lambda: -70 dBFS-Dichte ~ 1);
# (e) absolute Mindesthoehe delta ueber der adaptiven Schwelle (Bedingung 2 der SuperFlux-Spitzenwahl, "+delta").
import numpy as np
import b11_fluss as f
fs = f.fs
def energien(x):
    out = []
    for s in range(0, len(x) - f.N + 1, f.hop):
        psd = np.abs(np.fft.rfft(x[s:s + f.N] * f.w)) ** 2 * f.norm
        c = np.concatenate(([0.0], np.cumsum(psd)))
        e = np.zeros(221); g = f.gueltig
        e[g] = (c[f.Bi[g]] - c[f.V[g]]) / (f.Bi[g] - f.V[g])
        out.append(e)
    return out
def zaehle(E, modus):
    lam = 1.0 / (10 ** (-70 / 10) * 2.0 / fs)        # Dichte eines -70-dBFS-Weissrauschens -> 1
    hist = []; vor = None; n = 0; last = -5
    for i, e in enumerate(E):
        lb = np.log10(1 + lam * e) if modus == "kompr" else np.where(e > 0, np.log10(np.where(e > 0, e, 1)), -30.0)
        if vor is None: vor = lb; continue
        fl = np.sum(np.clip(lb - vor, 0, None))
        if len(hist) >= 16:
            h = np.array(hist[-16:]); s = np.sort(h); med = 0.5 * (s[7] + s[8]); mad = np.mean(np.abs(h - med))
            thr = med + 3 * mad
            if modus == "delta": thr = max(thr, med + 5.0)          # absolute Mindesthoehe 5 Dekaden-Summen ueber Median
            if fl > thr and mad > 0: n += 1
        hist.append(fl); vor = lb
    return n
n = int(30 * fs); t = np.arange(n) / fs
faelle = [("Weissrauschen -20 dBFS", np.random.default_rng(5).normal(0, 0.1, n)),
          ("Weissrauschen -50 dBFS", np.random.default_rng(5).normal(0, 0.003, n)),
          ("Dauerton 440 Hz, Boden 1e-4", 0.3 * np.sin(2 * np.pi * 440 * t) + np.random.default_rng(3).normal(0, 1e-4, n)),
          ("Ton +-30 Cent Vibrato", f.ton(30.0, 30, np.random.default_rng(12)))]
kl = sum(0.1 * np.sin(2 * np.pi * fr * t) for fr in (261.6, 329.6, 392.0, 523.3)) + np.random.default_rng(11).normal(0, 1e-3, n)
for s0 in range(int(0.5 * fs), n, int(fs)): kl[s0:s0 + 48] += 0.8 * np.hanning(48)
faelle.append(("Akkord + 30 Klicks (Soll 30)", kl))
for name, x in faelle:
    E = energien(x)
    print("%-30s Code %3d | log(1+x) %3d | +delta %3d" % (name, zaehle(E, "code"), zaehle(E, "kompr"), zaehle(E, "delta")))
