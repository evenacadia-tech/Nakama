"""Validate Phase 16 documentation/evidence only; never run or repair product code.

Run from the repository with Python 3.13. --draft checks the proposed append at
BEFUNDE's actual link base before the byte-preserving append is performed.
The isolated source/build copy is required for the recorded binary/hash checks.
"""
from __future__ import annotations

import argparse
import collections
import datetime
import hashlib
import importlib.util
import json
import re
import subprocess
import sys
import tomllib
from pathlib import Path
from urllib.parse import unquote

sys.dont_write_bytecode = True
EVIDENCE = Path(__file__).resolve().parent
REPO = EVIDENCE.parents[4]
AUDIT = EVIDENCE.parent.parent
ROH = EVIDENCE.parent
SNAPSHOT = Path(r"C:\na-audit16-aff2d818")
HEAD = "aff2d8188f33a9525aec1869449773bfdbe305a6"
OLD_HEAD = "e008811efa7650f3e0cc2332cc21f9e268cdd125"
ORIGINAL_SIZE = 243364
ORIGINAL_SHA = "3d20a6e7ab6a1e19d60eb5cdb6ade378daa60242b62a2c96e95555d4065e7b7d"
PRODUCT = ["broker", "eq-copilot", "tools"]


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def file_sha(path: Path) -> str:
    return sha(path.read_bytes())


def git(root: Path, *args: str, binary: bool = False):
    data = subprocess.check_output(["git", "-C", str(root), *args])
    return data if binary else data.decode("utf-8").strip()


def read_json(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--draft", type=Path)
    args = parser.parse_args()
    output = EVIDENCE / ("entwurfspruefung.json" if args.draft else "abschluss.json")
    # A partial/failed run must not leave a previous positive report in place.
    output.write_text('{"audit_document_integrity_ok": false, "status": "running"}\n', encoding="utf-8")
    checks: list[dict] = []
    details: dict = {}

    def check(name: str, ok: bool, **info):
        checks.append({"name": name, "ok": bool(ok), **info})

    try:
        original = git(REPO, "show", HEAD + ":docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md", binary=True)
        check("original_git_bytes", len(original) == ORIGINAL_SIZE and sha(original) == ORIGINAL_SHA,
              bytes=len(original), sha256=sha(original))
        current = (AUDIT / "BEFUNDE.md").read_bytes()
        if args.draft:
            check("live_document_still_original_before_append", current == original)
            appended = args.draft.read_bytes()
            proposed = original + appended
        else:
            proposed = current
            appended = current[ORIGINAL_SIZE:]
        check("historical_prefix_byte_equal", proposed[:ORIGINAL_SIZE] == original)
        text = appended.decode("utf-8")
        old_text = original.decode("utf-8-sig")
        check("phase_16_was_free", re.search(r"^## Phase 16\b", old_text, re.M) is None)
        check("exactly_one_phase_16_header", len(re.findall(r"^## Phase 16\b", text, re.M)) == 1)
        details["document"] = {"original_bytes": ORIGINAL_SIZE, "original_sha256": ORIGINAL_SHA,
                               "append_bytes": len(appended), "append_sha256": sha(appended),
                               "total_bytes": len(proposed), "total_sha256": sha(proposed)}

        for label, root in [("live", REPO), ("snapshot", SNAPSHOT)]:
            head = git(root, "rev-parse", "HEAD")
            tracked_product_diff = git(root, "diff", HEAD, "--", *PRODUCT)
            check(label + "_product_unchanged", tracked_product_diff == "")
            details[label + "_head"] = head
        check("snapshot_head", details["snapshot_head"] == HEAD)
        check("no_product_diff_since_phase15", git(REPO, "diff", OLD_HEAD, HEAD, "--", *PRODUCT) == "")
        base = read_json(EVIDENCE / "ausgangslage.json")
        trees = git(REPO, "ls-tree", HEAD, "--", *PRODUCT)
        check("baseline_tree_objects", trees == base["product_tree_objects"])
        details["product_tree_objects"] = trees

        own_prefix = "docs/audits/2026-09-15-tiefenaudit/"
        def owned(path: str) -> bool:
            return path == own_prefix + "BEFUNDE.md" or path.startswith(own_prefix + "roh/phase-16")
        changed = git(REPO, "diff", "--name-only", "HEAD").splitlines()
        staged = git(REPO, "diff", "--cached", "--name-only").splitlines()
        check("only_owned_tracked_changes", all(owned(p) for p in changed), paths=changed)
        check("only_owned_staged_changes", all(owned(p) for p in staged), paths=staged)
        status = git(REPO, "status", "--short", "--untracked-files=normal")
        extra = [line[3:] for line in status.splitlines() if line.startswith("?? ") and not owned(line[3:])]
        check("foreign_untracked_entries_preserved", set(extra) == {"briefing-hub/", "nimbalyst-local/"}, entries=extra)
        details["git_status"] = status

        new_rows = re.findall(r"^\| (T3-16-\d\d) \| (\w+) \| ([^|]+) \| ([^|]+) \|", text, re.M)
        expected_ids = {f"T3-16-{n:02}" for n in range(1, 5)}
        check("new_finding_ids_exact", len(new_rows) == 4 and {r[0] for r in new_rows} == expected_ids)
        severities = dict(collections.Counter(r[1] for r in new_rows))
        categories = dict(collections.Counter(r[3].strip() for r in new_rows))
        check("new_severity_totals", severities == {"high": 1, "medium": 3}, counts=severities)
        check("new_category_totals", categories == {"DEFEKT": 2, "LÜCKE": 2}, counts=categories)
        counts = text.split("### Zähler dieses Nachtrags", 1)[1].split("### Aktuelle Arbeitsübersicht", 1)[0]
        check("new_count_table", all(row in counts for row in [
            "| critical | 0 | 0 | 0 | 0 |", "| high | 1 | 0 | 0 | 1 |",
            "| medium | 1 | 2 | 0 | 3 |", "| low | 0 | 0 | 0 | 0 |", "| Gesamt | 2 | 2 | 0 | 4 |"
        ]))
        old_ids = set(re.findall(r"^\| (T3-\d\d-\d\d) \|", old_text, re.M))
        excluded = {f"T3-12-{n:02}" for n in range(1, 6)}
        wanted = (old_ids - excluded) | expected_ids
        fable = text.split("### Aktuelle Arbeitsübersicht für Fable", 1)[1]
        bundle_rows = [l for l in fable.splitlines() if l.startswith("| **")]
        bundle_ids = [i for l in bundle_rows for i in re.findall(r"T3-\d\d-\d\d", l.split("|")[1])]
        check("fable_covers_all_nonsecurity_ids_once", set(bundle_ids) == wanted and len(bundle_ids) == len(wanted),
              historical_ids=len(old_ids), nonsecurity_and_new=len(wanted), rows=len(bundle_rows),
              missing=sorted(wanted-set(bundle_ids)), extra=sorted(set(bundle_ids)-wanted),
              duplicates=sorted(k for k,v in collections.Counter(bundle_ids).items() if v != 1))
        check("security_ids_excluded_from_work_bundles", not (set(bundle_ids) & excluded))
        expected_fields = {f"F{n:02}" for n in range(1, 20)}
        check("central_19_fields", set(re.findall(r"^\| (F\d\d)\b", text, re.M)) == expected_fields)
        coverage = (ROH / "phase-16-abdeckung.md").read_text(encoding="utf-8")
        check("raw_matrix_19_fields", set(re.findall(r"^\| (F\d\d)\b", coverage, re.M)) == expected_fields)
        packets = ["a-verdrahtung", "b-verbindungen", "c-persistenz", "d-dsp", "e-last-lebenslauf", "f-gegenpruefung"]
        check("six_packet_reports", all((ROH / f"phase-16-{name}.md").is_file() for name in packets))

        spec = importlib.util.spec_from_file_location("audit16_dokuriegel", REPO / "tools/plan/dokuriegel.py")
        mod = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(mod)
        docs = [(AUDIT / "BEFUNDE.md", text)] + [(p, p.read_text(encoding="utf-8")) for p in sorted(ROH.glob("phase-16*.md"))]
        docs += [(p, p.read_text(encoding="utf-8")) for p in sorted(EVIDENCE.rglob("*.md"))]
        broken_links = []
        bad_source_lines = []
        doku_errors = []
        doku_named = []
        link_count = 0
        source_refs = set()
        for doc, body in docs:
            rel = doc.relative_to(REPO)
            doku_errors += mod.tabellen_pruefen(rel, body)
            errors, named = mod.verweise_pruefen(rel, body)
            # The actual final report is created by this checker after its own checks.
            errors = [e for e in errors if not (args.draft and e.endswith("roh/phase-16-nachweise/abschluss.json"))
                      and not (args.draft and e.endswith("phase-16-nachweise/abschluss.json"))
                      and not (args.draft and e.endswith(": abschluss.json"))]
            doku_errors += errors
            doku_named += named
            for target in re.findall(r"\[[^\]\n]*\]\(([^)]+)\)", body):
                target = unquote(target.strip("<>"))
                if re.match(r"^[a-zA-Z]+://", target) or target.startswith("#"):
                    continue
                target = target.split("#", 1)[0]
                resolved = (doc.parent / target).resolve()
                link_count += 1
                if not resolved.exists() and not (args.draft and resolved == EVIDENCE / "abschluss.json"):
                    broken_links.append({"doc": str(rel), "target": target})
            for path, first, last in re.findall(r"\b((?:broker|eq-copilot|tools|docs)/[^\s`|;:()<>]+\.(?:cpp|h|rs|md|json|ps1|py)):(\d+)(?:-(\d+))?", body):
                key = (path, int(first), int(last or first))
                source_refs.add(key)
                p = REPO / path
                if not p.is_file() or not (1 <= key[1] <= key[2] <= len(p.read_bytes().splitlines())):
                    bad_source_lines.append({"doc": str(rel), "source": key})
        check("all_new_markdown_links_exist", not broken_links, count=link_count, missing=broken_links)
        check("explicit_source_line_references_exist", not bad_source_lines, count=len(source_refs), invalid=bad_source_lines)
        check("dokuriegel_new_content", not doku_errors, errors=doku_errors, marked=doku_named)
        (EVIDENCE / "dokuriegel.txt").write_text("\n".join(doku_errors + doku_named + [f"Dokuriegel new content: {len(doku_errors)} Befund(e)"]) + "\n", encoding="utf-8")

        manifest_mismatches = []
        product_hash_count = 0
        eol_only_differences = {}
        def hash_check(path: Path, expected: str):
            if not path.is_file() or file_sha(path).lower() != expected.lower():
                manifest_mismatches.append(str(path))
        def product_hash_check(relative: str, expected: str):
            # The manifests describe actual bytes compiled in the isolated copy.
            # Git's Windows checkout may use CRLF where the live checkout uses LF.
            # Permit exactly that equivalence, never arbitrary whitespace changes.
            snapshot_file, live_file = SNAPSHOT / relative, REPO / relative
            hash_check(snapshot_file, expected)
            if not snapshot_file.is_file() or not live_file.is_file():
                manifest_mismatches.append(str(live_file))
                return
            a, b = snapshot_file.read_bytes(), live_file.read_bytes()
            if a == b:
                return
            if a.replace(b"\r\n", b"\n") == b.replace(b"\r\n", b"\n"):
                eol_only_differences[relative] = {
                    "snapshot_sha256": sha(a), "live_sha256": sha(b),
                    "lf_normalized_sha256": sha(a.replace(b"\r\n", b"\n"))}
            else:
                manifest_mismatches.append(str(live_file))
        manifests = {key:read_json(EVIDENCE / name) for key,name in {
            "a":"a/quellmanifest.json", "b":"b/manifest.json", "c":"c/manifest-reviewed.json",
            "d":"d/manifest.json", "e":"e/manifest.json", "er":"e/resources-manifest.json"}.items()}
        for key, source_field in [("a","sources"),("b","sources_sha256"),("c","sources_sha256"),("d","sourcesSha256")]:
            for path, expected in manifests[key][source_field].items():
                product_hash_check(path, expected)
                product_hash_count += 1
        for entry in manifests["e"]["product_sources"]:
            product_hash_check(entry["path"], entry["sha256"])
            product_hash_count += 1
        for name,expected in manifests["a"]["audit_artifacts"].items():
            hash_check(EVIDENCE / "a" / name, expected)
        for key in ["b","c"]:
            for name,expected in manifests[key]["harness_sha256"].items():
                hash_check(EVIDENCE / key / name, expected)
            hash_check(Path(manifests[key]["binary"]),manifests[key]["binary_sha256"])
        hash_check(SNAPSHOT / "build-d/Release/audit16d.exe", manifests["d"]["binarySha256"])
        for key in ["e","er"]:
            for entry in manifests[key]["evidence"]:
                hash_check(EVIDENCE / "e" / entry["name"],entry["sha256"])
            hash_check(Path(manifests[key]["binary"]),manifests[key]["binary_sha256"])
        hash_check(EVIDENCE / "c/PipeClientIsoliert.cpp",manifests["e"]["pipe_seam_sha256"])
        cmake = (EVIDENCE / "c/CMakeLists.txt").read_bytes()
        hash_check(EVIDENCE / "c/CMakeLists.txt", manifests["er"]["shared_cmake_sha256"])
        lines = cmake.splitlines(keepends=True)
        ix = next(i for i,l in enumerate(lines) if b'if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/../e/resources_probe.cpp")' in l)
        prior = b"".join(lines[:ix])
        candidates = [prior, prior.rstrip(b"\r\n") + b"\r\n", prior.rstrip(b"\r\n") + b"\n"]
        check("timing_cmake_before_resources_addition_matches", any(sha(x) == manifests["e"]["shared_cmake_sha256"] for x in candidates))
        check("recorded_source_artifact_binary_hashes", not manifest_mismatches,
              product_hash_entries=product_hash_count, mismatches=manifest_mismatches)
        details["source_identity_mode"] = "Compiled snapshot bytes must match manifests exactly; live checkout may differ only by CRLF/LF, additionally checked by git diff against audited HEAD."
        details["live_snapshot_eol_only_differences"] = eol_only_differences
        package_key = lambda x:(x["name"],x["version"],x.get("source"),x.get("checksum"))
        base_packages = {package_key(x) for x in tomllib.loads((SNAPSHOT / "broker/Cargo.lock").read_text(encoding="utf-8"))["package"]}
        for packet,name,version in [("a","nakama-phase16-audit-a","0.1.0"),("b","audit16-b","0.0.0")]:
            actual = {package_key(x) for x in tomllib.loads((EVIDENCE / packet / "Cargo.lock").read_text(encoding="utf-8"))["package"]}
            check(packet + "_locked_dependency_versions", not(base_packages-actual) and actual-base_packages == {(name,version,None,None)})

        required_markers = {
            "a/lauf-pinned.txt":["test result: ok. 4 passed"],
            "b/components-run.txt":["EXIT=0"], "b/wire-run.txt":["EXIT=0"],
            "c/ergebnisse-reviewed.txt":["checks=33 failures=0"],
            "d/ergebnisse.txt":["HARNESS_FINISHED", "EXPECTED_FAIL"],
            "e/ergebnisse.txt":["ASSERTION_FAILURES=0"],
            "e/resources-run.txt":["RESOURCE_RUN_COMPLETE=36"]}
        for rel,markers in required_markers.items():
            body=(EVIDENCE / rel).read_text(encoding="utf-8-sig")
            check("saved_result_" + rel, all(s in body for s in markers))
        ebody=(EVIDENCE / "e/ergebnisse.txt").read_text(encoding="utf-8-sig")
        epasses=len(re.findall(r"^PASS ",ebody,re.M))
        check("e_17_assertions",epasses==17 and not re.search(r"^FAIL ",ebody,re.M),count=epasses)
        esummary=manifests["e"]
        check("e_final_run_exit",esummary["run_exit_code"]==0)
        timing=read_json(EVIDENCE / "e/summary.json")
        mixed=timing["stereo48k_mixed"]
        check("e_csv_summary_limits",all(timing[key]["64"]["service_overruns"]==0 for key in ["stereo48k64","stereo96k64","mono48k64"])
              and sum(x["service_overruns"] for x in mixed.values())==93 and mixed["1"]["service_overruns"]==92)

        artifacts=[p for p in sorted(EVIDENCE.rglob("*")) if p.is_file() and p.name not in {"abschluss.json","entwurfspruefung.json"}]
        forbidden=[str(p.relative_to(EVIDENCE)) for p in artifacts if p.suffix.lower() in {".exe",".dll",".pdb",".obj",".lib",".db",".sqlite"}]
        check("only_documentary_source_and_text_evidence",not forbidden,forbidden=forbidden)
        details["artifact_sha256"]={p.relative_to(EVIDENCE).as_posix():file_sha(p) for p in artifacts}
        details["report_sha256"]={p.relative_to(AUDIT).as_posix():file_sha(p) for p,_ in docs if p.name!="BEFUNDE.md"}
        diff=subprocess.run(["git","-C",str(REPO),"diff","--check"],capture_output=True,text=True,encoding="utf-8")
        check("git_diff_check",diff.returncode==0,output=diff.stdout+diff.stderr)
        if staged:
            cached=subprocess.run(["git","-C",str(REPO),"diff","--cached","--check"],capture_output=True,text=True,encoding="utf-8")
            check("git_cached_diff_check",cached.returncode==0,output=cached.stdout+cached.stderr)
            index_mismatches=[]
            evidence_prefix=EVIDENCE.relative_to(REPO).as_posix()+"/"
            for path in staged:
                if path.startswith(evidence_prefix) and REPO/path != output:
                    if git(REPO,"show",":"+path,binary=True) != (REPO/path).read_bytes():
                        index_mismatches.append(path)
            check("staged_evidence_bytes_preserved",not index_mismatches,mismatches=index_mismatches,
                  note="Current output report is excluded because this checker is writing it; stage it again after completion.")
            index_document=git(REPO,"show",":docs/audits/2026-09-15-tiefenaudit/BEFUNDE.md",binary=True)
            check("historical_prefix_in_git_index",index_document[:ORIGINAL_SIZE]==original)
        # Include untracked documentary files; git diff alone does not see them.
        whitespace=[]
        for p,body in docs:
            for n,line in enumerate(body.splitlines(),1):
                if line.rstrip(" \t") != line:
                    whitespace.append(str(p.relative_to(REPO))+":"+str(n))
        check("new_markdown_trailing_whitespace",not whitespace,lines=whitespace)
    except Exception as exc:
        check("checker_completed",False,error=repr(exc))
    result={"captured_at_utc":datetime.datetime.now(datetime.timezone.utc).isoformat(),
            "audited_product_head":HEAD,"audit_document_integrity_ok":bool(checks) and all(c["ok"] for c in checks),
            "scope":"documentation, stored evidence and source identity only; no product tests executed by this checker; no release/host PASS",
            "draft":str(args.draft) if args.draft else None,"checks":checks,**details}
    output.write_text(json.dumps(result,ensure_ascii=False,indent=2)+"\n",encoding="utf-8")
    failures=[c for c in checks if not c["ok"]]
    print(json.dumps({"report":str(output),"checks":len(checks),"failures":failures,
                      "audit_document_integrity_ok":result["audit_document_integrity_ok"]},ensure_ascii=False))
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
