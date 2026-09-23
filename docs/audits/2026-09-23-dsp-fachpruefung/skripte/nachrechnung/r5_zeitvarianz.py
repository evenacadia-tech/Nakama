"""Rechnung 5 - Zeitvarianz: Koeffizientenwechsel DF2T-Biquad gegen Simper-SVF.

Nakama-Verfahren (DspKern.cpp:533-647, :759-905, DspProgramm.h:87-168):
  * Statisches Band (RBJ-DF2T): Wechsel mit gleicher Topologie und innerhalb der
    Wertekriterien (Frequenz <= Faktor 2, Q <= Faktor 4, Gain <= 20 dB) = "Rampe":
    256 Samples lineare Mischung der fuenf Koeffizienten, t = 1/256 .. 1, Zustand
    wandert mit. Sonst "Crossfade": alte Bank laeuft weiter, der geaenderte Slot
    startet in der neuen Bank KALT (Zustand 0), linear gemischt t = 0 .. 255/256.
  * Dynamisches Band (SVF): alle 8 Samples Neuentwurf (in einer Rampe mit linear
    gemischten grundG = tan(pi f/fs), q, gainDb), dazwischen lineare Mischung
    der sechs SVF-Koeffizienten; der neue Satz wirkt am Entwurfssample mit
    Gewicht 0, acht Samples spaeter voll. Integratorzustaende wandern mit.

Szenarien (fs = 48 kHz):
  A  Bell 1 kHz +12 dB Q 6 -> 3 kHz -12 dB Q 6  (Faktor 3 und 24 dB: im Produkt Crossfade, kalt)
  A' dasselbe rueckwaerts
  B  Bell 1 kHz +12 dB Q 6 -> 2 kHz -6 dB Q 6   (genau auf dem Kriterium: im Produkt Rampe 256)
  C  Bell 1 kHz Q 6, 0 -> -12 dB in einem Steuerschritt (dynamisches Band, 8 Samples) und zurueck
Referenzen:
  R1 "ideales Umschalten": Ueberblendung der beiden EINGESCHWUNGENEN Filter mit
     demselben t-Verlauf wie das Verfahren (Sprung = hartes Umschalten).
  R2 "eingefroren": je Sample das eingeschwungene LTI-Filter mit den
     Koeffizienten dieses Samples (misst nur das Gedaechtnis der Struktur).
Metriken ueber [n0, n0+8192): Spitze und RMS des Residuums in dBFS
(Sinus -6 dBFS = Amplitude 0,5; Rauschen -20 dBFS RMS), Sinus: schlechteste von
16 Umschaltphasen; Rauschen: schlechtester von 4 Seeds; Ueberschwingen =
max|y| ueber max der beiden stationaeren Spitzen."""
import math
import numpy as np
from numba import njit
import nk

FS = 48000.0
N0 = 24000
FENSTER = 8192
L = N0 + FENSTER + 600
HIST = 9000


def db(x):
    return 20 * math.log10(max(x, 1e-300))


# ------------------------------------------------------------------ Koeffizientenverlaeufe
def verlauf_df2t(c_alt, c_neu, N, n0=N0, n=L):
    C = np.empty((5, n))
    for k in range(5):
        C[k, :] = c_alt[k]
        C[k, n0:] = c_neu[k]
        if N > 1:
            t = np.arange(1, N + 1) / N          # erstes Sample 1/N, letztes 1 (DspKern.cpp:795)
            C[k, n0:n0 + N] = c_alt[k] + (c_neu[k] - c_alt[k]) * t
    return C


def svf_aus_gk(g, k, m0, m1, m2):
    a1 = 1.0 / (1.0 + g * (g + k)); a2 = g * a1; a3 = g * a2
    return a1, a2, a3, m0, m1, m2


def verlauf_svf_gk(s_alt, s_neu, N, n0=N0, n=L):
    """Sprung (N<=1) oder g, k, m0..m2 linear ueber N Samples, a1..a3 je Sample aus g,k."""
    C = np.empty((6, n))
    for i in range(n):
        if i < n0:
            t = 0.0
        elif N <= 1 or i >= n0 + N:
            t = 1.0
        else:
            t = (i - n0 + 1) / N
        g = s_alt[6] + (s_neu[6] - s_alt[6]) * t
        kk = s_alt[7] + (s_neu[7] - s_alt[7]) * t
        m = [s_alt[3 + j] + (s_neu[3 + j] - s_alt[3 + j]) * t for j in range(3)]
        C[:, i] = svf_aus_gk(g, kk, *m)
    return C


def verlauf_svf_produkt(p_alt, p_neu, N, phase, n0=N0, n=L, typ="bell"):
    """Nakama-Steuerrate: Parameter (grundG, q, gainDb) je Entwurf mit dem t des
    Entwurfssamples linear gemischt (DspKern.cpp:873-889), Entwurf alle 8
    Samples, Koeffizienten dazwischen linear (DspKern.cpp:893-895).
    N = Rampenlaenge (1 = Sprung der Parameter). phase = Lage der Entwuerfe."""
    C = np.empty((6, n))
    von = nk.SVF[typ](*p_alt)[:6]
    nach = von
    rest = phase  # Samples bis zum naechsten Entwurf
    for i in range(n):
        if rest <= 0:
            if i < n0:
                t = 0.0
            elif N <= 1 or i >= n0 + N:
                t = 1.0
            else:
                t = 1.0 - (n0 + N - i - 1) / N
            par = [p_alt[j] + (p_neu[j] - p_alt[j]) * t for j in range(3)]
            von = nach
            nach = nk.SVF[typ](*par)[:6]
            rest = 8
        ts = 1.0 - rest / 8.0
        C[:, i] = [von[j] + (nach[j] - von[j]) * ts for j in range(6)]
        rest -= 1
    return C


# ------------------------------------------------------------------ Referenzen
@njit(cache=True)
def eingefroren_df2t(C, x, i0, i1, hist):
    out = np.empty(i1 - i0)
    for n in range(i0, i1):
        b0 = C[0, n]; b1 = C[1, n]; b2 = C[2, n]; a1 = C[3, n]; a2 = C[4, n]
        z1 = 0.0; z2 = 0.0; y = 0.0
        for m in range(n - hist, n + 1):
            xi = x[m]
            y = b0 * xi + z1
            z1 = b1 * xi - a1 * y + z2
            z2 = b2 * xi - a2 * y
        out[n - i0] = y
    return out


@njit(cache=True)
def eingefroren_svf(C, x, i0, i1, hist):
    out = np.empty(i1 - i0)
    for n in range(i0, i1):
        a1 = C[0, n]; a2 = C[1, n]; a3 = C[2, n]; m0 = C[3, n]; m1 = C[4, n]; m2 = C[5, n]
        c1 = 0.0; c2 = 0.0; y = 0.0
        for m in range(n - hist, n + 1):
            xi = x[m]
            v3 = xi - c2
            v1 = a1 * c1 + a2 * v3
            v2 = c2 + a2 * c1 + a3 * v3
            c1 = 2.0 * v1 - c1
            c2 = 2.0 * v2 - c2
            y = m0 * xi + m1 * v1 + m2 * v2
        out[n - i0] = y
    return out


def run_df2t(C, x):
    y, _, _ = nk.df2t_run_tv(C[0], C[1], C[2], C[3], C[4], x, 0.0, 0.0)
    return y


def run_svf(C, x):
    y, _, _ = nk.svf_run_tv(C[0], C[1], C[2], C[3], C[4], C[5], x, 0.0, 0.0)
    return y


def frozen_sinus(C, art, w, amp, phi, i0, i1):
    """R2 fuer Sinus analytisch: Amplitude*Im(H_n e^{j(w n + phi)})."""
    n = np.arange(i0, i1)
    if art == "df2t":
        zi = np.exp(-1j * w)
        H = (C[0, i0:i1] + C[1, i0:i1] * zi + C[2, i0:i1] * zi * zi) / (1 + C[3, i0:i1] * zi + C[4, i0:i1] * zi * zi)
    else:
        H = np.array([nk.svf_H(tuple(C[:, m]), np.array([w]))[0] for m in range(i0, i1)])
    return amp * np.imag(H * np.exp(1j * (w * n + phi)))


# ------------------------------------------------------------------ ein Fall
def fall(x, art, C, t_ref, y_alt, y_neu, sinus=None, kalt=None):
    """Liefert Spitze/RMS gegen R1 und R2 sowie max|y| im Fenster."""
    i0, i1 = N0, N0 + FENSTER
    if kalt is not None:          # Produkt-Crossfade: neue Bank kalt ab n0
        y = kalt
    elif art == "df2t":
        y = run_df2t(C, x)
    else:
        y = run_svf(C, x)
    r1 = y[i0:i1] - ((1 - t_ref[i0:i1]) * y_alt[i0:i1] + t_ref[i0:i1] * y_neu[i0:i1])
    if C is None:
        r2 = r1
    else:
        # R2: im Uebergang eingefroren, danach = neues Filter eingeschwungen
        nT = int(np.max(np.nonzero(np.any(np.abs(np.diff(C, axis=1)) > 0, axis=0))[0])) + 2 if np.any(np.abs(np.diff(C, axis=1)) > 0) else N0 + 1
        nT = max(nT, N0 + 1)
        ref = y_neu[i0:i1].copy()
        if sinus is not None:
            w, amp, phi = sinus
            ref[:nT - i0] = frozen_sinus(C, art, w, amp, phi, i0, nT)
        else:
            f = eingefroren_df2t if art == "df2t" else eingefroren_svf
            ref[:nT - i0] = f(C, x, i0, nT, HIST)
        r2 = y[i0:i1] - ref
    return (np.max(np.abs(r1)), np.sqrt(np.mean(r1 ** 2)), np.max(np.abs(r2)), np.sqrt(np.mean(r2 ** 2)),
            np.max(np.abs(y[i0:i1 - 4096])))


def szenario(name, par_alt, par_neu, methoden, eingaenge):
    """par = (f0, q, gainDb)."""
    c_alt = nk.rbj_bell(FS, *par_alt); c_neu = nk.rbj_bell(FS, *par_neu)
    sa = nk.svf_bell(nk.svf_grundG(par_alt[0], FS), par_alt[1], par_alt[2])
    sn = nk.svf_bell(nk.svf_grundG(par_neu[0], FS), par_neu[1], par_neu[2])
    p_alt = (nk.svf_grundG(par_alt[0], FS), par_alt[1], par_alt[2])
    p_neu = (nk.svf_grundG(par_neu[0], FS), par_neu[1], par_neu[2])
    print("\n=== Szenario %s: Bell %g Hz %+g dB Q %g -> %g Hz %+g dB Q %g" % (name, par_alt[0], par_alt[2], par_alt[1],
                                                                          par_neu[0], par_neu[2], par_neu[1]))
    kopf = "%-34s" % "Verfahren"
    for en in eingaenge:
        kopf += " | %-40s" % (en + ": R1 Spitze/RMS  R2 Spitze  Ueberschw.")
    print(kopf)
    for mname, art, N in methoden:
        zeile = "%-34s" % mname
        for en in eingaenge:
            worst = None
            if en.startswith("Sinus"):
                fq = float(en.split()[1].replace("kHz", "")) * 1000.0
                w = 2 * np.pi * fq / FS
                per = FS / fq
                laeufe = [("sin", w, 2 * np.pi * k / 16.0) for k in range(16)]
            else:
                laeufe = [("rausch", s, None) for s in range(4)]
            for lauf in laeufe:
                if lauf[0] == "sin":
                    _, w, phi = lauf
                    x = 0.5 * np.sin(w * np.arange(L) + phi)
                    sinus = (w, 0.5, phi)
                else:
                    x = np.random.default_rng(100 + lauf[1]).standard_normal(L) * 0.1
                    sinus = None
                y_alt = nk.df2t_run(*c_alt, x); y_neu = nk.df2t_run(*c_neu, x)
                t_ref = np.zeros(L); t_ref[N0:] = 1.0
                kalt = None; C = None
                if art == "xfade_kalt":
                    t = np.arange(N) / N                      # t = 1 - rest/256 (DspKern.cpp:1120)
                    t_ref[N0:N0 + N] = t
                    neu_kalt = np.zeros(L); neu_kalt[N0:] = nk.df2t_run(*c_neu, x[N0:])
                    kalt = y_alt.copy(); kalt[N0:] = neu_kalt[N0:]
                    kalt[N0:N0 + N] = (1 - t) * y_alt[N0:N0 + N] + t * neu_kalt[N0:N0 + N]
                elif art == "df2t":
                    C = verlauf_df2t(c_alt, c_neu, N)
                    if N > 1:
                        t_ref[N0:N0 + N] = np.arange(1, N + 1) / N
                elif art == "svf_gk":
                    C = verlauf_svf_gk(sa, sn, N)
                    if N > 1:
                        t_ref[N0:N0 + N] = np.arange(1, N + 1) / N
                    art_eff = "svf"
                elif art.startswith("svf_produkt"):
                    pass
                if art.startswith("svf_produkt"):
                    # schlechteste der 8 Steuerphasen
                    best = None
                    for ph in range(8):
                        Cp = verlauf_svf_produkt(p_alt, p_neu, N, ph)
                        tr = np.zeros(L); tr[N0:] = 1.0
                        if N > 1:
                            tr[N0:N0 + N] = np.arange(1, N + 1) / N
                        r = fall(x, "svf", Cp, tr, y_alt, y_neu, sinus)
                        best = r if best is None else tuple(max(a, b) for a, b in zip(best, r))
                    r = best
                else:
                    r = fall(x, "svf" if art == "svf_gk" else "df2t", C, t_ref, y_alt, y_neu, sinus, kalt)
                worst = r if worst is None else tuple(max(a, b) for a, b in zip(worst, r))
            if en.startswith("Sinus"):
                stat = 0.5 * max(abs(nk.biquad_H(c_alt, np.array([w]))[0]), abs(nk.biquad_H(c_neu, np.array([w]))[0]))
                ueb = "%+5.1f dB" % db(worst[4] / stat)
            else:
                ueb = "   -    "
            zeile += " | %6.1f/%6.1f  %6.1f  %s          " % (db(worst[0]), db(worst[1]), db(worst[2]), ueb)
        print(zeile)


def main():
    meth_A = [("DF2T Sprung (Zustand bleibt)", "df2t", 1),
              ("DF2T linear 64", "df2t", 64),
              ("DF2T linear 256 (=Nakama-Rampe)", "df2t", 256),
              ("DF2T linear 512", "df2t", 512),
              ("SVF Sprung", "svf_gk", 1),
              ("SVF g,k,m linear 64", "svf_gk", 64),
              ("SVF g,k,m linear 256", "svf_gk", 256),
              ("SVF g,k,m linear 512", "svf_gk", 512),
              ("SVF Nakama-Steuerrate, Rampe 256", "svf_produkt", 256),
              ("Nakama-Crossfade 256, neu kalt", "xfade_kalt", 256)]
    ein = ["Sinus 1kHz", "Sinus 2kHz", "Sinus 3kHz", "Rauschen"]
    szenario("A", (1000.0, 6.0, 12.0), (3000.0, 6.0, -12.0), meth_A, ein)
    szenario("A'", (3000.0, 6.0, -12.0), (1000.0, 6.0, 12.0), meth_A, ein)
    szenario("B", (1000.0, 6.0, 12.0), (2000.0, 6.0, -6.0), meth_A, ["Sinus 1kHz", "Sinus 2kHz", "Rauschen"])
    meth_C = [("DF2T Sprung", "df2t", 1),
              ("DF2T linear 8", "df2t", 8),
              ("SVF Sprung", "svf_gk", 1),
              ("SVF g,k,m linear 8", "svf_gk", 8),
              ("SVF Nakama-Steuerrate (8)", "svf_produkt", 1)]
    szenario("C", (1000.0, 6.0, 0.0), (1000.0, 6.0, -12.0), meth_C, ["Sinus 1kHz", "Rauschen"])
    szenario("C'", (1000.0, 6.0, -12.0), (1000.0, 6.0, 0.0), meth_C, ["Sinus 1kHz", "Rauschen"])


if __name__ == "__main__":
    main()
