# B5/W2/K6: Uebergangszeiten (DspProgramm.h:87,91: kFadeSamples = kRampeSamples = 256; SondeProcessor.cpp:369-370: 0 < fs <= 768 kHz,
#           DspProgramm.h:101: kMinSamplerateHz = 44100).
# K11: Kennlinie DspFilter.h:428-442.
# K12: M/S-Konvention DspKern.cpp:998-1009 (/2) gegen Entwurf-Definition 'energienormiert' (Entwurf :2657, 1/sqrt2).
import numpy as np
from v2lib import kennlinie

print("=== B5: 256 Samples in ms je Rate ===")
for fs in (44100, 48000, 88200, 96000, 176400, 192000, 384000, 768000):
    print(f"  {fs/1000:6.1f} kHz: {256/fs*1000:6.3f} ms  (Entwurf 41.2 Dry<->Processed: 5-20 ms -> {'ja' if 5 <= 256/fs*1000 <= 20 else 'NEIN'})")
print(f"  Grenzrate fuer >= 5 ms: {256/0.005:.0f} Hz")

print()
print("=== K11: statische Kennlinie, Sinus auf Bandmitte eines Bells (out_dB = in_dB + g_dyn) ===")
t = -30.0
for r in (-3.0, -6.0, -12.0, +6.0, +12.0):
    xs = np.arange(-40.0, -5.0, 0.5)
    g = np.array([kennlinie(x, t, r) for x in xs])
    steig = np.diff(xs + g) / np.diff(xs)
    innen = steig[(xs[:-1] >= t) & (xs[:-1] < t + 12)]
    ratio = (1 / innen[0]) if innen[0] != 0 else np.inf
    print(f"  Range {r:+5.1f} dB: Steigung unter t {steig[0]:.3f}, im 12-dB-Bereich {innen[0]:.3f} (Ratio {ratio:.2f}:1), ueber t+12 {steig[-1]:.3f};"
          f" Knick (Steigungssprung) bei t: {innen[0]-steig[0]:+.3f}, bei t+12: {steig[-1]-innen[0]:+.3f}")

print()
print("=== K12: Width-Pegel und Detektorpegel je Konvention ===")
rng = np.random.default_rng(3)
n = 400000
L = rng.standard_normal(n); R = rng.standard_normal(n)       # unkorreliert, gleiche Leistung
for w in (0.0, 1.0, 1.4, 2.0):
    for name, k in (("/2 (Code)", 0.5), ("1/sqrt2 (Entwurf 40.1)", 1 / np.sqrt(2))):
        m = (L + R) * k; s = (L - R) * k
        s = s * w
        Lo = (m + s) / (2 * k); Ro = (m - s) / (2 * k)
        p = 10 * np.log10((np.mean(Lo**2) + np.mean(Ro**2)) / (np.mean(L**2) + np.mean(R**2)))
        if name.startswith("/2"):
            p2 = p
        else:
            print(f"  width {w:3.1f}: Ausgangspegel unkorreliertes Stereo /2: {p2:+6.2f} dB, 1/sqrt2: {p:+6.2f} dB")
x = rng.standard_normal(n)
for name, k in (("/2 (Code)", 0.5), ("1/sqrt2", 1 / np.sqrt(2))):
    m = (x + x) * k
    stereo = (np.mean(x**2) + np.mean(x**2)) / 2           # E-5: mittlere Leistung beider Komponenten
    print(f"  Mittensignal L=R: Detektorleistung mid-Band gegen stereo-Band ({name}): {10*np.log10(np.mean(m**2)/stereo):+.2f} dB")
