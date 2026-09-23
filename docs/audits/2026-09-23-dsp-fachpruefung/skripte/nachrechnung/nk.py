"""Gemeinsame Bausteine der Nachrechnung.

Die Entwuerfe sind 1:1 aus
  eq-copilot/plugin/dsp/DspFilter.h   (RBJ, DF2T-Tick)
  eq-copilot/plugin/dsp/DspSvf.h      (Simper-TPT-SVF, Tick)
  eq-copilot/plugin/dsp/DspKern.cpp   (mische, lerp)
uebernommen, inklusive Reihenfolge der Rechenschritte.
"""
import math
import numpy as np
from numba import njit

kPi = 3.14159265358979323846
assert kPi == math.pi
kNyquistAnteil = 0.45


# ---------------------------------------------------------------- RBJ (DspFilter.h)
def rbj_bell(fs, f, q, gdb):
    A = math.pow(10.0, gdb / 40.0)
    w0 = 2.0 * kPi * f / fs
    alpha = math.sin(w0) / (2.0 * q)
    c = math.cos(w0)
    a0 = 1.0 + alpha / A
    return ((1.0 + alpha * A) / a0, (-2.0 * c) / a0, (1.0 - alpha * A) / a0,
            (-2.0 * c) / a0, (1.0 - alpha / A) / a0)


def rbj_lowshelf(fs, f, q, gdb):
    A = math.pow(10.0, gdb / 40.0)
    w0 = 2.0 * kPi * f / fs
    alpha = math.sin(w0) / (2.0 * q)
    c = math.cos(w0)
    zwei = 2.0 * math.sqrt(A) * alpha
    a0 = (A + 1.0) + (A - 1.0) * c + zwei
    return (A * ((A + 1.0) - (A - 1.0) * c + zwei) / a0,
            2.0 * A * ((A - 1.0) - (A + 1.0) * c) / a0,
            A * ((A + 1.0) - (A - 1.0) * c - zwei) / a0,
            -2.0 * ((A - 1.0) + (A + 1.0) * c) / a0,
            ((A + 1.0) + (A - 1.0) * c - zwei) / a0)


def rbj_highshelf(fs, f, q, gdb):
    A = math.pow(10.0, gdb / 40.0)
    w0 = 2.0 * kPi * f / fs
    alpha = math.sin(w0) / (2.0 * q)
    c = math.cos(w0)
    zwei = 2.0 * math.sqrt(A) * alpha
    a0 = (A + 1.0) - (A - 1.0) * c + zwei
    return (A * ((A + 1.0) + (A - 1.0) * c + zwei) / a0,
            -2.0 * A * ((A - 1.0) + (A + 1.0) * c) / a0,
            A * ((A + 1.0) + (A - 1.0) * c - zwei) / a0,
            2.0 * ((A - 1.0) - (A + 1.0) * c) / a0,
            ((A + 1.0) - (A - 1.0) * c - zwei) / a0)


def rbj_lowcut(fs, f, q):
    w0 = 2.0 * kPi * f / fs
    alpha = math.sin(w0) / (2.0 * q)
    c = math.cos(w0)
    a0 = 1.0 + alpha
    return ((1.0 + c) * 0.5 / a0, -(1.0 + c) / a0, (1.0 + c) * 0.5 / a0,
            (-2.0 * c) / a0, (1.0 - alpha) / a0)


RBJ = {"bell": rbj_bell, "lowshelf": rbj_lowshelf, "highshelf": rbj_highshelf}


# ---------------------------------------------------------------- SVF (DspSvf.h)
def svf_grundG(f, fs):
    return math.tan(kPi * f / fs)


def _svf_integratoren(g, k):
    a1 = 1.0 / (1.0 + g * (g + k))
    a2 = g * a1
    a3 = g * a2
    return a1, a2, a3


def svf_bell(grundG, q, gdb):
    A = math.pow(10.0, gdb / 40.0)
    k = 1.0 / (q * A)
    a1, a2, a3 = _svf_integratoren(grundG, k)
    return (a1, a2, a3, 1.0, k * (A * A - 1.0), 0.0, grundG, k)


def svf_lowshelf(grundG, q, gdb):
    A = math.pow(10.0, gdb / 40.0)
    g = grundG / math.sqrt(A)
    k = 1.0 / q
    a1, a2, a3 = _svf_integratoren(g, k)
    return (a1, a2, a3, 1.0, k * (A - 1.0), A * A - 1.0, g, k)


def svf_highshelf(grundG, q, gdb):
    A = math.pow(10.0, gdb / 40.0)
    g = grundG * math.sqrt(A)
    k = 1.0 / q
    a1, a2, a3 = _svf_integratoren(g, k)
    return (a1, a2, a3, A * A, k * (1.0 - A) * A, 1.0 - A * A, g, k)


SVF = {"bell": svf_bell, "lowshelf": svf_lowshelf, "highshelf": svf_highshelf}


# ---------------------------------------------------------------- Frequenzgaenge
def biquad_H(c, w):
    """H(e^{jw}) eines Biquads (b0,b1,b2,a1,a2); w in rad/Sample (Array)."""
    b0, b1, b2, a1, a2 = c
    zi = np.exp(-1j * w)
    zi2 = zi * zi
    return (b0 + b1 * zi + b2 * zi2) / (1.0 + a1 * zi + a2 * zi2)


def svf_statespace(c):
    """Zustandsraum des SVF-Ticks (DspSvf.h:73-81), direkt aus der Rekursion:
       Zustand s=[ic1eq, ic2eq]; s' = M s + B x; y = C s + D x."""
    a1, a2, a3, m0, m1, m2 = c[:6]
    M = np.array([[2 * a1 - 1, -2 * a2], [2 * a2, 1 - 2 * a3]])
    B = np.array([2 * a2, 2 * a3])
    C = np.array([m1 * a1 + m2 * a2, -m1 * a2 + m2 * (1 - a3)])
    D = m0 + m1 * a2 + m2 * a3
    return M, B, C, D


def svf_H(c, w):
    """H(e^{jw}) = C (zI-M)^-1 B + D des tatsaechlichen SVF-Ticks."""
    M, B, C, D = svf_statespace(c)
    z = np.exp(1j * w)
    # 2x2-Inverse geschlossen
    m11, m12, m21, m22 = M[0, 0], M[0, 1], M[1, 0], M[1, 1]
    d11 = z - m11
    d22 = z - m22
    det = d11 * d22 - m12 * m21
    # (zI-M)^-1 = 1/det [[d22, m12],[m21, d11]]
    v0 = (d22 * B[0] + m12 * B[1]) / det
    v1 = (m21 * B[0] + d11 * B[1]) / det
    return C[0] * v0 + C[1] * v1 + D


def analog_bell(f, f0, q, gdb):
    A = 10.0 ** (gdb / 40.0)
    s = 1j * np.asarray(f) / f0
    return (s * s + s * (A / q) + 1.0) / (s * s + s / (A * q) + 1.0)


def analog_highshelf(f, f0, q, gdb):
    A = 10.0 ** (gdb / 40.0)
    s = 1j * np.asarray(f) / f0
    sa = math.sqrt(A)
    return A * (A * s * s + (sa / q) * s + 1.0) / (s * s + (sa / q) * s + A)


def analog_lowshelf(f, f0, q, gdb):
    A = 10.0 ** (gdb / 40.0)
    s = 1j * np.asarray(f) / f0
    sa = math.sqrt(A)
    return A * (s * s + (sa / q) * s + A) / (A * s * s + (sa / q) * s + 1.0)


def db(x):
    return 20.0 * np.log10(np.abs(x))


# ---------------------------------------------------------------- Zeitbereich (numba)
@njit(cache=True)
def df2t_run(b0, b1, b2, a1, a2, x):
    """DspFilter.h:90-96 - Reihenfolge der Rechenschritte wie im Produkt.
    Typ folgt den Argumenten (float64 oder float32)."""
    y = np.empty_like(x)
    z1 = x[0] - x[0]
    z2 = z1
    for i in range(x.size):
        xi = x[i]
        yi = b0 * xi + z1
        z1 = b1 * xi - a1 * yi + z2
        z2 = b2 * xi - a2 * yi
        y[i] = yi
    return y


@njit(cache=True)
def df2t_run_tv(B0, B1, B2, A1, A2, x, z1, z2):
    """DF2T mit Koeffizienten je Sample; Startzustand (z1, z2)."""
    y = np.empty_like(x)
    for i in range(x.size):
        xi = x[i]
        yi = B0[i] * xi + z1
        z1 = B1[i] * xi - A1[i] * yi + z2
        z2 = B2[i] * xi - A2[i] * yi
        y[i] = yi
    return y, z1, z2


@njit(cache=True)
def svf_run(a1, a2, a3, m0, m1, m2, x):
    """DspSvf.h:73-81."""
    y = np.empty_like(x)
    ic1 = 0.0
    ic2 = 0.0
    for i in range(x.size):
        xi = x[i]
        v3 = xi - ic2
        v1 = a1 * ic1 + a2 * v3
        v2 = ic2 + a2 * ic1 + a3 * v3
        ic1 = 2.0 * v1 - ic1
        ic2 = 2.0 * v2 - ic2
        y[i] = m0 * xi + m1 * v1 + m2 * v2
    return y


@njit(cache=True)
def svf_run_tv(A1, A2, A3, M0, M1, M2, x, ic1, ic2):
    y = np.empty_like(x)
    for i in range(x.size):
        xi = x[i]
        v3 = xi - ic2
        v1 = A1[i] * ic1 + A2[i] * v3
        v2 = ic2 + A2[i] * ic1 + A3[i] * v3
        ic1 = 2.0 * v1 - ic1
        ic2 = 2.0 * v2 - ic2
        y[i] = M0[i] * xi + M1[i] * v1 + M2[i] * v2
    return y, ic1, ic2


# ---- double-double Referenz (fehlerfreie Transformationen, ohne FMA)
@njit(cache=True, inline="always")
def _two_sum(a, b):
    s = a + b
    bb = s - a
    e = (a - (s - bb)) + (b - bb)
    return s, e


@njit(cache=True, inline="always")
def _qts(a, b):
    s = a + b
    e = b - (s - a)
    return s, e


@njit(cache=True, inline="always")
def _split(a):
    c = 134217729.0 * a
    abig = c - a
    ahi = c - abig
    return ahi, a - ahi


@njit(cache=True, inline="always")
def _two_prod(a, b):
    p = a * b
    ah, al = _split(a)
    bh, bl = _split(b)
    e = ((ah * bh - p) + ah * bl + al * bh) + al * bl
    return p, e


@njit(cache=True, inline="always")
def _dd_add(ah, al, bh, bl):
    s, e = _two_sum(ah, bh)
    e += al + bl
    return _qts(s, e)


@njit(cache=True, inline="always")
def _dd_mul_d(ah, al, b):
    """(ah+al) * b, b ein double."""
    p, e = _two_prod(ah, b)
    e += al * b
    return _qts(p, e)


@njit(cache=True)
def df2t_run_dd(b0, b1, b2, a1, a2, x):
    """Dieselbe DF2T-Rekursion in double-double (~106 Bit). Koeffizienten und
    Eingang sind exakte doubles; Rueckgabe hi und lo."""
    n = x.size
    yh = np.empty(n)
    yl = np.empty(n)
    z1h = 0.0; z1l = 0.0; z2h = 0.0; z2l = 0.0
    for i in range(n):
        xi = x[i]
        # y = b0*x + z1
        th, tl = _two_prod(b0, xi)
        y_h, y_l = _dd_add(th, tl, z1h, z1l)
        # z1 = b1*x - a1*y + z2
        ph, pl = _two_prod(b1, xi)
        qh, ql = _dd_mul_d(y_h, y_l, a1)
        sh, sl = _dd_add(ph, pl, -qh, -ql)
        z1h, z1l = _dd_add(sh, sl, z2h, z2l)
        # z2 = b2*x - a2*y
        ph, pl = _two_prod(b2, xi)
        qh, ql = _dd_mul_d(y_h, y_l, a2)
        z2h, z2l = _dd_add(ph, pl, -qh, -ql)
        yh[i] = y_h
        yl[i] = y_l
    return yh, yl


@njit(cache=True)
def svf_run32(a1, a2, a3, m0, m1, m2, x):
    """DspSvf.h-Tick in float32 (alle Argumente float32)."""
    y = np.empty_like(x)
    two = np.float32(2.0)
    ic1 = np.float32(0.0)
    ic2 = np.float32(0.0)
    for i in range(x.size):
        xi = x[i]
        v3 = xi - ic2
        v1 = a1 * ic1 + a2 * v3
        v2 = ic2 + a2 * ic1 + a3 * v3
        ic1 = two * v1 - ic1
        ic2 = two * v2 - ic2
        y[i] = m0 * xi + m1 * v1 + m2 * v2
    return y


@njit(cache=True)
def svf_run_dd(a1, a2, a3, m0, m1, m2, x):
    """Derselbe SVF-Tick in double-double; Koeffizienten exakte doubles."""
    n = x.size
    yh = np.empty(n)
    yl = np.empty(n)
    c1h = 0.0; c1l = 0.0; c2h = 0.0; c2l = 0.0
    for i in range(n):
        xi = x[i]
        v3h, v3l = _dd_add(xi, 0.0, -c2h, -c2l)
        t1h, t1l = _dd_mul_d(c1h, c1l, a1)
        t2h, t2l = _dd_mul_d(v3h, v3l, a2)
        v1h, v1l = _dd_add(t1h, t1l, t2h, t2l)
        t3h, t3l = _dd_mul_d(c1h, c1l, a2)
        t4h, t4l = _dd_mul_d(v3h, v3l, a3)
        sh, sl = _dd_add(c2h, c2l, t3h, t3l)
        v2h, v2l = _dd_add(sh, sl, t4h, t4l)
        c1h, c1l = _dd_add(2.0 * v1h, 2.0 * v1l, -c1h, -c1l)
        c2h, c2l = _dd_add(2.0 * v2h, 2.0 * v2l, -c2h, -c2l)
        p0h, p0l = _two_prod(m0, xi)
        p1h, p1l = _dd_mul_d(v1h, v1l, m1)
        p2h, p2l = _dd_mul_d(v2h, v2l, m2)
        sh, sl = _dd_add(p0h, p0l, p1h, p1l)
        y_h, y_l = _dd_add(sh, sl, p2h, p2l)
        yh[i] = y_h
        yl[i] = y_l
    return yh, yl
