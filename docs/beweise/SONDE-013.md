# SONDE-013 — S20–22: Dynamik, Stereo, PRE/POST, Passage und der manuelle Experimentkern

| Merkmal | Wert |
|---|---|
| Ticket | S20–22, `SONDE-013` (Phase P4–P5) |
| Phase | **Etappe 1 — Verhaltensmatrix, Nacharbeit 1 abgeschlossen, Wiederprüfung 1 offen** (Spezifikation vor Code) |
| Urteil | **offen** — dieses Manifest ist weder Bau- noch Prüfurteil |
| Prüfstufe | T1+T2 gefordert (`docs/bauaufteilung-sonden.md`:392); in dieser Etappe wird keine Stufe gefahren |
| Basis-SHA | `ed9bbf7fec951a061749abf143cb2158c1c4ee52` (gemessen mit `git rev-parse HEAD`) |
| Kanon vorher | GRUEN 40/40 auf `a627ebb`, Rohausgabe `docs/beweise/roh/NAK-121-a627ebb.md`. `git diff --stat a627ebb..ed9bbf7 -- broker eq-copilot tools` ist **leer**: der Codestand von `ed9bbf7` ist mit dem beglaubigten Kanonstand identisch, die acht Commits dazwischen ändern ausschließlich Dateien unter `docs/` (`git diff --name-only a627ebb..ed9bbf7` liefert 8 Pfade, alle mit diesem Präfix). |
| Matrixprüfung 1 | Codex `gpt-5.6-sol`, Effort **high**, lesend, Thread `01a06922-fd1d-7222-b180-fcddb1fdfc2d`; `HEAD` vor und nach der Prüfung `367a0ea732c9e883df12f2bfc5abb2f67df6da5d`. Urteil **NEEDS_WORK**, sieben Defekte. Rohausgabe `docs/beweise/roh/SONDE-013-matrixpruefung-1-367a0ea.txt`, Auftrag `docs/beweise/roh/SONDE-013-matrixpruefung-1-auftrag.txt`. Alle sieben Befunde sind an der Quelle als Defekt bestätigt; Wortlaut, Regel und geänderte Zeilen stehen in §8. |
| Änderungssatz dieser Etappe | Erster Commit `4a2f50a` und Entscheidcommit `367a0ea`: dieses Manifest und `docs/beweise/roh/SONDE-013-etappe-1-auftrag.txt`. Nacharbeit 1: dieses Manifest sowie die drei Rohdateien `docs/beweise/roh/SONDE-013-matrixpruefung-1-auftrag.txt`, `docs/beweise/roh/SONDE-013-matrixpruefung-1-367a0ea.txt` und `docs/beweise/roh/SONDE-013-nacharbeit-1-auftrag.txt` unverändert. Kein Produkt-, Test-, Schema-, Fixture- oder Werkzeugcode. |
| Grenze | Kein Produkt-, Test-, Schema-, Fixture- oder Werkzeugcode. Keine Änderung an `docs/plan/`, `docs/offene-punkte.md`, `docs/PLAN-STAND.md` oder anderen Manifesten. |

> **Keine Urteilsmarke, kein Belegfeld.** Dieses Manifest trägt bewusst weder
> eine `NAKAMA-URTEIL`-Marke noch einen Eintrag im `beleg`-Feld von
> `docs/plan/plan.json`. Beides setzt erst Etappe 2 beziehungsweise der
> Abschluss — sonst zeigte `docs/PLAN-STAND.md` ein Spezifikationsmanifest als
> „gebaut".

**Warum Etappe 1 überhaupt:** Das Ticket berührt drei Klassen, die im
Nachhinein teuer sind. **Verträge** — die v3-Schemas sind nach dem ersten
Cross-Language-Fixture nicht mehr frei änderbar, und `evidence_snapshot` ist
`additionalProperties: false`. **Lebenszyklen** — ein `experiment_begin` ohne
definiertes Terminalereignis ist ein unbegrenzt wachsender SQLite-Bestand.
**Nebenläufigkeit** — die Metriken entstehen im Audio-Thread, werden vom Worker
publiziert und vom Broker quarantänisiert. Die Matrix in §3 friert deshalb die
aus den verbindlichen Quellen belegbare Verhaltensgrenze ein und benennt die
Stellen, an denen der spätere Bau ohne weiteren Vertragsentscheid nicht ehrlich
fortfahren kann.

---

## 1. Gate-Text, wörtlich

### 1.1 Entwurf §58 „P4 — Vergleichsevidenz und manueller Versuch"

`docs/FL-Nakama-Sonden-Design-Entwurf.md`:4272-4290, vollständig:

> ## 58. P4 — Vergleichsevidenz und manueller Versuch
>
> **Zweck:** Die Landkarte wird von Pegelanzeigen zu belastbarer, passagengebundener Evidenz.
>
> **Lieferumfang:**
>
> - Dynamik-, Headroom-, Stereo-, Korrelations- und Ereignismetriken mit Qualitätsklasse;
> - PRE/POST-Paare, Restlag/Alignment, Coverage und ehrliche Herabstufung bei Modulation oder
>   unbekannter Zeit;
> - manuell markierte Passage, Fingerprint und Comparability-Score;
> - lokaler Hörmarker mit fail-closed `playing=true`, `recording=false`, Realtime und Editor offen;
> - `Experiment` für immutable Baseline/Kandidat, zunächst `manual_external`, Lautheitsabgleich,
>   Blindurteil und Guardrail-Deltas;
> - Invalidierung bei Marker, Preview, Seek, Drop, geändertem Material oder Messpunkt.
>
> **Exit-Gate:** Referenzkorpus, Loop-/Seek-/PDC-Goldens und adversariale Vergleichsfixtures bestehen.
> Kein unbekannter Zeitpfad erzeugt eine starke Cross-Probe- oder PRE/POST-Aussage. Hörmarker- und
> Experimentende schließen alle Taintintervalle; der reguläre Pfad nullt danach wieder.

### 1.2 Tickettabelle des Entwurfs

`docs/FL-Nakama-Sonden-Design-Entwurf.md`:4474, wörtlich:

```
| `SONDE-013` | Dynamik/Stereo/PRE-POST/Passage und manueller Experimentkern | P4-Korpus und inverse Taintpfade grün |
```

### 1.3 Bauaufteilung

`docs/bauaufteilung-sonden.md`:392, wörtlich:

```
| S20–22 | `SONDE-013` | Dynamik/Stereo/PRE-POST/Passage + manueller Experimentkern | T1+T2 |
```

### 1.4 Planschritt

`docs/plan/plan.json`, Schritt `S20–22`, wörtlich:

```json
{
  "id": "S20–22",
  "leitungsname": "Vergleiche, Passagen und manuelle Hörversuche auswertbar machen",
  "ticket": "SONDE-013",
  "text": "Dynamik, Stereo, vor/nach der Kette, Passagen — und der manuelle Experimentkern.",
  "beleg": "",
  "stufe": "T2"
}
```

### 1.5 Registerzeilen dieses Tickets

Beide tragen in `docs/offene-punkte.md` die Klasse **[Planarbeit · S20–22]** und
gehören damit zum Lieferumfang. Wörtlich:

**NAK-47** (`docs/offene-punkte.md`:124):

> **[Planarbeit · S20–22]** **Die Hör-Markierung schneidet hart, statt auszublenden** (Fund aus G0, aus Codex' widerlegtem Gate-1-Pfad verifiziert geborgen; `docs/beweise/G0.md` §3.1). `hartAus()` (`eq-copilot/plugin/src/HoerMarkierung.h:419-424`) setzt `fade = 0.0` sofort. Fällt `erlaubt` weg, während die Markierung hörbar ist (Editor schließt, Transport stoppt, Echtzeitbeweis verfällt), springt das Signal im nächsten Block abrupt vom gefilterten auf den Originalpfad — hörbarer Klick. Kein Bruch von Gate 1 (das Audio wird ja gerade **nicht** mehr verändert), aber ein Bruch des Startbudgets §49.3 „A/B-Zustandswechsel: kein Klick; identischer Zustand nullt innerhalb numerischer Toleranz". Dieselbe harte Kante steht in `HoerMarkierung.h:281` (`n > wetKapazitaet` ⇒ `hartAus()`). Fix wäre ein kurzer erzwungener Ausfade statt des Sofortschnitts — verlangt aber Sorgfalt, weil der Neutralpfad danach **bit-transparent** bleiben muss (MarkierungTest). Eigentümer offen; spätestens wenn die Markierung aus dem Provisorium in die Figma-UI wandert.

**NAK-68** (`docs/offene-punkte.md`:109):

> **[Planarbeit · S20–22]** **Ein Frame sagt nicht, über wie viel Audio er integriert wurde.** Folge des Entwurfsentscheids zu T2-1 (`docs/beweise/SONDE-009.md` §10.1: „die Grenze schneidet den Inhalt, nicht die Uhr"): `liveSamples`/`evidenzSamples` überleben eine Epochengrenze, damit eine Folge dichter Grenzen die Telemetrie nicht verstummen lässt. Der Frame, der kurz nach einer Grenze fällig wird, trägt deshalb **weniger** integrierte Fenster als einer im Dauerbetrieb — an den Bändern ist das über die Bitmap ehrlich („kein Bit" statt „0 dB"), an den **Rahmenskalaren** (`peakDb`, `crestDb`, `breite`, `korrelation`) aber nicht: sie werden über einen kürzeren Rahmen gerechnet und sehen genauso aus wie sonst. Ein Empfänger kann heute „leise" nicht von „kurz gemessen" unterscheiden. ⚠️ Das ist **kein** neuer Fehler dieses Entscheids — die Integrationslänge schwankte schon vorher (der Rahmen ist 4800…4800+Blockgröße Samples lang, und nur AKTIVE Fenster gehen in die Bandakkus, also hängt sie ohnehin am Inhalt); der Entscheid vergrößert nur die Spanne. Wer es angeht: ein Feld `integration_samples` (oder `rahmen_samples`) im `FeatureFrame` **und** in `table Frame` von `nakama_telemetry_v1.fbs` — dann kann ein Empfänger einen dünnen Frame erkennen und selbst entscheiden, ob er ihn anzeigt. Gehört zu `SONDE-010` (dort entsteht der erste echte Leser) und trifft dieselbe `.fbs`-Fläche wie **NAK-59**, also am besten in einem Änderungssatz mit ihm. Kein Produktcode hängt heute daran.

> **Anmerkung zur NAK-68-Zuordnung.** Die Registerzeile nennt als Eigentümer
> `SONDE-010` und als bevorzugten Änderungssatz den von NAK-59. NAK-59 ist am
> 02.09.2026 geschlossen (`band_stereo`, Feld-ID 10), SONDE-010 ist
> abgeschlossen. Die Klassenmarke **[Planarbeit · S20–22]** und die
> Registerübersicht (`docs/offene-punkte.md`:73) weisen die Zeile diesem Ticket
> zu; der jüngere und konkretere Eintrag gewinnt. Der Widerspruch wird hier
> benannt und nicht kaschiert; die Registerdatei selbst rührt diese Etappe
> auftragsgemäß nicht an (§4, O-03).

### 1.6 Belegklassen dieser Matrix

Definition wie `docs/beweise/SONDE-012.md` §1.1:

- **BELEGT** — Die Zusage folgt aus einer verbindlichen Quelle. Dass der heutige
  Produktpfad sie noch nicht erfüllt, macht sie nicht offen.
- **BAULÜCKE** — Die Zusage ist belegt, aber mindestens eine heute nötige
  Implementierungs- oder Testkante fehlt.
- **ENTSCHEIDEN-VORSCHLAG** — Eine aus den Quellen nicht ableitbare Technikfrage.
  Die Zeile trägt Vorschlag, Begründung und Alternativen; entschieden wird sie
  vom Dirigenten in Etappe 2, nicht hier.
- **OFFEN** — Mindestens ein Teil der Zusage lässt sich aus keiner Quelle
  ableiten. Diese Zeile ist **kein** stiller Produktentscheid.

Jede Zeile nennt ein bestehendes Prüfbein aus `tools/beweise.ps1` oder einen
ausdrücklich mit **NEU** gekennzeichneten Testnamen. Ein neuer Testname ist
Teil der späteren Bauaufgabe, nicht die Behauptung, der Test existiere bereits.

---

## 2. Ist-Stand, an der Quelle gemessen

Alles in diesem Paragraphen ist an `ed9bbf7` gelesen, nicht aus Dokumentation
übernommen. Zeilennummern gelten für diesen Stand.

### 2.1 FeatureEngine v2 und die Rahmenskalare

`eq-copilot/plugin/core/analysis/FeatureEngine.h`:420 trägt die Klasse; das
Ergebnisobjekt ist `struct FeatureFrame` ab :345. Die vier vom Ticket genannten
Rahmenskalare stehen dort als Wert **plus Präsenzbit** (:368-372):

```
bool  peakGesetzt        { false };  float peakDb      { 0.0f };
bool  crestGesetzt       { false };  float crestDb     { 0.0f };
bool  psrGesetzt         { false };  float psrDb       { 0.0f };
bool  breiteGesetzt      { false };  float breite      { 0.0f };
bool  korrelationGesetzt { false };  float korrelation { 0.0f };
```

Gerechnet werden sie in einem Zug am Rahmenende (:1984-2023). Drei Kanten sind
für die Matrix wichtig und heute schon so gebaut:

- **Breite** wird nur gesetzt, wenn `rahmenMid2 + rahmenSide2 > 0` (:2008-2013).
  Bei Stille bleibt das Bit aus — es entsteht keine numerische Null.
- **Korrelation** wird nur gesetzt, wenn der Nenner
  `sqrt(rahmenL2) * sqrt(rahmenR2) > 0` **und** das Ergebnis endlich ist
  (:2014-2022). Bei echtem Mono (L bitgleich R, dupliziert) ist der Nenner
  positiv und die Korrelation exakt 1; bei einem stillen Kanal fällt das Bit weg.
- **Nyquist-Kappe:** `18 kHz ODER 0,95 · Nyquist, was kleiner ist`
  (:433-437, angewandt :944 und :975).

Der `DynamicsEvent`-Strom aus Entwurf §39.1 **existiert lokal nur zur
Hälfte**; diese Stelle ist in Nacharbeit 1 korrigiert (§8, D6 — vorher stand
hier „existiert bereits lokal"). `struct Ereignis` (:290) trägt Samplezeit,
Epoche/Segment (:297-301), `staerke` in MAD über der adaptiven Schwelle,
`bandZentrumHz` und **zwei** Qualitätsbits, `qualitaetFluss` und
`qualitaetPeak` (:305-306). Erzeugt wird ein Ereignis aber **ausschließlich**
bei Flussüberschreitung: die Bedingung lautet `fluss > schwelle && mad > 0.0`
(:1607), `qualitaetFluss` wird darin konstant auf `true` gesetzt (:1619), und
`qualitaetPeak` trägt lediglich das Crest-Zusatzbit `peakDb - rmsDb > 12 dB`
(:1605, :1620). Eine **Peaksteigung** — der Anstieg des Rahmenpeaks gegenüber
dem Vorrahmen — wird nirgends gerechnet. Entwurf §39.1:2592-2599 verlangt den
Detektor aber aus spektralem Fluss, Peaksteigung **und** Crest; der
eigenständige Gegenbeleg für sehr kurze Impulse fehlt heute also ganz. Der
Ring ist fest gedeckelt auf `kEreignisPlaetze = 64` (:448) mit
`kFlussHistorie = 16` für Median/MAD (:450); Überlauf verdrängt den ältesten
Eintrag (:1646-1650). **Er hat heute keinen Weg auf die Leitung** — siehe 2.4.
Der Transport dieses Stroms ist Matrixzeile M-05, der fehlende Peakpfad die
neue Zeile M-86.

True Peak liegt bisher nur im älteren Gen-Pfad
(`eq-copilot/plugin/src/AnalyseEngine.h`:102 `truePeakDb`, :351
`truePeakLinear`, ausdrücklich „dBTP über die ganze Passage (ungegated)"), nicht
im `FeatureFrame` und nicht auf dem v3-Draht.

Gemessen wird der Frame heute vom Bein **B5** `EqCopAnalysisGoldenTest`
(`tools/beweise.ps1`:623).

### 2.2 Loudness-Akkumulator und `unsicherheitLu()`

`eq-copilot/plugin/core/analysis/LoudnessAccumulator.h`:106 ist die
fixed-memory Umsetzung aus SONDE-008: Histogrammbins plus kompensierte Summen
(`struct KompSumme` :86-102), kein wachsender Vektor. Die öffentliche Fläche:

- `integriert(double& heraus)` (:283) — gegatete Integrierte, `false`, wenn noch
  keine belastbare Basis existiert;
- `unsicherheitLu()` (:321) — die **selbstgemeldete Schranke** der
  Quantisierung. Der Kopfkommentar (:33) sagt ausdrücklich, sie macht die
  Schranke „auslesbar, statt sie zu behaupten"; bei Blöcken ohne sicheren Bin
  wird sie unendlich statt still klein (:268);
- `kurz(double&)` (:354) — Kurz-LUFS;
- Diagnosezähler ab :372, darunter `bloeckeNichtEndlich()` (:383).

Der Weg zur Telemetrie: `eq-copilot/plugin/src/AnalyseEngine.cpp`:701-725
bildet das Paar, `eq-copilot/plugin/sonde/SondeProcessor.cpp`:362-375 setzt
entweder **beide** Felder oder **nur** den Status:

```
if (lautheit.paar) { frame.lufsIGesetzt = true; ... frame.lufsIUnsicherheitGesetzt = true; }
else               { frame.lufsIStatusGesetzt = true; frame.lufsIStatus = lautheit.status; }
```

Das ist der SONDE-012-Entscheid E-A02 (`docs/beweise/SONDE-012.md` §7): Paar
und Status schließen einander aus. Gemessen von **B9**
`EqCopLoudnessGoldenTest` und **B12** `EqCopSonde012LoudnessSourceTest`.

### 2.3 `band_stereo` im FlatBuffers-Frame und das fehlende Integrationsfenster

`eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs`:216-239 ist der
heutige `table Frame`. Er trägt vierzehn Felder, ID 0 bis 13; `band_stereo` hat
ID 10 (:230), belegt seit NAK-59, registriert in
`eq-copilot/schemas/v3/flatbuffers/FELD-IDS.json`:86. Der C++-Träger ist
`FeatureEngine.h`:353-362 (`liveBreite` plus Bitmap, ausdrücklich nur auf dem
64er-Livegitter, weil §33.2 Breite/Korrelation nicht für den Evidenzsnapshot
führt).

**Ein Feld `integration_samples` gibt es nicht** — weder im `.fbs` noch im
`FeatureFrame`. Das ist NAK-68 im Ist-Zustand: die Bandbitmap ist ehrlich, die
Rahmenskalare sind es nicht. Nächste freie Feld-ID im `table Frame` ist **14**.

### 2.4 Hörmarkierung

`eq-copilot/plugin/src/HoerMarkierung.h`:276 ist der Audiothread-Eintritt
`verarbeite(puffer, kanaele, erlaubt)`. Zwei Kanten sind bewiesen bit-transparent:

- `!erlaubt` (:306-313): `hartAus()`, Atomics auf false/0, **`return` vor jedem
  Schreibzugriff** — der Puffer bleibt unangetastet;
- `!zielAn && fade <= 0.0` (:314-320): ebenfalls `return`, „Ruhe = echter
  Originalpfad".

`hartAus()` (:419-424) setzt `fade = 0.0`, `pulsPos = 0` und leert die
Filterzustände — **sofort, ohne Ausblendung**. Das ist NAK-47. Dieselbe harte
Kante liegt auf dem Oversize-Pfad (:280-282: `n > wetKapazitaet` ⇒ `hartAus()`).

Der `erlaubt`-Term wird in `eq-copilot/plugin/src/PluginProcessor.cpp`:447-452
gebildet:

```
const bool erlaubt = istMainKlassifiziert
                  && (echtzeitOk || testEchtzeit)
                  && spielt
                  && ! isNonRealtime()
                  && (editorOffen || testEchtzeit);
```

Gemessen an Entwurf §58 („fail-closed `playing=true`, `recording=false`,
Realtime und Editor offen") **fehlt der Recording-Term**. Der Zustand ist im
Prozessor vorhanden — `aufnahmeGueltig` und `aufnahmeAktiv` werden aus der
Hostbrücke gesetzt (:335-336, mit ausdrücklichem Gültigkeitsbit) und reisen
bereits im `state_report` (:802-803) —, er steht nur nicht in diesem Term.
Bein **A3** `EqCopMarkierungTest` misst den heutigen Term samt U10-Gegenprobe.

Die Markierung sitzt bis heute im **Gen/Main**-Prozessor, nicht in
`eq-copilot/plugin/sonde/SondeProcessor.cpp`.

**Der Interventions-Producer fehlt.** `PluginProcessor.cpp`:967 setzt
`k.hoermarkierung = markierung.hoerbar()` — ein Bit im v2-Heartbeat. Genau das
verwirft Entwurf §34.2 wörtlich: „Ein 1-Hz-Heartbeat-Boolean reicht dafür nicht:
Ein kurzer Hörmarker kann vollständig zwischen zwei Heartbeats liegen." Einen
vorallokierten RT→Control-Ring, der `audible_intervention_begin/end` erzeugt,
gibt es im Plugin nicht.

### 2.5 `passage` und `experiment` im Broker-Store

`broker/src/store/migration.rs` legt die Tabellen bereits an — Migration 1 ist
gefahren und ihr SQL-Text ist über `migration_1_checksum()` (:150-155)
verriegelt:

- `passages` (:68-76) mit `passage_id`, Projektbindung, `session_epoch`,
  `last_event_ord`, `state_jcs` und Sessionindex;
- `experiments` (:113-121) in derselben Form;
- `user_verdicts` (:122-130);
- `experiment_events` (:131-135) als Verknüpfung `experiment_id × event_uuid`
  gegen das `event_log`.

`broker/src/store/writer.rs`:553 und :558 ordnen die Objektschlüssel `passage`
und `experiment` diesen Tabellen zu. Der Reset-Pfad (:374-376) löscht sie
mit. **Die Tabellen sind also da; es gibt keinen Schreiber und keinen Leser für
ihren Inhalt** — kein Rust-Modul außer Migration und Writer-Zuordnung nennt sie.

Auf dem Draht sind `experiment_begin`, `experiment_abort`, `experiment_result`
und `user_verdict` in `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`
**nicht definiert** — sie stehen im `reserviert`-Block (2.6). Ein v3-Parser
lehnt sie heute ab.

Ein **PRE/POST-Vergleich existiert bereits**, aber auf dem v2-Legacy-Pfad:
`broker/src/lib.rs`:656 `paare_auswerten(&[SensorEintrag])`. Er kennt die
Herabstufungslogik, die §58 verlangt, schon in Grundzügen — harte Ausschlüsse
nach Hörmarkierung (:713-726), Trennung/stale (:729-747), fehlende Projektzeit
(:775-792), Fensterüberlappung ≤ 0 (:796-806), und die drei Abstufungen auf
„wahrscheinlich" bei Loop-/Seek-Sprüngen, unter 80 % Überdeckung und
verschiedenen Host-PIDs (:809-826). Das ist der Formvorläufer, nicht der
v3-Vertrag: er arbeitet auf `SensorEintrag`/v2-Hello, nicht auf
`probe_descriptor` mit `measurement_position` und `pair_id`.

Die v3-Paarhälften sind dagegen vertraglich schon vorhanden:
`eq-ipc-v3.schema.json`:613 („PRE-Hälfte eines über `pair_id` verbundenen
PRE-/POST-Paares") und :642 (POST-Hälfte), diskriminiert über
`measurement_position` (:573-576). `pair_id` ist auch persistent
(`eq-copilot/plugin/state/NakamaState.cpp`:29, :448-449, :672-674).

### 2.6 Reservierte Namen

`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` führt heute 27 Familien
(18 definiert, 9 reserviert), geprüft von **A5**
`tools/eq-copilot/pruefe_v3_vertrag.py`. Für dieses Ticket sind vier Einträge
bindend:

- `experiment_begin` — Eigentümer **`SONDE-013` (P4)**, Grund: „Baseline/
  Kandidat/Vergleichbarkeit entstehen mit dem manuellen Experimentkern."
- `experiment_abort` — Eigentümer **`SONDE-013` (P4)**, Grund: „Gegenpfad zu
  experiment_begin."
- `experiment_result` — Eigentümer **`SONDE-017` (P7)**, Grund: „Deltas und
  Urteil setzen den Active-Compare-Pfad voraus."
- `user_verdict` — Eigentümer **`SONDE-014` (P5)**, Grund: „Userurteil bindet an
  Finding/Proposal-IDs, die es vor P5 nicht gibt."

Dazu ein reserviertes **Feld**: `evidence_snapshot.ereignisse`, Eigentümer
**`SONDE-013` (P4)** — „der DynamicsEvent-Strom entsteht erst in §39.1 mit dem
Dynamik- und Experimentkern."

Der Auftrag nennt für §33.3 alle vier Experimentfamilien nebeneinander. Das
Reservierungsregister ist jünger und feiner: **SONDE-013 besitzt nur
`experiment_begin` und `experiment_abort`.** Der Widerspruch ist in §4 als V-01
geführt und in der Matrix aufgelöst.

### 2.7 Taint-Logik

Die Vertragsseite ist vollständig, die Consumer-Seite ist gebaut, die
Producer-Seite fehlt.

**Entwurf.** §34.2 (:2183-2220) verlangt: Preview, Focus, Delta und Hörmarker
markieren nachgelagerte Messungen als beeinflusst; ein **angewendeter** DSP-
Zustand ist dagegen kein dauerhafter Taint, sondern segmentiert die Timeline
nach `state_revision` und erlaubt nach Warm-up eine neue `post_committed`-
Baseline (:2199-2204). Overflow, Control-Disconnect oder eine Sequenzlücke
setzen sticky `intervention_state_unknown`; starke Evidenz bleibt bis zu einem
expliziten Neutral-/Sequenz-Resync gesperrt (:2211-2215). §44.2 (:3000-3007)
ergänzt: Preview-/Focus-/Delta-/Marker-Matrix liegt **hinter** den Analysetaps,
und weil nachgelagerte Probes die Intervention dennoch hören, greift zusätzlich
die sitzungsweite Taint-Logik aus §34.

**Schema.** `eq-ipc-v3.schema.json`:1048-1066 definiert
`audible_intervention_begin` mit `intervention_id`, `adresse`,
`event_sequence` (monoton je Instanz, Lücke ⇒ sticky unknown), der geschlossenen
Menge `art: hoermarkierung | preview | focus_burst | experiment` und optionalem
`project_sample_start`. :1067-1084 definiert `audible_intervention_end` mit
`tail_samples` („der Bereich wird länger quarantänisiert als die Intervention
selbst dauerte"). :1085-1097 definiert `evidence_invalidate` mit den Gründen
`intervention`, `routing_unbekannt`, `sequenzluecke`, `epochwechsel`,
`state_revision` und dem über `art` diskriminierten `invalidate_umfang`
(IDs, Bereich, ganze Sitzung).

**Broker.** `broker/src/coordinator/intervention.rs` ist der fail-closed
Automat: `sequenz_annehmen` (:22-34) setzt das Sticky-Bit bei Lücke oder
unbekanntem Link; `intervention_begin` (:37-64) lehnt Duplikat-ID und Überlauf
über `MAX_AKTIVE_INTERVENTIONEN` ab; `intervention_end` (:66-92) behandelt ein
End ohne bekanntes Begin ausdrücklich **nicht** als sauberen Neutralzustand und
zieht `tail_samples_offen` auf das Maximum hoch; `neutral_resync` (:139-152) ist
der einzige Entsperrweg. `interventionssicht` (:154-168) und
`evidence_dispatch` (:171-182) leiten daraus `starke_evidenz_erlaubt` ab —
`evidence_dispatch` zählt das Urteil unter demselben Lock, damit ein Begin nicht
erst nach dem Dispatch wirkt. Der v2-Legacy-Marker teilt denselben Bestand
(:96-122); Trennung setzt Unknown.

**Was fehlt:** der Producer im Plugin (siehe 2.4) und jeder Erzeuger von
`evidence_invalidate` — die Familie ist definiert, aber im Broker sendet sie
niemand.

### 2.8 SourcesModel und der Gen-Empfänger

`eq-copilot/plugin/src/SourcesModel.h`:49 ist die Zeilenstruktur des
Main-Quellenmodells aus SONDE-012, :75-77 die absolute `Sicht` mit
`revision`. An Messwerten trägt es heute genau zwei Dinge:
`fensterDauerMs` (:69) und das atomare Lautheitspaar `lufsPaarVorhanden` /
`lufsI` / `lufsIUnsicherheitLu` (:70-72). Dynamik, Stereo, Korrelation,
Headroom und Ereignisse haben dort keinen Platz — sie kommen zwar im
`FeatureFrame` an, werden vom Modell aber nicht geführt.
`messStaleFristMs(fensterDauerMs)` (:130) ist die E-L16-Frist.

Der SONDE-012-Kopf (`docs/beweise/SONDE-012.md`) hält den Abschlussstand fest:
T2 PASS 2026-09-01, Basis `9abb66f`, End-SHA `c978148`, Kanon GRUEN 39/39. Die
für dieses Ticket bindenden Vertragsentscheide aus dessen §7:

- **E-A02** — integrierte Lautheit reist nur als atomares Paar mit endlicher
  Unsicherheit; Status und Paar schließen einander aus. Jede weitere Metrik
  dieses Tickets erbt diese Form.
- **E-L16** — die Messachse verlässt `fresh`, sobald das Wallclock-Alter des
  jüngsten gültigen Frames `max(2 × intrinsische Fensterdauer, 1.000 ms)`
  überschreitet. Die Frist lebt in Mains Quellenmodell, nicht im Broker.
- **E-C01** — Capabilitybits sind nicht additiv erweiterbar (Major-Regel aus
  `eq-copilot/schemas/v3/README.md`).

### 2.9 Vorstand aus den älteren Manifesten

- `docs/beweise/SONDE-008.md` — T2 PASS 24.08.2026. Bringt `StampedAudioQueue`,
  Ein-Block-Quarantäne und den fixed-memory `LoudnessAccumulator`. Die
  Quarantäneregel aus Entwurf §32.3 („Kein FFT-, Loudness-, Korrelations- oder
  Fingerprintfenster darf eine echte oder mögliche Epochengrenze überbrücken")
  gilt damit auch für jedes Fenster, das dieses Ticket ergänzt.
- `docs/beweise/SONDE-009.md` — T2 PASS 24.08.2026, Gate-Text „Drop/Seek/Loop
  trennt jedes offene Fenster". Der Entscheid in §10.1 lautet **„die Grenze
  schneidet den Inhalt, nicht die Uhr"** und ist die direkte Ursache von NAK-68.
- `docs/beweise/SONDE-010.md` — T2 PASS 30.08.2026. v3-Clients und
  Rust-Envelopeparser; der Draht, auf dem neue Felder reisen müssten.
- `docs/beweise/SONDE-011.md` — T2 PASS 01.09.2026. Coordinator, Eviction,
  SQLite-Migration 1 (die die Passage-/Experimenttabellen anlegt), Outbox.
- `docs/beweise/SONDE-012.md` — T2 PASS 01.09.2026, siehe 2.8.

### 2.10 Gelesene Fachquellen im Entwurf

`docs/FL-Nakama-Sonden-Design-Entwurf.md`, jeweils vollständig gelesen:
§3.4 Pre/Post-Paar (:699-711), §10 Kernfunktion 5 PRE/POST (:915-953),
§11 Kernfunktion 9 Dynamik/Headroom (:952-985), §12 Kernfunktion 10
Stereo/Phase (:985-1010), §15 Kernfunktion 14 Vorher/Nachher samt
Vergleichbarkeitsregeln (:1077-1120), §32.4 Passage und Content-Fingerprint
(:1967-1985), §33.2 Datenkadenzen (:2038-2050), §33.3 Nachrichtenfamilien
(:2051-2090), §33.5 Persistenz- und Zustandswahrheit (:2129-2165),
§34 Evidenz- und Datenmodell mit 34.1-34.3 (:2166-2237),
§38 Technikdesign PRE/POST (:2466-2566), §39 Technikdesign Dynamik
(:2568-2622), §40 Technikdesign Stereo (:2624-2671), §41.1 Hörmodi (:2673-2690),
§43 Experiment (:2824-2900, Startgates um :2864), §44.2 Analysetaps
(:3000-3007), §48.1 Echtzeit- und Ressourcenvertrag (:3397-3435),
§49.2 harte Systemgates (:3546-3567), §49.3 messbare Startbudgets (:3568-3590),
§49.4 Evidenzqualität (:3591-3600), §58 (:4272-4290).

---

## 3. Verhaltensmatrix

Spalten: **ID** · **Ausgangszustand × Ereignis** · **Zusage samt Reihenfolge und
Frist** · **Test, der genau diese Zeile misst** · **Quelle und Belegstatus**.
Ein Testname mit **NEU** existiert noch nicht.

### 3.1 Dynamik-, Headroom-, Stereo-, Korrelations- und Ereignismetriken

| ID | Ausgangszustand × Ereignis | Zusage samt Reihenfolge und Frist | Test | Quelle / Belegstatus |
|---|---|---|---|---|
| M-01 | Sonde misst; Rahmen wird fällig | Der Frame trägt Momentary (400 ms), Short-term (3 s) und Integrated nach BS.1770-5 mit EBU-R128-Gating. Reihenfolge unverändert: Ganzblock aus der `StampedAudioQueue` → Analyseworker → `merkmale.frame()` → Serialisierung. Kadenz Live 10 Hz (§33.2). Integrated reist weiter nur atomar mit `unsicherheitLu` (E-A02). | Bestehend **B9** `EqCopLoudnessGoldenTest` für den Akkumulator, **B5** `EqCopAnalysisGoldenTest` für LUFS-S; **NEU** `EqCopSonde013DynamicsTest`, Fall `momentary_short_integrated_are_separate_windows`. | Entwurf §39.1:2572-2580; §33.2:2038-2043; `LoudnessAccumulator.h`:283,321,354. **BELEGT, BAULÜCKE** (Momentary 400 ms existiert heute nicht als eigenes Fenster) |
| M-02 | Sonde misst; True Peak wird gebraucht | True Peak ist BS.1770-konform. Der vorhandene 8-fach-Pfad bleibt **nur**, wenn er das offizielle EBU-Testset bei 48 kHz und generierte Mehrsampleraten-Goldens besteht; sonst wird der BS.1770-Polyphase-Referenzpfad verwendet. Toleranz ±0,1 dB (§49.3). Kein Rateweg: `truePeakDb` in `AnalyseEngine.h`:102 ist heute Gen-lokal und ungegated über die ganze Passage. | **NEU** `EqCopSonde013TruePeakGoldenTest` mit dem EBU-Testset als Fixture und einem Sampleraten-Sweep; das Bein muss die Entscheidung 8-fach gegen Polyphase **messbar** treffen, nicht behaupten. | Entwurf §39.1:2578-2582,2588-2591; §49.3:3584. **BELEGT, BAULÜCKE** |
| M-03 | Passage liegt vor; Headroomaussage wird gebildet | `PLR = Passage-True-Peak-Maximum minus LUFS-I`, `PSR(3 s) = True-Peak-Maximum desselben 3-s-Fensters minus LUFS-S`. Beide werden ausdrücklich als ergänzende Produktmetriken bezeichnet, nie als EBU-Qualitätsurteil. Headroom reist in dBTP und als Verteilung über die Passage, nicht als Einzelspitze. | **NEU** `EqCopSonde013DynamicsTest`, Fall `plr_psr_definitions_and_labels`; **NEU** `EqCopSonde013DynamicsTest`, Fall `headroom_is_distribution_not_single_peak`. | Entwurf §39.1:2583-2586; §39.2:2610-2612. **BELEGT, BAULÜCKE** (`psrDb` existiert im Frame :370, aber gegen LUFS-S desselben Rahmens, nicht gegen ein 3-s-True-Peak-Maximum) |
| M-04 | Sonde misst; Dynamikaussage wird gebildet | Crest wird in **mehreren** Fenstern geführt, nicht nur als globales Maximum. LRA gilt erst nach mindestens rund 60 s geeignetem Material; davor ist der Wert ausdrücklich `nicht belastbar` und wird nicht numerisch vorgetäuscht. | **NEU** `EqCopSonde013DynamicsTest`, Fälle `crest_multiple_windows` und `lra_below_sixty_seconds_is_not_a_number`. | Entwurf §39.1:2580-2587; §39.3:2617. **BELEGT, BAULÜCKE** |
| M-05 | Ereignisring hat Einträge; Evidenzsnapshot wird fällig | Diese Zeile ist **nur der Transport**. Der lokal **teilweise** gebaute `DynamicsEvent`-Strom reist als `evidence_snapshot.ereignisse`. Jedes Ereignis trägt Samplezeit, Stärke, Bandzentrum, Dauer und Qualität; die zwei Qualitätsbits trennen den SuperFlux-Flusspfad vom eigenständigen Peakpfad. Dass der Peakpfad heute überhaupt nicht auslöst, ist nicht diese Zeile, sondern **M-86**. Kadenz 1 bis 4 Hz; bei Überlast wird die Kadenz reduziert, nie der Ring stillschweigend geleert. Der Ring bleibt auf 64 Plätze gedeckelt und zählt seine Verluste. | Bestehend **B5** für die lokale Erzeugung; **NEU** **A5** `pruefe_v3_vertrag.py`-Fall `evidence_snapshot_ereignisse_belegt`; **NEU** `EqCopSonde013EventWireTest`, Fall `events_travel_with_quality_and_loss_counter`. | Entwurf §39.1:2593-2600; §33.2:2041; `reservierte-nachrichten-v1.json` (Feldname für SONDE-013 verbrannt); `FeatureEngine.h`:290-313,448,1607-1650. **BELEGT, BAULÜCKE** — Abgrenzung zu M-86 aus Nacharbeit 1 (§8, D6) |
| M-06 | Beliebige Metrik verlässt die Sonde | Jede Metrik trägt eine Qualitätsklasse mit. Eine Gesamtklasse wird nicht aus einem Mittelwert gerettet: ein harter Mangel bei Session, Passage, Coverage oder Alignment begrenzt die Gesamtaussage. Gewichte und Schwellen leben in einer versionierten `metrics_version`, die im Frame bereits mitreist. | **NEU** `EqCopSonde013QualityClassTest`, Fall `hard_deficiency_caps_overall_class`; **NEU** **A5**-Fall `metrics_version_bindet_schwellen`. | Entwurf §34.3:2224-2237; `.fbs`:218 (`metrics_version` ID 2). **BELEGT, BAULÜCKE** |
| M-07 | Beliebige Metrik; ein nicht endlicher Zwischenwert entsteht | Nichtendliches wird beim **Erzeugen** in Wert 0 mit `gueltig=false` übersetzt und gezählt, nie sanitisiert auf die Leitung gelassen. Ein NaN oder Inf im Frame ist ein Senderfehler und wird von beiden Lesern abgelehnt. Bei den Rahmenskalaren fehlt statt dessen das Präsenzbit. Der Zähler wird ausgewertet, nicht ignoriert. | Bestehend **B3c** `EqCopSchemaTest` und **A10** `erzeuge_fb_fixtures.py` für die Ablehnung; bestehend **B9** für `bloeckeNichtEndlich()`; **NEU** `EqCopSonde013MetricsEdgeTest`, Fall `non_finite_never_reaches_wire_for_new_metrics`. | `.fbs`:208-215; `quantisierung-v1.json`; `LoudnessAccumulator.h`:383; CLAUDE.md NaN-Ehrlichkeit. **BELEGT** für den heutigen Bestand, **BAULÜCKE** für jede neue Metrik |
| M-08 | Monomaterial oder ein stiller Kanal; Stereometrik wird gebildet | Bei echtem Mono ist Korrelation exakt 1 und Breite 0; bei einem stillen Kanal fällt das Präsenzbit weg, statt 0 oder NaN zu senden. Der Mono-Folddown wird am **wirklich gefalteten** Puffer gemessen, nicht aus einem Korrelationsskalar geschätzt, und stimmt innerhalb 0,25 dB. Physischer Mono-Check ist `mono=(L+R)/2`, energienormiertes M/S ist `M=(L+R)/√2`, `S=(L−R)/√2` — die beiden werden nicht vermischt. | Bestehend `FeatureEngine.h`:2008-2022 für die Präsenzbits; **NEU** `EqCopSonde013StereoGoldenTest`, Fälle `mono_identity`, `silent_channel_has_no_bit`, `folddown_matches_real_buffer_within_0p25db`. | Entwurf §40.1:2637-2652; §40.3:2666-2669; `FeatureEngine.h`:2008-2022. **BELEGT, BAULÜCKE** |
| M-09 | Epochengrenze liegt kurz zurück; nächster Rahmen wird fällig | Der Frame nennt, über wie viel Audio er integriert wurde. Ein Feld `integration_samples` im `FeatureFrame` **und** in `table Frame` (nächste freie Feld-ID 14) macht einen dünnen Frame erkennbar; der Empfänger entscheidet selbst, ob er ihn anzeigt. Ohne das Feld kann er heute „leise" nicht von „kurz gemessen" unterscheiden. Save/Load ist nicht betroffen, der Wire-Vertrag schon: neues Feld ⇒ `schema_minor`-Bump plus beidseitige Fixtures. | **NEU** **A10** `erzeuge_fb_fixtures.py`-Fall `integration_samples_id14`; **NEU** **A9** `pruefe_flatc_drift.py` bleibt Drift-0-Wache; **NEU** **B3c**-Fall `integration_samples_wird_von_beiden_lesern_klassifiziert`; **NEU** `EqCopSonde013MetricsEdgeTest`, Fall `thin_frame_after_boundary_is_recognisable`. | NAK-47/NAK-68-Zitat §1.5; `docs/beweise/SONDE-009.md` §10.1; `.fbs`:216-239; `FELD-IDS.json`. **BELEGT, BAULÜCKE** |
| M-10 | Samplerate wechselt; Bandmetrik wird gebildet | Die Nyquist-Kappe bleibt `min(18 kHz, 0,95 · Nyquist)`. Bänder darüber bekommen kein Gültigkeitsbit. Jede neue bandweise Metrik dieses Tickets erbt dieselbe Kappe; keine zweite Kappenregel entsteht. | Bestehend **B5** (Kappe greift bei 22,05 kHz wirklich); **NEU** `EqCopSonde013StereoGoldenTest`, Fall `band_stereo_metrics_share_the_nyquist_cap`. | `FeatureEngine.h`:433-437,944,975; Entwurf §35.1:2222-2226. **BELEGT, BAULÜCKE für neue Bandmetriken** |
| M-11 | Stereoanalyse; zwei globale Skalare reichen nicht | Der Worker liefert aus komplexen L/R-STFTs bandweise Mid-/Side-Energie und Side-Anteil in dB, bandweise Pearson-Korrelation in kurzen und mittleren Fenstern, Magnitude-Squared Coherence, **bandweise Interchannel-Phase**, gemessenen Mono-Folddown-Verlust sowie L/R-Balance, Zeitperzentile und Persistenz. Kohärenz ist keine Einzel-FFT-Metrik: Auto- und Cross-Spektren werden über mindestens **acht** gültige überlappende Welch-Frames gemittelt. Jedes Band führt zwei Metadatenfelder der Evidenz mit: **Fensterdauer in ms** und **Freiheitsgrade**, also die Zahl der gemittelten gültigen Welch-Frames. Fail-closed in zwei Stufen: Kohärenz ist `null` bei zu wenig Energie oder weniger als acht Frames, und die **Phase wird nur** in Bändern ausgewertet, deren Kohärenz eine benannte Schwelle überschreitet — sonst ist auch sie `null`, nie ein geschätzter Wert. Alle drei Felder reisen auf dem Evidenzpfad (E-05) und stehen in §3.8 als von SONDE-013 belegte Felder des `evidence_snapshot`. | **NEU** `EqCopSonde013StereoGoldenTest`, Fälle `bandwise_ms_and_correlation`, `phase_only_in_coherent_bands`, `coherence_carries_window_and_dof`, `coherence_is_null_below_eight_frames` (ersetzt `coherence_needs_eight_welch_frames`), `persistence_is_reported`. | Entwurf §40.1:2628-2660, wörtlich :2642-2645 („Fensterdauer und Freiheitsgrade werden Teil der Evidenz", „Bei zu wenig Energie oder Frames ist Kohärenz `null`", „Interchannel-Phase wird nur in ausreichend kohärenten Bändern interpretiert"). **BELEGT, BAULÜCKE** — Phase, Fensterdauer und Freiheitsgrade aus Nacharbeit 1 (§8, D7) |
| M-12 | Auffälliger Stereozustand erkannt | Breite ist kein Qualitätswert. Es entsteht **kein** Vorschlag zu Laufzeit- oder Polaritätskorrektur ohne kohärentes Paar, stabilen Lag und nachweislich bessere Mono-Summe; bei niedriger Kohärenz gibt es keine Lag- oder Polaritätsempfehlung. Statische Breitenänderung und bandbegrenzte M/S-Korrektur sind verschiedene Vorschlagstypen. Musikalisch unabhängige Busse werden nie automatisch gegeneinander verschoben. | **NEU** `EqCopSonde013StereoGoldenTest`, Fälle `low_coherence_yields_no_recommendation` und `width_alone_is_never_a_defect`. | Entwurf §40.2:2661-2665; §40.3:2668. **BELEGT, BAULÜCKE** |
| M-86 | Sehr kurzer Impuls; der spektrale Fluss überschreitet die adaptive Schwelle **nicht** | Der Detektor hat **zwei unabhängige Auslöser**. Peaksteigung — der Anstieg des Rahmenpeaks gegenüber dem Vorrahmen in dB je Rahmen — und Crest erzeugen ein Ereignis auch ohne Flussüberschreitung; es trägt `qualitaetFluss = false` und `qualitaetPeak = true`. Ein Flussereignis trägt `qualitaetFluss = true`. Lösen beide Pfade im selben Rahmen aus, entsteht **genau ein** Ereignis mit beiden Bits. Ring-Deckel 64 und Verlustzähler gelten unverändert. Die Schwellen der Peaksteigung wählt der Bau begründet und misst sie am Golden; sie sind Startwerte, kein Literal ohne Beleg (§5.3, Risiko 5). | **NEU** **B5** `EqCopAnalysisGoldenTest`-Fälle `short_impulse_triggers_peak_path_only`, `flux_event_carries_flux_quality` und `both_paths_yield_one_event`. | Entwurf §39.1:2592-2599 (Detektor aus spektralem Fluss, Peaksteigung und Crest); Ist-Stand §2.1 (`FeatureEngine.h`:1605-1621 — heute nur Flussauslöser, `qualitaetFluss` konstant `true`, Peaksteigung unbenutzt). **BELEGT, BAULÜCKE** — neue Zeile aus Nacharbeit 1 (§8, D6) |

### 3.2 PRE/POST-Paare, Alignment und ehrliche Herabstufung

| ID | Ausgangszustand × Ereignis | Zusage samt Reihenfolge und Frist | Test | Quelle / Belegstatus |
|---|---|---|---|---|
| M-13 | Zwei Sonden melden `measurement_position` `pre` beziehungsweise `post` mit derselben `pair_id` | Ein Paar entsteht nur aus **genau einer** PRE- und **genau einer** POST-Hälfte derselben `pair_id` in derselben Session. Reihenfolge: Descriptor validieren → Paarkandidat bilden → Vollständigkeit prüfen → erst dann eine Paaraussage erzeugen. Eine fehlende Hälfte ergibt einen benannten unvollständigen Zustand, nie eine halbe Aussage. Zwei PRE oder zwei POST derselben `pair_id` sind ein Konflikt, kein Zufallssieger. | **NEU** `sonde013_prepost.rs`, Fall `pairing_requires_exactly_one_pre_and_one_post`; **NEU** derselbe Test, Fall `duplicate_half_is_conflict_not_first_wins`; bestehend **A5** für die Discriminatorform. | `eq-ipc-v3.schema.json`:573-576,613,642; Entwurf §3.4:699-711; §10:930-935. **BELEGT, BAULÜCKE im v3-Pfad** (v2-Vorläufer `broker/src/lib.rs`:656) |
| M-14 | Vollständiges Paar; Aussage wird gebildet | Es gibt **drei getrennte Ergebnisse**, nie einen einzigen Differenzwert: rohe Messdifferenz derselben Projektfenster, ausgerichtetes pegelbezogenes Delta nach sicherer Restlag- und Gainschätzung, und interpretierte Wirkung. Die Trennung verhindert, dass mehr Pegel als mehr Höhen oder eine Laufzeitänderung als EQ-Effekt erscheint. | **NEU** `sonde013_prepost.rs`, Fall `three_results_are_never_collapsed`. | Entwurf §38.1:2470-2477. **BELEGT, BAULÜCKE** |
| M-15 | Paar liefert Features; Restlag wird geschätzt | Erster Anker ist `project_sample_start` aus dem Host. Danach schätzt der Broker auf gemeinsamem Material einen Restlag über normierte Kreuzkorrelation mehrerer bandbegrenzter Hüllkurven und Onsetfolgen. Suchraum höchstens ±2 s bei mindestens 6 s Capture; bei kürzerem Capture wird er proportional verkleinert. Host-PDC darf ihn um einen erwarteten Offset zentrieren, ersetzt die Prüfung aber nicht. Ergebnis ist ausdrücklich **keine** behauptete Samplegenauigkeit. | **NEU** `sonde013_prepost.rs`, Fall `feature_alignment_search_space_and_pdc_is_hint_only`; **NEU** Golden-Korpus mit 0 bis 2 s bekannter Verzögerung, Treffer innerhalb **eines Feature-Hops**. | Entwurf §38.2:2480-2506; §38.5:2551-2553. **BELEGT, BAULÜCKE** |
| M-16 | Restlag geschätzt; Alignment-Score wird gesetzt | Genau vier Klassen: `feature_aligned`, `audio_aligned`, `probable`, `unclear`. Akzeptiert wird nur, wenn Korrelationsspitze und Peak-to-Sidelobe-Verhältnis ausreichen, mehrere Bänder einen konsistenten Lag liefern, der Lag über Teilfenster stabil bleibt und weder Transportlücke noch andere Samplerate vorliegt. Geschätzter Lag, seine Auflösung und seine Streuung bleiben sichtbar. Rauschen oder anderes Material ergibt `unclear`. | **NEU** `sonde013_prepost.rs`, Fall `alignment_class_has_four_values_and_each_criterion_falls_alone`; **NEU** Fall `noise_yields_unclear`. | Entwurf §38.2:2493-2506; §38.5:2551-2555. **BELEGT, BAULÜCKE** |
| M-17 | Paar mit Alignment; Coverage wird bestimmt | Coverage ist die gemeinsame Abdeckung der beiden Messfenster in Projektzeit. Reihenfolge: beide Fenster in Projektsamples auflösen → Überlappung berechnen → gegen das **kürzere** Fenster normieren. Zwei gültige Fenster an entgegengesetzten `i64`-Rändern dürfen weder paniken noch umbrechen und dadurch als deckungsgleich gelten. Überlappung ≤ 0 heißt „vermutlich verschiedene Passagen", nicht 0 % Ähnlichkeit. | **NEU** `sonde013_prepost.rs`, Fall `coverage_is_saturating_and_normalised_to_shorter_window`; Formvorläufer bestehend `broker/src/lib.rs`:793-806. | Entwurf §58 Lieferumfang; `broker/src/lib.rs`:784-806. **BELEGT, BAULÜCKE im v3-Pfad** |
| M-18 | Paar; die Kette dazwischen ist zeitvariabel oder nichtlinear | Inkonsistente Pegel- und Spektralrelationen werden **markiert**. Danach darf Nakama keinen festen Übertragungsfrequenzgang behaupten. Kompression, Modulation, Saturation und wechselnde Latenz erzeugen keine falsche statische EQ-Behauptung; das Ergebnis lautet dann „wahrscheinliche PRE/POST-Wirkung" statt einer kausalen Behauptung. | **NEU** `EqCopSonde013PrePostGoldenTest` mit adversarialen Fixtures Kompressor, Tremolo, Saturation und wechselnder Latenz, Fall `nonlinear_chain_never_yields_static_eq_claim`. | Entwurf §38.3:2509-2513; §38.5:2560; §10 Grenze:948-950. **BELEGT, BAULÜCKE** |
| M-19 | Paar; Transferfunktion wird erwogen | `H₁(f)=Sᵧₓ/Sₓₓ` und Magnitude-Squared Coherence entstehen **nur im lokalen Compare-Routing** und nur für eine hinreichend stationäre lineare Kette. Ein Transferwert ist nur in Bändern mit genügend PRE-Energie und zunächst mindestens 0,8 Kohärenz zulässig; ab rund 0,9 gilt er als sehr belastbar. Unterhalb davon und bei **verteilten** Probes zeigt Nakama robuste P10/P50/P95-Differenzen statt eines vermeintlichen Frequenzgangs. Da es in P4 kein Compare-Routing gibt, ist der verteilte Zweig der einzige aktive. | **NEU** `sonde013_prepost.rs`, Fall `distributed_pair_never_produces_transfer_function`. | Entwurf §38.3:2515-2521. **BELEGT**; **BAULÜCKE nur im Bein** — der Compare-Routing-Zweig selbst gehört ausdrücklich nicht in dieses Ticket (§5.3) |
| M-20 | Paar; Vergleichspegel wird gebraucht | Der Vergleichspegel wird für die gewählte Passage **vorab** gemessen und während des A/B eingefroren. Eine kontinuierliche automatische Nachregelung während des Umschaltens ist ausgeschlossen: sie bewertet Transienten und Stille falsch und wird selbst zum hörbaren Prozessor. | **NEU** `EqCopSonde013PrePostGoldenTest`, Fall `match_gain_is_frozen_before_ab_not_tracked`. | Entwurf §38.3:2523-2526; §43.1:2833. **BELEGT, BAULÜCKE** |
| M-21 | Zeitpfad zwischen zwei Instanzen ist nicht validiert; Cross-Probe- oder PRE/POST-Aussage wird angefragt | Solange FLs PDC-/Presentation-Abbildung nicht bewiesen ist, lautet der Schlüssel `raw_project_frame_key = (instance_id, transport_epoch, sample_rate, resolution, frame_start)` und behauptet **keine** Inhaltsgleichheit zwischen Instanzen. Erst eine validierte Presentation-Abbildung **plus** Alignmentqualität erzeugt `aligned_frame_key = (session_epoch, timeline_epoch, sample_rate, resolution, aligned_start)` für starke Cross-Probe-Evidenz. Exit-Gate wörtlich: **„Kein unbekannter Zeitpfad erzeugt eine starke Cross-Probe- oder PRE/POST-Aussage."** | **NEU** `sonde013_prepost.rs`, Fall `unknown_time_path_can_never_reach_strong` als Gate-Bein — es muss über **alle** Wege scheitern, nicht nur über den erwarteten; **NEU** Mutationsprobe: ein künstlich auf `aligned` gehobener Schlüssel bringt das Bein zum Fallen. | Entwurf §35.1:2255-2266; §58 Exit-Gate:4288-4289. **BELEGT, BAULÜCKE** |
| M-22 | Eine Hälfte ist getrennt, stale, sammelt noch oder lief ohne Transport | Harte Ausschlüsse führen zu `unclear` mit benanntem Grund, nicht zu einer schwachen Zahl: fehlende Hälfte, getrennte Hälfte, stale Hälfte, Zustand nicht messbereit, fehlende Projektzeit, Fensterlänge ≤ 0. Der v2-Vorläufer führt genau diese Fälle bereits; der v3-Pfad erbt sie samt Wortlautklasse. | **NEU** `sonde013_prepost.rs`, Fall `hard_exclusions_each_name_their_reason`; Formvorläufer bestehend `broker/src/lib.rs`:700-806, gedeckt von **A4**. | `broker/src/lib.rs`:700-806; Entwurf §58 („ehrliche Herabstufung"). **BELEGT, BAULÜCKE im v3-Pfad** |
| M-23 | Paar ist vollständig, aber die Überdeckung ist gering, die aktive Messzeit klafft auseinander oder die Hälften laufen in verschiedenen Prozessen | Herabstufung auf `probable` mit benanntem Grund statt Ausschluss. Im v3-Pfad sind es **genau drei** Gründe: Überlappung unter 80 % des kürzeren Fensters; aktive Messzeit um mehr als 10 % verschieden (Smart Disable oder Stille auf einem Punkt); verschiedene Host-PIDs. Der vierte Grund des v2-Vorläufers, „Loop-/Seek-Sprünge im Messfenster", **entfällt im v3-Pfad** und bleibt nur als Vorläufer benannt: eine Passage bindet an genau eine Transportepoche (§32.4), also **invalidiert** ein Sprung die Messung, statt sie herabzustufen — Regel und Abgrenzung stehen in M-53. Keine Herabstufung ist ein stiller Ausschluss. | **NEU** `sonde013_prepost.rs`, Fall `each_downgrade_reason_falls_alone` mit einem Fall je Grund, also **drei**; **NEU** `sonde013_taint.rs`, Fälle `seek_inside_passage_invalidates_instead_of_downgrading` und `loop_wrap_at_passage_edge_starts_next_pass`; Formvorläufer bestehend `broker/src/lib.rs`:809-826 (dort vier Gründe — v2-Erbe, Verlauf). | `broker/src/lib.rs`:809-826 (v2-Vorläufer, Plan §5.7); Entwurf §58:4285 („Invalidierung bei Marker, Preview, Seek, Drop, geändertem Material oder Messpunkt"); §32.4:1969; §15 Vergleichbarkeitsregeln:1093-1098. **BELEGT, BAULÜCKE im v3-Pfad** — Sprunggrund gestrichen in Nacharbeit 1 (§8, D3) |
| M-24 | Paar; hörbares Delta wird angefragt | Feature-Telemetrie kann `POST − PRE` niemals als Audio erzeugen. Ohne nachgewiesenes Compare-Routing bietet Gen nur Messvergleich plus angeleiteten manuellen Bypass. Hörbares Delta wird ohne Routingnachweis **nicht** freigeschaltet. | **NEU** `EqCopSonde013PrePostGoldenTest`, Fall `audible_delta_locked_without_compare_routing`. | Entwurf §38.4:2528-2549; §38.5:2562. **BELEGT**; **BAULÜCKE nur im Bein** — die Freischaltung gehört zu P8, hier gilt nur die Sperre |

### 3.3 Manuell markierte Passage, Fingerprint und Comparability-Score

| ID | Ausgangszustand × Ereignis | Zusage samt Reihenfolge und Frist | Test | Quelle / Belegstatus |
|---|---|---|---|---|
| M-25 | User markiert eine Passage | Eine `Passage` speichert Start und Ende, Transportepoche, aktive Quellen, Abdeckung, manuelles Label und den Fingerprint. Reihenfolge: Grenzen in Projektsamples auflösen → Transportepoche binden → aktives Quellenset einfrieren → Abdeckung berechnen → Fingerprint bilden → Objekt in den Store schreiben. Autoritative Ablage ist der Store; `MainProjectState` hält die manuellen Passagen als Projektintent, der mit der FL-Projektdatei reist. | **NEU** `sonde013_passage.rs`, Fall `passage_carries_all_six_fields`; **NEU** `EqCopSonde013PassageStateTest`, Fall `manual_passage_persists_in_main_project_state_and_marks_host_dirty`. | Entwurf §32.4:1967-1975; §33.5:2135-2142 (Zeile „manuelle Passagen"); `broker/src/store/migration.rs`:68-76. **BELEGT, BAULÜCKE** |
| M-26 | Passage wird gebildet; Fingerprint entsteht | Der Fingerprint besteht aus quantisierten Bandenergie-, Chroma- und Onset-Verläufen. Er enthält **kein rekonstruierbares PCM** und dient nur dazu, grob anderes musikalisches Material zu erkennen. Ein Experiment kann vollständig inklusive Evidenz-IDs exportiert und ohne PCM gelöscht werden. | **NEU** `EqCopSonde013FingerprintGoldenTest`, Fall `fingerprint_carries_no_reconstructible_pcm` mit adversarialer Rückrechenprobe; **NEU** `sonde013_passage.rs`, Fall `export_and_delete_without_pcm`. | Entwurf §32.4:1969-1971; §43.4:2818 (Exportkriterium). **BELEGT, BAULÜCKE** |
| M-27 | Fingerprintfenster läuft; Epochengrenze tritt ein | Kein Fingerprintfenster überbrückt eine echte oder mögliche Epochengrenze. Es gilt dieselbe Ein-Block-Quarantäne wie für FFT-, Loudness- und Korrelationsfenster: erst der monotone Beginn des Folgeblocks versiegelt zur Veröffentlichung; stoppt der Callback vorher, bleibt das Fenster unvollständig. Ein nachträglich sichtbarer Loop-Wrap kann so keine persistierte Evidenz kontaminieren. | Bestehend **B4** `EqCopQueueStressTest` für die Quarantäne selbst; **NEU** `EqCopSonde013FingerprintGoldenTest`, Fall `fingerprint_window_never_crosses_epoch_boundary`. | Entwurf §32.3:1955-1966; `docs/beweise/SONDE-008.md`. **BELEGT, BAULÜCKE** |
| M-28 | Zwei Passagen; Vergleichbarkeit wird bestimmt | Vergleichbarkeit entsteht aus **fünf** Belegen, nicht aus einem: identischer Projektbereich, gleicher beziehungsweise sehr ähnlicher Fingerprint, vergleichbare aktive Quellen, gleiche Samplerate und Messpunktklasse, ausreichende gemeinsame Abdeckung. **Kein einzelner Hash hebt einen Widerspruch der anderen Belege auf.** | **NEU** `sonde013_passage.rs`, Fall `comparability_needs_all_five_and_hash_alone_never_wins` mit einem Gegenbeispiel je Beleg. | Entwurf §32.4:1971-1983. **BELEGT, BAULÜCKE** |
| M-29 | Vergleichbarkeit wird als Score ausgedrückt | Startgates für **starke** Vergleichbarkeit sind 95 % Zeitüberdeckung, Aktivquellen-Jaccard mindestens 0,9 und Upstream-Feature-Cosine mindestens 0,95. Sie sind ausdrücklich **Startwerte, am Korpus zu kalibrieren**, und leben deshalb in der versionierten `metrics_version`, nicht als Literal im Produktpfad. Unterhalb eines Gates gibt es keinen starken Vergleich, sondern eine benannte Herabstufung. | **NEU** `sonde013_passage.rs`, Fall `each_start_gate_falls_alone_at_its_threshold` mit Gleichheitsfall und erstem Unterschreiten je Gate; **NEU** **A5**-Fall `comparability_schwellen_haengen_an_metrics_version`. | Entwurf §43.2:2861-2866; §34.3:2235-2237. **BELEGT, BAULÜCKE** |
| M-30 | Passage liegt vor; Abdeckung wird gebraucht | Abdeckung ist die tatsächlich gemessene Signalabdeckung der Passage, nicht ihre Wanddauer. Smart Disable und Suspend gelten als **fehlende** Coverage; nach Resume oder Discontinuity beginnt eine neue Transportepoche. Eine Passage mit unzureichender Abdeckung erhält kein Siegerlabel. | **NEU** `sonde013_passage.rs`, Fall `coverage_is_measured_signal_not_wall_duration`; **NEU** derselbe Test, Fall `suspend_counts_as_missing_coverage`. | Entwurf §48.2:3437-3439; §43.2:2870; §34.2:2196. **BELEGT, BAULÜCKE** |
| M-31 | Passage soll gebildet werden, aber das Material wechselt mittendrin | Bei anderem Material oder unzureichender Abdeckung gibt es **kein** Siegerlabel. Der Fingerprintvergleich meldet den Wechsel, statt zwei Hälften zu mitteln. Das ist der Produktschutz aus §15: „Warnung oder Sperre, wenn das musikalische Material nicht vergleichbar ist." | **NEU** `EqCopSonde013FingerprintGoldenTest`, Fall `material_change_mid_passage_is_detected_not_averaged`. | Entwurf §15:1098; §43.2:2870. **BELEGT, BAULÜCKE** |
| M-32 | Passage existiert im Store; Broker startet neu oder Main lädt neu | Die Passage bleibt rekonstruierbar: Store ist autoritativ, `MainProjectState` hält kompakte aktuelle IDs. Fehlt oder wird die SQLite-Datenbank gelöscht, bleiben Projekt-Recall und DSP vollständig — nur historische Passagen und Experimente fehlen. Umgekehrt darf ein alter Datenbankeintrag nie einen neueren Plugin-State zurücksetzen. | Bestehend **A4-SI** `store_crash_matrix` für die Rekonstruktionsform; **NEU** `sonde013_passage.rs`, Fall `passage_survives_restart_and_missing_db_degrades_gracefully`. | Entwurf §33.5:2129-2160; `broker/src/store/migration.rs`:68-76. **BELEGT, BAULÜCKE** |

### 3.4 Lokaler Hörmarker fail-closed und die NAK-47-Ausblendung

| ID | Ausgangszustand × Ereignis | Zusage samt Reihenfolge und Frist | Test | Quelle / Belegstatus |
|---|---|---|---|---|
| M-33 | Beliebiger Zustand; Hörmarker wird angefragt | Der Marker färbt **nur**, wenn alle vier Terme gelten: gültiges `playing=true`, `recording=false`, Realtime bewiesen und Editor offen. Fehlt einer, bleibt der Puffer bitidentisch. Heute stehen drei davon im Term (`PluginProcessor.cpp`:446-452); **`recording=false` fehlt**, obwohl `aufnahmeGueltig` und `aufnahmeAktiv` im Prozessor vorliegen (:335-336) und bereits im `state_report` reisen (:802-803). Ein **unbekannter** Aufnahmezustand blockiert wie ein aktiver — fail-closed, nicht fail-open. | Bestehend **A3** `EqCopMarkierungTest` für die drei vorhandenen Terme samt U10-Gegenprobe; **NEU** **A3**-Fall `recording_true_and_recording_unknown_both_block`, mit Gegenprobe bei gültigem `recording=false`. | Entwurf §58:4283; §49.2 Gate 1:3550-3552; §33.4:2116-2118 (dieselbe Regel für Remote-Preview); `PluginProcessor.cpp`:335-336,446-452,802-803. **BELEGT, BAULÜCKE** |
| M-34 | Marker ist hörbar; `erlaubt` fällt weg (Editor schließt, Transport stoppt, Echtzeitbeweis verfällt) | Statt des Sofortschnitts läuft ein kurzer **erzwungener Ausfade**. Reihenfolge im Audiothread: `erlaubt`-Verlust erkennen → Zielverstärkung 0 setzen → über die vorgesehene Rampe ausblenden → erst nach erreichtem Null die Filterzustände leeren und den Neutralpfad einnehmen. Frist: die Ausblendung ist auf eine feste, blockgrößenunabhängige Dauer begrenzt und darf keine Pluginlatenz und keinen Tail erzeugen. Budget §49.3: „A/B-Zustandswechsel: kein Klick; identischer Zustand nullt innerhalb numerischer Toleranz." | **NEU** **A3**-Fall `permission_loss_fades_instead_of_cutting`, gemessen als maximale Sampledifferenz an der Kante gegen eine Klickschwelle; **NEU** **A3**-Fall `fade_out_adds_no_latency_and_no_tail`. | NAK-47 (§1.5); Entwurf §49.3:3585; `HoerMarkierung.h`:306-313,419-424. **BELEGT, BAULÜCKE** |
| M-35 | Ausfade ist abgeschlossen | Der Neutralpfad ist danach **bit-transparent**: der Puffer wird nicht mehr angefasst, nicht mit 1,0 multipliziert und nicht durch einen Identity-Filter geschickt. Das ist die Kante, die den NAK-47-Fix schwierig macht — sie darf durch ihn nicht verlorengehen. | Bestehend **A3** und **A1** `EqCopNullTest` als Regressionswache; **NEU** **A3**-Fall `after_fade_the_neutral_path_is_bit_identical` mit Bitvergleich über mehrere Blockgrößen. | Entwurf §49.2 Gate 1:3550-3552; §44.2:2997-2999 („ein rechnerischer Identity-Filter reicht nicht als Nullvertrag"); `HoerMarkierung.h`:306-313. **BELEGT**, heute erfüllt; **BAULÜCKE nur im Bein** (Bitvergleich über mehrere Blockgrößen fehlt) |
| M-36 | Marker ist hörbar; der Hostblock übersteigt `wetKapazitaet` | Derselbe Ausfadevertrag gilt auf dem Oversize-Pfad. Heute steht dort ebenfalls `hartAus()` (`HoerMarkierung.h`:280-282). Entschieden ist ein **erzwungener Ausfade innerhalb der Kapazität, danach Riegel bis `prepareToPlay`**: der Wet-Pfad läuft über die ersten `min(n, wetKapazitaet)` Samples mit einer linearen Rampe auf 0 der Länge `min(Rampenlänge, wetKapazitaet)`; Rest des Blocks und jeder Folgeblock sind bitidentischer Neutralpfad, und ein erneutes `erlaubt` blendet in dieser Laufzeit nicht wieder ein. Vollständiger Wortlaut in §7.1, E-01. | **NEU** **A3**-Fall `oversize_block_fades_within_capacity_then_latches` (ersetzt `oversize_block_does_not_cut_mid_buffer`). | NAK-47 (zweite Fundstelle); `HoerMarkierung.h`:280-282; §7.1 E-01. **BELEGT, ENTSCHIEDEN** — §7.1 E-01 |
| M-37 | Marker beginnt zu klingen | Das Ereignis reist **sofort** als `audible_intervention_begin` mit `art=hoermarkierung`, `intervention_id`, gebundener Adresse, monotoner `event_sequence` und, soweit verfügbar, `project_sample_start`. Reihenfolge: Audiothread schreibt lockfrei in einen kleinen **vorallokierten** RT→Control-Ring → Control-Worker sendet. **Der Audiothread berührt nie die Pipe.** Das heutige Heartbeat-Bit (`PluginProcessor.cpp`:967) genügt nicht, weil ein kurzer Marker vollständig zwischen zwei Heartbeats liegen kann. | **NEU** `EqCopSonde013InterventionRingTest`, Fälle `begin_is_immediate_not_heartbeat_bound`, `audio_thread_allocates_zero_and_touches_no_pipe`; bestehend `broker/tests/coordinator_model.rs` für die Empfängerseite, gedeckt von **A4**. | Entwurf §34.2:2205-2211; `eq-ipc-v3.schema.json`:1048-1066; `PluginProcessor.cpp`:967. **BELEGT, BAULÜCKE** |
| M-38 | Marker endet | `audible_intervention_end` reist mit derselben `intervention_id`, fortlaufender `event_sequence`, optionalem `project_sample_end` und einem **konservativen** `tail_samples`: der Bereich wird länger quarantänisiert, als die Intervention gedauert hat. Der Marker endet erst nach abgeschlossenem Ausfade (M-34), nicht bei Verlust der Erlaubnis. | **NEU** `EqCopSonde013InterventionRingTest`, Fall `end_carries_conservative_tail_and_follows_the_fade`; bestehend `broker/src/coordinator/intervention.rs`:66-92 als Empfänger, gedeckt von **A4**. | Entwurf §34.2:2205-2211; `eq-ipc-v3.schema.json`:1067-1084. **BELEGT, BAULÜCKE** |
| M-39 | RT→Control-Ring läuft über, Control trennt oder eine Sequenzlücke tritt auf | Dieser Ring verwendet für P0 **kein** `drop-oldest`. Overflow, Control-Disconnect oder Sequenzlücke setzen sticky `intervention_state_unknown`. Dann werden alle betroffenen `timeline_epoch`-Bereiche seit dem letzten bestätigten Event konservativ quarantänisiert beziehungsweise rückwirkend invalidiert; starke Evidenz bleibt bis zu einem expliziten Neutral-/Sequenz-Resync gesperrt. **Ein verlorenes Begin oder End darf niemals eine scheinbar saubere Baseline erzeugen.** | Bestehend `broker/src/coordinator/intervention.rs`:22-34,66-92,124-136,139-152 über **A4**; **NEU** `EqCopSonde013InterventionRingTest`, Fall `ring_overflow_reports_instead_of_dropping_oldest`. | Entwurf §34.2:2212-2220; `intervention.rs`. **BELEGT im Broker, BAULÜCKE im Producer** |

### 3.5 Experiment `manual_external`

| ID | Ausgangszustand × Ereignis | Zusage samt Reihenfolge und Frist | Test | Quelle / Belegstatus |
|---|---|---|---|---|
| M-40 | User will eine Fremdänderung nachmessen; Versuch beginnt | `experiment_begin` mit `execution_mode=manual_external`. Reihenfolge: Passage bestimmen → Baseline messen → Baseline **verriegeln** → `experiment_begin` mit unveränderlichen Referenzen in den append-only Store → User um die Änderung bitten → dieselbe Passage erneut erfassen. Der Versuch trägt Passage- und Upstream-Fingerprint, aktives Quellenset, Messpunktklassen, eingefrorenen Match-Gain und Alignmentqualität. | **NEU** `sonde013_experiment.rs`, Fall `manual_external_begin_locks_baseline`; **NEU** **A5**-Fall `experiment_begin_belegt_von_sonde013`. | Entwurf §43.1:2828-2840,2852-2860; `reservierte-nachrichten-v1.json` (Eigentümer SONDE-013). **BELEGT, BAULÜCKE** |
| M-41 | Versuch läuft; der User ändert erneut | Eine erneute Änderung erzeugt einen **neuen Kandidaten** und überschreibt die Baseline nicht. Baseline und Kandidat sind unveränderliche Referenzen; nur so ist der Vergleich nach Reconnect und UI-Neustart rekonstruierbar. | **NEU** `sonde013_experiment.rs`, Fall `second_change_creates_new_candidate_not_a_new_baseline`. | Entwurf §43.1:2842-2844. **BELEGT, BAULÜCKE** |
| M-42 | `manual_external`; der Fremdzustand ist weder lesbar noch reproduzierbar | Statt State-Hashes speichert der Versuch eine Usernotiz, optional einen Werkzeugnamen und optional einen vom User bereitgestellten Preset- oder Screenshot-Hash. Er ist klar als `manuell · nicht automatisch wiederherstellbar` markiert, erlaubt **kein** Nakama-Revert und erhält eine niedrigere Reproduzierbarkeitsklasse. Die Mess- und Hörbewertung bleibt dennoch gültig, wenn Passage, Upstream-Fingerprint und Coverage passen. | **NEU** `sonde013_experiment.rs`, Fall `manual_external_has_no_state_hash_and_no_revert`; **NEU** derselbe Test, Fall `lower_reproducibility_class_is_explicit`. | Entwurf §43.1:2846-2856; §43.4:2812-2815. **BELEGT, BAULÜCKE** |
| M-43 | Baseline und Kandidat liegen vor; Lautheit wird abgeglichen | Der Match-Gain wird für die Passage **vorab** gemessen und für die Dauer des Versuchs eingefroren; er ist Teil der unveränderlichen Referenzen. Eine Klangwertung ohne vorherigen Lautheitsabgleich ist unzulässig. Keine laufende Nachregelung (siehe M-20). | **NEU** `sonde013_experiment.rs`, Fall `match_gain_is_frozen_in_the_immutable_reference`; **NEU** `EqCopSonde013ExperimentUiTest`, Fall `no_sound_verdict_before_loudness_match`. | Entwurf §43.1:2837; §15 Vergleichbarkeitsregeln:1096; §38.3:2523-2526. **BELEGT, BAULÜCKE** |
| M-44 | Vergleich steht; der User soll urteilen | Der optionale blind- beziehungsweise randomisierte A/B-Modus verringert Erwartungseffekte. **Identität und Reihenfolge werden erst nach dem Urteil aufgedeckt** und vorher im append-only Experimentereignis gebunden — das ist die Kante, die verhindert, dass die Reihenfolge nachträglich zum Urteil passend erzählt wird. Die UI hält sie bis dahin verborgen. | **NEU** `sonde013_experiment.rs`, Fall `blind_order_is_bound_before_the_verdict_and_revealed_after`; **NEU** `EqCopSonde013ExperimentUiTest`, Fall `ui_cannot_read_the_order_before_the_verdict`. | Entwurf §43.2:2872-2874; §43.4:2816-2817. **BELEGT, BAULÜCKE** |
| M-45 | Vergleich wird ausgewertet | Vier getrennte Achsen, nie eine Gesamtnote: **Zielmetrik** (hat sich der adressierte Befund verändert), **Guardrails** (wurden Loudness, Peak, Transient, Breite oder geschützte Bereiche schlechter), **Effektstabilität** (bleibt Richtung und Größenordnung über Teilfenster) und **Hörurteil** (ausdrücklich Userdaten, keine Messung). Metrikdeltas erhalten per Block-Bootstrap ein Unsicherheitsintervall; für 221 gleichzeitig gescannte Bänder werden zusammenhängende Cluster beziehungsweise FDR-Korrektur verwendet. | **NEU** `sonde013_experiment.rs`, Fall `four_axes_are_reported_separately`; **NEU** `EqCopSonde013ExperimentGoldenTest`, Fälle `block_bootstrap_interval` und `band_scan_uses_cluster_or_fdr`. | Entwurf §43.2:2867-2872. **BELEGT, BAULÜCKE** |
| M-46 | Auswertung steht; ein Urteil wird formuliert | Genau fünf zulässige Aussagen: Ziel verbessert und Guardrails stabil; messbar anders, musikalisches Urteil offen; keine belastbare Änderung; Ziel verbessert, aber geschützte Eigenschaft verschlechtert; Vergleich nicht gültig. **Nie** „objektiv besser" allein aus einem Metrikdelta. `Behalten` ist nur möglich, wenn der User es bestätigt. Ein nicht vergleichbares Experiment erhält kein starkes Siegerurteil (harter Gate 6 aus §49.2). | **NEU** `sonde013_experiment.rs`, Fall `verdict_vocabulary_is_closed_and_keep_needs_user`; **NEU** Gate-Bein `sonde013_experiment.rs`, Fall `gate6_incomparable_never_gets_a_strong_winner` mit adversarialen Gegenbeispielen. | Entwurf §43.3:2876-2888; §49.2 Gate 6:3560. **BELEGT, BAULÜCKE** |
| M-47 | Versuch läuft; der User misst ihn nie nach | `experiment_abort` schließt ihn mit einem terminalen `aborted`-Ereignis im append-only Store ab. Das ist ausdrücklich der **häufigste Realfall** und Teil des Vertrags, nicht ein Fehlerpfad. Auslöser eines terminalen `aborted` sind **genau zwei**: expliziter Userabbruch (`experiment_abort`) und Verdrängung durch einen Bestandsdeckel (§7.1 E-03, Grund `verdraengt`, siehe M-48). **Sitzungsende, Projektwechsel, Reconnect, UI-Neustart und Brokerneustart brechen nicht ab** — ein offenes Experiment überdauert sie und bleibt danach rekonstruierbar (§43.1, siehe M-50). | **NEU** `sonde013_experiment.rs`, Fall `abort_writes_terminal_event_for_each_trigger` mit genau diesen zwei Auslöserklassen **und** der Gegenprobe, dass Sitzungsende und Neustart **kein** Ereignis schreiben; **NEU** **A5**-Fall `experiment_abort_belegt_von_sonde013`. | Entwurf §43.1:2839-2845; §7.1 E-03; `reservierte-nachrichten-v1.json` (Eigentümer SONDE-013). **BELEGT, BAULÜCKE** — Auslöserliste korrigiert in Nacharbeit 1 (§8, D4) |
| M-48 | Versuch hat kein Terminalereignis | Er gilt als **offen** und fällt unter die Retention. Offene Zeilen sammeln sich nicht unbegrenzt an — **weder je Projektbindung noch projektübergreifend**. Zwei Deckel: `N_projekt` je Projektbindung (mindestens 2) und `N_global` über alle Projektbindungen (mindestens `N_projekt`); beide sind benannte Konstanten des Stores, kein Literal im Pfad, vom Bau begründet gewählt. Reihenfolge beim Anlegen: betroffenen Deckel prüfen → ältestes offenes Experiment **des jeweiligen Bereichs** mit terminalem `aborted` und Grund `verdraengt` schließen → neues anlegen. Der append-only Store verliert dabei nichts; keine alte Zeile wird geändert. Terminale und benannte Experimente bleiben, bis der User sie löscht (Roadmap 15). Keine Wanduhrfrist und kein Sitzungsende-Abbruch (M-47). | **NEU** `sonde013_experiment.rs`, Fälle `open_cap_per_project_at_n_and_n_plus_one` und `open_cap_global_at_n_and_n_plus_one` (Grenztests an `N` und `N+1`, Muster C-09). | Entwurf §43.1:2842-2845 (Retention aus Roadmap 15); `broker/src/store/migration.rs`:131-135; §7.1 E-03. **BELEGT, ENTSCHIEDEN** — §7.1 E-03, um den globalen Deckel ergänzt in Nacharbeit 1 (§8, D5) |
| M-49 | `manual_external`-Versuch ist ausgewertet; das Ergebnis soll persistent werden | Das Ergebnis reist als **neue Wirefamilie** `experiment_manual_result` im Eigentum von SONDE-013 von Gen zum Broker. Sie trägt `experiment_id`, Zieladresse, `base_revision`, `ttl_ms` und Schema-/Capability-Version wie jede steuernde Nachricht (M-66), dazu Hörurteil, aufgedeckte Blindreihenfolge, Usernotiz und optionalen Werkzeugnamen; **keine** State-Hashes, **keine** Candidate-Revision, **keinen** Match-Gain. Zielmetrik, Guardrail-Deltas und Effektstabilität rechnet der **Broker** aus Baseline- und Resultatevidenz im Store und schreibt das terminale `result`-Ereignis mit `execution_mode = manual_external` append-only in `experiment_events`; der Rückweg zu Gen läuft über den bestehenden Outbox-/Snapshot-Pfad, nicht über eine weitere Familie. Die Familie `experiment_result` **bleibt vollständig reserviert bei SONDE-017** — SONDE-013 belegt weder einen fremden Namen noch einen Teilzweig davon (M-73). | **NEU** **A5**-Fälle `experiment_manual_result_definiert`, `experiment_result_bleibt_reserviert_fuer_sonde017` und `summe_ist_28`; **NEU** **B10** `EqCopIpcTest` für die C++-Hälfte; **NEU** `sonde013_experiment.rs`, Fall `manual_result_writes_terminal_event_and_deltas`. | `reservierte-nachrichten-v1.json`:156-158 (Eigentümer von `experiment_result`); `tools/eq-copilot/pruefe_v3_vertrag.py`:1063-1079 (Namens- und Summenprüfung kennt nur ganze Familien); Entwurf §43.1:2839-2845; §58 („Blindurteil und Guardrail-Deltas"). **BELEGT, ENTSCHIEDEN** — §7.1 E-02, ersetzt in Nacharbeit 1 (§8, D1) |
| M-50 | Broker oder Main startet neu; ein Versuch war offen | Baseline, Kandidat und Match-Gain bleiben nach Broker- und Main-Neustart reproduzierbar; ein manueller Versuch weist seine schwächere Reproduzierbarkeit dabei ehrlich aus. Reihenfolge: Store lesen → unveränderliche Referenzen wiederherstellen → offenen Zustand anzeigen → **keine** stillschweigende Fortsetzung einer Messung über den Neustart hinweg. | Bestehend **A4-SI** `store_crash_matrix` für die Rekonstruktionsform; **NEU** `sonde013_experiment.rs`, Fall `experiment_survives_restart_without_silent_continuation`. | Entwurf §43.1:2842-2844; §43.4:2814-2815. **BELEGT, BAULÜCKE** |
| M-51 | Versuch soll exportiert oder gelöscht werden | Ein Experiment kann **vollständig inklusive Evidenz-IDs** exportiert und **ohne PCM** gelöscht werden. Löschen ist der Gegenpfad zum Anlegen und gehört in denselben Änderungssatz. | **NEU** `sonde013_experiment.rs`, Fall `export_is_complete_and_delete_leaves_no_pcm`. | Entwurf §43.4:2818; CLAUDE.md Änderungssatzregel. **BELEGT, BAULÜCKE** |

### 3.6 Invalidierung

| ID | Ausgangszustand × Ereignis | Zusage samt Reihenfolge und Frist | Test | Quelle / Belegstatus |
|---|---|---|---|---|
| M-52 | Evidenz ist eingegangen; ein Hörmarker oder eine Preview überlappt sie | Der Broker quarantänisiert den überlappenden Routing- und Timelinebereich; bei **unbekanntem Routing die ganze Sitzung**. Bereits eingegangene Evidenz wird per ID oder Bereich invalidiert. Reihenfolge: Interventionsereignis annehmen → Bereich bestimmen → `evidence_invalidate` mit Grund `intervention` und passendem `invalidate_umfang` senden → Evidenz als ausgeschlossen führen. Das per Frame übertragene Flag bleibt ein **zusätzlicher lokaler Beleg, nie die alleinige Garantie**. | **NEU** `sonde013_taint.rs`, Fall `marker_invalidates_overlapping_range_and_unknown_routing_takes_the_session`; bestehend Schemaform über **A5**. | Entwurf §34.2:2196-2211; `eq-ipc-v3.schema.json`:1085-1097. **BELEGT im Schema, BAULÜCKE im Sender** |
| M-53 | Evidenzfenster läuft; Seek, Loop-Wrap, Drop oder Kontinuitätsbruch tritt ein | Der Bruch beendet die alte Zeitreihe. Kein Fenster wird über die Grenze interpoliert; ein Drop erzeugt Zähler, sichtbare Lücke und eine neue Kontinuitätsgrenze. Grund auf dem Draht ist `epochwechsel`. Bis zur neuen gültigen Baseline gibt es keinen alten Wert und keine 0. **Passagenbindung (Nacharbeit 1):** Eine Passage bindet an genau eine Transportepoche (§32.4). Ein Seek, ein Loop-Wrap **innerhalb** der Passage (Loop kürzer als die Passage) oder ein Drop während der laufenden Passagenmessung invalidiert diese Messung — Grund `epochwechsel` beziehungsweise `sequenzluecke` — und die Messung beginnt mit der nächsten gültigen Epoche neu; kein Frame, der eine Grenze überspannt, geht in eine Paarauswertung ein (B5-Garantie aus SONDE-009). Ein Loop-Wrap **an** der Passagengrenze (Loop deckt die Passage vollständig) beendet einen Durchlauf und beginnt den nächsten — das ist **keine** Invalidierung und **keine** Herabstufung (Abgrenzung zu M-23). | Bestehend **B5** für die lokale Fenstertrennung und **B4** für den Drop-Pfad; **NEU** `sonde013_taint.rs`, Fall `epoch_change_invalidates_open_evidence`, dazu die zwei Passagenfälle aus M-23. | Entwurf §48.1:3413-3417; §34.2; §32.4:1969; §58:4285; `docs/beweise/SONDE-009.md`. **BELEGT, BAULÜCKE in der Invalidierungsnachricht** — Passagenbindung aus Nacharbeit 1 (§8, D3) |
| M-54 | Passage ist gebunden; das musikalische Material ändert sich | Geändertes Material invalidiert die abhängige Evidenz. Erkannt wird es über den Fingerprintvergleich (M-31), nicht über eine Zeitheuristik. Die heutige Gründemenge kennt den Fall nicht; sie wird um `material_wechsel` und `messpunkt_wechsel` (M-55) erweitert. **Kein Steuerkopf und kein zweites Versionsfeld im Payload:** `evidence_invalidate` trägt nur `type`, `grund` und `umfang` (`eq-ipc-v3.schema.json`:1085-1094), und die Fassungswahl läuft über `message_family` und `schema_minor` des Wire-Envelopes. Beide Gründe kommen deshalb mit **Fassung 2** des `wire_envelope_schema_minor` (§3.8, §7.1 E-04). Ein Leser der Fassung 1 lehnt sie ab, statt sie still auf einen bekannten Grund abzubilden. | **NEU** `sonde013_taint.rs`, Fall `material_change_invalidates_dependent_evidence`. | Entwurf §58 Lieferumfang („geändertem Material"); `eq-ipc-v3.schema.json`:1085-1094 (kein Steuerkopf); `reservierte-nachrichten-v1.json`:7-11 (`auswahlregel`). **BELEGT, ENTSCHIEDEN** — §7.1 E-04, ersetzt in Nacharbeit 1 (§8, D2) |
| M-55 | Evidenz hängt an einem Messpunkt; der Messpunkt ändert sich | Ein geänderter Messpunkt oder eine geänderte Messpunktklasse invalidiert die abhängige Evidenz. Das ist die Kante zu Gate 7 (§49.2): eine Standard-Insertprobe wird nie als exakter Mastersummenbeitrag bezeichnet — folglich darf Evidenz von `insert` nach einem Wechsel auf `post_fader_contribution` nicht mitwandern. | Bestehend **A16** `EqCopProbeeqNullTest` und **B2** für die Klassenmatrix; **NEU** `sonde013_taint.rs`, Fall `measurement_position_change_invalidates_dependent_evidence`. | Entwurf §58; §49.2 Gate 7:3561; `eq-ipc-v3.schema.json`:573-576. **BELEGT, BAULÜCKE** |
| M-56 | Ein `apply_transaction` bestätigt einen neuen DSP-Zustand | Ein **angewendeter** DSP-Zustand ist ausdrücklich **kein dauerhafter Taint**. Sein Apply schließt die alte Baseline, segmentiert die Timeline nach `state_revision`, macht abhängige Findings stale und erlaubt nach definierter Warm-up-Zeit eine neue gültige `post_committed`-Baseline dieser Revision. Grund auf dem Draht ist `state_revision`. | **NEU** `sonde013_taint.rs`, Fall `applied_state_segments_instead_of_tainting`. | Entwurf §34.2:2199-2205; `eq-ipc-v3.schema.json`:1091-1093. **BELEGT**; **BAULÜCKE nur im Bein** — der Apply-Pfad selbst gehört zu P6/P7, hier gilt nur die Klassifikationsregel |
| M-57 | Invalidierung wird gesendet | `evidence_invalidate` trägt genau einen Grund aus der geschlossenen Menge und genau einen über `art` diskriminierten Umfang: `evidence_ids`, Samplebereich oder ganze Sitzung. Ein `art=sample_range` ohne Bereich und ein `art=ganze_sitzung` **mit** Bereich sind ungültig — der Discriminator ist kein Etikett ohne Wirkung. | Bestehend **A5** und **B3c** für die Schemaform; **NEU** `sonde013_taint.rs`, Fall `invalidate_scope_discriminator_rejects_mismatched_payloads`. | `eq-ipc-v3.schema.json`:1085-1104. **BELEGT, BAULÜCKE im Sender** |

### 3.7 Taint und die inversen Taintpfade

| ID | Ausgangszustand × Ereignis | Zusage samt Reihenfolge und Frist | Test | Quelle / Belegstatus |
|---|---|---|---|---|
| M-58 | Ein oder mehrere Taintintervalle sind offen; der Hörmarker endet | Das Markerende **schließt** sein Taintintervall. Reihenfolge: Ausfade abgeschlossen (M-34) → `audible_intervention_end` mit konservativem `tail_samples` → Coordinator entfernt die `intervention_id` und zieht `tail_samples_offen` hoch → nach Ablauf des Nachlaufs ist der reguläre Pfad wieder frei. Das Ende allein genügt nicht: der Nachlauf muss abgelaufen sein. | Bestehend `broker/src/coordinator/intervention.rs`:66-92,128-137 über **A4**; **NEU** `sonde013_taint.rs`, Fall `marker_end_closes_its_interval_only_after_the_tail`. | Entwurf §58 Exit-Gate:4288-4290; `intervention.rs`:86-92. **BELEGT im Broker, BAULÜCKE im Producer** |
| M-59 | Ein Experiment ist aktiv (`art=experiment`); es endet mit `result` oder `abort` | Beide Terminalereignisse schließen alle zugehörigen Taintintervalle. Ein Versuch, der ohne Terminalereignis endet, lässt sie **nicht** offen zurück: die Retention (M-48) schreibt das terminale `aborted` und schließt sie damit. | **NEU** `sonde013_taint.rs`, Fall `both_experiment_terminals_close_all_intervals`; **NEU** derselbe Test, Fall `retention_abort_also_closes_intervals`. | Entwurf §58 Exit-Gate:4288-4290; §43.1:2842-2846; `eq-ipc-v3.schema.json`:1062 (`art: experiment`). **BELEGT, BAULÜCKE** |
| M-60 | Alle Intervalle sind geschlossen und der Nachlauf ist abgelaufen | **Der reguläre Pfad nullt wieder.** Messbar heißt das: `interventionssicht().starke_evidenz_erlaubt` ist `true`, `aktive` ist 0, `tail_samples_offen` ist 0 und `unknown` ist `false`; der nächste `evidence_dispatch()` wird angenommen statt gesperrt. Das ist die Zeile, die der Gate-Text „inverse Taintpfade grün" wörtlich verlangt. | Bestehend `broker/src/coordinator/intervention.rs`:154-182 über **A4**; **NEU** `sonde013_taint.rs`, Fall `inverse_path_returns_to_zero_after_every_intervention_kind` über alle vier `art`-Werte. | Entwurf §58:4289-4290; Tickettabelle:4474; `intervention.rs`:154-182. **BELEGT im Broker, BAULÜCKE für die Experimentart** |
| M-61 | Sticky `intervention_state_unknown` steht | Nur `neutral_resync` löst es. Er setzt die bestätigte Sequenzbasis für den Link, leert **aktive IDs und Nachlauf gemeinsam** und ist eine ausdrückliche Coordinator-Lebenszyklusoperation, keine neue Wirefamilie und kein normales End. Ein `end` ohne bekanntes Begin löst es nicht. | Bestehend `broker/src/coordinator/intervention.rs`:80-92,139-152 über **A4**; **NEU** `sonde013_taint.rs`, Fall `only_neutral_resync_clears_sticky_unknown` mit einem Gegenbeispiel je Nicht-Weg. | `intervention.rs`:80-92,139-152; Entwurf §34.2:2212-2220. **BELEGT**, heute erfüllt; **BAULÜCKE nur im Bein** |
| M-62 | Eine nachgelagerte Sonde hört die Intervention einer vorgelagerten | Die Taint-Logik ist **sitzungsweit**, nicht instanzlokal. Preview-, Focus-, Delta- und Markermatrix liegen hinter den Analysetaps der eigenen Instanz — das schützt die eigene Messung, nicht die der nachgelagerten. Deshalb greift zusätzlich die Sitzungsquarantäne aus §34. Bei unbekanntem Routing ist der Umfang die ganze Sitzung. | **NEU** `sonde013_taint.rs`, Fall `downstream_probe_is_tainted_by_upstream_intervention`; **NEU** derselbe Test, Fall `unknown_routing_widens_to_whole_session`. | Entwurf §44.2:3000-3007; §34.2:2207-2210. **BELEGT, BAULÜCKE** |
| M-63 | Taint steht; eine starke Evidenzaussage wird angefragt | Der Riegel liegt **vor** dem Evidence-Commit und zählt sein Urteil unter demselben Lock — so kann ein Begin nicht erst nach dem Dispatch wirken. Gesperrt heißt: keine starke Aussage, nicht „leiser Wert". Der Zähler `evidence_gesperrt` wird ausgewertet, nicht ignoriert. | Bestehend `broker/src/coordinator/intervention.rs`:171-182 über **A4**; **NEU** `sonde013_taint.rs`, Fall `dispatch_lock_ordering_holds_under_concurrent_begin`. | `intervention.rs`:169-182; Prüfliste §A („Zähler werden ausgewertet"). **BELEGT**, heute erfüllt; **BAULÜCKE nur im Bein** |

### 3.8 Vertragsfolgen

| ID | Ausgangszustand × Ereignis | Zusage samt Reihenfolge und Frist | Test | Quelle / Belegstatus |
|---|---|---|---|---|
| M-64 | `table Frame` soll `integration_samples` tragen | Neues Feld an der nächsten freien Feld-ID **14**, mit expliziter ID im `.fbs`. Reihenfolge im **einen** Änderungssatz: `.fbs` ändern → `flatc` neu generieren für C++ und Rust → `FELD-IDS.json` nachziehen → Binärfixtures neu erzeugen → beide Leser klassifizieren identisch → `schema_minor` des Wire-Envelopes anheben. Ein alter Leser darf am neuen Feld nicht brechen; ein neuer Leser darf es bei Abwesenheit nicht als 0 lesen. | Bestehend **A9** `pruefe_flatc_drift.py` (Drift 0, jedes Tabellenfeld mit expliziter ID), **A10** `erzeuge_fb_fixtures.py`, **B3c** `EqCopSchemaTest`; alle drei brauchen **NEU** je einen Fall für ID 14. | `.fbs`:216-239; `FELD-IDS.json`; `reservierte-nachrichten-v1.json` (`wire_envelope_schema_minor`). **BELEGT, BAULÜCKE** |
| M-65 | `evidence_snapshot` soll `ereignisse` tragen | Das Objekt ist `additionalProperties: false` und sein Feldname für SONDE-013 reserviert. Das Belegen ist deshalb **kein additiver Schritt**: es braucht einen `schema_minor`-Bump der Familie P1, einen Eintrag in `belegte_felder` von `reservierte-nachrichten-v1.json`, ein Negativfixture und beidseitige Leser. Ein alter Empfänger, der das Feld nicht kennt, lehnt es nach heutigem Schema ab — die Fassungswahl über `message_family` und `schema_minor` im Envelope ist der einzige zulässige Weg. | Bestehend **A5** `pruefe_v3_vertrag.py` (Summe der Familien, reservierte Namen ohne Nutzlast, Negativfixture je Definition), **A8** `erzeuge_v3_fixtures.py`, **B3c**; alle drei **NEU** um den Fall erweitert. | `eq-ipc-v3.schema.json`:1002-1019; `reservierte-nachrichten-v1.json`. **BELEGT, BAULÜCKE** |
| M-66 | Die Experimentfamilien dieses Tickets sollen definiert werden | **Drei** Familien: `experiment_begin` und `experiment_abort` wandern aus `reserviert` nach `definiert`; `experiment_manual_result` entsteht **neu und direkt in `definiert`** mit Eigentümer SONDE-013 (§7.1 E-02). Damit steigt `gesamt_erwartet` von **27 auf 28**, die Aufteilung geht von 18/9 auf **21/7**. `pruefe_v3_vertrag.py`:1073-1074 prüft `len(definiert) + len(reserviert) == gesamt_erwartet` gegen die Zahl im Register selbst — das Bein fällt, wenn eine der beiden Seiten nicht mitzieht. Alle drei Familien tragen wie jede steuernde Nachricht die Steuerfelder (`command_id` beziehungsweise `experiment_id`, Zieladresse, `base_revision`, begrenzte `ttl_ms`, Schema-/Capability-Version); die Deadline leitet der Empfänger aus seiner **eigenen** monotonen Uhr ab. Der Versionsschritt ist **Fassung 2** des `wire_envelope_schema_minor`, deren Eintrag alle drei Familien nennt. | Bestehend **A5**; **NEU** **A5**-Fälle `zwei_familien_wandern_von_reserviert_nach_definiert`, `experiment_manual_result_definiert` und `summe_ist_28`; **NEU** **B10** `EqCopIpcTest`-Fall für die C++-Hälfte. | `reservierte-nachrichten-v1.json`; `tools/eq-copilot/pruefe_v3_vertrag.py`:1063-1079; Entwurf §33.3:2051-2090. **BELEGT, BAULÜCKE** — dritte Familie, Summe 28 und Aufteilung 21/7 aus Nacharbeit 1 (§8, D1) |
| M-67 | Ein neuer Vertragsteil trägt Discriminator, Zieladresse, Revision oder Capability | **Nicht additiv erweiterbar.** Ein neuer Discriminatorzweig ist ein Major-Schritt, kein Minor. Konkret betroffen in diesem Ticket: der Vergleichbarkeits-Umfang, der Alignment-Score (vier Klassen, M-16), die Verdikt-Menge (fünf Aussagen, M-46), der `execution_mode` und die `invalidate_umfang`-Arten. Jede dieser Mengen ist beim ersten Bau **abschließend** zu wählen. | Bestehend **A5** und **B3c** für die heutige Diskriminatorprüfung; **NEU** je ein Negativfixture pro neuem Discriminator. | `eq-copilot/schemas/v3/README.md`; CLAUDE.md („Für v3 sind Discriminator, Zieladresse, Revision und Capability nicht additiv erweiterbar"); SONDE-012 E-C01. **BELEGT, BAULÜCKE** |
| M-68 | Passagen und Experimente sollen persistent werden | Die Tabellen existieren bereits aus Migration 1 (`passages`, `experiments`, `experiment_events`, `user_verdicts`). Braucht dieses Ticket eine Spalte oder einen Index mehr, ist das **Migration 2** mit Vor- **und** Rückweg im selben Änderungssatz; `migration_1_checksum()` verriegelt den Text der ersten Migration, ein nachträglicher Eingriff dort fällt auf. Ein alter Datenbankeintrag darf nie einen neueren Plugin-State zurücksetzen. | Bestehend **A4** und **A4-SI** für Migration 1 und die Killmatrix; **NEU** `sonde013_store.rs`, Fall `migration_2_has_forward_and_backward_path`, falls Migration 2 nötig wird. | `broker/src/store/migration.rs`:68-135,150-155; Entwurf §33.5:2155-2160. **BELEGT**; **BAULÜCKE nur im Bein**, falls Migration 2 nötig wird — das entscheidet der Bau in Etappe 2 |
| M-69 | Eine manuelle Passage soll mit dem FL-Projekt reisen | `MainProjectState` ist autoritativ für manuelle Passagen (§33.5). Damit gilt die State-Invariante: neues persistentes Feld zuerst versionieren, Altstände laden, unbekannte Felder zerstören keinen alten Consumer, Save und Load **gemeinsam** testen, und jede persistente Änderung meldet dem Host Dirty-State. Unabhängige Teilversionen für Main-Project-State und Probe-DSP bleiben getrennt. | Bestehend **B2** `EqCopStateMigrationTest` und **A12** `erzeuge_state_fixtures.py`; **NEU** `EqCopSonde013PassageStateTest`, Fälle `save_load_roundtrip_is_byte_identical` und `unknown_newer_field_stays_read_only_with_original_bytes`. | Entwurf §33.5:2135-2142,2155-2160; CLAUDE.md State-Invariante. **BELEGT, BAULÜCKE** |
| M-70 | Ein Vertrag ändert sich in einer Sprache | Writer, Reader, Migration, Fixtures und **beide** Cross-Language-Verbraucher liegen im selben Änderungssatz. Die vier Beine, die das messen: **A5** (JSON-Vertrag samt Abdeckung), **A8** (JSON-Fixturekorpus bytegleich), **A10** (Binärfixturekorpus bytegleich), **B3c** (C++ klassifiziert wie das Manifest, Urteil **und** Verletzungsmenge). Ein Fixture ohne Verbraucher in beiden Sprachen ist ein Befund. | Bestehend **A5**, **A8**, **A10**, **B3c**; sie sind der Riegel selbst, nicht nur der Test. | Prüfliste §F; Entwurf §33.1; SONDE-005-Gate. **BELEGT** — heute erfüllt, muss erfüllt bleiben |
| M-71 | Generierter Code weicht vom `.fbs` ab | Codegen-Drift ist 0: die Neugenerierung aus dem `.fbs` ist bytegleich zum committeten C++- und Rust-Code, `flatc`, C++-Header und Rust-Crate tragen dieselbe gepinnte Version, und jedes Tabellenfeld trägt eine explizite Feld-ID. Ein neues Feld ohne ID fällt hier. | Bestehend **A9** `pruefe_flatc_drift.py`. | `tools/beweise.ps1`:497. **BELEGT** — heute erfüllt, muss erfüllt bleiben |
| M-72 | Eine Zusage dieses Tickets ändert sich | Drei Stellen werden nachgezogen: Runnerkopf `tools/beweise.ps1`, Skriptkopf des betroffenen Werkzeugs und der lebende Kopf dieses Manifests. Ein `git grep` nach dem Kernbegriff genügt als Nachweis. Eine Runner-Behauptung sagt nicht mehr, als der Test misst, und nicht mehr, als das Gate verlangt. | Bestehend: der Kanonlauf selbst verweigert die Beglaubigung, wenn Prüfbinaries älter als ihre Quellen sind. | Prüfliste §E; `tools/beweise.ps1`. **BELEGT** — Verfahrensregel für Etappe 2 |
| M-73 | Ein reservierter Name soll belegt werden | Der Vertragsanteil, der bricht, wenn man ihn offen lässt, ist der **Name**, nicht die Nutzlast. Belegte Namen wandern mit Eigentümer und Grund in `belegte_nachrichten` beziehungsweise `belegte_felder`; ein reservierter Discriminator wird bis dahin vom v3-Parser abgelehnt. Kein Ticket belegt einen Namen, dessen Eigentümer ein anderes Ticket ist — das betrifft hier `experiment_result` (SONDE-017) und `user_verdict` (SONDE-014). Der Vertrag kennt zudem nur **ganze** reservierte Familien, keine teilreservierten Discriminatorzweige (`pruefe_v3_vertrag.py`:1063-1079): ein halb belegter fremder Name ist kein gültiger Zwischenzustand. | Bestehend **A5** (reservierte Namen bleiben ohne Nutzlast, Summe stimmt); **NEU** **A5**-Fall `fremde_eigentuemer_bleiben_unberuehrt`. | `reservierte-nachrichten-v1.json`; `tools/eq-copilot/pruefe_v3_vertrag.py`:1063-1079; Bauaufteilung §6.2 („Kein Vorgriff"). **BELEGT, BAULÜCKE** — in Nacharbeit 1 bestätigt und um die Teilzweigregel geschärft (§8, D1) |

**Ein einziger Fassungsschritt für alle P1-Vertragsänderungen dieses
Tickets** (Nacharbeit 1, §8, D1 und D2). Der `wire_envelope_schema_minor`
im Register steht heute auf `aktuell: 1` und gehört der Familie **P1**
(`reservierte-nachrichten-v1.json`:7-11). SONDE-013 hebt ihn **genau einmal**
auf **2**; der Eintrag `fassungen."2"` nennt alles, was diese Fassung
ausmacht:

- die drei neu definierten Familien `experiment_begin`, `experiment_abort`
  und `experiment_manual_result` (M-66);
- das belegte Feld `evidence_snapshot.ereignisse` (M-65) samt den
  bandweisen Kohärenzfeldern aus M-11 (Phase, Fensterdauer, Freiheitsgrade);
- die zwei neuen Invalidierungsgründe, etwa als
  `evidence_invalidate_grund_erweitert: ["material_wechsel",
  "messpunkt_wechsel"]` (M-54, M-55).

Ein Leser der Fassung 1 lehnt jede dieser Neuerungen ab; das ist je Neuerung
ein Negativfixture (A5, A8) und ein B3c-Zwilling. Die Fassungswahl läuft
ausschließlich über `message_family` und `schema_minor` des Wire-Envelopes —
im Payload entsteht **kein** weiteres Versionsfeld und **kein** Steuerkopf
(`auswahlregel` im Register; `evidence_invalidate` trägt nur `type`, `grund`
und `umfang`, `eq-ipc-v3.schema.json`:1085-1094). **Nicht** Teil der
Fassung 2 ist `integration_samples` (M-64): das ist die FlatBuffers-Fläche
P2, deren `schema_major`/`schema_minor` bewusst nur im 16-Byte-Envelope
leben (`nakama_telemetry_v1.fbs`:28-33) und nicht im P1-Register geführt
werden. A9 (Codegen-Drift 0) ist von der Fassung 2 nur berührt, wenn ein
`.fbs` angefasst wird — für D1 und D2 also nicht.

### 3.9 Audio-Thread-Invarianten

| ID | Ausgangszustand × Ereignis | Zusage samt Reihenfolge und Frist | Test | Quelle / Belegstatus |
|---|---|---|---|---|
| M-74 | Beliebige neue Metrik oder Ereignisquelle dieses Tickets läuft im Audiocallback | Keine Heapallokation, kein Mutex, keine Datei-, Pipe-, Log-, UI- oder Modellarbeit. Audio zu Worker ausschließlich über vorallokierte SPSC-Strukturen und atomare Generationen. Feste Obergrenzen für Sonden, Bänder, Ereignisse, Queue-Tiefe, JSON-Größe und Historie — der Ereignisring bleibt bei 64 Plätzen. Denormal-Schutz, NaN-/Inf-Sanitisierung und definierter Mono-/Stereo-/Sidechain-Fallback. | Bestehend **B4** `EqCopQueueStressTest` (0 Allokationen über 4000 Blöcke wechselnder Größe mit Transportkanten) und **B12**; **NEU** derselbe Allokationszähler über die neuen Metrik- und Ringpfade. | Entwurf §48.1:3401-3412; CLAUDE.md Grundgesetz; `FeatureEngine.h`:448. **BELEGT, BAULÜCKE für neue Pfade** |
| M-75 | Analysestau tritt ein | Der ganze **Analyse**block wird verworfen, der Audioblock immer weiterverarbeitet. Degradationsreihenfolge: Fokus-Burst → 221-Band-Snapshot → 64-Band-Liveframe; P0-Steuerung und Audio bleiben erhalten. Ein Drop erzeugt Zähler, sichtbare Lücke und neue Kontinuitätsgrenze und wird **nie** interpoliert. | Bestehend **B4** (Überlauf verwirft den ganzen Block, nie eine Teilmenge; Oversize fällt für die Analyse und lässt Audio unberührt) und **B12**; **NEU** `EqCopSonde013DynamicsTest`, Fall `overload_drops_analysis_never_audio`. | Entwurf §48.1:3403-3412,3424-3428. **BELEGT, BAULÜCKE für neue Pfade** |
| M-76 | Passiver Zustand; Nulltest läuft | Passthrough bleibt **bitidentisch**, 0 Samples Latenz, kein Tail, kein Hostparameter — für Gen und für Probeeq. Kein Teil dieses Tickets ändert daran etwas: Metriken sind Abgriffe, keine Prozessoren. NaN und Inf werden gezählt, aber nicht verändert. | Bestehend **A1** `EqCopNullTest` und **A16** `EqCopProbeeqNullTest`; sie sind die Regressionswache dieses Tickets. | Entwurf §49.2 Gate 1:3550-3552; CLAUDE.md Grundgesetz; `tools/beweise.ps1`:433-440,592. **BELEGT** — heute erfüllt, muss erfüllt bleiben |
| M-77 | Hostblock übersteigt die vorallokierte Slotkapazität | Er wird **vollständig nur für die Analyse** verworfen, erhöht `oversize_drop`, schließt alle offenen Analysefenster und startet ein neues `continuity_segment`. Audio läuft weiter. `maximumExpectedSamplesPerBlock` ist bei JUCE nur ein Hinweis; das gilt auch für jedes neue Fenster dieses Tickets. | Bestehend **B4**; **NEU** `EqCopSonde013DynamicsTest`, Fall `oversize_closes_new_windows_too`. | Entwurf §48.1:3419-3428. **BELEGT, BAULÜCKE für neue Fenster** |
| M-78 | Telemetriepfad staut zurück | P2 darf `drop-oldest` verwenden; sein Ausfall degradiert Analyse, nicht Control. Höchstens ein aktueller Liveframe je Probe; alte Frames werden verworfen. **P0 wartet nie hinter alten Spektren.** Der P0-Interventionsring (M-39) ist davon ausdrücklich ausgenommen und verwendet kein `drop-oldest`. | Bestehend **A22** `pruefe_ipc_last.py` (32 Sondenpaare fluten P2, kein P0-Frame geht verloren) und **A24** `pruefe_session_soak.py`; **NEU** derselbe Lasttest mit aktivem Interventionsring. | Entwurf §33.1:2030-2036; §49.3:3581; `intervention.rs`. **BELEGT, BAULÜCKE für den Ring unter Last** |

### 3.10 P4-Referenzkorpus und Exit-Gate

| ID | Ausgangszustand × Ereignis | Zusage samt Reihenfolge und Frist | Test | Quelle / Belegstatus |
|---|---|---|---|---|
| M-79 | Exit-Gate wird gefahren | Der Korpus besteht aus vier Klassen: **Referenzkorpus** (Loudness und True Peak gegen validierte Referenz, ±0,1 LU beziehungsweise ±0,1 dB), **Loop-/Seek-/PDC-Goldens**, **adversariale Vergleichsfixtures** und **Stereo-Goldens**. Alle vier bestehen, sonst ist das Gate rot. Neben Precision und Recall zählen Kalibrierung, Brier Score, Coverage, Enthaltungsrate und die Zahl **falscher starker Behauptungen**. Ein konservatives `unsicher` ist besser als eine überzeugende falsche Ursache. | **NEU** Sammelbein `pruefe_p4_korpus.py` mit einem Unterbericht je Klasse; bestehend **B9** und **A2** `EqCopGoldenTest` als Vorstufe. | Entwurf §58 Exit-Gate:4288-4290; §49.4:3591-3600. **BELEGT, BAULÜCKE** |
| M-80 | Korpus soll reproduzierbar sein | Ein Erzeuger-Skript baut jede Fixture **bytegleich** neu; ein `--pruefen`-Lauf vergleicht den committeten Bestand gegen die Neuerzeugung samt `MANIFEST` und meldet jede verwaiste Datei. Das ist dasselbe Muster wie **A6**, **A7**, **A8**, **A10**, **A12** und **A20**. Bytegleich geprüfte Fixtures brauchen passende `.gitattributes`-Regeln, sonst verdirbt die Zeilenendenormalisierung den Vergleich. | **NEU** `erzeuge_p4_korpus.py` mit `--pruefen` als Kanonbein; **NEU** `.gitattributes`-Eintrag für die neuen Binärfixtures im selben Änderungssatz. | `tools/beweise.ps1`:488-490,512,572; CLAUDE.md Maschinen-Landminen. **BELEGT, BAULÜCKE** |
| M-81 | Loudness- und True-Peak-Referenz wird geprüft | Toleranz ±0,1 LU und ±0,1 dB gegen eine validierte Referenz auf Standard-Testmaterial. Der 8-fach-True-Peak-Pfad besteht das offizielle EBU-Testset bei 48 kHz **plus** äquivalente generierte Mehrsampleraten-Goldens — oder er wird durch den BS.1770-Polyphase-Referenzpfad ersetzt (M-02). LRA wird bei zu kurzem oder zu stillem Material nicht numerisch vorgetäuscht. | Bestehend **B9** für den Akkumulator; **NEU** `EqCopSonde013TruePeakGoldenTest` mit dem EBU-Testset. | Entwurf §39.3:2613-2621; §49.3:3584. **BELEGT, BAULÜCKE** |
| M-82 | Stereo-Goldens werden gefahren | Fünf Klassen werden über **alle** unterstützten Blockgrößen korrekt klassifiziert: Mono, identisches Stereo, Polaritätsinvertierung, bekannte Laufzeit und unkorrelierte Kanäle. Der angezeigte Mono-Verlust entspricht dem wirklich gefalteten Audiopuffer innerhalb 0,25 dB. | **NEU** `EqCopSonde013StereoGoldenTest` als Gate-Bein über den Blockgrößensweep. | Entwurf §40.3:2666-2671. **BELEGT, BAULÜCKE** |
| M-83 | PRE/POST-Goldens werden gefahren | Verteilte Feature-Goldens mit 0 bis 2 s bekannter Verzögerung finden den Lag innerhalb **eines Feature-Hops**. Der verteilte `q_db_0p1_i16`-Livepfad wird nicht gegen die 0,05-dB-Grenze des lokalen Audio-Compare geprüft; fokussierte 0,01-dB-Evidenz muss Gain innerhalb ±0,1 dB samt statistischem Intervall wiederfinden. Kompression, Modulation, Saturation und wechselnde Latenz erzeugen keine falsche statische EQ-Behauptung. | **NEU** `EqCopSonde013PrePostGoldenTest` als Gate-Bein mit dem Verzögerungssweep und den vier adversarialen Kettenklassen. | Entwurf §38.5:2551-2562. **BELEGT, BAULÜCKE** |
| M-84 | Loop-, Seek- und PDC-Goldens werden gefahren | Bekannte Impulsereignisse bleiben über Blockgrößen und Sampleraten **zeitlich stabil**. Jede Grenze — Drop, Seek laufend und gestoppt, Loop-Wrap, möglicher Straddle, Transportkante, Sampleratewechsel, Neuanlauf, Beweislagewechsel — trennt jedes offene Fenster, auch die neuen dieses Tickets. Kein Fenster überbrückt eine Grenze. | Bestehend **B5** `EqCopAnalysisGoldenTest` für die neun Grenzarten; **NEU** derselbe Sweep über die neuen Metrik-, Fingerprint- und Ereignisfenster. | Entwurf §39.3:2619; `docs/beweise/SONDE-009.md`; `tools/beweise.ps1`:623. **BELEGT, BAULÜCKE für neue Fenster** |
| M-85 | Adversariale Vergleichsfixtures werden gefahren | Ein **korrelierter, aber nicht kausaler Distraktor** wird als Alternative gezeigt und nicht als sicherer Auslöser bezeichnet. Ein nicht vergleichbares Experiment erhält kein starkes Siegerurteil. Eine Standard-Insertprobe wird nie als exakter Mastersummenbeitrag bezeichnet. Diese drei sind die Falsifikationsziele, die das Gate **G4** (`/c-review` DSP plus Codex) später gegen dieses Ticket fährt. | **NEU** `pruefe_p4_korpus.py`-Unterbericht `adversarial`, mit einem Fixture je Falsifikationsziel und der Enthaltungsrate als Messwert. | Entwurf §39.3:2621; §49.2 Gate 6 und 7:3560-3561; §49.4:3596-3600; `docs/bauaufteilung-sonden.md`:393 (Gate G4, Falsifikation Gate 5 und 6). **BELEGT, BAULÜCKE** |

### 3.11 Zählung

| Belegklasse | Zeilen | IDs |
|---|---:|---|
| **BELEGT**, heute vollständig gedeckt — kein neues Bein nötig | 4 | M-70, M-71, M-72, M-76 |
| **BELEGT**, **BAULÜCKE nur im Bein** — die Zusage gilt heute, ihre Messung fehlt | 7 | M-19, M-24, M-35, M-56, M-61, M-63, M-68 |
| **BELEGT**, **BAULÜCKE** in Implementierung und Messung | 71 | alle übrigen, einschließlich der in Nacharbeit 1 angelegten Zeile M-86 |
| **BELEGT**, **ENTSCHIEDEN** (§7.1) — Regel steht, Bau und Messung fehlen | 4 | M-36 (E-01), M-48 (E-03), M-49 (E-02), M-54 (E-04) |
| **ENTSCHEIDEN-VORSCHLAG** | 0 | — (alle vier in §7.1 entschieden, E-02/E-03/E-04 mit dem Wortlaut aus §8) |
| **OFFEN** | 0 | — |
| **gesamt** | **86** | M-01 bis M-86 |

Vier weitere Technikfragen ohne Quelle sind in §4 als **E-05** bis **E-08**
geführt; sie hängen nicht an einer einzelnen Matrixzeile, sondern am
Zuschnitt des Tickets, und werden deshalb dort und nicht in der Matrix
gestellt. Sie sind in §7.1 mitentschieden.

Nacharbeit 1 hat die Zählung an zwei Stellen verändert (§8): **M-86** ist
neu (D6, eigenständiger Peakpfad), und die vier vormaligen
ENTSCHEIDEN-VORSCHLAG-Zeilen tragen jetzt die Klasse **ENTSCHIEDEN**. Keine
Zeile ist entfallen; M-23 hat einen Herabstufungsgrund verloren (D3), aber
nicht ihre Zusage.

---

## 4. Offene Punkte

Regel: Eine Technikfrage ohne Quelle wird als **ENTSCHEIDEN-VORSCHLAG** mit
Begründung und Alternativen eingetragen; entschieden wird sie vom Dirigenten.
Eine **Produktfrage** wird nur benannt und einer Fläche zugeordnet, nie
beantwortet. Ein **Widerspruch** zwischen Quellen wird benannt, nicht kaschiert.

### 4.1 Technikfragen zur Entscheidung

#### E-01 — Ausfadevertrag auf dem Oversize-Pfad (Matrix M-36)

**Frage:** `HoerMarkierung.h`:280-282 ruft `hartAus()`, wenn der Hostblock
`wetKapazitaet` übersteigt. Der Block ist per Definition zu groß für die
vorallokierte Rampe. Wie sieht der NAK-47-Fix hier aus?

**Vorschlag:** Der Oversize-Fall wird **vorausschauend** behandelt statt
reaktiv. `prepareToPlay` kennt `maximumExpectedSamplesPerBlock` nur als Hinweis
(§48.1:3419-3421); trifft ein Block darüber ein, ist die Markierung in diesem
Block bereits still, weil der vorige Block sie beim Erkennen der Kapazitäts-
grenze ausgeblendet hat. Praktisch: der Marker meldet sich beim ersten
Oversize-Ereignis ab und bleibt bis zum nächsten `prepareToPlay` aus.

**Alternative A:** Rampe im Oversize-Block in festen internen Chunks fahren.
Kostet einen zweiten Pfad im Audiothread und damit eine zweite Stelle, an der
der Nullvertrag brechen kann.

**Alternative B:** `hartAus()` dort belassen und den Klick als Ausnahme
dokumentieren. Verletzt §49.3 („kein Klick") und lässt NAK-47 halb offen.

**Begründung der Empfehlung:** Alternative A verdoppelt den echtzeitkritischen
Pfad für einen Fall, der laut `oversize_drop`-Zähler ohnehin als Störung geführt
wird. Der Vorschlag hält den Audiothread einpfadig und macht das Verhalten
messbar.

#### E-02 — Weg des `manual_external`-Ergebnisses in den Store (Matrix M-49)

> **Ersetzt durch §7.1 nach Nacharbeit 1.** Der Vorschlag unten ist Verlauf.
> Bindend ist der Wortlaut in §7.1, E-02, in der Fassung aus §8, D1: neue
> Familie `experiment_manual_result`, `experiment_result` bleibt vollständig
> bei SONDE-017.

**Frage:** §58 verlangt Blindurteil und Guardrail-Deltas in P4. Die Wirefamilie
`experiment_result` gehört laut `reservierte-nachrichten-v1.json` aber
`SONDE-017`, Grund: „Deltas und Urteil setzen den Active-Compare-Pfad voraus."
Im `manual_external`-Fall gibt es keinen Active-Compare. Wie gelangt das
Ergebnis von Gen in den append-only Store?

**Vorschlag:** SONDE-013 belegt `experiment_result` **nicht**. Das Ergebnis
reist als Ereignis, das `experiment_abort` strukturell spiegelt — konkret als
zweite Familie im Eigentum von SONDE-013, deren Name jetzt zu wählen und im
Register zu verbrennen ist. Sie trägt Zielmetrik, Guardrail-Deltas,
Effektstabilität, Hörurteil und die aufgedeckte Blindreihenfolge; sie trägt
**keine** State-Hashes und keine Candidate-Revision, weil beide im manuellen
Modus nicht existieren.

**Alternative A:** `experiment_result` von SONDE-017 übernehmen und mit einem
`execution_mode`-Discriminator bauen, dessen `active_probe`-Zweig heute leer
bleibt. Verstößt gegen „Kein Vorgriff" (Bauaufteilung §6.2) und legt eine
Nutzlast fest, die SONDE-017 später tragen muss, ohne sie zu kennen.

**Alternative B:** Das Ergebnis nur lokal in `MainProjectState` halten und erst
in P7 in den Store spiegeln. Verstößt gegen §33.5, das Experimente ausdrücklich
dem SQLite-Store zuordnet und `MainProjectState` nur kompakte aktuelle IDs
zugesteht.

**Begründung der Empfehlung:** Der Vorschlag hält jedes Ticket bei seinem
Eigentum, verbrennt den neuen Namen sofort (das ist der Vertragsanteil, der
sonst bricht) und lässt SONDE-017 seinen Discriminator frei wählen.

#### E-03 — Retentionfrist für offene Experimente (Matrix M-48)

> **Ersetzt durch §7.1 nach Nacharbeit 1.** Der Vorschlag unten ist Verlauf.
> Bindend ist der Wortlaut in §7.1, E-03, in der Fassung aus §8, D4 und D5:
> zwei Bestandsdeckel (`N_projekt` und `N_global`), kein Sitzungsende-Abbruch.

**Frage:** §43.1 sagt, offene Experimente „sammeln sich nicht unbegrenzt an" und
fallen „unter die Retention aus Roadmap 15". Eine Frist nennt keine Quelle.

**Vorschlag:** Die Retention ist **nicht zeit-**, sondern **bestandsgebunden**:
je Projektbindung höchstens eine feste Zahl offener Experimente; beim
Überschreiten wird das älteste offene mit terminalem `aborted` geschlossen.
Zusätzlich schließt jedes Sitzungsende alle offenen Experimente dieser Sitzung.
Die Zahl lebt in der Konfiguration, nicht als Literal im Pfad.

**Alternative A:** Wanduhrfrist, etwa 24 Stunden. Braucht eine verlässliche
Wanduhr über Neustarts hinweg und macht das Verhalten von der Systemzeit
abhängig — dieselbe Klasse Problem, die §33.3 für Deadlines bereits ausschließt
(„Sender-Wandzeit darf nie einen Audio-Failsafe verlängern").

**Alternative B:** Keine automatische Retention, nur manuelles Löschen.
Widerspricht dem Wortlaut „sammeln sich nicht unbegrenzt an".

**Begründung der Empfehlung:** Ein Bestandsdeckel ist über Neustarts hinweg
deterministisch prüfbar und passt zu den festen Obergrenzen aus §48.1.

#### E-04 — Invalidierungsgrund für Materialwechsel (Matrix M-54)

> **Ersetzt durch §7.1 nach Nacharbeit 1.** Der Vorschlag unten ist Verlauf.
> Bindend ist der Wortlaut in §7.1, E-04, in der Fassung aus §8, D2: kein
> Steuerkopf, Fassung 2 des `wire_envelope_schema_minor`.

**Frage:** `evidence_invalidate.grund` ist eine geschlossene Menge aus fünf
Werten (`intervention`, `routing_unbekannt`, `sequenzluecke`, `epochwechsel`,
`state_revision`). §58 verlangt Invalidierung auch bei **geändertem Material**
und **geändertem Messpunkt**. Beide passen in keinen der fünf.

**Vorschlag:** Zwei Werte ergänzen — `material_wechsel` und
`messpunkt_wechsel`. Weil ein Discriminator laut CLAUDE.md **nicht additiv
erweiterbar** ist, ist das ein Major-Schritt am `grund`-Enum und gehört mit
Negativfixture, beidseitigen Lesern und `schema_minor`-Bump in denselben
Änderungssatz.

**Alternative A:** Beide unter `intervention` führen. Verfälscht die Semantik:
eine Materialänderung ist keine hörbare Intervention und darf keinen
Interventionszustand setzen.

**Alternative B:** Beide unter `epochwechsel` führen. Ein Materialwechsel im
laufenden Transport ist gerade **kein** Epochenwechsel; die Zuordnung würde die
Epochenlogik unbrauchbar machen.

**Begründung der Empfehlung:** Die Gründeliste ist maschinenlesbar und wird von
Empfängern zur Wiederherstellung genutzt. Ein falsch etikettierter Grund ist
schlimmer als ein neuer.

#### E-05 — Baut SONDE-013 den Evidenzpfad?

**Frage:** `evidence_snapshot` ist im Schema vollständig definiert
(`eq-ipc-v3.schema.json`:1002-1040, `verteilung` ist Pflicht), hat aber
**weder im Broker noch im Plugin einen Sender oder Empfänger** — an `ed9bbf7`
liefert `grep` auf den Namen in `broker/src` und `eq-copilot/plugin` keinen
einzigen Treffer außerhalb der Schemadatei. §33.2 ordnet die Ereignisse
ausdrücklich dem Evidenzsnapshot zu (1 bis 4 Hz), nicht dem Livepfad. Matrixzeile
M-05 setzt damit voraus, dass dieses Ticket den Evidenzpfad erst baut.

**Vorschlag:** Ja — der Evidenzpfad ist Teil von SONDE-013, aber als **eigene
Bauetappe vor** den Vergleichsfunktionen (§5, Etappe B). Ohne ihn hat weder der
Ereignisstrom noch die P10/P50/P95-Verteilung einen Weg, und die
PRE/POST-Auswertung (§38.3) verlangt genau diese Verteilungspunkte.

**Alternative A:** Die Ereignisse auf dem Livepfad (FlatBuffers, 10 Hz)
mitschicken. Widerspricht §33.2 und bläht den häufigen Frame um eine Liste
variabler Länge auf, deren Kadenz ausdrücklich niedriger sein soll.

**Alternative B:** Den Evidenzpfad als eigenes Vorticket vor SONDE-013 ziehen.
Ändert den Plan und verschiebt ein Gate; das ist eine Dirigentenentscheidung
über den Zuschnitt, keine Baufrage.

**Begründung der Empfehlung:** Der Pfad ist vertraglich fertig und hat keinen
offenen Entscheid mehr; ihn zu bauen ist Arbeit, kein Risiko. Ihn zu umgehen
verletzt eine Kadenzzusage.

#### E-06 — Ort der neuen Metriken: Sonde oder Broker

**Frage:** Momentary-400-ms, PLR, PSR über 3 s, mehrfenstriger Crest, bandweise
M/S und Kohärenz über acht Welch-Frames — entstehen sie in der Sonde (wie alle
heutigen Frameskalare) oder im Broker?

**Vorschlag:** In der **Sonde**, konsequent wie bisher. §35.1 sagt wörtlich:
„Jede Probe publiziert lokal berechnete Featureframes; der Broker führt sie nur
zusammen. Damit bleibt die Audioarbeit verteilt." Der Broker rechnet
ausschließlich das, was **mehrere** Quellen braucht: Paarauswertung,
Comparability, Ereignisverknüpfung zwischen Master und Quelle (§39.2).

**Alternative:** Rohe Bänder senden und alles zentral rechnen. Verlagert Last
auf einen Prozess, macht die Frames größer und widerspricht §35.1.

**Begründung:** Der Vorschlag ist keine echte Wahl, sondern die Fortschreibung
einer belegten Architekturregel — er steht hier nur, weil das Ticket zum ersten
Mal Metriken über **acht gemittelte Frames** verlangt und die Frage damit
konkret wird.

#### E-07 — Ort des Comparability-Score

**Frage:** Der Score aus Jaccard, Cosine und Zeitüberdeckung braucht das aktive
Quellenset **beider** Passagen. Rechnet ihn der Broker (der den Sessiongraphen
hält) oder Main (das die Passagen besitzt)?

**Vorschlag:** **Broker.** Er hält den Sessiongraphen und die Passagen im
Store; Main hält nur kompakte aktuelle IDs (§33.5). Ein Main-seitiger Score
müsste den Graphen spiegeln — genau die zweite konkurrierende Wahrheit, die
§33.5 verbietet.

**Alternative:** Main rechnet, weil dort die Anzeige entsteht. Zwingt Main, den
vollen Quellzustand zweier Passagen zu halten.

#### E-08 — Ort des Hörmarkers

**Frage:** §58 nennt den „lokalen Hörmarker" im P4-Lieferumfang. Er sitzt heute
in Gen/Main (`PluginProcessor.cpp`:453), nicht in Probeeq
(`SondeProcessor.cpp`). §34.2 spricht dagegen von „lokalen Markern", die „den
Ereignisstempel zunächst lockfrei in einen kleinen vorallokierten
RT→Control-Ring" schreiben — im Plural und im Kontext der Sonden.

**Vorschlag:** Der Marker bleibt in dieser Etappe, wo er ist. SONDE-013 baut den
**Interventions-Producer** (M-37, M-38) an der vorhandenen Stelle und den
NAK-47-Ausfade (M-34) ebenfalls dort. Ein Marker in Probeeq wäre eine neue
Produktfähigkeit, keine Vertragsfolge — und Probeeq erhält laut Erratum (p)
ausdrücklich keinen lokalen Voll-Editor.

**Alternative:** Den Marker sofort auch in Probeeq bauen. Erweitert den
Ticketumfang um eine Produktentscheidung, die niemand getroffen hat.

### 4.2 Produktfragen — benannt, nicht beantwortet

Diese Punkte gehören dem User. Sie werden hier einer Fläche zugeordnet und
sonst nichts.

| ID | Frage | Fläche |
|---|---|---|
| P-01 | Wie markiert der User eine Passage — welche Geste, welcher sichtbare Zustand während der Markierung, und was passiert bei Transportstopp mitten in der Markierung? | Gen, erste Oberfläche |
| P-02 | Was zeigt Gen von einem **laufenden** Experiment, während der User im fremden Werkzeug arbeitet? Bleibt die Landkarte normal bedienbar oder gibt es einen sichtbaren Versuchszustand? | Gen, erste Oberfläche |
| P-03 | Ist der Blind-A/B-Modus der Standard oder eine Zuschaltung? §43.2 nennt ihn „optional"; das ist eine Bedienentscheidung, keine technische. | Gen, erste Oberfläche |
| P-04 | In welcher Reihenfolge zeigt Gen Zielmetrik, Guardrails, Effektstabilität und Hörurteil, und welche Guardrail-Verletzung ist sichtbar dringlicher als eine andere? | Gen, erste Oberfläche |
| P-05 | Wie sieht ein PRE/POST-Paar in der Quellenliste aus — zwei Zeilen, eine Zeile mit zwei Hälften, oder eine eigene Ansicht? | Gen, erste Oberfläche |
| P-06 | Soll der User eine Passage benennen können, und reist der Name mit dem FL-Projekt? Die Technik kann beides; die Frage ist, ob es das Produkt will. | Gen, erste Oberfläche |

### 4.3 Widersprüche zwischen Quellen

| ID | Widerspruch | Auflösung in dieser Matrix |
|---|---|---|
| V-01 | Der Auftrag nennt für §33.3 die vier Familien `experiment_begin`, `experiment_abort`, `experiment_result` und `user_verdict` gemeinsam. Das Reservierungsregister ordnet nur die ersten beiden `SONDE-013` zu; `experiment_result` gehört `SONDE-017`, `user_verdict` gehört `SONDE-014`. | Das Register ist jünger und feiner und gewinnt. Nach Nacharbeit 1 (§8, D1) definiert SONDE-013 **drei** Familien: `experiment_begin` und `experiment_abort` aus `reserviert` plus die neue Familie `experiment_manual_result` (M-66). `experiment_result` und `user_verdict` bleiben fremdes Eigentum (M-73). |
| V-02 | NAK-68 nennt als Eigentümer `SONDE-010` und als Änderungssatzpartner NAK-59. Beide sind abgeschlossen; die Klassenmarke der Zeile lautet **[Planarbeit · S20–22]**. | Die Klassenmarke gewinnt (§1.5). Die Registerdatei selbst wird in dieser Etappe auftragsgemäß nicht angefasst; das Nachziehen steht als O-03 unten. |
| V-03 | Entwurf §33.2 führt „Breite/Korrelation" nur bei der Live-Telemetrie, nicht beim Evidenzsnapshot — der Kommentar in `FeatureEngine.h`:358-360 hält das ausdrücklich fest. §40.1 verlangt dagegen bandweise Korrelation „in kurzen und mittleren Fenstern" und Kohärenz über acht Welch-Frames, was zur Evidenzkadenz passt. | Kein echter Widerspruch: §33.2 regelt, was der **Liveframe** trägt, §40.1, was der Worker **rechnet**. Die bandweisen Stereometriken reisen auf dem Evidenzpfad (M-11 zusammen mit E-05), die zwei Skalare bleiben live. |

### 4.4 Ausdrücklich außerhalb dieser Etappe

- **O-01** — Der lokale Compare-Routing-Pfad (§38.4, Sidechain-Aux, Fractional
  Delay, hörbares Delta) gehört zu P8 (`SONDE-018`). Dieses Ticket baut nur die
  Sperre (M-24) und den verteilten Zweig (M-19).
- **O-02** — `Intent`, `CauseHypothesis`, `Proposal` und `AssistantStep` gehören
  zu `SONDE-014` (S23–25). Dieses Ticket erzeugt Evidenz, keine Befunde.
- **O-03** — Die NAK-68-Zeile in `docs/offene-punkte.md` nennt einen
  abgeschlossenen Eigentümer (V-02). Das Nachziehen ist ein Doku-Änderungssatz
  außerhalb der Grenze dieser Etappe und gehört in den Abschluss von SONDE-013.
- **O-04** — `experiment_result` und `user_verdict` bleiben unberührt (M-73).
- **O-05** — Die Anzeige der Überdeckung als laufender Wert gehört ausdrücklich
  zu `SONDE-014` (`docs/bauaufteilung-sonden.md`:394, Antwort U16), die farbige
  Zone zu S31b.

---

## 5. Bauplan für Etappe 2

### 5.1 Reihenfolge der Bauetappen

Die Reihenfolge folgt der Abhängigkeit, nicht der Reihenfolge im Gate-Text.
Seit §7.1 und der Nacharbeit 1 ist **keine** Etappe mehr durch einen
offenen Vertragsentscheid blockiert; die Spalte nennt jetzt den Entscheid,
der die jeweilige Etappe bindet (5.4).

| Etappe | Inhalt | Kanonbeine, die am Ende dieser Etappe grün sein müssen | Bindender Entscheid |
|---|---|---|---|
| **A — Vertragsrunde** | `experiment_begin` und `experiment_abort` definieren, die neue Familie `experiment_manual_result` definieren und im Register verbrennen (§7.1 E-02), `evidence_snapshot.ereignisse` belegen (samt Kohärenzfeldern aus M-11), `grund`-Enum um `material_wechsel` und `messpunkt_wechsel` erweitern, `gesamt_erwartet` auf 28, **Fassung 2** des `wire_envelope_schema_minor` als **einen** Schritt, `integration_samples` als Feld-ID 14 auf der P2-Fläche, alle Fixtures und beide Leser. Matrix M-64 bis M-67, M-73. | A5, A8, A9, A10, A20, B3c, B10, A4 | — (E-02 und E-04 sind in §7.1 entschieden) |
| **B — Evidenzpfad** | `evidence_snapshot` bekommt Sender in der Sonde und Empfänger im Broker; Kadenz 1 bis 4 Hz mit Reduktion bei Überlast; P10/P50/P95, Abdeckung, Konvergenz. Matrix M-05 als Träger, Voraussetzung für M-11 und M-19. | A4, A5, A22, B3c, B10 | §7.1 E-05 |
| **C — Metriken in der Sonde** | Momentary 400 ms, True Peak nach M-02, PLR und PSR nach M-03, mehrfenstriger Crest, LRA mit 60-s-Regel, bandweise M/S und Kohärenz, `integration_samples` füllen. Matrix M-01 bis M-12, M-74 bis M-77. | A1, A16, B4, B5, B9, B12, neu `EqCopSonde013DynamicsTest`, `EqCopSonde013StereoGoldenTest`, `EqCopSonde013TruePeakGoldenTest` | §7.1 E-06 (Metriken entstehen in der Sonde) |
| **D — Interventions-Producer und NAK-47** | Vorallokierter RT→Control-Ring, `audible_intervention_begin/end` aus dem Plugin, erzwungener Ausfade statt `hartAus()`, `recording=false` im `erlaubt`-Term. Matrix M-33 bis M-39, M-58. | A1, A3, A4, A16, B10, neu `EqCopSonde013InterventionRingTest` | §7.1 E-01 (nur der Oversize-Zweig) |
| **E — Passage und Fingerprint** | Passageobjekt, Fingerprint, Store-Anbindung, `MainProjectState`-Persistenz mit Save/Load-Symmetrie und Host-Dirty. Matrix M-25 bis M-32, M-68, M-69. | A4, A4-SI, A12, B2, B4, neu `EqCopSonde013PassageStateTest`, `EqCopSonde013FingerprintGoldenTest` | — |
| **F — PRE/POST-Join** | v3-Paarbildung, Alignment mit vier Klassen, Coverage, Herabstufungen, `raw_project_frame_key` gegen `aligned_frame_key`. Matrix M-13 bis M-24. | A4, A22, A24, anzulegen `broker/tests/sonde013_prepost.rs`, `EqCopSonde013PrePostGoldenTest` | — |
| **G — Experiment `manual_external`** | Begin, Abort, Ergebnisweg über `experiment_manual_result` (§7.1 E-02), Match-Gain, Blindbindung, vier Auswertungsachsen, fünf Verdikte, zwei Bestandsdeckel statt Wanduhr (§7.1 E-03), Rekonstruktion über Sitzungsende und Neustart hinweg. Matrix M-40 bis M-51, M-59. | A4, A4-SI, A5, anzulegen `broker/tests/sonde013_experiment.rs`, `EqCopSonde013ExperimentUiTest`, `EqCopSonde013ExperimentGoldenTest` | — (E-02 und E-03 sind in §7.1 entschieden) |
| **H — Invalidierung, Taint und Korpus** | `evidence_invalidate`-Sender, sitzungsweite Quarantäne, inverse Pfade über alle vier `art`-Werte, P4-Referenzkorpus mit Erzeugerskript. Matrix M-52 bis M-57, M-60 bis M-63, M-79 bis M-85. | **voller Kanon** plus anzulegen `broker/tests/sonde013_taint.rs`, die anzulegenden `tools/eq-copilot/pruefe_p4_korpus.py` und `tools/eq-copilot/erzeuge_p4_korpus.py` | — |

Etappen C, E und F sind untereinander unabhängig und können in beliebiger
Reihenfolge laufen; H setzt alle voraus.

### 5.2 Ticketpfade — der spätere Prüfbereich

Ein Prüfer dieses Tickets liest genau diese Pfade. Alles außerhalb ist
Nebenbefund und geht nach `docs/offene-punkte.md`, nicht in das Urteil.

```
eq-copilot/schemas/v3/eq-ipc-v3.schema.json
eq-copilot/schemas/v3/reservierte-nachrichten-v1.json
eq-copilot/schemas/v3/flatbuffers/nakama_telemetry_v1.fbs
eq-copilot/schemas/v3/flatbuffers/FELD-IDS.json
eq-copilot/plugin/core/analysis/FeatureEngine.h
eq-copilot/plugin/core/analysis/LoudnessAccumulator.h
eq-copilot/plugin/src/HoerMarkierung.h
eq-copilot/plugin/src/PluginProcessor.cpp
eq-copilot/plugin/src/AnalyseEngine.cpp
eq-copilot/plugin/src/SourcesModel.h
eq-copilot/plugin/sonde/SondeProcessor.cpp
eq-copilot/plugin/state/NakamaState.cpp
eq-copilot/plugin/tests/
broker/src/coordinator/intervention.rs
broker/src/store/
broker/src/lib.rs
broker/tests/
tools/eq-copilot/pruefe_v3_vertrag.py
tools/eq-copilot/erzeuge_fb_fixtures.py
tools/eq-copilot/erzeuge_v3_fixtures.py
tools/beweise.ps1
```

Drei Stellen dieses Prüfbereichs sind nach Nacharbeit 1 die Riegel, an denen
ein Vertragsfehler zuerst auffällt (§8, D1 und D2):

- `eq-copilot/schemas/v3/reservierte-nachrichten-v1.json` — `definiert`
  wächst auf 21, `reserviert` fällt auf 7, `gesamt_erwartet` auf 28, und
  `wire_envelope_schema_minor.aktuell` geht **einmal** von 1 auf 2;
- `tools/eq-copilot/pruefe_v3_vertrag.py`:1063-1079 — Namens-, Disjunktheits-
  und Summenprüfung; sie kennt nur ganze Familien, also fällt jeder
  teilbelegte fremde Name hier;
- `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`:1085-1094 —
  `evidence_invalidate` bleibt bei `type`, `grund`, `umfang`; ein neu
  erfundener Steuerkopf wäre ein Befund, kein Fortschritt.

`eq-copilot/plugin/core/analysis/FeatureEngine.h` steht doppelt im
Prüfbereich: für den Transport des Ereignisstroms (M-05) **und** für den
eigenständigen Peakpfad, der heute fehlt (M-86, §8, D6).

### 5.3 Risiken

1. **Der Evidenzpfad ist größer, als der Gate-Text vermuten lässt.** Er ist
   vertraglich fertig, aber ohne jede Zeile Code (E-05). Wird er in Etappe B
   nicht sauber gebaut, hängen M-05, M-11 und M-19 in der Luft, und das
   Exit-Gate ist nicht ehrlich erreichbar.
2. **Der `.fbs` wird zweimal angefasst.** `integration_samples` (Etappe A) und
   spätere Felder anderer Tickets treffen dieselbe Fläche. Jede Änderung zieht
   `flatc`-Neugenerierung, `FELD-IDS.json`, beide Fixturekorpora und beide Leser
   nach — der Grund, warum NAK-68 und NAK-59 ursprünglich gemeinsam geplant
   waren. Da NAK-59 bereits geschlossen ist, trägt dieses Ticket den Umlauf
   allein.
3. **Der Interventions-Producer ist echtzeitkritisch.** Er schreibt aus dem
   Audiothread. Ein Ring ohne `drop-oldest` **muss** bei Overflow melden statt zu
   blockieren; ein Fehler hier verletzt das Grundgesetz direkt und ist im
   Nulltest nicht sichtbar, weil er kein Audio verändert.
4. **NAK-47 kann den Nullvertrag brechen.** Der Ausfade fügt dem Audiothread
   einen Zustand hinzu, der vorher nicht existierte. `EqCopMarkierungTest` und
   `EqCopNullTest` sind die Wachen; sie müssen **vor** dem Fix einmal absichtlich
   gebrochen worden sein, sonst ist der Fix unbelegt (Prüfliste §E).
5. **Die Schwellen sind Startwerte.** Jaccard 0,9, Cosine 0,95, Zeitüberdeckung
   95 %, Kohärenz 0,8 und 0,9 sind ausdrücklich kalibrierbar. Werden sie als
   Literale in den Produktpfad geschrieben statt an `metrics_version` gebunden,
   ist die spätere Kalibrierung ein Bruch statt einer Versionierung.
6. **Der v2-Vorläufer `paare_auswerten` ist eine Falle.** Er sieht aus wie die
   Lösung, arbeitet aber auf `SensorEintrag` statt auf `probe_descriptor`. Ihn zu
   erweitern statt den v3-Pfad zu bauen, hieße eine zweite Wahrheit anzulegen.

### 5.4 Welcher Vertragsentscheid welche Bauetappe bindet

Die vier ehemaligen Sperren (E-02, E-04, E-05, E-01) und der Bestandsdeckel
E-03 sind alle entschieden: E-01, E-05 bis E-08 in §7.1, E-02, E-03 und E-04
in §7.1 mit dem in Nacharbeit 1 ersetzten Wortlaut (§8, D1, D2, D4, D5). Die
Tabelle bleibt als Karte stehen — fünf Zeilen, weil E-03 seit dem Entscheid
Etappe G genauso bindet wie die vier vormaligen Sperren: sie sagt je Etappe,
welcher Entscheid sie bindet und was ohne ihn passiert wäre.

| Ehemalige Sperre | Betroffene Etappe | Bindende Regel, seit sie entschieden ist |
|---|---|---|
| **E-02** — Ergebnisweg des manuellen Versuchs | A und G | Neue Familie `experiment_manual_result` im Eigentum von SONDE-013; `experiment_result` bleibt vollständig bei SONDE-017, `gesamt_erwartet` steigt auf 28 (§7.1 E-02, §8 D1). Ohne den Entscheid hätte der Bau einen fremden Namen oder einen Teilzweig davon belegt — der Vertrag kennt beides nicht. |
| **E-04** — Invalidierungsgrund für Material- und Messpunktwechsel | A und H | Zwei neue Gründe, ausgeliefert mit **Fassung 2** des `wire_envelope_schema_minor`; kein Steuerkopf im Payload (§7.1 E-04, §8 D2). Ohne den Entscheid wäre entweder ein Steuerkopf erfunden oder die strikte Enum ohne wirksame Fassungswahl geändert worden. |
| **E-05** — Umfang des Evidenzpfads | B, und damit M-05, M-11, M-19 | Der Evidenzpfad ist Teil dieses Tickets, als eigene Etappe B vor den Vergleichsfunktionen; Ereignisse und die Kohärenzfelder aus M-11 reisen nur dort, nie im 10-Hz-Liveframe (§7.1 E-05). |
| **E-01** — Ausfade im Oversize-Block | D, nur dieser Zweig | Erzwungener Ausfade innerhalb der Kapazität, danach Riegel bis `prepareToPlay` (§7.1 E-01, Matrix M-36). |
| **E-03** — Retention offener Experimente | G | Zwei Bestandsdeckel, `N_projekt` und `N_global`, beide benannte Konstanten des Stores; Verdrängung schreibt ein terminales `aborted` mit Grund `verdraengt`. Sitzungsende, Reconnect und Neustart brechen **nicht** ab (§7.1 E-03, §8 D4 und D5). |

**E-06**, **E-07** und **E-08** binden Ort und Zuschnitt, nicht einen
Vertragsteil: Metriken entstehen in der Sonde, der Comparability-Score im
Broker, der Hörmarker bleibt in Gen/Main (§7.1). Sie sperren keine Etappe.

---

## 6. Prüfliste

`tools/dirigent/pruefliste.md`, Zeile für Zeile. Sie ist Kontext für den
Prüfer, keine Anforderungsquelle — die Anforderungen stehen in §1 und §3.

### A. Rückstau und Prioritätsklassen

| Zeile | Wo in diesem Manifest gemessen |
|---|---|
| Politik bei *voll* steht je Klasse ausdrücklich | **M-78**: P2 `drop-oldest`; **M-39**: der P0-Interventionsring ausdrücklich **nicht**, er meldet stattdessen sticky Unknown. Beide Politiken sind benannt, keine löscht still Akzeptiertes. |
| Puffer mit Abflussweg ohne Reconnect | **M-39** und **M-61**: `neutral_resync` ist eine Coordinator-Lebenszyklusoperation, kein Reconnect. |
| Schlüssel überleben Zwischenpuffer, Koaleszierung gilt dort auch | **M-21**: `raw_project_frame_key` und `aligned_frame_key`; **M-13**: `pair_id` überlebt die Paarbildung. |
| Niedrigere Klasse blockiert, höhere wird weiter gelesen | **M-78**, gemessen von **A22**. |
| Rückgabewerte und Zähler werden ausgewertet | **M-63** (`evidence_gesperrt`), **M-75** (`oversize_drop`), **M-05** (Verlustzähler des Ereignisrings). |
| Dieselbe Regel in beiden Sprachen, Test beidseitig | **M-70**: A5, A8, A10, B3c sind der Riegel; **M-64** verlangt beide Leser im selben Änderungssatz. |

### B. Lebenszyklus

| Zeile | Wo in diesem Manifest gemessen |
|---|---|
| Reihenfolge beim Verbinden festgelegt und gemessen | **M-40**: Baseline verriegeln **vor** `experiment_begin`; **M-37**: Ring schreiben, dann Worker sendet. |
| Beim Trennen zuerst abmelden, dann Fristen | **M-58**: Ausfade abgeschlossen, dann `end`, dann Nachlauf, dann frei. |
| Jeder Join hat eine Frist | **M-48**: Retention als Frist offener Experimente; **M-38**: `tail_samples` als Frist des Taintintervalls. |
| Nach dem Schließen wird nichts mehr geliefert | **M-59**: beide Terminalereignisse schließen **alle** Intervalle; **M-48** schließt auch die ohne Terminal. |
| Stopp-Fenster ist ein Test, kein Zufall | **M-50**: Neustart mit offenem Versuch, **keine** stillschweigende Fortsetzung. |

### C. Verträge und Längen

| Zeile | Wo in diesem Manifest gemessen |
|---|---|
| Textfeld gegen Schema-Länge geprüft, Negativfixture je Feld | **M-65**: Negativfixture ist Bedingung des Belegens; **M-42**: Usernotiz und optionaler Werkzeugname sind die neuen Textfelder dieses Tickets. |
| Exakte Feldmenge, `additionalProperties:false` in beiden Sprachen | **M-65** ausdrücklich: `evidence_snapshot` ist heute `additionalProperties: false` — das ist der Grund, warum das Belegen kein additiver Schritt ist. |
| Discriminator vor dem Inhalt geprüft, beide Richtungen | **M-57** (`invalidate_umfang`), **M-67** (fünf neue Diskriminatormengen), **M-13** (`measurement_position`). |
| NaN/Inf/Grenzwerte an jeder Zahl über den Draht | **M-07** für alle neuen Metriken; **M-17** ausdrücklich für die `i64`-Ränder der Projektzeit. |

### D. Bau- und Prüfriegel

| Zeile | Wo in diesem Manifest gemessen |
|---|---|
| Riegel fail-closed ohne Rohtextheuristik | **M-21**: der Zeitpfadriegel muss über **alle** Wege scheitern, mit Mutationsprobe. |
| Bein prüft Frische seiner Eingaben, Exit 3 statt grün | **M-72**: der Kanonlauf verweigert die Beglaubigung bei veralteten Prüfbinaries. |
| Festgeschriebene Hashes gelten für eine Auslieferung, nicht für den Kanon | Nicht zutreffend — dieses Ticket liefert kein Bundle aus und ändert keinen Installermanifest-Hash. |
| Was der Kanon nicht baut, darf er nicht als frisch bezeugen | **M-80**: das Erzeugerskript ist selbst ein Kanonbein mit `--pruefen`. |

### E. Behauptung ≤ Messung

| Zeile | Wo in diesem Manifest gemessen |
|---|---|
| Behauptung sagt nicht mehr, als der Test misst | Jede der 86 Matrixzeilen nennt ihren Test; **NEU** markiert, was noch nicht existiert. **M-74** verlangt für „keine Allokation" einen Allokationszähler. |
| Zahlen sind gemessen, nicht abgeschrieben | Kopf: Basis-SHA aus `git rev-parse HEAD`, Kanongleichstand aus `git diff --stat`. §2: jede Zeilennummer an `ed9bbf7` gelesen. Die Familienzahl 27 und die Aufteilung 18/9 aus `reservierte-nachrichten-v1.json`, nicht aus dem Entwurf. |
| Positionen im lebenden Kopf als Symbol, nicht als Zeilennummer | Der Kopf nennt Dateien und Commits, keine Zeilennummern. Zeilennummern stehen nur in §2 und §3 und sind dort ausdrücklich an `ed9bbf7` gebunden. |
| Lebender Kopf wird beim Abschluss nachgezogen | Verfahrensregel für Etappe 2, hier vorgemerkt (**M-72**). |
| Jede neue Prüfung einmal absichtlich gebrochen, Rohausgabe bei | **Risiko 4** in §5.3 macht es für die NAK-47-Wachen ausdrücklich zur Bedingung; **M-21** und **M-29** verlangen je eine Mutationsprobe. |
| Geänderte Zusage — drei Stellen | **M-72** wörtlich: Runnerkopf, Skriptkopf, lebender Manifestkopf. |
| Writer-Fixtures statt Handschrift | **M-80**: der Korpus wird vom Erzeugerskript gebaut und eingefroren; **M-68**: eine Migration-2-Fixture käme vom Writer. |

### F. Änderungssatz

| Zeile | Wo in diesem Manifest gemessen |
|---|---|
| speichern↔laden, starten↔stoppen, aktivieren↔abklingen im selben Commit | **M-69** (Save und Load gemeinsam), **M-40** mit **M-47** (begin und abort), **M-37** mit **M-38** (begin und end), **M-34** mit **M-35** (aktivieren und abklingen), **M-51** (anlegen und löschen), **M-68** (Migration mit Vor- und Rückweg). |
| Writer, Reader, Migration, Fixtures, Cross-Language-Verbraucher im selben Änderungssatz | **M-64**, **M-65**, **M-70** — Etappe A ist genau deshalb eine geschlossene Etappe und kein Nebenschritt. |

### Nicht zutreffende Zeilen

- **D, dritte Zeile** (Bundlehashes nach Relink): Dieses Ticket ändert kein
  Installermanifest und liefert kein Bundle aus; `A17` und `A18` sind nicht im
  Ticketpfad.
- Die Prüfliste kennt keine Zeile zu UI-Sichtprüfung. `EqCopShot` ist in diesem
  Ticket kein Beweismittel, weil P4 keine neue Oberfläche liefert — die
  Anzeigefragen stehen als P-01 bis P-06 in §4.2 und gehören einer späteren
  Fläche.

---

## 7. Dirigentenentscheide zu Etappe 1 (Fable, 2026-09-03)

Regel aus `.claude/skills/dirigent`: Eine Technikfrage ohne Quelle ist eine
**Lücke**; der Dirigent entscheidet die Regel in derselben Runde und trägt
sie ins Manifest ein. Produktfragen entscheidet der User. Dieser Paragraph
schließt die acht Punkte aus §4.1.

**Stand nach Nacharbeit 1 (03.09.2026).** Die Matrixprüfung 1 hat drei
dieser Entscheide als Defekt widerlegt. **E-02**, **E-03** und **E-04**
tragen deshalb unten den in §8 festgelegten neuen Wortlaut; der alte steht
dort als Verlauf. E-01 und E-05 bis E-08 gelten unverändert. Die vier
vormals mit ENTSCHEIDEN-VORSCHLAG geführten Matrixzeilen (M-36, M-48, M-49,
M-54) sind in §3 und §3.11 auf **ENTSCHIEDEN** nachgezogen; §4.1 verweist
bei E-02, E-03 und E-04 auf diesen Paragraphen. Bei einem Widerspruch
zwischen §3/§4 und §7 gewinnt §7, und innerhalb von §7 gewinnt der in §8
belegte neue Wortlaut.

### 7.1 Entscheide

| ID | Entscheid | Regel für Matrix und Bau | Test |
|---|---|---|---|
| **E-01** (M-36) | **Erzwungener Ausfade innerhalb der Kapazität, danach Riegel bis `prepareToPlay`.** Weder `hartAus()` (Alternative B, bricht §49.3) noch ein zweiter Chunk-Pfad im Audiothread (Alternative A). | Trifft ein Hostblock über `wetKapazitaet` ein, während die Markierung hörbar ist, läuft der Wet-Pfad genau über die ersten `min(n, wetKapazitaet)` Samples mit einer erzwungenen linearen Rampe auf 0, deren Länge `min(Rampenlänge, wetKapazitaet)` ist; der Rest des Blocks und jeder Folgeblock sind bitidentischer Neutralpfad. Nach der Rampe steht `fade = 0`, die Filterzustände werden geleert, und die Markierung bleibt bis zum nächsten `prepareToPlay` verriegelt — ein erneutes `erlaubt` blendet in dieser Laufzeit nicht wieder ein (kein Flattern bei wiederholten Oversize-Blöcken). Kein neuer Zustand außerhalb des vorhandenen Markerobjekts, keine Allokation, keine Latenz, kein Tail. | **NEU** A3 `oversize_block_fades_within_capacity_then_latches`: Kantendifferenz unter der Klickschwelle aus M-34, Rest des Blocks und Folgeblöcke bitidentisch zum Neutralpfad, Wiedereinblendung vor `prepareToPlay` bleibt aus, nach `prepareToPlay` wieder möglich. |
| **E-02** (M-49) | **Neue Familie `experiment_manual_result` im Eigentum von SONDE-013; `experiment_result` bleibt vollständig reserviert bei SONDE-017.** Ersetzt in Nacharbeit 1 den ersten Entscheid, der `experiment_result` mit einer `execution_mode`-Union belegen wollte — alter Wortlaut in §8, D1. Weder ein fremder Name noch ein Teilzweig davon, und kein rein lokales Ergebnis (Alternative B, bricht §33.5). | Der Vertrag kennt nur reservierte **Familien**, keine teilreservierten Discriminatorzweige: `tools/eq-copilot/pruefe_v3_vertrag.py`:1063-1079 prüft Namen, Disjunktheit und Summe je Familie; ein `active_probe`-Zweig im `oneOf` wäre bereits definiert, ein fehlender Zweig lediglich unbekannt. SONDE-013 definiert deshalb die neue Familie `experiment_manual_result` **direkt in `definiert`**; `gesamt_erwartet` steigt von 27 auf 28, die Aufteilung auf 21/7, und die Summenprüfung zieht über die Zahl im Register mit. Nutzlast: `experiment_id`, Zieladresse, `base_revision`, `ttl_ms`, Schema-/Capability-Version wie jede steuernde Nachricht (M-66), dazu Hörurteil, aufgedeckte Blindreihenfolge, Usernotiz und optionaler Werkzeugname; **keine** State-Hashes, **keine** Candidate-Revision, **kein** Match-Gain. Zielmetrik, Guardrail-Deltas und Effektstabilität rechnet der Broker aus Baseline- und Resultatevidenz im Store und schreibt das terminale `result`-Ereignis mit `execution_mode = manual_external` append-only in `experiment_events`; der Rückweg zu Gen läuft über den bestehenden Outbox-/Snapshot-Pfad. Versionsschritt: eine neue **Fassung 2** des `wire_envelope_schema_minor` (aktuell 1 → 2), deren `fassungen."2"`-Eintrag die drei neu definierten Familien `experiment_begin`, `experiment_abort` und `experiment_manual_result` nennt; ein Leser der Fassung 1 lehnt sie ab. M-73 bleibt unverändert: `experiment_result` und `user_verdict` sind fremdes Eigentum. | **NEU** A5-Fälle `experiment_manual_result_definiert`, `experiment_result_bleibt_reserviert_fuer_sonde017` und `summe_ist_28`; **NEU** B10 `EqCopIpcTest` für die C++-Hälfte; **NEU** `sonde013_experiment.rs`, Fall `manual_result_writes_terminal_event_and_deltas`. |
| **E-03** (M-48) | **Zwei bestandsgebundene Deckel — je Projektbindung und projektübergreifend —, keine Wanduhr, kein Sitzungsende-Abbruch.** In Nacharbeit 1 um den globalen Deckel ergänzt (§8, D5) und in der Auslöserliste geschärft (§8, D4); alter Wortlaut in §8, D5. | `N_projekt` je Projektbindung (mindestens 2) und `N_global` über alle Projektbindungen (mindestens `N_projekt`); beide sind benannte Konstanten des Stores, kein Literal im Pfad, vom Bauer begründet gewählt. Beim Überschreiten eines Deckels erhält das älteste offene Experiment **des jeweiligen Bereichs** ein terminales `aborted` mit maschinenlesbarem Grund `verdraengt`, bevor das neue angelegt wird; der append-only Store ändert keine alte Zeile. Ohne den globalen Deckel wüchse der Bestand offener Zeilen über fortlaufend neue Projektbindungen weiter unbegrenzt — genau das schließt Entwurf §43.1:2842-2845 aus. Terminale und benannte Experimente bleiben, bis der User sie löscht (Roadmap 15). Ein Sitzungsende, Projektwechsel, Reconnect, UI-Neustart oder Brokerneustart bricht **nicht** ab: §43.1 verlangt Rekonstruierbarkeit nach Reconnect und UI-Neustart, und ein manueller Versuch kann eine Sitzung überdauern. Damit sind die Auslöser eines terminalen `aborted` genau zwei — Userabbruch und Verdrängung (M-47). | **NEU** `sonde013_experiment.rs`, Fälle `open_cap_per_project_at_n_and_n_plus_one` und `open_cap_global_at_n_and_n_plus_one` (Grenztests an `N` und `N+1`, Muster C-09), `restart_keeps_open_experiment` sowie `abort_writes_terminal_event_for_each_trigger` mit der Gegenprobe, dass Sitzungsende und Neustart kein Ereignis schreiben. |
| **E-04** (M-54) | **Zwei neue Gründe `material_wechsel` und `messpunkt_wechsel`, ausgeliefert mit Fassung 2 des `wire_envelope_schema_minor` — kein Steuerkopf.** Ersetzt in Nacharbeit 1 den ersten Entscheid, der den Versionsschritt im `steuerkopf` verlangte — alter Wortlaut in §8, D2. Weder `intervention` noch `epochwechsel` als Ersatz. | `evidence_invalidate` besitzt keinen Steuerkopf: das Objekt trägt nur `type`, `grund` und `umfang` (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json`:1085-1094). Die geltende Fassungswahl läuft über `message_family` und `schema_minor` des Wire-Envelopes (`reservierte-nachrichten-v1.json`:7-11, Feld `auswahlregel`); im Payload entsteht kein weiteres Versionsfeld. Die zwei Gründe kommen deshalb mit derselben **Fassung 2**, deren Eintrag sie nennt — etwa als `evidence_invalidate_grund_erweitert` mit den Werten `material_wechsel` und `messpunkt_wechsel`. Ein Leser der Fassung 1 lehnt die neuen Gründe ab, statt sie still auf einen bekannten abzubilden (Negativfixture). Leser beider Sprachen tragen die Fassung 2: A5 und A8 auf der JSON-/Rust-Seite, B3c auf der C++-Seite. **A9** (Codegen-Drift 0) ist nur berührt, wenn ein `.fbs` angefasst wird — hier nicht. | **NEU** A5-Fälle `grund_material_wechsel`, `grund_messpunkt_wechsel`, `unbekannter_grund_wird_abgelehnt` und `fassung_1_leser_lehnt_neue_gruende_ab`; **NEU** B3c-Zwilling; `sonde013_taint.rs` wie M-54. |
| **E-05** | **Ja, der Evidenzpfad (`evidence_snapshot`, 1 bis 4 Hz) ist Teil von SONDE-013, als eigene Bauetappe B vor den Vergleichsfunktionen.** Keine Planänderung, kein Vorticket. | Sender in der Sonde, Empfänger im Broker und in Gen; Ereignisse reisen nur dort, nie im 10-Hz-Liveframe (§33.2). Etappe B endet mit einem eigenen Kanonlauf, bevor Etappe C beginnt. | M-05, M-11, M-19 wie in §3; Kanon nach Etappe B. |
| **E-06** | **Metriken entstehen in der Sonde**, der Broker rechnet nur Mehrquellen-Größen (§35.1, §39.2). | Momentary-400-ms, PLR, PSR, mehrfenstriger Crest, bandweise M/S und Kohärenz über acht Welch-Frames im Sondenworker; Paarauswertung, Comparability und Master↔Quelle-Ereignisverknüpfung im Broker. | wie §3.1, §3.2. |
| **E-07** | **Comparability-Score im Broker.** | Der Broker hält Sessiongraph und Passagen (§33.5); Gen hält nur IDs und erhält den Score über den Snapshot-Pfad. | wie §3.3. |
| **E-08** | **Der Hörmarker bleibt in Gen/Main.** Kein Marker in Probeeq in diesem Ticket. | Interventions-Producer (M-37, M-38) und NAK-47-Ausfade (M-34, M-36) an der vorhandenen Stelle in `PluginProcessor.cpp` / `HoerMarkierung.h`. Ein Marker in Probeeq wäre eine Produktfähigkeit ohne Entscheid und widerspricht Erratum (p). | wie §3.4. |

### 7.2 Widersprüche und Produktfragen

- **V-01 bis V-03** — die Auflösungen aus §4.3 gelten. Der erste Entscheid
  E-02 hatte V-01 umgedreht („`experiment_result` ist nicht mehr fremdes
  Eigentum"); das ist mit Nacharbeit 1 **zurückgenommen** (§8, D1):
  `experiment_result` bleibt Eigentum von SONDE-017, SONDE-013 definiert
  stattdessen `experiment_manual_result`. Die Registerzeile NAK-68
  (V-02/O-03) wird im Abschluss-Commit von SONDE-013 nachgezogen.
- **P-01 bis P-06** — Produktfragen zur Bedienung auf Gen Fläche 1. Keine
  davon blockiert den Datenweg dieses Tickets: Passage, Experiment,
  Evidenz und Store entstehen unabhängig von Geste, Reihenfolge und
  Sichtbarkeit. Sie gehen im Abschluss von SONDE-013 datiert als Karten in
  `docs/plan/fragen.json` (Design-Strang, Fünferblock) beziehungsweise als
  Zeilen nach `docs/offene-punkte.md`; bis zur Antwort baut Etappe 2 die
  Technik so, dass jede Antwort ohne Vertragsänderung umsetzbar bleibt
  (Blind-A/B als Zuschaltung mit Schalter, Passagenname als optionales
  Label im `MainProjectState` gemäß §33.5, PRE/POST-Paar als Beziehung
  zweier Quellen ohne Anzeigefestlegung).

### 7.3 Nächster Schritt

Matrixprüfung 1 ist gefahren: Codex `gpt-5.6-sol`, Effort high, lesend,
Thread `01a06922-fd1d-7222-b180-fcddb1fdfc2d`, HEAD
`367a0ea732c9e883df12f2bfc5abb2f67df6da5d`. Urteil **NEEDS_WORK** mit sieben
Defekten; alle sieben sind an der Quelle bestätigt und in dieser Nacharbeit
geschlossen (§8). Nächster Schritt ist die **Wiederprüfung 1** durch einen
erneut frischen, lesenden Codex-Thread über §3, §4, §5, §7 und §8 gegen
Gate-Text, Entwurf, Verträge und Code am neuen `HEAD`. Erst nach PASS oder
Konvergenzentscheid beginnt Etappe 2 mit einem frischen Opus-Worker.

---

## 8. Nacharbeit 1 nach Matrixprüfung 1 (2026-09-03)

Matrixprüfung 1: Codex `gpt-5.6-sol`, Effort high, lesend, Thread
`01a06922-fd1d-7222-b180-fcddb1fdfc2d`, HEAD
`367a0ea732c9e883df12f2bfc5abb2f67df6da5d` vor und nach der Prüfung. Auftrag
`docs/beweise/roh/SONDE-013-matrixpruefung-1-auftrag.txt`, Rohurteil
`docs/beweise/roh/SONDE-013-matrixpruefung-1-367a0ea.txt`. Urteil:
**NEEDS_WORK**, sieben Defekte — vier P1 zu Vertrag und Invalidierung, zwei P1
zu Detektor und Lebenszyklus, zwei P2 zu Retention und Evidenzfeldern.

Der Dirigent hat jeden der sieben Befunde an der Quelle nachgelesen, alle sieben
als Defekt bestätigt und je Befund die Regel festgelegt (Auftrag
`docs/beweise/roh/SONDE-013-nacharbeit-1-auftrag.txt`). Drei dieser Regeln
ersetzen Entscheide aus §7.1 — E-02, E-03 und E-04; ihr alter Wortlaut steht
hier als Verlauf. Diese Nacharbeit hat **ausschließlich** dieses Manifest
geändert; kein Produkt-, Test-, Schema-, Fixture- oder Werkzeugcode und keine
Datei unter `docs/plan/`.

### 8.1 D1 — Eigentümer von `experiment_result` (E-02, M-49)

**Befund, wörtlich (Codex, P1, Manifestzeile um 1004):**

> DEFEKT: E-02 belegt `experiment_result` für SONDE-013, obwohl die bestehende
> Reservierung die gesamte Familie SONDE-017 zuweist
> (`eq-copilot/schemas/v3/reservierte-nachrichten-v1.json:156-158`). Der Vertrag
> kennt nur reservierte Familien, keine teilreservierten Discriminator-Zweige
> (`tools/eq-copilot/pruefe_v3_vertrag.py:1063-1079`): Ein `active_probe`-Zweig
> im `oneOf` wäre bereits definiert, ein fehlender Zweig lediglich unbekannt.
> Damit greift E-02 einem fremden Ticket vor und widerspricht zugleich M-73.

**Regel (03.09.2026), ersetzt E-02:** `experiment_result` bleibt reserviert bei
SONDE-017; SONDE-013 belegt weder einen fremden Namen noch einen Teilzweig.
SONDE-013 definiert stattdessen die **neue Familie `experiment_manual_result`**
(Eigentümer SONDE-013, direkt in `definiert`; `gesamt_erwartet` steigt von 27
auf 28, die Summenprüfung in `pruefe_v3_vertrag.py` zieht mit). Nutzlast:
`experiment_id`, Zieladresse, `base_revision`, `ttl_ms`, Schema-/
Capability-Version wie jede steuernde Nachricht (M-66), dazu Hörurteil,
aufgedeckte Blindreihenfolge, Usernotiz und optionaler Werkzeugname; keine
State-Hashes, keine Candidate-Revision, kein Match-Gain. Der Broker rechnet
Zielmetrik, Guardrail-Deltas und Effektstabilität aus Baseline- und
Resultatevidenz im Store und schreibt das terminale `result`-Ereignis mit
`execution_mode = manual_external` append-only in `experiment_events`; der
Rückweg zu Gen läuft über den bestehenden Outbox-/Snapshot-Pfad.
Versionsschritt: eine neue Fassung 2 des `wire_envelope_schema_minor` im
Register (aktuell 1 → 2), deren Fassungseintrag die drei neu definierten
Familien nennt (`experiment_begin`, `experiment_abort`,
`experiment_manual_result`); ein Leser der Fassung 1 lehnt sie ab.

**Geändert:** §7.1 E-02 (neuer Wortlaut), M-49, M-66 (drei Familien, Summe 28,
Aufteilung 21/7), M-73 (unverändert in der Zusage, um die Teilzweigregel
geschärft), §3.8 (der eine Fassungsschritt), §4.1 E-02 (Verlaufshinweis), §4.3
V-01, §5.1 Etappen A und G, §5.2, §5.4, §7.2, Kopfzeile „Änderungssatz".

**Tests:** A5-Fälle `experiment_manual_result_definiert`,
`experiment_result_bleibt_reserviert_fuer_sonde017`, `summe_ist_28`; B10
`EqCopIpcTest` für die C++-Hälfte; `sonde013_experiment.rs`
`manual_result_writes_terminal_event_and_deltas`.

**Verlauf — der ersetzte Wortlaut von E-02 aus §7.1:**

```
| **E-02** (M-49) | **`experiment_result` wird jetzt als Wirefamilie mit `execution_mode`-Union belegt; SONDE-013 baut den Zweig `manual_external`, der Zweig `active_probe` bleibt als reservierter Name ohne Nutzlast bei SONDE-017.** Keine zweite Familie (Vorschlag des Workers), kein rein lokales Ergebnis (Alternative B, bricht §33.5). | Entwurf §43.1 modelliert **ein** Experiment mit `execution_mode` = `active_probe` oder `manual_external`; die Unterschiede liegen in den Referenzen, nicht in der Familie. Der Registergrund „Deltas und Urteil setzen den Active-Compare-Pfad voraus" trägt für den manuellen Modus nicht. Die Familie reist von Gen zum Broker und trägt im manuellen Zweig: `experiment_id`, Zieladresse, `base_revision`, `ttl_ms`, Schema-/Capability-Version wie jede steuernde Nachricht (M-66), dazu Hörurteil, aufgedeckte Blindreihenfolge, Usernotiz und optionalen Werkzeugnamen; **keine** State-Hashes, keine Candidate-Revision, keinen Match-Gain. Zielmetrik, Guardrail-Deltas und Effektstabilität rechnet der Broker aus Baseline- und Resultatevidenz im Store und schreibt das terminale `result`-Ereignis append-only in `experiment_events`; der Rückweg zu Gen läuft über den bestehenden Outbox-/Snapshot-Pfad, nicht über eine neue Familie. Register: `experiment_result` wandert von `reserviert` (SONDE-017) nach `definiert` mit Eigentümer „SONDE-013 (Familie, Zweig manual_external)", und `experiment_result.active_probe` wird als reservierter Zweig mit Eigentümer SONDE-017 ohne Nutzlast eingetragen; die Summe in `pruefe_v3_vertrag.py` zieht mit. M-66 zählt damit **drei** wandernde Familien, M-73 nennt nur noch `user_verdict` als fremdes Eigentum. Ein unbekannter oder reservierter `execution_mode` wird vom Parser abgelehnt (README v3 „oneOf und der Discriminator"). | **NEU** A5-Fälle `experiment_result_manual_zweig_definiert`, `experiment_result_active_probe_bleibt_reserviert`, `summe_bleibt_konsistent`; **NEU** B10 `EqCopIpcTest` für die C++-Hälfte (Senden aus Gen, Ablehnung von `active_probe`); **NEU** `sonde013_experiment.rs`, Fall `manual_result_writes_terminal_event_and_deltas`. |
```

### 8.2 D2 — Versionsschritt der neuen Invalidierungsgründe (E-04, M-54)

**Befund, wörtlich (Codex, P1, Manifestzeile um 1006):**

> DEFEKT: E-04 verlangt den Versionsschritt im `steuerkopf`, aber
> `evidence_invalidate` besitzt keinen solchen Kopf, sondern nur `type`, `grund`
> und `umfang` (`eq-copilot/schemas/v3/eq-ipc-v3.schema.json:1085-1094`). Die
> geltende Fassungswahl erfolgt über `message_family` und `schema_minor` des
> Wire-Envelopes (`reservierte-nachrichten-v1.json:7-11`); die aktuelle Regel
> würde daher entweder einen unbegründeten Steuerkopf erfinden oder die strikte
> Enum ohne wirksame Fassungswahl ändern.

**Regel (03.09.2026), ersetzt E-04:** Kein Steuerkopf. Die zwei neuen Gründe
`material_wechsel` und `messpunkt_wechsel` kommen mit derselben **Fassung 2**
des `wire_envelope_schema_minor` (Eintrag in der Fassung 2, etwa
`evidence_invalidate_grund_erweitert` mit den beiden Werten); der Empfänger
wählt die Schemafassung anhand `message_family` und `schema_minor` im
Wire-Envelope, im Payload entsteht kein weiteres Versionsfeld. Ein Leser der
Fassung 1 lehnt die neuen Gründe ab (Negativfixture); Leser beider Sprachen
(A5/A8 Rust und JSON, B3c C++) tragen die Fassung 2. **A9** ist nur berührt,
wenn ein `.fbs` angefasst wird — hier nicht.

**Geändert:** §7.1 E-04 (neuer Wortlaut), M-54, §3.8 (der eine
Fassungsschritt, mit der ausdrücklichen Abgrenzung zur P2-Fläche von M-64),
§4.1 E-04 (Verlaufshinweis), §5.2, §5.4.

**Verlauf — der ersetzte Wortlaut von E-04 aus §7.1:**

```
| **E-04** (M-54) | **Zwei neue Gründe `material_wechsel` und `messpunkt_wechsel`, versioniert, nicht additiv.** Weder `intervention` noch `epochwechsel` als Ersatz. | Die Enum `evidence_invalidate.grund` (`eq-ipc-v3.schema.json`:1092) ist Teil einer strikten Familie; die Erweiterung folgt der Regel „Additiv oder strikt" in `eq-copilot/schemas/v3/README.md` — Versionsschritt im `steuerkopf`, beidseitige Leser (C++ und Rust), je Wert ein Positiv- und ein Negativfixture (A5, A8, B3c), Codegen-Drift 0 (A9). Ein alter Leser lehnt den neuen Grund ab statt ihn still auf einen bekannten abzubilden. | **NEU** A5-Fälle `grund_material_wechsel`, `grund_messpunkt_wechsel`, `unbekannter_grund_wird_abgelehnt`; **NEU** B3c-Zwilling; `sonde013_taint.rs` wie M-54. |
```

### 8.3 D3 — Seek im Messfenster invalidiert, statt herabzustufen (M-23)

**Befund, wörtlich (Codex, P1, Manifestzeile um 465):**

> DEFEKT: Für ein Messfenster mit Seek ordnet M-23 nur `probable` an. §58
> verlangt dagegen ausdrücklich Invalidierung bei Seek
> (`docs/FL-Nakama-Sonden-Design-Entwurf.md:4285`), und M-53 selbst sagt, dass
> der Bruch die alte Zeitreihe beendet und kein Fenster über die Grenze
> fortgeführt wird. Der geplante Test würde somit genau das Weiterverwenden von
> Evidenz festschreiben, das das Gate ausschließt.

**Regel (03.09.2026):** Der Herabstufungsgrund „Loop-/Seek-Sprünge im
Messfenster" entfällt im v3-Pfad; er ist v2-Erbe (`broker/src/lib.rs`:809-826,
Plan §5.7) und bleibt als Vorläufer nur benannt. Im v3-Pfad gilt: Eine Passage
bindet an genau eine Transportepoche (§32.4). Ein Seek, ein Loop-Wrap innerhalb
der Passage (Loop kürzer als die Passage) oder ein Drop während der laufenden
Passagenmessung invalidiert diese Messung (M-53, Grund `epochwechsel`
beziehungsweise `sequenzluecke`); die Messung beginnt mit der nächsten gültigen
Epoche neu, und kein Frame, der eine Grenze überspannt, geht in eine
Paarauswertung ein (B5-Garantie aus SONDE-009). Ein Loop-Wrap an der
Passagengrenze (Loop deckt die Passage vollständig) beendet einen Durchlauf und
beginnt den nächsten; das ist keine Herabstufung. Die drei übrigen
Herabstufungsgründe bleiben `probable`: Überdeckung unter 80 % des kürzeren
Fensters, aktive Messzeit um mehr als 10 % verschieden, verschiedene Host-PIDs.

**Geändert:** M-23 (drei Gründe statt vier, Ausgangszustand entsprechend),
M-53 (Passagenbindung und die Abgrenzung des Loop-Wraps an der Grenze).

**Tests:** `each_downgrade_reason_falls_alone` auf drei Fälle; neu in
`sonde013_taint.rs` `seek_inside_passage_invalidates_instead_of_downgrading`
und `loop_wrap_at_passage_edge_starts_next_pass`.

### 8.4 D4 — Auslöser eines terminalen `aborted` (M-47 gegen E-03)

**Befund, wörtlich (Codex, P1, Manifestzeile um 504):**

> DEFEKT: M-47 verlangt samt Test, dass ein Sitzungsende jedes offene Experiment
> abbricht, während der bindende Entscheid E-03 ausdrücklich festlegt, dass
> Sitzungsende, Reconnect und UI-Neustart nicht abbrechen und
> `restart_keeps_open_experiment` bestehen muss. Beide Anforderungen landen in
> Etappe G und können nicht gleichzeitig erfüllt werden; zudem verlangt §43.1 die
> Rekonstruierbarkeit nach Reconnect und UI-Neustart
> (`docs/FL-Nakama-Sonden-Design-Entwurf.md:2839-2845`).

**Regel (03.09.2026):** E-03 gewinnt. Auslöser eines terminalen `aborted` sind
genau: expliziter Userabbruch (`experiment_abort`) und Verdrängung durch einen
Deckel (D5). Sitzungsende, Projektwechsel, Reconnect, UI-Neustart und
Brokerneustart brechen nicht ab; ein offenes Experiment überdauert sie und ist
danach rekonstruierbar (§43.1).

**Geändert:** M-47 (Auslöserliste), §7.1 E-03 (Auslöserzahl benannt).

**Tests:** `abort_writes_terminal_event_for_each_trigger` misst genau diese zwei
Auslöserklassen und zusätzlich negativ, dass Sitzungsende und Neustart **kein**
Ereignis schreiben; `restart_keeps_open_experiment` bleibt.

### 8.5 D5 — Projektübergreifender Deckel für offene Experimente (E-03, M-48)

**Befund, wörtlich (Codex, P2, Manifestzeile um 1005):**

> DEFEKT: Der Deckel E-03 gilt nur je Projektbindung. Werden in fortlaufend
> neuen Projekten jeweils bis zu `N` Experimente geöffnet, wächst der globale
> Bestand offener Zeilen weiterhin unbegrenzt, weil weder Wanduhr noch
> Sitzungsende schließen. Das verletzt die verbindliche Zusage „offene Zeilen
> sammeln sich nicht unbegrenzt an"
> (`docs/FL-Nakama-Sonden-Design-Entwurf.md:2842-2845`); erforderlich ist
> zusätzlich eine projektübergreifende Grenze oder ein anderer terminaler
> Retentionspfad.

**Regel (03.09.2026), ergänzt E-03:** Zwei Deckel, beide benannte Konstanten des
Stores, kein Literal im Pfad, vom Bauer begründet gewählt: `N_projekt` je
Projektbindung (mindestens 2) und `N_global` über alle Projektbindungen
(mindestens `N_projekt`). Beim Überschreiten eines Deckels erhält das älteste
offene Experiment des jeweiligen Bereichs ein terminales `aborted` mit Grund
`verdraengt`, bevor das neue angelegt wird; keine alte Zeile wird geändert.
Terminale und benannte Experimente bleiben, bis der User sie löscht
(Roadmap 15).

**Geändert:** §7.1 E-03 (neuer Wortlaut), M-48, §5.1 Etappe G, §5.4.

**Tests:** `open_cap_per_project_at_n_and_n_plus_one` und
`open_cap_global_at_n_and_n_plus_one` (Grenztests, Muster C-09).

**Verlauf — der ersetzte Wortlaut von E-03 aus §7.1:**

```
| **E-03** (M-48) | **Bestandsgebundene Retention je Projektbindung, keine Wanduhr, kein Sitzungsende-Abbruch.** | Je Projektbindung sind höchstens `N` Experimente offen; `N` ist eine benannte Konstante des Stores (kein Literal im Pfad), vom Bauer begründet gewählt, mindestens 2. Beim `N+1`-ten `experiment_begin` erhält das älteste offene Experiment ein terminales `aborted` mit maschinenlesbarem Grund `verdraengt`, bevor das neue angelegt wird; der append-only Store ändert keine alte Zeile. Ein Sitzungsende, Reconnect oder UI-Neustart bricht **nicht** ab — §43.1 verlangt Rekonstruierbarkeit nach Reconnect und UI-Neustart, und ein manueller Versuch kann eine Sitzung überdauern. | **NEU** `sonde013_experiment.rs`, Fälle `open_cap_at_n_and_n_plus_one` (Grenztest an `N` und `N+1`, Muster C-09), `restart_keeps_open_experiment`. |
```

### 8.6 D6 — Eigenständiger Peak-Ereignispfad (M-05, neue Zeile M-86)

**Befund, wörtlich (Codex, P1, Manifestzeile um 442):**

> DEFEKT: M-05 behandelt den lokalen `DynamicsEvent`-Strom als bereits
> vollständig und plant nur dessen Transport. Tatsächlich erzeugt
> `FeatureEngine` ein Ereignis ausschließlich bei `fluss > schwelle`, setzt dabei
> `qualitaetFluss` immer auf `true` und führt den Peakpfad nur als Zusatzbit
> (`FeatureEngine.h:1605-1621`); Peaksteigung wird gar nicht verwendet. Damit
> fehlen der eigenständige Gegenbeleg für sehr kurze Impulse und ein Test dafür,
> obwohl §39.1 den Detektor aus spektralem Fluss, Peaksteigung und Crest
> ausdrücklich verlangt (`docs/FL-Nakama-Sonden-Design-Entwurf.md:2592-2599`).

An der Quelle bestätigt: `eq-copilot/plugin/core/analysis/FeatureEngine.h`:1607
ist die einzige Erzeugungsbedingung (`fluss > schwelle && mad > 0.0`), :1619
setzt `qualitaetFluss` konstant `true`, :1605 und :1620 tragen nur das
Crest-Zusatzbit; eine Peaksteigung kommt im ganzen Header nicht vor.

**Regel (03.09.2026):** §2.1 wird korrigiert (Ereignis nur bei
Flussüberschreitung, `qualitaetFluss` konstant `true`, Peaksteigung unbenutzt).
M-05 bleibt der Transport; eine **neue Zeile M-86** (nächste freie ID) legt den
eigenständigen Peakpfad fest: Peaksteigung (Anstieg des Rahmenpeaks gegenüber
dem Vorrahmen in dB je Rahmen) und Crest erzeugen ein Ereignis auch ohne
Flussüberschreitung, mit `qualitaetFluss = false` und `qualitaetPeak = true`;
Flussereignisse tragen `qualitaetFluss = true`; lösen beide Pfade im selben
Rahmen aus, wird genau ein Ereignis mit beiden Bits abgelegt; Ring-Deckel 64 und
Verlustzähler bleiben; die Schwellen der Peaksteigung wählt der Bauer begründet
und misst sie am Golden.

**Geändert:** §2.1 (Ist-Stand korrigiert), M-05 (Transport, Abgrenzung zu
M-86), §3.1 um M-86 erweitert, §3.11 (86 Zeilen), §5.2.

**Tests:** B5 neu `short_impulse_triggers_peak_path_only`,
`flux_event_carries_flux_quality`, `both_paths_yield_one_event`.

### 8.7 D7 — Interchannel-Phase, Fensterdauer und Freiheitsgrade (M-11)

**Befund, wörtlich (Codex, P2, Manifestzeile um 448):**

> DEFEKT: M-11 lässt die im Entwurf vorgeschriebene Interchannel-Phase
> vollständig aus und bindet auch Fensterdauer und Freiheitsgrade nicht an die
> Evidenz. §40.1 verlangt beides ausdrücklich und erlaubt die
> Phaseninterpretation nur in ausreichend kohärenten Bändern
> (`docs/FL-Nakama-Sonden-Design-Entwurf.md:2632-2645`); keiner der genannten
> Tests misst diese Felder oder deren fail-closed Auswertung.

**Regel (03.09.2026):** M-11 nimmt auf: bandweise Interchannel-Phase,
ausgewertet nur in Bändern, deren Kohärenz eine benannte Schwelle überschreitet,
sonst `null`; Fensterdauer (ms) und Freiheitsgrade (Zahl der gemittelten
gültigen Welch-Frames) als Felder der Evidenz je Band; Kohärenz `null` bei zu
wenig Energie oder weniger als acht Frames. Die Felder reisen auf dem
Evidenzpfad (E-05) und werden in §3.8 als von SONDE-013 belegte Felder des
`evidence_snapshot` geführt.

**Geändert:** M-11 (Phase, Fensterdauer, Freiheitsgrade, zweistufiges
Fail-closed), §3.8 (Fassung 2 nennt die Felder mit).

**Tests:** `phase_only_in_coherent_bands`, `coherence_carries_window_and_dof`,
`coherence_is_null_below_eight_frames` (ersetzt
`coherence_needs_eight_welch_frames`).

### 8.8 Bilanz der Runde

| Defekt | Klasse | Regel greift in | Belegklasse danach |
|---|---|---|---|
| D1 | P1 Vertrag | §7.1 E-02, M-49, M-66, M-73, §3.8 | M-49 ENTSCHIEDEN |
| D2 | P1 Vertrag | §7.1 E-04, M-54, §3.8 | M-54 ENTSCHIEDEN |
| D3 | P1 Invalidierung | M-23, M-53 | beide BELEGT, BAULÜCKE |
| D4 | P1 Lebenszyklus | M-47, §7.1 E-03 | M-47 BELEGT, BAULÜCKE |
| D5 | P2 Retention | §7.1 E-03, M-48 | M-48 ENTSCHIEDEN |
| D6 | P1 Detektor | §2.1, M-05, neue Zeile M-86 | M-86 BELEGT, BAULÜCKE |
| D7 | P2 Evidenzfelder | M-11, §3.8 | M-11 BELEGT, BAULÜCKE |

Zählung nach der Runde (§3.11): 86 Zeilen — 4 vollständig gedeckt, 7 mit
BAULÜCKE nur im Bein, 71 mit BAULÜCKE in Implementierung und Messung, 4
ENTSCHIEDEN, 0 ENTSCHEIDEN-VORSCHLAG, 0 OFFEN. Gegenüber der geprüften Fassung:
eine Zeile mehr (M-86), vier Zeilen von ENTSCHEIDEN-VORSCHLAG nach ENTSCHIEDEN,
keine Zeile entfallen.

Keine Urteilsmarke und kein Belegfeld: diese Runde ist Nacharbeit an der
Spezifikation, kein Bau- und kein Prüfurteil. Das Urteil über die Matrix fällt
die Wiederprüfung 1 (§7.3).
