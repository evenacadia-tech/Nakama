"""Fault-path check of the current soak oracle, using a frozen real report as input.

No broker or plugin is started. No production source is patched. The historical
report is only a control fixture; this is not a new session-soak run.
"""
from __future__ import annotations

import argparse
import contextlib
import copy
import importlib.util
import io
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parents[4]
tool = ROOT / "tools/eq-copilot/pruefe_session_soak.py"
spec = importlib.util.spec_from_file_location("soak_oracle", tool)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)

fixture_source = ROOT / "docs/beweise/roh/NAK-246-4ff6f24.md"
report_lines = [line[len("Bericht: "):] for line in fixture_source.read_text(encoding="utf-8-sig").splitlines()
                if line.startswith("Bericht: {")]
assert len(report_lines) == 1, "Expected exactly one existing real A24 report"
control = json.loads(report_lines[0])
args = argparse.Namespace(sonden=16, minuten=2, neustarts=1, langsam=.25,
                          langsam_ms=120, mutant=None)


def run(label, report):
    output = io.StringIO()
    with contextlib.redirect_stdout(output):
        code = module.urteile(report, args)
    print(f"{label}_ORACLE_EXIT={code}")
    for line in output.getvalue().splitlines():
        if "S07" in line or "ROT" in line or line == "GRUEN":
            print(line)
    return code


control_exit = run("CONTROL", control)
assert control_exit == 0, "The frozen control must pass the current oracle"
missing = module.rss_bytes(0)  # Windows cannot open PID 0; tool returns its documented failure sentinel.
print(f"UNAVAILABLE_PROCESS_RSS={missing}")
assert missing == 0
fault = copy.deepcopy(control)
for process in ("client", "broker"):
    for point in fault["speicher"][process]:
        point["rss_bytes"] = missing
fault_exit = run("MISSING_RSS", fault)
print("EXPECTED: missing memory measurements cannot certify the memory budget")
print("OBSERVED_FALSE_GREEN=" + str(fault_exit == 0))
raise SystemExit(1 if fault_exit == 0 else 0)
