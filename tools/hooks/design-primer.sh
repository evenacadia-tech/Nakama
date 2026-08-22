#!/usr/bin/env bash
# SessionStart-Primer für den Design-Ordner design/ (seit 22.08.2026 im
# Nakama-Repo; vorher eigenes Repo Nakama-Design). Er MISST, was da ist, und
# behauptet nichts: Zahl der Projektquellen, Figma-Stände in der Truhe,
# Zustand der Prototyp-Schleuse. Die Regeln der Designarbeit stehen in
# design/LIES-MICH.md und CLAUDE.md (Design-Arbeitsmodell) — hier nicht
# abgeschrieben.
#
# Warum gezählt statt geschrieben: bis 21.08. stand hier "vier" Quellen, während
# fünf Dateien lagen; und "Schleuse zu", als der Designvertrag längst vorlag.
D="${CLAUDE_PROJECT_DIR:-.}/design"
[ -d "$D" ] || exit 0

n=$(ls "$D"/docs/*.md 2>/dev/null | wc -l | tr -d ' ')
vertrag=$(ls "$D"/abnahmen/*designvertrag*.md 2>/dev/null | head -1 | sed "s#^$D/##")
if [ -n "$vertrag" ]; then
  schleuse="OFFEN (design/$vertrag liegt vor)"
else
  schleuse="ZU (keine Datei mit 'designvertrag' im Namen in design/abnahmen/)"
fi
staende=$(ls "$D"/assets/figma/*.png 2>/dev/null | sed 's#.*/##' | tr '\n' ' ')
[ -n "$staende" ] || staende="(noch keiner abgelegt)"
juengste=$(ls "$D"/abnahmen/2*.md 2>/dev/null | sed 's#.*/##' | sort | tail -1)

cat <<TXT
=== Design (design/ — Figma des Users ist die Quelle, das Repo übersetzt) ===
Regeln: design/LIES-MICH.md (8 harte Regeln) · Fahrplan: design/docs/arbeitsplan.md
($n Projektquellen in design/docs/). Abnahmen: design/abnahmen/ (jüngste: ${juengste:-keine}).
Figma-Stände in der Truhe design/assets/figma/: $staende
Prototyp-Schleuse (tools/hooks/prototyp-schleuse.sh): $schleuse
Produkt englisch, Docs/Abnahmen/Commits deutsch. Richtung = User in Figma; Claude erfindet keine.
TXT
exit 0
