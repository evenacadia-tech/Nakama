"""Rechnung 5c - Diagnose der grossen Werte aus 5b: Pfad oder Struktur?

Je Fall und Eingangston:
  stat   = groesserer stationaerer Pegel (alt/neu) am Ton
  pfad   = max ueber die Rampe des EINGEFRORENEN Betrags |H_n(w)| des jeweiligen
           Koeffizientenpfads (DF2T-Mischung bzw. SVF-Steuerrate) - was ein
           unendlich langsamer Durchlauf desselben Pfads hoechstens liefern koennte
  ist    = tatsaechliches max|y| nach dem Wechsel
alles in dB relativ zu stat. Dazu die Lage des Maximums in ms.
Zusaetzlich: dieselbe Rampe als SVF mit Neuentwurf JEDES Sample (ohne 8er-Takt)
und der Nakama-Crossfade mit kaltem Start als Gegenprobe."""
import math
import numpy as np
import nk
from r5b_kriterienscan import rbj, svf_vec, zustand_df2t, zustand_svf, tau, N, AMP


def pfade(typ, fs, alt, neu, L, n0, phase, jedes_sample=False):
    n = np.arange(L)
    ca = np.array(rbj(typ, fs, *alt)); cn = np.array(rbj(typ, fs, *neu))
    t = np.zeros(L); t[n0:n0 + N] = np.arange(1, N + 1) / N; t[n0 + N:] = 1.0
    C = ca[:, None] + (cn - ca)[:, None] * t[None, :]
    pa = np.array([nk.svf_grundG(alt[0], fs), alt[1], alt[2]])
    pn = np.array([nk.svf_grundG(neu[0], fs), neu[1], neu[2]])
    def t_an(dd):
        return np.where(dd < n0, 0.0, np.where(dd >= n0 + N, 1.0, (dd - n0 + 1) / N))
    def entwurf(dd):
        tt = t_an(dd)
        par = pa[:, None] + (pn - pa)[:, None] * tt[None, :]
        return svf_vec(typ, par[0], par[1], par[2])
    if jedes_sample:
        S = entwurf(n)
    else:
        kk = np.floor((n - phase) / 8.0); d = phase + 8 * kk
        Dk = entwurf(d); Dk1 = entwurf(d - 8)
        S = Dk1 + (Dk - Dk1) * ((n - d) / 8.0)[None, :]
    return ca, cn, C, S


def fall(typ, fs, alt, neu, fx, phi=0.0):
    ca0 = rbj(typ, fs, *alt); cn0 = rbj(typ, fs, *neu)
    n_f = int(max(8192, 8 * max(tau(ca0), tau(cn0))))
    n0 = 64; L = n0 + N + n_f
    w = 2 * math.pi * fx / fs
    X = AMP * np.exp(1j * phi)
    x = np.imag(X * np.exp(1j * w * np.arange(L)))
    stat = AMP * max(abs(nk.biquad_H(ca0, np.array([w]))[0]), abs(nk.biquad_H(cn0, np.array([w]))[0]))
    out = {}
    for name, js in (("SVF-Takt8", False), ("SVF-jedes", True)):
        ca, cn, C, S = pfade(typ, fs, alt, neu, L, n0, 0, js)
        if name == "SVF-Takt8":
            # DF2T
            zi = np.exp(-1j * w)
            Hn = (C[0] + C[1] * zi + C[2] * zi * zi) / (1 + C[3] * zi + C[4] * zi * zi)
            z = zustand_df2t(ca, w, X)
            y, _, _ = nk.df2t_run_tv(*[C[k].copy() for k in range(5)], x, float(np.imag(z[0])), float(np.imag(z[1])))
            k = int(np.argmax(np.abs(y[n0:])))
            out["DF2T"] = (20 * math.log10(AMP * np.max(np.abs(Hn[n0:n0 + N + 16])) / stat),
                           20 * math.log10(np.max(np.abs(y[n0:])) / stat), k / fs * 1000)
        # SVF: eingefrorener Betrag entlang des Pfads (Zustandsraum je Sample, nur Rampe + 16)
        Hs = np.array([nk.svf_H(tuple(S[:, m]), np.array([w]))[0] for m in range(n0, n0 + N + 16)])
        sa = nk.SVF[typ](nk.svf_grundG(alt[0], fs), alt[1], alt[2])
        zs = zustand_svf(sa, w, X)
        y, _, _ = nk.svf_run_tv(*[S[k].copy() for k in range(6)], x, float(np.imag(zs[0])), float(np.imag(zs[1])))
        k = int(np.argmax(np.abs(y[n0:])))
        out[name] = (20 * math.log10(AMP * np.max(np.abs(Hs)) / stat), 20 * math.log10(np.max(np.abs(y[n0:])) / stat), k / fs * 1000)
    # Nakama-Crossfade, neu kalt (die alte Bank laeuft eingeschwungen weiter)
    ya_full = AMP * np.abs(nk.biquad_H(ca0, np.array([w]))[0])  # nur fuer Kontrolle
    z = zustand_df2t(np.array(ca0), w, X)
    y_alt, _, _ = nk.df2t_run_tv(*[np.full(L, v) for v in ca0], x, float(np.imag(z[0])), float(np.imag(z[1])))
    y_neu_kalt = np.zeros(L); y_neu_kalt[n0:] = nk.df2t_run(*cn0, x[n0:])
    tt = np.zeros(L); tt[n0:n0 + 256] = np.arange(256) / 256.0; tt[n0 + 256:] = 1.0
    yx = (1 - tt) * y_alt + tt * y_neu_kalt
    out["Xfade-kalt"] = (float("nan"), 20 * math.log10(np.max(np.abs(yx[n0:])) / stat), int(np.argmax(np.abs(yx[n0:]))) / fs * 1000)
    return 20 * math.log10(stat), out


def main():
    faelle = [("highshelf", 48000.0, (50.0, 24.0, 12.0), (25.0, 24.0, -8.0)),
              ("highshelf", 192000.0, (1000.0, 24.0, 12.0), (500.0, 24.0, -8.0)),
              ("lowshelf", 192000.0, (10000.0, 24.0, -8.0), (5000.0, 24.0, 12.0)),
              ("lowshelf", 48000.0, (1000.0, 24.0, -8.0), (500.0, 24.0, 12.0)),
              ("bell", 48000.0, (1000.0, 24.0, 12.0), (500.0, 24.0, -8.0)),
              ("bell", 48000.0, (1000.0, 6.0, 12.0), (500.0, 6.0, -8.0)),
              ("highshelf", 48000.0, (1000.0, 0.707, 12.0), (500.0, 0.707, -8.0)),
              ("lowshelf", 48000.0, (1000.0, 0.707, -8.0), (500.0, 0.707, 12.0))]
    print("dB relativ zum groesseren stationaeren Pegel am Ton: Pfad (eingefroren, max) / ist (max|y|) @ ms")
    for typ, fs, alt, neu in faelle:
        print("\n%s fs=%g  %g Hz Q%g %+g dB -> %g Hz Q%g %+g dB" % (typ, fs, *alt, *neu))
        for fx in (alt[0], neu[0], math.sqrt(alt[0] * neu[0])):
            worst = {}
            for phi in (0.0, 0.5 * math.pi, math.pi, 1.5 * math.pi):
                s, out = fall(typ, fs, alt, neu, fx, phi)
                for k, v in out.items():
                    if k not in worst or v[1] > worst[k][1]:
                        worst[k] = v
            print("  Ton %7.1f Hz (stat %+6.1f dBFS): " % (fx, s)
                  + " | ".join("%s Pfad %+5.1f / ist %+5.1f @%5.1f ms" % (k, v[0], v[1], v[2]) for k, v in worst.items()))


if __name__ == "__main__":
    main()
