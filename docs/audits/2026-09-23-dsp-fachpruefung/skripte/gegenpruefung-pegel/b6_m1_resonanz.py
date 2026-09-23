# B6 unabhaengig: M1-Resonanzsuche (AnalyseEngine.cpp:41-62, 91-99, 440-498, 596-690, 965-1046)
# im Bassbereich. Erwartungswert-Spektrum (kein Zufall): rosa Grundrauschen + stationaerer Sinus.
# Zwei Pfade werden getrennt geprueft:
#  (P) Persistenz je Segment: Median ueber +-6 Baender braucht >= 5 ENDLICHE Werte (NaN = Band ohne Bin)
#  (K) Kandidat auf der Kompositkurve (Luecken linear in Leistung interpoliert), Breite <= 1/6 Okt.
import numpy as np

BPO, NB = 24, 221
lo = 30.0*2**(np.arange(NB)/BPO); hi = 30.0*2**((np.arange(NB)+1)/BPO); zen = np.sqrt(lo*hi)
erster_ab = lambda f: int(np.argmax(zen >= f))
iZ200, iN160, iN250 = erster_ab(200), erster_ab(160), erster_ab(250)

def hann_W(nu, N):
    # |sum w_n e^{-j2pi nu n/N}| fuer periodisches Hann, geschlossen: N/2*D(nu) - N/4*(D(nu-1)+D(nu+1)), D = Dirichlet-Kern normiert
    def D(x):
        x = np.asarray(x, float); out = np.ones_like(x, dtype=complex)
        m = np.abs(np.sin(np.pi*x/N)) > 1e-15
        out[m] = np.sin(np.pi*x[m])/(N*np.sin(np.pi*x[m]/N))*np.exp(-1j*np.pi*x[m]*(N-1)/N)
        return out
    return 0.5*N*D(nu) - 0.25*N*(D(nu-1) + D(nu+1))

def welch_psd(fs, N, f0, a_sin, rosa_c):
    k = np.arange(N//2 + 1); f = k*fs/N
    w = 0.5 - 0.5*np.cos(2*np.pi*np.arange(N)/N); sw2 = np.sum(w*w)
    dicht = 2.0/(fs*sw2)
    psd = rosa_c/np.maximum(f, fs/N)                         # Grundrauschen als Erwartungsdichte
    if a_sin > 0:
        X = 0.5*a_sin*hann_W(k - f0*N/fs, N)
        psd = psd + np.abs(X)**2*dicht
    return psd

def baender(fs, N, psd):
    von = np.ceil(lo*N/fs).astype(int); bis = np.minimum(np.ceil(hi*N/fs).astype(int), N//2 + 1)
    d = np.full(NB, np.nan)
    for b in range(NB):
        if bis[b] > von[b]: d[b] = psd[von[b]:bis[b]].mean()
    return d

def median_um(db, b, v, bi):
    werte = [db[min(max(b+j, v), bi-1)] for j in range(-6, 7)]
    werte = [x for x in werte if np.isfinite(x)]
    return np.median(werte) if len(werte) >= 5 else np.nan

def pruefe(fs, f0, ueber_db=15.0):
    N = 16384; skala = 1.5*fs/8192
    rosa = baender(fs, N, welch_psd(fs, N, f0, 0.0, 1e-6))
    b0 = int(np.argmin(np.abs(zen - f0)))
    ref = np.nanmean(rosa[max(0, b0-3):b0+4])
    # Sinusamplitude so, dass sein Band ~ueber_db ueber dem Rauschband liegt (grob, nur Szenario)
    a = np.sqrt(2*ref*(10**(ueber_db/10))*fs/N*1.5)
    d = baender(fs, N, welch_psd(fs, N, f0, a, 1e-6))
    db = 10*np.log10(d*skala)                               # NaN bleibt NaN (segmentInBaender)
    # (P) Persistenzpfad (akkuVon=0, akkuBis=iN250), zustaendig 0..iZ200
    exc_ok = np.zeros(NB, bool); med_ok = np.zeros(NB, bool)
    for b in range(0, iZ200):
        if not np.isfinite(db[b]): continue
        m = median_um(db, b, 0, iN250)
        med_ok[b] = np.isfinite(m); exc_ok[b] = np.isfinite(m) and db[b] - m >= 6.0
    # (K) Kompositkurve: lineare Interpolation der Luecken in Leistung ueber den Bandindex
    lin = d*skala; idx = np.where(np.isfinite(lin))[0]
    komp = 10*np.log10(np.interp(np.arange(NB), idx, lin[idx]))
    glatt = np.array([median_um(komp, b, 0, NB) for b in range(NB)])
    ex = komp - glatt
    kand = [b for b in range(1, NB-1) if ex[b] >= 6 and ex[b] >= ex[b-1] and ex[b] >= ex[b+1]]
    if not kand: return "kein Excess-Maximum >= 6 dB", med_ok, None
    b = max(kand, key=lambda i: ex[i])
    v = b; w = b
    while v > 0 and ex[v-1] >= ex[b]-6: v -= 1
    while w < NB-1 and ex[w+1] >= ex[b]-6: w += 1
    breite = (w - v + 1)/BPO
    pers = any(exc_ok[j] for j in range(max(0, b-1), min(NB, b+2)))
    grund = []
    if breite > 1/6 + 1e-9: grund.append("zu breit %.3f Okt" % breite)
    if not pers: grund.append("Persistenz 0 (Median %s)" % ("NaN: < 5 endliche Nachbarn" if not any(med_ok[max(0,b-1):b+2]) else "endlich, aber Excess < 6 dB"))
    return ("ERKANNT (%.1f Hz, %.3f Okt)" % (zen[b], breite)) if not grund else "VERWORFEN: " + "; ".join(grund), med_ok, b

for fs in [44100.0, 48000.0, 96000.0]:
    N = 16384
    von = np.ceil(lo*N/fs).astype(int); bis = np.minimum(np.ceil(hi*N/fs).astype(int), N//2+1)
    endlich = bis > von
    # Struktur: ab welchem Band liefert der Persistenzmedian IMMER einen Wert?
    nfin = np.array([sum(endlich[min(max(b+j, 0), iN250-1)] for j in range(-6, 7)) for b in range(iZ200)])
    immer = next(b for b in range(iZ200) if all(nfin[b:] >= 5))
    print("fs=%6.0f: Bassstufe %.2f Hz/Bin; Baender ohne Bin unter 250 Hz: %d; Persistenzmedian ab Band %d (%.1f Hz) stets endlich"
          % (fs, fs/N, int((~endlich[:iN250]).sum()), immer, zen[immer]))
    grenzen = [30, 40, 45, 50, 60, 80, 120, 200]
    scan = np.arange(31.0, 199.0, 0.25)
    erg = [pruefe(fs, f0)[0] for f0 in scan]
    ok = np.array([e.startswith("ERKANNT") for e in erg])
    zeile = []
    for a, b in zip(grenzen[:-1], grenzen[1:]):
        m = (scan >= a) & (scan < b)
        zeile.append("%d-%d Hz: %3.0f%%" % (a, b, 100*ok[m].mean()))
    gr = {}
    for e, f0 in zip(erg, scan):
        if f0 < 60 and not e.startswith("ERKANNT"):
            key = e.split(":")[0] if e.startswith("kein") else e.split(": ")[1].split(" ")[0] + (" +Pers" if "Persistenz" in e else "")
            gr[key] = gr.get(key, 0) + 1
    print("     erkannt (Sinus +15 dB, 0,25-Hz-Raster): " + " | ".join(zeile))
    print("     unter 60 Hz verworfen wegen: %s; tiefster erkannter Sinus %.2f Hz" % (gr, scan[ok][0] if ok.any() else float('nan')))
