# Arbeitsregeln für das Wissensarchiv

Diese Regeln gelten für alle Dateien unter `wissen/`.

## Vor jeder Änderung

1. `wissen/LIES-MICH.md` und `wissen/INDEX.md` lesen.
2. Die im Index benannte kanonische Projektquelle öffnen, bevor eine Aussage
   über Nakamas Ist-Zustand gemacht wird.
3. Bestehende Einträge nach demselben Sachverhalt und derselben Quelle
   durchsuchen. Nicht duplizieren.

## Eingehende Extrakte

Ein vom User übergebener NotebookLM-Text oder ein direkt geprüftes
YouTube-Video wird in das passende Thema eingearbeitet. Es entsteht keine
zusätzliche Kopie des Extrakts oder Transkripts. Der Inhalt wird paraphrasiert,
strukturiert und mit der verfügbaren Herkunft versehen. Fehlende Metadaten
werden als unbekannt markiert, nicht erfunden.

Ein neuer Wissenseintrag braucht mindestens:

- Titel;
- Autor oder `unbekannt`;
- Jahr oder `unbekannt`;
- DOI oder Fund-URL;
- Eingangsart, zum Beispiel `notebooklm-auszug`, `youtube-transkript` oder
  `web-recherche`;
- mindestens ein Thema;
- einen ehrlichen Prüfstand.

## Ablage

- Der Eintrag lebt genau einmal in seinem wichtigsten Themenordner.
- Weitere betroffene Themen werden im Frontmatter und über Links genannt.
- Gibt es bereits einen Eintrag zum Sachverhalt, wird er erweitert statt ein
  zweiter angelegt.
- Dateinamen folgen `JJJJ-MM-TT-kurzer-name.md`.
- Jeder neue oder geänderte Eintrag wird in `wissen/INDEX.md` verlinkt.

## Wahrheitsgrenzen

- Code und Tests schlagen Forschungsnotizen bei Aussagen über den Ist-Zustand.
- Figma und freigegebene Goldens besitzen sichtbare Pixel.
- `UX-CONTRACT.md` besitzt das freigegebene beobachtbare UI-Verhalten.
- Forschungswissen darf einen bestehenden User-Entscheid nicht still
  überschreiben.
- Eine neue verbindliche Produktentscheidung braucht die ausdrückliche
  Entscheidung des Users und wird in das bereits zuständige Register
  übernommen, nicht parallel unter `wissen/` gepflegt.

## Prüfstand

Zulässige Werte:

- `nur-extrakt`: Nur der übergebene Extrakt war verfügbar.
- `metadaten-geprueft`: Titel, Autor, Jahr und DOI oder URL wurden bestätigt.
- `quellen-geprueft`: Die relevante Aussage wurde an der Primärquelle geprüft.
- `mehrfach-belegt`: Mehrere unabhängige Quellen tragen die Erkenntnis.

Nie einen höheren Prüfstand eintragen, als die tatsächlich gelesene Grundlage
erlaubt.

## Visuelles Wissen

Ein Bild ohne Kontext ist kein Wissenseintrag. Jede verwendete Abbildung nennt
Quelle, Seite oder Zeitstempel und trennt Beobachtung von Interpretation. Bei
einem fremden Bild wird nur die vom User bereitgestellte beziehungsweise
zulässige Datei eingecheckt; andernfalls bleibt ein genauer Verweis und eine
eigene Beschreibung.

## Abschluss

Vor dem Commit relative Links prüfen, den Diff auf doppelte oder überhöhte
Aussagen lesen und sicherstellen, dass keine Roh-PDF im Änderungssatz liegt.
