# B11 -> Experiment-Guardrail "Transient" (experiment_verdrahtung.rs:1120-1127, Schwelle 2,0 experiment.rs:575):
# Onset je Evidenzfenster = Summe der Staerken (fluss-med)/mad (Spektrum.h:697-699, evidenz.rs:305-314).
# Stationaeres Material ohne jede Aenderung: wie oft reisst |mittel(Resultat) - mittel(Baseline)| > 2,0?
import numpy as np
import b11_fluss as f
def staerken(frames):
    hist = []; vor = None; out = []
    for i, lb in enumerate(frames):
        if lb is None: continue
        if vor is None: vor = lb; continue
        fl = np.sum(np.clip(lb - vor, 0, None)); s = 0.0
        if len(hist) >= 16:
            h = np.array(hist[-16:]); q = np.sort(h); med = 0.5 * (q[7] + q[8]); mad = np.mean(np.abs(h - med))
            if fl > med + 3 * mad and mad > 0: s = (fl - med) / mad
        out.append(s); hist.append(fl); vor = lb
    return np.array(out)
fs = f.fs; n = int(300 * fs); t = np.arange(n) / fs
rng = np.random.default_rng(21)
for name, x in (("Weissrauschen", rng.normal(0, 0.1, n)),
                ("Synth 110 Hz, 20 Obertoene, float32", sum((0.3/h)*np.sin(2*np.pi*110*h*t) for h in range(1, 21)).astype(np.float32).astype(np.float64))):
    s = staerken(f.logbaender(x))
    # Evidenzfenster ~0,32 s = 7,5 Hops -> abwechselnd 7 und 8 Frames
    grenzen = np.cumsum([7 if i % 2 == 0 else 8 for i in range(len(s))]); grenzen = grenzen[grenzen < len(s)]
    onset = np.array([seg.sum() for seg in np.split(s, grenzen)])
    teile = []
    for k in (4, 8, 16, 25):
        R = 4000; riss = 0
        for _ in range(R):
            a = rng.choice(onset, k); b = rng.choice(onset, k)
            riss += abs(b.mean() - a.mean()) > 2.0
        teile.append("%2d+%2d Fenster: %4.1f %%" % (k, k, 100 * riss / R))
    print("%-36s Onset/Fenster Mittel %.2f SD %.2f | Guardrail 'Transient' gerissen: " % (name, onset.mean(), onset.std()) + " | ".join(teile))
