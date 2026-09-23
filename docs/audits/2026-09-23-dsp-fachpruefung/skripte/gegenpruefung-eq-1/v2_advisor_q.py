# Befund 2 (K14): Advisor-Guete gegen RBJ-Bandbreite und gegen den DSP-Vertrag.
# proposal.rs:796-798  q = (24 / (bis - von).max(1)).clamp(0.1, 20)
# screening.rs:81-94   bandintervall_der_gruppe: von = ceil(g*221/64), bis = min(ceil((g+1)*221/64), 221)
# enthaltung.rs:118-121 volles Gitter [0, 221) -> Ursachenklasse DatenReichenNicht -> MoreData (proposal.rs:469, 709) -> kein q (proposal.rs:776-779)
# HoerMarkierung.h:62-66 rbjAlphaBw: alpha = sin(w0)*sinh(ln2/2*BW*w0/sin(w0))  -> analog 1/Q = 2 sinh(ln2/2*BW)
import math
from collections import Counter

B, G = 221, 64
ceil = lambda x: (x + G - 1) // G
groessen = []
for g in range(G):
    von = ceil(g * B); bis = min(ceil((g + 1) * B), B); bis = max(bis, von + 1)
    groessen.append(bis - von)
print("Bandzahl je Gruppe (Befundintervall):", dict(Counter(groessen)))

def q_heur(n): return min(max(24.0 / max(n, 1), 0.1), 20.0)
def q_rbj(n):
    bw = n / 24.0
    return 1.0 / (2.0 * math.sinh(math.log(2) / 2 * bw))

for n in sorted(set(groessen)) + [161, 221]:
    qh, qr = q_heur(n), q_rbj(n)
    # Bandbreite (Oktaven, RBJ-Definition) die der Heuristik-Q entspricht
    bw_h = 2 / math.log(2) * math.asinh(1 / (2 * qh))
    print(f"  n = {n:3d} Baender = {n/24:5.3f} Okt: q_heur {qh:6.3f}  RBJ-konsistent {qr:6.3f}  Verhaeltnis {qh/qr:5.3f}; "
          f"q_heur entspricht {bw_h:5.3f} Okt ({bw_h/(n/24):4.2f}-fach breiter); Grenzen [{qh/2:.3f}, {qh*2:.3f}]"
          f"{'  < 0,15 (DSP-Vertrag min)' if qh < 0.15 else ''}")
print("q < 0,15 erst ab n > 160 Baendern =", 160 / 24, "Oktaven")
