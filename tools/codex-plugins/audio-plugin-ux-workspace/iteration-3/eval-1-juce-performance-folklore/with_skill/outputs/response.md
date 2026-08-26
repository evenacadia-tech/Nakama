Nein. Der flüssige Web-Simulator ist kein Beleg für einen permanenten nativen 60-FPS-Repaint. Sein eigener Scheduler arbeitet bereits bedarfsgesteuert: `requestAnimationFrame` läuft nur bei aktiver Messung, Wiedergabe und Verbindung; der 50-ms-Tick nur, wenn der Adapter ihn benötigt ([app.js](/C:/Users/phili/Documents/Nakama/design/prototyp/src/app.js:84)). Der UX-Vertrag verlangt eine Reaktion von Griff, Wert und Kurve innerhalb eines Frames, aber keine dauernde Vollflächenanimation; das Spektrum erhält Daten mit 20 Hz und interpoliert nur die Darstellung ([UX-CONTRACT.md](/C:/Users/phili/Documents/Nakama/UX-CONTRACT.md:66)).

Auch der native Editor ist bewusst anders gebaut: Er pollt derzeit mit 30 Hz, ruft `repaint()` aber nur bei neuen Daten, geänderten UI-Zuständen, Pipe-Status oder Meldungswechseln auf ([PluginEditor.cpp](/C:/Users/phili/Documents/Nakama/eq-copilot/plugin/src/PluginEditor.cpp:191)). Ein permanenter 60-Hz-Timer wäre daher eine messbare Regression im Leerlauf.

Die drei JUCE-Schalter sind keine globalen Optimierungen:

- `setBufferedToImage` puffert Komponente und Kinder; jede Änderung eines animierten Kindes invalidiert den Elternpuffer. Für Spektrum, Kurve oder bewegte Griffe kann das zusätzliche Kopierarbeit erzeugen.
- `setOpaque` ist nur korrekt, wenn wirklich jedes Pixel deckend gemalt wird. Andernfalls entstehen Darstellungsfehler; viele kleine opaque Geschwister können durch die Verdeckungsprüfung sogar teurer werden.
- `setPaintingIsUnclipped` ist nur für einfache kinderlose Komponenten zulässig, die garantiert nie außerhalb ihrer Grenzen zeichnen. Bei Komponenten mit Kindern greift es nicht.

Das entspricht sowohl der eingebundenen JUCE-8.0.9-Implementierung als auch der [offiziellen JUCE-Component-Dokumentation](https://docs.juce.com/master/classjuce_1_1Component.html).

Der aktuelle Release-PaintBench liefert bei 1200 × 832 über 240 Frames durchschnittlich 3,96 ms, p95 5,93 ms und maximal 7,80 ms pro vollständigem `paint()`. Das ist ein Offscreen-Softwaretest ohne FL-Studio- und Message-Loop-Last, also kein Host-Beweis. Es zeigt aber das Risiko: 60 erzwungene Voll-Paints entsprächen rechnerisch bereits rund 238 ms Malzeit pro Sekunde und Editor, bevor Host, Automation und weitere Instanzen hinzukommen.

Die technische Entscheidung lautet deshalb:

- kein permanenter 60-FPS-Timer;
- Repaints weiterhin über Zustandsänderungen und begrenzte Dirty-Regionen;
- statische Gehäuse-, Material- und Beschriftungsebenen von Spektrum, Kurve und Griffen trennen;
- Buffering oder Flags nur an isolierten Komponenten und jeweils einzeln gegen die unveränderte Basis messen;
- Animationstakt nur während direkter Manipulation, Messung oder Spektruminterpolation aktivieren und im Leerlauf vollständig stoppen.

Der verbindliche Host-Test läuft als Windows-Release-VST3 in FL Studio bei 100, 125, 150 und 200 Prozent sowie mit 1, 4 und 16 gleichzeitig geöffneten Editoren. Die Worst-Load-Szene ist: EQ Center bei 200 Prozent, Master plus 16 Probes, acht aktive Bänder, laufendes 20-Hz-Spektrum mit Interpolation, Comparison-, Draft-, Protected-Range- und Status-Overlays sowie eingehende Host-Automation während Band-Drag und numerischer Eingabe. Dieselbe Matrix wird zusätzlich unverändert im Leerlauf gemessen.

Erfasst werden Repaint-Anzahl und Dirty-Fläche, durchschnittliche sowie p95/p99/maximale Paint-Zeit, Eingabe-bis-sichtbare-Reaktion, Message-Thread- und Idle-CPU, verlorene UI-Frames und Audio-Xruns. Bestanden ist eine Änderung nur, wenn Griff, Wert und Kurve innerhalb eines Frames autoritativ bleiben, im Leerlauf keine fortlaufenden Voll-Repaints entstehen, keine Audioaussetzer auftreten und p95 sowie Worst Frame gegenüber der Basis über die Zielmatrix nachweislich besser werden.

Quellen-Ledger des Forschungsarchivs: `wissen/INDEX.md` → `w-2026-08-26-juce-ui-performance-messen` → `wissen/engineering/2026-08-26-juce-ui-performance-messen.md` → Verifikationsstand `quellen-geprueft`.
