# Nakama Briefing

Private Seite: <https://nakama-briefing.philipld.chatgpt.site>
(User 22.08.2026: „das ist der neue und einzige hub , alle anderen artefakte
sind hiermit nichtmehr zu beachten").

Sie ist das gemeinsame, knappe Briefing für Phil, Claude und Codex. Das alte
Claude-Artefakt ist stillgelegt (zeigt nur noch die neue Adresse) und wird
nicht mehr gebaut, gelesen oder veröffentlicht.

## Der Ablauf je Session

1. `py -3.13 tools/hub/hub_sync.py holen` — liest `/api/hub`: Antworten von
   Phil landen in `docs/hub/hub.json` unter `antworten` (Status `neu`), neue
   Punkte von der Seite unter `eingang` (`typ: punkt`, idempotent je ID).
2. Einarbeiten: jede Antwort ist User-Wort (Datum + Wortlaut ins Register
   bzw. in `design/abnahmen/`, dann Status `eingearbeitet` + `ergebnis`);
   Punkte aus dem Eingang in Karten oder Plan überführen.
3. Bei einem echten Statuswechsel `hub.json` fortschreiben und
   `py -3.13 tools/hub/hub_sync.py senden` — prüft die Regeln, POSTet den
   Vollstand als `Claude` an `/api/state`, liest per GET gegen und verweigert,
   wenn die Seite Antworten trägt, die `hub.json` noch nicht kennt. Danach
   `hub.json` per Pathspec committen.

`hub_sync.py pruefen` prüft nur. Die Adresse steht in `hub.json` (`hub_url`);
`NAKAMA_HUB_URL` überschreibt sie (z. B. für `vinext dev` der Seite).

## Was wo liegt

| Inhalt | Ort |
|---|---|
| Sichtbare Briefing-Seite | `/` |
| Vollständiger Stand für Claude und Codex | `GET /api/hub` → `hub`, `answers`, `items`, `stateUpdatedBy` |
| Verbindliche Quelldatei für den Projektstand | `docs/hub/hub.json` |
| Stand auf die Seite schreiben | `POST /api/state` `{ "author": "Claude", "hub": <hub.json> }` (≤ 500 KB, kein Login nötig — gemessen 22.08.) |
| Antworten von Phil | `answers` in `/api/hub` (`questionId` = Karten-/Punkt-ID wie `U2.3`, bei Seiten-Punkten `item.<id>`) |
| Neu angelegte Entscheidungen, Updates und Blocker | `items` in `/api/hub`; anlegen über die Seite oder `POST /api/items` |
| Bilder der Karten (Quelle) | `docs/hub/bilder/` — die Seite hält Kopien unter `briefing-hub/public/images/` |
| Fragenkatalog der Seite (Alltagssprache, je Punkt Vorschlag/Alternative) | `briefing-hub/data/friendly-copy.ts` |
| Quellcode der Seite (OpenAI Sites: vinext + Cloudflare D1) | `briefing-hub/` |
| Start- und Stop-Erinnerung | `tools/hooks/hub-primer.sh`, `tools/hooks/hub-stop.sh` (Probe: `tools/hub/test_stop_hook.sh`) |

## Grenzen der Seite (vor dem Nachziehen wissen)

- Die Seite rendert **ihren** Fragenkatalog (`friendly-copy.ts`), nicht die
  `punkte`-Texte aus `hub.json`. Ein neuer Unterpunkt oder eine neue Karte mit
  Fragen braucht dort einen Eintrag und einen neuen Deploy — `hub.json` allein
  zeigt ihn nicht. Heute fehlt dort `U2.16` (17 von 18 Gen-Lesarten).
- Neue Bilder ebenso: nach `docs/hub/bilder/` UND `briefing-hub/public/images/`.
- `hub.json` muss `stand`, `bei_dir`, `plan`, `design` tragen, sonst lehnt
  `/api/state` ab; `hub_sync.py pruefen` prüft strenger.

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
