# Reviews — Eingang

Hier landen Code-Reviews (Codex, andere Modelle) als Datei
`JJJJ-MM-TT-<name>.md`, Inhalt unverändert. Seit 22.08.2026 kommen sie
direkt ins Repo (Codex arbeitet im selben Repo; der User legt eine Datei
hier ab) — die frühere Upload-Fläche des Claude-Artefakts ist stillgelegt
(User: „alle anderen artefakte sind hiermit nichtmehr zu beachten"). Eine
kurze Review passt auch als Punkt auf die Briefing-Seite
(`https://nakama-briefing.philipld.chatgpt.site`, „Neuen Punkt anlegen",
Art „Update"); `py -3.13 tools/hub/hub_sync.py holen` trägt sie in
`docs/hub/hub.json` (`eingang`) ein.

Der Status jeder Review steht in `docs/hub/hub.json` (`reviews`), nicht hier.
Regel (Bauaufteilung §2, T3): **jeder Befund wird gegen die Quelldatei
verifiziert, bevor gehandelt wird** — bestätigt ⇒ gefixt oder NAK-Zeile in
`docs/offene-punkte.md` · widerlegt ⇒ eine Zeile, warum · nichts verschwindet
still. Rund ein Viertel der bisherigen AI-Auditbefunde war falsch.
