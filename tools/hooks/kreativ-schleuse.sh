#!/usr/bin/env bash
# PreToolUse: KREATIV-SCHLEUSE (User-Regel 17.08.2026).
# Der kreative Prozess wird nie übersprungen: Design-Artefakte unter
# eq-copilot/design/ dürfen erst entstehen/geändert werden, wenn die Idee
# MIT dem User ausgearbeitet, konkretisiert und freigegeben wurde.
# Freigabe-Marker: .claude/kreativ-freigabe.md (im Nakama-Workspace) —
# enthält die freigegebene Vorstellung in den Worten des Users + Datum;
# gilt 24 Stunden. Exit 2 blockt den Tool-Call, stderr geht an Claude.
#
# Seit 22.08.2026 prüft die Schleuse auch BASH-Schreibbefehle über den
# gemeinsamen Kern tools/hooks/lib/schreibziel.sh. Vorher las sie nur
# "file_path" — gemessen am 22.08.: `cat > eq-copilot/design/neu.html`
# lief mit exit 0 durch. In einer Umgebung, die Dateiänderungen über Bash
# ausdrücklich bevorzugt, war die Schleuse damit faktisch aus.
# Seit demselben Tag deckt sie auch das Archiv mit ab
# (eq-copilot/design/archive/…): der frühere Extra-Wächter
# guard-codex-besitz.sh für nakama-spectral-field-vorentwurf.html ist damit
# überflüssig geworden und entfallen.
#
# Gegenprobe (beide Richtungen, beide Schleusen): bash tools/hooks/schleusen-probe.sh

. "$(dirname "${BASH_SOURCE[0]}")/lib/schreibziel.sh"

input=$(cat)
schreibziel_trifft "$input" "eq-copilot/design/" || exit 0

# KREATIV_MARKER: nur für die Gegenprobe, die den GESCHLOSSENEN Zustand gegen
# einen nicht existierenden Marker misst, ohne die echte Freigabe anzufassen.
MARKER="${KREATIV_MARKER:-${CLAUDE_PROJECT_DIR:-.}/.claude/kreativ-freigabe.md}"
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
