import json, pathlib
K = pathlib.Path("eq-copilot/fixtures/p4-korpus")
klassen = {n: json.loads((K/f"{n}.json").read_text(encoding="utf-8"))
           for n in ("referenz","zeitachse","vergleich","stereo")}
alle = [f for k in klassen.values() for f in k["faelle"]]

def kenn(faelle, unsicher):
    stark = [f for f in faelle if f["aussage"]=="stark"]
    enth  = [f for f in faelle if f["aussage"]=="unsicher"]
    passt = lambda f: True if f["aussage"]=="unsicher" else f["wahrheit"] not in unsicher
    fs = [f for f in stark if not passt(f)]
    fw = [f for f in faelle if f["aussage"]=="schwach" and not passt(f)]
    moegl = len([f for f in faelle if f["wahrheit"] not in
                 ("unbekannt","unvergleichbar","nicht_kausal","nicht_exakt")])
    richtig = len(stark)-len(fs)
    beh = [f for f in faelle if f["aussage"]!="unsicher"]
    n=len(beh)
    brier = sum((float(f["konfidenz"])-(1.0 if passt(f) else 0.0))**2 for f in beh)/n if n else 0.0
    mk = sum(float(f["konfidenz"]) for f in beh)/n if n else 0.0
    tq = sum(1 for f in beh if passt(f))/n if n else 0.0
    return dict(n=len(faelle), stark=len(stark), fs=len(fs), fw=len(fw),
                fs_namen=[f["fall"] for f in fs], fw_namen=[f["fall"] for f in fw],
                prec=richtig/len(stark) if stark else 1.0,
                rec=richtig/moegl if moegl else 1.0,
                brier=brier, kal=abs(mk-tq),
                cov=(len(faelle)-len(enth))/len(faelle))

HEUTE=("unbekannt",)
R1=("unbekannt","unvergleichbar","nicht_kausal","nicht_exakt")
for name, u in (("HEUTE _passt", HEUTE), ("R1 UNSICHERE_WAHRHEITEN", R1)):
    g = kenn(alle, u)
    print(f"--- {name} ---")
    print(f"  gesamt: falsche_starke={g['fs']} falsche_schwache={g['fw']} {g['fw_namen']}")
    print(f"  brier={g['brier']:.4f} (Gate <0.25)  kalibrierung={g['kal']:.4f} (Gate <=0.15)  coverage={g['cov']:.2f} (Gate >=0.5)")
    for kn in sorted(klassen):
        k = kenn(klassen[kn]["faelle"], u)
        print(f"  {kn:<10} fs={k['fs']} fw={k['fw']} prec={k['prec']:.2f} rec={k['rec']:.2f} brier={k['brier']:.4f} cov={k['cov']:.2f}")

# C6-Falsifikation: gate6-Fall auf stark, mit heutigem _passt
print("\n--- C6-Falsifikation: gate6_incomparable_never_gets_a_strong_winner -> stark (heutiges _passt) ---")
mut = [dict(f) for f in alle]
for f in mut:
    if f["fall"]=="gate6_incomparable_never_gets_a_strong_winner":
        f["aussage"]="stark"; f["konfidenz"]=0.95
g = kenn(mut, HEUTE)
print(f"  falsche_starke={g['fs']} -> Gate {'ROT' if g['fs']>0 else 'GRUEN'} | recall={g['rec']:.4f} (>1 ist unmoeglich)")
g2 = kenn(mut, R1)
print(f"  unter R1: falsche_starke={g2['fs']} {g2['fs_namen']} -> Gate {'ROT' if g2['fs']>0 else 'GRUEN'}")

# recall > 1 zeigen: vergleich-Klasse, alle vier unsicheren auf stark
print("\n--- recall-Riegel: alle sechs vergleich-Faelle auf stark (heutiges _passt) ---")
mv = [dict(f) for f in klassen["vergleich"]["faelle"]]
for f in mv: f["aussage"]="stark"
k = kenn(mv, HEUTE)
print(f"  falsche_starke={k['fs']} precision={k['prec']:.2f} recall={k['rec']:.4f}")
