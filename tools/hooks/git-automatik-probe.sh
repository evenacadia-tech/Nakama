#!/usr/bin/env bash
# Gegenprobe der Git-Automatik: Riegel (blockt) und Auto-Push (schickt raus).
# PRUEFLISTE 5.1: "Ein Gate, das nicht scheitern kann, beweist nichts" — deshalb
# beide Richtungen. Ein Riegel, der zu viel blockt, wird abgeschaltet und
# schuetzt danach gar nichts; genau daran ist die Prototyp-Schleuse am 21.08.
# zweimal gescheitert.
#   Aufruf (vom Repo-Root):  bash tools/hooks/git-automatik-probe.sh
cd "$(dirname "$0")/../.." || exit 1
RIEGEL=tools/hooks/git-riegel.sh
fehler=0 gesamt=0

probe(){                       # probe <soll:block|durch> <name> <befehl>
  local soll="$1" name="$2" befehl="$3" code ist json
  gesamt=$((gesamt+1))
  json=$(printf '{"tool_name":"Bash","tool_input":{"command":"%s"}}' "$befehl")
  printf '%s' "$json" | bash "$RIEGEL" >/dev/null 2>&1; code=$?
  ist=durch; [ "$code" -eq 2 ] && ist=block
  if [ "$ist" = "$soll" ]; then
    printf 'ok    %-5s  %s\n' "$soll" "$name"
  else
    printf 'FEHL  soll=%-5s ist=%-5s  %s\n' "$soll" "$ist" "$name"; fehler=$((fehler+1))
  fi
}

echo "=== GIT-RIEGEL: muss blocken (Projektregel) ==="
probe block "git add -A"                  'git add -A'
probe block "git add --all"               'git add --all'
probe block "git add ."                   'git add .'
probe block "git add -u ohne Pathspec"    'git add -u'
probe block "commit --amend"              'git commit --amend --no-edit'
probe block "commit -q --amend"           'git commit -q --amend'
probe block "add -A hinter &&"            'git status && git add -A'

echo
echo "=== GIT-RIEGEL: muss blocken (unumkehrbar / veroeffentlicht) ==="
probe block "push --force"                'git push --force'
probe block "push -f origin master"       'git push -f origin master'
probe block "push --force-with-lease"     'git push --force-with-lease'
probe block "push --delete"               'git push origin --delete alt'
probe block "reset --hard"                'git reset --hard HEAD~1'
probe block "clean -fd"                   'git clean -fd'
probe block "checkout -- ."               'git checkout -- .'
probe block "restore ."                   'git restore .'
probe block "branch -D"                   'git branch -D alt'
probe block "push --force hinter &&"      'git commit -m x && git push --force'

echo
echo "=== GIT-RIEGEL: muss durchlassen (der Alltag) ==="
probe durch "add mit Pathspec"            'git add -- tools/hooks/ docs/x.md'
probe durch "add -u MIT Pathspec"         'git add -u -- docs/handoffs'
probe durch "commit -F -"                 'git commit -q -F -'
probe durch "commit -m normal"            'git commit -m Hooks aufgeraeumt'
probe durch "push normal"                 'git push'
probe durch "pull --rebase"               'git pull --rebase'
probe durch "rm --cached"                 'git rm -q --cached alt.md'
probe durch "clean -n (Trockenlauf)"      'git clean -n'
probe durch "restore mit Pfad"            'git restore -- docs/x.md'
probe durch "status/log/diff"             'git status --short && git log --oneline -3'
# Text ist kein Befehl — beide Faelle waren beim Bau echte Fehlblockaden.
probe durch "Nachricht nennt add -A"      'git commit -m endlich kein git add -A mehr'
probe durch "Nachricht nennt --amend"     'git commit -m nie --amend in diesem Baum'
probe durch "Heredoc zitiert force-push"  'git commit -F - <<EOF
Riegel: git push --force bleibt Entscheidung des Users
EOF'
probe durch "Notausgang gesetzt"          'NAKAMA_GIT_RIEGEL_AUS=1 git push --force'
probe durch "gar kein git"                'py -3.13 tools/hub/hub_sync.py holen'

echo
echo "=== Gegenprobe von Gemini 3.1 Pro (22.08.2026, agy -p) ==="
# Zweite Stimme, adversarial: der Riegel-Quelltext ging als Text hinein, die
# Faelle kamen zurueck. Jeder wurde hier gemessen, keiner geglaubt — einer der
# neun Befunde war falsch (Push auf eine URL blockte nie).
echo "--- muss blocken (von Gemini gefundene Luecken) ---"
probe block "git mit absolutem Pfad"      '/usr/bin/git push --force'
probe block "git -C mit Argument"         'git -C /pfad/zum/repo add -A'
probe block "git --git-dir mit Argument"  'git --git-dir .git push -f'
probe block "force per Refspec +branch"   'git push origin +master'
probe block "Loeschung numerischer Branch" 'git push origin :123-bugfix'
probe block "branch -d -f statt -D"       'git branch -d -f alt'
probe block "Notausgang nur im Kommentar" 'git reset --hard # NAKAMA_GIT_RIEGEL_AUS=1'

echo "--- muss durchlassen (von Gemini gefundene Fehlblockaden) ---"
probe durch "Semikolon in der Nachricht"  'git commit -m "fix bug ; git reset --hard war die Ursache"'
probe durch "echo nennt force-push"       'echo "Bitte niemals git push --force nutzen"'
probe durch "grep sucht nach add -A"      'grep "git add -A" README.md'
probe durch "Push auf direkte URL"        'git push https://github.com/evenacadia-tech/Nakama.git master'
# Der Notausgang muss weiter funktionieren, wenn er ECHT gesetzt ist.
probe durch "Notausgang am Befehlsanfang" 'NAKAMA_GIT_RIEGEL_AUS=1 git push --force'
probe durch "Notausgang hinter &&"        'git status && NAKAMA_GIT_RIEGEL_AUS=1 git reset --hard'

echo
echo "=== FREMDMODELL-RIEGEL (Gemini/agy bleibt lesend) ==="
# Liegt in dieser Probe, weil sie der eine Lauf fuer alle Riegel ist: ein
# zweiter Probenlaeufer waere ein zweiter Ort, an dem jemand vergisst ihn zu
# starten. User-Regel 22.08.2026: "ausschlisslich für audits und read only".
fprobe(){                      # fprobe <soll:block|durch> <name> <befehl>
  local soll="$1" name="$2" befehl="$3" code ist json
  gesamt=$((gesamt+1))
  json=$(printf '{"tool_name":"Bash","tool_input":{"command":"%s"}}' "$befehl")
  printf '%s' "$json" | bash tools/hooks/fremdmodell-riegel.sh >/dev/null 2>&1; code=$?
  ist=durch; [ "$code" -eq 2 ] && ist=block
  if [ "$ist" = "$soll" ]; then
    printf 'ok    %-5s  %s\n' "$soll" "$name"
  else
    printf 'FEHL  soll=%-5s ist=%-5s  %s\n' "$soll" "$ist" "$name"; fehler=$((fehler+1))
  fi
}
echo "--- muss blocken (macht agy zum Schreiber) ---"
fprobe block "skip-permissions"          'agy -p "pruefe das" --dangerously-skip-permissions'
fprobe block "skip-permissions absolut"  'C:/Users/phili/AppData/Local/agy/bin/agy.exe -p x --dangerously-skip-permissions'
fprobe block "mode accept-edits"         'agy -p "aendere das" --mode accept-edits'
fprobe block "mode=accept-edits"         'agy -p x --mode=accept-edits'
fprobe block "hinter &&"                 'git status && agy -p x --dangerously-skip-permissions'
echo "--- muss durchlassen (Pruefen ist erlaubt) ---"
fprobe durch "normaler Pruefauftrag"     'agy -p "finde Luecken in diesem Code" --model gemini-3.1-pro-high'
fprobe durch "Prompt aus Datei"          'agy -p "$(cat /tmp/pruefung.txt)" --model gemini-3.1-pro-high --output-format text'
fprobe durch "mode plan"                 'agy -p x --mode plan'
fprobe durch "Text erwaehnt den Schalter" 'echo "niemals agy --dangerously-skip-permissions benutzen"'
fprobe durch "anderes Werkzeug"          'codex exec --dangerously-skip-permissions'

echo
echo "=== AUTO-PUSH: Torwaechter (ohne Netzzugriff pruefbar) ==="
gesamt=$((gesamt+1))
AUS=$(printf '{"session_id":"probe","tool_name":"Bash"}' | bash tools/hooks/auto-push.sh 2>&1)
VORAUS=$(git rev-list --count "@{u}..HEAD" 2>/dev/null || echo "?")
if [ "$VORAUS" = "0" ] && [ -z "$AUS" ]; then
  printf 'ok    %-5s  %s\n' "still" "nichts voraus -> kein Netzzugriff, keine Ausgabe"
elif [ "$VORAUS" != "0" ] && printf '%s' "$AUS" | grep -q "auto-push"; then
  printf 'ok    %-5s  %s\n' "aktiv" "$VORAUS Commit(s) voraus -> Hook hat gehandelt"
else
  printf 'FEHL  voraus=%s  Ausgabe=%s\n' "$VORAUS" "${AUS:-(leer)}"; fehler=$((fehler+1))
fi

echo
if [ "$fehler" -eq 0 ]; then
  echo "GIT-AUTOMATIK: $gesamt/$gesamt Proben richtig."
  exit 0
fi
echo "GIT-AUTOMATIK: $fehler von $gesamt Proben falsch."
exit 1
