# Sauberer Synthieton wie in FL (float32-Samples, kein Rauschboden): Ausloesungen in 30 s
import numpy as np
import b11_fluss as f
n = int(30 * f.fs); t = np.arange(n) / f.fs
for name, x in (("Sinus 440 Hz, float32", (0.3*np.sin(2*np.pi*440*t)).astype(np.float32).astype(np.float64)),
                ("Saegezahn-artig 110 Hz (20 Obertoene), float32", sum((0.3/h)*np.sin(2*np.pi*110*h*t) for h in range(1, 21)).astype(np.float32).astype(np.float64)),
                ("Sinus 440 Hz, float64 ohne Boden", 0.3*np.sin(2*np.pi*440*t))):
    c = f.detektor(f.logbaender(x))
    print("  %-48s %3d Ausloesungen / %d Frames" % (name, c[0], c[1]))
