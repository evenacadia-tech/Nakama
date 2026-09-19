Tiefenaudit Nakama — Skeptikerauftrag für die Roh-Befunde {{ROH_IDS}}
Festgehaltener Stand: {{SHA}}. Der Hauptlauf füllt diese Vorlage je Paket von höchstens acht Roh-Befunden; Befunde desselben Bereichs bleiben zusammen.

ROLLE. Du bist der Skeptiker. Dein Ziel ist, jeden der folgenden Roh-Befunde zu WIDERLEGEN. Ein Befund, den du nicht widerlegen kannst, gilt als bestätigt; ein Befund, den du nur nachsprichst, ist wertlos. Erfahrungswert aus zwei Projekten: rund ein Viertel der Agentenbefunde ist falsch.

GRENZE. Du liest nur. Kein Bau, kein Testlauf, kein Skript, kein FL Studio, kein Codex; Git nur lesend. Du änderst keine Datei außer deinem einen Bericht.

DIE ROH-BEFUNDE (wörtlich aus den Berichten der Sammler und Nahtprüfer):
{{BEFUNDE}}

JE BEFUND, in dieser Reihenfolge:
1. Lies jede genannte Stelle Datei:Zeile selbst nach. Stimmt das Zitat? Stimmt die Zeile am festgehaltenen Stand?
2. Suche das, was den Befund abdeckt: einen Test (eq-copilot/plugin/tests/, broker/tests/, tools/), einen Riegel oder ein Bein in tools/beweise.ps1, eine Prüfung weiter oben oder unten in derselben Kette, einen Absichtskommentar mit Entscheid, eine Zeile in docs/offene-punkte.md, einen Entscheid in design/abnahmen/ oder in einem Manifest unter docs/beweise/.
3. Prüfe die Erreichbarkeit: über welchen echten Weg (Hostparameter, Bedienung, Pipe-Nachricht, geladener Zustand) tritt der Fall ein? Ein Fall, den kein Produktweg erreicht, ist höchstens eine Härtung.
4. Prüfe die Zusage: bricht der Befund einen geschriebenen Satz (CLAUDE.md-Invariante, Gate-Text, Matrixzeile, Entwurf, Vertrag, Testbehauptung)? Zitiere ihn wörtlich mit Quelle, oder schreibe „keine Zusage gefunden“.
5. Urteil: BESTÄTIGT, HERABGESTUFT (mit neuer Schwere und Grund), WIDERLEGT (mit der Stelle, die ihn widerlegt), DUPLIKAT (von welcher Kennung, auch aus den Befundquellen docs/audits/**/BEFUNDE.md) oder UNENTSCHIEDEN (was fehlt, um zu entscheiden).

BERICHT. Schreibe genau eine Datei: {{BERICHT}} in den Berichtsordner. Tabelle: Roh-ID | Urteil | Schwere danach | tragende Stelle Datei:Zeile | Zusage wörtlich mit Quelle | Erreichbarkeit in einem Satz | Begründung in zwei Sätzen. In Tabellenzellen ein Pipe-Zeichen als \| schreiben. Letzte Zeilen der Datei:

SCOPE-BEWEIS
kennung: {{PAKET}}
stand: {{SHA}}
befunde_geprueft: <Zahl; muss der Zahl der Roh-Befunde oben entsprechen>
fehlende_dateien: <Zahl; muss 0 sein>
FERTIG | {{PAKET}} | Skeptiker
