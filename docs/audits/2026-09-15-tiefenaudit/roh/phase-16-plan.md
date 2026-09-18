# Phase 16 – Prüfplan und feste Ausgangslage

Auftrag vom 18.09.2026: funktionale Verlässlichkeit, Korrektheit und Qualität vertiefen; bis zur integrierten Befundliste und Fable-Arbeitsübersicht arbeiten. Die Durchführung verwendet Codex-Agenten und das Vorgehen des ultracode-Skills, keinen nativen Claude-Workflow. Die Arbeitsartefakte liegen gemäß Auftrag ausschließlich im Auditordner.

## Grenze und Herkunft

- Ausgangs-HEAD: `aff2d8188f33a9525aec1869449773bfdbe305a6`.
- Unzugeordnete, unveränderte Einträge beim Start: `briefing-hub/`, `nimbalyst-local/`.
- Isolierte lokale Kopie für Tests: `C:\na-audit16-aff2d818`, erstellt mit `git clone --local -c core.longpaths=true`. Eigene Buildverzeichnisse und gegebenenfalls eindeutig isolierte Probe-Pipes.
- Im gemeinsamen Checkout nur dieser Audit, Berichte und reproduzierbare Nachweise. Produktquellen bleiben unverändert. Keine Installation, keine FL-Projekte, keine Produktions-Pipe. Sicherheitsbereiche und bestehende Sicherheitsbefunde bleiben ausgeschlossen; Design bleibt geparkt.
- Profile: deep und gezielte soak-Teilprüfungen. Kein pauschaler Kanonlauf, keine Releasefreigabe. Frühere Befunde sind Suchhinweise; aktuelle Quelle und aktuelle Gegenproben entscheiden.

## Pakete und Zusammenarbeit

| Paket | Auftrag | Verantwortung und Ergebnisdatei |
| --- | --- | --- |
| A | Produktverdrahtung und verspätete Ergebnisse: Auslöser bis Verbraucher; Generation, Revision, Eigentümer; direkte Testgegenwege | Unabhängiger Finder, `phase-16-a-verdrahtung.md` |
| B | Telemetrie, Linkwechsel, zwei Mains, Eviction, Konflikte, P1-Verlust, Store-Ausfall | Unabhängiger Finder, `phase-16-b-verbindungen.md` |
| C | Persistenz, älterer Recall, Klonen/Preset, ausstehende Befehle, Dirty/Revision, read-only, Evidenzsegmentierung | Hauptsession, anschließend unabhängiger Angriff, `phase-16-c-persistenz.md` |
| D | Echter DSP: Aus/Ein, Mono/Auto-Gain, Gitter/Nyquist, Shelves, Steuertakt, Reload, zusammengesetzte Änderungen | Unabhängiger Finder, `phase-16-d-dsp.md` |
| E | prepare/release/reset, Bypass/Offline, aktive Mehrinstanzlast, Ressourcen über Zyklen | Zweite Agentenwelle, `phase-16-e-last-lebenslauf.md` |
| F | Gegenbeweise, Produkteinstiege, Assertions, Testgrenzen, unabhängiger Angriff jedes neuen Kandidaten | Zweite Welle und zentrale Quellengegenprüfung, `phase-16-f-gegenpruefung.md` |

Die Hauptsession erstellt Abdeckungsmatrix, Integrationsentscheidungen, Quellen- und Artefaktprüfung und die abschließende Fable-Übergabe. Kandidaten erhalten erst nach fremdem Angriff und zentraler Quellprüfung eine T3-ID. Einzelfehler werden nicht durch Varianten doppelt gezählt.

## Zuordnung aller 19 Ausgangsfelder

Die Nummern F01–F19 folgen der Reihenfolge in Phase 15, Abschnitt „Offene Prüffelder ohne Befund-ID“. Endurteile mit Begründung werden in `phase-16-abdeckung.md` und `BEFUNDE.md` ergänzt.

| Feld | Verbleibende Frage | Zuständiges Paket |
| --- | --- | --- |
| F01 Hypothesenrechnung | Gilt der gesammelte Mitgliedschafts-/Führungs-/Deskriptorstand beim Rückschreiben noch? | A, F |
| F02 Same-Instance-Replay | Können ausstehende Befehle einen erneut geladenen Stand überschreiben? | C, A, F |
| F03 Vorschläge | Gültigkeitsprüfung nach Rechnen und heutige Zustellung? | A |
| F04 Experimentresultat | Zwischen Messung und Übernahme ausgeschlossene Belege? | A, F |
| F05 PRE/POST | Erreichbarkeit und Rückschreibprüfung nach der Reparatur von T3-08-04? | A |
| F06 DSP-Evidenz | Wirkung von State-Hash-/Revisionswechsel auf Evidenzsegmente? | C, A |
| F07 Mono-Auto-Gain | Kompensation tatsächlich unwirksamer right-/side-Bänder? | D, E |
| F08 Sampleraten unter 40 kHz | Angenommene Raten und Gitter über Nyquist? | D |
| F09 Gain-Telemetrie | Zielwert, tatsächlicher Gain, Zeit- und Generationsbezug? | D, A |
| F10 Shelves mit hohem Q | Zulässige Antwort, Produktregel und echtes Ausgangsaudio? | D |
| F11 Steuertaktphasen | Gleichheit bei Phasen 1/7 und Blocks 1/7/64/333? | D |
| F12 Dynamischer Reload | Kalte gegenüber eingeschwungener Historie und zugesagte Reload-Treue? | D, C |
| F13 Telemetrie-Entkopplungsrennen | Beendet T1 beim späten Ende die neue T2-Kopplung? | B |
| F14 Telemetrie dauerhaft abgewiesen | Weiterer Control-Betrieb, Diagnose und Rückweg? | B |
| F15 Geschriebene P1-Ereignisse | Annahme, Linkschluss und tatsächliche Wiederholung? | B |
| F16 Zwei Mains | Diagnose, Probezuordnung und Entfernung einer Main? | B |
| F17 Replay nach Eviction | Autorisierung vor erstem Heartbeat und Wiederherstellung? | B |
| F18 Konfliktriegel über Zyklen | Lebensdauer, Quarantäne und Wirkung auf andere Clients? | B, E |
| F19 FL-Preset und Identität | Produkt-State-Übernahme gegenüber tatsächlichem FL-Presetverhalten? | C, E |

## Integrationsgates

1. Jeder Bereich bekommt ein begründetes Endurteil, kein bloß erneut übertragenes „offen“.
2. Jeder neue Befund benennt Wirkung, reproduzierbare Reihenfolge, heutige Quellstellen, Aufruf- und Gegenwege, Zusage oder fehlende Regel, stärkstes Gegenargument, Testgrenze, Duplikatabgrenzung und kleinsten Fix samt Regressionstest.
3. Erreichbarkeit getrennt: heutiger Produkteingang; gebauter API-Vertrag; erst nach Reparatur; noch nicht gebaut. Quellenherleitung, Nachrechnung, echte Komponentenausführung und echter Host bleiben getrennt.
4. Alle neuen Kandidaten werden unabhängig angegriffen; widersprüchliche Urteile werden durch Quellenprüfung, Gegenprobe oder engere Aussage aufgelöst.
5. Alte Dokumentbytes bleiben vollständig als Präfix erhalten. Nur neue Phase und datierte Korrekturen ergänzen. Prüfen: Quellenänderungen, IDs, Summen, lokale Links, Nachweisdateien, eigener Diff und `git diff --check`.
6. Fable erhält zusammengehörige Ursachen, Abhängigkeiten, Invarianten, gezielte Regressionen, offene Produktentscheidungen und eine Reihenfolge nach realer Wirkung.
