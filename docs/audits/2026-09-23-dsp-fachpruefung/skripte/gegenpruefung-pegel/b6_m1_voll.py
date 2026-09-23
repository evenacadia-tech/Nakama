# B6 ergaenzend: M1-Resonanzsuche ueber alle Stufen (AnalyseEngine.cpp:210-213 feste 16384/8192/4096/2048,
# Komposit :662-688 mit Naehten 160-250 Hz und 1,6-2,5 kHz, Persistenz aus der zustaendigen Stufe :1016-1017).
# Erwartungswert-Spektren: rosa Grundrauschen + stationaerer Sinus (+15 dB ueber dem Rauschband).
import numpy as np
import importlib.util
spec = importlib.util.spec_from_file_location("b6", "b6_m1_resonanz.py")
src = open("b6_m1_resonanz.py", encoding="utf-8").read().split("for fs in [44100.0")[0]
ns = {}; exec(src, ns)
lo, hi, zen, NB, BPO = ns["lo"], ns["hi"], ns["zen"], ns["NB"], ns["BPO"]
welch_psd, baender, median_um = ns["welch_psd"], ns["baender"], ns["median_um"]
ea = lambda f: int(np.argmax(zen >= f))
iZ200, iZ2000, iN160, iN250, iN1600, iN2500 = ea(200), ea(2000), ea(160), ea(250), ea(1600), ea(2500)

def stufe(fs, N, f0, a, von, bis, skala):
    d = baender(fs, N, welch_psd(fs, N, f0, a, 1e-6))
    dbw = 10*np.log10(d*skala)                     # je Segment, NaN = Band ohne Bin
    k = np.full(NB, np.nan); k[von:bis] = d[von:bis]*skala
    idx = np.where(np.isfinite(k))[0]
    kk = np.interp(np.arange(NB), idx, k[idx])      # interpoliereLuecken (Raender klemmen)
    return 10*np.log10(kk), dbw

def pruefe(fs, f0, ueber=15.0):
    skala = 1.5*fs/8192
    # Amplitude: Sinus-Spitzenbin ~ueber dB ueber der Rauschdichte bei f0 (Stufe mit ihrer Punktzahl)
    N0 = 16384 if f0 < 200 else (4096 if f0 < 2000 else 2048)
    a = np.sqrt((1e-6/f0)*10**(ueber/10)*3*fs/N0)
    tief, dbB = stufe(fs, 16384, f0, a, 0, iN250, skala)
    mitte, dbM = stufe(fs, 4096, f0, a, iN160, iN2500, skala)
    hoch, dbH = stufe(fs, 2048, f0, a, iN1600, NB, skala)
    komp = np.empty(NB)
    for b in range(NB):
        f = zen[b]
        if b < iN160: v = tief[b]
        elif b < iN250: t = np.log(f/160)/np.log(250/160); v = tief[b]*(1-t) + mitte[b]*t
        elif b < iN1600: v = mitte[b]
        elif b < iN2500: t = np.log(f/1600)/np.log(2500/1600); v = mitte[b]*(1-t) + hoch[b]*t
        else: v = hoch[b]
        komp[b] = v
    glatt = np.array([median_um(komp, b, 0, NB) for b in range(NB)]); ex = komp - glatt
    kand = [b for b in range(1, NB-1) if ex[b] >= 6 and ex[b] >= ex[b-1] and ex[b] >= ex[b+1]]
    if not kand: return False, "kein Maximum"
    b = max(kand, key=lambda i: ex[i]); v = w = b
    while v > 0 and ex[v-1] >= ex[b]-6: v -= 1
    while w < NB-1 and ex[w+1] >= ex[b]-6: w += 1
    breite = (w-v+1)/BPO
    if b < iZ200: dbw, av, ab = dbB, 0, iN250
    elif b < iZ2000: dbw, av, ab = dbM, iN160, iN2500
    else: dbw, av, ab = dbH, iN1600, NB
    pers = False
    for j in range(max(0, b-1), min(NB, b+2)):
        if np.isfinite(dbw[j]):
            m = median_um(dbw, j, av, ab)
            pers = pers or (np.isfinite(m) and dbw[j] - m >= 6)
    if breite > 1/6 + 1e-9: return False, "zu breit"
    if not pers: return False, "Persistenz"
    return True, "ok"

for fs in [48000.0, 96000.0, 192000.0]:
    grenzen = [30, 45, 60, 100, 150, 200, 300, 500, 800, 1200, 2000, 4000]
    zeile = []
    for a_, b_ in zip(grenzen[:-1], grenzen[1:]):
        scan = np.geomspace(a_, b_, 40, endpoint=False)
        ok = [pruefe(fs, f)[0] for f in scan]
        zeile.append("%d-%d: %3.0f%%" % (a_, b_, 100*np.mean(ok)))
    print("fs=%6.0f Erkennungsquote Sinus +15 dB: " % fs + " | ".join(zeile))
