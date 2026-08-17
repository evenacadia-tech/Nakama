#!/bin/bash
# PreToolUse Edit|Write: KREATIV-SCHLEUSE (User-Regel 17.08.2026).
# Der kreative Prozess wird nie übersprungen: Design-Artefakte unter
# eq-copilot/design/ dürfen erst entstehen/geändert werden, wenn die Idee
# MIT dem User ausgearbeitet, konkretisiert und freigegeben wurde.
# Freigabe-Marker: .claude/kreativ-freigabe.md (im Nakama-Workspace) —
# enthält die freigegebene Vorstellung in den Worten des Users + Datum;
# gilt 24 Stunden. Exit 2 blockt den Tool-Call, stderr geht an Claude.

INPUT=$(cat 2>/dev/null || printf '')
FILE=$(printf '%s' "$INPUT" | grep -o '"file_path"[[:space:]]*:[[:space:]]*"[^"]*"' | head -n1 | sed 's/.*"\([^"]*\)"$/\1/')
# JSON-escapte Backslashes normalisieren: alles auf einfache Slashes.
FILE=$(printf '%s' "$FILE" | tr '\\' '/' | sed 's#//*#/#g')

# Nur Design-Artefakte betreffen die Schleuse.
case "$FILE" in
  *eq-copilot/design/*) ;;
  *) exit 0 ;;
esac

MARKER="${CLAUDE_PROJECT_DIR:-.}/.claude/kreativ-freigabe.md"
if [ -f "$MARKER" ]; then
  jetzt=$(date +%s)
  alter=$(stat -c %Y "$MARKER" 2>/dev/null || echo 0)
  if [ $((jetzt - alter)) -lt 86400 ]; then
    exit 0
  fi
  echo "KREATIV-SCHLEUSE: Der Freigabe-Marker ist älter als 24 h. Die Freigabe ist verbraucht — neue Design-Arbeit heißt: erst wieder mit dem User die Idee ausarbeiten und konkretisieren, dann mit seiner Freigabe den Marker neu schreiben." >&2
  exit 2
fi

echo "KREATIV-SCHLEUSE: Design-Artefakt ohne ausgearbeitete Idee. Der kreative Prozess wird NIE übersprungen: (1) Idee mit dem User besprechen, (2) Richtungen in WORTEN durchspielen und konkretisieren, (3) warten, bis der User sagt, welche Vorstellung gebaut wird. Erst DANN .claude/kreativ-freigabe.md schreiben (die freigegebene Vorstellung in den Worten des Users + Datum) und bauen. Kunst ist wertlos ohne ihren Entstehungsprozess." >&2
exit 2
