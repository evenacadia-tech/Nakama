#!/usr/bin/env bash
# PreToolUse-Schleuse: blockt Schreibzugriffe unter prototyp/, solange
# keine einzige Abnahme existiert (Arbeitsplan Phase 0: erst /interview
# designvertrag, dann bauen).
input=$(cat)
fp=$(printf '%s' "$input" | grep -o '"file_path"[^,}]*' | head -1)
case "$fp" in
  *prototyp*) ;;
  *) exit 0 ;;
esac
count=0
for f in abnahmen/*.md; do
  [ -e "$f" ] || continue
  case "$f" in *LIES-MICH*) continue ;; esac
  count=$((count + 1))
done
if [ "$count" -eq 0 ]; then
  echo "PROTOTYP-SCHLEUSE: In abnahmen/ liegt noch keine Abnahme. Erst /interview designvertrag fuehren und das Ergebnis als Abnahme festhalten (docs/arbeitsplan.md, Phase 0) — dann darf in prototyp/ gebaut werden." >&2
  exit 2
fi
exit 0
