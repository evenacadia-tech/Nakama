#!/usr/bin/env bash
# PostToolUse (Bash|PowerShell) + SessionStart: haelt docs/PLAN-STAND.md frisch.
#
# User 23.08.2026: "es gibt keine automatische aktualisation , das heisst er
# wird driften und somit drifted plan stand auch." Genau das schliesst dieser
# Hook: das Blatt kann hoechstens einen Commit hinter der Wahrheit liegen.
#
# Ausloeser ist NICHT der Text des Befehls, sondern der gemessene Zustand
# (dasselbe Muster wie auto-push.sh): weicht der Quellstand im Blatt vom
# gemessenen ab, oder liegt unter den Quellen etwas Uncommittetes, wird neu
# gerechnet. Damit greift es auch bei Commits aus einem Skript oder einer
# parallelen Session.
#
# 🔑 Keine Schleife: gestempelt wird der QUELLSTAND (letzter Commit, der
# docs/plan, docs/beweise oder tools/plan angefasst hat), nicht HEAD. Der
# Commit dieses Blattes beruehrt keine Quelle, also bleibt der Quellstand
# stehen und der naechste Lauf hat nichts mehr zu tun. Mit HEAD als Stempel
# committete sich der Hook endlos selbst.
#
# Der Commit nimmt AUSDRUECKLICH nur den einen Pfad (Pathspec-Form, die den
# Index gar nicht anfasst) — im Repo laufen parallele Sessions, und fremde
# uncommittete Dateien duerfen nie mitgehen.

set -u

NAK="${CLAUDE_PROJECT_DIR:-$HOME/Projekte/Nakama}"
BLATT="$NAK/docs/PLAN-STAND.md"
QUELLEN=(docs/plan docs/beweise tools/plan)

git -C "$NAK" rev-parse --git-dir >/dev/null 2>&1 || exit 0

# Nicht mitten in Merge/Rebase und nicht von einem losgeloesten HEAD.
GITDIR=$(git -C "$NAK" rev-parse --git-dir 2>/dev/null)
[ -e "$GITDIR/MERGE_HEAD" ] && exit 0
{ [ -d "$GITDIR/rebase-merge" ] || [ -d "$GITDIR/rebase-apply" ]; } && exit 0
BRANCH=$(git -C "$NAK" rev-parse --abbrev-ref HEAD 2>/dev/null)
{ [ -z "$BRANCH" ] || [ "$BRANCH" = "HEAD" ]; } && exit 0

PYTHON=""
for cand in py python3 python; do
  if command -v "$cand" >/dev/null 2>&1; then PYTHON="$cand"; break; fi
done
[ -z "$PYTHON" ] && exit 0
[ "$PYTHON" = "py" ] && set -- -3.13 || set --

# --- Die Messung: muss ueberhaupt gerechnet werden? -------------------------
QUELLSTAND=$(git -C "$NAK" log -1 --format=%h -- "${QUELLEN[@]}" 2>/dev/null)
GESTEMPELT=$(sed -n 's/^<!-- quellstand: \([0-9a-f]*\) -->$/\1/p' "$BLATT" 2>/dev/null | head -n1)
SCHMUTZIG=$(git -C "$NAK" status --porcelain -- "${QUELLEN[@]}" 2>/dev/null)

if [ -f "$BLATT" ] && [ -n "$QUELLSTAND" ] && [ "$GESTEMPELT" = "$QUELLSTAND" ] && [ -z "$SCHMUTZIG" ]; then
  exit 0     # Blatt ist auf dem Stand der Quellen — nichts zu tun
fi

AUSGABE=$(cd "$NAK" && "$PYTHON" "$@" tools/plan/planstand.py 2>&1)
CODE=$?

# Exit 4 = eine Urteilsmarke ist unlesbar. Das Blatt ist trotzdem geschrieben
# (und traegt die Warnung sichtbar), aber es muss jemand ansehen.
GEAENDERT=$(git -C "$NAK" status --porcelain -- docs/PLAN-STAND.md 2>/dev/null)
if [ -z "$GEAENDERT" ]; then
  [ "$CODE" -eq 4 ] && printf '%s\n' "[planstand] $AUSGABE"
  exit 0
fi

# Nur committen, wenn die Quellen selbst sauber sind: sonst haengt das Blatt an
# einem Stand, den es committet behauptet, aber niemand festgehalten hat.
if [ -n "$SCHMUTZIG" ]; then
  TEXT="[planstand] Blatt neu gerechnet, aber NICHT committet — unter ${QUELLEN[*]} liegt Uncommittetes.
$AUSGABE"
else
  MELD=$(cd "$NAK" && git commit -q -m "Planstand neu gerechnet (Quellstand $QUELLSTAND)" -- docs/PLAN-STAND.md 2>&1)
  if [ $? -eq 0 ]; then
    TEXT="[planstand] $AUSGABE — als eigener Commit festgehalten."
  else
    TEXT="[planstand] Blatt neu gerechnet, Commit fehlgeschlagen: $(printf '%s' "$MELD" | tail -2)"
  fi
fi

HOOK_CONTEXT="$TEXT" "$PYTHON" "$@" - <<'PYEOF'
import json, os
print(json.dumps({
    "hookSpecificOutput": {
        "hookEventName": "PostToolUse",
        "additionalContext": os.environ.get("HOOK_CONTEXT", ""),
    }
}))
PYEOF
exit 0
