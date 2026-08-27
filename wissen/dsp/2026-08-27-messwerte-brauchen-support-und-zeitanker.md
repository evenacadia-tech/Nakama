---
id: w-2026-08-27-messwerte-brauchen-support-und-zeitanker
typ: wissenseintrag
status: aktiv
datum: 2026-08-27
aktualisiert: 2026-08-27
hauptbereich: dsp
weitere_bereiche: [engineering, fl-studio]
pruefstand: mehrfach-belegt
---

# Messwerte brauchen Supportintervall und Zeitanker

## Menschliches Warum

Ein Produzent verlässt sich darauf, dass zwei angezeigte Messwerte wirklich
denselben musikalischen Moment beschreiben. Wenn eine Kurve ältere Samples,
eine Zahl drei Sekunden und ein Hostzustand nur den letzten Block umfasst, kann
eine scheinbar präzise Differenz eine falsche Ursache nahelegen.

Die Kausalkette lautet:

> Weil ein Produzent aus Messunterschieden eine Hör- und EQ-Entscheidung
> ableitet, muss jeder Wert offenlegen, welche Samples und welche Zeitbasis ihn
> tragen, damit Nakama nur vergleichbare Evidenz gegenüberstellt.

## Belegtes Wissen

Welchs klassische Spektralschätzung zerlegt ein Signal in möglicherweise
überlappende Segmente, fenstert deren Daten und mittelt die modifizierten
Periodogramme. Ein Spektralwert repräsentiert damit nicht nur den Block, in dem
er veröffentlicht wird, sondern den vollständigen Support seiner Fenster und
Überlappungen.

Loudness besitzt andere Horizonte. Der EBU-Mode definiert Momentary über 400 ms,
Short-term über 3 s und Integrated vom Start bis zum Stop. Mehrere Kennzahlen in
einem Frame können deshalb korrekt sein und trotzdem verschiedene
Supportintervalle besitzen.

VST3s `ProcessContext` beschreibt `projectTimeSamples` für den aktuellen
Verarbeitungskontext als gültig; kontinuierliche Projektzeit und weitere
Angaben sind optional und über Zustandsflags gekennzeichnet. Daraus folgt nicht
automatisch, dass ein Snapshot des letzten Hostblocks rückwirkend für das
früheste Sample eines langen FFT- oder Loudnessfensters gilt.

## Fehlerlexikon und Gegenbeweis

| Fehlerquelle | Falsche Aussage | Erforderlicher Gegenbeweis |
|---|---|---|
| Triggerblock als Messbeginn | FFT/Loudness scheint jünger als ihre Samples | frühester tatsächlicher Support je Kennzahl |
| ein `sample_count` für alle Metriken | verschiedene Fenster wirken deckungsgleich | Support oder Provenienz pro Metrikfamilie |
| Projektzeit aus lokalem Strom extrapolieren | Loop, Seek oder Drop wird als Kontinuität verkauft | nur nachgewiesene zusammenhängende Hostintervalle |
| fehlende Daten als Null | "kein Beleg" wird zu "kein Problem" | eigener missing/stale/invalid-Zustand |
| Hostsnapshot vom Seal-Block | Aufnahme-, Loop- oder Latenzzustand wird zu früh datiert | eigener Snapshotanker oder enger Gültigkeitsbereich |
| Reset löscht nur einen Akkumulator | Overlap, Filter oder fertiger Altframe überlebt | Generation schneidet Queue, Engines und Ausgabe gemeinsam |
| Drop erweitert Projektfenster | nicht gemessene Samples zählen als Evidenz | nur angenommene Blöcke erweitern Support |
| zwei Sonden ohne gemeinsame Zeitabbildung | Baseline und Kandidat wirken synchron | kompatible Zeitbasis, Coverage und Frische beweisen |
| Preview taintet Baseline/Kandidat | Eigenwirkung wird als Systemursache erkannt | aktive Revision und Messgeneration in Provenienz |
| Int64-Arithmetik an Projektgrenzen | Wrap oder undefiniertes Verhalten | Grenzfixtures für beide Vorzeichen und checked arithmetic |

## Bedeutung für Nakama

Die Debugging-Session reparierte genau solche Wahrheitsfehler: FFT-Ereignisse
trugen zunächst die Zeit des auslösenden Hostblocks statt des Fensteranfangs;
Frames zählten nur den Triggerblock statt aller integrierten Samples;
Projektintervalle wurden ohne bewiesene Kontinuität und sogar über verworfene
Queue-Blöcke erweitert. Resets schnitten alte Queuedaten nicht vollständig ab.
Die Fixes führen Supportbeginn, tatsächliche Vereinigung angenommener Samples,
Generation und fail-closed Projektzeit zusammen.

Für die geplanten Vergleichs-, Ursachen- und Vorschlagsphasen ist der nächste
Fehlerraum größer: FFT mit 4 096 beziehungsweise 16 384 Samples, skalare
100-ms-Messung und 3-s-Short-term-Loudness dürfen nicht durch einen gemeinsamen
Frameanker zu scheinbar gleicher Evidenz werden. Vor einem Cross-Probe-Vergleich
müssen Zeitabbildung, Coverage, Frische und aktive Revision kompatibel sein.
Fehlende Evidenz bleibt unbekannt und wird nicht zu Null.

Die aktuelle offene Vertragsfrage NAK-83 gehört genau hierher: Ein Frame kann
früheren Messsupport enthalten als der optionale Recording-, Cycle- oder
Presentation-Latency-Snapshot. Vor dem ersten Verbraucher muss entweder der
Snapshot einen eigenen Anker erhalten oder seine Aussage ausdrücklich auf den
Seal-Block begrenzt werden.

## Grenzen, Widersprüche und offene Fragen

Welch, EBU und VST3 definieren jeweils einen Teil des Problems; keiner legt das
Nakama-Frameformat fest. Die gemeinsame Provenienzregel ist daher eine aus den
Quellen und den gefundenen Fehlern abgeleitete Nakama-Hypothese. Konkrete
Feldnamen und Fenster bleiben dem technischen Vertrag vorbehalten.

## Quellen

- Titel: *The Use of Fast Fourier Transform for the Estimation of Power Spectra: A Method Based on Time Averaging Over Short, Modified Periodograms*
- Autor: Peter D. Welch
- Jahr: 1967
- DOI: https://doi.org/10.1109/TAU.1967.1161901
- Titel: *EBU Mode Metering to supplement Loudness normalisation in accordance with EBU R 128*
- Autor: European Broadcasting Union
- URL: https://tech.ebu.ch/publications/tech3341
- Überblick: https://tech.ebu.ch/loudness/
- Titel: *VST 3 ProcessContext Struct Reference*
- Autor: Steinberg Media Technologies
- URL: https://steinbergmedia.github.io/vst3_doc/vstinterfaces/structSteinberg_1_1Vst_1_1ProcessContext.html
- Eingang: `web-recherche`
- geprüft: Originalpaper/DOI, offizielle EBU- und Steinberg-Referenz sowie
  Nakamas aktuelle Messkern-Implementierung und Debugging-Beweise

## Verknüpfungen

- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/02-realtime-messkern.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/02-realtime-messkern.md)
- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/05-adversarial-integration.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/05-adversarial-integration.md)
- [`../../docs/FL-Nakama-Sonden-Design-Entwurf.md`](../../docs/FL-Nakama-Sonden-Design-Entwurf.md)
- [`../engineering/2026-08-27-realtime-worst-case-statt-durchschnitt.md`](../engineering/2026-08-27-realtime-worst-case-statt-durchschnitt.md)
