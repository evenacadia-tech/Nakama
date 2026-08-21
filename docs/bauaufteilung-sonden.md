# Bauaufteilung Sondenkern — Sessions, Gates, adversariale Prüfung

- **Stand:** 2026-08-20
- **Gehört zu:** `docs/FL-Nakama-Sonden-Design-Entwurf.md` (Fassung 0.4, Codex)
  und `docs/pruefbericht-sondenentwurf-2026-08-20.md`
- **Was das hier ist:** wie *ich* den Plan bauen würde. Der Entwurf ordnet
  **Fähigkeitsgrenzen** (P0–P9), dieses Dokument ordnet **Arbeitscontainer**
  (Sessions) und die Prüfung dazwischen. Der Entwurf bleibt unangetastet.
- **Status:** **Bauentscheidung erteilt** (User, 20.08.2026: „okay dann fangen
  wir damit nächste session an"). Nächste Session startet bei **S0**.
- **Nachgezogen 20.08. auf Entwurf 0.4:** Fassung 0.4 hat alle fünf Befunde des
  Prüfberichts eingearbeitet; NAK-19…23 sind geschlossen (`ab80522`). Phasen
  P0–P9 und die 19 Tickets sind unverändert — der Sessionplan unten trägt
  weiter. Zwei Vorlaufpunkte sind dadurch entfallen (§1.2/§1.3).

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
+ ab P1: EqCopStateMigrationTest
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

### 1.2 ~~NAK-19~~ — erledigt, P3 ist nicht mehr blockiert

Der Produktentscheid vom 20.08. (Entwurf §0.3) löst die Kollision auf: Der
Hörkompass-Zielvertrag gilt der **eigenständigen Prisma-App**, das
Master-Plugin bekommt eine **konventionelle Arbeits-UI**. Quellenliste,
Heatmap und Detailansicht sind dort legitime Dauerarbeitsflächen; ihr Maßstab
ist Lesbarkeit, nicht „leeres Glas". **P3 kann ohne weitere Entscheidung
gebaut werden.**

### 1.3 ~~NAK-23a~~ — erledigt, aber die Festlegung bleibt gültig

Der widersprüchliche Bündelungssatz ist in 0.4 entfernt. Die Sachentscheidung
für `SONDE-005` steht damit weiterhin: **1 Batch = N Frames EINER Quelle**,
Bündelung nur auf dem Broker→Main-Weg über einen Wrapper. Grund: der
Sonde→Broker-Weg hat Cap 2 mit replace-oldest — dort kann nie gebündelt
werden; ein Schema, das es erlaubt, hätte ein totes Feld.

### 1.4 Neu aus Entwurf §0.4 — die Interims-UI ist ein Vertrag

Das Arbeitsmodell „Technik voraus, Design parallel" macht die schlichte
Bedien-UI **prüfbar**, nicht beliebig. In jeder UI-Fassung sichtbar sein
müssen: **Frische/stale · Unsicherheit/Konfidenz · `arming`/`audible_ready` ·
Capability-Degradation · Konfliktauflösung · welche Aktion gerade NICHT aktiv
ist.** Das wandert unten in die T1-Liste — es ist der Punkt, an dem eine
Interims-UI sonst still einen nicht existierenden Zustand vortäuscht.

Betroffen sind **drei** Oberflächen, nicht eine: Main (volle Arbeitsfläche),
der Editor der Active Probe (lokal bedienbares EQ-Plugin, ab P6) und der
Editor der Passive Probe (minimale Status-/Identitätskachel, null
Hostparameter — kommt in `SONDE-007b` mit).

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

### Vorlauf

| # | Inhalt | Prüfung |
|---|---|---|
| ~~**S0**~~ | ~~Beweis-Runner `tools/beweise.ps1`, Manifest-Vorlage, `docs/beweise/`~~ — **erledigt 20.08.**, T1 gefahren | T1 ✅ |

### P0 · Bestand einfrieren, Hostgrenzen beweisen

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| ~~S1~~ | `SONDE-004a` | ~~Wegwerf-Target mit zwei deklarierten Aux-Bussen + Impulsharness~~ — **erledigt 20.08.**, Selbsttest 41/41, Manifest `docs/beweise/SONDE-004a.md` | T1 ✅ |
| — | — | **👤 User-Termin A (FL):** Aux-Layout, Kanalreihenfolge, Recall, PDC-Impulse | — |
| ~~S2~~ | `SONDE-001` + `002` | ~~Identität einfrieren~~ — **gebaut 20.08.**: Manifest aus dem gebauten `moduleinfo.json`, `EqCopIdentityTest` (63 Prüfungen: reservierte CIDs nachgerechnet, VST2-Pfad negativ bewiesen, **Freeze auch an der CMake-Quelle**), Schema-1-Goldens für alle vier Rollen. Die `.flp`-Legacy-Fixture bleibt ausdrücklich offen (nur in FL erzeugbar → Termin B). | T1 ✅ · T2 Runde 1 = NEEDS_WORK, Runde 2 siehe `docs/beweise/SONDE-001-002.md` |
| ~~S3~~ | `SONDE-003` | ~~JUCE-Bridge-Patch: Context-Anwesenheit, Parameterpunkte, Buslatenz, Quellhash-Gate~~ — **gebaut 21.08.** (`1e91d54`): Patch mit neun Anker-genauen Stellen, Quellhash-Gate mit **allen drei Zweigen vorgeführt** (unberührt patcht + misst nach · gepatcht No-Op · fremd Bauabbruch), `EqCopHostContextTest` **91/91**, Kanon von 5/5 auf **6/6** gewachsen, `pluginval` Strenge 8 SUCCESS. Manifest `docs/beweise/SONDE-003.md`. **T2 brauchte drei Runden** — Runde 1 und 2 waren NEEDS_WORK und fanden je einen echten Vertragsbruch (der zweite eine Regression aus der ersten Nacharbeit); beide gefixt, beide Riegel nachweislich zum Fallen gebracht. | T1 ✅ · T2 ✅ |
| ~~**S3b**~~ | — | ~~Nachtrag 21.08. — der Plan hat Termin B unterschätzt: eine Klickliste allein misst nichts.~~ — **gebaut 21.08.**: `EqCopHostProbe` (`NkHp`, Wegwerfware) ist das erste Ziel, das die Hostbrücke BENUTZT. Misst Context-Anwesenheit, Gültigkeitsbits (immer/manchmal/nie), Zeitsprünge für Seek·Loop·Smart Disable **mit Fehlalarm-Riegel**, Offline-Render, float/double, Presentation-Latency und samplegenaue Automation → JSON. Selbsttest **89/89**, `pluginval` Strenge 8 SUCCESS, Kanon von 6/6 auf **7/7**. Klickliste `eq-copilot/docs/FL-TERMIN-B-HOSTZEIT.md` (inkl. Legacy-`.flp`-Fixture aus §54 Punkt 2). Manifest `docs/beweise/SONDE-003b.md`. **T2 brauchte vier Runden** — Runde 1 fand einen Blocker (der Editor schnitt genau die Automationszeilen ab; ich hatte das Gerät nie gerendert), Runde 2 eine Prüfung, die nicht fehlschlagen konnte, Runde 3 einen Zähler, der Blöcke statt Änderungen zählte. Alle gefixt, jeder Riegel nachweislich zum Fallen gebracht. | T1 ✅ · T2 ✅ |
| — | — | **👤 User-Termin B (FL):** Live/Stop/Seek/Loop-Straddle/Render/Smart Disable | — |
| S4 | — | Auswertung beider Termine → Capabilityreport mit Rohmessdaten | T1+T2 |
| **G0** | — | **Gate:** `/c-review` (Bridge-Patch) + Codex. Falsifikation: Gate 1, Gate 5 | **T3** |

`SONDE-003` ist das riskanteste kleine Ticket des ganzen Plans — ein
idempotenter Patch am gevendorten JUCE mit Quellhash-Abbruch, der bei jedem
JUCE-Update neu bewiesen werden muss. Eigene Session, eigener Prüfer.

### P1 · Verträge, State, neutrale Shells

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| S5 | `SONDE-005a` | v3-JSON-Schemas + Bandgitter + gültige/ungültige Fixtures | T1+T2 |
| S6 | `SONDE-005b` | FlatBuffers (Feld-IDs!), gepinntes `flatc`, Codegen-Drift-Test | T1+T2 |
| S7 | `SONDE-006` | State-Schema 2, Parameterbestand, reine Schema-1-Migration | T1+T2 |
| S8 | `SONDE-007a` | **Gemeinsamer Kern ohne `JucePlugin_*`-Konstanten** (NAK-23b) | T1+T2 |
| S9 | `SONDE-007b` | Drei Ziele, Lifecycle-Klassifikation, Installer-Manifest | T1+T2 |
| **G1** | — | **Gate:** `/c-review` + `/rust-review` + Codex. Falsifikation: Gate 1, Gate 7 | **T3** |

S8 ist der Umbau, den der Entwurf unterschätzt: Die heutige Begründung gegen
eine geteilte Static-Lib steht wörtlich im CMake-Kopf. Wenn der geteilte Kern
auch nur **eine** `JucePlugin_*`-Konstante sieht, bekommen zwei Bundles die
Identität des dritten — und genau das Identitäts-Golden aus S2 fällt.
Deshalb getrennt von S9.

### P2 · Messkern, IPC v3, Store — die größte Phase

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| S10–11 | `SONDE-008` | `StampedAudioQueue`, Ein-Block-Quarantäne, fixed-memory Loudness | T1+T2 |
| S12–13 | `SONDE-009` | FeatureEngine v2: Zeit-, Validity-, Event-, Bandverträge | T1+T2 |
| S14–15 | `SONDE-010` | v3-Control-/Telemetry-Clients (C++) + Rust-Envelopeparser | T1+T2 |
| S16–17 | `SONDE-011` | Coordinator, monotone Eviction, SQLite-Migration 1, Outbox | T1+T2 |
| **G2** | — | **Gate:** volles Programm — `/c-review` + `/rust-review` + `/security-review` + Codex. Falsifikation: Gate 1, 2, 5 | **T3** |

S10 ersetzt den `AbstractFifo` im **Audiothread**. Das ist der gefährlichste
Eingriff der ganzen Phase. Der Prüfer bekommt hier zusätzlich einen
Blockgrößen-Fuzz über `maximumExpectedSamplesPerBlock` hinaus.

Ab hier: **R0 · Contract/Internal** erreicht.

### P3 · Passive Landkarte → R1

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| S18–19 | `SONDE-012` | Join, Führung, Frische, Messpunktwahrheit, Fehlerzustände | T1+T2 |
| **G3** | — | **Gate:** `/rust-review` + Codex + 60-min-Soak. Falsifikation: Gate 7 | **T3** |

### P4 · Vergleichsevidenz → P5 · Ursachen → R2

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| S20–22 | `SONDE-013` | Dynamik/Stereo/PRE-POST/Passage + manueller Experimentkern | T1+T2 |
| **G4** | — | **Gate:** `/c-review` (DSP) + Codex. Falsifikation: Gate 5, 6 | **T3** |
| S23–25 | `SONDE-014` | Intent, CauseHypothesis, Proposal, AssistantStep + Evaluationskorpus | T1+T2 |
| **G5** | — | **Gate:** Codex + adversariale Gegenbeispiele. Falsifikation: Gate 6, 7, 8 | **T3** |

G5 ist das ungewöhnlichste Gate: Der Prüfer soll **eine falsche starke
Ursachenbehauptung provozieren** — korrelierter Distraktor, Parent-Duplikat,
verschobene Passage. Enthaltung („mehr Daten nötig") ist das gewünschte
Ergebnis, nicht ein Treffer.

**R2 · Passive Beta** — 9 von 12 Kernfunktionen, Audio vollständig passiv.
**Das ist der Punkt, an dem das Produkt zum ersten Mal wirklich nützt.**

### P6–P7 · Aktiver Kern → R3

| # | Ticket | Inhalt | Prüfung |
|---|---|---|---|
| S26–28 | `SONDE-015` | Lokaler Active-DSP, Bankpool, State/Automation, A/B | T1+T2 |
| **G6** | — | **Gate:** `/c-review` **max effort** + Codex + ThreadSanitizer-Äquivalent + Worst-Case-CPU. Falsifikation: Gate 1, 2, 3 | **T3** |
| S29–31 | `SONDE-016` + `017` | Pairing/HMAC (**mit NAK-21-Reihenfolge!**), Lease, Apply/Revert, Active-Compare | T1+T2 |
| **G7** | — | **Gate:** `/security-review` + `/rust-review` + Codex + 10 000 Befehlsstress. Falsifikation: Gate 3, 4, 8 | **T3** |

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
| Vorlauf + P0 | 5 | 1 | 2 |
| P1 | 5 | 1 | — |
| P2 → **R0** | 8 | 1 | — |
| P3 → **R1** | 2 | 1 | — |
| P4 | 3 | 1 | — |
| P5 → **R2** | 3 | 1 | — |
| P6 | 3 | 1 | — |
| P7 → **R3** | 3 | 1 | — |
| P8 | 2 | 1 | — |
| P9 → **R4** | 2 | 1 | — |
| **Gesamt** | **36** | **10** | **2** |

**Bis R2 (das erste wirklich nützliche Produkt): 26 Bau-Sessions + 6 Gates.**
**Bis R4 (voller Sondenkern): 36 + 10 = 46 Sessions.**

Das ist eine **Hypothese mit Nachmessung**, keine Schätzung, die ich
verteidige. **Rekalibrierungspunkt nach G1:** Wenn P0+P1 statt 10 Sessions 15
gekostet haben, wird der Rest mit Faktor 1,5 fortgeschrieben und dieses
Dokument korrigiert — nicht die Realität passend geredet.

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
| 8 | KI-Ausgabe ändert Zahlen, Ziel oder Aktion außerhalb des Proposals | G5, G7 |

---

## 6. Regeln, die für alle Sessions gelten

1. **Ein Ticket, ein Commit-Bündel, ein Manifest.** Kein Ticket endet ohne
   `docs/beweise/SONDE-0NN.md` mit roher Ausgabe.
2. **Kein Vorgriff.** Keine Session simuliert eine spätere Capability. Fehlt
   ein Hostbeweis, wird das Capabilitybit **nicht gesetzt** — nicht heuristisch
   ersetzt.
3. **Beide Hälften im selben Änderungssatz.** Wer `save` baut, baut `load`.
   Wer `bind` baut, baut `unbind`. Ohne Ausnahme.
4. **Fremde Dateien nicht anfassen.** Der Sondenentwurf gehört Codex;
   Korrekturen daran gehen über NAK-Zeilen, nicht über Edits.
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
