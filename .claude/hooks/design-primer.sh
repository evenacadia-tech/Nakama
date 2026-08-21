#!/usr/bin/env bash
# SessionStart-Primer: hält jede Session (insbes. Opus) im engen Rahmen.
#
# Die Zahl der Quellen wird GEZAEHLT, nicht geschrieben. Sie stand hier bis zum
# 2026-08-21 als "vier", waehrend fuenf Dateien in docs/ lagen und CLAUDE.md
# von fuenf sprach — eine falsche Zahl im Ersten, was jede Session liest.
# PRUEFLISTE 4.3: Zahlwoerter herleiten, nicht schreiben.
#
# Ebenso wird der Zustand der Schleuse GEMESSEN, nicht behauptet: sie ist
# offen, sobald abnahmen/*designvertrag*.md existiert (prototyp-schleuse.sh).
# Bis zum 2026-08-21 stand hier fest "NICHT erledigt" — seit dem Designvertrag
# vom 2026-08-21 waere das eine Luege gewesen.
n=$(ls docs/*.md 2>/dev/null | wc -l | tr -d ' ')
[ "$n" -gt 0 ] && quellen="Die $n Projektquellen in docs/ sind vollstaendig." \
               || quellen="Die Projektquellen in docs/ sind vollstaendig."
vertrag=$(ls abnahmen/*designvertrag*.md 2>/dev/null | head -1)
if [ -n "$vertrag" ]; then
  phase0="Phase 0 (Designvertrag) ist erledigt:
   $vertrag — die Schleuse vor prototyp/ ist OFFEN."
else
  phase0="Phase 0 (Designvertrag) ist NICHT erledigt —
   die Schleuse vor prototyp/ ist zu."
fi
staende=$(ls assets/figma/*.png 2>/dev/null | sed 's#.*/##' | tr '\n' ' ')
[ -n "$staende" ] || staende="(noch keiner abgelegt)"

cat <<EOF
=== NAKAMA-DESIGN Primer ===
1. FAHRPLAN: docs/arbeitsplan.md sagt, wo wir stehen und was als
   Naechstes kommt. $phase0
   Die Arbeit heisst UEBERSETZEN (Phase 1b): Gen -> Probeeq -> Suna.
2. DIE VORGABE: die Figma-Staende des Users in assets/figma/ sind das
   Design — Claude erfindet keine Richtung, keine Metapher, keine Farbwelt
   (Designvertrag 2026-08-21: "Figma ist Quelle; Repo setzt um").
   Vorhanden: $staende
   Je Stand: assets/figma/LIES-MICH.md sagt, was darauf zu sehen ist.
   ASSETTRUHE ZUERST: assets/ ist vor jeder Gestaltungsentscheidung die
   erste Quelle. Verwenden statt erfinden; Fehlendes beim User erfragen.
   assets/verworfen/ ist Verlauf (Stilprofil der verworfenen Welt) — nicht
   verwenden.
3. EIN SCHRITT PRO RUNDE: eine Frage oeffnen, am lebenden Blatt zeigen,
   User-Urteil einholen, in abnahmen/ festhalten. Dann erst weiter.
4. RICHTUNG = USER, in Figma. 2-3 Varianten NEBENEINANDER nur noch dort,
   wo Figma eine Frage offenlaesst (Zustand, Grenzfall). Abnahmen sind
   bindend; ein neuer Figma-Stand ist neues User-Wort. Unklare
   Vorstellung? /interview <phase> — fragen ist billig, Fehlbau nicht.
   Glas-und-Licht-Welt und Lichtleiter-Fassung sind VERWORFEN (User
   2026-08-21), nicht mehr vorlegen.
5. METHODE: docs/CLAUDE-UX-UI-ARBEITSKERN.md bei jeder Designaufgabe
   befolgen (UX vor UI, Wireframe vor Politur, Schlusscheck).
6. SICHTBARKEIT: docs/DESIGN-GESETZE.md VOR jeder Anordnungs- und
   Sichtbarkeitsentscheidung lesen. Beim Uebersetzen ist die Aufgabenliste
   nach Haeufigkeit die Pruefliste: jedes Element sagt, welchen Handgriff
   es bedient.
7. KEIN KONTEXT-IMPORT: nicht in fremden Ordnern/Repos/Altdokumenten
   recherchieren. Erlaubte Quelle von aussen: assets/figma/.
   $quellen
8. BEWEIS: jedes gezeigte Ergebnis mit Playwright-Screenshot belegen.
   werkzeug/PRUEFLISTE.md sagt, was vorher gemessen wird.
9. SPRACHE: das Produkt ist englisch ("Englisch — mein Wort", 2026-08-21);
   Docs, Abnahmen, Commits, Gespraech deutsch.
EOF
