# Reviews — Eingang aus dem Nakama-Hub

Hier landen Code-Reviews (Codex, andere Modelle), die der User über die
Upload-Fläche des Hubs (`docs/hub/`) abgelegt hat — geholt mit
`py -3.13 tools/hub/hub_eingang.py <gelesene-hub-seite.html>`, Dateiname
`JJJJ-MM-TT-<name>.md`, Inhalt unverändert.

Der Status jeder Review steht in `docs/hub/hub.json` (`reviews`), nicht hier.
Regel (Bauaufteilung §2, T3): **jeder Befund wird gegen die Quelldatei
verifiziert, bevor gehandelt wird** — bestätigt ⇒ gefixt oder NAK-Zeile in
`docs/offene-punkte.md` · widerlegt ⇒ eine Zeile, warum · nichts verschwindet
still. Rund ein Viertel der bisherigen AI-Auditbefunde war falsch.
