---
id: w-2026-08-26-audiodevcon-videoauswahl
typ: wissenseintrag
status: aktiv
datum: 2026-08-26
aktualisiert: 2026-08-26
hauptbereich: engineering
weitere_bereiche: [ux, ui-motion, dsp, fl-studio]
pruefstand: metadaten-geprueft
---

# AudioDevCon: kuratierte Videoauswahl für Nakama

Der Channel [ADC – Audio Developer Conference](https://www.youtube.com/@audiodevcon/videos)
enthielt beim Scan 650 Videos. Ausgewählt wurde nicht nach Popularität,
sondern nach direktem Bezug zu Nakamas aktueller Arbeit: Audio-UX,
Figma/Web-Referenz zu nativer JUCE-UI, Windows/FL Studio, Echtzeitsicherheit,
Messvisualisierung und Plugin-Validierung.

## Priorität 1

Diese sechs Videos bilden den Kern. Alle sechs sind ausgewertet und in die
jeweiligen Themenblätter eingearbeitet. Wo kein Transkript verfügbar war,
bleibt der Erkenntnisumfang ausdrücklich auf direkt geprüfte Folien,
Beschreibung und Kapitel begrenzt.

| Video | Bereich | Warum für Nakama wichtig | Prüfstand |
|---|---|---|---|
| [Enabling Unique Sound Experiences Through User-Centred Design](https://www.youtube.com/watch?v=UEyGN9DG7Ns) | UX, Produkt | Nutzeranforderungen, Audio-Feedback, Designsystem und funktionale Prototypen | vollständiges Auto-Transkript geprüft; [eingearbeitet](../ux/2026-08-26-nutzerzentriertes-audio-design.md) |
| [PSD to DAW – Building a Pixel-Perfect UI Pipeline](https://www.youtube.com/watch?v=hebLkAR5X3I) | UI/Motion, Engineering | native Asset-Pipeline, visuelle Treue, Resampling, Layering und Displaygrößen | vollständiger 52-Folien-Satz, Beschreibung und Ressourcen-Repository geprüft; kein Transkript; [eingearbeitet](../ui-motion/2026-08-26-visuelle-treue-native-ui-pipeline.md) |
| [Measuring and Improving UI Performance with the JUCE C++ Framework](https://www.youtube.com/watch?v=0n9x6R0fheo) | Engineering, UI/Motion | Komponentenbaum, Paint-Kosten, Text, Caching und Windows-Direct2D in JUCE | vollständiges Auto-Transkript, Demonstrationen und aktuelle JUCE-Referenz geprüft; [eingearbeitet](2026-08-26-juce-ui-performance-messen.md) |
| [Modernizing Legacy Audio Plugin Codebases – Lessons from FL Studio’s Plugin Suite](https://www.youtube.com/watch?v=zY8uHzAdnzk) | FL Studio, Engineering | echte FL-Studio-Pluginmigration, GUI-Modernisierung, Validierung, CI und Installer | vollständiges Auto-Transkript und sichtbare Kernfolien geprüft; [eingearbeitet](../fl-studio/2026-08-26-plugin-modernisierung-ohne-vertrauensbruch.md) |
| [The UX of Audio Experiences](https://www.youtube.com/watch?v=E3zUIR8xkrw) | UX, DSP | Psychoakustik, Kontext und falsche Annahmen bei Audio-Interaktionen | Beschreibung, vollständige Kapitelstruktur und ausgewählte sichtbare Folien geprüft; kein Transkript, Aussagen bewusst begrenzt; [eingearbeitet](../ux/2026-08-26-audio-interaktion-kontext-statt-annahmen.md) |
| [Building Inclusive Audio Tools](https://www.youtube.com/watch?v=O5xX9a7P-SU) | UX, UI/Motion | komplexe Audio-Controls für Screenreader und Tastatur, WCAG/ETSI, praktische Tests | vollständiges Auto-Transkript, Demonstrationen, aktuelle W3C- und JUCE-Referenz geprüft; [eingearbeitet](../ux/2026-08-26-zugaengliche-audio-controls.md) |

## Priorität 2

Diese Videos sind relevant, aber nach den sechs Kernvorträgen dran.

| Video | Bereich | Erwarteter Nutzen |
|---|---|---|
| [Real-Time, Low Latency and High Temporal Resolution Spectrograms](https://www.youtube.com/watch?v=QbNPA5QJ6OU) | DSP, UI/Motion | Datenkadenz, Zeitauflösung und Darstellung für Nakamas Spektrum |
| [Mind the Spike – Benchmarking for Worst-Case Execution Time in Realtime Code](https://www.youtube.com/watch?v=7RrOjl996WQ) | Engineering | Worst-Case statt Durchschnitt; passend zu Nakamas Echtzeit- und p95-Prüfungen |
| [VST3 History, Advantages and Best Practice](https://www.youtube.com/watch?v=0QBWXC8KNz0) | Engineering, FL Studio | ältere, aber grundlegende VST3-Semantik und Host-/Plugin-Grenzen |
| [Hybrid Prototyping With Web Tech and JUCE/C++](https://www.youtube.com/watch?v=nRAQ2a7ghYA) | UI/Motion, Engineering | Vergleichsfolie für Nakamas Web-Simulator und spätere native Übertragung |
| [Pluginval – Yeah, but Why Validate Plugins?](https://www.youtube.com/watch?v=Q97LBXqgMus) | Engineering | Plugin-Validierung, Fehlerklassen und Teststrategie |
| [Test-Driven Development for Audio Plugins](https://www.youtube.com/watch?v=aLOlRSu6p00) | Engineering, DSP | reproduzierbare Audio- und Zustandsprüfungen |

## Bewusst zurückgestellt

Nicht in die erste Auswahl kamen Synthese-, Reverb-, Granular-, Android-,
Game-Audio-, GPU-/ML- und Vertriebsthemen. Sie sind fachlich interessant,
lösen aber keine aktuelle Nakama-Frage. Der Cubase/Metal-Vortrag bleibt trotz
guter Performance-Inhalte ebenfalls zurückgestellt, weil Nakamas erste
Zielumgebung Windows und FL Studio ist und der JUCE-Vortrag die unmittelbarere
Grundlage liefert.

## Verarbeitungsregel

Ein Video wird erst nach Beschreibung, Transkript beziehungsweise visueller
Prüfung inhaltlich übernommen. Im Archiv landen nur Erkenntnisse,
Zeitstempel, Grenzen und Nakama-Transfer. Vollständige Transkripte werden nicht
gespeichert.
