# Nakama-Wissensarchiv

Dieses Verzeichnis ist der zentrale Einstieg für recherchiertes Wissen zu
Nakama. Das gesamte Git-Repository bleibt die einzige Projektwahrheit.
Bestehende Verträge, Designquellen und technische Dokumente werden verlinkt,
nicht kopiert.

## Grundsatz

Ein Sachverhalt lebt genau einmal im passenden Wissensbereich. Neue Extrakte
ergänzen einen vorhandenen Eintrag oder eröffnen dort einen neuen. Es gibt
keine parallele Quellenablage und keine zweite NotebookLM-Datenbank im Repo.

Der Arbeitsweg ist kurz:

```text
PDF → NotebookLM-Extrakt → Übergabe an Codex → richtiger Wissensbereich
    → Commit und Push → PDF löschen
```

## Eingang eines Papers

Für die Übergabe reichen:

- Titel, Autorinnen oder Autoren und Jahr;
- DOI oder Fund-URL;
- der NotebookLM-Extrakt;
- bei visueller Relevanz die Abbildungsnummer, Seitenzahl und gegebenenfalls
  ein ausgewählter Screenshot.

Codex übernimmt Einordnung, Zusammenführung mit vorhandenem Wissen,
Widerspruchsprüfung, visuelle Karten, Indexpflege, Commit und Push. Die Vorlage
liegt unter [`vorlagen/wissenseintrag.md`](vorlagen/wissenseintrag.md).
YouTube-Videos kann Codex direkt prüfen; auch dort werden nur relevante
Erkenntnisse und präzise Zeitstempel übernommen, keine Transkript-Sammlung.

Ein Forschungsbefund ist noch keine Produktentscheidung. Verbindliche
Entscheide bleiben in den bereits vorhandenen Projektregistern und werden nur
nach ausdrücklicher Entscheidung des Users geändert.

## Visuelle Belege

Visuelles Wissen ist in jedem Thema zulässig. Wiederverwendbare Bilder und
deren Einordnung liegen unter `medien/`; ein Themenblatt verweist auf sie,
statt Kopien anzulegen. Jede visuelle Karte trennt:

- was objektiv sichtbar ist;
- was die Quelle dazu behauptet;
- was wir daraus für Nakama ableiten.

## Einstieg

Die vollständige Karte steht in [`INDEX.md`](INDEX.md). Arbeitsregeln für
Agenten stehen in [`AGENTS.md`](AGENTS.md).
