#!/bin/bash
# SessionStart hook: den Nakama-Hub vorlesen.
#
# Der Hub ist die gemeinsame Uebersicht fuer den User (Projektleiter, kein Coder)
# und jede Claude-Session: was im Plan erledigt/offen ist, was bei ihm liegt,
# der Figma-Stand der drei Apps. Quelle ist docs/hub/hub.json; die private Seite
# spiegelt den Stand und haelt Antworten sowie neue Punkte dauerhaft fest.
# Pflicht jeder Session (CLAUDE.md, Abschnitt "Hub"): beim Start lesen, neue
# Antworten aufgreifen, am Ende nachziehen. Dieser Hook macht den Kopf sichtbar
# und misst die Drift.

NAK="${CLAUDE_PROJECT_DIR:-$HOME/Projekte/Nakama}"
HUB="$NAK/docs/hub/hub.json"
SITE="https://nakama-briefing.philipld.chatgpt.site"
[ -f "$HUB" ] || { echo "=== Nakama-Hub: docs/hub/hub.json fehlt ==="; exit 0; }

py_() { py -3.13 - "$@" 2>/dev/null || python3 - "$@" 2>/dev/null; }

# Kopfzahlen aus hub.json (ohne jq — Python ist auf beiden Rechnern da)
KOPF=$(py_ "$HUB" <<'EOF'
import json, sys
sys.stdout.reconfigure(encoding='utf-8', errors='replace')  # Konsole ist cp1252
h = json.load(open(sys.argv[1], encoding="utf-8"))
bd = h.get("bei_dir", [])
jetzt = [b for b in bd if b.get("dringlichkeit") == "jetzt"]
z = [r for p in h["plan"] for r in p["zeilen"]]
erl = sum(1 for r in z if r["status"] == "erledigt")
nxt = [r for r in z if r["status"] == "naechster"]
print(f"Stand: {h['stand']} — {h.get('stand_notiz','')}")
print(f"Plan: {erl}/{len(z)} Zeilen erledigt · naechster Schritt: {nxt[0]['id'] + ' ' + nxt[0]['text'][:70] if nxt else '?'}")
print(f"Bei dir (jetzt): {len(jetzt)} — " + " · ".join(b['id'] + ' ' + b['titel'] for b in jetzt))
EOF
)

echo "=== Nakama Briefing (Pflicht: lesen · Antworten aufgreifen · nachziehen) ==="
echo "Seite: $SITE"
echo "Maschinenansicht: $SITE/api/hub"
echo "$KOPF"

# Drift: Commits seit dem letzten Commit, der hub.json angefasst hat
LETZTER=$(git -C "$NAK" log -1 --format=%h -- docs/hub/hub.json 2>/dev/null)
if [ -n "$LETZTER" ]; then
  N=$(git -C "$NAK" rev-list --count "$LETZTER..HEAD" 2>/dev/null || echo 0)
  if [ "${N:-0}" -gt 0 ]; then
    echo "DRIFT: $N Commit(s) seit dem letzten Hub-Stand ($LETZTER) — hub.json vor der Arbeit nachziehen, wenn sich Plan oder 'bei dir' dadurch geaendert haben:"
    git -C "$NAK" log --oneline "$LETZTER..HEAD" 2>/dev/null | head -8 | sed 's/^/  /'
  else
    echo "Drift: keine — Hub ist auf dem Stand von HEAD."
  fi
fi
echo "Schritte: (1) py -3.13 tools/hub/hub_sync.py holen  -> Antworten/Punkte der Seite nach hub.json;"
echo "          (2) einarbeiten (User-Wort mit Datum + Wortlaut ins Register/Abnahmen);"
echo "          (3) hub.json fortschreiben -> py -3.13 tools/hub/hub_sync.py senden -> hub.json committen. Kein Artefakt mehr."
