# Paket 2 — State und IPC, erste Welle

**Auftrag:** D2/D3/D4/D6/D8 gegen den heutigen Code prüfen; Save/Load, Callbackbesitz, Befehlsannahme, Reload und Verbraucherübergänge lesen.

**Ergebnis:** Alte Ursachen quellseitig behandelt. Die Mitgliederpublikation innerhalb einer Reload-Generation blieb als Kandidat und wurde anschließend im getrennten Paket 4 direkt untersucht.

**Nicht entschieden:** Writer-Headroom gegen Reader-Sammlungsgrenzen (alter V1 / NAK-252). Ein Harness wurde vorbereitet, ein abgeschlossener Build- und Roundtrip-Beleg liegt nicht vor. Deshalb kein bestätigter Stateverlust und kein neuer Fehler-ID-Eintrag.

**Status:** Agentenlauf durch automatische Sicherheitsblockade abgebrochen. Kein vollständiger Paket-PASS. Nach der letzten User-Einschränkung keine Wiederaufnahme.

**Schreibgrenze:** Produktquellen unverändert; unvollständige Scratchentwürfe sind kein Beleg. Integration in Abschnitt 3 und 5 der Claude-Übergabe.
