# Paket 1 — Audio und DSP

**Auftrag:** Unabhängige erneute Prüfung des Audiopfads, Slot-/Bankbesitzes, Analyseabgriffs und alter D1/D7/D10. Produktquellen nur lesen; lokale Harnesses außerhalb des Repositories.

**Ergebnis:** F09 (nicht publizierter Tap ohne Lücke) und F12 (float-Verengung hinter Endlichkeitsprüfung). Alte Ursachen von D1/D7/D10 im aktuellen Code geändert; ohne abgeschlossenen Kanon daraus kein vollständiger neuer PASS.

**Gegenprüfung:** Öffentliche Parameterprüfung der endgültigen Harnesses erfolgreich. Vorläufige Fassungen mit unvollständigen Parametern oder Hilfs-Stub verworfen. F09 verwendet Original-DSP und Queue, aber nachgebildete Sonde-Abzweigung. F12 verwendet Original-DSP und Hörmarkierung. Die endgültigen Quellen und Rohlogs stehen unter evidence/audio/.

**Schreibgrenze:** Kein Produktcode geändert. Paket abgeschlossen; spätere Schlussarbeit nur Dokumentation. Einzelheiten und Korrekturziele: F09/F12 in UEBERGABE-AN-CLAUDE.md.
