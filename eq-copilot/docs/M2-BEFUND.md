# M2-Befund — Multi-Instanz und Projektbindung, headless bewiesen

Stand 2026-08-14, Anschluss an M1.1 (Live-Abnahme „passt so"). Prüfmaßstab:
Plan §11 M2 + §5.7 + §8.4. Alles hier Genannte wurde **ausgeführt und gesehen**.

## Was gebaut wurde

### Protokoll v2 (`eq-ipc.schema.json`, Plugin 0.2.0)

- **hello v2**: `instance_nonce` — flüchtige Verbindungs-ID pro Plugin-Instanz
  (Plan §8.2). Unterscheidet zwei lebende Verbindungen derselben persistenten
  Sensor-ID nach FL-Duplikation.
- **heartbeat v2** trägt den **kompakten Messstand** (`measurement`): Zustand,
  LUFS I/S, True Peak, Crest, Centroid/low_frac/width/corr, LTAS-Komposit
  (221 Bänder, auf 0,1 dB gerundet — volle Auflösung bleibt der lokalen
  Snapshot-Datei), **Projektzeit-Fenster** {von, bis, Sprünge}.
- **heartbeat_ack** (Broker → Plugin, NEU): quittiert jeden v2-Heartbeat und
  trägt das **Konflikt-Flag**. v1 konnte dem Plugin strukturell nichts melden
  (der Client las nach dem welcome nie wieder).
- **Abwärtskompatibilität**: der Broker nimmt v1-hello weiter an; v1-Sensoren
  erscheinen als „altes Plugin (v1) — ohne Messdaten". Ein v2-Plugin gegen
  einen alten v1-Broker bekommt ein klares reject (Plan §9.2).

### Projektzeit-Fenster (Plugin, Audiothread)

Nur Atomics, nur während `isPlaying`: min/max von `projectTimeSamples`,
Sprungzähler bei > 64 Samples Abweichung vom erwarteten Fortschritt (Loop,
Seek, Stop-Rücksprung — auch über Pausen hinweg). Reset zusammen mit
„Neu messen" und bei Samplerate-Wechsel (das Fenster BESCHREIBT die
akkumulierte Messung — alte Sample-Achse neben frischer Messung wäre gelogen;
Selbst-Audit-Fund). `spruenge > 0` heißt ehrlich: [von, bis] enthält Lücken
oder Wiederholungen.

### Broker (Rust) — Register-Umbau

- **Konflikt über lebende Nonces** statt Einzel-Flag: `lebende > 1` ⇒
  Konflikt; das jüngste hello besitzt die Schreibrechte auf stats/messung
  (kein Flackern zweier Instanzen); stirbt der Besitzer, erbt die überlebende
  Verbindung. v1-Verbindungen bekommen server-vergebene Nonces — eine Logik
  für beide Welten.
- **stale**: verbunden, aber > 5 s ohne Heartbeat — sichtbar markiert, nie
  entfernt (M2-Abnahme „stale/fehlende Sensoren verschwinden nicht still").
- **Sessions**: Gruppierung nach `host_pid` — mehrere FL-Prozesse bleiben in
  Übersicht und Aggregat getrennt (Plan §8.4).
- **PRE/POST-Paare** (Plan §5.7, Herabstufung bei JEDER Unsicherheit):
  `ausgerichtet` nur wenn beide messbereit + sprungfreie Fenster + ≥ 80 %
  Deckung des kürzeren + gleiche PID + ähnliche aktive Messzeit;
  `wahrscheinlich` bei Sprüngen/Teilüberdeckung/verschiedenen Prozessen
  (Bridge?); sonst `unklar` mit benanntem Grund. **`ausgerichtet` ist eine
  Fenster-Aussage, keine samplegenaue Kausalität — PDC-Schätzung existiert in
  M2 nicht.**
- **Profilbindung**: sensor_id → stabile `profile_id` der App (F3.1-Zusage),
  persistiert in `eq-copilot-bindungen.json` (atomisch, tmp+rename;
  beschädigte Datei = sichtbarer Fehler), überlebt App-Neustart und Reconnect.
- **Aggregat-Snapshot** (`eq-aggregat.schema.json`): Momentaufnahme aller
  (nach Profil/PID gefilterten) Sensoren → eine Datei neben den
  Plugin-Snapshots. Gemeinsames Fenster = SCHNITT der Projektzeit-Fenster;
  jede Einschränkung (v1-Sensor, fehlendes Fenster, Sprünge, getrennt/stale,
  Konflikt, kein Schnitt) steht benannt in `warnungen`.

### Konflikt-UI (sichtbare Entscheidung, Plan §8.4)

Plugin-Editor: roter Knopf „Kennung doppelt!" erscheint, solange der Broker
den Konflikt per ACK meldet. Popover in Lernsprache erklärt die Ursache
(FL-Duplikation) und bietet **„Dieser Instanz eine neue Kennung geben"** —
neue UUID + frisches hello; die andere Instanz behält ihre. Nichts wird still
neu gebunden.

### Sensorübersicht in der Hub-App

Toolbar-Statusknopf (5-s-Poll, Zähler + rotes ⚠ bei stale/Konflikt) → Modal
(1-s-Poll): Messpunkte je FL-Prozess, Rollen im Plugin-Vokabular („Misst
diese Spur" …), Zustand in Lernsprache („hört zu …", „sendet nicht mehr",
„altes Plugin (v1)"), LUFS/Spitze/Musikzeit, Sprung-Warnung, Profil-Dropdown
je Messpunkt, Paar-Ampel mit Grund, „Gemeinsamen Snapshot festhalten".

## M2-Fund: Pipe-Namens-Squatting (behoben)

Die Ende-zu-Ende-Probe fand den Konflikt zunächst „nie" — weil **zwei Broker
auf demselben Pipenamen lauschten** (die laufende Hub-App + der
Probe-Broker): Windows verteilt Clients dann zufällig, jeder Broker sah nur
einen der beiden Duplikat-Sensoren. Dasselbe passierte im Alltag bei
Zombie-App + neuer App — **still**. Härtung: die erste Pipe-Instanz trägt
jetzt `FILE_FLAG_FIRST_PIPE_INSTANCE`; ein zweiter Broker scheitert beim
Start mit klarem Fehler („Pipe schon belegt — läuft bereits ein Broker?").
Die Probe läuft seitdem auf einem eigenen Namen (`…eq-copilot.m2probe`,
`probe_lauf()` ohne globalen Zustand).

## Headless bewiesen

| Prüfung | Werkzeug | Ergebnis |
|---|---|---|
| v1-hello angenommen (Kompat), v2 mit Nonce, Version 3 → reject 1..2 | `cargo test eq_copilot` | ok |
| 16 Sensoren korrekt benannt (Registertest) · 2 host_pids = getrennte Sessions · stale sichtbar, nie entfernt · Konflikt kommt und geht mit der zweiten Verbindung, Besitzer-Erbe · Profilbindung überlebt Reconnect | `cargo test eq_copilot` | ok |
| Paar-Herabstufung: deckungsgleich → ausgerichtet · Sprünge/Teilüberdeckung/2 Prozesse → wahrscheinlich · ohne Überlappung/v1-Partner/ohne Partner → unklar mit Grund | `cargo test eq_copilot` | ok |
| heartbeat_ack Ende-zu-Ende (echte Pipe): ACK trägt seq + Konflikt; feindliches 600er-LTAS gezählt verworfen, Verbindung lebt; zweiter Broker auf belegtem Namen scheitert sichtbar | `cargo test eq_copilot` | **35/35 ok** |
| Bindungs-Persistenz: fehlend = leer, rund, beschädigt/falsches Schema = benannter Fehler | `cargo test eq_copilot` | ok |
| Aggregat: Schnittfenster korrekt, Profil-/PID-Filter lässt Fremde nie hinein, v1-Warnung, Datei geschrieben | `cargo test eq_copilot` | ok |
| Passthrough bitgleich · Latenz 0 · NaN gezählt · State stabil (mit v2-Client) | `EqCopNullTest` | **NULLTEST OK** |
| Kreuzvalidierung 88 Prüfungen unverändert (Engine unberührt) | `EqCopGoldenTest` | **GOLDEN OK 88/88** |
| VST3-Lebenszyklus, Editor, Bus-Layouts, State-Recall, Parameter-Fuzz | `pluginval --strictness-level 8` | **SUCCESS** |
| Ende-zu-Ende C++↔Rust (eigener Pipename): v2-welcome + measurement-Heartbeats + ACKs, dann Duplikat → **beide** Instanzen sehen das Flag, nach Trennung fällt es | `eqcop-broker-probe` + `EqCopPipeProbe` | **PROBE OK v2 · KONFLIKT OK · KONFLIKT-ENDE OK** |
| Frontend | `svelte-check` + `npm run build` | 0 Fehler / grün |
| Voller Rust-Bestand | `cargo test` | 732 grün; 2 rote sind fremde Baustellen (`scans_real_db`-Vorbefund · Scan-Helfer-Kindprozess stirbt unter paralleler Harness-Last — kein Berührungspunkt mit dem M2-Diff) |

## Bekannte Grenzen (ehrlich, nicht heimlich)

- **PDC/Zeitversatz wird nicht geschätzt** — `ausgerichtet` ist die höchste
  M2-Stufe und eine Fenster-Aussage. Samplegenaue PRE/POST-Ausrichtung
  (Korrelation) ist M3+.
- Das Projektzeit-Fenster ist **[min, max] + Sprungzähler**, keine
  Segmentliste — „welche Takte genau" kommt mit den Passagen-Fenstern (M3).
- Die App-Sensorübersicht zeigt den LTAS **nicht** — Zahlenkarten reichen für
  die Betriebssicht; der Graph lebt im Plugin. Aggregat-Dateien tragen das
  LTAS-Komposit für spätere Auswertung.
- „16 Sensoren benannt" ist ein Registertest; die 16-Instanzen-LAST in FL
  bleibt Alltags-Beobachtungspunkt (M1-Anleitung §M1 gilt weiter).
- Der Toolbar-Zähler pollt alle 5 s — ein Sensor erscheint dort bis zu 5 s
  verzögert (das offene Panel pollt 1 s).

## Nur in FL prüfbar (User)

Installierte Kopie in `Program Files\Common Files\VST3` durch
`eq-copilot/install/EQ-Copilot.vst3` **ersetzen** (UAC) — SHA-256 der DLL:
`566FF29CB1E619652A6318BF3A8289639078B3B7B3ABA978B235A9BE678EF7B8`.
Danach: FL-TESTANLEITUNG §M2 (Duplikat-Konflikt, PRE/POST-Paar,
Sensorübersicht, gemeinsamer Snapshot).
