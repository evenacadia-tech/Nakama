#!/bin/bash
# PostToolUse Edit|Write: kontextsensitive Erinnerungen für die zwei
# heikelsten Nakama-Flächen. Nicht blockierend (immer Exit 0); der stdout-
# JSON-Kontext formt die nächste Antwort.
#
# 1. plugin/src/* → Realtime-Regeln + Beweis-Kanon
# 2. schemas/*.schema.json → Versionierungs-Vertrag

INPUT=$(cat 2>/dev/null || printf '')
FILE=$(printf '%s' "$INPUT" | grep -o '"file_path"[[:space:]]*:[[:space:]]*"[^"]*"' | head -n1 | sed 's/.*"\([^"]*\)"$/\1/')

CONTEXT=""

case "$FILE" in
  *eq-copilot/plugin/src/*|*eq-copilot\\plugin\\src\\*)
    CONTEXT="[nakama-realtime] Edit in plugin/src erkannt: $(basename "$FILE")
Vor 'fertig' gelten die Grundgesetz-Regeln und der Beweis-Kanon:
- processBlock: keine Sperre, keine Allokation, keine Datei/Pipe/Netz/Logs;
  Passthrough bleibt sampleidentisch (NullTest beweist es).
- NaN-Riegel und Nyquist-Kappe nicht umgehen; neue Rechenwege NaN-ehrlich.
- Editor: Component::SafePointer in Async-/Popover-Callbacks; ein
  Snapshot-Zug pro Timer-Tick; malen nur bei neuer Revision.
- Betroffene Beweise laufen lassen: EqCopGoldenTest + EqCopNullTest,
  bei Editor-Sicht EqCopShot, bei paint-Kosten EqCopPaintBench,
  bei Pipe-Themen EqCopPipeProbe (eigener Probe-Pipename!)."
    ;;
  *eq-copilot/schemas/*.schema.json|*eq-copilot\\schemas\\*.schema.json)
    CONTEXT="[nakama-schema] Schema-Edit erkannt: $(basename "$FILE")
Schemas sind versionierte Verträge (Bauplan §1/§8.3):
- Neue Felder erfordern ZUERST eine Versionierung.
- Alte Snapshots müssen ohne die neuen Felder weiter laden.
- Unbekannte neue Felder dürfen alte Consumer (Plugin UND Broker UND
  Hub-App) nicht zerstören.
- Save und Load im selben Änderungssatz testen (cargo test eq_copilot +
  GoldenTest, wenn Snapshot-Format betroffen)."
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
