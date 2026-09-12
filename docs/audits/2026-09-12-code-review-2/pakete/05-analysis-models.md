# Paket 5 — Ausweitung auf Analysefachmodelle

**Auftrag:** Bisher schwächer geprüfte fachliche Orakel und Fensterbedeutungen untersuchen: Aktivität, Spektrum, Passage, Loudness, Headroom sowie ausgewählte Stereo-/Fingerprintfragen.

**Übernommen:** F06 (Dichte gegen Energiegate), F07 (True-Peak-Vorgeschichte am vorab gebundenen Passagenstart), F08 (64-Rahmen-Headroom statt vollständiger Passage), F10 (Gültigkeit kürzer als betroffene Loudnesshistorie).

**Beweisart:** Direkte Original-FeatureEngine-Header mit gültigen StampedBlocks. Harnesses drucken Werte und enden regulär. Kein vollständiger Sonde-/Wire-/DAW-Lauf. Die unabhängigen Referenzen sind Signalpegel, isolierter gleicher Ausschnitt, vollständige Verteilung beziehungsweise unbeschädigter Dauerlauf.

**Nicht übernommen:** Absolutwert der Stereo-Mid/Side-Größe, Fingerprint ohne Onsets und K-Filterhistorie. Fachliche Bedeutung beziehungsweise Gegenbelege noch unzureichend geklärt; zusätzliche Logwerte sind keine zusätzlichen bestätigten Defekte.

**N01:** Hinweis auf unvollständige EBU-Testauswahl durch Root nach dem Stoppsignal ausschließlich lesend gegen Originaltest, Korpuserzeuger und Primärquelle überprüft. Keine fehlenden Normsignale neu ausgeführt.

**Status:** Vorhandene Läufe abgeschlossen, weitere Arbeit auf User-Signal unterbrochen. Artefakte unter evidence/models/. Kein Produktcode geändert.
