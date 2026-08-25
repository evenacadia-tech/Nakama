#!/usr/bin/env bash
# Kern der aktiven Kreativ-Schleuse: "Schreibt dieser Werkzeugaufruf IN den
# Ordner X?" — genau eine Antwort, ein Ort. Die frühere Prototyp-Schleuse hat
# dieselbe Logik bis zu ihrer Stilllegung am 24.08.2026 verwendet.
#
# Herkunft: die sieben Praezisierungen der Prototyp-Schleuse (21.08.2026,
# jede aus einem echten Fehler) lagen bis zum 22.08. nur in prototyp-schleuse.sh.
# Die Kreativ-Schleuse las weiter nur "file_path" und liess `cat >
# eq-copilot/design/neu.html` ungebremst durch (gemessen 22.08.: exit 0) —
# in einer Arbeitsumgebung, die Dateiaenderungen ueber Bash ausdruecklich
# bevorzugt, war das nicht die theoretische Luecke, sondern der Regelweg.
# Deshalb steht die Logik hier und wird von der Kreativ-Schleuse gelesen.
#
# Die sieben Praezisierungen, die dieser Kern traegt:
# 1. Geprueft wird auf den ORDNER als Pfadsegment, nicht auf die Zeichenfolge
#    irgendwo im Pfad — sonst blockt eine Schleuse ihre eigene Datei.
# 2. Auch BASH wird geprueft, nicht nur Werkzeuge mit file_path.
# 3. Aus dem JSON wird der WERT geholt, nicht der Treffer samt Schluesselnamen.
# 4. Der Befehl muss den Ordner WIRKLICH ANSPRECHEN, nicht bloss erwaehnen.
#    Eine Schleuse, die den falschen Handgriff blockt, wird abgeschaltet —
#    und schuetzt danach gar nichts.
# 5. Bei cp/mv/ln/rsync/install ist nur das LETZTE Argument das Ziel:
#    `cp X/a.html sicherung/a.html` rettet etwas heraus und muss durch.
# 6. HEREDOC-RUEMPFE werden vorher entfernt. Text, der einen Schreibbefehl
#    bloss ZITIERT (Commit-Nachricht, sed-Muster), ist kein Befehl.
# 7. Lesen (cat, ls, grep) blockt nie.
#
# Gegenprobe beider Richtungen, beide Schleusen: bash tools/hooks/schleusen-probe.sh

# befehl_klartext <json-vom-stdin>
#   Gibt den Bash-/PowerShell-Befehl als Klartext aus: JSON-Zeilenumbrueche echt,
#   HEREDOC-RUEMPFE entfernt, Backslashes zu Schraegstrichen, Anfuehrungszeichen
#   zu Leerzeichen. Wer einen Befehl bewertet, bewertet IHN — nicht den Text, den
#   er transportiert: beide Fehlblockaden vom 21.08.2026 waren Heredocs, die
#   einen Schreibbefehl bloss zitierten (Commit-Nachricht, sed-Muster).
befehl_klartext_roh() {
  local roh
  roh=$(printf '%s' "$1" | sed -n 's/.*"command"[[:space:]]*:[[:space:]]*"//p' | head -1)
  printf '%s' "$roh" | sed 's/\\n/\n/g' | awk '
    ende != "" { if ($0 == ende) ende = ""; next }
    { print
      if (match($0, /<<-?[[:space:]]*["'"'"']?[A-Za-z_][A-Za-z0-9_]*/)) {
        s = substr($0, RSTART, RLENGTH)
        sub(/^<<-?[[:space:]]*["'"'"']?/, "", s)
        ende = s
      } }'
}

befehl_klartext() {
  local cmd
  cmd=$(befehl_klartext_roh "$1")
  cmd="${cmd//\\\\//}"; cmd="${cmd//\\//}"; cmd="${cmd//\"/ }"; cmd="${cmd//\'/ }"
  printf '%s' "$cmd"
}

# befehl_ohne_zitate <json-vom-stdin>
#   Wie befehl_klartext, entfernt aber zusaetzlich JEDE Zeichenkette in
#   Anfuehrungszeichen. Fuer Werkzeuge, die BEFEHLE bewerten (git-riegel.sh):
#   was in Anfuehrungszeichen steht, ist ein Argument — Text, den ein Programm
#   liest, nie ein Befehl, den die Shell ausfuehrt.
#   Gemessen am 22.08.2026 (Gegenprobe durch Gemini 3.1 Pro): ohne diesen Schritt
#   blockte der Riegel `echo "Bitte niemals git push --force nutzen"`,
#   `grep "git add -A" README.md` und jede Commit-Nachricht mit Semikolon.
#   NICHT fuer die Schleusen: die suchen PFADE, und ein Pfad darf in
#   Anfuehrungszeichen stehen (`cat > "prototyp/a.html"`).
befehl_ohne_zitate() {
  local cmd
  cmd=$(befehl_klartext_roh "$1")
  # Erst die Zitate weg, DANN normalisieren — sonst sind die Anfuehrungszeichen
  # schon Leerzeichen und die Grenze der Zeichenkette nicht mehr auffindbar.
  cmd=$(printf '%s' "$cmd" | sed 's/"[^"]*"/ /g' | sed "s/'[^']*'/ /g")
  cmd="${cmd//\\\\//}"; cmd="${cmd//\\//}"
  # Uebrig gebliebene EINZELNE Anfuehrungszeichen zu Leerzeichen. Klingt nach
  # Kosmetik, ist es nicht: die Rohextraktion schneidet den Wert nicht am Ende
  # ab, der Befehl traegt also immer noch den JSON-Schwanz ("} ...). Ohne diesen
  # Schritt endet `git add -A` als `git add -A"}}` — und jedes Muster, das nach
  # dem Flag ein Leerzeichen oder Zeilenende verlangt, greift nicht mehr.
  # Gemessen 22.08.2026: der Riegel liess danach seine eigenen Kernfaelle durch.
  cmd="${cmd//\"/ }"; cmd="${cmd//\'/ }"
  printf '%s' "$cmd"
}

# schreibziel_trifft <json-vom-stdin> <ordner-mit-schraegstrich>
#   Rueckgabe 0 = der Aufruf schreibt in den Ordner, 1 = er tut es nicht.
#   <ordner> ist ein Pfadstueck mit abschliessendem Schraegstrich,
#   z. B. "prototyp/" oder "eq-copilot/design/".
schreibziel_trifft() {
  local input="$1" ordner="$2"
  local fp cmd T A E V muster

  # a) Werkzeuge mit file_path (Write, Edit, MultiEdit, NotebookEdit …)
  fp=$(printf '%s' "$input" \
       | sed -n 's/.*"file_path"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' | head -1)
  # Windows-Backslashes zu Schraegstrichen — ZWEISTUFIG, dann Doppelungen weg.
  # Im JSON steht ein Windows-Pfad als "C:\\Users\\x"; roh extrahiert bleibt das
  # Paar stehen. Ein einstufiges Ersetzen macht daraus "C://Users//x" — und ein
  # Ordnermuster aus ZWEI Segmenten ("eq-copilot/design/") findet sich darin
  # nicht wieder, waehrend ein einsegmentiges ("prototyp/") zufaellig ueberlebt.
  # Genau daran ist die erste Fassung dieses Kerns in der Gegenprobe gescheitert
  # (22.08.2026, Fall "Edit (absolut, Windows)").
  fp="${fp//\\\\//}"; fp="${fp//\\//}"
  while [ "$fp" != "${fp//\/\//\/}" ]; do fp="${fp//\/\//\/}"; done
  case "$fp" in
    */"$ordner"*|"$ordner"*) return 0 ;;
  esac

  # b) Bash: nur SCHREIBEN blockt, und nur wenn das ZIEL im Ordner liegt.
  cmd=$(befehl_klartext "$input")

  case "$cmd" in
    *"$ordner"*) ;;
    *) return 1 ;;
  esac

  T="[^[:space:]|;&<>]*${ordner}[^[:space:]|;&<>]*"   # ein Pfad IM Ordner
  A="[^[:space:]|;&<>]+"                              # irgendein Argument
  E="[[:space:]]*(\$|[|;&})])"                        # Ende des Befehlsstuecks
  V="(^|[|;&(){}[:space:]])"                          # Befehlsanfang

  for muster in \
    ">[[:space:]]*$T" \
    "${V}(cp|mv|install|rsync|ln)([[:space:]]+$A){0,4}[[:space:]]+$T$E" \
    "${V}(tee|touch|mkdir)([[:space:]]+$A){0,3}[[:space:]]+$T" \
    "sed[[:space:]]+(-[^[:space:]]+[[:space:]]+)*-i([[:space:]]+$A){0,2}[[:space:]]+$T" \
    "of=$T" \
    "(writeFileSync|appendFileSync|createWriteStream|Out-File|Set-Content|New-Item)[^)]{0,80}${ordner}"
  do
    printf '%s' "$cmd" | grep -Eq "$muster" && return 0
  done

  return 1
}
