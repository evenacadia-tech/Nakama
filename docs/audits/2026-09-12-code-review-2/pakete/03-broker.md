# Paket 3 — Broker, Berechnung, Queue und Store

**Auftrag:** Unabhängig Zustandsberechnung, Zustellordnung, Persistenz und Rückwege sowie D5/D9 prüfen. Reproduktionen nur mit lokalem Original-Crate und isolierten Dateien.

**Vorhandene Ergebnisse:** F02 aktualisiert NAK-253 vom Verdacht zu einem beobachteten Ergebnisrücksprung. F03 reproduziert eine Queuefolge über zwei Objektschlüssel. F14 reproduziert die bekannte NAK-163-Kombination von Storefehler und Retention.

**Kontrollen und Grenzen:** Acht bestehende Original-Queue-Tests bestehen. F03 nutzt beschriftete Bytes, keine vollständigen Wire-Nachrichten; Consumerwirkung nur quellbelegt. F02 zeichnet den Snapshot-Payload am Sender auf, nicht an einer realen Pipe. F14 verwendet eine isolierte SQLite-Datei. Keine Produktionspipe.

**Status:** Rohbelege entstanden vor den automatischen Blockaden; abschließende Agentensynthese abgebrochen. Root hat Harnessquellen, Produktstellen, Logs und bestehendes Register lesend zusammengeführt. Kein Agentenabschluss oder vollständiger Broker-E2E-PASS behauptet.

**Artefakte:** evidence/broker/ enthält Original-Crate-Verweis, Testquellen, vier Logs und Kontrolllog. Produktcode unverändert.
