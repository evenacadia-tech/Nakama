#!/bin/bash
# Stop hook: verhindert, dass eine Session mit neuen Commits endet, ohne den
# Nakama-Hub (docs/hub/hub.json) angefasst zu haben — hoechstens EINMAL je
# Session (Marker), damit daraus nie eine Endlosschleife wird.
#
# Blockt (JSON decision=block), wenn seit Sessionstart Commits entstanden sind
# und hub.json seit Sessionstart nicht veraendert wurde.
# Still in allen anderen Faellen, bei stop_hook_active=true und ohne Marker.
# Sessionstart-Marker schreibt tools/hooks/session-start-marker.sh
# (Zeile 1 Epoch, Zeile 2 HEAD). Env-Ueberschreibungen nur fuer den Test.

set -u
STDIN_JSON=$(cat 2>/dev/null || printf '')
SESSION_ID=$(printf '%s' "$STDIN_JSON" | grep -o '"session_id":[[:space:]]*"[^"]*"' | head -n1 | sed 's/.*"\([^"]*\)"$/\1/')
[ -z "$SESSION_ID" ] && exit 0
printf '%s' "$STDIN_JSON" | grep -q '"stop_hook_active":[[:space:]]*true' && exit 0

NAK="${CLAUDE_PROJECT_DIR:-$HOME/Projekte/Nakama}"
SESSIONS_DIR="${NAKAMA_SESSIONS_DIR:-$HOME/.claude/sessions}"
MARKER="$SESSIONS_DIR/${SESSION_ID}.start"
NAG="$SESSIONS_DIR/${SESSION_ID}.hub-nag"
HUB_JSON="$NAK/docs/hub/hub.json"

[ -f "$MARKER" ] || exit 0
[ -f "$NAG" ] && exit 0
[ -f "$HUB_JSON" ] || exit 0

START_TS=$(sed -n '1p' "$MARKER")
START_SHA=$(sed -n '2p' "$MARKER")
case "$START_TS" in ''|*[!0-9]*) exit 0 ;; esac

HEAD_SHA=$(git -C "$NAK" rev-parse --short HEAD 2>/dev/null || printf '')
COMMITS=0
if [ -n "$START_SHA" ] && [ "$START_SHA" != "no-git" ] && [ -n "$HEAD_SHA" ] && [ "$START_SHA" != "$HEAD_SHA" ]; then
  COMMITS=$(git -C "$NAK" rev-list --count "$START_SHA..HEAD" 2>/dev/null || echo 0)
fi

HUB_TS=$(stat -c %Y "$HUB_JSON" 2>/dev/null || echo 0)
HUB_TOUCHED=0
[ "$HUB_TS" -gt "$START_TS" ] && HUB_TOUCHED=1

GRUND=""
if [ "${COMMITS:-0}" -gt 0 ] && [ "$HUB_TOUCHED" -eq 0 ]; then
  GRUND="Nakama-Hub nicht nachgezogen: $COMMITS Commit(s) diese Session, docs/hub/hub.json unveraendert. Pflicht (CLAUDE.md, Abschnitt Hub): den Stand auf https://nakama-briefing.philipld.chatgpt.site/api/hub lesen, hub.json bei echten Statusaenderungen fortschreiben und den Vollstand ueber /api/state synchronisieren. Das alte Claude-Artefakt nicht mehr bauen. Ist wirklich nichts zu aendern, bleibt hub.json unveraendert."
fi

[ -z "$GRUND" ] && exit 0
mkdir -p "$SESSIONS_DIR" 2>/dev/null
: > "$NAG"
ESC=$(printf '%s' "$GRUND" | sed 's/\\/\\\\/g; s/"/\\"/g')
printf '{"decision":"block","reason":"%s"}\n' "$ESC"
exit 0
