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
| Uploads aus der gelesenen Seite ins Repo holen | `py -3.13 tools/hub/hub_eingang.py <gelesene.html>` → `docs/reviews/` |
| Artefakt-Adresse | `artefakt_url` in `hub.json` (eine Adresse, bleibt beim Redeploy) |
| Hooks | `tools/hooks/hub-primer.sh` (SessionStart: vorlesen + Drift) · `tools/hooks/hub-stop.sh` (Stop: Commits ohne Hub-Update ⇒ einmal blocken) |
| Proben | `tools/hub/test_stop_hook.sh` (5 Fälle) · `tools/hub/test/upload_probe.js` (Upload mit gestubbtem `window.claude`, via Playwright-MCP `browser_run_code_unsafe`) |

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
- **Bilder** kommen beim Build aus der Design-Truhe
  (`Nakama-Design/assets/figma/`, neuester Stand je App, auf Inhalt
  zugeschnitten, halbiert auf 2×) — das Technik-Repo hält keine Kopie.
  Auf dem Zweitrechner muss das Design-Repo neben diesem liegen
  (oder `NAKAMA_DESIGN` zeigt darauf).

## Wie der Upload funktioniert (und warum so)

Die Artefakt-Fähigkeit „Dateien ablegen" gibt es für diesen Account nicht;
verfügbar ist `artifact` (die Seite darf sich selbst als neue Version
veröffentlichen). Deshalb: Datei wählen → die Seite liest sie → hängt sie an
`eingang` in ihrem eingebetteten Zustand → veröffentlicht sich komplett neu
(`artifact.publish(html)`, aus den autorisierten Bausteinen, nie aus dem
Live-DOM). Claude liest die Seite (`Artifact read`), holt den Eingang ins Repo
und veröffentlicht mit geleertem Eingang. Ein Upload zwischen Lesen und
Veröffentlichen meldet sich als Konflikt — dann neu lesen, nie `force`.
