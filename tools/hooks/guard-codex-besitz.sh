#!/bin/bash
# PreToolUse Edit|Write: Codex-Besitz schützen.
# nakama-spectral-field-vorentwurf.html gehört der Codex-Session (Plan-Doc:
# "Codex-Besitz; Neubau kollidiert nicht"). Claudes Design-Blatt ist
# nakama-spectral-field-claude.html. Exit 2 blockt den Tool-Call und gibt
# stderr als Begründung an Claude zurück.

INPUT=$(cat 2>/dev/null || printf '')
FILE=$(printf '%s' "$INPUT" | grep -o '"file_path"[[:space:]]*:[[:space:]]*"[^"]*"' | head -n1 | sed 's/.*"\([^"]*\)"$/\1/')

case "$FILE" in
  *nakama-spectral-field-vorentwurf.html*)
    echo "BLOCK: nakama-spectral-field-vorentwurf.html ist CODEX-BESITZ (parallele Session). Claudes Design-Blatt ist nakama-spectral-field-claude.html — dort arbeiten. Lesen ist erlaubt, Editieren nicht." >&2
    exit 2
    ;;
esac

exit 0
