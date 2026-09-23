# Gegenprobe zu SONDE-015.md:1621 ("+12-dB-Bell mit Q 12: Betragsmittel -0,28 dB, Energiemittel -0,95 dB").
import numpy as np
from v2lib import bell, resp
k = np.arange(121); gitter = 20.0 * (1000.0 ** (k / 120.0))
for fs in (44100.0, 48000.0, 96000.0):
    for fc in (100.0, 1000.0, 10000.0):
        H = np.abs(resp(bell(fs, fc, 12.0, 12.0), fs, gitter))
        ff = 20.0 * (1000.0 ** np.linspace(0, 1, 200001)); Hd = np.abs(resp(bell(fs, fc, 12.0, 12.0), fs, ff))
        print(f"fs {fs:.0f} fc {fc:6.0f}: Gitter Energie {-10*np.log10(np.mean(H**2)):+.3f} dB, Betrag {-20*np.log10(np.mean(H)):+.3f} dB | dicht Energie {-10*np.log10(np.mean(Hd**2)):+.3f}, Betrag {-20*np.log10(np.mean(Hd)):+.3f}")
