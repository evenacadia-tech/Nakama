"""Rechnung 5b - Ueberschwingen der Nakama-Rampe INNERHALB der Wertekriterien
(DspProgramm.h:144-168: Frequenz <= Faktor 2, Q <= Faktor 4, Gain <= 20 dB).

Vergleich (beide wie im Produkt):
  DF2T-Rampe: RBJ-Koeffizienten 256 Samples linear gemischt, Zustand wandert mit.
  SVF-Rampe : Entwurf alle 8 Samples mit linear gemischten (grundG, q, gainDb),
              Koeffizienten dazwischen linear; schlechtere von 2 Steuerphasen.
Eingang: Sinus -6 dBFS bei f_alt, f_neu und sqrt(f_alt f_neu), je 4 Phasen.
Beide Filter starten ANALYTISCH eingeschwungen (Zustandsraum-Loesung fuer die
komplexe Exponentielle) kurz vor dem Wechsel - kein langer Vorlauf noetig.
Mass: Ueberschwingen = max|y| nach dem Wechsel ueber max(stationaere Spitze alt,
neu) in dB. Fenster: Rampe + max(8192, 8 tau) Samples."""
import math
import itertools
import numpy as np
import nk

N = 256
AMP = 0.5


def rbj(typ, fs, f, q, g):
    return nk.RBJ[typ](fs, f, q, g)


def svf_vec(typ, gG, q, gdb):
    A = np.power(10.0, gdb / 40.0)
    one = np.ones_like(A)
    if typ == "bell":
        g = gG; k = 1.0 / (q * A); m0 = one; m1 = k * (A * A - 1.0); m2 = 0.0 * one
    elif typ == "lowshelf":
        g = gG / np.sqrt(A); k = 1.0 / q; m0 = one; m1 = k * (A - 1.0); m2 = A * A - 1.0
    else:
        g = gG * np.sqrt(A); k = 1.0 / q; m0 = A * A; m1 = k * (1.0 - A) * A; m2 = 1.0 - A * A
    a1 = 1.0 / (1.0 + g * (g + k)); a2 = g * a1; a3 = g * a2
    return np.vstack([a1, a2, a3, m0, m1, m2])


def zustand_df2t(c, w, X):
    b0, b1, b2, a1, a2 = c
    M = np.array([[-a1, 1.0], [-a2, 0.0]]); B = np.array([b1 - a1 * b0, b2 - a2 * b0])
    S = np.linalg.solve(np.exp(1j * w) * np.eye(2) - M, B)
    return S * X


def zustand_svf(c, w, X):
    Mm, B, C, D = nk.svf_statespace(c)
    S = np.linalg.solve(np.exp(1j * w) * np.eye(2) - Mm, B)
    return S * X


def tau(c):
    p = np.roots([1.0, c[3], c[4]])
    return -1.0 / math.log(float(np.max(np.abs(p))))


def lauf(typ, fs, alt, neu, w, phi, n_fenster, phase):
    """alt/neu = (f, q, g). Liefert (max|y| DF2T-Rampe, max|y| SVF-Rampe)."""
    s0 = 64                      # Wechsel bei n0 = s0 (relativ), vorher 64 Samples stationaer
    n0 = s0
    L = n0 + N + n_fenster
    n = np.arange(L)
    X = AMP * np.exp(1j * phi)
    x = np.imag(X * np.exp(1j * w * n))
    # DF2T-Rampe
    ca = np.array(rbj(typ, fs, *alt)); cn = np.array(rbj(typ, fs, *neu))
    t = np.zeros(L); t[n0:n0 + N] = np.arange(1, N + 1) / N; t[n0 + N:] = 1.0
    C = ca[:, None] + (cn - ca)[:, None] * t[None, :]
    z = zustand_df2t(ca, w, X)                 # Zustand vor Sample 0 (komplex)
    y1, _, _ = nk.df2t_run_tv(C[0].copy(), C[1].copy(), C[2].copy(), C[3].copy(), C[4].copy(), x,
                              float(np.imag(z[0])), float(np.imag(z[1])))
    # SVF-Rampe mit Steuerrate 8
    pa = np.array([nk.svf_grundG(alt[0], fs), alt[1], alt[2]])
    pn = np.array([nk.svf_grundG(neu[0], fs), neu[1], neu[2]])
    kk = np.floor((n - phase) / 8.0)
    d = phase + 8 * kk                          # Entwurfssample dieses Samples
    def t_an(dd):
        tt = np.where(dd < n0, 0.0, np.where(dd >= n0 + N, 1.0, (dd - n0 + 1) / N))
        return tt
    def entwurf(dd):
        tt = t_an(dd)
        par = pa[:, None] + (pn - pa)[:, None] * tt[None, :]
        return svf_vec(typ, par[0], par[1], par[2])
    Dk = entwurf(d); Dk1 = entwurf(d - 8)
    frac = (n - d) / 8.0
    S = Dk1 + (Dk - Dk1) * frac[None, :]
    sa = nk.SVF[typ](pa[0], pa[1], pa[2])
    zs = zustand_svf(sa, w, X)
    y2, _, _ = nk.svf_run_tv(S[0].copy(), S[1].copy(), S[2].copy(), S[3].copy(), S[4].copy(), S[5].copy(), x,
                             float(np.imag(zs[0])), float(np.imag(zs[1])))
    # Plausibilitaet: vor dem Wechsel muessen beide stationaer sein
    ya = AMP * abs(nk.biquad_H(tuple(ca), np.array([w]))[0])
    pre = max(np.max(np.abs(y1[:n0])), np.max(np.abs(y2[:n0])))
    assert pre <= ya * (1 + 1e-6) + 1e-12, (pre, ya)
    # Nakama-Crossfade (alte Bank eingeschwungen weiter, neue Bank kalt ab n0)
    y_alt, _, _ = nk.df2t_run_tv(*[np.full(L, v) for v in ca], x, float(np.imag(z[0])), float(np.imag(z[1])))
    y_nk = np.zeros(L); y_nk[n0:] = nk.df2t_run(*cn, x[n0:])
    tt = np.zeros(L); tt[n0:n0 + N] = np.arange(N) / N; tt[n0 + N:] = 1.0
    y3 = (1 - tt) * y_alt + tt * y_nk
    return np.max(np.abs(y1[n0:])), np.max(np.abs(y2[n0:])), np.max(np.abs(y3[n0:]))


def main():
    import csv
    typen = ["bell", "lowshelf", "highshelf"]
    raten = [48000.0, 192000.0]
    f_alt_liste = [50.0, 200.0, 1000.0, 5000.0, 10000.0]
    q_paare = [(0.707, 0.707), (6.0, 6.0), (24.0, 24.0), (6.0, 24.0), (24.0, 6.0)]
    g_paare = [(12.0, 12.0), (-12.0, -12.0), (12.0, -8.0), (-8.0, 12.0)]
    zeilen = []
    for typ, fs, fa, richtung, (qa, qn), (ga, gn) in itertools.product(typen, raten, f_alt_liste, ("ab", "auf"), q_paare, g_paare):
        fn = fa / 2.0 if richtung == "ab" else fa * 2.0
        if max(fa, fn) > min(20000.0, 0.45 * fs) or min(fa, fn) < 20.0:
            continue
        alt = (fa, qa, ga); neu = (fn, qn, gn)
        ca = rbj(typ, fs, *alt); cn = rbj(typ, fs, *neu)
        n_f = int(max(8192, 8 * max(tau(ca), tau(cn))))
        fd = np.logspace(math.log10(5.0), math.log10(0.4999 * fs), 20000)
        hmax = AMP * max(np.max(np.abs(nk.biquad_H(ca, 2 * np.pi * fd / fs))), np.max(np.abs(nk.biquad_H(cn, 2 * np.pi * fd / fs))))
        rel = [-1e9] * 3; absol = [-1e9] * 3
        for fx in (fa, fn, math.sqrt(fa * fn)):
            w = 2 * math.pi * fx / fs
            stat = AMP * max(abs(nk.biquad_H(ca, np.array([w]))[0]), abs(nk.biquad_H(cn, np.array([w]))[0]))
            for phi in (0.0, 0.5 * math.pi, math.pi, 1.5 * math.pi):
                m = [-1e9] * 3
                for ph in (0, 4):
                    md, ms, mx = lauf(typ, fs, alt, neu, w, phi, n_f, ph)
                    m = [md, max(m[1], ms), mx]
                for k in range(3):
                    rel[k] = max(rel[k], 20 * math.log10(m[k] / stat))
                    absol[k] = max(absol[k], 20 * math.log10(m[k] / hmax))
        zeilen.append(dict(typ=typ, fs=fs, fa=fa, fn=fn, qa=qa, qn=qn, ga=ga, gn=gn,
                           df2t_rel=rel[0], svf_rel=rel[1], xf_rel=rel[2],
                           df2t_abs=absol[0], svf_abs=absol[1], xf_abs=absol[2]))
    with open("r5b_faelle.csv", "w", newline="") as fh:
        wtr = csv.DictWriter(fh, fieldnames=list(zeilen[0].keys())); wtr.writeheader(); wtr.writerows(zeilen)
    print("Faelle: %d (Rampe 256 innerhalb der Kriterien, Faktor 2 auf/ab, 48/192 kHz, Sinus -6 dBFS)" % len(zeilen))
    print("rel = max|y| ueber groesserem stationaerem Pegel am Ton; abs = max|y| ueber der hoechsten stationaeren Spitze beider Filter (alle Frequenzen)")
    print("%-9s %-13s | %-26s | %-26s | %-26s" % ("Typ", "Q-Paar", "DF2T-Rampe rel / abs", "SVF-Rampe rel / abs", "Xfade kalt rel / abs"))
    for typ in typen:
        for qp in (["0.707"], ["6", "24"]):
            sel = [z for z in zeilen if z["typ"] == typ and ((z["qa"] == 0.707) == (qp == ["0.707"]))]
            print("%-9s %-13s | %+6.1f / %+6.1f dB           | %+6.1f / %+6.1f dB           | %+6.1f / %+6.1f dB"
                  % (typ, "Q 0,707" if qp == ["0.707"] else "Q 6..24", max(z["df2t_rel"] for z in sel), max(z["df2t_abs"] for z in sel),
                     max(z["svf_rel"] for z in sel), max(z["svf_abs"] for z in sel), max(z["xf_rel"] for z in sel), max(z["xf_abs"] for z in sel)))
    for typ in typen:
        sel = [z for z in zeilen if z["typ"] == typ]
        print("  %-9s Faelle mit rel > +1 dB: DF2T %d, SVF %d, Xfade %d von %d | abs > 0 dB: DF2T %d, SVF %d, Xfade %d"
              % (typ, sum(z["df2t_rel"] > 1 for z in sel), sum(z["svf_rel"] > 1 for z in sel), sum(z["xf_rel"] > 1 for z in sel), len(sel),
                 sum(z["df2t_abs"] > 0 for z in sel), sum(z["svf_abs"] > 0 for z in sel), sum(z["xf_abs"] > 0 for z in sel)))
    zeilen.sort(key=lambda z: -max(z["df2t_abs"], z["svf_abs"]))
    print("Schlechteste 8 nach abs (ueber der hoechsten stationaeren Spitze):")
    for z in zeilen[:8]:
        print("  %(typ)-9s fs=%(fs)-6g %(fa)6g->%(fn)-6g Hz Q %(qa)g->%(qn)g %(ga)+g->%(gn)+g dB: DF2T %(df2t_abs)+5.1f | SVF %(svf_abs)+5.1f | Xfade %(xf_abs)+5.1f" % z)


if __name__ == "__main__":
    main()
