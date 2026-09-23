# K1/A8 Mono-Bass. Quelle: DspProgramm.cpp:278-279 (entwurfLowCut(fs, fc, 1/sqrt 2) als monoBassHochpass),
# DspKern.cpp:998-1009 (m=(L+R)/2, s=(L-R)/2, NUR s durch den Hochpass, dann s*=w, L=m+s, R=m-s).
# (a) analytisch per freqz, (b) Zeitbereich: exakt die Samplefolge des Kerns, (c) Gegenentwurf LR4 auf S + Allpass auf M.
import numpy as np
from scipy import signal
from v2lib import lowcut, highcut, resp, db20

fs = 48000.0
print("=== (a) analytisch, fs = 48 kHz, Hochpass RBJ Q = 1/sqrt(2) nur auf S ===")
for fc in (60.0, 120.0, 250.0):
    hp = lowcut(fs, fc, 1 / np.sqrt(2))
    lp = highcut(fs, fc, 1 / np.sqrt(2))
    print(f"fc = {fc:.0f} Hz")
    for k in (0.25, 0.5, 1.0, 2.0, 4.0, 10.0):
        f = k * fc
        H = resp(hp, fs, f)[0]
        # hart links: L = x, R = 0 -> M = S = x/2 ; L' = x/2 (1+H), R' = x/2 (1-H)
        rl = db20((1 - H) / (1 + H))
        # Gegenentwurf: S' = HP^2 S (LR4-Hochpass), M' = (LP^2 + HP^2) M (LR4-Summe = Allpass)
        Hhp4 = resp(hp, fs, f)[0] ** 2
        Hlp4 = resp(lp, fs, f)[0] ** 2
        Hap = Hhp4 + Hlp4
        rl_lr4 = db20((Hap - Hhp4) / (Hap + Hhp4))
        print(f"  {k:5.2f}*fc = {f:7.1f} Hz: Rest-Seite {db20(H):7.2f} dB, Phase {np.degrees(np.angle(H)):7.1f} Grad,"
              f" R/L hart links {rl:7.2f} dB | LR4+Allpass: R/L {rl_lr4:8.2f} dB, |Allpass| {db20(Hap):+.4f} dB,"
              f" Allpass-Phase {np.degrees(np.angle(Hap)):7.1f} Grad")

print()
print("=== (b) Zeitbereich, exakt die Samplefolge DspKern.cpp:998-1009 (width = 1), hart links, fc = 120 Hz ===")
fc = 120.0
b, a = lowcut(fs, fc, 1 / np.sqrt(2))
for k in (0.5, 2.0, 4.0, 10.0):
    f = k * fc
    n = int(fs * 3)
    t = np.arange(n) / fs
    L = np.sin(2 * PI * f * t) if False else np.sin(2 * np.pi * f * t)
    R = np.zeros(n)
    m = (L + R) * 0.5
    s = (L - R) * 0.5
    s = signal.lfilter(b, a, s)
    Lo, Ro = m + s, m - s
    seg = slice(int(fs * 2), n)   # eingeschwungen
    rms = lambda v: np.sqrt(np.mean(v[seg] ** 2))
    print(f"  {f:7.1f} Hz: L' {20*np.log10(rms(Lo)/rms(L)):+7.2f} dB, R' {20*np.log10(rms(Ro)/rms(L)):+7.2f} dB,"
          f" R/L {20*np.log10(rms(Ro)/rms(Lo)):+7.2f} dB, Monosumme (L'+R') gegen (L+R): max |diff| = {np.max(np.abs((Lo+Ro)-(L+R))):.2e}")

print()
print("=== (c) zum Vergleich: dieselbe Stufe mit HOEHERER Ordnung, aber weiter ohne Kompensation auf M ===")
fc = 120.0
hp = lowcut(fs, fc, 1 / np.sqrt(2))
for k in (0.5, 2.0, 4.0, 10.0):
    f = k * fc
    H4 = resp(hp, fs, f)[0] ** 2       # LR4-Hochpass allein auf S, M unberuehrt
    print(f"  {f:7.1f} Hz: LR4 nur auf S (ohne Allpass auf M): Rest-Seite {db20(H4):7.2f} dB, R/L hart links {db20((1-H4)/(1+H4)):7.2f} dB")

print()
print("=== (d) Sprung links: Richtungsstabilitaet unterhalb fc (Anteil, der nach rechts kippt) ===")
for fc in (120.0,):
    hp = lowcut(fs, fc, 1 / np.sqrt(2))
    fr = np.geomspace(fc / 8, fc, 200)
    H = resp(hp, fs, fr)
    rl = db20((1 - H) / (1 + H))
    i = np.argmax(rl)
    print(f"  fc = {fc:.0f} Hz: groesstes R/L unterhalb fc = {rl[i]:+.2f} dB bei {fr[i]:.1f} Hz ({fr[i]/fc:.3f}*fc)")
