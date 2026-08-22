#!/usr/bin/env bash
# Gemeinsamer Kern beider Schleusen: "Schreibt dieser Werkzeugaufruf IN den
# Ordner X?" — genau eine Antwort, ein Ort.
#
# Herkunft: die sieben Praezisierungen der Prototyp-Schleuse (21.08.2026,
# jede aus einem echten Fehler) lagen bis zum 22.08. nur in prototyp-schleuse.sh.
# Die Kreativ-Schleuse las weiter nur "file_path" und liess `cat >
# eq-copilot/design/neu.html` ungebremst durch (gemessen 22.08.: exit 0) —
# in einer Arbeitsumgebung, die Dateiaenderungen ueber Bash ausdruecklich
# bevorzugt, war das nicht die theoretische Luecke, sondern der Regelweg.
# Deshalb steht die Logik jetzt hier und wird von beiden Schleusen gelesen.
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

# schreibziel_trifft <json-vom-stdin> <ordner-mit-schraegstrich>
#   Rueckgabe 0 = der Aufruf schreibt in den Ordner, 1 = er tut es nicht.
#   <ordner> ist ein Pfadstueck mit abschliessendem Schraegstrich,
#   z. B. "prototyp/" oder "eq-copilot/design/".
schreibziel_trifft() {
  local input="$1" ordner="$2"
  local fp roh cmd T A E V muster

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
  roh=$(printf '%s' "$input" | sed -n 's/.*"command"[[:space:]]*:[[:space:]]*"//p' | head -1)
  # JSON-Zeilenumbrueche echt machen, dann jeden Heredoc-Rumpf wegwerfen.
  cmd=$(printf '%s' "$roh" | sed 's/\\n/\n/g' | awk '
    ende != "" { if ($0 == ende) ende = ""; next }
    { print
      if (match($0, /<<-?[[:space:]]*["'"'"']?[A-Za-z_][A-Za-z0-9_]*/)) {
        s = substr($0, RSTART, RLENGTH)
        sub(/^<<-?[[:space:]]*["'"'"']?/, "", s)
        ende = s
      } }')
  cmd="${cmd//\\\\//}"; cmd="${cmd//\\//}"; cmd="${cmd//\"/ }"; cmd="${cmd//\'/ }"

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
