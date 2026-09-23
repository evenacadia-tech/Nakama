# Wie viele Hauptstufen-Welchframes (4096/2048) fallen in ein Evidenzfenster?
# FeatureEngine.h:680-682 (Rahmen ab 0,1 s, blockweise geprueft), Frame.h:45-46 (Evidenz ab evidenzIntervallS),
# Spektrum.h:325-326 (Zaehler je Block), Frame.h:161-162 (Stereoakkus je Evidenzfenster geleert), Stereo.h:226 (>= 8).
import numpy as np
def fenster(fs, block, intervall, dauer_s=60.0, N=4096, hop=2048):
    live = evid = 0; gefuellt = 0; frames = 0; zaehl = []
    for _ in range(int(dauer_s * fs / block)):
        # Samples in die Stufe
        rest = block
        while rest > 0:
            nimm = min(rest, N - gefuellt)
            gefuellt += nimm; rest -= nimm
            if gefuellt == N:
                frames += 1; gefuellt = N - hop
        live += block; evid += block
        if live / fs >= 0.1:
            live = 0
            if evid / fs >= intervall:
                zaehl.append(frames); frames = 0; evid = 0
    z = np.array(zaehl[2:])
    return z.min(), z.max(), (z >= 8).mean()
for fs in (44100.0, 48000.0, 96000.0):
    for intervall in (0.25, 0.5):
        teile = []
        for block in (64, 128, 256, 512, 1024, 2048):
            lo, hi, anteil = fenster(fs, block, intervall)
            teile.append("B%4d: %d-%d Frames, >=8 in %3.0f %%" % (block, lo, hi, 100 * anteil))
        print("fs %6.0f, Intervall %.2f s | " % (fs, intervall) + " | ".join(teile))
