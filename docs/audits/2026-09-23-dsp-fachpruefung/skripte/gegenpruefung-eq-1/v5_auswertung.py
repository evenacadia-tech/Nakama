import json
import numpy as np
rows = json.load(open('v5_rampe.json'))
def fmt(r):
    t, fa, qa, ga, fb, qb, gb = r['fall']
    return f"{t:9s} f {fa:>5}->{fb:<5} Q {qa:>5}->{qb:<5} g {ga:>3}->{gb:<3}"

print(f"{len(rows)} Faelle")
P = np.array([r['P'] for r in rows]); S = np.array([r['S'] for r in rows]); X = np.array([r['X'] for r in rows])
D = np.array([r['PmS'] for r in rows])
print(f"Produkt  (Spitze ueber groesserem stationaerem Pegel): max {P.max():+.2f} dB, Median {np.median(P):+.2f}, Anteil > 1 dB: {np.mean(P>1)*100:.0f} %")
print(f"SVF-Param                                         : max {S.max():+.2f} dB, Median {np.median(S):+.2f}, Anteil > 1 dB: {np.mean(S>1)*100:.0f} %")
print(f"Crossfade (A warm, B kalt)                        : max {X.max():+.2f} dB, Median {np.median(X):+.2f}")
print(f"Verfahrensanteil Produkt - SVF je Ton (max ueber Toene): max {D.max():+.2f} dB, Median {np.median(D):+.2f}")

print("\nTop 15 Produkt-Spitzen (mit SVF und Quasistatik am selben Ton):")
for r in sorted(rows, key=lambda r: -r['P'])[:15]:
    print(f"  {fmt(r)} | P {r['P']:+6.2f} @ {r['tonP']:7.1f} Hz | SVF dort {r['S_at_kP']:+6.2f} | QS-Param dort {r['QSp_at_kP']:+6.2f} | QS-Mix dort {r['QSm_at_kP']:+6.2f} | X max {r['X']:+5.2f}")

print("\nTop 15 Verfahrensanteile (Produkt - SVF am selben Ton) und absolute Pegel (re Eingang) dort:")
for r in sorted(rows, key=lambda r: -r['PmS'])[:15]:
    print(f"  {fmt(r)} | P-S {r['PmS']:+6.2f} dB @ {r['tonD']:7.1f} Hz | abs P {r['absP_D']:+6.1f} dB, abs S {r['absS_D']:+6.1f} dB re Eingang | Pmax {r['P']:+6.2f}")

# Verfahrensanteil nur dort, wo der Ton hoerbar relevant ist: absolute Spitze >= -20 dB re Eingang
rel = [r for r in rows if r['absP_D'] >= -20]
print(f"\nNur Stellen mit absoluter Produktspitze >= -20 dB re Eingang: {len(rel)} Faelle; max P-S {max(r['PmS'] for r in rel):+.2f} dB")
for r in sorted(rel, key=lambda r: -r['PmS'])[:10]:
    print(f"  {fmt(r)} | P-S {r['PmS']:+6.2f} dB @ {r['tonD']:7.1f} Hz | abs P {r['absP_D']:+6.1f}, abs S {r['absS_D']:+6.1f} dB re Eingang")

print("\nJe Typ: max Produkt, max SVF, max (P-S) mit abs P >= -20 dB")
for typ in ('bell', 'lowshelf', 'highshelf', 'notch', 'lowcut', 'highcut'):
    sub = [r for r in rows if r['fall'][0] == typ]
    subr = [r for r in sub if r['absP_D'] >= -20]
    print(f"  {typ:9s}: n={len(sub):3d}  Pmax {max(r['P'] for r in sub):+6.2f}  Smax {max(r['S'] for r in sub):+6.2f}  "
          f"Xmax {max(r['X'] for r in sub):+5.2f}  max(P-S) {max(r['PmS'] for r in sub):+6.2f}  max(P-S | abs>=-20) {max([r['PmS'] for r in subr] or [float('nan')]):+6.2f}")

print("\nJe Kriteriumsart: max (P-S) mit abs P >= -20 dB")
def art(c):
    t, fa, qa, ga, fb, qb, gb = c
    s = []
    if fa != fb: s.append('f')
    if qa != qb: s.append('Q')
    if ga != gb: s.append('g')
    return '+'.join(s)
for a in sorted(set(art(r['fall']) for r in rows)):
    sub = [r for r in rows if art(r['fall']) == a]; subr = [r for r in sub if r['absP_D'] >= -20]
    print(f"  {a:6s}: n={len(sub):3d}  Pmax {max(r['P'] for r in sub):+6.2f}  Smax {max(r['S'] for r in sub):+6.2f}  max(P-S | abs>=-20) {max([r['PmS'] for r in subr] or [float('nan')]):+6.2f}")
