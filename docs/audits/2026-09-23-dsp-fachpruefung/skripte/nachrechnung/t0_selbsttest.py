"""Selbsttest der Werkzeuge: numba-Kerne bitgleich zu reinem Python (kein FMA),
float32-Kern wirklich float32, dd-Referenz plausibel, Zustandsraum-H des SVF
gleich der simulierten Impulsantwort."""
import numpy as np
import nk

rng = np.random.default_rng(1)
x = rng.standard_normal(20000)
c = nk.rbj_bell(48000.0, 1000.0, 2.0, 9.0)

# 1) float64: numba vs reines Python
def py_df2t(c, x):
    b0, b1, b2, a1, a2 = c
    z1 = z2 = 0.0
    out = []
    for xi in x:
        xi = float(xi)
        yi = b0 * xi + z1
        z1 = b1 * xi - a1 * yi + z2
        z2 = b2 * xi - a2 * yi
        out.append(yi)
    return np.array(out)

y_nb = nk.df2t_run(*c, x)
y_py = py_df2t(c, x)
print("float64 numba==python bitgleich:", np.array_equal(y_nb, y_py))

# 2) float32: numba vs numpy.float32-Skalare
c32 = [np.float32(v) for v in c]
x32 = x[:3000].astype(np.float32)
y32_nb = nk.df2t_run(*c32, x32)
b0, b1, b2, a1, a2 = c32
z1 = np.float32(0); z2 = np.float32(0); out = []
for xi in x32:
    yi = b0 * xi + z1
    z1 = b1 * xi - a1 * yi + z2
    z2 = b2 * xi - a2 * yi
    out.append(yi)
y32_py = np.array(out, dtype=np.float32)
print("float32 dtype:", y32_nb.dtype, " numba==numpy.float32 bitgleich:", np.array_equal(y32_nb, y32_py))

# 3) dd-Referenz: Differenz double - dd klein
yh, yl = nk.df2t_run_dd(*c, x)
err = (y_nb - yh) - yl
print("double vs dd: max|err| = %.3e  (rms y = %.3f)" % (np.max(np.abs(err)), np.sqrt(np.mean(y_nb**2))))

# 4) SVF-Zustandsraum vs Simulation
fs = 48000.0
for typ in ("bell", "lowshelf", "highshelf"):
    cs = nk.SVF[typ](nk.svf_grundG(3000.0, fs), 1.3, 7.0)
    imp = np.zeros(1 << 16); imp[0] = 1.0
    h = nk.svf_run(*cs[:6], imp)
    Hf = np.fft.rfft(h)
    w = 2 * np.pi * np.arange(Hf.size) / imp.size
    Hs = nk.svf_H(cs, w)
    print(typ, "SVF Zustandsraum vs FFT(Impuls): max|dB| = %.2e" % np.max(np.abs(nk.db(Hf) - nk.db(Hs))))

# 5) RBJ biquad_H vs Simulation
imp = np.zeros(1 << 16); imp[0] = 1.0
h = nk.df2t_run(*c, imp)
Hf = np.fft.rfft(h); w = 2 * np.pi * np.arange(Hf.size) / imp.size
print("RBJ biquad_H vs FFT(Impuls): max|dB| = %.2e" % np.max(np.abs(nk.db(Hf) - nk.db(nk.biquad_H(c, w)))))
