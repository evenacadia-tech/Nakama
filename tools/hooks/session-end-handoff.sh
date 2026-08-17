#!/bin/bash
# SessionEnd hook: Auto-Handoff nach docs/handoffs/ schreiben — das Git-Delta
# der Session über BEIDE Repos (Nakama-Workspace + FL-Studio, wo der Code
# lebt). SessionEnd kann nicht blocken (die Session stirbt) — rein
# bash-generiertes Artefakt, keine Modell-Intelligenz. Das Modell kann
# während der Session ein reicheres Memo ergänzen.
#
# Liest session_id + reason aus dem JSON-stdin, Marker aus SessionStart
# (4 Zeilen: Start-Epoch · Nakama-HEAD · cwd · FL-Studio-HEAD).
# Immer Exit 0 — SessionEnd-Fehler dürfen keine User-Fehler zeigen.

set -u

STDIN_JSON=$(cat 2>/dev/null || printf '')
SESSION_ID=$(printf '%s' "$STDIN_JSON" | grep -o '"session_id":[[:space:]]*"[^"]*"' | head -n1 | sed 's/.*"\([^"]*\)"$/\1/')
REASON=$(printf '%s' "$STDIN_JSON" | grep -o '"reason":[[:space:]]*"[^"]*"' | head -n1 | sed 's/.*"\([^"]*\)"$/\1/')

[ -z "$SESSION_ID" ] && exit 0
[ -z "$REASON" ]     && REASON="unknown"

# Nur aus dem Nakama-Workspace heraus schreiben — sonst landet das Handoff
# im falschen Baum.
REPO_ROOT=$(git -C "$PWD" rev-parse --show-toplevel 2>/dev/null || printf '')
[ -z "$REPO_ROOT" ] && exit 0
case "$REPO_ROOT" in
  *[Nn]akama*) ;;
  *) exit 0 ;;
esac

FL="$HOME/FL-Studio"
HANDOFF_DIR="$REPO_ROOT/docs/handoffs"
mkdir -p "$HANDOFF_DIR" 2>/dev/null || exit 0

MARKER="$HOME/.claude/sessions/${SESSION_ID}.start"
NOW=$(date +%s)
DATE=$(date +%Y-%m-%d)
TIME=$(date +%H%M)
SID_SHORT=$(printf '%s' "$SESSION_ID" | cut -c1-8)
OUT="$HANDOFF_DIR/auto-handoff-${DATE}-${TIME}-${SID_SHORT}.md"

START_TS=""; START_SHA=""; START_FL_SHA=""
if [ -f "$MARKER" ]; then
  START_TS=$(sed -n '1p' "$MARKER")
  START_SHA=$(sed -n '2p' "$MARKER")
  START_FL_SHA=$(sed -n '4p' "$MARKER")
fi

END_SHA=$(git -C "$REPO_ROOT" rev-parse --short HEAD 2>/dev/null || printf 'unknown')
END_FL_SHA=$(git -C "$FL" rev-parse --short HEAD 2>/dev/null || printf 'unknown')

DURATION=""
if [ -n "$START_TS" ]; then
  MINS=$(( (NOW - START_TS) / 60 ))
  DURATION="${MINS} min"
fi

repo_delta() {
  # $1 = Repo-Pfad, $2 = Start-SHA, $3 = End-SHA, $4 = optionaler Pathspec
  local repo="$1" start="$2" end="$3" spec="${4:-}"
  if [ -n "$start" ] && [ "$start" != "no-git" ] && [ "$start" != "$end" ]; then
    printf '```\n'
    if [ -n "$spec" ]; then
      git -C "$repo" log --oneline "${start}..HEAD" -- $spec 2>/dev/null || printf '(Commits nicht lesbar)\n'
    else
      git -C "$repo" log --oneline "${start}..HEAD" 2>/dev/null || printf '(Commits nicht lesbar)\n'
    fi
    printf '```\n\n```\n'
    if [ -n "$spec" ]; then
      git -C "$repo" diff --stat "${start}..HEAD" -- $spec 2>/dev/null || printf '(Diff nicht lesbar)\n'
    else
      git -C "$repo" diff --stat "${start}..HEAD" 2>/dev/null || printf '(Diff nicht lesbar)\n'
    fi
    printf '```\n\n'
  else
    printf '_Keine neuen Commits._\n\n'
  fi
}

{
  printf '# Auto-Handoff — %s %s\n\n' "$DATE" "$(date +%H:%M)"
  printf '**Session:** `%s`  \n' "$SESSION_ID"
  printf '**Grund:** %s  \n' "$REASON"
  [ -n "$DURATION" ] && printf '**Dauer:** %s  \n' "$DURATION"
  printf '**Nakama-HEAD:** `%s` → `%s`  \n' "${START_SHA:-?}" "$END_SHA"
  printf '**FL-Studio-HEAD:** `%s` → `%s`  \n\n' "${START_FL_SHA:-?}" "$END_FL_SHA"

  printf '## Nakama-Workspace (Wissen/Kontext)\n\n'
  repo_delta "$REPO_ROOT" "$START_SHA" "$END_SHA" ""

  printf '## FL-Studio-Repo (Code, Scope eq-copilot + Broker)\n\n'
  repo_delta "$FL" "$START_FL_SHA" "$END_FL_SHA" "eq-copilot plugin-hub-app"

  STATUS=$(git -C "$FL" status --porcelain -- eq-copilot plugin-hub-app 2>/dev/null)
  if [ -n "$STATUS" ]; then
    printf '## Uncommitted im FL-Studio-Repo bei Session-Ende\n\n'
    printf '```\n%s\n```\n\n' "$STATUS"
  fi

  printf -- '---\n_Auto-generiert von `session-end-handoff.sh`. Reichere Memos: Memory `project_session_*.md`._\n'
} > "$OUT" 2>/dev/null

[ -f "$MARKER" ] && rm -f "$MARKER" 2>/dev/null

# Rotation nach ANZAHL, nicht Alter: nur die neuesten 5 behalten.
HANDOFF_KEEP=5
ls -1t "$HANDOFF_DIR"/auto-handoff-*.md 2>/dev/null \
  | tail -n "+$((HANDOFF_KEEP + 1))" \
  | while IFS= read -r stale; do rm -f "$stale" 2>/dev/null; done

exit 0
