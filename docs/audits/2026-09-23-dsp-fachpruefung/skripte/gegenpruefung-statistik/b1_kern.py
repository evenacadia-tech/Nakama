# B1: exakte Nachbildung von experiment.rs:1481-1607 (Wuerfel, block_bootstrap,
# bootstrap_p) und :1675-1697 (fdr_signifikant) sowie der Achse Zielmetrik
# (:481-495). Die Ziehungen haengen NUR von Saat, n und Blocklaenge ab, nicht von
# den Daten -> als Zaehlmatrix W (400 x n) vorab, dann Mittel = W @ x / n.
import json, struct, numpy as np

M = 6364136223846793005
INC = 1442695040888963407
MASK = (1 << 64) - 1
B = 400          # BOOTSTRAP_ZIEHUNGEN, experiment.rs:585
BL = 4           # BOOTSTRAP_BLOCK, experiment.rs:583

_cache = {}
def gewichte(n, saat, bl=BL, ziehungen=B):
    key = (n, saat, bl, ziehungen)
    if key in _cache:
        return _cache[key]
    bl = min(bl, n); bloecke = n - bl + 1
    z = (saat * M + 1) & MASK                     # Wuerfel::neu
    W = np.zeros((ziehungen, n))
    for r in range(ziehungen):
        g = 0
        while g < n:
            z = (z * M + INC) & MASK              # naechste
            start = (z >> 33) % bloecke
            for k in range(bl):
                if g >= n:
                    break
                W[r, start + k] += 1.0
                g += 1
    _cache[key] = W
    return W

def block_bootstrap(x, saat=42, alpha=0.05):
    n = len(x)
    W = gewichte(n, saat)
    mittel = np.sort(W @ x / n)
    u = int(np.floor((alpha / 2.0) * B))
    o = min(int(np.ceil((1.0 - alpha / 2.0) * B)), B - 1)
    return mittel[min(u, B - 1)], mittel[o]

def p_aus_mitteln(mittel):
    nicht_pos = np.sum(~(mittel > 0.0), axis=-1)
    nicht_neg = np.sum(~(mittel < 0.0), axis=-1)
    kleiner = np.minimum(nicht_pos, nicht_neg)
    return np.minimum(2.0 * (kleiner + 1.0) / (B + 1.0), 1.0)

def fdr(p, q=0.05):
    m = len(p)
    aus = np.zeros(m, bool)
    if m == 0:
        return aus
    idx = np.argsort(p, kind="stable")
    grenze = -1
    for rang, i in enumerate(idx):
        if p[i] <= ((rang + 1) / m) * q:
            grenze = rang
    if grenze >= 0:
        aus[idx[:grenze + 1]] = True
    return aus

# Messbare Evidenzbaender bei 48 kHz (Zeit.h:120-145, Kappe 18 kHz, Trennung 200 Hz)
FIX = r"C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json"
fein = json.load(open(FIX, encoding="utf-8"))
h2d = lambda s: struct.unpack(">d", bytes.fromhex(s))[0]
KANTEN = np.array([h2d(s) for s in fein["kanten_hz"]["hex64"]])
MITTEN = np.array([h2d(s) for s in fein["mitten_hz"]["hex64"]]) if "mitten_hz" in fein else np.sqrt(KANTEN[:-1] * KANTEN[1:])

def stufen_bins(fs=48000.0):
    kappe = min(18000.0, 0.95 * fs * 0.5)
    trenn = int(np.argmax(MITTEN >= 200.0))
    out = {}
    for b in range(221):
        N = 16384 if b < trenn else 4096
        lo, hi = KANTEN[b], KANTEN[b + 1]
        if hi > kappe:
            continue
        v = int(np.ceil(lo * N / fs)); w = min(int(np.ceil(hi * N / fs)), N // 2 + 1)
        if w > v:
            out[b] = (N, v, w)
    return out, trenn

BAENDER, TRENN = stufen_bins()
MESSBAR = np.array(sorted(BAENDER))

def urteil(D, baender=MESSBAR):
    """D: (n_fenster x m) Fensterdeltas; liefert (unten, oben, n_sig, befund)."""
    n, m = D.shape
    fm = D.mean(axis=1)
    unten, oben = block_bootstrap(fm, 42)
    Ws = np.stack([gewichte(n, 42 + int(b)) for b in baender])      # (m,400,n)
    mittel = np.einsum("bzn,nb->bz", Ws, D) / n
    p = p_aus_mitteln(mittel)
    sig = int(fdr(p).sum())
    if sig > 0 and unten > 0.0:
        bef = "Verbessert"
    elif sig > 0 and oben < 0.0:
        bef = "Verschlechtert"
    else:
        bef = "Unveraendert"
    return unten, oben, sig, bef, p

if __name__ == "__main__":
    print("messbare Evidenzbaender bei 48 kHz: %d (Trennindex %d)" % (len(MESSBAR), TRENN))
    # Selbstpruefung gegen den Golden-Gedanken: konstante Reihe -> p_min
    print("p_min =", 2 / 401, " p(konstant 1.0, n=8) =", p_aus_mitteln(gewichte(8, 42) @ np.ones(8) / 8))
    for n in [1, 2, 3, 4, 5, 6, 8]:
        W = gewichte(n, 42)
        print("n=%d: verschiedene Bootstrapmittel-Muster: %d" % (n, len({tuple(r) for r in W})))
