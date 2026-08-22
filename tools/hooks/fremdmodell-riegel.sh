#!/usr/bin/env bash
# PreToolUse (Bash|PowerShell): fremde Modell-CLIs bleiben LESEND.
#
# User 22.08.2026, woertlich: "gemini ist nicht sehr zuverlässig, ich möchte
# dass er ausschlisslich für audits und read only benutzt wird. falls du ihn mal
# eigenständig nutzen solltest".
#
# Der Anlass steht in derselben Session: Gemini 3.1 Pro hat den git-riegel
# adversarial geprueft und neun Befunde geliefert — acht davon waren echt, EINER
# war frei erfunden (Push auf eine direkte URL werde blockiert; gemessen: wird
# er nicht). Als Pruefer ist das ein gutes Verhaeltnis, als Schreiber waere es
# eines zu viel: ein erfundener Befund kostet eine Messung, eine erfundene
# Aenderung kostet den Baum.
#
# Geblockt wird deshalb genau das, was `agy` vom Pruefer zum Schreiber macht:
#   --dangerously-skip-permissions  genehmigt JEDES Werkzeug ohne Rueckfrage
#   --mode accept-edits             nimmt Dateiaenderungen automatisch an
# Ohne diese Schalter verweigert die Antigravity-CLI im Kopflos-Modus von selbst
# jedes Werkzeug, das eine Genehmigung braucht (gemessen 22.08.: der Agent wollte
# das Dateisystem lesen und wurde von seinem eigenen Rechteschutz gestoppt).
# LESEN bleibt also erlaubt — der Riegel nimmt nur die Schreibhand.
#
# Die belastbarste Form bleibt ohnehin die, die in dieser Session gemessen wurde:
# den Quelltext als TEXT in den Prompt geben statt Verzeichniszugriff. Ein
# Pruefer, der nichts anfassen kann, braucht kein Vertrauen.
#
# Codex faellt nicht unter diesen Riegel: er laeuft ueber den
# codex:codex-rescue-Agenten, der seinerseits nur lesend arbeitet.
#
# Gegenprobe (beide Richtungen): bash tools/hooks/git-automatik-probe.sh

. "$(dirname "${BASH_SOURCE[0]}")/lib/schreibziel.sh"

input=$(cat)
cmd=$(befehl_ohne_zitate "$input")
[ -n "$cmd" ] || exit 0

case "$cmd" in
  *agy*) ;;
  *) exit 0 ;;
esac

V='(^|[[:space:](){}])'
A="${V}([^[:space:]]*/)?agy(\.exe)?([[:space:]]|$)"

printf '%s' "$cmd" | grep -Eq "$A" || exit 0

if printf '%s' "$cmd" | grep -Eq -- "--dangerously-skip-permissions"; then
  echo "FREMDMODELL-RIEGEL: --dangerously-skip-permissions macht die Antigravity-CLI zum Schreiber. User-Regel 22.08.2026: Gemini ausschliesslich fuer Audits und read only. Ohne den Schalter verweigert die CLI schreibende Werkzeuge von selbst; fuer eine Pruefung den Quelltext als Text in den Prompt geben." >&2
  exit 2
fi
if printf '%s' "$cmd" | grep -Eq -- "--mode[[:space:]=]+accept-edits"; then
  echo "FREMDMODELL-RIEGEL: --mode accept-edits nimmt Dateiaenderungen automatisch an. User-Regel 22.08.2026: Gemini ausschliesslich fuer Audits und read only. Fuer eine Pruefung ohne Schreibrecht: --mode plan oder gar keinen Modus setzen." >&2
  exit 2
fi

exit 0
