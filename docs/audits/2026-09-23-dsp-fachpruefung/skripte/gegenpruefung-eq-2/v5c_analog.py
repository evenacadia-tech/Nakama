# Analoge Gegenprobe (RBJ-Prototyp, unabhaengig vom Digitalentwurf): Bell +12 dB, Q 12 und Q 24, Mitte 1 kHz,
# Mittel ueber log-Frequenz 20 Hz..20 kHz.
import numpy as np
A = 10 ** (12 / 40)
ff = 20.0 * (1000.0 ** np.linspace(0, 1, 2000001))
for Q in (12.0, 24.0):
    s = 1j * ff / 1000.0
    H = (s * s + s * A / Q + 1) / (s * s + s / (A * Q) + 1)
    print(f"Q {Q:4.0f}: Energie {-10*np.log10(np.mean(np.abs(H)**2)):+.3f} dB, Betrag {-20*np.log10(np.mean(np.abs(H))):+.3f} dB")
