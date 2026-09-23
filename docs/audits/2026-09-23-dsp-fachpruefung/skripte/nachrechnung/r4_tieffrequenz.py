"""Rechnung 4 - numerische Robustheit tiefer Frequenzen, fs = 192 kHz, f0 = 20 Hz.

Filter: RBJ-Bell +-12 dB mit Q 24 und Q 0,15, low_cut (RBJ-HPF) Q 24 und Q 0,15.
Je Filter:
  - Pole: Betrag r, 1 - r; Rauschverstaerkung der Rueckkopplung 1/A(z).
  - Koeffizientenempfindlichkeit: (i) a1 um 1 ulp (double) verschoben,
    (ii) alle fuenf Koeffizienten auf float32 gerundet -> max|dB-Abweichung|
    des Frequenzgangs 2 Hz..20 kHz (analytisch).
  - Simulation DF2T in float64 und float32 (bitgenau wie DspFilter.h, float32 =
    jede Operation in float32): Impulsantwort 2^23 Samples -> FFT -> Betrag gegen
    Sollgang; Sinus 20 Hz und 1 kHz (-6 dBFS) und weisses Rauschen (-20 dBFS RMS)
    gegen eine double-double-Referenz derselben Rekursion.
  - Zum Vergleich der Struktur dieselben Bells als SVF (DspSvf.h) in float64/float32."""
import math
import numpy as np
import nk

FS = 192000.0
N_IMP = 1 << 23
N_SIG = 1 << 22


def filter_liste():
    L = []
    for q in (24.0, 0.15):
        for g in (12.0, -12.0):
            L.append(("Bell 20 Hz Q%g %+g dB" % (q, g), nk.rbj_bell(FS, 20.0, q, g),
                      nk.svf_bell(nk.svf_grundG(20.0, FS), q, g)))
    for q in (24.0, 0.15):
        L.append(("low_cut 20 Hz Q%g" % q, nk.rbj_lowcut(FS, 20.0, q), None))
    return L


def fgang_db(c, f):
    return nk.db(nk.biquad_H(c, 2 * np.pi * f / FS))


def rms_db(e):
    return 20 * math.log10(max(np.sqrt(np.mean(e ** 2)), 1e-300))


def peak_db(e):
    return 20 * math.log10(max(np.max(np.abs(e)), 1e-300))


def main():
    f_band = np.logspace(math.log10(2.0), math.log10(20000.0), 4000)
    t = np.arange(N_SIG)
    sig = {"Sinus 20 Hz": 0.5 * np.sin(2 * np.pi * 20.0 * t / FS),
           "Sinus 1 kHz": 0.5 * np.sin(2 * np.pi * 1000.0 * t / FS),
           "Rauschen": np.random.default_rng(11).standard_normal(N_SIG) * 0.1}
    imp = np.zeros(N_IMP); imp[0] = 1.0
    fbin = np.fft.rfftfreq(N_IMP, 1.0 / FS)
    sel = (fbin >= 2.0) & (fbin <= 20000.0)

    for name, c, cs in filter_liste():
        print("\n=== %s  (fs = 192 kHz)" % name)
        b0, b1, b2, a1, a2 = c
        p = np.roots([1.0, a1, a2])
        r = float(np.max(np.abs(p)))
        hA = nk.df2t_run(1.0, 0.0, 0.0, a1, a2, imp)
        ng = float(np.sum(hA ** 2))
        print("  Koeffizienten: b = %.17g %.17g %.17g ; a1 = %.17g a2 = %.17g" % (b0, b1, b2, a1, a2))
        print("  Pole: %s, max|p| = %.10f, 1-r = %.3e; Rauschverstaerkung 1/A(z): %.1f dB"
              % ("komplex" if abs(p[0].imag) > 0 else "reell", r, 1 - r, 10 * math.log10(ng)))
        # Empfindlichkeit
        soll = fgang_db(c, f_band)
        c_ulp = (b0, b1, b2, float(np.nextafter(a1, np.inf)), a2)
        d_ulp = np.max(np.abs(fgang_db(c_ulp, f_band) - soll))
        c32 = tuple(np.float32(v) for v in c)
        c32d = tuple(float(v) for v in c32)
        d32 = fgang_db(c32d, f_band) - soll
        i32 = int(np.argmax(np.abs(d32)))
        p32 = np.roots([1.0, c32d[3], c32d[4]])
        s_dc = (b0 + b1 + b2) / (1 + a1 + a2)
        s_dc32 = (c32d[0] + c32d[1] + c32d[2]) / (1 + c32d[3] + c32d[4])
        print("  Empfindlichkeit: a1 +1 ulp (double) -> max|dB| %.1e;  float32-Koeffizienten -> max|dB| %.2f bei %.1f Hz"
              % (d_ulp, d32[i32], f_band[i32]))
        ldb = lambda v: ("%.3f" % (20 * math.log10(abs(v)))) if v != 0 else "-inf (exakt 0)"
        print("    |H(DC)| double %s dB, float32-Koeff. %s dB; Pole float32 max|p| = %.8f (double %.8f)"
              % (ldb(s_dc), ldb(s_dc32), float(np.max(np.abs(p32))), r))
        print("    1+a1+a2 = %.4e; relative Aufloesung von a1 in double %.1e, in float32 %.1e"
              % (1 + a1 + a2, (np.spacing(abs(a1))) / abs(1 + a1 + a2),
                 float(np.spacing(np.float32(abs(a1)))) / abs(1 + a1 + a2)))
        # Impulsantwort -> FFT
        for tag, h in (("DF2T float64", nk.df2t_run(*c, imp)),
                       ("DF2T float32", nk.df2t_run(*c32, imp.astype(np.float32)).astype(np.float64))):
            H = np.fft.rfft(h)[sel]
            dev = nk.db(H) - fgang_db(c, fbin[sel])
            j = int(np.argmax(np.abs(dev)))
            k20 = int(np.argmin(np.abs(fbin[sel] - 20.0)))
            print("  %s Impulsantwort+FFT gegen Sollgang: max|dB| %.2e bei %.2f Hz, bei 20 Hz %.2e dB, Rest am Ende %.1e"
                  % (tag, dev[j], fbin[sel][j], dev[k20], np.max(np.abs(h[-1000:]))))
        # Signale gegen dd-Referenz
        for sname, x in sig.items():
            x32 = x.astype(np.float32)
            yh, yl = nk.df2t_run_dd(*c, x)
            e64 = (nk.df2t_run(*c, x) - yh) - yl
            y32 = nk.df2t_run(*c32, x32).astype(np.float64)
            e32_tot = (y32 - yh) - yl
            yh2, yl2 = nk.df2t_run_dd(*c32d, x32.astype(np.float64))
            e32_arith = (y32 - yh2) - yl2
            hl = slice(N_SIG // 2, None)
            line = "  %-11s DF2T: float64-Fehler %7.1f dBFS RMS (%6.1f Spitze) | float32 gesamt %6.1f dBFS RMS (%6.1f Spitze), davon Rechenrauschen %6.1f" % (
                sname, rms_db(e64[hl]), peak_db(e64[hl]), rms_db(e32_tot[hl]), peak_db(e32_tot[hl]), rms_db(e32_arith[hl]))
            print(line)
            if sname.startswith("Sinus"):
                f = 20.0 if "20 Hz" in sname else 1000.0
                w = 2 * np.pi * f / FS
                M = np.c_[np.sin(w * t[hl]), np.cos(w * t[hl])]
                a64, *_ = np.linalg.lstsq(M, nk.df2t_run(*c, x)[hl], rcond=None)
                a32, *_ = np.linalg.lstsq(M, y32[hl], rcond=None)
                soll_f = float(fgang_db(c, np.array([f]))[0])
                print("    gemessener Gain bei %g Hz: Soll %.4f dB | float64 %+.2e dB | float32 %+.3f dB"
                      % (f, soll_f, 20 * math.log10(np.hypot(*a64) / 0.5) - soll_f, 20 * math.log10(np.hypot(*a32) / 0.5) - soll_f))
            if cs is not None:
                cs6 = cs[:6]
                cs32 = tuple(np.float32(v) for v in cs6)
                sh, sl = nk.svf_run_dd(*cs6, x)
                es64 = (nk.svf_run(*cs6, x) - sh) - sl
                ys32 = nk.svf_run32(*cs32, x32).astype(np.float64)
                es32 = (ys32 - sh) - sl
                sh2, sl2 = nk.svf_run_dd(*tuple(float(v) for v in cs32), x32.astype(np.float64))
                es32a = (ys32 - sh2) - sl2
                print("  %-11s SVF : float64-Fehler %7.1f dBFS RMS (%6.1f Spitze) | float32 gesamt %6.1f dBFS RMS (%6.1f Spitze), davon Rechenrauschen %6.1f" % (
                    sname, rms_db(es64[hl]), peak_db(es64[hl]), rms_db(es32[hl]), peak_db(es32[hl]), rms_db(es32a[hl])))
        if cs is not None:
            cs32d = tuple(float(np.float32(v)) for v in cs[:6])
            d = nk.db(nk.svf_H(cs32d, 2 * np.pi * f_band / FS)) - soll
            print("  SVF float32-Koeffizienten -> max|dB| %.3f (Vergleich RBJ %.2f)" % (np.max(np.abs(d)), np.max(np.abs(d32))))


if __name__ == "__main__":
    main()
