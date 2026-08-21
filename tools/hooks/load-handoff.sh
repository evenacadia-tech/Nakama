#!/bin/bash
# SessionStart hook: jüngstes Nakama-Session-Memo aus dem Memory-Verzeichnis
# injizieren, damit eine frische Session den letzten Stand ohne manuelles
# Kontext-Pasten sieht.

# Bis 21.08.2026 zeigte dieser Pfad auf den Vorgänger-Workspace (Dokumente-Ordner) vom
# 17.08. — der Hook lieferte seit dem Umzug still nichts und zählte seine
# Cap-/Budget-Warnungen gegen das falsche Verzeichnis (Kontext-Inventur).
MEMORY_DIR="$HOME/.claude/projects/C--Users-phili-Projekte-Nakama/memory"
LATEST=$(ls -t "$MEMORY_DIR"/project_session_*.md 2>/dev/null | head -1)
[ -n "$LATEST" ] && { echo "=== Jüngster Nakama-Handoff: $(basename "$LATEST") ==="; cat "$LATEST"; }

# Selbstbegrenzungs-Hinweis für Session-Memos. Hier wird NICHT gelöscht —
# Memos tragen [[links]] und speisen MEMORY.md; Konsolidieren braucht Urteil
# (ältestes in MEMORY.md "Letzte Sessions" falten, DANN Datei löschen). Cap 6.
MEMO_CAP=6
MEMO_COUNT=$(ls -1 "$MEMORY_DIR"/project_session_*.md 2>/dev/null | wc -l | tr -d ' ')
if [ "${MEMO_COUNT:-0}" -gt "$MEMO_CAP" ]; then
  echo ""
  echo "=== Memory-Hygiene: ${MEMO_COUNT} project_session_*-Memos (Cap ${MEMO_CAP}) ==="
  echo "Beim Schreiben des heutigen Memos zuerst die ältesten $((MEMO_COUNT - MEMO_CAP))"
  echo "in MEMORY.md 'Letzte Sessions' falten und die Einzeldateien löschen."
  echo "Link-sichere Konsolidierung, kein blindes Löschen. Älteste:"
  ls -1t "$MEMORY_DIR"/project_session_*.md 2>/dev/null | tail -n "$((MEMO_COUNT - MEMO_CAP))" | while IFS= read -r m; do echo "  - $(basename "$m")"; done
fi

# Budget-Hinweis — der Harness lädt nur ~24,4 KB von MEMORY.md; darüber
# trunkiert das Ende des Index still. Warnen BEVOR es überläuft.
MEMORY_INDEX="$MEMORY_DIR/MEMORY.md"
if [ -f "$MEMORY_INDEX" ]; then
  IDX_BYTES=$(wc -c < "$MEMORY_INDEX" | tr -d ' ')
  if [ "${IDX_BYTES:-0}" -gt 22528 ]; then
    echo ""
    echo "=== Memory-Budget: MEMORY.md hat $((IDX_BYTES / 1024)) KB (Ziel 22, harte Kappe 24,4) ==="
    if [ "$IDX_BYTES" -gt 24986 ]; then
      echo "ÜBER der Harness-Kappe — das Index-Ende ist diese Session bereits trunkiert. Vor Memory-Nutzung komprimieren."
    else
      echo "Nähert sich der Kappe — bald komprimieren, bevor die unteren Einträge trunkieren."
    fi
  fi
fi
