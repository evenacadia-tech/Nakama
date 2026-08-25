#!/usr/bin/env bash
# Gegenprobe der aktiven Kreativ-Schleuse.
# Aufruf vom Repo-Root: bash tools/hooks/schleusen-probe.sh
#
# Die Probe misst beide Richtungen: verbotene Schreibziele müssen blocken,
# harmlose Lesezugriffe und explizit freigegebene Schreibziele durchlaufen.
cd "$(dirname "$0")/../.." || exit 1
KHOOK=tools/hooks/kreativ-schleuse.sh
fehler=0
gesamt=0

LEER=$(mktemp -d) || exit 1
cleanup() {
  [ -n "${LEER:-}" ] && [ -d "$LEER" ] && rm -rf -- "$LEER"
}
trap cleanup EXIT
MARKER="$LEER/gibt-es-nicht.md"

kprobe() { # kprobe <soll:block|durch> <name> <json>
  local soll="$1" name="$2" json="$3" code ist
  gesamt=$((gesamt + 1))
  printf '%s' "$json" | KREATIV_MARKER="$MARKER" bash "$KHOOK" >/dev/null 2>&1
  code=$?
  ist=durch
  [ "$code" -eq 2 ] && ist=block
  if [ "$ist" = "$soll" ]; then
    printf 'ok    %-5s  %s\n' "$soll" "$name"
  else
    printf 'FEHL  soll=%-5s ist=%-5s  %s\n' "$soll" "$ist" "$name"
    fehler=$((fehler + 1))
  fi
}

j() {
  printf '{"tool_name":"%s","tool_input":{"%s":"%s"}}' "$1" "$2" "$3"
}

echo "=== KREATIV-SCHLEUSE (eq-copilot/design/, Freigabe .claude/kreativ-freigabe.md) ==="
echo "--- muss blocken ---"
kprobe block "Write nach eq-copilot/design/"          "$(j Write file_path 'eq-copilot/design/tokens.json')"
kprobe block "Edit (absolut, Windows)"                "$(j Edit file_path 'C:\\\\Users\\\\x\\\\Nakama\\\\eq-copilot\\\\design\\\\a.html')"
kprobe block "Bash: cat > eq-copilot/design/"         "$(j Bash command 'cat > eq-copilot/design/neu.html')"
kprobe block "Bash: sed -i im Archiv"                 "$(j Bash command 'sed -i s/a/b/ eq-copilot/design/archive/alt-2026-08/x.html')"
kprobe block "Bash: Codex-Vorentwurf im Archiv"       "$(j Bash command 'cat > eq-copilot/design/archive/alt-2026-08/nakama-spectral-field-vorentwurf.html')"
kprobe block "Bash: cp in die Prisma-Studie"          "$(j Bash command 'cp a.png eq-copilot/design/prisma-studie/a.png')"

echo
echo "--- muss durchlassen ---"
kprobe durch "Write nach design/"                     "$(j Write file_path 'design/abnahmen/2026-08-22-x.md')"
kprobe durch "Edit im Plugin-Code"                    "$(j Edit file_path 'eq-copilot/plugin/src/PluginEditor.cpp')"
kprobe durch "Bash: lesen im Archiv"                  "$(j Bash command 'cat eq-copilot/design/archive/alt-2026-08/x.html')"
kprobe durch "Bash: grep in eq-copilot/design/"       "$(j Bash command 'grep -rn token eq-copilot/design/')"
kprobe durch "Bash: Pfad nur erwähnt"                 "$(j Bash command 'echo eq-copilot/design/ ist Provisorium')"
kprobe durch "Bash: cp weg aus eq-copilot/design/"    "$(j Bash command 'cp eq-copilot/design/tokens.json sicherung/tokens.json')"
kprobe durch "Bash: Heredoc zitiert Schreibbefehl"    "$(j Bash command 'git commit -F - <<EOF
Loch: cat > eq-copilot/design/neu.html lief durch
EOF')"

echo
echo "--- mit frischer Freigabe: muss durchlassen ---"
MARKER="$LEER/kreativ-freigabe.md"
: > "$MARKER"
kprobe durch "Write mit Freigabe"                     "$(j Write file_path 'eq-copilot/design/tokens.json')"
kprobe durch "Bash mit Freigabe"                      "$(j Bash command 'cat > eq-copilot/design/neu.html')"

echo "--- mit verbrauchter Freigabe (>24 h): muss blocken ---"
touch -d '2 days ago' "$MARKER" 2>/dev/null || touch -t 202608200900 "$MARKER"
kprobe block "Write mit abgelaufener Freigabe"        "$(j Write file_path 'eq-copilot/design/tokens.json')"

echo
if [ "$fehler" -eq 0 ]; then
  echo "KREATIV-SCHLEUSE: $gesamt/$gesamt Proben richtig."
  exit 0
fi
echo "KREATIV-SCHLEUSE: $fehler von $gesamt Proben falsch."
exit 1
