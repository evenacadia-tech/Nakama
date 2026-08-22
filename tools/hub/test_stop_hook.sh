#!/bin/bash
# Probe fuer tools/hooks/hub-stop.sh — fuenf Faelle, alle gegen das echte Repo.
#   bash tools/hub/test_stop_hook.sh   -> "STOP-HOOK OK (5/5)" oder Fehlerzeilen
set -u
NAK="$(cd "$(dirname "$0")/../.." && pwd)"
HOOK="$NAK/tools/hooks/hub-stop.sh"
TMP=$(mktemp -d)
export NAKAMA_SESSIONS_DIR="$TMP"
export CLAUDE_PROJECT_DIR="$NAK"
HEAD=$(git -C "$NAK" rev-parse --short HEAD)
PREV=$(git -C "$NAK" rev-parse --short HEAD~1)
NOW=$(date +%s)
HUB_TS=$(stat -c %Y "$NAK/docs/hub/hub.json")
OK=0; N=0

lauf() { # $1 Fallname · $2 Start-Epoch · $3 Start-SHA · $4 stop_hook_active · $5 erwartet (block|still)
  N=$((N+1))
  SID="probe-$N"
  printf '%s\n%s\n%s\nx\n' "$2" "$3" "$NAK" > "$TMP/$SID.start"
  OUT=$(printf '{"session_id":"%s","stop_hook_active":%s}' "$SID" "$4" | bash "$HOOK")
  if [ "$5" = "block" ]; then
    if printf '%s' "$OUT" | grep -q '"decision":"block"'; then OK=$((OK+1)); echo "  ok    $1"; else echo "  FEHLT $1 — erwartet block, bekam: '$OUT'"; fi
  else
    if [ -z "$OUT" ]; then OK=$((OK+1)); echo "  ok    $1"; else echo "  FEHLT $1 — erwartet still, bekam: '$OUT'"; fi
  fi
}

# hub.html fuer Fall (c) sicher juenger als hub.json machen (Test-Kopie, nicht das Original anfassen)
# -> wir pruefen Fall (c) ueber den Start-Epoch: hub.json zaehlt als "angefasst", hub.html ist vom Build neuer oder gleich.
touch "$NAK/docs/hub/hub.html" 2>/dev/null

lauf "a) Commits, Hub unangefasst -> block"            $((NOW+1000)) "$PREV" false block
lauf "b) keine Commits -> still"                        $((NOW+1000)) "$HEAD" false still
lauf "c) Commits, Hub angefasst und gebaut -> still"    $((HUB_TS-10)) "$PREV" false still
lauf "d) wie a), aber stop_hook_active -> still"        $((NOW+1000)) "$PREV" true  still
# e) wie a) zweimal: der zweite Lauf derselben Session muss still sein (Marker)
N=$((N+1)); SID="probe-$N"; printf '%s\n%s\n%s\nx\n' $((NOW+1000)) "$PREV" "$NAK" > "$TMP/$SID.start"
printf '{"session_id":"%s"}' "$SID" | bash "$HOOK" > /dev/null
OUT=$(printf '{"session_id":"%s"}' "$SID" | bash "$HOOK")
if [ -z "$OUT" ] && [ -f "$TMP/$SID.hub-nag" ]; then OK=$((OK+1)); echo "  ok    e) zweiter Stop derselben Session -> still (Marker)"; else echo "  FEHLT e) — '$OUT'"; fi

rm -rf "$TMP"
echo "STOP-HOOK OK ($OK/$N)"
[ "$OK" -eq "$N" ]
