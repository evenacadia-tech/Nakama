# B5: Streuung von Einzelperiodogramm-Bandwerten (keine Musik, reines stationaeres Rauschen).
# (1) chi^2_2-Theorie; (2) M2-Evidenz: Perzentil (Frame.h:425-434, linear) ueber die Einzelfenster
#     EINES Evidenzfensters (~0,32 s: 7 Hauptstufenfenster, 1-2 Bassfenster); (3) M1: Histogramm mit
#     1-dB-Rundung (AnalyseEngine.cpp:500-503) und Rang ceil(p*N) (:816-833), dann Diagnose.cpp:209-223.
import numpy as np
rng = np.random.default_rng(55)
ln = np.log
db = lambda x: 10 * np.log10(x)

print("(1) chi^2_2 (1 Bin, Mono), Populationsquantile in dB:")
q = {p: -ln(1 - p) for p in (0.10, 0.50, 0.95)}
print("    P95-P50 = %.3f dB   P95-P10 = %.3f dB   P50-P10 = %.3f dB"
      % (db(q[.95] / q[.50]), db(q[.95] / q[.10]), db(q[.50] / q[.10])))

# Hann-periodisch, echte FFT: Bandwerte aus 1, 2, 3 Bins (korrelierte Nachbarbins beruecksichtigt)
def bandwerte(nbins, N=4096, frames=20000, stereo_unkorr=False):
    w = 0.5 - 0.5 * np.cos(2 * np.pi * np.arange(N) / N)
    out = []
    k0 = 200
    for _ in range(frames // 500):
        x = rng.normal(0, 1, (500, N))
        X = np.fft.rfft(x * w, axis=1)
        P = np.abs(X[:, k0:k0 + nbins]) ** 2
        if stereo_unkorr:
            y = rng.normal(0, 1, (500, N)); Y = np.fft.rfft(y * w, axis=1)
            P = 0.5 * (P + np.abs(Y[:, k0:k0 + nbins]) ** 2)
        out.append(P.mean(axis=1))
    return np.concatenate(out)

def perz_linear(v, p):
    v = np.sort(v); n = len(v)
    if n == 1: return v[0]
    pos = p * (n - 1); lo = int(pos); hi = min(lo + 1, n - 1); f = pos - lo
    return v[lo] + f * (v[hi] - v[lo])

print("\n(2) M2-Evidenz: P-Werte je Evidenzfenster aus n Einzelfenstern (linear interpoliert), Mittel ueber Fenster")
for nbins in (1, 2, 3, 6):
    v = db(bandwerte(nbins))
    pop = (np.percentile(v, 95) - np.percentile(v, 50), np.percentile(v, 95) - np.percentile(v, 10))
    zeilen = []
    for n in (2, 7, 23):
        m = len(v) // n
        s = v[:m * n].reshape(m, n)
        d9550 = np.mean([perz_linear(r, .95) - perz_linear(r, .50) for r in s])
        d9510 = np.mean([perz_linear(r, .95) - perz_linear(r, .10) for r in s])
        zeilen.append("n=%2d: P95-P50 %.2f, P95-P10 %.2f" % (n, d9550, d9510))
    print("  %d Bin(s): Population P95-P50 %.2f / P95-P10 %.2f dB | " % (nbins, *pop) + " | ".join(zeilen))

def m1_spanne(vals_lin, offset_db):
    """M1: Histogramm ganzzahliger dB (lround), Rang ceil(p*N) -> P50, P95; Diagnose-Spanne."""
    d = np.round(db(vals_lin) + offset_db)          # lround ~ round-half-away; Mass-0-Unterschied
    d = np.sort(d); N = len(d)
    p50 = d[int(np.ceil(0.50 * N)) - 1]; p95 = d[int(np.ceil(0.95 * N)) - 1]
    return p95 - p50

print("\n(3) M1-Diagnose (Resonanzkarte) fuer stationaeres Material, 60 s bei 4096er-Stufe (~1400 Segmente):")
print("    Anteil 'kommt in Wellen' (lround(P95-P50) > 6) und 'wellig' (Spanne > 10 -> Werkzeugwechsel), ueber zufaellige Absolutpegel")
def m1_fall(name, erzeuger, laeufe=400):
    wellen = wellig = 0; sp = []
    for _ in range(laeufe):
        v = erzeuger()
        s = m1_spanne(v, rng.uniform(0, 1))
        sp.append(s); wellen += s > 6; wellig += s > 10
    print("  %-44s Spanne-Median %4.1f dB  'Wellen' %5.1f %%  'wellig'(>10) %5.1f %%"
          % (name, np.median(sp), 100 * wellen / laeufe, 100 * wellig / laeufe))

pool1 = bandwerte(1, frames=400000); pool2 = bandwerte(2, frames=200000); pool3 = bandwerte(3, frames=200000)
pool1s = bandwerte(1, frames=200000, stereo_unkorr=True)
zieh = lambda pool: (lambda: rng.choice(pool, 1400))
m1_fall("Rauschen, 1 Bin, Mono", zieh(pool1))
m1_fall("Rauschen, 1 Bin, Stereo unkorreliert", zieh(pool1s))
m1_fall("Rauschen, 2 Bins, Mono", zieh(pool2))
m1_fall("Rauschen, 3 Bins, Mono", zieh(pool3))

# Stationaerer Ton im Band (1 Bin, Ton auf Binmitte) plus Rauschen: Rice-Leistung |A + n|^2
def ton(excess_db):
    # Bandpegel (T+N) liegt excess_db ueber dem Rauschboden N=1 -> T = 10^(ex/10) - 1
    T = 10 ** (excess_db / 10) - 1
    def f():
        n = (rng.normal(0, 1, 1400) + 1j * rng.normal(0, 1, 1400)) / np.sqrt(2)
        return np.abs(np.sqrt(T) + n) ** 2
    return f
for ex in (6, 10, 20):
    m1_fall("stationaerer Ton + Rauschen, 1 Bin, Excess %d dB" % ex, ton(ex))
