# Fable-5-Umformulierungsregeln (destilliert)

Quelle: `https://platform.claude.com/docs/en/build-with-claude/prompt-engineering/prompting-claude-fable-5`
(Stand der Destillation: 2026-08-17 — bei Zweifel die Quelle neu lesen.)

Auftrag des Umformulierers: aus einer rohen Anweisung einen Auftrag machen,
den Fable 5 mit minimaler Fehlerquote in einem Zug richtig ausführt.
**Kürze ist Teil der Korrektheit** — die Doku warnt ausdrücklich: zu
präskriptive Prompts degradieren Fable-5-Output; ein kurzer Steuersatz
wirkt so gut wie eine Verhaltensliste.

## Die Regeln

1. **Ergebnis zuerst, dann das Warum.** Erster Satz = was am Ende stehen
   soll. Danach der Intent nach dem Doku-Muster: „Ich arbeite an [größere
   Aufgabe] für [wen]. Gebraucht wird [was das Ergebnis ermöglicht]. Vor
   diesem Hintergrund: [Auftrag]." Das Warum aus dem Projektkontext
   ableiten (CLAUDE.md, design-stand, offene-punkte); wo es geraten ist,
   als `[Annahme: …]` markieren.

2. **Grenzen aussprechen.** Was NICHT gemacht werden soll, explizit: kein
   Neben-Refactoring, keine „Modernisierung" von Legacy-Namen/Front, keine
   Features über den Auftrag hinaus. Ist die Rohanweisung eine
   Problembeschreibung (kein Änderungsauftrag), den Auftrag als
   Befund-Lieferung formulieren („analysiere und berichte — noch nicht
   fixen").

3. **Task-spezifische Fakten statt Boilerplate.** Konkrete Pfade, betroffene
   Dateien, anzuknüpfende IDs aus offene-punkte, das betroffene
   Gegenpfad-Paar (starten↔stoppen · speichern↔laden · …) — aber KEINE
   Wiederholung von Grundgesetz, Beweis-Kanon oder Verwürfen im Wortlaut:
   die injiziert der Session-Harness bereits. Nur benennen, WELCHE Beweise
   für diesen Auftrag gelten (z. B. „GoldenTest + NullTest müssen grün
   bleiben; Editor-Sicht per EqCopShot belegen").

4. **Erfolgskriterium benennen.** Woran erkennt man, dass es fertig ist —
   messbar oder sichtbar, mit Beleg aus der Session („fertig heißt: X läuft,
   Beweis Y genannt"). Nicht Verifiziertes soll als offen berichtet werden.

5. **Handeln statt überplanen.** Wenn der Auftrag klar ist, einen Satz wie:
   „Sobald genug Information zum Handeln da ist: handeln — Feststehendes
   nicht erneut herleiten." Keine Aufforderung, erst einen Plan zur Freigabe
   vorzulegen — außer die Rohanweisung verlangt es.

6. **Pausen definieren, knapp.** Nur anhalten bei Irreversiblem, echter
   Scope-Entscheidung oder Input, den nur der User geben kann (UAC-Klick,
   Design-Freeze, Optik-Abnahme). Nicht jede Pausenart aufzählen — der
   eine Satz genügt.

7. **Bei langen/autonomen Läufen:** Selbstprüfung einbauen — „Etabliere
   eine Prüfmethode und verifiziere in Intervallen mit frischen
   Verifikations-Subagenten gegen die Spezifikation" (frischer Kontext
   schlägt Selbstkritik). Bei kurzen Aufträgen weglassen.

8. **Verbote:** Keine Anweisungen, das eigene Reasoning im Antworttext
   wiederzugeben (löst bei Fable 5 die `reasoning_extraction`-Sperre aus).
   Keine Kontext-Budget-Hinweise. Keine ALL-CAPS-Dringlichkeit, keine
   Drohungen, keine Rollen-Theatralik.

9. **Ambiguitäten nicht wegraten.** Wo zwei Lesarten zu verschiedenen
   Ergebnissen führen, NICHT stillschweigend eine wählen: entweder die
   plausiblere wählen und als `[Annahme: …]` markieren (harmlos) oder als
   OFFENE-FRAGE ausweisen (ergebnisrelevant). Maximal 3 Fragen, nur echte.

10. **Sprache und Form.** Deutsch, kurze Absätze oder wenige Spiegelstriche,
    keine nummerierten 12-Schritte-Anleitungen für Dinge, die das Modell
    selbst sequenzieren kann. Der fertige Auftrag ist typischerweise
    5–15 Zeilen — deutlich mehr ist ein Warnsignal für Boilerplate.

## Miniatur-Beispiel

Roh: „mach den nebel schöner"

Umgeformt:
> Verfeinere die Befund-Nebel-Darstellung im Claude-Musterblatt
> (`Nakama-Design/werkzeug/…` — Beispiel; das alte Spectral-Field-Blatt liegt im Archiv). Ich arbeite auf
> den Design-Freeze des Tiefenfelds zu; der Nebel muss WO + Schwere eines
> Befunds tragen (Dicke = Schwere) und darf die Nebel→Licht-Evidenzachse
> nicht brechen. [Annahme: „schöner" = weicheres Wogen + weniger harte
> Kanten, nicht mehr Nebelfläche.] Nur das Nebel-Rendering anfassen —
> Chips, Kali-Panel und die anderen Sichtwerkzeuge bleiben. Fertig heißt:
> `node --check` grün, `render-blatt.mjs`-Renders erneuert, Vergleich
> vorher/nachher benannt; Optik-Abnahme bleibt beim User am lebenden Blatt.
