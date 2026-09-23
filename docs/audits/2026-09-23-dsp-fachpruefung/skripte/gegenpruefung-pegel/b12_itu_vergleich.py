# Kontext zu B12: BS.1770-5 Anhang 2, Beispielfilter (4x, 48 Taps, S. 18/19) im selben Burst-Test wie F)
import numpy as np
from scipy import signal
itu = np.array([
 [ 0.0017089843750,  0.0109863281250, -0.0196533203125,  0.0332031250000, -0.0594482421875,  0.1373291015625,
   0.9721679687500, -0.1022949218750,  0.0476074218750, -0.0266113281250,  0.0148925781250, -0.0083007812500],
 [-0.0291748046875,  0.0292968750000, -0.0517578125000,  0.0891113281250, -0.1665039062500,  0.4650878906250,
   0.7797851562500, -0.2003173828125,  0.1015625000000, -0.0582275390625,  0.0330810546875, -0.0189208984375],
 [-0.0189208984375,  0.0330810546875, -0.0582275390625,  0.1015625000000, -0.2003173828125,  0.7797851562500,
   0.4650878906250, -0.1665039062500,  0.0891113281250, -0.0517578125000,  0.0292968750000, -0.0291748046875],
 [-0.0083007812500,  0.0148925781250, -0.0266113281250,  0.0476074218750, -0.1022949218750,  0.9721679687500,
   0.1373291015625, -0.0594482421875,  0.0332031250000, -0.0196533203125,  0.0109863281250,  0.0017089843750]])
db = lambda v: 20*np.log10(v)
tp = lambda x: max(np.max(np.abs(signal.lfilter(g, [1.0], x))) for g in itu)
print("Zweiggang ITU-Beispiel: ", end="")
for fr in [0.30, 0.40, 0.42, 0.45]:
    w = 2*np.pi*fr; m = [abs(np.sum(g*np.exp(-1j*w*np.arange(12)))) for g in itu]
    print("f/fs=%.2f min %+.2f max %+.2f dB | " % (fr, db(min(m)), db(max(m))), end="")
print()
rng = np.random.default_rng(12); B = 0.03
for f0 in [0.25, 0.35, 0.40, 0.42, 0.44]:
    errs = []
    for _ in range(300):
        tau = rng.uniform(0, 1); phi = rng.uniform(0, 2*np.pi)
        win = lambda t: np.i0(8.0*np.sqrt(np.clip(1 - (t/550.0)**2, 0, 1)))/np.i0(8.0)
        s = lambda t: 2*B*np.sinc(2*B*t)*np.cos(2*np.pi*f0*t + phi)*win(t)
        x = s(np.arange(-600, 601) - tau); td = np.linspace(-20, 20, 160001)
        errs.append(db(tp(x)/np.max(np.abs(s(td)))))
    errs = np.array(errs)
    print("ITU-4x Burst f0=%.2f fs: min %+.3f max %+.3f dB, ausserhalb +0,2/-0,4: %.0f%%" % (f0, errs.min(), errs.max(), 100*np.mean((errs < -0.4) | (errs > 0.2))))
