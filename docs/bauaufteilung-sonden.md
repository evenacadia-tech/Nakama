# Bauaufteilung Sondenkern — Sessions, Gates, adversariale Prüfung

- **Stand:** 2026-08-21
- **Gehört zu:** `docs/FL-Nakama-Sonden-Design-Entwurf.md` (Fassung 0.4 +
  Errata-Block 21.08.2026 — der Block hat Vorrang) und
  `docs/pruefbericht-sondenentwurf-2026-08-20.md`
- **Was das hier ist:** wie *ich* den Plan bauen würde. Der Entwurf ordnet
  **Fähigkeitsgrenzen** (P0–P9), dieses Dokument ordnet **Arbeitscontainer**
  (Sessions) und die Prüfung dazwischen.
- **Status:** **Bauentscheidung erteilt** (User, 20.08.2026: „okay dann fangen
  wir damit nächste session an"). Nächste Session startet bei **S0**.
- **Nachgezogen 20.08. auf Entwurf 0.4:** Fassung 0.4 hat alle fünf Befunde des
  Prüfberichts eingearbeitet; NAK-19…23 sind geschlossen (`ab80522`). Phasen
  P0–P9 und die 19 Tickets sind unverändert — der Sessionplan unten trägt
  weiter. Zwei Vorlaufpunkte sind dadurch entfallen (§1.2/§1.3).
- **Nachgezogen 21.08. nach dem Kontext-Interview:** Namen laut User (21.08.:
  „Nakama Gen = Main app · aktive sonde = Nakama Probeeq · passive sonde =
  Nakama Suna · Bundle = Nakama Studio") — **Main = Gen, Active Probe =
  Probeeq, Passive Probe = Suna**; die alten Arbeitstitel stehen unten weiter,
  wo sie Ticket- oder Entwurfsbezeichnungen sind. Berichtigt: §1.3
  (Batch-Form war die Gegenposition zu Entwurf §33.1 und zum gebauten `.fbs`),
  §1.2 (Hörkompass ist Studie), §3 S2/S5/S6 (T2-Stände aus den Manifesten,
  kein Häkchen ohne PASS), §4 (S3b gezählt), §5 (Gate 8 gegenstandslos),
  §6.4 (Eigentum des Entwurfs). Kanon: **15 Beine** (A11 `pruefe_v2_schemas.py`
  neu am 21.08.).
- **Nachgezogen 22.08.:** S7 (`SONDE-006`) gebaut, Kanon **17 Beine** (B2
  `EqCopStateMigrationTest`, A12 `erzeuge_state_fixtures.py --pruefen`);
  Manifest `docs/beweise/SONDE-006.md`. Danach am selben Tag **Termin B
  gemessen und S4 gebaut** (Capabilityreport, Kanon **18 Beine** mit A13; seit
  S8 **19** mit A14);
  P0 ist damit bis auf **Gate G0** (T3, eigene Session) vollständig. Nächste
  Flächen: G0, dann S8.
- **Nachgezogen 23.08. (Umschnitt, NAK-64):** Entscheid **„Gen wird
  EQ-Zentrale"** (Register 23.08.; Entwurf-Erratum (n); Wortlaut
  `../design/abnahmen/2026-08-23-gen-eq-zentrale.md` und
  `…interview-struktur.md`): **zwei** Apps statt drei — Suna ist in Probeeq
  aufgegangen, die EQ-Bedienung aller Sonden liegt zentral auf Gens
  Seite 2, Gen erhält einen vollwertigen Master-EQ, das ±3-dB-Remote-Limit
  ist entfallen (U14). Umgeschnitten: §1 (Oberflächen) und §3 P6–P7 —
  **S28b und S31b sind neu**, Summe 37 → **39** Sessions;
  `docs/plan/plan.json` trägt denselben Schnitt. Spielregeln der UI:
  `../design/docs/ui-spielregeln-eq-zentrale.md` (44 Regeln).

---

## 0. Das Grundprinzip

> **Eine Session endet nicht, wenn der Kontext voll ist, sondern wenn ein
> Zuwachs vollständig BEWIESEN ist.**

Der Code ist klein — 6 251 Zeilen über Plugin und Broker zusammen, größte
Datei 1 163 Zeilen. Kontext ist hier **nicht** der Engpass. Der Engpass ist
die **Beweisfläche**: wie viel lässt sich in einem zusammenhängenden Durchgang
so belegen, dass ein fremder Prüfer ohne meinen Kontext PASS sagen kann.

Daraus folgt die Sessiongrenze: **1 Session = 1 Ticket + sein Beweismanifest
+ sein Frischkontext-Prüfer.** Große Tickets brauchen mehrere Sessions, kleine
werden gebündelt. Eine Session, die Code ohne Manifest hinterlässt, ist nicht
fertig, sondern **offen** — auch wenn alles kompiliert.

---

## 1. Session 0 — was VOR SONDE-001 fehlt

Nach der 0.4-Einarbeitung bleibt **ein** echter Vorlaufpunkt (§1.1). §1.2/§1.3
sind erledigt und stehen nur noch als Beleg; §1.4 ist neue Vorgabe aus §0.4,
die in die Prüflisten fließt.

### 1.1 Es gibt keine CI

`SONDE-001` verlangt „…laufen im CI". `.github/workflows/` existiert nicht.
Eine echte GitHub-CI für einen JUCE-VST3-Build auf Windows ist für dieses
Projekt unverhältnismäßig (eine Maschine, privat, kein Team).

**Entscheidung: statt CI ein lokaler Beweis-Runner.** `tools/beweise.ps1`
fährt den kompletten Kanon nacheinander und schreibt die **rohe** Ausgabe in
ein Manifest:

```
eq-copilot\build\...\EqCopNullTest.exe
eq-copilot\build\...\EqCopGoldenTest.exe eq-copilot\fixtures
eq-copilot\build\...\EqCopMarkierungTest.exe
cargo test --manifest-path broker/Cargo.toml
+ ab P0: EqCopIdentityTest
+ ab P1: EqCopStateMigrationTest, EqCopSchemaTest
+ ab P2: EqCopHostContextTest, EqCopQueueStressTest, EqCopAnalysisGoldenTest
+ ab P6: EqCopDspGoldenTest, EqCopTransactionTest
```

Ohne diesen Runner kostet jedes Manifest 10 Handgriffe und wird deshalb
irgendwann geschludert. Mit ihm ist es **ein** Befehl — das ist der ganze
Unterschied zwischen „Beweisstandard" und „Beweisvorsatz".

**Stand 20.08.2026: gebaut und gefahren.** `tools/beweise.ps1` liegt vor,
`docs/beweise/VORLAGE.md` ist die Ticket-Vorlage, `docs/beweise/S0-basislinie.md`
ist die Regressions-Basislinie (4/4 grün auf `b7d37ce`). Zwei Zusätze über die
Vorgabe hinaus, beide negativ geprüft: der Runner misst den **Baustand**
(Quell-mtime vs. Binär-mtime + SHA-256) und verweigert mit Exitcode 4 die
Beglaubigung, wenn die Prüfbinaries älter sind als die Quellen; und die sieben
**geplanten** Prüfbinaries stehen bereits in seiner Tabelle — sie erscheinen als
neutrale Zeile „geplant (ab P?)" und laufen automatisch als Pflicht mit, sobald
das jeweilige Ticket sie gebaut hat.

**Stand 21.08.2026: 15 Beine laufen, 5 stehen als geplant.** Laufend: A1
NullTest · A2 Golden · A3 Markierung · A4 `cargo test` · A5–A10 die sechs
Python-Beine des v3-Vertrags (Referenzbein, drei Bytegleichheits-Riegel,
`flatc`-Drift, FlatBuffers-Fixtures) · **A11 `pruefe_v2_schemas.py`** (neu
21.08.: die fünf v2-Schemas waren bis dahin von keinem Bein gelesen) · B1
Identität · B3 Hostkontext · B3b Host-Probe · B3c Schema. Geplant: B4 QueueStress und B5 AnalysisGolden (P2), B6 DspGolden
und B7 Transaction (P6). **Seit 22.08. (SONDE-006) laufen 17 Beine:** B2
StateMigration und A12 `erzeuge_state_fixtures.py --pruefen` sind Pflicht. Nicht im Kanon, obwohl gebaut: `EqCopAuxSpikeTest`
(41 Prüfungen, NAK-37).

### 1.2 ~~NAK-19~~ — erledigt, P3 ist nicht mehr blockiert

Der Produktentscheid vom 20.08. (Entwurf §0.3) löst die Kollision auf: Der
Hörkompass-Zielvertrag gilt der **eigenständigen Prisma-App**, das
Master-Plugin bekommt eine **konventionelle Arbeits-UI**. Quellenliste,
Heatmap und Detailansicht sind dort legitime Dauerarbeitsflächen; ihr Maßstab
ist Lesbarkeit, nicht „leeres Glas". **P3 kann ohne weitere Entscheidung
gebaut werden.**

**Nachtrag 21.08.:** Der Hörkompass ist seit dem Kontext-Interview „Alles nur
Studie" (User), die Prisma-App ist geparkt, kein Bauziel (Entwurf-Errata (c)).
Die Kollision ist damit nicht nur aufgelöst, sondern gegenstandslos. Der
Gestaltungs-Vorgabe der Plugin-UI ist der Figma-Stand des Users (Errata (h));
die Lesbarkeitsregel des Geschmacksprofils bindet nur die Studie, die
Lesbarkeitsregel des Design-Repos („in 2 Sekunden ablesbar", dessen Regel 6)
gilt dort weiter; was P3 in Gen zeigt (Overview +
Detail laut Design-Abnahme 20.08.), kommt von dort.

### 1.3 ~~NAK-23a~~ — erledigt; Batch-Form berichtigt 21.08.

Der widersprüchliche Bündelungssatz ist in 0.4 entfernt. **Bis zum 21.08.
stand hier die Gegenposition** („1 Batch = N Frames EINER Quelle, Bündelung
nur Broker→Main über einen Wrapper") als „Festlegung bleibt gültig" — das war
falsch. Entwurf §33.1 (seit 0.4, Prüfbericht Befund D) und das gebaute
`eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs` sagen: **ein
`FeatureBatch` ist eine Liste von `(source, frame)`-Einträgen mit höchstens
EINEM aktuellen Frame je Quelle**; Sonde→Broker ist der Sonderfall mit genau
einem Eintrag (Cap 2, replace-oldest, §53.9), nur Broker→Main bündelt die
aktuellen Frames mehrerer Sonden in einem Write; **keine zweite
Wrapper-Ebene**. Zwei Einträge mit derselben `instance_id` sind ungültig —
Leserregel `quelle_doppelt` in beiden handgeschriebenen Lesern
(`schemas/v3/flatbuffers/README.md`). Richtig an der alten Fassung war nur die
Begründung, warum Sonde→Broker nie mehr als einen Frame trägt.

### 1.4 Neu aus Entwurf §0.4 — die Interims-UI ist ein Vertrag

Das Arbeitsmodell „Technik voraus, Design parallel" macht die schlichte
Bedien-UI **prüfbar**, nicht beliebig. In jeder UI-Fassung sichtbar sein
müssen: **Frische/stale · Unsicherheit/Konfidenz · `arming`/`audible_ready` ·
Capability-Degradation · Konfliktauflösung · welche Aktion gerade NICHT aktiv
ist.** Das wandert unten in die T1-Liste — es ist der Punkt, an dem eine
Interims-UI sonst still einen nicht existierenden Zustand vortäuscht.

Betroffen sind **seit 23.08. zwei** Oberflächen (Umschnitt, Kopfzeile):
**Gen** (Seite 1 Übersicht/Befunde/Advisor + **Seite 2 EQ-Zentrale**, die
alle Sonden UND den Master-EQ bedient) und die **Minimal-Rückfallfläche der
einen Sonde** (**Probeeq** — Suna ist in Probeeq aufgegangen; die alte
Suna-Kachel-Arbeit und ihre offenen U6-Punkte gelten für die Rückfallfläche
weiter, soweit sie eine behält). Die funktionale UI kommt aus
`design/docs/funktions-und-bedien-blueprint.md` und den jüngsten Abnahmen unter
`design/abnahmen/`; frühere Figma-Stände sind Verlauf. Beide Gen-Seiten teilen
einen vergrößerbaren 76:43-Rahmen: 760×430 ist das kompakte Minimum und
950×538 die Standardgröße (Entscheid 01.09.2026). Der
Probeeq-Stand 700×420 ist nur noch historisches **Working Design** der
EQ-Fläche (User 23.08.), als eigenständige Voll-UI Verlauf. Offen in NAK-65
bleibt der Umbau und die visuelle Abnahme der minimalen
Probeeq-Rückfallfläche; Ausgangsbasis ist der aktuelle Suna-Entwurf
`Nakama-Design` Node `6:2864` (260×84).
Prüfmaßstab jeder UI-Fassung zusätzlich:
`../design/docs/ui-spielregeln-eq-zentrale.md` und
`../design/abnahmen/2026-09-01-gen-fenstergroesse-und-resize.md`. Vorschlag (nicht
abgenommen): `SONDE-007b` baut keine eigene Kachelgestaltung, sondern den
Vertrag (Zustände, Pflichtinhalt) und nimmt die Gestaltung aus dem
Figma-Stand — Entwurf-Errata (h)/(j).

---

## 2. Die drei Prüfebenen

Das ist der Kern deiner Anforderung. Die Ebene richtet sich danach, **was**
angefasst wurde, nicht danach, wie groß der Diff ist.

### T1 · Selbstaudit — jeder Commit, gleicher Kontext

Feste Liste, nie abgekürzt:

1. **Numerische Ränder:** NaN, ±inf, 0, negativ, Überlauf, Wrap.
2. **Gegenpfad vorhanden?** Save↔Load, Bind↔Unbind, Start↔Stop, Apply↔Revert,
   Subscribe↔Unsubscribe, Install↔Rollback.
3. **Behauptungs-Integrität:** Existiert jede genannte Datei/Funktion/Zeile
   wirklich? (Diese Session hat NAK-20 nur gefunden, weil ich die Zeile
   aufgemacht habe statt sie zu zitieren.)
4. **Lügt der UI-Text?** Zeigt die Anzeige einen Zustand, den es nicht gibt?
5. **Anzeige-Pflichten erfüllt?** (Entwurf §0.4, nur bei UI-Diff) Frische/stale ·
   Konfidenz · `arming`/`audible_ready` · Capability-Degradation · Konflikt ·
   „welche Aktion gerade NICHT aktiv ist".
6. **Audiothread:** Allokation, Lock, I/O, Log — nachweislich keins?

Kosten: Minuten. Findet Fehler, bevor sie ein Prüfer sieht.

### T2 · Frischkontext-Prüfer — jedes Ticket, als Subagent

Ein `evaluator`-Agent **ohne meinen Kontext** bekommt exakt drei Dinge:

- den Diff,
- das Beweismanifest,
- den **Gate-Text des Tickets aus dem Entwurf** (nicht meine Zusammenfassung).

Er antwortet PASS oder NEEDS_WORK. NEEDS_WORK ist **blockierend**, kein
Hinweis. Der Trick ist, dass er meine Zusammenfassung nie sieht — er kann
also nicht meine Formulierung bestätigen, nur den Diff gegen den Plantext.

### T3 · Adversariale Runde — jedes Phasengate, EIGENE Session

Hier lebt „adversiales Prüfen je größerem Implement". Zusammensetzung nach
angefasster Fläche:

| Was die Phase angefasst hat | Prüfmittel |
|---|---|
| C++ Audiothread, DSP, State | `/c-review` — Memory-Corruption, Integer-Overflow, Races; dazu Realtime-Guard- und Blockgrößen-Stresslauf |
| Rust-Broker, IPC, Store | `/rust-review` — unsafe/FFI, Panic-DoS, Nebenläufigkeit; dazu `transport_fuzz` + `store_crash_matrix` |
| Sicherheitsgrenze (Pairing, HMAC, Impersonation, DPAPI, Pipe-DACL) | `/security-review` + handgeschriebene Negativvektoren |
| **jede** Phase | **Codex gegen dieselbe Phase** — zweites Modell, andere Trainingsverteilung |

**Warum Codex zwingend dazugehört:** Das Projekt hat Mode-Collapse schon
einmal teuer bezahlt (Geschmacksprofil §4). Zwei Läufe desselben Modells
finden korrelierte Fehler. Ein zweites Modell ist die einzige billige Quelle
echter Unabhängigkeit.

#### Die Falsifikations-Vorgabe

Ein Prüfer, der „prüfe, ob X stimmt" bekommt, bestätigt X. Deshalb bekommt er
**Bruchaufträge**. Die acht harten Gates aus Entwurf §49.2 sind die
Zielscheiben:

> „Finde einen Eingabepfad, der Gate 1 verletzt: Passive Probe, neutrales Main
> oder neutraler Hard-Bypass verändert einen gültigen Audiopuffer. Antworte
> mit dem konkreten Pfad oder mit ‚kein Pfad gefunden' — nicht mit einer
> Einschätzung."

Pro Phase die passenden Gates (Tabelle in §5).

#### Und die Prüfer selbst werden geprüft

Erfahrungswert dieses Projekts: **~25 % der AI-Auditbefunde waren falsch.**
Mehr Prüfung erzeugt sonst nur mehr Rauschen. Regel:

- **Jeder Befund wird gegen die Quelldatei verifiziert, bevor gehandelt wird.**
- Bestätigter Befund ⇒ **gefixt** oder als NAK-Zeile in `offene-punkte.md`.
- Widerlegter Befund ⇒ **eine Zeile im Gate-Protokoll, warum er falsch war.**
- **Kein Befund verschwindet still.** Das macht die Prüfung auditierbar statt
  zeremoniell.

### Das Beweismanifest

Pro Ticket `docs/beweise/SONDE-0NN.md`, eine Tabelle:

| Behauptung | Befehl | **Rohe Ausgabe** | Datum |

Harte Regel: **Eine Behauptung ohne eingefügte Rohausgabe ist ein
gescheitertes Ticket, kein fast fertiges.** Das ist der bestehende Standard
„ausgeführt und gesehen" — nur als Datei, die ein Fremder lesen kann.

---

## 3. Sessionplan

Reihenfolge weicht in **einem** Punkt vom Entwurf ab: `SONDE-004` ist
vorgezogen (NAK-22 — höchstes Ausfallrisiko, billigste Messung, größte
Umfangswirkung).

> **Die Spalte „Prüfung" ist Verlauf, kein Status (Nachtrag 30.08.2026, PR1 Befund D-6).**
> Der verbindliche Stand wird gerechnet und steht in `docs/PLAN-STAND.md`: Beleg da =
> gebaut, Urteilsmarke im Manifest = abgenommen (`docs/plan/LIES-MICH.md`). Die Zellen
> unten beschreiben den Stand ihres jeweiligen Tages und werden nicht laufend
> nachgezogen; wo eine Zelle der gerechneten Marke bis zum 30.08.2026 widersprach, steht
> die Berichtigung datiert dahinter. Zwei Statusquellen zu pflegen war der Fehler — hier
> gilt ab jetzt die gerechnete.

### Vorlauf

| # | Inhalt | Prüfung |
|---|---|---|
| ~~**S0**~~ | ~~Beweis-Runner `tools/beweise.ps1`, Manifest-Vorlage, `docs/beweise/`~~ — **erledigt 20.08.**, T1 gefahren | T1 ✅ |

### P0 · Bestand einfrieren, Hostgrenzen beweisen

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| ~~S1~~ | `SONDE-004a` | ~~Wegwerf-Target mit zwei deklarierten Aux-Bussen + Impulsharness~~ — **erledigt 20.08.**, Selbsttest 41/41, Manifest `docs/beweise/SONDE-004a.md` | T1 ✅ |
| — | — | **👤 User-Termin A (FL):** Aux-Layout, Kanalreihenfolge, Recall, PDC-Impulse | — |
| ~~S2~~ | `SONDE-001` + `002` | ~~Identität einfrieren~~ — **gebaut 20.08.**: Manifest aus dem gebauten `moduleinfo.json`, `EqCopIdentityTest` (63 Prüfungen: reservierte CIDs nachgerechnet, VST2-Pfad negativ bewiesen, **Freeze auch an der CMake-Quelle**), Schema-1-Goldens für alle vier Rollen. Die `.flp`-Legacy-Fixture bleibt ausdrücklich offen (nur in FL erzeugbar → Termin B). | T1 ✅ · T2 ✅ (Runde 1 und 2 NEEDS_WORK, **Runde 3 PASS** — `docs/beweise/SONDE-001-002.md` §5) |
| ~~S3~~ | `SONDE-003` | ~~JUCE-Bridge-Patch: Context-Anwesenheit, Parameterpunkte, Buslatenz, Quellhash-Gate~~ — **gebaut 21.08.** (`1e91d54`): Patch mit neun Anker-genauen Stellen, Quellhash-Gate mit **allen drei Zweigen vorgeführt** (unberührt patcht + misst nach · gepatcht No-Op · fremd Bauabbruch), `EqCopHostContextTest` **91/91**, Kanon von 5/5 auf **6/6** gewachsen, `pluginval` Strenge 8 SUCCESS. Manifest `docs/beweise/SONDE-003.md`. **T2 brauchte drei Runden** — Runde 1 und 2 waren NEEDS_WORK und fanden je einen echten Vertragsbruch (der zweite eine Regression aus der ersten Nacharbeit); beide gefixt, beide Riegel nachweislich zum Fallen gebracht. | T1 ✅ · T2 ✅ |
| ~~**S3b**~~ | — | ~~Nachtrag 21.08. — der Plan hat Termin B unterschätzt: eine Klickliste allein misst nichts.~~ — **gebaut 21.08.**: `EqCopHostProbe` (`NkHp`, Wegwerfware) ist das erste Ziel, das die Hostbrücke BENUTZT. Misst Context-Anwesenheit, Gültigkeitsbits (immer/manchmal/nie), Zeitsprünge für Seek·Loop·Smart Disable **mit Fehlalarm-Riegel**, Offline-Render, float/double, Presentation-Latency und samplegenaue Automation → JSON. Selbsttest **85/85** (89 nur mit PNG-Ziel, NAK-34), `pluginval` Strenge 8 SUCCESS, Kanon von 6/6 auf **7/7**. Klickliste `eq-copilot/docs/FL-TERMIN-B-HOSTZEIT.md` (inkl. Legacy-`.flp`-Fixture aus §54 Punkt 2). Manifest `docs/beweise/SONDE-003b.md`. **T2 brauchte vier Runden** — Runde 1 fand einen Blocker (der Editor schnitt genau die Automationszeilen ab; ich hatte das Gerät nie gerendert), Runde 2 eine Prüfung, die nicht fehlschlagen konnte, Runde 3 einen Zähler, der Blöcke statt Änderungen zählte. Alle gefixt, jeder Riegel nachweislich zum Fallen gebracht. | T1 ✅ · T2 ✅ |
| — | — | ~~**👤 User-Termin B (FL):** Live/Stop/Seek/Loop-Straddle/Render/Smart Disable~~ — **gemessen 22.08.** (User: Aufbau, Smart disable, Export, Bericht; Claude: Transport über FL-MCP). `docs/beweise/termin-b/` | — |
| ~~S4~~ | — | ~~Auswertung beider Termine → Capabilityreport mit Rohmessdaten~~ — **gebaut 22.08.**: `eq-copilot/identity/host-capabilities-fl-v1.json` (zehn §53.6-Bits in der v3-Vertragsform, jedes mit Rohfeld), Kanon-Bein **A13** `pruefe_host_capabilities.py` (48 Prüfungen: Vertragsform, jede „feld = wert"-Angabe des Reports gegen die Rohdateien, Smart-Disable-Fenster, Versionen). **2 supported, 8 unsupported** — `sample_accurate_automation` unsupported, weil FL nie >1 Punkt je Block liefert (Puffer-Zerteilung bis 1 Sample); `presentation_latency`, `aux_priority_sidechain` und `aux_compare_pre` nach T2 herabgestuft (Meldung ohne Impulsgolden; Aux-Wege ohne PDC-Last und ohne unterscheidbare Kanalreihenfolge — Termin A2, NAK-44); `contribution_aux` ungemessen ⇒ unsupported (§54). Manifest `docs/beweise/SONDE-004.md`. | T1 ✅ · T2 Runde 1 NEEDS_WORK → nachgearbeitet · **Runde 2 PASS** (zehn nicht-blockierende Befunde geschlossen) |
| ~~**G0**~~ | — | ~~**Gate:** `/c-review` (Bridge-Patch) + Codex. Falsifikation: Gate 1, Gate 5~~ — **gefahren 22.08., Urteil PASS** (`docs/beweise/G0.md`): beide Bruchaufträge gescheitert, die P0-Kernfläche (Patch, Quellhash-Gate, `NakamaHostBridge.h`, HostProbe) trägt **keinen** überlebenden Befund. Codex' einziger Gate-1-Kandidat (gecachtes `editorOffen` je Block) an der Quelle **widerlegt** — der Neutralpfad wird nicht übersprungen, er greift einen Block später, und `editorOffen` ist zwischen Editor-Konstruktor und -Destruktor sauber gepaart. 22 bestätigte Befunde außerhalb P0 (Altbestand `PipeClient`/`PluginProcessor`, Build-Härtung, Werkzeuge) als NAK-47/48/49 erfasst. **Damit ist P0 vollständig und der Schließungsvorbehalt §65 für `SONDE-005` fällt.** | **T3** ✅ |

`SONDE-003` ist das riskanteste kleine Ticket des ganzen Plans — ein
idempotenter Patch am gevendorten JUCE mit Quellhash-Abbruch, der bei jedem
JUCE-Update neu bewiesen werden muss. Eigene Session, eigener Prüfer.

### P1 · Verträge, State, neutrale Shells

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| ~~S5~~ | `SONDE-005a` | ~~v3-JSON-Schemas + Bandgitter + gültige/ungültige Fixtures~~ — **gebaut 21.08.**: `schemas/v3/` (17 der 25 Nachrichtenfamilien aus §33.3 definiert, 8 namentlich **reserviert** mit Eigentümerticket), beide Bandgitter als eingefrorene Zahlenfixture (221 + 64, IEC 61260-1 mit Halbschritt — von vier denkbaren Konventionen trifft nur diese die 221 aus §33.2), Quantisierungsvertrag mit 61 Vektoren, **153 Fixtures (Stand nach T2-Runde 2; Baustand-Zeile trug 131)** mit handgeschriebenem Manifest, und **drei** Prüfbeine (`jsonschema`-Referenz · `EqCopSchemaTest` · `contract_cross_language`). Kanon von 7/7 auf **8/8** gewachsen, `pluginval` Strenge 8 SUCCESS. Manifest `docs/beweise/SONDE-005a.md`. Der Korpus fand drei echte Fehler (geschachtelter Discriminator, Nicht-Objekt-Wurzel, eine falsche handgeschriebene Erwartung) und eine dokumentierte Abweichung zwischen den Beinen (RFC 4627 vs. 8259). **Schließungsvorbehalt §65:** gilt bis S4 als *vorbereitet*, nicht *geschlossen*. | T1 ✅ · **T2 offen** — kein PASS im Manifest. Runde 1 NEEDS_WORK (sechs Vertragsbrueche gegen den Plantext, ein gemessener Cross-Language-Bruch am Zahlenbereich), Runde 2 NEEDS_WORK (der daraufhin gebaute Riegel befragte auf der C++-Seite denselben ueberlaufenden Leser, gegen den er schuetzt), Runde 3 am 21.08. zusammen mit S6 NEEDS_WORK (`docs/beweise/SONDE-005b.md` §6). Alle Befunde nachgearbeitet (`SONDE-005a.md` §6, `SONDE-005b.md` §6.3); ein abschliessendes Pruefer-Urteil steht aus. Bis 21.08. stand hier „T2 ✅" — gegen VORLAGE-Regel „Haekchen erst NACH dem Lauf". **Berichtigt 30.08.2026: abgenommen, T3 PASS 2026-08-28** (`docs/PLAN-STAND.md`). |
| ~~S6~~ | `SONDE-005b` | ~~FlatBuffers (Feld-IDs!), gepinntes `flatc`, Codegen-Drift-Test~~ — **gebaut 21.08.**: `.fbs` mit explizitem `id` an jedem der 47 Felder, `flatc` auf einen **Commit** gepinnt (der Upstream fuehrt fuer 25.12.19 zwei Tags), Compiler/C++-Header/Rust-Crate aus derselben Quelle, **Drift 0** und beide Riegel beim Fallen vorgefuehrt. Zwei handgeschriebene Binaerleser gegen 40 Fixtures mit handgeschriebenem Manifest; ein Byte bringt beide mit demselben Pfad und derselben Regel zum Fallen. Kanon 12/12 -> **14/14** (seit A11: 15). Manifest `docs/beweise/SONDE-005b.md`. | T1 ✅ · **T2 offen** — Runde 3 gelaufen 21.08., Urteil NEEDS_WORK (`SONDE-005b.md` §6.1): ein Byte-Mutations-Fuzz fand, dass der C++-Leser ungueltiges UTF-8 als gueltig nahm und der Prozess daran starb (Heap-Korruption), dazu NUL-Laengen, Puffer unter 8 Byte (Rust-Panic) und zehn weitere Befunde an Manifest, Riegeln und Pins; alle bestaetigten gefixt (§6.3), Binaerfixtures 40 → 47. Kein PASS eingetragen. **Berichtigt 30.08.2026: abgenommen, T3 PASS 2026-08-28** (`docs/PLAN-STAND.md`). |
| ~~S7~~ | `SONDE-006` | ~~State-Schema 2, Parameterbestand, reine Schema-1-Migration~~ — **gebaut 22.08.**: `NakamaState` schema 2 (Common/MainProject/Parameters; Dsp/Pairing reserviert) mit Kind-Matrix, reine Schema-1-Migration mit **4 bytegleichen Goldens**, unbekanntes Major ⇒ read-only mit Originalbytes (17 Fälle gemessen), Host-Dirty über `withNonParameterStateChanged` (vorher fehlte jeder `updateHostDisplay`-Aufruf im Plugin), Duplicate sichtbar. Parameterbestand **109 IDs** handgeschrieben (`schemas/state/nakama-parameter-v1.json`), C++-Tabelle deckungsgleich gemessen. `state_hash` = SHA-256 über RFC-8785-Kanon mit **eigenem JSON-Leser** (JUCE flusht Subnormale, verweigert `""`); drei Beine (C++ 130 Prüfungen, Python `rfc8785`, Rust `serde_json_canonicalizer`) bytegleich gegen einen Korpus, dessen Zahlenvektoren den **vom RFC gedruckten** Text tragen. Kanon 15 → **17** (B2 + A12). Manifest `docs/beweise/SONDE-006.md`. | T1 ✅ · T2 ✅ (**PASS** 22.08., neun nicht-blockierende Befunde nachgearbeitet, Manifest §5/§6) |
| ~~S8~~ | `SONDE-007a` | ~~**Gemeinsamer Kern ohne `JucePlugin_*`-Konstanten**~~ (NAK-23b) — **gebaut 22.08.**: `NakamaKern` als echte Static-Lib mit den vier geteilten Quellen, einmal übersetzt statt je Ziel (acht Verbraucher). Die Bauform ist **gemessen, nicht angenommen** — drei Wegwerf-Experimente: `$<COMPILE_ONLY:>` streift `INTERFACE_SOURCES` nicht ab; zwei Kopien derselben JUCE-Modulquelle linken zwar sauber, halten aber nur bei deckungsgleicher Übersetzung; die **Kopf-Fassade** (Includes+Defines aus dem Modulziel abgeleitet, Quellen nicht geerbt) übersetzt die Modulquelle genau einmal. Vier Riegel mit Arbeitsteilung, **jeder beim Fallen vorgeführt**: K1 Präprozessor (46 Makros) · K2 Linkhülle per Regex · K2b gleiche JUCE-Konfiguration wie der Verbraucher (aus dem Selbstaudit) · K3 = Kanon-Bein **A14**, misst das **Artefakt**. A14 trägt seine eigene Gegenprobe — und die hat die erste Fassung des Beins widerlegt (CIDs liegen als 16 rohe Bytes in COM-vertauschter Ordnung, nicht als Hextext). Kanon 18 → **19**. Identität, Passthrough, Golden, Markierung unverändert; `pluginval` Strenge 8 SUCCESS. Manifest `docs/beweise/SONDE-007a.md`. | T1 ✅ · **T2 gefahren 23.08., Urteil NEEDS_WORK** (`SONDE-007a.md` §5): die zentrale Behauptung hielt unter eigener Messung (0 `JucePlugin_`-Defines im Kern, Fassade trägt, alle acht nachgezählten Zahlen stimmen), aber fünf Befunde — allen voran eine **echte Regression des Änderungssatzes**: als eigene Lib erbt der Kern die PUBLIC-Empfehlungsschalter seiner Verbraucher nicht mehr und übersetzte als einziger Code im Baum unter `/W1` statt `/W4`. Alle fünf am selben Tag geschlossen (§6), fünfter Riegel **K2c** gebaut und beim Fallen vorgeführt, Kanon danach 19/19, `pluginval` 8 erneut SUCCESS. **Kein PASS auf den neuen Stand** — ein zweites Urteil steht aus. **Berichtigt 30.08.2026: abgenommen, T3 PASS 2026-08-30** (`docs/PLAN-STAND.md`). |
| S9 | `SONDE-007b` | Drei Ziele, Lifecycle-Klassifikation, Installer-Manifest — **Abschnitte 1+2 von 3 gebaut 23.08.**: Identität aus `plugin-identities-v1.json` statt aus dem Bauskript (NAK-52 geschlossen, `8e32baf`), dann `Nakama Suna.vst3` (`NkPr`) und `Nakama Probeeq.vst3` (`NkAc`) aus **einer** geteilten Quelle über zwei dünne Target-Schichten. Die in S2 nur gerechneten CIDs sind erstmals an Artefakten gemessen (§53.5 „P1 verifiziert das erste Moduleinfo"), kein Bundle trägt eine fremde Ziel-CID. K2b/K2c messen jetzt gegen **jeden** Kern-Verbraucher (12). Grundgesetz je Bundle bewiesen (A15/A16: Passthrough bitgleich, 0 Latenz, kein Tail, kein Hostparameter); der Gegenpfad speichern↔laden fand **zwei echte Fehler**. Kanon 19 → **21**, `pluginval` 8 an beiden neuen SUCCESS. **Abschnitt 3 gebaut 23.08.** (`42bfe6e` + `ff0e0b8`): `state::Lebenslauf` macht §53.5 zu Code — `unclassified` und audio-neutral beim Laden, `legacy` für Schema-1 `sensor\|pre\|post`, `main` für `hub`/bestätigten Schema-2-Main-State, `main` an einer frischen Instanz nur nach geöffnetem Editor **und** Rollenwahl. „Ein Scannerlauf klassifiziert nicht" ist gemessen, nicht behauptet. Der Automat hat Zähne: die Hör-Markierung fällt unter „audio-neutral" (an Audio gemessen — dieselbe Markierung färbt vorher kein Sample, nachher schon), `darfBrokerStarten()` ist der Haken für SONDE-010. Dazu das Installer-Manifest (Vertrag + Auslieferung + `Install-Nakama.ps1` mit Rückweg und NAK-41-Riegel) — **NAK-32 geschlossen**. Kanon 21 → **23** (A17, B8), `pluginval` 8 auch am Main-Bundle SUCCESS. Manifest `docs/beweise/SONDE-007b.md` | **Berichtigt 30.08.2026: abgenommen, T3 PASS 2026-08-30** (`docs/PLAN-STAND.md`) |
| S9b | `SONDE-007c` | **Suna-Ziel stilllegen** (vorgezogen aus S28b, User-Entscheid 28.08.2026, `design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md`): `NakamaSuna_VST3` aus dem Bau, A15 verabschiedet sich deklariert aus dem Kanon, Installer-Manifest auf zwei Bundles plus Broker, Suna-Eintrag in `plugin-identities-v1.json` als stillgelegt markiert — eingefrorene Kennungen von Gen und Probeeq unberührt (NAK-30), Rückweg vollständig. Gate: Kanon grün ohne A15, A17 mit zwei Bundles, `pluginval` SUCCESS an beiden, keine fremde CID. Danach gilt §55 Klausel 1 für **beide** Ziele: scannen und laden in FL | **Stand 30.08.2026: abgenommen, T2 PASS 2026-08-30** |
| **G1** | — | **Gate:** `/c-review` + `/rust-review` + Codex. Falsifikation: Gate 1, Gate 7 | **T3** |

S8 ist der Umbau, den der Entwurf unterschätzt: Die heutige Begründung gegen
eine geteilte Static-Lib steht wörtlich im CMake-Kopf. Wenn der geteilte Kern
auch nur **eine** `JucePlugin_*`-Konstante sieht, bekommen zwei Bundles die
Identität des dritten — und genau das Identitäts-Golden aus S2 fällt.
Deshalb getrennt von S9.

**Nachtrag 22.08. (gebaut):** Die Warnung stimmte, aber der eigentliche
Widerstand lag woanders — nicht bei den Identitätskonstanten (die hält schon
K1 auf), sondern bei JUCEs Modulsystem: Module sind INTERFACE-Bibliotheken,
deren `.cpp` in jedes konsumierende Ziel hineinkompiliert wird. Eine
Static-Lib, die sie linkt, trägt eine zweite Kopie von `juce_core.obj`. Die
gebaute Antwort ist eine Kopf-Fassade, die aus den Modulzielen **ableitet**,
statt sie zu linken. Was S9 daraus mitnimmt: der Kern übersetzt gegen
JUCE-Köpfe, die Implementierung kommt vom verbrauchenden Ziel — die drei
Bundles müssen dieselben Module linken, sonst fehlt dem Kern zur Linkzeit
ein Symbol.

### P2 · Messkern, IPC v3, Store — die größte Phase

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| ~~S10–11~~ | `SONDE-008` | ~~`StampedAudioQueue`, Ein-Block-Quarantäne, fixed-memory Loudness~~ — **gebaut 23.08.**: der `AbstractFifo` im Audiothread ist ersetzt durch zwei feste SPSC-Ringe (Samples + Deskriptoren), die einen Block **ganz oder gar nicht** veröffentlichen; Teilblöcke gibt es nicht mehr. Ein-Block-Quarantäne versiegelt erst mit bewiesener Fortsetzung; ⚠️ eine **stehende** Projektzeit ist bewusst KEIN Bruch (FL zerteilt Puffer bis 1 Sample — Termin B; ohne diese Zeile verlöre die Analyse an jeder Teilungsgrenze still einen Block, NAK-56). `prepareToPlay` ruft kein `fifo.reset()` mehr in einen laufenden Leser hinein, sondern meldet einen Neuanlauf, den der Produzent einlöst. Der fixed-memory `LoudnessAccumulator` ersetzt `kZellen` und den 4×/s neu allozierten Zweitdurchgang: absolutes Gate als kompensierte Laufsumme (exakt), nur die Auswahl fürs relative Gate aus einem Histogramm, `unsicherheitLu()` macht die Quantisierung auslesbar. **Die Hostbrücke aus S3 ist damit erstmals im Produkt verdrahtet** — nur sie kann „Context fehlt" ausdrücken, und ohne das wäre jeder Zeitstempel geraten (NAK-24 geschlossen). Dazu der User-Entscheid vom 22.08. (Hub `U10`, „Nein, nur mit Signal"): das fail-open `∨ ¬hatTransport` der Hör-Markierung ist gefallen (NAK-35 geschlossen) — in FL ohne Wirkung, weil der Zweig dort tot war. Kanon 24 → **26** (B4 `EqCopQueueStressTest` 68/0, B9 `EqCopLoudnessGoldenTest` 66/0), Beweislauf GRÜN 26/26 beglaubigt, `pluginval` 8 an allen drei Bundles SUCCESS. Der EBU-Golden fand **zwei echte Fehler**, beide im adversarialen Teil. Manifest `docs/beweise/SONDE-008.md`. **ABGENOMMEN 24.08.**: T2 Runde 2, zweiter frischer Prüfer (Manifest §10) — alle vier Befunde geschlossen UND jeder von einem Bein gedeckt, das der Prüfer mit einer eigenen Mutation zum Fallen brachte (M1–M8, jede echt gebaut, jede byteweise zurückgestellt); Kanon 28/28 grün und beglaubigt. Drei neue Befunde, alle niedrig und keiner auf dem heutigen Ziel erreichbar → NAK-71; zwei Werkzeug-Landminen → NAK-72/73. Was das PASS nicht deckt, steht in §10.7 (vor allem: kein Lauf in FL, NAK-56; der gepatchte Wrapper ist nie gefahren). | T1 ✅ · **T2 ✅ PASS 24.08.** — abgenommen |
| ~~S12–13~~ | `SONDE-009` | ~~FeatureEngine v2: Zeit-, Validity-, Event-, Bandverträge~~ — **gebaut 23.08.** (`f14924a` · `357786e` · `133526e`): neun Grenzarten, jede mit eigener Ursache und eigenem Prüffall; an jeder fällt **jedes** offene Fenster — FFT-Fenster **beider** Auflösungsstufen (16384/4096), Loudness-Zelle, 3-s-Historie, Korrelationsfenster, Fluss-Vorgänger **und der K-Filterzustand**. Zwei Stufen sind nicht Genauigkeit, sondern die Beweisfläche: zwei gleichzeitig offene Fenster von 341 ms und 85 ms. Der v3-Bandvertrag liegt als **eingefrorene Bitmuster** im Code (erzeugt + zwei unabhängige Riegel), alle 61 Quantisierungsvektoren treffen. **NAK-29 geschlossen im Erzeuger** (6 Fälle, 6 Nummern), **NAK-56-Werkbankhälfte geschlossen** (`nakamaBlockEmpfangen()` am echten Prozessor, Gegenprobe Brücke 0x7f gegen Playhead 0x3). Kanon 26 → **28** (B5 mit 120 Prüfungen, A19). Bitidentität gemessen: Null-, Golden-, Markierungs- und QueueStress-Test unverändert grün. **Sieben Mutationen vorgeführt** — und eine davon (M7) schlug beim ersten Anlauf NICHT aus: der Prüfpunkt war blind, weil LUFS-S über 3 s mittelt und die betroffene Zelle herausgelaufen war (Manifest §5.1). Manifest `docs/beweise/SONDE-009.md`. | T1 ✅ · **Berichtigt 30.08.2026: abgenommen, T2 PASS 2026-08-24** (`docs/PLAN-STAND.md`) |
| S14–15 | `SONDE-010` | v3-Control-/Telemetry-Clients (C++) + Rust-Envelopeparser | **Stand 30.08.2026: abgenommen, T2 PASS 2026-08-30** |
| **PR1** | `Planprüfung` | **Zwischenhalt (User-Wort 29.08.2026):** Implementierung anhalten, den Gesamtplan gegen alle bisherigen Entscheide auf Folgebrüche und gegen den Funktionsstandard vergleichbarer Plugins anderer Hersteller prüfen; Anpassungen direkt in Plan, Bauaufteilung, Entwurf und Tickets einarbeiten. Liegt nach G1-PASS und vor `SONDE-011`, weil dort Datenformen festgeschrieben werden (Begründung in `docs/plan/plan.json`) | **T3** |
| S16–17 | `SONDE-011` | Coordinator, monotone Eviction, SQLite-Migration 1, Outbox — **dazu seit PR1 Runde 1 (30.08.2026, Codex-Befunde B1–B4) als ERSTER Lieferumfang eine v3-Vertragsrunde vor R0:** NAK-28 (`unsubscribe_session` — §33.3 von 25 auf 26 oder dokumentierter Gegenpfad über die Trennung), NAK-29 (bedingte Pflichten im Transportstempel als diskriminierte Union im Schema; im Erzeuger seit SONDE-009 gesperrt), NAK-40 (Abbildung einer nicht-hex32-`instance_id` auf die v3-Adresse — `SONDE-010` hat sie entgegen der Registerzeile nicht entschieden), NAK-59 (Band-Stereo im FlatBuffers-Frame) und NAK-10 (Broker liest das Hörmarkierungs-Flag). Dazu **fünf reservierte Namen ohne Nutzlast**: ein Deskriptor je Gen-Eingang für `post_fader_contribution` (NAK-79 ist am 24.08. entschieden — keine Sondenklasse trägt die Position, Empfänger ist Gen; das Instanzfeld bleibt fail-closed, Nutzlast mit dem Aux-Bus in `SONDE-018`), `state_report.dsp` samt Bestätigung der tatsächlich angewandten Werte (Nutzlast S26–28 / S29–31), der Betriebszustand des Sonden-EQ (Eigentümer S29–31, **ohne** Anzeigezusage) und optionale, hostgelieferte Felder für Bus-Name und Mixer-Index im `probe_descriptor`, dazu der reservierte Familienname für Referenz-Nachbilden (Antwort U19, 30.08.2026: „Bleibt hinten“). Ferner der Broker-Autostart (NAK-13) mit Start **und** Stopp im selben Änderungssatz | T1+T2 |
| **G2** | — | **Gate:** volles Programm — `/c-review` + `/rust-review` + `/security-review` + Codex. Falsifikation: Gate 1, 2, 5 | **T3** |

S10 ersetzt den `AbstractFifo` im **Audiothread**. Das ist der gefährlichste
Eingriff der ganzen Phase. Der Prüfer bekommt hier zusätzlich einen
Blockgrößen-Fuzz über `maximumExpectedSamplesPerBlock` hinaus.

Ab hier: **R0 · Contract/Internal** erreicht.

### P3 · Passive Landkarte → R1

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| S18–19 | `SONDE-012` | Join, Führung, Frische, Messpunktwahrheit, Fehlerzustände — **dazu seit PR1 (30.08.2026, Befunde D-2/D-5):** Messung des Host-Kanalkontexts im echten FL (VST3 `ChannelContext::IInfoListener`, JUCE `updateTrackProperties`), das zugehörige Capabilitybit und der Vertragsort für Bus-Name und Mixer-Reihenfolge des Sonden-Durchschalters (Entscheid 23.08., Festlegungen 11/12) — oder der am 30.08.2026 entschiedene Rückfall (**Antwort U20, „Eigener Name"**: der je Sonde vom User vergebene Name aus dem heutigen `label`-Feld — User-Wort, untrusted, Länge begrenzen; ein vom Host gelieferter Bus-Name ersetzt ihn automatisch und hat Vorrang, Mixer-Nummer und Verbindungsreihenfolge entfallen als Rückfall). `probe_descriptor` ist nicht additiv, die Erweiterung ist eine v3-Versionierung. **Berichtigt in PR1 Runde 1 (30.08.2026, Befund B4):** der zuvor hier stehende „EQ-Zustand je Sonde als anzeigbare Wahrheit" ist gestrichen — der Entscheid vom 23.08. (Interview, Fragen 13/16) bindet den roten/grünen EQ-Punkt an Probeeqs lokale Rückfallfläche, nicht an Gens Landkarte. Das Vertragsfeld bleibt als **Reservierung** in der v3-Vertragsrunde von S16–17, Eigentümer S29–31, ohne Anzeigezusage. **Antwort U17.5 (30.08.2026, „Ja, auf der Übersicht“):** die seit `SONDE-008` gemessene Lautheit wird sichtbar — je Quelle ein Wert auf Gens erster Seite, nicht auf der EQ-Fläche; mit ihr die Konfidenz aus `unsicherheitLu()` (Entwurf §0.4/§1.4). Beantwortet die Anzeigefrage aus NAK-57 für diesen einen Wert, NAK-57 bleibt im Übrigen offen | T1+T2 |
| **G3** | — | **Gate:** `/rust-review` + Codex + 60-min-Soak. Falsifikation: Gate 7 | **T3** |

### P4 · Vergleichsevidenz → P5 · Ursachen → R2

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| S20–22 | `SONDE-013` | Dynamik/Stereo/PRE-POST/Passage + manueller Experimentkern | T1+T2 |
| **G4** | — | **Gate:** `/c-review` (DSP) + Codex. Falsifikation: Gate 5, 6 | **T3** |
| S23–25 | `SONDE-014` | Intent, CauseHypothesis, Proposal, AssistantStep + Evaluationskorpus — **dazu seit Antwort U16 (30.08.2026, „Ja, erste Fassung")** der Datenweg für die Überdeckungsanzeige: die Überdeckung als laufender, je Frequenzbereich auflösbarer Wert mit Anzeigekadenz, nicht nur als Befundsatz. Braucht sie ein eigenes v3-Feld, gehört der Name in die Vertragsrunde von S16–17 (liegt vor R0). Die farbige Zone selbst ist S31b | T1+T2 |
| **G5** | — | **Gate:** Codex + adversariale Gegenbeispiele. Falsifikation: Gate 6, 7, 8 | **T3** |

G5 ist das ungewöhnlichste Gate: Der Prüfer soll **eine falsche starke
Ursachenbehauptung provozieren** — korrelierter Distraktor, Parent-Duplikat,
verschobene Passage. Enthaltung („mehr Daten nötig") ist das gewünschte
Ergebnis, nicht ein Treffer.

**R2 · Passive Beta** — 9 von 12 Kernfunktionen, Audio vollständig passiv.
**Das ist der Punkt, an dem das Produkt zum ersten Mal wirklich nützt.**

### P6–P7 · Aktiver Kern: EQ in Sonde und Gen → R3

**Umschnitt 23.08.** (Kopfzeile; NAK-64): Der EQ rechnet in der Sonde und
als Master-EQ in Gen, bedient wird zentral auf Gens Seite 2. Der Fernweg
(`SONDE-016/017`) ist damit der **Hauptbedienweg**, nicht mehr der
Nebenpfad; die EQ-Bedien-UI entsteht **einmal** (S31b) statt je App.

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| S26–28 | `SONDE-015` | Lokaler Active-DSP, Bankpool, State/Automation, A/B — **plus Layout v2** (neue IDs, nie umgewidmet): `mix` je Instanz, die Schutz-Zonen und **Auto-Gain** im gespeicherten Zustand (Entscheide 23.08. Interview R2/R3 und **24.08.**, `design/abnahmen/2026-08-24-auto-gain-schalter.md`: Schalter AUTO neben OUTPUT, Standard aus — nachgetragen mit PR1, Befund D-3) — **dazu seit PR1 Runde 1 (30.08.2026, Befund B3) die Nutzlast von `state_report.dsp`:** der bestätigte DSP samt Schutzgrenzen, dessen Name S16–17 reserviert. Entwurf §33.5 verlangt, dass Broker/Main ihn per `state_report` lesen; heute trägt `state_report` (`eq-ipc-v3.schema.json`, `additionalProperties: false`) nur Version, Revision, Hash, Aufnahmezustand und Undo-Tiefe. **Antwort U15 (30.08.2026, „Nein, dauerhaft ohne“):** Nakama bleibt dauerhaft minimalphasig — kein zweiter, linearphasiger Modus, kein gespeicherter Phasenwert, kein Umschalter und **keine** Namensreservierung; die gemeldete Latenz bleibt null, S28b und Prüftor G6 bleiben unverändert. **Antwort U18 (30.08.2026, „Beides“):** Layout v2 bekommt zusätzlich den Platz für ein eigenes **Preset-Objekt** — eine Voreinstellung ist nicht der Projektzustand, sie braucht eigene IDs und eine eigene Version. Sie trägt Klanginhalt (Bänder, Bypass, `mix`, Schutz-Zonen, Auto-Gain) und **nie** Identität (Zieladresse, `instance_id`, Bus-Name, `label`); dieselbe Grenze gilt beim Kopieren von Sonde zu Sonde, das über die Fernsteuerung aus S29–31 läuft, nicht über einen neuen Sonde-zu-Sonde-Kanal | T1+T2 |
| S28b | `SONDE-015b` | **Gen-Master-EQ:** derselbe Kern auf dem Master (aus = beweisbar bitidentisch, das NullTest-Bein wächst mit) (die Stilllegung des Suna-Ziels ist am 28.08.2026 nach S9b `SONDE-007c` vorgezogen) — **dazu seit PR1 (30.08.2026, Befund D-1) die versionierte Zustands-Erweiterung für `plugin_kind = main`:** die Kind-Matrix in `schemas/state/nakama-state-v2.md` §2.1 verbietet `main` heute `Parameters` **und** `Dsp`, `nakama-parameter-v1.json` führt den Bestand nur für `active_probe`. Ohne diesen Ort hat Gens EQ weder Hostautomation noch `state_hash`, Recall oder Undo-Ring. Versionierung mit Beleg, kein Edit: Writer, Leser, Migration, Goldens und Kanon-Bein im selben Änderungssatz; eingefrorene Class-IDs unberührt (NAK-30) | T1+T2 |
| **G6** | — | **Gate:** `/c-review` **max effort** + Codex + ThreadSanitizer-Äquivalent + Worst-Case-CPU — deckt seit dem Umschnitt auch Gens Master-Pfad. Falsifikation: Gate 1, 2, 3 | **T3** |
| S29–31 | `SONDE-016` + `017` | Pairing/HMAC (**mit NAK-21-Reihenfolge!**), Lease, Apply/Revert, Active-Compare — als Hauptbedienweg: **Drei-Stufen-Geste** (Preview per Halten · Apply mit 10-s-Lease-Startwert · Confirm), volle manuelle Bereiche **ohne ±3-Cap** (U14), Schutz-Zonen als Ablehnungsgrund + harte Sperre der Bedienwege, **zweistufiger Mix** (ganze App ↔ gewählte Spur) — **dazu seit PR1 Runde 1 (30.08.2026, Befunde B3/B4):** `command_ack` bestätigt die **tatsächlich angewandten Werte**, nicht nur `state_hash` (Entwurf §33.4 verlangt es wörtlich; alle fünf Zweige tragen heute nur `command_id`, `ergebnis`, `state_revision`, `state_hash`, `code`), und der Betriebszustand des Sonden-EQ wird hier geführt, weil Gen ihn für die Fernsteuerung braucht — Technik ohne Anzeigezusage | T1+T2 |
| **G7** | — | **Gate:** `/security-review` + `/rust-review` + Codex + 10 000 Befehlsstress. Falsifikation: Gate 3, 4, 8 | **T3** |
| S31b | `SONDE-020` | **EQ-Zentrale-UI:** Gen Seite 2, Prüfmaßstab `ui-spielregeln-eq-zentrale.md`; dazu die Minimal-Rückfallfläche der Sonde. **Sichtbare Grundlage ist seit dem Entscheid vom 25.08.2026 das Bild `design/assets/rework-basis-2026-08-25/gen-page-2-eq-center.png`** — die frühere Figma-Lieferung aus NAK-65 ist Verlauf und keine Startbedingung mehr (berichtigt mit PR1, 30.08.2026, Befund D-4). Lücken der Bildvorlage werden einzeln vorgelegt, nicht erraten. **Antwort U16 (30.08.2026, „Ja, erste Fassung"):** die Überdeckung zweier Quellen erscheint als farbige Zone im Kurvenbild, Teil der ersten nützlichen Fassung; sie zeigt nur, was S23–25 belegt, und setzt keine eigene Schwelle. **Antwort U17 (30.08.2026, Button für Button):** Band direkt aus dem Spektrum ziehen (U17.1 „Ja“ — erste Reaktion des Users war „VERSTEHE Die frage nicht“, das „Ja“ kam nach erneuter Erklärung), Spitzenwerte festhalten (U17.2 „Ja“), Anzeige einstellbar in Höhe, Feinheit, Geschwindigkeit und Neigung als **reine Darstellung** bei unverändertem Messraster (U17.3 „Ja“) — und **keine** Tonnamen, **keine** Klaviatur (U17.4 „Nein“), womit auch die FL-Oktavschreibweise keine Anzeigefrage mehr ist. **Antwort U18 (30.08.2026, „Beides“):** Preset-Liste (mitgelieferte und eigene, eigene sicher- und überschreibbar) und der Handgriff, eine fertige Kurve von einer Sonde auf eine andere zu übertragen — mit sichtbarer Quelle und sichtbarem Ziel, weil er den Klangzustand der Zielsonde überschreibt. Für das Kopieren gibt es keinen belegten Marktstandard (**Lücke L-3**) und keine Bildvorlage; fehlt die Fläche im Rework-Bild, wird sie einzeln vorgelegt | T1+T2 |

G6 ist der härteste Gate des Plans. Vier vorallokierte Bänke mit lockfreiem
Ownership-Protokoll, Reclaim-ACK über SPSC-Ring, atomarer Blockrandtausch —
das ist die Stelle, an der ein Race nicht crasht, sondern **klickt**. Hier
lohnt sich `/c-review` auf höchster Stufe wirklich.

### P8–P9 · Entmaskierung + Härtung → R4

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| S32–33 | `SONDE-018` | Sidechain-Unmasking — **nur wenn G0/User-Termin A grün war** | T1+T2 |
| **G8** | — | **Gate:** `/c-review` + Codex + Hör-/Stemkorpus. Falsifikation: Gate 5 | **T3** |
| S34–35 | `SONDE-019` | Distribution, Migration, Soak, Privacy, Rollback | T1+T2 |
| **G9** | — | **Gate:** alle acht Gates + pluginval Stufe 8 + 30-min-32-Sonden-Soak | **T3** |

---

## 4. Summe

| Abschnitt | Bau-Sessions | Gates | User-Termine |
|---|---:|---:|---:|
| Vorlauf + P0 | 6 (S0–S4 + Nachtrag S3b) | 1 | 2 |
| P1 | 6 | 1 | — |
| P2 → **R0** | 8 | 1 | — |
| P3 → **R1** | 2 | 1 | — |
| P4 | 3 | 1 | — |
| P5 → **R2** | 3 | 1 | — |
| P6 | 4 | 1 | — |
| P7 → **R3** | 4 | 1 | — |
| P8 | 2 | 1 | — |
| P9 → **R4** | 2 | 1 | — |
| **Gesamt** | **40** | **10** | **2** |

**Bis R2 (das erste wirklich nützliche Produkt): 28 Bau-Sessions + 6 Gates.**
**Bis R4 (voller Sondenkern): 40 + 10 = 50 Sessions** (23.08.: +2 durch den
EQ-Zentrale-Umschnitt — S28b Gen-Master-EQ, S31b EQ-Zentrale-UI; 28.08.: +1 durch S9b
Suna-Stilllegung — in §3 nachgezogen, hier erst mit PR1 am 30.08.2026, Befund D-6 (d)).
Der Zwischenhalt **PR1** ist keine Bau-Session und zählt in dieser Tabelle nicht mit.

Das ist eine **Hypothese mit Nachmessung**, keine Schätzung, die ich
verteidige. **Rekalibrierungspunkt nach G1:** Wenn P0+P1 statt 11 Sessions 16
gekostet haben, wird der Rest mit Faktor 1,5 fortgeschrieben und dieses
Dokument korrigiert — nicht die Realität passend geredet. (Bis 21.08. zählte
die Tabelle 36 ohne den Nachtrag S3b; die erste Nachmessung ist damit schon
da: P0 hat eine Session mehr gebraucht als geplant, weil eine Klickliste
allein nichts misst.)

---

## 5. Falsifikations-Zuordnung der acht harten Gates

Aus Entwurf §49.2. Jeder T3-Prüfer bekommt die für seine Phase markierten als
**Bruchauftrag**, nicht als Prüfliste.

| # | Hartes Gate | wird angegriffen ab |
|---:|---|---|
| 1 | Passive/neutrale Instanz verändert einen gültigen Audiopuffer | G0, G1, G2, G6 |
| 2 | IPC/Broker/DB/UI/KI blockiert den Audiothread | G2, G6 |
| 3 | Preview überlebt Lease, Stop, Render, Reload — oder startet bei Recording | G6, G7 |
| 4 | Remote-Apply umgeht Hard Cap, `base_revision`, Nonce, Schutz, Record-Gate | G7 |
| 5 | Telemetrie steuert samplegenauen Gain / erzeugt hörbares Delta | G0, G2, G4, G8 |
| 6 | Nicht vergleichbares Experiment bekommt ein starkes Siegerurteil | G4, G5 |
| 7 | Standard-Insertprobe wird als exakter Summenbeitrag bezeichnet | G1, G3, G5 |
| 8 | ~~KI-Ausgabe ändert Zahlen, Ziel oder Aktion außerhalb des Proposals~~ — **gegenstandslos seit 21.08.** | ~~G5, G7~~ |

**Nachtrag 21.08.:** Die KI-/Claude-Schicht ist aus dem Produkt (User: „Nein –
raus aus dem Produkt"; Entwurf-Errata (e)). Gate 8 entfällt, Gate 2 gilt ohne
den Teil „KI". G5 und G7 behalten ihre übrigen Gates (6, 7 bzw. 3, 4); der
Bruchauftrag „falsche starke Ursachenbehauptung provozieren" an G5 bleibt —
er zielte auf die deterministische Policy, nicht auf ein Modell.

---

## 6. Regeln, die für alle Sessions gelten

1. **Ein Ticket, ein Commit-Bündel, ein Manifest.** Kein Ticket endet ohne
   `docs/beweise/SONDE-0NN.md` mit roher Ausgabe.
2. **Kein Vorgriff.** Keine Session simuliert eine spätere Capability. Fehlt
   ein Hostbeweis, wird das Capabilitybit **nicht gesetzt** — nicht heuristisch
   ersetzt.
3. **Beide Hälften im selben Änderungssatz.** Wer `save` baut, baut `load`.
   Wer `bind` baut, baut `unbind`. Ohne Ausnahme.
4. **Fremde Dateien nicht anfassen.** Gemeint sind uncommittete Dateien
   einer parallelen Session (`git status` zeigt sie) — nie editieren, eigene
   Edits per Pathspec committen. **Berichtigt 21.08.:** bis dahin stand hier
   „Der Sondenentwurf gehört Codex; Korrekturen daran gehen über NAK-Zeilen,
   nicht über Edits." Das stimmte schon am 20.08. nicht mehr — Fassung 0.4
   sind 217 geänderte Zeilen (184+/33−) in vier Commits derselben Claude-Session
   (`ab80522`…`d5dacde`), Fassungen 0.1–0.3 stammen von Codex. Heute gilt:
   **Der Entwurf gehört dem Repo.** Er trägt einen Errata-Block (21.08.), der
   Vorrang hat; neue Entscheide kommen dorthin — nur mit Datum und Wortlaut
   des Users — und Befunde als NAK-Zeile. Der Text unter dem Block wird nicht
   umgeschrieben, damit die Prüfer-Regel „Gate-Text aus dem Entwurf, nicht
   meine Zusammenfassung" (T2) einen stabilen Bezug behält.
5. **Doku im selben Commit.** Wird eine Zähl- oder Bestandsaussage in
   `CLAUDE.md` / `plugin-wissen.md` unwahr, zieht sie mit.
6. **Ein T3-Befund verschwindet nie still** — gefixt, oder NAK-Zeile, oder
   protokollierte Widerlegung.
7. **Grüner Build ≠ fertige Phase.** Der Entwurf sagt es (§53.1); hier steht
   es nochmal, weil es die Regel ist, die am ehesten rutscht.

---

## 7. Was ich bewusst NICHT vorschlage

- **Keine GitHub-CI.** Eine Maschine, ein Entwickler, ein JUCE-Windows-Build —
  der lokale Beweis-Runner leistet dasselbe zu einem Bruchteil der Wartung.
  Wird das Projekt je mehrköpfig, ist der Runner die Vorlage für die CI.
- **Keine Parallelarbeit in Worktrees vor G2.** P0/P1 hängen alle am
  Identitätsmanifest; parallele Zweige würden es duplizieren. Ab P4 sind
  DSP-Metriken (C++) und Storearbeit (Rust) sauber trennbar — dort lohnt es.
- **Kein Harness-Autopilot für die Audiothread-Tickets** (S10–11, S26–28).
  Genau dort ist der Fehler still und teuer; er gehört unter direkte
  Beobachtung, nicht in einen unbeaufsichtigten Lauf.
- **Keine Mutationstests im ersten Durchgang.** Sinnvoll wären sie für die
  Broker-Zustandsmaschine (`coordinator_model`), aber erst wenn deren
  Vertragstests stehen — sonst misst man die Abdeckung von Nichts.

---

_Neben `docs/FL-Nakama-Sonden-Design-Entwurf.md` zu lesen, nicht statt._
