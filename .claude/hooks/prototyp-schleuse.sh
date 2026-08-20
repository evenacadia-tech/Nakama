#!/usr/bin/env bash
# PreToolUse-Schleuse: blockt Schreibzugriffe IM ORDNER prototyp/, solange kein
# abgenommener DESIGNVERTRAG existiert (Arbeitsplan Phase 0).
#
# Zwei Praezisierungen gegenueber der ersten Fassung:
# 1. Es wird auf den Ordner geprueft, nicht auf die Zeichenfolge "prototyp"
#    irgendwo im Pfad — sonst blockt die Schleuse ihre eigene Datei
#    (.claude/hooks/prototyp-schleuse.sh) und jede Datei, die zufaellig so heisst.
# 2. Verlangt wird eine Datei mit "designvertrag" im Namen, nicht irgendeine
#    Abnahme: sonst oeffnet jede beilaeufige Abnahme (Methode, Werkzeug,
#    Sessionnotiz) das Tor fuer etwas, wozu sie nichts sagt.
input=$(cat)
fp=$(printf '%s' "$input" | grep -o '"file_path"[^,}]*' | head -1)
fp="${fp//\\//}"   # Windows-Backslashes zu Schraegstrichen
case "$fp" in
  */prototyp/*) ;;
  *) exit 0 ;;
esac
count=0
for f in abnahmen/*designvertrag*.md; do
  [ -e "$f" ] || continue
  count=$((count + 1))
done
if [ "$count" -eq 0 ]; then
  echo "PROTOTYP-SCHLEUSE: Es liegt keine Designvertrag-Abnahme in abnahmen/." >&2
  echo "Erst die Welt entscheiden (werkzeug/weltenwahl.html), dann den Vertrag als" >&2
  echo "abnahmen/JJJJ-MM-TT-designvertrag.md festhalten (docs/arbeitsplan.md," >&2
  echo "Phase 0) — dann darf in prototyp/ gebaut werden." >&2
  exit 2
fi
exit 0
