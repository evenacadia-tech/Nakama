# B11: Flussdetektor nach Spektrum.h:412-429 (PSD), :580-714 (Fluss, Schwelle, Ausloesung), Zeit.h:120-145
# (Bandzuordnung Hauptstufe), FeatureEngine.h:376-443 (4096/2048, Trennung 200 Hz, Historie 16).
# Peakpfad (:672-678) loest bei den Testsignalen nicht aus (Rahmenpeak steigt < 12 dB) und wird weggelassen.
# Varianten zum Vergleich: SuperFlux-Maximumfilter (+-1 Band auf dem Referenzframe, Boeck/Widmer 2013 Gl. 5/6)
# und eine minimale Spitzenwahl (lokales Maximum gegen den Vorframe, kein Ereignis im direkt folgenden Frame).
import json, struct, sys, numpy as np
FIX = r"C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json"
fein = json.load(open(FIX, encoding="utf-8"))
h2d = lambda s: struct.unpack(">d", bytes.fromhex(s))[0]
K = np.array([h2d(s) for s in fein["kanten_hz"]["hex64"]])
MIT = np.array([h2d(s) for s in fein["mitten_hz"]["hex64"]])
fs = 48000.0; N = 4096; hop = 2048
w = 0.5 - 0.5 * np.cos(2 * np.pi * np.arange(N) / N)
norm = 2.0 / (fs * np.sum(w * w))
kappe = min(18000.0, 0.95 * fs * 0.5)
trenn = int(np.argmax(MIT >= 200.0))
V = np.zeros(221, int); Bi = np.zeros(221, int)
for b in range(trenn, 221):
    if K[b + 1] > kappe: continue
    v = int(np.ceil(K[b] * N / fs)); bi = min(int(np.ceil(K[b + 1] * N / fs)), N // 2 + 1)
    V[b] = v; Bi[b] = max(v, bi)
gueltig = Bi > V

def logbaender(x):
    frames = []
    for s in range(0, len(x) - N + 1, hop):
        seg = x[s:s + N]
        if 10 * np.log10(np.sum(np.abs(np.fft.rfft(seg * w)) ** 2) * norm * fs / N + 1e-300) <= -60.0:
            frames.append(None); continue                      # inaktiv: kein Fluss, kein Vorgaenger-Update
        psd = np.abs(np.fft.rfft(seg * w)) ** 2 * norm         # Mono: |M|^2, S = 0
        c = np.concatenate(([0.0], np.cumsum(psd)))
        e = np.zeros(221)
        e[gueltig] = (c[Bi[gueltig]] - c[V[gueltig]]) / (Bi[gueltig] - V[gueltig])
        frames.append(np.where(e > 0, np.log10(np.where(e > 0, e, 1.0)), -30.0))
    return frames

def detektor(frames, maxfilter=False, spitzenwahl=False):
    hist = []; vor = None; ev = []; fl_alt = None; n_flux = 0
    for i, lb in enumerate(frames):
        if lb is None: continue
        if vor is None:
            vor = lb; continue
        ref = vor
        if maxfilter:
            ref = vor.copy()
            ref[1:-1] = np.maximum(np.maximum(vor[:-2], vor[1:-1]), vor[2:])
        fluss = np.sum(np.clip(lb - ref, 0, None))
        n_flux += 1
        if len(hist) >= 16:
            h = np.array(hist[-16:])
            s = np.sort(h); med = 0.5 * (s[7] + s[8])
            mad = np.mean(np.abs(h - med))
            aus = fluss > med + 3.0 * mad and mad > 0.0
            if aus and spitzenwahl:
                aus = (fl_alt is None or fluss >= fl_alt) and (not ev or ev[-1] != i - 1)
            if aus: ev.append(i)
        hist.append(fluss); fl_alt = fluss; vor = lb
    folge = sum(1 for a, b in zip(ev, ev[1:]) if b == a + 1)
    return len(ev), n_flux, folge

def ton(sek, cent, rng, f0=440.0, rate=5.5, harm=7):
    t = np.arange(int(sek * fs)) / fs
    inst = f0 * 2 ** ((cent / 1200.0) * np.sin(2 * np.pi * rate * t))
    ph = 2 * np.pi * np.cumsum(inst) / fs
    return sum((0.3 / h) * np.sin(h * ph) for h in range(1, harm + 1)) + rng.normal(0, 1e-4, len(t))

if __name__ == "__main__":
    print("Hauptstufenbaender mit Bins: %d (ab Band %d)" % (gueltig.sum(), trenn))
    z = []
    for seed in range(8):
        rng = np.random.default_rng(100 + seed)
        fr = logbaender(rng.normal(0, 0.1, int(30 * fs)))
        z.append((detektor(fr), detektor(fr, maxfilter=True), detektor(fr, spitzenwahl=True)))
    a = np.array([[x[0][0], x[1][0], x[2][0], x[0][2]] for x in z])
    print("Weissrauschen 30 s (%d Flussframes), 8 Saaten: Code %.1f (min %d, max %d) | +Maxfilter %.1f | +Spitzenwahl %.1f | davon direkt folgend (Code) %.1f"
          % (z[0][0][1], a[:, 0].mean(), a[:, 0].min(), a[:, 0].max(), a[:, 1].mean(), a[:, 2].mean(), a[:, 3].mean()))
    rng = np.random.default_rng(9)
    for cent in (0, 15, 30, 50, 100):
        fr = logbaender(ton(30.0, cent, rng))
        c = detektor(fr); m = detektor(fr, maxfilter=True); s = detektor(fr, spitzenwahl=True)
        print("Ton 440 Hz + 6 Obertoene, Vibrato +-%3d Cent, 5,5 Hz, 30 s: Code %3d (Folge %2d) | +Maxfilter %3d | +Spitzenwahl %3d" % (cent, c[0], c[2], m[0], s[0]))
    rng = np.random.default_rng(10)
    for cent in (0, 50, 100):
        fr = logbaender(ton(30.0, cent, rng, f0=220.0, rate=5.0, harm=12))
        c = detektor(fr); m = detektor(fr, maxfilter=True)
        print("Ton 220 Hz + 11 Obertoene, Vibrato +-%3d Cent, 5 Hz, 30 s:   Code %3d | +Maxfilter %3d" % (cent, c[0], m[0]))
    # Echte Einsaetze: Klick jede Sekunde auf stehendem Akkord
    rng = np.random.default_rng(11)
    n = int(30 * fs); t = np.arange(n) / fs
    akk = sum(0.1 * np.sin(2 * np.pi * f * t) for f in (261.6, 329.6, 392.0, 523.3)) + rng.normal(0, 1e-3, n)
    kl = akk.copy()
    for s0 in range(int(0.5 * fs), n, int(fs)): kl[s0:s0 + 48] += 0.8 * np.hanning(48)
    for name, x in (("stehender Akkord", akk), ("Akkord + 30 Klicks", kl)):
        fr = logbaender(x); c = detektor(fr); s = detektor(fr, spitzenwahl=True)
        print("%-20s 30 s: Code %3d (direkt folgend %2d) | +Spitzenwahl %3d" % (name, c[0], c[2], s[0]))
