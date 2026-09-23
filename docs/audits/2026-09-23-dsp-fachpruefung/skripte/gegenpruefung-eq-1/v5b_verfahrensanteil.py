# Verfahrensanteil je Ton: E = P_rel - max(S_rel, 0)
#   = was das Produkt ueber den groesseren stationaeren Pegel UND ueber das hinaus erzeugt,
#     was ein modulationsfester Sweep (TPT-SVF, Parameter interpoliert) ohnehin erzeugt.
# Nur Toene mit absoluter Produktspitze >= -20 dB re Eingang (hoerbar relevant).
import json, math
import numpy as np
import v5_rampe as v

rows = json.load(open('v5_rampe.json'))
out = []
for r in rows:
    c = tuple(r['fall'])
    P, S, Xf, QSm, QSp, absP, absS = v.fall(*c)
    E = P - np.maximum(S, 0.0)
    m = absP >= -20.0
    if not np.any(m):
        continue
    k = int(np.argmax(np.where(m, E, -1e9)))
    out.append(dict(fall=c, E=float(E[k]), ton=float(v.TOENE[k]), P=float(P[k]), S=float(S[k]), X=float(Xf[k]),
                    QSm=float(QSm[k]), QSp=float(QSp[k]), absP=float(absP[k]), absS=float(absS[k]),
                    Pmax_rel_hoerbar=float(np.max(np.where(m, P, -1e9)))))
json.dump(out, open('v5b.json', 'w'))
E = np.array([o['E'] for o in out])
print(f"{len(out)} Faelle mit hoerbar relevanten Toenen; Verfahrensanteil E: max {E.max():+.2f} dB, "
      f"Anteil E > 1 dB: {np.mean(E>1)*100:.1f} %, > 3 dB: {np.mean(E>3)*100:.1f} %, > 6 dB: {np.mean(E>6)*100:.1f} %")
def fmt(c):
    t, fa, qa, ga, fb, qb, gb = c
    return f"{t:9s} f {fa:>5}->{fb:<5} Q {qa:>5}->{qb:<5} g {ga:>3}->{gb:<3}"
print("Top 20 nach E:")
for o in sorted(out, key=lambda o: -o['E'])[:20]:
    print(f"  {fmt(o['fall'])} | E {o['E']:+6.2f} @ {o['ton']:7.1f} Hz | P {o['P']:+6.2f} S {o['S']:+6.2f} X {o['X']:+6.2f} "
          f"QS-Mix {o['QSm']:+6.2f} QS-Param {o['QSp']:+6.2f} | abs P {o['absP']:+6.1f} abs S {o['absS']:+6.1f} dB re Eingang")
print("\nJe Typ und Kriteriumsart: max E")
def art(c):
    t, fa, qa, ga, fb, qb, gb = c
    return '+'.join(s for s, d in (('f', fa != fb), ('Q', qa != qb), ('g', ga != gb)) if d)
for typ in ('bell', 'lowshelf', 'highshelf', 'notch', 'lowcut', 'highcut'):
    zeile = []
    for a in ('f', 'Q', 'g', 'f+Q', 'f+Q+g'):
        sub = [o['E'] for o in out if o['fall'][0] == typ and art(o['fall']) == a]
        if sub: zeile.append(f"{a}: {max(sub):+6.2f}")
    print(f"  {typ:9s} " + " | ".join(zeile))
# Q <= 6 (gewoehnlichere Einstellungen)
sub = [o for o in out if max(o['fall'][2], o['fall'][5]) <= 6]
print(f"\nNur Q <= 6 an beiden Enden: {len(sub)} Faelle, max E {max(o['E'] for o in sub):+.2f} dB")
for o in sorted(sub, key=lambda o: -o['E'])[:8]:
    print(f"  {fmt(o['fall'])} | E {o['E']:+6.2f} @ {o['ton']:7.1f} Hz | P {o['P']:+6.2f} S {o['S']:+6.2f} | abs P {o['absP']:+6.1f}")
sub = [o for o in out if max(o['fall'][2], o['fall'][5]) <= 3 and o['fall'][0] in ('bell','lowshelf','highshelf')]
print(f"\nNur Bell/Shelf, Q <= 3: {len(sub)} Faelle, max E {max(o['E'] for o in sub):+.2f} dB")
for o in sorted(sub, key=lambda o: -o['E'])[:8]:
    print(f"  {fmt(o['fall'])} | E {o['E']:+6.2f} @ {o['ton']:7.1f} Hz | P {o['P']:+6.2f} S {o['S']:+6.2f} | abs P {o['absP']:+6.1f}")
