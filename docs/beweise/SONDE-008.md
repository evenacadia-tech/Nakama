# Beweismanifest — SONDE-008 «StampedAudioQueue, Ein-Block-Quarantäne, fixed-memory Loudness»

| Feld | Wert |
|---|---|
| Ticket | `SONDE-008` |
| Phase / Session | P2 / S10–11 |
| Gate-Text (Quelle) | `docs/FL-Nakama-Sonden-Design-Entwurf.md` §65, Tickettabelle — **wörtlich**: „`StampedAudioQueue`, Quarantäne und fixed-memory Loudness \| keine Teilblöcke/unbegrenzten Vektoren; RT-/EBU-Goldens grün" |
| Commits | `7fa1cf5` (Implementierung + zwei Kanon-Beine) · Folgecommits siehe §7 |
| Datum | 2026-08-23 |
| Prüfstufen | T1 ☑ · T2 ☐ · T3 ☐ (nur am Gate) |

> ⚠️ **T2 ist NICHT gelaufen.** Der Dirigent spawnt ihn mit frischem Kontext.
> Das Ticket ist damit **gebaut, nicht abgenommen** — dieselbe Lage wie S9 vor
> seiner Prüfrunde.

---

## 1. Ticket-Behauptungen

Jede Zeile ist eine Behauptung aus dem **Gate-Text**, nicht aus meinem Kopf.
Der Gate-Text hat drei Hälften; die vierte Zeile ist die Bedingung, unter der
das Ticket überhaupt zulässig ist (Grundgesetz).

| # | Behauptung (Gate-Text) | Befehl | Ergebnis | Rohausgabe | Datum |
|---|---|---|---|---|---|
| 1 | **keine Teilblöcke** — die Analyseübergabe veröffentlicht ganz oder gar nicht | `EqCopQueueStressTest.exe` | ✅ 68/0 | [↓ B1](#b1) | 23.08. |
| 2 | **keine unbegrenzten Vektoren** — fixed-memory Loudness | `EqCopLoudnessGoldenTest.exe` | ✅ 66/0 | [↓ B2](#b2) | 23.08. |
| 3 | **RT-Golden** — der Umbau ändert kein Sample; Audiothread ohne Allokation | `EqCopNullTest` · `EqCopMarkierungTest` · `EqCopLebenslaufTest` · B4 §J/§L | ✅ | [↓ B3](#b3) | 23.08. |
| 4 | **EBU-Golden** — Loudness bleibt in der Toleranz aus §49 (±0,1 LU) | `EqCopLoudnessGoldenTest` · `EqCopGoldenTest` | ✅ 0,000000000 LU bzw. 239/0 | [↓ B2](#b2), [↓ B4](#b4) | 23.08. |
| 5 | Kanon unverändert grün, mit den zwei neuen Beinen | `tools/beweise.ps1 -Bauen` | ✅ 26/26, Exit 0, beglaubigt | [↓ §3](#3-kanon-lauf) | 23.08. |

---

## 2. Rohe Ausgaben

<a id="b1"></a>
### B1 · Keine Teilblöcke — Queue, Quarantäne, Blockgrößen-Stress

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe`
· **Exitcode:** 0 · **Datum:** 2026-08-23

```text
(siehe Kanon-Lauf §3, Bein B4 — die Rohausgabe steht dort vollständig)
```

<a id="b2"></a>
### B2 · Keine unbegrenzten Vektoren + EBU-Golden

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe`
· **Exitcode:** 0 · **Datum:** 2026-08-23

```text
(siehe Kanon-Lauf §3, Bein B9 — die Rohausgabe steht dort vollständig)
```

<a id="b3"></a>
### B3 · RT-Golden: kein Sample verändert

**Befehle und Exitcodes:** siehe Kanon-Lauf §3, Beine A1 (NullTest), A3
(Markierung), B8 (Lebenslauf) sowie B4 §J/§K/§L. · **Datum:** 2026-08-23

<a id="b4"></a>
### B4 · Die bestehende Analyse-Referenz hält

**Befehl:** `EqCopGoldenTest.exe eq-copilot\fixtures` · **Exitcode:** 0
· **Datum:** 2026-08-23 — siehe Kanon-Lauf §3, Bein A2.

---

## 3. Kanon-Lauf

```powershell
pwsh -File tools/beweise.ps1 -Bauen -Ziel docs/beweise/SONDE-008.md -Anhaengen -Titel 'SONDE-008'
```

_(Der angehängte Abschnitt erscheint unter dieser Zeile.)_

---

## 4. Was gebaut wurde — und warum genau so

### 4.1 Die zwei Ringe (`plugin/core/StampedAudioQueue.h`)

Der bisherige Weg war ein `juce::AbstractFifo` über 65 536 interleavten Frames.
Sein Vertrag lautet „nimm, was gerade passt": `prepareToWrite (n, s1, n1, s2,
n2)` gibt zurück, wie viel Platz da ist, und der Audiothread schrieb bei
Platzmangel einen **Teilblock** und zählte den Rest als `framesDropped`. Für
zeitabhängige Analyse ist das die schlimmste Verlustform: der Worker sieht
danach einen lückenlosen Samplestrom, dem in der Mitte Zeit fehlt, und **kann
das nicht mehr sehen**. Entwurf §48.1 sagt darum wörtlich „ausschließlich ganz
oder gar nicht" und §53.7 „Reduktion erfolgt über Kadenz und Ganzblockdrop, nie
über Teilblockkopien".

Zwei Ringe, weil ein Ring „ganz oder gar nicht" gar nicht ausdrücken kann: erst
der Deskriptor, der **nach** den Samples mit Release-Semantik erscheint, macht
einen Block für den Consumer sichtbar. Es gibt genau **eine** Ordnungskante
(`deskSchreib` release/acquire) — eine zweite gäbe nur eine zweite Stelle, an
der die Ordnung falsch sein kann.

**`maxBlockFrames` ist eine feste Layout-Konstante (16 384), nicht aus
`maximumExpectedSamplesPerBlock` abgeleitet.** Der Entwurf nennt diesen Hinweis
selbst unzuverlässig (§48.1: „bei JUCE nur ein Hinweis"); FL zerteilt Puffer
sogar bis auf **1 Sample** (Capabilityreport S4). Eine Slotkapazität, die an
einem unzuverlässigen Wert hängt, wäre zur Laufzeit nicht vorhersagbar und die
Oversize-Regel nicht deterministisch prüfbar. Die feste Grenze gehört
stattdessen in die QA-Matrix — genau das, was §48.1 letzter Satz verlangt.

**Startgrößen (§53.7, „keine ABI"):** 131 072 Frames Sample-Ring je Stereo-Tap,
2 048 Deskriptoren, 16 384 Frames Slotkapazität. Gemessene High-Water-Mark im
Betrieb steht in `analyseGroessterBlock()`.

**Der lokale Strom zählt auch verworfene Blöcke.** Ein Zähler, der nur
Angenommenes zählt, könnte „hier fehlt Zeit" nicht mehr ausdrücken — es wäre
derselbe Fehler wie beim Teilblock. 🔑 Das ist dieselbe Lehre wie in der
Hostbrücke: *ein Zähler beschreibt, was der HOST geliefert hat, nicht was in
unsere Struktur passte.*

### 4.2 Kein `fifo.reset()` mehr vom Nachrichtenthread

`prepareToPlay` rief bisher `fifo.reset()` — der Nachrichtenthread verstellte
damit **beide Enden** eines SPSC-Rings, während der Workerthread las. Das ist
genau die Stelle, an der ein Ring still Müll liefert. Der neue Weg: der
Nachrichtenthread meldet nur einen Wunsch (`neustartAnfordern()`), der
**Produzent** löst ihn ein (`startFolge` +1, Lücke offen), und der Consument
erkennt Reste des alten Anlaufs an der kleineren `startFolge` und trägt sie
nicht in die Analyse (dort kann die Samplerate eine andere gewesen sein).
Dasselbe Ergebnis wie der alte Reset, ohne den Fremdzugriff — gemessen in B4 §N
(40 veraltete Blöcke korrekt übersprungen).

### 4.3 Die Ein-Block-Quarantäne — und wie „zeitlich konsistent" gelesen wird

§53.7: „Worker hält den jüngsten vollständigen Block in Ein-Block-Quarantäne.
Erst der zeitlich konsistente Folgeblock versiegelt ihn." §32.3: „Stoppt der
Callback vorher, bleiben sie unvollständig."

**Gelesen als:** ein Block geht an die Analyse, wenn und nur wenn sein
Nachfolger beweist, dass er ihn fortsetzt. Beweist er es nicht, fällt der
gehaltene Block. Der Preis ist **ein** Block je Diskontinuität (bei 512 Frames
= 10,7 ms), gezählt und auslesbar; der Gegenwert ist, dass kein Analysefenster
über eine Epochengrenze reicht, deren Lage erst nachträglich sichtbar wurde.
Dass §32.3 den Stopp-Fall ausdrücklich als „bleiben unvollständig" führt, ist
der Beleg, dass ein gehaltener Block fallen **darf**.

Die Kontinuitätsprüfung ist bewusst knapp und trägt drei Entscheidungen:

1. **Der lokale Strom muss lückenlos anschließen** — immer, auch ohne
   Hostkontext (§53.6: ohne `project_time_samples` gilt „nur lokaler monotoner
   Verlauf").
2. **Die Hostzeit prüft nur, wenn der Transport nachweislich läuft.** Bei
   gestopptem Transport steht `projectTimeSamples` still, während Audio
   weiterläuft (Vorhören am Instrument). Wäre die stehende Zeit ein Bruch,
   stürbe jede Analyse außerhalb der Wiedergabe.
3. ⚠️ **`neu.start == gehalten.start` ist KEIN Bruch.** FL zerteilt Puffer bis
   auf 1 Sample, um Automationspunkte zu setzen (Termin B / Capabilityreport
   S4: „Automation nie >1 Punkt je Block bei Offset 0 — FL zerteilt Puffer bis
   1 Sample"). Die Teilstücke tragen **dieselbe** Projektzeit und sind trotzdem
   lückenlose Fortsetzung. Eine stehende Zeit ist keine Aussage über einen
   Sprung. Ohne diese Zeile verlöre die Analyse in automationsreichen Projekten
   an jeder Teilungsgrenze einen Block — und zwar still. Ein echter Seek oder
   Loop-Wrap **bewegt** die Zeit und fällt weiterhin durch (B4 §H, gemessen).

**Nicht hier, sondern in `SONDE-009`:** Epochen-, Segment- und Fensterbuchhaltung
der Features („Drop/Seek/Loop trennt jedes offene Fenster", §65). Diese Schicht
liefert nur die Grenze.

### 4.4 Fixed-memory Loudness (`plugin/core/analysis/LoudnessAccumulator.h`)

Ausgebaut: `std::vector<double> kZellen` (10 Werte/s, unbegrenzt) und der
Zweitdurchgang in `finalisiereLoudness`, der **4×/s** einen frischen Vektor über
die ganze Sessionlänge anlegte und summierte — quadratischer Aufwand über die
Spieldauer. Der Kopfkommentar nannte das „unproblematisch über Stunden"; §48.1
sagt das Gegenteil.

**Warum das nicht trivial ist:** BS.1770 gated zweistufig, und das relative Gate
liegt 10 LU unter dem Mittel der absolut-gegateten Blöcke — es ist also erst
bekannt, wenn alle Blöcke da sind. Ein exakter Zweitdurchgang in konstantem
Speicher ist **unmöglich** (es ist im Kern eine Quantil-Summen-Abfrage).

**Die Aufteilung ist der ganze Trick:**

- Das **absolute** Gate läuft als kompensierte Laufsumme (Neumaier) plus Zähler
  — Γ_r ist damit **exakt**, ohne jede Historie.
- Nur die **Auswahl** für das relative Gate kommt aus einem Histogramm über die
  Blocklautheit (10 001 Bins à 0,01 LU über [−70, +30] LUFS). Die Bin-**Summen**
  sind exakt; unsicher ist allein die Zugehörigkeit **eines einzigen** Bins.
- `unsicherheitLu()` macht genau diese Schranke auslesbar, statt sie zu
  behaupten. §48.1 verlangt: „Der EBU-Korpus prüft, dass Quantisierung und
  Gating innerhalb der Toleranz aus Abschnitt 49 bleiben."

**Bitgleich geblieben:** der Kurzzeit-LUFS (3 s). Er liest die letzten 30 Zellen
in derselben Reihenfolge und mit derselben Assoziativität wie die alte
Vektorschleife — gemessen, nicht angenommen (B2, „Kurz-LUFS BITGLEICH" auf jedem
Korpus). Bei der **integrierten** Lautheit ist Bitgleichheit nicht erreichbar,
weil binweise Summation eine andere Reihenfolge hat; dort steht die gemessene
Schranke ≤ 1e-9 LU, wo kein Block im Grenzbin liegt.

**Fester Bedarf:** 120 252 Byte, `speicherBytes()` ist eine reine
Compile-Time-Größe. Vorgeführt: eine Million Zellen → **0 Allokationen**; die
Gegenprobe (die ausgebaute Rechnung) alloziert bei derselben Million **36**-mal.

### 4.5 Die Hostbrücke ist jetzt verdrahtet

`plugin-wissen.md` §2.1 hielt fest: „im Produkt kompiliert, aber **unbenutzt** …
Verbraucher SONDE-008/009". Mit diesem Ticket ist `EqCopilotProcessor` eine
`Senke`. Der Grund ist nicht Vollständigkeit, sondern Notwendigkeit: **nur die
Brücke kann „Context fehlt" ausdrücken.** JUCEs VST3-Playhead liefert nie
`nullopt`, weil der Wrapper seinen internen Context nullt und daraus ein
gefülltes `PositionInfo` baut (NAK-24). Ein Zeitstempel aus dieser Quelle wäre
geraten, nicht bewiesen — und „zeitgestempelt" ist der halbe Ticketname.

⚠️ Der Befund ist **nicht 1:1 mit `processBlock` gepaart** (Parameter-Flush mit
`numSamples == 0`, Wavelab-Riegel — steht so im Kopf von `NakamaHostBridge.h`).
Deshalb ein `frisch`-Bit, das `processBlock` verbraucht, statt einer Annahme.
Der Rückfallweg über `getPlayHead()` bleibt für Ziele ohne gepatchten Wrapper
(Konsolentests, andere Formate); ohne Playhead ist Transport ausdrücklich
**unbekannt**, nicht „gestoppt" und nicht „läuft".

---

## 5. Die Transport-Frage (Register `U10`) — geprüft, umgesetzt, begründet

**Auftrag:** CLAUDE.md-Register 22.08. („Nein, nur mit Signal") führt das
fail-open `∨ ¬hatTransport` der Hör-Markierung als abgewählt und „umzusetzen mit
S10–S13 (NAK-35/NAK-24)". Zu messen war am Entwurf, ob das in **SONDE-008s**
Scope fällt oder zu SONDE-009 gehört.

**Messung am Entwurf.** §0.1 benennt den Mangel und seine Ursache in einem Satz:
„Der eigentliche Mangel ist, dass `hatTransport` ‚Transport unbekannt' gar nicht
ausdrücken kann." Damit ist die Frage keine Geschmacksfrage mehr, sondern eine
Reihenfolgefrage: **der Term ist erst schließbar, wenn „unbekannt" ausdrückbar
ist.** Ausdrückbar wird es durch `processContextPresent` + `playing.gueltig` aus
der Hostbrücke (§53.7) — und genau diese Verdrahtung bringt SONDE-008 für den
Zeitstempel ohnehin mit (§4.5).

**Entscheid: hierher.** Die Alternative wäre gewesen, die ehrlichen Bits zu
verlegen und die Markierung weiter aus dem tautologischen `hatTransport` zu
speisen — **zwei Transport-Wahrheiten in einem `processBlock`**. Das ist genau
die „zweite Wahrheit", die dieses Projekt an anderen Stellen teuer bezahlt hat
(NAK-52, `Bundle::eqcp()`-Literal).

**Was zu SONDE-009 gehört und dort bleibt:** die Zeit-, Validity-, Event- und
Bandverträge der FeatureEngine (§65 `SONDE-009`) und das Trennen offener Fenster
an Drop/Seek/Loop. SONDE-008 liefert die Bits und die Grenze, nicht ihre
Auswertung.

**Neue Verriegelung** (`PluginProcessor.cpp`, gemessen):

```
erlaubt = klassifiziertAlsMain
        ∧ (echtzeitOk ∨ testEchtzeit)
        ∧ (spieltGültig ∧ spielt)          ← neu, ersetzt (spielt ∨ ¬hatTransport)
        ∧ ¬isNonRealtime
        ∧ (editorOffen ∨ testEchtzeit)
```

**Wirkung, ehrlich getrennt:**

- **In FL ändert sich nichts.** Dort lag `hatTransport` ab dem ersten Block auf
  `true`, der fail-open-Zweig war ein **toter Zweig** (Prüfbericht 1.2). Die
  wirksame Bedingung war schon vorher `spielt`.
- **Headless ändert sich etwas:** ohne Playhead und ohne Brücke färbt nichts
  mehr. Deshalb tragen `EqCopMarkierungTest` (T2/T5, T9, T6, T7b, T4) und
  `EqCopLebenslaufTest` (`faerbtAudio`) jetzt einen laufenden Playhead. Das ist
  keine Testkosmetik: ohne ihn wäre die Hälfte „und färben, sobald es so weit
  ist" still zu einer Tautologie geworden — grün, weil die Markierung generell
  stumm ist, nicht weil die Klassifikation greift.
- `testForciereEchtzeit` umgeht den Term **absichtlich nicht** — er umgeht nur,
  was an der Wanduhr hängt (Lebenszeichen, Editor). Dieselbe Begründung wie beim
  §53.5-Term aus S9.

**NAK-24 und NAK-35** sind damit geschlossen; die Zeilen in
`docs/offene-punkte.md` tragen Datum und Commit.

---

## 6. T1 — Selbstaudit

Feste Liste aus `docs/bauaufteilung-sonden.md` §2, gefahren über
`git diff e330052..7fa1cf5` plus die Folgecommits.

| # | Punkt | Befund |
|---|---|---|
| 1 | Numerische Ränder: NaN, ±inf, 0, negativ, Überlauf, Wrap | **Vier Funde, alle geschlossen und im Bein gemessen.** (a) `projectSampleStart` nahe `INT64_MAX`: `gehalten.start + sampleCount` liefe über — die Kontinuitätsprüfung fängt das VOR der Rechnung ab und wertet es als Grenze (B4 §H). (b) Negative Projektzeit (HostProbe hat sie in FL gesehen) ist eine gültige, fortsetzbare Zeit — gemessen. (c) `frames == 0` (VST3-Flush) ist kein Verlust und erzeugt keine Lücke (B4 §F). (d) Blocklautheit über dem Histogramm-Gitter: die erste Fassung klemmte in den obersten Bin mit der Begründung „liegt über JEDER Schwelle" — **falsch**, der Golden hat es widerlegt (`integriert()` lieferte gar keinen Wert). Jetzt eigener exakter Eimer. NaN/Inf-Zellen werden gezählt (`bloeckeNichtEndlich`), nie still als 0 verbucht. Ring-Wrap: 11 Umläufe bitgleich gemessen (B4 §B). |
| 2 | Gegenpfad vorhanden? | **Drei Paare, alle im selben Änderungssatz und gemessen.** *füllen↔leeren/Überlauf*: B4 §C fährt beide Hälften — voll ⇒ Ganzblockdrop, nach dem Leeren nimmt der Ring wieder an, der erste neue Block trägt die Lücke. *starten↔stoppen*: B4 §N (prepareToPlay mitten im Betrieb → Neuanlauf, alte Blöcke verworfen statt analysiert) und B4 §H (Quarantäne-`zuruecksetzen` verwirft den gehaltenen Block). *aktivieren↔abklingen*: unverändert die Hör-Markierung, `EqCopMarkierungTest` T2 (Fade-out endet im erwarteten Fenster, danach bitgleich). *speichern↔laden* ist von diesem Ticket nicht berührt (B2/StateMigration unverändert grün). |
| 3 | Behauptungs-Integrität | Geprüft **an der Quelldatei**, nicht zitiert: `NakamaHostBridge.h:218-224` (`Senke::nakamaBlockEmpfangen`) existiert und ist die Signatur, die der Prozessor überschreibt · der Patch ruft `verbinde (dynamic_cast<eqcop::hostbruecke::Senke*> (pluginInstance))` (Patchzeile 42) — die Verdrahtung hängt genau daran · `AnalyseEngine.cpp` enthält nach dem Umbau **keinen** `kZellen`-Treffer mehr (grep 0) · die im Manifest genannten Entwurfsstellen §48.1, §53.7, §32.3, §66.1, §65 und §49 sind einzeln aufgeschlagen und tragen den zitierten Text. Zahlen: Kanon 24 → **26** (`tools/beweise.ps1`, `$kanon`), Kern-Verbraucher 13 → **14** (Configure-Ausgabe). |
| 4 | Lügt der UI-Text? | **Ein Fund, geschlossen.** Nicht in der Plugin-UI (dieses Ticket fasst keine an), sondern im Golden selbst: die Prüfung „über dem Gitter" baute ihren Meldetext, **bevor** `integriert()` den Wert gesetzt hatte (Argument-Auswertungsreihenfolge ist unspezifiziert) — sie meldete grün und druckte dazu `akku=0.000000`. Ein Text, der etwas anderes sagt als die Prüfung, ist genau dieser Punkt. Jetzt erst rechnen, dann melden. **Der Editor** zeigt `framesDropped` unverändert in derselben Einheit (verlorene Analyse-Frames) — die Zahl kommt jetzt aus der Queue statt aus einem zweiten Atomic; eine zweite Quelle könnte nur auseinanderlaufen. |
| 5 | Anzeige-Pflichten (nur bei UI-Diff) | **Nicht betroffen** — kein UI-Diff. `PluginEditor.cpp` ist unverändert (Ticketgrenze: keine UI-Arbeit, Figma ist die Quelle). Die neue Telemetrie (Drops, Oversize, Quarantäne-Verwürfe, Kontinuitätsbrüche, `unsicherheitLu`) ist auslesbar, aber **nirgends angezeigt** — als offener Punkt geführt statt still gebaut. |
| 6 | Audiothread: nachweislich keine Allokation, kein Lock, kein I/O, kein Logging? | **Gemessen, nicht behauptet.** B4 §L: 4 000 Blöcke wechselnder Größe (1…4 096) mit Transportkanten und echtem Ringüberlauf ⇒ **0 Allokationen**. Der Zähler ist `thread_local` — der Workerthread *darf* allozieren (er rechnet die Messung), und ein globaler Zähler könnte beide nicht auseinanderhalten. Locks: der Produzentenpfad fasst nur Atomics an; `static_assert (std::atomic<std::uint64_t>::is_always_lock_free)` steht im Header, damit kein Mutex sich hinter einem Atomic versteckt. I/O/Logging: keins — die Queue ist JUCE-frei und kennt keinen Strom. Der gesamte Backing-Store entsteht im **Konstruktor**, vor dem Start des Workers; `prepareToPlay` fasst keinen Speicher mehr an. |

---

## 7. Befunde aus diesem Ticket

Kein Befund verschwindet still.

| Befund | Quelle | Gegen die **Quelldatei** verifiziert? | Ausgang |
|---|---|---|---|
| Quarantäne gab einen Zeiger auf `gehalten` heraus, das zwei Zeilen später überschrieben wurde — der Aufrufer hätte den **Nachfolger** bekommen, ununterscheidbar von einem gültigen Wert | T1 beim Schreiben | ☑ ja | **gefixt** vor dem ersten Lauf (eigenes Feld `versiegelterBlock`) |
| `fifo.reset()` aus `prepareToPlay` verstellte beide Enden eines SPSC-Rings vom Nachrichtenthread aus, während der Worker las | T1 (Bestandsbefund, durch den Umbau berührt) | ☑ ja (`PluginProcessor.cpp:70` alt) | **gefixt** — Neuanlauf über `startFolge`, §4.2 |
| Blocklautheit über `kBinOben` wurde in den obersten Bin geklemmt; bei durchweg überlautem Material liegt Γ_r darüber, der Bin fiel durchs relative Gate und `integriert()` lieferte **gar keinen Wert** | B9 §F (adversarial) | ☑ ja | **gefixt** — eigener exakter Eimer mit bekannter Mittelwert-Lautheit |
| Meine Behauptung „LUFS-I bitgleich" war falsch (binweise Summation hat eine andere Reihenfolge, Rest 5,8e-13 LU) | B9 §B | ☑ ja | **Behauptung berichtigt**, nicht der Code: ohne Grenzbin gilt ≤ 1e-9 LU; bitgleich ist der Kurz-LUFS |
| Golden-Meldetext wurde vor der Prüfung gebaut und druckte `akku=0.000000` zu einem grünen Ergebnis | T1 Punkt 4 | ☑ ja | **gefixt** — erst rechnen, dann melden |
| Adversarialer Sweep: 2,78 LU Abweichung, wenn 1 000 Blöcke gemeinsam im Grenzbin liegen | B9 §D | ☑ ja | **widerlegt als Genauigkeitsfehler, gemessen:** die Referenz selbst springt zwischen zwei benachbarten Läufen um **2,7814 LU**, wenn sich der Eingang um 1e-5 ändert — das relative Gate von BS.1770 ist dort **unstetig**. Die Abweichung ist kleiner als der Sprung der Norm. Die Prüfung misst seitdem gegen die Unstetigkeit und zusätzlich: wo `unsicherheitLu() ≤ 0,1 LU` meldet, liegt der Wert auch innerhalb 0,1 LU (76/76) |
| Risiko, nicht gemessen: liefert FL für die Teilstücke eines zerteilten Puffers wirklich dieselbe Projektzeit? | T1 beim Entwurf der Kontinuitätsregel | ☑ ja (Termin-B-Rohdaten gelesen: „Automation nie >1 Punkt je Block bei Offset 0 — FL zerteilt Puffer bis 1 Sample"; ob die Teilstücke die Zeit fortschreiben, steht dort **nicht**) | **NAK-56** — beide Fälle sind heute abgedeckt (fortschreitende Zeit ⇒ Fortsetzung, stehende Zeit ⇒ keine Aussage); der Zähler `analyseKontinuitaetsbrueche()` macht es beim nächsten FL-Termin sichtbar |
| Neue Telemetrie ist auslesbar, aber nirgends angezeigt | T1 Punkt 5 | ☑ ja (`PluginEditor.cpp` unverändert) | **NAK-57** — gehört zur Figma-Übersetzung, nicht in dieses Ticket |
| **Der Beglaubigungsriegel des Beweis-Runners war blind für drei Quellorte** — `plugin/core` (neu), aber auch `plugin/state` (seit SONDE-006; seit S8 der halbe `NakamaKern`) und `plugin/sonde` (seit S9 die Quelle **beider** neuen Bundles). Der Kommentar über der Liste sagt „JEDE Quelle, aus der eine Prüfbinärdatei entsteht" — zwei Tickets lang stimmte das nicht | T1 Punkt 3 beim Nachlesen der ersten Beglaubigung | ☑ ja (`tools/beweise.ps1`, `$quellOrte`) | **gefixt und vorgeführt** — siehe §7.1 |
| Die gedruckte Ortsliste im Manifest war eine **abgeschriebene** Aufzählung neben `$quellOrte` und wäre beim Fix sofort wieder falsch geworden | T1 Punkt 4 (derselbe Fund, eine Ebene weiter) | ☑ ja | **gefixt** — die Zeile erzeugt die Liste jetzt aus `$quellOrte` selbst |

### 7.1 Vorgeführt: der nachgezogene Beglaubigungsriegel

Ein Riegel, der nie angeschlagen hat, beweist nichts. Vorgehen: `LastWriteTime`
von `plugin/core/StampedAudioQueue.h` auf **jetzt** gesetzt, dann beide Fassungen
des Runners ohne `-Bauen` gefahren.

```text
Quelle auf jetzt gesetzt (vorher 08/23/2026 14:02:42)

ALTE Fassung (Stand HEAD, ohne plugin/core):
  Neueste Quelldatei (`plugin/src`, `tests`, `hostbridge`, `vertrag`, `hostprobe`,
  `spike`, `probe`, `cmake`, `third_party/patches`, CMakeLists): **2026-08-23 14:14:37**

NEUE Fassung (mit plugin/core, plugin/state, plugin/sonde):
  Neueste Quelldatei (…): **2026-08-23 14:23:07**
```

Die alte Fassung sieht die berührte Kopfdatei **nicht** — sie meldet den Stand
von 14:14:37 weiter, während die Quelle auf 14:23:07 steht. Die neue Fassung
sieht sie. Genau darin bestand die Blindheit: eine Änderung an
`StampedAudioQueue.h`, `NakamaState.cpp` oder `SondeProcessor.cpp` hätte den
Riegel nicht bewegt, und ein veralteter Lauf wäre als frisch beglaubigt worden.

Der endgültige Kanon-Lauf unten trägt die **erzeugte** Ortsliste — sie kann
nicht mehr von `$quellOrte` abweichen.


---

---

## Kanon-Lauf - SONDE-008

**Lauf:** 2026-08-23 14:32 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 26/26 Kanon-Laeufe bestanden | 3 geplante Pruefung(en) noch nicht gebaut | **Exitcode:** 0

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-08-23 14:32:54 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 7fa1cf5 S10-11/SONDE-008: StampedAudioQueue, Ein-Block-Quarantaene, fixed-memory Loudness |
| Commit (voll) | 7fa1cf5ecaafa0801b6c8e4490f670ebcf9da9c5 |
| Arbeitsbaum | 23 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

<details><summary>Unbestaetigte Dateien im Arbeitsbaum</summary>

```text
M CLAUDE.md
 M briefing-hub/app/briefing-app.tsx
 M briefing-hub/app/globals.css
 M briefing-hub/data/friendly-copy.ts
 M briefing-hub/data/hub.json
 M design/docs/arbeitsplan.md
 M design/docs/oberflaechen-spezifikation.md
 M docs/NEXT-SESSION.md
 M docs/bauaufteilung-sonden.md
 M docs/offene-punkte.md
 M docs/plugin-wissen.md
 D eq-copilot/design/ASSET-KIT.md
 D eq-copilot/design/eq-copilot-material-preview.png
 M eq-copilot/plugin/src/PipeClient.cpp
 M eq-copilot/plugin/src/PipeClient.h
 M eq-copilot/plugin/src/PluginProcessor.cpp
 M eq-copilot/plugin/src/PluginProcessor.h
 M nimbalyst-local/automations/planstand-nakama.md
 M tools/beweise.ps1
?? "Untitled Workspace/"
?? design/docs/interaktions-und-motion-spezifikation.md
?? docs/beweise/SONDE-008.md
?? nimbalyst-local/automations/planstand-nakama/
```

</details>

### Baustand der Pruefbinaries

| Binaerdatei | gebaut am | SHA-256 (16) | Stand |
|---|---|---|---|
| `EqCopNullTest` | 2026-08-23 14:27:00 | `3ECCDD35E46A1913` | frisch (Bau bestaetigt) |
| `EqCopGoldenTest` | 2026-08-23 14:15:40 | `B9550891A25110A1` | frisch (Bau bestaetigt) |
| `EqCopMarkierungTest` | 2026-08-23 14:27:06 | `2CF72B463B86EE99` | frisch (Bau bestaetigt) |
| `EqCopSunaNullTest` | 2026-08-23 02:39:18 | `5B130B91BBB13604` | frisch (Bau bestaetigt) |
| `EqCopProbeeqNullTest` | 2026-08-23 02:39:22 | `8E170931A5BA401C` | frisch (Bau bestaetigt) |
| `EqCopIdentityTest` | 2026-08-23 14:27:12 | `2C949A009B878E57` | frisch (Bau bestaetigt) |
| `EqCopStateMigrationTest` | 2026-08-23 14:27:18 | `66692ADE2F0C2345` | frisch (Bau bestaetigt) |
| `EqCopHostContextTest` | 2026-08-21 01:39:19 | `6A65DC17B0D96C5A` | frisch (Bau bestaetigt) |
| `EqCopHostProbeTest` | 2026-08-21 02:58:34 | `E41677CE401ACFE7` | frisch (Bau bestaetigt) |
| `EqCopSchemaTest` | 2026-08-23 02:13:02 | `787296A1A1A06364` | frisch (Bau bestaetigt) |
| `EqCopQueueStressTest` | 2026-08-23 14:27:25 | `88C288AF7A0BDC48` | frisch (Bau bestaetigt) |
| `EqCopLoudnessGoldenTest` | 2026-08-23 14:13:26 | `774D10FF03DA2B24` | frisch (Bau bestaetigt) |
| `EqCopLebenslaufTest` | 2026-08-23 14:27:30 | `DB3EB84B7D657115` | frisch (Bau bestaetigt) |

Neueste Quelldatei (`plugin/src`, `plugin/core`, `plugin/state`, `plugin/sonde`, `plugin/tests`, `plugin/hostbridge`, `plugin/vertrag`, `plugin/hostprobe`, `plugin/spike`, `plugin/probe`, `cmake`, `third_party/patches`, `plugin/CMakeLists.txt`, `CMakeLists.txt`): **2026-08-23 14:25:48**. `cargo test` uebersetzt selbst und ist damit immer frisch.

Der Zeitstempelvergleich ist hier nicht der Massstab: `-Bauen` hat unmittelbar vor diesem Lauf erfolgreich gebaut, das Buildsystem hat die Abhaengigkeiten also selbst geprueft.

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,06 s | [↓ A1](#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,36 s | [↓ A2](#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung. | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 5,68 s | [↓ A3](#a3) |
| A4 | Broker-Vertragstests gruen (Framing, Protokoll, Bindung, Aggregat, Server). | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 0,53 s | [↓ A4](#a4) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 0,61 s | [↓ A5](#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,11 s | [↓ A6](#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,12 s | [↓ A7](#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,15 s | [↓ A8](#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,23 s | [↓ A9](#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 0,62 s | [↓ A10](#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,22 s | [↓ A11](#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,22 s | [↓ A12](#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,18 s | [↓ A13](#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet: NakamaKern.lib enthaelt keinen eingefrorenen Identitaetswert (Namen, Viercodes, CIDs roh und COM-vertauscht) und genau seine eigenen Uebersetzungseinheiten, kein JUCE-Modulobjekt; die Gegenprobe findet dieselben Werte im gebauten EQ-Copilot-Bundle. | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 0,34 s | [↓ A14](#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete BUNDLE-ORDNER, `art` ist eine geschlossene Menge, jedes Ziel genau einmal, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile behauptet keine Pruefung ohne Mittel, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede Regel faellt an einem verdorbenen Manifest; Ordner-Hash v1 bytegleich in Python und PowerShell, Nicht-ASCII bricht beide Haelften ab. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,79 s | [↓ A17](#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 3,64 s | [↓ A18](#a18) |
| A15 | Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | [OK] Exit 0 | 0,05 s | [↓ A15](#a15) |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,05 s | [↓ A16](#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B1](#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar (gleiche instance_id, verschiedene runtime_nonce) und aufloesbar, Host-Dirty; Parametertabelle deckungsgleich mit dem Vertrag; RFC-8785-state_hash bytegleich zu Python und Rust. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 0,10 s | [↓ B2](#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [↓ B3](#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,08 s | [↓ B3b](#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert den Fixture-Korpus wie das Manifest (Urteil UND Verletzungsmenge), Bandgitter und Quantisierung bitgleich. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,15 s | [↓ B3c](#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 1,76 s | [↓ B4](#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht, l_j == -70,0 exakt und ueber dem Gitter geklemmt bleiben wertgleich. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,11 s | [↓ B9](#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [GEPLANT] geplant (ab P2) | - | - |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B8 | Lifecycle-Klassifikation §53.5: unclassified beim Laden und audio-neutral; Schema-1 sensor\|pre\|post -> legacy (immer passiv), hub bzw. bestaetigter Schema-2-Main-State -> main; ein Scannerlauf klassifiziert nicht; read-only nimmt die Klassifikation zurueck; Brokerstart nur fuer main mit offenem Editor; die Sondenbundles bleiben bis gueltigem State neutral und werden nie main. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,14 s | [↓ B8](#b8) |

### Rohe Ausgaben

<a id="a1"></a>
#### A1 | EqCopNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,06 s

stdout:

```text
  ok      Latenz ist 0 Samples
  ok      Tail ist 0 s
  ok      1000 Bloecke Rauschen: Ausgang bitgleich (Nulltest)
  ok      kein falscher NaN-Alarm bei endlichem Signal
  ok      NaN/Inf-Block: Ausgang bitgleich (nicht gefiltert)
  ok      NaN/Inf wurde als Mangel registriert
  ok      Mono-Layout wird angenommen
  ok      5.1 wird klar abgelehnt (kein stilles Downmixen)
  ok      Save/Load: Sensor-ID bleibt stabil
  ok      Save/Load: Rolle und Paar-ID bleiben
NULLTEST OK
```

stderr:

_(leer)_

<a id="a2"></a>
#### A2 | EqCopGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | **Exitcode:** 0 | **Dauer:** 9,36 s

stdout:

```text
sinus-1k-minus12     32 ok, 0 Fehler
pink-minus20         39 ok, 0 Fehler
resonanz-116hz       41 ok, 0 Fehler
stille-mit-tick       7 ok, 0 Fehler
diag-scoop-mitte     10 ok, 0 Fehler
diag-mulm            12 ok, 0 Fehler
diag-haerte           8 ok, 0 Fehler
diag-mulm-halb       10 ok, 0 Fehler
diag-wander-ton      25 ok, 0 Fehler
stereo-antiphase     10 ok, 0 Fehler
stereo-nur-links      8 ok, 0 Fehler
stereo-nur-rechts     5 ok, 0 Fehler
stereo-dekorr         7 ok, 0 Fehler
nan-recovery          9 ok, 0 Fehler
nyquist-32k           4 ok, 0 Fehler
leicht-publikation    8 ok, 0 Fehler
konvergenz            4 ok, 0 Fehler

GOLDEN OK — 239 Pruefungen ok, 0 Fehler (metrics m4.1-2026-08-15 · diagnose m4.1-2026-08-15)
```

stderr:

_(leer)_

<a id="a3"></a>
#### A3 | EqCopMarkierungTest

**Befehl:** `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | **Exitcode:** 0 | **Dauer:** 5,68 s

stdout:

```text
  ok      T7: NaN-Feld verhindert Engage
  ok      T7: fVon>fBis verhindert Engage
  ok      T7: fBis wird an der Nyquist-Kappe gefangen (Zone baut)
  ok      T7: 1/12-Okt-Resonanz-Puls baut (engste Karte)
  ok      T7: Puls-Stufe 0 ist exakte Identitaet (b == a)
  ok      T2: als Main klassifiziert (§53.5)
  ok      T2: Zonen-Solo-Auftrag (Mulm 120-300) baut
  ok      T2: Solo greift (Ausgang weicht ab)
  ok      T2: markierungHoerbar meldet aktiv
  ok      T2: Fade-out endet im erwarteten Fenster
  ok      T2: nach dem Fade wieder bitgleich transparent
  ok      T2: markierungHoerbar meldet aus
  ok      T5: Ausgang bleibt endlich
  ok      T5: kein Klick an Engage-/Disengage-Kanten
  ok      T9: als Main klassifiziert (§53.5)
  ok      T9: Puls-Auftrag 689 Hz baut
  ok      T9: Schwellphase ist deutlich hoerbar
  ok      T9: Ruhephase <= -120 dBFS Rest (Identitaet)
  ok      T6: als Main klassifiziert (§53.5)
  ok      T6: der geladene Main-Stand klassifiziert die neue Instanz (§53.5)
  ok      T6: Save/Load laedt IMMER neutral (kein Markierungszustand)
  ok      T7: als Main klassifiziert (§53.5)
  ok      T7: Mono-Solo laeuft und bleibt endlich
  ok      T3: als Main klassifiziert (§53.5)
  ok      T3: Freilauf mit gesetztem Latch — JEDES Sample bitgleich (Render sauber)
  ok      T3: Freilauf meldet den Latch-Fall an den Editor
  ok      T3: Echtzeit-Beweis dauert mindestens ~2 Fenster (kein Fruehstart)
  ok      T3: Echtzeit-Taktung schaltet die Markierung frei
  ok      T3: isNonRealtime schneidet sofort auf neutral
  ok      T10: Markierung nach Offline-Ende wieder beweisbar
  ok      T10: Transport-Stopp schneidet sofort auf neutral
  ok      T4: beide Instanzen als Main klassifiziert (§53.5)
  ok      T4: Markierung war waehrend der Messung hoerbar
  ok      T4: keine Analyse-Drops (Vergleich belastbar)
  ok      T4: beide Messungen messbereit
  ok      T4: genug Baender fuer den Vergleich
  ok      T4: LTAS mit/ohne Markierung gleich (Abgriff vor Faerbung)
MARKIERUNGSTEST OK
```

stderr:

_(leer)_

<a id="a4"></a>
#### A4 | broker

**Befehl:** `cargo test --manifest-path broker/Cargo.toml --color never` | **Exitcode:** 0 | **Dauer:** 0,53 s

stdout:

```text

running 51 tests
test framing::tests::abbruch_mitten_im_frame ... ok
test framing::tests::laengengrenze_beidseitig ... ok
test protokoll::tests::feindliches_ltas_array_faellt_am_guard ... ok
test framing::tests::hin_und_zurueck ... ok
test framing::tests::kein_utf8_wird_verworfen ... ok
test aggregat::tests::profilfilter_laesst_fremde_sensoren_nie_still_hinein ... ok
test aggregat::tests::schnittfenster_und_paare_im_dokument ... ok
test aggregat::tests::filter_trennt_prozesse_und_v1_bekommt_warnung ... ok
test protokoll::tests::heartbeat_v1_ohne_measurement_parst_weiter ... ok
test aggregat::tests::schreiben_erzeugt_datei_im_snapshot_ordner ... ok
test bindung::tests::runde_laden_schreiben_laden ... ok
test bindung::tests::beschaedigte_datei_ist_sichtbarer_fehler ... ok
test protokoll::tests::heartbeat_v2_mit_messstand_und_fenster ... ok
test protokoll::tests::unbekannter_typ_ist_parsefehler_kein_absturz ... ok
test protokoll::tests::v1_hello_bleibt_angenommen_und_welcome_spiegelt_v1 ... ok
test protokoll::tests::v2_hello_mit_nonce_wird_angenommen ... ok
test protokoll::tests::version_3_bekommt_reject_mit_spanne ... ok
test register_tests::konflikt_kommt_und_geht_mit_der_zweiten_verbindung ... ok
test register_tests::paar_ausgerichtet_bei_deckungsgleichen_fenstern ... ok
test register_tests::paar_in_verschiedenen_prozessen_hoechstens_wahrscheinlich ... ok
test register_tests::paar_mit_v1_partner_bleibt_unklar_mit_klarem_grund ... ok
test register_tests::paar_ohne_partner_ist_unvollstaendig ... ok
test register_tests::paar_wird_bei_spruengen_herabgestuft ... ok
test register_tests::paar_wird_bei_teilueberdeckung_herabgestuft_und_ohne_ueberlappung_unklar ... ok
test register_tests::profilbindung_haelt_ueber_reconnect ... ok
test register_tests::sechzehn_sensoren_werden_korrekt_benannt ... ok
test register_tests::stale_wird_sichtbar_aber_nie_entfernt ... ok
test register_tests::zwei_fl_prozesse_bleiben_getrennte_sessions ... ok
test server::tests::falsche_protokollversion_bekommt_reject ... ok
test server::tests::user_sid_ist_wohlgeformt ... ok
test server::tests::feindliches_ltas_array_wird_gezaehlt_verworfen ... ok
test server::tests::zweiter_broker_auf_demselben_namen_scheitert_sichtbar ... ok
test server::tests::v2_heartbeat_bekommt_ack_und_messstand_landet_im_register ... ok
test vertrag::tests::additiv_ohne_maxproperties_bricht_das_laden ... ok
test vertrag::tests::discriminator_als_zahl_bricht_das_laden ... ok
test vertrag::tests::fehlendes_pflichtfeld_zeigt_auf_das_elternobjekt ... ok
test vertrag::tests::haengende_referenz_bricht_das_laden ... ok
test vertrag::tests::integer_akzeptiert_1punkt0 ... ok
test vertrag::tests::laenge_zaehlt_codepunkte_nicht_bytes ... ok
test vertrag::tests::maxlength_als_gleitkommazahl_bricht_das_laden ... ok
test vertrag::tests::oneof_ohne_discriminator_bricht_das_laden ... ok
test vertrag::tests::required_mit_nicht_string_bricht_das_laden ... ok
test vertrag::tests::type_als_zahl_bricht_das_laden ... ok
test vertrag::tests::typfehler_erzeugt_keine_lawine ... ok
test vertrag::tests::unbekannter_discriminator_wird_abgelehnt ... ok
test vertrag::tests::unbekanntes_schluesselwort_bricht_das_laden ... ok
test vertrag::tests::verletzungen_sind_kanonisch_sortiert_und_doppelfrei ... ok
test vertrag::tests::zahlengleichheit_ist_numerisch ... ok
test server::tests::feindliches_laengenpraefix_beendet_nur_diese_verbindung ... ok
test server::tests::handshake_heartbeat_und_geordneter_abschied ... ok
test server::tests::doppelte_sensor_id_wird_als_konflikt_sichtbar ... ok

test result: ok. 51 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.10s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s


running 7 tests
test bandwertgrenzen_stimmen_mit_dem_vertrag ... ok
test quantisierung_stimmt_mit_den_testvektoren ... ok
test textriegel_deckt_die_gemeinsame_falltabelle ... ok
test bandgitter_ist_lesbar_und_in_sich_stimmig ... ok
test fb_korpus_klassifiziert_wie_das_manifest ... ok
test jcs_fixtures_stimmen_mit_manifest ... ok
test korpus_klassifiziert_wie_das_manifest ... ok

test result: ok. 7 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.09s


running 0 tests

test result: ok. 0 passed; 0 failed; 0 ignored; 0 measured; 0 filtered out; finished in 0.00s
```

stderr:

```text
    Finished `test` profile [unoptimized + debuginfo] target(s) in 0.06s
     Running unittests src\lib.rs (broker\target\debug\deps\eqcop_broker-3bd50970b42ce1c0.exe)
     Running unittests src\main.rs (broker\target\debug\deps\eqcop_broker-2cd7b6c7665d865b.exe)
     Running unittests src\bin\eqcop-broker-probe.rs (broker\target\debug\deps\eqcop_broker_probe-00ee01d68ca97620.exe)
     Running tests\contract_cross_language.rs (broker\target\debug\deps\contract_cross_language-76d6c7b1dc93a662.exe)
   Doc-tests eqcop_broker
```

<a id="a5"></a>
#### A5 | pruefe_v3_vertrag.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | **Exitcode:** 0 | **Dauer:** 0,61 s

stdout:

```text
jsonschema 4.26.0 (draft 2020-12)
47 Definitionen, 17 Nachrichtenfamilien, 153 Fixtures
Pruefungen: 177 bestanden, 0 gescheitert

Abdeckung je Schluesselwort (mit Negativfixture / vorhanden).
HART sind nur enum und const: dort ist ein falscher Wert ein echtes
Produktrisiko. Die uebrigen Quoten stehen hier VOLLSTAENDIG, damit
Unterabdeckung sichtbar bleibt statt still zu sein.
  additionalProperties      6 / 38  
  const                     4 / 4    HART
  enum                     14 / 14   HART
  exclusiveMinimum          2 / 2   
  items                     1 / 4   
  maxItems                  3 / 4   
  maxLength                 2 / 15  
  maxProperties             2 / 3   
  maximum                   4 / 14  
  minItems                  2 / 4   
  minLength                 4 / 9   
  minimum                   6 / 40  
  oneOf                     4 / 4   
  pattern                   1 / 1   
  required                 38 / 169 
  type                      8 / 134 

$defs ohne Negativfixture: keine
```

stderr:

```text
C:\Users\phili\Projekte\Nakama\tools\eq-copilot\pruefe_v3_vertrag.py:610: DeprecationWarning: Accessing jsonschema.__version__ is deprecated and will be removed in a future release. Use importlib.metadata directly to query for jsonschema's version.
  print(f"jsonschema {jsonschema.__version__} (draft 2020-12)")
```

<a id="a6"></a>
#### A6 | erzeuge_bandgitter.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
221 Baender (fein, nakama_1_24_oct_30_18k_v1)
64 Gruppen (grob, nakama_log64_v1), Summe 221
Gruppengroessen: [3, 4]
Kanten gesamt: 30.360373 Hz .. 17959.392773 Hz
Mitten gesamt: 30.801975 Hz .. 17701.912526 Hz
Pruefungen: 26 bestanden, 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_1_24_oct_30_18k_v1.json sha256=61db4ff041660efbfd8226d572180b1c80d3cd495be5165c1758b35cdf39a7ba
  bytegleich: eq-copilot\schemas\v3\bandgitter\nakama_log64_v1.json sha256=8f6645b20a55e747ffcb835e9b7a0104eab3a8fa556e00d4b98126a0bacaee75
```

stderr:

_(leer)_

<a id="a7"></a>
#### A7 | erzeuge_quantisierung.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,12 s

stdout:

```text
3 Kodierungen, 61 Testvektoren
Pruefungen: 0 gescheitert
  bytegleich: eq-copilot\schemas\v3\quantisierung-v1.json sha256=147ab43a6ab87f3e5bc8843df52cd9a9aa9572e50a0556e1ad1e5d53c931c3d1
```

stderr:

_(leer)_

<a id="a8"></a>
#### A8 | erzeuge_v3_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
36 gueltige, 117 ungueltige Fixtures
  bytegleich: 155 Dateien, MANIFEST sha256=bc72c521906af48592413ffa28064bdeb8feddee2955ec50093fbdda4a421b2c
```

stderr:

_(leer)_

<a id="a9"></a>
#### A9 | pruefe_flatc_drift.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | **Exitcode:** 0 | **Dauer:** 0,23 s

stdout:

```text
Gepinnt: flatbuffers 25.12.19 @ 7e163021e59c
  flatc: 25.12.19  (C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatc.exe)
  Rust-Crate: 25.12.19 (Cargo.lock; Anforderung 25.12.19)
  Feld-IDs: 0 rot

Codegen-Drift:
  ok:  eq-copilot/plugin/vertrag/generiert/nakama_telemetry_v1_generated.h  1190 Zeilen  sha256=4a5990a207892734...
  ok:  broker/src/generiert/nakama_telemetry_v1_generated.rs  2026 Zeilen  sha256=d327702c8f318daa...

Drift: 0 Dateien
```

stderr:

_(leer)_

<a id="a10"></a>
#### A10 | erzeuge_fb_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,62 s

stdout:

```text
9 gueltige, 42 ungueltige Fixtures
  bytegleich: 52 Dateien, MANIFEST sha256=15bcde347cfa559c0b2992c5f9f5f6e7583f64e7b4ba33347b4c46f808942a35
```

stderr:

_(leer)_

<a id="a11"></a>
#### A11 | pruefe_v2_schemas.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | **Exitcode:** 0 | **Dauer:** 0,22 s

stdout:

```text
Datei                            $id                                        JSON  Metaschema
eq-aggregat.schema.json          evenacadia.eq-copilot.aggregat.v1          ok    ok
eq-ipc.schema.json               evenacadia.eq-copilot.ipc.v2               ok    ok
eq-measurement.schema.json       evenacadia.eq-copilot.measurement.v1       ok    ok
eq-report.schema.json            evenacadia.eq-copilot.report.v1            ok    ok
eq-snapshot.schema.json          evenacadia.eq-copilot.snapshot.v3          ok    ok

GRUEN — 5 v2-Vertraege: gueltiges JSON, gueltiges Schema, $id eingefroren.
```

stderr:

_(leer)_

<a id="a12"></a>
#### A12 | erzeuge_state_fixtures.py

**Befehl:** `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | **Exitcode:** 0 | **Dauer:** 0,22 s

stdout:

```text
Parameterbestand: 109 Parameter, 109 eindeutige IDs, 5 global + 13 x 8 Slots
RFC-8785-Tabelle: 24 Zahlen gegen den RFC-Text bestaetigt, 28 Handvektoren bestaetigt, 2 abgelehnt (NaN, Infinity)
JCS-Dokumente: 13 Hand-Erwartungen von rfc8785 bestaetigt
DTO: 6 gueltige gehasht, 15 ungueltige mit Grund
STATE-FIXTURES OK (36 Dateien bytegleich, keine verwaiste Datei)
```

stderr:

_(leer)_

<a id="a13"></a>
#### A13 | pruefe_host_capabilities.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | **Exitcode:** 0 | **Dauer:** 0,18 s

stdout:

```text
  ok      capabilities entspricht v3 $defs/capabilities (10 Bits, supported|unsupported, strikt)
  ok      genau die zehn Schluessel aus §53.6
  ok      jedes Bit hat einen Beleg
  ok      host_context_presence: Kontext in allen 259298 Bloecken, bruecke_liefert
  ok      project_time_samples: alle sieben Kontextfelder IMMER gueltig
  ok      project_time_samples: Spruenge 2 vor / 51 zurueck / 5 ueber Stop, 0 negativ
  ok      project_time_samples: Render als 2587 Offline-Bloecke mit offline_an/aus
  ok      sample_accurate_automation: 83303 Punkte, max 1 je Block, Offset immer 0, nicht belegt
  ok      Blockgroessen 1..4096 (FL zerteilt an Automationspunkten)
  ok      presentation_latency: Eingang 3924 / Ausgang 4410 gemeldet
  ok      presentation_latency: genau ein verworfener Wertwechsel (NAK-43)
  ok      float64_processing: nur float, nie double
  ok      Rohdateien tragen host=FruityLoops, wrapper=VST3, Samplerate wie im Report
  ok      keine Kontextverluste, verworfenen Busmeldungen, Ueberlaeufe oder unplausiblen Punkte
  ok      Report nennt FL- und JUCE-Version (§54 Lieferumfang 6)
  ok      Ereignisprotokoll vollstaendig (98 Ereignisse, kein Ringueberlauf)
  ok      8 Transportabschnitte (an..aus) gefunden: 8
  ok      Smart Disable: stummes Fenster Ereignisse 61..66 ohne zeitsprung_vor, Wrap-Kadenz [3003, 3003] regelmaessig
  ok      Smart Disable: stummes Fenster Ereignisse 88..96 ohne zeitsprung_vor, Wrap-Kadenz [2833, 2833, 2832, 2834] regelmaessig
  ok      genau zwei Vorwaertsspruenge, beide in der Nachmessung (Ereignisse [79, 82])
  ok      Termin A 001701: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 001701: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: priority_sidechain aktiv, 2 Kanaele, Versatz 0
  ok      Termin A 002722: compare_pre aktiv, 2 Kanaele, Versatz 0
  ok      47 'feld = wert'-Angaben der Belegtexte gegen die Rohdateien aufgeloest, alle stimmen
  ok      host_context_presence=supported traegt einen Termin (B)
  ok      host_context_presence=supported behauptet kein 'Golden nicht erbracht'
  ok      host_context_presence: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      project_time_samples=supported traegt einen Termin (B)
  ok      project_time_samples=supported behauptet kein 'Golden nicht erbracht'
  ok      project_time_samples: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      sample_accurate_automation=unsupported traegt den festen Fallback aus §53.6
  ok      sample_accurate_automation: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency=unsupported traegt den festen Fallback aus §53.6
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      presentation_latency: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_compare_pre=unsupported traegt den festen Fallback aus §53.6
  ok      aux_compare_pre: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      aux_priority_sidechain=unsupported traegt den festen Fallback aus §53.6
  ok      aux_priority_sidechain: Rohdatei existiert (docs/beweise/termin-a/aux-spike-20260822-001701.json)
  ok      contribution_aux=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing=unsupported traegt den festen Fallback aus §53.6
  ok      float64_processing: Rohdatei existiert (docs/beweise/termin-b/host-probe-20260822-132644.json)
  ok      binary_telemetry=unsupported traegt den festen Fallback aus §53.6
  ok      remote_control=unsupported traegt den festen Fallback aus §53.6
  ok      die zehn Bits stehen so, wie die Rohdaten es tragen (2 supported, 8 unsupported)
  ok      herabgestufte Bits tragen die Fallbacks aus §53.6
  ok      gemessene_hosttatsachen: Blockgroessen und Tempo stimmen mit den Rohfeldern
  ok      Belegtext presentation_latency nennt genau die gemeldeten Latenzwerte
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 38 (zeitsprung_zurueck 1969)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 40 (zeitsprung_zurueck 394)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 41 (zeitsprung_zurueck 1575)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 79 (zeitsprung_vor 49901)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 82 (zeitsprung_vor 65960)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 84 (zeitsprung_zurueck -38559)
  ok      gemessene_hosttatsachen.seeks nennt Ereignis 50 (zeitsprung_zurueck -78836)
  ok      Song-Loop vor Export = [539634, 539635] Samples wie im Report
  ok      Pattern-Loop = 173250 Samples wie im Report
  ok      Song-Loop nach Export = [509118, 509119] Samples wie im Report (509119)
  ok      der erste Bericht (75 Ereignisse) ist das Praefix des zweiten
  ok      ereignisse_je_art stimmt mit der Ereignisliste ueberein

HOST-CAPABILITIES OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a14"></a>
#### A14 | pruefe_kern_identitaetsfrei.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | **Exitcode:** 0 | **Dauer:** 0,34 s

stdout:

```text
Kern      : eq-copilot\build\plugin\Release\NakamaKern.lib  (709564 Byte)
Gegenprobe: eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3  (7052800 Byte)
Nadeln    : 17 aus eq-copilot\identity\plugin-identities-v1.json

[0] Frische - misst dieses Bein den aktuellen Quellstand?
  ok      NakamaKern.lib ist nicht aelter als die Kernquellen
  ok      die Lib wurde mit der heutigen Definemenge gebaut (15 aus der Projektdatei)

[1] Gegenprobe - findet der Scanner die Werte dort, wo sie stehen muessen?
  ok      Gegenprobe findet hersteller.name = 'evenacadia' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.produktname = 'EQ-Copilot' im gebauten Bundle  [ascii,utf-16le]
  ok      Gegenprobe findet main.plugin_code = 'Eqcp' im gebauten Bundle  [ascii]
  ok      Gegenprobe findet main.component_cid = 'ABCDEF019182FAEB45766E6145716370' im gebauten Bundle  [roh16-com]
  ok      Gegenprobe findet main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' im gebauten Bundle  [roh16-com]

[2] Kern - keine dieser Nadeln darf im Objektcode des Kerns liegen
  ok      NakamaKern.lib traegt active-probe.bundle = 'Nakama Probeeq.vst3' NICHT
  ok      NakamaKern.lib traegt active-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B4163' NICHT
  ok      NakamaKern.lib traegt active-probe.plugin_code = 'NkAc' NICHT
  ok      NakamaKern.lib traegt active-probe.produktname = 'Nakama Probeeq' NICHT
  ok      NakamaKern.lib traegt hersteller.code = 'Evna' NICHT
  ok      NakamaKern.lib traegt hersteller.name = 'evenacadia' NICHT
  ok      NakamaKern.lib traegt main.bundle = 'EQ-Copilot.vst3' NICHT
  ok      NakamaKern.lib traegt main.component_cid = 'ABCDEF019182FAEB45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.controller_cid = 'ABCDEF011234ABCD45766E6145716370' NICHT
  ok      NakamaKern.lib traegt main.plugin_code = 'Eqcp' NICHT
  ok      NakamaKern.lib traegt main.produktname = 'EQ-Copilot' NICHT
  ok      NakamaKern.lib traegt passive-probe.bundle = 'Nakama Suna.vst3' NICHT
  ok      NakamaKern.lib traegt passive-probe.component_cid = 'ABCDEF019182FAEB45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.controller_cid = 'ABCDEF011234ABCD45766E614E6B5072' NICHT
  ok      NakamaKern.lib traegt passive-probe.plugin_code = 'NkPr' NICHT
  ok      NakamaKern.lib traegt passive-probe.produktname = 'Nakama Suna' NICHT

[3] Bauform - der Kern enthaelt genau seine eigenen Objekte
  ok      Archivmitglieder sind genau die 5 Kernobjekte
  ok      kein JUCE-Modulobjekt im Kern (die Kopf-Fassade haelt)

26 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a17"></a>
#### A17 | pruefe_installer_manifest.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | **Exitcode:** 0 | **Dauer:** 0,79 s

stdout:

```text
[1] Struktur - eine Identitaet, ein Ort
  ok      Manifest traegt das Vertragsschema nakama.installer/v1  [nakama.installer/v1]
  ok      es zeigt auf die eingefrorene Identitaetsdatei  [eq-copilot/identity/plugin-identities-v1.json]
  ok      jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      jedes Ziel der Identitaetsdatei hat genau einen VST3-Eintrag  [3 vs 3]
  ok      jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      kein Viercode, keine Class-ID im Installer-Manifest
  ok      genau ein Broker-Artefakt, aus dem Release-Pfad der Crate  [broker/target/release/eqcop-broker.exe]
  ok      der Broker-Binaername kommt aus broker/Cargo.toml  [eqcop-broker]
  ok      VST3 nach Common Files, Broker geschuetzt unter Program Files  [C:/Program Files/Common Files/VST3 | C:/Program Files/evenacadia/Nakama]
  ok      die Signaturzeile behauptet keine Pruefung ohne Mittel  [kein Zertifikat, Grund steht da]
  ok      jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[2] Gegenprobe - dieselben Regeln an einem verdorbenen Manifest
  ok      faellt am verdorbenen Manifest: Manifest traegt das Vertragsschema nakama.installer/v1
  ok      faellt am verdorbenen Manifest: es zeigt auf die eingefrorene Identitaetsdatei
  ok      faellt am verdorbenen Manifest: jede `art` ist vst3 oder broker - eine geschlossene Menge
  ok      faellt am verdorbenen Manifest: jedes Ziel der Identitaetsdatei hat genau einen VST3-Eintrag
  ok      faellt am verdorbenen Manifest: jeder Quellpfad ist der Bundle-ORDNER aus Ziel + Identitaet
  ok      faellt am verdorbenen Manifest: kein Viercode, keine Class-ID im Installer-Manifest
  ok      faellt am verdorbenen Manifest: genau ein Broker-Artefakt, aus dem Release-Pfad der Crate
  ok      faellt am verdorbenen Manifest: der Broker-Binaername kommt aus broker/Cargo.toml
  ok      faellt am verdorbenen Manifest: VST3 nach Common Files, Broker geschuetzt unter Program Files
  ok      faellt am verdorbenen Manifest: die Signaturzeile behauptet keine Pruefung ohne Mittel
  ok      faellt am verdorbenen Manifest: jedes sha256 ist null oder ein SHA-256 in Grossbuchstaben
  ok      faellt am verdorbenen Manifest: jeder bekannte Stand traegt Hash, hash_art, Ziel und state_schema
  ok      faellt am verdorbenen Manifest: der Rueckweg ist vollstaendig beschrieben (NAK-41 benannt)

[3] Auslieferungsstand
  hinweis nicht ausgeliefert - 4 Artefakt(e) ohne Hash: main, passive-probe, active-probe, eqcop-broker.exe
          Install-Nakama.ps1 bricht in diesem Zustand ab (hashes_null_bedeutet).

[4] Ordner-Hash v1 - Python gegen PowerShell
  ok      die PowerShell-Haelfte laeuft durch
  ok      Python liefert einen SHA-256  [9DF0E95A3747AFBA]
  ok      beide Sprachen bilden BYTEGLEICH denselben Ordner-Hash  [py 9DF0E95A3747AFBA | ps 9DF0E95A3747AFBA]
  ok      Nicht-ASCII im Pfad bricht ab (Python)
  ok      Nicht-ASCII im Pfad bricht ab (PowerShell)  [Exit 1]

31 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a18"></a>
#### A18 | pruefe_installer_gegenpfad.py

**Befehl:** `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | **Exitcode:** 0 | **Dauer:** 3,64 s

stdout:

```text
[0] Sandbox-Kopie des ECHTEN Skripts, genau eine Zeile ersetzt
  ok      Riegel 3 ('Ruhe') steht unveraendert im Original
      - $null -ne (Get-Process -Name 'FL64', 'FL', 'FL Studio' -ErrorAction SilentlyContinue)
      + $false   # SANDBOX-PROBE (Kanon-Bein A18): kein Host haelt hier ein Bundle

[1] Installieren
  ok      Installationslauf endet mit Exit 0  [Exit 0]

[2] Die Auslieferungseinheit ist der Ordner (T2-5 a/b)
  ok      main: Bundle traegt moduleinfo.json  [EQ-Copilot.vst3]
  ok      passive-probe: Bundle traegt moduleinfo.json  [Nakama Suna.vst3]
  ok      active-probe: Bundle traegt moduleinfo.json  [Nakama Probeeq.vst3]
  ok      main: moduleinfo.json ist die NEUE Fassung, nicht die des Vorstands
  ok      main: die Datei des Vorstands ist WEG - getauscht, nicht gemischt
  ok      Broker liegt im selbst angelegten Verzeichnis

[3] -Pruefen sieht den ganzen Ordner (T2-5 c)
  ok      alle vier Artefakte melden `aktuell`  [4x]
  ok      eine Aenderung NUR am moduleinfo.json faellt auf

[4] NAK-41: unbekannter Vorstand verweigert den Rueckweg
  ok      frischer Installationslauf ueber den Vorstand  [Exit 0]
  ok      Rueckweg verweigert, NAK-41 benannt  [Exit 1]
  ok      der unbekannte Vorstand zaehlt wie aelter
  ok      die verweigerte Wiederherstellung hat ihre eigene Quelle NICHT zerstoert

[5] Das datei-innen-Nachschlagen findet einen historischen Stand
  ok      Ordner-Hash und Dateihash des Vorstands sind verschieden  [innen 029EDDFF4223 | ordner 23687ACF5AF5]
  ok      Rueckweg laeuft jetzt ohne -Erzwingen durch  [Exit 0]

[6] Der Rueckweg stellt den VORZUSTAND her (T2-5 d)
  ok      main: auch die Datei des Vorstands ist zurueck - der ganze Ordner
  ok      main: bytegleich zum Vorzustand  [23687ACF5AF54865]
  ok      passive-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Suna.vst3]
  ok      active-probe: KEIN leeres .vst3-Gehaeuse zurueckgeblieben  [Nakama Probeeq.vst3]
  ok      Broker entfernt
  ok      das selbst angelegte Verzeichnis BLEIBT, weil fremde Dateien darin liegen

[7] Leer geraeumt verschwindet die selbst angelegte Kette
  ok      Vorzustand: die Broker-Kette gibt es nicht
  ok      der naechste Lauf legt die Kette selbst an
  ok      die ganze selbst angelegte Kette ist weg
  ok      das VORGEFUNDENE Zielverzeichnis bleibt unangetastet

[8] Der art-Riegel im Ausfuehrenden (T2-4)
  ok      eine dritte Artefaktsorte bricht ab, statt im Broker-Zweig zu landen  [Exit 1]

27 ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a15"></a>
#### A15 | EqCopSunaNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse passive_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [passive_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [172 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [passive_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [172 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="a16"></a>
#### A16 | EqCopProbeeqNullTest

**Befehl:** `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
SONDE-NULLTEST — nakama-sonde-testschale (Produktklasse active_probe)
  ok      frischer Zustand traegt die Produktklasse des Bundles  [active_probe]
  ok      der Bundlevertrag laesst die eigene Produktklasse zu
  ok      der Bundlevertrag laesst 'main' NICHT zu
  ok      der Bundlevertrag laesst 'legacy' NICHT zu
  ok      das Bundle meldet dem Host keinen Parameter  [0]
  ok      keine erfundene Oberflaeche (Gestaltung kommt aus Figma)
  ok      frische Instanz ist neutral, trotz fester Produktklasse  [unclassified]
  ok      eine Sonde darf den Broker nie starten
  ok      bitgleich bei 44100 Hz, Block 1
  ok      0 Samples Latenz bei 44100 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 16
  ok      0 Samples Latenz bei 44100 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 64
  ok      0 Samples Latenz bei 44100 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 512
  ok      0 Samples Latenz bei 44100 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 44100 Hz, Block 4096
  ok      0 Samples Latenz bei 44100 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 1
  ok      0 Samples Latenz bei 48000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 16
  ok      0 Samples Latenz bei 48000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 64
  ok      0 Samples Latenz bei 48000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 512
  ok      0 Samples Latenz bei 48000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 48000 Hz, Block 4096
  ok      0 Samples Latenz bei 48000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 1
  ok      0 Samples Latenz bei 96000 Hz, Block 1  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 16
  ok      0 Samples Latenz bei 96000 Hz, Block 16  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 64
  ok      0 Samples Latenz bei 96000 Hz, Block 64  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 512
  ok      0 Samples Latenz bei 96000 Hz, Block 512  [0]
  ok      kein MIDI erzeugt
  ok      bitgleich bei 96000 Hz, Block 4096
  ok      0 Samples Latenz bei 96000 Hz, Block 4096  [0]
  ok      kein MIDI erzeugt
  ok      kein Tail  [0]
  ok      Stille bleibt bitgenau still
  ok      Zustand laesst sich speichern  [3439 Bytes]
  ok      geladener Zustand ist derselbe (Instanz-ID, Klasse, Position, Label)
  ok      der eigene Stand kommt NICHT als read-only zurueck
  ok      nach gueltigem State traegt der Lebenslauf die Produktklasse  [active_probe]
  ok      speichern -> laden -> speichern ist bytegleich  [3439 Bytes]
  ok      Muellbytes lassen den gehaltenen Zustand unveraendert
  ok      Nullzeiger/Laenge 0 lassen den gehaltenen Zustand unveraendert

SONDE-NULLTEST OK - 62 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b1"></a>
#### B1 | EqCopIdentityTest

**Befehl:** `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

stdout:

```text
  ok      Identitaetsmanifest gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\identity\plugin-identities-v1.json]
  ok      Manifest ist gueltiges JSON
  ok      Herstellercode im Manifest  [Evna]
  ok      Viercode ist vier Zeichen lang: hersteller.code  [4 Zeichen]
  ok      Manifest kennt drei Ziele  [3]
  ok      Manifest kennt das heutige Ziel 'main'
  ok      jedes Ziel im Manifest hat hier eine Zeile  [3 vs 3]
  ok      plugin/CMakeLists.txt gefunden
  ok      CMake: Zielblock juce_add_plugin(EqCopilot) gefunden
  ok      CMake: Defineblock von EqCopilot gefunden
  ok      CMake-Quelle: das Bauskript liest 'main' aus dem Identitaetsmanifest
  ok      CMake-Quelle: PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle: PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle: COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle: der Viercode steht nicht literal im Zielblock  [Eqcp]
  ok      CMake-Quelle: der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      CMake-Quelle: der Produktname steht nicht literal im Zielblock
  ok      CMake-Quelle: JUCE_VST3_CAN_REPLACE_VST2=0 steht im Zielblock
  ok      CMake-Quelle: der VST2-Ersatzpfad ist nicht eingeschaltet
  ok      CMake-Quelle: das Define steht auch sonst nirgends auf 1
  ok      CMake: Zielblock juce_add_plugin(${ziel}) der Sondenfunktion gefunden
  ok      CMake: Defineblock der Sondenfunktion gefunden
  ok      CMake-Quelle: die Sondenfunktion liest ihre Identitaet aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PLUGIN_MANUFACTURER_CODE kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): PRODUCT_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): COMPANY_NAME kommt aus dem Manifest
  ok      CMake-Quelle (Sonde): der Herstellercode steht nicht literal im Zielblock  [Evna]
  ok      passive-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkPr]
  ok      passive-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Suna]
  ok      passive-probe: NakamaSuna entsteht ueber nakama_sonde_ziel()
  ok      passive-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaSuna    passive-probe NAKAMA_SONDE_PASSIV)]
  ok      active-probe: der Viercode steht nicht literal im Sonden-Zielblock  [NkAc]
  ok      active-probe: der Produktname steht nicht literal im Sonden-Zielblock  [Nakama Probeeq]
  ok      active-probe: NakamaProbeeq entsteht ueber nakama_sonde_ziel()
  ok      active-probe: der Aufruf uebergibt genau diese Manifest-ID  [nakama_sonde_ziel(NakamaProbeeq active-probe  NAKAMA_SONDE_AKTIV)]
  ok      CMake-Quelle (Sonde): JUCE_VST3_CAN_REPLACE_VST2=0 steht im Defineblock
  ok      main: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\Resources\moduleinfo.json]
  ok      main: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [23 Aug 2026 2:33:03pm vs 23 Aug 2026 1:59:09pm]
  ok      main: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      main: Produktname wie im Manifest  [EQ-Copilot]
  ok      main: Vendor wie im Manifest  [evenacadia]
  ok      main: genau zwei Klassen (Component + Controller)  [2]
  ok      main: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E6145716370]
  ok      main: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E6145716370]
  ok      main: Unterkategorien wie im Manifest
  ok      main: keine fremde Ziel-CID im Bundle
  ok      passive-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\Resources\moduleinfo.json]
  ok      passive-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [23 Aug 2026 2:33:05pm vs 23 Aug 2026 1:59:09pm]
  ok      passive-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      passive-probe: Produktname wie im Manifest  [Nakama Suna]
  ok      passive-probe: Vendor wie im Manifest  [evenacadia]
  ok      passive-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      passive-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B5072]
  ok      passive-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B5072]
  ok      passive-probe: Unterkategorien wie im Manifest
  ok      passive-probe: keine fremde Ziel-CID im Bundle
  ok      active-probe: moduleinfo.json des gebauten Bundles gefunden  [C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\Resources\moduleinfo.json]
  ok      active-probe: moduleinfo.json ist nicht aelter als plugin/CMakeLists.txt  [23 Aug 2026 2:33:05pm vs 23 Aug 2026 1:59:09pm]
  ok      active-probe: moduleinfo.json ist nach dem Kommaputz parsebar
  ok      active-probe: Produktname wie im Manifest  [Nakama Probeeq]
  ok      active-probe: Vendor wie im Manifest  [evenacadia]
  ok      active-probe: genau zwei Klassen (Component + Controller)  [2]
  ok      active-probe: die Audio-Module-Klasse traegt die Component-CID  [ABCDEF019182FAEB45766E614E6B4163]
  ok      active-probe: die Controller-Klasse traegt die Controller-CID  [ABCDEF011234ABCD45766E614E6B4163]
  ok      active-probe: Unterkategorien wie im Manifest
  ok      active-probe: keine fremde Ziel-CID im Bundle
  ok      Viercode ist vier Zeichen lang: main.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: main Component (Eqcp)  [ABCDEF019182FAEB45766E6145716370]
  ok      CID nachgerechnet: main Controller (Eqcp)  [ABCDEF011234ABCD45766E6145716370]
  ok      Viercode ist vier Zeichen lang: passive-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: passive-probe Component (NkPr)  [ABCDEF019182FAEB45766E614E6B5072]
  ok      CID nachgerechnet: passive-probe Controller (NkPr)  [ABCDEF011234ABCD45766E614E6B5072]
  ok      Viercode ist vier Zeichen lang: active-probe.plugin_code  [4 Zeichen]
  ok      CID nachgerechnet: active-probe Component (NkAc)  [ABCDEF019182FAEB45766E614E6B4163]
  ok      CID nachgerechnet: active-probe Controller (NkAc)  [ABCDEF011234ABCD45766E614E6B4163]
  ok      VST2-Ersatzpfad wuerde eine ANDERE CID erzeugen - das Define ist aus  [5653544571637065712D636F70696C6F]
  ok      die echte CID stammt aus dem jucePluginId-Pfad
  ok      Golden-Ordner vorhanden  [C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\identity]
  ok      Rolle 'sensor' geladen  [sensor]
  ok      Rolle 'sensor': Sensor-ID unveraendert uebernommen
  ok      Rolle 'sensor': Label uebernommen  [Klavier A]
  ok      Rolle 'sensor': Paar-ID uebernommen
  ok      Golden 'sensor' gelesen  [state-schema1-sensor.bin]
  ok      Schema-1-Saat 'sensor' ist bytegleich zum eingefrorenen Golden  [128 Bytes]
  ok      Rolle 'sensor' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'sensor' laedt in eine frische Instanz zurueck
  ok      Rolle 'hub' geladen  [hub]
  ok      Rolle 'hub': Sensor-ID unveraendert uebernommen
  ok      Rolle 'hub': Label uebernommen  [Leitstand]
  ok      Rolle 'hub': Paar-ID uebernommen
  ok      Golden 'hub' gelesen  [state-schema1-hub.bin]
  ok      Schema-1-Saat 'hub' ist bytegleich zum eingefrorenen Golden  [125 Bytes]
  ok      Rolle 'hub' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'hub' laedt in eine frische Instanz zurueck
  ok      Rolle 'pre' geladen  [pre]
  ok      Rolle 'pre': Sensor-ID unveraendert uebernommen
  ok      Rolle 'pre': Label uebernommen  [Chor PRE]
  ok      Rolle 'pre': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'pre' gelesen  [state-schema1-pre.bin]
  ok      Schema-1-Saat 'pre' ist bytegleich zum eingefrorenen Golden  [133 Bytes]
  ok      Rolle 'pre' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'pre' laedt in eine frische Instanz zurueck
  ok      Rolle 'post' geladen  [post]
  ok      Rolle 'post': Sensor-ID unveraendert uebernommen
  ok      Rolle 'post': Label uebernommen  [Chor POST]
  ok      Rolle 'post': Paar-ID uebernommen  [paar-chor]
  ok      Golden 'post' gelesen  [state-schema1-post.bin]
  ok      Schema-1-Saat 'post' ist bytegleich zum eingefrorenen Golden  [135 Bytes]
  ok      Rolle 'post' speichert Schema 2 (Migration, SONDE-006)
  ok      Golden 'post' laedt in eine frische Instanz zurueck
  ok      fremder ValueTree-Typ wird abgelehnt, Zustand bleibt
  ok      Muellbytes aendern den Zustand nicht

IDENTITY-TEST OK - 114 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b2"></a>
#### B2 | EqCopStateMigrationTest

**Befehl:** `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | **Exitcode:** 0 | **Dauer:** 0,10 s

stdout:

```text
Vertrag:  C:\Users\phili\Projekte\Nakama\eq-copilot\schemas\state\nakama-parameter-v1.json
Fixtures: C:\Users\phili\Projekte\Nakama\eq-copilot\fixtures\state

  ok      nakama-parameter-v1.json ist gueltiges JSON
  ok      anzahl_parameter == 109  [109]
  ok      slot_anzahl == 8
  ok      dsp_schema_version == 1
  ok      ids-Liste hat 109 Eintraege
  ok      alle 109 IDs in Vertragsreihenfolge identisch  [109]
  ok      109 Beschreibungen (Typ, Grenzen, Default, Enumwoerter, Wechsel, topologisch) deckungsgleich  [109]
  ok      indexVonId: erste, letzte, nicht vorhandene ID
  ok      == Parametertabelle deckungsgleich mit nakama-parameter-v1.json
  ok      fixtures/state/MANIFEST.json gelesen
  ok      jcs/zahlen.json gelesen
  ok      Zahlenvektoren: 52 bytegleich, 2 abgelehnt (NaN/Inf)  [54 gesamt]
  ok      NaN, +Inf, -Inf werden abgelehnt
  ok      -0 wird zu 0
  ok      JCS-Dokumente bytegleich + SHA-256 gleich  [13/13]
  ok      doppelter Schluessel erkannt  [doppelter Schluessel: a]
  ok      doppelter Schluessel nach Escape-Aufloesung erkannt  [doppelter Schluessel: A]
  ok      gleicher Schluessel in verschiedenen Objekten ist erlaubt
  ok      leerer Schluessel ist erlaubt (RFC 8785)
  ok      Subnormale werden gelesen, nicht geflusht
  ok      Ueberlauf ist ein Fehler, kein inf  [Zahl nicht darstellbar: 1e999]
  ok      hohes Surrogat ohne Paar ist ein Fehler  [hohes Surrogat ohne Paar]
  ok      rohes Steuerzeichen in Zeichenkette ist ein Fehler  [rohes Steuerzeichen in Zeichenkette]
  ok      Grammatikfehler werden abgewiesen
  ok      Leser nimmt Whitespace, Vorzeichen, Exponenten, Escapes
  ok      == JCS: alle Zahlenvektoren und Dokumente bytegleich zum Manifest
  ok      gueltige DTOs: state_hash wie im Manifest  [6/6]
  ok      standardSatz() hasht wie dto/gueltig/default.json  [0701d0f24ee7e438464aa7d15604cfc5a034a85ad9a7fac9ca85c2caf9cbc1bb]
  ok      ungueltige DTOs mit dem erwarteten Grund abgelehnt  [15/15]
  ok      NaN im Satz faellt vor dem Hash  [nichtendlich]
  ok      Inf im Satz faellt vor dem Hash  [nichtendlich]
  ok      Bereich im Satz faellt vor dem Hash  [bereich]
  ok      Enumindex im Satz faellt vor dem Hash  [enum]
  ok      == DTO: 6 gueltige gehasht, 15 ungueltige vor dem Hash abgelehnt
  ok      Schema-1 'sensor' migriert
  ok      'sensor': Herkunft schema1Migriert
  ok      'sensor' -> legacy+insert  [legacy+insert]
  ok      'sensor': sensor_id bytegleich als instance_id
  ok      'sensor': label und pair_id bytegleich
  ok      'sensor': project_binding_id NICHT erfunden
  ok      'sensor': v2-Rolle rueckwaerts identisch  [sensor]
  ok      'sensor': MainProject genau fuer main
  ok      'sensor': pair_id-Eigenschaft nur wenn nicht leer
  ok      'sensor': Migration deterministisch
  ok      'sensor': Migration der Migration ist Identitaet
  ok      Golden 'sensor' gelesen  [aus-schema1-sensor.bin]
  ok      Schema-1 'hub' migriert
  ok      'hub': Herkunft schema1Migriert
  ok      'hub' -> main+insert  [main+insert]
  ok      'hub': sensor_id bytegleich als instance_id
  ok      'hub': label und pair_id bytegleich
  ok      'hub': project_binding_id NICHT erfunden
  ok      'hub': v2-Rolle rueckwaerts identisch  [hub]
  ok      'hub': MainProject genau fuer main
  ok      'hub': pair_id-Eigenschaft nur wenn nicht leer
  ok      'hub': Migration deterministisch
  ok      'hub': Migration der Migration ist Identitaet
  ok      Golden 'hub' gelesen  [aus-schema1-hub.bin]
  ok      Schema-1 'pre' migriert
  ok      'pre': Herkunft schema1Migriert
  ok      'pre' -> legacy+pre  [legacy+pre]
  ok      'pre': sensor_id bytegleich als instance_id
  ok      'pre': label und pair_id bytegleich
  ok      'pre': project_binding_id NICHT erfunden
  ok      'pre': v2-Rolle rueckwaerts identisch  [pre]
  ok      'pre': MainProject genau fuer main
  ok      'pre': pair_id-Eigenschaft nur wenn nicht leer
  ok      'pre': Migration deterministisch
  ok      'pre': Migration der Migration ist Identitaet
  ok      Golden 'pre' gelesen  [aus-schema1-pre.bin]
  ok      Schema-1 'post' migriert
  ok      'post': Herkunft schema1Migriert
  ok      'post' -> legacy+post  [legacy+post]
  ok      'post': sensor_id bytegleich als instance_id
  ok      'post': label und pair_id bytegleich
  ok      'post': project_binding_id NICHT erfunden
  ok      'post': v2-Rolle rueckwaerts identisch  [post]
  ok      'post': MainProject genau fuer main
  ok      'post': pair_id-Eigenschaft nur wenn nicht leer
  ok      'post': Migration deterministisch
  ok      'post': Migration der Migration ist Identitaet
  ok      Golden 'post' gelesen  [aus-schema1-post.bin]
  ok      4 Rollen bytegleich zum Schema-2-Golden  [4]
  ok      read-only-Fixture fremdes-major-3.bin bytegleich
  ok      unbekannte Schema-1-Rolle ist nicht migrierbar  [schema-1 role unknown: dirigent]
  ok      EqCopilotState schema 2 ist kein Schema 1  [EqCopilotState schema 2 is not schema 1]
  ok      == Migration: 4 Rollen bytegleich zum Golden, idempotent
  ok      Schema 2 laedt
  ok      speichern->laden->speichern bytegleich  [209 Bytes]
  ok      Common feldgleich nach Roundtrip (inkl. Nicht-ASCII-Label)
  ok      Stand mit unbekannter additiver Eigenschaft laedt
  ok      unbekannte Eigenschaften ueberleben den Roundtrip, die Aenderung kommt an
  ok      active_probe mit Parameters laedt (Bundle NkAc)
  ok      109 Parameterwerte bit-exakt durch den Roundtrip  [109]
  ok      Parameters-Roundtrip bytegleich
  ok      state_hash ueberlebt den Roundtrip  [eea230574ce175deb6def340f8a858d762d49b6c213935e5ffbc1c1f8a833452]
  ok      == Roundtrip: bytegleich, additive Eigenschaft erhalten, 109 Werte bit-exakt
  ok      17 Faelle read-only, Bytes unveraendert, Aenderungen verweigert  [17]
  ok      fremder Baumtyp wird ignoriert, Zustand bleibt
  ok      Muellbytes werden ignoriert
  ok      read-only gesetzt
  ok      gueltiger Stand hebt read-only wieder auf
  ok      == Unbekanntes Major: 17 Faelle read-only, Originalbytes bytegleich zurueck
  ok      Duplikat: gleiche instance_id (der State IST der Messpunkt)  [11111111-2222-3333-4444-555555555555]
  ok      Duplikat: verschiedene runtime_nonce
  ok      neueSensorId loest auf
  ok      neue instance_id: 32 Hex, verschieden von beiden  [101909b9f4d34970a01105f80892ad2c]
  ok      Label und Rolle bleiben bei der Aufloesung
  ok      Aufloesung meldet genau einmal Host-Dirty  [1]
  ok      neue instance_id wird gespeichert und geladen
  ok      == Duplicate: gleiche instance_id, verschiedene runtime_nonce, Aufloesung mit Host-Dirty
  ok      Laden + Migration melden NICHT dirty  [0]
  ok      Herkunft schema1Migriert
  ok      setzeBindung ohne Aenderung: keine Meldung
  ok      setzeBindung mit Aenderung: genau eine Meldung  [0]
  ok      Aenderung kam an
  ok      Rollenwechsel zu hub: zweite Meldung
  ok      hub speichert MainProject und kein pair_id
  ok      getStateInformation meldet nichts
  ok      unbekannte v2-Rolle wird verweigert, keine Meldung
  ok      read-only mit fremdem Major 9  [9]
  ok      read-only verweigert setzeBindung und neueSensorId ohne Meldung
  ok      == Host-Dirty: Aenderung meldet, Laden schweigt, read-only verweigert
  ok      nie restauriert: Herkunft frisch
  ok      frisch: legacy+insert = v2 'sensor', leeres Label
  ok      frisch: instance_id ist hex32  [fff204dd689f4f13b8ac52b13d0bc478]
  ok      frisch speichert NakamaState schema 2, legacy
  ok      Recall: Schema-2-Golden laedt feldgleich in eine frische Instanz
  ok      Recall: Save nach Recall ist bytegleich zum Golden
  ok      == Frisch/Recall: Herkunft frisch, Recall in frische Instanz feldgleich

STATE-MIGRATION-TEST OK - 130 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

<a id="b3"></a>
#### B3 | EqCopHostContextTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | **Exitcode:** 0 | **Dauer:** 0,05 s

stdout:

```text
== A · Quellhash-Gate (JUCE 8.0.9) ==
  ok      JUCE-Wrapper liegt am gepinnten Ort: C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\juce-src\modules\juce_audio_plugin_client\juce_audio_plugin_client_VST3.cpp
  ok      Patchdatei liegt im Repo: juce-8.0.9-nakama-vst3-bridge.patch
  ok      die beiden gepinnten Hashes sind verschieden und vollstaendig (der Patch ist nicht leer)
  ok      Wrapper liess sich lesen
  ok      Wrapper-Quellhash == gepinnter GEPATCHTER Stand (gemessen: 6e5d4660d960836a…)
  ok      Wrapper ist NICHT der unberuehrte Auslieferungsstand
  ok      Patch: Bruecken-Header eingebunden
  ok      Patch: IAudioPresentationLatency als Basisklasse
  ok      Patch: Interface auch im queryInterface
  ok      Patch: Buslatenz-Methode umgesetzt
  ok      Patch: Blockbeginn verdrahtet
  ok      Patch: Context-Anwesenheit verdrahtet
  ok      Patch: fehlender Context wird ausdruecklich gemeldet
  ok      Patch: alle Parameterpunkte werden beobachtet
  ok      Patch: Uebergabe an die Senke verdrahtet
  ok      Patch: Bruecke als Wrapper-Member
  ok      Patch: Senke wird EINMAL beim Erzeugen aufgeloest
  ok      Patch nimmt nichts weg: JUCEs eigener Parameterweg steht unveraendert
  ok      ohne NAKAMA_HOST_BRIDGE bleibt der Wrapper wortgleich JUCE (Vorbelegung 0)
  ok      Patchdatei ist ein echter Unified Diff
  ok      Patch zielt auf den VST3-Wrapper
== B · Context-Anwesenheit und Gueltigkeitsbits ==
  ok      Senke wird genau einmal je Block gerufen
  ok      fehlender Context: processContextPresent == false
  ok      fehlender Context: Projektzeit ist UNGUELTIG, nicht 0
  ok      fehlender Context: Tempo und PPQ ungueltig
  ok      genullter, aber VORHANDENER Context: processContextPresent == true (der Kernbeweis)
  ok      genullter Context: Projektzeit gueltig und 0 â eine Aussage, keine Vermutung
  ok      genullter Context: Samplerate 0 wird roh gefuehrt (KEINE Sekundenrechnung, sonst 0/0 = NaN)
  ok      genullter Context: Tempo bleibt ungueltig
  ok      genullter Context: playing ist gueltig false â nicht 'unbekannt'
  ok      voller Context: anwesend
  ok      voller Context: projectTimeSamples 123456
  ok      voller Context: continousTimeSamples 999888 (SDK-Schreibweise korrekt gelesen)
  ok      voller Context: playing true
  ok      voller Context: recording true
  ok      voller Context: Tempo 128
  ok      voller Context: PPQ 8.25
  ok      voller Context: Taktbeginn 8.0 PPQ
  ok      voller Context: Cycle 4.0â12.0 PPQ, aktiv (PPQ, nicht Samples)
  ok      Teil-Context: playing true
  ok      Teil-Context: Tempo/PPQ/Takt/ContTime/Cycle bleiben ungueltig (nichts erfunden)
  ok      Teil-Context: Cycle nicht aktiv
  ok      beginneBlock loescht den Vorblock-Context (kein Nachleuchten alter Werte)
== C · Parameterpunkte: Reihenfolge, Offsets, Ueberlauf ==
  ok      drei Punkte angekommen
  ok      Punkte sind nach Sample-Offset sortiert
  ok      sauberer Block: sampleAccurateAutomation == true
  ok      Blockgroesse wird mitgefuehrt
  ok      kleinerer Offset zuerst, auch wenn spaeter geliefert
  ok      gleicher Offset behaelt die Hostreihenfolge (stabil sortiert)
  ok      Punkte aus mehreren Queues sind global aufsteigend
  ok      Ueberlauf: es bleiben genau kMaxParameterEreignisse Punkte
  ok      Ueberlauf: alle 6 verworfenen Punkte sind gezaehlt
  ok      Ueberlauf: sampleAccurateAutomation faellt fuer den GANZEN Block
  ok      Ueberlauf: Rueckfallwert ist der LETZTE Hostpunkt 0.777, nicht 0.511 (gemessen: 0.7770)
  ok      Ueberlauf des Ereignisrings verwirft KEINEN Letztwert
  ok      negativer Offset: geklemmt auf 0, gezaehlt, Samplegenauigkeit faellt
  ok      Offset == Blockgroesse: geklemmt auf letztes Sample, gezaehlt, Samplegenauigkeit faellt
  ok      Flush-Block (Blockgroesse 0) mit Offset 0 ist plausibel
  ok      Flush-Block mit Offset != 0 wird als unplausibel gemeldet
  ok      NaN-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      NaN-Parameterwert bleibt NaN (verworfen wird die Zusicherung, nicht der Messwert)
  ok      Inf-Parameterwert: gezaehlt, Samplegenauigkeit faellt
  ok      Zaehler und Fallbackbit werden je Block zurueckgesetzt
  ok      Ring randvoll: der 513. Punkt laeuft ueber
  ok      NaN als Punkt 513 wird GEZAEHLT - der Zaehler beschreibt den Host, nicht die Ringgroesse
  ok      NaN nach Ueberlauf: Zusicherung faellt
  ok      NaN nach Ueberlauf ist Rueckfallwert - aber ueber den Zaehler als unplausibel erkennbar
  ok      unplausibler Offset als Punkt 513 wird ebenfalls gezaehlt
  ok      Inf als Punkt 513 wird ebenfalls gezaehlt
  ok      Letztwert-Tabelle: genau kMaxLetztwerte Parameter werden gefuehrt
  ok      Letztwert-Tabelle: die 3 ueberzaehligen Parameter sind gezaehlt
  ok      Letztwert-Ueberlauf laesst die Zusicherung ebenfalls fallen
  ok      Letztwert-Ueberlauf: der ueberzaehlige Parameter meldet ehrlich 'nichts'
  ok      ohne Ueberlauf: Rueckfallwert deckt sich mit dem letzten Ringeintrag (0.30)
  ok      ohne Ueberlauf: der Ring traegt denselben Wert am Ende
  ok      Block ohne Automation: 0 Punkte, Zusicherung bleibt gueltig
== D · Buslatenz: die zwei Bedeutungen der Null ==
  ok      ohne Hostmeldung: gemeldet == false (nicht '0 Samples Latenz')
  ok      Host meldet 0: gemeldet == true bei samples == 0 (die Trennung, um die es geht)
  ok      Ausgangsbus 1: 480 Samples
  ok      Ausgangsbus 0 bleibt ungemeldet (keine Streuung)
  ok      Eingangsbus 0 behaelt seine fruehere Meldung ueber Bloecke hinweg
  ok      Busindex ausserhalb [0, kMaxBusse): verworfen und gezaehlt, kein Speicherfehler
  ok      der Buszaehler steht im Blockbefund, ist also fuer die Senke sichtbar
  ok      Abfrage ausserhalb des Bereichs liefert einen leeren Eintrag
== E · Fallbackbit und Realtime-Zusicherungen ==
  ok      ohne Senke: Bruecke meldet sich als unverbunden
  ok      ohne Senke: beginneBlock/kontextAus/punkt/uebergib laufen folgenlos durch
  ok      Rueckfallweg: hatLetztenBlockwert liefert den LETZTEN Wert des Blocks (0.90)
  ok      Rueckfallweg: unbekannte Parameter-ID liefert ehrlich 'nichts', nicht 0
  ok      200 Bloecke Ã  64 Punkte: 0 Allokationen im Blockpfad (gezaehlt, nicht behauptet)
  ok      64-Bit-Atomics sind lock-frei â kein versteckter Mutex im Audiothread
  ok      Bruecke bleibt vorallokiert und klein (7880 Byte)

HOSTKONTEXT OK — 91/91 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3b"></a>
#### B3b | EqCopHostProbeTest

**Befehl:** `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | **Exitcode:** 0 | **Dauer:** 0,08 s

stdout:

```text
== A - Grundgesetz: das Geraet greift nicht ins Audio ein ==
  ok      meldet 0 Samples Latenz
  ok      meldet 0 s Tail
  ok      200 Bloecke float: Ausgang bitgleich zum Eingang
  ok      double-Block: Ausgang bitgleich zum Eingang
  ok      double-Praezision wird dem Host angeboten
  ok      5.1 wird klar abgelehnt, nicht still gemischt
== B - Die Bruecke findet die Senke (so wie der Wrapper sie sucht) ==
  ok      dynamic_cast<Senke*> auf den AudioProcessor gelingt - genau dieser Cast steht im Patch
  ok      vor dem ersten Block meldet das Geraet ehrlich: Bruecke liefert NICHT
  ok      nach dem ersten Bruecken-Block meldet es: Bruecke liefert
== C - Context-Anwesenheit und Gueltigkeiten ==
  ok      acht Bloecke mit Context gezaehlt
  ok      der eine Block OHNE Context ist getrennt gezaehlt
  ok      neun verarbeitete Bloecke
  ok      neun Senke-Aufrufe - keine Asymmetrie ohne Grund
  ok      der Kontextverlust ist als Ereignis vermerkt
  ok      Tempo war in jedem Context gueltig
  ok      continousTimeSamples war NIE gueltig - und wird nicht erfunden
== D - Zeitspruenge: Seek, Loop, Smart Disable - und Fehlalarm-Riegel ==
  ok      fortlaufendes Spiel meldet KEINEN Sprung (Fehlalarm-Riegel)
  ok      Seek vorwaerts wird als Sprung erkannt
  ok      die Sprungweite wird mitgemessen
  ok      Seek rueckwaerts wird als Sprung erkannt
  ok      nach Stop/Play wird der Positionswechsel NICHT als laufender Sprung gemeldet
  ok      er verschwindet aber auch nicht: eigener Zaehler 'ueber Stop/Play' (T2-Befund 21.08.)
  ok      und ein eigenes Ereignis, damit eine 0 bei den Spruengen nicht mehrdeutig ist
  ok      Stop und Play sind als Transportereignisse vermerkt
  ok      Loop-Wechsel erscheint als Rueckwaertssprung
  ok      Schleife-an ist genau einmal vermerkt
  ok      der Schleifenzustand wird gefuehrt
  ok      Smart-Disable-Luecke erscheint als Vorwaertssprung mit exakter Weite (48000)
== E - Samplegenaue Automation: der Kernbeweis von Termin B ==
  ok      ohne Automation wird nichts behauptet
  ok      ein Punkt je Block: gezaehlt, aber NICHT als samplegenau ausgegeben
  ok      alle Punkte werden aufsummiert
  ok      der dichteste Block wird gemerkt
  ok      Mehrpunkt-Block gezaehlt - DAS ist der Beweis fuer Samplegenauigkeit
  ok      kleinster und groesster Offset werden gefuehrt
  ok      der erste Mehrpunkt-Block ist als Ereignis vermerkt
  ok      Mehrpunkt-Block OHNE Zusicherung zaehlt NICHT als samplegenau
  ok      er wird stattdessen getrennt gezaehlt - kein Befund verschwindet
  ok      Unplausibilitaeten und Ueberlaeufe der Bruecke werden AUFSUMMIERT uebernommen, nicht geglaettet
  ok      beide Bloecke ohne Zusicherung sind gezaehlt (der mit Offset- und der mit Wertfehler)
== F - Presentation-Latency: nie gemeldet ist etwas anderes als 0 ==
  ok      ohne Hostmeldung: 'nie gemeldet' - und KEIN Eintrag wird erfunden
  ok      Host meldet 0: als GEMELDET gefuehrt, Wert 0 - die Trennung bleibt erhalten
  ok      die Gegenrichtung bleibt ungemeldet - es wird kein Wert erfunden
  ok      Ausgang Bus 0 = 1024 wird NEBEN dem Eingang gefuehrt, nicht verschluckt
  ok      auch ein dritter Bus kommt an
  ok      ein nicht gemeldeter Bus dazwischen bleibt ungemeldet
  ok      der zuerst gemeldete Latenzwert rastet ein
  ok      die spaetere Aenderung wird gezaehlt statt still verworfen
  ok      50 weitere Bloecke mit DEMSELBEN geaenderten Wert zaehlen NICHT weiter (Uebergaenge, nicht Bloecke)
  ok      ein dritter, wieder anderer Wert zaehlt als zweiter Uebergang
  ok      der Bericht listet genau die drei gemeldeten Eintraege - keinen erfundenen
  ok      und der Wertwechsel-Zaehler steht im Bericht - mit der Einheit im Schluesselnamen
== G - Senke ohne processBlock (Flush/Hostriegel) ==
  ok      Senke-Aufrufe und verarbeitete Bloecke werden GETRENNT gezaehlt
  ok      der Block ohne Verarbeitung ist ausdruecklich vermerkt
== H - Gegenpfad: Bericht schreiben und wieder einlesen ==
  ok      der Bericht ist nicht leer
  ok      der Bericht ist gueltiges JSON (Rueckweg geprueft, nicht angenommen)
  ok      Schemaname steht im Bericht
  ok      der Bericht traegt die Kernaussage 'samplegenau belegt'
  ok      die Blockzahl im Bericht deckt sich mit der Messung
  ok      das Ereignisprotokoll liegt im Bericht
  ok      die Gueltigkeitsmaske steht im Bericht
  ok      Bericht wurde als Datei geschrieben: C:\Users\phili\AppData\Roaming\evenacadia\nakama\spike\host-probe-20260823-143329.json
  ok      die geschriebene Datei laesst sich wieder einlesen und traegt dieselbe Messung
== I - Zuruecksetzen und Ringueberlauf ==
  ok      Zuruecksetzen leert die Messung - und erzeugt dabei keinen Scheinsprung
  ok      nach dem Zuruecksetzen wird die laufende Aufnahme wieder als aufnahme_an gemeldet
  ok      mehr Ereignisse als Ringplaetze wurden erzeugt
  ok      die Zaehler je Art ueberleben den Ringueberlauf - kein Befund verschwindet still
  ok      der Ring gibt genau seine Kapazitaet heraus
  ok      die herausgegebenen Ereignisse stehen in Blockreihenfolge (juengste zuletzt)
== I2 - Offline-Uebergang und negative Projektzeit ==
  ok      Offline- und Echtzeitbloecke getrennt gezaehlt
  ok      der Wechsel IN den Render ist vermerkt
  ok      der Wechsel ZURUECK in Echtzeit ist ebenfalls vermerkt - nicht nur der erste Block ueberhaupt
  ok      negative Projektzeit wird GEZAEHLT, nicht still uebersprungen
  ok      der erste negative Wert ist als Ereignis vermerkt
  ok      im negativen Bereich wird kein Scheinsprung erfunden
== J - Audiothread: keine Allokation ==
  ok      500 Bloecke mit Kontext, Transportwechseln und je 8 Automationspunkten: 0 Allokationen
== J2 - Nebenlaeufig lesen, waehrend der Audiothread schreibt ==
  ok      der zweite Thread hat waehrenddessen wirklich geschrieben (32303 Bloecke)
  ok      200 nebenlaeufige Lesevorgaenge liefern durchweg plausible Eintraege und Zaehler
  ok      der letzte Lesevorgang hat Eintraege geliefert
  hinweis   Grenze: Rauchtest, kein Beweis der Tearing-Freiheit (der Wiederholpfad wird selten bis nie betreten)
== K - Anzeige: passt der Inhalt ueberhaupt ins Fenster? ==
  ok      der Editor laesst sich erzeugen
  ok      Fensterhoehe 850 deckt den Leerzustand (850 noetig)
  ok      Fensterhoehe deckt auch den Messzustand (850 noetig)
  ok      keine Textzeile liegt unter den Knoepfen (Text bis 760, Knopfstreifen ab 794)
  ok      keine Wertzeile ohne Beschriftung - und 27 Zeilen wurden dabei wirklich geprueft
  ok      hoechstens EINE unbeschriftete Hinweiszeile (gefunden: 1)
  ok      und der Riegel faellt: eine untergeschobene Wertzeile ohne Beschriftung wird erkannt

HOSTPROBE OK - 85/85 Pruefungen bestanden.
```

stderr:

_(leer)_

<a id="b3c"></a>
#### B3c | EqCopSchemaTest

**Befehl:** `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | **Exitcode:** 0 | **Dauer:** 0,15 s

stdout:

```text
EqCopSchemaTest - v3-Vertrag (SONDE-005a)
-----------------------------------------
[ok]   Textriegel deckt die gemeinsame Falltabelle  [59 Faelle]
[ok]   Falltabelle hat Substanz  [59 Faelle]
[ok]   unbekanntes Schluesselwort bricht den Ladevorgang  [unbekanntes Schluesselwort #/multipleOf]
[ok]   oneOf ohne Discriminator bricht den Ladevorgang  [falscher Werttyp fuer #/oneOf; oneOf ohne x-nakama-discriminator bei #]
[ok]   additives Objekt ohne maxProperties bricht den Ladevorgang  [additives Objekt # ohne maxProperties]
[ok]   unbekanntes Muster bricht den Ladevorgang  [unbekanntes Muster #: ^S-1-.*$]
[ok]   nicht-lokale Referenz bricht den Ladevorgang  [nicht-lokale Referenz bei #: https://fremd/schema.json]
[ok]   haengende Referenz bricht den Ladevorgang  [haengende Referenz bei #: #/$defs/gibtsnicht hat kein Ziel]
[ok]   maxLength als Gleitkommazahl bricht den Ladevorgang  [falscher Werttyp fuer #/maxLength]
[ok]   type als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/type]
[ok]   required mit Nicht-String bricht den Ladevorgang  [falscher Werttyp fuer #/required]
[ok]   Discriminator als Zahl bricht den Ladevorgang  [falscher Werttyp fuer #/x-nakama-discriminator]
[ok]   integer akzeptiert 1.0 (draft 2020-12)
[ok]   integer lehnt 1.5 ab
[ok]   Typfehler erzeugt keine Lawine  [1 Verletzung(en)]
[ok]   unbekannter Discriminator wird abgelehnt
[ok]   Nicht-Objekt an der Wurzel meldet an der Instanz, nicht an /type
[ok]   fehlendes Pflichtfeld zeigt auf das Elternobjekt
[ok]   Laenge zaehlt Codepunkte, nicht Bytes
[ok]   vier Codepunkte fallen bei maxLength 3
[ok]   Verletzungen sind kanonisch sortiert und doppelfrei  [3]
[ok]   objektwertiges const vergleicht reihenfolgeunabhaengig
[ok]   objektwertiges const sieht einen Unterschied in der Tiefe
[ok]   objektwertiges const sieht eine fehlende Eigenschaft
[ok]   Binaerkorpus klassifiziert wie das Manifest (51 Fixtures)
[ok]   Binaerkorpus hat Substanz  [51 Fixtures]
[ok]   Binaer-Manifestzahlen passen zur Fixtureliste
[ok]   kaputtes UTF-8 in der SID: 200x dasselbe Urteil, kein Absturz  [BL-1 aus T2-Runde 3]
[ok]   Bandwertgrenzen des Lesers stimmen mit quantisierung-v1.json
[ok]   Traegergrenzen folgen aus den dB-Werten mal Skalierung
[ok]   v3-Schema haelt die Engine-Teilmenge ein
[ok]   Textriegel lehnt ab: ungueltig/einsames-surrogat-im-label.json  [hohes Surrogat ohne Paar an Position 746]
[ok]   Textriegel lehnt ab: ungueltig/leerer-objektschluessel.json  [leerer Objektschluessel an Position 835]
[ok]   Textriegel lehnt ab: ungueltig/nul-escape-im-label.json  [NUL-Escape in Zeichenkette an Position 741]
[ok]   Skalar-Wurzel wird schon im Parser abgelehnt: ungueltig/wurzel-ist-string.json
[ok]   Textriegel lehnt ab: ungueltig/zahl-fuehrende-null.json  [fuehrende Null in "091" an Position 356]
[ok]   Textriegel lehnt ab: ungueltig/zahl-jenseits-u64.json  [Ganzzahl ausserhalb 2^53-1: 18446744073709552016]
[ok]   Textriegel lehnt ab: ungueltig/zahl-nicht-endlich.json  [Zahl ausserhalb +/-1e308: 1e400]
[ok]   Textriegel lehnt ab: ungueltig/zahl-ueber-2hoch53.json  [Ganzzahl ausserhalb 2^53-1: 9007199254740992]
[ok]   Korpus klassifiziert wie das Manifest (153 Fixtures)
[ok]   Korpus hat Substanz  [153 Fixtures]
[ok]   Manifestzahlen passen zur Fixtureliste
[ok]   feines Gitter hat 221 Baender
[ok]   222 Kanten, 221 Mitten  [222/221]
[ok]   Kanten streng monoton
[ok]   jede Mitte liegt echt zwischen ihren Kanten
[ok]   keine Mitte ist NaN oder unendlich
[ok]   alle Mitten liegen in 30 Hz .. 18 kHz
[ok]   grobes Gitter hat 64 Gruppen
[ok]   65 grobe Kanten
[ok]   Gruppen sind lueckenlos und ueberlappungsfrei
[ok]   keine leere Gruppe
[ok]   Gruppen decken genau 221 feine Baender  [221]
[ok]   grobe Kanten sind BITGLEICHE Kopien feiner Kanten
[ok]   Quantisierung stimmt mit den Testvektoren (61)
[ok]   genug Vektoren  [61]
-----------------------------------------
56 bestanden, 0 gescheitert
```

stderr:

_(leer)_

<a id="b4"></a>
#### B4 | EqCopQueueStressTest

**Befehl:** `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | **Exitcode:** 0 | **Dauer:** 1,76 s

stdout:

```text
== Nakama SONDE-008 - StampedAudioQueue, Quarantaene, Blockgroessen ==
Produktionsstrom GenStrom: 1 Tap(s), 131072 Frames Ring, 2048 Deskriptoren, max 16384 Frames je Block.

== A - Ganzblockaufnahme: Inhalt, Stempel, Reihenfolge ==
  ok      drei Bloecke, drei Deskriptoren  [3]
  ok      jeder Frame kommt bitgleich und an derselben Stelle heraus
  ok      Stempel: lokaler Strom und Projektzeit stimmen je Block
  ok      nach dem Leeren ist die Queue leer
  ok      kein Verlust bei Normallast
== B - Wrap: der Ring laeuft mehrfach um ==
  ok      der Ring ist wirklich mehrfach umgelaufen  [11 Umlaeufe]
  ok      ueber jeden Wrap hinweg bitgleicher Inhalt
  ok      kein Frame verloren, keiner doppelt  [12000/12000]
== C - Ueberlauf des SAMPLE-Rings: ganz oder gar nicht ==
  ok      vier Bloecke passen, danach ist Schluss  [4]
  ok      beide abgewiesenen Bloecke sind gezaehlt  [2]
  ok      verlorene Frames = 2 x 256 (keine Teilmenge)  [512]
  ok      im Ring liegen NUR vollstaendige Bloecke  [4]
  ok      nach dem Leeren nimmt der Ring wieder an
  ok      der erste Block nach dem Verlust traegt kFlagLueckeDavor
  ok      und beginnt ein neues continuity_segment  [1]
== D - Ueberlauf des DESKRIPTOR-Rings (der zweite Ring zaehlt) ==
  ok      genau `deskriptorPlaetze` Bloecke passen  [8]
  ok      die vier ueberzaehligen sind gezaehlt  [4]
  ok      und genau so viele stehen zum Lesen bereit  [8]
== E - Oversize: ein Block ueber der Slotkapazitaet ==
  ok      ein Block ueber maxBlockFrames wird als GANZES abgewiesen
  ok      und als Oversize gezaehlt, nicht als Ueberlauf  [1/0]
  ok      der Ueberlaufzaehler bleibt sauber
  ok      nichts Halbes im Ring
  ok      die verlorene Zeit ist vollstaendig gemeldet  [257]
  ok      genau maxBlockFrames passt noch (Kante, nicht Ausschluss)
== F - Flush (numSamples == 0) ist kein Verlust ==
  ok      ein Flush-Block wird nicht eingestellt
  ok      und zaehlt NICHT als Verlust - er traegt keine Audiozeit
  ok      der Folgeblock geht normal
  ok      und traegt keine Luecke
== G - Mono und Mehrfach-Tap ==
  ok      Mono: R traegt das L-Duplikat (FIFO-Vertrag der Engine)
  ok      und der Deskriptor sagt kanaele == 1
  ok      Drei-Tap-Layout: Block angenommen
  ok      das Bitset meldet genau die belegten Taps  [3]
  ok      jeder Tap liegt getrennt und richtig; der inaktive liefert false
  ok      eine falsche Tapzahl wird abgewiesen statt halb kopiert
== H - Ein-Block-Quarantaene ==
  ok      5 Bloecke rein, 4 versiegelt - einer bleibt in Quarantaene  [4]
  ok      und die Quarantaene haelt ihn wirklich
  ok      kein Bruch bei lueckenlosem Strom
  ok      der Seek ist als Bruch erkannt  [1]
  ok      genau EIN Block faellt an der Grenze - der Strom laeuft weiter
  ok      der Block NACH der Grenze ist schon wieder in Quarantaene
  ok      Stopp verwirft den gehaltenen Block („bleiben sie unvollstaendig“)
  ok      stehende Projektzeit (Teilpuffer/Stop) ist kein Bruch  [0]
  ok      und alle bis auf den juengsten sind versiegelt  [5]
  ok      Vorlauf ohne Bruch
  ok      Transportkante play->stop ist ein Bruch  [1]
  ok      ohne Hostkontext traegt der lokale monotone Strom (§53.6)
  ok      und versiegelt normal  [3]
  ok      Projektzeit nahe INT64_MAX: Ueberlauf ist eine Grenze, keine Fortsetzung
  ok      negative Projektzeit ist eine gueltige, fortsetzbare Zeit (HostProbe sah sie)
== I - Blockgroessen-Fuzz auf der Queue (1 .. ueber die Kante) ==
  20 000 Bloecke, Groessen 1..288: angenommen 17809, Ueberlauf-Drops 0, Oversize-Drops 2191, groesster Block 256, versiegelt 15872, Brueche 1936
  ok      jeder gelesene Frame steht an der Stelle, die sein Stempel nennt
  ok      es gab nie einen Block mit sampleCount 0
  ok      die Oversize-Faelle sind aufgetreten UND vollstaendig gezaehlt  [2191]
  ok      kein angenommener Block war je groesser als die Slotkapazitaet  [256]
  ok      es wurde wirklich gelesen  [2288127 Frames]
== J - verdrahtet: Passthrough bitgleich ueber alle Blockgroessen ==
  ok      18 Blockgroessen von 1 bis 16 384: kein Sample veraendert
  ok      0 Samples Latenz  [0]
  ok      kein Tail
  angenommen 18, groesster Block 16384, Ueberlauf 0, Oversize 0
== K - verdrahtet: Oversize aendert das Audio nicht ==
  ok      ein Block ueber der Slotkapazitaet laeuft unveraendert durch
  ok      und ist als Oversize-Drop der ANALYSE gezaehlt  [1]
== L - verdrahtet: der Audiothread alloziert nicht ==
  ok      als Main klassifiziert (§53.5)
  ok      4 000 Bloecke wechselnder Groesse mit Transportkanten: 0 Allokationen  [0]
  dabei Ueberlauf-Drops 3830 (der Worker kommt bei 4 000 Bloecken ohne Pause nicht nach - genau dafuer ist der Ganzblockdrop da)
== M - verdrahtet: die Messung bekommt den Strom wirklich ==
  eingespeist 287744, analysiert 287232, Drops 0, versiegelt 561, Quarantaene-Verwuerfe 0, Brueche 0
  ok      bei Normallast kein einziger Drop
  ok      und kein Kontinuitaetsbruch (lueckenloser Transport)
  ok      die Engine sieht den ganzen Strom bis auf den Block in Quarantaene  [512 Frames offen]
  ok      Kurz-LUFS steht (der fixed-memory Akku wird wirklich gefuettert)  [-8.684240]
  ok      framesDropped im Stats-Snapshot kommt aus derselben Quelle
== N - Gegenpfad: prepareToPlay mitten im Betrieb ==
  ok      auch der erste Block nach prepareToPlay alloziert nicht  [0]
  veraltete Bloecke (alter Anlauf, nicht analysiert): 40
  ok      ein Neuanlauf ist kein Oversize

68 Pruefungen, 0 Fehler.
QUEUE-STRESSTEST OK
```

stderr:

_(leer)_

<a id="b9"></a>
#### B9 | EqCopLoudnessGoldenTest

**Befehl:** `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | **Exitcode:** 0 | **Dauer:** 0,11 s

stdout:

```text
== Nakama SONDE-008 - EBU-Golden der fixed-memory Loudness ==
Referenz: die ausgebaute Rechnung (unbegrenzter Vektor + Zweitdurchgang).
Toleranz: +-0.1 LU (Entwurf §49).
Histogramm: 10001 Bins a 0.01 LU ueber [-70.0, 30.0] LUFS.

== A - konstante Pegel (kein Block am Gate) ==
  ok      konstant z=1.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=1.00000: LUFS-I innerhalb ±0,1 LU  [akku=-0.691000 ref=-0.691000 d=0.000000000]
  ok      konstant z=1.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=1.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=1.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=1.00000: Kurz-LUFS BITGLEICH  [akku=-0.691000000000000 ref=-0.691000000000000]
  ok      konstant z=0.01000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.01000: LUFS-I innerhalb ±0,1 LU  [akku=-20.691000 ref=-20.691000 d=0.000000000]
  ok      konstant z=0.01000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.01000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.01000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.01000: Kurz-LUFS BITGLEICH  [akku=-20.690999999999999 ref=-20.690999999999999]
  ok      konstant z=0.00001: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=0.00001: LUFS-I innerhalb ±0,1 LU  [akku=-50.691000 ref=-50.691000 d=0.000000000]
  ok      konstant z=0.00001: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=0.00001: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=0.00001: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=0.00001: Kurz-LUFS BITGLEICH  [akku=-50.691000000000003 ref=-50.691000000000003]
  ok      konstant z=100.00000: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      konstant z=100.00000: LUFS-I innerhalb ±0,1 LU  [akku=19.309000 ref=19.309000 d=0.000000000]
  ok      konstant z=100.00000: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      konstant z=100.00000: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000000]
  ok      konstant z=100.00000: Kurz-LUFS Gueltigkeit gleich
  ok      konstant z=100.00000: Kurz-LUFS BITGLEICH  [akku=19.309000000000001 ref=19.309000000000001]
== B - Rampe, Rauschen, lange Laufzeit ==
  ok      Rampe 30 dB ueber 90 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Rampe 30 dB ueber 90 s: LUFS-I innerhalb ±0,1 LU  [akku=-7.083701 ref=-7.083701 d=0.000000000]
  ok      Rampe 30 dB ueber 90 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.007384000]
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS Gueltigkeit gleich
  ok      Rampe 30 dB ueber 90 s: Kurz-LUFS BITGLEICH  [akku=-1.198087457573391 ref=-1.198087457573391]
  ok      Pseudo-Zufall 300 s: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Pseudo-Zufall 300 s: LUFS-I innerhalb ±0,1 LU  [akku=-9.750816 ref=-9.750816 d=0.000000000]
  ok      Pseudo-Zufall 300 s: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.001599230]
  ok      Pseudo-Zufall 300 s: Kurz-LUFS Gueltigkeit gleich
  ok      Pseudo-Zufall 300 s: Kurz-LUFS BITGLEICH  [akku=-12.109256046331003 ref=-12.109256046331003]
  ok      eine Stunde (36 000 Zellen): Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      eine Stunde (36 000 Zellen): LUFS-I innerhalb ±0,1 LU  [akku=-15.230810 ref=-15.230810 d=0.000000000]
  ok      eine Stunde (36 000 Zellen): unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      eine Stunde (36 000 Zellen): ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000583]
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS Gueltigkeit gleich
  ok      eine Stunde (36 000 Zellen): Kurz-LUFS BITGLEICH  [akku=-14.735286948147577 ref=-14.735286948147577]
== C - Pausen unter dem absoluten Gate (-70 LUFS) ==
  ok      Musik + echte Stille: Gueltigkeit wie die Referenz  [akku=1 ref=1]
  ok      Musik + echte Stille: LUFS-I innerhalb ±0,1 LU  [akku=-8.281644 ref=-8.281644 d=0.000000000]
  ok      Musik + echte Stille: unsicherheitLu() deckt den Fehler  [d=0.000000000 schranke=0.000000000]
  ok      Musik + echte Stille: ohne Grenzbin nur Rundungsrest (<= 1e-9 LU)  [d=0.000000000000027]
  ok      Musik + echte Stille: Kurz-LUFS Gueltigkeit gleich
  ok      Musik + echte Stille: Kurz-LUFS BITGLEICH  [akku=-12.451912590513381 ref=-12.451912590513381]
== D - adversarial: 1000 Bloecke im Grenzbin des relativen Gates ==
  Sweep: 201 Laeufe, groesste Abweichung 2.781360 LU bei Faktor 1.000000, groesste gemeldete Schranke 2.781412 LU
  Groesster Sprung der REFERENZ zwischen zwei benachbarten Laeufen (Eingang aendert sich um 1e-5): 2.781403 LU
  ok      Sweep vollstaendig gefahren  [201/201]
  ok      Sweep hat den Grenzbin wirklich getroffen (Gegenprobe)  [sonst pruefte D nichts; schranke=2.781412]
  ok      die selbstgemeldete Schranke deckt JEDEN Lauf  [201/201]
  ok      wo die Schranke <= 0,1 LU meldet, liegt der Wert auch innerhalb 0,1 LU  [76/76]
  ok      die Abweichung bleibt unter dem eigenen Sprung der Referenz (Kante, kein Fehler)  [maxD=2.781360 refSprung=2.781403]
  (Der adversariale Fall geht NICHT in die Toleranzbilanz unten ein - er misst eine Unstetigkeit der Norm, keine Rechengenauigkeit.)
== E - fixed memory: eine Million Zellen ==
  Akku:     0 Allokationen, LUFS-I -8.386510, Schranke 0.000000000 LU
  Referenz: 36 Allokationen, LUFS-I -8.386510
  ok      1 000 000 Zellen: 0 Allokationen im Akku  [0]
  ok      Gegenprobe: die alte Rechnung alloziert sehr wohl  [36]
  ok      1 000 000 Zellen: LUFS-I innerhalb ±0,1 LU  [d=0.000000000]
  Fester Bedarf laut speicherBytes(): 120252 Byte - haengt an keiner Laufzeitgroesse.
  ok      speicherBytes() ist eine reine Compile-Time-Groesse
== F - numerische Raender ==
  ok      leer: kein LUFS-I (null, nie eine Zahl)
  ok      leer: kein Kurz-LUFS
  ok      leer: Schranke 0
  ok      alles unter -70 LUFS: beide ungueltig (Referenz-JSON: null)
  ok      l_j == -70,0 exakt: Gueltigkeit wie die Referenz  [akku=0 ref=0]
  ok      NaN/Inf-Zellen sind gezaehlt  [6]
  ok      NaN/Inf vergiften das Gating nicht  [-7.680700]
  ok      ueber dem Gitter: eigener exakter Eimer, wertgleich  [akku=59.309000 ref=59.309000 ueberGitter=197]
  ok      500 Zellen ergeben 497 Bloecke (400 ms, Hop 100 ms)  [500/497]
  ok      zuruecksetzen(): 0 Allokationen  [0]
  ok      zuruecksetzen(): der Akku ist danach wirklich leer

Schlimmste gemessene Abweichung ueber alle Korpora: 0.000000000 LU (Toleranz 0.1).
66 Pruefungen, 0 Fehler.
LOUDNESS-GOLDEN OK
```

stderr:

_(leer)_

<a id="b8"></a>
#### B8 | EqCopLebenslaufTest

**Befehl:** `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | **Exitcode:** 0 | **Dauer:** 0,14 s

stdout:

```text
LEBENSLAUF-TEST — §53.5 Lifecycle-Klassifikation

[1] Der Automat (nakama::state::Lebenslauf)
  ok      frisch: unclassified  [unclassified]
  ok      frisch: neutral
  ok      frisch: keine Audio-Ausnahme (audio-neutral)
  ok      frisch: kein Brokerstart
  ok      Schema-1 'sensor' -> legacy  [legacy]
  ok      Schema-1 'sensor': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'pre' -> legacy  [legacy]
  ok      Schema-1 'pre': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'post' -> legacy  [legacy]
  ok      Schema-1 'post': bleibt passiv (keine Audio-Ausnahme)
  ok      Schema-1 'hub' -> main  [main]
  ok      Schema-1 'hub': Audio-Ausnahme moeglich
  ok      Schema-2 main (Kind-Matrix bestaetigt) -> main
  ok      Schema-2 legacy -> legacy
  ok      Vorbereitung: erst main
  ok      unbekanntes Major danach -> zurueck auf unclassified
  ok      Muell (ignoriert) aendert nichts - main bleibt main
  ok      Muell auf einen frischen Automaten klassifiziert nicht
  ok      explizite Initialisierung OHNE Editor: abgelehnt
  ok      ... und der Automat bleibt neutral
  ok      der geoeffnete Editor allein klassifiziert NICHT
  ok      Editor offen + explizite Initialisierung: angenommen
  ok      ... -> main
  ok      main + offener Editor darf den Broker starten
  ok      Editor zu: die Klassifikation bleibt (der Stand hat sich nicht geaendert)
  ok      main OHNE offenen Editor darf NICHT starten
  ok      Rueckweg: derselbe Akt fuehrt von main zurueck
  ok      ... -> legacy
  ok      legacy darf nie starten - auch mit Editor nicht
  ok      legacy ist wieder audio-neutral
  ok      Sonde (NkPr) frisch: neutral trotz fester Produktklasse
  ok      Sonde (NkPr): gueltiger eigener Stand -> passive_probe
  ok      Sonde (NkPr): kein Brokerstart
  ok      Sonde (NkPr): auch mit offenem Editor kein Brokerstart
  ok      Sonde (NkPr): kein Akt macht sie zum Main
  ok      Sonde (NkAc): gueltiger eigener Stand -> active_probe
  ok      Sonde (NkAc): ein fremder Main-Stand faellt auf neutral, nie auf main

[2] Verdrahtet: EqCopilotProcessor (Gen)
  ok      Scannerlauf: klassifiziert NICHT (53.5)  [unclassified]
  ok      Scannerlauf: kein Brokerstart
  ok      Scannerlauf: unclassified bleibt audio-neutral (kein Sample gefaerbt)
  ok      Scannerlauf: Markierung meldet sich nicht hoerbar
  ok      Altprojekt 'sensor' -> legacy  [legacy]
  ok      legacy mit offenem Editor: kein Brokerstart
  ok      legacy ist 'immer passiv' - kein Sample gefaerbt
  ok      Altprojekt 'hub' -> main  [main]
  ok      main, aber Editor zu: noch kein Brokerstart
  ok      main + offener Editor: Brokerstart erlaubt
  ok      main: DIESELBE Markierung faerbt jetzt (der Riegel kann fallen)
  ok      Vorbereitung: Rollenwahl aendert den Stand
  ok      Rollenwahl OHNE offenen Editor klassifiziert nicht (53.5)  [unclassified]
  ok      der geoeffnete Editor allein holt es auch nicht nach
  ok      Vorbereitung: der User waehlt die Rolle im offenen Editor
  ok      frische Instanz: Editor offen + Rollenwahl 'hub' -> main
  ok      frische Instanz als main: faerbt
  ok      Rueckweg: 'sensor' klassifiziert zurueck auf legacy
  ok      Rueckweg: der Schnitt auf neutral greift sofort
  ok      Vorbereitung: main
  ok      der fremde Stand kommt read-only zurueck
  ok      read-only -> zurueck auf unclassified
  ok      read-only: kein Brokerstart
  ok      read-only: wieder audio-neutral

LEBENSLAUF-TEST OK - 61 Pruefungen ok, 0 Fehler
```

stderr:

_(leer)_

### Bau vor dem Lauf (`-Bauen`)

**build** | Exit 0 | 11,90 s

<details><summary>Rohe Ausgabe</summary>

```text
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_rc_lib.dir\Release\EqCopNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_rc_lib.dir\Release\EqCopGoldenTest_rc_lib.lib
  EqCopGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopMarkierungTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_rc_lib.dir\Release\EqCopMarkierungTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopMarkierungTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSunaNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_rc_lib.dir\Release\EqCopSunaNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopSunaNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSunaNullTest_artefacts\Release\EqCopSunaNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopProbeeqNullTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_rc_lib.dir\Release\EqCopProbeeqNullTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopProbeeqNullTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopIdentityTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_rc_lib.dir\Release\EqCopIdentityTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopIdentityTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopStateMigrationTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_rc_lib.dir\Release\EqCopStateMigrationTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopStateMigrationTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostContextTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_rc_lib.dir\Release\EqCopHostContextTest_rc_lib.lib
  EqCopHostContextTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopHostProbeTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_rc_lib.dir\Release\EqCopHostProbeTest_rc_lib.lib
  EqCopHostProbeTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopSchemaTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_rc_lib.dir\Release\EqCopSchemaTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  flatbuffers.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\_deps\flatbuffers-build\Release\flatbuffers.lib
  EqCopSchemaTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopQueueStressTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_rc_lib.dir\Release\EqCopQueueStressTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopQueueStressTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLoudnessGoldenTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_rc_lib.dir\Release\EqCopLoudnessGoldenTest_rc_lib.lib
  EqCopLoudnessGoldenTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopLebenslaufTest_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_rc_lib.dir\Release\EqCopLebenslaufTest_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopLebenslaufTest.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  EqCopilot_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_rc_lib.dir\Release\EqCopilot_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  EqCopilot.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\EQ-Copilot_SharedCode.lib
  EqCopilot_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\EqCopilot_vst3_helper.exe
  EqCopilot_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\EqCopilot_artefacts\Release\VST3\EQ-Copilot.vst3\Contents\x86_64-win\EQ-Copilot.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/EqCopilot_artefacts/Release/VST3/EQ-Copilot.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaSuna_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_rc_lib.dir\Release\NakamaSuna_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaSuna.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\Nakama Suna_SharedCode.lib
  NakamaSuna_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaSuna_vst3_helper.exe
  NakamaSuna_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaSuna_artefacts\Release\VST3\Nakama Suna.vst3\Contents\x86_64-win\Nakama Suna.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaSuna_artefacts/Release/VST3/Nakama Suna.vst3
MSBuild-Version 17.14.40+3e7442088 für .NET Framework

  Checking File Globs
  NakamaProbeeq_rc_lib.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_rc_lib.dir\Release\NakamaProbeeq_rc_lib.lib
  NakamaKern.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaKern.lib
  NakamaProbeeq.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\Nakama Probeeq_SharedCode.lib
  NakamaProbeeq_vst3_helper.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\Release\NakamaProbeeq_vst3_helper.exe
  NakamaProbeeq_VST3.vcxproj -> C:\Users\phili\Projekte\Nakama\eq-copilot\build\plugin\NakamaProbeeq_artefacts\Release\VST3\Nakama Probeeq.vst3\Contents\x86_64-win\Nakama Probeeq.vst3
  removing moduleinfo.json
  creating C:/Users/phili/Projekte/Nakama/eq-copilot/build/plugin/NakamaProbeeq_artefacts/Release/VST3/Nakama Probeeq.vst3
```

</details>

