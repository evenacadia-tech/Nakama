"""Rechnung 3 - Ist die Ruheantwort des SVF (DspSvf.h) identisch mit RBJ (DspFilter.h)?

(a) Frequenzbereich: H_svf aus dem ZUSTANDSRAUM der tatsaechlichen Tick-Rekursion
    (nicht aus der analytischen Formel in DspProgramm.cpp, die die Gleichheit
    schon voraussetzt) gegen H_rbj aus den Produktkoeffizienten.
    Raster: fs 44,1k..192k (6 Raten), f0 20 Hz..0,45 fs (41 log.), Q 0,15..24
    (21 log.), Gain -24..+24 dB in 3-dB-Schritten (Vertrag +-12, dynamische
    Summe gain+g_dyn bis +-24), Typ bell/low_shelf/high_shelf.
(b) Gleiches Raster in mpmath (60 Stellen) fuer die schlechtesten Faelle:
    exakte Entwuerfe -> Differenz 0 beweist die algebraische Gleichheit, die
    Restabweichung in double ist Rundung.
(c) Zeitbereich: beide Produkt-Ticks (double) mit Rauschen, Vergleich
    Sample fuer Sample."""
import math
import numpy as np
import mpmath as mp
import nk

RATEN = [44100.0, 48000.0, 88200.0, 96000.0, 176400.0, 192000.0]
TYPEN = ["bell", "lowshelf", "highshelf"]
QS = np.logspace(math.log10(0.15), math.log10(24.0), 21)
GAINS = np.arange(-24.0, 24.0 + 1e-9, 3.0)


def raster_f0(fs):
    return np.logspace(math.log10(20.0), math.log10(0.45 * fs), 41)


def fgitter(fs):
    return np.logspace(math.log10(1.0), math.log10(0.4999 * fs), 600)


def main():
    gesamt = {}
    worst = {}
    for typ in TYPEN:
        for fs in RATEN:
            f = fgitter(fs)
            w = 2 * np.pi * f / fs
            for f0 in raster_f0(fs):
                wf = np.append(w, 2 * np.pi * f0 / fs)
                gG = nk.svf_grundG(f0, fs)
                for q in QS:
                    for g in GAINS:
                        c_r = nk.RBJ[typ](fs, f0, q, g)
                        c_s = nk.SVF[typ](gG, q, g)
                        d = np.max(np.abs(20 * np.log10(np.abs(nk.svf_H(c_s, wf)) / np.abs(nk.biquad_H(c_r, wf)))))
                        vertrag = (f0 <= 20000.0) and abs(g) <= 12.0
                        key = (typ, "vertrag" if vertrag else "erweitert")
                        if d > gesamt.get(key, -1):
                            gesamt[key] = d
                            worst[key] = (fs, f0, q, g)
    print("(a) max |20log10(|H_svf|/|H_rbj|)| ueber das Raster (1 Hz..0,4999 fs):")
    for k in sorted(gesamt):
        print("   %-10s %-10s  %.2e dB   schlechtester Fall fs=%g f0=%.1f Q=%.3f g=%+g" % (k[0], k[1], gesamt[k], *worst[k]))

    # (b) mpmath fuer die schlechtesten Faelle
    mp.mp.dps = 60
    print("\n(b) mpmath (60 Stellen) in den schlechtesten Faellen:")
    for k in sorted(worst):
        typ = k[0]; fs, f0, q, g = worst[k]
        fs_m, f0_m, q_m, g_m = mp.mpf(fs), mp.mpf(f0), mp.mpf(q), mp.mpf(g)
        A = mp.power(10, g_m / 40); w0 = 2 * mp.pi * f0_m / fs_m
        al = mp.sin(w0) / (2 * q_m); c = mp.cos(w0)
        if typ == "bell":
            a0 = 1 + al / A; cr = [(1 + al * A) / a0, -2 * c / a0, (1 - al * A) / a0, -2 * c / a0, (1 - al / A) / a0]
            gg = mp.tan(mp.pi * f0_m / fs_m); kk = 1 / (q_m * A); m = [1, kk * (A * A - 1), 0]
        elif typ == "lowshelf":
            z = 2 * mp.sqrt(A) * al; a0 = (A + 1) + (A - 1) * c + z
            cr = [A * ((A + 1) - (A - 1) * c + z) / a0, 2 * A * ((A - 1) - (A + 1) * c) / a0,
                  A * ((A + 1) - (A - 1) * c - z) / a0, -2 * ((A - 1) + (A + 1) * c) / a0, ((A + 1) + (A - 1) * c - z) / a0]
            gg = mp.tan(mp.pi * f0_m / fs_m) / mp.sqrt(A); kk = 1 / q_m; m = [1, kk * (A - 1), A * A - 1]
        else:
            z = 2 * mp.sqrt(A) * al; a0 = (A + 1) - (A - 1) * c + z
            cr = [A * ((A + 1) + (A - 1) * c + z) / a0, -2 * A * ((A - 1) + (A + 1) * c) / a0,
                  A * ((A + 1) + (A - 1) * c - z) / a0, 2 * ((A - 1) - (A + 1) * c) / a0, ((A + 1) - (A - 1) * c - z) / a0]
            gg = mp.tan(mp.pi * f0_m / fs_m) * mp.sqrt(A); kk = 1 / q_m; m = [A * A, kk * (1 - A) * A, 1 - A * A]
        a1s = 1 / (1 + gg * (gg + kk)); a2s = gg * a1s; a3s = gg * a2s

        def Hs(wv, a1s=a1s, a2s=a2s, a3s=a3s, m=m):
            M = mp.matrix([[2 * a1s - 1, -2 * a2s], [2 * a2s, 1 - 2 * a3s]])
            B = mp.matrix([2 * a2s, 2 * a3s])
            C = mp.matrix([[m[1] * a1s + m[2] * a2s, -m[1] * a2s + m[2] * (1 - a3s)]])
            D = m[0] + m[1] * a2s + m[2] * a3s
            zz = mp.exp(1j * wv)
            return (C * mp.inverse(zz * mp.eye(2) - M) * B)[0] + D

        def Hr(wv, cr=cr):
            zi = mp.exp(-1j * wv)
            return (cr[0] + cr[1] * zi + cr[2] * zi ** 2) / (1 + cr[3] * zi + cr[4] * zi ** 2)

        dmax_exakt = 0
        for fv in np.logspace(0, math.log10(0.4999 * fs), 60):
            wv = 2 * mp.pi * mp.mpf(fv) / fs_m
            dmax_exakt = max(dmax_exakt, abs(20 * mp.log10(abs(Hs(wv)) / abs(Hr(wv)))))
        # dieselben double-Koeffizienten, exakt ausgewertet
        c_r = nk.RBJ[typ](fs, f0, q, g); c_s = nk.SVF[typ](nk.svf_grundG(f0, fs), q, g)
        dmax_dbl = 0
        for fv in np.logspace(0, math.log10(0.4999 * fs), 60):
            wv = 2 * mp.pi * mp.mpf(fv) / fs_m
            hs = Hs(wv, mp.mpf(c_s[0]), mp.mpf(c_s[1]), mp.mpf(c_s[2]), [mp.mpf(c_s[3]), mp.mpf(c_s[4]), mp.mpf(c_s[5])])
            hr = Hr(wv, [mp.mpf(v) for v in c_r])
            dmax_dbl = max(dmax_dbl, abs(20 * mp.log10(abs(hs) / abs(hr))))
        print("   %-10s %-10s exakte Entwuerfe: %s dB | double-Koeffizienten exakt ausgewertet: %.2e dB"
              % (k[0], k[1], mp.nstr(dmax_exakt, 3), float(dmax_dbl)))

    # (c) Zeitbereich
    print("\n(c) Zeitbereich (Rauschen, 2^16 Samples, beide Produkt-Ticks double): max|y_svf - y_rbj| / rms(y)")
    rng = np.random.default_rng(7)
    x = rng.standard_normal(1 << 16) * 0.1
    zeit_worst = 0; zeit_fall = None
    for typ in TYPEN:
        for fs in RATEN:
            for f0 in (20.0, 1000.0, min(20000.0, 0.45 * fs), 0.45 * fs):
                for q in (0.15, 1.0, 24.0):
                    for g in (-24.0, -12.0, 0.0, 12.0, 24.0):
                        c_r = nk.RBJ[typ](fs, f0, q, g); c_s = nk.SVF[typ](nk.svf_grundG(f0, fs), q, g)
                        yr = nk.df2t_run(*c_r, x); ys = nk.svf_run(*c_s[:6], x)
                        r = np.max(np.abs(ys - yr)) / np.sqrt(np.mean(yr ** 2))
                        if r > zeit_worst:
                            zeit_worst = r; zeit_fall = (typ, fs, f0, q, g)
    print("   schlechtester Fall: %.2e (%.1f dB unter dem Signal) bei %s" % (zeit_worst, 20 * math.log10(zeit_worst), zeit_fall))
    # Wer rauscht: gegen dd-Referenz der RBJ-Form im schlechtesten Fall
    typ, fs, f0, q, g = zeit_fall
    c_r = nk.RBJ[typ](fs, f0, q, g); c_s = nk.SVF[typ](nk.svf_grundG(f0, fs), q, g)
    yh, yl = nk.df2t_run_dd(*c_r, x)
    yr = nk.df2t_run(*c_r, x); ys = nk.svf_run(*c_s[:6], x)
    rms = np.sqrt(np.mean(yh ** 2))
    print("   dort: RBJ-DF2T(double) gegen dd-Referenz %.1f dB, SVF(double) gegen dd-Referenz %.1f dB (rel. zum Signal-RMS)"
          % (20 * math.log10(np.max(np.abs((yr - yh) - yl)) / rms), 20 * math.log10(np.max(np.abs((ys - yh) - yl)) / rms)))


if __name__ == "__main__":
    main()
