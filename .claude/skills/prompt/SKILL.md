---
name: prompt
description: Formt eine rohe User-Anweisung in einen Fable-5-optimalen Auftrag um (Opus-Umformulierer nach der Anthropic-Prompting-Doku), zeigt das Ergebnis und führt es aus. Nutzen, wenn der User /prompt tippt — die Rohanweisung folgt nach dem Skill-Namen. Modus "nur <anweisung>": nur umformulieren + anzeigen, nicht ausführen (z. B. für eine frische Session oder ein anderes Tool). Senkt Fehler durch klaren Intent, Scope-Zaun und benannte Beweise — NICHT für triviale Ein-Satz-Aufträge, die schon eindeutig sind.
argument-hint: "[nur] <rohe Anweisung>"
---

# /prompt — Auftrag Fable-5-optimal umformulieren

Der User gibt eine rohe Anweisung; ein **Opus-Subagent** formt sie nach den
Regeln der Anthropic-Doku (destilliert in `references/fable5-regeln.md`,
im selben Ordner wie dieses SKILL.md) in einen präzisen Auftrag um. Danach
wird der umgeformte Auftrag angezeigt und ausgeführt.

## Ablauf

1. **Rohanweisung** = `$ARGUMENTS`. Leer ⇒ kurz nachfragen, was umgeformt
   werden soll. Beginnt sie mit `nur ` ⇒ Modus **nur-anzeigen** (Schritt 5
   entfällt), das `nur ` wird entfernt.

2. **Opus-Umformulierer dispatchen** (Agent-Tool, `model: "opus"`,
   `run_in_background: false`). Prompt an den Agenten:

   > Du bist ein Prompt-Umformulierer für das Nakama-Projekt. Lies zuerst
   > (1) `references/fable5-regeln.md` neben dem aufrufenden SKILL.md und
   > (2) das `CLAUDE.md` des Workspace; wenn die Anweisung Design betrifft
   > zusätzlich `docs/design-stand.md`, wenn sie Plugin/Engine/Broker
   > betrifft `docs/plugin-wissen.md`, wenn sie an Offenes anknüpft
   > `docs/offene-punkte.md`. Forme dann die folgende Rohanweisung nach
   > den Regeln um. Gib GENAU dieses Format zurück:
   >
   > ```
   > AUFTRAG:
   > <der umgeformte Prompt, Deutsch>
   > ---
   > OFFENE-FRAGEN:
   > <0–3 Zeilen; nur echte Ambiguitäten, deren Antwort das Ergebnis
   > ändert — oder "keine">
   > ```
   >
   > Rohanweisung: <hier $ARGUMENTS einsetzen>

3. **Anzeigen:** Den `AUFTRAG:`-Block wörtlich in einem Codeblock zeigen —
   der User soll sehen (und lernen), was aus seiner Anweisung wurde.

4. **Echte offene Fragen** (falls der Agent welche liefert) VOR der
   Ausführung per AskUserQuestion klären. Keine erfundenen Rückfragen —
   „keine" heißt keine.

5. **Ausführen:** Den umgeformten Auftrag als den eigentlichen Auftrag
   dieser Runde behandeln und vollständig abarbeiten (inkl. der darin
   benannten Beweise). Im Modus **nur-anzeigen** stattdessen hier enden.

## Leitplanken

- Der Umformulierer darf den Prompt NICHT aufblähen: CLAUDE.md + Primer
  injizieren Grundgesetz, Beweis-Kanon und Verwürfe bereits jede Session.
  Sein Mehrwert ist Klarheit, Intent (Warum), Scope-Zaun und
  task-spezifische Fakten — nicht Boilerplate. Details in den Regeln.
- Vom Umformulierer als Annahme markierte Stellen (`[Annahme: …]`) bleiben
  im angezeigten Auftrag sichtbar; sind sie riskant, gehören sie in die
  offenen Fragen.
- Der umgeformte Auftrag ersetzt die Rohanweisung, nicht das Urteil:
  widerspricht er erkennbar dem, was der User wollte, gewinnt der User —
  kurz benennen und die Rohanweisung ausführen.
