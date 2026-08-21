#!/bin/bash
# PreToolUse Edit|Write: Codex-Besitz schützen.
# (Seit 21.08.2026 liegt das Blatt im Archiv eq-copilot/design/archive/alt-2026-08/;
# der Guard bleibt, weil er am Dateinamen hängt und nichts kostet.)
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
