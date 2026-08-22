#!/bin/bash
# PostToolUse Edit|Write|Bash: kontextsensitive Erinnerungen für die zwei
# heikelsten Nakama-Flächen. Nicht blockierend (immer Exit 0); der stdout-
# JSON-Kontext formt die nächste Antwort.
#
# 1. plugin/src/* → Realtime-Regeln + Beweis-Kanon
# 2. schemas/*.schema.json → Versionierungs-Vertrag
#
# Seit 22.08.2026 wird auch der Bash-Befehl abgeklopft: Arbeitsumgebungen, die
# Dateiänderungen über Bash bevorzugen (sed -i, Heredoc), lieferten nie ein
# "file_path" — die Erinnerung schwieg also genau dort, wo am meisten über
# Bash editiert wird. Für eine ERINNERUNG (nie blockierend) ist grobes Treffen
# richtig: ein überflüssiger Hinweis kostet zwei Zeilen, ein ausgebliebener
# eine Realtime-Regel. Wer BLOCKEN will, nimmt lib/schreibziel.sh.

INPUT=$(cat 2>/dev/null || printf '')
FILE=$(printf '%s' "$INPUT" | grep -o '"file_path"[[:space:]]*:[[:space:]]*"[^"]*"' | head -n1 | sed 's/.*"\([^"]*\)"$/\1/')
if [ -z "$FILE" ]; then
  # Bash: ersten Pfad im Befehl nehmen, der eine der beiden Flächen nennt.
  FILE=$(printf '%s' "$INPUT" \
    | grep -oE '[^"[:space:];|&]*eq-copilot[/\\](plugin[/\\]src|schemas)[/\\][^"[:space:];|&]*' \
    | head -n1)
fi
FILE=$(printf '%s' "$FILE" | sed 's#\\\\#/#g; s#\\#/#g')

CONTEXT=""

case "$FILE" in
  *eq-copilot/plugin/src/*)
    CONTEXT="[nakama-realtime] Edit in plugin/src: $(basename "$FILE") — es gelten
Grundgesetz (processBlock: keine Sperre/Allokation/IO; Passthrough
sampleidentisch bei Gen/Suna — Probeeq ist ein EQ; NaN-Riegel + Nyquist-Kappe; Editor: SafePointer, ein
Snapshot-Zug pro Tick) und der Beweis-Kanon: die vom Edit betroffenen
Beweise vor 'fertig' laufen lassen und das Ergebnis nennen."
    ;;
  *eq-copilot/schemas/*.schema.json)
    CONTEXT="[nakama-schema] Schema-Edit: $(basename "$FILE") — Schemas sind
versionierte Verträge: neue Felder ⇒ ERST Versionierung; alte Snapshots
laden weiter; unbekannte Felder brechen keine Consumer (Plugin, Broker); Save + Load im selben Änderungssatz testen."
    ;;
  *)
    exit 0
    ;;
esac

PYTHON=""
for cand in python3 python py; do
  if command -v "$cand" >/dev/null 2>&1; then PYTHON="$cand"; break; fi
done

if [ -z "$PYTHON" ]; then
  printf '%s\n' "$CONTEXT"
  exit 0
fi

HOOK_CONTEXT="$CONTEXT" "$PYTHON" - <<'PYEOF'
import json, os
print(json.dumps({
    "hookSpecificOutput": {
        "hookEventName": "PostToolUse",
        "additionalContext": os.environ.get("HOOK_CONTEXT", ""),
    }
}))
PYEOF
exit 0
