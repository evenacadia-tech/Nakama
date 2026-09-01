# 2026-09-01 — Technische UI-Architektur, Fünferblock 01

## Status

**1 von 5 Entscheidungen gesammelt.** Der gemeinsame Umbau, die vollständige
Sichtprüfung und die Integration in die Detailabnahmen sowie den Blueprint
folgen nach Entscheidung 5.

Für diesen Block gilt der neue User-Entscheid:

> „WÄRE ES nicht sinnvoller wir sammeln wieder ein paar entscheidungen? nach
> jeder frage direkt umbauen ist doch sehr umständlich ? wir können das fest
> machen. immer 5 fragen dann umbauen“

## 1/5 — Dynamic beim ersten Öffnen aktivieren

Auf die unmittelbar zuvor gestellte Architekturfrage wählt der User:

> „C“

Damit ist für Gen Fläche 2 gesammelt:

- Ein Klick auf `DYN · OFF` setzt `dynamic_enabled` für das ausgewählte Band
  auf `true` und öffnet im selben Handgriff die bereits entschiedene
  zweireihige Dynamic-Werteansicht.
- Ein Klick auf `DYN · ON` öffnet beziehungsweise verlässt nur diese Ansicht;
  er schaltet den Klangzustand nicht aus.
- Das Verlassen oder Schließen der Werteansicht lässt Dynamic aktiv.
- Der ausdrückliche Bedienort zum späteren Ausschalten bleibt eine echte
  offene Architekturfrage dieses Blocks.

Technisch bleiben `dynamic_enabled` und die fünf Werteparameter getrennte
persistente Parameter. Ein späteres Ausschalten darf `dynamic_range_db`,
`threshold_db`, `attack_ms`, `hold_ms` und `release_ms` daher nicht still auf
Defaults zurücksetzen.

Vor Eingang der Taktänderung war dieser erste Übergang in der laufenden Skizze
bereits begonnen und an einem zuvor inaktiven Band geprüft worden. Der
kohärente Fünfer-Umbau samt vollständigem Sichtbeleg und Repo-Integration bleibt
trotzdem bis 5/5 zurückgestellt.
