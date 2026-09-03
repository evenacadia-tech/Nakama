# SONDE-013 — S20–22: Dynamik, Stereo, PRE/POST, Passage und der manuelle Experimentkern

| Merkmal | Wert |
|---|---|
| Ticket | S20–22, `SONDE-013` (Phase P4–P5) |
| Phase | **Etappe 2 — Bau, Etappen A und B stehen; Etappe C ist die nächste.** Etappe 1 (Verhaltensmatrix) ist am 2026-09-04 nach Matrixprüfung 1, Nacharbeit 1, Wiederprüfung 1, Nacharbeit 2 und Wiederprüfung 2 (PASS) abgenommen; die Matrix in §3 ist ab hier die Spezifikation. Bauauftrag: `docs/beweise/roh/SONDE-013-etappe-2-auftrag.txt`, Bau-Verlauf in §10 |
| Urteil | **offen** — T1 setzt der Erbauer nach der letzten Bauetappe, T2 der Dirigent nach der Codex-Abnahme |
| Prüfstufe | T1+T2 gefordert (`docs/bauaufteilung-sonden.md`:392). T1 ist das Selbstaudit des Erbauers am Ende der letzten Bauetappe, T2 setzt der Dirigent nach der Codex-Abnahme |
| Basis-SHA | Etappe 1 begann auf `ed9bbf7fec951a061749abf143cb2158c1c4ee52`; **Etappe 2 baut ab `0fdbb4a09c75e2c93ab9b76e7fcf5d92d0ef17e4`** (Abschluss der Matrixetappe), beide mit `git rev-parse HEAD` gemessen |
| Kanon vorher | GRUEN 40/40 auf `a627ebb`, Rohausgabe `docs/beweise/roh/NAK-121-a627ebb.md`. `git diff --stat a627ebb..ed9bbf7 -- broker eq-copilot tools` ist **leer**: der Codestand von `ed9bbf7` ist mit dem beglaubigten Kanonstand identisch, die acht Commits dazwischen ändern ausschließlich Dateien unter `docs/` (`git diff --name-only a627ebb..ed9bbf7` liefert 8 Pfade, alle mit diesem Präfix). |
| Matrixprüfung 1 | Codex `gpt-5.6-sol`, Effort **high**, lesend, Thread `01a06922-fd1d-7222-b180-fcddb1fdfc2d`; `HEAD` vor und nach der Prüfung `367a0ea732c9e883df12f2bfc5abb2f67df6da5d`. Urteil **NEEDS_WORK**, sieben Defekte. Rohausgabe `docs/beweise/roh/SONDE-013-matrixpruefung-1-367a0ea.txt`, Auftrag `docs/beweise/roh/SONDE-013-matrixpruefung-1-auftrag.txt`. Alle sieben Befunde sind an der Quelle als Defekt bestätigt; Wortlaut, Regel und geänderte Zeilen stehen in §8. |
| Wiederprüfung 1 | Codex `gpt-5.6-sol`, Effort **high**, lesend, Thread `01a06942-e2a8-70b3-8bde-45ca621e859d`; `HEAD` `ca20f3ab9c757540b6b7da36576e43d6755bc3e0`. Der Lauf gibt **keine URTEIL-Zeile** aus; der Dirigent wertet ihn als **NEEDS_WORK**: alle sieben Befunde der Matrixprüfung 1 sind als geschlossen bestätigt, dafür stehen **zwei Defekte** (Versionswiderspruch M-66 gegen M-67, uneindeutiger Drop-Grund in M-53) und **eine Lücke** (M-86 ohne Bauetappe). Rohausgabe `docs/beweise/roh/SONDE-013-wiederpruefung-1-ca20f3a.txt`, Auftrag `docs/beweise/roh/SONDE-013-wiederpruefung-1-auftrag.txt`. Der Dokuriegel lief in diesem Codex-Lauf wegen eines lokalen Python-Startfehlers nicht; er wird lokal gefahren. Wortlaut, Regel und geänderte Zeilen stehen in §9. |
| Wiederprüfung 2 | Codex `gpt-5.6-sol`, Effort **high**, lesend, Thread `01a06955-10da-7892-b465-cd52b7119ac9`; `HEAD` vor und nach dem Lauf `6e8bebb3cae7d836c51bd7ae5362a69b06b72ace`; Prüfbereich Fixdiff `ca20f3a...6e8bebb -- docs/beweise/SONDE-013.md`. **URTEIL: PASS** — R2-1 (M-67 gegen v3-README und Register), R2-2 (M-23/M-53 gegen SONDE-009 und Schema) und R2-3 (§5.1/§5.2, M-01 bis M-86 je genau einer Etappe zugeordnet) geschlossen, nichts gebrochen. Auftrag `docs/beweise/roh/SONDE-013-wiederpruefung-2-auftrag.txt`, Rohausgabe `docs/beweise/roh/SONDE-013-wiederpruefung-2-6e8bebb.txt`. Damit ist Etappe 1 abgenommen (Budget: zwei von drei Nacharbeitsrunden verbraucht). |
| Rundenbilanz Etappe 1 | `py -3.13 tools/dirigent/rundenbilanz.py --runden ed9bbf7 4a2f50a 367a0ea ca20f3a 6e8bebb`: Matrix `ed9bbf7..4a2f50a` Doku 2 Dateien +1019/−0; Entscheide `4a2f50a..367a0ea` Doku +51/−0; Nacharbeit 1 `367a0ea..ca20f3a` Doku 4 Dateien +533/−63; Nacharbeit 2 `ca20f3a..6e8bebb` Doku 4 Dateien +286/−17. Produkt und Tests 0 Zeilen in allen vier Runden — erwartet, weil Etappe 1 nach der Regel „Spezifikation vor Code" ausschließlich das Manifest schreibt; das Konvergenzsignal des Werkzeugs greift erst ab Etappe 2. |
| Änderungssatz dieser Etappe | Erster Commit `4a2f50a` und Entscheidcommit `367a0ea`: dieses Manifest und `docs/beweise/roh/SONDE-013-etappe-1-auftrag.txt`. Nacharbeit 1: dieses Manifest sowie die drei Rohdateien `docs/beweise/roh/SONDE-013-matrixpruefung-1-auftrag.txt`, `docs/beweise/roh/SONDE-013-matrixpruefung-1-367a0ea.txt` und `docs/beweise/roh/SONDE-013-nacharbeit-1-auftrag.txt` unverändert. Nacharbeit 2: dieses Manifest sowie die drei Rohdateien `docs/beweise/roh/SONDE-013-wiederpruefung-1-auftrag.txt`, `docs/beweise/roh/SONDE-013-wiederpruefung-1-ca20f3a.txt` und `docs/beweise/roh/SONDE-013-nacharbeit-2-auftrag.txt` unverändert. Kein Produkt-, Test-, Schema-, Fixture- oder Werkzeugcode. |
| Kanon nachher | **GRUEN 40/40** auf `8d8fc96` (Etappe A), Rohausgabe `docs/beweise/roh/SONDE-013-8d8fc96-dirty.md`. Der Lauf stempelt `-dirty`, weil beim Baustand-Schnappschuss **eine** unbestätigte Datei im Baum lag — dieses Manifest, also Dokumentation; die Rohdatei nennt sie namentlich. Jede gemessene Prüfbinärdatei trägt „frisch (Bau bestätigt)" aus den Quellen von `8d8fc96`; kein Produkt-, Test-, Schema- oder Werkzeugcode war unbestätigt. Die Beinzahl bleibt 40 und ist um kein Bein gesunken. **Zweiter Lauf auf sauberem Baum** (`fcaa55c`, Rohausgabe `docs/beweise/roh/SONDE-013-fcaa55c.md`): **ROT 39/40** an einer zeitabhängigen Zusage in B10 (`parken_uebergeht_den_backoff/control/access_denied`, geparkt bei Versuch 3 statt 4). Dieselbe Binärdatei unmittelbar danach allein: 313/0 grün. Der Pfad liegt außerhalb der Ticketpfade und außerhalb des Änderungssatzes; Diagnose und Folge stehen in §10.1. |
| Testanzahl | A5 461 Prüfungen · B3c 75 · B10 313 · B16 40 (neu) · B5 237 · Broker 202 Lib-Tests plus alle Integrationsbeine · JSON-Fixturekorpus 285 (75 gültig, 210 ungültig) · Binärkorpus 104 (20 gültig, 84 ungültig) · Envelope-Korpus 37. Alle Zahlen aus dem Lauf dieser Sitzung, nicht abgeschrieben. |
| Änderungssatz Etappe 2 | Fortlaufend in §10 je Bauetappe geführt: Commits, Beine, Rotbeweise, Abweichungen von §5 und Nebenbefunde. |
| Grenze | Etappe 2 baut ausschließlich, was §3 zusagt. Prüfbereich sind die Ticketpfade aus §5.2; jede Datei außerhalb steht mit Begründung in §10. `docs/offene-punkte.md` und `docs/PLAN-STAND.md` bleiben unberührt — Nebenbefunde sammelt §10, der Dirigent zieht sie im Abschluss nach. |

> **Belegfeld gesetzt, Urteilsmarke noch nicht.** Seit dem ersten Commit der
> Etappe 2 (`65fa1db`) zeigt das `beleg`-Feld von Schritt `S20–22` in
> `docs/plan/plan.json` auf dieses Manifest — es beschreibt jetzt gebauten
> Stand, nicht mehr nur Spezifikation. Eine `NAKAMA-URTEIL`-Marke trägt es
> weiterhin **nicht**: T1 kommt erst, wenn alle acht Bauetappen stehen, T2
> vom Dirigenten nach der Codex-Abnahme. Bis dahin zeigt
> `docs/PLAN-STAND.md` den Schritt als in Arbeit und nicht als fertig.

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
| M-05 | Ereignisring hat Einträge; Evidenzsnapshot wird fällig | Diese Zeile ist **nur der Transport**. Der lokal **teilweise** gebaute `DynamicsEvent`-Strom reist als `evidence_snapshot.ereignisse`. Jedes Ereignis trägt Samplezeit, Stärke, Bandzentrum, Dauer und Qualität; die zwei Qualitätsbits trennen den SuperFlux-Flusspfad vom eigenständigen Peakpfad. Dass der Peakpfad heute überhaupt nicht auslöst, ist nicht diese Zeile, sondern **M-86**. Kadenz 1 bis 4 Hz; bei Überlast wird die Kadenz reduziert, nie der Ring stillschweigend geleert. Der Ring bleibt auf 64 Plätze gedeckelt und zählt seine Verluste. | **gemessen** (2026-09-04, Etappe B): **A5**::`evidence_snapshot_ereignisse_belegt`, `ereignisring_bleibt_bei_64_und_zaehlt_verluste`; **B16** `EqCopSonde013EventWireTest`::`events_travel_with_quality_and_loss_counter/alle_drei_reisen`, `…/beide_bits_getrennt`, `…/verlustzaehler`, `fremde_epoche_reist_nicht_und_wird_gezaehlt` sowie die Kadenzfälle; **A4** `coordinator_model`::`evidenzsnapshot_wird_angenommen_und_zusammengefasst`, `…_fremder_adresse_wird_verworfen`, `…_wird_bei_offener_intervention_gesperrt`, `…_der_fassung_2_faellt_bei_einem_leser_der_fassung_1`. Rotbeweise `docs/beweise/roh/SONDE-013-rot-M-05-verlust.txt`, `-M-05-verteilung.txt`, `-M-05-grenze.txt`, `-M-05-riegel.txt`. | Entwurf §39.1:2593-2600; §33.2:2041; `reservierte-nachrichten-v1.json` (Feldname für SONDE-013 verbrannt); `FeatureEngine.h`:290-313,448,1607-1650. **BELEGT, gemessen** (Etappe B, 2026-09-04) — Abgrenzung zu M-86 aus Nacharbeit 1 (§8, D6); der Peakpfad selbst bleibt M-86 in Etappe C |
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
| M-23 | Paar ist vollständig, aber die Überdeckung ist gering, die aktive Messzeit klafft auseinander oder die Hälften laufen in verschiedenen Prozessen | Herabstufung auf `probable` mit benanntem Grund statt Ausschluss. Im v3-Pfad sind es **genau drei** Gründe: Überlappung unter 80 % des kürzeren Fensters; aktive Messzeit um mehr als 10 % verschieden (Smart Disable oder Stille auf einem Punkt); verschiedene Host-PIDs. Der vierte Grund des v2-Vorläufers, „Loop-/Seek-Sprünge im Messfenster", **entfällt im v3-Pfad** und bleibt nur als Vorläufer benannt: eine Passage bindet an genau eine Transportepoche (§32.4), also **invalidiert** ein Sprung die Messung, statt sie herabzustufen; der Grund auf dem Draht ist dabei je Auslöser eindeutig — Seek und Loop-Wrap innerhalb der Passage `epochwechsel`, Drop `sequenzluecke` (M-53). Regel und Abgrenzung stehen in M-53. Keine Herabstufung ist ein stiller Ausschluss. | **NEU** `sonde013_prepost.rs`, Fall `each_downgrade_reason_falls_alone` mit einem Fall je Grund, also **drei**; **NEU** `sonde013_taint.rs`, Fälle `seek_inside_passage_invalidates_instead_of_downgrading` und `loop_wrap_at_passage_edge_starts_next_pass`; Formvorläufer bestehend `broker/src/lib.rs`:809-826 (dort vier Gründe — v2-Erbe, Verlauf). | `broker/src/lib.rs`:809-826 (v2-Vorläufer, Plan §5.7); Entwurf §58:4285 („Invalidierung bei Marker, Preview, Seek, Drop, geändertem Material oder Messpunkt"); §32.4:1969; §15 Vergleichbarkeitsregeln:1093-1098. **BELEGT, BAULÜCKE im v3-Pfad** — Sprunggrund gestrichen in Nacharbeit 1 (§8, D3) |
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
| M-53 | Evidenzfenster läuft; Seek, Loop-Wrap, Drop oder Kontinuitätsbruch tritt ein | Der Bruch beendet die alte Zeitreihe. Kein Fenster wird über die Grenze interpoliert; ein Drop erzeugt Zähler, sichtbare Lücke und eine neue Kontinuitätsgrenze. **Der Grund auf dem Draht ist je Auslöser eindeutig** und folgt der Zeitvertragsregel aus SONDE-009 („Drop zaehlt als Segment, alles andere als Epoche", B5 §F/G1-G2): ein **Seek** trägt `epochwechsel`; ein **Loop-Wrap innerhalb** der Passage trägt `epochwechsel`; jeder **andere Kontinuitätsbruch** trägt `epochwechsel`; ein **Drop** trägt `sequenzluecke`. Kein Auslöser trägt zwei Gründe, und keiner bleibt der Implementierung überlassen. Bis zur neuen gültigen Baseline gibt es keinen alten Wert und keine 0. **Passagenbindung (Nacharbeit 1):** Eine Passage bindet an genau eine Transportepoche (§32.4). Ein Seek, ein Loop-Wrap **innerhalb** der Passage (Loop kürzer als die Passage) oder ein Drop während der laufenden Passagenmessung invalidiert diese Messung — jeweils mit dem oben je Auslöser festgelegten Grund — und die Messung beginnt mit der nächsten gültigen Epoche neu; kein Frame, der eine Grenze überspannt, geht in eine Paarauswertung ein (B5-Garantie aus SONDE-009). Ein Loop-Wrap **an** der Passagengrenze (Loop deckt die Passage vollständig) beendet einen Durchlauf und beginnt den nächsten — das ist **keine** Invalidierung und **keine** Herabstufung (Abgrenzung zu M-23). | Bestehend **B5** für die lokale Fenstertrennung und **B4** für den Drop-Pfad; **NEU** `sonde013_taint.rs`, Fall `epoch_change_invalidates_open_evidence`; er misst den **Grundwert je Auslöser** — Seek → `epochwechsel`, Loop-Wrap innerhalb → `epochwechsel`, Drop → `sequenzluecke` —, sodass ein vertauschter Grund fällt statt grün zu werden. Dazu die zwei Passagenfälle aus M-23. | Entwurf §48.1:3413-3417; §34.2; §32.4:1969; §58:4285; `docs/beweise/SONDE-009.md`:4544,1954,1958 (B5-Zeitvertrag). **BELEGT, BAULÜCKE in der Invalidierungsnachricht** — Passagenbindung aus Nacharbeit 1 (§8, D3), Grundzuordnung je Auslöser aus Nacharbeit 2 (§9, R2-2) |
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
| M-64 | `table Frame` soll `integration_samples` tragen | Neues Feld an der nächsten freien Feld-ID **14**, mit expliziter ID im `.fbs`. Reihenfolge im **einen** Änderungssatz: `.fbs` ändern → `flatc` neu generieren für C++ und Rust → `FELD-IDS.json` nachziehen → Binärfixtures neu erzeugen → beide Leser klassifizieren identisch → `schema_minor` des Wire-Envelopes anheben. Ein alter Leser darf am neuen Feld nicht brechen; ein neuer Leser darf es bei Abwesenheit nicht als 0 lesen. | **gemessen** (2026-09-04, Etappe A): **A9** `pruefe_flatc_drift.py` (Drift 0, 52 Felder, jedes mit expliziter ID), **A10** `erzeuge_fb_fixtures.py`::`integration-samples-id14`, `-duenner-rahmen`, `-evidenzrahmen`, `-null`, **B3c** `EqCopSchemaTest`::`integration_samples_wird_von_beiden_lesern_klassifiziert`, `…_abwesend_bleibt_gueltig_und_ist_keine_null`, `…_null_ist_ein_senderfehler`. Rotbeweise `docs/beweise/roh/SONDE-013-rot-M-64.txt` (Feld-ID-Drift) und `docs/beweise/roh/SONDE-013-rot-M-64-leser.txt` (Regel aus dem Rust-Leser entfernt). | `.fbs`:216-239; `FELD-IDS.json`; `reservierte-nachrichten-v1.json` (`wire_envelope_schema_minor`). **BELEGT, gemessen** (Etappe A, 2026-09-04) |
| M-65 | `evidence_snapshot` soll `ereignisse` tragen | Das Objekt ist `additionalProperties: false` und sein Feldname für SONDE-013 reserviert. Das Belegen ist deshalb **kein additiver Schritt**: es braucht einen `schema_minor`-Bump der Familie P1, einen Eintrag in `belegte_felder` von `reservierte-nachrichten-v1.json`, ein Negativfixture und beidseitige Leser. Ein alter Empfänger, der das Feld nicht kennt, lehnt es nach heutigem Schema ab — die Fassungswahl über `message_family` und `schema_minor` im Envelope ist der einzige zulässige Weg. | **gemessen** (2026-09-04, Etappe A): **A5**::`evidence_snapshot_ereignisse_belegt`, `ereignisring_bleibt_bei_64_und_zaehlt_verluste`, `dynamics_ereignis_traegt_beide_qualitaetsbits`, `kohaerenz_traegt_fenster_und_freiheitsgrade`; **A8** `erzeuge_v3_fixtures.py`::`evidence-snapshot-mit-ereignissen-und-stereo` samt sieben Negativfixtures; **B3c** klassifiziert denselben Korpus. Rotbeweis `docs/beweise/roh/SONDE-013-rot-M-65.txt`. | `eq-ipc-v3.schema.json`:1002-1019; `reservierte-nachrichten-v1.json`. **BELEGT, gemessen** (Etappe A, 2026-09-04) |
| M-66 | Die Experimentfamilien dieses Tickets sollen definiert werden | **Drei** Familien: `experiment_begin` und `experiment_abort` wandern aus `reserviert` nach `definiert`; `experiment_manual_result` entsteht **neu und direkt in `definiert`** mit Eigentümer SONDE-013 (§7.1 E-02). Damit steigt `gesamt_erwartet` von **27 auf 28**, die Aufteilung geht von 18/9 auf **21/7**. `pruefe_v3_vertrag.py`:1073-1074 prüft `len(definiert) + len(reserviert) == gesamt_erwartet` gegen die Zahl im Register selbst — das Bein fällt, wenn eine der beiden Seiten nicht mitzieht. Alle drei Familien tragen wie jede steuernde Nachricht die Steuerfelder (`command_id` beziehungsweise `experiment_id`, Zieladresse, `base_revision`, begrenzte `ttl_ms`, Schema-/Capability-Version); die Deadline leitet der Empfänger aus seiner **eigenen** monotonen Uhr ab. Der Versionsschritt ist **Fassung 2** des `wire_envelope_schema_minor`, deren Eintrag alle drei Familien nennt. | **gemessen** (2026-09-04, Etappe A): **A5**::`zwei_familien_wandern_von_reserviert_nach_definiert`, `experiment_manual_result_definiert`, `summe_ist_28`; **B10** `EqCopIpcTest`::`experiment_familien_werden_von_der_cpp_engine_angenommen`, `fremder_execution_mode_faellt_an_derselben_engine`, `experiment_manual_result_ist_ein_persistenzpflichtiger_p0_befehl`. Rotbeweis `docs/beweise/roh/SONDE-013-rot-M-66.txt`. | `reservierte-nachrichten-v1.json`; `tools/eq-copilot/pruefe_v3_vertrag.py`:1063-1079; Entwurf §33.3:2051-2090. **BELEGT, gemessen** (Etappe A, 2026-09-04) — dritte Familie, Summe 28 und Aufteilung 21/7 aus Nacharbeit 1 (§8, D1) |
| M-67 | Ein neuer Vertragsteil trägt Discriminator, Zieladresse, Revision oder Capability | **Nicht additiv erweiterbar.** Das heißt: jede Erweiterung eines Discriminators, einer Zieladresse, einer Revision oder einer Capability ist ein **ausdrücklicher Versionsschritt mit Fassungswahl**, den ein Leser der alten Fassung **ablehnt** — nie eine stille Ergänzung. Welche Achse den Schritt trägt, hängt an der erweiterten Menge und ist keine Ermessensfrage: ein neuer oberster Discriminatorzweig kommt mit der **nächsten Fassung des `wire_envelope_schema_minor`** (in diesem Ticket Fassung 2, M-66); eine Änderung am **geschlossenen Capabilitysatz** ist `schema_major`. Beleg für die zweite Achse: `eq-copilot/schemas/v3/README.md` „Capabilities — und warum sie keine eigene Version tragen" — der Satz ist `additionalProperties: false` und alle zehn sind Pflicht, „damit ist jede Aenderung an ihm ein Major-Schritt, und `schema_major` IST seine Version"; SONDE-012 E-C01 meint mit „eine Wire-Erweiterung wäre NAK-27/Major" genau diesen Zehner-Satz, nicht die Familienliste. Beleg für die erste Achse: SONDE-012 hat die neue definierte Familie `session_command` — einen neuen obersten `oneOf`-Zweig (`eq-ipc-v3.schema.json`:16) — als **Fassung 1** des `wire_envelope_schema_minor` ausgeliefert (`reservierte-nachrichten-v1.json`:16-27, `session_command: true` in `fassungen` Eintrag `1`), nicht als Major-Schritt. Konkret betroffen in diesem Ticket: der Vergleichbarkeits-Umfang, der Alignment-Score (vier Klassen, M-16), die Verdikt-Menge (fünf Aussagen, M-46), der `execution_mode` und die `invalidate_umfang`-Arten. Jede dieser Mengen ist beim ersten Bau **abschließend** zu wählen. | **gemessen** (2026-09-04, Etappe A): **A5**::`fassung_2_nennt_jede_neuerung_dieses_tickets`, `fassung_1_leser_lehnt_neue_gruende_ab`, `fassung_1_leser_lehnt_die_drei_experimentfamilien_ab`, `fassung_1_leser_lehnt_ereignisse_und_stereo_ab`, `unbekannter_grund_wird_abgelehnt`, `Gegenprobe: der Rueckbau auf Fassung 1 aendert das Schema wirklich`; je ein **A8**-Negativfixture pro neuem Discriminator (`experiment-begin-fremder-execution-mode`, `-behauptet-wiederherstellbarkeit`, `experiment-abort-fremder-grund`, `experiment-manual-result-fremdes-urteil`, `-fremde-blindreihenfolge`, `experiment-referenz-fremde-alignmentklasse`, `-fremde-messpunktklasse`), von **B3c** in derselben Klassifikation gemessen. Rotbeweis `docs/beweise/roh/SONDE-013-rot-M-67.txt`. | `eq-copilot/schemas/v3/README.md`:328-350; `reservierte-nachrichten-v1.json`:7-29; `eq-ipc-v3.schema.json`:16; CLAUDE.md („Für v3 sind Discriminator, Zieladresse, Revision und Capability nicht additiv erweiterbar"); SONDE-012 E-C01. **BELEGT, gemessen** (Etappe A, 2026-09-04) — Versionsregel korrigiert in Nacharbeit 2 (§9, R2-1) |
| M-68 | Passagen und Experimente sollen persistent werden | Die Tabellen existieren bereits aus Migration 1 (`passages`, `experiments`, `experiment_events`, `user_verdicts`). Braucht dieses Ticket eine Spalte oder einen Index mehr, ist das **Migration 2** mit Vor- **und** Rückweg im selben Änderungssatz; `migration_1_checksum()` verriegelt den Text der ersten Migration, ein nachträglicher Eingriff dort fällt auf. Ein alter Datenbankeintrag darf nie einen neueren Plugin-State zurücksetzen. | Bestehend **A4** und **A4-SI** für Migration 1 und die Killmatrix; **NEU** `sonde013_store.rs`, Fall `migration_2_has_forward_and_backward_path`, falls Migration 2 nötig wird. | `broker/src/store/migration.rs`:68-135,150-155; Entwurf §33.5:2155-2160. **BELEGT**; **BAULÜCKE nur im Bein**, falls Migration 2 nötig wird — das entscheidet der Bau in Etappe 2 |
| M-69 | Eine manuelle Passage soll mit dem FL-Projekt reisen | `MainProjectState` ist autoritativ für manuelle Passagen (§33.5). Damit gilt die State-Invariante: neues persistentes Feld zuerst versionieren, Altstände laden, unbekannte Felder zerstören keinen alten Consumer, Save und Load **gemeinsam** testen, und jede persistente Änderung meldet dem Host Dirty-State. Unabhängige Teilversionen für Main-Project-State und Probe-DSP bleiben getrennt. | Bestehend **B2** `EqCopStateMigrationTest` und **A12** `erzeuge_state_fixtures.py`; **NEU** `EqCopSonde013PassageStateTest`, Fälle `save_load_roundtrip_is_byte_identical` und `unknown_newer_field_stays_read_only_with_original_bytes`. | Entwurf §33.5:2135-2142,2155-2160; CLAUDE.md State-Invariante. **BELEGT, BAULÜCKE** |
| M-70 | Ein Vertrag ändert sich in einer Sprache | Writer, Reader, Migration, Fixtures und **beide** Cross-Language-Verbraucher liegen im selben Änderungssatz. Die vier Beine, die das messen: **A5** (JSON-Vertrag samt Abdeckung), **A8** (JSON-Fixturekorpus bytegleich), **A10** (Binärfixturekorpus bytegleich), **B3c** (C++ klassifiziert wie das Manifest, Urteil **und** Verletzungsmenge). Ein Fixture ohne Verbraucher in beiden Sprachen ist ein Befund. | **gemessen** (2026-09-04, Etappe A): **A5** 461/0, **A8** 287 Dateien bytegleich, **A10** 105 Dateien bytegleich, **B3c** 75/0 — der Fassungsschritt lag in EINEM Änderungssatz (Commit `8d8fc96`), Schema, Register, `.fbs`, Codegen, Fixtures und beide Leser gemeinsam. | Prüfliste §F; Entwurf §33.1; SONDE-005-Gate. **BELEGT, gemessen** (Etappe A, 2026-09-04) — muss in jeder folgenden Etappe erfüllt bleiben |
| M-71 | Generierter Code weicht vom `.fbs` ab | Codegen-Drift ist 0: die Neugenerierung aus dem `.fbs` ist bytegleich zum committeten C++- und Rust-Code, `flatc`, C++-Header und Rust-Crate tragen dieselbe gepinnte Version, und jedes Tabellenfeld trägt eine explizite Feld-ID. Ein neues Feld ohne ID fällt hier. | **gemessen** (2026-09-04, Etappe A): **A9** `pruefe_flatc_drift.py` — Drift 0 über beide erzeugten Dateien nach der Neugenerierung für Feld-ID 14, 52 Felder, keines ohne ID. | `tools/beweise.ps1`:497. **BELEGT, gemessen** (Etappe A, 2026-09-04) — muss in jeder folgenden Etappe erfüllt bleiben |
| M-72 | Eine Zusage dieses Tickets ändert sich | Drei Stellen werden nachgezogen: Runnerkopf `tools/beweise.ps1`, Skriptkopf des betroffenen Werkzeugs und der lebende Kopf dieses Manifests. Ein `git grep` nach dem Kernbegriff genügt als Nachweis. Eine Runner-Behauptung sagt nicht mehr, als der Test misst, und nicht mehr, als das Gate verlangt. | **gemessen** (2026-09-04, Etappe A): drei Stellen nachgezogen — Runnerkopf `tools/beweise.ps1` (Behauptungen von A5, A10, B3c, B10), Skriptkopf `tools/eq-copilot/pruefe_v3_vertrag.py` (vierte Aufgabe: der Fassungsschritt) und dieser Manifestkopf. Der Kanonlauf verweigert die Beglaubigung weiterhin bei veralteten Prüfbinaries. | Prüfliste §E; `tools/beweise.ps1`. **BELEGT, gemessen** (Etappe A, 2026-09-04) — Verfahrensregel für jede weitere Etappe |
| M-73 | Ein reservierter Name soll belegt werden | Der Vertragsanteil, der bricht, wenn man ihn offen lässt, ist der **Name**, nicht die Nutzlast. Belegte Namen wandern mit Eigentümer und Grund in `belegte_nachrichten` beziehungsweise `belegte_felder`; ein reservierter Discriminator wird bis dahin vom v3-Parser abgelehnt. Kein Ticket belegt einen Namen, dessen Eigentümer ein anderes Ticket ist — das betrifft hier `experiment_result` (SONDE-017) und `user_verdict` (SONDE-014). Der Vertrag kennt zudem nur **ganze** reservierte Familien, keine teilreservierten Discriminatorzweige (`pruefe_v3_vertrag.py`:1063-1079): ein halb belegter fremder Name ist kein gültiger Zwischenzustand. | **gemessen** (2026-09-04, Etappe A): **A5**::`fremde_eigentuemer_bleiben_unberuehrt`, `experiment_result_bleibt_reserviert_fuer_sonde017` — beide prüfen zusätzlich, dass weder `experiment_result` noch `user_verdict` ein `$defs` bekommt, auch nicht als Präfix eines anderen Namens. | `reservierte-nachrichten-v1.json`; `tools/eq-copilot/pruefe_v3_vertrag.py`:1063-1079; Bauaufteilung §6.2 („Kein Vorgriff"). **BELEGT, gemessen** (Etappe A, 2026-09-04) — in Nacharbeit 1 bestätigt und um die Teilzweigregel geschärft (§8, D1) |

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
> Steuerkopf, Fassung 2 des `wire_envelope_schema_minor`. Überholt ist damit
> auch die Einordnung „Major-Schritt" im Vorschlag unten: der Schritt läuft
> über die Fassung des `wire_envelope_schema_minor`, nicht über `schema_major`
> (§9, R2-1).

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
| **A — Vertragsrunde** | `experiment_begin` und `experiment_abort` definieren, die neue Familie `experiment_manual_result` definieren und im Register verbrennen (§7.1 E-02), `evidence_snapshot.ereignisse` belegen (samt Kohärenzfeldern aus M-11), `grund`-Enum um `material_wechsel` und `messpunkt_wechsel` erweitern, `gesamt_erwartet` auf 28, **Fassung 2** des `wire_envelope_schema_minor` als **einen** Schritt, `integration_samples` als Feld-ID 14 auf der P2-Fläche, alle Fixtures und beide Leser. Matrix M-64 bis M-67, M-70 bis M-73. | A5, A8, A9, A10, A20, B3c, B10, A4 | — (E-02 und E-04 sind in §7.1 entschieden) |
| **B — Evidenzpfad** | `evidence_snapshot` bekommt Sender in der Sonde und Empfänger im Broker; Kadenz 1 bis 4 Hz mit Reduktion bei Überlast; P10/P50/P95, Abdeckung, Konvergenz. Matrix M-05. Diese Etappe ist Voraussetzung für M-11 (Etappe C) und M-19 (Etappe F), übernimmt sie aber nicht. | A4, A5, A22, B3c, B10 | §7.1 E-05 |
| **C — Metriken in der Sonde** | Momentary 400 ms, True Peak nach M-02, PLR und PSR nach M-03, mehrfenstriger Crest, LRA mit 60-s-Regel, bandweise M/S und Kohärenz, `integration_samples` füllen, **eigenständiger Peakpfad** im Detektor (Peaksteigung und Crest lösen auch ohne Flussüberschreitung aus, M-86). Matrix M-01 bis M-04, M-06 bis M-12, M-74 bis M-77, M-86. M-05 ist der Transport desselben Ereignisstroms und liegt bei Etappe B. | A1, A16, B4, B5, B9, B12, neu `EqCopSonde013DynamicsTest`, `EqCopSonde013StereoGoldenTest`, `EqCopSonde013TruePeakGoldenTest`, dazu die drei **B5**-Fälle des Peakpfads `short_impulse_triggers_peak_path_only`, `flux_event_carries_flux_quality` und `both_paths_yield_one_event` (M-86) | §7.1 E-06 (Metriken entstehen in der Sonde) |
| **D — Interventions-Producer und NAK-47** | Vorallokierter RT→Control-Ring, `audible_intervention_begin/end` aus dem Plugin, erzwungener Ausfade statt `hartAus()`, `recording=false` im `erlaubt`-Term. Matrix M-33 bis M-39, M-58, M-78. | A1, A3, A4, A16, B10, neu `EqCopSonde013InterventionRingTest`, A22 und A24 mit aktivem Ring (M-78) | §7.1 E-01 (nur der Oversize-Zweig) |
| **E — Passage und Fingerprint** | Passageobjekt, Fingerprint, Store-Anbindung, `MainProjectState`-Persistenz mit Save/Load-Symmetrie und Host-Dirty. Matrix M-25 bis M-32, M-68, M-69. | A4, A4-SI, A12, B2, B4, neu `EqCopSonde013PassageStateTest`, `EqCopSonde013FingerprintGoldenTest` | — |
| **F — PRE/POST-Join** | v3-Paarbildung, Alignment mit vier Klassen, Coverage, Herabstufungen, `raw_project_frame_key` gegen `aligned_frame_key`. Matrix M-13 bis M-24. | A4, A22, A24, anzulegen `broker/tests/sonde013_prepost.rs`, `EqCopSonde013PrePostGoldenTest` | — |
| **G — Experiment `manual_external`** | Begin, Abort, Ergebnisweg über `experiment_manual_result` (§7.1 E-02), Match-Gain, Blindbindung, vier Auswertungsachsen, fünf Verdikte, zwei Bestandsdeckel statt Wanduhr (§7.1 E-03), Rekonstruktion über Sitzungsende und Neustart hinweg. Matrix M-40 bis M-51, M-59. | A4, A4-SI, A5, anzulegen `broker/tests/sonde013_experiment.rs`, `EqCopSonde013ExperimentUiTest`, `EqCopSonde013ExperimentGoldenTest` | — (E-02 und E-03 sind in §7.1 entschieden) |
| **H — Invalidierung, Taint und Korpus** | `evidence_invalidate`-Sender, sitzungsweite Quarantäne, inverse Pfade über alle vier `art`-Werte, P4-Referenzkorpus mit Erzeugerskript. Matrix M-52 bis M-57, M-60 bis M-63, M-79 bis M-85. | **voller Kanon** plus anzulegen `broker/tests/sonde013_taint.rs`, die anzulegenden `tools/eq-copilot/pruefe_p4_korpus.py` und `tools/eq-copilot/erzeuge_p4_korpus.py` | — |

Etappen C, E und F sind untereinander unabhängig und können in beliebiger
Reihenfolge laufen; H setzt alle voraus.

**Jede der 86 Matrixzeilen liegt in genau einer Etappe** (geprüft in
Nacharbeit 2, §9, R2-3). Vier davon sind Regressionswachen, die schon heute
erfüllt sind: M-70, M-71 und M-72 stehen bei **A**, weil dort der Vertrag,
der `.fbs` und die Werkzeugköpfe zuerst angefasst werden, M-76 bei **C**;
alle vier müssen am Ende **jeder** folgenden Etappe grün bleiben. Das ist
keine zweite Zuordnung, sondern die Zusage der Zeilen selbst.

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
Prüfbereich: für den Transport des Ereignisstroms (M-05, Etappe B) **und** für
den eigenständigen Peakpfad, der heute fehlt (M-86, Etappe C; §8, D6 und §9,
R2-3). Den Peakpfad messen die drei **B5**-Fälle
`short_impulse_triggers_peak_path_only`, `flux_event_carries_flux_quality` und
`both_paths_yield_one_event`; ohne sie ist Etappe C nicht abgeschlossen.

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
Konvergenzentscheid beginnt Etappe 2 mit einem frischen Opus-Worker. Die
Wiederprüfung 1 ist inzwischen gefahren; ihr Ergebnis und der daraus folgende
nächste Schritt stehen in §9.

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
Passagenmessung invalidiert diese Messung (M-53); die Messung beginnt mit der
nächsten gültigen Epoche neu, und kein Frame, der eine Grenze überspannt, geht in eine
Paarauswertung ein (B5-Garantie aus SONDE-009). Ein Loop-Wrap an der
Passagengrenze (Loop deckt die Passage vollständig) beendet einen Durchlauf und
beginnt den nächsten; das ist keine Herabstufung. Die drei übrigen
Herabstufungsgründe bleiben `probable`: Überdeckung unter 80 % des kürzeren
Fensters, aktive Messzeit um mehr als 10 % verschieden, verschiedene Host-PIDs.

**Geändert:** M-23 (drei Gründe statt vier, Ausgangszustand entsprechend),
M-53 (Passagenbindung und die Abgrenzung des Loop-Wraps an der Grenze).

**Nachgeschärft in Nacharbeit 2 (§9, R2-2):** Welchen Grund welcher Auslöser
trägt, stand hier zunächst als „`epochwechsel` beziehungsweise
`sequenzluecke`" und war damit nicht entscheidbar. Die eindeutige Zuordnung —
Seek und Loop-Wrap innerhalb der Passage `epochwechsel`, Drop `sequenzluecke` —
steht jetzt in M-53 und wird dort auch gemessen.

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
die Wiederprüfung 1 (§7.3) — gefahren, Ergebnis in §9.

---

## 9. Nacharbeit 2 nach Wiederprüfung 1 (2026-09-03)

Wiederprüfung 1: Codex `gpt-5.6-sol`, Effort high, lesend, Thread
`01a06942-e2a8-70b3-8bde-45ca621e859d`, `HEAD` vor und nach der Prüfung
`ca20f3ab9c757540b6b7da36576e43d6755bc3e0`. Der Lauf gibt **keine
URTEIL-Zeile** aus; der Dirigent wertet ihn als **NEEDS_WORK**. Er bestätigt
alle sieben Befunde der Matrixprüfung 1 als geschlossen und erhebt **zwei
Defekte** und **eine Lücke**. Rohausgabe
`docs/beweise/roh/SONDE-013-wiederpruefung-1-ca20f3a.txt`, Auftrag
`docs/beweise/roh/SONDE-013-wiederpruefung-1-auftrag.txt`, Auftrag dieser Runde
`docs/beweise/roh/SONDE-013-nacharbeit-2-auftrag.txt`. Der Dokuriegel konnte im
Codex-Lauf wegen eines lokalen Python-Startfehlers des Prüfers nicht laufen; er
ist für diese Runde lokal gefahren.

Alle drei Punkte sind vom Dirigenten an der Quelle geprüft; je Punkt stehen
unten der Wortlaut, die Regel und die geänderte Stelle.

### 9.1 R2-1 — Versionsschritt eines neuen Discriminatorzweigs (M-66 gegen M-67)

**Befund, wörtlich (Codex, P1, Manifestzeile 551):**

> DEFEKT: Sobald `experiment_manual_result` in `definiert` aufgenommen wird,
> erzwingt `pruefe_namen` (Zeilen 1064–1069) dafür einen neuen obersten
> `oneOf`-/`type`-Discriminatorzweig. M-66 versioniert diesen hier mit
> `schema_minor = 2`, während M-67 unmittelbar danach für jeden neuen
> Discriminatorzweig ausdrücklich einen Major-Schritt und keinen Minor
> verlangt; Etappe A kann daher beide Matrixzeilen nicht gleichzeitig erfüllen.
> Das widerspricht zudem der strikten v3-Vertragsinvariante in CLAUDE.md
> [im Rohtext als Verweis auf `CLAUDE.md`:196-199].

Der Mechanismus ist an der Quelle bestätigt: `pruefe_v3_vertrag.py`:1064-1069
liest die `oneOf`-Zweige des Schemas und vergleicht sie mit `definiert` — jeder
neu definierte Name zieht also genau einen neuen obersten Zweig nach sich.

**Regel (03.09.2026): M-66 ist richtig, M-67 wird korrigiert.** Der Vertrag
kennt **zwei** Versionsachsen, und M-67 hatte sie zusammengeworfen.

- `eq-copilot/schemas/v3/README.md`:328-350 („Capabilities — und warum sie
  keine eigene Version tragen") schreibt den Major-Schritt ausschließlich für
  Änderungen am **geschlossenen Capabilitysatz** vor: er ist
  `additionalProperties: false`, alle zehn Namen sind Pflicht, „damit ist jede
  Aenderung an ihm ein Major-Schritt, und `schema_major` IST seine Version".
  SONDE-012 E-C01 meint mit „eine Wire-Erweiterung wäre NAK-27/Major" genau
  diesen Zehner-Satz.
- Für einen neuen **Discriminatorzweig** gilt statt dessen der Präzedenzfall
  aus SONDE-012: die neue definierte Familie `session_command` ist ein neuer
  oberster `oneOf`-Zweig (`eq-ipc-v3.schema.json`:16) und wurde als **Fassung
  1** des `wire_envelope_schema_minor` ausgeliefert
  (`reservierte-nachrichten-v1.json`:16-27), nicht als Major-Schritt.
- „Nicht additiv erweiterbar" (`CLAUDE.md`:198-199) heißt deshalb **nicht**
  „immer Major", sondern: jede Erweiterung eines Discriminators, einer
  Zieladresse, einer Revision oder einer Capability ist ein ausdrücklicher
  Versionsschritt mit Fassungswahl, den ein Leser der alten Fassung **ablehnt**
  — nie eine stille Ergänzung. Welche Achse den Schritt trägt, entscheidet die
  erweiterte Menge: Discriminator ergibt die nächste Fassung des
  `wire_envelope_schema_minor` (hier Fassung 2), der Capabilitysatz ergibt
  `schema_major`.

**Verlauf — der ersetzte Wortlaut von M-67:**

> **Nicht additiv erweiterbar.** Ein neuer Discriminatorzweig ist ein
> Major-Schritt, kein Minor.

Unverändert bleiben in M-67 die Aufzählung der betroffenen Mengen
(Vergleichbarkeits-Umfang, Alignment-Score, Verdikt-Menge, `execution_mode`,
`invalidate_umfang`-Arten) und die Zusage, sie beim ersten Bau abschließend zu
wählen.

**Geändert:** M-67 (Zusage, Test- und Quellenspalte). M-66 bleibt unverändert.

**Tests:** die Negativfixtures je neuem Discriminator bleiben; neu kommt der
Fassungsnachweis selbst hinzu — ein Leser der **Fassung 1** lehnt die **Fassung
2** ab (Negativfixture in **A5** und **A8**, **B3c**-Zwilling), statt sie still
auf einen bekannten Zweig abzubilden. In M-66 war dieser Fall nicht genannt;
der Fließtext von §3.8 verlangte ihn bereits.

### 9.2 R2-2 — Drop trägt genau einen Invalidierungsgrund (M-53)

**Befund, wörtlich (Codex, P2, Manifestzeile 528):**

> DEFEKT: Für einen Drop während der Passagenmessung verlangt M-53 zunächst
> pauschal `epochwechsel`, weil Drop in der Auslöserliste der ersten Aussage
> steht, nennt später aber `epochwechsel` beziehungsweise `sequenzluecke`. Da
> beide Werte laut `eq-ipc-v3.schema.json:1092` gültig sind und die geplanten
> Tests den konkreten Grund nicht prüfen, kann eine widersprüchliche
> Implementierung trotzdem grün werden; die Zuordnung für Drop muss innerhalb
> der Zeile eindeutig sein.

An der Quelle bestätigt: `eq-copilot/schemas/v3/eq-ipc-v3.schema.json`:1092
führt `sequenzluecke` und `epochwechsel` beide in derselben geschlossenen
`grund`-Enum — die Zeile konnte also beides meinen.

**Regel (03.09.2026):** Die Zuordnung folgt dem bereits bewiesenen Zeitvertrag
aus SONDE-009 — „Drop zaehlt als Segment, alles andere als Epoche"
(`docs/beweise/SONDE-009.md`:4544, gemessen in **B5** §F als G1 „Drop
(kFlagLueckeDavor): zaehlt als SEGMENT, nicht als Epoche" und G2 „Seek waehrend
Wiedergabe: zaehlt als EPOCHE, nicht als Segment"). Daraus folgt eindeutig:

| Auslöser | Grund auf dem Draht |
|---|---|
| Drop | `sequenzluecke` |
| Seek | `epochwechsel` |
| Loop-Wrap **innerhalb** der Passage | `epochwechsel` |
| jeder andere Kontinuitätsbruch | `epochwechsel` |

M-53 nennt die Zuordnung jetzt in einem Satz je Auslöser, ohne
„beziehungsweise". Dieselbe Zuordnung steht in M-23 und in §8.3.

**Geändert:** M-53 (Zusage und Testspalte), M-23 (Grund je Auslöser benannt),
§8.3 (das mehrdeutige „beziehungsweise" entfernt und die Stelle als
nachgeschärft gekennzeichnet).

**Tests:** `epoch_change_invalidates_open_evidence` misst den **Grundwert je
Auslöser** — Seek zu `epochwechsel`, Loop-Wrap innerhalb zu `epochwechsel`,
Drop zu `sequenzluecke`. Ein vertauschter Grund fällt, statt grün zu werden.

### 9.3 R2-3 — M-86 und vier weitere Zeilen ohne Bauetappe (§5.1)

**Befund, wörtlich (Codex, P2, Manifestzeile 462):**

> LÜCKE: Die neue Zeile M-86 schließt D6 in der Matrix, wird aber von keiner
> Bauetappe in §5.1 erfasst: Etappe C nennt weiterhin nur M-01 bis M-12 und
> M-74 bis M-77, und die späteren Etappen überspringen M-86 ebenfalls. Dadurch
> kann der Etappenplan abgeschlossen werden, ohne den eigenständigen Peakpfad
> und seine drei B5-Fälle zu bauen; M-86 muss samt Tests der
> Metrik-/FeatureEngine-Etappe zugeordnet werden.

**Regel (03.09.2026, Lücke vom Dirigenten entschieden):** M-86 gehört zu
**Etappe C** („Metriken in der Sonde"); die Etappenzeile nennt M-86 und die
drei B5-Fälle `short_impulse_triggers_peak_path_only`,
`flux_event_carries_flux_quality` und `both_paths_yield_one_event`. §5.2 führt
dieselben drei Fälle an der Stelle, an der `FeatureEngine.h` doppelt im
Prüfbereich steht.

Die zugleich verlangte Vollständigkeitsprüfung „jede Matrixzeile M-01 bis M-86
in genau einer Etappe" hat neben M-86 **vier weitere unzugeordnete Zeilen** und
**eine Doppelzuordnung** gefunden:

| Zeile | Vorher | Jetzt | Warum dort |
|---|---|---|---|
| M-70 | keine Etappe | **A** | Writer, Reader, Migration, Fixtures und beide Cross-Language-Verbraucher liegen im selben Änderungssatz — der entsteht in der Vertragsrunde |
| M-71 | keine Etappe | **A** | Codegen-Drift 0 (**A9**); der einzige `.fbs`-Eingriff dieses Tickets, `integration_samples` als Feld-ID 14, liegt in A |
| M-72 | keine Etappe | **A** | Runnerkopf, Skriptkopf und Manifestkopf werden zuerst in A angefasst |
| M-78 | keine Etappe | **D** | der Zweig, der neu gemessen wird, ist **A22** und **A24** mit **aktivem Interventionsring** — der Ring entsteht in D |
| M-86 | keine Etappe | **C** | eigenständiger Peakpfad im Detektor der Sonde |
| M-05 | **B und C** | **B** | M-05 ist ausdrücklich „nur der Transport"; der Bereichsausdruck „M-01 bis M-12" bei C hatte ihn stillschweigend mitgenommen |

Damit liegen alle 86 Zeilen in genau einer Etappe, ohne Lücke und ohne
Doppelung. M-70, M-71, M-72 und M-76 sind Regressionswachen: sie stehen an der
Etappe ihres ersten Eingriffs und müssen am Ende **jeder** folgenden Etappe
grün bleiben — das ist die Zusage der Zeilen selbst, keine zweite Zuordnung.
Eine neue Zusage ist nicht entstanden; geändert wurde ausschließlich die
Zuordnung.

**Geändert:** §5.1 (Etappen A, B, C und D sowie der Satz unter der Tabelle),
§5.2 (die drei B5-Fälle des Peakpfads benannt).

### 9.4 Bilanz der Runde

| Punkt | Klasse | Regel greift in | Stand danach |
|---|---|---|---|
| R2-1 | P1 Vertrag | M-67 | M-67 nennt beide Versionsachsen und deckt sich mit M-66 |
| R2-2 | P1 Invalidierung | M-53, M-23, §8.3 | Grund je Auslöser eindeutig und gemessen |
| R2-3 | P2 Bauplan | §5.1, §5.2 | 86 von 86 Zeilen in genau einer Etappe |

Die **Zählung in §3.11 ändert sich nicht**: es ist keine Matrixzeile
hinzugekommen, entfallen oder in eine andere Belegklasse gewandert — R2-1 und
R2-2 schärfen den Wortlaut zweier Zusagen, R2-3 ordnet bestehende Zeilen
Bauetappen zu. Weiterhin 86 Zeilen, 4 vollständig gedeckt, 7 mit BAULÜCKE nur
im Bein, 71 mit BAULÜCKE in Implementierung und Messung, 4 ENTSCHIEDEN, 0
ENTSCHEIDEN-VORSCHLAG, 0 OFFEN.

Keine Urteilsmarke und kein Belegfeld: auch diese Runde ist Nacharbeit an der
Spezifikation, kein Bau- und kein Prüfurteil. Nächster Schritt ist die
**Wiederprüfung 2** durch einen erneut frischen, lesenden Codex-Thread über §3,
§4, §5, §7, §8 und §9 gegen Gate-Text, Entwurf, Verträge und Code am neuen
`HEAD`. Erst nach PASS oder Konvergenzentscheid beginnt Etappe 2 mit einem
frischen Opus-Worker.

---

## 10. Bau-Verlauf Etappe 2 (ab 2026-09-04)

Append-only. Je Etappe: Commits, Tests, Kanonlauf, Abweichungen von §5 mit
Begründung, Nebenbefunde außerhalb des Tickets als Zeilen für
`docs/offene-punkte.md` (die Datei selbst rührt diese Etappe nicht an; der
Dirigent zieht sie im Abschluss nach). Die Matrix in §3 ist ab hier die
Spezifikation: was dort nicht steht, wird nicht gebaut.

### 10.1 Etappe A — Vertragsrunde (2026-09-04)

**Commits**

| SHA | Was |
|---|---|
| `65fa1db` | Belegfeld `S20–22` in `docs/plan/plan.json` auf `docs/beweise/SONDE-013.md`; Kopfzeile Phase auf die Bauphase. `docs/PLAN-STAND.md` unberührt |
| `8d8fc96` | Der ganze Vertragsänderungssatz: Schema, Register, `.fbs`, Codegen, `FELD-IDS.json`, beide Leser, beide Fixturekorpora, drei Prüfwerkzeuge, zwei C++-Beine, Runnerbehauptungen, fünf Rotbeweise |

**Gebaute Matrixzeilen:** M-64, M-65, M-66, M-67 (Vertragsfolgen) sowie die
vier Regressionswachen M-70, M-71, M-72, M-73. Alle acht tragen in §3 jetzt
die Belegklasse **BELEGT, gemessen** und in der Testspalte den konkreten
Fall samt Rotbeweisdatei.

**Was der Vertrag jetzt trägt**

- **Drei Familien** statt zwei: `experiment_begin` und `experiment_abort`
  wandern aus `reserviert` nach `definiert`, `experiment_manual_result`
  entsteht neu und direkt in `definiert` (§7.1 E-02). `gesamt_erwartet`
  27 → 28, Aufteilung 18/9 → **21/7**. `experiment_result` bleibt
  vollständig bei SONDE-017, `user_verdict` bei SONDE-014.
- **`evidence_snapshot.ereignisse`** ist belegt: `liste` (0…64 Einträge, der
  Ringdeckel aus §48.1) plus `verloren`. Der Verlustzähler steht **im** Feld,
  damit ein Empfänger „nichts passiert" von „verdrängt" unterscheiden kann.
  Jedes Ereignis trägt Samplezeit, Stärke, Bandzentrum, Dauer und **beide**
  Qualitätsbits — die Trennung von Fluss- und Peakpfad, die M-86 in Etappe C
  auslösbar macht.
- **`evidence_snapshot.stereo`** trägt die bandweise Stereoevidenz aus §40.1:
  Mid/Side/Seitenanteil in dB, Korrelation kurz und mittel, Kohärenz, Phase,
  Persistenz, Zeitperzentile, gemessener Mono-Folddown und L/R-Balance, dazu
  **je Band** Fensterdauer und Freiheitsgrade. Kohärenz und Phase tragen ihr
  Gültigkeitsbit nur unter den Bedingungen aus §40.1; die zwei Stufen sind
  Erzeugerdisziplin und werden in Etappe C am Golden gemessen, hier
  ausdrücklich **nicht** behauptet.
- **Zwei neue Invalidierungsgründe** `material_wechsel` und
  `messpunkt_wechsel`, ohne Steuerkopf im Payload (§7.1 E-04).
- **`integration_samples`** an Feld-ID **14** der P2-Fläche (NAK-68).

**Der Riegel dieser Etappe** ist der Fassungsschritt selbst.
`pruefe_v3_vertrag.py` baut aus der committeten Fassung 2 die **Fassung 1**
zurück — die Liste dafür ist der `fassungen`-Eintrag im Register, keine
zweite Kopie, die driften könnte — und misst, dass der alte Leser jede
Neuerung **ablehnt**. Dazu die Gegenprobe, dass der Rückbau überhaupt etwas
entfernt: ein Rückbau, der nichts ändert, machte jede Zeile darüber wertlos.

**Belege aus dieser Sitzung**

| Bein | Ergebnis |
|---|---|
| **A5** `pruefe_v3_vertrag.py --abdeckung` | 461 bestanden, 0 gescheitert; 21 Familien, 72 Definitionen, 285 Fixtures; `const` 15/15 und `enum` 22/22 HART, keine harte Lücke, keine Definition ohne Negativfixture |
| **A8** `erzeuge_v3_fixtures.py --pruefen` | 287 Dateien bytegleich (75 gültig, 210 ungültig) |
| **A9** `pruefe_flatc_drift.py` | Drift 0 über beide erzeugten Dateien; 52 Felder, keines ohne explizite ID |
| **A10** `erzeuge_fb_fixtures.py --pruefen` | 105 Dateien bytegleich (20 gültig, 84 ungültig) |
| **A20** `erzeuge_envelope_fixtures.py --pruefen` | 38 Dateien bytegleich, 14 Regeln mit je einem Negativfall |
| **B3c** `EqCopSchemaTest` | 75 bestanden, 0 gescheitert; beide Korpora klassifiziert wie das Manifest |
| **B10** `EqCopIpcTest` | 313 bestanden, 0 gescheitert |
| **A4** `cargo test` (Broker, alle Ziele) | 198 Lib-Tests plus alle Integrationsbeine grün |
| **Voller Kanon** `tools/beweise.ps1 -Bauen` | **GRUEN 40/40** auf `8d8fc96`, Rohausgabe `docs/beweise/roh/SONDE-013-8d8fc96-dirty.md`; 2 geplante Beine noch nicht gebaut (B6, B7 ab P6), 1 stillgelegtes (A15). Zum `-dirty`-Stempel siehe den Kopf: die einzige unbestätigte Datei war dieses Manifest |

**Rotbeweise** (`docs/beweise/roh/`)

| Datei | Eingebauter Fehler | Fallendes Bein |
|---|---|---|
| `SONDE-013-rot-M-64.txt` | Feld-ID von `integration_samples` im Register auf 15 statt 14 | A9 |
| `SONDE-013-rot-M-64-leser.txt` | Regel `integration_samples_null` aus dem Rust-Leser entfernt | `contract_cross_language` |
| `SONDE-013-rot-M-65.txt` | belegtes Feld `ereignisse` wieder aus `evidence_snapshot` entfernt | A5 |
| `SONDE-013-rot-M-66.txt` | `gesamt_erwartet` zurück auf 27 bei 28 geführten Familien | A5 |
| `SONDE-013-rot-M-67.txt` | Rückbau auf Fassung 1 lässt `material_wechsel` stehen — der alte Leser nähme den neuen Grund still an | A5 |

Jede Datei trägt beide Läufe: den roten mit Fehler und den grünen ohne ihn,
mit demselben Befehl und vollständiger Rohausgabe.

**Zweiter Kanonlauf auf sauberem Baum — und was er gefunden hat.** Weil der
erste Lauf `-dirty` gestempelt war (die eine unbestätigte Datei war dieses
Manifest), lief der Kanon auf `fcaa55c` mit **sauberem Arbeitsbaum** erneut.
Ergebnis: **ROT, 39 von 40**. Das einzige gefallene Bein ist **B10**, und
darin genau eine Prüfung:

```
FEHLER  parken_uebergeht_den_backoff/control/access_denied
        [Aufstieg 495 997 ms, geparkt 1 bei Versuch 3, weitere Versuche 0,
         Neustart nach 5 ms, Versuch DANACH nach 497 ms
         (erwartet 350..1200, ohne Reset >= 4000)]
```

Die Prüfung erwartet, dass der Backoff-Aufstieg beim **vierten** Versuch
geparkt wird; unter Last parkte er beim **dritten**. Sie ist zeitabhängig,
nicht inhaltlich. Dieselbe Binärdatei, unmittelbar danach allein gefahren:

```
ok  parken_uebergeht_den_backoff/control/access_denied
    [Aufstieg 496 996 ms, geparkt 1 bei Versuch 4, … Versuch DANACH nach 496 ms]
ALLE PRUEFUNGEN GRUEN — 313 Pruefungen, 0 Fehler
```

**Warum das kein Befund dieses Tickets ist:** der Pfad ist der
Auth-/Backoff-Aufstieg des `ControlClient`; der Änderungssatz der Etappe A
fasst ihn nicht an (`tools/dirigent/rundenbilanz.py` und der Diff zeigen
Schema, Register, `.fbs`, Codegen, Leser, Fixtures, Prüfwerkzeuge und zwei
Testdateien). Der erste Lauf hat dieselbe Prüfung mit derselben Binärdatei
bestanden, der Einzellauf danach ebenfalls.

**Was daraus folgt, nicht was bequem wäre:** Etappe A ist damit **nicht** auf
einem sauberen Baum grün beglaubigt. Der belastbare Stand ist: 40 von 40
grün auf `8d8fc96` bei einer unbestätigten Dokumentationsdatei, plus die
Einzelmessung von B10 auf dem Endstand. Der nächste vollständige Kanonlauf am
Ende der Etappe B ist die Beglaubigung für beide Etappen. Die zeitabhängige
Zusage selbst wird **nicht** in diesem Ticket gelockert — sie liegt außerhalb
der Ticketpfade aus §5.2, und eine Toleranz nebenbei zu verschieben wäre
genau der Neben-Refactor, den `CLAUDE.md` ausschließt. Sie steht unten als
Nebenbefund.

**Rundenbilanz** `py -3.13 tools/dirigent/rundenbilanz.py 0fdbb4a..HEAD`:
Produkt 53 Dateien +3966/−34, Tests 2 Dateien +149/−0, Prüfwerkzeug 4 Dateien
+557/−10, Doku 7 Dateien +395/−3. Der große Produktanteil ist erwartet: er
besteht überwiegend aus den 40 neuen Fixturedateien und dem neu erzeugten
FlatBuffers-Code, nicht aus handgeschriebenem Produktpfad.

**Abweichungen von §5, mit Begründung**

- **`preset_hash` nicht gebaut.** M-42 nennt „optional einen vom User
  bereitgestellten Preset- oder Screenshot-Hash" als Inhalt des
  Versuchs. Der bindende Entscheid §7.1 E-02 zählt die Nutzlast von
  `experiment_manual_result` dagegen **abschließend** auf und nennt ihn nicht.
  Ein Feld zu erfinden, das der Entscheid nicht trägt, wäre genau der
  Vertragsvorgriff, den D1 gerade zurückgenommen hat. Das Feld entsteht
  deshalb **nicht** in diesem Ticket; es steht unten als Nebenbefund.
- **`stereo_bandwerte` ohne `saturated`.** Der bestehende `bandwerte`-Zweig
  für float32 führt die Sättigungsmarke mit. Sie gehört zur Quantisierung und
  kann bei float32 nie `true` werden; sie mitzuführen wäre ein totes Feld auf
  der Leitung (CLAUDE.md, keine toten Elemente). Die drei neuen Bandsätze
  tragen sie deshalb nicht.
- **Drei statt einer Bandsatz-Definition.** `stereo_bandwerte` (dB),
  `stereo_bandwerte_normiert` ([−1, 1]) und `stereo_bandwerte_phase`
  (±π mit Rundungsspielraum) statt eines unbeschränkten Satzes. Eine
  Korrelation über 1 oder eine Phase außerhalb ±π ist ein Erzeugerfehler und
  soll auf der Leitung fallen, nicht erst in der Anzeige.

**Dateien außerhalb der Ticketpfade aus §5.2, mit Begründung.** M-64 und M-70
verlangen Writer, Reader, Codegen und Fixtures im **selben** Änderungssatz.
Die Liste in §5.2 nennt die Vertrags- und Produktdateien, nicht die
Leser-/Encoderdateien, die derselbe Satz zwingend mitzieht. Betroffen sind:

| Datei | Warum sie im Satz liegen muss |
|---|---|
| `eq-copilot/plugin/vertrag/NakamaTelemetrie.h`, `.cpp` | der C++-Leser. M-64 verlangt „beide Leser klassifizieren identisch" — ohne ihn gäbe es kein zweites Urteil zur neuen Regel `integration_samples_null` |
| `broker/src/telemetrie.rs` | derselbe Leser auf der Rust-Seite, mit wortgleicher Regel |
| `eq-copilot/plugin/core/ipc/TelemetryClient.cpp` | der Encoder. Ein Feld ohne Schreiber wäre ein Vertragsteil ohne Erzeuger |
| `eq-copilot/plugin/vertrag/generiert/…_generated.h`, `broker/src/generiert/…_generated.rs` | Codegen aus dem gepinnten `flatc`. M-71 verlangt Drift 0, also müssen sie mit dem `.fbs` reisen |
| `broker/src/transport/server_v3/mod.rs` | ein Testfall trug die Familienzahlen 18/9 als Literal und fiel am Fassungsschritt — genau seine Aufgabe. Nachgezogen auf 21/7, keine Produktänderung |
| `eq-copilot/fixtures/v3/**` | beide Fixturekorpora samt MANIFEST, von A8/A10 bytegleich geprüft |

Kein weiterer Pfad ist angefasst; `docs/offene-punkte.md` und
`docs/PLAN-STAND.md` bleiben unberührt.

**Nebenbefunde für `docs/offene-punkte.md`** (Datei in dieser Etappe nicht
angefasst)

- **[SONDE-013 · P4]** `experiment_manual_result` trägt keinen vom User
  bereitgestellten Preset- oder Screenshot-Hash, obwohl §43.1 ihn als
  optionalen Beleg der Fremdänderung nennt (Matrix M-42). Der bindende
  Entscheid §7.1 E-02 zählt die Nutzlast abschließend auf und enthält ihn
  nicht; ihn zu ergänzen wäre ein eigener Fassungsschritt am
  `wire_envelope_schema_minor`. Kein Produktcode hängt heute daran.
  Eigentümer offen; sinnvoll gemeinsam mit der Bedienfrage P-02.
- **[Kanon · B10]** `parken_uebergeht_den_backoff/…/access_denied` in
  `eq-copilot/plugin/tests/IpcTestMain.cpp` ist **zeitabhängig** und fällt
  unter Last: es erwartet das Parken beim vierten Backoff-Versuch, unter Last
  parkt der Aufstieg beim dritten. Am 04.09.2026 im zweiten Kanonlauf
  (`fcaa55c`) einmal rot, im ersten Lauf und im Einzellauf derselben
  Binärdatei grün. Der Pfad gehört nicht zu SONDE-013; eine Toleranz hier
  nebenbei zu verschieben wäre ein Neben-Refactor. Wer es angeht: die Zusage
  auf „geparkt spätestens bei Versuch N" umstellen statt auf eine exakte
  Versuchsnummer, oder den Aufstieg an eine injizierbare Uhr hängen wie
  `WorkerCadence`.
- **[SONDE-013 · P4]** `broker/src/transport/server_v3/mod.rs` trug die
  Familienzahlen 18/9 als Literal in einem Testfall
  (`join_kandidat_laeuft_ueber_probe_pipe_ohne_join_candidate_nachrichtenfamilie`).
  Der Fall fiel beim Fassungsschritt und wurde auf 21/7 nachgezogen — richtig
  so, denn er ist genau die Wache dagegen, dass Schema und Register
  auseinanderlaufen. Auffällig bleibt, dass **dieselbe** Summe an drei Orten
  steht (Register, A5, dieser Rust-Test); ein vierter Ort wäre einer zu viel.

**Stand nach Etappe A:** Etappe A ist fertig und gepusht. Nächste Etappe ist
**B — Evidenzpfad** (M-05: Sender in der Sonde, Empfänger im Broker, Kadenz
1 bis 4 Hz mit Reduktion bei Überlast, P10/P50/P95, Abdeckung, Konvergenz).
Der Vertrag dafür steht seit dieser Etappe vollständig.

---

### 10.2 Etappe B — Evidenzpfad (2026-09-04)

**Gebaute Matrixzeile:** M-05. Sie ist laut §3 „nur der Transport" — der
Detektor selbst (M-86) bleibt Etappe C. Gebaut wurde deshalb der ganze Weg,
den es bis hierher **überhaupt nicht gab**: Erzeuger in der Sonde, Empfänger
im Broker, und die drei Inhalte, die §33.2 dem Snapshot zuschreibt und die
kein Liveframe trägt — Verteilungspunkte, Abdeckung, Konvergenz.

**Was vorher fehlte, und wie weit.** An `ed9bbf7` nennt `evidence_snapshot`
außer der Schemadatei **keine einzige** Zeile Produktcode, weder in
`broker/src` noch in `eq-copilot/plugin`. Das Feld `verteilung` ist im Schema
Pflicht, hatte aber keinen Erzeuger; die 221 Evidenzbänder wurden in der
`FeatureEngine` gefüllt und liefen nirgendwohin.

#### Ein Fund, der Etappe A betrifft

Beim Bau des Empfängers fiel auf, dass die **Fassung 2 aus Etappe A gar nicht
lesbar** war — und schlimmer, dass die Fassung 1 sie stillschweigend
angenommen hätte:

- `broker/src/coordinator/schema.rs` führte `JSON_SCHEMA_MINOR_AKTIV = 1`.
  Eine Nachricht mit `schema_minor = 2` hätte keinen Leser gefunden.
- Dieselbe Datei initialisierte `MINOR_1` aus `v3_schema_wurzel()` — dem
  **committeten**, also seit Etappe A dem Fassung-2-Schema. Der „Leser der
  Fassung 1" hätte damit `experiment_begin` und die zwei neuen
  Invalidierungsgründe **angenommen**. Genau das Gegenteil von M-67.
- `v3_schema_minor_0_wurzel()` setzte ebenfalls auf der committeten Fassung
  auf und erbte den Fehler eine Ebene tiefer.
- Auf der C++-Seite stand `kJsonSchemaMinor = 1`, und `SourcesModel.cpp`
  benutzte **dieselbe** Konstante für zwei verschiedene Fragen: „ist das die
  neueste Fassung?" und „trägt der Snapshot die Mitgliedshülle?". Ein Bump
  hätte die zweite Frage still umgedeutet und Minor-1-Snapshots in keinen
  Zweig fallen lassen.

Warum A5 das nicht gefunden hat: `pruefe_v3_vertrag.py` baut sich seine
Fassung 1 **selbst** zurück und misst die Regel an diesem Rückbau. Die Regel
stimmte; der Leser, der sie durchsetzen soll, war ein anderer. Der Unterschied
zwischen „die Regel testen" und „das Ding testen, das sie implementiert" ist
genau dieser Fund.

**Behoben** in diesem Änderungssatz, mit Rotbeweis:

- `JSON_SCHEMA_MINOR_AKTIV = 2`, neuer `MINOR_2`-Zweig.
- `v3_schema_minor_1_wurzel()` baut die Fassung 2 zurück; die Rückbauten sind
  jetzt eine **Kette** (2 → 1 → 0), jede Fassung nimmt genau ihre eigenen
  Zusätze zurück.
- Die Rückbauliste steht als Daten (`FASSUNG_2_FAMILIEN` und Nachbarn) und
  wird von `fassung_2_liste_deckt_sich_mit_dem_register` gegen den
  `fassungen."2"`-Eintrag des Registers geprüft. Drei handgepflegte Kopien
  derselben Liste sind hier schon einmal auseinandergelaufen (T2-Runde 2,
  BF-5); die Abweichung fällt jetzt am Riegel.
- C++: `kJsonSchemaMinor = 2` **plus** die neue, getrennte Konstante
  `kJsonSchemaMinorSessionHuelle = 1`. Die Formfrage hängt an der Fassung, die
  die Form eingeführt hat, nicht an der jeweils neuesten.

#### Was der Evidenzpfad jetzt kann

**Verteilung (P10/P50/P95).** Je Band ein fester Ring der jüngsten
`kVerteilungPlaetze = 64` Fensterwerte; am Rahmenende wird eine Kopie sortiert
und exakt interpoliert. Kein Histogramm: dessen Genauigkeit wäre eine
Behauptung über die Binbreite, keine Messung. Der Frame trägt
`evidenzFenster` — die Zahl der Fenster hinter dem **schwächsten** gezeigten
Punkt, also das Minimum über die Bänder mit Bit. Dieselbe Ehrlichkeit wie
`integration_samples` bei den Rahmenskalaren.

**Abdeckung.** Aktive Analysefenster durch alle Analysefenster des
Evidenzfensters — gemessenes Signal, nicht Wanddauer (§48.2). Beide Zähler
stehen nebeneinander, weil einer allein „keine Fenster gesehen" nicht von
„nur Stille gesehen" trennen könnte.

**Konvergenz.** Je Band der Abstand zwischen dem Median der ersten und dem der
zweiten Hälfte des behaltenen Fensters, gemittelt und über
`kKonvergenzSpanneDb` auf [0, 1] abgebildet. Bänder mit weniger als vier
Werten gehen nicht ein: zwei Mediane aus je einem Wert sind kein
Konvergenzbeleg.

**Kadenz 1 bis 4 Hz.** Der Evidenzabstand ist einstellbar und geklemmt. Die
Reduktion liegt in der **Engine**, nicht beim Sender — wer einen fälligen
Snapshot einfach nicht sendete, würde sein Fenster trotzdem leeren und die
Messung wegwerfen. Bei Rückstau (`zurWiederholung`/`abgewiesen`) verdoppelt
die Sonde den Abstand Richtung 1 Hz, sonst halbiert sie ihn Richtung 4 Hz.

**Ereignisstrom.** Der Snapshot trägt die Ereignisse des Fensters mit beiden
Qualitätsbits und einem Verlustzähler. Es reisen nur Ereignisse **dieser**
Epoche und **dieses** Segments und nur mit gültigem Anker; alles andere zählt
als verloren, statt an einer erfundenen Null zu hängen. Ein leerer Strom ohne
Verlust lässt das Feld ganz weg — Abwesenheit heißt „diese Quelle liefert
keine Ereignisse", ein leerer Strom **mit** Verlust reist.

**Konfidenzklasse der Sonde.** Höchstens `mittel`. §34.3 begrenzt die
Gesamtklasse an harten Mängeln bei Session, Passage, Coverage oder Alignment;
von diesen vier kennt eine Sonde nur die Coverage. `stark` von einer Sonde
wäre eine Aussage über drei Dinge, die sie nicht sieht.

**Empfänger im Broker.** Vertrag, Adresse gegen den sendenden Link, dann der
Interventionsriegel `evidence_dispatch()` — der VOR dem Commit liegt und sein
Urteil unter demselben Lock zählt (M-63). Gesperrt heißt verworfen, nicht
abgeschwächt gespeichert.

**Belege aus dieser Sitzung**

| Bein | Ergebnis |
|---|---|
| **B16** `EqCopSonde013EventWireTest` (NEU) | 40 Prüfungen, 0 gescheitert |
| **A5** `pruefe_v3_vertrag.py --abdeckung` | 461 bestanden, 0 gescheitert |
| **A4** `cargo test` (Broker) | 202 Lib-Tests (198 + 4 Fassungsleiter) plus alle Integrationsbeine grün; `coordinator_model` 50 Tests |
| **B3c** `EqCopSchemaTest` | 75 bestanden, 0 gescheitert |
| **B5** `EqCopAnalysisGoldenTest` | 237 bestanden, 0 Fehler — einschließlich der um drei Träger erweiterten Vollständigkeitsprobe |
| **B12** `EqCopSonde012LoudnessSourceTest` | 4 bestanden, 0 gescheitert, 0 Audiothread-Allokationen |
| **B13/B14/B11** (Fassungswechsel) | 41/41, 10/10, 9 bestanden |

**Rotbeweise** (`docs/beweise/roh/`)

| Datei | Eingebauter Fehler | Fallendes Bein |
|---|---|---|
| `SONDE-013-rot-M-05-verlust.txt` | Verlustzähler auf 0 festgenagelt | B16 |
| `SONDE-013-rot-M-05-verteilung.txt` | P10 und P95 auf den Median gelegt | B16 |
| `SONDE-013-rot-M-05-grenze.txt` | Verteilungsring überlebt die Transportgrenze | B5 und B16 |
| `SONDE-013-rot-M-05-riegel.txt` | Interventionsriegel vor dem Evidence-Commit übersprungen | `coordinator_model` |
| `SONDE-013-rot-M-67-leser.txt` | Leser der Fassung 1 lädt wieder das committete Schema | `fassungsleiter` |

Der letzte ist der wichtigste: er ist genau der Fehler, den diese Etappe
gefunden hat, und beweist, dass der neue Riegel ihn fängt.

**Drei Befunde am eigenen Test, die erst der Lauf gezeigt hat**

1. **Stack statt Heap.** Der Verteilungsring als Feld im Objekt (rund 58 KiB)
   sprengte in B5 den 1-MiB-Stack — die Zwillingsprobe G13 hält zwei Engines
   nebeneinander. `STATUS_STACK_OVERFLOW`, reproduzierbar. Er liegt jetzt im
   Heap wie `kurzZellen`, `ereignisse` und `flussHistorie` daneben, angelegt
   in `vorbereiten()`; der Audiothread alloziert weiterhin nie.
2. **Ein FFT-Fenster über einer kurzen Stille ist kein stilles Fenster.** Der
   erste Abdeckungsfall schaltete alle 50 ms zwischen Ton und Stille — bei
   4096 Samples Fensterlänge (85 ms) nimmt jedes Fenster das Aktivgate, und
   die Abdeckung blieb bei exakt 1,000. Der Fall läuft jetzt mit 1 Hz
   Evidenzkadenz und 250-ms-Blöcken.
3. **Nach dem Snapshot ist der Träger leer — das ist der Vertrag.** Der
   Grenzfall prüfte den Ring unmittelbar nach der Veröffentlichung und fand
   ihn erwartungsgemäß leer, hätte also nichts gemessen. Er füllt jetzt erst
   ein Stück des nächsten Fensters.

**Abweichungen von §5, mit Begründung**

- **`konfidenz.verteilung_fenster` ist KEIN Fassungsschritt.** Die Zahl
  hinter den Perzentilen musste irgendwo reisen. `evidence_snapshot` ist
  strikt, ein Pflichtfeld dort wäre eine zweite Fassung gewesen — und §3.8
  sagt, SONDE-013 hebt den Minor **genau einmal**. `konfidenz` ist dagegen im
  Vertrag ausdrücklich additiv (§34.3: „die bestehenden Komponenten *werden
  erweitert um*", `additionalProperties: true`). Ein weiteres Feld dort ist
  das, wofür die Additivität da ist. `maxProperties` steigt entsprechend von
  14 auf 15 (deklarierte + 8, die Regel prüft A5 selbst).
- **Neue Datei außerhalb §5.2.** `eq-copilot/plugin/vertrag/NakamaEvidenz.*`
  ist ein Vertragsserialisierer, dieselbe Rolle wie `NakamaTelemetrie` für
  die FlatBuffers-Seite, und liegt deshalb neben ihr. Die Alternative wäre
  die JSON-Form im Sondenprozessor **und** im Test gewesen — zwei Kopien
  derselben Wire-Form, genau die Drift aus BF-5.
- **Neues Kanonbein B16.** Die Beinzahl wächst von 40 auf 41. §5.1 nennt für
  Etappe B nur bestehende Beine; ohne ein eigenes Bein hätte der Erzeuger
  aber keinen Messpunkt — B3c misst den Vertrag, nicht die Frage, ob der
  Snapshot mehr sagt als der Liveframe.

**Dateien außerhalb der Ticketpfade aus §5.2, mit Begründung**

| Datei | Warum |
|---|---|
| `broker/src/coordinator/{schema,senke,zustand,mod}.rs`, `evidenz.rs` (neu) | Empfänger und Fassungsleiter. §5.2 nennt vom Coordinator nur `intervention.rs`; ein `evidence_snapshot`-Empfänger kann nur dort liegen, wo P1 verteilt wird |
| `eq-copilot/plugin/core/ipc/WireEnvelope.h` | die zwei Fassungskonstanten. Ohne sie reist die Fassung 2 nicht |
| `eq-copilot/plugin/src/SourcesModel.cpp` | vier Vergleiche, die bisher „neueste Fassung" mit „trägt die Mitgliedshülle" verwechselten |
| `eq-copilot/plugin/vertrag/NakamaEvidenz.*`, `eq-copilot/plugin/CMakeLists.txt` | der Serialisierer und seine Anbindung |
| `eq-copilot/plugin/tests/Sonde013EventWireTest.cpp` | das neue Bein — `eq-copilot/plugin/tests/` steht in §5.2 |

**Nebenbefunde für `docs/offene-punkte.md`**

- **[SONDE-013 · P4]** Die Fassungsnummer der P1-Verträge steht jetzt an
  **vier** Orten: `reservierte-nachrichten-v1.json` (`aktuell`),
  `pruefe_v3_vertrag.py` (`fassung_1_schema`), `broker/src/coordinator/schema.rs`
  (`JSON_SCHEMA_MINOR_AKTIV` und die Rückbaukette) und
  `eq-copilot/plugin/core/ipc/WireEnvelope.h` (`kJsonSchemaMinor`). Drei davon
  prüfen sich gegen das Register; die C++-Konstante nicht. Wer es angeht: ein
  Bein, das `kJsonSchemaMinor` gegen `wire_envelope_schema_minor.aktuell`
  hält — dieselbe Form wie `fassung_2_liste_deckt_sich_mit_dem_register`.
- **[SONDE-013 · P4]** Der Evidenzsnapshot reist heute zum Broker und wird
  dort zusammengefasst, aber **nicht** an Gen weitergereicht. Der
  Outbox-/Snapshot-Pfad dafür gehört den Etappen F und G; bis dahin ist die
  Sicht `evidenz_sicht()` nur über den Coordinator lesbar. Kein Produktcode
  hängt heute daran.

**Stand nach Etappe B:** Etappe B ist fertig. Nächste Etappe ist **C —
Metriken in der Sonde** (M-01 bis M-04, M-06 bis M-12, M-74 bis M-77, M-86).
Der Evidenzpfad, den C zum Transport seiner bandweisen Stereoevidenz braucht,
steht seit dieser Etappe.

---

## Kanon-Lauf - SONDE-013

**Lauf:** 2026-09-04 00:47 | **Runner:** `tools/beweise.ps1` | **Urteil:** GRUEN - 40/40 Kanon-Laeufe bestanden | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 0 | **Rohausgabe:** [roh/SONDE-013-8d8fc96-dirty.md](roh/SONDE-013-8d8fc96-dirty.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-04 00:47:26 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | 8d8fc96 SONDE-013 Etappe 2 A: Vertragsrunde — Fassung 2, drei Experimentfamilien, integration_samples ID 14 |
| Commit (voll) | 8d8fc9663c7098f14ce7577b71e4b649ba65bd9f |
| Arbeitsbaum | 1 unbestaetigte Datei(en) - dieser Lauf beweist NICHT allein den Commit |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,09 s | [A1](roh/SONDE-013-8d8fc96-dirty.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,62 s | [A2](roh/SONDE-013-8d8fc96-dirty.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,47 s | [A3](roh/SONDE-013-8d8fc96-dirty.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 74,88 s | [A4](roh/SONDE-013-8d8fc96-dirty.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 12,68 s | [A4-SI](roh/SONDE-013-8d8fc96-dirty.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 65,72 s | [A4b](roh/SONDE-013-8d8fc96-dirty.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 2,37 s | [A5](roh/SONDE-013-8d8fc96-dirty.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,13 s | [A6](roh/SONDE-013-8d8fc96-dirty.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,12 s | [A7](roh/SONDE-013-8d8fc96-dirty.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,20 s | [A8](roh/SONDE-013-8d8fc96-dirty.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,24 s | [A9](roh/SONDE-013-8d8fc96-dirty.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen und integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,47 s | [A10](roh/SONDE-013-8d8fc96-dirty.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,24 s | [A11](roh/SONDE-013-8d8fc96-dirty.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,25 s | [A12](roh/SONDE-013-8d8fc96-dirty.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,19 s | [A13](roh/SONDE-013-8d8fc96-dirty.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 5,02 s | [A14](roh/SONDE-013-8d8fc96-dirty.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 1,01 s | [A17](roh/SONDE-013-8d8fc96-dirty.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 33,30 s | [A18](roh/SONDE-013-8d8fc96-dirty.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [A19](roh/SONDE-013-8d8fc96-dirty.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,29 s | [A20](roh/SONDE-013-8d8fc96-dirty.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,33 s | [A21](roh/SONDE-013-8d8fc96-dirty.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 10,00 s | [A22](roh/SONDE-013-8d8fc96-dirty.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,57 s | [A24](roh/SONDE-013-8d8fc96-dirty.md#a24) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,65 s | [A23](roh/SONDE-013-8d8fc96-dirty.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,10 s | [A16](roh/SONDE-013-8d8fc96-dirty.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,12 s | [B1](roh/SONDE-013-8d8fc96-dirty.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,10 s | [B2](roh/SONDE-013-8d8fc96-dirty.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,05 s | [B3](roh/SONDE-013-8d8fc96-dirty.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,09 s | [B3b](roh/SONDE-013-8d8fc96-dirty.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,31 s | [B3c](roh/SONDE-013-8d8fc96-dirty.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,45 s | [B4](roh/SONDE-013-8d8fc96-dirty.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [B9](roh/SONDE-013-8d8fc96-dirty.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,12 s | [B5](roh/SONDE-013-8d8fc96-dirty.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [OK] Exit 0 | 213,97 s | [B10](roh/SONDE-013-8d8fc96-dirty.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 0,13 s | [B11](roh/SONDE-013-8d8fc96-dirty.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,57 s | [B12](roh/SONDE-013-8d8fc96-dirty.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,10 s | [B13](roh/SONDE-013-8d8fc96-dirty.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,20 s | [B14](roh/SONDE-013-8d8fc96-dirty.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,14 s | [B15](roh/SONDE-013-8d8fc96-dirty.md#b15) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,12 s | [B8](roh/SONDE-013-8d8fc96-dirty.md#b8) |


---

## Kanon-Lauf - SONDE-013

**Lauf:** 2026-09-04 01:03 | **Runner:** `tools/beweise.ps1` | **Urteil:** ROT - 1 von 40 Kanon-Laeufen fehlgeschlagen | 2 geplante Pruefung(en) noch nicht gebaut | 1 stillgelegte(s) Bein(e), siehe Uebersicht | **Exitcode:** 2 | **Rohausgabe:** [roh/SONDE-013-fcaa55c.md](roh/SONDE-013-fcaa55c.md)

### Kopf - woran gemessen wurde

| Feld | Wert |
|---|---|
| Zeitpunkt | 2026-09-04 01:03:30 +02:00 |
| Rechner | SCHUBBINATOR200 \| Windows 10.0.26200.0 |
| Zweig | master |
| Commit | fcaa55c SONDE-013 Etappe 2 A: Manifest nachgezogen — Matrixzeilen gemessen, Kanon 40/40, Paragraph 10 angelegt |
| Commit (voll) | fcaa55cdb5f19f2f4876ca5e8aaf667fdabc2497 |
| Arbeitsbaum | sauber |
| JUCE gepinnt | 8.0.9 |
| JUCE auf Platte | 8.0.9-dirty |
| FL Studio | FL Studio 2025 25.2.5.5319 \| FL Studio 2026 26.1.4.5589 |
| cargo | cargo 1.93.1 (083ac5135 2025-12-15) |
| rustc | rustc 1.93.1 (01f6ddf75 2026-02-11) |
| PowerShell | 7.6.5 |
| cmake | cmake version 3.31.6-msvc6 |

### Uebersicht

| # | Behauptung | Befehl | Ergebnis | Dauer | Rohausgabe |
|---|---|---|---|---|---|
| A1 | Passthrough ist bitgleich; 0 Samples Latenz, 0 Tail; NaN/Inf werden gezaehlt, aber nicht veraendert. | `eq-copilot\build\plugin\EqCopNullTest_artefacts\Release\EqCopNullTest.exe` | [OK] Exit 0 | 0,09 s | [A1](roh/SONDE-013-fcaa55c.md#a1) |
| A2 | AnalyseEngine deckt sich mit der eingefrorenen Offline-Referenz (Fixture-SHA-256 als Determinismus-Riegel). | `eq-copilot\build\plugin\EqCopGoldenTest_artefacts\Release\EqCopGoldenTest.exe eq-copilot\fixtures` | [OK] Exit 0 | 9,84 s | [A2](roh/SONDE-013-fcaa55c.md#a2) |
| A3 | Hoer-Markierung bleibt verriegelt: Render/Freilauf bitgleich, Analyse-Abgriff sitzt vor der Faerbung, und der U10-Term ist gedeckt - OHNE Playhead faerbt bei sonst voller Erlaubnis kein Sample (T11, mit Gegenprobe bei laufendem Transport). | `eq-copilot\build\plugin\EqCopMarkierungTest_artefacts\Release\EqCopMarkierungTest.exe` | [OK] Exit 0 | 6,51 s | [A3](roh/SONDE-013-fcaa55c.md#a3) |
| A4 | Broker-Vertragstests: eingefrorene v2/v3-Vertraege sowie SONDE-011 Phase B mit Coordinator/Session, monotoner Liveness/Eviction, SQLite-Migration 1, Single-Writer, Projektionen, Snapshot-Outbox, dauerhaften Konfliktriegeln, produktiver v2+v3-Verdrahtung und der nicht isolationspflichtigen Killmatrix. | `cargo test --manifest-path broker/Cargo.toml --color never` | [OK] Exit 0 | 71,06 s | [A4](roh/SONDE-013-fcaa55c.md#a4) |
| A4-SI | SONDE-011 Phase-B-Systemintegration auf Probe-Pipenamen: echter C++-ControlClient wiederholt persistenzpflichtige Befehle ueber Brokerkills mit derselben command_id; Store/Coordinator liefern nur absolute session_snapshot-Pushes, koaleszieren Snapshot-Schuld, halten Locks aus externer Arbeit heraus und bereinigen Eviction/Nonce vor spaeterem Push. | `cargo test --manifest-path broker/Cargo.toml --color never --test store_crash_matrix -- --ignored --test-threads=1` | [OK] Exit 0 | 12,76 s | [A4-SI](roh/SONDE-013-fcaa55c.md#a4-si) |
| A4b | C++-PipeClient: stop/reconnect und ganze Frames sind zeitlich begrenzt, SQOS ist Identification, Peerbytes sind UTF-8/NUL-sauber, ACK-Sequenzen streng und u64-Zaehler wire-sicher. | `eq-copilot\build\plugin\EqCopPipeClientTest_artefacts\Release\EqCopPipeClientTest.exe` | [OK] Exit 0 | 65,51 s | [A4b](roh/SONDE-013-fcaa55c.md#a4b) |
| A5 | Referenzbein (jsonschema, draft 2020-12): Schema haelt die Engine-Teilmenge ein, Textriegel deckt jede gemessene Kante, jedes Fixture wird wie im Manifest klassifiziert, jede Definition hat ein Negativfixture. Seit SONDE-013 zusaetzlich der Fassungsschritt: der aus dem Register zurueckgebaute Leser der Fassung 1 LEHNT jede Neuerung der Fassung 2 ab (drei Experimentfamilien, evidence_snapshot.ereignisse/.stereo, zwei neue Invalidierungsgruende), und die Summe der registrierten Familien stimmt mit dem oneOf ueberein. | `py -3.13 tools\eq-copilot\pruefe_v3_vertrag.py --abdeckung` | [OK] Exit 0 | 2,36 s | [A5](roh/SONDE-013-fcaa55c.md#a5) |
| A6 | Beide Bandgitter sind bytegleich zur Neuerzeugung; 221 Baender, 64 Gruppen als exakte Partition. | `py -3.13 tools\eq-copilot\erzeuge_bandgitter.py --pruefen` | [OK] Exit 0 | 0,12 s | [A6](roh/SONDE-013-fcaa55c.md#a6) |
| A7 | Quantisierungsvertrag bytegleich zur Neuerzeugung; Rundung, Saettigung und Nichtendliches als Testvektoren. | `py -3.13 tools\eq-copilot\erzeuge_quantisierung.py --pruefen` | [OK] Exit 0 | 0,13 s | [A7](roh/SONDE-013-fcaa55c.md#a7) |
| A8 | Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei. | `py -3.13 tools\eq-copilot\erzeuge_v3_fixtures.py --pruefen` | [OK] Exit 0 | 0,20 s | [A8](roh/SONDE-013-fcaa55c.md#a8) |
| A9 | Codegen-Drift ist 0: die Neugenerierung aus dem .fbs ist bytegleich zum committeten C++- und Rust-Code; flatc, C++-Header und Rust-Crate tragen dieselbe gepinnte Version; jedes Tabellenfeld traegt eine explizite Feld-ID. | `py -3.13 tools\eq-copilot\pruefe_flatc_drift.py` | [OK] Exit 0 | 0,24 s | [A9](roh/SONDE-013-fcaa55c.md#a9) |
| A10 | Binaerer Fixture-Korpus und sein MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; sechs NAK-29-Transportrelationen, band_stereo ID 10 samt saturated-/Grenzfaellen und integration_samples ID 14 (voller Rahmen, duenner Rahmen, gesendete 0 als Senderfehler) sind in beiden Lesern gedeckt. | `py -3.13 tools\eq-copilot\erzeuge_fb_fixtures.py --pruefen` | [OK] Exit 0 | 1,57 s | [A10](roh/SONDE-013-fcaa55c.md#a10) |
| A11 | Die fuenf v2-Vertraege (ipc v2, measurement v1, report v1, snapshot v3, aggregat v1) sind gueltiges JSON und gueltige JSON-Schemas; ihre $id-Familie ist eingefroren. | `py -3.13 tools\eq-copilot\pruefe_v2_schemas.py` | [OK] Exit 0 | 0,23 s | [A11](roh/SONDE-013-fcaa55c.md#a11) |
| A12 | Parameterbestand (109 IDs, §53.8) haelt den Vertrag; RFC-8785-Zahlenvektoren tragen den RFC-Text und werden von rfc8785 bestaetigt; State-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung. | `py -3.13 tools\eq-copilot\erzeuge_state_fixtures.py --pruefen` | [OK] Exit 0 | 0,23 s | [A12](roh/SONDE-013-fcaa55c.md#a12) |
| A13 | Capabilityreport FL: die zehn Bits aus §53.6 entsprechen der v3-Vertragsform und stehen so, wie die Rohdaten der Termine A und B sie tragen; jedes supported hat einen Termin, jedes unsupported seinen festen Fallback. | `py -3.13 tools\eq-copilot\pruefe_host_capabilities.py` | [OK] Exit 0 | 0,19 s | [A13](roh/SONDE-013-fcaa55c.md#a13) |
| A14 | Der gemeinsame Kern traegt keine Bundle-Identitaet (Entwurf §53.4): NakamaKern.lib wird vor der Messung geloescht und von MSBuild neu erzeugt (jede Kernquelle uebersetzt, Objekte und Lib juenger als der Zeitanker, Objektzahl = Quellzahl), enthaelt danach keinen eingefrorenen Identitaetswert aus plugin-identities-v1.json (Text in ASCII/UTF-16LE, Viercodes als Integer beider Byteordnungen, CIDs roh und COM-vertauscht) und genau die erwarteten Kernobjekte ohne JUCE-Modulobjekt; die Gegenprobe findet die Pflichtnadeln im gebauten Gen-Bundle, sonst waere das Schweigen ueber den Kern wertlos. Fehlende Voraussetzung (kein Neubau, Artefakt nicht auffindbar, --nur-messen) endet mit Exit 3, mit registriertem Befund mit 2, nie mit 0. Seit NAK-100 (30.08.2026) ohne K1b-Textscan, Ortsriegel, JUCE-Baum-Riegel und Frischewachen (Haertung NAK-103). | `py -3.13 tools\eq-copilot\pruefe_kern_identitaetsfrei.py` | [OK] Exit 0 | 4,88 s | [A14](roh/SONDE-013-fcaa55c.md#a14) |
| A17 | Installer-Manifest: keine zweite Identitaet (kein Viercode, keine Class-ID, kein Produkt- oder Bundlename ausser im Pfad), jeder Quellpfad ist der aus Ziel + Identitaetsdatei nachgerechnete Bundle-Ordner, `art` ist eine geschlossene Menge, jedes aktive Ziel genau einmal und jedes stillgelegte benannt mit lesbarer Marke, Broker aus dem Crate-Namen, Zielverzeichnisse geschuetzt, Signaturzeile ehrlich, Rueckweg samt NAK-41-Riegel und hash_art vollstaendig; jede der 15 Regeln faellt an einem verdorbenen Manifest, die konkreten Umgehungen fallen einzeln. [4] vergleicht Artefakthashes im Kanon weich und mit --release hart; [4b] berichtet den installierten Stand; [5] kreuzprueft den Ordner-Hash; [6] faehrt die produktive C++-WinVerifyTrust-/Thumbprint-Kette mit dem OS-vertrauenswuerdigen Signerzertifikat einer signierten Windows-Fixture fuer gueltig und falsch sowie einer unsigned Temp-Kopie fuer fehlend, ohne den Zertifikatsspeicher zu aendern. Jede gelesene JSON-Datei wird vor dem Zugriff strukturell geprueft; unerwartete Ausnahmen enden kontrolliert. | `py -3.13 tools\eq-copilot\pruefe_installer_manifest.py` | [OK] Exit 0 | 0,99 s | [A17](roh/SONDE-013-fcaa55c.md#a17) |
| A18 | Gegenpfad installieren<->Rueckweg gefahren (Sandbox, nichts installiert): Erstinstallation traegt moduleinfo.json, ein Tausch ersetzt statt zu mischen, -Pruefen sieht den ganzen Ordner, der Rueckweg stellt den Vorzustand bytegleich her und laesst KEIN leeres .vst3-Gehaeuse stehen, selbst angelegte Ordner verschwinden nur leer, der NAK-41-Riegel greift und zerstoert seine eigene Wiederholungsquelle nicht, eine dritte Artefaktsorte bricht ab. | `py -3.13 tools\eq-copilot\pruefe_installer_gegenpfad.py` | [OK] Exit 0 | 32,79 s | [A18](roh/SONDE-013-fcaa55c.md#a18) |
| A19 | BandGridZahlen.h ist bytegleich aus den zwei eingefrorenen Gitterfixturen erzeugt; die 64 Live-Gruppen partitionieren die 221 Feinbaender lueckenlos und ueberschneidungsfrei, und die groben Kanten sind bitgleiche Kopien feiner Kanten (kein zweites Filterbank-Gitter). | `py -3.13 tools\eq-copilot\erzeuge_bandgitter_header.py --pruefen` | [OK] Exit 0 | 0,14 s | [A19](roh/SONDE-013-fcaa55c.md#a19) |
| A20 | Envelope-Fixture-Korpus und MANIFEST bytegleich zur Neuerzeugung; keine verwaiste Datei; jede der 14 Envelope-Regeln hat mindestens ein Negativfixture. | `py -3.13 tools\eq-copilot\erzeuge_envelope_fixtures.py --pruefen` | [OK] Exit 0 | 0,31 s | [A20](roh/SONDE-013-fcaa55c.md#a20) |
| A21 | v3-Envelope unter Zufall: 20 000 Zufallspuffer bringen den Pruefer nie aus dem Tritt und JEDER angenommene Frame erfuellt jede Kopfregel; 3000 gekippte P2-Payloadbits fallen einzeln an der CRC; feindliche Laengen (0, 15, ueber der Grenze, 0xFFFFFFFF) und die u32-Grenze von 16+payload_len enden in der erwarteten Fehlerklasse, ohne Absturz und ohne Eingabeallokation in Groesse der behaupteten Laenge; 300 Runden Fragmentierung (byteweise und in Zufallshaeppchen) liefern exakt dieselben Frames; ein kaputter Frame beendet den Strom statt zu resynchronisieren; die Ratengrenze haelt unter Flut. | `cargo test --manifest-path broker/Cargo.toml --test transport_fuzz --color never` | [OK] Exit 0 | 0,33 s | [A21](roh/SONDE-013-fcaa55c.md#a21) |
| A22 | Ende-zu-Ende ueber die PROBE-Pipe, zwei Sprachen ein Draht: 32 echte C++-Sondenpaare koppeln sich am echten Rust-Listener (32 Control + 32 Telemetry, jede Telemetrieverbindung ueber link_id + challenge + gleiche runtime_nonce), fluten P2 bis die Schleuse mit Cap 2 nachweislich ersetzt, und WAEHRENDDESSEN geht kein einziger P0-Frame verloren; die P0-Antwortlatenz bleibt unter der Schranke. Keine Verbindung wird wegen Envelope, Rate oder P0-Ueberlauf geschlossen. | `py -3.13 tools\eq-copilot\pruefe_ipc_last.py` | [OK] Exit 0 | 9,94 s | [A22](roh/SONDE-013-fcaa55c.md#a22) |
| A24 | G3-SOAK Dauerlauf ueber eine ausschliessliche Probe-Pipe: 1 Main mit subscribe_session und echtem SourcesModel plus 16 Sondenpaare und 16 echte SondeProcessor am Ganzblockpfad laufen gegen den echten Rust-Coordinator, waehrend ein Viertel der Sonden langsam liest und der Broker mitten im Lauf getoetet und neu gestartet wird. Gemessen: Mitgliedschaft und fuehrendes_main driften nie, kein P0 geht ausserhalb der Neustartfenster verloren und seine ACK-p95 bleibt unter 1.000 ms, kein Mitglied wird ausserhalb der Fenster stale oder evicted, der Snapshot ist nach jedem Neustart binnen 60 s wieder vollstaendig und die alte broker_epoch kommt nie wieder, Working Set beider Prozesse waechst je Generation um hoechstens max(10 %, 16 MiB), und der Sondenpfad verliert keinen Ganzblock. Beide Programme verweigern Produktions- und Golden-Pipename mit Exit 3, gefahren vor dem Lauf. | `py -3.13 tools\eq-copilot\pruefe_session_soak.py --sonden 16 --minuten 2 --neustarts 1` | [OK] Exit 0 | 148,63 s | [A24](roh/SONDE-013-fcaa55c.md#a24) |
| A23 | SONDE-012 E-L06 integriert ueber eine ausschliessliche Probe-Pipe: synthetische C++-Quellen senden vollstaendige Messfenster durch den echten Rust-Coordinator an das echte Main-SourcesModel. Der Endpunkt ist dessen Revision samt Anzeige-Invalidierung. Bei 16 und 32 Quellen liegt p95 fuer 2048/4096 Samples unter 300 ms und fuer 16384 Samples unter 750 ms, jeweils ab dem ersten Sample des Fensters. | `py -3.13 tools\eq-copilot\pruefe_sonde012_sources_latency.py` | [OK] Exit 0 | 4,62 s | [A23](roh/SONDE-013-fcaa55c.md#a23) |
| A15 | STILLGELEGT - mass bis 28.08.2026 Nakama Suna (NkPr): Passthrough bitgleich ueber drei Samplerates und fuenf Blockgroessen, 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur passive_probe zu; speichern-laden-speichern bytegleich. | `(nicht gefahren)` | [STILLGELEGT] seit 2026-08-28 (S9b/SONDE-007c): das Ziel NakamaSuna ist stillgelegt - Suna ist in Nakama Probeeq aufgegangen (design/abnahmen/2026-08-28-suna-stilllegung-vorgezogen.md). Weder gebaut noch gefahren; die Zeile bleibt sichtbar, damit die Kanonzahl nicht still sinkt. | - | - |
| A16 | Nakama Probeeq (NkAc): heute ebenfalls Passthrough bitgleich (die EQ-DSP kommt in P6), 0 Samples Latenz, kein Tail, kein Hostparameter; Bundlevertrag laesst nur active_probe zu; speichern-laden-speichern bytegleich. Seit der T3-Nacharbeit 29.08.2026 (G1 §4.2) faehrt dasselbe Bein die Gate-7-Kette AM GEBAUTEN BUNDLE Nakama Probeeq - Klasse active_probe, denn die Sondenschale traegt genau EINE Klasse je Uebersetzung: ein sonst gueltiger Stand mit measurement_position=post_fader_contribution kommt ueber setStateInformation read-only zurueck, die Instanz bleibt neutral, die Originalbytes gehen unveraendert an den Host zurueck und ein erneutes Laden waescht nichts; auch eine bereits klassifizierte Instanz faellt beim Nachreichen desselben Standes auf neutral zurueck; Gegenprobe mit insert laedt normal. Zusaetzlich misst dasselbe Bein direkt an der oeffentlichen positionErlaubt, dass post_fader_contribution fuer alle vier Klassen abgelehnt wird - das gemeinsame Ergebnis von Capability-Vorpruefung und Klassenmatrix, ohne die beiden Haelften zu trennen. Die vollstaendige 16er-Matrix samt Bundlevertraegen misst B2. | `eq-copilot\build\plugin\EqCopProbeeqNullTest_artefacts\Release\EqCopProbeeqNullTest.exe` | [OK] Exit 0 | 0,10 s | [A16](roh/SONDE-013-fcaa55c.md#a16) |
| B1 | Bundle-Identitaet (CIDs, JUCE_VST3_CAN_REPLACE_VST2=0) eingefroren. | `eq-copilot\build\plugin\EqCopIdentityTest_artefacts\Release\EqCopIdentityTest.exe` | [OK] Exit 0 | 0,14 s | [B1](roh/SONDE-013-fcaa55c.md#b1) |
| B2 | State-Schema 2: Roundtrip bytegleich, Schema-1-Migration rein und golden, unbekanntes Major read-only mit Originalbytes, Duplicate erkennbar und aufloesbar, Host-Dirty; alle vier Klassen sperren post_fader_contribution; NAK-40-Aliasvektoren adressieren die bytegleich erhaltene Legacy-instance_id ohne Alias im State; Parametertabelle und RFC-8785-state_hash sprachuebergreifend. | `eq-copilot\build\plugin\EqCopStateMigrationTest_artefacts\Release\EqCopStateMigrationTest.exe` | [OK] Exit 0 | 1,13 s | [B2](roh/SONDE-013-fcaa55c.md#b2) |
| B3 | Hostkontext (Anwesenheit, Parameterpunkte, Buslatenz) wird gemessen, nicht geraten; Quellhash-Gate des JUCE-Patches gruen. | `eq-copilot\build\plugin\EqCopHostContextTest_artefacts\Release\EqCopHostContextTest.exe` | [OK] Exit 0 | 0,07 s | [B3](roh/SONDE-013-fcaa55c.md#b3) |
| B3b | Termin-B-Messgeraet: Passthrough bitgleich, Sprung-/Automations-/Latenzmessung inkl. Fehlalarm-Riegel, Bericht-Rueckweg, 0 Allokationen. | `eq-copilot\build\plugin\EqCopHostProbeTest_artefacts\Release\EqCopHostProbeTest.exe` | [OK] Exit 0 | 0,07 s | [B3b](roh/SONDE-013-fcaa55c.md#b3b) |
| B3c | v3-Vertrag: C++ klassifiziert JSON- und FlatBuffers-Korpus wie das Manifest (Urteil UND Verletzungsmenge), einschliesslich Boolean/RFC-6901-Discriminatoren, NAK-29 in beiden Darstellungen und band_stereo ID 10; Bandgitter und Quantisierung bitgleich. Seit SONDE-013 misst dasselbe Bein integration_samples ID 14 ueber Encoder und Leser: gesetzt kommt Bit UND Wert zurueck, Abwesenheit bleibt gueltig und ist KEINE 0, und eine gesendete 0 faellt als Senderfehler. | `eq-copilot\build\plugin\EqCopSchemaTest_artefacts\Release\EqCopSchemaTest.exe` | [OK] Exit 0 | 0,36 s | [B3c](roh/SONDE-013-fcaa55c.md#b3c) |
| B4 | StampedAudioQueue und Ein-Block-Quarantaene: Ganzblockaufnahme bitgleich ueber jeden Ringumlauf, Ueberlauf BEIDER Ringe verwirft den ganzen Block und nie eine Teilmenge, Oversize ueber der Slotkapazitaet faellt fuer die Analyse und laesst Audio unberuehrt, Flush (numSamples 0) ist kein Verlust, Mono dupliziert L, Mehrfach-Tap-Layout traegt; Quarantaene versiegelt erst mit bewiesener Fortsetzung, Seek und Transportkante verwerfen genau EINEN Block, stehende Projektzeit (FL-Teilpuffer) ist kein Bruch, Projektzeit-Ueberlauf und negative Zeit sind behandelt; Worker-Publikation folgt monotonen 50-/250-ms-Deadlines statt Batchzahl, holt nach Pausen nicht auf und laesst wartende Reset-/Frame-Aufrufer vor; verdrahtet: Passthrough bitgleich ueber 18 Blockgroessen von 1 bis 16384, 0 Samples Latenz, kein Tail, 0 Allokationen im Audiothread ueber 4000 Bloecke wechselnder Groesse mit Transportkanten, und die Engine bekommt den Strom bis auf den Block in Quarantaene. | `eq-copilot\build\plugin\EqCopQueueStressTest_artefacts\Release\EqCopQueueStressTest.exe` | [OK] Exit 0 | 3,42 s | [B4](roh/SONDE-013-fcaa55c.md#b4) |
| B9 | Fixed-memory Loudness (§48.1): der LoudnessAccumulator deckt sich mit der ausgebauten Vektorrechnung innerhalb ±0,1 LU (Entwurf §49) ueber konstante, rampende, zufaellige und einstuendige Korpora sowie ueber Stille unter dem absoluten Gate; Kurz-LUFS ist BITGLEICH; ein adversarialer Sweep legt 1000 Bloecke in den Grenzbin des relativen Gates und die selbstgemeldete Schranke unsicherheitLu() deckt jeden Lauf; eine Million Zellen laufen mit 0 Allokationen durch, waehrend die Gegenprobe (alte Rechnung) allozert; NaN/Inf-Zellen sind gezaehlt statt still als 0 verbucht und l_j == -70,0 exakt bleibt wertgleich; ueber dem Feingitter traegt ein OBERBAND aus Bins von 1 LU bis ueber lautheit(DBL_MAX) - mit ZWEI Pegeln darin (Korpus des T2-Pruefers), einem adversarialen Sweep im Oberband-Grenzbin, beiden Richtungen der Naht zwischen den Aufloesungen und der Gegenprobe, dass kein Block durch das Raster faellt. | `eq-copilot\build\plugin\EqCopLoudnessGoldenTest_artefacts\Release\EqCopLoudnessGoldenTest.exe` | [OK] Exit 0 | 0,13 s | [B9](roh/SONDE-013-fcaa55c.md#b9) |
| B5 | FeatureEngine v2 haelt Zeit-, Validity-, Event- und Bandvertraege: Bandgitter und alle 61 Quantisierungsvektoren bitgleich zum v3-Vertrag, Bitmap LSB-first mit Fuellbits 0, FFT gegen Parseval und einen Sinus auf der Binmitte, K-Gewichtung ueber 20 Hz..20 kHz unter 0,1 dB an der BS.1770-Referenzkette; Drop/Seek(laufend UND gestoppt)/Loop-Wrap/moeglicher Straddle/Transportkante/Sampleratewechsel/Neuanlauf/Beweislagewechsel trennen JEDES offene Fenster - auch den K-Filterzustand, bitgleich gemessen - waehrend FL-Teilstuecke mit stehender Projektzeit lokal weiterlaufen, aber kein unbewiesenes Projektintervall oder FFT-Event erzeugen; Frame-Stempel umfassen den echten ueberlappenden FFT-Support und bleiben an int64/u32-Grenzen darstellbar; Drop zaehlt als Segment, alles andere als Epoche; alle sieben Erzeuger-Stempelregeln fallen einzeln und mit eigener Nummer; kein spektraler Fluss ueberbrueckt eine Grenze, der Ereignisring ist fest gedeckelt; LUFS-S trifft die analytisch gerechnete Erwartung unter 0,1 LU; die Nyquist-Kappe greift bei 22,05 kHz wirklich; verdrahtet: alle sieben Gueltigkeitsbits kommen ueber die Hostbruecke durch, der Playhead-Rueckfallweg nachweislich nur zwei. | `eq-copilot\build\plugin\EqCopAnalysisGoldenTest_artefacts\Release\EqCopAnalysisGoldenTest.exe` | [OK] Exit 0 | 2,10 s | [B5](roh/SONDE-013-fcaa55c.md#b5) |
| B6 | Aktiver DSP-Kern liefert die eingefrorene Referenzantwort. | `eq-copilot\build\plugin\EqCopDspGoldenTest_artefacts\Release\EqCopDspGoldenTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B7 | Apply/Revert ist transaktional - kein halber Zustand ueberlebt. | `eq-copilot\build\plugin\EqCopTransactionTest_artefacts\Release\EqCopTransactionTest.exe` | [GEPLANT] geplant (ab P6) | - | - |
| B10 | C++-v3-Vertrag und SONDE-011 Phase B: Envelope/CRC/Pipetoken und begrenzte P0/P1/P2-Politiken bleiben gruen; das In-Flight-Register gibt persistente Befehle erst bei angewandt, idempotent_wiederholt oder endgueltigem Fehler frei und reiht vor ACK dieselbe command_id wieder ein; Autostart verbindet zuerst, prueft Manifest-SHA-256 und bei gesetztem Thumbprint WinVerifyTrust plus Signer, spawnt verborgen und mutex-idempotent, haelt Backoff/Timeout/Cooldown-Grenzen und beendet keinen Brokerprozess. Seit SONDE-013 nimmt die C++-Vertragsengine die drei Experimentfamilien aus den committeten Fixtures an, lehnt einen fremden execution_mode an derselben Engine ab, und ein experiment_manual_result laeuft als persistenzpflichtiger P0-Befehl durch das In-Flight-Register wie jede andere steuernde Nachricht. | `eq-copilot\build\plugin\EqCopIpcTest_artefacts\Release\EqCopIpcTest.exe` | [ROT] Exit 1 | 211,64 s | [B10](roh/SONDE-013-fcaa55c.md#b10) |
| B11 | Probeeq uebernimmt JUCE updateTrackProperties ausschliesslich auf dem Message-Thread; gueltige Hostnamen reisen codepointgetreu in heartbeat.runtime, leer/Whitespace/121 Codepoints/C0/C1 gelten als Fehlen; Hostvorrang veraendert das gespeicherte User-Label nicht, Clear stellt dessen Rueckfall her. Persistierte Messpunkte insert/pre/post und Betrieb active/suspended/offline reisen ohne Synthese; der lokale C++-Pfad behauptet nicht, dass FL den Callback liefert. | `eq-copilot\build\plugin\EqCopSonde012HostChannelContextTest_artefacts\Release\EqCopSonde012HostChannelContextTest.exe` | [OK] Exit 0 | 0,12 s | [B11](roh/SONDE-013-fcaa55c.md#b11) |
| B12 | Probeeq fuehrt Audio ueber die vorallokierte Ganzblockqueue zum Analyseworker und P2: LUFS-I reist nur atomar mit endlicher unsicherheitLu und ohne Status; collecting/gated reisen ohne Zahlenpaar; halbe, nichtendliche oder statusbehaftete Paare werden nicht serialisiert. Seek, Loop, Oversize-Drop und Kontinuitaetsbruch starten eine leere Reihe; der gemessene Audiopfad alloziert dabei 0-mal und verwirft bei Ueberlast nur Analyse. | `eq-copilot\build\plugin\EqCopSonde012LoudnessSourceTest_artefacts\Release\EqCopSonde012LoudnessSourceTest.exe` | [OK] Exit 0 | 1,54 s | [B12](roh/SONDE-013-fcaa55c.md#b12) |
| B13 | Gen/Main ersetzt sein Quellenmodell durch absolute session_snapshot-Sichten und haelt Mitgliedschaft, Control, Messung, Namensherkunft und Capability-Evidenz getrennt; Frischegrenzen, Messpunkte, Betrieb, Fehlerkanal, Lautheitspaar, Hostvorrang, Sortierung und Hauptziel werden einzeln gemessen. Die integrierte 16-/32-Quellen-Zeitmessung gehoert A23. | `eq-copilot\build\plugin\EqCopSonde012SourcesModelTest_artefacts\Release\EqCopSonde012SourcesModelTest.exe` | [OK] Exit 0 | 0,11 s | [B13](roh/SONDE-013-fcaa55c.md#b13) |
| B14 | MainProject Save/Load erhaelt bestaetigte Quellidentitaet und User-Label, aber weder Control-Liveness noch Runtime-Nonce oder Messframe; Join, Benennen und Unbind melden je echte persistente Aenderung Host-Dirty, No-op, Save und Load nicht; Reload baut einen frischen subscribe_session-Auftrag. | `eq-copilot\build\plugin\EqCopSonde012ProjectReloadTest_artefacts\Release\EqCopSonde012ProjectReloadTest.exe` | [OK] Exit 0 | 0,19 s | [B14](roh/SONDE-013-fcaa55c.md#b14) |
| B15 | EqCopShot rendert den deklarierten SONDE-012-Sichtsatz bei exakt 760x430: 0/1/16 Quellen, fresh/partial/stale/disconnected/invalid, Namens- und Lautheitsgegenpfade, alle Diagnosezustaende, Label-/Unnamed-Rueckfall, Findings, genau ein Hauptziel mit Referenzen sowie das deaktivierte Label-Feld eines nicht fuehrenden Main. | `eq-copilot\build\plugin\EqCopShot_artefacts\Release\EqCopShot.exe --sonde012-suite eq-copilot/build/sonde012-shots` | [OK] Exit 0 | 2,18 s | [B15](roh/SONDE-013-fcaa55c.md#b15) |
| B8 | Lifecycle-Klassifikation §53.5 bleibt erhalten; SONDE-011 startet den Broker nur ueber state::Lebenslauf::darfBrokerStarten() bei Main plus offenem Editor. Alle Negativzustaende lassen den Launcher unberuehrt, und die instrumentierte Gegenprobe misst null Broker-Lifecycle-Aufrufe aus processBlock beziehungsweise dem Audiothread. | `eq-copilot\build\plugin\EqCopLebenslaufTest_artefacts\Release\EqCopLebenslaufTest.exe` | [OK] Exit 0 | 0,12 s | [B8](roh/SONDE-013-fcaa55c.md#b8) |

