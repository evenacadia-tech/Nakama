# B10: Bandkohaerenz nach Stereo.h:52-91 und :225-245 — Kreuz- und Autospektren ueber BINS und Frames
# eines Bandes summiert, DANN |Sxy|^2/(Sxx*Syy). Gegenrechnung: echte MSC je Bin (Mittelung nur ueber
# Frames), dann ueber das Band gemittelt; dazu die analytische Erwartung |sum_k e^{-i 2 pi k df tau}|^2/K^2.
import json, struct, numpy as np
rng = np.random.default_rng(1001)
FIX = r"C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json"
fein = json.load(open(FIX, encoding="utf-8"))
h2d = lambda s: struct.unpack(">d", bytes.fromhex(s))[0]
K = np.array([h2d(s) for s in fein["kanten_hz"]["hex64"]])
MIT = np.array([h2d(s) for s in fein["mitten_hz"]["hex64"]])
fs = 48000.0; N = 4096; hop = 2048
w = 0.5 - 0.5 * np.cos(2 * np.pi * np.arange(N) / N)          # periodisches Hann
def bins(b):
    v = int(np.ceil(K[b] * N / fs)); bi = min(int(np.ceil(K[b + 1] * N / fs)), N // 2 + 1)
    return v, max(v, bi)

def messe(delay, baender, frames=8, laeufe=60):
    code = {b: [] for b in baender}; msc = {b: [] for b in baender}
    n = N + (frames - 1) * hop
    for _ in range(laeufe):
        x = rng.normal(0, 1, n + delay + 10)
        l = x[delay + 10: delay + 10 + n]; r = x[10: 10 + n]           # R = L um `delay` Samples verzoegert
        acc = {b: np.zeros(6) for b in baender}
        perbin = {b: [0, 0, 0] for b in baender}
        for f in range(frames):
            s = f * hop
            m = 0.5 * (l[s:s + N] + r[s:s + N]); sd = 0.5 * (l[s:s + N] - r[s:s + N])
            M = np.fft.rfft(m * w); S = np.fft.rfft(sd * w)
            L = M + S; R = M - S                                          # Stereo.h:71-72
            for b in baender:
                v, bi = bins(b)
                cr = L[v:bi] * np.conj(R[v:bi])
                acc[b] += [np.sum(np.abs(L[v:bi]) ** 2), np.sum(np.abs(R[v:bi]) ** 2), cr.real.sum(), cr.imag.sum(), 0, 0]
                perbin[b][0] = perbin[b][0] + cr; perbin[b][1] = perbin[b][1] + np.abs(L[v:bi]) ** 2
                perbin[b][2] = perbin[b][2] + np.abs(R[v:bi]) ** 2
        for b in baender:
            sll, srr, re, im = acc[b][:4]
            code[b].append((re * re + im * im) / (sll * srr))                # Stereo.h:229-230
            msc[b].append(np.mean(np.abs(perbin[b][0]) ** 2 / (perbin[b][1] * perbin[b][2])))
    return {b: (np.mean(code[b]), np.mean(msc[b])) for b in baender}

ziel = [1000, 5000, 8000, 10000, 12500, 15800]
baender = [int(np.argmin(np.abs(MIT - f))) for f in ziel]
print("Baender (Index, Mitte Hz, Bins):", [(b, round(MIT[b]), bins(b)[1] - bins(b)[0]) for b in baender])
for d_ms in (0.25, 0.5, 1.0, 2.0):
    d = int(round(d_ms * fs / 1000))
    r = messe(d, baender)
    teile = []
    for b in baender:
        v, bi = bins(b); kk = np.arange(v, bi)
        ana = np.abs(np.sum(np.exp(-2j * np.pi * kk * (fs / N) * d / fs))) ** 2 / len(kk) ** 2
        teile.append("%5d Hz: Code %.3f | MSC je Bin %.3f | analyt. %.3f" % (round(MIT[b]), r[b][0], r[b][1], ana))
    print("Laufzeit %.2f ms (%d Samples), 8 Frames:\n   " % (d_ms, d) + "\n   ".join(teile))
# Schwelle 0,8: ab welcher Frequenz faellt eine 1-ms-Laufzeit analytisch darunter?
d = 48
for b in range(65, 221):
    v, bi = bins(b)
    if bi <= v: continue
    kk = np.arange(v, bi)
    ana = np.abs(np.sum(np.exp(-2j * np.pi * kk * (fs / N) * d / fs))) ** 2 / len(kk) ** 2
    if ana < 0.8:
        print("1 ms: analytische Bandkohaerenz faellt erstmals unter 0,8 bei Band %d (%.0f Hz, %d Bins): %.3f" % (b, MIT[b], len(kk), ana))
        break
# Unkorrelierte Kanaele (Bias bei 8 Frames), Vergleich
