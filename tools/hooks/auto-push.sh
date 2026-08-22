#!/usr/bin/env bash
# PostToolUse (Bash|PowerShell): pusht selbsttaetig, sobald der Branch dem Remote
# voraus ist.
#
# User 22.08.2026: "wie funktioniert das commiten und pushen hier, ich möchte
# dass das so automatisch wie möglich ist". CLAUDE.md verlangt ohnehin "nach
# Commits pushen", weil der User wechselnd an Desktop und Laptop arbeitet — bis
# heute war das aber eine Absicht, die jede Session neu einhalten musste. Ein
# vergessener Push kostet keinen Fehler, sondern einen halben Arbeitstag auf dem
# falschen Rechner.
#
# Ausloeser ist NICHT der Text des Befehls, sondern der gemessene Zustand:
# liegen Commits vor origin/master, gehen sie raus. Damit greift es auch bei
# Commits aus PowerShell, aus einem Skript oder aus einer parallelen Session —
# committet ist committet.
#
# Selbstbegrenzung: der teure Teil ist der Netzzugriff, der billige die Frage
# "gibt es ueberhaupt etwas?". Die Frage ist rein lokal (rev-list) und laeuft
# nach jedem Bash-Aufruf; gepusht wird nur, wenn sie ja sagt. Scheitert ein
# Push, merkt sich der Hook den HEAD und schweigt, bis ein neuer Commit da ist —
# sonst wiederholte er den Fehlschlag bei jedem folgenden Befehl.

set -u

STDIN_JSON=$(cat 2>/dev/null || printf '')
SESSION_ID=$(printf '%s' "$STDIN_JSON" | grep -o '"session_id":[[:space:]]*"[^"]*"' | head -n1 | sed 's/.*"\([^"]*\)"$/\1/')

NAK="${CLAUDE_PROJECT_DIR:-$HOME/Projekte/Nakama}"
git -C "$NAK" rev-parse --git-dir >/dev/null 2>&1 || exit 0

# Kein Push mitten in Merge/Rebase und nicht von einem losgeloesten HEAD.
GITDIR=$(git -C "$NAK" rev-parse --git-dir 2>/dev/null)
[ -e "$GITDIR/MERGE_HEAD" ] && exit 0
[ -d "$GITDIR/rebase-merge" ] || [ -d "$GITDIR/rebase-apply" ] && exit 0
BRANCH=$(git -C "$NAK" rev-parse --abbrev-ref HEAD 2>/dev/null)
[ -z "$BRANCH" ] || [ "$BRANCH" = "HEAD" ] && exit 0

VORAUS=$(git -C "$NAK" rev-list --count "@{u}..HEAD" 2>/dev/null) || exit 0
case "${VORAUS:-0}" in ''|*[!0-9]*|0) exit 0 ;; esac

HEAD_SHA=$(git -C "$NAK" rev-parse --short HEAD 2>/dev/null)
MARKER="${NAKAMA_SESSIONS_DIR:-$HOME/.claude/sessions}/${SESSION_ID:-ohne-id}.push-fehler"
if [ -f "$MARKER" ] && [ "$(cat "$MARKER" 2>/dev/null)" = "$HEAD_SHA" ]; then
  exit 0   # dieser Stand ist schon einmal gescheitert — nicht bei jedem Befehl neu versuchen
fi

AUSGABE=$(cd "$NAK" && GIT_TERMINAL_PROMPT=0 timeout 90 git push 2>&1)
CODE=$?

if [ "$CODE" -eq 0 ]; then
  rm -f "$MARKER" 2>/dev/null
  TEXT="[auto-push] $VORAUS Commit(s) nach $BRANCH gepusht ($HEAD_SHA). Der zweite Rechner sieht den Stand."
else
  mkdir -p "$(dirname "$MARKER")" 2>/dev/null
  printf '%s' "$HEAD_SHA" > "$MARKER" 2>/dev/null
  TEXT="[auto-push] FEHLGESCHLAGEN (Exit $CODE) — $VORAUS Commit(s) liegen weiter nur lokal.
$(printf '%s' "$AUSGABE" | tail -4)
Naechster Schritt bei 'non-fast-forward': git pull --rebase, Konflikte loesen, dann pusht der Hook beim naechsten Commit von selbst. Bis dahin schweigt er fuer diesen Stand."
fi

PYTHON=""
for cand in python3 python py; do
  if command -v "$cand" >/dev/null 2>&1; then PYTHON="$cand"; break; fi
done
if [ -z "$PYTHON" ]; then
  printf '%s\n' "$TEXT"
  exit 0
fi
HOOK_CONTEXT="$TEXT" "$PYTHON" - <<'PYEOF'
import json, os
print(json.dumps({
    "hookSpecificOutput": {
        "hookEventName": "PostToolUse",
        "additionalContext": os.environ.get("HOOK_CONTEXT", ""),
    }
}))
PYEOF
exit 0
