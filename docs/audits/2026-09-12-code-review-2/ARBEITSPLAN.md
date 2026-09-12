# Zweiter unabhängiger Nakama-Audit

**Abschlussnachtrag 12.09.2026:** Der folgende Plan dokumentiert den ursprünglichen Arbeitsumfang. Nach automatischen Blockaden und der letzten User-Einschränkung wurden Agentenexperimente, Sicherheits-/Fehler-/Grenzfallprüfungen und der laufende vollständige Kanon beendet. Der Abschluss beschränkt sich auf lesende Auswertung vorhandener Belege und eine separate Übergabedatei. Keine Wiederaufnahme der gestoppten Aktionen aus dem ursprünglichen Plan ableiten. Ergebnis: Nacharbeit erforderlich, kein Gesamt-PASS; UEBERGABE-AN-CLAUDE.md unter docs/audits/2026-09-12-code-review-2/.

Auftrag 12.09.2026: Den nachgearbeiteten Codeaudit vom 10.09. mit maximaler Tiefe und unabhängigen Agenten erneut durchführen. Anschließend ausdrücklich auf bisher zu kurz gekommene Bereiche ausweiten. Die weitere SONDE-015-Implementierung bleibt pausiert.

Ausgangsrevision: `ea6ddd57c6658429ccb283f5d207e26341606267`. Arbeitsstand nach der vorgeschriebenen reinen Planrechnung: `91bb5fa3` (kein Produktcode geändert). Startcheckout sauber; `git pull --ff-only` aktuell. NAK-246 meldet T2 PASS; das ersetzt keinen Beleg dieser Runde.

Arbeitsform: Codex-Mehragentenworkflow nach ultracode, kein nativer Claude Dynamic Workflow. Root verantwortet Quellenstand, einzige gemeinsame Builds, Gegenprüfung, Berichte und Lieferung. Drei gleichzeitig lesende Agenten; weitere unabhängige Prüfpakete in einer zweiten Welle. Keine Produktänderungen und keine Fortsetzung geplanter Features in diesem Audit. Repros binden unveränderte Produktionsquellen ein und leben in getrennten Scratchverzeichnissen oder als Auditbelege.

## Welle 1

- Audio/DSP: vollständige real erreichbare Audiokette, Bank-/Slotbesitz, Übergänge, Nulltest, RT, Zahlenränder; danach alte D1/D7/D10 gegenprüfen.
- C++ State/IPC: Serialisierung/Migration, ACK/Persistenz/Reload, Generationen, Callbackbesitz, Shutdown und Verbraucher; danach D2/D3/D4/D6/D8 gegenprüfen.
- Rust: Coordinator, Queue/Outbox, Persistenz, Idempotenz, Sessionwechsel und Wireordnung; danach D5/D9/O1/M3 gegenprüfen.
- Root: frischer kanonischer Build/Testlauf, tatsächliche Testanbindung, Gesundheits-/Prüfwerkzeuge, Coveragekarte, Integration und unabhängige Widerlegung der Kandidaten.

## Welle 2: Ausweitung

Aus der belegten Abdeckung ableiten, mindestens: Analysefachmodelle und fachliche Orakel; Hostanbindung/Plugin-Lebenszyklus; Installer/Repair/Rückweg; Vertrags-/Prüfwerkzeugempfindlichkeit. Andere Agenten prüfen übernommene Hochrisikobefunde gegen, ohne das gewünschte Urteil vorzugeben. Hardware-, FL- und Installationsbelege bleiben getrennt von Headless-Tests.

## Beweisregeln und Abschluss

Jeder Befund braucht Anforderungsquelle, erreichbare Auslösekette, Wirkung, genaue Quelldateien/Zeilen, Gegenargumente und Beweisstatus. Reproduziert, quellbelegt, Härtung, bereits registriert, vorgesehen und Fehlalarm trennen. Keine neue Releaseanforderung erfinden; kein grünes Gesamturteil bei offenem notwendigem Gegenbeweis. Testlauf nur auf Probe-Pipes. Keine Installation.

Frischen Kanon mit `pwsh -File tools/beweise.ps1 -Bauen -Titel 'Zweiter unabhaengiger Codeaudit 2026-09-12'` ausführen; der Runner wählt seinen vorgesehenen zeitgestempelten Manifestpfad. Gezielte unabhängige Repros und passende Tests ergänzen. Vor Abschluss Quellenstand/Status/Diff prüfen, Auditbericht mit Abdeckung, alten und neuen Befunden sowie Rohbelegen unter `docs/audits/2026-09-12-code-review-2/` liefern. Nur eigene Pfade committen und pushen.

Der vorgeschriebene Dokuriegel zum gerechneten Planblatt meldete zwei bereits geplante, fehlende künftige Manifeste NAK-226/NAK-227 und einen als historisch erkannten Pfad. Das wird nicht als neuer Produktdefekt ausgegeben.
