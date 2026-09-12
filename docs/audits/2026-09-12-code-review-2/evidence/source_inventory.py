"""Audit inventory and source fingerprint; no coverage claim or code mutation."""
from __future__ import annotations

import hashlib
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parents[4]
PREFIXES = (
    "eq-copilot/plugin/", "eq-copilot/schemas/", "eq-copilot/identity/",
    "eq-copilot/install/", "eq-copilot/cmake/", "broker/src/", "broker/tests/",
    "tools/eq-copilot/", "tools/plan/",
)
EXPLICIT = {"eq-copilot/CMakeLists.txt", "broker/Cargo.toml", "broker/Cargo.lock",
            "tools/beweise.ps1", "tools/beweise-roh.ps1"}
GENERATED = ("/generiert/", "/generated/", "/assets/", "/binarydata/")
SUFFIXES = {".cpp", ".h", ".hpp", ".rs", ".py", ".ps1", ".json", ".fbs", ".cmake"}


def snapshot():
    names = subprocess.check_output(["git", "ls-files", "-z"], cwd=ROOT).decode().split("\0")
    result = []
    for name in sorted(names):
        if name not in EXPLICIT and not (
            name.startswith(PREFIXES)
            and (pathlib.PurePosixPath(name).suffix in SUFFIXES or name.endswith("CMakeLists.txt"))
            and not any(part in name.lower() for part in GENERATED)
        ):
            continue
        content = (ROOT / name).read_bytes()
        result.append({"path": name, "bytes": len(content),
                       "lines": len(content.splitlines()),
                       "sha256": hashlib.sha256(content).hexdigest()})
    return {"revision": subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT).decode().strip(),
            "scope": "Tracked selected first-party source, contracts, tooling, tests; inventory is not deep-review coverage",
            "files": result, "file_count": len(result), "line_count": sum(f["lines"] for f in result)}


if __name__ == "__main__":
    current = snapshot()
    if len(sys.argv) == 3 and sys.argv[1] == "--verify":
        old = json.loads(pathlib.Path(sys.argv[2]).read_text(encoding="utf-8"))
        before = {f["path"]: f["sha256"] for f in old["files"]}
        after = {f["path"]: f["sha256"] for f in current["files"]}
        changed = [p for p in sorted(before.keys() | after.keys()) if before.get(p) != after.get(p)]
        print(json.dumps({"unchanged": not changed, "changed": changed,
                          "checked_files": len(after), "revision": current["revision"]}, indent=2))
        sys.exit(bool(changed))
    print(json.dumps(current, ensure_ascii=False, indent=2))
