#!/usr/bin/env bash
# SessionStart-Primer: hält jede Session (insbes. Opus) im engen Rahmen.
#
# Die Zahl der Quellen wird GEZAEHLT, nicht geschrieben. Sie stand hier bis zum
# 2026-08-21 als "vier", waehrend fuenf Dateien in docs/ lagen und CLAUDE.md
# von fuenf sprach — eine falsche Zahl im Ersten, was jede Session liest.
# PRUEFLISTE 4.3: Zahlwoerter herleiten, nicht schreiben.
n=$(ls docs/*.md 2>/dev/null | wc -l | tr -d ' ')
[ "$n" -gt 0 ] && quellen="Die $n Projektquellen in docs/ sind vollstaendig." \
               || quellen="Die Projektquellen in docs/ sind vollstaendig."

cat <<EOF
=== NAKAMA-DESIGN Primer ===
1. FAHRPLAN: docs/arbeitsplan.md sagt, wo wir stehen und was als
   Naechstes kommt. Phase 0 (Designvertrag) ist NICHT erledigt — die
   Schleuse vor prototyp/ ist deshalb weiterhin zu.
2. ASSETTRUHE ZUERST: assets/ ist vor jeder Gestaltungsentscheidung die
   erste Quelle. Verwenden statt erfinden; Fehlendes beim User erfragen.
   ACHTUNG: assets/nakama-stilprofil.json stammt aus der VERWORFENEN Welt
   (Stand vor der Weltenwahl) und ist kein gueltiger Tokensatz.
3. EIN SCHRITT PRO RUNDE: eine Frage oeffnen, am lebenden Blatt zeigen,
   User-Urteil einholen, in abnahmen/ festhalten. Dann erst weiter.
4. RICHTUNG = USER: bei Gabelungen 2-3 Varianten NEBENEINANDER zeigen,
   nie selbst die Stilrichtung waehlen. Abnahmen sind bindend. Unklare
   Vorstellung? /interview <phase> — fragen ist billig, Fehlbau nicht.
   Welt und Fassung sind ARBEITSANNAHMEN, nicht abgenommen.
5. METHODE: docs/CLAUDE-UX-UI-ARBEITSKERN.md bei jeder Designaufgabe
   befolgen (UX vor UI, Wireframe vor Politur, Schlusscheck).
6. SICHTBARKEIT: docs/DESIGN-GESETZE.md VOR jeder Anordnungs- und
   Sichtbarkeitsentscheidung lesen. Ohne Aufgabenliste nach Haeufigkeit
   kein Wireframe.
7. KEIN KONTEXT-IMPORT: nicht in fremden Ordnern/Repos/Altdokumenten
   recherchieren. $quellen
8. BEWEIS: jedes gezeigte Ergebnis mit Playwright-Screenshot belegen.
   werkzeug/PRUEFLISTE.md sagt, was vorher gemessen wird.
EOF
