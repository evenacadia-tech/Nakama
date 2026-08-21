#!/usr/bin/env bash
# Gegenprobe der Prototyp-Schleuse. PRUEFLISTE 5.1: "Ein Gate, das nicht
# scheitern kann, beweist nichts." Diese Probe prueft BEIDE Richtungen —
# was blocken muss, und was durchmuss. Ohne die zweite Haelfte waere eine
# Schleuse, die alles blockt, "bestanden" — und genau die wird abgeschaltet.
#
# Die Faelle unten sind keine Erfindung: jeder ist bei der Reparatur am
# 2026-08-21 einmal falsch ausgegangen.
#   Aufruf:  bash .claude/hooks/schleusen-probe.sh
cd "$(dirname "$0")/../.." || exit 1
HOOK=.claude/hooks/prototyp-schleuse.sh
fehler=0 gesamt=0

probe(){                       # probe <soll:block|durch> <name> <json>
  local soll="$1" name="$2" json="$3" code ist
  gesamt=$((gesamt+1))
  printf '%s' "$json" | bash "$HOOK" >/dev/null 2>&1; code=$?
  ist=durch; [ "$code" -eq 2 ] && ist=block
  if [ "$ist" = "$soll" ]; then
    printf 'ok    %-5s  %s\n' "$soll" "$name"
  else
    printf 'FEHL  soll=%-5s ist=%-5s  %s\n' "$soll" "$ist" "$name"; fehler=$((fehler+1))
  fi
}
j(){ printf '{"tool_name":"%s","tool_input":{"%s":"%s"}}' "$1" "$2" "$3"; }

echo "--- muss blocken ---"
probe block "Write nach prototyp/ (absolut, Windows)" "$(j Write file_path 'C:\\\\Users\\\\x\\\\prototyp\\\\main.html')"
probe block "Edit in prototyp/ (relativ)"             "$(j Edit  file_path 'prototyp/main.html')"
probe block "Bash: cat > prototyp/"                   "$(j Bash  command   'cat > prototyp/main.html')"
probe block "Bash: cat >./prototyp/"                  "$(j Bash  command   'cat >./prototyp/main.html')"
probe block "Bash: >> anhaengen"                      "$(j Bash  command   'echo x >> prototyp/main.html')"
probe block "Bash: absoluter Umlenkpfad"              "$(j Bash  command   'echo x > C:/Users/x/prototyp/a.html')"
probe block "Bash: cp nach prototyp/"                 "$(j Bash  command   'cp werkzeug/a.html prototyp/a.html')"
probe block "Bash: mv nach prototyp/"                 "$(j Bash  command   'mv a.html prototyp/a.html')"
probe block "Bash: sed -i an Datei in prototyp/"      "$(j Bash  command   'sed -i s/a/b/ prototyp/a.html')"
probe block "Bash: tee nach prototyp/"                "$(j Bash  command   'echo x | tee prototyp/a.html')"
probe block "Bash: node writeFileSync"                "$(j Bash  command   'node -e fs.writeFileSync(prototyp/a.html, x)')"
probe block "Bash: mkdir in prototyp/"                "$(j Bash  command   'mkdir -p prototyp/teile')"

echo
echo "--- muss durchlassen ---"
probe durch "Write nach werkzeug/"                    "$(j Write file_path 'C:\\\\Users\\\\x\\\\werkzeug\\\\a.html')"
probe durch "Schleuse gegen sich selbst"              "$(j Edit  file_path '.claude/hooks/prototyp-schleuse.sh')"
probe durch "Bash: cat prototyp/ (lesen)"             "$(j Bash  command   'cat prototyp/LIES-MICH.md')"
probe durch "Bash: ls prototyp/"                      "$(j Bash  command   'ls -la prototyp/')"
probe durch "Bash: grep in prototyp/"                 "$(j Bash  command   'grep -rn foo prototyp/')"
probe durch "Bash: prototyp/ nur erwaehnt"            "$(j Bash  command   'echo prototyp/ ist noch leer')"
probe durch "Bash: schreiben daneben"                 "$(j Bash  command   'cat > werkzeug/a.html')"
# Der Fall, an dem die erste Bash-Fassung gescheitert ist: geschrieben wird nach
# .claude/, das Wort prototyp/ steht nur im Suchmuster.
probe durch "Bash: sed -i an Datei DANEBEN, Muster nennt prototyp/" \
      "$(j Bash  command   'sed -i s/prototyp-alt/prototyp-neu/ .claude/hooks/schleusen-probe.sh')"
probe durch "Bash: cp WEG von prototyp/"              "$(j Bash  command   'cp prototyp/a.html sicherung/a.html')"
# Beide Fehlblockaden dieser Sitzung waren Heredocs: Text, der einen
# Schreibbefehl bloss ZITIERT. Text ist kein Befehl.
probe durch "Bash: Heredoc zitiert cat > prototyp/"       "$(j Bash  command   'git commit -F - <<EOF
repariert: cat > prototyp/main.html lief durch
EOF')"
probe durch "Bash: Heredoc zitiert cp nach prototyp/"       "$(j Bash  command   'cat <<TXT
cp a.html prototyp/a.html war das Loch
TXT')"
probe block "Bash: echte Umlenkung MIT Heredoc-Rumpf"       "$(j Bash  command   'cat > prototyp/main.html <<EOF
hallo
EOF')"

echo
if [ "$fehler" -eq 0 ]; then
  echo "SCHLEUSE: $gesamt/$gesamt Proben richtig — sie blockt das Richtige und laesst das Richtige durch."
  exit 0
fi
echo "SCHLEUSE: $fehler von $gesamt Proben falsch."
exit 1
