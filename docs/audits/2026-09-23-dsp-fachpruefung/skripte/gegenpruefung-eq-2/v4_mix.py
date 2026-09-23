# K9/A9 Linearer Mix mit phasendrehenden Filtern. Quelle: DspKern.cpp:1033-1052
# (l *= ag; l = mix*l + (1-mix)*dry; l *= out), Dry = Tap pre_nakama (dryL, DspKern.cpp:1285-1286),
# Wet = Kette nach den Baendern. Frequenzgang des Ausgangs: H_aus = mix*ag*H + (1-mix).
import numpy as np
from v2lib import lowcut, highcut, bell, lowshelf, resp, db20

fs = 48000.0
f = np.geomspace(10.0, 20000.0, 200000)


def mixgang(ba, mix, ag=1.0):
    H = resp(ba, fs, f)
    return mix * ag * H + (1 - mix)


def bericht(name, ba, mixe=(0.25, 0.5, 0.75)):
    H = resp(ba, fs, f)
    print(f"  {name}:")
    for mx in mixe:
        Ha = mixgang(ba, mx)
        i = np.argmin(np.abs(Ha))
        # "halbe Kurve" im Sinn von SONDE-015.md:1592: in dB halbe Wirkung bzw. linear mix*|H| + (1-mix)
        lin_ohne_phase = mx * np.abs(H[i]) + (1 - mx)
        print(f"    Mix {mx:4.2f}: tiefste Stelle {db20(Ha[i]):+7.2f} dB bei {f[i]:7.1f} Hz"
              f" (Wet dort {db20(H[i]):+6.2f} dB, Phase {np.degrees(np.angle(H[i])):+6.1f} Grad;"
              f" ohne Phasenbeachtung waere mix*|H|+(1-mix) = {db20(lin_ohne_phase):+6.2f} dB)")


print("=== Mix-Frequenzgang bei 48 kHz ===")
bericht("low_cut 100 Hz Q 4", lowcut(fs, 100.0, 4.0))
bericht("low_cut 100 Hz Q 0,707", lowcut(fs, 100.0, 0.707))
bericht("low_cut 100 Hz Q 1 (Vertragsdefault)", lowcut(fs, 100.0, 1.0))
bericht("high_cut 8 kHz Q 0,707", highcut(fs, 8000.0, 0.707))
bericht("bell 1 kHz Q 1 -12 dB (minimalphasig, Phase klein)", bell(fs, 1000.0, 1.0, -12.0))
bericht("low_shelf 200 Hz Q 0,707 -12 dB", lowshelf(fs, 200.0, 0.707, -12.0))

print()
print("=== Mix 0,5 mit Auto-Gain im Wet-Zweig (Reihenfolge DspKern.cpp:1040-1048) ===")
# Beispiel: low_cut 100 Hz Q 4 mit einem angenommenen Auto-Gain-Faktor von -1 dB
for ag_db in (0.0, -1.0, +3.0):
    Ha = mixgang(lowcut(fs, 100.0, 4.0), 0.5, 10 ** (ag_db / 20))
    i = np.argmin(np.abs(Ha))
    print(f"  ag {ag_db:+.1f} dB: tiefste Stelle {db20(Ha[i]):+7.2f} dB bei {f[i]:.1f} Hz; oben (10 kHz) {db20(Ha[np.argmin(np.abs(f-10000))]):+.2f} dB")
