#!/bin/bash
# SessionStart hook: Wahrheitskern + Live-Stand injizieren.
#
# Bis zum 21.08.2026 stand hier eine STATISCHE „Systemkarte" (Stand 18.08.,
# „bewusstes Duplikat der CLAUDE.md-Invarianten, nie zum Pointer kürzen").
# Ergebnis der Kontext-Inventur: 18 CLAUDE.md-Commits später war die Karte
# 0-mal nachgezogen — eine frische Session bekam zwei Wahrheiten mit zwei
# Daten (36 statt 56 Broker-Tests, Prototyp „ABGENOMMEN", keine Sondenfamilie).
# Deshalb liest der Hook den Kern jetzt AUS CLAUDE.md (Block zwischen den
# WAHRHEITSKERN-Markern) — eine Quelle, zwei Leser. Die Fade-Resilienz bleibt:
# der Hook feuert auch nach Compaction (source=compact), CLAUDE.md nicht.

NAK="${CLAUDE_PROJECT_DIR:-$HOME/Projekte/Nakama}"
KARTE="$NAK/CLAUDE.md"

if [ -f "$KARTE" ]; then
  echo "=== Nakama-Wahrheitskern (aus CLAUDE.md — einzige Quelle; Register der User-Entscheide dort) ==="
  sed -n '/<!-- WAHRHEITSKERN:ANFANG/,/WAHRHEITSKERN:ENDE -->/p' "$KARTE" \
    | sed -e '/<!-- WAHRHEITSKERN:ANFANG/,/-->$/d' -e '/WAHRHEITSKERN:ENDE -->/d'
else
  echo "=== Nakama: CLAUDE.md nicht gefunden unter $KARTE — Pfad prüfen ==="
fi

echo ""
echo "=== Live-Stand (git in diesem Workspace — Code ist Wahrheit) ==="
git -C "$NAK" log --oneline -6 2>/dev/null \
  || echo "(Nakama-Repo nicht lesbar — Pfad prüfen: $NAK)"
DIRTY=$(git -C "$NAK" status --porcelain 2>/dev/null)
if [ -n "$DIRTY" ]; then
  echo ""
  echo "-- Uncommitted (evtl. parallele Session — eigene Edits sofort per Pathspec committen, nie --amend, fremde Dateien nie anfassen):"
  echo "$DIRTY" | head -10 | sed 's/^/  /'
  N=$(echo "$DIRTY" | wc -l | tr -d ' ')
  if [ "$N" -gt 10 ]; then echo "  … ($N gesamt)"; fi
fi

# Exitcode explizit: der letzte Befehl oben darf nie den Hook-Status bestimmen
# (bis 22.08.2026 lieferte „[ N -gt 10 ] && …" bei 1–10 Dateien Exit 1 — der
# Harness verwarf dann die ganze Ausgabe als „hook error, no stderr").
exit 0
