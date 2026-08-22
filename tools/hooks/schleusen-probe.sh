#!/usr/bin/env bash
# Gegenprobe BEIDER Schleusen (Prototyp + Kreativ). PRUEFLISTE 5.1: "Ein Gate,
# das nicht scheitern kann, beweist nichts." Diese Probe prueft BEIDE
# Richtungen — was blocken muss, und was durchmuss. Ohne die zweite Haelfte
# waere eine Schleuse, die alles blockt, "bestanden" — und genau die wird
# abgeschaltet.
#
# Die Faelle unten sind keine Erfindung: jeder Prototyp-Fall ist bei der
# Reparatur am 2026-08-21 einmal falsch ausgegangen, und die Kreativ-Faelle
# stehen fuer das Loch, das am 2026-08-22 gemessen wurde (Bash-Schreibbefehl
# lief mit exit 0 durch, weil die Schleuse nur "file_path" las).
#   Aufruf (vom Repo-Root):  bash tools/hooks/schleusen-probe.sh
# Beide Schleusen teilen sich seit 22.08.2026 den Kern lib/schreibziel.sh —
# diese Probe misst ihn ueber beide Hooks.
cd "$(dirname "$0")/../.." || exit 1
HOOK=tools/hooks/prototyp-schleuse.sh
KHOOK=tools/hooks/kreativ-schleuse.sh
fehler=0 gesamt=0

# Die "muss blocken"-Haelfte misst den GESCHLOSSENEN Zustand: sie zeigt der
# Schleuse ein leeres Abnahmen-Verzeichnis (SCHLEUSE_ABNAHMEN) bzw. einen
# Marker, den es nicht gibt (KREATIV_MARKER), damit die Probe auch dann etwas
# beweist, wenn Vertrag und Freigabe laengst liegen (seit 2026-08-21 bzw.
# 2026-08-22 liegen sie; vorher meldete die Probe dann 13/24 falsch).
LEER=$(mktemp -d)
trap 'rm -rf "$LEER"' EXIT
ABN="$LEER"                    # Standard: geschlossen; Abschnitt 3 schaltet um
MARKER="$LEER/gibt-es-nicht.md"

probe(){                       # probe <soll:block|durch> <name> <json>
  local soll="$1" name="$2" json="$3" code ist
  gesamt=$((gesamt+1))
  printf '%s' "$json" | SCHLEUSE_ABNAHMEN="$ABN" bash "$HOOK" >/dev/null 2>&1; code=$?
  ist=durch; [ "$code" -eq 2 ] && ist=block
  if [ "$ist" = "$soll" ]; then
    printf 'ok    %-5s  %s\n' "$soll" "$name"
  else
    printf 'FEHL  soll=%-5s ist=%-5s  %s\n' "$soll" "$ist" "$name"; fehler=$((fehler+1))
  fi
}
kprobe(){                      # kprobe <soll:block|durch> <name> <json>
  local soll="$1" name="$2" json="$3" code ist
  gesamt=$((gesamt+1))
  printf '%s' "$json" | KREATIV_MARKER="$MARKER" bash "$KHOOK" >/dev/null 2>&1; code=$?
  ist=durch; [ "$code" -eq 2 ] && ist=block
  if [ "$ist" = "$soll" ]; then
    printf 'ok    %-5s  %s\n' "$soll" "$name"
  else
    printf 'FEHL  soll=%-5s ist=%-5s  %s\n' "$soll" "$ist" "$name"; fehler=$((fehler+1))
  fi
}
j(){ printf '{"tool_name":"%s","tool_input":{"%s":"%s"}}' "$1" "$2" "$3"; }

echo "=== PROTOTYP-SCHLEUSE (design/prototyp/, Vertrag in design/abnahmen/) ==="
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
probe durch "Schleuse gegen sich selbst"              "$(j Edit  file_path 'tools/hooks/prototyp-schleuse.sh')"
probe durch "Bash: cat prototyp/ (lesen)"             "$(j Bash  command   'cat prototyp/LIES-MICH.md')"
probe durch "Bash: ls prototyp/"                      "$(j Bash  command   'ls -la prototyp/')"
probe durch "Bash: grep in prototyp/"                 "$(j Bash  command   'grep -rn foo prototyp/')"
probe durch "Bash: prototyp/ nur erwaehnt"            "$(j Bash  command   'echo prototyp/ ist noch leer')"
probe durch "Bash: schreiben daneben"                 "$(j Bash  command   'cat > werkzeug/a.html')"
# Der Fall, an dem die erste Bash-Fassung gescheitert ist: geschrieben wird nach
# .claude/, das Wort prototyp/ steht nur im Suchmuster.
probe durch "Bash: sed -i an Datei DANEBEN, Muster nennt prototyp/" \
      "$(j Bash  command   'sed -i s/prototyp-alt/prototyp-neu/ tools/hooks/schleusen-probe.sh')"
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
echo "--- mit Vertrag (echtes design/abnahmen/): muss durchlassen ---"
ABN=design/abnahmen
if ls design/abnahmen/*designvertrag*.md >/dev/null 2>&1; then
  probe durch "Write nach prototyp/ MIT Vertrag"      "$(j Write file_path 'prototyp/gen.html')"
  probe durch "Bash: cat > prototyp/ MIT Vertrag"     "$(j Bash  command   'cat > prototyp/gen.html')"
else
  echo "(kein Designvertrag in design/abnahmen/ — Abschnitt uebersprungen)"
fi

echo
echo "=== KREATIV-SCHLEUSE (eq-copilot/design/, Freigabe .claude/kreativ-freigabe.md) ==="
echo "--- muss blocken ---"
kprobe block "Write nach eq-copilot/design/"          "$(j Write file_path 'eq-copilot/design/tokens.json')"
kprobe block "Edit (absolut, Windows)"                "$(j Edit  file_path 'C:\\\\Users\\\\x\\\\Nakama\\\\eq-copilot\\\\design\\\\a.html')"
# Das am 22.08. gemessene Loch: Bash war die bevorzugte Editierform und lief durch.
kprobe block "Bash: cat > eq-copilot/design/"         "$(j Bash  command   'cat > eq-copilot/design/neu.html')"
kprobe block "Bash: sed -i im Archiv"                 "$(j Bash  command   'sed -i s/a/b/ eq-copilot/design/archive/alt-2026-08/x.html')"
# Loeste bis 22.08. der eigene Waechter guard-codex-besitz.sh; jetzt hier mit drin.
kprobe block "Bash: Codex-Vorentwurf im Archiv"       "$(j Bash  command   'cat > eq-copilot/design/archive/alt-2026-08/nakama-spectral-field-vorentwurf.html')"
kprobe block "Bash: cp in die Prisma-Studie"          "$(j Bash  command   'cp a.png eq-copilot/design/prisma-studie/a.png')"

echo
echo "--- muss durchlassen ---"
kprobe durch "Write nach design/ (neues Design-Repo)" "$(j Write file_path 'design/abnahmen/2026-08-22-x.md')"
kprobe durch "Edit im Plugin-Code"                    "$(j Edit  file_path 'eq-copilot/plugin/src/PluginEditor.cpp')"
kprobe durch "Bash: lesen im Archiv"                  "$(j Bash  command   'cat eq-copilot/design/archive/alt-2026-08/x.html')"
kprobe durch "Bash: grep in eq-copilot/design/"       "$(j Bash  command   'grep -rn token eq-copilot/design/')"
kprobe durch "Bash: Pfad nur erwaehnt"                "$(j Bash  command   'echo eq-copilot/design/ ist Provisorium')"
kprobe durch "Bash: cp WEG aus eq-copilot/design/"    "$(j Bash  command   'cp eq-copilot/design/tokens.json sicherung/tokens.json')"
kprobe durch "Bash: Heredoc zitiert den Schreibbefehl" "$(j Bash  command   'git commit -F - <<EOF
Loch: cat > eq-copilot/design/neu.html lief durch
EOF')"

echo
echo "--- mit frischer Freigabe: muss durchlassen ---"
MARKER="$LEER/kreativ-freigabe.md"; : > "$MARKER"
kprobe durch "Write nach eq-copilot/design/ MIT Freigabe" "$(j Write file_path 'eq-copilot/design/tokens.json')"
kprobe durch "Bash: cat > … MIT Freigabe"                "$(j Bash  command   'cat > eq-copilot/design/neu.html')"
echo "--- mit verbrauchter Freigabe (>24 h): muss blocken ---"
touch -d '2 days ago' "$MARKER" 2>/dev/null || touch -t 202608200900 "$MARKER"
kprobe block "Write MIT abgelaufener Freigabe"           "$(j Write file_path 'eq-copilot/design/tokens.json')"

echo
if [ "$fehler" -eq 0 ]; then
  echo "SCHLEUSEN: $gesamt/$gesamt Proben richtig — sie blocken das Richtige und lassen das Richtige durch."
  exit 0
fi
echo "SCHLEUSEN: $fehler von $gesamt Proben falsch."
exit 1
