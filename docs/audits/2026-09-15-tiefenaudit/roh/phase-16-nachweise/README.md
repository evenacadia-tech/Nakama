# Phase 16: Nachweise und Wiederholung

Geprüfter Produktstand: `aff2d8188f33a9525aec1869449773bfdbe305a6` in der isolierten Kopie `C:\na-audit16-aff2d818`. [Ausgangslage](ausgangslage.json), [zentrale Abschlussprüfung](abschluss.json), [Hauptbericht](../../BEFUNDE.md). Die Prüfprogramme linken unveränderte Produktkomponenten; sie sind keine neuen Produktfunktionen und ihre erfolgreichen Reproduktionsassertions bedeuten teilweise ausdrücklich **Fehler reproduziert**, nicht Fehler repariert.

## Herkunft und bytegenaue Ablage

- A: [Cargo-Projekt](a/Cargo.toml), [Lockdatei](a/Cargo.lock), [Quellen-/Artefaktmanifest](a/quellmanifest.json), [Lauf](a/lauf-pinned.txt).
- B: [Caller/Runner](b/run.ps1), [eingefrorene Lockdatei](b/Cargo.lock), [Manifest](b/manifest.json), [Komponenten](b/components-run.txt) und [echter Listener](b/wire-run.txt). B/C-Manifeste wurden nach den bereits beobachteten erfolgreichen Läufen erfasst; diese Erfassung war kein weiterer Testlauf.
- C: [CMake](c/CMakeLists.txt), [echter Prozessorcaller](c/state_probe.cpp), [finales Manifest](c/manifest-reviewed.json), [nach Gegenprüfung neu gebaut](c/build-reviewed.log) und [33/33 ausgeführt](c/ergebnisse-reviewed.txt).
- D: [Runner](d/run.ps1), [Manifest](d/manifest.json), [Messungen](d/ergebnisse.txt); unabhängige Wiederholung und Orakelprüfung in [F-D](../phase-16-f-gegenpruefung-d.md).
- E: [Timing-/Prozessorlauf](e/manifest.json), [Ressourcenlauf](e/resources-manifest.json), [vollständiger Bericht mit CSV-Links](../phase-16-e-last-lebenslauf.md).

Die Manifeste halten die **tatsächlich kompilierten Bytes der isolierten Quelle** fest. Der gemeinsame Windows-Checkout unterscheidet sich bei einigen Dateien nur in CRLF/LF. Der Abschlussprüfer verlangt für die isolierten Quellen exakte Hashgleichheit und für den Live-Checkout höchstens diese konkret ausgewiesene Zeilenendenabweichung; zusätzlich muss der Git-Produktdiff zum geprüften HEAD leer sein. Keine sonstige Whitespace-Normalisierung ist zugelassen.

Die lokale `.gitattributes` erhält alle Nachweisbytes auch in Git unverändert und erkennt CRLF als reguläres Windows-Zeilenende an. Nur unveränderte Compiler-/Programmausgaben (`.log`, `.txt`) sind von Format-Whitespaceprüfungen ausgenommen: etwa D-Ausgaben enden absichtlich mit einem Leerzeichen. Code und Markdown werden auf nachlaufende Leerzeichen, zusätzliche Leerzeilen am Dateiende und Leerzeichen vor Tabs geprüft. Rohlogs werden nicht nachträglich geglättet, um einen Formatcheck zu bestehen.

E ergänzte sein Ressourcen-Bauziel erst nach der Audiotimingmessung. Dessen Manifest hält das neue CMake fest; das ältere Timingmanifest hält die damalige Datei. Der Abschlussprüfer rekonstruiert genau diese ältere Datei durch Entfernen ausschließlich des ergänzten Ressourcen-Ziels und vergleicht ihren Hash. Die bestehenden State-/Timing-Zieldefinitionen und ihre aufgezeichneten Binaries blieben dabei unverändert.

## Wiederholung in einer eigenen Nachweiskopie

Die Programme sind absichtlich auf die genannte isolierte Produktkopie eingefroren. Vor einer Wiederholung HEAD und leeren Produktdiff prüfen. MSVC 2022 x64, CMake, Python 3.13, Cargo sowie die vom Produkt-CMake vorgesehenen JUCE-/FlatBuffers-Quellen werden benötigt. In diesem Lauf lagen letztere unter `C:\na-audit16-aff2d818\deps\juce-src` und `deps\flatbuffers-src`; sie wurden aus dem vorhandenen Build übernommen, nicht durch nachgeschriebene Ersatzbibliotheken ersetzt.

Zuerst **diesen Nachweisordner in einen neuen Unterordner der isolierten Kopie kopieren**. Die Runner B/D/E schreiben dort neue Logs und Manifeste; niemals zum Wiederholen die aufgezeichneten Originale überschreiben. Die folgenden Aufrufe beziehen sich auf diese neue Kopie und eigene Buildverzeichnisse. Audio-/Ressourcenläufe nacheinander und ohne konkurrierenden Compiler fahren.

| Paket | Aufruf relativ zur neuen Nachweiskopie | Ergebnisgrenze |
| --- | --- | --- |
| A | `cargo test --offline --locked --manifest-path a/Cargo.toml --test verdrahtung -- --nocapture`, mit `CARGO_TARGET_DIR` in einem eigenen Buildordner der isolierten Kopie | Vier Coordinatorprüfungen, keine Plugininstanz. |
| B | `pwsh -File b/run.ps1 -Mode components -BuildRoot C:\na-audit16-aff2d818\build-b-wiederholung`, danach derselbe Aufruf mit `-Mode wire` | Native Probe-Clients; ausschließlich die im Drahtcaller überprüften Probe-Pipes. |
| C | `pwsh -File c/run.ps1 -BuildRoot C:\na-audit16-aff2d818\build-c-wiederholung` | Echter Gen/SondeProcessor, v3-Testserver; v2-OS-Ziel auf PID-Probe-Pipe umgeleitet. |
| D | `pwsh -File d/run.ps1 -BuildRoot C:\na-audit16-aff2d818\build-d-wiederholung` | Echter DSP-/Transaktionskern; `EXPECTED_FAIL` ist das noch verletzte künftige Produktorakel. |
| E Last | `pwsh -File e/run.ps1 -BuildRoot C:\na-audit16-aff2d818\build-e-wiederholung` | Echte aktive Prozessoren und Analyse, keine angebundenen v3-Clients/ASIO/FL. |
| E Ressourcen | `pwsh -File e/resources-run.ps1 -BuildRoot C:\na-audit16-aff2d818\build-e-wiederholung` | 36 Wiederholungen derselben echten Lebenszyklusfunktion; kein sichtbares Hostfenster. |

Die reine Dokument-/Quellen-/Belegprüfung erfolgt vom Repository aus mit `py -3.13 docs/audits/2026-09-15-tiefenaudit/roh/phase-16-nachweise/abschluss_pruefen.py`. Sie startet **keine** Produktprogramme. Für die Binaryprüfungen muss die im Manifest genannte isolierte Buildkopie noch vorhanden sein; fehlt sie, ist das kein erneut bestandener Ausführungsnachweis. Historische Build-/Fixturefehler in C/E bleiben erhalten und werden in den jeweiligen Rohberichten von den finalen Läufen getrennt.
