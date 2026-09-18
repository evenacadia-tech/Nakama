"""Summarize measured callbacks; no product formulas or generated measurements."""
from pathlib import Path
import csv
import json
import math
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).parent
out = {}
for name in ("stereo48k64", "stereo96k64", "stereo48k_mixed", "mono48k64", "offline48k333"):
    with (root / (name + ".csv")).open(newline="", encoding="utf-8-sig") as f:
        rows = list(csv.DictReader(f))
    groups = {}
    for n in sorted({int(r["frames"]) for r in rows}):
        selected = [r for r in rows if int(r["frames"]) == n]
        durations = sorted(float(r["chain_us"]) for r in selected)
        ratios = sorted(float(r["chain_us"]) / float(r["deadline_us"]) for r in selected)
        groups[n] = dict(count=len(selected), deadline_us=float(selected[0]["deadline_us"]),
                         p50_us=durations[math.ceil(.50 * (len(durations)-1))],
                         p99_us=durations[math.ceil(.99 * (len(durations)-1))],
                         maximum_us=max(durations),
                         maximum_ratio=max(ratios),
                         service_overruns=sum(float(r["chain_us"]) > float(r["deadline_us"]) for r in selected),
                         max_start_lateness_us=max(float(r["late_start_us"]) for r in selected))
    out[name] = groups
with (root / "resources.csv").open(newline="", encoding="utf-8-sig") as f:
    resource_rows = list(csv.DictReader(f))
out["resources"] = resource_rows
text = json.dumps(out, indent=2)
(root / "summary.json").write_text(text + "\n", encoding="utf-8")
print(text)
