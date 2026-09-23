"""Rechnung 1 - Cramping: RBJ-Entwurf (DspFilter.h) gegen den analogen Prototyp
desselben Typs, gleiches f0/Q/Gain, s = j*2*pi*f.

Auswertung 20 Hz .. 20 kHz (alle vier Raten haben fs/2 > 20 kHz).
Ausgabe: Tabellen auf stdout, alle Faelle in r1_faelle.csv, Schwellen in
r1_schwellen.csv, Bild r1_cramping.png."""
import math
import csv
import numpy as np
from scipy.optimize import brentq
import nk

RATEN = [44100.0, 48000.0, 88200.0, 96000.0]
QS = [0.7, 2.0, 6.0]
GAINS = [6.0, -6.0, 12.0, -12.0]
F_AUS = np.logspace(math.log10(20.0), math.log10(20000.0), 20001)


def fcap(fs):
    # Vertrag 20..20000 Hz, Laufzeit zusaetzlich 0,45 fs (DspFilter.h:109-114)
    return min(20000.0, nk.kNyquistAnteil * fs)


def bell_faelle(fs):
    return [1000.0, 5000.0, 10000.0, 14000.0, 16000.0, 18000.0, fcap(fs)]


def dig_db(c, f, fs):
    return nk.db(nk.biquad_H(c, 2 * np.pi * np.asarray(f) / fs))


def bell_kennzahlen(fs, f0, q, g):
    c = nk.rbj_bell(fs, f0, q, g)
    d = dig_db(c, F_AUS, fs)
    a = nk.db(nk.analog_bell(F_AUS, f0, q, g))
    dev = d - a
    i = int(np.argmax(np.abs(dev)))
    # Spitze: Extremum des digitalen Betrags nahe f0
    lo, hi = f0 / 1.5, min(f0 * 1.5, 0.4999 * fs)
    ff = np.logspace(math.log10(lo), math.log10(hi), 40001)
    dd = dig_db(c, ff, fs)
    j = int(np.argmax(dd) if g > 0 else np.argmin(dd))
    j = min(max(j, 1), ff.size - 2)
    # Parabel in log f
    y0, y1, y2 = dd[j - 1], dd[j], dd[j + 1]
    den = (y0 - 2 * y1 + y2)
    off = 0.5 * (y0 - y2) / den if den != 0 else 0.0
    lf = math.log(ff[j]) + off * (math.log(ff[j + 1]) - math.log(ff[j]))
    fpk = math.exp(lf)
    shift_cent = 1200 * math.log2(fpk / f0)
    pk_db = float(dig_db(c, [fpk], fs)[0])
    # Halbgain-Punkte (g/2 in dB) - dort ist die analoge Bandbreite exakt f0/Q
    ziel = g / 2.0
    fun = lambda f: float(dig_db(c, [f], fs)[0]) - ziel
    fl_d = brentq(fun, 1.0, f0, xtol=1e-9, rtol=1e-14)
    fu_d = brentq(fun, f0, 0.5 * fs * (1 - 1e-12), xtol=1e-9, rtol=1e-14)
    r = 1.0 / (2.0 * q)
    fl_a = f0 * (math.sqrt(1 + r * r) - r)
    fu_a = f0 * (math.sqrt(1 + r * r) + r)
    q_eff = f0 / (fu_d - fl_d)
    return dict(fs=fs, f0=f0, q=q, g=g, maxdev=float(dev[i]), f_maxdev=float(F_AUS[i]),
                dev20k=float(dev[-1]), shift_cent=shift_cent, peak_db=pk_db,
                fl_d=fl_d, fu_d=fu_d, fl_a=fl_a, fu_a=fu_a, q_eff=q_eff,
                bw_ratio=(fu_d - fl_d) / (fu_a - fl_a))


def shelf_kennzahlen(fs, f0, q, g):
    c = nk.rbj_highshelf(fs, f0, q, g)
    d = dig_db(c, F_AUS, fs)
    a = nk.db(nk.analog_highshelf(F_AUS, f0, q, g))
    dev = d - a
    i = int(np.argmax(np.abs(dev)))
    fun = lambda f: float(dig_db(c, [f], fs)[0]) - g / 2.0
    fmid = brentq(fun, 20.0, 0.5 * fs * (1 - 1e-9), xtol=1e-9, rtol=1e-14)
    ab = {}
    for s in (0.1, 0.5, 1.0):
        k = np.nonzero(np.abs(dev) >= s)[0]
        ab[s] = float(F_AUS[k[0]]) if k.size else float("nan")
    return dict(fs=fs, f0=f0, q=q, g=g, maxdev=float(dev[i]), f_maxdev=float(F_AUS[i]),
                dev20k=float(dev[-1]), mid_shift_cent=1200 * math.log2(fmid / f0),
                ab01=ab[0.1], ab05=ab[0.5], ab1=ab[1.0])


def maxdev_bell(fs, f0, q, g, fgrid):
    c = nk.rbj_bell(fs, f0, q, g)
    return float(np.max(np.abs(dig_db(c, fgrid, fs) - nk.db(nk.analog_bell(fgrid, f0, q, g)))))


def main():
    rows = []
    for fs in RATEN:
        for f0 in bell_faelle(fs):
            for q in QS:
                for g in GAINS:
                    rows.append(bell_kennzahlen(fs, f0, q, g))
    with open("r1_faelle.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0].keys()))
        w.writeheader(); w.writerows(rows)

    # Spiegelung Boost/Cut pruefen
    spieg = 0.0
    for r in rows:
        if r["g"] > 0:
            m = [x for x in rows if x["fs"] == r["fs"] and x["f0"] == r["f0"] and x["q"] == r["q"] and x["g"] == -r["g"]][0]
            spieg = max(spieg, abs(r["maxdev"] + m["maxdev"]))
    print("Cut ist exakt Kehrwert des Boosts: max |dev(+g)+dev(-g)| = %.1e dB" % spieg)
    print("max |Spitzenverschiebung| ueber alle Bell-Faelle: %.2e Cent" % max(abs(r["shift_cent"]) for r in rows))
    print("max |Spitzenpegel - g| ueber alle Bell-Faelle: %.2e dB" % max(abs(r["peak_db"] - r["g"]) for r in rows))

    for g in (12.0, 6.0):
        print("\nBELL +%g dB: max. Abweichung digital-analog in dB (20 Hz-20 kHz) | Q_eff (Halbgain-Punkte, analog = Q)" % g)
        print("%-7s %-8s" % ("fs", "f0") + "".join("   Q=%-4g dev   Qeff  f_u,d/f_u,a" % q for q in QS))
        for fs in RATEN:
            for f0 in bell_faelle(fs):
                line = "%-7g %-8g" % (fs, f0)
                for q in QS:
                    r = [x for x in rows if x["fs"] == fs and x["f0"] == f0 and x["q"] == q and x["g"] == g][0]
                    line += "   %+7.2f@%5.0f %6.2f %6.3f " % (r["maxdev"], r["f_maxdev"], r["q_eff"], r["fu_d"] / r["fu_a"])
                print(line)

    # Schwellen: ab welchem f0 wird die Abweichung > 0,05/0,1/0,5/1 dB?
    fgrid = np.logspace(math.log10(20.0), math.log10(20000.0), 8001)
    print("\nSCHWELLEN Bell: kleinstes f0 [Hz], ab dem max|Abweichung| >= 0,05 / 0,1 / 0,5 / 1,0 dB (Boost; Cut gespiegelt)")
    srows = []
    for g in (12.0, 6.0):
        for fs in RATEN:
            f0s = 100.0 * 2 ** (np.arange(0, 1 + 48 * math.log2(fcap(fs) / 100.0)) / 48.0)
            f0s = list(f0s[f0s < fcap(fs)]) + [fcap(fs)]
            f0s = np.array(f0s)
            for q in QS:
                md = np.array([maxdev_bell(fs, f0, q, g, fgrid) for f0 in f0s])
                out = []
                for s in (0.05, 0.1, 0.5, 1.0):
                    k = np.nonzero(md >= s)[0]
                    if k.size == 0:
                        out.append(float("nan"))
                    elif k[0] == 0:
                        out.append(f0s[0])
                    else:
                        # log-lineare Interpolation
                        a, b = k[0] - 1, k[0]
                        t = (s - md[a]) / (md[b] - md[a])
                        out.append(math.exp(math.log(f0s[a]) + t * (math.log(f0s[b]) - math.log(f0s[a]))))
                srows.append(dict(g=g, fs=fs, q=q, s005=out[0], s01=out[1], s05=out[2], s1=out[3],
                                  max_at_cap=md[-1]))
                print("g=%+5.1f fs=%-6g Q=%-4g  0,05dB: %8.0f  0,1dB: %8.0f  0,5dB: %8.0f  1dB: %8.0f   (bei f0=%g: %.2f dB)"
                      % (g, fs, q, *out, fcap(fs), md[-1]))
    with open("r1_schwellen.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(srows[0].keys()))
        w.writeheader(); w.writerows(srows)

    # High-Shelf
    print("\nHIGH-SHELF Q=0,707: max. Abweichung (dB) und wo; ab welcher Frequenz |dev| >= 0,1/0,5/1 dB; Halbgain-Punkt-Verschiebung")
    hs = []
    for fs in RATEN:
        for f0 in (5000.0, 10000.0, 15000.0):
            for g in GAINS:
                r = shelf_kennzahlen(fs, f0, 0.707, g)
                hs.append(r)
                print("fs=%-6g f0=%-6g g=%+5.1f  max %+6.2f dB bei %7.0f Hz | bei 20k %+6.2f | ab 0,1dB %7.0f  0,5dB %7.0f  1dB %7.0f | Mitte %+.1e Cent"
                      % (fs, f0, g, r["maxdev"], r["f_maxdev"], r["dev20k"], r["ab01"], r["ab05"], r["ab1"], r["mid_shift_cent"]))
    with open("r1_highshelf.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(hs[0].keys()))
        w.writeheader(); w.writerows(hs)

    # Bild
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    fig, ax = plt.subplots(1, 3, figsize=(16, 5))
    for k, fs in enumerate((44100.0, 48000.0)):
        for f0, col in zip((5000.0, 10000.0, 16000.0, fcap(fs)), ("C0", "C1", "C2", "C3")):
            c = nk.rbj_bell(fs, f0, 2.0, 12.0)
            ax[k].semilogx(F_AUS, dig_db(c, F_AUS, fs), col, label="RBJ f0=%g" % f0)
            ax[k].semilogx(F_AUS, nk.db(nk.analog_bell(F_AUS, f0, 2.0, 12.0)), col + "--", label="analog")
        ax[k].set_title("Bell +12 dB Q 2, fs=%g" % fs); ax[k].set_xlim(1000, 20000); ax[k].grid(True, which="both")
        ax[k].legend(fontsize=7)
    for f0, col in zip((5000.0, 10000.0, 15000.0), ("C0", "C1", "C2")):
        c = nk.rbj_highshelf(44100.0, f0, 0.707, 12.0)
        ax[2].semilogx(F_AUS, dig_db(c, F_AUS, 44100.0), col, label="RBJ f0=%g" % f0)
        ax[2].semilogx(F_AUS, nk.db(nk.analog_highshelf(F_AUS, f0, 0.707, 12.0)), col + "--", label="analog")
    ax[2].set_title("High-Shelf +12 dB Q 0,707, fs=44100"); ax[2].set_xlim(1000, 20000); ax[2].grid(True, which="both")
    ax[2].legend(fontsize=7)
    fig.tight_layout(); fig.savefig("r1_cramping.png", dpi=90)


if __name__ == "__main__":
    main()
