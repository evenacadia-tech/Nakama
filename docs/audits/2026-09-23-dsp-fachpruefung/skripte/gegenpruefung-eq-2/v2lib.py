# Unabhaengige Nachbildung (verif2) der Produktformeln, eigenhaendig abgeschrieben aus:
#   eq-copilot/plugin/dsp/DspFilter.h:116-238  (RBJ-Entwuerfe, Bandpass-Detektor)
#   eq-copilot/plugin/dsp/DspFilter.h:306-326, 363-409 (Pole, Pegelstufe, Huellkurve)
#   eq-copilot/plugin/dsp/DspFilter.h:417-442 (leistungInDb, Kennlinie)
#   eq-copilot/plugin/dsp/DspProgramm.cpp:76-83, 175-214 (Auto-Gain-Gitter, Stereoformel)
#   eq-copilot/plugin/dsp/DspKern.cpp:998-1009 (M/S-Stufe mit Mono-Bass), :1033-1052 (Auto-Gain, Mix)
# Frequenzgaenge ueber scipy.signal.freqz (anderer Rechenweg als die Handformel der Vorpruefer),
# Zeitbereich ueber scipy.signal.lfilter (Direct Form II transponiert, wie DspFilter.h:90-96).
import numpy as np
from scipy import signal

PI = np.pi


def _base(fs, f, q):
    w0 = 2.0 * PI * f / fs
    return w0, np.sin(w0) / (2.0 * q), np.cos(w0)


def bell(fs, f, q, g):
    A = 10.0 ** (g / 40.0)
    w0, al, c = _base(fs, f, q)
    a0 = 1.0 + al / A
    return np.array([1.0 + al * A, -2.0 * c, 1.0 - al * A]) / a0, np.array([a0, -2.0 * c, 1.0 - al / A]) / a0


def lowshelf(fs, f, q, g):
    A = 10.0 ** (g / 40.0)
    w0, al, c = _base(fs, f, q)
    zw = 2.0 * np.sqrt(A) * al
    a0 = (A + 1) + (A - 1) * c + zw
    b = np.array([A * ((A + 1) - (A - 1) * c + zw), 2 * A * ((A - 1) - (A + 1) * c), A * ((A + 1) - (A - 1) * c - zw)])
    a = np.array([a0, -2 * ((A - 1) + (A + 1) * c), (A + 1) + (A - 1) * c - zw])
    return b / a0, a / a0


def highshelf(fs, f, q, g):
    A = 10.0 ** (g / 40.0)
    w0, al, c = _base(fs, f, q)
    zw = 2.0 * np.sqrt(A) * al
    a0 = (A + 1) - (A - 1) * c + zw
    b = np.array([A * ((A + 1) + (A - 1) * c + zw), -2 * A * ((A - 1) + (A + 1) * c), A * ((A + 1) + (A - 1) * c - zw)])
    a = np.array([a0, 2 * ((A - 1) - (A + 1) * c), (A + 1) - (A - 1) * c - zw])
    return b / a0, a / a0


def lowcut(fs, f, q, g=0.0):
    w0, al, c = _base(fs, f, q)
    a0 = 1.0 + al
    return np.array([(1 + c) * 0.5, -(1 + c), (1 + c) * 0.5]) / a0, np.array([a0, -2 * c, 1 - al]) / a0


def highcut(fs, f, q, g=0.0):
    w0, al, c = _base(fs, f, q)
    a0 = 1.0 + al
    return np.array([(1 - c) * 0.5, (1 - c), (1 - c) * 0.5]) / a0, np.array([a0, -2 * c, 1 - al]) / a0


def bandpass(fs, f, q):
    w0, al, c = _base(fs, f, q)
    a0 = 1.0 + al
    return np.array([al, 0.0, -al]) / a0, np.array([a0, -2 * c, 1 - al]) / a0


def resp(ba, fs, freqs):
    b, a = ba
    _, h = signal.freqz(b, a, worN=np.atleast_1d(np.asarray(freqs, dtype=float)), fs=fs)
    return h


def db20(x):
    return 20.0 * np.log10(np.maximum(np.abs(x), 1e-300))


def pol(ms, fs):
    tau = ms * 1e-3
    return 0.0 if not (tau > 0 and fs > 0) else float(np.exp(-1.0 / (fs * tau)))


def kennlinie(pegel_db, t, r, knie=12.0):
    ueber = pegel_db - t
    if not (ueber > 0):
        return 0.0
    u = 1.0 if ueber >= knie else ueber / knie
    return r * u


def leistung_db(p):
    if not (p > 0) or not np.isfinite(p):
        return -240.0
    d = 10 * np.log10(p)
    return max(d, -240.0)


def detektor_kette(x0, x1, fs, f, q, attack_ms, hold_ms, release_ms, fenster_ms=10.0):
    """Detektor-Bandpass -> Momentanleistung (E-5-Mittel bei zwei Komponenten)
    -> Pegelstufe (symmetrischer Ein-Pol, DspFilter.h:363-368)
    -> Huellkurve (DspFilter.h:393-409). Gibt die Huellkurvenleistung je Sample zurueck."""
    b, a = bandpass(fs, f, q)
    d0 = signal.lfilter(b, a, x0)
    if x1 is not None:
        d1 = signal.lfilter(b, a, x1)
        p_in = (d0 * d0 + d1 * d1) * 0.5
    else:
        p_in = d0 * d0
    pp = pol(fenster_ms, fs)
    ap, rp = pol(attack_ms, fs), pol(release_ms, fs)
    hold = int(round(hold_ms * 1e-3 * fs))
    # Pegelstufe: lineares IIR, exakt wie leistung = pp*leistung + (1-pp)*ein
    if pp > 0:
        pegel = signal.lfilter([1 - pp], [1, -pp], p_in)
    else:
        pegel = p_in
    env = np.empty_like(pegel)
    e = 0.0
    hr = 0
    for n, v in enumerate(pegel):
        if v > e:
            e = ap * e + (1 - ap) * v
            hr = hold
        elif hr > 0:
            hr -= 1
        else:
            e = rp * e + (1 - rp) * v
        env[n] = e
    return env
