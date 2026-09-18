"""Phase-16-Herkunft: liest Git/Dateien, schreibt nur den benannten Auditordner.

Ausfuehrung: py -3.13 ausgangslage.py --repo <repo> --snapshot <isolierte-kopie>
"""
import argparse
import hashlib
import json
import platform
import subprocess
from datetime import datetime, timezone
from pathlib import Path

p = argparse.ArgumentParser()
p.add_argument('--repo', required=True, type=Path)
p.add_argument('--snapshot', required=True, type=Path)
a = p.parse_args()
def git(*args):
    return subprocess.check_output(['git', '-C', str(a.repo), *args], text=True, encoding='utf-8').strip()
rel = 'docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md'
original = (a.repo / rel).read_bytes()
backup = a.snapshot / 'audit16-original-BEFUNDE.bin'
if backup.exists():
    raise SystemExit('Originalsicherung besteht bereits; kein Ueberschreiben')
backup.write_bytes(original)
product_paths = ['broker', 'eq-copilot', 'tools', 'CMakeLists.txt']
data = {
    'utc': datetime.now(timezone.utc).isoformat(),
    'repo': str(a.repo), 'snapshot': str(a.snapshot),
    'head': git('rev-parse', 'HEAD'),
    'snapshot_head': subprocess.check_output(['git', '-C', str(a.snapshot), 'rev-parse', 'HEAD'], text=True).strip(),
    'status': git('status', '--short'),
    'historical_phase15_head': 'e008811efa7650f3e0cc2332cc21f9e268cdd125',
    'since_phase15': git('diff', '--stat', 'e008811efa7650f3e0cc2332cc21f9e268cdd125', 'HEAD'),
    'product_diff_since_phase15': git('diff', '--name-status', 'e008811efa7650f3e0cc2332cc21f9e268cdd125', 'HEAD', '--', *product_paths),
    'product_worktree_diff': git('diff', 'HEAD', '--name-status', '--', *product_paths),
    'product_tree_objects': git('ls-tree', 'HEAD', *product_paths),
    'befunde_original': {'bytes': len(original), 'sha256': hashlib.sha256(original).hexdigest(), 'lines': len(original.splitlines()), 'backup': str(backup)},
    'platform': platform.platform(), 'python': platform.python_version(),
    'tracked_instructions': git('ls-files', '**/AGENTS.md', 'AGENTS.md'),
}
out = Path(__file__).with_name('ausgangslage.json')
out.write_text(json.dumps(data, indent=2, ensure_ascii=False) + '\n', encoding='utf-8')
print(json.dumps(data, indent=2, ensure_ascii=False))
