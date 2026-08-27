---
id: w-2026-08-27-realtime-worst-case-statt-durchschnitt
typ: wissenseintrag
status: aktiv
datum: 2026-08-27
aktualisiert: 2026-08-27
hauptbereich: engineering
weitere_bereiche: [dsp, fl-studio]
pruefstand: mehrfach-belegt
---

# Realtime-Qualität misst den seltenen Spike

## Menschliches Warum

Eine Audiosession klingt nicht zu 99,9 Prozent störungsfrei. Ein einziger
Deadline-Verstoß kann als Klick hörbar sein, die Aufnahme entwerten oder das
Vertrauen in einen aktiven Eingriff zerstören. Durchschnittlich schnelle
Verarbeitung ist deshalb nicht dasselbe wie verlässliche Echtzeit.

Die Kausalkette lautet:

> Weil ein Produzent auch in einer langen, belasteten Session keinen einzelnen
> hörbaren Aussetzer akzeptieren kann, muss Nakama seltene Laufzeitspitzen und
> Übergangszustände messen, damit ein guter Mittelwert kein reales Glitchrisiko
> verdeckt.

## Belegtes Wissen

Christian Luthers ADC-Vortrag *Mind the Spike* betrachtet ausdrücklich die
Worst-Case Execution Time. Cache-Misses, Pipeline-Stalls, Thread-Contention und
OS-Jitter erzeugen Ausreißer, die konventionelle Mittelwerte und kurze
Benchmarks kaum erfassen. Der Foliensatz zeigt, warum Standardabweichung und
beobachtetes Maximum bei fetten Verteilungsschwänzen fast nutzlos sein können,
und nutzt Zipf-Plot, Tail-Index und Extreme Value Theory als Diagnose für sehr
seltene Spitzen.

Der Vortrag nennt außerdem Quellen, die auf dem Audiopfad keine harte obere
Grenze besitzen: Präemption, Speicherallokation, I/O, Mutexe, Systemaufrufe und
Spinlocks. Die Statistik ersetzt keinen Echtzeitvertrag; sie macht verborgene
Tail-Probleme sichtbar und bewertet das gemessene System unter den getesteten
Bedingungen.

## Fehlerlexikon und Gegenbeweis

| Fehlerquelle | Warum Mittelwert/Maximum täuscht | Erforderlicher Gegenbeweis |
|---|---|---|
| nur Mittelwert, Standardabweichung, p95 | ein seltener fetter Tail bleibt unsichtbar | vollständige Verteilung, hoher Quantilbereich, Zipf/Tail-Diagnose |
| nur stationärer DSP-Test | Banktausch, Bypass oder Reset ist nicht enthalten | jede Zustandskante als eigene Lastklasse |
| warmes Cacheprofil | erster Block, neue Instanz und kalter Codepfad fehlen | Cold-start- und Multi-Instance-Läufe |
| nur Nominalblockgröße | kleine Blöcke verkürzen die absolute Deadline | Matrix aus Rate, Blockgröße und Instanzzahl |
| Denormal-Tail | Stille ist teurer als Signal, aber selten im Test | lange Abkling- und Nullsignalstrecke |
| Off-thread-Arbeit als automatisch sicher | Queuefüllung oder Worker-Starvation verfälscht Wahrheit | Sättigung, Drop, Reset und Stop gleichzeitig |
| Cadence aus Arbeitsmenge | schnellere CPU ändert Zeitsemantik; Pause erzeugt Catch-up-Sturm | monotone Deadlines mit Suspend-/Debugger-Sprung |
| Messinstrument beeinflusst Laufzeit | Profiler verändert Scheduling und Cache | leichte Telemetrie gegen externe/zweite Messung |
| Tail-Modell als Garantie | unbekannte unbeschränkte Quelle bleibt möglich | Codevertrag plus Kill-/Stress-/Hosttest, nicht Statistik allein |

Für Nakamas aktive Phasen müssen insbesondere Bank-Swap, Automation mit
Sample-Offsets, Sidechain-Verlust, Revert, erster Block nach `prepare`,
Oversize-Block, Denormal-Tail und mehrere Instanzen getrennt gemessen werden.
Ein durchschnittlich günstiger Pfad darf einen hörbaren Übergang nicht
verstecken.

## Bedeutung für Nakama

Die Debugging-Session fand mehrere Beispiele, in denen nicht der normale DSP,
sondern die seltene Kante falsch war: Worker-Drain konnte Reset und UI
verhungern lassen, arbeitsmengenbasierte Schleifen machten die Kadenz
CPU-abhängig, ungültige Sample-Rate ließ alten Zustand aktiv und Queue-Drops
erweiterten den behaupteten Messbereich. Repariert wurden begrenzte Batches,
faire Übergabe, `steady_clock`-Deadlines ohne Catch-up-Sturm und fail-closed
Generations-/Drop-Semantik.

Für SONDE-015 bis SONDE-017 folgt daraus: Der WCET-Gegenbeweis gehört an die
vollständige aktive Transaktion, nicht nur an die Filterroutine. Steuerung,
Bankwechsel, Lease, ACK/Retry und UI-Telemetrie müssen den Audiopfad unter
Sättigung unverändert lassen. Die im Bauplan genannten p95-Werte bleiben
nützliche Betriebsmetriken, sind aber kein alleiniger Realtime-Beweis.

## Grenzen, Widersprüche und offene Fragen

EVT kann nur die erhobenen Daten und Modellannahmen bewerten. Der Vortrag selbst
warnt vor manueller Aufsicht bei der Tail-Auswahl. Der vollständige Text des
Original-Foliensatzes wurde geprüft; eine visuelle Renderprüfung war in dieser
Session technisch nicht verfügbar, daher werden keine Diagrammformen oder
optischen Aussagen daraus übernommen.

## Quellen

- Titel: *Mind the Spike – Benchmarking for Worst-Case Execution Time in Realtime Code*
- Autor: Christian Luther
- Jahr: 2025
- Veranstalter: Audio Developer Conference
- Session: https://conference.audio.dev/session/2025/mind-the-spike/
- Originalfolien: https://audiodeveloperconference.b-cdn.net/wp-content/uploads/2025/07/Christian-Luther-ADC25-Christian-Luther-Mind-The-Spike.pptx
- Video: https://www.youtube.com/watch?v=7RrOjl996WQ
- Eingang: `web-recherche`
- geprüft: ADC-Sessionbeschreibung und vollständiger Text aller 20
  Originalfolien; kein visueller Folienrender

## Verknüpfungen

- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/02-realtime-messkern.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/02-realtime-messkern.md)
- [`../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/07-worker-cadence.md`](../../.workflow/ultracode/20260827-001038-sondenplan-debug/packets/07-worker-cadence.md)
- [`../../docs/FL-Nakama-Sonden-Design-Entwurf.md`](../../docs/FL-Nakama-Sonden-Design-Entwurf.md)
- [`2026-08-26-audiodevcon-videoauswahl.md`](2026-08-26-audiodevcon-videoauswahl.md)
- [`../dsp/2026-08-27-messwerte-brauchen-support-und-zeitanker.md`](../dsp/2026-08-27-messwerte-brauchen-support-und-zeitanker.md)
