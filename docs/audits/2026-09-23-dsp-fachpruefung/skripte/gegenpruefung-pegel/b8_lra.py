# B8 unabhaengig: LRA mit 1-s-Hop + 0,1-LU-Histogramm (Lautheit.h:65-87, 255-314; FeatureEngine.h:409-426)
# gegen EBU Tech 3342 V4 §3.1 (>= 10 Hz, MATLAB-Referenz §5).
import numpy as np
rng = np.random.default_rng(3342)

def st_folge(zellen, hop):              # zellen: K-gewichtete Mittelenergie je 100-ms-Zelle
    out = []
    for i in range(30, len(zellen)+1, hop):
        out.append(-0.691 + 10*np.log10(np.mean(zellen[i-30:i]) + 1e-300))
    return np.array(out)

def lra_ebu(st):                        # Tech 3342 §5 (MATLAB), woertlich nachgebaut
    a = st[st >= -70]
    integ = 10*np.log10(np.mean(10**(a/10)))
    r = np.sort(a[a >= integ - 20]); n = len(r)
    return r[int(round((n-1)*0.10 + 1)) - 1], r[int(round((n-1)*0.95 + 1)) - 1]

def lra_nakama(st):                     # Lautheit.h:98-104, 255-314
    unten, br, nb = -70.0, 0.1, 1001
    h = np.zeros(nb, np.int64)
    for v in st:
        if v >= -70.0:
            b = int(np.floor((v - unten)/br))
            if 0 <= b < nb: h[b] += 1
    if h.sum() < 60: return None        # kLraMindestSekunden 60 bei 1 s je Wert
    mitte = unten + (np.arange(nb)+0.5)*br
    n = h.sum(); mittel = -0.691 + 10*np.log10(np.sum(h*10**((mitte+0.691)/10))/n)
    sel = mitte >= mittel - 20; ges = h[sel].sum()
    def p(q):
        lauf = 0
        for b in np.where(sel)[0]:
            lauf += h[b]
            if lauf >= q*ges: return mitte[b]
    return p(0.95) - p(0.10)

def zellen_ton(pegel, dauer):           # 1-kHz-Stereoton: Zellen so, dass LUFS = Pegel (harte Uebergaenge)
    z = []
    for L, d in zip(pegel, dauer):
        z += [10**((L + 0.691)/10)] * int(round(d/0.1))
    return np.array(z)

print("A) EBU Tech 3342 Tabelle 1, Faelle 1-4 (Soll +-1 LU):")
for nm, pg, soll in [("#1 -20/-30", [-20, -30], 10), ("#2 -20/-15", [-20, -15], 5), ("#3 -40/-20", [-40, -20], 20),
                     ("#4 -50/-35/-20/-35/-50", [-50, -35, -20, -35, -50], 15)]:
    z = zellen_ton(pg, [20]*len(pg))
    lo, hi = lra_ebu(st_folge(z, 1))
    lo1, hi1 = lra_ebu(st_folge(z, 10))
    print("   %-24s Soll %2d | 10 Hz (Norm) %.2f | 1 Hz Normrechnung %.2f | Nakama 1 Hz+Histogramm %s"
          % (nm, soll, hi-lo, hi1-lo1, "kein Wert (<60 Werte)" if lra_nakama(st_folge(z, 10)) is None else "%.2f" % lra_nakama(st_folge(z, 10))))

# B) Musikartige Verlaeufe: Abschnitte mit eigenem Pegel, Einschwingen, langsame Drift und Fluktuation
def musik(dauer_s, sprung_db=6.0):
    n = int(dauer_s*10); z = np.empty(n); t = 0
    basis = -18.0
    while t < n:
        lang = int(rng.uniform(6, 25)*10); basis = np.clip(basis + rng.normal(0, sprung_db), -35, -8)
        seg = basis + np.cumsum(rng.normal(0, 0.35, lang)) * 0.3 + rng.normal(0, 2.0, lang)
        z[t:t+lang] = seg[:n-t]; t += lang
    fade = min(80, n//6); z[-fade:] += np.linspace(0, -30, fade)      # Ausblendung am Ende
    return 10**((z + 0.691)/10)

print("\nB) Musikartige Verlaeufe: LRA(10 Hz, Norm) gegen 1-Hz-Unterabtastung (alle 10 Phasen) und Nakama-Weg")
for dauer in [60, 75, 90, 120, 180, 300]:
    ab1, abn, ab1max = [], [], []
    for rep in range(200):
        z = musik(dauer)
        st10 = st_folge(z, 1)
        lo, hi = lra_ebu(st10); ref = hi - lo
        phasen = []
        for ph in range(10):
            s1 = st10[ph::10]
            l1, h1 = lra_ebu(s1); phasen.append((h1 - l1) - ref)
        ab1.append(np.mean(np.abs(phasen))); ab1max.append(np.max(np.abs(phasen)))
        nk = lra_nakama(st_folge(z, 10))
        if nk is not None: abn.append(nk - ref)
    abn = np.array(abn)
    print("   %3d s: |1 Hz - 10 Hz| mittel %.2f LU, p95 der Phasen-Maxima %.2f LU, max %.2f LU | Nakama-10Hz: n=%3d, mittel %+.2f, p5..p95 [%+.2f, %+.2f] LU"
          % (dauer, np.mean(ab1), np.percentile(ab1max, 95), np.max(ab1max), len(abn),
             abn.mean() if len(abn) else np.nan, np.percentile(abn, 5) if len(abn) else np.nan, np.percentile(abn, 95) if len(abn) else np.nan))
