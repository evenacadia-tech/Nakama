#!/bin/bash
# SessionStart hook: Marker mit Zeitstempel + git-HEADs schreiben, damit
# session-end-handoff.sh beim Beenden ein Delta-Handoff erzeugen kann.
#
# Liest session_id aus dem JSON-stdin. Marker ${session_id}.start unter
# ~/.claude/sessions/ trägt vier Zeilen:
#   1. Epoch-Sekunden beim Start
#   2. git-HEAD (kurz) des Nakama-Workspaces (oder "no-git")
#   3. cwd beim Start
#   4. (frei — bis 18.08.2026 FL-Studio-HEAD; Code lebt seit dem Umzug hier)

set -u

STDIN_JSON=$(cat 2>/dev/null || printf '')
SESSION_ID=$(printf '%s' "$STDIN_JSON" | grep -o '"session_id":[[:space:]]*"[^"]*"' | head -n1 | sed 's/.*"\([^"]*\)"$/\1/')
[ -z "$SESSION_ID" ] && exit 0

SESSIONS_DIR="$HOME/.claude/sessions"
mkdir -p "$SESSIONS_DIR" 2>/dev/null || exit 0

# Selbstbegrenzung: Marker abgestürzter Sessions sammeln sich sonst ewig.
find "$SESSIONS_DIR" -name '*.start' -type f -mtime +7 -delete 2>/dev/null

MARKER="$SESSIONS_DIR/${SESSION_ID}.start"
TS=$(date +%s)
HEAD_SHA=$(git -C "$PWD" rev-parse --short HEAD 2>/dev/null || printf 'no-git')
FL_SHA="entfallen-seit-umzug"

{
  printf '%s\n' "$TS"
  printf '%s\n' "$HEAD_SHA"
  printf '%s\n' "$PWD"
  printf '%s\n' "$FL_SHA"
} > "$MARKER" 2>/dev/null

exit 0
