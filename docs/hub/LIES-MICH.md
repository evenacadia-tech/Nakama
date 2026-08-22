# Nakama-Hub — das gemeinsame Briefing

**Für wen:** den User (Projektleiter, kein Coder) und jede Claude-Session.
**Was:** eine Seite auf claude.ai, die sagt, was im Plan erledigt und offen
ist, was beim User liegt (Entscheide, Handgriffe, Wissen vor dem Klick), der
aktuelle Figma-Stand der drei Apps mit je einem Bild, und eine Upload-Fläche
für Codex-Reviews (`.md`). Auftrag des Users vom 22.08.2026: „ein gemeinsames
briefing und übersichtshub … das soll die arbeit verbessern nicht ein eigenes
projekt werden".

| Was | Wo |
|---|---|
| **Quelle** (von Claude gepflegt, Klartext) | `docs/hub/hub.json` |
| Vorlage der Seite (CSS + JS, rendert aus dem JSON) | `tools/hub/seite.html` |
| Build (prüft JSON, bettet Bilder ein, schreibt `hub.html`) | `py -3.13 tools/hub/baue_hub.py` (`--pruefen` = nur prüfen) |
| Gebaute Seite (gitignoriert, regenerierbar) | `docs/hub/hub.html` |
| Uploads aus der gelesenen Seite ins Repo holen | `py -3.13 tools/hub/hub_eingang.py <gelesene.html>` → Reviews nach `docs/reviews/`, Bilder bytegleich nach `docs/hub/eingang/` (+ `uploads` in `hub.json` mit Notiz) |
| Zeigebilder der Karten (Ausschnitte, Screenshots; committet) | `docs/hub/bilder/`, je Karte `bilder: [{datei, text}]` |
| Antworten des Users (Wahl + Text je Karte/Unterpunkt) | `antworten` in `hub.json` — `{id: {wahl, text, datum, status: neu·gelesen·eingearbeitet, ergebnis}}`; Unterpunkte je Karte in `punkte: [{id, titel, gewicht, entwurf, alternative, wahlen?}]` |
| Artefakt-Adresse | `artefakt_url` in `hub.json` (eine Adresse, bleibt beim Redeploy) |
| Hooks | `tools/hooks/hub-primer.sh` (SessionStart: vorlesen + Drift) · `tools/hooks/hub-stop.sh` (Stop: Commits ohne Hub-Update ⇒ einmal blocken) |
| Proben | `tools/hub/test/antwort_probe.js` (drei Antworten, Entwurf überlebt Neuladen, Speichern → Volldokument) · `tools/hub/test_stop_hook.sh` (5 Fälle) · `tools/hub/test/upload_probe.js` + `upload_probe_2.js` (Review + Bild + Notiz mit gestubbtem `window.claude`, via Playwright-MCP `browser_run_code_unsafe`; dazwischen `hub_eingang.py docs/hub/hub-published.html`) |

## Pflicht jeder Session (steht auch in CLAUDE.md, Abschnitt „Hub")

1. **Lesen.** Der SessionStart-Hook liest Kopfzahlen und Drift vor. Bei Drift
   (Commits seit dem letzten Hub-Commit) vor der Arbeit `hub.json` nachziehen.
2. **Eingang prüfen.** `Artifact(action='read', url=<artefakt_url>)` → Datei →
   `hub_eingang.py <Datei>`. Jede geholte Review: Befund für Befund gegen die
   Quelldatei prüfen (T3-Regel der Bauaufteilung §2), Status in `hub.json`
   fortschreiben. Nichts verschwindet still.
3. **Nachziehen.** Am Ende der Session (und bei jedem Statuswechsel: Session
   erledigt, neue Entscheidung beim User, neuer Figma-Stand): `hub.json`
   fortschreiben → `baue_hub.py` → `Artifact(file_path=docs/hub/hub.html,
   url=<artefakt_url>, favicon wie bisher, capabilities={'artifact': {}})` →
   `hub.json` per Pathspec committen. Der Stop-Hook erinnert einmal, wenn es
   fehlt.

## Regeln für den Inhalt

- **Klartext für den Projektleiter**, keine Kindersprache, keine Code-Namen
  ohne Erklärung (Ticket-Kürzel dürfen als Zusatz stehen).
- **Erledigt nur mit Beleg** (Manifest in `docs/beweise/`) — der Build
  verweigert eine „erledigt"-Zeile ohne Beleg und Datum.
- **Ein Entscheid des Users steht erst hier, wenn er mit Datum und Wortlaut
  im Register (CLAUDE.md) ist.** Eigene Vorschläge heißen so.
- **Genau eine Plan-Zeile ist „naechster"** (der Build prüft es).
- **„Bei dir"** trägt je Punkt: was, warum, wo (Belegstelle), seit; vier
  Dringlichkeiten: `jetzt` · `wenn du dazu kommst` · `wissen` · `später`.
- **Zeigen, nicht beschreiben** (User 22.08.): geht es um etwas Sichtbares,
  liegt das Bild in der Karte (`bilder`). Ausschnitte aus der Truhe schneidet
  man mit `auf_inhalt_zuschneiden` aus `baue_hub.py` + PIL; Dateiname
  `u<N>-<was>.png`. Hohe Bilder (Höhe > 2× Breite) zeigt die Seite aufklappbar.
- **Antworten** (`antworten`): `hub_eingang.py` setzt neue/geänderte auf
  `neu`. Einarbeiten heißt: Wortlaut mit Datum ins Register (CLAUDE.md) oder
  in `Nakama-Design/abnahmen/`, dann `status: eingearbeitet` + `ergebnis`
  (ein Satz: was daraus wurde). Ist eine Karte erledigt und fliegt raus, ihre
  Antworten nach `docs/hub/antworten-archiv.md` übertragen (der Build
  verweigert verwaiste IDs). Antworten nie im Chat erfragen, die auf der
  Seite beantwortbar sind.
- **Uploads des Users** (`uploads`): Status fortschreiben (`eingegangen – noch
  nicht bearbeitet` → z. B. `in die Truhe übernommen`) und in `ergebnis` sagen,
  was passiert ist. Die Datei in `docs/hub/eingang/` bleibt liegen — sie ist
  der bytegleiche Beleg (SHA-256 für die Truhe daraus bilden).
- **Bilder** kommen beim Build aus der Design-Truhe
  (`Nakama-Design/assets/figma/`, neuester Stand je App, auf Inhalt
  zugeschnitten, halbiert auf 2×) — das Technik-Repo hält keine Kopie.
  Auf dem Zweitrechner muss das Design-Repo neben diesem liegen
  (oder `NAKAMA_DESIGN` zeigt darauf).

## Wie der Upload funktioniert (und warum so)

Die Seite nimmt `.md`/`.txt` (als Text) und PNG/JPG/WebP/GIF (als
Data-URL, Originalbytes, bis 8 MB je Datei) plus eine Notiz entgegen.

Die Artefakt-Fähigkeit „Dateien ablegen" gibt es für diesen Account nicht;
verfügbar ist `artifact` (die Seite darf sich selbst als neue Version
veröffentlichen). Deshalb: Datei wählen → die Seite liest sie → hängt sie an
`eingang` in ihrem eingebetteten Zustand → veröffentlicht sich komplett neu
(`artifact.publish(html)`, aus den autorisierten Bausteinen, nie aus dem
Live-DOM). Claude liest die Seite (`Artifact read`), holt den Eingang ins Repo
und veröffentlicht mit geleertem Eingang. Ein Upload zwischen Lesen und
Veröffentlichen meldet sich als Konflikt — dann neu lesen, nie `force`.
