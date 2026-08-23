# Reviews — Eingang

Hier landen Code-Reviews (Codex, andere Modelle) als Datei
`JJJJ-MM-TT-<name>.md`, Inhalt unverändert. Seit 22.08.2026 kommen sie
direkt ins Repo (Codex arbeitet im selben Repo; der User legt eine Datei
hier ab) — die frühere Upload-Fläche des Claude-Artefakts ist stillgelegt
(User: „alle anderen artefakte sind hiermit nichtmehr zu beachten"). Die
Briefing-Seite, die bis zum 23.08.2026 ein zweiter Eingang war, ist
abgeschafft — Reviews kommen nur noch als Datei hierher.

**Der Status einer Review steht dort, wo sie gemessen wurde:** im
Beweismanifest des betroffenen Tickets (`docs/beweise/<Ticket>.md`, Abschnitt
T2/T3), und ihr Urteil als Marke im Kopf derselben Datei
(`<!-- NAKAMA-URTEIL: T2 PASS 2026-08-22 -->`). Daraus rechnet
`tools/plan/planstand.py` den Planstand — eine zweite Statusliste gibt es
bewusst nicht mehr.
Regel (Bauaufteilung §2, T3): **jeder Befund wird gegen die Quelldatei
verifiziert, bevor gehandelt wird** — bestätigt ⇒ gefixt oder NAK-Zeile in
`docs/offene-punkte.md` · widerlegt ⇒ eine Zeile, warum · nichts verschwindet
still. Rund ein Viertel der bisherigen AI-Auditbefunde war falsch.
