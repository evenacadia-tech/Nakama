"""Gegenprobe zum Tiefenaudit P15-D-03: Low-Shelf +6 dB Q 0,707, 5 kHz -> 50 Hz
(Faktor 100, heute wegen R-311-13 ein Crossfade), 1-kHz-Ton 0,5 bei 48 kHz."""
import math, numpy as np, nk
from r5b_kriterienscan import lauf, rbj, tau, AMP
fs = 48000.0; alt = (5000.0, 0.707, 6.0); neu = (50.0, 0.707, 6.0)
w = 2 * math.pi * 1000.0 / fs
ca, cn = rbj("lowshelf", fs, *alt), rbj("lowshelf", fs, *neu)
stat = AMP * max(abs(nk.biquad_H(ca, np.array([w]))[0]), abs(nk.biquad_H(cn, np.array([w]))[0]))
n_f = int(max(8192, 8 * max(tau(ca), tau(cn))))
best = [0, 0, 0]
for phi in np.linspace(0, 2 * math.pi, 16, endpoint=False):
    for ph in (0, 4):
        r = lauf("lowshelf", fs, alt, neu, w, phi, n_f, ph)
        best = [max(b, v) for b, v in zip(best, r)]
print("stationaer max %.3f; DF2T-Rampe %.2f (%+.1f dB) | SVF-Rampe %.2f (%+.1f dB) | Crossfade kalt %.2f (%+.1f dB)"
      % (stat, best[0], 20*math.log10(best[0]/stat), best[1], 20*math.log10(best[1]/stat), best[2], 20*math.log10(best[2]/stat)))
