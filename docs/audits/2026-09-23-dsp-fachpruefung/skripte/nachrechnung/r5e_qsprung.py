"""Mechanismus-Probe: nur Q aendert sich (Bell 50 Hz +12 dB, 48 kHz), Ton auf f0.
Erwartung SVF: y = x + k(A^2-1) v1; v1 traegt die mit kleinem k aufgebaute
Resonanz (Amplitude x/k), k waechst in 5,3 ms um Faktor 4 -> Ausgang ~ +10 dB."""
import math, numpy as np, nk
from r5b_kriterienscan import lauf, rbj, tau, AMP
fs = 48000.0
for qa, qn in ((24.0, 6.0), (6.0, 24.0), (24.0, 12.0), (12.0, 6.0), (2.0, 0.5)):
    alt = (50.0, qa, 12.0); neu = (50.0, qn, 12.0)
    ca, cn = rbj("bell", fs, *alt), rbj("bell", fs, *neu)
    w = 2 * math.pi * 50.0 / fs
    stat = AMP * max(abs(nk.biquad_H(ca, np.array([w]))[0]), abs(nk.biquad_H(cn, np.array([w]))[0]))
    n_f = int(max(8192, 8 * max(tau(ca), tau(cn))))
    b = [0, 0, 0]
    for phi in (0.0, 0.5 * math.pi, math.pi, 1.5 * math.pi):
        for ph in (0, 4):
            b = [max(u, v) for u, v in zip(b, lauf("bell", fs, alt, neu, w, phi, n_f, ph))]
    print("Q %4g -> %-4g: DF2T-Rampe %+5.1f dB | SVF-Rampe %+5.1f dB | Crossfade kalt %+5.1f dB (ueber stationaer, Ton auf f0)"
          % (qa, qn, 20*math.log10(b[0]/stat), 20*math.log10(b[1]/stat), 20*math.log10(b[2]/stat)))
