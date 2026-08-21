#!/usr/bin/env bash
# PreToolUse-Schleuse: blockt Schreibzugriffe IM ORDNER prototyp/, solange kein
# abgenommener DESIGNVERTRAG existiert (Arbeitsplan Phase 0).
#
# Sieben Praezisierungen, jede aus einem echten Fehler:
# 1. Es wird auf den Ordner geprueft, nicht auf die Zeichenfolge "prototyp"
#    irgendwo im Pfad — sonst blockt die Schleuse ihre eigene Datei
#    (.claude/hooks/prototyp-schleuse.sh) und jede Datei, die zufaellig so heisst.
# 2. Verlangt wird eine Datei mit "designvertrag" im Namen, nicht irgendeine
#    Abnahme: sonst oeffnet jede beilaeufige Abnahme (Methode, Werkzeug,
#    Sessionnotiz) das Tor fuer etwas, wozu sie nichts sagt.
# 3. Auch BASH wird geprueft. Vorher las die Schleuse nur "file_path" und der
#    Matcher hiess "Write|Edit" — ein `cat > prototyp/main.html` lief ungebremst
#    durch (gemessen 2026-08-21: exit 0 statt exit 2). Da die Arbeitsumgebung
#    Dateiaenderungen ueber Bash ausdruecklich bevorzugt, war das nicht die
#    theoretische Luecke, sondern der wahrscheinliche Weg.
# 4. Aus dem JSON wird der WERT geholt, nicht der Treffer samt Schluesselnamen.
#    Vorher klebte "file_path": bzw. "command": am Wert. Folge: ein relativer
#    Pfad fiel durch das Muster `*/prototyp/*`, und cp/mv am Befehlsanfang
#    galten nicht als Befehlsanfang, weil ein Doppelpunkt davorstand.
# 5. Der Schreibbefehl muss prototyp/ WIRKLICH ANSPRECHEN, nicht bloss im selben
#    Befehl vorkommen. Eine Schleuse, die den falschen Handgriff blockt, wird
#    abgeschaltet — und schuetzt danach gar nichts.
# 6. Bei cp/mv/ln/rsync/install ist nur das LETZTE Argument das Ziel.
#    `cp prototyp/a.html sicherung/a.html` rettet etwas heraus und muss durch.
# 7. HEREDOC-RUEMPFE werden vorher entfernt. Beide falschen Blockaden dieser
#    Sitzung waren Heredocs: einmal ein sed-Muster, einmal eine Commit-
#    Nachricht, die `cat > prototyp/main.html` bloss ZITIERTE. Text ist kein
#    Befehl.
#
# Gegenprobe (beide Richtungen): .claude/hooks/schleusen-probe.sh
input=$(cat)

# --- betrifft der Aufruf den Ordner prototyp/ ueberhaupt? -------------------
betroffen=0

# a) Werkzeuge mit file_path (Write, Edit, NotebookEdit …) — exakt, ohne Raten
fp=$(printf '%s' "$input" \
     | sed -n 's/.*"file_path"[[:space:]]*:[[:space:]]*"\([^"]*\)".*/\1/p' | head -1)
fp="${fp//\\//}"   # Windows-Backslashes zu Schraegstrichen
case "$fp" in
  */prototyp/*|prototyp/*) betroffen=1 ;;
esac

# b) Bash: nur SCHREIBEN blockt — Lesen (cat, ls, grep) nicht. Und geblockt wird
#    nur, wenn das ZIEL des Schreibbefehls in prototyp/ liegt.
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
  *prototyp/*)
    T='[^[:space:]|;&<>]*prototyp/[^[:space:]|;&<>]*'   # ein Pfad IN prototyp/
    A='[^[:space:]|;&<>]+'                              # irgendein Argument
    E='[[:space:]]*($|[|;&})])'                         # Ende des Befehlsstuecks
    V='(^|[|;&(){}[:space:]])'                          # Befehlsanfang
    treffer(){ printf '%s' "$cmd" | grep -Eq "$1" && betroffen=1; }

    treffer ">[[:space:]]*$T"                                    # > prototyp/x
    treffer "${V}(cp|mv|install|rsync|ln)([[:space:]]+$A){0,4}[[:space:]]+$T$E"
    treffer "${V}(tee|touch|mkdir)([[:space:]]+$A){0,3}[[:space:]]+$T"
    treffer "sed[[:space:]]+(-[^[:space:]]+[[:space:]]+)*-i([[:space:]]+$A){0,2}[[:space:]]+$T"
    treffer "of=$T"                                              # dd
    treffer "(writeFileSync|appendFileSync|createWriteStream|Out-File|Set-Content|New-Item)[^)]{0,80}prototyp/"
    ;;
esac

[ "$betroffen" -eq 1 ] || exit 0

# --- liegt ein abgenommener Designvertrag vor? -----------------------------
# SCHLEUSE_ABNAHMEN: nur fuer die Gegenprobe (schleusen-probe.sh), die den
# GESCHLOSSENEN Zustand gegen ein leeres Verzeichnis misst, ohne den echten
# Vertrag anzufassen. Im Betrieb bleibt es abnahmen/.
ABNAHMEN="${SCHLEUSE_ABNAHMEN:-abnahmen}"
count=0
for f in "$ABNAHMEN"/*designvertrag*.md; do
  [ -e "$f" ] || continue
  count=$((count + 1))
done
if [ "$count" -eq 0 ]; then
  echo "PROTOTYP-SCHLEUSE: Es liegt keine Designvertrag-Abnahme in abnahmen/." >&2
  echo "Der Designvertrag entsteht aus dem User-Wort (Interview/Figma-Stand) und wird als" >&2
  echo "abnahmen/JJJJ-MM-TT-designvertrag.md festgehalten (docs/arbeitsplan.md," >&2
  echo "Phase 0) — dann darf in prototyp/ gebaut werden." >&2
  echo "Bis dahin gehoeren lebende Blaetter nach werkzeug/." >&2
  exit 2
fi
exit 0
