# Verifikations- und Provenienznotiz

Datum 12.09.2026. Dies ist die Nachweisübersicht zum Audit, keine Freigabe des Produkts. UEBERGABE-AN-CLAUDE.md enthält die vollständige Interpretation.

## Quellenstand

- Startcheckout sauber auf ea6ddd57c6658429ccb283f5d207e26341606267; anfänglicher Fast-Forward-Abgleich ohne neue Änderungen.
- Vorgeschriebene Planstandrechnung: py -3.13 tools/plan/planstand.py; 33/45 abgenommen. Eigener reiner Dokumentcommit 91bb5fa34fa9c949611656c787c343cf7ffee0b7, separat gepusht.
- Inventar auf dieser Basis: 327 ausgewählte eigene Dateien, 221.045 Zeilen, SHA-256 je Datei in evidence/source-inventory.json. Diese Zählung misst nicht die Tiefe der Prüfung.
- Spätere fremde Commits bis 3225b1e7e45a4b44dad27a4452cb3379688b55dd betreffen nur Dokumente und Orchestrierung. Der nur lesende Git-Vergleich liegt in evidence/source-state.txt. Produktstand der belegten Stellen unverändert.
- Beim Abschluss war außer dem eigenen neuen Auditordner kein uncommittierter Pfad sichtbar. Kein Fremdpfad wurde bearbeitet oder für den Auditcommit gestagt.

## Ausgeführte Prüfungen vor dem Stoppsignal

| Prüfung | Ergebnis | Rohbeleg / Grenze |
|---|---|---|
| Qualitäts-Preflight | Werkzeuge verfügbar, kein Preflight-Fund | Sessionausgabe, kein eigener vollständiger Qualitätsbeweis |
| Dokuriegel für PLAN-STAND | Exit 1 für geplante fehlende NAK-226/NAK-227-Verweise und historischen Pfad | Als bekannte Dokumentgrenzen eingeordnet |
| py -3.13 tools/plan/gesundheit.py --mit-selbsttest --json | Selbsttest 121/121; Gesamt-Exit 4 | evidence/gesundheit-raw.txt und gesundheit.json |
| cargo audit --json, Arbeitsverzeichnis broker | Exit 0; 0 Advisories für 66 Abhängigkeiten | evidence/cargo-audit.json, begrenzter Datenbankabgleich |
| Sources-Publikation | Fünf kontrollierte Läufe jeweils 13/14, Exit 1; seriell 14/14, Exit 0 | evidence/state/run-1.txt bis run-5.txt; run-serial.txt |
| Revisionsroundtrip | 20/24 Erwartungen, Exit 1 | evidence/state/run-numbers.txt |
| Broker-Hypothesenreihenfolge | Vergleich fehlgeschlagen, Cargo-Exit 101 | evidence/broker/v2.log; Original-Crate und Testhaken |
| Original-Queue-Reihenfolge | (3, 2) statt (2, 3), Cargo-Exit 101 | evidence/broker/queue.log; keine vollständigen Wirepayloads |
| Bestehende Original-Queue-Tests | 8/8, Exit 0 | evidence/broker/queue-controls.log |
| Invalidierungsrollback | Vergleich fehlgeschlagen, Cargo-Exit 101 | evidence/broker/rollback.log; isolierte SQLite-Datei |
| DSP-Tap/Queue-Verbund | Zeitachse und Gap falsch, Exit 1 | evidence/audio/gap-validated-log.txt; Sonde-Abzweigung nachgebildet |
| Aktiver DSP / Hörmarkierung, float-Verengung | Öffentliche Parameterprüfung 1; beobachtete ungezählte Inf-Ausgaben | evidence/audio/repro-validated-log.txt; Beobachtungsausgabe bei regulärem Exit |
| Analysemodelle | Aktivität, Passage, Headroom, Loudness aus Original-FeatureEngine | evidence/models/models-results.txt und models-extra-results.txt; reguläre Exits, kein vollständiger Host-/Wirelauf |
| Sonde-Prozessor / VST3-Wrapper | Erfolgreiche Host-Aufrufe; teilweise Vierkanalbearbeitung und Restpeak nach Reset | evidence/host/run.log und vst3_full.log; lokaler Testhost, keine DAW |
| RSS-Orakel | Historische Kontrolle grün; fehlende RSS ebenfalls grün; Gegenbeleg-Exit 1 | evidence/repro-soak-missing-rss.txt; kein neuer Soak |

Die jeweiligen archivierten Harnessquellen und Buildargumente beschreiben die damals ausgeführten Programme. Absolute Scratchpfade sind erhalten. Aus den vorliegenden Logdateien wurden keine fehlenden Kommandos, Laufzeiten oder weiteren Wiederholungen erfunden.

## Abgebrochen oder nicht ausgeführt

Der Aufruf pwsh -NoProfile -File tools/beweise.ps1 -Bauen -Titel 'Zweiter unabhaengiger Codeaudit 2026-09-12' wurde gestartet. Er meldete den Build der vorgesehenen Ziele, lieferte aber keinen abgeschlossenen eigenen Kanonbericht. Root unterbrach den noch laufenden Prozess auf die spätere User-Einschränkung; Rückgabe Exit 1. Dieser Exit ist eine Unterbrechung, kein lokalisiertes Produkt-Testversagen und kein PASS. Frische Einzelbinaries entstanden, daraus folgt kein kompletter Lauf.

Clippy, Sanitizer, vollständige DAW-/Callback-Performance, Installation/Upgrade/Repair/Rückweg und die in der Übergabe aufgeführten offenen Kandidaten sind nicht abgeschlossen. Insbesondere kein aktuelles 64/64 und kein vollständiges formales c-review-Pipeline-Ergebnis behauptet. Die vorhandene EBU-Prüfung wurde nur lesend mit der Primärquelle abgeglichen; keine fehlenden Signale nachträglich ausgeführt.

## Abschluss ausschließlich an Dokumenten

Nach dem Stoppsignal: Agentenstatus gelesen, verbleibende Läufe beendet, Quellen und vorhandene Ausgaben gelesen, 52 bestehende ausgewählte Belegdateien in die neuen Unterordner kopiert, Markdown geschrieben. Keine Produktprogramme oder neuen Fehler-/Grenzfallharnesses gestartet.

Die abschließende Dokumentprüfung umfasst Datei- und Linkexistenz, lesbare UTF-8-Dateien, Belegmanifest, Git-Diff und Whitespaceprüfung. Ihr eigener Bericht liegt in evidence/document-check.txt. Sie ist keine Produktprüfung. Checksummen der archivierten Belege stehen in evidence/archive-sha256.txt; die Datei selbst ist von ihrer eigenen Prüfsumme ausgenommen.

Die Rohbelege behalten ihre ursprünglichen Zeilenenden und Leerzeilen; die lokale .gitattributes verhindert Git-Textnormalisierung ausschließlich in evidence/. Deshalb meldet der allgemeine Whitespacecheck ursprüngliche Archivformatierung, insbesondere doppelte CR-Zeilenenden in zwei damaligen CMD-Dateien. Diese Belege wurden nicht zur kosmetischen Bereinigung verändert. Die eigene Markdown-Dokumentation wird separat ohne solche Ausnahmen geprüft. Die eingeordneten Archivhinweise stehen in evidence/whitespace-review.txt. Das ist keine Ausnahme für Produktcode und keine behauptete allgemeine Whitespace-Fehlerfreiheit.

Das geprüfte VST3-Modul wurde beim Dokumentabschluss ausschließlich als Datei gehasht. SHA-256 stimmt mit dem im Hostpaket verwendeten Modul überein: 0004D7B8C9F29ADE4710BB0FCFA25B8A622966B314A4630592594BDE816925A9.

Die User-Datei UEBERGABE-AN-CLAUDE.md ist eigenständig verständlich. Weder unveröffentlichte Agentengespräche noch diese ergänzende Notiz sind nötig, um ihre Befunde, Prioritäten und Nachweisgrenzen zu verstehen.
