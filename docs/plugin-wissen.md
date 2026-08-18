# Plugin-Wissen — wie Nakama/EQ-Copilot heute funktioniert

> **Stand: 2026-08-17** (Plugin 0.3.0 installiert · metrics/diagnose
> `m4.1-2026-08-15` · Snapshot v3 · Protokoll v2). Quelle der Wahrheit ist
> der Code unter `C:\Users\phili\FL-Studio\eq-copilot\` — bei Widerspruch
> gewinnt die Datei. Anker sind `Datei:Zeile` zum Stand dieses Dokuments.

## Zehn-Sekunden-Form

| Schicht | Ort | Sprache |
|---|---|---|
| VST3-Plugin | `eq-copilot/plugin/src/` | JUCE 8.0.9 / C++20 |
| Verträge | `eq-copilot/schemas/` | JSON Schema 2020-12 |
| Broker (eigenständig seit 18.08., `eqcop-broker.exe`) | `broker/` | Rust + windows-sys |
| Sensorübersicht (heimatlos, NAK-12 — Referenzkopie) | `broker/sensoruebersicht-referenz/` | Svelte 5 |
| Fixtures | `tools/eq-copilot/erzeuge_fixtures.py` | Python 3.13 + numpy |

Drei Threads im Plugin: **Audiothread** (processBlock) · **Worker** (besitzt
die AnalyseEngine) · **Pipe-Thread** (besitzt den PipeClient) — plus der
JUCE-Message-Thread für den Editor.

## 1 · Audiopfad

`processBlock` (`PluginProcessor.cpp:86-204`), Reihenfolge ist Vertrag:
ScopedNoDenormals → RMS-+NaN-Scan (liest nur, zählt `nanSeen`, verändert
nie) → Projektzeit-Fenster (akkumuliert nur bei `getIsPlaying()`,
Sprungtoleranz 64 Samples, Sprünge werden gezählt) → **Analyse-Abgriff in
den FIFO** → erst DANACH die Hör-Markierung als einziger Buffer-Schreiber.

- Thread-Übergabe: `juce::AbstractFifo` (lock-frei, SPSC) über einen einmal
  allozierten HeapBlock, 65 536 Frames interleaved L/R
  (`PluginProcessor.h:139-141`). **Überlast: Frames werden verworfen und
  gezählt (`framesDropped`), Audio nie blockiert.**
- Passthrough-Garantie: 0 Samples Latenz, kein Tail, Busse nur mono/stereo
  mit in==out (kein stiller Downmix). NullTest beweist Bitgleichheit.
- **Echtzeit-Beweis („Lebenszeichen", `PluginProcessor.cpp:211-272`):**
  Audio-Zeit/Wand-Zeit-Verhältnis; zwei saubere Fenster ⇒ `echtzeitOk`;
  Verhältnis > 1,5 (Offline-Render/Freewheel) löscht den Beweis sofort.

### Die eine Ausnahme: Hör-Markierung (0.3.0)

`HoerMarkierung.h` (header-only; Konzept: `docs/HOER-MARKIERUNG-KONZEPT.md`).
Färbt auf bewussten Klick das **Monitorsignal**, damit man HÖRT, wo der
Befund sitzt — **Solo** (nur das Problemband: Resonanz = 2 kaskadierte
Bandpässe; Zone = Butterworth-HP/LP-Paar) oder **Puls** (Band schwillt im
250/375-ms-Raster). Verriegelung: nur bei bewiesener Echtzeit ∧ Editor
offen ∧ Transport ∧ `!isNonRealtime()` — **Render bleibt bitidentisch**
(MarkierungTest beweist es, inkl. Freewheel-Schnitt). Message-Thread baut
den kompletten Auftrag (POD, alles vorberechnet), Audiothread liest über
einen 4-Slot-Ring; Totmann-Timer 10 min; `Markierung aus`-Knopf immer
sichtbar. Der Analyse-Abgriff sitzt VOR der Färbung (T4: LTAS mit/ohne
Marker gleich).

## 2 · AnalyseEngine — die drei Uhren

`PluginProcessor.cpp:274-326`:

| Takt | Was läuft |
|---|---|
| ~20 Hz (50-ms-Wait) | FIFO leeren → `verarbeite()`; 4 von 5 Ticks `auswertenLeicht()` |
| ~4 Hz (jeder 5. Tick) | `auswerten()` schwer: Gating, LTAS-Komposit, Abdeckung, Resonanzen, Perzentile, Konvergenz |
| je 1 s AKTIVER Zeit | `zonenTick()` in `verarbeite()` — in Pausen vergeht keine Tick-Zeit |
| 1 Hz | Heartbeat mit `messKompakt()` (Pipe-Thread) |
| 30 Hz | Editor-Timer-POLL (nicht die Malrate) |

- **`auswertenLeicht()`** publiziert nur über `fuelleBasis()`: Live-Kurve
  (3-s-EMA), Kurz-LUFS, True Peak, Crest, Zustand, Revision. `fuelleBasis()`
  ist die EINE Quelle — Leicht- und Schwerpfad können nicht divergieren.
  Ohne neue Samples publiziert niemand (Leerlauf-Riegel beidseitig).
- **FFT:** 4 Welch-Stufen (Hann periodic, 50 % Hop): Bass 16384 (<200 Hz) ·
  **Referenz 8192 (ganze Achse — die Kreuzvalidierungs-Achse)** · Mitten
  4096 · Höhen 2048; Blend in log-f über die Nähte. PSD =
  **Kanalenergie-Mittel** (L²+R²)/2, nicht Mid-Mix (Antiphase-Fix).
- **LTAS-Raster:** 1/24-Oktave, 221 Bänder bis <18 kHz; Nyquist-Kappe:
  darüber bleibt die Kurve NaN, nie fortgeschriebene Randwerte.
- **LUFS** BS.1770 (K-Weighting RBJ +4 dB Shelf/1500 Hz + HP 38 Hz;
  100-ms-Zellen; Integriert = 400-ms-Blöcke, Gates −70 abs/−10 rel;
  Short = 3 s). `lufsGueltig=false` ⇒ JSON `null`, nie 0.
- **True Peak:** 8× Polyphase, Kaiser β=5, 161 Taps, ungegatet.
- **Abdeckung:** 1-dB-Pegelhistogramm je Band über aktive Segmente;
  1/3-Okt-Gruppen ⇒ `belastbar` (≥0,60) / `eingeschraenkt` / `nichtMessbar`.
- **Resonanz-Kandidaten:** Prominenz gegen **Median ±6 Bänder (~1/2 Okt)**
  (nie Mittel — der Peak zog es hoch); Exzess ≥6 dB, max 6 Kandidaten;
  Persistenz aus `excessSegmente` derselben Basislinie (≥0,50 dauerhaft,
  ≥0,15 zeitweise); Abdeckungs-Gate = Sinus-Landmine-Filter.
- **Perzentile P10/P50/P95** je Band aus dem vorhandenen Histogramm;
  P95−P50 steuert die Charakter-Zeile („steht ruhig" ↔ „kommt in Wellen").
- **Zonen-Zeitverlauf:** je Tick Live-EMA gegen die Schulterlinie der Zone
  (Geometrie EINMAL in `ZonenRegeln.h`, geteilt mit der Diagnose);
  m4.1-Fix: Mindestpegel −80 dB je Region, sonst zählte Quasi-Stille.
- **Konvergenz „KURVE STEHT":** alle ~10 s Aktivzeit Komposit gegen den
  Stand von vor einem Fenster; UI zeigt es bei mean <0,35 dB. Rein
  informativ — fließt bewusst NICHT in die Konfidenz.
- **NaN-Riegel:** Engine ersetzt nicht-endliche Samples VOR jeder Arithmetik
  durch Stille und zählt `nanErsetzt`; Audiothread zählt nur.
- **Snapshot-Übergabe:** `MessSnapshot` doppelt gepuffert unter `snapMutex`
  (Worker↔Editor, nie Audiothread); `revision` = monotoner Zähler über
  beide Pfade, übersteht `zuruecksetzen()` absichtlich.

## 3 · Editor (Material-Kit-Front, wird durch Spectral Field ersetzt)

`PluginEditor.cpp` + `EqCopilotAssetKit.h` (`skin::`) + `LeitstandTokens.h`
(generiert aus `design/tokens.json`; die Front nutzt die `copilot_*`-Gruppe —
die `nakama_*`-Tokens sind Altbestand einer früheren dunklen Front,
aktuell unreferenziert).

- Raster 750×520 Einheiten, Standard 1200×832, Resize 600×416…1950×1352
  mit **fester Ratio** (fällt im Spectral-Field-Umbau).
- Auf der Bühne: gecachte Gerätefront · Marken-Header · zwei Meter-Schienen
  (LUFS-S links, True Peak rechts) · Analyseglas 604×252 · Mikroleiste
  (LTAS/Ansicht/Glättung/„KURVE STEHT") · log-f- + dB-Raster ·
  Abdeckungs-Zonenstreifen · Hör-Markierungs-Tönung (pulsphasensynchron) ·
  drei Kurven (live/Vergleich/Haupt) · Resonanz-Dreiecke ·
  7 Statuszellen · Werkzeugtasten (Glättung, Ansicht natürlich↔begradigt,
  Festhalten, Vergleich, Messpunkt, „n Auffälligkeiten", Neu messen,
  „Kennung doppelt!" nur bei Konflikt, „Markierung aus" nur bei Latch).
- **Repaint-Disziplin (der M3a-FPS-Fix):** 30-Hz-Timer ist Poll, kein
  Maltakt. EIN Snapshot-Zug pro Tick in die Anzeige-Kopie; `repaint()` nur
  bei neuer Revision / UI-Änderung / Pipe-Wechsel / Meldungsfenster;
  `paint()` liest nur die Timer-Kopie. **Im Leerlauf malt der Editor exakt
  nichts.** paint() selbst kostete avg 2,42 ms @1200×832 — war nie der
  Engpass; der wahrgenommene Ruckel war die 4-Hz-Datenkadenz.
- Diagnose-Karten: `BefundListe` im CallOutBox (Titel · Gemessen · Wirkung ·
  Tu · Warum · Hören · Sicherheit + [Solo]/[Puls]; Loch kann nicht pulsen);
  Diagnose rechnet 1×/s auf der Anzeige-Kopie. `Component::SafePointer`
  überall, wo FL den Editor mit offenem Popup schließen darf.
- **`EqCopShot.exe <ziel.png> [breite]`:** echter Processor + echte
  20-s-Messung offscreen → PNG. Headless-Lehre: JUCE-8-Editor-Timer in
  Konsolen-Tests brauchen `juce::Timer::callPendingTimersSynchronously()`.

## 4 · IPC / Broker (Protokoll v2)

- **Pipe:** `\\.\pipe\evenacadia.eq-copilot.v1` — der Name bleibt „v1",
  die Protokollversion wird im Handshake verhandelt (`EqCopilotIds.h:18`,
  `mod.rs:23-26`). Framing: u32-LE-Länge + UTF-8-JSON, max 262 144 Bytes.
- **Handshake:** Client sendet immer zuerst `hello` (protocol_version,
  plugin_version, host_pid, sensor{id, nonce, role, label, pair_id},
  audio{sr, block, ch}) → Broker `welcome` (akzeptierte Version 1|2) oder
  `reject`. v1-welcome ⇒ nie blockierend lesen (alter Broker antwortet
  nichts).
- **Heartbeat 1 Hz** trägt stats + in v2 den **Messstand** (`messKompakt()`):
  Zustand, aktiv/gesamt s, LUFS I/S, True Peak, Crest, Centroid, low_frac,
  width, corr — **`null` heißt „nicht messbar", nie 0** — plus
  `ltas_komposit_db` (221 Werte, auf 0,1 dB gerundet; volle Auflösung nur
  in der lokalen Snapshot-Datei), Projektfenster, `hoermarkierung`-Flag.
- **`heartbeat_ack` mit Konflikt-Flag:** zweite lebende Verbindung mit
  gleicher `sensor_id` (FL-Duplikation) ⇒ Editor zeigt „Kennung doppelt!";
  der USER löst per neuer Kennung (frische UUID + Reconnect). Der Client
  MUSS den Ack lesen, sonst kippt die Verbindung nach Heartbeat 1.
- **Register:** nur die JÜNGSTE hello-Nonce darf Stats schreiben
  (kein Flackern zweier Instanzen); `stale` nach 5 s ohne Heartbeat;
  Getrennte bleiben mit Zeitstempel sichtbar, nie still entfernt.
- **PRE/POST-Paare mit ehrlicher Herabstufung:** harte Ausschlüsse ⇒
  `unklar`; Loop/Seek-Sprünge, Überlappung <80 %, fremde host_pid,
  Aktivzeit-Differenz >10 % ⇒ `wahrscheinlich`; nur sauber ⇒
  `ausgerichtet`. `grund` ist nie leer.
- **Aggregat:** Broker friert den letzten gemeldeten Stand ein (misst
  selbst nichts); gemeinsames Fenster = Schnittmenge; atomisch nach
  `%LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\aggregat-<ms>.json`.
- **Härtung:** erste Pipe-Instanz mit `FILE_FLAG_FIRST_PIPE_INSTANCE` —
  fremder Besitzer ⇒ Broker verweigert Start (zwei Broker auf einem Namen
  stahlen sich still Clients). SDDL nur aktueller User, keine Remote-
  Clients. Ungültige Pakete werden gezählt und verworfen, nie gekürzt.
- **Probe:** `eqcop-broker-probe.exe [sekunden] [pipe-name]`, Default
  `…m2probe` — nie der Produktionsname.
- **Betrieb seit 18.08.2026:** eigenständiger Prozess
  `broker\target\release\eqcop-broker.exe [--bindungen <pfad>]`
  (Standard: `%APPDATA%\evenacadia\nakama\eq-copilot-bindungen.json`).
  Die Hub-App startet und kennt ihn nicht mehr; ihre früheren Kommandos
  (`eq_copilot_status/profil_binden/aggregat_schreiben`) sind entfernt —
  `broker_status()`/`profil_binden()`/`aggregat_schreiben()` sind
  Bibliotheksfunktionen der Crate ohne UI (NAK-12); kein Autostart (NAK-13).

## 5 · Snapshots & Diagnose

- **eq-snapshot v3** (`schreibeSnapshotDatei()`,
  `PluginProcessor.cpp:452-660`): Versionen, Sensor, Zustand,
  `nan_ersetzt_samples`, Loudness, Spektral, Stereo, LTAS (Zentren +
  Komposit + 8192er-Referenz), Abdeckung, **v3: Perzentile + Zonen-Zeit +
  Konvergenz**, Resonanzen, **Befunde** (v2), `raw_audio: null` per
  Vertrag. NaN/±inf ⇒ JSON `null`. Ablage
  `%LOCALAPPDATA%\evenacadia\EQ-Copilot\snapshots\snapshot-….json`.
- **Heute gibt es KEIN Befundarchiv im Plugin** — `Festhalten` schreibt die
  v3-Datei UND friert die Vergleichskurve ein; das Archiv ist reines
  Bauplan-Ziel (§8) für die Spectral-Field-Front.
- **Diagnose** (`Diagnose.cpp`, pur/zustandslos auf der Snapshot-KOPIE;
  dieselbe Funktion speist Hinweis-Knopf, Snapshot-Datei und GoldenTest):
  5 Klassen — Resonanz (2 stärkste Kandidaten) · Mitten-Loch (500–2000
  ≥3 dB UNTER der Schulterlinie; erste Idee: Balance vor EQ) · Mulm
  (120–300 ≥4 dB drüber) · Härte (2,5k–5k ≥4 dB drüber; dynamisch zuerst)
  · Höhen-Hype (8k–14k > 2k–6k +1 dB). Alles **eigenkurven-relativ**
  (Schulterlinien) — kein kalibrierter Zielkorridor. Zwei-Kriterien-Test
  gegen Einzelpeak-Verzerrung; Konfidenz aus 4 Komponenten, die
  limitierende wird BENANNT; Zonen-Persistenz seit M3a aus dem echten
  Zeitverlauf (Zonen können ehrlich „hoch"; zitiere M3A-BEFUND, nicht
  M3-KERN-BEFUND — letzterer ist überholt). Notennamen in FL-Zählung
  (116 Hz = A#3). m4.1: welliger Pegel (>10 dB) überstimmt Persistenz bei
  der Werkzeugwahl.

## 6 · Beweise (was jedes Werkzeug beweist)

| Werkzeug | Beweis | Letzter Stand |
|---|---|---|
| `EqCopGoldenTest <fixtures>` | Kreuzvalidierung gegen eingefrorene Offline-Referenz + Diagnose-Struktur + 8 adversariale Fälle (Antiphase, nur-L/R, NaN-Recovery, Nyquist-32k, Leichtpfad, Konvergenz). SHA-Riegel auf den WAVs; Pink ⇒ 0 Karten, 1-kHz-Sinus ⇒ genau 1 | **GOLDEN OK 239/239** |
| `EqCopNullTest` | Bitgleicher Passthrough, 0 Latenz/Tail, NaN gezählt-nicht-gefiltert, Bus-Layouts, State-Roundtrip | **NULLTEST OK** |
| `EqCopMarkierungTest` | Hör-Markierung T2–T10: klickfrei, Rest bitidentisch, Puls-Rest ≤−120 dBFS, Freewheel/Render bleibt bitidentisch, Analyse-Abgriff vor Färbung | **30/30** |
| `EqCopShot` | Sichtprüfung ohne FL (echte Messung) | SHOT OK |
| `EqCopPaintBench [breite] [frames]` | isolierte paint()-Kosten | 2,42 ms @1200×832 |
| `EqCopPipeProbe "<pipe>"` | v2-Handshake + Konflikt-Roundtrip | PROBE OK v2 |
| `pluginval --strictness-level 8` | VST3-Lebenszyklus, Editor, Fuzz | SUCCESS |
| `cargo test … eq_copilot` | Register/Protokoll/Paare/Aggregat | grün |

Fixtures: `py -3.13 tools/eq-copilot/erzeuge_fixtures.py` — deterministisch
(feste Seeds/Phasen, 20 s · 48 kHz · L==R, weil die Offline-Referenz den
Mid-Mix misst und nur bei L==R beide Mathematiken identisch sind); WAVs
nicht committet, nur `golden-referenz.json` (SHAs). Stereo-Korrektheit
beweisen die adversarialen Fälle im GoldenTest.

## 7 · Bekannte Klarstellungen (Code vs. Doku)

- `eq-measurement.schema.json` + `eq-report.schema.json` (je v1) sind
  **geplante** M0-Verträge — von keinem Code referenziert. Live sind
  `eq-ipc` v2, `eq-snapshot` v3, `eq-aggregat` v1.
- Das `hoermarkierung`-Flag wird vom Plugin gesendet, aber der Rust-Broker
  liest es noch nicht (serde verwirft es still) — bewusst offen, siehe
  `docs/offene-punkte.md` NAK-10.
- Die `nakama_*`-Farbtokens in `LeitstandTokens.h` sind Altbestand; die
  Material-Kit-Front nutzt `copilot_*`.
