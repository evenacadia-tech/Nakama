"""Rechnung 2 - Decramping-Vergleich fuer dieselben Bell-Faelle wie Rechnung 1.

Quellen (im Ordner abgelegt, gelesen):
  Vicanek, "Matched Second Order Digital Filters", 14.02.2016
    (vicanek.de/articles/BiquadFits.pdf) - Gl. (12) Pole per Impulsinvarianz,
    Gl. (25)-(29) Betragsdarstellung und Rueckrechnung der b,
    Abschnitt 4.4 Gl. (42)-(45) "Matched Peaking EQ".
  Orfanidis, "Digital Parametric Equalizer Design With Prescribed
    Nyquist-Frequency Gain", JAES 45(6) 1997, Autorenfassung - Gl. (16)-(21)
    und MATLAB-Funktion peq.m (Anhang B), hier woertlich uebertragen.

Parametrisierung auf den RBJ-Prototyp: G = 10^(dB/20), Vicanek Gl. (42) ist
derselbe Prototyp wie RBJ (sqrt(G) = A). Orfanidis mit G0 = 1, GB = sqrt(G)
(Halbgain in dB), Dw = w0/Q - dort ist die analoge Bandbreite exakt w0/Q
(r1: analoge Halbgain-Punkte f0*(sqrt(1+r^2) -/+ r), Differenz f0/Q)."""
import math
import csv
import numpy as np
import nk
from r1_cramping import RATEN, QS, GAINS, F_AUS, bell_faelle, dig_db


def vicanek_peak(fs, f0, Q, gdb, q_regel="prototyp"):
    G = 10.0 ** (gdb / 20.0)
    w0 = 2 * math.pi * f0 / fs
    # Gl. (11)/(12): Nenner des Prototyps s^2 + 2 q w0 s + w0^2. Beim Peaking-
    # Prototyp Gl. (42) ist der Nenner w0^2 + s w0/(sqrt(G) Q) + s^2, also
    # q = 1/(2 Q sqrt(G)). ("naiv" = 1/(2Q) nur als Gegenprobe.)
    q = 1.0 / (2.0 * Q * math.sqrt(G)) if q_regel == "prototyp" else 1.0 / (2.0 * Q)
    if q <= 1.0:
        a1 = -2.0 * math.exp(-q * w0) * math.cos(math.sqrt(1.0 - q * q) * w0)
    else:
        a1 = -2.0 * math.exp(-q * w0) * math.cosh(math.sqrt(q * q - 1.0) * w0)
    a2 = math.exp(-2.0 * q * w0)
    s2 = math.sin(w0 / 2.0) ** 2
    phi0, phi1 = 1.0 - s2, s2
    phi2 = 4.0 * phi0 * phi1
    A0 = (1.0 + a1 + a2) ** 2
    A1 = (1.0 - a1 + a2) ** 2
    A2 = -4.0 * a2
    B0 = A0                                             # Bedingung 1
    R1 = (A0 * phi0 + A1 * phi1 + A2 * phi2) * G * G    # Gl. (44)
    R2 = (-A0 + A1 + 4.0 * (phi0 - phi1) * A2) * G * G
    B2 = (R1 - R2 * phi1 - B0) / (4.0 * phi1 * phi1)    # Gl. (45)
    B1 = R2 + B0 + 4.0 * (phi1 - phi0) * B2
    if B1 < 0:
        return None
    W = 0.5 * (math.sqrt(B0) + math.sqrt(B1))          # Gl. (29)
    if W * W + B2 < 0:
        return None
    b0 = 0.5 * (W + math.sqrt(W * W + B2))
    b1 = 0.5 * (math.sqrt(B0) - math.sqrt(B1))
    b2 = -B2 / (4.0 * b0)
    return (b0, b1, b2, a1, a2)


def orfanidis_peq(G0, G, GB, w0, Dw):
    """peq.m (Orfanidis 1997, Anhang B) Zeile fuer Zeile."""
    pi = math.pi
    F = abs(G ** 2 - GB ** 2)
    G00 = abs(G ** 2 - G0 ** 2)
    F00 = abs(GB ** 2 - G0 ** 2)
    num = G0 ** 2 * (w0 ** 2 - pi ** 2) ** 2 + G ** 2 * F00 * pi ** 2 * Dw ** 2 / F
    den = (w0 ** 2 - pi ** 2) ** 2 + F00 * pi ** 2 * Dw ** 2 / F
    G1 = math.sqrt(num / den)
    G01 = abs(G ** 2 - G0 * G1)
    G11 = abs(G ** 2 - G1 ** 2)
    F01 = abs(GB ** 2 - G0 * G1)
    F11 = abs(GB ** 2 - G1 ** 2)
    W2 = math.sqrt(G11 / G00) * math.tan(w0 / 2) ** 2
    DW = (1 + math.sqrt(F00 / F11) * W2) * math.tan(Dw / 2)
    C = F11 * DW ** 2 - 2 * W2 * (F01 - math.sqrt(F00 * F11))
    D = 2 * W2 * (G01 - math.sqrt(G00 * G11))
    A = math.sqrt((C + D) / F)
    B = math.sqrt((G ** 2 * C + GB ** 2 * D) / F)
    n = 1 + W2 + A
    b = ((G1 + G0 * W2 + B) / n, -2 * (G1 - G0 * W2) / n, (G1 - B + G0 * W2) / n)
    a = (-2 * (1 - W2) / n, (1 + W2 - A) / n)
    return (b[0], b[1], b[2], a[0], a[1]), G1


def orfanidis_rbj(fs, f0, Q, gdb):
    G = 10.0 ** (gdb / 20.0)
    GB = math.sqrt(G)
    w0 = 2 * math.pi * f0 / fs
    c, G1 = orfanidis_peq(1.0, G, GB, w0, w0 / Q)
    # Gueltigkeitsbereich Gl. (37): G0 < G1 < GB < G (Boost), umgekehrt (Cut)
    gueltig = (1.0 < G1 < GB) if G > 1 else (1.0 > G1 > GB)
    return c, G1, gueltig


def pz_ok(c):
    b0, b1, b2, a1, a2 = c
    p = np.roots([1.0, a1, a2]); z = np.roots([b0, b1, b2])
    return float(np.max(np.abs(p))), float(np.max(np.abs(z)))


def maxdev(c, fs, f0, Q, g):
    d = dig_db(c, F_AUS, fs) - nk.db(nk.analog_bell(F_AUS, f0, Q, g))
    i = int(np.argmax(np.abs(d)))
    return float(d[i]), float(F_AUS[i])


def eigenschaften_vicanek(c, fs, f0, g):
    w0 = 2 * math.pi * f0 / fs
    h = lambda w: abs(nk.biquad_H(c, np.array([w]))[0])
    dc = 20 * math.log10(h(1e-12))
    pk = 20 * math.log10(h(w0))
    e = 1e-6 * w0
    slope = (h(w0 + e) ** 2 - h(w0 - e) ** 2) / (2 * e)
    return dc, pk - g, slope


def eigenschaften_orfanidis(c, fs, f0, Q, g, G1):
    w0 = 2 * math.pi * f0 / fs
    G = 10 ** (g / 20); GB = math.sqrt(G)
    h = lambda w: abs(nk.biquad_H(c, np.array([w]))[0])
    from scipy.optimize import brentq
    fun = lambda w: h(w) - GB
    try:
        w1 = brentq(fun, 1e-9, w0); w2 = brentq(fun, w0, math.pi - 1e-12)
        bw_fehler = (w2 - w1) / (w0 / Q) - 1
    except ValueError:
        bw_fehler = float("nan")
    return 20 * math.log10(h(1e-12)), 20 * math.log10(h(w0)) - g, 20 * math.log10(h(math.pi)) - 20 * math.log10(G1), bw_fehler


def main():
    # Gegenprobe der Pol-Lesart bei 1 kHz (dort ist RBJ fast exakt analog)
    for regel in ("prototyp", "naiv"):
        c = vicanek_peak(48000.0, 1000.0, 2.0, 12.0, regel)
        print("Vicanek-Pol-Lesart %-9s: 1 kHz Q2 +12 dB @48k  max|dev| = %.4f dB" % (regel, abs(maxdev(c, 48000.0, 1000.0, 2.0, 12.0)[0])))

    rows = []
    worst_v = [0, 0, 0]; worst_o = [0, 0, 0, 0]
    maxpole = 0; maxzero = 0
    for fs in RATEN:
        for f0 in bell_faelle(fs):
            for Q in QS:
                for g in GAINS:
                    c_r = nk.rbj_bell(fs, f0, Q, g)
                    c_v = vicanek_peak(fs, f0, Q, g)
                    c_o, G1, ok = orfanidis_rbj(fs, f0, Q, g)
                    dr, fr = maxdev(c_r, fs, f0, Q, g)
                    if c_v is not None:
                        dv, fv = maxdev(c_v, fs, f0, Q, g)
                        ev = eigenschaften_vicanek(c_v, fs, f0, g)
                        worst_v = [max(worst_v[0], abs(ev[0])), max(worst_v[1], abs(ev[1])), max(worst_v[2], abs(ev[2]))]
                        p, z = pz_ok(c_v); maxpole = max(maxpole, p); maxzero = max(maxzero, z)
                    else:
                        dv, fv = float("nan"), float("nan")
                    do, fo = maxdev(c_o, fs, f0, Q, g)
                    eo = eigenschaften_orfanidis(c_o, fs, f0, Q, g, G1)
                    if ok:
                        worst_o = [max(worst_o[k], abs(eo[k])) if not math.isnan(eo[k]) else worst_o[k] for k in range(4)]
                    rows.append(dict(fs=fs, f0=f0, Q=Q, g=g, rbj=dr, f_rbj=fr, vicanek=dv, f_vic=fv,
                                     orfanidis=do, f_orf=fo, orf_gueltig=ok, G1_db=20 * math.log10(G1)))
    with open("r2_faelle.csv", "w", newline="") as fh:
        w = csv.DictWriter(fh, fieldnames=list(rows[0].keys()))
        w.writeheader(); w.writerows(rows)

    print("Vicanek-Eigenschaften (alle Faelle): max|DC| = %.1e dB, max|H(w0)-G| = %.1e dB, max|d|H|^2/dw @w0| = %.1e"
          % tuple(worst_v))
    print("Vicanek: max Polbetrag %.6f, max Nullstellenbetrag %.6f (minimalphasig, stabil)" % (maxpole, maxzero))
    print("Orfanidis-Eigenschaften (gueltige Faelle): max|DC| %.1e dB, max|H(w0)-G| %.1e dB, max|H(pi)-G1| %.1e dB, max|Bandbreitenfehler| %.1e"
          % tuple(worst_o))
    ng = sum(1 for r in rows if not r["orf_gueltig"])
    print("Orfanidis ausserhalb Gl. (37) (G1 nicht zwischen G0 und GB): %d von %d Faellen" % (ng, len(rows)))

    for g in (12.0, 6.0):
        print("\n+%g dB: max|Abweichung| gegen analog (dB), RBJ / Vicanek / Orfanidis (* = ausserhalb Gl. 37)" % g)
        for fs in RATEN:
            for f0 in bell_faelle(fs):
                line = "%-6g %-6g" % (fs, f0)
                for Q in QS:
                    r = [x for x in rows if x["fs"] == fs and x["f0"] == f0 and x["Q"] == Q and x["g"] == g][0]
                    line += "  Q%-3g %5.2f / %5.3f / %5.3f%s" % (Q, abs(r["rbj"]), abs(r["vicanek"]), abs(r["orfanidis"]),
                                                              "" if r["orf_gueltig"] else "*")
                print(line)
    # Cut-Seite: ist Vicanek fuer -g genauso gut?
    print("\nmax|Abweichung| je Verfahren getrennt nach Vorzeichen des Gains (alle Faelle):")
    for sgn in (1, -1):
        sel = [r for r in rows if np.sign(r["g"]) == sgn]
        print("  %s: RBJ %.2f  Vicanek %.3f  Orfanidis(gueltig) %.3f  Orfanidis(alle) %.2f"
              % ("Boost" if sgn > 0 else "Cut  ", max(abs(r["rbj"]) for r in sel), max(abs(r["vicanek"]) for r in sel),
                 max(abs(r["orfanidis"]) for r in sel if r["orf_gueltig"]), max(abs(r["orfanidis"]) for r in sel)))
    # 1-kHz-Gegenfall: aendert Decramping dort etwas?
    sel = [r for r in rows if r["f0"] == 1000.0]
    print("  f0 = 1 kHz: RBJ max %.3f dB, Vicanek max %.4f dB, Orfanidis max %.4f dB" %
          (max(abs(r["rbj"]) for r in sel), max(abs(r["vicanek"]) for r in sel), max(abs(r["orfanidis"]) for r in sel)))

    # Bild
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
    fig, ax = plt.subplots(1, 2, figsize=(14, 5))
    for k, fs in enumerate((44100.0, 48000.0)):
        for f0, col in zip((10000.0, 16000.0, min(20000.0, 0.45 * fs)), ("C0", "C1", "C2")):
            a = nk.db(nk.analog_bell(F_AUS, f0, 2.0, 12.0))
            ax[k].semilogx(F_AUS, a, col + ":", lw=2, label="analog f0=%g" % f0)
            ax[k].semilogx(F_AUS, dig_db(nk.rbj_bell(fs, f0, 2.0, 12.0), F_AUS, fs), col + "--", label="RBJ")
            ax[k].semilogx(F_AUS, dig_db(vicanek_peak(fs, f0, 2.0, 12.0), F_AUS, fs), col, label="Vicanek")
        ax[k].set_xlim(2000, 20000); ax[k].grid(True, which="both"); ax[k].legend(fontsize=7)
        ax[k].set_title("Bell +12 dB Q 2, fs=%g: analog (punktiert), RBJ (gestrichelt), Vicanek" % fs)
    fig.tight_layout(); fig.savefig("r2_decramping.png", dpi=90)


if __name__ == "__main__":
    main()


# ---------------------------------------------------------------------------
# NACHTRAG (eigene Uebertragung, KEINE Primaerquelle): matched High-Shelf.
# Rahmen aus Vicanek Gl. (12), (25)-(29); Pole per Impulsinvarianz aus dem
# Nenner des RBJ-High-Shelf-Prototyps s^2 + (sqrt(A)/Q) w0 s + A w0^2
# (w_n = sqrt(A) w0, q = 1/(2Q)); Zaehler aus drei Betragsbedingungen:
# DC, fs/2 und f0 exakt wie der analoge Prototyp.
def matched_highshelf_eigen(fs, f0, Q, gdb):
    A = 10.0 ** (gdb / 40.0)
    w0 = 2 * math.pi * f0 / fs
    wn = math.sqrt(A) * w0
    q = 1.0 / (2.0 * Q)
    if q <= 1.0:
        a1 = -2.0 * math.exp(-q * wn) * math.cos(math.sqrt(1.0 - q * q) * wn)
    else:
        a1 = -2.0 * math.exp(-q * wn) * math.cosh(math.sqrt(q * q - 1.0) * wn)
    a2 = math.exp(-2.0 * q * wn)
    A0 = (1.0 + a1 + a2) ** 2; A1 = (1.0 - a1 + a2) ** 2; A2 = -4.0 * a2
    ha2 = lambda w: abs(nk.analog_highshelf(np.array([w / (2 * math.pi) * fs]), f0, Q, gdb)[0]) ** 2
    B0 = A0 * ha2(0.0)
    B1 = A1 * ha2(math.pi)
    s2 = math.sin(w0 / 2.0) ** 2
    phi0, phi1 = 1.0 - s2, s2
    phi2 = 4.0 * phi0 * phi1
    B2 = ((A0 * phi0 + A1 * phi1 + A2 * phi2) * ha2(w0) - B0 * phi0 - B1 * phi1) / phi2
    W = 0.5 * (math.sqrt(B0) + math.sqrt(B1))
    if W * W + B2 < 0:
        return None
    b0 = 0.5 * (W + math.sqrt(W * W + B2))
    b1 = 0.5 * (math.sqrt(B0) - math.sqrt(B1))
    b2 = -B2 / (4.0 * b0)
    return (b0, b1, b2, a1, a2)


def nachtrag():
    print("\n--- Cut-Seite (-12 dB), max|Abweichung| RBJ / Vicanek / Orfanidis:")
    for fs in (44100.0, 48000.0):
        for f0 in bell_faelle(fs)[2:]:
            line = "%-6g %-6g" % (fs, f0)
            for Q in QS:
                c_r = nk.rbj_bell(fs, f0, Q, -12.0); c_v = vicanek_peak(fs, f0, Q, -12.0)
                c_o, G1, ok = orfanidis_rbj(fs, f0, Q, -12.0)
                line += "  Q%-3g %5.2f / %5.3f / %5.3f%s" % (Q, abs(maxdev(c_r, fs, f0, Q, -12.0)[0]),
                                                       abs(maxdev(c_v, fs, f0, Q, -12.0)[0]),
                                                       abs(maxdev(c_o, fs, f0, Q, -12.0)[0]), "" if ok else "*")
            print(line)
    print("\n--- NACHTRAG eigene Uebertragung (keine Primaerquelle): matched High-Shelf Q 0,707, max|Abweichung| RBJ -> matched")
    worst = 0.0
    for fs in RATEN:
        line = "%-6g" % fs
        for f0 in (5000.0, 10000.0, 15000.0):
            for g in (12.0, -12.0, 6.0):
                c_r = nk.rbj_highshelf(fs, f0, 0.707, g)
                c_m = matched_highshelf_eigen(fs, f0, 0.707, g)
                dr = np.max(np.abs(dig_db(c_r, F_AUS, fs) - nk.db(nk.analog_highshelf(F_AUS, f0, 0.707, g))))
                dm = np.max(np.abs(dig_db(c_m, F_AUS, fs) - nk.db(nk.analog_highshelf(F_AUS, f0, 0.707, g))))
                p, z = pz_ok(c_m)
                worst = max(worst, dm)
                line += "  %g/%+g: %.2f->%.3f%s" % (f0 / 1000, g, dr, dm, "" if (p < 1 and z < 1) else "!")
        print(line)
    print("  schlechtester matched-Shelf-Fall: %.3f dB" % worst)


if __name__ == "__main__":
    nachtrag()
