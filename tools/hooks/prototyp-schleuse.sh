#!/usr/bin/env bash
# PreToolUse-Schleuse: blockt Schreibzugriffe IM ORDNER prototyp/, solange kein
# abgenommener DESIGNVERTRAG existiert (Arbeitsplan Phase 0).
#
# Die Frage "schreibt dieser Aufruf ueberhaupt nach prototyp/?" beantwortet
# seit 22.08.2026 der gemeinsame Kern tools/hooks/lib/schreibziel.sh — dort
# stehen auch die sieben Praezisierungen, die jede aus einem echten Fehler
# stammen (Bash statt nur file_path, Heredoc-Ruempfe, cp-Ziel als letztes
# Argument, Ordner statt Zeichenfolge, …). Vorher lag diese Logik nur hier,
# und die Kreativ-Schleuse hatte dasselbe Loch ungeschuetzt offen.
#
# Seit 22.08.2026 im Nakama-Repo: der Ordner heisst design/prototyp/, die
# Abnahmen liegen in design/abnahmen/ (Muster */prototyp/* greift weiter).
# Gegenprobe (beide Richtungen, beide Schleusen): bash tools/hooks/schleusen-probe.sh

. "$(dirname "${BASH_SOURCE[0]}")/lib/schreibziel.sh"

input=$(cat)
schreibziel_trifft "$input" "prototyp/" || exit 0

# --- liegt ein abgenommener Designvertrag vor? -----------------------------
# SCHLEUSE_ABNAHMEN: nur fuer die Gegenprobe (schleusen-probe.sh), die den
# GESCHLOSSENEN Zustand gegen ein leeres Verzeichnis misst, ohne den echten
# Vertrag anzufassen. Im Betrieb bleibt es abnahmen/.
ABNAHMEN="${SCHLEUSE_ABNAHMEN:-${CLAUDE_PROJECT_DIR:-.}/design/abnahmen}"
count=0
for f in "$ABNAHMEN"/*designvertrag*.md; do
  [ -e "$f" ] || continue
  count=$((count + 1))
done
if [ "$count" -eq 0 ]; then
  echo "PROTOTYP-SCHLEUSE: Es liegt keine Designvertrag-Abnahme in design/abnahmen/." >&2
  echo "Der Designvertrag entsteht aus dem User-Wort (Interview/Figma-Stand) und wird als" >&2
  echo "design/abnahmen/JJJJ-MM-TT-designvertrag.md festgehalten (design/docs/arbeitsplan.md," >&2
  echo "Phase 0) — dann darf in design/prototyp/ gebaut werden." >&2
  echo "Bis dahin gehoeren lebende Blaetter nach design/werkzeug/." >&2
  exit 2
fi
exit 0
