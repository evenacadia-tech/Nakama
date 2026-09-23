# M1-Bandbins (AnalyseEngine.cpp:176-196, 91-99, 426-435): 1/24-Okt ab 30 Hz, zustaendige Stufe je Band.
import numpy as np
fs = 48000.0
lo = 30*2**(np.arange(221)/24); hi = 30*2**((np.arange(221)+1)/24); z = np.sqrt(lo*hi)
i200 = int(np.argmax(z >= 200)); i2000 = int(np.argmax(z >= 2000))
def nb(b, N):
    v = int(np.ceil(lo[b]*N/fs)); w = min(int(np.ceil(hi[b]*N/fs)), N//2+1); return max(0, w - v)
cnt = []
for b in range(221):
    N = 16384 if b < i200 else (4096 if b < i2000 else 2048)
    cnt.append((z[b], nb(b, N)))
for a, e in ((30, 100), (100, 200), (200, 400), (400, 800), (800, 2000), (2000, 4000), (4000, 18000)):
    sel = [c for f, c in cnt if a <= f < e]
    print("%5d-%5d Hz: Baender %3d, Bins je Band %s" % (a, e, len(sel), dict(zip(*np.unique(sel, return_counts=True)))))
