#!/bin/bash
# SessionStart hook: Marker mit Zeitstempel + git-HEAD schreiben. Verbraucher
# ist seit 23.08.2026 nur noch commit-stop.sh — er misst daran, ob diese
# Session Aenderungen erzeugt hat, die uncommittet liegenblieben.
# (Frueher lasen ihn auch session-end-handoff.sh — entfallen 22.08., weil sein
# Erzeugnis niemand las, Commit ef1efee — und hub-stop.sh, entfallen 23.08.
# mit der Briefing-Seite: es gibt keinen Hub mehr nachzuziehen, der Planstand
# wird gerechnet. Die *.hub-nag-Zeile unten bleibt als Aufraeumregel fuer
# Altlasten stehen; neue entstehen keine mehr.)
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
# NUR die eigenen Endungen, nie pauschal: dieses Verzeichnis gehört dem
# Harness mit (<pid>.json und <pid>.<hash>.key sind seine Sessionregister) —
# ein `find … -delete` ohne Namensfilter würde die laufende Session treffen.
# .hub-nag und die mahnung-*-Reste abgeschaffter Hooks räumte bis 22.08.2026
# niemand ab (gemessen: 31 tote mahnung-Marker); seit dem Wegfall des
# SessionEnd-Handoffs putzt auch nichts mehr die .start-Marker am Sessionende.
for muster in '*.start' '*.hub-nag' '*.mahnung-*'; do
  find "$SESSIONS_DIR" -name "$muster" -type f -mtime +7 -delete 2>/dev/null
done

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
