#!/usr/bin/env bash
# Stop hook: verhindert, dass eine Session mit uncommitteter EIGENER Arbeit endet.
#
# Gegenstueck zu auto-push.sh — der schickt raus, was committet ist; dieser hier
# sorgt dafuer, dass ueberhaupt committet wird. Zusammen sind sie der Gegenpfad
# zur Automatik-Zusage vom 22.08.2026 (User: "so automatisch wie möglich"):
# ohne den Waechter waere "automatisch" nur die Halbstrecke, weil ein vergessener
# Commit gar nicht erst zum Push kommt.
#
# Kandidat ist, was seit dem Sessionstart-Marker (Zeile 1: Epoch) veraendert
# wurde UND uncommittet ist. Die Zeitmarke unterscheidet nicht sicher zwischen
# eigener und fremder Arbeit — in diesem Baum arbeiten parallele Sessions. Der
# Hook entscheidet das deshalb NICHT, er legt die Liste vor: eigene Dateien
# committen (expliziter Pathspec), fremde stehen lassen und benennen.
#
# Hoechstens EINMAL je Session (Marker), damit daraus nie eine Schleife wird —
# dieselbe Selbstbegrenzung wie im entfallenen hub-stop.sh (23.08.2026).

set -u
STDIN_JSON=$(cat 2>/dev/null || printf '')
SESSION_ID=$(printf '%s' "$STDIN_JSON" | grep -o '"session_id":[[:space:]]*"[^"]*"' | head -n1 | sed 's/.*"\([^"]*\)"$/\1/')
[ -z "$SESSION_ID" ] && exit 0
printf '%s' "$STDIN_JSON" | grep -q '"stop_hook_active":[[:space:]]*true' && exit 0

NAK="${CLAUDE_PROJECT_DIR:-$HOME/Projekte/Nakama}"
SESSIONS_DIR="${NAKAMA_SESSIONS_DIR:-$HOME/.claude/sessions}"
MARKER="$SESSIONS_DIR/${SESSION_ID}.start"
NAG="$SESSIONS_DIR/${SESSION_ID}.commit-nag"

[ -f "$MARKER" ] || exit 0
[ -f "$NAG" ] && exit 0
git -C "$NAK" rev-parse --git-dir >/dev/null 2>&1 || exit 0

START_TS=$(sed -n '1p' "$MARKER")
case "$START_TS" in ''|*[!0-9]*) exit 0 ;; esac

STATUS=$(git -C "$NAK" status --porcelain 2>/dev/null)
[ -z "$STATUS" ] && exit 0

KANDIDATEN=""
while IFS= read -r zeile; do
  [ -n "$zeile" ] || continue
  pfad=${zeile:3}
  case "$pfad" in *" -> "*) pfad=${pfad##* -> } ;; esac
  pfad=${pfad#\"}; pfad=${pfad%\"}
  voll="$NAK/$pfad"
  [ -e "$voll" ] || continue
  m=$(stat -c %Y "$voll" 2>/dev/null || echo 0)
  [ "${m:-0}" -gt "$START_TS" ] && KANDIDATEN="$KANDIDATEN $pfad"
done <<EOF
$STATUS
EOF

[ -z "$KANDIDATEN" ] && exit 0

mkdir -p "$SESSIONS_DIR" 2>/dev/null
: > "$NAG"
GRUND="Uncommittet und seit Sessionstart veraendert:$KANDIDATEN — eigene Edits jetzt mit explizitem Pathspec committen (git add -- <pfade> && git commit); der auto-push-Hook schickt sie danach von selbst zum Remote. Was einer parallelen Session gehoert, bleibt liegen und wird in der Antwort benannt. Ist wirklich nichts davon eigene Arbeit, genuegt dieser Satz — der Waechter meldet sich je Session nur einmal."
ESC=$(printf '%s' "$GRUND" | sed 's/\\/\\\\/g; s/"/\\"/g')
printf '{"decision":"block","reason":"%s"}\n' "$ESC"
exit 0
