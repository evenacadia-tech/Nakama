> **ARCHIV (21.08.2026). Technik-/UX-Übergabe 14.08. (Hub-App, Tauri, FL-Pfade). Keine Vorgabe mehr. Gültige Produktdefinition: CLAUDE.md (Wahrheitskern + Entscheide-Register); technischer Entwurf: docs/FL-Nakama-Sonden-Design-Entwurf.md.**

# Claude-Übergabe — EQ-Copilot Technik-, DSP- und UX-Review

> **Vorranghinweis 2026-08-16:** Der sichtbare Produktname ist jetzt
> **Nakama**. Die technischen Sicherheits-, DSP-, Pipe- und Testbefunde dieses
> Dokuments bleiben relevant. Das alte **Arbeitspaket G — JUCE-Darstellung und
> UX** ist für die sichtbare Hauptansicht jedoch durch
> `NAKAMA-SPECTRAL-FIELD-BAUPLAN.md` ersetzt: bildschirmfüllender Materialgraph,
> textfreie Werkzeugkreise, überlagerbare Problemsymbole, umschaltbare
> Farbpakete und ein manuell geöffnetes Befundarchiv.

> **Umzugshinweis 2026-08-18:** Der Workspace ist jetzt
> `C:\Users\phili\Projekte\Nakama`; der Broker ist eine EIGENSTÄNDIGE
> Crate in `broker/` (aus `plugin-hub-app` herausgelöst). Absolute
> `C:\Users\phili\FL-Studio\…`-Pfade im Dokument sind historisch —
> die gültigen Prüfbefehle stehen in §17 (aktualisiert).

**Stand:** 2026-08-14
**Workspace (historisch, s. Umzugshinweis):** `C:\Users\phili\FL-Studio`
**Betroffener Scope:** `eq-copilot` sowie der Broker (damals in `plugin-hub-app`, heute `broker/`)
**Art der Übergabe:** ausführlicher Review mit priorisiertem technischen Bauplan; in dieser Review-Session wurden keine Produktdateien geändert

> **Umsetzungsstand (fortgeschrieben 2026-08-14, gleicher Tag):**
> **Änderungssatz 1 ist GEBAUT und bewiesen** — Commits `41e9424` (P0 Rust-FFI:
> Alignment, RAII, SAFETY, GetLastError, Spawn-Leak) und `dca9a40` (metrics
> `m3-2026-08-14`: Kanalenergie-Spektrum statt Mid-Mix, NaN-Riegel mit Zähler,
> Nyquist-Kappe; Fixtures v2 L==R + 6 adversariale Fälle). Beweise: NULLTEST OK
> · **GOLDEN OK 178/178** · pluginval 8 SUCCESS · PROBE OK v2+KONFLIKT ·
> cargo eq_copilot 36/36 · clippy-Abnahme aus Abschnitt 6 erfüllt · voller
> cargo-Lauf 734/735 (der eine Fehler ist der fremde Vorbefund
> `scanner::scans_real_db`, Abschnitt 16). Details, Abgrenzungen und
> Restrisiken: **`CS1-BEFUND.md`**. Die Abschnitte 6–9 sind damit umgesetzt,
> soweit sie zu Satz 1 gehören; Loudness-Wachstum (8.1), Worker/Snapshot (8.3)
> und True-Peak-Konformitätstests (9) bleiben für Änderungssatz 2 offen.

---

## 1. Auftrag

Den EQ-Copilot technisch und visuell substanziell verbessern. Gemeint sind ausdrücklich:

- DSP- und Messgenauigkeit
- Echtzeit- und Langzeitperformance
- Stabilität des C++↔Rust-Pipe-Pfads
- Qualität, Nachvollziehbarkeit und Priorisierung der Diagnosen
- JUCE-Darstellung und direkte Bedienung im Plugin
- Companion-Darstellung im Plugin-Hub
- Barrierefreiheit, Fehlerzustände und wahrheitsgetreue UI
- belastbare Regressionstests

**Nicht Teil des Auftrags:** Profilverwaltung, Profilanlage, Preset-Organisation, sonstige Verwaltungsfeatures.

Die wichtigste Leitlinie lautet: Erst die Messung und Evidenz technisch belastbar machen, danach die Resultate prominenter und überzeugender darstellen. Die Oberfläche darf keine höhere Gewissheit suggerieren, als die Messlogik tatsächlich besitzt.

---

## 2. Verbindliche Projektregeln

Vor Änderungen lesen:

1. `C:\Users\phili\FL-Studio\AGENTS.md`
2. `C:\Users\phili\FL-Studio\.leitstand\maps\index.json`
3. `C:\Users\phili\FL-Studio\.leitstand\maps\eq-copilot.md`
4. `C:\Users\phili\FL-Studio\eq-copilot\README.md`

Invarianten aus der Scope-Karte:

- Das VST bleibt bittransparent und schreibt nie ins Audiobuffer.
- Im Audio-Thread: keine Allokation, keine Locks, keine Datei- oder Netzwerkzugriffe.
- Der Analyse-Worker besitzt die `AnalyseEngine`; die UI konsumiert nur veröffentlichte Snapshots.
- JSON-Schemas sind Verträge, nicht bloß lose Dokumentation.
- Metriken und Transportformate bleiben versioniert.
- Save/Load, Connect/Disconnect sowie Start/Stop immer einschließlich Gegenpfad prüfen.

Bei Rust-FFI/Win32-Handle-Arbeit zusätzlich die globale Skill-Anleitung laden:

`C:\Users\phili\.codex\skills\unsafe-checker\SKILL.md`

---

## 3. Architekturübersicht

### JUCE/C++-Plugin

- `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\PluginProcessor.cpp`
  - transparenter Audio-Passthrough
  - Kopie in Analyse-FIFO
  - Worker, Snapshot und Serialisierung
- `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\AnalyseEngine.cpp`
  - Welch-LTAS
  - LUFS, True Peak und Dynamikmetriken
  - Spektrum, Stereo, Messabdeckung und Resonanzen
- `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\Diagnose.cpp`
  - deterministische Befundregeln und Priorisierung
- `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\PipeClient.cpp`
  - Win32-Named-Pipe-Client
- `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\PluginEditor.cpp`
  - Graph, Toolbar, Messwerte und Diagnose-Callout

### Rust-Broker

- `C:\Users\phili\FL-Studio\plugin-hub-app\src-tauri\src\eq_copilot\server.rs`
- `C:\Users\phili\FL-Studio\plugin-hub-app\src-tauri\src\eq_copilot\protokoll.rs`
- `C:\Users\phili\FL-Studio\plugin-hub-app\src-tauri\src\eq_copilot\mod.rs`

### Svelte-Companion

- `C:\Users\phili\FL-Studio\plugin-hub-app\src\lib\eq-copilot\Sensoruebersicht.svelte`
- Einhängung in `C:\Users\phili\FL-Studio\plugin-hub-app\src\routes\+page.svelte`

### Verträge und Referenzdesign

- `C:\Users\phili\FL-Studio\eq-copilot\schemas\`
- `C:\Users\phili\FL-Studio\FL-EQ-Copilot-Mockup.html`

Das Mockup ist bei Legende, PRE→POST-Darstellung, anklickbaren Findings, Keyboard-Readout und responsiven Modi bereits weiter als die aktuelle JUCE-Implementierung. Es soll als fachliche UX-Vorlage genutzt werden, nicht als bloße Inspiration verworfen werden.

---

## 4. Kurzurteil

### Was bereits gut ist

- Das Plugin ist im Nulltest bittransparent.
- Im Audio-Thread wurden keine offensichtlichen Locks oder Dateizugriffe gefunden.
- Analyse und Diagnose sind deterministisch.
- Die Architektur Audio-Thread → FIFO → Analyse-Worker → Snapshot → UI ist grundsätzlich richtig.
- C++-Goldens, Rust-EQ-Tests und der Pipe-Happy-Path sind grün.
- Die bestehende UI besitzt bereits eine brauchbare Grundstruktur und einen Vergleichsmodus.

### Warum trotzdem technische Arbeit nötig ist

Der Copilot ist noch nicht zuverlässig genug für einen dauerhaften Einsatz auf beliebigen Inserts. Die wichtigsten Ursachen:

1. Die Spektralanalyse kann breite und gegenphasige Inhalte auslöschen.
2. Integrated Loudness wird mit jeder Sessionminute teurer.
3. NaN/Inf kann den Analysezustand dauerhaft vergiften.
4. Diagnosekonfidenz und Priorität sind teilweise nicht aus echter Evidenz abgeleitet.
5. Im Rust-Pipe-Server existiert ein mögliches Undefined-Behavior-Problem.
6. Reconnect und Shutdown können an einem blockierten Pipe-Read hängen.
7. Die JUCE-Toolbar ist bei allen erlaubten Größen geometrisch überbelegt.
8. Die Companion-Ansicht zeigt nur einen Bruchteil der bereits vorhandenen Messdaten.

---

## 5. Priorisierte Befunde

| Priorität | Bereich | Befund |
|---|---|---|
| **P0** | Rust/FFI | `Vec<u8>` wird möglicherweise unter-ausgerichtet als `TOKEN_USER` dereferenziert |
| **P1** | DSP | `0.5 × (L+R)` löscht Side-/Antiphase-Anteile aus Spektrum und Aktivität |
| **P1** | Performance | Integrated-Loudness-Auswertung wächst effektiv quadratisch mit der Sessionlänge |
| **P1** | Stabilität | NaN/Inf gelangt in die Analyse und kann Akkumulatoren dauerhaft vergiften |
| **P1** | Pipe | Reconnect/Stop unterbricht blockierendes `ReadFile` nicht zuverlässig |
| **P1** | Diagnose | Kontextfreie Balance-Regeln und teilweise synthetische Konfidenz |
| **P1** | JUCE-UX | Toolbar überlappt bei Minimum, Default und teilweise Maximum |
| **P2** | DSP | Sample-Rate-Abhängigkeit und erfundene Werte oberhalb verfügbarer Bänder |
| **P2** | Diagnose | Resonanzen werden zu stark nach reinem dB-Überstand sortiert |
| **P2** | UI | Graph, Befunde und Vergleich sind zu schwach miteinander verzahnt |
| **P2** | Companion | Polling, Fehlerzustände, Design-Drift und fehlende Analysevisualisierung |
| **P2** | Verträge | Schema, C++, Rust und TypeScript können unabhängig voneinander driften |

---

## 6. Arbeitspaket A — P0 Rust/FFI-Sicherheit

### Befund

In `C:\Users\phili\FL-Studio\plugin-hub-app\src-tauri\src\eq_copilot\server.rs:69-83` wird ein Buffer als `Vec<u8>` angelegt, zu `*const TOKEN_USER` gecastet und dereferenziert.

`Vec<u8>` garantiert nur Alignment 1. `TOKEN_USER` benötigt ein höheres Alignment. Auch wenn der Windows-Allocator in der Praxis häufig großzügiger ausrichtet, ist die Dereferenzierung nach Rust-Regeln potenziell Undefined Behavior.

Der gezielte Clippy-Lauf hat den Pointer-Alignment-Verstoß bestätigt. Zusätzlich fehlen Sicherheitsbegründungen an `unsafe impl Send for Sicherheit` und den Unsafe-Blöcken.

### Zielzustand

- korrekt ausgerichteter Speicher, z. B. über einen typisierten/alignment-sicheren Buffer oder geeigneten Windows-Wrapper
- Raw Handles sofort in RAII-Guards überführen
- bei Fehlern keine Handle-Leaks
- für jeden Unsafe-Block eine konkrete `// SAFETY:`-Begründung
- `unsafe impl Send` nur behalten, wenn Lebensdauer, Immutabilität und einmalige Freigabe nachweisbar sind
- `GetLastError()` vor weiteren Win32-Aufrufen wie `CloseHandle` sichern

### Abnahme

- kein `clippy::cast_ptr_alignment`
- kein `clippy::undocumented_unsafe_blocks`
- kein fehlender Kommentar am `unsafe impl`
- Fehlerpfade besitzen RAII und leaken beim Thread-Spawn-Fehler kein Pipe-Handle

---

## 7. Arbeitspaket B — Stereo- und Spektralanalyse korrigieren

### Befund

In `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\AnalyseEngine.cpp:281-308` wird für Spektrum und Aktivität `mono = 0.5 × (L+R)` verwendet.

Folgen:

- Antiphase-/Side-only-Signale können vollständig verschwinden.
- Hard-panned Material wird zu niedrig bewertet.
- Breite Pads, Chöre und Hallfelder verlieren messbare Energie.
- Ein hörbares Signal kann als inaktiv oder spektral dünn erscheinen.

Das ist für den Stil des Users besonders kritisch, weil breite Chöre, Hallräume, Piano und hybride Flächen zentral sind.

### Zielzustand

Für die tonale Energiebewertung nicht erst Samples summieren, sondern Kanalenergien kombinieren:

```text
PSD = (PSD_L + PSD_R) / 2
```

Zusätzlich sinnvoll:

- Mid-LTAS
- Side-LTAS
- bandweises M/S-Verhältnis
- bandweise Kohärenz/Korrelation
- Low-End-Mono-Kompatibilität, mindestens 20–150 Hz

Die globale Aktivität muss aus summierter Kanalenergie entstehen, nicht nur aus dem Mid-Signal.

### Abnahme

Neue Regressionstests für:

- `L = Signal`, `R = -Signal`
- nur links
- nur rechts
- Mono
- Side-only
- dekorrelierter Stereo-Hall

Der Antiphase-Fall muss ein volles Spektrum und aktive Messzeit liefern, nicht „keine Daten“.

Wichtig: Auch die Python-/Offline-Referenz prüfen. Wenn sie denselben `(L+R)/2`-Fehler verwendet, dürfen die C++-Goldens diesen Fehler nicht einfach weiter konservieren.

---

## 8. Arbeitspaket C — Langzeitperformance und robuste Zustände

### 8.1 Integrated Loudness

`C:\Users\phili\FL-Studio\eq-copilot\plugin\src\AnalyseEngine.h:222-227` hält eine unbegrenzt wachsende Zellhistorie. In `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\AnalyseEngine.cpp:523-570` wird die komplette Historie bei jeder Aktualisierung mehrfach durchlaufen und ein temporärer Vektor aufgebaut.

Das führt über die Gesamtlaufzeit zu effektiv quadratischer Arbeit.

#### Zielzustand

- kurzfristig Integrated LUFS seltener aktualisieren
- dauerhaft inkrementelle oder komprimierte Gate-Statistik
- Ringbuffer für Momentary/Short-Term
- keine wiederholte temporäre Vektorallokation
- CPU einer Instanz nach acht Stunden nicht höher als nach fünf Minuten

### 8.2 NaN/Inf

`C:\Users\phili\FL-Studio\eq-copilot\plugin\src\PluginProcessor.cpp:86-104` und `:157-173` erkennen nicht-endliche Samples, kopieren sie jedoch weiterhin in die Analyse-FIFO.

#### Zielzustand

- Audioausgabe unverändert lassen
- nur die Analysekopie sanitisieren
- NaN/Inf dort durch `0.0f` oder eine explizit gewählte stabile Strategie ersetzen
- Fehler pro Messepoche zählen
- Filterzustände müssen automatisch wieder zu gültigen Werten zurückkehren
- UI unterscheidet „seit Reset“ und „seit Pluginstart“

### 8.3 Snapshot und Worker

Weitere Verbesserungen:

- statt 50-ms-Idle-Polling Event/Semaphore/Atomic-Notify einsetzen
- Snapshot-Publikation als unveränderlicher Double Buffer oder versionierter fester Block
- keine unnötigen Vektorkopien bei 30-Hz-UI und Pipe-Publikation
- bei `prepareToPlay` Reset und Workerzustand über eine Messepochen-ID koordinieren
- Diagnosecache beim Reset sofort leeren

### Abnahme

- Benchmark 1/4/8 Stunden, jeweils mit 1, 32 und wenn möglich 64 Instanzen
- stabile CPU- und Speichernutzung
- NaN-Block zwischen gültigen Sinusblöcken: automatische Recovery innerhalb eines definierten Analysefensters
- kein alter Diagnosebefund nach Reset ohne klare Kennzeichnung

---

## 9. Arbeitspaket D — Sample Rate, Gültigkeit und True Peak

### Befund

Die LTAS-Auswertung reicht fest bis 18 kHz. `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\AnalyseEngine.cpp:432-447` füllt Randlücken mit dem letzten gültigen Wert. Bei niedriger Sample Rate kann damit ein nicht messbarer Hochfrequenzbereich wie echte Evidenz aussehen.

Centroid und Rolloff hängen außerdem vom jeweiligen Nyquist-Limit ab; Ultraschall kann die Werte bei hohen Raten verfälschen.

### Zielzustand

- fester hörbarer Analysebereich, z. B. 20 Hz bis `min(20 kHz, 0.95 × Nyquist)`
- nicht verfügbare Bänder explizit als ungültig/unavailable transportieren
- keine Randwertfortschreibung oberhalb Nyquist
- per-band validity statt nur globaler Gültigkeit
- optional interner Analyse-Resampler auf feste Rate
- Flatness entweder korrekt pro Segment bestimmen oder als approximative Metrik sichtbar kennzeichnen

### True-Peak-Erweiterung

Die bestehende Streaming-Implementierung braucht Konformitätstests:

- BS.1770-/EBU-Testvektoren
- phasenverschobene Near-Nyquist-Sinusfälle
- Impuls direkt am Block-/Streamende
- mehrere Sample Rates und Blockgrößen
- definierter Umgang mit Filtertail beim Transportende

---

## 10. Arbeitspaket E — Diagnose v2

### 10.1 Echte statt synthetische Konfidenz

In `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\Diagnose.cpp:229-234` werden Bestandteile wie Persistenz und Zielvertrauen teilweise mit Konstanten belegt. Die UI darf daraus keine scheinpräzise Messkonfidenz machen.

#### Zielzustand

Konfidenz aus echten Größen berechnen:

- Anteil auffälliger Analysefenster
- Varianz der Abweichung
- zeitliche Persistenz
- Bandbreite
- Messabdeckung
- Stabilität über unterschiedliche Lautheitsbereiche
- Übereinstimmung unterschiedlicher FFT-/Fenstergrößen

Falls diese Evidenz noch fehlt, statt Prozentwerten klar „Heuristik“ anzeigen.

### 10.2 Priorität evidenzbasiert bestimmen

`C:\Users\phili\FL-Studio\eq-copilot\plugin\src\Diagnose.cpp:371-399` folgt zu stark festen Regelklassen.

Empfohlener Score:

```text
Priorität =
  Schweregrad
× Persistenz
× Konfidenz
× betroffene Bandbreite
× Signalabdeckung
× Kontextrelevanz
```

Stilgewichtung nur als nachgelagerter, sichtbarer Faktor.

### 10.3 Objektive Defekte von tonalen Hypothesen trennen

Überall zulässige objektive Befunde:

- schmale Resonanz
- DC/Sub-Rumble
- True-Peak-/Clipping-Risiko
- Phasen-/Mono-Risiko
- ungewöhnlich instabile Energie
- unzureichende Messabdeckung

Kontextabhängige Hypothesen:

- zu dunkel/hell
- zu viel Air
- Mittenloch
- zu warmer Grundton
- zu wenig Präsenz

Auf beliebigen Einzelinstrumenten dürfen Mixbalance-Regeln nicht wie universelle Wahrheiten formuliert werden. Beispielsweise ist viel Air bei Overheads, Atem-/Chor-Layern oder Atmosphären nicht automatisch falsch.

### 10.4 Resonanzranking verbessern

In `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\Diagnose.cpp:249-255` dominiert der dB-Überstand. Ein kurzer Peak kann so eine dauerhaft störende Resonanz verdrängen.

Empfohlen:

```text
Resonanzscore =
  dB-Überstand
× zeitliche Häufigkeit
× Persistenz
× Messabdeckung
× spektrale Stabilität
```

Statische Resonanzen und transiente Klingeltöne als getrennte Befundtypen behandeln.

### 10.5 Empfehlungen messbarer machen

Aus Frequenz und Bandbreite ableiten:

- Q-Startwert
- sinnvoller Gain-Bereich statt Scheingenauigkeit
- statisch oder dynamisch
- bei dynamischer Bearbeitung: Range, Zeitbereich und Stop-Bedingung
- erwartete Nebenwirkung

Beispiel:

> 310 Hz, Q ungefähr 1,4, zunächst −1,5 bis −2,5 dB. Wenn die Auffälligkeit nur bei starken Pianoakkorden auftritt, dynamisch mit maximal −3 dB Range. Stoppen, sobald der Körper hörbar dünner wird.

---

## 11. Arbeitspaket F — Pipe und Broker härten

### 11.1 Reconnect/Shutdown

`C:\Users\phili\FL-Studio\eq-copilot\plugin\src\PipeClient.cpp:134-139` setzt beim Reconnect nur ein Flag und signalisiert eine Condition Variable. Ein Thread, der in `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\PipeClient.cpp:251-255` auf ein ACK wartet, wird dadurch nicht zuverlässig geweckt.

#### Zielzustand

Bevorzugt:

- Overlapped I/O
- eigener Stop-/Reconnect-Event
- feste Read-/Write-Deadlines
- expliziter Verbindungszustandsautomat

Mindestens:

- `CancelIoEx(activeHandle, nullptr)`
- aktives Handle beim Reconnect schließen
- ACK-Timeout
- ACK-Sequenz und Token validieren
- bei unerwartetem ACK oder Frame Verbindung schließen
- komplette Writes in einer Schleife absichern

### 11.2 Brokerprotokoll

`C:\Users\phili\FL-Studio\plugin-hub-app\src-tauri\src\eq_copilot\protokoll.rs:118-126` validiert zu wenig.

Ergänzen:

- `is_finite()` für alle Floats
- plausible Wertebereiche
- LTAS-Länge je `metrics_version`
- korrekte Fensterreihenfolge
- Stringlängen
- strenge unbekannte Felder, soweit kompatibel
- Messversion und Rollen-/Audio-Grenzen

Ein zweites `Hello` in `C:\Users\phili\FL-Studio\plugin-hub-app\src-tauri\src\eq_copilot\server.rs:357-368` entweder verbieten oder vollständig wie den ersten Handshake prüfen.

Ungültiger Heartbeat-Token darf nicht ohne ACK zu einem dauerhaft blockierten Client führen: Verbindung verwerfen.

### 11.3 Liveness und Vergleich

- intern `Instant` statt Wall-Clock für Liveness verwenden; Epochzeit nur zur Anzeige
- Liveness pro Verbindung statt nur pro Sensor-ID
- bei doppelten Sensoren Owner-Failover sauber abbilden
- `metrics_version` beim Paarvergleich prüfen
- `OnceLock<Result<...>>` nicht so verwenden, dass ein transienter Startfehler bis zum App-Neustart eingefroren bleibt
- vergiftete Mutexes als sichtbaren Fehler behandeln statt mit `.expect()` die App zu paniken

### 11.4 Zeitliche Paarung

Der aktuelle Vergleich prüft nur eine grobe Überlappung der Projektfenster in `C:\Users\phili\FL-Studio\plugin-hub-app\src-tauri\src\eq_copilot\mod.rs:404-431`.

Für echten PRE/POST-Vergleich ergänzen:

- Messepochen-ID
- Zeitfenster-ID
- tatsächlicher Start-/Endpunkt
- optional grober Energie-Fingerprint
- Deltas nur aus gemeinsam beobachteten Fenstern

### Abnahme

- Broker akzeptiert Verbindung und sendet nie ACK: Reconnect und Plugin-Destruktor enden innerhalb einer festen Frist
- partielle Frames, falscher Token, falsche Sequenz und zweite Hello-Nachricht werden deterministisch behandelt
- begrenzte Anzahl hängender Clients
- transienter Pipe-Namenskonflikt kann ohne App-Neustart recovern

---

## 12. Arbeitspaket G — JUCE-Darstellung und UX

### 12.1 Responsive Toolbar

In `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\PluginEditor.cpp:476-500` bekommen alle Controls feste Breiten, teils auch unsichtbare Controls. Die Skalierung in `C:\Users\phili\FL-Studio\eq-copilot\plugin\src\PluginEditor.h:58-59` fällt nicht unter 1,0.

Die benötigte Breite überschreitet den verfügbaren Platz bei Minimum, Default und teilweise Maximum.

#### Zielzustand

Drei Layoutmodi:

1. **Kompakt:** wichtigste Aktion plus Icons, Rest im Overflow
2. **Standard:** zwei logisch getrennte Reihen/Gruppen
3. **Erweitert:** vollständige Beschriftungen und Vergleichssteuerung

Nur sichtbare Controls reservieren Platz. Skalierung aus Breite und Höhe ableiten.

### 12.2 Permanenter Befund-Inspector

Der aktuelle Callout kann höher als der Editor werden. Diagnosen sind das Hauptprodukt und dürfen nicht in einer flüchtigen Textblase versteckt sein.

#### Zielzustand

- rechts angedockt bei breiten Fenstern
- unterer Drawer bei schmalen Fenstern
- einzelne auswählbare Befundkarten
- nächster/vorheriger Befund
- kopierbare Werte
- Tastaturfokus
- klarer Messzeitraum und Evidenzstatus

Ein Klick auf eine Karte:

- hebt den Frequenzbereich im Graphen hervor
- zeigt Mittelpunkt und Bandbreite
- blendet einen vorgeschlagenen EQ-Bell transparent ein
- verändert niemals Audio oder Parameter automatisch

### 12.3 Graphinteraktion

Ergänzen:

- Crosshair mit Hz/dB
- nächstes Analyseband
- Hovermarkierung und Tooltips
- klickbare Befundmarker
- Karte ↔ Marker synchronisieren
- Mausrad-Zoom
- Doppelklick zurück auf Auto
- Keyboard-Navigation links/rechts
- Perzentil-basierte Y-Autoskalierung mit langsamer Rückkehr
- optionale 40-/60-/80-dB-Sichtbereiche

Die aktuelle Y-Skalierung darf nicht nach einem frühen lauten Ereignis bis zum Reset dauerhaft zu groß bleiben.

### 12.4 Vergleich

„Festhalten“ nimmt aktuell einen Snapshot für die Anzeige und einen weiteren beim Dateischreiben. Diese können voneinander abweichen.

#### Zielzustand

- genau ein unveränderlicher Snapshot
- „Vergleich einfrieren“ und „Exportieren“ trennen
- aktuelle Kurve durchgezogen
- eingefrorene Kurve gestrichelt
- Deltafläche zwischen beiden
- Δ-dB je Band
- sichtbare Legende und Zeitangabe
- Vergleich separat löschbar

Das Referenzverhalten ist in `C:\Users\phili\FL-Studio\FL-EQ-Copilot-Mockup.html` bereits angelegt.

### 12.5 Statuswerte und Benennung

`C:\Users\phili\FL-Studio\eq-copilot\plugin\src\PluginEditor.cpp:731-747` zeichnet alle Werte in eine lange Zeile. Bei kleinen Fenstern wird sie abgeschnitten.

Stattdessen responsive Chips oder ein kleines Raster:

- LUFS-I
- True Peak
- Crest
- Breite
- Korrelation
- Messdauer
- Abdeckung

„Dynamik“ derzeit in „Crest / Peak–RMS“ umbenennen, sofern tatsächlich Crest angezeigt wird. LRA und DR nicht sprachlich vortäuschen.

### 12.6 Accessibility

- JUCE-Accessibility-Handler für Graph und Befundliste
- klare Tabreihenfolge
- sichtbare Fokusrahmen
- Screenreader-Zusammenfassung der Messung
- Kurven nicht nur über Farbe unterscheiden, sondern auch über Linienmuster
- Marker zusätzlich über Form/Symbol unterscheiden
- Reset-, Vergleichs- und Exportsemantik mit Tooltips erklären

---

## 13. Arbeitspaket H — Svelte-Companion

### 13.1 Vorhandene Messdaten wirklich zeigen

`C:\Users\phili\FL-Studio\plugin-hub-app\src\lib\eq-copilot\Sensoruebersicht.svelte` zeigt bisher überwiegend Verbindung, LUFS, Peak und Zeit, obwohl der Broker bereits LTAS, Centroid, Breite, Korrelation und Fensterdaten liefert.

Zielbild:

- überlagerte Sensor-LTAS
- PRE/POST-Deltakurve
- bandweise Differenztabelle
- Synchronitäts-/Messfensterstatus
- Befundkarten für das Paar
- sichtbare Warnung bei Versions-, Zeitraum- oder Sensor-Mismatch

### 13.2 Polling und Zustand

Der feste Ein-Sekunden-Interval kann Invokes überlappen und Antworten außer Reihenfolge anwenden. Hauptseite und Modal pollen zusätzlich parallel.

Bevorzugt:

- Tauri-Events für Sensor-, Mess- und Paaränderungen
- gemeinsamer zentraler Store
- langsamer Fallback-Poll

Alternativ mindestens:

- Single-Flight
- rekursives `setTimeout`
- Generation-ID gegen veraltete Antworten

### 13.3 Fehlerzustände

Der Initialfehler wird gespeichert, im `status === null`-Zweig aber nicht angezeigt.

Ergänzen:

- expliziter Fehler statt endlosem „Lade Brokerstatus“
- Retry
- letztes erfolgreiches Update
- Stale-Badge bei alten Daten
- alte Fehlermeldung nach erfolgreichem Abruf löschen
- unbekannte Zustände nicht als „messbereit“ darstellen

### 13.4 Darstellung und Accessibility

- keine eigene dunkle Legacy-Palette; gemeinsame Leitstand-/EQ-Tokens verwenden
- Farben/Tokens idealerweise aus einer neutralen Quelle für CSS und C++ generieren
- Modal mit Focus Trap, Initialfokus und Fokus-Rückgabe
- `aria-live` für Verbindungs- und Messänderungen
- Icon-only-Button mit sichtbarer Beschriftung oder belastbarem `aria-label`
- gesunde/gesamte Sensoren getrennt anzeigen; stale nicht als normal verbunden zählen
- EQ-Ansicht lazy laden; aktueller Frontend-Build meldet einen großen Node-Chunk

---

## 14. Arbeitspaket I — Verträge und Testsystem

### 14.1 Schema als echte Single Source

Aktuell können C++, Rust, JSON-Schema und lokale TypeScript-Interfaces unabhängig driften.

Konkreter bereits sichtbarer Verlust:

- `Befund.dauerhaft` existiert im C++-Modell
- Snapshot-Serialisierung und Snapshot-Schema transportieren es nicht strukturell

Ziel:

- `modus: dauerhaft | zeitweise` oder ein äquivalentes strukturelles Feld
- generierte oder zumindest gegeneinander getestete Vertragstypen
- Schema-Validierung für echte C++-Snapshots und Heartbeats
- unbekannte/ungültige/range-fremde Werte ablehnen
- NaN/Infinity explizit testen

### 14.2 Fehlende Regressionen

Ergänzen:

1. Anti-phase, Side-only, Hard-pan, dekorrelierter Hall
2. 32/44,1/48/96/192 kHz
3. Blockgrößen 1, 64, 127, 512, 2048 und ggf. Nullblock
4. NaN-Recovery statt nur unveränderte Audioausgabe
5. 1/4/8-Stunden-Performance mit vielen Instanzen
6. True-Peak-Konformität
7. Broker ohne ACK und mit partiellen Frames
8. Reconnect-/Destruktor-Deadlines
9. Schema-Konformität über C++ → Rust → TypeScript
10. Screenshot-/Layouttests bei 520×340, 960×540 und 2400×1500 sowie 100/150/200 % DPI
11. reale Musikstücke mit fachlich markierten True/False Positives

Die bestehenden Goldens sind synthetische 20-Sekunden-Signale bei 48 kHz und fester Blockgröße. Sie sind gute Regressionsanker, aber keine ausreichende Validierung musikalischer Urteilsgüte.

### 14.3 CTest

Die C++-Testprogramme existieren, sind aber nicht als klarer vollständiger CTest-Lauf registriert. `enable_testing()` und `add_test()` einführen, damit CI und lokale Verifikation nicht von einer manuell gepflegten Befehlsfolge abhängen.

---

## 15. Empfohlene Implementierungsreihenfolge

### Änderungssatz 1 — Sicherheit und Messkorrektheit

1. Rust-Alignment, RAII und Unsafe-Begründungen
2. Stereoanalyse auf Kanalenergie plus M/S umstellen
3. NaN/Inf nur im Analysezweig sanitisieren
4. Sample-Rate-Gültigkeit korrekt modellieren
5. adversariale DSP- und FFI-Tests

### Änderungssatz 2 — Dauerbetrieb

1. Loudness inkrementell/komprimiert berechnen
2. Snapshot-Publikation verbessern
3. Worker ohne permanentes Polling
4. Pipe mit Deadlines und echtem Cancel
5. Langzeit-/Viele-Instanzen-Benchmark

### Änderungssatz 3 — Diagnose v2

1. echte Persistenz und Konfidenz
2. objektive Defekte von tonalen Hypothesen trennen
3. evidenzbasiertes Prioritätsscoring
4. bessere Resonanzbewertung
5. Q, Wertebereich und Stop-Bedingungen ableiten
6. realer Musik-Korpus

### Änderungssatz 4 — Produkt-UX

1. responsive JUCE-Toolbar
2. permanenter Befund-Inspector
3. Karten ↔ Graph-Verknüpfung
4. Crosshair, Legende und Deltafläche
5. wahrheitsgetreue Messwertnamen
6. Accessibility
7. Companion auf dieselbe Daten- und Designsprache bringen

Nicht zuerst die Oberfläche polieren und technische Unsicherheit verdecken. Änderungssatz 1 und 2 sind Voraussetzung für eine glaubwürdige Diagnose-UX.

---

## 16. Bereits ausgeführte Verifikation

### Erfolgreich

- CMake Release-Build für VST3, Nulltest, Golden-Test und Pipe-Probe
- Nulltest: **10/10**, bittransparent
- Golden-Test: **135/135**
- Rust EQ-Copilot-Filter: **35/35**
- Pipe E2E: Protokoll v2, Konflikt und Konfliktende erfolgreich
- Svelte Check: **0 Fehler, 0 Warnungen**
- Frontend-Build erfolgreich

### Vollständiger Rust-Lauf

Der komplette Workspace-Lauf hatte **732 erfolgreiche und zwei fehlschlagende Tests außerhalb des EQ-Copilot-Scopes**:

- `bausteine::scan::tests::echter_lauf_mit_kindprozess_und_abbruch`
- `scanner::tests::scans_real_db`

Nicht als EQ-Copilot-Regression deklarieren, aber beim abschließenden Workspace-Gate ehrlich aufführen.

### Clippy-Befunde im EQ-Scope

- Pointer-Alignment-Verstoß in `server.rs`
- fehlende Sicherheitskommentare an Unsafe-Blöcken
- fehlende Begründung am `unsafe impl Send`
- kleinere Warnung: unnötiges `mut` in `eq_copilot/mod.rs`

### Frontend-Hinweis

Der Build meldete einen Node-Chunk von ungefähr 769 kB minifiziert. Lazy Loading/Code Splitting prüfen.

---

## 17. Verbindliche Prüfbefehle nach Änderungen

PowerShell, Workspace `C:\Users\phili\Projekte\Nakama` (seit dem
Workspace-Umzug 18.08.2026; der Broker ist eine eigenständige Crate in
`broker/`, die Hub-App-Kommandos existieren nicht mehr):

```powershell
cmake --build eq-copilot\build --config Release --target EqCopilot_VST3 EqCopPipeProbe EqCopNullTest EqCopGoldenTest

& 'eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe'

& 'eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe' 'eq-copilot\fixtures'

cargo test --manifest-path broker\Cargo.toml

cargo clippy --manifest-path broker\Cargo.toml --lib --message-format=short -- -W clippy::undocumented_unsafe_blocks -W clippy::cast_ptr_alignment
```

Zusätzlich den realen Pipe-Probe-Lauf mit einem eindeutigen Test-Pipe-Namen ausführen. Nach UI-Änderungen die JUCE-Oberfläche bei allen drei Fenstergrößen und mehreren Windows-DPI-Stufen visuell prüfen; reine Build-Erfolge reichen dort nicht.

---

## 18. Definition of Done

Die Überarbeitung ist erst abgeschlossen, wenn:

- Anti-phase und Side-only korrekt analysiert werden.
- NaN/Inf den Analyzer nicht dauerhaft zerstört.
- Langzeitsessions keine laufzeitabhängige CPU-Eskalation zeigen.
- alle Rust-FFI-Alignment- und Unsafe-Befunde beseitigt oder nachweisbar begründet sind.
- Reconnect und Shutdown bei schweigendem Broker eine feste Deadline einhalten.
- Konfidenz und Priorität auf gemessener Evidenz beruhen oder ehrlich als Heuristik bezeichnet werden.
- objektive Probleme von stilistischen Hypothesen getrennt sind.
- die JUCE-Toolbar bei Minimum, Default und Maximum ohne Überlappung funktioniert.
- Befundkarten, Graphmarker und Vergleich visuell wie funktional verbunden sind.
- die Companion-Ansicht LTAS und PRE/POST-Deltas tatsächlich darstellt.
- Initial-, Fehler-, Stale- und Mismatch-Zustände sichtbar sind.
- Accessibility und Tastaturbedienung nicht nur nachträglich behauptet, sondern geprüft wurden.
- die realen C++-, Rust-, Pipe- und Frontend-Checks ausgeführt und ihre Resultate gesehen wurden.

---

## 19. Bestehender Dirty-Workspace — nicht überschreiben

Bereits vor dieser Übergabe vorhanden:

```text
M  C:\Users\phili\FL-Studio\.codex\config.toml
M  C:\Users\phili\FL-Studio\plugin-auswahl.json
M  C:\Users\phili\FL-Studio\plugin-hub-app\src-tauri\Cargo.toml
M  C:\Users\phili\FL-Studio\test-results.json
?? C:\Users\phili\FL-Studio\eq-copilot\*.webp
```

Diese Änderungen und Designbilder gehören nicht automatisch zum eigenen Änderungssatz. Vor jedem Edit erst den aktuellen Diff prüfen und fremde Änderungen erhalten.

---

## 20. Empfohlener Startpunkt für Claude

1. Workspace-Regeln und EQ-Scope-Karte lesen.
2. Dirty Diff prüfen, insbesondere `Cargo.toml` nicht blind überschreiben.
3. **Nur Änderungssatz 1** als ersten kohärenten Bauabschnitt umsetzen.
4. Zuerst P0-Rust-Alignment, danach Stereoanalyse und NaN-Recovery.
5. Für jeden Fix einen reproduzierbaren fehlenden Test voranstellen.
6. Alle Scope-Checks aus Abschnitt 17 ausführen.
7. Resultate und noch offene Risiken in dieser Übergabe oder einer neuen Abschlussübergabe wahrheitsgetreu fortschreiben.
