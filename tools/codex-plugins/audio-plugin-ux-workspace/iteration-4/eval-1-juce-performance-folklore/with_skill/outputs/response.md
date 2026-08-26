Nein — diese Pauschalstrategie übernehmen wir nicht. Die flüssige Web-Darstellung ist eine User-Beobachtung, aber kein Beleg für JUCE-Painting, FL-Studio-Last oder Audio-Thread-Stabilität; der Web-Prototyp besitzt nur sein abgenommenes Interaktions- und Motion-Verhalten. [Nakama-Transfer](C:/Users/phili/Documents/Nakama/tools/codex-plugins/audio-plugin-ux/skills/audio-plugin-ux/references/nakama-transfer.md:54)

Dokumentiert sind klare Bedingungen für die drei JUCE-Schalter:

- `setOpaque(true)` ist nur korrekt, wenn die Komponente wirklich jeden Pixel deckend zeichnet; selbst die Verdeckungsprüfung hat Kosten.
- `setPaintingIsUnclipped(true)` ist nur für einfache Komponenten zulässig, die garantiert nie außerhalb ihrer Grenzen zeichnen; falscher Einsatz erzeugt Artefakte.
- `setBufferedToImage(true)` puffert die Komponente einschließlich ihrer Kinder. Jeder Repaint der Komponente oder eines Kindes entwertet den betreffenden Cache – bei Spektrum, Kurve oder animierten Controls kann das zusätzliche statt weniger Arbeit erzeugen. [JUCE Component API](https://docs.juce.com/master/classjuce_1_1Component.html)
- Ein JUCE-`Timer` läuft auf dem Message-Thread, bis er gestoppt wird; seine Taktung ist ausdrücklich nicht präzise garantiert. Ein permanenter 60-Hz-Timer beweist daher weder 60 sichtbare FPS noch gute Host-Performance. [JUCE Timer API](https://docs.juce.com/master/classjuce_1_1Timer.html)

Die native Richtung ist deshalb:

- Statische Schale und unveränderte Beschriftungen von Kurve, Spektrum und transienten Controls trennen.
- Nur bei neuer Messung, geänderter Automation oder echter Interaktion die betroffene Region neu zeichnen.
- Während eines EQ-Zugs Cursor, Griff, Zahlenwert und Kurve unmittelbar zusammen aktualisieren. Nur so kann der Produzent die hörbare Änderung der aktuellen Geste beziehungsweise dem autoritativen Host-Zustand zuordnen. Wenn das Frame-Budget knapp wird, sinkt zuerst die Analyzer- oder Dekorationsrate – nicht die sichtbare Parameterwahrheit. Das entspricht dem aktuellen Verhaltenvertrag. [UX-CONTRACT.md](C:/Users/phili/Documents/Nakama/UX-CONTRACT.md:64)
- Ohne Geste, neue Daten oder laufende Messung den Repaint-Timer stoppen beziehungsweise stark absenken; ein versteckter oder ruhender Editor darf Host-Ressourcen nicht dauerhaft binden.

Vor dem Festschreiben profilen wir Baseline und jeden Schalter einzeln: benannter Windows-Release-VST3-Build in FL Studio, jede unterstützte Skalierung, eine und mehrere Instanzen, geöffnet/verdeckt, echter Stillstand und direkte Feinmanipulation sowie maximale unterstützte Analyzer-, Graphobjekt- und Overlay-Last mit laufender Host-Automation. Erfasst werden Eingabe-bis-Sichtreaktion, Paint-/Frame-Kosten einschließlich Perzentilen und schlechtestem Frame, Repaint-Fläche und -Anzahl, Analyzer-Takt, UI-CPU/GPU sowie Audio-Thread-Gesundheit und Aussetzer. Offen bleibt bis dahin, welche konkrete Komponente von welchem Flag profitiert; universelle Grenzwerte gibt die Quelle nicht her.

Archivroute: [Archiv-Einstieg](C:/Users/phili/Documents/Nakama/wissen/LIES-MICH.md:73) → [Archivregeln](C:/Users/phili/Documents/Nakama/wissen/AGENTS.md:72) → [Index](C:/Users/phili/Documents/Nakama/wissen/INDEX.md:29) → [Engineering-Router](C:/Users/phili/Documents/Nakama/wissen/engineering/LIES-MICH.md:1) → aktiver Eintrag `w-2026-08-26-juce-ui-performance-messen`, Pfad `wissen/engineering/2026-08-26-juce-ui-performance-messen.md`, Prüfstand `quellen-geprueft`. [Aktiver Performance-Eintrag](C:/Users/phili/Documents/Nakama/wissen/engineering/2026-08-26-juce-ui-performance-messen.md:1)
