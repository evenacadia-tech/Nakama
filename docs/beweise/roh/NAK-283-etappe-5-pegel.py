# NAK-283 Etappe 5 - Buehnenpegel M-44 bis M-47 aus den Konstanten nachgerechnet (Bauer, 13.09.2026)
# Aufruf: PYTHONIOENCODING=utf-8 py -3.13 docs/beweise/roh/NAK-283-etappe-5-pegel.py
# kHauptPunkte = 4096, kBassPunkte = 16384 (FeatureEngine.h), kAktivGateDb = -60.0;
# Versatz ohne Integration: 10*log10(fs/punkte); Histogrammklasse 0,01 dB ab -200 dBTP.
import math
H, B = 4096, 16384
gate = -60.0
print("Versatz 10*log10(fs/N) je Stufe:")
for fs in (44100, 48000, 96000, 192000):
    print(f"  fs={fs}: Haupt {10*math.log10(fs/H):.2f} dB, Bass {10*math.log10(fs/B):.2f} dB")
print("Differenz der Stufen:", f"{10*math.log10(B/H):.2f} dB")
lo = gate + 10*math.log10(48000/H); hi = gate + 10*math.log10(96000/H)
print(f"M-44 Fenster fuer L: {lo:.2f} < L < {hi:.2f}; L=-48.0: {-48.0-lo:.2f} dB ueber unterer, {hi+48.0:.2f} dB unter oberer Kante")
for fs in (44100, 48000, 96000, 192000):
    v = -48.0 - 10*math.log10(fs/H)
    print(f"  M-44 ohne Δf fs={fs}: Haupt {v:.2f} dB -> {'aktiv' if v > gate else 'inaktiv'}")
L1, L2 = -52.3, -63.0
print(f"M-45 L1 ohne Δf bei 48k: Bass {L1-10*math.log10(48000/B):.2f}, Haupt {L1-10*math.log10(48000/H):.2f}")
print(f"M-45 L2 Δf fest aus kHauptPunkte: Bass {L2+10*math.log10(B/H):.2f}, Haupt {L2:.2f}")
print(f"M-45 L2-Fenster der Mutation: {gate-10*math.log10(B/H):.2f} .. {gate:.2f}")
print(f"M-46 (a) 44.1k ohne Δf: Bass {-58-10*math.log10(44100/B):.2f}, Haupt {-58-10*math.log10(44100/H):.2f}")
print(f"M-47 96k ohne Δf: Bass {-58-10*math.log10(96000/B):.2f}, Haupt {-58-10*math.log10(96000/H):.2f}")
for db in (-48.0, -52.3, -63.0, -58.0, -62.0):
    print(f"  Amplitude fuer {db} dB mittlere Kanalenergie: {math.sqrt(2)*10**(db/20):.6f}")
k = (-6.0206 + 200)/0.01
print("Klasse -6.0206:", int(k), "Mitte", -200 + (int(k)+0.5)*0.01)
print("Histogramm Bytes:", 26000*4)
