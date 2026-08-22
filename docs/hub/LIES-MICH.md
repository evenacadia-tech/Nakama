# Nakama Briefing

Private Seite: <https://nakama-briefing.philipld.chatgpt.site>

Sie ist das gemeinsame, knappe Briefing für Phil, Claude und Codex. Das alte
Claude-Artefakt ist außer Betrieb. Es wird nicht mehr gebaut, gelesen oder
veröffentlicht.

## Der einfache Ablauf

1. Zu Beginn die Seite oder `/api/hub` lesen.
2. Neue Antworten und neue Punkte zuerst aufgreifen.
3. Bei einem echten Statuswechsel `docs/hub/hub.json` aktualisieren und den
   vollständigen Stand über `/api/state` auf die Seite schreiben.

Mehr Ablauf braucht der Hub nicht.

## Was wo liegt

| Inhalt | Ort |
|---|---|
| Sichtbare Briefing-Seite | `/` |
| Vollständiger Stand für Claude und Codex | `/api/hub` |
| Verbindliche Quelldatei für den Projektstand | `docs/hub/hub.json` |
| Antworten von Phil | `answers` in `/api/hub` |
| Neu angelegte Entscheidungen, Updates und Blocker | `items` in `/api/hub` |
| Quellcode der neuen Seite | `briefing-hub/` |
| Start- und Stop-Erinnerung | `tools/hooks/hub-primer.sh`, `tools/hooks/hub-stop.sh` |

## Schreiben auf die Seite

- Phil antwortet direkt auf der Seite.
- Phil, Claude und Codex legen über „Neuen Punkt anlegen" kurze Entscheidungen,
  Updates oder Blocker an.
- Claude und Codex lesen den Gesamtstand über `GET /api/hub`.
- Ein aktualisiertes `docs/hub/hub.json` wird als vollständiges Dokument mit
  `{ "hub": <Dokument>, "author": "Claude" }` beziehungsweise `Codex` an
  `POST /api/state` gesendet. Dafür den angemeldeten Browser auf derselben
  Seite verwenden.

## Inhaltsregeln

- Klartext für den Projektleiter. Kürzel höchstens als Zusatz.
- Ein Punkt besteht aus Titel, kurzer Erklärung und, wenn nötig, einem Satz
  „warum jetzt".
- Sichtbare Fragen bekommen das Bild direkt dazu.
- „Erledigt" nur mit Beleg und Datum.
- User-Antworten mit Datum und Wortlaut ins Register oder in die passende
  Design-Abnahme übernehmen.
- Keine Frage im Chat wiederholen, die auf der Seite beantwortbar ist.
- Keine Unterseiten, Freigabeketten oder Pflegefelder hinzufügen, solange ein
  kurzer Punkt ausreicht.

Die Dateien unter `tools/hub/`, die noch `hub.html` oder das alte Artefakt
erzeugen, bleiben nur als Verlauf im Repository. Sie gehören nicht mehr zum
laufenden Briefing-Ablauf.
