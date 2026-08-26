# Arbeitsregeln für das Wissensarchiv

Diese Regeln gelten für alle Dateien unter `wissen/`.

## Vor jeder Änderung

1. `wissen/LIES-MICH.md` und `wissen/INDEX.md` lesen.
2. Die im Index benannte kanonische Projektquelle öffnen, bevor eine Aussage
   über Nakamas Ist-Zustand gemacht wird.
3. Bestehende Einträge nach demselben Sachverhalt und derselben Quelle
   durchsuchen. Nicht duplizieren.

## Denkpflicht: Warum vor Was

Die primären Leser sind nachfolgende KI-Agenten. Optimiere Einträge deshalb
für begründete Entscheidungen, nicht für eine möglichst vollständige
Quellenzusammenfassung.

Jeder neue Befund wird, soweit die Quelle es erlaubt, als Kausalkette erfasst:

1. Welcher Mensch befindet sich in welcher Situation?
2. Welches Ziel, Problem, Risiko oder welche Unsicherheit hat er?
3. Welcher Wahrnehmungs-, Verständnis- oder Bedienbedarf entsteht daraus?
4. Welche Entwurfs- oder Technikfolge beantwortet genau diesen Bedarf?
5. Woran wäre für Menschen beobachtbar, dass die Lösung funktioniert?

Bei Produkt, UX, UI und Motion sind alle fünf Antworten Pflicht. Bei
technischen Themen muss die Kette mindestens bis zur erlebbaren Wirkung
reichen, zum Beispiel Vertrauen, Reaktionsfähigkeit, Audiokontinuität oder
sichere Wiederherstellung. Wenn die Quelle das menschliche Warum nicht belegt,
wird es als Nakama-Hypothese markiert und nicht als Forschungsergebnis
ausgegeben.

- Keine Feature- oder Best-Practice-Liste ohne Ursache und Wirkung übernehmen.
- Ein visuelles Beispiel zeigt eine mögliche Antwort, beweist aber nicht den
  Bedarf und ist keine Stilvorgabe.
- Nicht nur den Erfolgsweg erfassen: Unsicherheit, Fehler, Unterbrechung und
  Recovery gehören zum menschlichen Nutzungskontext.
- Eine Entwurfsfolge muss benennen, was sichtbar, bedienbar oder hörbar werden
  soll und warum.

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
- ein menschliches Warum oder bei rein technischer Forschung die daraus
  entstehende erlebbare Wirkung;
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
