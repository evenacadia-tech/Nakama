#!/usr/bin/env bash
# PreToolUse (Bash|PowerShell): der einzige Riegel, der im Bypass-Modus noch beisst.
#
# Warum es ihn gibt (22.08.2026, User: "ich möchte dass das so automatisch wie
# möglich ist"): Committen und Pushen sollen ohne Rueckfrage laufen. Die
# Berechtigungsliste kann das nicht absichern — global steht
# defaultMode=bypassPermissions, und der Modus "skips permission prompts"
# (Doku code.claude.com/docs/en/permissions). Eine ask-Regel erzeugt einen
# Prompt, und genau der wird uebersprungen: die ask-Liste des Projekts
# (git push, git reset --hard, rm -rf) war damit wirkungslos.
# Dieselbe Doku nennt den Weg: "A blocking hook also takes precedence over allow
# rules. A hook that exits with code 2 stops the tool call before permission
# rules are evaluated." Also: alles erlauben, das Gefaehrliche per Hook blocken.
#
# Geblockt wird genau zweierlei:
#   (a) was die Projektregeln VERBIETEN, weil parallele Sessions im selben Baum
#       arbeiten: `git add -A`/`.`/`-u` ohne Pathspec und `git commit --amend`.
#       Bisher standen diese Regeln nur in CLAUDE.md — als Prosa, die jede
#       Session neu lesen und einhalten musste.
#   (b) was unumkehrbar ist oder veroeffentlichten Stand zerstoert:
#       force-push, reset --hard, clean -f, Massen-Verwurf, Branch-Loeschung.
#       Diese bleiben eine bewusste Entscheidung des Users.
#
# Notausgang: NAKAMA_GIT_RIEGEL_AUS=1 vor den Befehl setzen. Absicht statt
# Einstellungsaenderung — ein Riegel, den man nur durch Editieren der
# settings.json loeswird, wird irgendwann ganz abgeschaltet.
#
# Gegenprobe (beide Richtungen): bash tools/hooks/git-automatik-probe.sh

. "$(dirname "${BASH_SOURCE[0]}")/lib/schreibziel.sh"

input=$(cat)
cmd=$(befehl_ohne_zitate "$input")
[ -n "$cmd" ] || exit 0

# Der Notausgang muss GESETZT sein, nicht ERWAEHNT: am Anfang des Befehls oder
# eines Befehlsstuecks. Vorher genuegte die Zeichenkette irgendwo — auch in
# einem Kommentar (`git reset --hard # NAKAMA_GIT_RIEGEL_AUS=1`) hob sie den
# ganzen Riegel auf (Gegenprobe Gemini 3.1 Pro, 22.08.2026).
if printf '%s' "$cmd" | grep -Eq '(^|[;&|][[:space:]]*)NAKAMA_GIT_RIEGEL_AUS=1[[:space:]]'; then
  exit 0
fi
case "$cmd" in
  *git*) ;;
  *) exit 0 ;;
esac

# Prueftext: EIN Befehlsstueck je Zeile, und in jedem Stueck faellt alles hinter
# -m/-F weg. Zwei Fehlblockaden, die das verhindert (beide beim Bau gemessen):
# `git commit -m "endlich kein git add -A mehr"` — eine Commit-Nachricht ist
# Text, kein Befehl; und die Zerlegung an &&/;/| haelt gleichzeitig den
# umgekehrten Fall dicht, dass ein zweiter Befehl HINTER der Nachricht steht
# (`git commit -m x && git push --force`) und sonst mit ihr weggeschnitten wuerde.
pruef=$(printf '%s' "$cmd" \
  | sed 's/&&/\n/g; s/||/\n/g; s/[;|]/\n/g' \
  | sed 's/[[:space:]]-\(m\|F\)[[:space:]].*$//; s/[[:space:]]--\(message\|file\)[[:space:]].*$//')

# Zwei Erweiterungen aus der Gegenprobe (Gemini 3.1 Pro, 22.08.2026), beide
# gemessen: (1) ein Pfad vor dem Programmnamen (`/usr/bin/git`, `./git`) liess
# das Muster scheitern; (2) ein Vorschalter MIT Argument (`git -C /pfad add -A`,
# `git --git-dir .git push -f`) riss die Kette auf, weil nur argumentlose Flags
# vorgesehen waren — und `git -C` ist die Form, in der die Hooks dieses Repos
# selbst git aufrufen.
V='(^|[[:space:](){}])'             # Befehlsanfang im Stueck
G="${V}([^[:space:]]*/)?git([[:space:]]+-[^[:space:]]+([[:space:]]+[^-][^[:space:]]*)?)*[[:space:]]+"

trifft() { printf '%s' "$pruef" | grep -Eq "$1"; }
riegel() {
  echo "GIT-RIEGEL: $1" >&2
  echo "Notausgang, wenn der User es ausdruecklich will: NAKAMA_GIT_RIEGEL_AUS=1 vor den Befehl." >&2
  exit 2
}

# (a) Projektregeln: parallele Sessions im selben Baum
if trifft "${G}add[[:space:]]+(-A|--all)([[:space:]]|$)" || trifft "${G}add[[:space:]]+\.([[:space:]]|$)"; then
  riegel "git add -A/. nimmt fremde Edits mit. In diesem Baum arbeiten parallele Sessions (CLAUDE.md): nur explizite Pathspecs, z. B. 'git add -- tools/hooks/ docs/x.md'."
fi
# -u braucht die Pruefung ZEILENWEISE: "hat dieses Stueck -u UND keinen Pathspec".
# Zwei getrennte greps ueber den ganzen Text wuerden ein '--' aus einem anderen
# Befehlsstueck als Freibrief lesen.
if printf '%s' "$pruef" | awk '/(^|[ (){}])git( +-[^ ]+)* +add/ && / -u( |$)/ && !/ -- / { gefunden = 1 } END { exit !gefunden }'; then
  riegel "git add -u ohne Pathspec staged jede Aenderung an verfolgten Dateien, auch fremde. Mit Pfad: 'git add -u -- <pfad>'."
fi
if trifft "${G}commit([[:space:]]+[^[:space:]]+)*[[:space:]]+--amend"; then
  riegel "git commit --amend schreibt Historie um, die eine parallele Session oder der zweite Rechner schon haben kann (CLAUDE.md: nie --amend). Stattdessen einen neuen Commit."
fi

# (b) unumkehrbar oder veroeffentlichter Stand
if trifft "${G}push([[:space:]]+[^[:space:]]+)*[[:space:]]+(--force|--force-with-lease|-f)([[:space:]]|=|$)"; then
  riegel "force-push ueberschreibt den Stand im Remote, den der zweite Rechner schon geholt haben kann. Das ist eine Entscheidung des Users, keine Automatik."
fi
# Refspec-Formen: `+ref` erzwingt den Push auch ohne --force, `:ref` loescht.
# Vorher verlangte das Muster hinter dem Doppelpunkt einen BUCHSTABEN, womit
# `git push origin :123-bugfix` durchlief, und das Plus wurde nur mit
# nachfolgendem Doppelpunkt erkannt (Gegenprobe Gemini 3.1 Pro, 22.08.2026).
if trifft "${G}push([[:space:]]+[^[:space:]]+)*[[:space:]]+(--delete|:[A-Za-z0-9_])" \
   || trifft "${G}push[^;&|]*[[:space:]]\+[A-Za-z0-9_][^[:space:]]*"; then
  riegel "Dieser Push loescht oder erzwingt eine Remote-Referenz (:ref loescht, +ref ueberschreibt ohne --force). Entscheidung des Users."
fi
if trifft "${G}reset[[:space:]]+([^[:space:]]+[[:space:]]+)*--hard"; then
  riegel "git reset --hard verwirft uncommittete Arbeit — auch die einer parallelen Session. Sichern (git stash / Kopie) oder gezielt 'git restore -- <pfad>'."
fi
if trifft "${G}clean[[:space:]]+-[a-zA-Z]*f"; then
  riegel "git clean -f loescht unverfolgte Dateien unwiederbringlich. Erst 'git clean -n' zeigen lassen, dann der User entscheidet."
fi
if trifft "${G}(checkout|restore)[[:space:]]+(--[^[:space:]]+[[:space:]]+)*(--[[:space:]]+)?\.([[:space:]]|$)"; then
  riegel "Massen-Verwurf des Arbeitsbaums. Gezielt je Pfad zuruecknehmen: 'git restore -- <pfad>'."
fi
# -D und "-d zusammen mit -f" sind dasselbe: erzwungenes Loeschen, auch nicht
# gemergter Commits (Gegenprobe Gemini 3.1 Pro, 22.08.2026).
if trifft "${G}branch[[:space:]]+([^[:space:]]+[[:space:]]+)*-[a-zA-Z]*D" \
   || printf '%s' "$pruef" | awk '/(^|[ (){}])([^ ]*\/)?git( +-[^ ]+( +[^-][^ ]*)?)* +branch/ && / -[a-zA-Z]*d( |$)/ && / -[a-zA-Z]*f( |$)/ { g = 1 } END { exit !g }'; then
  riegel "Erzwungenes Loeschen eines Branches samt nicht gemergter Commits. Entscheidung des Users."
fi

exit 0
