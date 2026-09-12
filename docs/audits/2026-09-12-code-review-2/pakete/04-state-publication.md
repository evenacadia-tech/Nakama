# Paket 4 — Quellenpublikation und persistente Roundtrips

**Auftrag:** Den Publikationskandidaten unabhängig bis zum erreichbaren Prozessorablauf verfolgen; parallele Anwendung und Publikation nicht miteinander verwechseln.

**Belegt:** F01 über echten Worker und Message-Thread, fünf kontrollierte Ergebnisse mit falschem Modellstand, serieller Kontrollablauf korrekt. State bleibt korrekt gespeichert; behauptet wird die Abweichung des Modells. Kein Reload erforderlich.

**Zusätzlicher vorhandener Beleg:** F11 für akzeptierte maximale Revisionen: vier MAX−1-Kontrollen korrekt, vier MAX-Fälle fehlerhaft. P3, kein normaler Bedienfall.

**Provenienz:** Vorhandene B14-Testinfrastruktur und unveränderte Prozessorkompilate; Hashliste, Compiler-/Linkargumente, Harnessquellen und Rohlogs unter evidence/state/. Keine kopierte Produktimplementierung.

**Status:** Die belegten Läufe waren abgeschlossen. Eine weitere Untersuchung an Evidenzrücknahme und Reload wurde durch automatische Sicherheitsblockade nicht abgeschlossen. Unausgeführter Entwurf ausdrücklich kein Befund. Nach User-Einschränkung keine weiteren Experimente.

**Schreibgrenze:** Produktquellen unverändert. Root hat Ergebnisse und Einschränkungen in F01/F11 und die offenen Kandidaten integriert.
