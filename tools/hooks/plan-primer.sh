#!/usr/bin/env bash
# SessionStart hook: den Planstand vorlesen — gerechnet, nicht gepflegt.
#
# Loest hub-primer.sh ab (User 23.08.2026: die Briefing-Seite ist abgeschafft).
# Der Unterschied ist nicht die Darstellung, sondern die Pflicht: der alte
# Primer trug jeder Session drei Handgriffe auf (holen · einarbeiten ·
# senden), und wer sie vergass, hinterliess einen falschen Stand. Hier gibt es
# nichts nachzuziehen — tools/hooks/planstand.sh haelt das Blatt selbst frisch.
#
# Zuerst messen lassen (der Aufruf tut nichts, wenn nichts zu tun ist), dann
# den Kopf des Blattes vorlesen.

NAK="${CLAUDE_PROJECT_DIR:-$HOME/Projekte/Nakama}"
BLATT="$NAK/docs/PLAN-STAND.md"
FRAGEN="$NAK/docs/plan/fragen.json"

bash "$NAK/tools/hooks/planstand.sh" >/dev/null 2>&1

echo "=== Nakama Planstand (gerechnet aus dem Repo — nichts nachzuziehen) ==="
if [ ! -f "$BLATT" ]; then
  echo "docs/PLAN-STAND.md fehlt — erzeugen mit: py -3.13 tools/plan/planstand.py"
  exit 0
fi

# Die Zeilen, die den Stand tragen: Kopfzahl, Balken, "Als Naechstes",
# "Wartet auf ein Urteil", offene Fragen, Warnungen.
# "Bei dir liegen" bewusst NICHT: die Fragen listet der Block unten reicher auf.
grep -E '^\*\*(Stand|Als Nächstes|Wartet auf ein Urteil)|^> ⚠️' "$BLATT" 2>/dev/null | head -n 8

PYTHON=""
for cand in py python3 python; do
  if command -v "$cand" >/dev/null 2>&1; then PYTHON="$cand"; break; fi
done
if [ -n "$PYTHON" ] && [ -f "$FRAGEN" ]; then
  [ "$PYTHON" = "py" ] && set -- -3.13 || set --
  "$PYTHON" "$@" - "$FRAGEN" <<'PYEOF' 2>/dev/null
import json, sys
sys.stdout.reconfigure(encoding="utf-8", errors="replace")   # Konsole ist cp1252
f = json.load(open(sys.argv[1], encoding="utf-8"))
offen = f.get("offen", [])
if offen:
    jetzt = [o for o in offen if o.get("dringlichkeit") == "jetzt"]
    print(f"Offene Fragen an den User: {len(offen)}"
          + (f", davon {len(jetzt)} dringend" if jetzt else "")
          + " — im Chat stellen mit /fragen (nie im Chat improvisieren:")
    print("  der Skill haelt die Antwort woertlich fest und arbeitet sie ein).")
    for o in offen[:3]:
        print(f"  {o.get('id','?'):5} {o.get('art','')} — {o.get('titel','')[:72]}")
    if len(offen) > 3:
        print(f"  … und {len(offen)-3} weitere")
PYEOF
fi

echo "Quellen: docs/plan/plan.json (Text) · Urteilsmarken in docs/beweise/ (Status) · docs/plan/fragen.json (Fragen)"
echo "Regel: docs/PLAN-STAND.md NIE von Hand editieren — es wird gerechnet und vom Hook committet."
