#!/usr/bin/env python3
"""Tiefenaudit 3 (NAK-308): Abschluss einer Codex-Phase durch den Dirigenten.

    py -3.13 tools/pruefung/tiefenaudit_phase.py --abschluss NN [--naechste MM] [--modell M]

Prueft, dass nur die Audit-Dateien der Phase geaendert sind, liest Urteil,
Zaehler, Thread und Zeiten, zieht die Phasentabelle in BEFUNDE.md nach,
committet mit Pathspec, pusht und startet die naechste Phase abgekoppelt ueber
tools/pruefung/codex-audit-lauf.ps1. Nur Standardbibliothek. Exit 2 = Befund
(fremde Aenderung, fehlender Abschnitt), sonst 0.
"""
import argparse, collections, glob, io, os, re, subprocess, sys, tempfile

AUDIT = "docs/audits/2026-09-15-tiefenaudit"
BEF = f"{AUDIT}/BEFUNDE.md"


def sh(*a, **k):
    return subprocess.run(a, capture_output=True, text=True, encoding="utf-8", errors="replace", **k)


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--abschluss", required=True)
    p.add_argument("--naechste")
    p.add_argument("--modell", default="gpt-6-astra")
    a = p.parse_args()
    nn = a.abschluss
    status = [l for l in sh("git", "status", "--short").stdout.splitlines()
              if "briefing-hub" not in l and "nimbalyst" not in l]
    fremd = [l for l in status if not (l.endswith(BEF) or f"roh/phase-{nn}-" in l
                                       or l.endswith("tools/pruefung/tiefenaudit_phase.py"))]
    if fremd:
        print("FREMDE AENDERUNG, kein Commit:", *fremd, sep="\n  "); return 2
    s = io.open(BEF, encoding="utf-8", newline="").read()
    m = re.search(rf"^## Phase {nn} .*?$\n+^(URTEIL: .*?)$", s, re.M | re.S)
    if not m:
        print(f"Abschnitt Phase {nn} oder URTEIL fehlt in BEFUNDE.md"); return 2
    urteil = m.group(1).strip()
    z = collections.Counter(l.split("|")[2].strip() for l in s.splitlines() if l.startswith(f"| T3-{nn}-"))
    sv = ", ".join(f"{z[k]} {k}" for k in ("critical", "high", "medium", "low") if z[k]) or "keine Befunde"
    zahlen = re.search(r"DEFEKTE (\d+) · LÜCKEN (\d+) · HÄRTUNGEN (\d+)", urteil)
    kurz = (f"{zahlen[1]} Defekte, {zahlen[2]} Lücken, {zahlen[3]} Härtungen" if zahlen
            else urteil.split("—")[0].replace("URTEIL:", "").strip())
    temp = tempfile.gettempdir()
    logs = sorted(glob.glob(os.path.join(temp, f"nakama-tiefenaudit-p{nn}-*-start.log")))
    jsonls = sorted(glob.glob(os.path.join(temp, f"nakama-tiefenaudit-p{nn}-*.jsonl")))
    head_sha = re.search(r"HEAD=([0-9a-f]{40})", io.open(logs[0], encoding="utf-8").read())[1] if logs else "?"
    tid = "?"
    if jsonls:
        t = re.search(r'"thread_id":"([0-9a-f-]{8,})"', io.open(jsonls[0], encoding="utf-8", errors="replace").read(6000))
        tid = t[1][:8] if t else "?"
    zeiten = []
    for lg in logs:
        txt = io.open(lg, encoding="utf-8").read()
        st = re.search(r"START (\d{4}-\d\d-\d\d)T(\d\d:\d\d)", txt); en = re.search(r"ENDE \d{4}-\d\d-\d\dT(\d\d:\d\d)", txt)
        if st and en: zeiten.append(f"{st[2]}–{en[1]}")
    rep = glob.glob(f"{AUDIT}/roh/phase-{nn}-*.md")
    rep = [r for r in rep if not r.endswith("-auftrag.txt")]
    zeile = None
    for l in s.splitlines():
        if l.startswith(f"| {nn} | "):
            zeile = l; break
    if not zeile:
        print(f"Tabellenzeile {nn} fehlt"); return 2
    c = zeile.split("|")
    # Phasen ohne eigene Befundzeilen (Skeptiker, Synthese): Urteilstext statt Schwerezaehler.
    detail = f"{kurz} ({sv})" if zahlen else urteil.replace("URTEIL:", "").strip()
    datum = re.search(r"START (\d{4})-(\d\d)-(\d\d)", io.open(logs[0], encoding="utf-8").read()) if logs else None
    tag = f"{datum[3]}.{datum[2]}.{datum[1]}" if datum else "?"
    c[4] = f" **gelaufen** {tag} {', '.join(zeiten)}, Thread `{tid}`: {detail} "
    c[5] = f" `{head_sha[:8]}` "
    s = s.replace(zeile, "|".join(c))
    io.open(BEF, "w", encoding="utf-8", newline="").write(s)
    msg = os.path.join(temp, f"nakama-commit-p{nn}.txt")
    io.open(msg, "w", encoding="utf-8").write(
        f"Tiefenaudit 3 Phase {nn} gelaufen: {kurz}\n\n{urteil}\n\nCodex {a.modell}, Thread {tid}, HEAD {head_sha[:8]} vor und nach dem Lauf; "
        f"Zeiten {', '.join(zeiten)}; Schwere {sv}.\n\nCo-Authored-By: Claude Fable 5.1 <noreply@anthropic.com>\n"
        "Claude-Session: https://claude.ai/code/session_01NX21t2VgLcVJPWNUCGkDy3\n")
    sh("git", "add", AUDIT, "tools/pruefung/tiefenaudit_phase.py")
    sh("git", "commit", "-q", "-F", msg, "--", AUDIT, "tools/pruefung/tiefenaudit_phase.py")
    sh("git", "push", "-q", "origin", "master")
    head = sh("git", "rev-parse", "HEAD").stdout.strip()
    origin = sh("git", "rev-parse", "--short", "origin/master").stdout.strip()
    print(f"Phase {nn}: {detail}; Thread {tid}; {', '.join(zeiten)}; commit {head[:8]} origin {origin}")
    if a.naechste:
        # Start ueber Start-Process (bewaehrt); ein Popen mit DETACHED_PROCESS
        # startete am 15.09.2026 keinen Lauf (kein Startlog, kein Prozess).
        befehl = ("Start-Process pwsh -WindowStyle Hidden -WorkingDirectory (Get-Location) -ArgumentList "
                  f"'-NoProfile','-File','tools/pruefung/codex-audit-lauf.ps1','-Phase','{a.naechste}',"
                  f"'-HeadSha','{head}','-Model','{a.modell}'")
        r = sh("pwsh", "-NoProfile", "-Command", befehl)
        if r.returncode != 0:
            print("Start fehlgeschlagen:", r.stderr.strip()[-300:]); return 2
        print(f"Phase {a.naechste} gestartet auf {head} mit {a.modell}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
