# B3 + B6 unabhaengig, mit dem eingefrorenen Gitter aus eq-copilot/schemas/v3/bandgitter/.
# Zuordnung wie Zeit.h:105-145 (trennIndex ueber Bandmitte >= 200 Hz, Bin k gehoert zu [lo, hi) per ceil),
# Bandwert wie Spektrum.h:488-495 (MITTEL der Bin-Dichten), Livegruppe wie Frame.h:337-351
# (SUMME der Feinband-Mittel), bandpassung wie zusammenhang.rs:190-212, bandpegel wie maskierung.rs:133-152.
import json, struct, math
import numpy as np
G = r"C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\v3\bandgitter"
fein = json.load(open(G + r"\nakama_1_24_oct_30_18k_v1.json", encoding="utf-8"))
grob = json.load(open(G + r"\nakama_log64_v1.json", encoding="utf-8"))
dec = lambda s: struct.unpack(">d", bytes.fromhex(s))[0]
kan = np.array([dec(s) for s in fein["kanten_hz"]["hex64"]]); mit = np.array([dec(s) for s in fein["mitten_hz"]["hex64"]])
bw = kan[1:] - kan[:-1]
gruppen = [(g["fein_von"], g["fein_bis_exkl"]) for g in grob["gruppen"]]
trenn = int(np.argmax(mit >= 200.0))
print("Gitter: %d Baender, Kanten %.2f..%.2f Hz, trennIndex %d (Mitte %.1f Hz); Gruppen je 3/4 Feinbaender: %s"
      % (len(mit), kan[0], kan[-1], trenn, mit[trenn], sorted(set(b - a for a, b in gruppen))))

def zuordnung(fs):
    kappe = min(18000.0, 0.95*fs*0.5); nb = np.zeros(221, int); ueber = np.zeros(221, bool)
    for b in range(221):
        N = 16384 if b < trenn else 4096
        if kan[b+1] > kappe: ueber[b] = True; continue
        v = math.ceil(kan[b]*N/fs); w = min(math.ceil(kan[b+1]*N/fs), N//2 + 1)
        nb[b] = max(v, w) - v
    return nb, ueber

print("\nB6) FeatureEngine: Baender ohne Bin (unterhalb der Kappe) je Samplerate")
res = {}
for fs in [44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0]:
    nb, ue = zuordnung(fs); leer = (nb == 0) & ~ue; res[fs] = nb
    print("   fs=%6.0f: leer %3d/221 (Bass %2d, Haupt %2d), hoechste leere Bandmitte %6.0f Hz, ueber Kappe %d, Baender mit 1 Bin %3d"
          % (fs, leer.sum(), (leer & (np.arange(221) < trenn)).sum(), (leer & (np.arange(221) >= trenn)).sum(),
             mit[leer].max() if leer.any() else float('nan'), ue.sum(), (nb == 1).sum()))

print("\nB3a) Livegruppen: Summe der Feinband-DICHTEN. Weissrauschen (Dichte P konstant) -> Livewert 10log10(k_belegt)+10log10(P)")
for fs in [48000.0, 96000.0, 192000.0]:
    nb = res[fs]
    kb = np.array([(nb[a:b] > 0).sum() for a, b in gruppen]); kt = np.array([b - a for a, b in gruppen])
    v = kb > 0
    print("   fs=%6.0f: Livewert streut %.2f dB (nur Gruppen mit Wert); Gruppen mit fehlenden Feinbaendern %d, davon Einbruch >= 3 dB: %d; erste 12 k_belegt/k: %s"
          % (fs, 10*np.log10(kb[v].max()/kb[v].min()), ((kb > 0) & (kb < kt)).sum(), ((kb > 0) & (10*np.log10(kt/np.maximum(kb, 1)) >= 3)).sum(),
             " ".join("%d/%d" % (x, y) for x, y in zip(kb[:12], kt[:12]))))
    # Energietreue Alternative: Summe Dichte*Bandbreite ueber gemessene Feinbaender (fuer Rosa flach)
print("   (Rosa Rauschen: echte Bandleistung je Livegruppe ist ~ proportional zur Oktavbreite der Gruppe;"
      " die Dichtesumme faellt dagegen mit -3 dB/Okt.)")

print("\nB3b) bandpassung ('Anteil der LINEAREN Leistung', zusammenhang.rs:185-212) aus Dichten gegen echten Leistungsanteil")
def anteile(d, sel):
    return d[sel].sum()/d.sum(), (d*bw)[sel].sum()/(d*bw).sum()
for nm, d in [("weiss", np.ones(221)), ("rosa", 1.0/mit), ("-4,5 dB/Okt", mit**-1.5)]:
    zeile = []
    for lo, hi in [(60, 120), (250, 500), (1000, 2000), (4000, 8000), (8000, 16000)]:
        sel = (mit >= lo) & (mit < hi); a, w = anteile(d, sel)
        zeile.append("%g-%g Hz: %.2f%% statt %.2f%%" % (lo, hi, 100*a, 100*w))
    print("   %-11s " % nm + " | ".join(zeile))
# Screening-Gruppe (bandintervall_der_gruppe, screening.rs:81-93) fuer rosa Rauschen
def gi(g):
    c = lambda z: (z + 63)//64
    return c(g*221), min(c((g+1)*221), 221)
for g in [5, 20, 40, 60]:
    a, b = gi(g); sel = np.zeros(221, bool); sel[a:b] = True
    x, y = anteile(1.0/mit, sel)
    print("   Screening-Gruppe %2d (Baender %3d-%3d, %.0f-%.0f Hz), rosa: gruppenenergie %.4f statt %.4f" % (g, a, b-1, kan[a], kan[b], x, y))

print("\nB3c) Rangumkehr, Befundband 8-16 kHz: A = 40 %% Leistung im Band + 60 %% bei 80-160 Hz; B = 30 %% im Band + 70 %% bei 3-6 kHz")
def quelle(anteile_baender):
    p = np.zeros(221)
    for (lo, hi), anteil in anteile_baender:
        sel = (mit >= lo) & (mit < hi); p[sel] += anteil*bw[sel]/bw[sel].sum()   # Leistung je Band, im Bereich rosa-flach verteilt
    return p/bw                                                                   # -> Dichte je Band
sel = (mit >= 8000) & (mit < 16000)
A = quelle([((8000, 16000), 0.4), ((80, 160), 0.6)]); B = quelle([((8000, 16000), 0.3), ((3000, 6000), 0.7)])
for nm, d in [("A", A), ("B", B)]:
    a, w = anteile(d, sel); print("   %s: bandpassung aus Dichten %.4f, echter Leistungsanteil %.4f" % (nm, a, w))

print("\nB3d) maskierung wert_db = pegel_b - pegel_a (Summe der Dichten ueber dasselbe Intervall)")
for (lo, hi), ta, tb in [((1000, 2000), -3.0, +3.0), ((100, 400), -6.0, 0.0), ((2000, 2520), -3.0, +3.0)]:
    s = (mit >= lo) & (mit < hi)
    da = (mit/1000.0)**(ta/(10*np.log10(2))); db = (mit/1000.0)**(tb/(10*np.log10(2)))
    wd = 10*np.log10(db[s].sum()/da[s].sum()); wp = 10*np.log10((db*bw)[s].sum()/(da*bw)[s].sum())
    print("   %4g-%4g Hz, Neigung a %+.0f / b %+.0f dB/Okt: Dichtesumme %+.2f dB, Leistung %+.2f dB, Differenz %+.2f dB" % (lo, hi, ta, tb, wd, wp, wd-wp))
