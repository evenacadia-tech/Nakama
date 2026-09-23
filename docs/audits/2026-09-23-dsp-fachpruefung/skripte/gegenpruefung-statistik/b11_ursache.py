# B11 Ursachenprobe: Ist die Ausloesung pegelunabhaengig (log-Differenz ohne Kompression/Boden)?
import numpy as np
import b11_fluss as f
fs = f.fs
def lauf(x, name):
    fr = f.logbaender(x)
    c = f.detektor(fr)
    print("  %-58s Code %3d / %d Frames" % (name, c[0], c[1]))
n = int(30 * fs); t = np.arange(n) / fs
for sig in (0.1, 0.01, 0.003):
    lauf(np.random.default_rng(5).normal(0, sig, n), "Weissrauschen sigma %.3f (%.0f dBFS)" % (sig, 20*np.log10(sig)))
ton = lambda boden, seed=3: 0.3*np.sin(2*np.pi*440*t) + (np.random.default_rng(seed).normal(0, boden, n) if boden > 0 else 0)
for boden in (1e-3, 1e-4, 1e-6, 0.0):
    lauf(ton(boden), "Dauerton 440 Hz ohne Vibrato, Rauschboden %g" % boden)
# Anteil der Flussvarianz aus Baendern, die nur Rauschboden tragen (Ton 440 Hz, Boden 1e-4)
fr = f.logbaender(ton(1e-4))
d = np.array([np.clip(b - a, 0, None) for a, b in zip(fr[:-1], fr[1:])])
bandanteil = d.var(axis=0)
nahe = np.zeros(221, bool)
for h in range(1, 20):
    fh = 440*h
    if fh < 18000:
        b = int(np.argmin(np.abs(f.MIT - fh))); nahe[max(b-2,0):b+3] = True
print("  Ton+Boden 1e-4: Summe der Flussmittel aus Rauschboden-Baendern %.1f von %.1f (Anteil %.0f %%)"
      % (d[:, ~nahe].sum(axis=1).mean(), d.sum(axis=1).mean(), 100*d[:, ~nahe].sum(axis=1).mean()/d.sum(axis=1).mean()))
